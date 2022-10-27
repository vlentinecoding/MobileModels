/**
 * @brief OSC calibration code for LC898129
 * Driver offset calibration code
 *
 * @author (C) 2021 ON Semiconductor.
 *
 * @file OSCAdj.c
 **/
//**************************
// Include Header File
//**************************
//#include "Ois.h"
//#include "OisLc898129.h"
//#include <stdlib.h>
#include "PhoneUpdate.h"
#include "onsemi_ois_interface.h"

//****************************************************
//	LOCAL FUNCTIONS
//****************************************************
UINT_8	FlashInformationMatErase(struct cam_ois_ctrl_t *o_ctrl, UINT_8 ucMat );
UINT_8	FlashInformationMatProgram(struct cam_ois_ctrl_t *o_ctrl, UINT_32 *ulWriteVal, UINT_32 ulFLA_ADR );
UINT_8	ProtectRelease(struct cam_ois_ctrl_t *o_ctrl, UINT_8 ucMat );
UINT_8	Protect(struct cam_ois_ctrl_t *o_ctrl);
UINT_8	VerifyRead(struct cam_ois_ctrl_t *o_ctrl, UINT_32 *ulWriteVal, UINT_32 ulFLA_ADR );
UINT_32	TrimAdj( UINT_32 );
UINT_8	CheckCHECKCODE(struct cam_ois_ctrl_t *o_ctrl, UINT_16 usCVer );

//****************************************************
//	LOCAL RAM  LIST
//****************************************************
volatile unsigned char UcOscAdjFlg = 0 ; // For Measure trigger

//****************************************************
//	LOCAL Define LIST
//****************************************************
#define MEASSTR  0x01
#define MEASCNT  0x08
#define MEASDIV  0x0A
#define MEASFIX  0x80

#define I2C_TIME_MODE  1 // 0 : disable, 1 : enable

//********************************************************************************
// Function Name 	: TimPro
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Interrupt Process Function for OSC adj at every 4msec
//********************************************************************************
void	TimPro(struct cam_ois_ctrl_t *o_ctrl)
{
	if ( UcOscAdjFlg ) {
		if ( UcOscAdjFlg == MEASSTR ) {
		// TRACE(" TimPro1 : \n") ;
			IOWrite32A(o_ctrl, OSCCNT , 0x00000001, 0); /* measure start */
			UcOscAdjFlg = MEASDIV ;
		} else if ( UcOscAdjFlg == MEASDIV ){
//TRACE(" TimPro2 : \n") ;
			IOWrite32A(o_ctrl, OSCCNT , 0x00000000, 0); /* measure end */
			UcOscAdjFlg = MEASFIX ;
//TRACE(" TimPro3 : \n") ;
		}
	}
}

//********************************************************************************
// Function Name 	: OscAdj
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: OSC Clock adjustment
//********************************************************************************
#if I2C_TIME_MODE == 1
#define I2C_CLOCK 370 // I2C 370kbps
//#define I2C_CLOCK 400 // I2C 400kbps
#define OSC_TGT_COUNT (80000*6/I2C_CLOCK) // I2C 400kbps : 16.25usec * 80MHz = 1300count.
#define DIFF_THRESHOLD (OSC_TGT_COUNT*625/100000) // 7.5 --> 7
#else // I2C_TIME_MODE
//#define OSC_TGT_COUNT 20000
#define OSC_ADJMES_TIME 4000 // 4[msec]
#endif // I2C_TIME_MODE

UINT_8 OscAdj(struct cam_ois_ctrl_t *o_ctrl, UINT_16 *OscValue )
{
	INT_8 i;
	UINT_32 ulTrim = 0, ulTrim_bak = 0, ulTrim_bak2 = 0;
	UINT_32 ulCkcnt, ulCkcnt2;
	UINT_32 ulDspdiv_bak;
	UINT_32 ulFrq_diff, ulFrq_diff2;
	UINT_8 ucResult = 0;

	UINT_32 ulrominfo;
#if I2C_TIME_MODE == 1
#else // I2C_TIME_MODE
	UINT_32 OSC_TGT_COUNT ;
	UINT_32 DIFF_THRESHOLD ;
#endif // I2C_TIME_MODE

	//Boot Sequence
	IORead32A(o_ctrl, ROMINFO, &ulrominfo ); // Vendor Reserved Status.
	if ((ulrominfo != 0x0000000C) && (ulrominfo != 0x0000000B)){
		ucResult = 0x01; // Error
		return( ucResult );
	}
	IORead32A(o_ctrl, SYSDSP_DSPDIV, &ulDspdiv_bak ); // DSP Clock Divide Setting.

#if I2C_TIME_MODE == 1
	IOWrite32A(o_ctrl, SYSDSP_DSPDIV, 0x00000000, 0); // 80MHz(129)
#else	// I2C_TIME_MODE
	//IOWrite32A(o_ctrl, SYSDSP_DSPDIV, 0x00000004, 0); // 80MHz(129)
	IOWrite32A(o_ctrl, SYSDSP_DSPDIV, 0x00000007, 0); // 80MHz(129)
#endif	// I2C_TIME_MODE

	IOWrite32A(o_ctrl, SYSDSP_SOFTRES, 0x0003037F, 0); // Soft Reset Setting : OSC counter = Normal
	IOWrite32A(o_ctrl, PERICLKON, 0x0000005C, 0); // Peripheral clock On/Off setting : OSC counter = on.

TRACE("Adj1 \n");

	////////////////////////////////////////
	// OIS Measure Trim_SAR (NoTest)
	////////////////////////////////////////

#if I2C_TIME_MODE == 1
		IOWrite32A(o_ctrl, OSCCNT , 0x00000021, 0);// I2C TIME MODE Enable
#endif	// I2C_TIME_MODE

	for( i = 8; i >= 0; i-- ) {
		ulTrim |= (unsigned long)( 1 << i ); // trim[i] = 1b;
TRACE("	i = %d    FRQTRM = %08xh ", i , (int)ulTrim ) ;
		IOWrite32A(o_ctrl, FRQTRM, ~ulTrim & 0x000001FF, 0); // ~trim

#if I2C_TIME_MODE == 1
		IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt ); // OSC clock count.
#else	// I2C_TIME_MODE
		UcOscAdjFlg = MEASSTR; // Start trigger ON
		while ( UcOscAdjFlg != MEASFIX ) {
			; //TRACE(" while IN : \n") ;
		}
//TRACE(" while OUT : \n") ;

		IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt ); // OSC clock count.

		//Calculate target count.
		OSC_TGT_COUNT = GetOscTgtCount(OSC_ADJMES_TIME) ;
#endif // I2C_TIME_MODE

		if( ulCkcnt >= OSC_TGT_COUNT ){ // 80MHz ( 80[MHz] * 1/4 * 1[msec] = 20000[count]. )
			ulTrim -= (unsigned long)(1 << i); // trim[i] = 0b;
		}
TRACE("cnt = %d    \n", (int)ulCkcnt );
	}

	// OSC Adj Trim_I (NoTest)
TRACE("Adj2 \n") ;
	ulTrim_bak = ulTrim; // trim_back = trim;
	ulTrim_bak2 = ( ( ( ulTrim_bak & 0x00000070 ) >> 1) | ( ulTrim_bak & 0x00000007 ) ); // trim_back2 = trim_bak[6:4] | trim_bak[2:0] ;

TRACE("Adj2-1 : \n") ;
TRACE("ulTrim      = %08xh \n", (int)ulTrim);
TRACE("ulTrim_bak  = %08xh \n", (int)ulTrim_bak);
TRACE("ulTrim_bak2 = %08xh \n", (int)ulTrim_bak2);

	if( ulTrim_bak2 & 0x00000001 ){ // if(trim_bak2[0]){
		if( ( ulTrim_bak2 & 0x0000003F ) != 0x0000003F){ //  if(trim_bak2[5:0] != 3Fh){
			ulTrim_bak2 ++; //   trim_bak2 ++;
			ulTrim = ( ulTrim_bak & 0x00000180 ) | ( (ulTrim_bak2 & 0x00000038) << 1 ) // trim = { trim_bak[8:7], trim_bak2[5:3],
				| ( ulTrim_bak & 0x00000008 ) | ( ulTrim_bak2 & 0x00000007 ); // trim_bak[3],   trim_bak2[2:0] }
TRACE(" 1 : \n") ;
		}else{
			ulTrim = ulTrim_bak;
TRACE(" 2 : \n") ;
		}
	}else{
		ulTrim = ulTrim_bak;
		ulTrim_bak++;
TRACE(" 3 : \n") ;
	}

TRACE(" Adj2-2 : \n") ;
TRACE(" ulTrim      = %08xh \n", (int)ulTrim      );
TRACE(" ulTrim_bak  = %08xh \n", (int)ulTrim_bak  );
TRACE(" ulTrim_bak2 = %08xh \n", (int)ulTrim_bak2 );

TRACE("i = %d    FRQTRM = %08xh \n", i , (int)ulTrim );

	IOWrite32A(o_ctrl, FRQTRM, ~ulTrim & 0x000001FF, 0); // ~trim

#if I2C_TIME_MODE == 1
		IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt2 ); // OSC clock count.
#else	// I2C_TIME_MODE
	UcOscAdjFlg = MEASSTR ; // Start trigger ON
	while( UcOscAdjFlg != MEASFIX )
	{
		;
	}

	IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt2 ); // OSC clock count.

	// Calculate target count.
	OSC_TGT_COUNT = GetOscTgtCount(OSC_ADJMES_TIME);
#endif // I2C_TIME_MODE

TRACE("Adj2-3 : \n") ;
TRACE("cnt = %d    \n", (int)ulCkcnt2 ) ;
	ulFrq_diff = abs(ulCkcnt - OSC_TGT_COUNT);
	ulFrq_diff2 = abs(ulCkcnt2 - OSC_TGT_COUNT);
	if (ulFrq_diff < ulFrq_diff2){
		ulTrim = ulTrim_bak;
TRACE("4 : \n");
	}

TRACE("ulTrim = %08xh \n", (int)ulTrim );

	IOWrite32A(o_ctrl, FRQTRM, ~ulTrim & 0x000001FF, 0); // ~trim

	////////////////////////////////////////
	// OSC Adj Verigy (NoTest)
	////////////////////////////////////////

#if I2C_TIME_MODE == 1
		IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt ); // OSC clock count.
#else	// I2C_TIME_MODE
	UcOscAdjFlg = MEASSTR; // Start trigger ON
	while( UcOscAdjFlg != MEASFIX ) // wait 1msec
	{
		;
	}

	// Calculate target count.
	OSC_TGT_COUNT = GetOscTgtCount(OSC_ADJMES_TIME) ;

	IORead32A(o_ctrl, OSCCKCNT, &ulCkcnt ); // OSC clock count.
#endif // I2C_TIME_MODE

#if I2C_TIME_MODE == 1
		IOWrite32A(o_ctrl, OSCCNT , 0x00000000, 0); // I2C TIME MODE Disable
#endif	// I2C_TIME_MODE

	ulFrq_diff = abs( ulCkcnt - OSC_TGT_COUNT );

#if I2C_TIME_MODE == 1
#else	// I2C_TIME_MODE
	DIFF_THRESHOLD = OSC_TGT_COUNT * (UINT_32)5 / (UINT_32)800 ; // 80MHz 0.5MHz
TRACE( " DIFF_THRESHOLD = %08x \n", DIFF_THRESHOLD );
#endif	// I2C_TIME_MODE

	if( ulFrq_diff <= DIFF_THRESHOLD ){
		ucResult = 0x00; // OK
	}else{
		ucResult = 0x02; // NG
	}

	IOWrite32A(o_ctrl, SYSDSP_DSPDIV, ulDspdiv_bak, 0); // DSP Clock Divide Setting.

	*OscValue = (unsigned short)ulTrim;
	return( ucResult );
}

//********************************************************************************
// Function Name 	: WriteOscAdjVal
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Write Osc adjustment value
//********************************************************************************
#define GainXP    0x0B //TRM.12h[15:12] INF2.22h[15:12]
#define GainXM    0x0B //TRM.12h[31:28] INF2.22h[31:28]
#define GainYP    0x0B //TRM.13h[15:12] INF2.23h[15:12]
#define GainYM    0x0B //TRM.13h[31:28] INF2.23h[31:28]
#define GainAFP   0x09 //TRM.14h[15:12] INF2.24h[15:12]
#define GainAFM   0x09 //TRM.14h[31:28] INF2.24h[31:28
#define GainVal   0x0B

#define OfstXP    0x40 //
#define OfstXM    0x40 //
#define OfstYP    0x40 //
#define OfstYM    0x40 //
#define OfstAFP   0x0100 //
#define OfstAFM   0x0100 //
#define OfstVal   0x40 //

UINT_8 WriteOscAdjVal(struct cam_ois_ctrl_t *o_ctrl, UINT_16 usOscVal, UINT_16 usCVer )
{
	UINT_16 usResult;
	UINT_32 ulWriteVal0[16];
	UINT_32 ulWriteVal1[16];
	UINT_32 ulCheckSumVal;
	UINT_16 i;
	UINT_32 ulTrimRegVal;
	UINT_32 ulAdjTrim;

	IOWrite32A(o_ctrl, SYSDSP_DSPDIV, 0x00000001, 0); // 80MHz(129)

	 if( (usCVer == 0x0144) || (usCVer == 0x0145) ) { // LC898129 ES2
		IOWrite32A(o_ctrl, 0xE07008, 0x00000000, 0); // 1 word read
		IOWrite32A(o_ctrl, 0xE074CC, 0x00000080, 0); // RSTB_FLA Bit7 Don not change
		IOWrite32A(o_ctrl, 0xE0700C, 0x00200000, 0); // FLA_ADR User Trimming Mat
		msleep( 1 ); // 20usec
		IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD Read
		IORead32A(o_ctrl, 0xE07000, &ulTrimRegVal ); // FLA_RDAT

		usResult = ProtectRelease(o_ctrl, TRIM_MAT ); // TRIM_MAT
		if( usResult != 0x00 ){
			return ( 0x01 ); // Error
		}

		// Adjustment trimming data
		ulAdjTrim = TrimAdj( ulTrimRegVal );
		IOWrite32A(o_ctrl, 0xE074CC, 0x00000081, 0);
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
		IOWrite32A(o_ctrl, 0xE0700C, 0x00200000, 0);
		IOWrite32A(o_ctrl, 0xE07010, 0x00000002, 0);
		IOWrite32A(o_ctrl, 0xE07004, ulAdjTrim, 0);
		IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0);
		IORead32A(o_ctrl, 0xE07000, &ulTrimRegVal );
		TRACE("ulTrimRegVal=%08X\n", ulTrimRegVal);

		// Info Mat0 Erase
		usResult = FlashInformationMatErase(o_ctrl, INF_MAT0 );
		if( usResult != 0x00 ){
			return ( 0x02 ); // Error
		}

		// Info Mat1 Erase
		usResult = FlashInformationMatErase(o_ctrl, INF_MAT1 );
		if( usResult != 0x00 ){
			return ( 0x02 ); // Error
		}

		// Info Mat2 Erase
		usResult = FlashInformationMatErase(o_ctrl, INF_MAT2 );
		if( usResult != 0x00 ){
			return ( 0x02 ); // Error
		}

		// Flash Trimming Mat Erase
		usResult = FlashInformationMatErase(o_ctrl, TRIM_MAT );
		if( usResult != 0x00 ){
			return ( 0x02 ); // Error
		}

		// Data
		ulWriteVal0[0] = ulTrimRegVal;
		ulWriteVal0[1] = ~ulTrimRegVal;
		for( i = 2; i <= 15; i++ ){ // 14 times
			ulWriteVal0[i] = 0xFFFFFFFF;
		}

		// Flash Trimming Mat Page 0 Program
		usResult = FlashInformationMatProgram(o_ctrl, ulWriteVal0, 0x00100000 );
		if( usResult != 0x00 ){
			return ( 0x03 ); // Error
		}

		// Data
		ulWriteVal1[0] = 0x55FFFFFF;
		ulWriteVal1[1] = 0x0000AF00 | ( (UINT_32)(~usOscVal & 0x01FF) << 16 ); // { 0000 000b, OscAdjVal[8], OscAdjVal[7:0], 8F00h } ~ulTrim & 0x000001FF
		ulWriteVal1[2] = 0x0F000F00 | ( (UINT_32)GainXM  << 28 ) | ( (UINT_32)OfstXM  << 16 ) | ( (UINT_32)GainXP  << 12 ) | ( (UINT_32)0x3F ); // { GainVal[3:0], FFFh, GainVal[3:0], FFFh }
		ulWriteVal1[3] = 0x0F000F00 | ( (UINT_32)GainYM  << 28 ) | ( (UINT_32)OfstYM  << 16 ) | ( (UINT_32)GainYP  << 12 ) | ( (UINT_32)0x3F ); // { GainVal[3:0], FFFh, GainVal[3:0], FFFh }
		ulWriteVal1[4] = 0x00000000 | ( (UINT_32)GainAFM << 28 ) | ( (UINT_32)OfstAFM << 16 ) | ( (UINT_32)GainAFP << 12 ) | ( (UINT_32)OfstAFP ); // { GainVal[3:0], FFFh, GainVal[3:0], FFFh }
		ulWriteVal1[5] = 0x0F000F00 | ( (UINT_32)GainVal << 28 ) | ( (UINT_32)OfstVal << 16 ) | ( (UINT_32)GainVal << 12 ) | ( (UINT_32)OfstVal ); // { GainVal[3:0], FFFh, GainVal[3:0], FFFh }
		ulWriteVal1[6] = 0x0F000F00 | ( (UINT_32)GainVal << 28 ) | ( (UINT_32)OfstVal << 16 ) | ( (UINT_32)GainVal << 12 ) | ( (UINT_32)OfstVal ); // { GainVal[3:0], FFFh, GainVal[3:0], FFFh }
		ulWriteVal1[11] = ulWriteVal1[10] = ulWriteVal1[9] = ulWriteVal1[8] = ulWriteVal1[7] = 0xFFFFFFFF;
		ulCheckSumVal = 0;
		for( i = 0; i < 12; i++ ){ // 12 times
			ulCheckSumVal += ulWriteVal1[i];
		}
		ulWriteVal1[12] = ulCheckSumVal;
		ulWriteVal1[13] = 0xFFFF0000 | MAKER_CODE; // { FFFFh, CCCCh }
		ulWriteVal1[14] = 0x99756768;
		ulWriteVal1[15] = 0x01AC29AC;

		// Flash Trimming Mat Page 1 Program
		usResult = FlashInformationMatProgram(o_ctrl, ulWriteVal1, 0x00100010 );
		if( usResult != 0x00 ){
			return ( 0x04 ); // Error
		}

		// Protect
		usResult = Protect(o_ctrl);
		if( usResult != 0x00 ){
			return ( 0x05 ); // Error
		}

		// Verify Read (Flash Trimming Mat Page 0)
		usResult = VerifyRead(o_ctrl, ulWriteVal0, 0x00100000 );
		if( usResult != 0x00 ){
			return ( 0x06 ); // Error
		}

		// Verify Read (Flash Trimming Mat Page 1)
		usResult = VerifyRead(o_ctrl, ulWriteVal1, 0x00100010 );
		if( usResult != 0x00 ){
			return ( 0x07 ); // Error
		}

	}

	return ( 0x00 );
}

//********************************************************************************
// Function Name 	: ProtectRelease
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Protect Release
//********************************************************************************
//129
//Trimming Mat			5B29h
//Information Mat0,1,2	C5ADh
//128
//Trimming Mat			5B29h
//Information Mat0,1	C5ADh
//Information Mat2		6A4Bh
UINT_8 ProtectRelease(struct cam_ois_ctrl_t *o_ctrl, UINT_8 ucMat )
{
	UINT_32 ulReadVal;

	// Protect Release
	IOWrite32A(o_ctrl, 0xE07554, 0xAAAAAAAA, 0); // UNLK_CODE1
	IOWrite32A(o_ctrl, 0xE07AA8, 0x55555555, 0); // UNLK_CODE2
	IOWrite32A(o_ctrl, 0xE074CC, 0x00000001, 0); // RSTB_FLA
	IOWrite32A(o_ctrl, 0xE07664, 0x00000010, 0); // CLK_FLAON
	if( (ucMat == INF_MAT0) || (ucMat == INF_MAT1) || (ucMat == INF_MAT2) ) { // Information Mat0,1,2
		IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0); // UNLK_CODE3
	}
	else if( ucMat == TRIM_MAT ) { // Trimming Mat
		IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0); // UNLK_CODE3
		IOWrite32A(o_ctrl, 0xE07CCC, 0x00005B29, 0); // UNLK_CODE3
	}
	else{
		return ( 0x02 ); // Error
	}
	IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 0); // UNLK_CODE3
	IORead32A(o_ctrl, 0xE07014, &ulReadVal ); // FLAWP
	if( ( (ulReadVal == 0x1185) && ((ucMat == INF_MAT0) || (ucMat == INF_MAT1) || (ucMat == INF_MAT2)))
	 || ( (ulReadVal == 0x1187) && ((ucMat == TRIM_MAT)                                             ))){
		return ( 0x00 ); // Success
	}
	else{
		return ( 0x01 ); // Error
	}
}

//********************************************************************************
// Function Name 	: FlashInformationMatErase
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Flash Information Mat Erase
//********************************************************************************
UINT_8	FlashInformationMatErase(struct cam_ois_ctrl_t *o_ctrl, UINT_8 ucMat )
{
	UINT_32 ulReadVal;
	UINT_32 ulCnt = 0;

	// Flash Information Mat erase
	switch( ucMat ){
	case INF_MAT0:
		IOWrite32A(o_ctrl, 0xE0700C, 0x00010000, 0); // FLA_ADR Information Mat 0
		break;
	case INF_MAT1:
		IOWrite32A(o_ctrl, 0xE0700C, 0x00020000, 0); // FLA_ADR Information Mat 1
		break;
	case INF_MAT2:
		IOWrite32A(o_ctrl, 0xE0700C, 0x00040000, 0); // FLA_ADR Information Mat 2
		break;
	case TRIM_MAT:
		IOWrite32A(o_ctrl, 0xE0700C, 0x00100000, 0); // FLA_ADR Trimming Mat
		break;
	default :
		return ( 0x01 ); // Error
		break;
	}

	msleep( 1 );
	IOWrite32A(o_ctrl, 0xE07010, 0x00000004, 0); // CMD
	msleep( 8 );
	do{
		IORead32A(o_ctrl, 0xE07018, &ulReadVal ); // FLAINT
		if( (ulReadVal & 0x80) == 0x00 ){
			return ( 0x00 ); // Success
		}
		msleep( 1 );
	}while( ulCnt++ < 10 );

	return ( 0x01 ); // Error
}

//********************************************************************************
// Function Name 	: FlashInformationMatProgram
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Flash Information Mat Program
//********************************************************************************
UINT_8 FlashInformationMatProgram(struct cam_ois_ctrl_t *o_ctrl, UINT_32 *ulWriteVal, UINT_32 ulFLA_ADR )
{
	UINT_32 ulReadVal;
	UINT_32 ulCnt = 0;
	INT_16 i;

	// Flash Information Mat 2 Page 2 Program
	IOWrite32A(o_ctrl, 0xE0700C, ulFLA_ADR, 0); // FLA_ADR Information Mat 2 Page 2
	// 0x00040000 : Information Mat 2 Page 0
	// 0x00040010 : Information Mat 2 Page 1
	// 0x00040020 : Information Mat 2 Page 2
	// 0x00040030 : Information Mat 2 Page 3
	// 0x00100000 : Trimming Mat      Page 0
	// 0x00100010 : Trimming Mat      Page 1
	IOWrite32A(o_ctrl, 0xE07010, 0x00000002, 0); // CMD
	for ( i = 0; i < 16; i++ ){
		IOWrite32A(o_ctrl, 0xE07004, ulWriteVal[i], 0); // FLA_WDAT
	}
	do {
		IORead32A(o_ctrl, 0xE07018, &ulReadVal ); // FLAINT
		if( (ulReadVal & 0x80) == 0 ){
			break; // Success
		}
		msleep( 1 );
	}while( ulCnt++ < 10 );

	IOWrite32A(o_ctrl, 0xE07010, 0x00000008, 0); // CMD PROGRAM (Page Program)

	msleep( 3 );
	ulCnt = 0;
	do{
		IORead32A(o_ctrl, 0xE07018, &ulReadVal ); // FLAINT
TRACE( " FlashInformationMatProgram : ulReadVal = %02x \n", ulReadVal );
		if( (ulReadVal & 0x80) == 0 ){
			return ( 0x00 ); // Success
		}
		msleep( 1 );
	}while( ulCnt++ < 10 );

	return ( 1 ); // Error
}

//********************************************************************************
// Function Name 	: Protect
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Protect
//********************************************************************************
UINT_8 Protect(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32 ulReadVal;

	// Protect
	IOWrite32A(o_ctrl, 0xE07014, 0x00000010, 0); // FLAWP
	IORead32A(o_ctrl, 0xE07014, &ulReadVal ); // FLAWP
	if( ulReadVal == 0x0000 ){
		return ( 0x00 ); // Success
	}
	else{
		return ( 0x01 ); // Error
	}
}

//********************************************************************************
// Function Name 	: VerifyRead
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Verify Read
//********************************************************************************
UINT_8	VerifyRead(struct cam_ois_ctrl_t *o_ctrl, UINT_32 *ulWriteVal, UINT_32 ulFLA_ADR )
{
	INT_16 i;
	UINT_32 ulReadVal;

	IOWrite32A(o_ctrl, 0xE07008, 0x0000000F, 0); // ACSCNT 0x0F + 1 = 16Word

	//IOWrite32A(o_ctrl, 0xE0700C, 0x00040020, 0); // FLA_ADR Information Mat 2 Page 2
	IOWrite32A(o_ctrl, 0xE0700C, ulFLA_ADR, 0); // FLA_ADR

	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD

	for( i = 0; i < 16; i++ ){
		IORead32A(o_ctrl, 0xE07000, &ulReadVal ); // FLA_RDAT
		if( ulReadVal != ulWriteVal[i] ){
			return ( 0x01 ); // Error
		}
	}
	return ( 0x00 ); // Success

}

//********************************************************************************
// Function Name 	: DebugRead
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Verify Read
//********************************************************************************
UINT_8	DebugRead(struct cam_ois_ctrl_t *o_ctrl, UINT_32 ulFLA_ADR )
{
	INT_16		i;
	UINT_32		ulReadVal;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
//	msleep( 5 ) ;

	IOWrite32A(o_ctrl, 0xE07008, 0x0000000F, 0); // ACSCNT	0x0F + 1 = 16Word
//	msleep( 5 ) ;

	//IOWrite32A(o_ctrl, 0xE0700C, 0x00040020, 0); // FLA_ADR		Information Mat 2 Page 2
	IOWrite32A(o_ctrl, 0xE0700C, ulFLA_ADR, 0); // FLA_ADR
	//IOWrite32A(o_ctrl, 0xE0700C, 0x00100000, 0); // FLA_ADR
//	msleep( 5 ) ;

	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD
//	msleep( 5 ) ;

	for( i = 0; i < 16; i++ ){
		IORead32A(o_ctrl, 0xE07000, &ulReadVal ); // FLA_RDAT
//		msleep( 5 ) ;
		TRACE( " ulReadVal[%02x] = %08x \n", i, ulReadVal );
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
//	msleep( 5 ) ;

	return ( 0x00 ); // Success

}

//********************************************************************************
// Function Name 	: DebugWrite
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Verify Read
//********************************************************************************
UINT_8 DebugWrite(struct cam_ois_ctrl_t *o_ctrl)
{
	//INT_16i;
	//UINT_32 ulReadVal;
	UINT_16 usResult;
	UINT_32 ulWriteVal0[16];
	UINT_32 ulWriteVal1[16];
	UINT_16 i;
	UINT_32 ulTrimRegVal;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);

	IOWrite32A(o_ctrl, SYSDSP_DSPDIV, 0x00000001, 0);// 80MHz(129)
	IOWrite32A(o_ctrl, 0xE074CC, 0x00000080, 0);  // RSTB_FLA	Bit7 Don not change
	IOWrite32A(o_ctrl, 0xE0700C, 0x00200000, 0); // FLA_ADR	User Trimming Mat
	msleep( 1 ); // 20usec
	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD		Read
	IORead32A(o_ctrl, 0xE07000, &ulTrimRegVal ); // FLA_RDAT

	usResult = ProtectRelease(o_ctrl, TRIM_MAT ); // TRIM_MAT
	if( usResult != 0x00 ){
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
		return ( 0x01 ); // Error
	}

	// Flash Trimming Mat Erase
	usResult = FlashInformationMatErase(o_ctrl, TRIM_MAT );
	if( usResult != 0x00 ){
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
		return ( 0x02 ); // Error
	}

	// Data
	ulWriteVal0[0] = ulTrimRegVal;
	ulWriteVal0[1] = ~ulTrimRegVal;
	for( i = 2; i <= 15; i++ ){ // 14 times
		ulWriteVal0[i] = 0x00000000;
	}
	#if 0
	ulWriteVal0[0]  = 0xc4fc3f12;
	ulWriteVal0[1]  = 0x3b03c0ed;
	ulWriteVal0[2]  = 0xffffffff;
	ulWriteVal0[3]  = 0xffffffff;
	ulWriteVal0[4]  = 0xffffffff;
	ulWriteVal0[5]  = 0xffffffff;
	ulWriteVal0[6]  = 0xffffffff;
	ulWriteVal0[7]  = 0xffffffff;
	ulWriteVal0[8]  = 0xffffffff;
	ulWriteVal0[9]  = 0xffffffff;
	ulWriteVal0[10] = 0xffffffff;
	ulWriteVal0[11] = 0xffffffff;
	ulWriteVal0[12] = 0xffffffff;
	ulWriteVal0[13] = 0xffffffff;
	ulWriteVal0[14] = 0xffffffff;
	ulWriteVal0[15] = 0xffffffff;
	#endif

	// Flash Trimming Mat Page 0 Program
	usResult = FlashInformationMatProgram(o_ctrl, ulWriteVal0, 0x00100000 );
	if( usResult != 0x00 ){
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
		return ( 0x03 ); // Error
	}

	// Data
	for( i = 0; i <= 15; i++ ){ // 16 times
		ulWriteVal1[i] = 0x00000000;
	}

	#if 0
	ulWriteVal1[0]  = 0x55ffffff;
	ulWriteVal1[1]  = 0x00d18f00;
	ulWriteVal1[2]  = 0xbf40bf3f;
	ulWriteVal1[3]  = 0xbf40bf3f;
	ulWriteVal1[4]  = 0x91009100;
	ulWriteVal1[5]  = 0xbf40bf40;
	ulWriteVal1[6]  = 0xbf40bf40;
	ulWriteVal1[7]  = 0xffffffff;
	ulWriteVal1[8]  = 0xffffffff;
	ulWriteVal1[9]  = 0xffffffff;
	ulWriteVal1[10] = 0xffffffff;
	ulWriteVal1[11] = 0xffffffff;
	ulWriteVal1[12] = 0xe4d51cf8;
	ulWriteVal1[13] = 0xffffcccc;
	ulWriteVal1[14] = 0x99756768;
	ulWriteVal1[15] = 0x01ac29ac;
	#endif

	// Flash Trimming Mat Page 1 Program
	usResult = FlashInformationMatProgram(o_ctrl, ulWriteVal1, 0x00100010 );
	if( usResult != 0x00 ){
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
		return ( 0x04 ); // Error
	}

	// Protect
	usResult = Protect(o_ctrl);
	if( usResult != 0x00 ){
		IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
		return ( 0x05 ); // Error
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
	return ( 0x00 ); // Success

}

//********************************************************************************
// Function Name 	: OscAdjMain
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: OscAdjMain
//********************************************************************************
UINT_8 OscAdjMain(struct cam_ois_ctrl_t *o_ctrl)
{
	UINT_32 ulCVer;
	UINT_8 ucResult = 0x00;
	UINT_16 usOscVal = 0x0000;
	UINT_8 ucErrCode;

#if I2C_TIME_MODE == 1
#else	// I2C_TIME_MODE
	UINT_16 UsOscCan[3];
	UINT_16 UsMidOsc;
	UINT_8 UcOscSts = 0x00;
#endif	// I2C_TIME_MODE

TRACE("OscAdjMain \n");

	IOWrite32A(o_ctrl, 0xE0701C , 0x00000000, 0);

	IORead32A(o_ctrl, SYSDSP_CVER , &ulCVer );	// SYSDSP_CVER

	if (CheckCHECKCODE(o_ctrl, (UINT_16)ulCVer ) == 0x00) { // Check CHECKCODE
		TRACE( "OSC TSET \n" );
#if I2C_TIME_MODE == 1
		if( OscAdj(o_ctrl, &usOscVal ) == 0x00 ){ // OSC Clock adjustment
#else	// I2C_TIME_MODE
		UcOscSts	|= OscAdj(o_ctrl, &usOscVal ) ;
		UsOscCan[0] = usOscVal ;

		UcOscSts	|= OscAdj(o_ctrl, &usOscVal ) ;
		UsOscCan[1] = usOscVal ;

		UcOscSts	|= OscAdj(o_ctrl, &usOscVal ) ;
		UsOscCan[2] = usOscVal ;

		if( (UsOscCan[0] > UsOscCan[1]) && (UsOscCan[0] > UsOscCan[2]) ){
			UsMidOsc = ( UsOscCan[1] > UsOscCan[2]) ? UsOscCan[1] : UsOscCan[2] ;
		}else if( (UsOscCan[1] > UsOscCan[0]) && (UsOscCan[1] > UsOscCan[2]) ){
			UsMidOsc = ( UsOscCan[0] > UsOscCan[2]) ? UsOscCan[0] : UsOscCan[2] ;
		}else{
			UsMidOsc = ( UsOscCan[0] > UsOscCan[1]) ? UsOscCan[0] : UsOscCan[1] ;
		}

		TRACE( "%02X, %02X, %02X, Mid %02X \n", UsOscCan[0], UsOscCan[1], UsOscCan[2], UsMidOsc );
		usOscVal = UsMidOsc;

		if( UcOscSts == 0x00 ){ // OSC Clock adjustment
#endif	// I2C_TIME_MODE
			ucErrCode = WriteOscAdjVal(o_ctrl, usOscVal, ulCVer );
			if( ucErrCode == 0x00 ){ // Write Osc adjustment value
				ucResult = 0x00; // OK
			}
			else{
				TRACE( " WriteOscAdjVal() : NG (%02x)\n", ucErrCode);
				ucResult = 0x03;
			}
			BootMode(o_ctrl);
//			setI2cSlvAddr( DEFAULT_SLAVE_ADR, 2 );
		}
		else{
			TRACE( " OscAdj() : NG \n" );
			ucResult = 0x02;
		}
	}
	else{
		TRACE( " CheckCHECKCODE() : Done or Error \n" );
		ucResult = 0x01;
	}

	IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
	return( ucResult );
}

//********************************************************************************
// Function Name 	: CheckCHECKCODE
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Verify Read
//********************************************************************************
UINT_8 CheckCHECKCODE(struct cam_ois_ctrl_t *o_ctrl, UINT_16 usCVer )
{
	UINT_32 ulCheckCode1;
	UINT_32 ulCheckCode2;
	UINT_32 ulTrimRegVal;
	UINT_32 ulAdjTrim;

	IOWrite32A(o_ctrl, 0xE07008, 0x00000001, 0); // ACSCNT 0x01 + 1 = 2Word

	if( usCVer == 0x0141  ){ // LC898129 ES1
		IOWrite32A(o_ctrl, 0xE0700C, 0x0004002E, 0); // FLA_ADR Information Mat 2 Page 2
	}
	else if( (usCVer == 0x0144) || (usCVer == 0x0145) ) { // LC898129 ES2 or ES3
		IOWrite32A(o_ctrl, 0xE0700C, 0x0010001E, 0); // FLA_ADR Trimming Mat
	}
	else{
		return ( 0x02 ); // Error
	}

	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD
	IORead32A(o_ctrl, 0xE07000, &ulCheckCode1 ); // FLA_RDAT
	IORead32A(o_ctrl, 0xE07000, &ulCheckCode2 ); // FLA_RDAT

	IOWrite32A(o_ctrl, 0xE07008, 0x00000000, 0); // 1 word read
	IOWrite32A(o_ctrl, 0xE074CC, 0x00000080, 0); // RSTB_FLA	Bit7 Don not change
	IOWrite32A(o_ctrl, 0xE0700C, 0x00200000, 0); // FLA_ADR	User Trimming Mat
	IOWrite32A(o_ctrl, 0xE07010, 0x00000001, 0); // CMD		Read
	IORead32A(o_ctrl, 0xE07000, &ulTrimRegVal );// FLA_RDAT
	ulAdjTrim = TrimAdj( ulTrimRegVal );

TRACE( " ulCheckCode1 = %08x \n", (UINT_32)ulCheckCode1 );
TRACE( " ulCheckCode2 = %08x \n", (UINT_32)ulCheckCode2 );
TRACE( " ulTrimRegVal = %08x \n", (UINT_32)ulTrimRegVal );
TRACE( " ulAdjTrim    = %08x \n", (UINT_32)ulAdjTrim );

	if( ulCheckCode1 == 0x99756768 && ulCheckCode2 == 0x01AC29AC && ulTrimRegVal == ulAdjTrim ){
		return ( 0x01 ); // Done
	}

	return ( 0x00 ); // Not done
}

//********************************************************************************
// Function Name 	: GetOscTgtCount
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Get Osc Target Count
//********************************************************************************
UINT_32 GetOscTgtCount( UINT_16 time_us )
{
	UINT_32 ulOSC_TGT_COUNT;

	ulOSC_TGT_COUNT = (UINT_32)80 * (UINT_32)time_us / (UINT_32)7; //80MHz
TRACE( " ulOSC_TGT_COUNT = %08x \n", ulOSC_TGT_COUNT );

	return ( ulOSC_TGT_COUNT ); // Not done
}

//********************************************************************************
// Function Name 	: TrimAdj
// Retun Value		: adjTrim
// Argment Value	: srcTrim
// Explanation		: Adjustment triming data
//********************************************************************************
UINT_32 TrimAdj( UINT_32 srcTrim )
{
	UINT_8 bytTrim[4];
	UINT_8 bytSum = 0;
	UINT_32 adjTrim = 0;

	bytTrim[0] = (UINT_8)(srcTrim >> 24); // MSB
	bytTrim[1] = (UINT_8)(srcTrim >> 16); //
	bytTrim[2] = (UINT_8)(srcTrim >> 8); //
	bytTrim[3] = (UINT_8)(srcTrim >> 0); // LSB

	bytSum	= (bytTrim[3] & 0x0F) + (bytTrim[3] >> 4)
			+ (bytTrim[2] & 0x0F) + (bytTrim[2] >> 4)
			+ (bytTrim[1] & 0x08) + (bytTrim[1] >> 4);
	bytSum &= 0x0F;

	if( bytTrim[0] == 0x13 ) {
		// correct trimed
		adjTrim = srcTrim;
	} else if( (bytSum == (bytTrim[0] >> 4)) && (srcTrim != 0x00000000) ) {
		adjTrim = (UINT_32)(0x13000000)
				| ((UINT_32)bytTrim[1] << 16)
				| ((UINT_32)bytTrim[2] << 8)
				| ((UINT_32)bytTrim[3] << 0);
	} else {
		adjTrim = 0x13040000;
	}
	TRACE("srcTrim=%08X, Checksum=%02X, adjTrim=%08X\n", srcTrim, bytSum, adjTrim );
	return (adjTrim);
}
