/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_input.c
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA845X  input interface implementation for accelerometer
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
#include <asm/uaccess.h>
#include <linux/input.h>
#include "mma_regs.h"
#include <mach/hardware.h>

#define ALT_RIGHT	1	//32	
#define ALT_LEFT	2	//16
#define SHIFT_RIGHT	4	//128
#define SHIFT_LEFT	8	//64
#define SYM		16	//4

struct DirectionLookup{
	int direction;
	int modifier;
	int keyoffset;
};

struct ModifierTable{
	int modifier;
	int keycode;
};

static struct ModifierTable ModifierKeycode[] = {
	{SYM, 0x61},
	{ALT_LEFT, 0x38},
	{ALT_RIGHT, 0x64},
	{SHIFT_RIGHT, 0x36},
	{SHIFT_LEFT, 0x2a},
};

static struct DirectionLookup DirToModifier[] = {
	{ 0,  0 , 0 },
	{ 32, SHIFT_RIGHT |  0 , 0 },
	{ 16, SHIFT_LEFT |  0 , 0 },
	{ 8, ALT_RIGHT |  0 , 0 },
	{ 40, ALT_LEFT |  0 , 0 },
	{ 24, SHIFT_RIGHT | ALT_RIGHT |  0 , 0 },
	{ 4, SHIFT_LEFT | ALT_LEFT |  0 , 0 },
	{ 36, SHIFT_RIGHT | ALT_LEFT |  0 , 0 },
	{ 20, SHIFT_LEFT | ALT_RIGHT |  0 , 0 },
	{ 2, SHIFT_RIGHT |  0 , 1 },
	{ 34, SHIFT_LEFT |  0 , 1 },
	{ 18, ALT_RIGHT |  0 , 1 },
	{ 10, ALT_LEFT |  0 , 1 },
	{ 42, SHIFT_RIGHT | ALT_RIGHT |  0 , 1 },
	{ 26, SHIFT_LEFT | ALT_LEFT |  0 , 1 },
	{ 6, SHIFT_RIGHT | ALT_LEFT |  0 , 1 },
	{ 38, SHIFT_LEFT | ALT_RIGHT |  0 , 1 },
	{ 22, SHIFT_RIGHT | SHIFT_LEFT |  0 , 0 },
	{ 1, SHIFT_RIGHT | SHIFT_LEFT | ALT_LEFT |  0 , 0 },
	{ 33, SHIFT_RIGHT | SHIFT_LEFT | ALT_RIGHT |  0 , 0 },
	{ 17, SHIFT_RIGHT | SHIFT_LEFT | ALT_RIGHT | ALT_LEFT |  0 , 0 },
	{ 9, ALT_RIGHT | ALT_LEFT |  0 , 0 },
	{ 41, SHIFT_LEFT | ALT_RIGHT | ALT_LEFT |  0 , 0 },
	{ 25, SHIFT_RIGHT | ALT_RIGHT | ALT_LEFT |  0 , 0 },
	{ 5, SHIFT_RIGHT | SHIFT_LEFT |  0 , 1 },
	{ 37, SHIFT_RIGHT | SHIFT_LEFT | ALT_LEFT |  0 , 1 },
	{ 21, SHIFT_RIGHT | SHIFT_LEFT | ALT_RIGHT |  0 , 1 },
};

/*!
* This method is used to convert the direction from chip layer to input layer.
* value  : Direction value from chip layer.
* return       : Converted direction value in format required for input layer.
*/
static int GetDirection(int value)
{
	int DirVal = 0;
	if(value & (0x1 << 5))
	{
		DirVal |= ((value & (0x01<<4)) ? 2 : 1) << 4;
	}

	if(value & (0x1 << 3))
	{
		DirVal |= ((value & (0x01<<2)) ? 2 : 1) << 2;
	}

	if(value & (0x1 << 1))
	{
		DirVal |= ((value & (0x01<<0)) ? 2 : 1) << 0;
	}

	return DirVal;
}

static int ReportKeyWithDirection(struct mxc_mma_device_t *pDev, int key, int dir)
{
	int modifier = 0;
	int i = 0;
	int count = (sizeof(DirToModifier)/sizeof(DirToModifier[0]));

	for(i = 0; i < count; i++)
	{
		if(dir == DirToModifier[i].direction)
		{
			modifier = DirToModifier[i].modifier;
			key += DirToModifier[i].keyoffset;
			break;
		}
	}

	if(i >= count)
	{
		printk("### Invalid/New key combination [0x%x]\r\n", dir);
		return 0;
	}

	count = sizeof(ModifierKeycode)/sizeof(ModifierKeycode[0]);

	for(i = 0; i < count; i++)
	{
		if(modifier & ModifierKeycode[i].modifier)
			input_event(pDev->inp2, EV_KEY, ModifierKeycode[i].keycode, 1);
	}

	input_event(pDev->inp2, EV_KEY, key, 1);
	input_event(pDev->inp2, EV_KEY, key, 0);
	
	for(i = count-1; i >= 0; i--)
	{
		if(modifier & ModifierKeycode[i].modifier)
			input_event(pDev->inp2, EV_KEY, ModifierKeycode[i].keycode, 0);
	}
	return 0;
}

/*!
* This method is used to initialize input interface.
* dev  : Pointer to device structure.
* return 0   : After successful registration of input device.
*/

int InitializeInputInterface(struct mxc_mma_device_t *pDev)
{
	int RetVal = 0;
	int i = 0;

	if(pDev == NULL)
	{
		printk("%s: pDev => NULL pointer\r\n", __func__);
		return -EPERM;
	}

	pDev->inp1 = input_allocate_device();

	if (!pDev->inp1)
       	{
		RetVal = -ENOMEM;
		printk(KERN_ERR
		       "%s: Failed to allocate input device-1\n", __func__);
		return RetVal;
	}

	set_bit(EV_ABS, pDev->inp1->evbit);						// Accelerometer readings

	/* yaw */
	input_set_abs_params(pDev->inp1, ABS_RX, 0, 360, 0, 0);
	/* pitch */
	input_set_abs_params(pDev->inp1, ABS_RY, -180, 180, 0, 0);
	/* roll */
	input_set_abs_params(pDev->inp1, ABS_RZ, -90, 90, 0, 0);

	/* x-axis acceleration */
	input_set_abs_params(pDev->inp1, ABS_X, -32768, 32767, 0, 0);
	/* y-axis acceleration */
	input_set_abs_params(pDev->inp1, ABS_Y, -32768, 32767, 0, 0);
	/* z-axis acceleration */
	input_set_abs_params(pDev->inp1, ABS_Z, -32768, 32767, 0, 0);

	pDev->inp1->name = "mma845x";

	RetVal = input_register_device(pDev->inp1);

	if (RetVal) 
	{
		printk(KERN_ERR "%s: Unable to register input device: %s\n",__func__, pDev->inp1->name);
		return RetVal;
	}
	
	/* Register input device 2 */
	pDev->inp2 = input_allocate_device();

	if (!pDev->inp2)
       	{
		RetVal = -ENOMEM;
		printk(KERN_ERR "%s: Failed to allocate input device-2\n", __func__);
		return RetVal;
	}

	/* Initialize all event codes as this is a configurable param and may change runtime from user space */
	for(i = 0x20; i < 0x40; i++)
		input_set_abs_params(pDev->inp2, i, 0, 255, 0, 0);

	pDev->inp2->mscbit[0] = BIT_MASK(MSC_RAW) | BIT_MASK(MSC_SCAN);


	pDev->inp2->name = "Accl1";

	set_bit(EV_ABS, pDev->inp2->evbit);
	set_bit(EV_KEY, pDev->inp2->evbit);
	set_bit(EV_MSC, pDev->inp2->evbit);

	RetVal = input_register_device(pDev->inp2);

	bitmap_fill(pDev->inp2->keybit, KEY_MAX);

	if (RetVal) 
	{
		printk(KERN_ERR "%s: Unable to register input device: %s\n", __func__, pDev->inp2->name);
		return RetVal;
	}
	return RetVal;	
}

/*!
* This method is used to deinitialize input interface.
* dev  : Pointer to device structure.
*/

int DeInitializeInputInterface(struct mxc_mma_device_t *pDev)
{
	if(pDev == NULL)
	{
		printk("%s: pDev => NULL pointer\r\n", __func__);
		return -EPERM;
	}

	if(pDev->inp1)
	{
		input_unregister_device(pDev->inp1);
		input_free_device(pDev->inp1);
	}
	
	if(pDev->inp2)
	{
		input_unregister_device(pDev->inp2);
		input_free_device(pDev->inp2);
	}
	return 0;
}

/*!
* This method is used to report event.
* dev   : Pointer to device structure.
* type  : Event type.
* buff  : Pointer to buffer.
* return 0
*/

int ReportEvent(struct mxc_mma_device_t *pDev, int type, void *buff)
{
	if(pDev == NULL)
		return -ENOMEM;

	switch(type)
	{
		case ACCL_DATA:
			{
				pAcclData_t pData = (pAcclData_t)buff;

				if(pDev->inp1 && pData)
				{
					if(pData->x != 0xffff)
						input_report_abs(pDev->inp1, ABS_X, pData->x);
					if(pData->y != 0xffff)
						input_report_abs(pDev->inp1, ABS_Y, pData->y);
					if(pData->z != 0xffff)
						input_report_abs(pDev->inp1, ABS_Z, pData->z);
					input_sync(pDev->inp1);
#ifdef DEBUG				
					printk("X: [%04x] Y:[%04x] Z: [%04x]\r\n", pData->x, pData->y, pData->z);
#endif
				}
			}
			break;

		case ACCL_LNDPRT:
			{
				u32 * pData = (u32 *)buff;

				if(pDev->inp1 && pData)
				{
					input_report_abs(pDev->inp1, ABS_RX, *pData);
					pData++;
					input_report_abs(pDev->inp1, ABS_RY, *pData);
					pData++;
					input_report_abs(pDev->inp1, ABS_RZ, *pData);
					pData++;
					if(*pData & (0x01<<0))
					{
						// "Back" 
						input_event(pDev->inp2, EV_KEY, 0x68, *pData);
						input_event(pDev->inp2, EV_KEY, 0x68, 0);
					}
					else{
						// "Front"
						input_event(pDev->inp2, EV_KEY, 0x6b, *pData);
						input_event(pDev->inp2, EV_KEY, 0x6b, 0);
					}

					if(*pData & (0x01<<5))
					{
						input_report_abs(pDev->inp2, pDev->ornt_event_type, (u8 )*pData);
						switch((*pData & 0x06) >> 1)
						{
							case 0:
								input_event(pDev->inp2, EV_KEY, 0x67, *pData);
								input_event(pDev->inp2, EV_KEY, 0x67, 0);
								break;
							case 1:
								input_event(pDev->inp2, EV_KEY, 0x6c, *pData);
								input_event(pDev->inp2, EV_KEY, 0x6c, 0);
								break;
							case 2:
								input_event(pDev->inp2, EV_KEY, 0x6a, *pData);
								input_event(pDev->inp2, EV_KEY, 0x6a, 0);
								break;
							case 3:
								input_event(pDev->inp2, EV_KEY, 0x69, *pData);
								input_event(pDev->inp2, EV_KEY, 0x69, 0);
								break;
						}

					}
					input_sync(pDev->inp1);
					input_sync(pDev->inp2);
#ifdef DEBUG				
					printk("X: [%04x] Y:[%04x] Z: [%04x]\r\n", pData->x, pData->y, pData->z);
#endif
				}
			}
			break;

		case ACCL_FF_MT:
			{
				int value = (int)(*(char *)buff);
				int dir = GetDirection(value);
				ReportKeyWithDirection(pDev, 0x1e, dir);

			}
			break;

		case ACCL_PULSE:
			{
				int value = (int)(*(char *)buff);
				int dir = GetDirection(value);				
				if(pDev->inp2)
				{
					if(value & (0x01<<7))
					{
						if(value & (0x01 << 6))
						{
							ReportKeyWithDirection(pDev, 0x1c, dir);

							input_report_abs(pDev->inp2, pDev->event_type_double, value);
							input_report_abs(pDev->inp2, pDev->event_type_double, 0);
						}
						else
						{
							ReportKeyWithDirection(pDev, 0x20, dir);
							input_report_abs(pDev->inp2, pDev->event_type_single, value);
							input_report_abs(pDev->inp2, pDev->event_type_single, 0);
						}
						input_sync(pDev->inp2);
					}
				}
			}
			break;

		case ACCL_TRANS:
			{
				int value = (int)(*(char *)buff);
				int dir = GetDirection(value);
				if(pDev->inp2)
				{
					ReportKeyWithDirection(pDev, 0x22, dir);
					input_report_abs(pDev->inp2, pDev->trans_event_type, value);
					input_report_abs(pDev->inp2, pDev->trans_event_type, 0);
					input_sync(pDev->inp2);
				}
			}
			break;

		default:
			printk("%s:: Unhandled input type\r\n", __func__);
			break;
	}

	return 0;
}


