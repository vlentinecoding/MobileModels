/**
 * @brief		LC898129 Global declaration & prototype declaration
 *
 * @author		Copyright (C) 2021, ON Semiconductor, all right reserved.
 *
 * @file		Lc898129.h
 **/

/************************************************/
/*	Command	*/
/************************************************/

#include "PhoneUpdate.h"

int lc898129_download_ois_fw(struct cam_ois_ctrl_t *o_ctrl);

//==============================================================================
//DMA
//==============================================================================
#define		GYRO_RAM_X						0x0088
				// GyroFilterDelay.h GYRO_RAM_t
#define			GYRO_RAM_GYROX_OFFSET			0x0000 + GYRO_RAM_X
#define			GYRO_RAM_GX2X4XF_IN				0x0004 + GYRO_RAM_X
#define			GYRO_RAM_GX2X4XF_OUT			0x0008 + GYRO_RAM_X
#define			GYRO_RAM_GXFAST					0x000C + GYRO_RAM_X
#define			GYRO_RAM_GXSLOW					0x0010 + GYRO_RAM_X
#define			GYRO_RAM_GYROX_G1OUT			0x0014 + GYRO_RAM_X
#define			GYRO_RAM_GYROX_G2OUT			0x0018 + GYRO_RAM_X
#define			GYRO_RAM_GYROX_G3OUT			0x001C + GYRO_RAM_X
#define			GYRO_RAM_GYROX_OUT				0x0020 + GYRO_RAM_X

#define		GYRO_RAM_Y						0x00AC
				// GyroFilterDelay.h GYRO_RAM_t
#define			GYRO_RAM_GYROY_OFFSET			0x0000 + GYRO_RAM_Y
#define			GYRO_RAM_GY2X4XF_IN				0x0004 + GYRO_RAM_Y
#define			GYRO_RAM_GY2X4XF_OUT			0x0008 + GYRO_RAM_Y
#define			GYRO_RAM_GYFAST					0x000C + GYRO_RAM_Y
#define			GYRO_RAM_GYSLOW					0x0010 + GYRO_RAM_Y
#define			GYRO_RAM_GYROY_G1OUT			0x0014 + GYRO_RAM_Y
#define			GYRO_RAM_GYROY_G2OUT			0x0018 + GYRO_RAM_Y
#define			GYRO_RAM_GYROY_G3OUT			0x001C + GYRO_RAM_Y
#define			GYRO_RAM_GYROY_OUT				0x0020 + GYRO_RAM_Y

#define		GYRO_RAM_COMMON					0x00D0
				// GyroFilterDelay.h GYRO_RAM_COMMON_t
#define			LC898129_GYRO_RAM_GX_ADIDAT				0x0000 + GYRO_RAM_COMMON
#define			GYRO_RAM_GY_ADIDAT				0x0004 + GYRO_RAM_COMMON
#define			GYRO_RAM_SINDX					0x0008 + GYRO_RAM_COMMON
#define			GYRO_RAM_SINDY					0x000C + GYRO_RAM_COMMON
#define			GYRO_RAM_GXLENSZ				0x0010 + GYRO_RAM_COMMON
#define			GYRO_RAM_GYLENSZ				0x0014 + GYRO_RAM_COMMON
#define			GYRO_RAM_GXOX_OUT				0x0018 + GYRO_RAM_COMMON
#define			GYRO_RAM_GYOX_OUT				0x001C + GYRO_RAM_COMMON
#define			GYRO_RAM_GXOFFZ					0x0020 + GYRO_RAM_COMMON	//0x000000F0 : GYRO_RAM.GXOFFZ
#define			GYRO_RAM_GYOFFZ					0x0024 + GYRO_RAM_COMMON	//0x000000F4 : GYRO_RAM.GYOFFZ
#define			GYRO_RAM_LIMITX					0x0028 + GYRO_RAM_COMMON
#define			GYRO_RAM_LIMITY					0x002C + GYRO_RAM_COMMON
#define			GYRO_RAM_GYROX_AFCnt			0x0030 + GYRO_RAM_COMMON
#define			GYRO_RAM_GYROY_AFCnt			0x0034 + GYRO_RAM_COMMON
#define			GYRO_RAM_GYRO_Switch			0x0038 + GYRO_RAM_COMMON		// 1Byte

#define		MsData						0x02C4
				// GyroFilterDelay.h MS_DATA_t
#define			MsData_GYRO_0					0x0000 + MsData
#define			MsData_GYRO_1					0x0004 + MsData
#define			MsData_GYRO_2					0x0008 + MsData
#define			MsData_ACCL_0					0x000C + MsData
#define			MsData_ACCL_1					0x0010 + MsData
#define			MsData_ACCL_2					0x0014 + MsData

#define		InfoMat						0x0414
				// LC898129_FROM.h InfoMat_t
#define			InfoMat_Mat0					0x0000 + MsData
#define			InfoMat_Mat1					0x0100 + MsData
#define			InfoMat_Mat2					0x0200 + MsData


#define		WaitTimerData					0x0790						// match LC898129_LVI_Billu16mm_v03
				// CommonLibrary.h  WaitTimer_Type
#define			WaitTimerData_UiWaitCounter		0x0000 + WaitTimerData			//0x00000790 : WaitTimerData.UiWaitCounter
#define			WaitTimerData_UiTargetCount		0x0004 + WaitTimerData			//0x00000794 : WaitTimerData.UiTargetCount


#define		StMeasureFunc					0x07A0							// match LC898129_LVI_Billu16mm_v03
				// MeasureFilter.h	MeasureFunction_Type
#define			StMeasFunc_SiSampleNum			0x0000 + StMeasureFunc			//0x000007A0 : StMeasureFunc.SiSampleNum
#define			StMeasFunc_SiSampleMax			0x0004 + StMeasureFunc			//0x000007A4 : StMeasureFunc.SiSampleMax

#define		StMeasureFunc_MFA				0x07A8							// match LC898129_LVI_Billu16mm_v03
#define			StMeasFunc_MFA_SiMax1			0x0000 + StMeasureFunc_MFA		//0x000007A8 : StMeasureFunc.MeasureFilterA.SiMax1
#define			StMeasFunc_MFA_SiMin1			0x0004 + StMeasureFunc_MFA		//0x000007AC : StMeasureFunc.MeasureFilterA.SiMin1
#define			StMeasFunc_MFA_UiAmp1			0x0008 + StMeasureFunc_MFA		//0x000007B0 : StMeasureFunc.MeasureFilterA.UiAmp1
//#define		StMeasFunc_MFA_UiDUMMY1			0x000C + StMeasureFunc_MFA
#define			StMeasFunc_MFA_LLiIntegral1		0x0010 + StMeasureFunc_MFA		//0x000007B8 : StMeasureFunc.MeasureFilterA.LLiIntegral1
#define			StMeasFunc_MFA_LLiAbsInteg1		0x0018 + StMeasureFunc_MFA		//0x000007C0 : StMeasureFunc.MeasureFilterA.LLiAbsInteg1
#define			StMeasFunc_MFA_PiMeasureRam1	0x0020 + StMeasureFunc_MFA		//0x000007C8 : StMeasureFunc.MeasureFilterA.PiMeasureRam1

#define		StMeasureFunc_MFB				0x07D0							// match LC898129_LVI_Billu16mm_v03
#define			StMeasFunc_MFB_SiMax2			0x0000 + StMeasureFunc_MFB		//0x000007D0 : StMeasureFunc.MeasureFilterB.SiMax2
#define			StMeasFunc_MFB_SiMin2			0x0004 + StMeasureFunc_MFB		//0x000007D4 : StMeasureFunc.MeasureFilterB.SiMin2
#define			StMeasFunc_MFB_UiAmp2			0x0008 + StMeasureFunc_MFB		//0x000007D8 : StMeasureFunc.MeasureFilterB.UiAmp2
//#define		StMeasFunc_MFB_UiDUMMY1			0x000C + StMeasureFunc_MFB
#define			StMeasFunc_MFB_LLiIntegral2		0x0010 + StMeasureFunc_MFB		//0x000007E0 : StMeasureFunc.MeasureFilterB.LLiIntegral2
#define			StMeasFunc_MFB_LLiAbsInteg2		0x0018 + StMeasureFunc_MFB		//0x000007E8 : StMeasureFunc.MeasureFilterB.LLiAbsInteg2
#define			StMeasFunc_MFB_PiMeasureRam2	0x0020 + StMeasureFunc_MFB		//0x000007F0 : StMeasureFunc.MeasureFilterB.PiMeasureRam2

#define		MeasureFilterA_Delay			0x07F8							// match LC898129_LVI_Billu16mm_v03
				// MeasureFilter.h	MeasureFilter_Delay_Type
#define			MeasureFilterA_Delay_z11		0x0000 + MeasureFilterA_Delay	//0x000007F8 : MeasureFilterA_Delay.z11
#define			MeasureFilterA_Delay_z12		0x0004 + MeasureFilterA_Delay	//0x000007FC : MeasureFilterA_Delay.z12
#define			MeasureFilterA_Delay_z21		0x0008 + MeasureFilterA_Delay	//0x00000800 : MeasureFilterA_Delay.z21
#define			MeasureFilterA_Delay_z22		0x000C + MeasureFilterA_Delay	//0x00000804 : MeasureFilterA_Delay.z22

#define		MeasureFilterB_Delay			0x0808							// match LC898129_LVI_Billu16mm_v03
				// MeasureFilter.h	MeasureFilter_Delay_Type
#define			MeasureFilterB_Delay_z11		0x0000 + MeasureFilterB_Delay	//0x00000808 : MeasureFilterB_Delay.z11
#define			MeasureFilterB_Delay_z12		0x0004 + MeasureFilterB_Delay	//0x0000080C : MeasureFilterB_Delay.z12
#define			MeasureFilterB_Delay_z21		0x0008 + MeasureFilterB_Delay	//0x00000810 : MeasureFilterB_Delay.z21
#define			MeasureFilterB_Delay_z22		0x000C + MeasureFilterB_Delay	//0x00000814 : MeasureFilterB_Delay.z22

#define		SinWaveC						0x0818								// match LC898129_LVI_Billu16mm_v03
#define			SinWaveC_Pt						0x0000 + SinWaveC
#define			SinWaveC_Regsiter				0x0004 + SinWaveC
//#define			SinWaveC_SignFlag				0x0004 + SinWaveC_Regsiter

#define		SinWave							0x0824								// match LC898129_LVI_Billu16mm_v03
				// SinGenerator.h SinWave_t
#define			SinWave_Offset					0x0000 + SinWave
#define			SinWave_Phase					0x0004 + SinWave
#define			SinWave_Gain					0x0008 + SinWave
#define			SinWave_Output					0x000C + SinWave
#define			SinWave_OutAddr					0x0010 + SinWave

#define		CosWave							0x0838								// match LC898129_LVI_Billu16mm_v03
				// SinGenerator.h SinWave_t
#define			CosWave_Offset					0x0000 + CosWave
#define			CosWave_Phase					0x0004 + CosWave
#define			CosWave_Gain					0x0008 + CosWave
#define			CosWave_Output					0x000C + CosWave
#define			CosWave_OutAddr					0x0010 + CosWave


#define			GyroRAM_Z_GYRO_OFFSET		0x0168					//0x00000168 : GyroRAM_Z.GYRO_OFFSET

#define		GyroFilterDelayX_GXH1Z2			0x004C					//GyroFilterDelayX.delay3[1]
#define		GyroFilterDelayY_GYH1Z2			0x0074					//GyroFilterDelayY.delay3[1]

#define			GYRO_ZRAM_GZ_ADIDAT			0x018C					//0x0000018C : GYRO_ZRAM.GZ_ADIDAT
#define			GYRO_ZRAM_GZOFFZ			0x0198					//0x00000198 : GYRO_ZRAM.GZOFFZ

#define		AcclFilDly_X					0x01B0					//0x000001B0 : AcclFilterDelayX.delay1[2]
#define		AcclFilDly_Y					0x01E0					//0x000001E0 : AcclFilterDelayY.delay1[2]
#define		AcclFilDly_Z					0x0210					//0x00000210 : AcclFilterDelayZ.delay1[2]

#define		AcclRAM_X						0x0240					//AcclRAM_X.AC_ADIDAT
#define			ACCLRAM_X_AC_ADIDAT			(0x0000 + AcclRAM_X)	//0x00000240 : AcclRAM_X.AC_ADIDAT
#define			ACCLRAM_X_AC_OFFSET			(0x0004 + AcclRAM_X)	//0x00000244 : AcclRAM_X.AC_OFFSET

#define		AcclRAM_Y						0x026C					//AcclRAM_Y.AC_ADIDAT
#define			ACCLRAM_Y_AC_ADIDAT			(0x0000 + AcclRAM_Y)	//0x0000026C : AcclRAM_Y.AC_ADIDAT
#define			ACCLRAM_Y_AC_OFFSET			(0x0004 + AcclRAM_Y)	//0x00000270 : AcclRAM_Y.AC_OFFSET

#define		AcclRAM_Z						0x0298					//AcclRAM_Z.AC_ADIDAT
#define			ACCLRAM_Z_AC_ADIDAT			(0x0000 + AcclRAM_Z)	//0x00000298 : AcclRAM_Z.AC_ADIDAT
#define			ACCLRAM_Z_AC_OFFSET			(0x0004 + AcclRAM_Z)	//0x0000029C : AcclRAM_Z.AC_OFFSET



//==============================================================================
//DMB
//==============================================================================
#define		MeasureFilterA_Coeff			0x8878
				// MeasureFilter.h  MeasureFilter_Type
#define			MeasureFilterA_Coeff_b1			0x0000 + MeasureFilterA_Coeff	//0x00800878 : MeasureFilterA_Coeff.b1
#define			MeasureFilterA_Coeff_c1			0x0004 + MeasureFilterA_Coeff	//0x0080087C : MeasureFilterA_Coeff.c1
#define			MeasureFilterA_Coeff_a1			0x0008 + MeasureFilterA_Coeff	//0x00800880 : MeasureFilterA_Coeff.a1
#define			MeasureFilterA_Coeff_b2			0x000C + MeasureFilterA_Coeff	//0x00800884 : MeasureFilterA_Coeff.b2
#define			MeasureFilterA_Coeff_c2			0x0010 + MeasureFilterA_Coeff	//0x00800888 : MeasureFilterA_Coeff.c2
#define			MeasureFilterA_Coeff_a2			0x0014 + MeasureFilterA_Coeff	//0x0080088C : MeasureFilterA_Coeff.a2

#define		MeasureFilterB_Coeff			0x8890
				// MeasureFilter.h  MeasureFilter_Type
#define			MeasureFilterB_Coeff_b1			0x0000 + MeasureFilterB_Coeff	//0x00800890 : MeasureFilterB_Coeff.b1
#define			MeasureFilterB_Coeff_c1			0x0004 + MeasureFilterB_Coeff	//0x00800894 : MeasureFilterB_Coeff.c1
#define			MeasureFilterB_Coeff_a1			0x0008 + MeasureFilterB_Coeff	//0x00800898 : MeasureFilterB_Coeff.a1
#define			MeasureFilterB_Coeff_b2			0x000C + MeasureFilterB_Coeff	//0x0080089C : MeasureFilterB_Coeff.b2
#define			MeasureFilterB_Coeff_c2			0x0010 + MeasureFilterB_Coeff	//0x008008A0 : MeasureFilterB_Coeff.c2
#define			MeasureFilterB_Coeff_a2			0x0014 + MeasureFilterB_Coeff	//0x008008A4 : MeasureFilterB_Coeff.a2




#define		GyroFilterTableX				0x806C
				// GyroFilterCoeff.h  DM_GFC_t
#define			GyroFilterTableX_gx45x			(0x0000 + GyroFilterTableX)		//0x0080006C : GyroFilterTableX.coeff1[4]
#define			GyroFilterTableX_gx45y			(0x0004 + GyroFilterTableX)

#define		GyroFilterTableY				0x80CC
				// GyroFilterCoeff.h  DM_GFC_t
#define			GyroFilterTableY_gy45y			(0x0000 + GyroFilterTableY)		//0x008000CC : GyroFilterTableY.coeff1[4]
#define			GyroFilterTableY_gy45x			(0x0004 + GyroFilterTableY)

#define		Accl45Filter					0x8334
#define			Accl45Filter_XAmain				(0x0000 + Accl45Filter )		//0x00800334 : Accl45FilterX.Amain
#define			Accl45Filter_XAsub				(0x0004 + Accl45Filter )		//0x00800338 : Accl45FilterX.Asub
#define			Accl45Filter_YAmain				(0x0008 + Accl45Filter )		//0x0080033C : Accl45FilterY.Amain
#define			Accl45Filter_YAsub				(0x000C + Accl45Filter )		//0x00800340 : Accl45FilterY.Asub

#define		MotionSensor_Sel				0x8350
#define			MS_SEL_GX0						(0x0000 + MotionSensor_Sel)		//0x00800350 : MS_SEL.GX[2]
#define			MS_SEL_GX1						(0x0004 + MotionSensor_Sel)
#define			MS_SEL_GY0						(0x0008 + MotionSensor_Sel)		//0x00800358 : MS_SEL.GY[2]
#define			MS_SEL_GY1						(0x000C + MotionSensor_Sel)
#define			MS_SEL_GZ						(0x0010 + MotionSensor_Sel)		//0x00800360 : MS_SEL.GZ
#define			MS_SEL_AX0						(0x0014 + MotionSensor_Sel)		//0x00800364 : MS_SEL.AX[2]
#define			MS_SEL_AX1						(0x0018 + MotionSensor_Sel)
#define			MS_SEL_AY0						(0x001C + MotionSensor_Sel)		//0x0080036C : MS_SEL.AY[2]
#define			MS_SEL_AY1						(0x0020 + MotionSensor_Sel)
#define			MS_SEL_AZ						(0x0024 + MotionSensor_Sel)		//0x00800374 : MS_SEL.AZ

