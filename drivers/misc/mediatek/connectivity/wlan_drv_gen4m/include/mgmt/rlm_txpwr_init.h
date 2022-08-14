/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *						include/mgmt/rlm_txpwr_init.h#1
 */

/*! \file   "rlm_txpwr_init.h"
 *    \brief
 */

#ifndef _RLM_TXPWR_INIT_H
#define _RLM_TXPWR_INIT_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
/*Support Tx Power Range : 63~ -64 (unit : 0.5dBm)*/

#define PWR_LIMIT_2G4_IN_MW_MHZ    BIT(0)
#define PWR_LIMIT_UNII1_IN_MW_MHZ  BIT(1)
#define PWR_LIMIT_UNII2A_IN_MW_MHZ BIT(2)
#define PWR_LIMIT_UNII2C_IN_MW_MHZ BIT(3)
#define PWR_LIMIT_UNII3_IN_MW_MHZ  BIT(4)

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
/*Set to MAX_TX_PWR = 63dBm if larger than it*/
struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT
	g_rRlmPowerLimitDefault[] = {

	{	{'A', 'R'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'A', 'U'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'N', 'Z'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'A', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'B', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'B', 'R'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'B', 'G'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'C', 'A'}
		, {60, 46, 48, 48, 60}
		, 0
	}
	,
	{	{'C', 'L'}
		, {43, 43, 43, 63, 43}
		, 0
	}
	,
	{	{'C', 'N'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'H', 'R'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'C', 'Z'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'D', 'K'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'D', 'M'}
		, {60, 34, 46, 46, 60}
		, 0
	}
	,
	{	{'E', 'C'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'E', 'G'}
		, {40, 46, 46, 63, 46}
		, 0
	}
	,
	{	{'S', 'V'}
		, {60, 34, 46, 46, 60}
		, 0
	}
	,
	{	{'F', 'I'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'F', 'R'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'G', 'R'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'H', 'N'}
		, {60, 60, 48, 48, 60}
		, 0
	}
	,
	{	{'H', 'K'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'H', 'U'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'I', 'S'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'I', 'N'}
		, {63, 46, 46, 63, 46}
		, 0
	}
	,
	{	{'I', 'D'}
		, {46, 63, 63, 63, 46}
		, 0
	}
	,
	{	{'I', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'I', 'L'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'I', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'J', 'P'}
		, {46, 46, 46, 46, 63}
		, 0
	}
	,
	{	{'K', 'R'}
		, {46, 34, 46, 46, 46}
		, 0
	}
	,
	{	{'L', 'U'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'Y'}
		, {54, 60, 60, 63, 60}
		, 0
	}
	,
	{	{'M', 'X'}
		, {60, 46, 48, 63, 60}
		, 0
	}
	,
	{	{'N', 'L'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'N', 'O'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'O', 'M'}
		, {40, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'P', 'K'}
		, {40, 63, 63, 63, 40}
		, 0
	}
	,
	{	{'P', 'Y'}
		, {60, 46, 46, 48, 60}
		, 0
	}
	,
	{	{'P', 'E'}
		, {54, 46, 48, 42, 48}
		, 0
	}
	,
	{	{'P', 'H'}
		, {48, 48, 48, 48, 48}
		, 0
	}
	,
	{	{'P', 'L'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'P', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'Q', 'A'}
		, {40, 63, 63, 63, 40}
		, 0
	}
	,
	{	{'R', 'O'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'R', 'U'}
		, {48, 40, 40, 60, 60}
		, 0
	}
	,
	{	{'S', 'A'}
		, {40, 46, 46, 46, 46}
		, 0
	}
	,
	{	{'S', 'G'}
		, {46, 40, 40, 60, 60}
		, 0
	}
	,
	{	{'S', 'K'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'S', 'I'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'Z', 'A'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'E', 'S'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'S', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'C', 'H'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'T', 'W'}
		, {60, 48, 48, 48, 60}
		, 0
	}
	,
	{	{'T', 'H'}
		, {40, 46, 46, 60, 60}
		, 0
	}
	,
	{	{'A', 'E'}
		, {40, 46, 46, 60, 60}
		, 0
	}
	,
	{	{'G', 'B'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'U', 'Y'}
		, {60, 34, 46, 46, 60}
		, 0
	}
	,
	{	{'U', 'S'}
		, {60, 48, 48, 48, 60}
		, 0
	}
	,
	{	{'V', 'E'}
		, {60, 46, 46, 63, 46}
		, 0
	}
	,
	{	{'V', 'N'}
		, {40, 46, 46, 60, 34}
		, 0
	}
	,
	{	{'A', 'O'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'B', 'Z'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'B', 'J'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'B', 'T'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'B', 'O'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'B', 'I'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'M'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'F'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'D'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'K', 'M'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'D'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'G'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'I'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'D', 'J'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'Q'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'E', 'R'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'F', 'J'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'A'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'M'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'N'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'W'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'R', 'K'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'K', 'G'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'L', 'Y'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'M', 'G'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'M', 'L'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'N', 'R'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'N', 'C'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'T'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'C'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'L'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'B'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'O'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'R'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'Z'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'J'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'G'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'O'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'M'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'V'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'V', 'U'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'Y', 'E'}
		, {40, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'A', 'S'}
		, {60, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'A', 'I'}
		, {60, 34, 48, 60, 60}
		, 0
	}
	,
	{	{'B', 'M'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'K', 'Y'}
		, {60, 34, 48, 60, 60}
		, 0
	}
	,
	{	{'G', 'U'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'F', 'M'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'P', 'R'}
		, {60, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'V', 'I'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'A', 'Z'}
		, {40, 34, 48, 60, 60}
		, 0
	}
	,
	{	{'B', 'W'}
		, {40, 46, 46, 60, 60}
		, 0
	}
	,
	{	{'K', 'H'}
		, {40, 46, 46, 48, 60}
		, 0
	}
	,
	{	{'C', 'X'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'C', 'O'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'C', 'R'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'G', 'D'}
		, {40, 46, 46, 60, 60}
		, 0
	}
	,
	{	{'G', 'T'}
		, {40, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'K', 'I'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'L', 'B'}
		, {40, 46, 46, 46, 46}
		, 0
	}
	,
	{	{'L', 'R'}
		, {60, 46, 60, 63, 63}
		, 0
	}
	,
	{	{'M', 'N'}
		, {46, 32, 46, 46, 58}
		, 0
	}
	,
	{	{'A', 'N'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'N', 'I'}
		, {60, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'P', 'W'}
		, {60, 60, 60, 60, 60}
		, 0
	}
	,
	{	{'W', 'S'}
		, {40, 40, 40, 40, 60}
		, 0
	}
	,
	{	{'L', 'K'}
		, {46, 46, 46, 46, 46}
		, 0
	}
	,
	{	{'T', 'T'}
		, {60, 46, 46, 60, 60}
		, 0
	}
	,
	{	{'A', 'W'}
		, {60, 46, 60, 60, 63}
		, 0
	}
	,
	{	{'L', 'A'}
		, {40, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'U', 'G'}
		, {40, 46, 46, 48, 60}
		, 0
	}
	,
	{	{'M', 'M'}
		, {40, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'A', 'L'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'D', 'Z'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'A', 'D'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'B', 'Y'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'B', 'A'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'V', 'G'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'C', 'V'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'C', 'Y'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'E', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'E', 'T'}
		, {40, 40, 40, 40, 28}
		, 0
	}
	,
	{	{'G', 'F'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'P', 'F'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'T', 'F'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'G', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'D', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'G', 'H'}
		, {40, 34, 48, 60, 28}
		, 0
	}
	,
	{	{'G', 'P'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'I', 'Q'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'K', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'L', 'V'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'L', 'S'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'L', 'I'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'L', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'K'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'Q'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'R'}
		, {40, 46, 46, 46, 28}
		, 0
	}
	,
	{	{'M', 'U'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'Y', 'T'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'D'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'C'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'S'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'R', 'E'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'M', 'F'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'S', 'M'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'S', 'N'}
		, {40, 40, 40, 60, 28}
		, 0
	}
	,
	{	{'R', 'S'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'T', 'R'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'T', 'C'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'V', 'A'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'A', 'M'}
		, {40, 40, 40, 63, 63}
		, 0
	}
	,
	{	{'K', 'W'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'M', 'A'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'N', 'E'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'T', 'N'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'N', 'P'}
		, {60, 46, 46, 63, 60}
		, 0
	}
	,
	{	{'A', 'F'}
		, {40, 46, 63, 63, 63}
		, 0
	}
	,
	{	{'A', 'G'}
		, {40, 46, 48, 63, 54}
		, 0
	}
	,
	{	{'B', 'S'}
		, {63, 46, 60, 63, 63}
		, 0
	}
	,
	{	{'B', 'H'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'B', 'B'}
		, {40, 46, 48, 63, 54}
		, 0
	}
	,
	{	{'B', 'N'}
		, {46, 46, 46, 63, 60}
		, 0
	}
	,
	{	{'M', 'V'}
		, {40, 46, 46, 63, 40}
		, 0
	}
	,
	{	{'P', 'A'}
		, {60, 34, 48, 63, 60}
		, 0
	}
	,
	{	{'Z', 'M'}
		, {60, 46, 46, 63, 60}
		, 0
	}
	,
	{	{'J', 'O'}
		, {40, 46, 63, 63, 46}
		, 0
	}
	,
	{	{'P', 'G'}
		, {40, 46, 63, 63, 60}
		, 0
	}
	,
	{	{'B', 'F'}
		, {40, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'G', 'Y'}
		, {60, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'H', 'T'}
		, {40, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'J', 'M'}
		, {54, 63, 63, 63, 57}
		, 0
	}
	,
	{	{'M', 'O'}
		, {40, 63, 63, 63, 40}
		, 0
	}
	,
	{	{'M', 'W'}
		, {60, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'R', 'W'}
		, {40, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'K', 'N'}
		, {40, 63, 63, 63, 60}
		, 0
	}
	,
	{	{'T', 'Z'}
		, {40, 63, 63, 63, 40}
		, 0
	}
	,
	{	{'N', 'G'}
		, {40, 63, 46, 63, 60}
		, 0
	}
	,
	{	{'B', 'D'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'D', 'O'}
		, {63, 46, 46, 60, 63}
		, 0
	}
	,
	{	{'F', 'K'}
		, {40, 46, 46, 60, 28}
		, 0
	}
	,
	{	{'K', 'Z'}
		, {40, 34, 48, 60, 60}
		, 0
	}
	,
	{	{'M', 'Z'}
		, {40, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'N', 'A'}
		, {40, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'L', 'C'}
		, {40, 34, 48, 48, 60}
		, 0
	}
	,
	{	{'V', 'C'}
		, {40, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'U', 'A'}
		, {40, 46, 46, 46, 48}
		, 0
	}
	,
	{	{'U', 'Z'}
		, {40, 48, 48, 48, 60}
		, 0
	}
	,
	{	{'Z', 'W'}
		, {40, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'M', 'P'}
		, {60, 34, 46, 48, 60}
		, 0
	}
	,
	{	{'C', 'K'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'C', 'U'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'T', 'L'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'F', 'O'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'G', 'I'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'I', 'R'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'I', 'M'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'J', 'E'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'K', 'P'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'M', 'H'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'N', 'U'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'N', 'F'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'P', 'S'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'P', 'N'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'P', 'M'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'S'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'D'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
	{	{'S', 'Y'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
#if 1 /* Temp define as 6630 */
	{	{'E', 'H'}
		, {40, 46, 46, 63, 63}
		, 0
	}
	,
	{	{'G', 'G'}
		, {63, 63, 63, 63, 63}
		, 0
	}
	,
#endif
	{	{'Z', 'Z'}
		, {40, 34, 46, 48, 60}
		, 0
	}
	,

	/*Default*/
	{	{0, 0}
		, {63, 63, 63, 63, 63}
		, 0
	}
};

struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION_HE
	g_rRlmPowerLimitConfigurationHE[] = {
	/*Default*/
	{	{0, 0}
		, 36,
			{64, 64, 64, /* RU26 L,H,U */
			64, 64, 64,  /* RU52 L,H,U*/
			64, 64, 64,  /* RU106 L,H,U*/
			64, 64, 64,  /* RU242 L,H,U*/
			64, 64, 64,  /* RU484 L,H,U*/
			64, 64, 64}  /* RU996 L,H,U*/
	}
};


struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION
	g_rRlmPowerLimitConfiguration[] = {

	{	{'A', 'I'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'A', 'Z'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'B', 'W'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'G', 'D'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'L', 'B'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'L', 'R'}
		, 144, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'W', 'S'}
		, 165, {40, 40, 40, 40, 40, 40, 40, 40, 40}
	}
	,
	{	{'U', 'S'}
		, 1, {39, 34, 32, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 6, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 11, {39, 33, 32, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 3, {60, 60, 60, 31, 30, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 9, {60, 60, 60, 31, 30, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 36, {48, 36, 36, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 40, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 48, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 52, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 60, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 64, {48, 36, 36, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 100, {48, 37, 37, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 116, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 140, {48, 42, 42, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 149, {60, 41, 41, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 157, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 165, {60, 42, 42, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 38, {48, 48, 48, 27, 27, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 46, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 54, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 62, {48, 48, 48, 28, 28, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 102, {48, 48, 48, 29, 29, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 110, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 134, {48, 48, 48, 41, 41, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 151, {60, 60, 60, 36, 36, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 159, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 42, {48, 48, 48, 48, 48, 24, 24, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 58, {48, 48, 48, 48, 48, 26, 26, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 106, {48, 48, 48, 48, 48, 25, 25, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 122, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'U', 'S'}
		, 155, {60, 60, 60, 60, 60, 36, 36, 60, 60}
	}
	,
	{	{'U', 'S'}
		, 50, {48, 48, 48, 48, 48, 48, 48, 24, 24}
	}
	,
	{	{'U', 'S'}
		, 114, {48, 48, 48, 48, 48, 48, 48, 24, 24}
	}
	,
	{	{'T', 'W'}
		, 1, {39, 34, 32, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 6, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 11, {39, 33, 32, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 3, {60, 60, 60, 31, 30, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 9, {60, 60, 60, 31, 30, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 36, {48, 36, 36, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 40, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 48, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 52, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 60, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 64, {48, 36, 36, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 100, {48, 37, 37, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 116, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 140, {48, 42, 42, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 149, {60, 41, 41, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 157, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 165, {60, 42, 42, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 38, {48, 48, 48, 27, 27, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 46, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 54, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 62, {48, 48, 48, 28, 28, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 102, {48, 48, 48, 29, 29, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 110, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 134, {48, 48, 48, 41, 41, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 151, {60, 60, 60, 36, 36, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 159, {60, 60, 60, 60, 60, 60, 60, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 42, {48, 48, 48, 48, 48, 24, 24, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 58, {48, 48, 48, 48, 48, 26, 26, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 106, {48, 48, 48, 48, 48, 25, 25, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 122, {48, 48, 48, 48, 48, 48, 48, 48, 48}
	}
	,
	{	{'T', 'W'}
		, 155, {60, 60, 60, 60, 60, 36, 36, 60, 60}
	}
	,
	{	{'T', 'W'}
		, 50, {48, 48, 48, 48, 48, 48, 48, 24, 24}
	}
	,
	{	{'T', 'W'}
		, 114, {48, 48, 48, 48, 48, 48, 48, 24, 24}
	}
	,

	/*Default*/
	{	{0, 0}
		, 165, {63, 63, 63, 63, 63, 63, 63, 63, 63}
	}
};

#endif

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

#endif /* _RLM_TXPWR_INIT_H */
