/*
 * Rockchip RK3288 VPU codec driver
 *
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _RK3288_VPU_ENC_JPEG_H_
#define _RK3288_VPU_ENC_JPEG_H_
#include "rockchip_vpu_common.h"

#include <linux/types.h>
#include <linux/sort.h>

int rk3288_vpu_enc_jpeg_init(struct rockchip_vpu_ctx *ctx);

void rk3288_vpu_enc_jpeg_exit(struct rockchip_vpu_ctx *ctx);

void rk3288_vpu_enc_jpeg_run(struct rockchip_vpu_ctx *ctx);

void rk3288_vpu_enc_jpeg_done(struct rockchip_vpu_ctx *ctx,
			  enum vb2_buffer_state result);
#endif
