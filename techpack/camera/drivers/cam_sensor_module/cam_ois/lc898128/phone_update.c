// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021_2021, The Linux Foundation. All rights reserved.
 */

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

#include "phone_update.h"

#include "Xuchang_FromCode_03_00.h"
#include "Xuchang_FromCode_07_00.h"

/* Burst Length for updating to PMEM */
#define BURST_LENGTH_UC     (3*20) // 60 Total:63Byte Burst
/* Burst Length for updating to Flash */
#define BURST_LENGTH_FC     32 // 32 Total: 35Byte Burst
#define ONE_MSEC_COUNT      15

const DOWNLOAD_TBL_EXT DTbl[] = {
	{0x0300, CcUpdataCode128_03_00, UpDataCodeSize_03_00,  UpDataCodeCheckSum_03_00, CcFromCode128_03_00, sizeof(CcFromCode128_03_00), FromCheckSum_03_00, FromCheckSumSize_03_00 },
	{0x0100, CcUpdataCode128_07_00, UpDataCodeSize_07_00,  UpDataCodeCheckSum_07_00, CcFromCode128_07_00, sizeof(CcFromCode128_07_00), FromCheckSum_07_00, FromCheckSumSize_07_00 },
	{0xFFFF, (void*)0, 0, 0, (void*)0, 0, 0, 0}
};

static void SetGyroCoef(struct cam_ois_ctrl_t *o_ctrl, uint8_t);
static void SetAccelCoef(struct cam_ois_ctrl_t *o_ctrl, uint8_t);

//********************************************************************************
// Function Name  : UnlockCodeSet
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : <Flash Memory> Unlock Code Set
// History        : First edition
//********************************************************************************
uint8_t UnlockCodeSet(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlReadVal, UlCnt = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -1;
	}

	do {
		IOWrite32A(o_ctrl, 0xE07554, 0xAAAAAAAA, 0); // UNLK_CODE1(E0_7554h) = AAAA_AAAAh
		IOWrite32A(o_ctrl, 0xE07AA8, 0x55555555, 0); // UNLK_CODE2(E0_7AA8h) = 5555_5555h
		IORead32A(o_ctrl, 0xE07014, &UlReadVal);
		if( (UlReadVal & 0x00000080) != 0 )
			return 0; // Check UNLOCK(E0_7014h[7])
		usleep_range(1000, 1001);
	} while ( UlCnt++ < 10 );
	return 1;
}

//********************************************************************************
// Function Name  : UnlockCodeClear
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : <Flash Memory> Clear Unlock Code
// History        : First edition
//********************************************************************************
uint8_t UnlockCodeClear(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlDataVal, UlCnt = 0;

	do {
		IOWrite32A(o_ctrl, 0xE07014, 0x00000010, 0); // UNLK_CODE3(E0_7014h[4]) = 1
		IORead32A(o_ctrl, 0xE07014, &UlDataVal);
		if( (UlDataVal & 0x00000080) == 0 )
			return 0; // Check UNLOCK(E0_7014h[7])
		usleep_range(1000, 1001);
	} while( UlCnt++ < 10 );
	return 3;
}

//********************************************************************************
// Function Name  : WritePermission
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : LC898128 Command
// History        : First edition 2018.05.15
//********************************************************************************
void WritePermission(struct cam_ois_ctrl_t *o_ctrl)
{
	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	IOWrite32A(o_ctrl, 0xE074CC, 0x00000001, 0); // RSTB_FLA_WR(E0_74CCh[0])=1
	IOWrite32A(o_ctrl, 0xE07664, 0x00000010, 0); // FLA_WR_ON(E0_7664h[4])=1
}

//********************************************************************************
// Function Name  : AddtionalUnlockCodeSet
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : LC898128 Command
// History        : First edition 2018.05.15
//********************************************************************************
void AddtionalUnlockCodeSet(struct cam_ois_ctrl_t *o_ctrl)
{
	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}
	IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 0); // UNLK_CODE3(E0_7CCCh) = 0000_ACD5h
}

uint8_t CoreResetwithoutMC128(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlReadVal = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	IOWrite32A(o_ctrl, 0xE07554, 0xAAAAAAAA, 0);
	IOWrite32A(o_ctrl, 0xE07AA8, 0x55555555, 0);

	IOWrite32A(o_ctrl, 0xE074CC, 0x00000001, 0);
	IOWrite32A(o_ctrl, 0xE07664, 0x00000010, 0);
	IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 0);
	IOWrite32A(o_ctrl, 0xE0700C, 0x00000000, 0);
	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	IOWrite32A(o_ctrl, 0xE07010, 0x00000004, 100000);
	usleep_range(100000, 100001);
	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
	IOWrite32A(o_ctrl, 0xE07014, 0x00000010, 0);

	IOWrite32A(o_ctrl, 0xD00060, 0x00000001, 15000) ;
	usleep_range(15000, 15001);
	IORead32A(o_ctrl, ROMINFO, &UlReadVal);

	CAM_DBG(CAM_OIS, "[OISDEBUG] read from addr 0x%04x =  0x%x", ROMINFO, UlReadVal);
	switch ((uint8_t)UlReadVal) {
	case 0x08:
	case 0x0D:
		break;
	default:
		return(0xE0 | (uint8_t)UlReadVal);
	}

	return 0;
}

uint8_t PmemUpdate128(struct cam_ois_ctrl_t *o_ctrl, DOWNLOAD_TBL_EXT* ptr)
{
	uint8_t data[BURST_LENGTH_UC + 2 ];
	uint32_t Remainder;
	const uint8_t *NcDataVal = ptr->UpdataCode;
	uint8_t ReadData[8];
	long long CheckSumCode = ptr->SizeUpdataCodeCksm;
	uint8_t *p = (uint8_t *)&CheckSumCode;
	uint32_t i;
	uint32_t j;
	uint32_t UlReadVal;
	uint32_t UlCnt;
	uint32_t UlNum;
//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x3000, 0x00080000, 0); // Pmem address set

	data[0] = 0x40;
	data[1] = 0x00;


	Remainder = ((ptr->SizeUpdataCode * 5) / BURST_LENGTH_UC);
	for (i = 0; i < Remainder; i++) {
		UlNum = 2;
		for (j = 0; j < BURST_LENGTH_UC; j++){
			data[UlNum] = *NcDataVal++;
			if((j % 5) == 4)
				TRACE("\n");
			UlNum++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_UC + 2, 0);
	}
	Remainder = ((ptr->SizeUpdataCode * 5) % BURST_LENGTH_UC);
	if (Remainder != 0) {
		UlNum = 2;
		for (j = 0; j < Remainder; j++){
			data[UlNum++] = *NcDataVal++;
		}
		CntWrt(o_ctrl, data, Remainder + 2, 0);
	}
//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
	data[0] = 0xF0;
	data[1] = 0x0E;
	data[2] = (unsigned char)((ptr->SizeUpdataCode >> 8) & 0x000000FF);
	data[3] = (unsigned char)(ptr->SizeUpdataCode & 0x000000FF);
	data[4] = 0x00;
	data[5] = 0x00;

	CntWrt(o_ctrl, data, 6, 0);
	UlCnt = 0;
	do {
		usleep_range(1000, 1001);
		if(UlCnt++ > 10) {
			IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
			return 0x21;
		}
		RamRead32A(o_ctrl, 0x0088, &UlReadVal); // PmCheck.ExecFlag
	} while (UlReadVal != 0);

	CntRd(o_ctrl, 0xF00E, ReadData, 8);

	IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
	for (i = 0; i < 8; i++) {
		if (ReadData[ 7 - i ] != *p++ ) {
			return 0x22;
		}
	}

	return 0;
}

//********************************************************************************
// Function Name  : EraseUserMat128
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : User Mat All Erase
// History        : First edition
//********************************************************************************
uint8_t EraseUserMat128(struct cam_ois_ctrl_t *o_ctrl, uint8_t StartBlock, uint8_t EndBlock)
{
	uint32_t i;
	uint32_t UlReadVal;
	uint32_t UlCnt;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF007, 0x00000000, 0); // FlashAccess Setup

	//***** User Mat *****
	for(i = StartBlock; i<EndBlock ; i++) {
		RamWrite32A(o_ctrl, 0xF00A, ( i << 10 ), 0); // FromCmd.Addr
		RamWrite32A(o_ctrl, 0xF00C, 0x00000020, 0); // FromCmd.Control

		usleep_range(5000, 5001);
		UlCnt = 0;
		do {
			usleep_range(1000, 1001);
			if( UlCnt++ > 10 ){
				IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
				return 0x31; // block erase timeout ng
			}
			RamRead32A(o_ctrl, 0xF00C, &UlReadVal); // FromCmd.Control
		} while ( UlReadVal != 0 );
	}
	IOWrite32A(o_ctrl, 0xE0701C ,0x00000002, 0);
	return 0;

}

uint8_t ProgramFlash128_Standard(struct cam_ois_ctrl_t *o_ctrl, DOWNLOAD_TBL_EXT* ptr)
{
	uint32_t UlReadVal, UlCnt , UlNum ;
	uint8_t data[(BURST_LENGTH_FC + 3)];
	uint32_t i;
	uint32_t j;

	const uint8_t *NcFromVal = ptr->FromCode + 64;
	const uint8_t *NcFromVal1st = ptr->FromCode;
	uint8_t UcOddEvn;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF007, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00A, 0x00000010, 0 );
	data[0] = 0xF0;
	data[1] = 0x08;
	data[2] = 0x00;

	for ( i = 1; i < (ptr->SizeFromCode / 64); i++)
	{
		if( ++UcOddEvn > 1)
			UcOddEvn = 0;
		if (UcOddEvn == 0)
			data[1] = 0x08;
		else
			data[1] = 0x09;

#if (BURST_LENGTH_FC == 32)
		data[2] = 0x00;
		UlNum = 3;
		for (j = 0 ; j < BURST_LENGTH_FC; j++) {
			data[UlNum++] = *NcFromVal++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0);
		data[2] = 0x20;
		UlNum = 3;
		for (j = 0 ; j < BURST_LENGTH_FC; j++){
			data[UlNum++] = *NcFromVal++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0);
#elif (BURST_LENGTH_FC == 64)
		UlNum = 3;
		for(j=0 ; j < BURST_LENGTH_FC; j++){
			data[UlNum++] = *NcFromVal++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC+3, 0);
#endif

		RamWrite32A(o_ctrl, 0xF00B, 0x00000010, 0 );
		UlCnt = 0;
		if (UcOddEvn == 0){
			do {
				RamRead32A(o_ctrl, 0xF00C, &UlReadVal );
				if( UlCnt++ > 250 ) {
					IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
					return (0x41);
				}
			} while (UlReadVal != 0);
			RamWrite32A(o_ctrl, 0xF00C, 0x00000004, 0 );
		} else {
			do {
				RamRead32A(o_ctrl, 0xF00C, &UlReadVal );
				if( UlCnt++ > 250 ) {
					IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
					return (0x41);
				}
			} while (UlReadVal != 0);
			RamWrite32A(o_ctrl, 0xF00C, 0x00000008, 0);
		}
	}
	UlCnt = 0;
	do{
		usleep_range(1000, 1001);
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal );
		if( UlCnt++ > 250 ) {
			IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
			return (0x41);
		}
	} while ((UlReadVal & 0x0000000C) != 0);

	{
		RamWrite32A(o_ctrl, 0xF00A, 0x00000000, 0 );
		data[1] = 0x08;

#if (BURST_LENGTH_FC == 32)
		data[2] = 0x00;
		UlNum = 3;
		for (j = 0 ; j < BURST_LENGTH_FC; j++){
			data[UlNum++] = *NcFromVal1st++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0);
		data[2] = 0x20;
		UlNum = 3;
		for (j = 0; j < BURST_LENGTH_FC; j++){
			data[UlNum++] = *NcFromVal1st++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0);
#elif (BURST_LENGTH_FC == 64)
		data[2] = 0x00;
		UlNum = 3;
		for (j = 0; j < BURST_LENGTH_FC; j++){
			data[UlNum++] = *NcFromVal1st++;
		}
		CntWrt(o_ctrl, data, BURST_LENGTH_FC + 3, 0);
#endif

		RamWrite32A(o_ctrl, 0xF00B, 0x00000010, 0);
		UlCnt = 0;
		do{
			RamRead32A(o_ctrl, 0xF00C, &UlReadVal );
			if( UlCnt++ > 250 ) {
				IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
				return (0x41) ;
			}
		}while ( UlReadVal != 0 );
	 	RamWrite32A(o_ctrl, 0xF00C, 0x00000004, 0 );
	}

	UlCnt = 0;
	do{
		usleep_range(1000, 1001);
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal );
		if( UlCnt++ > 250 ) {
			IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
			return (0x41) ;
		}
	}while ( (UlReadVal & 0x0000000C) != 0 );

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
	return( 0 );
}

//********************************************************************************
// Function Name  : FlashMultiRead
// Retun Value    : NON
// Argment Value  : NON
// Explanation    : <Flash Memory> Flash Multi Read( 4 Byte * length  max read : 128byte)
// History        : First edition
//********************************************************************************
uint8_t FlashMultiRead(struct cam_ois_ctrl_t *o_ctrl, uint8_t SelMat, uint32_t UlAddress, uint32_t *PulData , uint8_t UcLength)
{
	uint8_t i;

	// fail safe
	// reject irregular mat
	if( SelMat != USER_MAT && SelMat != INF_MAT0 && SelMat != INF_MAT1 && SelMat != INF_MAT2)
		return 10; // INF_MAT2 Read only Access
	// reject command if address inner NVR3
	if( UlAddress > 0x000003FF )
		return 9;

	IOWrite32A(o_ctrl, FLASHROM_ACSCNT, 0x00000000 | (uint32_t)(UcLength-1), 0);
	IOWrite32A(o_ctrl, FLASHROM_FLA_ADR, ((uint32_t)SelMat << 16) | ( UlAddress & 0x00003FFF ), 0);

	IOWrite32A(o_ctrl, FLASHROM_FLAMODE, 0x00000000, 0);
	IOWrite32A(o_ctrl, FLASHROM_CMD, 0x00000001, 0);
	for( i=0 ; i < UcLength ; i++ ){
		IORead32A(o_ctrl, FLASHROM_FLA_RDAT, &PulData[i]);
		CAM_DBG(CAM_OIS, "Read Data[%d] = 0x%08x", i, PulData[i]);
	}
	IOWrite32A(o_ctrl, FLASHROM_FLAMODE, 0x00000002, 0);

	return 0;
}

//********************************************************************************
// Function Name 	: FlashBlockErase
// Retun Value		: 0 : Success, 1 : Unlock Error, 2 : Time Out Error
// Argment Value	: Use Mat , Flash Address
// Explanation		: <Flash Memory> Block Erase
// History		: First edition
// Unit of erase	: informaton mat  128 Byte
//					: user mat         64 Byte
//********************************************************************************
uint8_t FlashBlockErase(struct cam_ois_ctrl_t *o_ctrl, uint8_t SelMat , uint32_t SetAddress)
{
	uint32_t UlReadVal, UlCnt;
	uint8_t ans = 0;

	// fail safe
	// reject irregular mat
	if (SelMat != USER_MAT && SelMat != INF_MAT0 && SelMat != INF_MAT1 && SelMat != INF_MAT2)
		return 10;
	// reject command if address inner NVR3
	if(SetAddress > 0x000003FF)
		return 9;

	// Flash write
	ans = UnlockCodeSet(o_ctrl);
	if (ans != 0)
		return ans; // Unlock Code Set

	WritePermission(o_ctrl); // Write permission
	if (SelMat != USER_MAT) {
		if (SelMat == INF_MAT2)
			IOWrite32A(o_ctrl, 0xE07CCC, 0x00006A4B, 0);
		else
			IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0); // additional unlock for INFO
	}
	AddtionalUnlockCodeSet(o_ctrl); // common additional unlock code set

	IOWrite32A(o_ctrl, FLASHROM_FLA_ADR, ((uint32_t)SelMat << 16) | ( SetAddress & 0x00003C00 ), 0);
	// Sector Erase Start
	IOWrite32A(o_ctrl, FLASHROM_FLAMODE, 0x00000000, 0);
	IOWrite32A(o_ctrl, FLASHROM_CMD, 4, 0);

	usleep_range(5000, 5001);

	UlCnt = 0;
	do {
		if (UlCnt++ > 100) {
			ans = 2;
			break;
		}
		IORead32A(o_ctrl, FLASHROM_FLAINT, &UlReadVal);
	} while (( UlReadVal & 0x00000080 ) != 0);

	IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
	ans = UnlockCodeClear(o_ctrl); // Unlock Code Clear
	if (ans != 0)
		return ans; // Unlock Code Set

	return ans;
}

//********************************************************************************
// Function Name 	: FlashBlockWrite
// Retun Value		: 0 : Success, 1 : Unlock Error, 2 : Time Out Error
// Argment Value	: Info Mat , Flash Address
// Explanation		: <Flash Memory> Block Erase
// History		: First edition
// Unit of erase	: informaton mat   64 Byte
//					: user mat         64 Byte
//********************************************************************************
uint8_t FlashBlockWrite(struct cam_ois_ctrl_t *o_ctrl, uint8_t SelMat, uint32_t SetAddress, uint32_t *PulData)
{
	uint32_t UlReadVal, UlCnt;
	uint8_t ans = 0;
	uint8_t i;

	// fail safe
	// reject irregular mat
	// if( SelMat != INF_MAT0 && SelMat != INF_MAT1  )
	// return 10; // USR MAT,INF_MAT2 Access
	if (SelMat != INF_MAT0 && SelMat != INF_MAT1 && SelMat != INF_MAT2)
		return 10; // USR MAT

	if (SetAddress > 0x000003FF)
		return 9;

	// Flash write
	ans = UnlockCodeSet(o_ctrl);
	if (ans != 0)
		return ans; // Unlock Code Set

	WritePermission(o_ctrl); // Write permission
	if (SelMat != USER_MAT) {
		if(SelMat == INF_MAT2)
			IOWrite32A(o_ctrl, 0xE07CCC, 0x00006A4B, 0);
		else
			IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0); // additional unlock for INFO
	}
	AddtionalUnlockCodeSet(o_ctrl);	// common additional unlock code set

	IOWrite32A(o_ctrl, FLASHROM_FLA_ADR, ((uint32_t)SelMat << 16) | ( SetAddress & 0x000010), 0);
	// page write Start
	IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000000, 0);
	IOWrite32A(o_ctrl, FLASHROM_CMD, 2, 0);

	// usleep_range(5000, 5001);

	UlCnt = 0;

	for (i = 0; i < 16; i++) {
		IOWrite32A(o_ctrl, FLASHROM_FLA_WDAT, PulData[i], 0); // Write data
		CAM_DBG(CAM_OIS, "Write Data[%d] = %08x", i , PulData[i] );
	}
	do {
		if (UlCnt++ > 100) {
			ans = 2;
			break;
		}
		IORead32A(o_ctrl, FLASHROM_FLAINT, &UlReadVal);
	} while(( UlReadVal & 0x00000080 ) != 0);

	CAM_DBG(CAM_OIS, "[FlashBlockWrite] BEGIN FLASHROM_CMD Write Data");
	// page program
	IOWrite32A(o_ctrl, FLASHROM_CMD, 8, 0);

        CAM_DBG(CAM_OIS, "[FlashBlockWrite] END FLASHROM_CMD Write Data");
	do {
		if ( UlCnt++ > 100 ) {
			ans = 2;
			break;
		}
		IORead32A(o_ctrl, FLASHROM_FLAINT, &UlReadVal);
	} while (( UlReadVal & 0x00000080 ) != 0);

	IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
	ans = UnlockCodeClear(o_ctrl);	// Unlock Code Clear
	CAM_DBG(CAM_OIS, "[FlashBlockWrite] END OF FlashBlockWrite ans = %d", ans);
	return ans;
}

//********************************************************************************
// Function Name 	: Mat2ReWrite
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Mat2 re-write function
// History		: First edition
//********************************************************************************
uint8_t Mat2ReWrite(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlMAT2[32];
	uint32_t UlCKSUM=0;
	uint32_t UlCkVal, UlCkVal_Bk;
	uint8_t ans, i;

	ans = FlashMultiRead(o_ctrl, INF_MAT2, 0, UlMAT2, 32);
	if (ans)
		return( 0xA0 );
	/* FT_REPRG check *****/
	if (UlMAT2[FT_REPRG] == PRDCT_WR || UlMAT2[FT_REPRG] == USER_WR){
		return( 0x00 );
	}

	/* Check code check *****/
	if (UlMAT2[CHECKCODE1] != CHECK_CODE1)
		return( 0xA1 );
	if (UlMAT2[CHECKCODE2] != CHECK_CODE2)
		return( 0xA2 );

	/* Check sum check *****/
	for (i = 16; i < MAT2_CKSM; i++){
		UlCKSUM += UlMAT2[i];
	}
	if(UlCKSUM != UlMAT2[MAT2_CKSM])
		return(0xA3);

	/* registor re-write flag *****/
	UlMAT2[FT_REPRG] = USER_WR;

	/* backup sum check before re-write *****/
	UlCkVal_Bk = 0;
	for (i = 0; i < 32; i++){ // S
		UlCkVal_Bk +=  UlMAT2[i];
	}

	/* Erase   ******************************************************/
	ans = FlashBlockErase(o_ctrl, INF_MAT2, 0); // all erase
	if (ans != 0)
		return 0xA4; // Unlock Code Set

	/* excute re-write *****/
	ans = FlashBlockWrite(o_ctrl, INF_MAT2 , 0, UlMAT2);
	if (ans != 0)
		return 0xA5; // Unlock Code Set
	ans = FlashBlockWrite(o_ctrl, INF_MAT2, (uint32_t)0x10, &UlMAT2[0x10]);
	if (ans != 0)
		return 0xA5; // Unlock Code Set

	ans = FlashMultiRead(o_ctrl, INF_MAT2, 0, UlMAT2, 32);
	if (ans)
		return 0xA0;
	UlCkVal = 0;
	for (i = 0; i < 32; i++) {
		UlCkVal += UlMAT2[i];
	}

	if (UlCkVal != UlCkVal_Bk)
		return 0xA6; // write data != writen data
	return 0x01; // re-write ok
}


//********************************************************************************
// Function Name 	: FlashUpdate128
//********************************************************************************
uint8_t FlashUpdate128(struct cam_ois_ctrl_t *o_ctrl, DOWNLOAD_TBL_EXT* ptr )
{
	uint8_t ans=0;
	uint32_t UlReadVal;
	uint32_t UlCnt;

	ans = CoreResetwithoutMC128(o_ctrl);
	if (ans != 0) return( ans );

	ans = Mat2ReWrite(o_ctrl);
	if (ans != 0 && ans != 1)
		return ans;

	ans = PmemUpdate128(o_ctrl, ptr );
	if (ans != 0)
		return ans;

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
	if (UnlockCodeSet(o_ctrl) != 0)
		return (0x33);
	WritePermission(o_ctrl);
	AddtionalUnlockCodeSet(o_ctrl);

 	ans = EraseUserMat128(o_ctrl, 0, 9);
	if (ans != 0){
		if (UnlockCodeClear(o_ctrl) != 0)
			return (0x32);
		else
			return (ans);
	}

	ans = ProgramFlash128_Standard(o_ctrl, ptr);
	if (ans != 0){
		if (UnlockCodeClear(o_ctrl) != 0)
			return (0x43);
		else
			return (ans);
	}

	if (UnlockCodeClear(o_ctrl) != 0)
		return (0x43);

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00A, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00D, ptr->SizeFromCodeValid, 0);

	RamWrite32A(o_ctrl, 0xF00C, 0x00000100, 0);
	usleep_range(6000, 6001);
	UlCnt = 0;
	do {
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal);
		if(UlCnt++ > 10) {
			IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
			return (0x51);
		}
		usleep_range(1000, 1001);
	} while (UlReadVal != 0);

	RamRead32A(o_ctrl, 0xF00D, &UlReadVal);

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0);
	if (UlReadVal != ptr->SizeFromCodeCksm) {
		return (0x52);
	}

	IOWrite32A(o_ctrl, SYSDSP_REMAP, 0x00001000, 0);
	usleep_range(15000, 15001);
	IORead32A(o_ctrl, ROMINFO, (uint32_t *)&UlReadVal);
	if (UlReadVal != 0x0A)
		return (0x53);

	return(0);
}

//********************************************************************************
// Function Name 	: FlashDownload_128
//********************************************************************************
uint8_t FlashDownload128(struct cam_ois_ctrl_t *o_ctrl, uint8_t chiperase, uint8_t ModuleVendor, uint8_t ActVer )
{
	DOWNLOAD_TBL_EXT* ptr ;

	ptr = ( DOWNLOAD_TBL_EXT * )DTbl ;
	do {
		if( ptr->Index == ( ((uint32_t)ModuleVendor<<8) + ActVer) ) {
			return FlashUpdate128(o_ctrl, ptr );
		}
		ptr++ ;
	} while (ptr->Index != 0xFFFF ) ;

	return 0xF0 ;
}
#if 0
static void XuChangSetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, uint32_t GyroOffsetX, uint32_t GyroOffsetY, uint32_t GyroOffsetZ)
{
	RamWrite32A(o_ctrl, GYRO_RAM_GXOFFZ, ((GyroOffsetX << 16) & 0xFFFF0000), 0);
	RamWrite32A(o_ctrl, GYRO_RAM_GYOFFZ, ((GyroOffsetY << 16) & 0xFFFF0000), 0);
	RamWrite32A(o_ctrl, GYRO_ZRAM_GZOFFZ, ((GyroOffsetZ << 16) & 0xFFFF0000), 0);
}

static void SetAcclOffset(struct cam_ois_ctrl_t *o_ctrl, uint32_t AcclOffsetX, uint32_t AcclOffsetY, uint32_t AcclOffsetZ)
{
	RamWrite32A(o_ctrl, ACCLRAM_X_AC_OFFSET, ((AcclOffsetX << 16) & 0xFFFF0000), 0);
	RamWrite32A(o_ctrl, ACCLRAM_Y_AC_OFFSET, ((AcclOffsetY << 16) & 0xFFFF0000), 0);
	RamWrite32A(o_ctrl, ACCLRAM_Z_AC_OFFSET, ((AcclOffsetZ << 16) & 0xFFFF0000), 0);
}
#endif
void XuChangGetGyroOffset(struct cam_ois_ctrl_t *o_ctrl, unsigned short* GyroOffsetX, unsigned short* GyroOffsetY, unsigned short* GyroOffsetZ)
{
	uint32_t ReadValX, ReadValY, ReadValZ;
	RamRead32A(o_ctrl, GYRO_RAM_GXOFFZ, &ReadValX);
	RamRead32A(o_ctrl, GYRO_RAM_GYOFFZ, &ReadValY);
	RamRead32A(o_ctrl, GYRO_ZRAM_GZOFFZ, &ReadValZ);
	*GyroOffsetX = (unsigned short)((ReadValX >> 16) & 0x0000FFFF);
	*GyroOffsetY = (unsigned short)((ReadValY >> 16) & 0x0000FFFF);
	*GyroOffsetZ = (unsigned short)((ReadValZ >> 16) & 0x0000FFFF);
}

void XuChangGetAcclOffset(struct cam_ois_ctrl_t *o_ctrl, unsigned short* AcclOffsetX, unsigned short* AcclOffsetY, unsigned short* AcclOffsetZ)
{
	uint32_t ReadValX, ReadValY, ReadValZ;
	RamRead32A(o_ctrl, ACCLRAM_X_AC_OFFSET, &ReadValX);
	RamRead32A(o_ctrl, ACCLRAM_Y_AC_OFFSET, &ReadValY);
	RamRead32A(o_ctrl, ACCLRAM_Z_AC_OFFSET, &ReadValZ);
	*AcclOffsetX = (unsigned short)((ReadValX >> 16) & 0x0000FFFF);
	*AcclOffsetY = (unsigned short)((ReadValY >> 16) & 0x0000FFFF);
	*AcclOffsetZ = (unsigned short)((ReadValZ >> 16) & 0x0000FFFF);
}

static void MeasFil(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t MeasFilaA, MeasFilaB, MeasFilaC;
	uint32_t MeasFilbA, MeasFilbB, MeasFilbC;

	MeasFilaA = 0x7FFFFFFF;
	MeasFilaB = 0x00000000;
	MeasFilaC = 0x00000000;
	MeasFilbA = 0x7FFFFFFF;
	MeasFilbB = 0x00000000;
	MeasFilbC = 0x00000000;

	RamWrite32A(o_ctrl, 0x8388, MeasFilaA, 0);
	RamWrite32A(o_ctrl, 0x8380, MeasFilaB, 0);
	RamWrite32A(o_ctrl, 0x8384, MeasFilaC, 0);

	RamWrite32A(o_ctrl, 0x8394, MeasFilbA, 0);
	RamWrite32A(o_ctrl, 0x838C, MeasFilbB, 0);
	RamWrite32A(o_ctrl, 0x8390, MeasFilbC, 0);

	RamWrite32A(o_ctrl, 0x83A0, MeasFilaA, 0);
	RamWrite32A(o_ctrl, 0x8398, MeasFilaB, 0);
	RamWrite32A(o_ctrl, 0x839C, MeasFilaC, 0);
	RamWrite32A(o_ctrl, 0x83AC, MeasFilbA, 0);
	RamWrite32A(o_ctrl, 0x83A4, MeasFilbB, 0);
	RamWrite32A(o_ctrl, 0x83A8, MeasFilbC, 0);
}

static void MemoryClear(struct cam_ois_ctrl_t *o_ctrl, uint32_t UsSourceAddress, uint32_t UsClearSize)
{
	uint32_t UsLoopIndex;

	for (UsLoopIndex = 0; UsLoopIndex < UsClearSize;) {
		RamWrite32A(o_ctrl, UsSourceAddress, 0x00000000, 0);
		UsSourceAddress += 4;
		UsLoopIndex += 4;
	}
}
static void SetTransDataAdr(struct cam_ois_ctrl_t *o_ctrl, uint32_t UsLowAddress, uint32_t UlLowAdrBeforeTrans)
{
	UnDwdVal StTrsVal;

	if(UlLowAdrBeforeTrans < 0x00009000){
		StTrsVal.StDwdVal.UsHigVal = (uint32_t)((UlLowAdrBeforeTrans & 0x0000F000) >> 8);
		StTrsVal.StDwdVal.UsLowVal = (uint32_t)(UlLowAdrBeforeTrans & 0x00000FFF);
	}else{
		StTrsVal.UlDwdVal = UlLowAdrBeforeTrans;
	}
	RamWrite32A(o_ctrl, UsLowAddress, StTrsVal.UlDwdVal, 0);
}

static void	SetWaitTime(struct cam_ois_ctrl_t *o_ctrl, uint32_t UsWaitTime)
{
	RamWrite32A(o_ctrl, 0x0324, 0, 0);
	RamWrite32A(o_ctrl, 0x0328, (uint32_t)(ONE_MSEC_COUNT * UsWaitTime), 0);
}
static void	ClrMesFil(struct cam_ois_ctrl_t *o_ctrl)
{
	RamWrite32A(o_ctrl, 0x02D0, 0, 0);
	RamWrite32A(o_ctrl, 0x02D4, 0, 0);

	RamWrite32A(o_ctrl, 0x02D8, 0, 0);
	RamWrite32A(o_ctrl, 0x02DC, 0, 0);

	RamWrite32A(o_ctrl, 0x02E0, 0, 0);
	RamWrite32A(o_ctrl, 0x02E4, 0, 0);

	RamWrite32A(o_ctrl, 0x02E8, 0, 0);
	RamWrite32A(o_ctrl, 0x02EC, 0, 0);
}

static void MeasAddressSelection(uint8_t mode, int32_t * measadr_a, int32_t * measadr_b)
{
	if (mode == 0){
		*measadr_a = GYRO_RAM_GX_ADIDAT;
		*measadr_b = GYRO_RAM_GY_ADIDAT;
	} else if (mode == 1){
		*measadr_a = GYRO_ZRAM_GZ_ADIDAT;
		*measadr_b = ACCLRAM_Z_AC_ADIDAT;
	} else {
		*measadr_a = ACCLRAM_X_AC_ADIDAT;
		*measadr_b = ACCLRAM_Y_AC_ADIDAT;
	}
}

static void MeasureStart(struct cam_ois_ctrl_t *o_ctrl, int32_t SlMeasureParameterNum, int32_t SlMeasureParameterA, int32_t SlMeasureParameterB)
{
	MemoryClear(o_ctrl, 0x0278, sizeof(MeasureFunction_Type));
	RamWrite32A(o_ctrl, 0x0280, 0x80000000, 0);
	RamWrite32A(o_ctrl, 0x02A8, 0x80000000, 0);
	RamWrite32A(o_ctrl, 0x0284, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, 0x02AC, 0x7FFFFFFF, 0);

	SetTransDataAdr(o_ctrl, 0x02A0, (uint32_t)SlMeasureParameterA);
	SetTransDataAdr(o_ctrl, 0x02C8, (uint32_t)SlMeasureParameterB);
	RamWrite32A(o_ctrl, 0x0278, 0, 0);
	ClrMesFil(o_ctrl);
	SetWaitTime(o_ctrl, 1);
	RamWrite32A(o_ctrl, 0x027C, SlMeasureParameterNum, 0);
}
static void MeasureWait(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t SlWaitTimerSt;
	uint32_t UsTimeOut = 2000;

	do {
		RamRead32A(o_ctrl, 0x027C, &SlWaitTimerSt);
		UsTimeOut--;
	} while (SlWaitTimerSt && UsTimeOut);

}
void SetSinWavGenInt(struct cam_ois_ctrl_t *o_ctrl)
{
	RamWrite32A(o_ctrl, 0x02FC, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x0300, 0x60000000, 0);
	RamWrite32A(o_ctrl, 0x0304, 0x00000000, 0);

	RamWrite32A(o_ctrl, 0x0310, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x0314, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x0318, 0x00000000, 0);

	RamWrite32A(o_ctrl, 0x02F4, 0x00000000, 0); // Sine Wave Stop

}

#define MESOF_NUM      2048
#define GYROFFSET_H    (0x06D6 << 16)
#define GSENS          (4096 << 16)
#define GSENS_MARG     (GSENS / 4)
#define POSTURETH      (GSENS - GSENS_MARG)
#define ZG_MRGN        (1310 << 16)
#define XYG_MRGN       (1024 << 16)
uint32_t XuChangMeasGyAcOffset(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlRsltSts;
	int32_t SlMeasureParameterA, SlMeasureParameterB;
	int32_t SlMeasureParameterNum;
	UnllnVal StMeasValueA, StMeasValueB;
	int32_t SlMeasureAveValueA[3], SlMeasureAveValueB[3];
	uint8_t i;
	int32_t SlMeasureAZ = 0;

	MeasFil(o_ctrl);

	SlMeasureParameterNum = MESOF_NUM;

	for (i = 0; i < 3; i++)
	{
		MeasAddressSelection(i, &SlMeasureParameterA, &SlMeasureParameterB);
		MeasureStart(o_ctrl, SlMeasureParameterNum, SlMeasureParameterA, SlMeasureParameterB);
		MeasureWait(o_ctrl);
		RamRead32A(o_ctrl, 0x0290, &StMeasValueA.StUllnVal.UlLowVal);
		RamRead32A(o_ctrl, 0x0290 + 4, &StMeasValueA.StUllnVal.UlHigVal);
		RamRead32A(o_ctrl, 0x02B8, &StMeasValueB.StUllnVal.UlLowVal);
		RamRead32A(o_ctrl, 0x02B8 + 4, &StMeasValueB.StUllnVal.UlHigVal);

		SlMeasureAveValueA[i] = (int32_t)((INT_64)StMeasValueA.UllnValue / SlMeasureParameterNum);
		SlMeasureAveValueB[i] = (int32_t)((INT_64)StMeasValueB.UllnValue / SlMeasureParameterNum);
	}

	UlRsltSts = EXE_END;

	if ((SlMeasureAveValueB[1]) >= POSTURETH){
		SlMeasureAZ = SlMeasureAveValueB[1] - (int32_t)GSENS;
	} else if ((SlMeasureAveValueB[1]) <= -POSTURETH){
		SlMeasureAZ = SlMeasureAveValueB[1] + (int32_t)GSENS;
	} else {
		// UlRsltSts |= EXE_AZADJ;
	}

	if (abs(SlMeasureAveValueA[0]) > GYROFFSET_H)
		UlRsltSts |= EXE_GXADJ;
	if (abs(SlMeasureAveValueB[0]) > GYROFFSET_H)
		UlRsltSts |= EXE_GYADJ;
	if (abs(SlMeasureAveValueA[1]) > GYROFFSET_H)
		UlRsltSts |= EXE_GZADJ;
	// if (   (SlMeasureAveValueB[1]) < POSTURETH)
		// UlRsltSts |= EXE_AZADJ;
	// if (abs(SlMeasureAveValueA[2]) > XYG_MRGN)
		// UlRsltSts |= EXE_AXADJ;
	// if (abs(SlMeasureAveValueB[2]) > XYG_MRGN)
		// UlRsltSts |= EXE_AYADJ;
	// if (abs(GSENS  - SlMeasureAveValueB[1]) > ZG_MRGN)
		// UlRsltSts |= EXE_AZADJ;
	// if (abs(SlMeasureAZ) > ZG_MRGN)
		// UlRsltSts |= EXE_AZADJ;

	if (UlRsltSts == EXE_END) {
		RamWrite32A(o_ctrl, GYRO_RAM_GXOFFZ, SlMeasureAveValueA[0], 0);
		RamWrite32A(o_ctrl, GYRO_RAM_GYOFFZ, SlMeasureAveValueB[0], 0);
		RamWrite32A(o_ctrl, GYRO_ZRAM_GZOFFZ, SlMeasureAveValueA[1], 0);
		RamWrite32A(o_ctrl, ACCLRAM_X_AC_OFFSET, SlMeasureAveValueA[2], 0);
		RamWrite32A(o_ctrl, ACCLRAM_Y_AC_OFFSET, SlMeasureAveValueB[2], 0);
		// RamWrite32A(o_ctrl, ACCLRAM_Z_AC_OFFSET, SlMeasureAveValueB[1] - (int32_t)GSENS, 0);
		RamWrite32A(o_ctrl, ACCLRAM_Z_AC_OFFSET, SlMeasureAZ, 0);

		RamWrite32A(o_ctrl, 0x01D8, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x01FC, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x0378, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x019C, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x01C4, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03C0 + 8, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03F0 + 8, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x0420 + 8, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03C0 + 12, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03F0 + 12, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x0420 + 12, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03C0 + 16, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03F0 + 16, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x0420 + 16, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03C0 + 20, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x03F0 + 20, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0x0420 + 20, 0x00000000, 0);
	}
	return(UlRsltSts);
}

const uint8_t PACT3Tbl[] = { 0x46, 0xB9 }; /* [Xuchang] */

uint8_t XuChangSetAngleCorrection(struct cam_ois_ctrl_t *o_ctrl, INT_8 DegreeGap, uint8_t SelectAct, uint8_t Arrangement )
{
	//double OffsetAngle = 0.0;
	int32_t Slgx45x = 0, Slgx45y = 0;
	int32_t Slgy45y = 0, Slgy45x = 0;

	uint8_t	UcCnvF = 0;
	uint8_t	UcFlag = 0;

	if( ( DegreeGap > 180) || ( DegreeGap < -180 ) ) return ( 1 );
	if( Arrangement >= 2 ) return ( 1 );

/************************************************************************/
/* Gyro angle correction */
/************************************************************************/
	switch(SelectAct) {
		// case 0x01 :
		// case 0x02 :
		// case 0x03 :
		// case 0x05 :
		// case 0x06 :
		// case 0x07 :
		// case 0x08 :
		// case 0x09 :
		default :
			// OffsetAngle = (double)( DegreeGap ) * 3.141592653589793238 / 180 ;
			// UcCnvF = PACT1Tbl[ Arrangement ];
			// UcCnvF = PACT2Tbl[ Arrangement ];
			UcCnvF = PACT3Tbl[ Arrangement ];
			break;
	}
	SetGyroCoef(o_ctrl, UcCnvF);
	SetAccelCoef(o_ctrl, UcCnvF);

	//***********************************************//
	// Gyro & Accel rotation correction
	//***********************************************//
	UcFlag = (UcCnvF & 0xF0);
	#if 0
	if (((Arrangement==0) && ((UcFlag == 0x00) || (UcFlag == 0x60) || (UcFlag == 0xA0) || (UcFlag == 0xC0)))
		|| ((Arrangement==1) && ((UcFlag == 0xF0) || (UcFlag == 0x90) || (UcFlag == 0x50) || (UcFlag == 0x30)))){
		Slgx45x = (int32_t)(cos(OffsetAngle)*2147483647.0);
		Slgx45y = (int32_t)(sin(OffsetAngle)*2147483647.0);
		Slgy45y = (int32_t)(cos(OffsetAngle)*2147483647.0);
		Slgy45x = (int32_t)(-sin(OffsetAngle)*2147483647.0);
	} else {
		Slgx45x = (int32_t)(cos(OffsetAngle)*2147483647.0);
		Slgx45y = (int32_t)(-sin(OffsetAngle)*2147483647.0);
		Slgy45y = (int32_t)(cos(OffsetAngle)*2147483647.0);
		Slgy45x = (int32_t)(sin(OffsetAngle)*2147483647.0);
	}
	#endif

	// if (DegreeGap == -90) {
		Slgx45x = 0;
		Slgx45y = 0x7fffffff;
		Slgy45y = 0;
		Slgy45x = 0x80000001;
	// }
	#if 0
	if (DegreeGap == 90) {
		Slgx45x = 0;
		Slgx45y = 0x80000001;
		Slgy45y = 0;
		Slgy45x = 0x7fffffff;
	}
	if (DegreeGap == 0) {
		Slgx45x = 0x7fffffff;
		Slgx45y = 0;
		Slgy45y = 0x7fffffff;
		Slgy45x = 0;
	}
	// if (DegreeGap == 180) {
		Slgx45x = 0x80000001;
		Slgx45y = 0;
		Slgy45y = 0x80000001;
		Slgy45x = 0;
	// }
#endif

	CAM_DBG(CAM_OIS, "XuChangSetAngleCorrection Slgx45x 0x%x, Slgx45y 0x%x, Slgx45x 0x%x, Slgx45y 0x%x", Slgx45x, Slgx45y, Slgy45y, Slgy45x);
	RamWrite32A(o_ctrl, 0x8270, (uint32_t)Slgx45x, 0);
	RamWrite32A(o_ctrl, 0x8274, (uint32_t)Slgx45y, 0);
	RamWrite32A(o_ctrl, 0x82D0, (uint32_t)Slgy45y, 0);
	RamWrite32A(o_ctrl, 0x82D4, (uint32_t)Slgy45x, 0);
	RamWrite32A(o_ctrl, 0x8640, (uint32_t)Slgx45x, 0);
	RamWrite32A(o_ctrl, 0x8644, (uint32_t)Slgx45y, 0);
	RamWrite32A(o_ctrl, 0x8648, (uint32_t)Slgy45y, 0);
	RamWrite32A(o_ctrl, 0x864C, (uint32_t)Slgy45x, 0);

	return (0);
}

static void SetGyroCoef(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcCnvF )
{
	int32_t Slgxx = 0, Slgxy = 0;
	int32_t Slgyy = 0, Slgyx = 0;
	int32_t Slgzp = 0;
	/************************************************/
	/* signal convet */
	/************************************************/
	switch( UcCnvF & 0xE0 ){
		/* HX <== GX , HY <== GY */
	case 0x00:
		Slgxx = 0x7FFFFFFF; Slgxy = 0x00000000; Slgyy = 0x7FFFFFFF; Slgyx = 0x00000000;	break; // HX<==GX(NEG), HY<==GY(NEG)
	case 0x20:
		Slgxx = 0x7FFFFFFF; Slgxy = 0x00000000;	Slgyy = 0x80000001; Slgyx = 0x00000000; break; //HX<==GX(NEG), HY<==GY(POS)
	case 0x40:
		Slgxx = 0x80000001; Slgxy = 0x00000000;	Slgyy = 0x7FFFFFFF; Slgyx = 0x00000000; break; //HX<==GX(POS), HY<==GY(NEG)
	case 0x60:
		Slgxx = 0x80000001; Slgxy = 0x00000000;	Slgyy = 0x80000001; Slgyx = 0x00000000; break; //HX<==GX(POS), HY<==GY(POS)
		/* HX <== GY , HY <== GX */
	case 0x80:
		Slgxx = 0x00000000; Slgxy = 0x7FFFFFFF;	Slgyy = 0x00000000; Slgyx = 0x7FFFFFFF; break; //HX<==GY(NEG), HY<==GX(NEG)
	case 0xA0:
		Slgxx = 0x00000000; Slgxy = 0x7FFFFFFF;	Slgyy = 0x00000000; Slgyx = 0x80000001; break; //HX<==GY(NEG), HY<==GX(POS)
	case 0xC0:
		Slgxx = 0x00000000; Slgxy = 0x80000001;	Slgyy = 0x00000000; Slgyx = 0x7FFFFFFF; break; //HX<==GY(POS), HY<==GX(NEG)
	case 0xE0:
		Slgxx = 0x00000000; Slgxy = 0x80000001;	Slgyy = 0x00000000; Slgyx = 0x80000001; break; //HX<==GY(NEG), HY<==GX(NEG)
	}
	switch( UcCnvF & 0x10 ){
	case 0x00:
		Slgzp = 0x7FFFFFFF;
		break;//GZ(POS)
	case 0x10:
		Slgzp = 0x80000001;
		break;//GZ(NEG)
	}
	RamWrite32A(o_ctrl, 0x865C, (uint32_t)Slgxx, 0);
	RamWrite32A(o_ctrl, 0x8660, (uint32_t)Slgxy, 0);
	RamWrite32A(o_ctrl, 0x8664, (uint32_t)Slgyy, 0);
	RamWrite32A(o_ctrl, 0x8668, (uint32_t)Slgyx, 0);
	RamWrite32A(o_ctrl, 0x866C, (uint32_t)Slgzp, 0);
}

static void SetAccelCoef(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcCnvF )
{
	int32_t Slaxx = 0, Slaxy = 0;
	int32_t Slayy = 0, Slayx = 0;
	int32_t Slazp = 0;

	switch( UcCnvF & 0x0E ){
		/* HX <== AX , HY <== AY */
	case 0x00:
		Slaxx = 0x7FFFFFFF; Slaxy = 0x00000000; Slayy = 0x7FFFFFFF; Slayx = 0x00000000;break; //HX<==AX(NEG), HY<==AY(NEG)
	case 0x02:
		Slaxx = 0x7FFFFFFF; Slaxy = 0x00000000; Slayy = 0x80000001; Slayx = 0x00000000;break; //HX<==AX(NEG), HY<==AY(POS)
	case 0x04:
		Slaxx = 0x80000001; Slaxy = 0x00000000; Slayy = 0x7FFFFFFF; Slayx = 0x00000000;break; //HX<==AX(POS), HY<==AY(NEG)
	case 0x06:
		Slaxx = 0x80000001; Slaxy = 0x00000000; Slayy = 0x80000001; Slayx = 0x00000000;break; //HX<==AX(POS), HY<==AY(POS)
		/* HX <== AY , HY <== AX */
	case 0x08:
		Slaxx = 0x00000000; Slaxy = 0x7FFFFFFF; Slayy = 0x00000000; Slayx = 0x7FFFFFFF;break; //HX<==AY(NEG), HY<==AX(NEG)
	case 0x0A:
		Slaxx = 0x00000000; Slaxy = 0x7FFFFFFF; Slayy = 0x00000000; Slayx = 0x80000001;break; //HX<==AY(NEG), HY<==AX(POS)
	case 0x0C:
		Slaxx = 0x00000000; Slaxy = 0x80000001; Slayy = 0x00000000; Slayx = 0x7FFFFFFF;break; //HX<==AY(POS), HY<==AX(NEG)
	case 0x0E:
		Slaxx = 0x00000000; Slaxy = 0x80000001; Slayy = 0x00000000; Slayx = 0x80000001;break; //HX<==AY(NEG), HY<==AX(NEG)
	}
	switch( UcCnvF & 0x01 ){
	case 0x00:
		Slazp = 0x7FFFFFFF;
		break; //AZ(POS)
	case 0x01:
		Slazp = 0x80000001;
		break; //AZ(NEG)
	}
	RamWrite32A(o_ctrl, 0x8670, (uint32_t)Slaxx, 0);
	RamWrite32A(o_ctrl, 0x8674, (uint32_t)Slaxy, 0);
	RamWrite32A(o_ctrl, 0x8678, (uint32_t)Slayy, 0);
	RamWrite32A(o_ctrl, 0x867C, (uint32_t)Slayx, 0);
	RamWrite32A(o_ctrl, 0x8680, (uint32_t)Slazp, 0);
}

uint8_t XuChangRdStatus(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcStBitChk )
{
	uint32_t UlReadVal;

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

void OisEna(struct cam_ois_ctrl_t *o_ctrl) // OIS ( SMA , VCM ) = ( OFF, ON )
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF012 , 0x00000001, 0) ;
	while ( UcStRd && (UlStCnt++ < CNT050MS )) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void OisDis(struct cam_ois_ctrl_t *o_ctrl) // OIS ( SMA , VCM ) = ( OFF, OFF )
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF012, 0x00000000, 0);
	while ( UcStRd && ( UlStCnt++ < CNT050MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void SetPanTiltMode(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcPnTmod )
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	switch ( UcPnTmod ) {
		case 0 :
			RamWrite32A(o_ctrl, 0xF011, 0x00000000, 0);
			break;
		case 1 :
			RamWrite32A(o_ctrl, 0xF011, 0x00000001, 0);
			break;
	}

	while( UcStRd && ( UlStCnt++ < CNT050MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void SscEna(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF01C, 0x00000001, 0);
	while ( UcStRd && ( UlStCnt++ < CNT050MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void SscDis(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF01C, 0x00000000, 0);
	while ( UcStRd && ( UlStCnt++ < CNT050MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

#define GEA_NUM     512
#define GEA_DIF_HIG 0x0083
#define GEA_DIF_LOW 0x0001

uint8_t	RunGea(struct cam_ois_ctrl_t *o_ctrl)
{
	UnllnVal StMeasValueA , StMeasValueB;
	int32_t SlMeasureParameterA , SlMeasureParameterB ;
	uint8_t UcRst, UcCnt, UcXLowCnt, UcYLowCnt, UcXHigCnt, UcYHigCnt;
	uint32_t UsGxoVal[10], UsGyoVal[10], UsDif;
	int32_t SlMeasureParameterNum , SlMeasureAveValueA , SlMeasureAveValueB;

	UcRst = EXE_END;
	UcXLowCnt = UcYLowCnt = UcXHigCnt = UcYHigCnt = 0;

	RamWrite32A(o_ctrl, 0x8388, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, 0x8380, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x8384, 0x00000000, 0);

	RamWrite32A(o_ctrl, 0x8394, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, 0x838C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x8390, 0x00000000, 0);

	RamWrite32A(o_ctrl, 0x83A0, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, 0x8398, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x839C, 0x00000000, 0);

	RamWrite32A(o_ctrl, 0x83AC, 0x7FFFFFFF, 0);
	RamWrite32A(o_ctrl, 0x83A4, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0x83A8, 0x00000000, 0);

	for( UcCnt = 0 ; UcCnt < 10 ; UcCnt++ )
	{
		SlMeasureParameterNum = GEA_NUM;
		SlMeasureParameterA = GYRO_RAM_GX_ADIDAT;
		SlMeasureParameterB = GYRO_RAM_GY_ADIDAT;

		MeasureStart(o_ctrl, SlMeasureParameterNum, SlMeasureParameterA, SlMeasureParameterB);

		MeasureWait(o_ctrl);

		RamRead32A(o_ctrl, 0x0290, &StMeasValueA.StUllnVal.UlLowVal);
		RamRead32A(o_ctrl, 0x0290 + 4, &StMeasValueA.StUllnVal.UlHigVal);
		RamRead32A(o_ctrl, 0x02B8, &StMeasValueB.StUllnVal.UlLowVal);
		RamRead32A(o_ctrl, 0x02B8 + 4, &StMeasValueB.StUllnVal.UlHigVal);
		SlMeasureAveValueA = (int32_t)( (INT_64)StMeasValueA.UllnValue / SlMeasureParameterNum);
		SlMeasureAveValueB = (int32_t)( (INT_64)StMeasValueB.UllnValue / SlMeasureParameterNum);
		UsGxoVal[UcCnt] = (uint32_t)( SlMeasureAveValueA >> 16);
		UsGyoVal[UcCnt] = (uint32_t)( SlMeasureAveValueB >> 16);

		if( UcCnt > 0 )
		{
			if ( (INT_16)UsGxoVal[0] > (INT_16)UsGxoVal[UcCnt] ) {
				UsDif = (uint32_t)((INT_16)UsGxoVal[0] - (INT_16)UsGxoVal[UcCnt]);
			} else {
				UsDif = (uint32_t)((INT_16)UsGxoVal[UcCnt] - (INT_16)UsGxoVal[0]);
			}

			if( UsDif > GEA_DIF_HIG ) {
				UcXHigCnt ++;
			}
			if( UsDif < GEA_DIF_LOW ) {
				UcXLowCnt ++;
			}

			if ( (INT_16)UsGyoVal[0] > (INT_16)UsGyoVal[UcCnt] ) {
				UsDif = (uint32_t)((INT_16)UsGyoVal[0] - (INT_16)UsGyoVal[UcCnt]) ;
			} else {
				UsDif = (uint32_t)((INT_16)UsGyoVal[UcCnt] - (INT_16)UsGyoVal[0]) ;
			}

			if( UsDif > GEA_DIF_HIG ) {
				UcYHigCnt ++ ;
			}
			if( UsDif < GEA_DIF_LOW ) {
				UcYLowCnt ++ ;
			}
		}
	}

	if( UcXHigCnt >= 1 ) {
		UcRst = UcRst | EXE_GXABOVE ;
	}
	if( UcXLowCnt > 8 ) {
		UcRst = UcRst | EXE_GXBELOW ;
	}

	if( UcYHigCnt >= 1 ) {
		UcRst = UcRst | EXE_GYABOVE ;
	}
	if( UcYLowCnt > 8 ) {
		UcRst = UcRst | EXE_GYBELOW ;
	}

	return( UcRst ) ;
}


void PreparationForPowerOff(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlReadVa;

	RamRead32A(o_ctrl, 0x8004, &UlReadVa);
	if((uint8_t)UlReadVa == 0x02){
		RamWrite32A(o_ctrl, CMD_GYRO_WR_ACCS, 0x00027000, 0);
	}
}

void	SrvOn(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t	UcStRd = 1;
	uint32_t	UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF010, 0x00000003, 0);

	while ( UcStRd && ( UlStCnt++ < CNT200MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void SrvOff(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF010, 0x00000000, 0);

	while( UcStRd && ( UlStCnt++ < CNT050MS)) {
		UcStRd = XuChangRdStatus(o_ctrl, 1);
	}
}

void VcmStandby(struct cam_ois_ctrl_t *o_ctrl)
{
	IOWrite32A(o_ctrl, 0xD00078, 0x00000000, 0);
	IOWrite32A(o_ctrl, 0xD00074, 0x00000010, 0);
	IOWrite32A(o_ctrl, 0xD00004, 0x00000005, 0);
}

void VcmActive(struct cam_ois_ctrl_t *o_ctrl)
{
	IOWrite32A(o_ctrl, 0xD00004, 0x00000007, 0);
	IOWrite32A(o_ctrl, 0xD00074, 0x00000000, 0);
	IOWrite32A(o_ctrl, 0xD00078, 0x0000033E, 0);
}

//********************************************************************************
// Function Name 	: WrGyroGainData
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Flash write gyro gain data
// History			: First edition
//********************************************************************************
uint8_t XuChangWrGyroGainData( struct cam_ois_ctrl_t *o_ctrl, uint8_t UcMode )
{
	uint32_t UlMAT0[32];
	uint32_t UlReadGxzoom , UlReadGyzoom;
	uint8_t ans = 0, i;
	uint16_t UsCkVal,UsCkVal_Bk ;

	CAM_DBG(CAM_OIS, "[WrGyroGainData]BEGIN WrGyroGainData");
	/* Back up ******************************************************/
	ans =FlashMultiRead( o_ctrl,INF_MAT0, 0, UlMAT0, 32 ); // check sum
	if( ans ) return( 1 );

	/* Erase   ******************************************************/
	ans = FlashBlockErase(o_ctrl,INF_MAT0, 0);// all erase
	if (ans != 0)
		return(2); // Unlock Code Set

	for (i = 0; i < 32; i++){
		CAM_DBG(CAM_OIS, "[WrGyroGainData] the origin date in flash [ %d ] = %08x\n",i, UlMAT0[i]);
	}

	/* modify   *****************************************************/
	if( UcMode ){
		RamRead32A( o_ctrl, GyroFilterTableX_gxzoom , &UlReadGxzoom );
		RamRead32A( o_ctrl, GyroFilterTableY_gyzoom , &UlReadGyzoom );

		UlMAT0[CALIBRATION_STATUS] &= ~(GYRO_GAIN_FLG);
		UlMAT0[GYRO_GAIN_X] = UlReadGxzoom;
		UlMAT0[GYRO_GAIN_Y] = UlReadGyzoom;
	}else{
		UlMAT0[CALIBRATION_STATUS] |= GYRO_GAIN_FLG;
		UlMAT0[GYRO_GAIN_X] = 0x3FFFFFFF;
		UlMAT0[GYRO_GAIN_Y] = 0x3FFFFFFF;
	}
	/* calcurate check sum ******************************************/
	UsCkVal = 0;
	for( i=0; i < 31; i++ ){
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>0);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>8);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>16);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>24);
		CAM_DBG(CAM_OIS, "[WrGyroGainData] calcurate check sum UlMAT0 [ %d ] = %08x\n",i, UlMAT0[i] );
	}
	// Remainder
	UsCkVal +=  (uint8_t)(UlMAT0[i]>>0);
	UsCkVal +=  (uint8_t)(UlMAT0[i]>>8);
	UlMAT0[MAT0_CKSM] = ((uint32_t)UsCkVal<<16) | (UlMAT0[MAT0_CKSM] & 0x0000FFFF);

	CAM_DBG(CAM_OIS, "[WrGyroGainData] calcurate check sum UlMAT0 [ %d ] = %08x\n",i, UlMAT0[i] );
	/* update ******************************************************/
	ans = FlashBlockWrite( o_ctrl,INF_MAT0 , 0 , UlMAT0 );
	if( ans != 0 ) return( 3 ); // Unlock Code Set
	ans = FlashBlockWrite( o_ctrl,INF_MAT0 , (uint32_t)0x10 , &UlMAT0[0x10]);
	if( ans != 0 ) return( 3 ) ; // Unlock Code Set
	/* Verify ******************************************************/
	UsCkVal_Bk = UsCkVal;
	ans =FlashMultiRead( o_ctrl,INF_MAT0, 0, UlMAT0, 32); // check sum
	if( ans ) return( 4 );

	UsCkVal = 0;
	for( i=0; i < 31; i++ ){
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>0);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>8);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>16);
		UsCkVal +=  (uint8_t)(UlMAT0[i]>>24);
	}
	// Remainder
	UsCkVal +=  (uint8_t)(UlMAT0[i]>>0);
	UsCkVal +=  (uint8_t)(UlMAT0[i]>>8);
	CAM_DBG(CAM_OIS, "[WrGyroGainData][RVAL]:[BVal]=[%04x]:[%04x]\n",UsCkVal, UsCkVal_Bk);

	if(UsCkVal != UsCkVal_Bk)
		return (5);

	CAM_DBG(CAM_OIS, "[WrGyroGainData] compelete ");
	return(0);
}

//********************************************************************************
// Function Name 	: CheckDrvOffAdj
// Retun Value		: Driver Offset Re-adjustment
// Argment Value	: NON
// Explanation		: Driver Offset Re-adjustment
// History		: First edition
//********************************************************************************
uint32_t CheckDrvOffAdj(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t UlReadDrvOffx, UlReadDrvOffy, UlReadDrvOffaf;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	IOWrite32A(o_ctrl, FLASHROM_ACSCNT, 2, 0); //3word
	IOWrite32A(o_ctrl, FLASHROM_FLA_ADR, ((uint32_t)INF_MAT1 << 16) | 0xD, 0);
	IOWrite32A(o_ctrl, FLASHROM_FLAMODE, 0x00000000, 0);
	IOWrite32A(o_ctrl, FLASHROM_CMD, 0x00000001, 0);

	IORead32A(o_ctrl, FLASHROM_FLA_RDAT, &UlReadDrvOffaf); // #13
	IORead32A(o_ctrl, FLASHROM_FLA_RDAT, &UlReadDrvOffx); // #14
	IORead32A(o_ctrl, FLASHROM_FLA_RDAT, &UlReadDrvOffy); // #15

	IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
	if(((UlReadDrvOffx & 0x000FF00) == 0x0000100) ||
		((UlReadDrvOffy & 0x000FF00) == 0x0000100) ||
		((UlReadDrvOffaf & 0x000FF00) == 0x0000800)) { //error
		return( 0x93 );
	}

	if( ((UlReadDrvOffx & 0x0000080) == 0) &&
		((UlReadDrvOffy & 0x00000080) ==0) &&
		((UlReadDrvOffaf & 0x00008000) ==0)) {
		return( 0 ); // 0 : Uppdated
	}

	return( 1 ); // 1 : Still not.
}

uint32_t DrvOffAdj(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t ans = 0;
	uint32_t UlReadVal;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	ans = CheckDrvOffAdj(o_ctrl);
	CAM_DBG(CAM_OIS, "[OISDEBUG] CheckDrvOffAdj result %d", ans);
	if (ans == 1) {
		CAM_DBG(CAM_OIS, "[OISDEBUG] ready to update prog fw");
	 	ans = CoreResetwithoutMC128(o_ctrl); // Start up to boot exection
	 	if(ans != 0)
			return( ans );
	 	ans = PmemUpdate128(o_ctrl, 0);
		if(ans != 0)
			return( ans );

		IOWrite32A(o_ctrl, FLASHROM_FLAMODE, 0x00000000, 0);
		RamWrite32A(o_ctrl, 0xF001,  0x00000000, 1000);
		IOWrite32A(o_ctrl, 0xE07CCC, 0x0000C5AD, 0); // additional unlock for INFO
		IOWrite32A(o_ctrl, 0xE07CCC, 0x0000ACD5, 10000); // UNLK_CODE3(E0_7CCCh) = 0000_ACD5h

		IOWrite32A(o_ctrl, FLASHROM_FLAMODE , 0x00000002, 0);
		IOWrite32A(o_ctrl, SYSDSP_REMAP, 0x00001000, 15000);// CORE_RST[12], MC_IGNORE2[10] = 1 PRAMSEL[7:6]=01b
		IORead32A(o_ctrl, ROMINFO, &UlReadVal);
		if(UlReadVal != 0x08)
			return 0x90;

		ans = CheckDrvOffAdj(o_ctrl);
	}
	return ans;
}

uint8_t MatVerify(struct cam_ois_ctrl_t *o_ctrl, uint32_t FwChecksum, uint32_t FwChecksumSize)
{
	uint32_t UlReadVal, UlCnt ;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00A, 0x00000000, 0);
	RamWrite32A(o_ctrl, 0xF00D, FwChecksumSize, 0);
	RamWrite32A(o_ctrl, 0xF00C, 0x00000100, 0);
	usleep_range(6000, 6001);
	UlCnt = 0;

	do {
		RamRead32A(o_ctrl, 0xF00C, &UlReadVal);
		if (UlCnt++ > 10) {
			IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
			return (0x51); // check sum excute ng
		}
		usleep_range(1000, 1001);
	} while (UlReadVal != 0);

	RamRead32A(o_ctrl, 0xF00D, &UlReadVal);

	if (UlReadVal != FwChecksum) {
		IOWrite32A(o_ctrl, 0xE0701C , 0x00000002, 0);
		return(0x52);
	}


	CAM_DBG(CAM_OIS, "UserMat Verify OK");
	// CoreReset
	// CORE_RST[12], MC_IGNORE2[10] = 1 PRAMSEL[7:6]=01b
	IOWrite32A(o_ctrl, SYSDSP_REMAP, 0x00001000, 0);
	usleep_range(15000, 15001);
	IORead32A(o_ctrl, ROMINFO,(uint32_t *)&UlReadVal);
	CAM_DBG(CAM_OIS, "UlReadVal = %x", UlReadVal);
	if (UlReadVal != 0x0A)
		return( 0x53 );

	CAM_DBG(CAM_OIS, "Remap OK");
	return 0;
}

void XuChangSetActiveMode(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t UcStRd = 1;
	uint32_t UlStCnt = 0;

	RamWrite32A(o_ctrl, 0xF019, 0x00000000, 0);
	while (UcStRd && (UlStCnt++ < CNT050MS))
		UcStRd = XuChangRdStatus(o_ctrl, 1);
}


//********************************************************************************
// Function Name 	: PageWrite128
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LC898128用 Info Mat page Write
// History			: First edition 						2020.08.28
//********************************************************************************
uint8_t PageWrite128(struct cam_ois_ctrl_t *o_ctrl, uint32_t UlAddr, uint32_t *pData )
{
	uint8_t  UcSndDat = FAILURE;
	uint32_t UlCnt;
	uint32_t UlDataVal;
	int j;

	//***** Page Write *****
	IOWrite32A(o_ctrl, 0xE0700C, UlAddr, 0 );
	IOWrite32A(o_ctrl, 0xE07010, 0x00000002, 0 );

	for(j=0 ; j<16 ; j++) {
		IOWrite32A(o_ctrl, 0xE07004, (*pData++), 0 );
	}

	UlCnt = 0;
	do{
		IORead32A(o_ctrl, 0xE07018, &UlDataVal );
		if( (UlDataVal & 0x00000080) == 0 ) {
			UcSndDat = SUCCESS;
			break;
		}
		usleep_range(1000, 1001);
	}while ( UlCnt++ < 10 );

	if( UcSndDat == SUCCESS ) {
		IOWrite32A(o_ctrl, 0xE07010, 0x00000008, 0 );

		UcSndDat = FAILURE;
		UlCnt = 0;
		do{
			IORead32A(o_ctrl, 0xE07018, &UlDataVal );
			if( (UlDataVal & 0x00000080) == 0 ) {
				UcSndDat = SUCCESS;
				break;
			}
			usleep_range(1000, 1001);
		}while ( UlCnt++ < 10 );
	} else {
		UcSndDat = FAILURE;
	}

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: UserMatPagekWrite128
// Retun Value		: NON
// Argment Value	: uint32_t Addr    Flash Memory Physical Address
//                  : uint32_t *pData  Data pointer
//                  : INT_32 Size     Pgae Size
// Explanation		: LC898128用 User Mat Page Write
// History			: First edition 						2020.08.28
//********************************************************************************
uint8_t UserMatPagekWrite128(struct cam_ois_ctrl_t *o_ctrl, uint32_t Addr, uint32_t *pData, int32_t Size )
{
	uint8_t  UcSndDat = FAILURE;
	uint32_t UlAddrVal;
	int i;

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000000, 0 );

	if( UnlockCodeSet(o_ctrl) == SUCCESS ) {
		WritePermission(o_ctrl);
		AddtionalUnlockCodeSet(o_ctrl);

		UlAddrVal = Addr;
		for(i=0 ; i<Size ; i++) {
			UcSndDat = PageWrite128(o_ctrl, UlAddrVal, pData );
			if( UcSndDat == FAILURE ) break;
			UlAddrVal = UlAddrVal + 0x00000010;
			pData = pData + 16;
		}

		if( UcSndDat == SUCCESS ) {
			if( UnlockCodeClear(o_ctrl) == FAILURE ) {
				UcSndDat = -4;
			}
		} else {
			UcSndDat = -3;
		}
	} else {
		UcSndDat = -1;
	}

	IOWrite32A(o_ctrl, 0xE0701C, 0x00000002, 0 );

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: CrossTalkCoeffDataSave
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Cross Talk Coefficent Save
// History			: First edition 						2020.08.28
//********************************************************************************
uint8_t CrossTalkCoeffDataSave(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t  UcSndDat = SUCCESS;
	uint8_t  UcWrAddr;
	UINT_16 UsAddr;
	uint32_t UlTemp;
	uint32_t UlBuffer[256];
	INT_64  UllCheckSum;
	int i;

	//clear Buffer
	UllCheckSum = 0;
	for(i=0 ; i<256 ; i++) UlBuffer[i] = 0x00000000;

	//read corsstalk correct coefficent from LC898129
	UcWrAddr = 0;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000707;
	UlBuffer[UcWrAddr++] = 0x00000002;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000000;

	UsAddr = 0x06A8;
	for(i=0 ; i<118 ; i++) {
		RamRead32A(o_ctrl, UsAddr, &UlTemp );
		UllCheckSum += (INT_64)UlTemp;
		UlBuffer[UcWrAddr++] = UlTemp;
		UsAddr += 0x0004;
	}

	UlBuffer[7] = (uint32_t)UllCheckSum;
	UcSndDat = UserMatPagekWrite128(o_ctrl, 0x00002400, (uint32_t *)&UlBuffer, 16 ); // Write User Mat

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: LinearityCoeffDataSave
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: LinearityC Coefficent Save
// History			: First edition 						2020.08.28
//********************************************************************************
uint8_t LinearityCoeffDataSave(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t  UcSndDat = SUCCESS;
	uint8_t  UcWrAddr;
	UINT_16 UsAddr;
	uint32_t UlTemp;
	uint32_t UlBuffer[256];
	INT_64  UllCheckSum;
	int i;

	//clear Buffer
	UllCheckSum = 0;
	for(i=0 ; i<256 ; i++) UlBuffer[i] = 0x00000000;

	//read Linearity correct coefficent from LC898129
	UcWrAddr = 0;
	UlBuffer[UcWrAddr++] = 0x00000000;
	UlBuffer[UcWrAddr++] = 0x00000707;
	UlBuffer[UcWrAddr++] = 0x00000002;
	UlBuffer[UcWrAddr++] = 0x0000001E;
	UlBuffer[UcWrAddr++] = 0x0000001E;
	UlBuffer[UcWrAddr++] = 0x0000001E;
	UlBuffer[UcWrAddr++] = 0x0000001E;
	UlBuffer[UcWrAddr++] = 0x00000000;

	UsAddr = 0x0880;
	for(i=0 ; i<116 ; i++) {
		RamRead32A(o_ctrl, UsAddr, &UlTemp );
		UllCheckSum += (INT_64)UlTemp;
		UlBuffer[UcWrAddr++] = UlTemp;
		UsAddr += 0x0004;
	}

	UlBuffer[7] = (uint32_t)UllCheckSum;
	UcSndDat = UserMatPagekWrite128(o_ctrl, 0x00002500, (uint32_t *)&UlBuffer, 16 );

	return UcSndDat;
}

//********************************************************************************
// Function Name 	: RecoveryCorrectCoeffDataSave
// Retun Value		: NON
// Argment Value	: NON
// Explanation		: Correct Coefficent Area Save
// History			: First edition 	2021.05.13
//********************************************************************************
uint8_t RecoveryCorrectCoeffDataSave(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t  UcSndDat = SUCCESS;						// 0x01=FAILURE, 0x00=SUCCESS

	UcSndDat = CrossTalkCoeffDataSave(o_ctrl);	// save CrossTalk Coeeficient
	if( UcSndDat != SUCCESS ) return UcSndDat;

	UcSndDat = LinearityCoeffDataSave(o_ctrl);	// save Linearity Coeeficient
	return UcSndDat;								// Set Data To Send Buffer
}

//********************************************************************************
// Function Name    : WrGyroOffsetData
// Retun Value      : NON
// Argment Value    : NON
// Explanation      : Flash write Temperature Offset
// History          : First edition  2021.05.06
//********************************************************************************
uint8_t WrGyroOffsetData(struct cam_ois_ctrl_t *o_ctrl, uint8_t UcMode)
{
	uint32_t UlMAT1[32];
	uint32_t UlReadGxOffset, UlReadGyOffset, UlReadGzOffset;
	uint32_t UlReadAxOffset, UlReadAyOffset, UlReadAzOffset;
	uint8_t ans = 0, i;
	uint16_t UsCkVal,UsCkVal_Bk ;

TRACE( "WrGyroOffsetData : Mode = %d\n", UcMode);
	/* Back up ******************************************************/
	ans = FlashMultiRead(o_ctrl, INF_MAT1, 0, UlMAT1, 32 ); // check sum
	if( ans )
		return 1 ;

	/* Erase   ******************************************************/
	ans = FlashBlockErase(o_ctrl, INF_MAT1 , 0 ); // all erase
	if( ans != 0 )
		return 2 ; // Unlock Code Set

	/* modify   *****************************************************/
	if( UcMode ){ // write
		RamRead32A(o_ctrl, GYRO_RAM_GXOFFZ , &UlReadGxOffset ) ;
		RamRead32A(o_ctrl, GYRO_RAM_GYOFFZ , &UlReadGyOffset ) ;
		RamRead32A(o_ctrl, GYRO_ZRAM_GZOFFZ , &UlReadGzOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_X_AC_OFFSET , &UlReadAxOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_Y_AC_OFFSET , &UlReadAyOffset ) ;
		RamRead32A(o_ctrl, ACCLRAM_Z_AC_OFFSET , &UlReadAzOffset ) ;

		//UlMAT0[CALIBRATION_STATUS] &= ~( GYRO_OFFSET_FLG );
		UlMAT1[G_OFFSET_XY] = (UlReadGxOffset&0xFFFF0000) | ((UlReadGyOffset&0xFFFF0000)>>16);
		UlMAT1[G_OFFSET_Z_AX] = (UlReadGzOffset&0xFFFF0000) | ((UlReadAxOffset&0xFFFF0000)>>16);
		UlMAT1[A_OFFSET_YZ] = (UlReadAyOffset&0xFFFF0000) | ((UlReadAzOffset&0xFFFF0000)>>16);
	}else{
		UlMAT1[G_OFFSET_XY] = 0xFFFFFFFF;
		UlMAT1[G_OFFSET_Z_AX] = 0xFFFFFFFF;
		UlMAT1[A_OFFSET_YZ] = 0xFFFFFFFF;
	}
	/* calcurate check sum ******************************************/
	UsCkVal = 0;
	for( i=0; i < 12; i++ ){
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>0);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>8);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>16);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>24);
TRACE( "UlMAT1[ %d ] = %08x\n", i, UlMAT1[i] );
	}
	// Remainder
	UsCkVal +=  (uint8_t)(UlMAT1[i]>>0);
	UsCkVal +=  (uint8_t)(UlMAT1[i]>>8);
	UlMAT1[MAT1_CKSM] = ((uint32_t)UsCkVal<<16) | ( UlMAT1[MAT1_CKSM] & 0x0000FFFF);
TRACE( "UlMAT1[ %d ] = %08x\n", i, UlMAT1[i] );

	/* update ******************************************************/
	ans = FlashBlockWrite(o_ctrl, INF_MAT1 , 0 , UlMAT1 );
	if( ans != 0 )
		return 3; // Unlock Code Set
	ans = FlashBlockWrite(o_ctrl, INF_MAT1 , (uint32_t)0x10 , &UlMAT1[0x10] );
	if( ans != 0 )
		return 3; // Unlock Code Set

	/* Verify ******************************************************/
	UsCkVal_Bk = UsCkVal;
	ans =FlashMultiRead(o_ctrl, INF_MAT1, 0, UlMAT1, 32 ); // check sum
	if( ans )
		return 4 ;

	UsCkVal = 0;
	for( i=0; i < 12; i++ ){
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>0);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>8);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>16);
		UsCkVal +=  (uint8_t)(UlMAT1[i]>>24);
	}
	// Remainder
	UsCkVal +=  (uint8_t)(UlMAT1[i]>>0);
	UsCkVal +=  (uint8_t)(UlMAT1[i]>>8);

TRACE( "[RVAL]:[BVal]=[%04x]:[%04x]\n",UsCkVal, UsCkVal_Bk );
	if( UsCkVal != UsCkVal_Bk )
		return 5;

TRACE( "WrGyroOffsetData____COMPLETE\n" );
	return 0;
}