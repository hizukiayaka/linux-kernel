// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 - 2017 Fuzhou Rockchip Electronics Co., Ltd
 *		Randy Li, <ayaka@soulik.info>
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

#include <linux/dma-buf.h>
#include <linux/dma-iommu.h>
#include <linux/iommu.h>
#include <linux/kref.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

#include "vcodec_iommu_dma.h"

struct vcodec_dma_buffer {
	struct list_head list;
	struct vcodec_dma_session *session;
	/* dma-buf */
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	enum dma_data_direction dir;

	dma_addr_t iova;
	unsigned long size;
	/* Only be used for identifying the buffer */
	int fd;

	struct kref ref;
	struct rcu_head rcu;
};

struct vcodec_dma_session {
	struct list_head buffer_list;
	/* the mutex for the above buffer list */
	struct mutex list_mutex;

	struct device *dev;
};

static struct vcodec_dma_buffer *
vcodec_dma_find_buffer(struct vcodec_dma_session *session, int fd)
{
	struct vcodec_dma_buffer *buffer = NULL;

	list_for_each_entry_rcu(buffer, &session->buffer_list, list) {
		/*
		 * As long as the last reference is hold by the buffer pool,
		 * the same fd won't be assigned to the other application.
		 */
		if (buffer->fd == fd)
			return buffer;
	}

	return NULL;
}

/* Release the buffer from the current list */
static void vcodec_dma_buffer_delete_rcu(struct kref *ref)
{
	struct vcodec_dma_buffer *buffer =
		container_of(ref, struct vcodec_dma_buffer, ref);

	mutex_lock(&buffer->session->list_mutex);
	list_del_rcu(&buffer->list);
	mutex_unlock(&buffer->session->list_mutex);

	dma_buf_unmap_attachment(buffer->attach, buffer->sgt, buffer->dir);
	dma_buf_detach(buffer->dma_buf, buffer->attach);
	dma_buf_put(buffer->dma_buf);
	kfree_rcu(buffer, rcu);
}

int vcodec_dma_release_fd(struct vcodec_dma_session *session, int fd)
{
	struct device *dev = session->dev;
	struct vcodec_dma_buffer *buffer = NULL;

	rcu_read_lock();
	buffer = vcodec_dma_find_buffer(session, fd);
	rcu_read_unlock();
	if (IS_ERR_OR_NULL(buffer)) {
		dev_err(dev, "can not find %d buffer in list to release\n", fd);

		return -EINVAL;
	}

	kref_put(&buffer->ref, vcodec_dma_buffer_delete_rcu);

	return 0;
}

dma_addr_t vcodec_dma_import_fd(struct vcodec_dma_session *session, int fd)
{
	struct vcodec_dma_buffer *buffer = NULL;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct dma_buf *dma_buf;
	int ret = 0;

	if (!session)
		return -EINVAL;

	dma_buf = dma_buf_get(fd);
	if (IS_ERR(dma_buf)) {
		ret = PTR_ERR(dma_buf);
		return ret;
	}

	rcu_read_lock();
	buffer = vcodec_dma_find_buffer(session, fd);
	if (IS_ERR_OR_NULL(buffer)) {
		rcu_read_unlock();
	} else {
		if (buffer->dma_buf == dma_buf) {
			if (kref_get_unless_zero(&buffer->ref)) {
				dma_buf_put(dma_buf);
				rcu_read_unlock();
				return buffer->iova;
			}
		}
		rcu_read_unlock();
		dev_dbg(session->dev,
			"missing the fd %d\n", fd);
		kref_put(&buffer->ref, vcodec_dma_buffer_delete_rcu);
	}

	/* A new DMA buffer */
	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		ret = -ENOMEM;
		goto err;
	}

	buffer->dma_buf = dma_buf;
	buffer->fd = fd;
	/* TODO */
	buffer->dir = DMA_BIDIRECTIONAL;

	kref_init(&buffer->ref);

	attach = dma_buf_attach(buffer->dma_buf, session->dev);
	if (IS_ERR(attach)) {
		ret = PTR_ERR(attach);
		goto fail_out;
	}

	sgt = dma_buf_map_attachment(attach, buffer->dir);
	if (IS_ERR(sgt)) {
		ret = PTR_ERR(sgt);
		goto fail_detach;
	}

	buffer->iova = sg_dma_address(sgt->sgl);
	buffer->size = sg_dma_len(sgt->sgl);

	buffer->attach = attach;
	buffer->sgt = sgt;

	/* Increase the reference for used outside the buffer pool */
	kref_get(&buffer->ref);

	INIT_LIST_HEAD(&buffer->list);

	mutex_lock(&session->list_mutex);
	buffer->session = session;
	list_add_tail_rcu(&buffer->list, &session->buffer_list);
	mutex_unlock(&session->list_mutex);

	return buffer->iova;

fail_detach:
	dma_buf_detach(buffer->dma_buf, attach);
fail_out:
	kfree(buffer);
err:
	dma_buf_put(dma_buf);
	return ret;
}

void vcodec_dma_destroy_session(struct vcodec_dma_session *session)
{
	struct vcodec_dma_buffer *buffer = NULL;

	if (!session)
		return;

	mutex_lock(&session->list_mutex);
	list_for_each_entry_rcu(buffer, &session->buffer_list, list) {
		list_del_rcu(&buffer->list);
		dma_buf_unmap_attachment(buffer->attach, buffer->sgt,
					 buffer->dir);
		dma_buf_detach(buffer->dma_buf, buffer->attach);
		dma_buf_put(buffer->dma_buf);
		kfree_rcu(buffer, rcu);
	}
	mutex_unlock(&session->list_mutex);

	kfree(session);
}

struct vcodec_dma_session *vcodec_dma_session_create(struct device *dev)
{
	struct vcodec_dma_session *session = NULL;

	session = kzalloc(sizeof(*session), GFP_KERNEL);
	if (!session)
		return session;

	INIT_LIST_HEAD(&session->buffer_list);
	mutex_init(&session->list_mutex);

	session->dev = dev;

	return session;
}

void vcodec_iommu_detach(struct vcodec_iommu_info *info)
{
	struct iommu_domain *domain = info->domain;
	struct iommu_group *group = info->group;

	iommu_detach_group(domain, group);
}

int vcodec_iommu_attach(struct vcodec_iommu_info *info)
{
	struct iommu_domain *domain = info->domain;
	struct iommu_group *group = info->group;
	int ret;

	ret = iommu_attach_group(domain, group);
	if (ret)
		return ret;

	return 0;
}

struct vcodec_iommu_info *vcodec_iommu_probe(struct device *dev)
{
	struct vcodec_iommu_info *info = NULL;
	int ret = 0;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		ret = -ENOMEM;
		goto err;
	}

	info->group = iommu_group_get(dev);
	if (!info->group) {
		ret = -EINVAL;
		goto err_free_info;
	}

	info->domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(info->domain)) {
		ret = PTR_ERR(info->domain);
		goto err_put_group;
	}

	return info;

err_put_group:
	iommu_group_put(info->group);
err_free_info:
	kfree(info);
err:
	return ERR_PTR(ret);
}

int vcodec_iommu_remove(struct vcodec_iommu_info *info)
{
	iommu_group_put(info->group);
	kfree(info);

	return 0;
}
