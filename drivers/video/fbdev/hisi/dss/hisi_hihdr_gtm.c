/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "hisi_hihdr_gtm.h"

static void hisi_dss_gtm_slf_lcproc_set_reg(struct dpu_fb_data_type *dpufd, char __iomem *hdr_base,
	gtm_basic_info_t* gtm_basic)
{
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_MIN_E,
		(gtm_basic->output_min_e << 16 | gtm_basic->input_min_e), 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_CLIP,
		(gtm_basic->ratiolutoutclip << 16 | gtm_basic->ratiolutinclip), 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_SCALE,
		((gtm_basic->lumaoutbit1 << 20) | (gtm_basic->lumaoutbit0 << 16)|
		(gtm_basic->chromaratiolutstep1 << 12) | (gtm_basic->chromaratiolutstep0 << 8)|
		(gtm_basic->lumaratiolutstep1 << 4)|(gtm_basic->lumaratiolutstep0 << 0)), 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_THRESHOLD,
		gtm_basic->chromalutthres << 16 | gtm_basic->lumalutthres, 32, 0);

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_Y_TRANSFM0,
		(gtm_basic->y_transform[1] << 16) | gtm_basic->y_transform[0], 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_Y_TRANSFM1, gtm_basic->y_transform[2], 32, 0);

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_OUT_BIT,
		(gtm_basic->chromaoutbit2 << 8 | gtm_basic->chromaoutbit1), 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_CRATIOCLIP, gtm_basic->chromaRatioclip, 32, 0);
}

static inline void hisi_dss_lut_set_reg(struct dpu_fb_data_type *dpufd, char __iomem *lut_base,
	uint32_t lut_size, uint32_t* lut)
{
	int i = 0;
	uint32_t lut_value = 0;
	for (i = 0; i < lut_size; i++) {
		lut_value = lut[i * 2 + 1] << 16 | lut[i * 2];
		dpufd->set_reg(dpufd, lut_base + (i * 4), lut_value, 32, 0);
	}
}

static void hisi_dss_gtm_slf_pq2slf_set_reg(struct dpu_fb_data_type *dpufd, char __iomem *hdr_base,
	gtm_basic_info_t* gtm_basic)
{
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_STEP1,
		((gtm_basic->pq2slf_step[3] << 24) | (gtm_basic->pq2slf_step[2] << 16) |
		(gtm_basic->pq2slf_step[1] << 8) | (gtm_basic->pq2slf_step[0])), 32, 0);/*chromaRatioclip=2048*/

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_STEP2,
		((gtm_basic->pq2slf_step[7] << 24) | (gtm_basic->pq2slf_step[6] << 16) |
		(gtm_basic->pq2slf_step[5] << 8) | (gtm_basic->pq2slf_step[4] )), 32, 0);/*chromaRatioclip=2048*/

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS1, gtm_basic->pq2slf_pos[0], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS2, gtm_basic->pq2slf_pos[1], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS3, gtm_basic->pq2slf_pos[2], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS4, gtm_basic->pq2slf_pos[3], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS5, gtm_basic->pq2slf_pos[4], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS6, gtm_basic->pq2slf_pos[5], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS7, gtm_basic->pq2slf_pos[6], 32, 0 );
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_POS8, gtm_basic->pq2slf_pos[7], 32, 0 );

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_NUM1,
		((gtm_basic->pq2slf_num[3] << 24) | (gtm_basic->pq2slf_num[2] << 16) |
		(gtm_basic->pq2slf_num[1] << 8) | (gtm_basic->pq2slf_num[0] )), 32, 0);

	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_PQ2SLF_NUM2,
		((gtm_basic->pq2slf_num[7] << 24) | (gtm_basic->pq2slf_num[6] << 16) |
		(gtm_basic->pq2slf_num[5] << 8) | (gtm_basic->pq2slf_num[4])), 32, 0);

	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_0,
		GTM_LUT_PQ2SLF_SIZE / 2, &gtm_basic->lut_pq2slf);
}

static void hisi_dss_gtm_lut_all_set_reg(struct dpu_fb_data_type *dpufd, char __iomem *hdr_base, gtm_dynamic_info_t *gtm_dynamic)
{
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_1,
		GTM_LUT_LUMA_SIZE / 2, &gtm_dynamic->lut_luma);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_2,
		GTM_LUT_CHROMA_SIZE / 2, &gtm_dynamic->lut_chroma);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_3,
		GTM_LUT_CHROMA0_SIZE / 2, &gtm_dynamic->lut_chroma0);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_4,
		GTM_LUT_CHROMA1_SIZE / 2, &gtm_dynamic->lut_chroma1);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_5,
		GTM_LUT_CHROMA2_SIZE / 2, &gtm_dynamic->lut_chroma2);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_6,
		GTM_LUT_LUMALUT0_SIZE / 2, &gtm_dynamic->lut_lumalut0);
	hisi_dss_lut_set_reg(dpufd, hdr_base + DSS_U_HDR_GTM_LUT_7,
		GTM_LUT_CHROMALUT0_SIZE /2 , &gtm_dynamic->lut_chromalut0);
}

static void hihdr_gtm_set_basic_reg(struct dpu_fb_data_type *dpufd, hihdr_gtm_info_t *gtm_info, char __iomem *hdr_base)
{
	if (gtm_info->basic_info.isinslf) {
		dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_CTRL, 0x1, 32, 0);
		dpufd->set_reg(dpufd, hdr_base + DSS_GTM_ROI, 0x9, 32, 0);
	} else {
		dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_CTRL, 0x3, 32, 0);
		dpufd->set_reg(dpufd, hdr_base + DSS_GTM_ROI, 0x8, 32, 0);
	}

	hisi_dss_gtm_slf_lcproc_set_reg(dpufd, hdr_base, &gtm_info->basic_info);
	hisi_dss_gtm_slf_pq2slf_set_reg(dpufd, hdr_base, &gtm_info->basic_info);
	hisi_dss_gtm_lut_all_set_reg(dpufd, hdr_base, &gtm_info->dynamic_info);
}

static void hihdr_gtm_set_dynamic_reg(struct dpu_fb_data_type *dpufd, hihdr_gtm_info_t *gtm_info, char __iomem *hdr_base)
{
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_LCPROC_MIN_E, (gtm_info->basic_info.output_min_e << 16 |
		gtm_info->basic_info.input_min_e), 32, 0);

	hisi_dss_gtm_lut_all_set_reg(dpufd, hdr_base, &gtm_info->dynamic_info);
}

static void hihdr_gtm_set_stop_reg(struct dpu_fb_data_type *dpufd, char __iomem *hdr_base)
{
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_SLF_CTRL, 0x0, 32, 0);
	dpufd->set_reg(dpufd, hdr_base + DSS_GTM_ROI, 0x0, 32, 0);
}

void hisi_dss_hihdr_gtm_set_reg(struct dpu_fb_data_type *dpufd, hihdr_gtm_info_t *gtm_info)
{
	char __iomem *hdr_base = NULL;

	dpu_check_and_no_retval((dpufd == NULL), ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval((gtm_info == NULL), ERR, "itm_info is NULL\n");
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;

	hdr_base = dpufd->dss_base + DSS_HDR_OFFSET;

	if (g_debug_effect_hihdr)
		DPU_FB_INFO("[hihdr] gtm flag = %d", gtm_info->flag);

	switch (gtm_info->flag) {
	case HDR_BASIC:
		dpufd->hihdr_basic_ready = dpufd->hihdr_basic_ready | HIHDR_HDR_BASIC_CONFIG_READY;
		hihdr_gtm_set_basic_reg(dpufd, gtm_info, hdr_base);
		break;
	case HDR_DYNAMIC:
		dpufd->hihdr_basic_ready = dpufd->hihdr_basic_ready & (~HIHDR_HDR_BASIC_CONFIG_READY);
		hihdr_gtm_set_dynamic_reg(dpufd, gtm_info, hdr_base);
		break;
	case HDR_CLOSE:
		dpufd->hihdr_basic_ready = dpufd->hihdr_basic_ready & (~HIHDR_HDR_BASIC_CONFIG_READY);
		hihdr_gtm_set_stop_reg(dpufd, hdr_base);
		break;
	case HDR_DEFAULT:
	default:
		break;
	}
}
