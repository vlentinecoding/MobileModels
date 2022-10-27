/**
 * LC898128 Global declaration & prototype declaration
 *
 * Copyright (C) 2021, ON Semiconductor, all right reserved.
 *
 **/

#ifndef PHONEUPDATE_H_
#define PHONEUPDATE_H_

#define MODULE_VENDOR 0
#define MDL_VER 0x00

#ifdef DEBUG
 extern void dbg_printf(const char *, ...);
 extern void dbg_Dump(const char *, int);
 #define TRACE_INIT(x) dbgu_init(x)
 #define TRACE_USB(fmt, ...) dbg_UsbData(fmt, ## __VA_ARGS__)
 #define TRACE(fmt, ...) dbg_printf(fmt, ## __VA_ARGS__)
 #define TRACE_DUMP(x,y) dbg_Dump(x,y)
#else
 #define TRACE_INIT(x)
 #define TRACE(...)
 #define TRACE_DUMP(x,y)
 #define TRACE_USB(...)
#endif

typedef signed char INT_8;
typedef short INT_16;
typedef long long INT_64;
typedef unsigned short UINT_16;
typedef unsigned long long UINT_64;

/* STRUCTURE DEFINE */
typedef struct {
	UINT_16 Index;
	const uint8_t* UpdataCode;
	uint32_t SizeUpdataCode;
	UINT_64 SizeUpdataCodeCksm;
	const uint8_t* FromCode;
	uint32_t SizeFromCode;
	UINT_64 SizeFromCodeCksm;
	uint32_t SizeFromCodeValid;
}DOWNLOAD_TBL_EXT;

typedef struct STRECALIB {
	INT_16 SsFctryOffX;
	INT_16 SsFctryOffY;
	INT_16 SsRecalOffX;
	INT_16 SsRecalOffY;
	INT_16 SsDiffX;
	INT_16 SsDiffY;
}stReCalib;

typedef struct {
	int32_t SiSampleNum;
	int32_t SiSampleMax;

	struct {
		int32_t SiMax1;
		int32_t SiMin1;
		uint32_t UiAmp1;
		INT_64 LLiIntegral1;
		INT_64 LLiAbsInteg1;
		int32_t PiMeasureRam1;
	}MeasureFilterA;

	struct {
		int32_t SiMax2;
		int32_t SiMin2;
		uint32_t UiAmp2;
		INT_64 LLiIntegral2;
		INT_64 LLiAbsInteg2;
		int32_t PiMeasureRam2;
	}MeasureFilterB;
}MeasureFunction_Type;

union	DWDVAL {
	uint32_t UlDwdVal;
	UINT_16 UsDwdVal[ 2 ];
	struct {
		UINT_16 UsLowVal;
		UINT_16 UsHigVal;
	}StDwdVal;
	struct {
		uint8_t UcRamVa0;
		uint8_t UcRamVa1;
		uint8_t UcRamVa2;
		uint8_t UcRamVa3;
	}StCdwVal;
};

typedef union DWDVAL UnDwdVal;

union ULLNVAL {
	UINT_64 UllnValue;
	uint32_t UlnValue[ 2 ];
	struct {
		uint32_t UlLowVal;
		uint32_t UlHigVal;
	}StUllnVal;
};

typedef union ULLNVAL UnllnVal;

#define EXE_END             0x00000002L
#define EXE_GXADJ           0x00000042L
#define EXE_GYADJ           0x00000082L
#define EXE_GZADJ           0x00400002L
#define EXE_AZADJ           0x00200002L
#define EXE_AYADJ           0x00100002L
#define EXE_AXADJ           0x00080002L
#define EXE_HXMVER          0x06
#define EXE_HYMVER          0x0A
#define EXE_GXABOVE         0x06
#define EXE_GXBELOW         0x0A
#define EXE_GYABOVE         0x12
#define EXE_GYBELOW         0x22

#define SUCCESS             0x00
#define FAILURE             0x01

#define FT_REPRG            15
#define PRDCT_WR            0x55555555
#define USER_WR             0xAAAAAAAA
#define MAT2_CKSM           29
#define CHECKCODE1          30
#define CHECK_CODE1         0x99756768
#define CHECKCODE2          31
#define CHECK_CODE2         0x01AC28AC

#define CMD_IO_ADR_ACCESS        0xC000 // !< IO Write Access
#define CMD_IO_DAT_ACCESS        0xD000 // !< IO Read Access
#define SYSDSP_DSPDIV            0xD00014
#define SYSDSP_SOFTRES           0xD0006C
#define SYSDSP_REMAP             0xD000AC
#define SYSDSP_CVER              0xD00100
#define ROMINFO                  0xE050D4
#define FLASHROM_128             0xE07000 // Flash Memory
#define FLASHROM_FLA_RDAT        (FLASHROM_128 + 0x00)
#define FLASHROM_FLA_WDAT        (FLASHROM_128 + 0x04)
#define FLASHROM_ACSCNT          (FLASHROM_128 + 0x08)
#define FLASHROM_FLA_ADR         (FLASHROM_128 + 0x0C)
#define USER_MAT                 0
#define INF_MAT0                 1
#define INF_MAT1                 2
#define INF_MAT2                 4
#define FLASHROM_CMD             (FLASHROM_128 + 0x10)
#define FLASHROM_FLAWP           (FLASHROM_128 + 0x14)
#define FLASHROM_FLAINT          (FLASHROM_128 + 0x18)
#define FLASHROM_FLAMODE         (FLASHROM_128 + 0x1C)
#define FLASHROM_TPECPW          (FLASHROM_128 + 0x20)
#define FLASHROM_TACC            (FLASHROM_128 + 0x24)

#define FLASHROM_ERR_FLA         (FLASHROM_128 + 0x98)
#define FLASHROM_RSTB_FLA        (FLASHROM_128 + 0x4CC)
#define FLASHROM_UNLK_CODE1      (FLASHROM_128 + 0x554)
#define FLASHROM_CLK_FLAON       (FLASHROM_128 + 0x664)
#define FLASHROM_UNLK_CODE2      (FLASHROM_128 + 0xAA8)
#define FLASHROM_UNLK_CODE3      (FLASHROM_128 + 0xCCC)

#define READ_STATUS_INI          0x01000000

#define HallFilterD_HXDAZ1       0x0048
#define HallFilterD_HYDAZ1       0x0098

#define HALL_RAM_HXOFF           0x00D8
#define HALL_RAM_HYOFF           0x0128
#define HALL_RAM_HXOFF1          0x00DC
#define HALL_RAM_HYOFF1          0x012C
#define HALL_RAM_HXOUT0          0x00E0
#define HALL_RAM_HYOUT0          0x0130
#define HALL_RAM_SINDX1          0x00F0
#define HALL_RAM_SINDY1          0x0140
#define HALL_RAM_HALL_X_OUT      0x00F4
#define HALL_RAM_HALL_Y_OUT      0x0144
#define HALL_RAM_HXIDAT          0x0178
#define HALL_RAM_HYIDAT          0x017C
#define HALL_RAM_GYROX_OUT       0x0180
#define HALL_RAM_GYROY_OUT       0x0184
#define HallFilterCoeffX_hxgain0 0x80F0
#define HallFilterCoeffY_hygain0 0x818C
#define Gyro_Limiter_X           0x8330
#define Gyro_Limiter_Y           0x8334
#define GyroFilterTableX_gxzoom  0x82B8
#define GyroFilterTableY_gyzoom  0x8318
#define GyroFilterTableX_gxlenz  0x82BC
#define GyroFilterTableY_gylenz  0x831C
#define GyroFilterShiftX         0x8338
#define GyroFilterShiftY         0x833C

#define GYRO_RAM_GX_ADIDAT       0x0220
#define GYRO_RAM_GY_ADIDAT       0x0224
#define GYRO_RAM_GXOFFZ          0x0240
#define GYRO_RAM_GYOFFZ          0x0244
#define GYRO_ZRAM_GZ_ADIDAT      0x039C
#define GYRO_ZRAM_GZOFFZ         0x03A8
#define ACCLRAM_X_AC_ADIDAT      0x0450
#define ACCLRAM_X_AC_OFFSET      0x0454
#define ACCLRAM_Y_AC_ADIDAT      0x047C
#define ACCLRAM_Y_AC_OFFSET      0x0480
#define ACCLRAM_Z_AC_ADIDAT      0x04A8
#define ACCLRAM_Z_AC_OFFSET      0x04AC

/* Command */
#define CMD_IO_ADR_ACCESS        0xC000
#define CMD_IO_DAT_ACCESS        0xD000
#define CMD_RETURN_TO_CENTER     0xF010
#define BOTH_SRV_OFF             0x00000000
#define XAXS_SRV_ON              0x00000001
#define YAXS_SRV_ON              0x00000002
#define BOTH_SRV_ON              0x00000003
#define CMD_PAN_TILT             0xF011
#define PAN_TILT_OFF             0x00000000
#define PAN_TILT_ON              0x00000001
#define CMD_OIS_ENABLE           0xF012
#define OIS_DISABLE              0x00000000
#define OIS_ENABLE               0x00000001
#define SMA_OIS_ENABLE           0x00010000
#define BOTH_OIS_ENABLE          0x00010001
#define CMD_MOVE_STILL_MODE      0xF013
#define MOVIE_MODE               0x00000000
#define STILL_MODE               0x00000001
#define MOVIE_MODE1              0x00000002
#define STILL_MODE1              0x00000003
#define MOVIE_MODE2              0x00000004
#define STILL_MODE2              0x00000005
#define MOVIE_MODE3              0x00000006
#define STILL_MODE3              0x00000007
#define CMD_GYROINITIALCOMMAND   0xF015
#define SET_ICM20690             0x00000000
#define SET_LSM6DSM              0x00000002
#define CMD_OSC_DETECTION        0xF017
#define OSC_DTCT_DISABLE         0x00000000
#define OSC_DTCT_ENABLE          0x00000001
#define CMD_SSC_ENABLE           0xF01C
#define SSC_DISABLE              0x00000000
#define SSC_ENABLE               0x00000001
#define CMD_GYRO_RD_ACCS         0xF01D
#define CMD_GYRO_WR_ACCS         0xF01E
// #define CMD_SMA_CONTROL       0xF01F
// #define SMA_STOP              0x00000000
// #define SMA_START             0x00000001
#define CMD_READ_STATUS          0xF100
#define READ_STATUS_INI          0x01000000

#define CNT050MS                 676
#define CNT100MS                 1352
#define CNT200MS                 2703

#define SiVerNum                 0x8000

// Calibration flags
#define     HALL_CALB_FLG       0x00008000
#define     HALL_CALB_BIT       0x00FF00FF
#define     GYRO_GAIN_FLG       0x00004000
#define     MIX2_CALB_FLG       0x00002000  // Mixing 2nd calibration
#define     ACTIVE_GG_FLG       0x00001000
#define     CAL_ANGLE_FLG       0x00000800  // angle correct calibration
#define     HLLN_CALB_FLG       0x00000400  // Hall linear calibration
#define     MIXI_CALB_FLG       0x00000200  // Mixing calibration

// Calibration Status
#define CALIBRATION_STATUS      0
// Hall Bias/Offset
#define HALL_BIAS_OFFSET        1  // 0:XBIAS 1:XOFFSET 2:YBIAS 3:YOFFSET
// Loop Gain Calibration
#define LOOP_GAIN_XY            2 // [1:0]X  [3:2]Y
// Lens Center Calibration
#define LENS_OFFSET             3 // [1:0]X  [3:2]Y
// Gyro Gain Calibration
#define GYRO_GAIN_X             4
#define GYRO_GAIN_Y             5
// Liniearity correction
#define LN_POS1                 6 // [3:2]Y  [1:0]X
#define LN_POS2                 7 // [3:2]Y  [1:0]X
#define LN_POS3                 8 // [3:2]Y  [1:0]X
#define LN_POS4                 9 // [3:2]Y  [1:0]X
#define LN_POS5                 10 // [3:2]Y  [1:0]X
#define LN_POS6                 11 // [3:2]Y  [1:0]X
#define LN_POS7                 12 // [3:2]Y  [1:0]X
#define LN_STEP                 13 // [3:2]Y  [1:0]X
// Gyro mixing correction
#define MIXING_X                14  // [3:2]XY [1:0]XX
#define MIXING_Y                15  // [3:2]YX [1:0]YY
#define MIXING_SFT              16  // [3:2]2ndA 1:YSFT 0:XSHT
#define MIXING_2ND              17  // [3:2]2ndC [1:0]2ndB
//// Gyro Offset Calibration
//#define   G_OFFSET_XY             18  // [3:2]GY offset [1:0]GX offset
//#define   G_OFFSET_Z_AX           19  // [3:2]AX offset [1:0]GZ offset
//#define   A_OFFSET_YZ             20  // [3:2]AZ offset [1:0]AY offset
// back up hall max and min
#define HL_XMAXMIN              18  // [3:2]MAX [1:0]MIN
#define HL_YMAXMIN              19   // [3:2]MAX [1:0]MIN
// Angle correct Correction.
#define ANGLC_X                 21  // [3:2]XY [1:0]XX
#define ANGLC_Y                 22  // [3:2]YX [1:0]YY

// Active Gyto Gain Correction.
#define AGG_000DEG              23  // [3:2]Y [1:0]X
#define AGG_090DEG              24  // [3:2]Y [1:0]X
#define AGG_180DEG              25  // [3:2]Y [1:0]X
#define AGG_270DEG              26  // [3:2]Y [1:0]X
#define AGG_UP                  27  // [3:2]Y [1:0]X
#define AGG_DOWN                28  // [3:2]Y [1:0]X

#define GYRO_PHASE_SELECT       29
#define OPTICAL_CENTER          30

// include check sum
#define MAT0_CKSM               31  // [3:2]AZ offset [1:0]AY offset
// Gyro Offset Calibration
#define G_OFFSET_XY             0   // [3:2]GX offset [1:0]GY offset
#define G_OFFSET_Z_AX           1   // [3:2]GZ offset [1:0]AX offset
#define A_OFFSET_YZ             2   // [3:2]AY offset [1:0]AZ offset
#define MAT1_CKSM               12  // [3:2]CheckSumH [1:0]CheckSumL
/* Prototype */
uint8_t FlashDownload128(struct cam_ois_ctrl_t *o_ctrl, uint8_t chiperase, uint8_t ModuleVendor, uint8_t ActVer );
void OisEna(struct cam_ois_ctrl_t *o_ctrl);
void OisDis(struct cam_ois_ctrl_t *o_ctrl);
void SrvOn(struct cam_ois_ctrl_t *o_ctrl);
void XuChangGetAcclOffset(struct cam_ois_ctrl_t *o_ctrl, unsigned short* AcclOffsetX, unsigned short* AcclOffsetY, unsigned short* AcclOffsetZ);
uint32_t XuChangMeasGyAcOffset(struct cam_ois_ctrl_t *o_ctrl);
void XuChangGetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, unsigned short* GyroOffsetX, unsigned short* GyroOffsetY, unsigned short* GyroOffsetZ);
uint8_t XuChangSetAngleCorrection(struct cam_ois_ctrl_t *o_ctrl, INT_8 DegreeGap, uint8_t SelectAct, uint8_t Arrangement );
void XuChangSetActiveMode(struct cam_ois_ctrl_t *o_ctrl);
uint8_t XuChangRdStatus(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcStBitChk );
uint8_t FlashMultiRead(struct cam_ois_ctrl_t *o_ctrl, uint8_t SelMat, uint32_t UlAddress, uint32_t *PulData , uint8_t UcLength);
uint8_t XuChangWrGyroGainData(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcMode);
uint8_t RecoveryCorrectCoeffDataSave(struct cam_ois_ctrl_t *o_ctrl);
uint8_t WrGyroOffsetData(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcMode);
#endif /* #ifndef OIS_H_ */
