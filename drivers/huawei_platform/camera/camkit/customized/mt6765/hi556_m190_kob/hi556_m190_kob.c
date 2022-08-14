

#include "camkit_driver_impl.h"
#include "camkit_sensor_i2c.h"
#include <securec.h>
#include <linux/delay.h>

#define RETRY_TIMES 3

#define OTP_CHECK_SUCCESSED 1
#define OTP_CHECK_FAILED 0

#define LSC_DATA_SIZE 1870
#define AWB_DATA_SIZE 9
#define MODULE_INFO_SIZE 7
#define AF_DATA_SIZE 5
#define SN_DATA_SIZE 25
/* 0~4:af_data 5:af_flag 6~30:sn_data 31:sn_flag 32~40:awb_data 41:awb_flag 42~1908:lsc_data 1909:lsc_flag*/
#define OTP_DATA_SIZE 1911

static unsigned char otp_have_read;

unsigned char hi556_data_af[AF_DATA_SIZE + 1] = {0}; /* Add checksum */
unsigned char hi556_data_lsc[LSC_DATA_SIZE + 1] = {0}; /* Add checksum */
unsigned char hi556_data_awb[AWB_DATA_SIZE + 1] = {0}; /* Add checksum */
unsigned char hi556_data_info[MODULE_INFO_SIZE + 1] = {0}; /* Add checksum */
unsigned char hi556_data_sn[SN_DATA_SIZE + 1] = {0}; /* Add checksum */
unsigned char hi556_module_id = 0;
unsigned char hi556_lsc_valid = 0;
unsigned char hi556_awb_valid = 0;
unsigned char hi556_af_valid = 0;
unsigned char hi556_sn_valid = 0;

static uint16 read_cmos_sensor(struct camkit_sensor_ctrl_t *sensor, uint32 addr)
{
	uint16 get_byte = 0;
	int rc = camkit_sensor_i2c_read(sensor, addr, &get_byte, CAMKIT_I2C_BYTE_DATA);
	if (rc != ERR_NONE)
		log_err("read failed, addr: 0x%x\n", addr);

	return get_byte;
}

static void write_cmos_sensor(struct camkit_sensor_ctrl_t *sensor,
		uint32 addr, uint32 para)
{
	int rc = camkit_sensor_i2c_write(sensor, addr, para, CAMKIT_I2C_WORD_DATA);
	if (rc != ERR_NONE)
		log_err("write failed, addr: 0x%x, 0x%x\n", addr, para);
}

static void write_cmos_sensor_8(struct camkit_sensor_ctrl_t *sensor,
		uint32 addr, uint32 para)
{
	int rc = camkit_sensor_i2c_write(sensor, addr, para, CAMKIT_I2C_BYTE_DATA);
	if (rc != ERR_NONE)
		log_err("write failed, addr: 0x%x, 0x%x\n", addr, para);
}

int camcal_read_insensor_hi556_func(unsigned char *eeprom_buff,
			    int eeprom_size)
{
	unsigned char * buffer_temp = eeprom_buff;
	if (!eeprom_buff || eeprom_size <= 0)
		return -1;

	if (hi556_af_valid)
		(void)memcpy_s(buffer_temp, AF_DATA_SIZE + 1,
			hi556_data_af, AF_DATA_SIZE + 1);
	buffer_temp += (AF_DATA_SIZE + 1);

	if (hi556_sn_valid)
		(void)memcpy_s(buffer_temp, SN_DATA_SIZE + 1,
			hi556_data_sn, SN_DATA_SIZE + 1);
	buffer_temp += (SN_DATA_SIZE + 1);

	if (hi556_awb_valid)
		(void)memcpy_s(buffer_temp, AWB_DATA_SIZE + 1,
			hi556_data_awb, AWB_DATA_SIZE + 1);
	buffer_temp += (AWB_DATA_SIZE + 1);

	if (hi556_lsc_valid)
		(void)memcpy_s(buffer_temp, LSC_DATA_SIZE + 1,
			hi556_data_lsc, LSC_DATA_SIZE + 1);
	buffer_temp += (LSC_DATA_SIZE + 1);

	log_info("hi556 driver read otp success!\n");
	return OTP_DATA_SIZE;
}

static void hi556_disable_otp_func(struct camkit_sensor_ctrl_t *sensor)
{
	write_cmos_sensor(sensor, 0x0a00, 0x00);
	mdelay(10);
	write_cmos_sensor(sensor, 0x003e, 0x00);
	write_cmos_sensor(sensor, 0x0a00, 0x00);
}

static int read_hi556_module_info(struct camkit_sensor_ctrl_t *sensor)
{
	int otp_grp_flag = 0, minfo_start_addr = 0;
	int year = 0, month = 0, day = 0;
	int position = 0,lens_id = 0,vcm_id = 0;
	int check_sum = 0, check_sum_cal = 0, i = 0;
	write_cmos_sensor_8(sensor, 0x010a,((0x0401)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(0x0401)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	otp_grp_flag = read_cmos_sensor(sensor, 0x0108);
	if (otp_grp_flag == 0x01) {
		minfo_start_addr = 0x0402;
	} else if (otp_grp_flag == 0x13) {
		minfo_start_addr = 0x040a;
	} else if (otp_grp_flag == 0x37) {
		minfo_start_addr = 0x0412;
	} else{
		log_err("no OTP hi556_data_info\n");
		return 0;
	}

	if (minfo_start_addr != 0) {
		write_cmos_sensor_8(sensor, 0x010a,((minfo_start_addr)>>8)&0xff);
		write_cmos_sensor_8(sensor, 0x010b,(minfo_start_addr)&0xff);
		write_cmos_sensor_8(sensor, 0x0102,0x01);
		for (i = 0; i < MODULE_INFO_SIZE + 1; i++)
			hi556_data_info[i]=read_cmos_sensor(sensor, 0x0108);

		for (i = 0; i < MODULE_INFO_SIZE; i++)
			check_sum_cal += hi556_data_info[i];

		check_sum_cal += otp_grp_flag;
		check_sum_cal = (check_sum_cal % 255) + 1;
		hi556_module_id = hi556_data_info[0];
		position = hi556_data_info[1];
		lens_id = hi556_data_info[2];
		vcm_id = hi556_data_info[3];
		year = hi556_data_info[4];
		month = hi556_data_info[5];
		day = hi556_data_info[6];
		check_sum = hi556_data_info[MODULE_INFO_SIZE];
	}
	log_info("=== HI556 INFO module_id=0x%x position=0x%x ===\n", hi556_module_id, position);
	log_info("=== HI556 INFO lens_id=0x%x,vcm_id=0x%x date is %d-%d-%d ===\n",lens_id, vcm_id,year,month,day);
	log_info("=== HI556 INFO check_sum=0x%x,check_sum_cal=0x%x ===\n", check_sum, check_sum_cal);
	if (check_sum == check_sum_cal)
		return 1;
	else
		return 0;
}

static int read_hi556_sn_info(struct camkit_sensor_ctrl_t *sensor)
{
	int otp_grp_flag = 0, sn_start_addr = 0;
	int check_sum_sn = 0, check_sum_sn_cal = 0;
	int i;

	/* lsc group 1 */
	write_cmos_sensor_8(sensor, 0x010a,((0x041a)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(0x041a)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	otp_grp_flag = read_cmos_sensor(sensor, 0x0108);
	log_info("sn_info_grp1_flag = 0x%x\n",otp_grp_flag);
	if (otp_grp_flag == 0x01) {
		sn_start_addr = 0x041b;
	} else if (otp_grp_flag == 0x13) {
		sn_start_addr = 0x0435;
	} else if (otp_grp_flag == 0x37) {
		sn_start_addr = 0x044f;
	} else {
		log_err("invalid module,error sn flag = 0x%x\n", otp_grp_flag);
		hi556_data_sn[SN_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}

	if (sn_start_addr != 0) {
		write_cmos_sensor_8(sensor, 0x010a,((sn_start_addr)>>8)&0xff);
		write_cmos_sensor_8(sensor, 0x010b,(sn_start_addr)&0xff);
		write_cmos_sensor_8(sensor, 0x0102,0x01);
		for (i = 0; i < SN_DATA_SIZE + 1; i++) {
			hi556_data_sn[i] = read_cmos_sensor(sensor, 0x0108);
			log_info("hi556_data_sn[%d]=0x%x\n",i,hi556_data_sn[i]);
		}

		for (i = 0; i < SN_DATA_SIZE; i++)
			check_sum_sn_cal += hi556_data_sn[i];

		check_sum_sn_cal += otp_grp_flag;
		log_info("check_sum_sn_cal =0x%x \n",check_sum_sn_cal);
		check_sum_sn = hi556_data_sn[SN_DATA_SIZE];
		check_sum_sn_cal = (check_sum_sn_cal % 255) + 1;
	}

	log_info("=== HI556 check_sum_sn=0x%x, check_sum_sn_cal=0x%x ===\n", check_sum_sn, check_sum_sn_cal);
	if (check_sum_sn == check_sum_sn_cal) {
		hi556_data_sn[SN_DATA_SIZE] = OTP_CHECK_SUCCESSED;
		return 1;
	} else {
		log_err("invalid module,sn checksum failed\n");
		hi556_data_sn[SN_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}
}

#define AWB_BLOCK_DATA_SIZE 17

static int read_hi556_awb_data(struct camkit_sensor_ctrl_t *sensor, int awb_start_addr, int otp_grp_flag)
{
	int check_sum_awb = 0, check_sum_awb_cal = 0;
	int r ,b ,gr, gb, golden_r, golden_b, golden_gr, golden_gb, i = 0;
	unsigned char awb_block[AWB_BLOCK_DATA_SIZE] = {0};

	write_cmos_sensor_8(sensor, 0x010a,((awb_start_addr)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(awb_start_addr)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	for (i = 0; i < AWB_BLOCK_DATA_SIZE; i++) {
		awb_block[i]=read_cmos_sensor(sensor, 0x0108);
		if (i < AWB_BLOCK_DATA_SIZE-1)
			check_sum_awb_cal += awb_block[i];
		log_info("=== awb_block[%d]=0x%x\n", i, awb_block[i]);
	}

	check_sum_awb_cal += otp_grp_flag;
	log_info("check_sum_awb_cal =0x%x \n",check_sum_awb_cal);
	r = ((awb_block[1]<<8)&0xff00)|(awb_block[0]&0xff);
	b = ((awb_block[3]<<8)&0xff00)|(awb_block[2]&0xff);
	gr = ((awb_block[5]<<8)&0xff00)|(awb_block[4]&0xff);
	gb = ((awb_block[7]<<8)&0xff00)|(awb_block[6]&0xff);
	golden_r = ((awb_block[9]<<8)&0xff00)|(awb_block[8]&0xff);
	golden_b = ((awb_block[11]<<8)&0xff00)|(awb_block[10]&0xff);
	golden_gr = ((awb_block[13]<<8)&0xff00)|(awb_block[12]&0xff);
	golden_gb = ((awb_block[15]<<8)&0xff00)|(awb_block[14]&0xff);
	check_sum_awb = awb_block[AWB_BLOCK_DATA_SIZE-1];
	check_sum_awb_cal = (check_sum_awb_cal % 255) + 1;

	log_info("=== HI556 AWB r=0x%x, b=0x%x, gr=%x, gb=0x%x ===\n", r, b,gb, gr);
	log_info("=== HI556 AWB gr=0x%x,gb=0x%x,gGr=%x, gGb=0x%x ===\n", golden_r, golden_b, golden_gr, golden_gb);
	log_info("=== HI556 AWB check_sum_awb=0x%x,check_sum_awb_cal=0x%x ===\n",check_sum_awb,check_sum_awb_cal);

	/*
	 * Actually, high byte is not used, because the sensor calibrated data type is raw8,
	 * r, g, b's max val is 255.
	 * So only upload the low byte to camcal module.
	 **/
	hi556_data_awb[0] = 1; /* awb flag */
	for (i = 0; i < (AWB_DATA_SIZE - 1); i++) {
		hi556_data_awb[i + 1] = awb_block[i * 2];
		log_info("=== hi556_data_awb[%d]=0x%x\n", i, hi556_data_awb[i]);
	}

	if (check_sum_awb == check_sum_awb_cal) {
		hi556_data_awb[AWB_DATA_SIZE] = OTP_CHECK_SUCCESSED;
		return 1;
	} else {
		log_err("invalid module,awb checksum failed \n");
		hi556_data_awb[AWB_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}
}

static int read_hi556_awb_info(struct camkit_sensor_ctrl_t *sensor)
{
	int otp_grp_flag = 0, awb_start_addr = 0;
	int ret = 0;

	write_cmos_sensor_8(sensor, 0x010a,((0x0469)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(0x0469)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	otp_grp_flag = read_cmos_sensor(sensor, 0x0108);
	log_info("awb_info otp_grp_flag = 0x%x\n",otp_grp_flag);
	if (otp_grp_flag == 0x01) {
		awb_start_addr = 0x046a;
	} else if (otp_grp_flag == 0x13) {
		awb_start_addr = 0x047b;
	} else if (otp_grp_flag == 0x37) {
		awb_start_addr = 0x048c;
	} else {
		log_err("invalid module,error awb flag = 0x%x\n", otp_grp_flag);
		hi556_data_awb[AWB_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}

	if (awb_start_addr != 0)
		ret = read_hi556_awb_data(sensor, awb_start_addr, otp_grp_flag);

	return ret;
}

static int read_hi556_lsc_info(struct camkit_sensor_ctrl_t *sensor)
{
	int otp_grp_flag = 0, lsc_start_addr = 0;
	int check_sum_lsc = 0, check_sum_lsc_cal = 0;
	int i;

	/* lsc group 1 */
	write_cmos_sensor_8(sensor, 0x010a,((0x049d)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(0x049d)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	otp_grp_flag = read_cmos_sensor(sensor, 0x0108);
	log_info("lsc_info otp_grp_flag = 0x%x\n",otp_grp_flag);
	if (otp_grp_flag == 0x01) {
		lsc_start_addr = 0x049e;
	} else if (otp_grp_flag == 0x13) {
		lsc_start_addr = 0x0beb;
	} else if (otp_grp_flag == 0x37) {
		lsc_start_addr = 0x1338;
	} else {
		log_err("invalid module,error lsc flag = 0x%x\n", otp_grp_flag);
		hi556_data_lsc[LSC_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}

	hi556_data_lsc[0] = otp_grp_flag; /* lsc flag */
	if (lsc_start_addr != 0) {
		write_cmos_sensor_8(sensor, 0x010a,((lsc_start_addr)>>8)&0xff);
		write_cmos_sensor_8(sensor, 0x010b,(lsc_start_addr)&0xff);
		write_cmos_sensor_8(sensor, 0x0102,0x01);
		for (i = 1; i < LSC_DATA_SIZE; i++)
			hi556_data_lsc[i] = read_cmos_sensor(sensor, 0x0108);

		for (i = 1; i < LSC_DATA_SIZE - 1; i++) {
			check_sum_lsc_cal += hi556_data_lsc[i];
			drv_dbg("hi556_data_lsc[%d]:0x%x \n", i, hi556_data_lsc[i]);
		}
		check_sum_lsc_cal += otp_grp_flag;
		log_info("check_sum_lsc_cal =0x%x \n",check_sum_lsc_cal);
		check_sum_lsc = hi556_data_lsc[LSC_DATA_SIZE - 1];
		check_sum_lsc_cal = (check_sum_lsc_cal % 255) + 1;
	}
	log_info("=== HI556 check_sum_lsc=0x%x, check_sum_lsc_cal=0x%x ===\n", check_sum_lsc, check_sum_lsc_cal);
	if (check_sum_lsc == check_sum_lsc_cal) {
		hi556_data_lsc[LSC_DATA_SIZE - 1] = OTP_CHECK_SUCCESSED;
		hi556_data_lsc[LSC_DATA_SIZE] = OTP_CHECK_SUCCESSED;
		return 1;
	} else {
		log_err("invalid module,lsc checksum failed \n");
		hi556_data_lsc[LSC_DATA_SIZE - 1] = OTP_CHECK_FAILED;
		hi556_data_lsc[LSC_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}
}

static int read_hi556_af_info(struct camkit_sensor_ctrl_t *sensor)
{
	int otp_grp_flag = 0, af_start_addr = 0;
	int check_sum_af = 0, check_sum_af_cal = 0;
	int i;
	unsigned char temp_data_af[AF_DATA_SIZE + 1] = {0};

	/* lsc group 1 */
	write_cmos_sensor_8(sensor, 0x010a,((0x1a85)>>8)&0xff);
	write_cmos_sensor_8(sensor, 0x010b,(0x1a85)&0xff);
	write_cmos_sensor_8(sensor, 0x0102,0x01);
	otp_grp_flag = read_cmos_sensor(sensor, 0x0108);
	log_info("af_info_grp1_flag = 0x%x\n",otp_grp_flag);
	if (otp_grp_flag == 0x01) {
		af_start_addr = 0x1a86;
	} else if (otp_grp_flag == 0x13) {
		af_start_addr = 0x1a8c;
	} else if (otp_grp_flag == 0x37) {
		af_start_addr = 0x1a92;
	} else {
		log_err("invalid module,error sn flag = 0x%x\n", otp_grp_flag);
		hi556_data_af[AF_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}

	if (af_start_addr != 0) {
		write_cmos_sensor_8(sensor, 0x010a,((af_start_addr)>>8)&0xff);
		write_cmos_sensor_8(sensor, 0x010b,(af_start_addr)&0xff);
		write_cmos_sensor_8(sensor, 0x0102,0x01);
		for (i = 0; i < AF_DATA_SIZE + 1; i++) {
			temp_data_af[i] = read_cmos_sensor(sensor, 0x0108);
			log_info("temp_data_af[%d]=0x%x\n",i,temp_data_af[i]);
		}

		for (i = 0; i < AF_DATA_SIZE; i++)
			check_sum_af_cal += temp_data_af[i];

		/* swap the LSB and MSB */
		hi556_data_af[0] = temp_data_af[0];
		for (i = 1; i < AF_DATA_SIZE; i++) {
			hi556_data_af[i] = temp_data_af[AF_DATA_SIZE - i];
			log_info("hi556_data_af[%d]=0x%x\n",i,hi556_data_af[i]);
		}

		check_sum_af_cal += otp_grp_flag;
		log_info("check_sum_af_cal =0x%x \n",check_sum_af_cal);
		check_sum_af = temp_data_af[AF_DATA_SIZE];
		check_sum_af_cal = (check_sum_af_cal % 255) + 1;
	}
	log_info("=== HI556 af check_sum=0x%x, check_sum_af_cal=0x%x ===\n", check_sum_af, check_sum_af_cal);
	if (check_sum_af == check_sum_af_cal) {
		hi556_data_af[AF_DATA_SIZE] = OTP_CHECK_SUCCESSED;
		return 1;
	} else {
		log_err("invalid module,checksum failed \n");
		hi556_data_af[AF_DATA_SIZE] = OTP_CHECK_FAILED;
		return 0;
	}
}

static void hi556_sensor_init_settings(struct camkit_sensor_ctrl_t *sensor)
{
	/* 1. sensor init */
	write_cmos_sensor(sensor, 0x0e00, 0x0102); /* tg_pmem_sckpw/sdly */
	write_cmos_sensor(sensor, 0x0e02, 0x0102); /* tg_pmem_sckpw/sdly */
	write_cmos_sensor(sensor, 0x0e0c, 0x0100); /* tg_pmem_rom_dly */
	write_cmos_sensor(sensor, 0x27fe, 0xe000); /* firmware start address-ROM */
	write_cmos_sensor(sensor, 0x0b0e, 0x8600); /* BGR enable */
	write_cmos_sensor(sensor, 0x0d04, 0x0100); /* STRB(OTP Busy) output enable */
	write_cmos_sensor(sensor, 0x0d02, 0x0707); /* STRB(OTP Busy) output drivability */
	write_cmos_sensor(sensor, 0x0f30, 0x6825); /* Analog PLL setting */
	write_cmos_sensor(sensor, 0x0f32, 0x7067); /* Analog CLKGEN setting */
	write_cmos_sensor(sensor, 0x0f02, 0x0106); /* PLL enable */
	write_cmos_sensor(sensor, 0x0a04, 0x0000); /* mipi disable */
	write_cmos_sensor(sensor, 0x0e0a, 0x0001); /* TG PMEM CEN anable */
	write_cmos_sensor(sensor, 0x004a, 0x0100); /* TG MCU enable */
	write_cmos_sensor(sensor, 0x003e, 0x1000); /* ROM OTP Continuous W/R mode enable */
	write_cmos_sensor(sensor, 0x0a00, 0x0100); /* Stream ON */

	/* 2. init OTP setting*/
	write_cmos_sensor_8(sensor, 0x0A02, 0x01); /* Fast sleep on */
	write_cmos_sensor_8(sensor, 0x0A00, 0x00);/* stand by on */
	mdelay(10);
	write_cmos_sensor_8(sensor, 0x0f02, 0x00);/* pll disable */
	write_cmos_sensor_8(sensor, 0x011a, 0x01);/* CP TRIM_H */
	write_cmos_sensor_8(sensor, 0x011b, 0x09);/* IPGM TRIM_H */
	write_cmos_sensor_8(sensor, 0x0d04, 0x01);/* Fsync(OTP busy)Output Enable */
	write_cmos_sensor_8(sensor, 0x0d00, 0x07);/* Fsync(OTP busy)Output Drivability */
	write_cmos_sensor_8(sensor, 0x003e, 0x10);/* OTP r/w mode */
	write_cmos_sensor_8(sensor, 0x0a00, 0x01);/* standby off */
}


static int hi556_sensor_otp_info(struct camkit_sensor_ctrl_t *sensor)
{
	int ret = 0;

	log_info("come to E!\n");

	if (otp_have_read) {
		log_info("otp have been read\n");
		return 0;
	}
	hi556_sensor_init_settings(sensor);

	/* 3. read eeprom data */
	/* minfo && awb group */
	ret = read_hi556_module_info(sensor);
	if (ret != 1) {
		hi556_module_id = 0;
		log_err("=== hi556_data_module invalid ===\n");
	}
	ret = read_hi556_sn_info(sensor);
	if (ret != 1) {
		hi556_sn_valid = 0;
		log_err("=== hi556_data_sn invalid ===\n");
	} else {
		hi556_sn_valid = 1;
	}

	ret = read_hi556_awb_info(sensor);
	if (ret != 1) {
		hi556_awb_valid = 0;
		log_err("=== hi556_data_awb invalid ===\n");
	} else {
		hi556_awb_valid = 1;
	}

	ret = read_hi556_lsc_info(sensor);
	if (ret != 1) {
		hi556_lsc_valid = 0;
		log_err("=== hi556_data_lsc invalid ===\n");
	} else {
		hi556_lsc_valid = 1;
	}

	ret = read_hi556_af_info(sensor);
	if (ret != 1) {
		hi556_af_valid = 0;
		log_err("=== hi556_data_af invalid ===\n");
	} else {
		hi556_af_valid = 1;
	}

	/* 4. disable otp function */
	hi556_disable_otp_func(sensor);
	otp_have_read = 1;
	if (hi556_module_id == 0 || hi556_lsc_valid == 0 || hi556_awb_valid == 0 ||hi556_af_valid == 0) {
		return 0;
	} else {
		return 1;
	}
}

static uint32 match_sensor_id(struct camkit_params *params)
{
	int32 rc;
	uint8 i = 0;
	uint8 retry = RETRY_TIMES;
	uint8 size;
	uint16 sensor_id = 0;
	struct camkit_sensor_params *sensor_params = params->sensor_params;
	struct camkit_module_params *module_params = params->module_params;
	struct camkit_sensor_info_t *sensor_info = &sensor_params->sensor_info;
	uint16 expect_id = sensor_info->sensor_id;
	enum camkit_i2c_data_type data_type = sensor_info->sensor_id_dt;

	spin_lock(&camkit_lock);
	/* init i2c config */
	sensor_params->sensor_ctrl.i2c_speed = sensor_info->i2c_speed;
	sensor_params->sensor_ctrl.addr_type = sensor_info->addr_type;
	spin_unlock(&camkit_lock);

	size = camkit_array_size(sensor_info->i2c_addr_table);
	log_info("sensor i2c addr num = %u", size);

	while ((i < size) && (sensor_info->i2c_addr_table[i] != 0xff)) {
		spin_lock(&camkit_lock);
		sensor_params->sensor_ctrl.i2c_write_id =
			sensor_info->i2c_addr_table[i];
		spin_unlock(&camkit_lock);
		do {
			log_info("to match sensor: %s", module_params->sensor_name);

			if (!data_type)
				rc = camkit_sensor_i2c_read(&sensor_params->sensor_ctrl,
					sensor_info->sensor_id_reg,
					&sensor_id, CAMKIT_I2C_WORD_DATA);
			else
				rc = camkit_sensor_i2c_read(&sensor_params->sensor_ctrl,
					sensor_info->sensor_id_reg,
					&sensor_id, data_type);
			if (rc == ERR_NONE && sensor_id == expect_id) {
				log_info("sensor id: 0x%x matched", sensor_id);
				return ERR_NONE;
			}
			retry--;
		} while (retry > 0);
		i++;
		retry = RETRY_TIMES;
	}

	log_info("sensor id mismatch, expect:0x%x, real:0x%x",
		expect_id, sensor_id);

	return ERR_IO;
}

static uint32 hi556_m190_kob_match_id(struct camkit_sensor *sensor,
	uint32 *match_id)
{
	uint32 rc;
	struct camkit_params *kit_params = NULL;
	struct camkit_sensor_params *sensor_params = NULL;

	return_err_if_null(sensor);
	kit_params = sensor->kit_params;

	return_err_if_null(kit_params);
	return_err_if_null(kit_params->sensor_params);
	return_err_if_null(kit_params->module_params);

	rc = match_sensor_id(kit_params);
	if (rc != ERR_NONE) {
		*match_id = 0xFFFFFFFF;
		return rc;
	}

	sensor_params = kit_params->sensor_params;
	hi556_sensor_otp_info(&sensor_params->sensor_ctrl);

	*match_id = kit_params->module_params->match_id;
	log_info("match id ok, sensor id: 0x%x, module: 0x%x, match id: 0x%x",
		kit_params->sensor_params->sensor_info.sensor_id,
		kit_params->module_params->module_code,
		kit_params->module_params->match_id);

	return ERR_NONE;
}

static struct sensor_kit_ops hi556_m190_kob_ops = {
	.sensor_open = camkit_open,
	.sensor_close = camkit_close,
	.match_id = hi556_m190_kob_match_id,
	.sensor_init = camkit_sensor_init,
	.get_sensor_info = camkit_get_sensor_info,
	.control = camkit_control,
	.get_scenario_pclk = camkit_get_scenario_pclk,
	.get_scenario_period = camkit_get_scenario_period,
	.set_test_pattern = camkit_set_test_pattern,
	.dump_reg = camkit_dump_reg,
	.set_auto_flicker = camkit_set_auto_flicker,
	.get_default_framerate = camkit_get_default_framerate,
	.get_crop_info = camkit_get_crop_info,
	.streaming_control = camkit_streaming_control,
	.get_mipi_pixel_rate = camkit_get_mipi_pixel_rate,
	.get_mipi_trail_val = camkit_get_mipi_trail_val,
	.get_sensor_pixel_rate = camkit_get_sensor_pixel_rate,
	.get_pdaf_capacity = camkit_get_pdaf_capacity,
	.get_binning_ratio = camkit_get_binning_ratio,
	.get_pdaf_info = camkit_get_pdaf_info,
	.get_vc_info = camkit_get_vc_info,
	.get_pdaf_regs_data = camkit_get_pdaf_regs_data,
	.set_pdaf_setting = camkit_set_pdaf_setting,
	.set_video_mode = camkit_set_video_mode,
	.set_shutter = camkit_set_shutter,
	.set_gain = camkit_set_gain,
	.set_dummy = camkit_set_dummy,
	.set_max_framerate = camkit_set_max_framerate,
	.set_scenario_framerate = camkit_set_scenario_framerate,
	.set_shutter_frame_length = camkit_set_shutter_frame_length,
	.set_current_fps = camkit_set_current_fps,
	.set_pdaf_mode = camkit_set_pdaf_mode,
};


static uint32 get_hi556_m190_kob_ops(struct sensor_kit_ops **ops)
{
	if (ops != NULL) {
		*ops = &hi556_m190_kob_ops;
	} else {
		log_err("get gc5035_m200_kob_ops operators fail");
		return ERR_INVAL;
	}

	otp_have_read = 0;
	hi556_module_id = 0;
	hi556_lsc_valid = 0;
	hi556_awb_valid = 0;
	hi556_af_valid = 0;
	hi556_sn_valid = 0;
	log_info("get gc5035_m200_kob operators OK");
	return ERR_NONE;
}

register_customized_driver(
	hi556_m190_kob,
	CAMKIT_SENSOR_IDX_MAIN,
	HI556_M190_KOB_SENSOR_ID,
	get_hi556_m190_kob_ops);
