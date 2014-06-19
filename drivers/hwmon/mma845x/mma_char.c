/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_char.c
  * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA845X char driver interface implementation for accelerometer.
 *
 ******************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
 * PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
 * AS A RESULT, FREESCALE SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH FREESCALE PARTS.


 ******************************************************************************
 * Revision 1.0.0 3/15/2012 First Release;
 ******************************************************************************
*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/err.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <asm/uaccess.h>
#include <linux/slab.h>

#include "mma_regs.h"

#define ACCL_FIFO_SIZE		(32)

/* IOCTL CODES */
#define MMA_GET_THRESHOLD		(0x10001)
#define MMA_SET_THRESHOLD		(0x10002)
#define MMA_SET_THRESHOLD_VALUE		(0x10003)
#define MMA_GET_THRESHOLD_VALUE		(0x10004)
#define MMA_GET_ACCLEROMETER_FLAG	(0x10005)
#define MMA_SET_ACCLEROMETER_FLAG	(0x10006)

/* Function prototypes */
static int mma_open(struct inode * inode, struct file * file);
static ssize_t mma_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
static int mma_release(struct inode *inode, struct file *file);
static long mma_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg);

typedef struct
{
	struct semaphore sem;
	int threshold;
	int read_pos;
}Context_t, *pContext_t;


/* FiFo to store accelerometer data */
AcclData_t AcclDataFifo[ACCL_FIFO_SIZE];
static int writepos;

static DECLARE_WAIT_QUEUE_HEAD(AcclDataQ);

/* 
 * This structure is the file operations structure, which specifies what
 * callbacks functions the kernel should call when a user mode process
 * attempts to perform these operations on the device.
 */
const struct file_operations mma_fops = {
	.owner	 = THIS_MODULE,
	.open	 = mma_open,
	.unlocked_ioctl 	 = mma_ioctl,
	.release = mma_release,
	.read 	 = mma_read,
};
/*!
* This method is used to update preesure FIFO. 
* pbuff : Pointer to the buffer containing pressure data.
* return 0    : After updating values to FIFO  
*/

int UpdateAcclFiFo(void * buff)
{
	pAcclData_t pData = (pAcclData_t)buff;
	/* Copy data to Fifo */
	AcclDataFifo[writepos].x = pData->x;
	AcclDataFifo[writepos].y = pData->y;
	AcclDataFifo[writepos].z = pData->z;
	
	if(++writepos >= ACCL_FIFO_SIZE)
		writepos = 0;

	wake_up(&AcclDataQ);
	return 0;
}

/*!
* This method is used to get the position of read pointer. 
* pos : Pointer to the buffer containing pressure data.
* return    : Position of read pointer 
*/

static inline int GetAvailableData(int pos)
{
	int ret = 0;
	
	ret = (pos <= writepos)?(writepos - pos):(ACCL_FIFO_SIZE - pos + writepos);
	return ret;
}

/*!
* Open call for accelerometer char driver. 
* inode : Pointer to the node to be opened.
* file  : Pointer to file structure.
* return 0    : After successful opening.  
*/

static int mma_open(struct inode * inode, struct file * file)
{
	pContext_t p = file->private_data;

	if (!p) {
		p = kmalloc(sizeof(*p), GFP_KERNEL);
		if (!p)
			return -ENOMEM;

		p->threshold = 5;
		p->read_pos = 0;
		sema_init(&p->sem, 1);
		file->private_data = p;
	}
	return 0;	
}

/*!
* read call for accelerometer char driver. 
* inode    : Pointer to the node to be opened.
* file     : Pointer to file structure.
* command  : contains command for register.
* arg      : contains data required to apply settings specified in cmd.
* return  0      : For successful	 
*/

static long mma_ioctl(struct file *filp, unsigned int command,
		unsigned long arg)
{
	pContext_t dev = filp->private_data;
	u32 threshold = 0;
	
	switch(command)
	{
		case MMA_GET_THRESHOLD:
			{

				if (copy_to_user(&arg, &(dev->threshold), sizeof(u32)))
			       	{
					printk("ioctl, copy to user failed\n");
					return -EFAULT;
				}
			}
			break;

		case MMA_GET_THRESHOLD_VALUE:
			{

				return dev->threshold;
			}
			break;

		case MMA_SET_THRESHOLD:
			{
				if (copy_from_user(&threshold, &arg, sizeof(u32)))
			       	{
					return -EFAULT;
				}
				if(threshold <= 0)
				{
					return -EINVAL;
				}
				
				dev->threshold = threshold;
			}
			break;

		case MMA_SET_THRESHOLD_VALUE:
			{
				threshold = arg;
				if(threshold <= 0)
				{
					printk("Invalid value [0x%x]\r\n", threshold);
					return -EINVAL;
				}
				dev->threshold = threshold;
			}
			break;

		case MMA_GET_ACCLEROMETER_FLAG:
			{

				return GetAcclerometerFlag();
			}
			break;

		case MMA_SET_ACCLEROMETER_FLAG:
			{
				int AcclFlag = arg;

				return SetAcclerometerFlag(AcclFlag);
			}
			break;

		default:
			{
				printk("%s:: Invalid IOCTL [0x%x]\r\n", __func__, command);
			}
	}
	return 0;
}

/*!
* read call for pressure char driver. 
* file  : Pointer to file structure.
* buf 	 : Pointer to user buffer in which data will be read.
* size  : Size of data be read.
* ppos  : Pointer to Offset in the file.
* return  1. no of bytes read - For successful read \
   	     2. ENOMEM - file private data pointer is NULL	 
*/

static ssize_t mma_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	int retval = 0;
	unsigned short nr = 0;
	pContext_t dev = file->private_data;
	int bytestocopy = 0;
	unsigned long bytescopied = 0;
	char __user *buff = buf;

	unsigned short hdr = 0xffff;

	if (!dev) 
	{
		return -ENOMEM;
	}

	/* Acquire semaphore to manage re-entrancy */
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;


	/* Loop till data available has crossed the watermark */
	nr = GetAvailableData(dev->read_pos);
	while (nr < dev->threshold ) 
	{ 
		/* Wait on accelerometer queue (AcclQ) till condition GetAvailableData(dev->read_pos) >= dev->threshold gets satisfied */
		if (wait_event_interruptible(AcclDataQ, 
					(GetAvailableData(dev->read_pos) >= dev->threshold)))
			return -ERESTARTSYS;
		nr = GetAvailableData(dev->read_pos);
	}

	bytescopied = copy_to_user(buff, &hdr, sizeof(unsigned short));
	retval += sizeof(unsigned short);
	buff += sizeof(unsigned short);
	bytescopied = copy_to_user(buff, &nr, sizeof(unsigned short));

	retval += sizeof(unsigned short);
	buff += sizeof(unsigned short);

	/* Loop here to copy bytes to user buffer */
	while(nr)
	{
		if(dev->read_pos + nr >= ACCL_FIFO_SIZE)
		{
			bytestocopy = ACCL_FIFO_SIZE - dev->read_pos ;
		}
		else
		{
			bytestocopy = nr;
		}

		/* Copy the required records to user buffer */
		bytescopied = copy_to_user(buff, &AcclDataFifo[dev->read_pos], bytestocopy * sizeof(AcclData_t));

		retval += bytestocopy * sizeof(AcclData_t);
		buff += bytestocopy * sizeof(AcclData_t);
		
		nr -= bytestocopy;

		/* Increment the read_pos */
		dev->read_pos += bytestocopy;
		if(dev->read_pos >= ACCL_FIFO_SIZE)
			dev->read_pos -= ACCL_FIFO_SIZE;
	}
	/* release the lock */
	up(&dev->sem); 
	/* Return the number of bytes written to buffer */
	return retval;
}
/*!
* Release call for accelerometer char driver. 
* inode : Pointer to the node to be opened.
* file  : Pointer to file structure.
* return 0    : After successful release of resources.  
*/

static int mma_release(struct inode *inode, struct file *file)
{
	pContext_t p = file->private_data;

	if (p) 
	{
		//mutex_destroy(&p->lock);
		kfree(p);
		p = NULL;
	}

	return 0;
}

