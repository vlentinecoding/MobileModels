

#include "camkit_driver_impl.h"
#include "camkit_sensor_i2c.h"
#include <securec.h>
#include <linux/delay.h>

#define RETRY_TIMES 3

#define OTP_CHECK_SUCCESSED 1
#define OTP_CHECK_FAILED 0

/* SENSOR PRIVATE INFO FOR OTP SETTINGS */
#define GC5035P_OTP_FOR_CUSTOMER            1
#define GC5035P_OTP_DEBUG                   0

/* DEBUG */
#if GC5035P_OTP_DEBUG
#define GC5035P_OTP_START_ADDR              0x0
#endif

#define GC5035P_OTP_DATA_LENGTH             1024

/* OTP FLAG TYPE */
#define GC5035P_OTP_FLAG_EMPTY              0x00
#define GC5035P_OTP_FLAG_VALID              0x01
#define GC5035P_OTP_FLAG_INVALID            0x02
#define GC5035P_OTP_FLAG_INVALID2           0x03
#define gc5035p_otp_get_offset(size)           (size << 3)
#define gc5035p_otp_get_2bit_flag(flag, bit)   ((flag >> bit) & 0x03)
#define gc5035p_otp_check_1bit_flag(flag, bit) (((flag >> bit) & 0x01) == GC5035P_OTP_FLAG_VALID)

/* 0~4:af_data 5:af_flag 6~25:sn_data 26:sn_flag */
#define GC5035P_OTP_BUF_SIZE                27

#define GC5035P_OTP_ID_SIZE                 9
#define GC5035P_OTP_ID_DATA_OFFSET          0x0020

/* OTP DPC PARAMETERS */
#define GC5035P_OTP_DPC_FLAG_OFFSET         0x0068
#define GC5035P_OTP_DPC_TOTAL_NUMBER_OFFSET 0x0070
#define GC5035P_OTP_DPC_ERROR_NUMBER_OFFSET 0x0078

/* OTP REGISTER UPDATE PARAMETERS */
#define GC5035P_OTP_REG_FLAG_OFFSET         0x0880
#define GC5035P_OTP_REG_DATA_OFFSET         0x0888
#define GC5035P_OTP_REG_MAX_GROUP           5
#define GC5035P_OTP_REG_BYTE_PER_GROUP      5
#define GC5035P_OTP_REG_REG_PER_GROUP       2
#define GC5035P_OTP_REG_BYTE_PER_REG        2
#define GC5035P_OTP_REG_DATA_SIZE           (GC5035P_OTP_REG_MAX_GROUP * GC5035P_OTP_REG_BYTE_PER_GROUP)
#define GC5035P_OTP_REG_REG_SIZE            (GC5035P_OTP_REG_MAX_GROUP * GC5035P_OTP_REG_REG_PER_GROUP)

#if GC5035P_OTP_FOR_CUSTOMER
#define GC5035P_OTP_CHECK_SUM_BYTE          1
#define GC5035P_OTP_GROUP_CNT               2

/* OTP MODULE INFO PARAMETERS */
#define GC5035P_OTP_MODULE_FLAG_OFFSET      	0x1ef0
#define GC5035P_OTP_MODULE_DATA_GROUP1_OFFSET   0x1ef8
#define GC5035P_OTP_MODULE_DATA_GROUP2_OFFSET   0x1f38
#define GC5035P_OTP_MODULE_DATA_SIZE        	8

/* OTP WB PARAMETERS */
#define GC5035P_OTP_WB_FLAG_OFFSET          	0x1f78
#define GC5035P_OTP_WB_DATA_GROUP1_OFFSET       0x1f80
#define GC5035P_OTP_WB_DATA_GROUP2_OFFSET       0x1fa0
#define GC5035P_OTP_WB_DATA_SIZE           		4
#define GC5035P_OTP_GOLDEN_DATA_GROUP1_OFFSET   0x1fc0
#define GC5035P_OTP_GOLDEN_DATA_GROUP2_OFFSET   0x1fe0
#define GC5035P_OTP_GOLDEN_DATA_SIZE        	4
#define GC5035P_OTP_WB_CAL_BASE             	0x0800
#define GC5035P_OTP_WB_GAIN_BASE            	0x0400

/* WB TYPICAL VALUE 0.5 */
#define GC5035P_OTP_WB_RG_TYPICAL           0x0400
#define GC5035P_OTP_WB_BG_TYPICAL           0x0400

/*OTP SN PARAMETERS */
#define GC5035P_OTP_SN_FLAG_OFFSET          0x1c90
#define GC5035P_OTP_SN_DATA_GROUP1_OFFSET   0x1c98
#define GC5035P_OTP_SN_DATA_GROUP2_OFFSET   0x1d90
#define GC5035P_OTP_SN_DATA_SIZE            31
#define GC5035P_OTP_SN_VALID_DATA_SIZE      20

/*OTP AF PARAMETERS */
#define GC5035P_OTP_AF_FLAG_OFFSET          0x1e88
#define GC5035P_OTP_AF_DATA_GROUP1_OFFSET   0x1e90
#define GC5035P_OTP_AF_DATA_GROUP2_OFFSET   0x1ec0
#define GC5035P_OTP_AF_DATA_SIZE            6
#endif

/* DPC STRUCTURE */
struct gc5035p_dpc_t {
	uint8 flag;
	uint16 total_num;
};

/* REGISTER UPDATE STRUCTURE */
struct gc5035p_reg_t {
	uint8 page;
	uint8 addr;
	uint8 value;
};

/* REGISTER UPDATE STRUCTURE */
struct gc5035p_reg_update_t {
	uint8 flag;
	uint8 cnt;
	struct gc5035p_reg_t reg[GC5035P_OTP_REG_REG_SIZE];
};

/* SN info STRUCTURE */
struct gc5035p_sn_info_t {
	uint8 data[GC5035P_OTP_SN_VALID_DATA_SIZE + 1];
};

/* AF info STRUCTURE */
struct gc5035p_af_info_t {
	uint8 direction;
	uint16 macro;
	uint16 infinity;
};

/* MODULE INFO STRUCTURE */
struct gc5035p_module_info_t {
	uint8 module_id;
	uint8 lens_id;
	uint8 reserved0;
	uint8 reserved1;
	uint8 year;
	uint8 month;
	uint8 day;
};

/* WB STRUCTURE */
struct gc5035p_wb_t {
	uint8  flag;
	uint16 rg;
	uint16 bg;
};

/* OTP STRUCTURE */
struct gc5035p_otp_t {
	uint8 otp_id[GC5035P_OTP_ID_SIZE];
	uint8 otp_buf[GC5035P_OTP_BUF_SIZE];
	struct gc5035p_dpc_t dpc;
	struct gc5035p_reg_update_t regs;
	struct gc5035p_wb_t wb;
	struct gc5035p_wb_t golden;
	struct gc5035p_sn_info_t sn;
	struct gc5035p_af_info_t af;
	uint8 have_read;
};

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
	int rc = camkit_sensor_i2c_write(sensor, addr, para, CAMKIT_I2C_BYTE_DATA);
	if (rc != ERR_NONE)
		log_err("write failed, addr: 0x%x, 0x%x\n", addr, para);
}

struct gc5035p_otp_t gc5035p_otp_data;
int camcal_read_insensor_gc5035p_func(unsigned char *eeprom_buff,
			    int eeprom_size)
{
	int i = 0;
	if (!eeprom_buff || eeprom_size <= 0)
		return -1;

	memcpy_s(eeprom_buff, eeprom_size, gc5035p_otp_data.otp_buf, GC5035P_OTP_BUF_SIZE);

	for (i = 0; i < GC5035P_OTP_BUF_SIZE; i++)
		log_info("gc5035_otp_buf[%d] = 0x%x\n", i, gc5035p_otp_data.otp_buf[i]);

	log_info("gc5035p driver read otp success!\n");


	return eeprom_size;
}

int camcal_read_insensor_gc5035p_qunhui_func(unsigned char *eeprom_buff,
			    int eeprom_size)
{
	return camcal_read_insensor_gc5035p_func(eeprom_buff, eeprom_size);
}

static uint8 gc5035p_otp_read_byte(struct camkit_sensor_ctrl_t *sensor, uint16 addr)
{
	write_cmos_sensor(sensor, 0xfe, 0x02);
	write_cmos_sensor(sensor, 0x69, (addr >> 8) & 0x1f);
	write_cmos_sensor(sensor, 0x6a, addr & 0xff);
	write_cmos_sensor(sensor, 0xf3, 0x20);
	return read_cmos_sensor(sensor, 0x6c);
}

static void gc5035p_otp_read_group(struct camkit_sensor_ctrl_t *sensor,
	uint16 addr, uint8 *data, uint16 length)
{
	uint16 i = 0;

	if ((((addr & 0x1fff) >> 3) + length) > GC5035P_OTP_DATA_LENGTH) {
		log_info("out of range, start addr: 0x%.4x, length = %d\n",
			addr & 0x1fff, length);
		return;
	}

	write_cmos_sensor(sensor, 0xfe, 0x02);
	write_cmos_sensor(sensor, 0x69, (addr >> 8) & 0x1f);
	write_cmos_sensor(sensor, 0x6a, addr & 0xff);
	write_cmos_sensor(sensor, 0xf3, 0x20);
	write_cmos_sensor(sensor, 0xf3, 0x12);

	for (i = 0; i < length; i++)
		data[i] = read_cmos_sensor(sensor, 0x6c);

	write_cmos_sensor(sensor, 0xf3, 0x00);
}

static void gc5035p_gcore_read_dpc(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 dpcflag = 0;
	struct gc5035p_dpc_t *pdpc = &gc5035p_otp_data.dpc;

	dpcflag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_DPC_FLAG_OFFSET);
	log_info("dpc flag = 0x%x\n", dpcflag);
	switch (gc5035p_otp_get_2bit_flag(dpcflag, 0)) {
	case GC5035P_OTP_FLAG_EMPTY: {
		log_err("dpc info is empty!!\n");
		pdpc->flag = GC5035P_OTP_FLAG_EMPTY;
		break;
	}
	case GC5035P_OTP_FLAG_VALID: {
		log_info("dpc info is valid!\n");
		pdpc->total_num =
			gc5035p_otp_read_byte(sensor,
			GC5035P_OTP_DPC_TOTAL_NUMBER_OFFSET)
			+ gc5035p_otp_read_byte(sensor,
			GC5035P_OTP_DPC_ERROR_NUMBER_OFFSET);
		pdpc->flag = GC5035P_OTP_FLAG_VALID;
		log_info("total_num = %d\n", pdpc->total_num);
		break;
	}
	default:
		pdpc->flag = GC5035P_OTP_FLAG_INVALID;
		break;
	}
}

static void gc5035p_gcore_read_reg(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 j = 0;
	uint16 base_group = 0;
	uint8 reg[GC5035P_OTP_REG_DATA_SIZE];
	struct gc5035p_reg_update_t *pregs = &gc5035p_otp_data.regs;

	memset_s(&reg, GC5035P_OTP_REG_DATA_SIZE, 0, GC5035P_OTP_REG_DATA_SIZE);
	pregs->flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_REG_FLAG_OFFSET);
	log_info("register update flag = 0x%x\n", pregs->flag);
	if (pregs->flag == GC5035P_OTP_FLAG_VALID) {
		gc5035p_otp_read_group(sensor, GC5035P_OTP_REG_DATA_OFFSET,
			&reg[0], GC5035P_OTP_REG_DATA_SIZE);

		for (i = 0; i < GC5035P_OTP_REG_MAX_GROUP; i++) {
			base_group = i * GC5035P_OTP_REG_BYTE_PER_GROUP;
			for (j = 0; j < GC5035P_OTP_REG_REG_PER_GROUP; j++)
				if (gc5035p_otp_check_1bit_flag(reg[base_group], (4 * j + 3))) {
					pregs->reg[pregs->cnt].page =
						(reg[base_group] >>
						(4 * j)) & 0x07;
					pregs->reg[pregs->cnt].addr =
						reg[base_group +
						j * GC5035P_OTP_REG_BYTE_PER_REG
						+ 1];
					pregs->reg[pregs->cnt].value =
						reg[base_group +
						j * GC5035P_OTP_REG_BYTE_PER_REG
						+ 2];
					log_info("register[%d] P%d:0x%x->0x%x\n",
						pregs->cnt,
						pregs->reg[pregs->cnt].page,
						pregs->reg[pregs->cnt].addr,
						pregs->reg[pregs->cnt].value);
					pregs->cnt++;
				}
		}
	}
}

static void gc5035p_otp_read_sn_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 flag = 0;
	uint16 check = 0;
	uint16 sn_start_offset = 0;
	uint8 info[GC5035P_OTP_SN_DATA_SIZE];

	(void)memset_s(&info, GC5035P_OTP_SN_DATA_SIZE, 0, GC5035P_OTP_SN_DATA_SIZE);
	flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_SN_FLAG_OFFSET);

	if ((flag & 0x03) == 0x01) {
		sn_start_offset = GC5035P_OTP_SN_DATA_GROUP2_OFFSET;
	} else if (((flag & 0x0c) >> 2) == 0x01) {
		sn_start_offset = GC5035P_OTP_SN_DATA_GROUP1_OFFSET;
	} else {
		log_err("Invalid module, error sn flag = 0x%x\n", flag);
		gc5035p_otp_data.otp_buf[26] = OTP_CHECK_FAILED; /* SN check flag offset */
		return;
	}

	gc5035p_otp_read_group(sensor, sn_start_offset, &info[0], GC5035P_OTP_SN_DATA_SIZE);
	for (i = 0; i < GC5035P_OTP_SN_DATA_SIZE - 1; i++)
		check += info[i];

	if ((check % 255 + 1) == info[GC5035P_OTP_SN_DATA_SIZE - 1]) { /* 255: checksum base value  */
		/* Mind the byteorder */
		memcpy_s(gc5035p_otp_data.sn.data, GC5035P_OTP_SN_VALID_DATA_SIZE,
			info, GC5035P_OTP_SN_VALID_DATA_SIZE);
		memcpy_s(&gc5035p_otp_data.otp_buf[6], GC5035P_OTP_SN_VALID_DATA_SIZE,
			info, GC5035P_OTP_SN_VALID_DATA_SIZE);
		gc5035p_otp_data.sn.data[GC5035P_OTP_SN_VALID_DATA_SIZE] = '\0';
		log_info("SN checksum success! otp sn info = %s\n", gc5035p_otp_data.sn.data);
		gc5035p_otp_data.otp_buf[26] = OTP_CHECK_SUCCESSED;
	} else {
		log_err("SN checksum failed! check sum = 0x%x, calculate result = 0x%x\n",
			info[GC5035P_OTP_SN_DATA_SIZE - 1], (check % 255 + 1));
		gc5035p_otp_data.otp_buf[26] = OTP_CHECK_FAILED;
	}
}

static void gc5035p_otp_read_af_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 flag = 0;
	uint16 check = 0;
	uint16 af_start_offset = 0;
	uint8 info[GC5035P_OTP_AF_DATA_SIZE];

	(void)memset_s(&info, GC5035P_OTP_AF_DATA_SIZE, 0, GC5035P_OTP_AF_DATA_SIZE);
	flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_AF_FLAG_OFFSET);
	if ((flag & 0x03) == 0x01) {
		af_start_offset = GC5035P_OTP_AF_DATA_GROUP2_OFFSET;
	} else if (((flag & 0x0c) >> 2) == 0x01) {
		af_start_offset = GC5035P_OTP_AF_DATA_GROUP1_OFFSET;
	} else {
		log_err("Invalid module, error af flag = 0x%x\n", flag);
		gc5035p_otp_data.otp_buf[5] = OTP_CHECK_FAILED; /* AF check flag offset */
		return;
	}

	gc5035p_otp_read_group(sensor, af_start_offset, &info[0], GC5035P_OTP_AF_DATA_SIZE);
	for (i = 0; i < GC5035P_OTP_AF_DATA_SIZE - 1; i++)
		check += info[i];

	if ((check % 255 + 1) == info[GC5035P_OTP_AF_DATA_SIZE - 1]) { /* 255: checksum base value  */
		gc5035p_otp_data.af.direction = info[0];
		gc5035p_otp_data.af.macro = info[1] + ((info[2]&0xff) << 8);
		gc5035p_otp_data.af.infinity = info[3] + ((info[4]&0xff) << 8);
		gc5035p_otp_data.otp_buf[0] = info[0];
		gc5035p_otp_data.otp_buf[1] = info[4];
		gc5035p_otp_data.otp_buf[2] = info[3];
		gc5035p_otp_data.otp_buf[3] = info[2];
		gc5035p_otp_data.otp_buf[4] = info[1];
		log_info("AF checksum success! direction = 0x%x\n", gc5035p_otp_data.af.direction);
		log_info("macro = 0x%x\n", gc5035p_otp_data.af.macro);
		log_info("infinity = 0x%x\n", gc5035p_otp_data.af.infinity);
		gc5035p_otp_data.otp_buf[5] = OTP_CHECK_SUCCESSED; /* AF check flag offset */
	} else {
		log_err("AF checksum failed! check sum = 0x%x, calculate result = 0x%x\n",
			info[GC5035P_OTP_AF_DATA_SIZE - 1], (check % 255 + 1));
		gc5035p_otp_data.otp_buf[5] = OTP_CHECK_FAILED;
	}
}

static uint8 gc5035p_otp_read_module_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 flag = 0;
	uint16 check = 0;
	uint16 module_start_offset = 0;
	uint8 info[GC5035P_OTP_MODULE_DATA_SIZE];
	struct gc5035p_module_info_t module_info = { 0 };

	(void)memset_s(&info, GC5035P_OTP_MODULE_DATA_SIZE, 0, GC5035P_OTP_MODULE_DATA_SIZE);
	(void)memset_s(&module_info, sizeof(struct gc5035p_module_info_t), 0, sizeof(struct gc5035p_module_info_t));
	flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_MODULE_FLAG_OFFSET);

	if ((flag & 0x03) == 0x01) {
		module_start_offset = GC5035P_OTP_MODULE_DATA_GROUP2_OFFSET;
	} else if (((flag & 0x0c) >> 2) == 0x01) {
		module_start_offset = GC5035P_OTP_MODULE_DATA_GROUP1_OFFSET;
	} else {
		log_err("Invalid module,error module info flag = 0x%x\n", flag);
		return 0;
	}

	gc5035p_otp_read_group(sensor, module_start_offset, &info[0], GC5035P_OTP_MODULE_DATA_SIZE);
	for (i = 0; i < GC5035P_OTP_MODULE_DATA_SIZE - 1; i++)
		check += info[i];

	if ((check % 255 + 1) == info[GC5035P_OTP_MODULE_DATA_SIZE - 1]) {
		module_info.module_id = info[0];
		module_info.lens_id = info[1];
		module_info.reserved0= info[2];
		module_info.reserved1= info[3];
		module_info.year = info[4];
		module_info.month = info[5];
		module_info.day = info[6];

		log_info("module info checksum success! module id = 0x%x\n",module_info.module_id);
		log_info("lens id = 0x%x\n",module_info.lens_id);
		log_info("year: %d, month: %d, day: %d\n",module_info.year, module_info.month, module_info.day);
		return module_info.module_id;
	} else {
		log_err("module info checksum failed! check sum = 0x%x, calculate result = 0x%x\n",
			info[GC5035P_OTP_MODULE_DATA_SIZE - 1], (check % 255 + 1));
		return 0;
	}
}

static void gc5035p_otp_read_gwb_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 flag = 0;
	uint16 golden_check = 0;
	uint16 golden_start_offset = 0;
	uint8 golden[GC5035P_OTP_GOLDEN_DATA_SIZE];
	struct gc5035p_wb_t *pgolden = &gc5035p_otp_data.golden;

	(void)memset_s(&golden, GC5035P_OTP_GOLDEN_DATA_SIZE, 0, GC5035P_OTP_GOLDEN_DATA_SIZE);
	flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_WB_FLAG_OFFSET);

	if ((flag & 0x30) == 0x10) {
		golden_start_offset = GC5035P_OTP_GOLDEN_DATA_GROUP2_OFFSET;
	} else if (((flag & 0xc0) >> 2) == 0x10) {
		golden_start_offset = GC5035P_OTP_GOLDEN_DATA_GROUP1_OFFSET;
	} else {
		log_err("Invalid module,error gawb flag = 0x%x\n", flag);
		pgolden->flag = pgolden->flag | GC5035P_OTP_FLAG_INVALID;
		return;
	}

	/* Golden AWB */
	gc5035p_otp_read_group(sensor, golden_start_offset, &golden[0], GC5035P_OTP_GOLDEN_DATA_SIZE);
	for (i = 0; i < GC5035P_OTP_GOLDEN_DATA_SIZE - 1; i++)
		golden_check += golden[i];

	if ((golden_check % 255 + 1) == golden[GC5035P_OTP_GOLDEN_DATA_SIZE - 1]) {
		pgolden->rg = (golden[0] | ((golden[1] & 0xf0) << 4));
		pgolden->bg = (((golden[1] & 0x0f) << 8) | golden[2]);
		pgolden->rg = pgolden->rg == 0 ? GC5035P_OTP_WB_RG_TYPICAL : pgolden->rg;
		pgolden->bg = pgolden->bg == 0 ? GC5035P_OTP_WB_BG_TYPICAL : pgolden->bg;
		pgolden->flag = pgolden->flag | GC5035P_OTP_FLAG_VALID;
		log_info("golden AWB checksum success! golden wb r/g = 0x%x\n", pgolden->rg);
		log_info("golden awb b/g = 0x%x\n", pgolden->bg);
	} else {
		pgolden->flag = pgolden->flag | GC5035P_OTP_FLAG_INVALID;
		log_err("golden AWB checksum failed! check sum = 0x%x, calculate result = 0x%x\n",
			golden[GC5035P_OTP_GOLDEN_DATA_SIZE - 1],
			(golden_check % 255 + 1));
	}
}

static void gc5035p_otp_read_wb_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;
	uint8 flag = 0;
	uint16 wb_check = 0;
	uint16 wb_start_offset = 0;
	uint8 wb[GC5035P_OTP_WB_DATA_SIZE];
	struct gc5035p_wb_t *pwb = &gc5035p_otp_data.wb;

	(void)memset_s(&wb, GC5035P_OTP_WB_DATA_SIZE, 0, GC5035P_OTP_WB_DATA_SIZE);
	flag = gc5035p_otp_read_byte(sensor, GC5035P_OTP_WB_FLAG_OFFSET);

	if ((flag & 0x03) == 0x01) {
		wb_start_offset = GC5035P_OTP_WB_DATA_GROUP2_OFFSET;
	} else if (((flag & 0x0c) >> 2) == 0x01) {
		wb_start_offset = GC5035P_OTP_WB_DATA_GROUP1_OFFSET;
	} else {
		log_err("Invalid module,error awb flag = 0x%x\n", flag);
		pwb->flag = pwb->flag | GC5035P_OTP_FLAG_INVALID;
		return;
	}

	/* AWB */
	gc5035p_otp_read_group(sensor, wb_start_offset, &wb[0], GC5035P_OTP_WB_DATA_SIZE);
	for (i = 0; i < GC5035P_OTP_WB_DATA_SIZE - 1; i++)
		wb_check += wb[i];

	if ((wb_check % 255 + 1) == wb[GC5035P_OTP_WB_DATA_SIZE - 1]) {
		pwb->rg = (wb[0] | ((wb[1] & 0xf0) << 4));
		pwb->bg = (((wb[1] & 0x0f) << 8) | wb[2]);
		pwb->rg = pwb->rg == 0 ? GC5035P_OTP_WB_RG_TYPICAL : pwb->rg;
		pwb->bg = pwb->bg == 0 ? GC5035P_OTP_WB_BG_TYPICAL : pwb->bg;
		pwb->flag = pwb->flag | GC5035P_OTP_FLAG_VALID;
		log_info("AWB checksum success! golden wb r/g = 0x%x\n", pwb->rg);
		log_info("awb b/g = 0x%x\n", pwb->bg);
	} else {
		pwb->flag = pwb->flag | GC5035P_OTP_FLAG_INVALID;
		log_err("AWB checksum failed! check sum = 0x%x, calculate result = 0x%x\n",
			wb[GC5035P_OTP_WB_DATA_SIZE - 1],
			(wb_check % 255 + 1));
	}
}

static uint8 gc5035p_otp_read_sensor_info(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 moduleid = 0;
#if GC5035P_OTP_DEBUG
	uint16 i = 0;
	uint8 debug[GC5035P_OTP_DATA_LENGTH];
#endif

	gc5035p_gcore_read_dpc(sensor);
	gc5035p_gcore_read_reg(sensor);

	moduleid = gc5035p_otp_read_module_info(sensor);
	gc5035p_otp_read_wb_info(sensor);
	gc5035p_otp_read_gwb_info(sensor);
	gc5035p_otp_read_sn_info(sensor);
	gc5035p_otp_read_af_info(sensor);

#if GC5035P_OTP_DEBUG
	(void)memset_s(&debug[0], GC5035P_OTP_DATA_LENGTH, 0, GC5035P_OTP_DATA_LENGTH);
	gc5035p_otp_read_group(sensor, GC5035P_OTP_START_ADDR, &debug[0], GC5035P_OTP_DATA_LENGTH);
	for (i = 0; i < GC5035P_OTP_DATA_LENGTH; i++)
		log_info("addr = 0x%x, data = 0x%x\n", GC5035P_OTP_START_ADDR + i * 8, debug[i]);
#endif

	return moduleid;
}

static void gc5035p_otp_update_dd(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 state = 0;
	uint8 n = 0;
	struct gc5035p_dpc_t *pdpc = &gc5035p_otp_data.dpc;

	if (pdpc->flag == GC5035P_OTP_FLAG_VALID) {
		log_info("DD auto load start!\n");
		write_cmos_sensor(sensor, 0xfe, 0x02);
		write_cmos_sensor(sensor, 0xbe, 0x00);
		write_cmos_sensor(sensor, 0xa9, 0x01);
		write_cmos_sensor(sensor, 0x09, 0x33);
		write_cmos_sensor(sensor, 0x01, (pdpc->total_num >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x02, pdpc->total_num & 0xff);
		write_cmos_sensor(sensor, 0x03, 0x00);
		write_cmos_sensor(sensor, 0x04, 0x80);
		write_cmos_sensor(sensor, 0x95, 0x0a);
		write_cmos_sensor(sensor, 0x96, 0x30);
		write_cmos_sensor(sensor, 0x97, 0x0a);
		write_cmos_sensor(sensor, 0x98, 0x32);
		write_cmos_sensor(sensor, 0x99, 0x07);
		write_cmos_sensor(sensor, 0x9a, 0xa9);
		write_cmos_sensor(sensor, 0xf3, 0x80);
		while (n < 3) {
			state = read_cmos_sensor(sensor, 0x06);
			if ((state | 0xfe) == 0xff)
				mdelay(10);
			else
				n = 3;
			n++;
		}
		write_cmos_sensor(sensor, 0xbe, 0x01);
		write_cmos_sensor(sensor, 0x09, 0x00);
		write_cmos_sensor(sensor, 0xfe, 0x01);
		write_cmos_sensor(sensor, 0x80, 0x02);
		write_cmos_sensor(sensor, 0xfe, 0x00);
	}
}

static void gc5035p_otp_update_wb(struct camkit_sensor_ctrl_t *sensor)
{
	uint16 r_gain = GC5035P_OTP_WB_GAIN_BASE;
	uint16 g_gain = GC5035P_OTP_WB_GAIN_BASE;
	uint16 b_gain = GC5035P_OTP_WB_GAIN_BASE;
	uint16 base_gain = GC5035P_OTP_WB_CAL_BASE;
	uint16 r_gain_curr = GC5035P_OTP_WB_CAL_BASE;
	uint16 g_gain_curr = GC5035P_OTP_WB_CAL_BASE;
	uint16 b_gain_curr = GC5035P_OTP_WB_CAL_BASE;
	uint16 rg_typical = GC5035P_OTP_WB_RG_TYPICAL;
	uint16 bg_typical = GC5035P_OTP_WB_BG_TYPICAL;
	struct gc5035p_wb_t *pwb = &gc5035p_otp_data.wb;
	struct gc5035p_wb_t *pgolden = &gc5035p_otp_data.golden;

	if (gc5035p_otp_check_1bit_flag(pgolden->flag, 0)) {
		rg_typical = pgolden->rg;
		bg_typical = pgolden->bg;
	} else {
		rg_typical = GC5035P_OTP_WB_RG_TYPICAL;
		bg_typical = GC5035P_OTP_WB_BG_TYPICAL;
	}

	if (gc5035p_otp_check_1bit_flag(pwb->flag, 0)) {
		r_gain_curr = GC5035P_OTP_WB_CAL_BASE * rg_typical / pwb->rg;
		b_gain_curr = GC5035P_OTP_WB_CAL_BASE * bg_typical / pwb->bg;
		g_gain_curr = GC5035P_OTP_WB_CAL_BASE;
		base_gain = (r_gain_curr < b_gain_curr) ?
			r_gain_curr : b_gain_curr;
		base_gain = (base_gain < g_gain_curr) ? base_gain : g_gain_curr;
		r_gain = GC5035P_OTP_WB_GAIN_BASE * r_gain_curr / base_gain;
		g_gain = GC5035P_OTP_WB_GAIN_BASE * g_gain_curr / base_gain;
		b_gain = GC5035P_OTP_WB_GAIN_BASE * b_gain_curr / base_gain;
		write_cmos_sensor(sensor, 0xfe, 0x04);
		write_cmos_sensor(sensor, 0x40, g_gain & 0xff);
		write_cmos_sensor(sensor, 0x41, r_gain & 0xff);
		write_cmos_sensor(sensor, 0x42, b_gain & 0xff);
		write_cmos_sensor(sensor, 0x43, g_gain & 0xff);
		write_cmos_sensor(sensor, 0x44, g_gain & 0xff);
		write_cmos_sensor(sensor, 0x45, r_gain & 0xff);
		write_cmos_sensor(sensor, 0x46, b_gain & 0xff);
		write_cmos_sensor(sensor, 0x47, g_gain & 0xff);
		write_cmos_sensor(sensor, 0x48, (g_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x49, (r_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4a, (b_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4b, (g_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4c, (g_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4d, (r_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4e, (b_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0x4f, (g_gain >> 8) & 0x07);
		write_cmos_sensor(sensor, 0xfe, 0x00);
	}
}

static void gc5035p_otp_update_reg(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0;

	log_info("reg count = %d\n", gc5035p_otp_data.regs.cnt);

	if (gc5035p_otp_check_1bit_flag(gc5035p_otp_data.regs.flag, 0))
		for (i = 0; i < gc5035p_otp_data.regs.cnt; i++) {
			write_cmos_sensor(sensor,
				0xfe, gc5035p_otp_data.regs.reg[i].page);
			write_cmos_sensor(sensor,
				gc5035p_otp_data.regs.reg[i].addr,
				gc5035p_otp_data.regs.reg[i].value);
			log_info("reg[%d] P%d:0x%x -> 0x%x\n",
				i, gc5035p_otp_data.regs.reg[i].page,
				gc5035p_otp_data.regs.reg[i].addr,
				gc5035p_otp_data.regs.reg[i].value);
		}
}

static void gc5035p_otp_update(struct camkit_sensor_ctrl_t *sensor)
{
	gc5035p_otp_update_dd(sensor);
	gc5035p_otp_update_wb(sensor);
	gc5035p_otp_update_reg(sensor);
}

static struct camkit_i2c_reg enable_otp[] = {
	{ 0xfc, 0x01, 0x00 },
	{ 0xf4, 0x40, 0x00 },
	{ 0xf5, 0xe9, 0x00 },
	{ 0xf6, 0x14, 0x00 },
	{ 0xf8, 0x40, 0x00 },
	{ 0xf9, 0x82, 0x00 },
	{ 0xfa, 0x00, 0x00 },
	{ 0xfc, 0x81, 0x00 },
	{ 0xfe, 0x00, 0x00 },
	{ 0x36, 0x01, 0x00 },
	{ 0xd3, 0x87, 0x00 },
	{ 0x36, 0x00, 0x00 },
	{ 0x33, 0x00, 0x00 },
	{ 0xf7, 0x01, 0x00 },
	{ 0xfc, 0x8e, 0x00 },
	{ 0xfe, 0x00, 0x00 },
	{ 0xee, 0x30, 0x00 },
	{ 0xfa, 0x10, 0x00 },
	{ 0xf5, 0xe9, 0x00 },
	{ 0xfe, 0x02, 0x00 },
	{ 0x67, 0xc0, 0x00 },
	{ 0x59, 0x3f, 0x00 },
	{ 0x55, 0x80, 0x00 },
	{ 0x65, 0x80, 0x00 },
	{ 0x66, 0x03, 0x00 },
	{ 0xfe, 0x00, 0x00 },
};

static struct camkit_i2c_reg disable_otp[] = {
	{ 0xfe, 0x02, 0x00 },
	{ 0x67, 0x00, 0x00 },
	{ 0xfe, 0x00, 0x00 },
	{ 0xfa, 0x00, 0x00 },
};

static void gc5035p_otp_identify(struct camkit_sensor_ctrl_t *sensor)
{
	int32 rc;

	if (gc5035p_otp_data.have_read)
		return;

	/* Enable otp read */
	rc = camkit_sensor_write_table(sensor, enable_otp,
			camkit_array_size(enable_otp), CAMKIT_I2C_BYTE_DATA);
	if (rc != ERR_NONE) {
		log_info("enable otp read failed\n");
		return;
	}

	/* read otp group id */
	gc5035p_otp_read_group(sensor, GC5035P_OTP_ID_DATA_OFFSET,
		&gc5035p_otp_data.otp_id[0],
		GC5035P_OTP_ID_SIZE);

	gc5035p_otp_read_sensor_info(sensor);

	/* Disable otp read */
	rc = camkit_sensor_write_table(sensor, disable_otp,
			camkit_array_size(disable_otp), CAMKIT_I2C_BYTE_DATA);
	if (rc != ERR_NONE) {
		log_err("disable otp read failed\n");
		return;
	}
	gc5035p_otp_data.have_read = 1;
}

static void gc5035p_otp_function(struct camkit_sensor_ctrl_t *sensor)
{
	uint8 i = 0, flag = 0;
	uint8 otp_id[GC5035P_OTP_ID_SIZE];

	(void)memset_s(&otp_id, GC5035P_OTP_ID_SIZE, 0, GC5035P_OTP_ID_SIZE);

	write_cmos_sensor(sensor, 0xfa, 0x10);
	write_cmos_sensor(sensor, 0xf5, 0xe9);
	write_cmos_sensor(sensor, 0xfe, 0x02);
	write_cmos_sensor(sensor, 0x67, 0xc0);
	write_cmos_sensor(sensor, 0x59, 0x3f);
	write_cmos_sensor(sensor, 0x55, 0x80);
	write_cmos_sensor(sensor, 0x65, 0x80);
	write_cmos_sensor(sensor, 0x66, 0x03);
	write_cmos_sensor(sensor, 0xfe, 0x00);

	gc5035p_otp_read_group(sensor, GC5035P_OTP_ID_DATA_OFFSET,
		&otp_id[0], GC5035P_OTP_ID_SIZE);
	for (i = 0; i < GC5035P_OTP_ID_SIZE; i++)
		if (otp_id[i] != gc5035p_otp_data.otp_id[i]) {
			flag = 1;
			break;
		}

	if (flag == 1) {
		log_err("otp id mismatch, read again");
		(void)memset_s(&gc5035p_otp_data, sizeof(struct gc5035p_otp_t), 0, sizeof(gc5035p_otp_data));
		for (i = 0; i < GC5035P_OTP_ID_SIZE; i++)
			gc5035p_otp_data.otp_id[i] = otp_id[i];
		gc5035p_otp_read_sensor_info(sensor);
	}

	gc5035p_otp_update(sensor);

	write_cmos_sensor(sensor, 0xfe, 0x02);
	write_cmos_sensor(sensor, 0x67, 0x00);
	write_cmos_sensor(sensor, 0xfe, 0x00);
	write_cmos_sensor(sensor, 0xfa, 0x00);
}

static uint32 gc5035_m200_kob_open(struct camkit_sensor *sensor)
{
	uint32 rc;
	struct camkit_params *kit_params = NULL;
	struct camkit_sensor_params *sensor_params = NULL;
	log_info("Enter");
	if (!sensor) {
		log_err("Invalid ptr\n");
		return ERR_IO;
	}
	kit_params = sensor->kit_params;
	return_err_if_null(kit_params);
	sensor_params = kit_params->sensor_params;
	return_err_if_null(sensor_params);

	rc = camkit_open(sensor);
	if (rc != ERR_NONE) {
		log_err("Camkit open failed\n");
		return rc;
	}

	gc5035p_otp_function(&sensor_params->sensor_ctrl);
	log_info("EXIT");

	return ERR_NONE;
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

uint32 gc5035_m200_kob_match_id(struct camkit_sensor *sensor,
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
	gc5035p_otp_identify(&sensor_params->sensor_ctrl); /* read otp */

	*match_id = kit_params->module_params->match_id;
	log_info("match id ok, sensor id: 0x%x, module: 0x%x, match id: 0x%x",
		kit_params->sensor_params->sensor_info.sensor_id,
		kit_params->module_params->module_code,
		kit_params->module_params->match_id);

	return ERR_NONE;
}

static struct sensor_kit_ops gc5035_m200_kob_ops = {
	.sensor_open = gc5035_m200_kob_open,
	.sensor_close = camkit_close,
	.match_id = gc5035_m200_kob_match_id,
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

uint32 get_gc5035_m200_kob_ops(struct sensor_kit_ops **ops)
{
	if (ops != NULL) {
		*ops = &gc5035_m200_kob_ops;
	} else {
		log_err("get gc5035_m200_kob_ops operators fail");
		return ERR_INVAL;
	}
	(void)memset_s(&gc5035p_otp_data, sizeof(struct gc5035p_otp_t), 0, sizeof(gc5035p_otp_data));
	log_info("get gc5035_m200_kob operators OK");
	return ERR_NONE;
}

register_customized_driver(
	gc5035_m200_kob,
	CAMKIT_SENSOR_IDX_MAIN,
	GC5035_M200_KOB_SENSOR_ID,
	get_gc5035_m200_kob_ops);
