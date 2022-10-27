/**
 * LC898129 Global declaration & prototype declaration
 *
 * Copyright (C) 2021, ON Semiconductor, all right reserved.
 *
 **/

#ifndef PHONEUPDATE_H_
#define PHONEUPDATE_H_

#include <cam_sensor_cmn_header.h>
#include "cam_ois_dev.h"
//==============================================================================
//
//==============================================================================
//#define MODULE_VENDOR 0x02  //O-FILM
//#define MODULE_VENDOR 0x03  //LVI
  #define MODULE_VENDOR 0x07  //SUNNY

#define END_USR 0x20  // End User Code
#define	MDL_VER 0x01  // ChangAn Project

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

typedef	signed char        INT_8;
typedef	short              INT_16;
typedef	int32_t            INT_32;
typedef	long long          INT_64;
typedef	unsigned char      UINT_8;
typedef	unsigned short     UINT_16;
typedef	uint32_t           UINT_32;
typedef	unsigned long long UINT_64;

//****************************************************
//	STRUCTURE DEFINE
//****************************************************
typedef struct {
	UINT_16 Index;
	const UINT_8* UpdataCode;
	UINT_32 SizeUpdataCode;
	UINT_64 SizeUpdataCodeCksm;
	const UINT_8* FromCode;
	UINT_32 SizeFromCode;
	UINT_64 SizeFromCodeCksm;
	UINT_32 SizeFromCodeValid;
} DOWNLOAD_TBL_EXT;

typedef struct STRECALIB {
	INT_16 SsFctryOffX;
	INT_16 SsFctryOffY;
	INT_16 SsRecalOffX;
	INT_16 SsRecalOffY;
	INT_16 SsDiffX;
	INT_16 SsDiffY;
} stReCalib;

typedef struct {
	INT_32 SiSampleNum;
	INT_32 SiSampleMax;

	struct {
		INT_32 SiMax1;
		INT_32 SiMin1;
		UINT_32 UiAmp1;
		INT_64 LLiIntegral1;
		INT_64 LLiAbsInteg1;
		INT_32 PiMeasureRam1;
	} MeasureFilterA;

	struct {
		INT_32 SiMax2;
		INT_32 SiMin2;
		UINT_32	UiAmp2;
		INT_64 LLiIntegral2;
		INT_64 LLiAbsInteg2;
		INT_32 PiMeasureRam2;
	} MeasureFilterB;
} MeasureFunction_Type;

union	DWDVAL {
	UINT_32	UlDwdVal ;
	UINT_16	UsDwdVal[ 2 ];
	struct {
		UINT_16	UsLowVal ;
		UINT_16	UsHigVal ;
	} StDwdVal ;
	struct {
		UINT_8	UcRamVa0 ;
		UINT_8	UcRamVa1 ;
		UINT_8	UcRamVa2 ;
		UINT_8	UcRamVa3 ;
	} StCdwVal ;
} ;

typedef union DWDVAL UnDwdVal;

union	ULLNVAL {
	UINT_64	UllnValue ;
	UINT_32	UlnValue[ 2 ] ;
	struct {
		UINT_32	UlLowVal ;
		UINT_32	UlHigVal ;
	} StUllnVal ;
} ;

typedef union ULLNVAL	UnllnVal;

#define EXE_END    0x00000002L
#define EXE_GXADJ  0x00000042L
#define EXE_GYADJ  0x00000082L
#define EXE_GZADJ  0x00400002L
#define EXE_AZADJ  0x00200002L
#define EXE_AYADJ  0x00100002L
#define EXE_AXADJ  0x00080002L

#define	SUCCESS  0x00
#define	FAILURE  0x01

#if 1
#define MAKER_CODE  0x7777 // ON Semi
#else
#define MAKER_CODE  0xAAAA // LVI
#endif

//==============================================================================
//
//==============================================================================
#define CMD_IO_ADR_ACCESS 0xC000 //!< IO Write Access
#define CMD_IO_DAT_ACCESS 0xD000 //!< IO Read Access

#define PERICLKON						0xD00000
#define SYSDSP_DSPDIV					0xD00014
#define SYSDSP_SOFTRES					0xD0006C
#define FRQTRM							0xD00098
#define SYSDSP_REMAP					0xD000AC
#define OSCCNT							0xD000D4
#define SYSDSP_CVER						0xD00100
#define OSCCKCNT						0xD00108
#define ROMINFO							0xE050D4

#define FLASHROM_129		0xE07000	// Flash Memory I/F
#define FLASHROM_FLA_RDAT					(FLASHROM_129 + 0x00)
#define FLASHROM_FLA_WDAT					(FLASHROM_129 + 0x04)
#define FLASHROM_ACSCNT						(FLASHROM_129 + 0x08)
#define FLASHROM_FLA_ADR					(FLASHROM_129 + 0x0C)
#define FLASHROM_CMD						(FLASHROM_129 + 0x10)
#define FLASHROM_FLAWP						(FLASHROM_129 + 0x14)
#define FLASHROM_FLAINT						(FLASHROM_129 + 0x18)
#define FLASHROM_FLAMODE					(FLASHROM_129 + 0x1C)
#define FLASHROM_TPECPW						(FLASHROM_129 + 0x20)
#define FLASHROM_TACC						(FLASHROM_129 + 0x24)

#define FLASHROM_ERR_FLA					(FLASHROM_129 + 0x98)
#define FLASHROM_RSTB_FLA					(FLASHROM_129 + 0x4CC)
#define FLASHROM_UNLK_CODE1					(FLASHROM_129 + 0x554)
#define FLASHROM_CLK_FLAON					(FLASHROM_129 + 0x664)
#define FLASHROM_UNLK_CODE2					(FLASHROM_129 + 0xAA8)
#define FLASHROM_UNLK_CODE3					(FLASHROM_129 + 0xCCC)

#define READ_STATUS_INI					0x01000000

#define USER_MAT				0
#define INF_MAT0				1
#define INF_MAT1				2
#define INF_MAT2				4
#define TRIM_MAT				16

#define G_OFFSET_XY             8
#define G_OFFSET_Z_AX           9
#define A_OFFSET_YZ             10

//==============================================================================
// Prototype
//==============================================================================

UINT_8 FlashProgram129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 chiperase, UINT_8 ModuleVendor, UINT_8 ActVer );
void BootMode(struct cam_ois_ctrl_t *o_ctrl);
UINT_8 SetAngleCorrection(struct cam_ois_ctrl_t *o_ctrl, INT_32 DegreeGap, UINT_8 SelectAct, UINT_8 Arrangement );
void SetActiveMode(struct cam_ois_ctrl_t *o_ctrl);
UINT_32 MeasGyAcOffset(struct cam_ois_ctrl_t *o_ctrl);
void GetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, UINT_16 *GyroOffsetX, UINT_16 *GyroOffsetY, UINT_16 *GyroOffsetZ);
UINT_8 InfoMatRead129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo, UINT_8 Addr, UINT_8 Size, UINT_32 *pData);
UINT_8 CalibrationDataSave0(struct cam_ois_ctrl_t *o_ctrl);
#endif /* #ifndef OIS_H_ */
