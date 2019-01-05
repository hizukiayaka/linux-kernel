/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2016 - 2017 Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef _ROCKCHIP_MPP_SERVICE_H_
#define _ROCKCHIP_MPP_SERVICE_H_

struct mpp_service_node;
struct mpp_service;

struct mpp_task;

void mpp_srv_push_pending(struct mpp_service_node *node, struct mpp_task *task);
struct mpp_task *mpp_srv_get_pending_task(struct mpp_service_node *node);

void mpp_srv_run(struct mpp_service_node *node, struct mpp_task *task);
void mpp_srv_done(struct mpp_service_node *node, struct mpp_task *task);
int mpp_srv_abort(struct mpp_service_node *node, struct mpp_task *task);

void mpp_srv_wait_to_run(struct mpp_service_node *node, struct mpp_task *task);
struct mpp_task *mpp_srv_get_cur_task(struct mpp_service_node *node);

int mpp_srv_is_running(struct mpp_service_node *node);

void *mpp_srv_attach(struct mpp_service *pservice, void *data);
void mpp_srv_detach(struct mpp_service_node *node);
#endif
