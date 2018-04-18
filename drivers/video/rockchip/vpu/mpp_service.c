// SPDX-License-Identifier: GPL-2.0-or-later
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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/slab.h>

#include "mpp_dev_common.h"
#include "mpp_service.h"

struct mpp_service {
	/* service critical time lock */
	struct completion running;
	struct mpp_task *cur_task;

	u32 dev_cnt;
	struct list_head subdev_list;
};

struct mpp_service_node {
	/* node structure global lock */
	struct mutex lock;
	struct mpp_service *parent;
	struct list_head pending;
};

/* service queue schedule */
void mpp_srv_push_pending(struct mpp_service_node *node, struct mpp_task *task)
{
	mutex_lock(&node->lock);
	list_add_tail(&task->service_link, &node->pending);
	mutex_unlock(&node->lock);
}
EXPORT_SYMBOL(mpp_srv_push_pending);

struct mpp_task *mpp_srv_get_pending_task(struct mpp_service_node *node)
{
	struct mpp_task *task = NULL;

	mutex_lock(&node->lock);
	if (!list_empty(&node->pending)) {
		task = list_first_entry(&node->pending, struct mpp_task,
					service_link);
		list_del_init(&task->service_link);
	}
	mutex_unlock(&node->lock);

	return task;
}
EXPORT_SYMBOL(mpp_srv_get_pending_task);

int mpp_srv_is_running(struct mpp_service_node *node)
{
	struct mpp_service *pservice = node->parent;

	return !try_wait_for_completion(&pservice->running);
}
EXPORT_SYMBOL(mpp_srv_is_running);

void mpp_srv_wait_to_run(struct mpp_service_node *node, struct mpp_task *task)
{
	struct mpp_service *pservice = node->parent;

	wait_for_completion(&pservice->running);
	pservice->cur_task = task;
}
EXPORT_SYMBOL(mpp_srv_wait_to_run);

struct mpp_task *mpp_srv_get_cur_task(struct mpp_service_node *node)
{
	struct mpp_service *pservice = node->parent;

	return pservice->cur_task;
}
EXPORT_SYMBOL(mpp_srv_get_cur_task);

void mpp_srv_done(struct mpp_service_node *node, struct mpp_task *task)
{
	struct mpp_service *pservice = node->parent;

	pservice->cur_task = NULL;
	complete(&pservice->running);
}
EXPORT_SYMBOL(mpp_srv_done);

int mpp_srv_abort(struct mpp_service_node *node, struct mpp_task *task)
{
	struct mpp_service *pservice = node->parent;

	if (task) {
		if (pservice->cur_task == task)
			pservice->cur_task = NULL;
	}
	complete(&pservice->running);

	return 0;
}
EXPORT_SYMBOL(mpp_srv_abort);

void *mpp_srv_attach(struct mpp_service *pservice, void *data)
{
	struct mpp_service_node *node = NULL;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return node;

	node->parent = pservice;
	mutex_init(&node->lock);
	INIT_LIST_HEAD(&node->pending);

	return node;
}
EXPORT_SYMBOL(mpp_srv_attach);

void mpp_srv_detach(struct mpp_service_node *node)
{
	kfree(node);
}
EXPORT_SYMBOL(mpp_srv_detach);

static void mpp_init_drvdata(struct mpp_service *pservice)
{
	init_completion(&pservice->running);
	complete(&pservice->running);
}

static int mpp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mpp_service *pservice = devm_kzalloc(dev, sizeof(*pservice),
						    GFP_KERNEL);
	if (!pservice)
		return -ENOMEM;

	mpp_init_drvdata(pservice);

	platform_set_drvdata(pdev, pservice);
	dev_info(dev, "init success\n");

	return 0;
}

static int mpp_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mpp_service_dt_ids[] = {
	{ .compatible = "rockchip,mpp-service", },
	{ },
};

static struct platform_driver mpp_driver = {
	.probe = mpp_probe,
	.remove = mpp_remove,
	.driver = {
		.name = "mpp",
		.of_match_table = of_match_ptr(mpp_service_dt_ids),
	},
};

static int __init mpp_service_init(void)
{
	int ret = platform_driver_register(&mpp_driver);

	if (ret) {
		pr_err("Platform device register failed (%d).\n", ret);
		return ret;
	}

	return ret;
}

static void __exit mpp_service_exit(void)
{
}

module_init(mpp_service_init);
module_exit(mpp_service_exit)
MODULE_LICENSE("GPL");
