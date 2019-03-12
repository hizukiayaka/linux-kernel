// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Randy Li, <ayaka@soulik.info>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/types.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>

#include "rbsp.h"

int rkvdec_hevc_write_sps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_sps *sps);

int rkvdec_hevc_write_pps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_pps *pps);

int rkvdec_hevc_write_rps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_pps *pps);
