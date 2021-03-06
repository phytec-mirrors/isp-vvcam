/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

/* process public and sample isp command. for complex modules, need new files.*/
#include "isp_driver.h"
#include "cma.h"
#include "isp_ioctl.h"
#include "mrv_all_bits.h"
#include "isp_types.h"

volatile MrvAllRegister_t *all_regs = NULL;

void isp_write_reg(struct isp_ic_dev *dev, u32 offset, u32 val)
{
	if (offset >= ISP_REG_SIZE)
		return;
	__raw_writel(val, dev->base + offset);
}

u32 isp_read_reg(struct isp_ic_dev *dev, u32 offset)
{
	u32 val = 0;

	if (offset >= ISP_REG_SIZE)
		return 0;
	val = __raw_readl(dev->base + offset);
	return val;
}

int isp_reset(struct isp_ic_dev *dev)
{
	pr_debug("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(vi_ircl), 0xFFFFFFFF);
	isp_write_reg(dev, REG_ADDR(vi_ircl), 0x0);
	return 0;
}

int isp_enable_tpg(struct isp_ic_dev *dev)
{
	u32 addr, isp_tpg_ctrl;

	pr_debug("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	isp_tpg_ctrl = isp_read_reg(dev, addr);
	REG_SET_SLICE(isp_tpg_ctrl, TPG_ENABLE, 1);
	isp_write_reg(dev, addr, isp_tpg_ctrl);
	return 0;
}

int isp_disable_tpg(struct isp_ic_dev *dev)
{
	u32 addr, isp_tpg_ctrl;

	pr_debug("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	isp_tpg_ctrl = isp_read_reg(dev, addr);
	REG_SET_SLICE(isp_tpg_ctrl, TPG_ENABLE, 0);
	isp_write_reg(dev, addr, isp_tpg_ctrl);
	return 0;
}

int isp_enable_bls(struct isp_ic_dev *dev)
{
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_ENABLE,
		      MRV_BLS_BLS_ENABLE_PROCESS);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	return 0;
}

int isp_disable_bls(struct isp_ic_dev *dev)
{
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_ENABLE,
		      MRV_BLS_BLS_ENABLE_BYPASS);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	return 0;
}

int isp_enable(struct isp_ic_dev *dev)
{
	u32 isp_ctrl, isp_imsc;

	pr_debug("enter %s\n", __func__);
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc |= (MRV_ISP_IMSC_ISP_OFF_MASK | MRV_ISP_IMSC_FRAME_MASK);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_disable(struct isp_ic_dev *dev)
{
	u32 isp_ctrl, isp_imsc;

	pr_debug("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc |= (MRV_ISP_IMSC_ISP_OFF_MASK | MRV_ISP_IMSC_FRAME_MASK);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_isr), MRV_ISP_ISR_ISP_OFF_MASK);
	return 0;
}

int isp_enable_lsc(struct isp_ic_dev *dev)
{
	u32 isp_lsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_lsc_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_lsc_ctrl, MRV_LSC_LSC_EN, 1U);
	isp_write_reg(dev, REG_ADDR(isp_lsc_ctrl), isp_lsc_ctrl);
	return 0;
}

int isp_disable_lsc(struct isp_ic_dev *dev)
{
	u32 isp_lsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_lsc_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_lsc_ctrl, MRV_LSC_LSC_EN, 0U);
	isp_write_reg(dev, REG_ADDR(isp_lsc_ctrl), isp_lsc_ctrl);
	return 0;
}

int isp_s_input(struct isp_ic_dev *dev)
{
	struct isp_context isp_ctx = *(&dev->ctx);
	u32 isp_ctrl, isp_acq_prop, isp_demosaic;

	pr_debug("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_MODE, isp_ctx.mode);
	isp_acq_prop = isp_read_reg(dev, REG_ADDR(isp_acq_prop));
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_SAMPLE_EDGE, isp_ctx.sample_edge);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_HSYNC_POL,
		      isp_ctx.hSyncLowPolarity);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_VSYNC_POL,
		      isp_ctx.vSyncLowPolarity);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_BAYER_PAT, isp_ctx.bayer_pattern);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_CONV_422, isp_ctx.sub_sampling);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_CCIR_SEQ, isp_ctx.seq_ccir);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_FIELD_SELECTION,
		      isp_ctx.field_selection);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_INPUT_SELECTION,
		      isp_ctx.input_selection);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_LATENCY_FIFO_SELECTION,
		      isp_ctx.latency_fifo);

	isp_write_reg(dev, REG_ADDR(isp_acq_prop), isp_acq_prop);
	isp_write_reg(dev, REG_ADDR(isp_acq_h_offs), isp_ctx.acqWindow.x);
	isp_write_reg(dev, REG_ADDR(isp_acq_v_offs), isp_ctx.acqWindow.y);
	isp_write_reg(dev, REG_ADDR(isp_acq_h_size), isp_ctx.acqWindow.width);
	isp_write_reg(dev, REG_ADDR(isp_acq_v_size), isp_ctx.acqWindow.height);

	isp_write_reg(dev, REG_ADDR(isp_out_h_offs),
		      (isp_ctx.ofWindow.x & MRV_ISP_ISP_OUT_H_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_v_offs),
		      (isp_ctx.ofWindow.y & MRV_ISP_ISP_OUT_V_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_h_size),
		      (isp_ctx.ofWindow.width & MRV_ISP_ISP_OUT_H_SIZE_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_v_size),
		      (isp_ctx.ofWindow.height & MRV_ISP_ISP_OUT_V_SIZE_MASK));

	isp_write_reg(dev, REG_ADDR(isp_is_h_offs),
		      (isp_ctx.isWindow.x & MRV_IS_IS_H_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_v_offs),
		      (isp_ctx.isWindow.y & MRV_IS_IS_V_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_h_size),
		      (isp_ctx.isWindow.width & MRV_IS_IS_H_SIZE_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_v_size),
		      (isp_ctx.isWindow.height & MRV_IS_IS_V_SIZE_MASK));

	isp_demosaic = isp_read_reg(dev, REG_ADDR(isp_demosaic));
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_BYPASS,
		      isp_ctx.bypass_mode);
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_TH,
		      isp_ctx.demosaic_threshold);
	isp_write_reg(dev, REG_ADDR(isp_demosaic), isp_demosaic);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_s_demosaic(struct isp_ic_dev *dev)
{
	struct isp_context isp_ctx = *(&dev->ctx);
	u32 isp_demosaic;
	pr_debug("enter %s\n", __func__);
	isp_demosaic = isp_read_reg(dev, REG_ADDR(isp_demosaic));
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_BYPASS,
		      isp_ctx.bypass_mode);
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_TH,
		      isp_ctx.demosaic_threshold);
	isp_write_reg(dev, REG_ADDR(isp_demosaic), isp_demosaic);
	return 0;;
}

int isp_s_tpg(struct isp_ic_dev *dev)
{
	struct isp_tpg_context tpg = *(&dev->tpg);
	u32 addr, regVal;

	pr_debug("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	regVal = isp_read_reg(dev, addr);
	REG_SET_SLICE(regVal, TPG_IMG_NUM, tpg.image_type);
	REG_SET_SLICE(regVal, TPG_CFA_PAT, tpg.bayer_pattern);
	REG_SET_SLICE(regVal, TPG_COLOR_DEPTH, tpg.color_depth);
	REG_SET_SLICE(regVal, TPG_RESOLUTION, tpg.resolution);
	isp_write_reg(dev, addr, regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_PIX_GAP_IN, tpg.pixleGap);
	REG_SET_SLICE(regVal, TPG_LINE_GAP_IN, tpg.lineGap);
	isp_write_reg(dev, REG_ADDR(isp_tpg_gap_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_PIX_GAP_STD_IN, tpg.gapStandard);
	isp_write_reg(dev, REG_ADDR(isp_tpg_gap_std_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_RANDOM_SEED, tpg.randomSeed);
	isp_write_reg(dev, REG_ADDR(isp_tpg_random_seed), regVal);
	REG_SET_SLICE(regVal, TPG_HTOTAL_IN, tpg.user_mode_h.total);
	REG_SET_SLICE(regVal, TPG_VTOTAL_IN, tpg.user_mode_v.total);
	isp_write_reg(dev, REG_ADDR(isp_tpg_total_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_HACT_IN, tpg.user_mode_h.act);
	REG_SET_SLICE(regVal, TPG_VACT_IN, tpg.user_mode_v.act);
	isp_write_reg(dev, REG_ADDR(isp_tpg_act_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_FP_H_IN, tpg.user_mode_h.fp);
	REG_SET_SLICE(regVal, TPG_FP_V_IN, tpg.user_mode_v.fp);
	isp_write_reg(dev, REG_ADDR(isp_tpg_fp_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_BP_H_IN, tpg.user_mode_h.bp);
	REG_SET_SLICE(regVal, TPG_BP_V_IN, tpg.user_mode_v.bp);
	isp_write_reg(dev, REG_ADDR(isp_tpg_bp_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_HS_W_IN, tpg.user_mode_h.sync);
	REG_SET_SLICE(regVal, TPG_VS_W_IN, tpg.user_mode_v.sync);
	isp_write_reg(dev, REG_ADDR(isp_tpg_w_in), regVal);
	return 0;
}

int isp_s_mux(struct isp_ic_dev *dev)
{
	struct isp_mux_context mux = *(&dev->mux);
	u32 vi_dpcl;

	pr_debug("enter %s\n", __func__);
	vi_dpcl = isp_read_reg(dev, REG_ADDR(vi_dpcl));
	REG_SET_SLICE(vi_dpcl, MRV_VI_MP_MUX, mux.mp_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_SPMUX, mux.sp_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_CHAN_MODE, mux.chan_mode);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_IEMUX, mux.ie_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_SWITCH, mux.dma_read_switch);
	REG_SET_SLICE(vi_dpcl, MRV_IF_SELECT, mux.if_select);
	isp_write_reg(dev, REG_ADDR(vi_dpcl), vi_dpcl);
	return 0;
}

int isp_s_bls(struct isp_ic_dev *dev)
{
	struct isp_bls_context bls = *(&dev->bls);
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_MODE, bls.mode);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_bls_a_fixed), bls.a);
	isp_write_reg(dev, REG_ADDR(isp_bls_b_fixed), bls.b);
	isp_write_reg(dev, REG_ADDR(isp_bls_c_fixed), bls.c);
	isp_write_reg(dev, REG_ADDR(isp_bls_d_fixed), bls.d);
	return 0;
}

int isp_enable_awb(struct isp_ic_dev *dev)
{
	u32 isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MODE, MRV_ISP_AWB_MODE_MEAS);
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_AWB_DONE_MASK);
	return 0;
}

int isp_disable_awb(struct isp_ic_dev *dev)
{
	u32 isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MODE, MRV_ISP_AWB_MODE_NOMEAS);
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc & ~MRV_ISP_IMSC_AWB_DONE_MASK);
	return 0;
}

int isp_s_awb(struct isp_ic_dev *dev, struct isp_awb_context *awb)
{
	u32 gain_data = 0;
	u32 isp_awb_thresh = 0;
	u32 isp_awb_ref = 0;
	u32 isp_awb_prop = 0;

	/* pr_debug("enter %s\n", __func__); */
	isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));

	if (awb->mode == MRV_ISP_AWB_MEAS_MODE_YCBCR) {
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MEAS_MODE,
			      MRV_ISP_AWB_MEAS_MODE_YCBCR);
		if (awb->max_y == 0) {
			REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
				      MRV_ISP_AWB_MAX_EN_DISABLE);
		} else {
			REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
				      MRV_ISP_AWB_MAX_EN_ENABLE);
		}
	} else if (awb->mode == MRV_ISP_AWB_MEAS_MODE_RGB) {
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
			      MRV_ISP_AWB_MAX_EN_DISABLE);
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MEAS_MODE,
			      MRV_ISP_AWB_MEAS_MODE_RGB);
	}
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);

	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MAX_Y, awb->max_y);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MIN_Y__MAX_G,
		      awb->min_y_max_g);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MAX_CSUM, awb->max_c_sum);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MIN_C, awb->min_c);
	isp_write_reg(dev, REG_ADDR(isp_awb_thresh), isp_awb_thresh);

	REG_SET_SLICE(isp_awb_ref, MRV_ISP_AWB_REF_CR__MAX_R, awb->refcr_max_r);
	REG_SET_SLICE(isp_awb_ref, MRV_ISP_AWB_REF_CB__MAX_B, awb->refcb_max_b);
	isp_write_reg(dev, REG_ADDR(isp_awb_ref), isp_awb_ref);
	isp_write_reg(dev, REG_ADDR(isp_awb_frames), 0);
	isp_write_reg(dev, REG_ADDR(isp_awb_h_offs),
		      (MRV_ISP_AWB_H_OFFS_MASK & awb->window.x));
	isp_write_reg(dev, REG_ADDR(isp_awb_v_offs),
		      (MRV_ISP_AWB_V_OFFS_MASK & awb->window.y));
	isp_write_reg(dev, REG_ADDR(isp_awb_h_size),
		      (MRV_ISP_AWB_H_SIZE_MASK & awb->window.width));
	isp_write_reg(dev, REG_ADDR(isp_awb_v_size),
		      (MRV_ISP_AWB_V_SIZE_MASK & awb->window.height));

	gain_data = 0UL;
	REG_SET_SLICE(gain_data, MRV_ISP_AWB_GAIN_R, awb->gain_r);
	REG_SET_SLICE(gain_data, MRV_ISP_AWB_GAIN_B, awb->gain_b);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_rb), gain_data);

	gain_data = 0UL;
	REG_SET_SLICE(gain_data, MRV_ISP_AWB_GAIN_GR, awb->gain_gr);
	REG_SET_SLICE(gain_data, MRV_ISP_AWB_GAIN_GB, awb->gain_gb);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_g), gain_data);
	return 0;
}

int isp_s_is(struct isp_ic_dev *dev)
{
	struct isp_is_context is = *(&dev->is);
	u32 isp_is_ctrl;
	u32 isp_is_displace;
	u32 isp_ctrl;

	pr_debug("enter %s\n", __func__);

	isp_is_ctrl = isp_read_reg(dev, REG_ADDR(isp_is_ctrl));

	if (!is.enable) {
		REG_SET_SLICE(isp_is_ctrl, MRV_IS_IS_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_is_ctrl), isp_is_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_is_ctrl, MRV_IS_IS_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_is_h_offs), is.window.x);
	isp_write_reg(dev, REG_ADDR(isp_is_v_offs), is.window.y);
	isp_write_reg(dev, REG_ADDR(isp_is_h_size), is.window.width);
	isp_write_reg(dev, REG_ADDR(isp_is_v_size), is.window.height);
	isp_write_reg(dev, REG_ADDR(isp_is_recenter),
		      is.recenter & MRV_IS_IS_RECENTER_MASK);
	isp_write_reg(dev, REG_ADDR(isp_is_max_dx), is.max_dx);
	isp_write_reg(dev, REG_ADDR(isp_is_max_dy), is.max_dy);
	isp_is_displace = isp_read_reg(dev, REG_ADDR(isp_is_displace));
	REG_SET_SLICE(isp_is_displace, MRV_IS_DX, is.displace_x);
	REG_SET_SLICE(isp_is_displace, MRV_IS_DY, is.displace_y);
	isp_write_reg(dev, REG_ADDR(isp_is_displace), isp_is_displace);
	isp_write_reg(dev, REG_ADDR(isp_is_ctrl), isp_is_ctrl);
	if (is.update) {
		isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
		REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		is.update = false;
	}
	return 0;
}

int isp_s_cnr(struct isp_ic_dev *dev)
{
	struct isp_cnr_context *cnr = &dev->cnr;
	u32 isp_ctrl;

	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	if (!cnr->enable) {
		REG_SET_SLICE(isp_ctrl, MRV_ISP_CNR_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_ctrl, MRV_ISP_CNR_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_cnr_linesize), cnr->line_width);
	isp_write_reg(dev, REG_ADDR(isp_cnr_threshold_c1), cnr->threshold_1);
	isp_write_reg(dev, REG_ADDR(isp_cnr_threshold_c2), cnr->threshold_2);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_start_stream(struct isp_ic_dev *dev, u32 numFrames)
{
	u32 isp_imsc, isp_ctrl;

	pr_debug("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(isp_sh_ctrl), 0x10);
	isp_write_reg(dev, REG_ADDR(isp_acq_nr_frames),
		      (MRV_ISP_ACQ_NR_FRAMES_MASK & numFrames));
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc |=
	    (MRV_ISP_IMSC_ISP_OFF_MASK | MRV_ISP_IMSC_FRAME_MASK |
	     MRV_ISP_IMSC_FRAME_IN_MASK);
	/* isp_imsc |= (MRV_ISP_IMSC_FRAME_MASK | MRV_ISP_IMSC_DATA_LOSS_MASK | MRV_ISP_IMSC_FRAME_IN_MASK); */
	isp_write_reg(dev, REG_ADDR(isp_icr), 0xFFFFFFFF);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CFG_UPD, 1);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 1);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_stop_stream(struct isp_ic_dev *dev)
{
	pr_debug("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(isp_imsc), 0);
	isp_disable(dev);
	return 0;
}

int isp_s_cc(struct isp_ic_dev *dev, struct isp_cc_context *cc)
{
	u32 isp_ctrl, addr;
	int i;

	pr_debug("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CSM_Y_RANGE, cc->conv_range_y_full);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CSM_C_RANGE, cc->conv_range_c_full);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	if (cc->update_curve) {
		addr = REG_ADDR(isp_cc_coeff_0);
		for (i = 0; i < 9; i++) {
			isp_write_reg(dev, addr + i * 4,
				      MRV_ISP_CC_COEFF_0_MASK & cc->lCoeff[i]);
		}
	}
	return 0;
}

int isp_s_xtalk(struct isp_ic_dev *dev, struct isp_xtalk_context *xtalk)
{
	int i;

	/* pr_debug("enter %s\n", __func__); */

	for (i = 0; i < 9; i++) {
		isp_write_reg(dev, REG_ADDR(cross_talk_coef_block_arr[i]),
			      MRV_ISP_CT_COEFF_MASK & xtalk->lCoeff[i]);
	}
	isp_write_reg(dev, REG_ADDR(isp_ct_offset_r),
		      (MRV_ISP_CT_OFFSET_R_MASK & xtalk->r));
	isp_write_reg(dev, REG_ADDR(isp_ct_offset_g),
		      (MRV_ISP_CT_OFFSET_G_MASK & xtalk->g));
	isp_write_reg(dev, REG_ADDR(isp_ct_offset_b),
		      (MRV_ISP_CT_OFFSET_B_MASK & xtalk->b));
	return 0;
}

int isp_enable_wb(struct isp_ic_dev *dev, bool bEnable)
{
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_AWB_ENABLE, bEnable);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_enable_gamma_out(struct isp_ic_dev *dev, bool bEnable)
{
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_debug("enter %s\n", __func__);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_OUT_ENABLE, bEnable);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_s_gamma_out(struct isp_ic_dev *dev, struct isp_gamma_out_context *gamma)
{
	u32 isp_gamma_out_mode;
	int i;

	isp_gamma_out_mode = isp_read_reg(dev, REG_ADDR(isp_gamma_out_mode));
	REG_SET_SLICE(isp_gamma_out_mode, MRV_ISP_EQU_SEGM, gamma->mode);
	isp_write_reg(dev, REG_ADDR(isp_gamma_out_mode), isp_gamma_out_mode);

	for (i = 0; i < 17; i++) {
		isp_write_reg(dev, REG_ADDR(gamma_out_y_block_arr[i]),
			      MRV_ISP_ISP_GAMMA_OUT_Y_MASK & gamma->curve[i]);
	}
	return 0;
}

int isp_s_lsc(struct isp_ic_dev *dev, struct isp_lsc_context *lsc)
{
	int i, n;
	u32 sram_addr;
	u32 isp_lsc_status;
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_debug("enter %s\n", __func__);

	/* Enable isp to enable ram clock for write correct table to ram. */
	if (!(isp_ctrl & 0x01)) {
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl | 0x01);
	}

	for (i = 0; i < CAEMRIC_GRAD_TBL_SIZE; i += 2) {
		isp_write_reg(dev, REG_ADDR(isp_lsc_xsize_01) + i * 2,
			      (lsc->x_size[i] & MRV_LSC_Y_SECT_SIZE_0_MASK) |
			      ((lsc->x_size[i + 1]
				<< MRV_LSC_X_SECT_SIZE_1_SHIFT)
			       & MRV_LSC_X_SECT_SIZE_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_ysize_01) + i * 2,
			      (lsc->y_size[i] & MRV_LSC_Y_SECT_SIZE_0_MASK) |
			      ((lsc->y_size[i + 1]
				<< MRV_LSC_Y_SECT_SIZE_1_SHIFT)
			       & MRV_LSC_Y_SECT_SIZE_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_xgrad_01) + i * 2,
			      (lsc->x_grad[i] & MRV_LSC_XGRAD_0_MASK) |
			      ((lsc->x_grad[i + 1]
				<< MRV_LSC_XGRAD_1_SHIFT)
			       & MRV_LSC_XGRAD_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_ygrad_01) + i * 2,
			      (lsc->y_grad[i] & MRV_LSC_YGRAD_0_MASK) |
			      ((lsc->y_grad[i + 1]
				<< MRV_LSC_YGRAD_1_SHIFT)
			       & MRV_LSC_YGRAD_1_MASK));
	}

	isp_lsc_status = isp_read_reg(dev, REG_ADDR(isp_lsc_status));
	sram_addr = (isp_lsc_status & 0x2U) ? 0U : 153U;
	isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_addr), sram_addr);

	for (n = 0;
	     n <
	     ((CAMERIC_MAX_LSC_SECTORS + 1) * (CAMERIC_MAX_LSC_SECTORS + 1));
	     n += CAMERIC_MAX_LSC_SECTORS + 1) {
		/* 17 sectors with 2 values in one DWORD = 9 DWORDs (8 steps + 1 outside loop) */
		for (i = 0; i < (CAMERIC_MAX_LSC_SECTORS); i += 2) {
			isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_data),
				      lsc->r[n +
					     i] | (lsc->r[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_data),
				      lsc->gr[n +
					      i] | (lsc->gr[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_data),
				      lsc->gb[n +
					      i] | (lsc->gb[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_data),
				      lsc->b[n +
					     i] | (lsc->b[n + i + 1] << 12));
		}
		isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_data),
			      lsc->r[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_data),
			      lsc->gr[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_data),
			      lsc->gb[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_data),
			      lsc->b[n + CAMERIC_MAX_LSC_SECTORS]);
	}

	isp_write_reg(dev, REG_ADDR(isp_lsc_table_sel),
		      (isp_lsc_status & 0x2U) ? 0U : 1U);
	/* restore isp */
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	return 0;
}

int isp_ioc_read_reg(struct isp_ic_dev *dev, void *__user args)
{
	struct isp_reg_t *reg = (struct isp_reg_t *)args;

	reg->val = isp_read_reg(dev, reg->offset);
	return 0;
}

int isp_ioc_write_reg(struct isp_ic_dev *dev, void *__user args)
{
	struct isp_reg_t *reg = (struct isp_reg_t *)args;

	isp_write_reg(dev, reg->offset, reg->val);
	return 0;
}

int isp_ioc_disable_isp_off(struct isp_ic_dev *dev, void *args)
{
	u32 isp_imsc;

	pr_debug("enter %s\n", __func__);
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc &= ~MRV_ISP_IMSC_ISP_OFF_MASK;
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);

	return 0;
}

int isp_g_awbmean(struct isp_ic_dev *dev, struct isp_awb_mean *mean)
{
	u32 reg = isp_read_reg(dev, REG_ADDR(isp_awb_mean));

	/* pr_debug("enter %s\n", __func__); */

	mean->g = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_Y__G);
	mean->b = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_CB__B);
	mean->r = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_CR__R);
	mean->no_white_count = isp_read_reg(dev, REG_ADDR(isp_awb_white_cnt));

	return 0;
}


int isp_s_exp(struct isp_ic_dev *dev, struct isp_exp_context *exp)
{
	u32 isp_exp_ctrl = isp_read_reg(dev, REG_ADDR(isp_exp_ctrl));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	pr_debug("enter %s\n", __func__);

	if (!exp->enable) {
		REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_START, 0);
		isp_write_reg(dev, REG_ADDR(isp_exp_ctrl), isp_exp_ctrl);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
			      isp_imsc & ~MRV_ISP_IMSC_EXP_END_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_exp_h_offset),
		      (MRV_AE_ISP_EXP_H_OFFSET_MASK & exp->window.x));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_offset),
		      (MRV_AE_ISP_EXP_V_OFFSET_MASK & exp->window.y));
	isp_write_reg(dev, REG_ADDR(isp_exp_h_size),
		      (MRV_AE_ISP_EXP_H_SIZE_MASK & exp->window.width));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_size),
		      (MRV_AE_ISP_EXP_V_SIZE_MASK & exp->window.height));
#ifdef ISP_AE_SHADOW
	isp_write_reg(dev, REG_ADDR(isp_exp_h_offset_shd),
		      (MRV_AE_ISP_EXP_H_OFFSET_MASK & exp->window.x));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_offset_shd),
		      (MRV_AE_ISP_EXP_V_OFFSET_MASK & exp->window.y));
	isp_write_reg(dev, REG_ADDR(isp_exp_h_size_shd),
		      (MRV_AE_ISP_EXP_H_SIZE_MASK & exp->window.width));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_size_shd),
		      (MRV_AE_ISP_EXP_V_SIZE_MASK & exp->window.height));
#endif
	REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_MEAS_MODE, exp->mode);
	REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_START, 1);
	isp_write_reg(dev, REG_ADDR(isp_exp_ctrl), isp_exp_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_EXP_END_MASK);

	return 0;
}

int isp_g_expmean(struct isp_ic_dev *dev, u8 *mean)
{
	int i = 0;

	/* pr_debug("enter %s\n", __func__); */
	if (!dev || !mean)
		return -EINVAL;
	for (; i < 25; i++) {
		mean[i] = isp_read_reg(dev, REG_ADDR(isp_exp_mean_00) + (i << 2));
	}

	return 0;
}

#ifdef ISP_HIST256
#define HIST_BIN_TOTAL 256
#else
#define HIST_BIN_TOTAL 16
#endif

int isp_s_hist(struct isp_ic_dev *dev)
{
	struct isp_hist_context *hist = &dev->hist;

#ifdef ISP_HIST256
	u32 isp_hist256_prop = isp_read_reg(dev, REG_ADDR(isp_hist256_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	int i;

	if (!hist->enable) {
		REG_SET_SLICE(isp_hist256_prop, MRV_HIST_MODE,
			      MRV_HIST_MODE_NONE);
		isp_write_reg(dev, REG_ADDR(isp_hist256_prop),
			      isp_hist256_prop);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
			      isp_imsc & ~MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_hist256_h_offs),
		      (MRV_HIST_H_OFFSET_MASK & hist->window.x));
	isp_write_reg(dev, REG_ADDR(isp_hist256_v_offs),
		      (MRV_HIST_V_OFFSET_MASK & hist->window.y));
	isp_write_reg(dev, REG_ADDR(isp_hist256_h_size),
		      (MRV_HIST_H_SIZE_MASK & hist->window.width));
	isp_write_reg(dev, REG_ADDR(isp_hist256_v_size),
		      (MRV_HIST_V_SIZE_MASK & hist->window.height));

	for (i = 0; i < 24; i += 4) {
		isp_write_reg(dev, REG_ADDR(isp_hist256_weight_00to30) + i,
			      hist->weight[i +
					   0] | (hist->weight[i +
							      1] << 8) |
			      (hist->weight[i + 2] << 16) | (hist->weight[i +
									  3] <<
							     24));
	}

	isp_write_reg(dev, REG_ADDR(isp_hist256_weight_44), hist->weight[24]);
	REG_SET_SLICE(isp_hist256_prop, MRV_HIST_STEPSIZE, hist->step_size);
	REG_SET_SLICE(isp_hist256_prop, MRV_HIST_MODE, hist->mode);
	isp_write_reg(dev, REG_ADDR(isp_hist256_prop), isp_hist256_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
#else
	u32 isp_hist_prop = isp_read_reg(dev, REG_ADDR(isp_hist_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	int i;

	pr_debug("enter %s\n", __func__);
	if (!hist->enable) {
		REG_SET_SLICE(isp_hist_prop, MRV_HIST_MODE, MRV_HIST_MODE_NONE);
		isp_write_reg(dev, REG_ADDR(isp_hist_prop), isp_hist_prop);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
			      isp_imsc & ~MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_hist_h_offs),
		      (MRV_HIST_H_OFFSET_MASK & hist->window.x));
	isp_write_reg(dev, REG_ADDR(isp_hist_v_offs),
		      (MRV_HIST_V_OFFSET_MASK & hist->window.y));
	isp_write_reg(dev, REG_ADDR(isp_hist_h_size),
		      (MRV_HIST_H_SIZE_MASK & hist->window.width));
	isp_write_reg(dev, REG_ADDR(isp_hist_v_size),
		      (MRV_HIST_V_SIZE_MASK & hist->window.height));

	for (i = 0; i < 24; i += 4) {
		isp_write_reg(dev, REG_ADDR(isp_hist_weight_00to30) + i,
			      hist->weight[i +
					   0] | (hist->weight[i +
							      1] << 8) |
			      (hist->weight[i + 2] << 16) | (hist->weight[i +
									  3] <<
							     24));
	}

	isp_write_reg(dev, REG_ADDR(isp_hist_weight_44), hist->weight[24]);
	REG_SET_SLICE(isp_hist_prop, MRV_HIST_STEPSIZE, hist->step_size);
	REG_SET_SLICE(isp_hist_prop, MRV_HIST_MODE, hist->mode);
	isp_write_reg(dev, REG_ADDR(isp_hist_prop), isp_hist_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
#endif
	return 0;
}

int isp_g_histmean(struct isp_ic_dev *dev, u32 *mean)
{
	int i = 0;

	/* pr_debug("enter %s\n", __func__); */
	if (!dev || !mean)
		return -EINVAL;
#ifdef ISP_HIST256
	for (; i < HIST_BIN_TOTAL; i++) {
		mean[i] = isp_read_reg(dev, REG_ADDR(isp_hist256_bin_n));
	}
#else
	for (; i < HIST_BIN_TOTAL; i++) {
		mean[i] =
		    isp_read_reg(dev,
				 REG_ADDR(histogram_measurement_result_arr[i]) +
				 (i << 2));
	}
#endif
	return 0;
}

int isp_s_dpcc(struct isp_ic_dev *dev)
{
	struct isp_dpcc_context *dpcc = &dev->dpcc;
	const u32 reg_gap = 20;
	int i;
	u32 isp_dpcc_mode = isp_read_reg(dev, REG_ADDR(isp_dpcc_mode));

	pr_debug("enter %s\n", __func__);

	if (!dpcc->enable) {
		REG_SET_SLICE(isp_dpcc_mode, MRV_DPCC_ISP_DPCC_ENABLE, 0);
	} else {
		REG_SET_SLICE(isp_dpcc_mode, MRV_DPCC_ISP_DPCC_ENABLE, 1);
	}

	isp_write_reg(dev, REG_ADDR(isp_dpcc_mode), dpcc->mode);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_output_mode), dpcc->outmode);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_set_use), dpcc->set_use);

	for (i = 0; i < 3; i++) {
		isp_write_reg(dev, REG_ADDR(isp_dpcc_methods_set_1) + (i << 2),
			      0x1FFF & dpcc->methods_set[i]);
		isp_write_reg(dev,
			      REG_ADDR(isp_dpcc_line_thresh_1) + i * reg_gap,
			      0xFFFF & dpcc->params[i].line_thresh);
		isp_write_reg(dev,
			      REG_ADDR(isp_dpcc_line_mad_fac_1) + i * reg_gap,
			      0x3F3F & dpcc->params[i].line_mad_fac);
		isp_write_reg(dev, REG_ADDR(isp_dpcc_pg_fac_1) + i * reg_gap,
			      0x3F3F & dpcc->params[i].pg_fac);
		isp_write_reg(dev,
			      REG_ADDR(isp_dpcc_rnd_thresh_1) + i * reg_gap,
			      0xFFFF & dpcc->params[i].rnd_thresh);
		isp_write_reg(dev, REG_ADDR(isp_dpcc_rg_fac_1) + i * reg_gap,
			      0x3F3F & dpcc->params[i].rg_fac);
	}

	isp_write_reg(dev, REG_ADDR(isp_dpcc_ro_limits), dpcc->ro_limits);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_rnd_offs), dpcc->rnd_offs);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_mode), isp_dpcc_mode);

	return 0;
}

int isp_s_flt(struct isp_ic_dev *dev, struct isp_flt_context *flt)
{
	u32 isp_flt_mode = isp_read_reg(dev, REG_ADDR(isp_filt_mode));
	struct flt_denoise_type {
		u32 thresh_sh0;
		u32 thresh_sh1;
		u32 thresh_bl0;
		u32 thresh_bl1;
		u32 stage_select;
		u32 vmode;
		u32 hmode;
	};

	static struct flt_denoise_type denoise_tbl[] = {
		{0, 0, 0, 0, 6, MRV_FILT_FILT_CHR_V_MODE_STATIC8,
		 MRV_FILT_FILT_CHR_H_MODE_BYPASS},
		{18, 33, 8, 2, 6, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{26, 44, 13, 5, 4, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{36, 51, 23, 10, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{41, 67, 26, 15, 3, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{75, 10, 50, 20, 3, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{90, 120, 60, 26, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{120, 150, 80, 51, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{170, 200, 140, 100, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{250, 300, 180, 150, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{1023, 1023, 1023, 1023, 2, MRV_FILT_FILT_CHR_V_MODE_STATIC12,
		 MRV_FILT_FILT_CHR_H_MODE_DYN_2},
		{1023, 1023, 1023, 1023, 0, MRV_FILT_FILT_CHR_V_MODE_BYPASS,
		 MRV_FILT_FILT_CHR_H_MODE_BYPASS},
	};

	struct flt_sharpen_type {
		u32 fac_sh0;
		u32 fac_sh1;
		u32 fac_mid;
		u32 fac_bl0;
		u32 fac_bl1;
	};

	static struct flt_sharpen_type sharpen_tbl[] = {
		{0x4, 0x4, 0x4, 0x2, 0x0},
		{0x7, 0x8, 0x6, 0x2, 0x0},
		{0xA, 0xC, 0x8, 0x4, 0x0},
		{0xC, 0x10, 0xA, 0x6, 0x2},
		{0x16, 0x16, 0xC, 0x8, 0x4},
		{0x14, 0x1B, 0x10, 0xA, 0x4},
		{0x1A, 0x20, 0x13, 0xC, 0x6},
		{0x1E, 0x26, 0x17, 0x10, 0x8},
		{0x24, 0x2C, 0x1D, 0x15, 0x0D},
		{0x2A, 0x30, 0x22, 0x1A, 0x14},
		{0x30, 0x3F, 0x28, 0x24, 0x20},
	};

	pr_debug("enter %s\n", __func__);
	if (!flt->enable) {
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
		return 0;
	}

	if (flt->denoise >= 0) {
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh0),
			      denoise_tbl[flt->denoise].thresh_sh0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh1),
			      denoise_tbl[flt->denoise].thresh_sh1);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl0),
			      denoise_tbl[flt->denoise].thresh_bl0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl1),
			      denoise_tbl[flt->denoise].thresh_bl1);
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_STAGE1_SELECT,
			      denoise_tbl[flt->denoise].stage_select);
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_CHR_V_MODE,
			      denoise_tbl[flt->denoise].vmode);
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_CHR_H_MODE,
			      denoise_tbl[flt->denoise].hmode);
	}

	if (flt->sharpen >= 0) {
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh0),
			      sharpen_tbl[flt->sharpen].fac_sh0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh1),
			      sharpen_tbl[flt->sharpen].fac_sh1);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_mid),
			      sharpen_tbl[flt->sharpen].fac_mid);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl0),
			      sharpen_tbl[flt->sharpen].fac_bl0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl1),
			      sharpen_tbl[flt->sharpen].fac_bl1);
	}

	REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_MODE,
		      MRV_FILT_FILT_MODE_DYNAMIC);
	isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
	REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
	isp_write_reg(dev, REG_ADDR(isp_filt_lum_weight), 0x00032040);

	return 0;
}

int isp_s_cac(struct isp_ic_dev *dev)
{
	struct isp_cac_context *cac = &dev->cac;
	u32 val = 0;
	u32 isp_cac_ctrl = isp_read_reg(dev, REG_ADDR(isp_cac_ctrl));

	pr_debug("enter %s\n", __func__);

	if (!cac->enable) {
		REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_CAC_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_cac_ctrl), isp_cac_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_H_CLIP_MODE, cac->hmode);
	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_V_CLIP_MODE, cac->vmode);
	isp_write_reg(dev, REG_ADDR(isp_cac_count_start),
		      cac->hstart | (cac->vstart << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_a), cac->ar | (cac->ab << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_b), cac->br | (cac->bb << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_c), cac->cr | (cac->cb << 16));

	REG_SET_SLICE(val, MRV_CAC_X_NS, cac->xns);
	REG_SET_SLICE(val, MRV_CAC_X_NF, cac->xnf);
	isp_write_reg(dev, REG_ADDR(isp_cac_x_norm), val);
	val = 0;
	REG_SET_SLICE(val, MRV_CAC_Y_NS, cac->yns);
	REG_SET_SLICE(val, MRV_CAC_Y_NF, cac->ynf);
	isp_write_reg(dev, REG_ADDR(isp_cac_y_norm), val);
	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_CAC_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_cac_ctrl), isp_cac_ctrl);

	return 0;
}

int isp_s_deg(struct isp_ic_dev *dev)
{
	struct isp_deg_context *deg = &dev->deg;
	int i;
	u32 isp_gamma_dx_lo = 0;
	u32 isp_gamma_dx_hi = 0;
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_debug("enter %s\n", __func__);

	if (!deg->enable) {
		REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_IN_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		return 0;
	}

	for (i = 0; i < 8; i++) {
		isp_gamma_dx_lo |= deg->segment[i] << (i << 2);
		isp_gamma_dx_hi |= deg->segment[i + 8] << (i << 2);
	}

	isp_write_reg(dev, REG_ADDR(isp_gamma_dx_lo), isp_gamma_dx_lo);
	isp_write_reg(dev, REG_ADDR(isp_gamma_dx_hi), isp_gamma_dx_hi);

	for (i = 0; i < 17; i++) {
		isp_write_reg(dev, REG_ADDR(degamma_r_y_block_arr[i]),
			      deg->r[i]);
		isp_write_reg(dev, REG_ADDR(degamma_g_y_block_arr[i]),
			      deg->g[i]);
		isp_write_reg(dev, REG_ADDR(degamma_b_y_block_arr[i]),
			      deg->b[i]);
	}

	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_IN_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	return 0;
}

u32 get_eff_coeff(int decimal)
{
	u32 value = 0;

	if (decimal <= -6) {
		value = 15;
	} else if (decimal <= -3) {
		value = 14;
	} else if (decimal == -2) {
		value = 13;
	} else if (decimal == -1) {
		value = 12;
	} else if (decimal == 0) {
		value = 0;
	} else if (decimal == 1) {
		value = 8;
	} else if (decimal == 2) {
		value = 9;
	} else if (decimal < 6) {
		value = 10;
	} else {
		value = 11;
	}

	return value;
}

int isp_s_ie(struct isp_ic_dev *dev)
{
	struct isp_ie_context *ie = &dev->ie;
	u32 img_eff_ctrl = isp_read_reg(dev, REG_ADDR(img_eff_ctrl));
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 vi_ircl = isp_read_reg(dev, REG_ADDR(vi_ircl));
	u32 img_eff_tint = isp_read_reg(dev, REG_ADDR(img_eff_tint));
	u32 img_eff_color_sel = isp_read_reg(dev, REG_ADDR(img_eff_color_sel));
	u32 mat[9];
	u32 sharpen = 0;
	int i;

	pr_debug("enter %s\n", __func__);

	REG_SET_SLICE(vi_ircl, MRV_VI_IE_SOFT_RST, 1);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	if (!ie->enable) {
		REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_CFG_UPD,
			      MRV_IMGEFF_CFG_UPD_UPDATE);
		REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_BYPASS_MODE,
			      MRV_IMGEFF_BYPASS_MODE_BYPASS);
		REG_SET_SLICE(vi_iccl, MRV_VI_IE_CLK_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
		isp_write_reg(dev, REG_ADDR(img_eff_ctrl), img_eff_ctrl);
		return 0;
	}

	REG_SET_SLICE(vi_ircl, MRV_VI_IE_SOFT_RST, 0);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	REG_SET_SLICE(vi_iccl, MRV_VI_IE_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_EFFECT_MODE, ie->mode);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_FULL_RANGE, ie->full_range);

	for (i = 0; i < 9; i++) {
		mat[i] = get_eff_coeff(ie->m[i]);
	}

	if (ie->mode == MRV_IMGEFF_EFFECT_MODE_SEPIA) {
		img_eff_tint = isp_read_reg(dev, REG_ADDR(img_eff_tint));
		REG_SET_SLICE(img_eff_tint, MRV_IMGEFF_INCR_CR, ie->tint_cr);
		REG_SET_SLICE(img_eff_tint, MRV_IMGEFF_INCR_CB, ie->tint_cb);
		isp_write_reg(dev, REG_ADDR(img_eff_tint), img_eff_tint);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_COLOR_SEL) {
		REG_SET_SLICE(img_eff_color_sel, MRV_IMGEFF_COLOR_SELECTION,
			      ie->color_sel);
		REG_SET_SLICE(img_eff_color_sel, MRV_IMGEFF_COLOR_THRESHOLD,
			      ie->color_thresh);
		isp_write_reg(dev, REG_ADDR(img_eff_color_sel),
			      img_eff_color_sel);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_EMBOSS) {
		isp_write_reg(dev, REG_ADDR(img_eff_mat_1),
			      mat[0] | (mat[1] << 4) | (mat[2] << 8) | (mat[3]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_2),
			      mat[4] | (mat[5] << 4) | (mat[6] << 8) | (mat[7]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_3), mat[8]);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_SKETCH ||
		   ie->mode == MRV_IMGEFF_EFFECT_MODE_SHARPEN) {
		isp_write_reg(dev, REG_ADDR(img_eff_mat_3),
			      (mat[0] << 4) | (mat[1] << 8) | (mat[2] << 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_4),
			      mat[3] | (mat[4] << 4) | (mat[5] << 8) | (mat[6]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_5),
			      mat[7] | (mat[8] << 4));
		REG_SET_SLICE(sharpen, MRV_IMGEFF_SHARP_FACTOR,
			      ie->sharpen_factor);
		REG_SET_SLICE(sharpen, MRV_IMGEFF_CORING_THR,
			      ie->sharpen_thresh);
		isp_write_reg(dev, REG_ADDR(img_eff_sharpen), sharpen);
	}
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_CFG_UPD,
		      MRV_IMGEFF_CFG_UPD_UPDATE);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_BYPASS_MODE,
		      MRV_IMGEFF_BYPASS_MODE_PROCESS);
	isp_write_reg(dev, REG_ADDR(img_eff_ctrl), img_eff_ctrl);

	return 0;
}

int isp_s_vsm(struct isp_ic_dev *dev)
{
	struct isp_vsm_context *vsm = &dev->vsm;
	u32 isp_vsm_mode = isp_read_reg(dev, REG_ADDR(isp_vsm_mode));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	pr_debug("enter %s\n", __func__);

	if (!vsm->enable) {
		REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_EN, 0);
		REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_IRQ_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_vsm_mode), isp_vsm_mode);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
			      isp_imsc & ~MRV_ISP_IMSC_VSM_END_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_vsm_h_offs), vsm->window.x);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_offs), vsm->window.y);
	isp_write_reg(dev, REG_ADDR(isp_vsm_h_size),
		      vsm->window.width & 0xFFFFE);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_size),
		      vsm->window.height & 0xFFFFE);
	isp_write_reg(dev, REG_ADDR(isp_vsm_h_segments), vsm->h_seg);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_segments), vsm->v_seg);
	REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_EN, 1);
	REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_IRQ_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_vsm_mode), isp_vsm_mode);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_VSM_END_MASK);

	return 0;
}

int isp_g_vsm(struct isp_ic_dev *dev, struct isp_vsm_result *vsm)
{
	pr_debug("enter %s\n", __func__);
	vsm->x = isp_read_reg(dev, REG_ADDR(isp_vsm_delta_h));
	vsm->y = isp_read_reg(dev, REG_ADDR(isp_vsm_delta_v));

	return 0;
}

u32 get_afm_shift(u32 count, u32 thresh)
{
	u32 grad = count;
	u32 shift = 0;

	while (grad > (thresh)) {
		++shift;
		grad >>= 1;
	}

	return shift;
}

int isp_s_afm(struct isp_ic_dev *dev)
{
	struct isp_afm_context *afm = &dev->afm;
	u32 mask =
	    (MRV_ISP_IMSC_AFM_FIN_MASK | MRV_ISP_IMSC_AFM_LUM_OF_MASK |
	     MRV_ISP_IMSC_AFM_SUM_OF_MASK);
	u32 max_count = 0;
	u32 lum_shift = 0;
	u32 afm_shift = 0;
	u32 pix_count = 0;
	u32 shift = 0;
	int i;

	u32 isp_afm_ctrl = isp_read_reg(dev, REG_ADDR(isp_afm_ctrl));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	pr_debug("enter %s\n", __func__);

	if (!afm->enable) {
		REG_SET_SLICE(isp_afm_ctrl, MRV_AFM_AFM_EN, 0);
		isp_imsc &= ~mask;
		isp_write_reg(dev, REG_ADDR(isp_afm_ctrl), isp_afm_ctrl);
		isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
		return 0;
	}

	for (i = 0; i < 3; i++) {
		isp_write_reg(dev, REG_ADDR(isp_afm_lt_a) + (i << 3),
			      (afm->window[i].x << 16) | afm->window[i].y);
		isp_write_reg(dev, REG_ADDR(isp_afm_rb_a) + (i << 3),
			      ((afm->window[i].x + afm->window[i].width -
				1) << 16) | ((afm->window[i].y +
					      afm->window[i].height - 1)));
		pix_count = afm->window[i].width * afm->window[i].height;
		if (pix_count > max_count)
			max_count = pix_count;
	}

	max_count >>= 1;
	lum_shift = get_afm_shift(max_count, 65793);
	afm_shift = get_afm_shift(max_count, 16384);
	REG_SET_SLICE(shift, MRV_AFM_LUM_VAR_SHIFT, lum_shift);
	REG_SET_SLICE(shift, MRV_AFM_AFM_VAR_SHIFT, afm_shift);
	isp_write_reg(dev, REG_ADDR(isp_afm_var_shift), shift);
	isp_write_reg(dev, REG_ADDR(isp_afm_thres), afm->thresh);
	REG_SET_SLICE(isp_afm_ctrl, MRV_AFM_AFM_EN, 1);
	isp_imsc |= mask;
	isp_write_reg(dev, REG_ADDR(isp_afm_ctrl), isp_afm_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);

	return 0;
}

int isp_g_afm(struct isp_ic_dev *dev, struct isp_afm_result *afm)
{
	pr_debug("enter %s\n", __func__);
	afm->sum_a = isp_read_reg(dev, REG_ADDR(isp_afm_sum_a));
	afm->sum_b = isp_read_reg(dev, REG_ADDR(isp_afm_sum_b));
	afm->sum_c = isp_read_reg(dev, REG_ADDR(isp_afm_sum_c));
	afm->lum_a = isp_read_reg(dev, REG_ADDR(isp_afm_lum_a));
	afm->lum_b = isp_read_reg(dev, REG_ADDR(isp_afm_lum_b));
	afm->lum_c = isp_read_reg(dev, REG_ADDR(isp_afm_lum_c));

	return 0;
}

int isp_s_simp(struct isp_ic_dev *dev)
{
	struct isp_simp_context *simp = &dev->simp;
	u32 vi_ircl = isp_read_reg(dev, REG_ADDR(vi_ircl));
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 super_imp_ctrl = isp_read_reg(dev, REG_ADDR(super_imp_ctrl));

	pr_debug("enter %s\n", __func__);

	REG_SET_SLICE(vi_ircl, MRV_VI_SIMP_SOFT_RST, 1);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	if (!simp->enable) {
		REG_SET_SLICE(vi_iccl, MRV_VI_SIMP_CLK_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
		return 0;
	}

	REG_SET_SLICE(vi_ircl, MRV_VI_SIMP_SOFT_RST, 0);
	isp_write_reg(dev, REG_ADDR(super_imp_offset_x), simp->x);
	isp_write_reg(dev, REG_ADDR(super_imp_offset_y), simp->y);
	isp_write_reg(dev, REG_ADDR(super_imp_color_y), simp->r);
	isp_write_reg(dev, REG_ADDR(super_imp_color_cb), simp->g);
	isp_write_reg(dev, REG_ADDR(super_imp_color_cr), simp->b);
	REG_SET_SLICE(super_imp_ctrl, MRV_SI_TRANSPARENCY_MODE,
		      simp->transparency_mode);
	REG_SET_SLICE(super_imp_ctrl, MRV_SI_REF_IMAGE, simp->ref_image);
	isp_write_reg(dev, REG_ADDR(super_imp_ctrl), super_imp_ctrl);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);
	REG_SET_SLICE(vi_iccl, MRV_VI_SIMP_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);

	return 0;
}

int isp_s_cproc(struct isp_ic_dev *dev, struct isp_cproc_context *cproc)
{
	u32 vi_ircl = isp_read_reg(dev, REG_ADDR(vi_ircl));
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 cproc_ctrl = isp_read_reg(dev, REG_ADDR(cproc_ctrl));

	pr_debug("enter %s: cproc->enable %d\n", __func__, cproc->enable);
	REG_SET_SLICE(vi_ircl, MRV_VI_CP_SOFT_RST, 1);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	if (!cproc->enable) {
		REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_ENABLE, 0);
		/*      REG_SET_SLICE(vi_iccl, MRV_VI_CP_CLK_ENABLE, 0); */
		/*      isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl); */
		isp_write_reg(dev, REG_ADDR(cproc_ctrl), cproc_ctrl);
		return 0;
	}

	REG_SET_SLICE(vi_ircl, MRV_VI_CP_SOFT_RST, 0);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);
	isp_write_reg(dev, REG_ADDR(cproc_contrast), cproc->contrast);
	isp_write_reg(dev, REG_ADDR(cproc_brightness), cproc->brightness);
	isp_write_reg(dev, REG_ADDR(cproc_saturation), cproc->saturation);
	isp_write_reg(dev, REG_ADDR(cproc_hue), cproc->hue);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_ENABLE, 1);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_C_OUT_RANGE,
		      cproc->c_out_full);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_Y_OUT_RANGE,
		      cproc->y_out_full);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_Y_IN_RANGE, cproc->y_in_full);
	REG_SET_SLICE(vi_iccl, MRV_VI_CP_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
	isp_write_reg(dev, REG_ADDR(cproc_ctrl), cproc_ctrl);

	return 0;
}

int isp_s_elawb(struct isp_ic_dev *dev)
{
	struct isp_elawb_context *elawb = &dev->elawb;
	u32 awb_meas_mode = isp_read_reg(dev, REG_ADDR(awb_meas_mode));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	u32 id = elawb->id;
	u32 data;

	if (!elawb->enable) {
		REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_IRQ_ENABLE, 0);
		REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_EN, 0);
		isp_write_reg(dev, REG_ADDR(awb_meas_mode), awb_meas_mode);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
			      isp_imsc & ~MRV_ISP_IMSC_AWB_DONE_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(awb_meas_h_offs), elawb->window.x);
	isp_write_reg(dev, REG_ADDR(awb_meas_v_offs), elawb->window.y);
	isp_write_reg(dev, REG_ADDR(awb_meas_h_size), elawb->window.width);
	isp_write_reg(dev, REG_ADDR(awb_meas_v_size), elawb->window.height);

	if (id > 0 && id < 9) {
		isp_write_reg(dev, REG_ADDR(awb_meas_center[id - 1].x),
			      elawb->info[id - 1].x);
		isp_write_reg(dev, REG_ADDR(awb_meas_center[id - 1].y),
			      elawb->info[id - 1].y);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a1),
			      elawb->info[id - 1].a1);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a2),
			      elawb->info[id - 1].a2);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a3),
			      elawb->info[id - 1].a3);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a4),
			      elawb->info[id - 1].a4);
		isp_write_reg(dev, REG_ADDR(awb_meas_rmax[id - 1]),
			      elawb->info[id - 1].r_max_sqr);
	}

	data = 0;
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_R, elawb->r);
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_B, elawb->b);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_rb), data);
	data = 0;
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_GR, elawb->gr);
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_GB, elawb->gb);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_g), data);

	REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_IRQ_ENABLE, 1);
	REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_EN, 1);
	isp_write_reg(dev, REG_ADDR(awb_meas_mode), awb_meas_mode);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
		      isp_imsc | MRV_ISP_IMSC_AWB_DONE_MASK);

	return 0;
}

int isp_ioc_qcap(struct isp_ic_dev *dev, void *args)
{
	/* use public VIDIOC_QUERYCAP to query the type of v4l-subdevs. */
	struct v4l2_capability *cap = (struct v4l2_capability *)args;
	strcpy((char *)cap->driver, "viv_isp_subdev");
	return 0;
}

int isp_ioc_g_status(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;
	/* val = isp_read_reg(REG_ADDR(isp_feature_version)); */
	viv_check_retval(copy_to_user(args, &val, sizeof(val)));
	return 0;
}

int isp_ioc_g_feature(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;
#ifdef ISP_WDR3
	val |= ISP_WDR3_SUPPORT;
#endif
#ifdef ISP_WDR_V3
	val |= ISP_WDR3_SUPPORT;
#endif
#ifdef ISP_MIV2
	val |= ISP_MIV2_SUPPORT;
#endif
#ifdef ISP_HDR_STITCH
	val |= ISP_HDR_STITCH_SUPPORT;
#endif
	viv_check_retval(copy_to_user(args, &val, sizeof(val)));

	return 0;
}

int isp_ioc_g_feature_veresion(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;

	/* val = isp_read_reg(REG_ADDR(isp_feature_version)); */
	viv_check_retval(copy_to_user(args, &val, sizeof(val)));

	return 0;
}

long isp_priv_ioctl(struct isp_ic_dev *dev, unsigned int cmd, void *args)
{
	int ret = -1;

	if (!dev) {
		return ret;
	}

	switch (cmd) {
	case ISPIOC_RESET:
		ret = isp_reset(dev);
		break;
	case ISPIOC_WRITE_REG:
		ret = isp_ioc_write_reg(dev, args);
		break;
	case ISPIOC_READ_REG:
		ret = isp_ioc_read_reg(dev, args);
		break;
	case ISPIOC_ENABLE_TPG:
		ret = isp_enable_tpg(dev);
		break;
	case ISPIOC_DISABLE_TPG:
		ret = isp_disable_tpg(dev);
		break;
	case ISPIOC_ENABLE_BLS:
		ret = isp_enable_bls(dev);
		break;
	case ISPIOC_DISABLE_BLS:
		ret = isp_disable_bls(dev);
		break;
	case ISPIOC_START_DMA_READ:
		ret = isp_ioc_start_dma_read(dev, args);
		break;
	case ISPIOC_MI_STOP:
		ret = isp_mi_stop(dev);
		break;
	case ISPIOC_DISABLE_ISP_OFF:
		ret = isp_ioc_disable_isp_off(dev, args);
		break;
	case ISPIOC_ISP_STOP:
		ret = isp_stop_stream(dev);
		break;
	case ISPIOC_ENABLE:
		ret = isp_enable(dev);
		break;
	case ISPIOC_DISABLE:
		ret = isp_disable(dev);
		break;
	case ISPIOC_ENABLE_LSC:
		ret = isp_enable_lsc(dev);
		break;
	case ISPIOC_DISABLE_LSC:
		ret = isp_disable_lsc(dev);
		break;
	case ISPIOC_ENABLE_AWB:
		ret = isp_enable_awb(dev);
		break;
	case ISPIOC_DISABLE_AWB:
		ret = isp_disable_awb(dev);
		break;
	case ISPIOC_ENABLE_WB:
		ret = isp_enable_wb(dev, 1);
		break;
	case ISPIOC_DISABLE_WB:
		ret = isp_enable_wb(dev, 0);
		break;
	case ISPIOC_ENABLE_GAMMA_OUT:
		ret = isp_enable_gamma_out(dev, 1);
		break;
	case ISPIOC_DISABLE_GAMMA_OUT:
		ret = isp_enable_gamma_out(dev, 0);
		break;
	case ISPIOC_S_IS:
		viv_check_retval(copy_from_user
				 (&dev->is, args, sizeof(dev->is)));
		ret = isp_s_is(dev);
		break;
	case ISPIOC_S_CC:
		ret = isp_s_cc(dev, (struct isp_cc_context *)args);
		break;
	case ISPIOC_S_IE:
		viv_check_retval(copy_from_user
				 (&dev->ie, args, sizeof(dev->ie)));
		ret = isp_s_ie(dev);
		break;
	case ISPIOC_S_TPG:
		viv_check_retval(copy_from_user
				 (&dev->tpg, args, sizeof(dev->tpg)));
		ret = isp_s_tpg(dev);
		break;
	case ISPIOC_S_BLS:
		viv_check_retval(copy_from_user
				 (&dev->bls, args, sizeof(dev->bls)));
		ret = isp_s_bls(dev);
		break;
	case ISPIOC_S_MUX:
		viv_check_retval(copy_from_user
				 (&dev->mux, args, sizeof(dev->mux)));
		ret = isp_s_mux(dev);
		break;
	case ISPIOC_S_AWB:
		ret = isp_s_awb(dev, (struct isp_awb_context *)args);
		break;
	case ISPIOC_S_LSC:
		ret = isp_s_lsc(dev, (struct isp_lsc_context *)args);
		break;
	case ISPIOC_S_DPF:
		ret = isp_s_dpf(dev, (struct isp_dpf_context *)args);
		break;
	case ISPIOC_S_EXP:
		ret = isp_s_exp(dev, (struct isp_exp_context *)args);
		break;
	case ISPIOC_S_CNR:
		viv_check_retval(copy_from_user
				 (&dev->cnr, args, sizeof(dev->cnr)));
		ret = isp_s_cnr(dev);
		break;
	case ISPIOC_S_FLT:
		ret = isp_s_flt(dev, (struct isp_flt_context *)args);
		break;
	case ISPIOC_S_CAC:
		viv_check_retval(copy_from_user
				 (&dev->cac, args, sizeof(dev->cac)));
		ret = isp_s_cac(dev);
		break;
	case ISPIOC_S_DEG:
		viv_check_retval(copy_from_user
				 (&dev->deg, args, sizeof(dev->deg)));
		ret = isp_s_deg(dev);
		break;
	case ISPIOC_S_VSM:
		viv_check_retval(copy_from_user
				 (&dev->vsm, args, sizeof(dev->vsm)));
		ret = isp_s_vsm(dev);
		break;
	case ISPIOC_S_AFM:
		viv_check_retval(copy_from_user
				 (&dev->afm, args, sizeof(dev->afm)));
		ret = isp_s_afm(dev);
		break;
	case ISPIOC_S_HDR:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr(dev);
		break;
	case ISPIOC_ENABLE_HDR:
		ret = isp_enable_hdr(dev);
		break;
	case ISPIOC_DISABLE_HDR:
		ret = isp_disable_hdr(dev);
		break;
	case ISPIOC_S_HIST:
		viv_check_retval(copy_from_user
				 (&dev->hist, args, sizeof(dev->hist)));
		ret = isp_s_hist(dev);
		break;
	case ISPIOC_S_DPCC:
		viv_check_retval(copy_from_user
				 (&dev->dpcc, args, sizeof(dev->dpcc)));
		ret = isp_s_dpcc(dev);
		break;
	case ISPIOC_S_WDR3:
		viv_check_retval(copy_from_user
				 (&dev->wdr3, args, sizeof(dev->wdr3)));
		ret = isp_s_wdr3(dev);
		break;
	case ISPIOC_S_SIMP:
		viv_check_retval(copy_from_user
				 (&dev->simp, args, sizeof(dev->simp)));
		ret = isp_s_simp(dev);
		break;
	case ISPIOC_S_COMP:
		viv_check_retval(copy_from_user
				 (&dev->comp, args, sizeof(dev->comp)));
		ret = isp_s_comp(dev);
		break;
	case ISPIOC_S_CPROC:
		ret = isp_s_cproc(dev, (struct isp_cproc_context *)args);
		break;
	case ISPIOC_S_XTALK:
		ret = isp_s_xtalk(dev, (struct isp_xtalk_context *)args);
		break;
	case ISPIOC_S_ELAWB:
		viv_check_retval(copy_from_user
				 (&dev->elawb, args, sizeof(dev->elawb)));
		ret = isp_s_elawb(dev);
		break;
	case ISPIOC_S_INPUT:
		viv_check_retval(copy_from_user
				 (&dev->ctx, args, sizeof(dev->ctx)));
		ret = isp_s_input(dev);
		break;
	case ISPIOC_S_DEMOSAIC:
		viv_check_retval(copy_from_user
				 (&dev->ctx, args, sizeof(dev->ctx)));
		ret = isp_s_demosaic(dev);
		break;
	case ISPIOC_MI_START:
		viv_check_retval(copy_from_user
				 (&dev->mi, args, sizeof(dev->mi)));
		ret = isp_mi_start(dev);
		break;
	case ISPIOC_S_HDR_WB:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr_wb(dev);
		break;
	case ISPIOC_S_HDR_BLS:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr_bls(dev);
		break;
	case ISPIOC_S_GAMMA_OUT:
		ret = isp_s_gamma_out(dev, (struct isp_gamma_out_context *)args);
		break;
	case ISPIOC_SET_BUFFER:
		ret = isp_set_buffer(dev, (struct isp_buffer_context *)args);
		break;
	case ISPIOC_START_CAPTURE:
		ret = isp_start_stream(dev, *((u32 *)args));
		break;
	case ISPIOC_G_AWBMEAN:
		ret = isp_g_awbmean(dev, (struct isp_awb_mean *)args);
		break;
	case ISPIOC_G_EXPMEAN:
		ret = isp_g_expmean(dev, (u8 *)args);
		break;
	case ISPIOC_G_HISTMEAN:
		ret = isp_g_histmean(dev, (u32 *)args);
		break;
	case ISPIOC_G_VSM:
		ret = isp_g_vsm(dev, (struct isp_vsm_result *)args);
		break;
	case ISPIOC_G_AFM:
		ret = isp_g_afm(dev, (struct isp_afm_result *)args);
		break;
	case ISPIOC_G_STATUS:
		ret = isp_ioc_g_status(dev, args);
		break;
	case ISPIOC_G_FEATURE:
		ret = isp_ioc_g_feature(dev, args);
		break;
	case ISPIOC_G_FEATURE_VERSION:
		ret = isp_ioc_g_feature_veresion(dev, args);
		break;
	case VIDIOC_QUERYCAP:
		ret = isp_ioc_qcap(dev, args);
		break;
	default:
		pr_err("unsupported command %d", cmd);
		break;
	}

	return ret;
}
