/*
 * hihonor_fg_glink.h
 *
 * hihonor_glink driver
 *
 * Copyright (c) 2021-2021 Honor Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HIHONOR_FG_GLINK_H_
#define _HIHONOR_FG_GLINK_H_

#ifdef CONFIG_HIHONOR_OEM_GLINK
void hihonor_glink_set_vterm_dec(unsigned int val);
#else
static inline void hihonor_glink_set_vterm_dec(unsigned int val)
{
}

#endif /* CONFIG_HIHONOR_OEM_GLINK */
#endif /* _HIHONOR_FG_GLINK_H_ */

