/**
 * LC898129 Flash update
 *
 * Copyright (C) 2021, ON Semiconductor, all right reserved.
 *
 **/

//**************************
//	Include Header File
//**************************
#include <linux/module.h>
#include <linux/firmware.h>
#include <cam_sensor_cmn_header.h>
#include "cam_ois_core.h"
#include "cam_ois_soc.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_res_mgr_api.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include <linux/vmalloc.h>
#include "onsemi_ois_interface.h"

#include	"UpdataCode129.h"
#include	"PhoneUpdate.h"
#include	"OSCAdj.h"
#include	"lc898129.h"

#if 	MODULE_VENDOR == 0x03
//#include	"FromCode_01_04.h"
//#include	"FromCode_00_01.h"
#elif 	MODULE_VENDOR == 0x07
#include	"FromCode_00_03.h" // Honor ChangAn F/W
//#include	"FromCode_00_02.h"
#elif 	MODULE_VENDOR == 0x02
//#include	"FromCode_00_03.h"
#endif

#define AtmelCodeVersion 0x0001

//**************************
//	define
//**************************
#define MeasureFunctionSW 1 // 0 : Disable
                                        // 1 : Enable

#define SetAngleCorrectSW 1 // 0 : Disable
                                        // 1 : Enable

#define HALL_ADJ				0
#define LOOPGAIN				1
#define THROUGH					2
#define NOISE					3
#define	OSCCHK					4
#define	GAINCURV				5
#define	SELFTEST				6
#define LOOPGAIN2				7


#define	FLASH_BLOCKS			14		// 1[Block]=4[KByte] (14*4=56[KByte])
#define	USER_RESERVE			0		// Reserved for customer data blocks
#define	ERASE_BLOCKS			(FLASH_BLOCKS - USER_RESERVE)

/* Burst Length for updating to PMEM */
//#define BURST_LENGTH_UC 		( 3*20 ) 	// 60 Total:63Byte Burst
#define BURST_LENGTH_UC 		( 6*20 ) 	// 120 Total:123Byte Burst
/* Burst Length for updating to Flash */
//#define BURST_LENGTH_FC 		( 32 )	 	// 32 Total: 35Byte Burst
#define BURST_LENGTH_FC 		( 64 )	 	// 64 Total: 67Byte Burst

#if SetAngleCorrectSW
static void	SetGyroCoef(struct cam_ois_ctrl_t *o_ctrl, UINT_8  );
static void	SetAccelCoef(struct cam_ois_ctrl_t *o_ctrl, UINT_8 );
#endif

//********************************************************************************
//
// Function Name 	: Measure Function Code
//
//********************************************************************************
#if MeasureFunctionSW

#define 	MESOF_NUM		2048
#define 	GYROFFSET_H		( 0x06D6 << 16 )
#define		GSENS			( 4096 << 16 )
#define		GSENS_MARG		(GSENS / 4)
#define		POSTURETH		(GSENS - GSENS_MARG)
#define		ZG_MRGN			(409 << 16)

//********************************************************************************
// Function Name 	: MeasFil
// Retun Value		: NON
// Argment Value	: Measure Filter Mode
// Explanation		: Measure Filter Setting Function
// History			: First edition
//********************************************************************************
static void MeasFil(struct cam_ois_ctrl_t *o_ctrl, UINT_8 UcMesMod ) // 18.0446kHz/15.027322kHz
{
	UINT_32	UlMeasFilaA , UlMeasFilaB , UlMeasFilaC ;
	UINT_32	UlMeasFilbA , UlMeasFilbB , UlMeasFilbC ;

	if( !UcMesMod ) { // Hall Bias&Offset Adjust
		UlMeasFilaA	=	0x03E4526B ;// LPF 150Hz
		UlMeasFilaB	=	0x03E4526B ;
		UlMeasFilaC	=	0x78375B2B ;
		UlMeasFilbA	=	0x7FFFFFFF ;// Through
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;

	} else if( UcMesMod == LOOPGAIN ) { // Loop Gain Adjust

		UlMeasFilaA	=	0x1621ECCD ;// LPF1000Hz
		UlMeasFilaB	=	0x1621ECCD ;
		UlMeasFilaC	=	0x53BC2664 ;
		UlMeasFilbA	=	0x7F33C48F ;// HPF30Hz
		UlMeasFilbB	=	0x80CC3B71 ;
		UlMeasFilbC	=	0x7E67891F ;

	} else if( UcMesMod == LOOPGAIN2 ) {				// Loop Gain Adjust2

		UlMeasFilaA	=	0x025D2733 ;// LPF90Hz
		UlMeasFilaB	=	0x025D2733 ;
		UlMeasFilaC	=	0x7B45B19B ;
		UlMeasFilbA	=	0x7FBBA379 ;// HPF10Hz
		UlMeasFilbB	=	0x80445C87 ;
		UlMeasFilbC	=	0x7F7746F1 ;

	} else if( UcMesMod == THROUGH ) {				// for Through

		UlMeasFilaA	=	0x7FFFFFFF ;// Through
		UlMeasFilaB	=	0x00000000 ;
		UlMeasFilaC	=	0x00000000 ;
		UlMeasFilbA	=	0x7FFFFFFF ;// Through
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;

	} else if( UcMesMod == NOISE ) {				// SINE WAVE TEST for NOISE

		UlMeasFilaA	=	0x03E4526B ;// LPF150Hz
		UlMeasFilaB	=	0x03E4526B ;
		UlMeasFilaC	=	0x78375B2B ;
		UlMeasFilbA	=	0x03E4526B ;// LPF150Hz
		UlMeasFilbB	=	0x03E4526B ;
		UlMeasFilbC	=	0x78375B2B ;

	} else if(UcMesMod == OSCCHK) {
		UlMeasFilaA	=	0x078DD83D ;// LPF300Hz
		UlMeasFilaB	=	0x078DD83D ;
		UlMeasFilaC	=	0x70E44F85 ;
		UlMeasFilbA	=	0x078DD83D ;// LPF300Hz
		UlMeasFilbB	=	0x078DD83D ;
		UlMeasFilbC	=	0x70E44F85 ;

	} else if( UcMesMod == SELFTEST ) {				// GYRO SELF TEST

		UlMeasFilaA	=	0x1621ECCD ;// LPF1000Hz
		UlMeasFilaB	=	0x1621ECCD ;
		UlMeasFilaC	=	0x53BC2664 ;
		UlMeasFilbA	=	0x7FFFFFFF ;// Through
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;

	} else {
		UlMeasFilaA	=	0x7FFFFFFF ;
		UlMeasFilaB	=	0x00000000 ;
		UlMeasFilaC	=	0x00000000 ;
		UlMeasFilbA	=	0x7FFFFFFF ;
		UlMeasFilbB	=	0x00000000 ;
		UlMeasFilbC	=	0x00000000 ;
	}

	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_a1, UlMeasFilaA, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_b1, UlMeasFilaB, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_c1, UlMeasFilaC, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_a2, UlMeasFilbA, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_b2, UlMeasFilbB, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Coeff_c2, UlMeasFilbC, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_a1, UlMeasFilaA, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_b1, UlMeasFilaB, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_c1, UlMeasFilaC, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_a2, UlMeasFilbA, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_b2, UlMeasFilbB, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Coeff_c2, UlMeasFilbC, 0) ;
}
static void	MemoryClear(struct cam_ois_ctrl_t *o_ctrl, UINT_16 UsSourceAddress, UINT_16 UsClearSize )
{
	UINT_16	UsLoopIndex ;

	for ( UsLoopIndex = 0 ; UsLoopIndex < UsClearSize ;  ) {
		RamWrite32A(o_ctrl, UsSourceAddress, 0x00000000, 0) ;
		UsSourceAddress += 4;
		UsLoopIndex += 4 ;
	}
}
static void	SetTransDataAdr(struct cam_ois_ctrl_t *o_ctrl, UINT_16 UsLowAddress , UINT_32 UlLowAdrBeforeTrans )
{
	UnDwdVal	StTrsVal ;

	if( UlLowAdrBeforeTrans < 0x00009000 ){
		StTrsVal.StDwdVal.UsHigVal = (UINT_16)(( UlLowAdrBeforeTrans & 0x0000F000 ) >> 8 ) ;
		StTrsVal.StDwdVal.UsLowVal = (UINT_16)( UlLowAdrBeforeTrans & 0x00000FFF ) ;
	}else{
		StTrsVal.UlDwdVal = UlLowAdrBeforeTrans ;
	}
	RamWrite32A(o_ctrl, UsLowAddress	,	StTrsVal.UlDwdVal, 0);

}
#define 	ONE_MSEC_COUNT	15
static void	SetWaitTime(struct cam_ois_ctrl_t *o_ctrl, UINT_16 UsWaitTime )
{
	RamWrite32A(o_ctrl, WaitTimerData_UiWaitCounter, 0, 0) ;
	RamWrite32A(o_ctrl, WaitTimerData_UiTargetCount, (UINT_32)(ONE_MSEC_COUNT * UsWaitTime), 0) ;
}
static void	ClrMesFil(struct cam_ois_ctrl_t *o_ctrl)
{
	RamWrite32A(o_ctrl, MeasureFilterA_Delay_z11, 0, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Delay_z12, 0, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterA_Delay_z21, 0, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterA_Delay_z22, 0, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterB_Delay_z11, 0, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Delay_z12, 0, 0) ;

	RamWrite32A(o_ctrl, MeasureFilterB_Delay_z21, 0, 0) ;
	RamWrite32A(o_ctrl, MeasureFilterB_Delay_z22, 0, 0) ;
}

//********************************************************************************
// Function Name 	: MeasAddressSelection
//********************************************************************************
static void MeasAddressSelection( UINT_8 mode , INT_32 * measadr_a , INT_32 * measadr_b )
{
	if( mode == 0 ){
		*measadr_a		=	LC898129_GYRO_RAM_GX_ADIDAT ;
		*measadr_b		=	GYRO_RAM_GY_ADIDAT ;
	}else if( mode == 1 ){
		*measadr_a		=	GYRO_ZRAM_GZ_ADIDAT ;
		*measadr_b		=	ACCLRAM_Z_AC_ADIDAT ;
	}else{
		*measadr_a		=	ACCLRAM_X_AC_ADIDAT ;
		*measadr_b		=	ACCLRAM_Y_AC_ADIDAT ;
	}
}

//********************************************************************************
// Function Name 	: MeasureStart
//********************************************************************************
static void	MeasureStart(struct cam_ois_ctrl_t *o_ctrl, INT_32 SlMeasureParameterNum , INT_32 SlMeasureParameterA , INT_32 SlMeasureParameterB )
{
	MemoryClear(o_ctrl, StMeasureFunc, sizeof( MeasureFunction_Type ) ) ;
	RamWrite32A(o_ctrl, StMeasFunc_MFA_SiMax1, 0x80000000, 0);
	RamWrite32A(o_ctrl, StMeasFunc_MFB_SiMax2, 0x80000000, 0);
	RamWrite32A(o_ctrl, StMeasFunc_MFA_SiMin1, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, StMeasFunc_MFB_SiMin2, 0x7FFFFFFF, 0);

	SetTransDataAdr(o_ctrl, StMeasFunc_MFA_PiMeasureRam1, ( UINT_32 )SlMeasureParameterA ) ;
	SetTransDataAdr(o_ctrl, StMeasFunc_MFB_PiMeasureRam2, ( UINT_32 )SlMeasureParameterB ) ;
	RamWrite32A(o_ctrl, StMeasFunc_SiSampleNum, 0, 0);
	ClrMesFil(o_ctrl) ;
	SetWaitTime(o_ctrl, 50) ;
	RamWrite32A(o_ctrl, StMeasFunc_SiSampleMax, SlMeasureParameterNum, 0);
}


//********************************************************************************
// Function Name 	: MeasureWait
//********************************************************************************
static void	MeasureWait(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32	SlWaitTimerSt ;
	UINT_16	UsTimeOut = 2000;

	do {
		RamRead32A(o_ctrl, StMeasFunc_SiSampleMax, &SlWaitTimerSt ) ;
		UsTimeOut--;
	} while ( SlWaitTimerSt && UsTimeOut );
}

//********************************************************************************
// Function Name 	: MeasGyAcOffset
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Measument Gyro & Accl Offset
// History			: First edition 2014.02.10 K.abe
//********************************************************************************
UINT_32	MeasGyAcOffset(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32	UlRsltSts;
	INT_32			SlMeasureParameterA , SlMeasureParameterB ;
	INT_32			SlMeasureParameterNum ;
	UnllnVal		StMeasValueA , StMeasValueB ;
	INT_32			SlMeasureAveValueA[3] , SlMeasureAveValueB[3] ;
	UINT_8			i ;

TRACE("MeasGyAcOffset\n") ;

	//ϒl
	MeasFil(o_ctrl, THROUGH ) ; 			// Set Measure Filter

	SlMeasureParameterNum	=	MESOF_NUM ; 			// Measurement times

	for( i=0 ; i<3 ; i++ )
	{
		MeasAddressSelection(i, &SlMeasureParameterA , &SlMeasureParameterB );
		MeasureStart(o_ctrl, SlMeasureParameterNum , SlMeasureParameterA , SlMeasureParameterB ) ; 			// Start measure
		MeasureWait(o_ctrl) ; 			// Wait complete of measurement

TRACE("Read Adr = %04x, %04xh \n",StMeasFunc_MFA_LLiIntegral1 + 4 , StMeasFunc_MFA_LLiIntegral1) ;
		RamRead32A(o_ctrl, StMeasFunc_MFA_LLiIntegral1 		, &StMeasValueA.StUllnVal.UlLowVal ) ;// X axis
		RamRead32A(o_ctrl, StMeasFunc_MFA_LLiIntegral1 + 4		, &StMeasValueA.StUllnVal.UlHigVal ) ;
		RamRead32A(o_ctrl, StMeasFunc_MFB_LLiIntegral2 		, &StMeasValueB.StUllnVal.UlLowVal ) ;// Y axis
		RamRead32A(o_ctrl, StMeasFunc_MFB_LLiIntegral2 + 4		, &StMeasValueB.StUllnVal.UlHigVal ) ;

TRACE("(%d) AOFT = %08x, %08xh \n",i,(unsigned int)StMeasValueA.StUllnVal.UlHigVal,(unsigned int)StMeasValueA.StUllnVal.UlLowVal) ;
TRACE("(%d) BOFT = %08x, %08xh \n",i,(unsigned int)StMeasValueB.StUllnVal.UlHigVal,(unsigned int)StMeasValueB.StUllnVal.UlLowVal) ;
		SlMeasureAveValueA[i] = (INT_32)( (INT_64)StMeasValueA.UllnValue / SlMeasureParameterNum ) ;
		SlMeasureAveValueB[i] = (INT_32)( (INT_64)StMeasValueB.UllnValue / SlMeasureParameterNum ) ;
TRACE("AVEOFT = %08xh \n",(unsigned int)SlMeasureAveValueA[i]) ;
TRACE("AVEOFT = %08xh \n",(unsigned int)SlMeasureAveValueB[i]) ;
	}

	UlRsltSts = EXE_END ;

	if( abs(SlMeasureAveValueA[0]) > GYROFFSET_H )					UlRsltSts |= EXE_GXADJ ;
	if( abs(SlMeasureAveValueB[0]) > GYROFFSET_H ) 					UlRsltSts |= EXE_GYADJ ;
	if( abs(SlMeasureAveValueA[1]) > GYROFFSET_H ) 					UlRsltSts |= EXE_GZADJ ;
	// if(    (SlMeasureAveValueB[1]) < POSTURETH )					UlRsltSts |= EXE_AZADJ ;
	// if( abs(SlMeasureAveValueA[2]) > ZG_MRGN )						UlRsltSts |= EXE_AXADJ ;
	// if( abs(SlMeasureAveValueB[2]) > ZG_MRGN )						UlRsltSts |= EXE_AYADJ ;
	// if( abs( GSENS  - SlMeasureAveValueB[1]) > ZG_MRGN )			UlRsltSts |= EXE_AZADJ ;

	if( UlRsltSts == EXE_END ){
		RamWrite32A(o_ctrl, GYRO_RAM_GXOFFZ ,		SlMeasureAveValueA[0], 0) ; 			// X axis Gyro offset
		RamWrite32A(o_ctrl, GYRO_RAM_GYOFFZ ,		SlMeasureAveValueB[0], 0) ; 			// Y axis Gyro offset
		RamWrite32A(o_ctrl, GYRO_ZRAM_GZOFFZ ,		SlMeasureAveValueA[1], 0) ; 			// Z axis Gyro offset
		RamWrite32A(o_ctrl, ACCLRAM_X_AC_OFFSET ,	SlMeasureAveValueA[2], 0) ; 			// X axis Accel offset
		RamWrite32A(o_ctrl, ACCLRAM_Y_AC_OFFSET ,	SlMeasureAveValueB[2], 0) ; 			// Y axis Accel offset
		RamWrite32A(o_ctrl, ACCLRAM_Z_AC_OFFSET , 	SlMeasureAveValueB[1] - (INT_32)GSENS, 0) ;// Z axis Accel offset

		RamWrite32A(o_ctrl, GYRO_RAM_GYROX_OFFSET , 0x00000000, 0) ; // X axis Drift Gyro offset
		RamWrite32A(o_ctrl, GYRO_RAM_GYROY_OFFSET , 0x00000000, 0) ; // Y axis Drift Gyro offset
		RamWrite32A(o_ctrl, GyroRAM_Z_GYRO_OFFSET , 0x00000000, 0) ; // Z axis Drift Gyro offset
		RamWrite32A(o_ctrl, GyroFilterDelayX_GXH1Z2 , 0x00000000, 0) ;// X axis H1Z2 Clear
		RamWrite32A(o_ctrl, GyroFilterDelayY_GYH1Z2 , 0x00000000, 0) ;// Y axis H1Z2 Clear
		RamWrite32A(o_ctrl, AcclFilDly_X + 8 ,  0x00000000, 0) ; 	// X axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Y + 8 ,  0x00000000, 0) ; 	// Y axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Z + 8 ,  0x00000000, 0) ; 	// Z axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_X + 12 , 0x00000000, 0) ; 	// X axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Y + 12 , 0x00000000, 0) ; 	// Y axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Z + 12 , 0x00000000, 0) ; 	// Z axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_X + 16 , 0x00000000, 0) ; 	// X axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Y + 16 , 0x00000000, 0) ; 	// Y axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Z + 16 , 0x00000000, 0) ; 	// Z axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_X + 20 , 0x00000000, 0) ; 	// X axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Y + 20 , 0x00000000, 0) ; 	// Y axis Accl LPF Clear
		RamWrite32A(o_ctrl, AcclFilDly_Z + 20 , 0x00000000, 0) ; 	// Z axis Accl LPF Clear
	}
	return( UlRsltSts );
}
#endif

//********************************************************************************
//
// Function Name 	: Set Angle Correct Code
//
//********************************************************************************
#if SetAngleCorrectSW
//********************************************************************************
// Function Name 	: SetGyroOffset
//********************************************************************************
void	SetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, UINT_16 GyroOffsetX, UINT_16 GyroOffsetY, UINT_16 GyroOffsetZ )
{
	RamWrite32A(o_ctrl, GYRO_RAM_GXOFFZ , (( GyroOffsetX << 16 ) & 0xFFFF0000 ), 0) ;
	RamWrite32A(o_ctrl, GYRO_RAM_GYOFFZ , (( GyroOffsetY << 16 ) & 0xFFFF0000 ), 0) ;
	RamWrite32A(o_ctrl, GYRO_ZRAM_GZOFFZ , (( GyroOffsetZ << 16 ) & 0xFFFF0000 ), 0) ;
}

//********************************************************************************
// Function Name 	: SetAcclOffset
//********************************************************************************
void	SetAcclOffset(struct cam_ois_ctrl_t *o_ctrl, UINT_16 AcclOffsetX, UINT_16 AcclOffsetY, UINT_16 AcclOffsetZ )
{
	RamWrite32A(o_ctrl, ACCLRAM_X_AC_OFFSET , ( ( AcclOffsetX << 16 ) & 0xFFFF0000 ), 0) ;
	RamWrite32A(o_ctrl, ACCLRAM_Y_AC_OFFSET , ( ( AcclOffsetY << 16 ) & 0xFFFF0000 ), 0) ;
	RamWrite32A(o_ctrl, ACCLRAM_Z_AC_OFFSET , ( ( AcclOffsetZ << 16 ) & 0xFFFF0000 ), 0) ;
}


//********************************************************************************
// Function Name 	: GetGyroOffset
//********************************************************************************
void	GetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, UINT_16* GyroOffsetX, UINT_16* GyroOffsetY, UINT_16* GyroOffsetZ )
{
	UINT_32	ReadValX, ReadValY, ReadValZ;
	RamRead32A(o_ctrl, GYRO_RAM_GXOFFZ  , &ReadValX );
	RamRead32A(o_ctrl, GYRO_RAM_GYOFFZ  , &ReadValY );
	RamRead32A(o_ctrl, GYRO_ZRAM_GZOFFZ , &ReadValZ );
	*GyroOffsetX = ( UINT_16 )(( ReadValX >> 16) & 0x0000FFFF );
	*GyroOffsetY = ( UINT_16 )(( ReadValY >> 16) & 0x0000FFFF );
	*GyroOffsetZ = ( UINT_16 )(( ReadValZ >> 16) & 0x0000FFFF );
}


//********************************************************************************
// Function Name 	: GetAcclOffset
//********************************************************************************
void	GetAcclOffset(struct cam_ois_ctrl_t *o_ctrl, UINT_16* AcclOffsetX, UINT_16* AcclOffsetY, UINT_16* AcclOffsetZ )
{
	UINT_32	ReadValX, ReadValY, ReadValZ;
	RamRead32A(o_ctrl, ACCLRAM_X_AC_OFFSET , &ReadValX );
	RamRead32A(o_ctrl, ACCLRAM_Y_AC_OFFSET , &ReadValY );
	RamRead32A(o_ctrl, ACCLRAM_Z_AC_OFFSET , &ReadValZ );
	*AcclOffsetX = ( UINT_16 )(( ReadValX >> 16) & 0x0000FFFF );
	*AcclOffsetY = ( UINT_16 )(( ReadValY >> 16) & 0x0000FFFF );
	*AcclOffsetZ = ( UINT_16 )(( ReadValZ >> 16) & 0x0000FFFF );
}


const UINT_8 PACT0Tbl[] = { 0xFF, 0xFF };/* Dummy table */
//const UINT_8 PACT1Tbl[] = { 0x64, 0x9B };/* [Huangshan] */
// const UINT_8 PACT1Tbl[] = { 0x46, 0xB9 };/* [Rose][Xuchang][ChangAn] */
const UINT_8 PACT1Tbl[] = { 0x64, 0x9B };/* [Rose][Xuchang][ChangAn] */ /* test for old module */

//********************************************************************************
// Function Name 	: SetAngleCorrection
//********************************************************************************
UINT_8 SetAngleCorrection(struct cam_ois_ctrl_t *o_ctrl, INT_32 DegreeGap, UINT_8 SelectAct, UINT_8 Arrangement )
{
	//double OffsetAngle = 0.0f;
	INT_32 Slgx45x = 0, Slgx45y = 0;
	INT_32 Slgy45y = 0, Slgy45x = 0;

	UINT_8	UcCnvF = 0;

	if( ( DegreeGap > 180.0f) || ( DegreeGap < -180.0f ) ) return ( 1 );
	if( Arrangement >= 2 ) return ( 1 );

/************************************************************************/
/*      	Gyro angle correction										*/
/************************************************************************/
	switch(SelectAct) {
		case 0x00 :
		case 0x01 :
		case 0x02 :
		case 0x03 :
		case 0x05 :
		case 0x06 :
		case 0x07 :
			//OffsetAngle = (double)( DegreeGap ) * 3.141592653589793238 / 180.0f ;
			UcCnvF = PACT1Tbl[ Arrangement ];
			break;
		default :
			break;
	}

	SetGyroCoef(o_ctrl, UcCnvF );
	SetAccelCoef(o_ctrl, UcCnvF );

	//***********************************************//
	// Gyro & Accel rotation correction
	//***********************************************//
	#if 0
	Slgx45x = (INT_32)( cos( OffsetAngle )*2147483647.0);
	Slgx45y = (INT_32)( sin( OffsetAngle )*2147483647.0);
	Slgy45y = (INT_32)( cos( OffsetAngle )*2147483647.0);
	Slgy45x = (INT_32)(-sin( OffsetAngle )*2147483647.0);
	#endif
	#if 0
	// 0
		Slgx45x = 0x7fffffff;
		Slgx45y = 0;
		Slgy45y = 0x7fffffff;
		Slgy45x = 0;

	// 180
		Slgx45x = 0x80000001;
		Slgx45y = 0;
		Slgy45y = 0x80000001;
		Slgy45x = 0;

	// 90
		Slgx45x = 0;
		Slgx45y = 0x7fffffff;
		Slgy45y = 0;
		Slgy45x = 0x80000001;
	#endif
	#if 1
	// -90
		Slgx45x = 0;
		Slgx45y = 0x80000001;
		Slgy45y = 0;
		Slgy45x = 0x7fffffff;
	#endif
	CAM_DBG(CAM_OIS, "SetAngleCorrection Slgx45x 0x%x, Slgx45y 0x%x, Slgx45x 0x%x, Slgx45y 0x%x", Slgx45x, Slgx45y, Slgy45y, Slgy45x);
	//***** map address is LC898129 Skelton Project *****
	RamWrite32A(o_ctrl, GyroFilterTableX_gx45x,  (UINT_32)Slgx45x, 0);
	RamWrite32A(o_ctrl, GyroFilterTableX_gx45y,  (UINT_32)Slgx45y, 0);
	RamWrite32A(o_ctrl, GyroFilterTableY_gy45y,  (UINT_32)Slgy45y, 0);
	RamWrite32A(o_ctrl, GyroFilterTableY_gy45x,  (UINT_32)Slgy45x, 0);
	RamWrite32A(o_ctrl, Accl45Filter_XAmain, 	 (UINT_32)Slgx45x, 0);
	RamWrite32A(o_ctrl, Accl45Filter_XAsub, 	 (UINT_32)Slgx45y, 0);
	RamWrite32A(o_ctrl, Accl45Filter_YAmain, 	 (UINT_32)Slgy45y, 0);
	RamWrite32A(o_ctrl, Accl45Filter_YAsub, 	 (UINT_32)Slgy45x, 0);

	return ( 0 );
}

static void	SetGyroCoef(struct cam_ois_ctrl_t *o_ctrl, UINT_8 UcCnvF )
{
	INT_32 Slgxx = 0, Slgxy = 0;
	INT_32 Slgyy = 0, Slgyx = 0;
	INT_32 Slgzp = 0;
	/************************************************/
	/*  signal convet								*/
	/************************************************/
	switch( UcCnvF & 0xE0 ){
		/* HX <== GX , HY <== GY */
	case 0x00:
		Slgxx = 0x7FFFFFFF ;Slgxy = 0x00000000 ;Slgyy = 0x7FFFFFFF ;Slgyx = 0x00000000 ;break;//HX<==GX(NEG), HY<==GY(NEG)
	case 0x20:
		Slgxx = 0x7FFFFFFF ;Slgxy = 0x00000000 ;Slgyy = 0x80000001 ;Slgyx = 0x00000000 ;break;//HX<==GX(NEG), HY<==GY(POS)
	case 0x40:
		Slgxx = 0x80000001 ;Slgxy = 0x00000000 ;Slgyy = 0x7FFFFFFF ;Slgyx = 0x00000000 ;break;//HX<==GX(POS), HY<==GY(NEG)
	case 0x60:
		Slgxx = 0x80000001 ;Slgxy = 0x00000000 ;Slgyy = 0x80000001 ;Slgyx = 0x00000000 ;break;//HX<==GX(POS), HY<==GY(POS)
		/* HX <== GY , HY <== GX */
	case 0x80:
		Slgxx = 0x00000000 ;Slgxy = 0x7FFFFFFF ;Slgyy = 0x00000000 ;Slgyx = 0x7FFFFFFF ;break;//HX<==GY(NEG), HY<==GX(NEG)
	case 0xA0:
		Slgxx = 0x00000000 ;Slgxy = 0x7FFFFFFF ;Slgyy = 0x00000000 ;Slgyx = 0x80000001 ;break;//HX<==GY(NEG), HY<==GX(POS)
	case 0xC0:
		Slgxx = 0x00000000 ;Slgxy = 0x80000001 ;Slgyy = 0x00000000 ;Slgyx = 0x7FFFFFFF ;break;//HX<==GY(POS), HY<==GX(NEG)
	case 0xE0:
		Slgxx = 0x00000000 ;Slgxy = 0x80000001 ;Slgyy = 0x00000000 ;Slgyx = 0x80000001 ;break;//HX<==GY(NEG), HY<==GX(NEG)
	}
	switch( UcCnvF & 0x10 ){
	case 0x00:
		Slgzp = 0x7FFFFFFF ;break; 																	//GZ(POS)
	case 0x10:
		Slgzp = 0x80000001 ;break; 																	//GZ(NEG)
	}

	//***** map address is LC898129 Skelton Project *****
	RamWrite32A(o_ctrl, MS_SEL_GX0 , (UINT_32)Slgxx, 0);
	RamWrite32A(o_ctrl, MS_SEL_GX1 , (UINT_32)Slgxy, 0);
	RamWrite32A(o_ctrl, MS_SEL_GY0 , (UINT_32)Slgyy, 0);
	RamWrite32A(o_ctrl, MS_SEL_GY1 , (UINT_32)Slgyx, 0);
	RamWrite32A(o_ctrl, MS_SEL_GZ ,  (UINT_32)Slgzp, 0);
}

static void	SetAccelCoef(struct cam_ois_ctrl_t *o_ctrl, UINT_8 UcCnvF )
{
	INT_32 Slaxx = 0, Slaxy = 0;
	INT_32 Slayy = 0, Slayx = 0;
	INT_32 Slazp = 0;

	switch( UcCnvF & 0x0E ){
		/* HX <== AX , HY <== AY */
	case 0x00:
		Slaxx = 0x7FFFFFFF ;Slaxy = 0x00000000 ;Slayy = 0x7FFFFFFF ;Slayx = 0x00000000 ;break;//HX<==AX(NEG), HY<==AY(NEG)
	case 0x02:
		Slaxx = 0x7FFFFFFF ;Slaxy = 0x00000000 ;Slayy = 0x80000001 ;Slayx = 0x00000000 ;break;//HX<==AX(NEG), HY<==AY(POS)
	case 0x04:
		Slaxx = 0x80000001 ;Slaxy = 0x00000000 ;Slayy = 0x7FFFFFFF ;Slayx = 0x00000000 ;break;//HX<==AX(POS), HY<==AY(NEG)
	case 0x06:
		Slaxx = 0x80000001 ;Slaxy = 0x00000000 ;Slayy = 0x80000001 ;Slayx = 0x00000000 ;break;//HX<==AX(POS), HY<==AY(POS)
		/* HX <== AY , HY <== AX */
	case 0x08:
		Slaxx = 0x00000000 ;Slaxy = 0x7FFFFFFF ;Slayy = 0x00000000 ;Slayx = 0x7FFFFFFF ;break;//HX<==AY(NEG), HY<==AX(NEG)
	case 0x0A:
		Slaxx = 0x00000000 ;Slaxy = 0x7FFFFFFF ;Slayy = 0x00000000 ;Slayx = 0x80000001 ;break;//HX<==AY(NEG), HY<==AX(POS)
	case 0x0C:
		Slaxx = 0x00000000 ;Slaxy = 0x80000001 ;Slayy = 0x00000000 ;Slayx = 0x7FFFFFFF ;break;//HX<==AY(POS), HY<==AX(NEG)
	case 0x0E:
		Slaxx = 0x00000000 ;Slaxy = 0x80000001 ;Slayy = 0x00000000 ;Slayx = 0x80000001 ;break;//HX<==AY(NEG), HY<==AX(NEG)
	}
	switch( UcCnvF & 0x01 ){
	case 0x00:
		Slazp = 0x7FFFFFFF ;break; 																	//AZ(POS)
	case 0x01:
		Slazp = 0x80000001 ;break; 																	//AZ(NEG)
	}

	//***** map address is LC898129 Skelton Project *****
	RamWrite32A(o_ctrl, MS_SEL_AX0 , (UINT_32)Slaxx, 0);
	RamWrite32A(o_ctrl, MS_SEL_AX1 , (UINT_32)Slaxy, 0);
	RamWrite32A(o_ctrl, MS_SEL_AY0 , (UINT_32)Slayy, 0);
	RamWrite32A(o_ctrl, MS_SEL_AY1 , (UINT_32)Slayx, 0);
	RamWrite32A(o_ctrl, MS_SEL_AZ ,  (UINT_32)Slazp, 0);

}
#endif

//********************************************************************************
// Function Name 	: BootMode
// Retun Value		: NON
// Argment Value	: NON
// Explanation		:
//********************************************************************************
void BootMode(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32	ReadVal;

	IORead32A(o_ctrl, SYSDSP_REMAP, &ReadVal ) ;
	ReadVal = (ReadVal & 0x1) | 0x00001400;
	IOWrite32A(o_ctrl, SYSDSP_REMAP, ReadVal, 0) ; 					// CORE_RST[12], MC_IGNORE2[10] = 1
	usleep_range(15000, 15010); 											// Wait 15ms
}

//********************************************************************************
// Function Name 	: UpdataCodeWrite129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129UpData CodePmemWriteB
// History			: First edition 								2019.03.11
//********************************************************************************
void UpdataCodeWrite129(struct cam_ois_ctrl_t *o_ctrl)
{
	// Pmemް̗vfvZB
	int WriteNum1 = UpDataCodeSize / 50; // 250byte̼ݼ_writeŁA]񐔂߂
	int WriteNum2 = UpDataCodeSize % 50; // c̼ݼ_writeŁA]Acbyte߂
	int WriteNum3 = WriteNum2 * 5 + 2; 	// c̼ݼ_writeŁAI2C]񐔂߂ ("+2"́Au0x40,0x00vCommandt)

	unsigned char	data[252];
	unsigned int Num;
	unsigned int i;

	unsigned int n;
	unsigned int m = 0;

	// Pmem address set
	data[0] = 0x30; // CmdH
	data[1] = 0x00; // CmdL
	data[2] = 0x00; // DataH
	data[3] = 0x08; // DataMH
	data[4] = 0x00; // DataML
	data[5] = 0x00; // DataL

	CntWrt(o_ctrl, data, 6, 0);

	// Pmem data write

	data[0] = 0x40; // CmdH
	data[1] = 0x00; // CmdL

	// 250byte̓]
	for(Num=0; Num<WriteNum1; Num++ ) {						// 250byteAwriteł񐔂񂵂ē]
		// Pmem̃f[^Zbg
		n = 2;
		for(i=0; i<50; i++ ) {
			data[n++] = CcUpdataCode129[m++];// 1byte
			data[n++] = CcUpdataCode129[m++];// 2byte
			data[n++] = CcUpdataCode129[m++];// 3byte
			data[n++] = CcUpdataCode129[m++];// 4byte
			data[n++] = CcUpdataCode129[m++];// 5byte
		}

		// Pmem̃f[^write (5byte~50==250byte)(Fullގ)
		CntWrt(o_ctrl, data, 252, 0);
	}

	// 250byteȉ̓]
	if(WriteNum2 != 0) {
		// Pmem̃f[^Zbg
		n = 2;
		for(i=0; i<WriteNum2; i++ ) {							// 250byteŁA]łȂ]]
			data[n++] = CcUpdataCode129[m++];// 1byte
			data[n++] = CcUpdataCode129[m++];// 2byte
			data[n++] = CcUpdataCode129[m++];// 3byte
			data[n++] = CcUpdataCode129[m++];// 4byte
			data[n++] = CcUpdataCode129[m++];// 5byte
		}

		// Pmem̃f[^write (5byte~18==90byte)(Fullގ)
		CntWrt(o_ctrl, data, WriteNum3, 0);
	}
}

//********************************************************************************
// Function Name 	: UpdataCodeWrite129_32
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129UpData CodePmemWriteB
// History			: First edition 								2020.01.24
//********************************************************************************
void UpdataCodeWrite129_32(struct cam_ois_ctrl_t *o_ctrl)
{
	// Pmemް̗vfvZB
	int WriteNum1 = UpDataCodeSize / 5; 	// 25byte̼ݼ_writeŁA]񐔂߂
	int WriteNum2 = UpDataCodeSize % 5; 	// c̼ݼ_writeŁA]Acbyte߂
	int WriteNum3 = WriteNum2 * 5 + 2; 	// c̼ݼ_writeŁAI2C]񐔂߂ ("+2"́Au0x40,0x00vCommandt)
//TRACE( "UpDataCodeSize = %d \n", UpDataCodeSize) ;
//TRACE( "WriteNum1 = %d \n", WriteNum1) ;
//TRACE( "WriteNum2 = %d \n", WriteNum2) ;
//TRACE( "WriteNum3 = %d \n", WriteNum3) ;

	unsigned char	data[32];
	unsigned int n;
	unsigned int m = 0;
	unsigned int Num=0;
	unsigned int i=0;

	// Pmem address set
	data[0] = 0x30; // CmdH
	data[1] = 0x00; // CmdL
	data[2] = 0x00; // DataH
	data[3] = 0x08; // DataMH
	data[4] = 0x00; // DataML
	data[5] = 0x00; // DataL

	CntWrt(o_ctrl, data, 6, 0);

	// Pmem data write

	data[0] = 0x40; // CmdH
	data[1] = 0x00; // CmdL

	// 25byte̓]
	for( Num=0; Num<WriteNum1; Num++ ) {						// 250byteAwriteł񐔂񂵂ē]
		// Pmem̃f[^Zbg
		n = 2;

		for( i=0; i<5; i++ ) {
			data[n++] = CcUpdataCode129[m++];// 1byte
			data[n++] = CcUpdataCode129[m++];// 2byte
			data[n++] = CcUpdataCode129[m++];// 3byte
			data[n++] = CcUpdataCode129[m++];// 4byte
			data[n++] = CcUpdataCode129[m++];// 5byte
		}

		// Pmem̃f[^write
		CntWrt(o_ctrl, data, 27, 0);
	}

	// 25byteȉ̓]
	if(WriteNum2 != 0) {
		// Pmem̃f[^Zbg
		n = 2;
		for( i=0; i<WriteNum2; i++ ) {							// 250byteŁA]łȂ]]
			data[n++] = CcUpdataCode129[m++];// 1byte
			data[n++] = CcUpdataCode129[m++];// 2byte
			data[n++] = CcUpdataCode129[m++];// 3byte
			data[n++] = CcUpdataCode129[m++];// 4byte
			data[n++] = CcUpdataCode129[m++];// 5byte
		}

		// Pmem̃f[^write
		CntWrt(o_ctrl, data, WriteNum3, 0);
	}
}

//********************************************************************************
// Function Name 	: UpdataCodeRead129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command  PmemreadsAUpdata CodeƔr
// History			: First edition 						2018.05.17
//********************************************************************************
UINT_8 UpdataCodeRead129(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned char	data[6];
	unsigned char	ReadData[5];
	unsigned int	ODNum = 0; 	// OrgDataNum
	unsigned char	NGflg = SUCCESS;// 0x01=NG  0x00=OK
	unsigned int 	i=0;
	unsigned char 	j=0;

	// PmemAhX set
	data[0] = 0x30; // CmdH
	data[1] = 0x00; // CmdL
	data[2] = 0x00; // DataH
	data[3] = 0x08; // DataMH
	data[4] = 0x00; // DataML
	data[5] = 0x00; // DataL
	CntWrt(o_ctrl, data, 6, 0);

	// Pmem f[^ read & verify
	for( i=0; i<UpDataCodeSize; i++ ) {
		CntRd(o_ctrl, 0x4000, ReadData , 5 );

		for(j=0; j<5; j++) {
			if(ReadData[j] != CcUpdataCode129[ ODNum++ ] ) { // Verify"NG"̏ꍇ
				NGflg = FAILURE;
			}
		}
	}

	return NGflg; // Verify
}

//********************************************************************************
// Function Name 	: UpdataCodeCheckSum129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898128 Command  Pmem̈̃`FbNTsƔ
// History			: First edition 						2018.05.07
//********************************************************************************
UINT_8 UpdataCodeCheckSum129(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned char	data[6];
	unsigned char	ReadData[8];
	unsigned long	UlCnt;
	uint32_t	UlReadVal;
	unsigned char	UcSndDat = SUCCESS;// 0x01=NG  0x00=OK

	int size = UpDataCodeSize;
	long long CheckSumCode = UpDataCodeCheckSum;
	unsigned char *p = (unsigned char *)&CheckSumCode;
	unsigned char i=0;

	// Program RAMCheckSum̋N
	data[0] = 0xF0; 									//CmdID
	data[1] = 0x0E; 									//CmdID
	data[2] = (unsigned char)((size >> 8) & 0x000000FF);//݃f[^(MSB)
	data[3] = (unsigned char)(size & 0x000000FF); 	//݃f[^
	data[4] = 0x00; 									//݃f[^
	data[5] = 0x00; 									//݃f[^(LSB)
	CntWrt(o_ctrl, data, 6, 0);

	// CheckSum̏I
	UlCnt = 0;
	do{
		if( UlCnt++ > 100 ) {
			UcSndDat = FAILURE;
			break;
		}
		RamRead32A(o_ctrl, 0x0088, &UlReadVal ); 					//PmCheck.ExecFlag̓ǂݏo
	}while ( UlReadVal != 0 );

	// CheckSuml̓ǂݏo
	if( UcSndDat == SUCCESS ) {
		CntRd(o_ctrl, 0xF00E, ReadData , 8 );

		// CheckSuml̔(ҒĺAHeaderdefineĂ)
		for(i=0; i<8; i++) {
			if(ReadData[7-i] != *p++ ) {  							// CheckSum Code̔
				UcSndDat = FAILURE;
			}
		}
	}

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: UnlockCodeSet129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command
// History			: First edition 						2018.05.15
//********************************************************************************
UINT_8 UnlockCodeSet129(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32 UlReadVal, UlCnt=0;

	do {
		IOWrite32A(o_ctrl, 0xE07554, 0xAAAAAAAA, 0); 					// UNLK_CODE1(E0_7554h) = AAAA_AAAAh
		IOWrite32A(o_ctrl, 0xE07AA8, 0x55555555, 0); 					// UNLK_CODE2(E0_7AA8h) = 5555_5555h
		IORead32A(o_ctrl, 0xE07014, &UlReadVal );
		if( (UlReadVal & 0x00000080) != 0 )	return ( SUCCESS ) ;// Check UNLOCK(E0_7014h[7]) ?
		usleep_range(1000, 1010);
	} while( UlCnt++ < 10 );
	return ( FAILURE );
}

//********************************************************************************
// Function Name 	: WritePermission129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command
// History			: First edition 						2018.05.15
//********************************************************************************
void WritePermission129(struct cam_ois_ctrl_t *o_ctrl)
{
	IOWrite32A(o_ctrl, 0xE074CC, 0x00000001, 0); 						// RSTB_FLA_WR(E0_74CCh[0])=1
	IOWrite32A(o_ctrl, 0xE07664, 0x00000010, 0); 						// FLA_WR_ON(E0_7664h[4])=1
}

//********************************************************************************
// Function Name 	: AddtionalUnlockCodeSet129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command
// History			: First edition 						2018.05.15
//********************************************************************************
void AddtionalUnlockCodeSet129(struct cam_ois_ctrl_t *o_ctrl)
{
	IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 0); 						// UNLK_CODE3(E0_7CCCh) = 0000_ACD5h
}

//********************************************************************************
// Function Name 	: UnlockCodeClear129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command
// History			: First edition 						2018.05.15
//********************************************************************************
UINT_8 UnlockCodeClear129(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32 UlDataVal, UlCnt=0;

	do {
		IOWrite32A(o_ctrl, 0xE07014, 0x00000010, 0 ); 					// UNLK_CODE3(E0_7014h[4]) = 1
		IORead32A(o_ctrl, 0xE07014, &UlDataVal );
		if( (UlDataVal & 0x00000080) == 0 )	return ( SUCCESS ) ;// Check UNLOCK(E0_7014h[7]) ?
		usleep_range(1000, 1010);
	} while( UlCnt++ < 10 );
	return ( FAILURE );
}

//********************************************************************************
// Function Name 	: AddtionalUnlockCodeSetInfo129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129 Command
// History			: First edition 						2018.05.15
//********************************************************************************
UINT_8 AddtionalUnlockCodeSetInfo129(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32 UlReadVal, UlCnt;
	UINT_8 UcSndDat;

	UcSndDat = FAILURE; 									// 0x01=NG  0x00=OK
	UlCnt = 0;
	do {
		IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0 ); 				// UNLK_CODE3(E0_7CCCh)=C5ADh
		IORead32A(o_ctrl, 0xE07014, &UlReadVal ); 				// UNLOCK(E0_7014h[2])
		if( (UlReadVal & 0x00000004) != 0 )	{					// Check UNLOCK(E0_7014h[2]) ?
			UcSndDat = SUCCESS;
			break;
		}
		usleep_range(1000, 1010);
	} while( UlCnt++ < 10 );

	if( UcSndDat == SUCCESS ) {
		UcSndDat = FAILURE; 								// 0x01=NG  0x00=OK
		UlCnt = 0;
		do {
			IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 0); 			// UNLK_CODE3(E0_7CCCh)=ACD5h
			IORead32A(o_ctrl, 0xE07014, &UlReadVal ); 			// UNLOCK(E0_7014h[0])
			if( (UlReadVal & 0x00000001) != 0 )	{				// Check UNLOCK(E0_7014h[0]) ?
				UcSndDat = SUCCESS;
				break;
			}
			usleep_range(1000, 1010);
		} while( UlCnt++ < 10 );
	}

	return UcSndDat;
}

//********************************************************************************
// Function Name    : InfoMatErase129
// Retun Value      : NON
// Argment Value    : UINT_8 InfoNo     0: Info Mat 0
//                  :                   1: Info Mat 1
//                  :                   2: Info Mat 2
// Explanation      : LC898129p Info Mat Erase
// History          : First edition 2019.08.31
//********************************************************************************
UINT_8 InfoMatErase129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo )
{
	UINT_8  UcSndDat = FAILURE; // 0x01=FAILURE, 0x00=OK
	UINT_32 UlCnt;
	UINT_32 UlDataVal;
	UINT_32 UlAddrVal;

TRACE("Info Mat Erase.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); 					// FLASH Standby Disable

	if( UnlockCodeSet129(o_ctrl) == SUCCESS ) {						// Unlock Code Set
		WritePermission129(o_ctrl); 							// Write Permission
		if( AddtionalUnlockCodeSetInfo129(o_ctrl) == SUCCESS ) {		// Additional Unlock Code Set
			//***** Block Erase Info0 Mat *****
			UlAddrVal = 0x00000000;
			UlAddrVal |= (UINT_32)((InfoNo & 0x01) + 0x01) << 16;
			IOWrite32A(o_ctrl, 0xE0700C, UlAddrVal, 0); 			// FLA_REGS(E0_700Ch[21]=0
			// FLA_TRMM(E0_700Ch[20]=0
			// FLA_INFM(E0_700Ch[18:16]=INF
			// FLA_ADR(E0_700Ch[13:0])=ADR
			IOWrite32A(o_ctrl, 0xE07010, 0x00000004, 0); 			// CMD(E0_7010h[3:0])=4h

			// Check BSY_FLA(E0_7018h[7]) flag
			UlCnt = 0;
			do{
				IORead32A(o_ctrl, 0xE07018, &UlDataVal ); 		// BSY_FLA(E0_7018h[7])
				if( (UlDataVal & 0x00000080) == 0 ) {
					UcSndDat = SUCCESS;
					break;
				}
				usleep_range(1000, 1010);
			}while ( UlCnt++ < 10 );

			if( UcSndDat == SUCCESS ) {
				if( UnlockCodeClear129(o_ctrl) == FAILURE ) {			// Unlock Code Clear
					UcSndDat = -4; // Unlock Code Clear Error
				}
			} else {
				UcSndDat = -3; // Info Mat Erase Error
			}
		} else {
			UcSndDat = -2; // Additional Unlock Code Error
		}
	} else {
		UcSndDat = -1; // Unlock Code Set Error
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: PageWrite129
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898129p Info Mat page Write
// History			: First edition 						2019.08.31
//********************************************************************************
UINT_8 PageWrite129(struct cam_ois_ctrl_t *o_ctrl, UINT_32 UlAddr, UINT_32 *pData )
{
	UINT_8  UcSndDat = FAILURE; 				// 0x01=FAILURE, 0x00=OK
	UINT_32 UlCnt;
	UINT_32 UlDataVal;
	int j=0 ;

	//***** Page Write *****
	IOWrite32A(o_ctrl, 0xE0700C, UlAddr, 0); 			// FLA_REGS(E0_700Ch[21]=0
													// FLA_TRMM(E0_700Ch[20]=0
													// FLA_INFM(E0_700Ch[18:16]=INF
													// FLA_ADR(E0_700Ch[13:0])=ADR
	IOWrite32A(o_ctrl, 0xE07010, 0x00000002, 0); 		// CMD(E0_7010h[3:0])=2h

	for(j=0 ; j<16 ; j++) {
		IOWrite32A(o_ctrl, 0xE07004, (*pData++), 0); 	// Set FLA_WDAT(E0_7004h[31:0])
	}

	// Check BSY_FLA(E0_7018h[7]) flag
	UlCnt = 0;
	do{
		IORead32A(o_ctrl, 0xE07018, &UlDataVal ); 	// BSY_FLA(E0_7018h[7])
		if( (UlDataVal & 0x00000080) == 0 ) {
			UcSndDat = SUCCESS;
			break;
		}
		usleep_range(1000, 1010);
	}while ( UlCnt++ < 10 );

	if( UcSndDat == SUCCESS ) {

		//***** Page Program *****
		IOWrite32A(o_ctrl, 0xE07010, 0x00000008, 0); 		// CMD(E0_7010h[3:0])=8h

		// Check BSY_FLA(E0_7018h[7]) flag
		UcSndDat = FAILURE;
		UlCnt = 0;
		do{
			IORead32A(o_ctrl, 0xE07018, &UlDataVal ); 	// BSY_FLA(E0_7018h[7])
			if( (UlDataVal & 0x00000080) == 0 ) {
				UcSndDat = SUCCESS;
				break;
			}
			usleep_range(1000, 1010);
		}while ( UlCnt++ < 10 );
	} else {
		UcSndDat = FAILURE; // Error
	}

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: PmemUpdate129
//********************************************************************************
UINT_8 PmemUpdate129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 dist, DOWNLOAD_TBL_EXT * ptr )
{
	UINT_8 data[BURST_LENGTH_UC +2 ];
	UINT_16 Remainder;
	UINT_8 ReadData[8];
	const UINT_8 *NcDataVal;
	UINT_16 SizeofCode;
	UINT_16 SizeofCheck;
	long long CheckSumCode;
	UINT_8 *p;
	UINT_32 i, j;
	UINT_32 UlReadVal, UlCnt , UlNum ;

	if( dist != 0 ) {
		NcDataVal = ptr->FromCode + 32;
		SizeofCode = (UINT_16)ptr->FromCode[9] << 8 | (UINT_16)ptr->FromCode[8];

		CheckSumCode = (long long)ptr->FromCode[19] << 56 | (long long)ptr->FromCode[18] << 48 |
			(long long)ptr->FromCode[17] << 40 | (long long)ptr->FromCode[16] << 32 |
			(long long)ptr->FromCode[15] << 24 | (long long)ptr->FromCode[14] << 16 |
			(long long)ptr->FromCode[13] << 8 | (long long)ptr->FromCode[12];

		SizeofCheck = SizeofCode;
	} else {
		NcDataVal = ptr->UpdataCode;
		SizeofCode = ptr->SizeUpdataCode;
		CheckSumCode = ptr->SizeUpdataCodeCksm;
		SizeofCheck = SizeofCode;
	}
	p = (UINT_8 *)&CheckSumCode;

//--------------------------------------------------------------------------------
// 1.
//--------------------------------------------------------------------------------
	RamWrite32A(o_ctrl, 0x3000, 0x00080000, 0);

	data[0] = 0x40;
	data[1] = 0x00;

	Remainder = ( (SizeofCode * 5) / BURST_LENGTH_UC );
	for(i=0 ; i< Remainder ; i++)
	{
		UlNum = 2;
		for(j=0 ; j < BURST_LENGTH_UC; j++){
			data[UlNum] =  *NcDataVal++;
			UlNum++;
		}

		CntWrt(o_ctrl, data, BURST_LENGTH_UC + 2, 0);
	}
	Remainder = ( (SizeofCode * 5) % BURST_LENGTH_UC);
	if (Remainder != 0 )
	{
		UlNum = 2;
		for(j=0 ; j < Remainder; j++){
			data[UlNum++] = *NcDataVal++;
		}
		CntWrt(o_ctrl, data, Remainder + 2, 0);  // Cmd 2Byte
	}

//--------------------------------------------------------------------------------
// 2.
//--------------------------------------------------------------------------------
	data[0] = 0xF0;
	data[1] = 0x0E;
	data[2] = (unsigned char)((SizeofCheck >> 8) & 0x000000FF);
	data[3] = (unsigned char)(SizeofCheck & 0x000000FF);
	data[4] = 0x00;
	data[5] = 0x00;

	CntWrt(o_ctrl, data, 6, 0) ;

	UlCnt = 0;
	do{
		usleep_range(1000, 1010);
		if( UlCnt++ > 10 ) {
			return (0x21) ;
		}
		RamRead32A(o_ctrl, 0x0088, &UlReadVal );
	}while ( UlReadVal != 0 );

	CntRd(o_ctrl, 0xF00E, ReadData , 8 );

	for( i=0; i<8; i++) {
		if(ReadData[7-i] != *p++ ) {
			return (0x22) ;
		}
	}
	if( dist != 0 ){
		RamWrite32A(o_ctrl, 0xF001, 0, 0);
	}

	return( 0 );
}

//********************************************************************************
// Function Name 	: EraseUserMat129
//********************************************************************************
UINT_8 EraseUserMat129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 StartBlock, UINT_8 EndBlock )
{
	UINT_32 i ;
	UINT_32 UlReadVal, UlCnt ;

	IOWrite32A(o_ctrl, 0xE0701C , 0x00000000, 0) ;
	RamWrite32A(o_ctrl, 0xF007, 0x00000000, 0) ;

	for( i = StartBlock; i < EndBlock; i++ ) {
		RamWrite32A(o_ctrl, 0xF00A, ( i << 10 ), 0) ;
		RamWrite32A(o_ctrl, 0xF00C, 0x00000020, 0) ;

		usleep_range(5000, 5010) ;
		UlCnt = 0 ;
		do {
			usleep_range(5000, 5010) ;
			if( UlCnt++ > 100 ) {
				IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ;
				return ( 0x31 ) ;
			}
			RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
		}while ( UlReadVal != 0 ) ;
	}
	IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ;
	return(0);
}

//********************************************************************************
// Function Name 	: ProgramFlash129_Standard
//********************************************************************************
UINT_8 ProgramFlash129_Standard(struct cam_ois_ctrl_t *o_ctrl, DOWNLOAD_TBL_EXT *ptr )
{
	UINT_32 UlReadVal, UlCnt , UlNum ;
	UINT_8 data[ ( BURST_LENGTH_FC + 3 ) ] ;
	UINT_32 i, j ;

	const UINT_8 *NcFromVal = ptr->FromCode + 64 ;
	const UINT_8 *NcFromVal1st = ptr->FromCode ;
	UINT_8 UcOddEvn = 0;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF007, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00A, 0x00000010, 0);
	data[ 0 ] = 0xF0;
	data[ 1 ] = 0x08;
	data[ 2 ] = 0x00;

	for( i = 1; i < ( ptr->SizeFromCode / 64 ); i++ ) {
		if( ++UcOddEvn > 1 ) UcOddEvn = 0 ;
		if( UcOddEvn == 0 ) data[ 1 ] = 0x08 ;
		else data[ 1 ] = 0x09 ;

#if (BURST_LENGTH_FC == 32)
		data[ 2 ] = 0x00 ;
		UlNum = 3 ;
		for( j = 0; j < BURST_LENGTH_FC; j++ ) {
			data[ UlNum++ ] = *NcFromVal++ ;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;

	  	data[ 2 ] = 0x20 ;
		UlNum = 3 ;
		for( j = 0; j < BURST_LENGTH_FC; j++ ) {
			data[ UlNum++ ] = *NcFromVal++ ;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;

#elif (BURST_LENGTH_FC == 64)
		UlNum = 3 ;
		for( j = 0; j < BURST_LENGTH_FC; j++ ) {
			data[ UlNum++ ] = *NcFromVal++ ;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;
#endif

		RamWrite32A(o_ctrl, 0xF00B, 0x00000010, 0) ;
		UlCnt = 0 ;
		if( UcOddEvn == 0 ) {
			do {
				usleep_range(1000, 1010);
				RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
				if( UlCnt++ > 250 ) {
					IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
					return ( 0x41 ) ;
				}
			} while ( UlReadVal != 0 ) ;
		 	RamWrite32A(o_ctrl, 0xF00C, 0x00000004, 0) ;
		} else {
			do {
				usleep_range(1000, 1010);
				RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
				if( UlCnt++ > 250 ) {
					IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
					return ( 0x41 ) ;
				}
			} while ( UlReadVal != 0 ) ;
			RamWrite32A(o_ctrl, 0xF00C, 0x00000008, 0) ;
		}
	}

	UlCnt = 0 ;
	do {
		usleep_range(1000, 1010) ;
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
		if( UlCnt++ > 250 ) {
			IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
			return ( 0x41 ) ;
		}
	} while ( UlReadVal != 0 ) ;

	RamWrite32A(o_ctrl, 0xF00A, 0x00000000, 0) ;
	data[ 1 ] = 0x08 ;

#if (BURST_LENGTH_FC == 32)
	data[ 2 ] = 0x00 ;
	UlNum = 3 ;
	for( j = 0; j < BURST_LENGTH_FC; j++ ) {
		data[ UlNum++ ] = *NcFromVal1st++ ;
	}
	CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;

	data[ 2 ] = 0x20 ;
	UlNum = 3 ;
	for( j = 0; j < BURST_LENGTH_FC; j++ ) {
		data[ UlNum++ ] = *NcFromVal1st++ ;
	}
	CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;
#elif (BURST_LENGTH_FC == 64)
	data[ 2 ] = 0x00 ;
	UlNum = 3 ;
	for( j = 0; j < BURST_LENGTH_FC; j++ ) {
		data[ UlNum++ ] = *NcFromVal1st++ ;
	}
	CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0) ;
#endif

	RamWrite32A(o_ctrl, 0xF00B, 0x00000010, 0) ;
	UlCnt = 0 ;
	do {
		usleep_range(1000, 1010);
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
		if( UlCnt++ > 250 ) {
			IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
			return ( 0x41 ) ;
		}
	} while ( UlReadVal != 0 ) ;
 	RamWrite32A(o_ctrl, 0xF00C, 0x00000004, 0) ;

	UlCnt = 0 ;
	do {
		usleep_range(1000, 1010) ;
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
		if( UlCnt++ > 250 ) {
			IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
			return ( 0x41 ) ;
		}
	} while ( (UlReadVal & 0x0000000C) != 0 ) ;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0) ;
	return( 0 );
}

//********************************************************************************
// Function Name 	: FlashUpdate129
//********************************************************************************
UINT_8 FlashUpdate129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 chiperase, DOWNLOAD_TBL_EXT* ptr )
{
	UINT_8 ans=0 ;
	UINT_32 UlReadVal, UlCnt ;

	// Boot Mode
	BootMode(o_ctrl);

//--------------------------------------------------------------------------------
// 1.
//--------------------------------------------------------------------------------
 	ans = PmemUpdate129(o_ctrl, 0, ptr ) ;
	if(ans != 0) return ( ans ) ;

//--------------------------------------------------------------------------------
// 2.
//--------------------------------------------------------------------------------
	if( UnlockCodeSet129(o_ctrl) != 0 ) return ( 0x33 ) ;
	WritePermission129(o_ctrl) ;
	AddtionalUnlockCodeSet129(o_ctrl) ;

	if( chiperase != 0 )
	 	ans = EraseUserMat129(o_ctrl, 0, ERASE_BLOCKS ) ;
	else
		ans = EraseUserMat129(o_ctrl, 0, ERASE_BLOCKS ) ;

	if(ans != 0){
		if( UnlockCodeClear129(o_ctrl) != 0 ) return ( 0x32 ) ;
		else return ( ans ) ;
	}
//--------------------------------------------------------------------------------
// 3.
//--------------------------------------------------------------------------------
	ans = ProgramFlash129_Standard(o_ctrl, ptr ) ;

	if(ans != 0){
		if( UnlockCodeClear129(o_ctrl) != 0 ) return ( 0x43 ) ;
		else	return ( ans ) ;
	}

	if( UnlockCodeClear129(o_ctrl) != 0 ) return ( 0x43 ) ;
//--------------------------------------------------------------------------------
// 4.
//--------------------------------------------------------------------------------
	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0) ;
	RamWrite32A(o_ctrl, 0xF00A, 0x00000000, 0) ;
	RamWrite32A(o_ctrl, 0xF00D, ptr->SizeFromCodeValid, 0) ;

	RamWrite32A(o_ctrl, 0xF00C, 0x00000100, 0) ;
	usleep_range(6000, 6010);
	UlCnt = 0 ;
	do {
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal ) ;
		if( UlCnt++ > 100 ) {
			IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ;
			return ( 0x51 ) ;
		}
		usleep_range(1000, 1010) ;
	} while ( UlReadVal != 0 ) ;

	RamRead32A(o_ctrl, 0xF00D, &UlReadVal );

	if( UlReadVal != ptr->SizeFromCodeCksm ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
		return( 0x52 );
	}

	IOWrite32A(o_ctrl, SYSDSP_REMAP, 0x00001000, 0) ;
	usleep_range(15000, 15010);
	IORead32A(o_ctrl, ROMINFO, (UINT_32 *)&UlReadVal ) ;
	if( UlReadVal != 0x0A) return( 0x53 );

	return ( 0 );
}

//********************************************************************************
// Function Name 	: FlashProgram129
// Retun Value		: NON
// Argment Value	: chiperase, ModuleVendor, ActVer
// Explanation		: Update DSP Code to Flash Memory of LC898129.
// History			: First edition 								2019.09.02
//                  : Second edition 								2019.12.09
//********************************************************************************
const DOWNLOAD_TBL_EXT CdTbl[] = {
#if		MODULE_VENDOR == 0x03	//LVI
//  {0x0301, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_00_01, sizeof(CcFromCode129_00_01), FromCheckSum_00_01, FromCheckSumSize_00_01 },
//	{0x0312, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_01_02, sizeof(CcFromCode129_01_02), FromCheckSum_01_02, FromCheckSumSize_01_02 },
//	{0x0314, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_01_04, sizeof(CcFromCode129_01_04), FromCheckSum_01_04, FromCheckSumSize_01_04 },
#elif 	MODULE_VENDOR == 0x07	//SUNNY
	{0x0703, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_00_03, sizeof(CcFromCode129_00_03), FromCheckSum_00_03, FromCheckSumSize_00_03 },
//	{0x0702, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_00_02, sizeof(CcFromCode129_00_02), FromCheckSum_00_02, FromCheckSumSize_00_02 },
#elif 	MODULE_VENDOR == 0x02	//O-FILM
//	{0x0201, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_00_03, sizeof(CcFromCode129_00_03), FromCheckSum_00_03, FromCheckSumSize_00_03 },
//	{0x0301, CcUpdataCode129, UpDataCodeSize,  UpDataCodeCheckSum, CcFromCode129_00_02, sizeof(CcFromCode129_00_02), FromCheckSum_00_02, FromCheckSumSize_00_02 },
#endif
	{0xFFFF,        (void*)0,              0,                   0,      (void*)0,                     0,            0,                0 }
};

UINT_8 FlashProgram129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 chiperase, UINT_8 ModuleVendor, UINT_8 ActVer )
{
	DOWNLOAD_TBL_EXT* ptr ;

	ptr = ( DOWNLOAD_TBL_EXT * )CdTbl ;
	do {
		if( ptr->Index == ( ((UINT_16)ModuleVendor << 8) + ActVer) ) {
			return FlashUpdate129(o_ctrl, chiperase, ptr );
		}
		ptr++ ;
	} while (ptr->Index != 0xFFFF ) ;

	return 0xF0 ;
}

//********************************************************************************
// Function Name 	: InfoMatWrite129
// Retun Value		: NON
// Argment Value	: UINT_8 InfoNo		0: Info Mat 0
//                  :                   1: Info Mat 1
//                  :                   2: Info Mat 2
//                  : UINT_32 *pData    Read Buffer Pointer
// Explanation		: LC898129p Info Mat Write
// History			: First edition 						2019.08.31
//********************************************************************************
UINT_8 InfoMatWrite129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo, UINT_32 *pData )
{
	UINT_8  UcSndDat = FAILURE; 								// 0x01=FAILURE, 0x00=OK
	UINT_32 UlAddrVal;
	int i=0 ;

TRACE("Info Mat Write.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); 						// FLASH Standby Disable

	if( UnlockCodeSet129(o_ctrl) == SUCCESS ) {							// Unlock Code Set
		WritePermission129(o_ctrl); 								// Write Permission
		if( AddtionalUnlockCodeSetInfo129(o_ctrl) == SUCCESS ) {			// Additional Unlock Code Set

			UlAddrVal = 0x00000000;
			UlAddrVal |= (UINT_32)((InfoNo & 0x01) + 0x01) << 16;

			for(i=0 ; i<4 ; i++) {
				UcSndDat = PageWrite129(o_ctrl, UlAddrVal, pData );
				if( UcSndDat == FAILURE ) break;
				UlAddrVal = UlAddrVal + 0x00000010;
				pData = pData + 16;
			}

			if( UcSndDat == SUCCESS ) {
				if( UnlockCodeClear129(o_ctrl) == FAILURE ) {			// Unlock Code Clear
					UcSndDat = -4; // Unlock Code Clear Error
				}
			} else {
				UcSndDat = -3; // Flash write/program Error
			}
		} else {
			UcSndDat = -2; // Additional Unlock Code Error
		}
	} else {
		UcSndDat = -1; // Unlock Code Set Error
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: InfoMatRead129
// Retun Value		: NON
// Argment Value	: UINT_8 InfoNo		0: Info Mat 0
//                  :                   1: Info Mat 1
//                  : UINT_8 Addr       0x00`0x3F
//                  : UINT_8 Size       0x00`0x3F
//                  : UINT_32 *pData    Read Buffer Pointer
// Explanation		: LC898129p Info Mat Read
// History			: First edition 						2019.08.31
//********************************************************************************
UINT_8 InfoMatRead129(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo, UINT_8 Addr, UINT_8 Size, UINT_32 *pData )
{
	UINT_8  UcSndDat = SUCCESS; 			// 0x01=FAILURE, 0x00=SUCCESS
	UINT_32 UlAddrVal;
	UINT_32 UlDataVal;
	int i=0 ;

TRACE("Info Mat Read.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); 	// FLASH Standby Disable

	UlAddrVal = 0x00000000;
	UlAddrVal |= (UINT_32)((InfoNo & 0x01) + 0x01) << 16;
	UlAddrVal |= (UINT_32)(Addr & 0x3F);

	IOWrite32A(o_ctrl, 0xE07008, (UINT_32)Size, 0); // set ACSCNT
	IOWrite32A(o_ctrl, 0xE0700C, UlAddrVal, 0); 	// set FLA_ADR(Info Mat0 address)
	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); 	// CMD(read operation command)

	for(i=0 ; i<=(int)Size ; i++) {
		IORead32A(o_ctrl, 0xE07000, &UlDataVal ); // FLA_RDAT
		*pData++ = UlDataVal;
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: RdInfMAT
// Retun Value		: NON
// Argment Value	: UINT_8 InfoNo
//                  : UINT_8 Addr       0x00`0x3F
//                  : UINT_8 Size       0x00`0x3F
//                  : UINT_32 *pData    Read Buffer Pointer
// Explanation		: LC898129p Info Mat Read
// History			: First edition 						2019.08.31
//********************************************************************************
UINT_8 RdInfMAT(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo, UINT_8 Addr, UINT_8 Size, UINT_32 *pData )
{
	UINT_8  UcSndDat = SUCCESS; 			// 0x01=FAILURE, 0x00=SUCCESS
	UINT_32 UlAddrVal;
	UINT_32 UlDataVal;
	int i=0 ;

TRACE("Info Mat Read.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); 	// FLASH Standby Disable

	UlAddrVal = 0x00000000;
	UlAddrVal |= (UINT_32)InfoNo << 16;
	UlAddrVal |= (UINT_32)(Addr & 0x3F);

	IOWrite32A(o_ctrl, 0xE07008, (UINT_32)Size, 0); // set ACSCNT
	IOWrite32A(o_ctrl, 0xE0700C, UlAddrVal, 0); 	// set FLA_ADR(Info Mat0 address)
	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); 	// CMD(read operation command)

	for(i=0 ; i<=(int)Size ; i++) {
		IORead32A(o_ctrl, 0xE07000, &UlDataVal ); // FLA_RDAT
		*pData++ = UlDataVal;
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: ErInfMAT
// Retun Value		: NON
// Argment Value	: UINT_8 InfoNo
// Explanation		: LC898129p Info Mat Erase
// History			: First edition 						2019.08.31
//********************************************************************************
UINT_8 ErInfMAT(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo )
{
	UINT_8  UcSndDat = FAILURE; 							// 0x01=FAILURE, 0x00=OK
	UINT_32 UlCnt;
	UINT_32 UlDataVal;
	UINT_32 UlAddrVal;

TRACE("Info Mat Erase.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); 					// FLASH Standby Disable

	if( UnlockCodeSet129(o_ctrl) == SUCCESS ) {						// Unlock Code Set
		WritePermission129(o_ctrl); 							// Write Permission
		if( AddtionalUnlockCodeSetInfo129(o_ctrl) == SUCCESS ) {		// Additional Unlock Code Set
			//***** Block Erase Info0 Mat *****
			UlAddrVal = 0x00000000;
			UlAddrVal |= (UINT_32)InfoNo << 16;
			IOWrite32A(o_ctrl, 0xE0700C, UlAddrVal, 0); 			// FLA_REGS(E0_700Ch[21]=0
																// FLA_TRMM(E0_700Ch[20]=0
																// FLA_INFM(E0_700Ch[18:16]=INF
																// FLA_ADR(E0_700Ch[13:0])=ADR
			IOWrite32A(o_ctrl, 0xE07010, 0x00000004, 0); 			// CMD(E0_7010h[3:0])=4h

			// Check BSY_FLA(E0_7018h[7]) flag
			UlCnt = 0;
			do{
				IORead32A(o_ctrl, 0xE07018, &UlDataVal ); 		// BSY_FLA(E0_7018h[7])
				if( (UlDataVal & 0x00000080) == 0 ) {
					UcSndDat = SUCCESS;
					break;
				}
				usleep_range(1000, 1010);
			}while ( UlCnt++ < 10 );

			if( UcSndDat == SUCCESS ) {
				if( UnlockCodeClear129(o_ctrl) == FAILURE ) {			// Unlock Code Clear
					UcSndDat = -4; // Unlock Code Clear Error
				}
			} else {
				UcSndDat = -3; // Info Mat Erase Error
			}
		} else {
			UcSndDat = -2; // Additional Unlock Code Error
		}
	} else {
		UcSndDat = -1; // Unlock Code Set Error
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: WrInfMAT
// Retun Value		: NON
// Argment Value	: UINT_8 InfoNo		0: Info Mat 0
//                  :                   1: Info Mat 1
//                  :                   2: Info Mat 2
//                  : UINT_32 *pData    Read Buffer Pointer
// Explanation		: LC898129p Info Mat Write
// History			: First edition 2019.08.31
//********************************************************************************
UINT_8 WrInfMAT(struct cam_ois_ctrl_t *o_ctrl, UINT_8 InfoNo, UINT_32 *pData )
{
	UINT_8  UcSndDat = FAILURE; // 0x01=FAILURE, 0x00=OK
	UINT_32 UlAddrVal;
	int i=0 ;

TRACE("Info Mat Write.\n");

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0); // FLASH Standby Disable

	if( UnlockCodeSet129(o_ctrl) == SUCCESS ) { // Unlock Code Set
		WritePermission129(o_ctrl); // Write Permission
		if( AddtionalUnlockCodeSetInfo129(o_ctrl) == SUCCESS ) { // Additional Unlock Code Set

			UlAddrVal = 0x00000000;
			UlAddrVal |= (UINT_32)InfoNo << 16;

			for(i=0 ; i<4 ; i++) {
				UcSndDat = PageWrite129(o_ctrl, UlAddrVal, pData );
				if( UcSndDat == FAILURE ) break;
				UlAddrVal = UlAddrVal + 0x00000010;
				pData = pData + 16;
			}

			if( UcSndDat == SUCCESS ) {
				if( UnlockCodeClear129(o_ctrl) == FAILURE ) { // Unlock Code Clear
					UcSndDat = -4; // Unlock Code Clear Error
				}
			} else {
				UcSndDat = -3; // Flash write/program Error
			}
		} else {
			UcSndDat = -2; // Additional Unlock Code Error
		}
	} else {
		UcSndDat = -1; // Unlock Code Set Error
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0); 	// FLASH Standby Enable

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: WrI2C_SlaveAddress
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Flash write I2C Slave Address
// History			: First edition 2020.07.01
//********************************************************************************
#if 0
UINT_8 WrI2C_SlaveAddress(struct cam_ois_ctrl_t *o_ctrl, UINT_8 SlaveAddr )
{
	UINT_32	UlMAT2[64];
	UINT_8 ans = 0;
	UINT_32	UlReadVal;

TRACE( "WrI2C_SlaveAddress : Mode = 0x%02x\n", SlaveAddr);

	IOWrite32A(o_ctrl, 0xE0701C , 0x00000000, 0) ; 	// Standby Disable

	/* Back up ******************************************************/
	ans = RdInfMAT(o_ctrl, INF_MAT2, 0, 0x3F, (UINT_32 *)&UlMAT2 );
	if( ans ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; // Standby Enable
		return( ans );
	}

	/* Erase   ******************************************************/
	ans = ErInfMAT(o_ctrl, INF_MAT2 ); 				// Erase Info Mat0
	if( ans != SUCCESS) {
		return( ans );
	}

	/* modify   *****************************************************/
	SlaveAddr = SlaveAddr & 0xFE; // Slave Address

	if( SlaveAddr != 0x00 ) {		// write
		UlMAT2[ 63 ] = (UlMAT2[ 63 ] & 0xFFFFFF00) | (UINT_32)SlaveAddr;
	} else {						// clear
		SlaveAddr = 0xFF;
		UlMAT2[ 63 ] = (UlMAT2[ 63 ] & 0xFFFFFF00) | (UINT_32)SlaveAddr;
	}
TRACE( "0x%08x : 0x%02x\n", UlMAT2[63], SlaveAddr);

	/* update ******************************************************/
	ans = WrInfMAT(o_ctrl, INF_MAT2, (UINT_32 *)&UlMAT2 );
	if( ans != 0 ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; 	// Standby Enable
		return( 3 ) ; 					// 0 - 63
	}

	/* Verify ******************************************************/
	ans = RdInfMAT(o_ctrl, INF_MAT2, 0, 0x3F, (UINT_32 *)&UlMAT2 );
	if( ans ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; 	// Standby Enable
		return( 4 );
	}

TRACE( "0x%08x : 0x%02x\n", UlMAT2[63], SlaveAddr);
	if ( (UINT_8)(UlMAT2[63] & 0x000000FF) != SlaveAddr ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; 	// Standby Enable
		return( 5 );
	}

TRACE( "WrI2C_SlaveAddresss___COMPLETE\n" );
	IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; 		// Standby Enable

	// CoreReset
	IOWrite32A(o_ctrl, SYSDSP_REMAP, 0x00001000, 0) ;
	usleep_range(15000, 15010) ;

	if( SlaveAddr == 0xFF ) SlaveAddr = 0x48;
	setI2cSlvAddr( SlaveAddr, 2 ); 				// AtmelSlave Address

	IORead32A(o_ctrl, ROMINFO,	(UINT_32 *)&UlReadVal ) ;
	if( UlReadVal != 0x0A) return( 6 );

	return(0);
}
#endif

static uint8_t	RdStatus(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcStBitChk )
{
	uint32_t	UlReadVal;

	RamRead32A(o_ctrl, 0xF100 , &UlReadVal );
	if( UcStBitChk ){
		UlReadVal &= READ_STATUS_INI ;
	}
	if( !UlReadVal ){
		return( SUCCESS );
	}else{
		return( FAILURE );
	}
}
#define		CNT050MS		 676
void SetActiveMode(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF019, 0x00000000, 0);
	while (UcStRd && (UlStCnt++ < CNT050MS))
		UcStRd = RdStatus(o_ctrl, 1);
}

//********************************************************************************
// Function Name    : CalibrationDataSave0
// Retun Value      : NON
// Argment Value    : NON
// Explanation      : Calibration Data Save for LC898129 SMA
// History          : First edition  2019.11.18
//********************************************************************************
UINT_8 CalibrationDataSave0(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_8 UcSndDat = SUCCESS; // 0x01=FAILURE, 0x00=SUCCESS
	UINT_32 UlBuffer[64];
	UINT_32 UlBufferB[64];
	UINT_32 UlXGG;
	UINT_32 UlYGG;
	UINT_8 i;

	UcSndDat = InfoMatRead129(o_ctrl, 0, 0, 0x3F, (UINT_32 *)&UlBuffer);
	if( UcSndDat != SUCCESS) {
		return UcSndDat;
	}

	// Erase Info Mat0
	UcSndDat = InfoMatErase129(o_ctrl, 0 ); // Erase Info Mat0
	if( UcSndDat != SUCCESS) {
		return UcSndDat;
	}

	RamRead32A(o_ctrl, 0x84B8 , &UlXGG) ;
	RamRead32A(o_ctrl, 0x84EC , &UlYGG) ;

	// Setting Calibration Data
	UlBuffer[0] = UlXGG;
	UlBuffer[1] = UlYGG;

	// Write to Info Mat0
	UcSndDat = InfoMatWrite129(o_ctrl, 0, UlBuffer); // Write Info Mat0
	if( UcSndDat != SUCCESS) {
		return UcSndDat;
	}

	// Read vreify from Info Mat0
	UcSndDat = InfoMatRead129(o_ctrl, 0, 0, 0x3F, (UINT_32 *)&UlBufferB );

	for(i=0 ; i<64 ; i++) {
		if(  UlBuffer[i] != UlBufferB[i] ) {
			UcSndDat = FAILURE;
		}
	}

	return UcSndDat; // Set Data To Send Buffer (If normal, 0x00 is returned.)
}

#if 0
//********************************************************************************
// Function Name    : GyroOffsetSave
// Retun Value      : NON
// Argment Value    : NON
// Explanation      : Save Gyro Offset Data & Accl Offset Data
// History          : First edition 2021.05.07
//********************************************************************************
UINT_8 GyroOffsetSave(struct cam_ois_ctrl_t *o_ctrl, UINT_8 UcMode)
{
	UINT_8 UcSndDat = SUCCESS; // 0x01=FAILURE, 0x00=SUCCESS
	UINT_32 UlBuffer[64];
	UINT_32 UlBufferB[64];
	UINT_8 ans = 0;
	UINT_32 UlReadGxOffset , UlReadGyOffset, UlReadGzOffset;
	UINT_32 UlReadAxOffset , UlReadAyOffset, UlReadAzOffset;
	UINT_8 i;

	/* Back up ******************************************************/
	ans = RdInfMAT(o_ctrl, INF_MAT0, 0, 0x3F, (UINT_32 *)&UlBuffer );
	if( ans ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; // Standby Enable
		return( ans );
	}

	/* Erase   ******************************************************/
	ans = ErInfMAT(o_ctrl, INF_MAT0 ); // Erase Info Mat0
	if( ans != SUCCESS) {
		return( ans );
	}

	/* update ******************************************************/
	if( UcMode ){
		RamRead32A(o_ctrl, GYRO_RAM_GXOFFZ , &UlReadGxOffset ) ;
		RamRead32A(o_ctrl, GYRO_RAM_GYOFFZ , &UlReadGyOffset ) ;
		RamRead32A(o_ctrl, GYRO_ZRAM_GZOFFZ , &UlReadGzOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_X_AC_OFFSET , &UlReadAxOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_Y_AC_OFFSET , &UlReadAyOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_Z_AC_OFFSET , &UlReadAzOffset ) ;

		UlBuffer[G_OFFSET_XY] = (UlReadGxOffset&0xFFFF0000) | ((UlReadGyOffset&0xFFFF0000)>>16);
		UlBuffer[G_OFFSET_Z_AX] = (UlReadGzOffset&0xFFFF0000) | ((UlReadAxOffset&0xFFFF0000)>>16);
		UlBuffer[A_OFFSET_YZ] = (UlReadAyOffset&0xFFFF0000) | ((UlReadAzOffset&0xFFFF0000)>>16);
	}else{
	    UlBuffer[G_OFFSET_XY] = 0xFFFFFFFF;
	    UlBuffer[G_OFFSET_Z_AX] = 0xFFFFFFFF;
	    UlBuffer[A_OFFSET_YZ] = 0xFFFFFFFF;
	}

	// Write to Info Mat0
	ans = WrInfMAT(o_ctrl, INF_MAT0, (UINT_32 *)&UlBuffer );
	if( ans != 0 ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; // Standby Enable
		return( 3 ) ; // 0 - 63
	}

	/* Verify ******************************************************/
	ans = RdInfMAT(o_ctrl, INF_MAT0, 0, 0x3F, (UINT_32 *)&UlBufferB );
	if( ans ) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0) ; // Standby Enable
		return( 4 );
	}

	for(i=0 ; i<64 ; i++) {
		if(  UlBuffer[i] != UlBufferB[i] ) {
			UcSndDat = FAILURE;
		}
	}

	return UcSndDat; // Set Data To Send Buffer (If normal, 0x00 is returned.)
}
#endif
//**** End of Line ****************************************************************************
