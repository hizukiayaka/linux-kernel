/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_sysfs.c
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA845X  methods and data structure related sysfs module
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

#include <linux/module.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>

#include "mma_regs.h"
#include <mach/hardware.h>
#include "mma845x.h"


#define LEN(x)	(sizeof(x)/sizeof(x[0]))

static char * DeviceState[] =
{
	"standby",
	"sleep",
	"wake",
	"off",
	NULL
};

/*!
* This method is used to parse the string according to delimeter.
* Data        : Pointer to buffer containing string to be parse.
* length      : Length of string in byte.
* Delimeter   : A punctuation character to separates to names.
* Tokens      : A token fo rparsing the string.
* Tokencnt    : Length of token string .
* return TokenCount : no. of token occurances found in input string.
 */
int ParseString(char *Data, int Length, char Delimiter, char *Tokens[], int TokenCnt)
{
	int TokenCount = 0;
	int Iterator;

	TokenCnt--;
	Tokens[TokenCount++] = Data;
	for(Iterator = 0; (Iterator < Length) && TokenCnt; Iterator++)
	{
		if(Data[Iterator] == Delimiter)
		{
			Data[Iterator] = '\0';
			Tokens[TokenCount] = &Data[Iterator + 1];
			TokenCount++;
			TokenCnt--;
		}
	}
	Iterator = 0;
	while(Iterator < TokenCount)
	{
		Iterator++;
	}

	return TokenCount;
}

/*!
* This method is used to get FIXPOINT number from string.
* buf      : Pointer to string.
* val      : Fixpoint variable in which fixpoint num will store.
* return -EINVAL : If string doesn't contain any number.
* return 0       : After successful conversion.
*/

FIXPOINT strict_strtofp(const char *buf, FIXPOINT *val)
{
	char * arr[2];
	FIXPOINT c = 0;
	unsigned long a = 0, b = 0;
	int ret, i, div = 0, len = 0;
	int prec = 3, scale = 1;

	ret = ParseString((char *) buf, strlen(buf), '.', arr, 2);
	if(ret == 0) 
	{
		return -EINVAL;
	}
	else
       	{
		if(strict_strtoul(arr[0], 10, &a) < 0)
		return -EINVAL;
	}
	if(ret < 2) 
	{
		b = 0;
	}
	else 
	{
		if(strict_strtoul(arr[1], 10, &b) < 0)
			return -EINVAL;
		
		len = strlen(arr[1]);
		if(arr[1][len-1] == '\n')
			len--;
		if(len > prec)
		{
			len = len - prec;
			div = 1;
		}
		else 
		{
			len = prec - len;
			div = 0;
		}
		for(i = 0; i < len; i++)
			scale *= 10;

		b = div ? (b / scale) : (b * scale);
	}
	c = (a * 1000) + b; 
	*val = c;
	return 0;
}

/* COMMON Attributes */
/*!
* This method is used to show name of chip.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
 */

ssize_t name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int RetVal = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(pDev)
	sprintf(buf, "%s\n", pChip->name);

	RetVal = strlen(buf) + 1;
	return RetVal;
}

/*!
* This method is used to show vendor.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
 */

ssize_t vendor_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s\n", VENDOR_NAME);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show device ID.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trgger set value.
* return     : Length of string.
*/

ssize_t devid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(pDev)
		sprintf(buf, "0x%x\n", pChip->ChipId);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show version.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	if(pDev)
		sprintf(buf, "%d", pDev->version);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show type.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d", pChip->devtype);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximun range.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t max_range_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d.%d m/s^2\n", FIXP_INT_PART(pChip->maxrange), FIXP_DEC_PART(pChip->maxrange));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximun resolution.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t max_res_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d.%d m/s^2\n", FIXP_INT_PART(pChip->maxres), FIXP_DEC_PART(pChip->maxres));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show norminal power.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t nominal_power_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	sprintf(buf, "%d\n", pChip->pNominal_powerList[pChip->odr]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show operation mode.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t operation_mode_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	sprintf(buf, "%s\n", DeviceState[pChip->state]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store operation mode.
* dev  		: Pointer to device structure.
* attr 		: Pointer to device attributes.
* buf  		: Pointer to buffer containing trigger set value.
* count		: Length of string.
* return -EINVAL	: For invalid operation mode. 
* return     		: Length of string.
*/

ssize_t operation_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = 0;
	int i = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	for(i = 0; DeviceState[i] != NULL; i++)
	{
		if(!strncmp(DeviceState[i], buf, count-1))
			break;
	}
	
	if(DeviceState[i] != NULL)
	{
		printk("Setting device state to %d (%s)\r\n", i, DeviceState[i]);
		// Set operational mode
		ret = pChip->SetRegVal(CMD_MODE, i);
		pChip->state = i;
		ret = count;
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

/*!
* This method is used to show Output Data Rate. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t odr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d.%d\n", FIXP_INT_PART(pChip->pOdrList[pChip->odr]), FIXP_DEC_PART(pChip->pOdrList[pChip->odr]));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Output data Rate. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t odr_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;
	size_t len = 0; 
	char tmpBuf[8] = "";
	
	while(pChip->pOdrList[i] != -1)
	{
		len = sprintf(tmpBuf, "%d.%d", FIXP_INT_PART(pChip->pOdrList[i]), FIXP_DEC_PART(pChip->pOdrList[i]));
		if(0 == strncmp((const char *)tmpBuf, buf, min(len, count)))
		{
			// Set ODR here
			pChip->SetRegVal(CMD_ODR, i);
			pChip->odr = i;
			return count;
		}
		i++;
	}

	return -EINVAL;
}

/*!
* This method is used to show supported Output Data Rate.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t supported_odr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	ssize_t ret = 0;
	int len = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pOdrList[i] != -1)
	{
		printk("%d I[%d] D[%d]\r\n", pChip->pOdrList[i], FIXP_INT_PART(pChip->pOdrList[i]), FIXP_DEC_PART(pChip->pOdrList[i]));
		sprintf(&buf[len], "%d.%d;", FIXP_INT_PART(pChip->pOdrList[i]), FIXP_DEC_PART(pChip->pOdrList[i]));
		len = strlen(buf);
		i++;
	}
	buf[len - 1] = '\0';

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show oversampling. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t oversampling_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", pChip->pSamplingList[pChip->oversampling]);

	ret = strlen(buf) + 1;
	return ret;

}

/*!
* This method is used to store oversampling. 
* dev  	   : Pointer to device structure.
* attr 	   : Pointer to device attributes.
* buf  	   : Pointer to buffer containing trigger set value.
* count	   : Length of string.
* return -EINVAL  : If string doesn't contain any number.
* return     	   : Length of string.
*/

ssize_t oversampling_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;
	int ret = 0;

	for(i = 0; pChip->pSamplingList[i] != NULL; i++)
	{
		if(!strncmp(pChip->pSamplingList[i], buf, count-1))
		break;
	}
	
	if(pChip->pSamplingList[i] != NULL)
	{
		printk("Setting device state to %d (%s)\r\n", i, pChip->pSamplingList[i]);
		// Set operational mode
		ret = pChip->SetRegVal(CMD_SAMPLE, i);
		pChip->oversampling = i;
		ret = count;
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

/*!
* This method is used to show oversampling values. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t oversampling_values_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0,len =0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pSamplingList[i] != NULL)
	{
		sprintf(&buf[len],"%s;",pChip->pSamplingList[i]);
		len = strlen(buf);
		i++;
	}

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show auto wakeup. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t auto_wakeup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->wakeup == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store auto wakeup. 
* dev  	  : Pointer to device structure.
* attr 	  : Pointer to device attributes.
* buf  	  : Pointer to buffer containing trigger set value.
* count	  : Length of string.
* return -EINVAL : Invalid auto_wakeup value.
* return     	  : Length of string.
*/

ssize_t auto_wakeup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("Autowakeup enable\r\n");
		pChip->SetRegVal(CMD_WAKEUP, 1);
		pChip->wakeup = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("Autowakeup disable\r\n");
		pChip->SetRegVal(CMD_WAKEUP, 0);
		pChip->wakeup = 0;
	}
	else
	{
		printk("Invalid auto_wakeup value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show resolution. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t resolutions_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int len = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pSupportedResolution[i] != -1)
	{
		sprintf(&buf[len], "%d;",pChip->pSupportedResolution[i]);
		len = strlen(buf);
		printk("pChip->pSupportedResolution[i] = %d \r\n",pChip->pSupportedResolution[i]);
		i++;
	}

	buf[len - 1] = '\0';
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store resolution. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL : Invalid Resolution.
* return     : Length of string.
*/

ssize_t resolution_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;
	unsigned long val = 0;

	strict_strtoul(buf, 10, &val);
	for(i = 0; pChip->pSupportedResolution[i] != -1; i++)
	{
		if(val == pChip->pSupportedResolution[i])
		{
			// Set resolution here
			printk("Setting resolution to %d (%d)\r\n", i, pChip->pSupportedResolution[i]);
			pChip->SetRegVal(CMD_RESOLUTION, i);
			pChip->resolution = i;
			return count;
		}
	}

	printk("Invalid Resolution, (buf: %s count: %d)\n", buf, count);
	return -EINVAL;
}

/*!
* This method is used to show resolution. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t resolution_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->pSupportedResolution[pChip->resolution]);

	ret = strlen(buf) + 1;
	return ret;
}


/*!
* This method is used to store range. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL : Invlaid Range.
* return     : Length of string.
*/

ssize_t range_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;
	size_t len = 0; 
	char tmpBuf[8] = "";

	while(pChip->pSupportedRange[i] != -1)
	{
		len = sprintf(tmpBuf, "%d.%d", FIXP_INT_PART(pChip->pSupportedRange[i]), FIXP_DEC_PART(pChip->pSupportedRange[i]));
		if(0 == strncmp(tmpBuf, buf, min(len, count)))
		{
			//  Set range here
			printk("Setting range to %d (%d.%d)\r\n", i,  FIXP_INT_PART(pChip->pSupportedRange[i]), FIXP_DEC_PART(pChip->pSupportedRange[i]));
			pChip->SetRegVal(CMD_RANGE, i);
			pChip->range = i;
			return count;
		}
		i++;
	}

	printk("Invalid Range, (buf: %s count: %d)\n", buf, count);
	return -EINVAL;
}

/*!
* This method is used to show range. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t range_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	ssize_t ret = 0;
	int i = 0;
	int len = 0; 
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "-%d.%d:%d.%d;", FIXP_INT_PART(pChip->pSupportedRange[pChip->range]), FIXP_DEC_PART(pChip->pSupportedRange[pChip->range]),
			FIXP_INT_PART(pChip->pSupportedRange[pChip->range]), FIXP_DEC_PART(pChip->pSupportedRange[pChip->range]));
	len = strlen(buf);

	for(i = 0; pChip->pSupportedRange[i] != -1; i++)
	{
		sprintf(&buf[len],"-%d.%d:%d.%d,", FIXP_INT_PART(pChip->pSupportedRange[i]), FIXP_DEC_PART(pChip->pSupportedRange[i]),
				FIXP_INT_PART(pChip->pSupportedRange[i]), FIXP_DEC_PART(pChip->pSupportedRange[i]));
		
		len = strlen(buf);
	}
	
	buf[len - 1] = '\n';
	buf[len] = '\0';
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show low range. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t range_low_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "-%d.%d\n", FIXP_INT_PART(pChip->pSupportedRange[pChip->range]), FIXP_DEC_PART(pChip->pSupportedRange[pChip->range]));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show high range. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t range_high_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d.%d\n", FIXP_INT_PART(pChip->pSupportedRange[pChip->range]), FIXP_DEC_PART(pChip->pSupportedRange[pChip->range]));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show precision value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t precision_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT precision;

	precision = (pChip->pSupportedRange[pChip->range] * FLOAT_TO_FIXP(1) * 1000) / (0x1 << (pChip->pSupportedResolution[pChip->resolution] - 1));
	sprintf(buf, "%d.%03d mg\n", FIXP_INT_PART(precision), FIXP_DEC_PART(precision));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show sample rate. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sample_rate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d.%d\n", FIXP_INT_PART(pChip->pOdrList[pChip->odr]), FIXP_DEC_PART(pChip->pOdrList[pChip->odr]));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

// TODO:Set value here
	// sprintf(buf, "%d\n", pChip->value);
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to store calibration offset. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL : Invalid calibration offset value.
* return     : Length of string.
*/

ssize_t calibration_offset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i=0;
	char *params[3];
	unsigned long val[3];

	ParseString((char *) buf, strlen(buf), ',', params, 3);
	while(params[i] != NULL && i < 3)
	{ 
		if (strict_strtoul(params[i], 10, &val[i]))
			return -EINVAL;
		printk("%lu \r\n", val[i]);
		if(val[i] > 255)
		{
			printk("Invalid calibration offset_ value(buf: %s count: %d)\n", buf, count);
			return -EINVAL;
		}
		i++;
	}
	pChip->SetCalOffset((int *)val);
	pChip->xCalOffset = val[0];	
	pChip->yCalOffset = val[1];	
	pChip->zCalOffset = val[2];	
	return count;
}

/*!
* This method is used to show calibration offset. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t calibration_offset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d,%d,%d\n", pChip->xCalOffset,pChip->yCalOffset,pChip->zCalOffset);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show fifo enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fifo_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->enablefifo == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store fifo enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL : Invalid fifo_enable value. 
* return     : Length of string.
*/

ssize_t fifo_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int ret = 0;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("FIFO enable\r\n");
		ret = pChip->SetRegVal(CMD_ENFIFO, 1);
		if(ret < 0)
			return -EINVAL;
		pChip->enablefifo = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("FIFO disable\r\n");
		ret = pChip->SetRegVal(CMD_ENFIFO, 0);
		if(ret < 0)
			return -EINVAL;
		pChip->enablefifo = 0;
	}
	else
	{
		printk("Invalid fifo_enable value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to store fifo threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return -EINVAL : Value is outof fifo_threshold range.
* return     : Length of string.
*/

ssize_t fifo_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	unsigned long val;
	int ret = 0;

	strict_strtoul(buf, 10, &val);
	if(val < pChip->min_fifo_th || val > pChip->max_fifo_th)
		return -EINVAL;

	ret = pChip->SetRegVal(CMD_FIFO_TH, val);
	if(ret < 0)
		return -EINVAL;

	pChip->fifo_threshold = val;
	return count;
}

/*!
* This method is used to show fifo threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fifo_threshold_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->fifo_threshold);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show fifo threshold minimum value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fifo_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->min_fifo_th);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show fifo threshold maximum value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fifo_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->max_fifo_th);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Freefall and Motion Detection threshold value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return -EINVAL  : Invalid Transient Detection threshold value. 
* return     : Length of string.
*/
ssize_t fm_threshold_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	char *params[2];
	FIXPOINT val[2];
	FIXPOINT threshold = 0;

	ParseString((char *)buf, strlen(buf), ';', params, 2);
	if(strict_strtofp(params[0], &val[0]) < 0)
		return -EINVAL;

	threshold = (FIXPOINT)( (long)(val[0] * 127)  / (8 * FLOAT_TO_FIXP(9.8)));
	printk("%s: >>> Setting FM threshold to %d\r\n", __func__, threshold);
	if((threshold < 1) || (threshold > 127 ))
	{
		printk("Invalid Trans threshold value");
		return -EINVAL;
	}
	val[0] = threshold;
	val[1] = 0;
	if(!strncmp(params[1], "OR", strlen("OR")))
	{
		val[1] = 1;
	}
	else if(!strncmp(params[1], "AND", strlen("AND")))
	{
		val[1] = 0;
	}

	printk("Setting fm threshold to %d LOGIC: [%d] %s\r\n", val[0], val[1], params[1] != NULL ? params[1] : "null");
	pChip->fm_threshold = val[0];
	pChip->fm_threshold_logic = val[1];
	pChip->SetRegVal(CMD_FM_THS, val[0]);
	return count;
}

/*!
* This method is used to show Freefall and Motion Detection threshold value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT threshold;
	
	threshold = (pChip->fm_threshold *((1 * 8 * FLOAT_TO_FIXP(9.8)) / 127));
	sprintf(buf, "%d.%03d;%s\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold), pChip->fm_threshold_logic ? "OR" : "AND");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum Freefall and Motion Detection threshold value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 ) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum Freefall and Motion Detection threshold value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8));
	if(pChip->oversampling == 1)
		threshold /= 2;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum Freefall and Motion Detection threshold step value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_threshold_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8)) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show Freefall and Motion Detection threshold value. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_threshold_num_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%d\n", 127);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Freefall and Motion Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* ccount: Length of string. 
* return -EINVAL: Invalid debounce count. 
* return     : Length of string.
*/

ssize_t fm_debounce_count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t ccount)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	char *params[2] = {NULL, NULL};
	unsigned int val[2];
	unsigned int count = pChip->fm_debounce_count;
	int func = 0;
	
	ParseString((char *)buf, strlen(buf), ';', params, 2);
		if(strict_strtoul(params[0], 10,  (unsigned long *) &val[0])<0)
	{
		printk("invalid\n");
		return -EINVAL;
	}
	if((val[0] < 0) || (val[0] > 255 ))
	{
		printk("Invalid debounce count");
		return -EINVAL;
	}

	count = val[0];
	if(count < pChip->Debounce_Count_Min)
		count = pChip->Debounce_Count_Min;

	if(params[1] != NULL)	
	{
		if(!strncmp(params[1], "decr", strlen("decr")))
		{
			// Decrement
			func = 0;
					}
		else if(!strncmp(params[1], "set_zero", strlen("set_zero")))
		{
			// Set zero
			func = 1;
		}
	}
	printk("Setting fm debounce count to %d function;%s\r\n", val[0] , (params[1] != NULL ? params[1] : "null"));

	pChip->SetRegVal(CMD_FM_DEBOUNCE, count);
	pChip->SetRegVal(CMD_FM_DEBOUNCE_MODE,func);

	pChip->fm_debounce_count = count;
	pChip->fm_debounce_function = func;
	
	return ccount;
}

/*!
* This method is used to show Freefall and Motion Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_debounce_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pFm_debounce_countList + (4 *(pChip->odr) + pChip->oversampling));
	//Formula = BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	DebounceCount = (pChip->fm_debounce_count * DebounceCount);

	sprintf(buf,"%d.%03d;%s \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount),(pChip->fm_debounce_function ? "set_zero": "decr"));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minumun Freefall and Motion Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_debounce_count_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pFm_debounce_countList + (4 *(pChip->odr) + pChip->oversampling));
	DebounceCount =  pChip->Debounce_Count_Min * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;

}

/*!
* This method is used to show maxmimum Freefall and Motion Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_debounce_count_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pFm_debounce_countList + (4 *(pChip->odr) + pChip->oversampling));
	DebounceCount =  pChip->Debounce_Count_Max * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show Freefall and Motion Detection debounce time step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_debounce_time_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;	
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pFm_debounce_countList + (4 *(pChip->odr) + pChip->oversampling));
	//Formula = BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Freefall and Motion Detection event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return -EINVAL: value is outof range. 
* return     : Length of string.
*/

ssize_t fm_event_type_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	unsigned long val;

	strict_strtoul(buf, 10, &val);
	if(val < 0 || val > 255)
		return -EINVAL;

	pDev->fm_event_type = val;
	return count;
}

/*!
* This method is used to show Freefall and Motion Detection event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_event_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", pDev->fm_event_type);
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to store Freefall and Motion Detection enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid FM_enable value.
* return     : Length of string.
*/

ssize_t fm_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	char *params[3];
	int en_val = 0;
	int i = 0;
	if( ParseString((char *) buf, strlen(buf), ',', params, 3) < 3)
		return -EINVAL;

	for(i = 0; i < 3; i++)
	{
		 if(!strncmp(params[i], "enable", strlen("enable")))
		{
			printk("FM detection enable\r\n");
			en_val |= 0x1 << i;
		}

		else if(!strncmp(params[i], "disable", strlen("disable")))
		{
			printk("FM detection disable\r\n");
			en_val &= ~(0x1 << i);
		}
		else
		{
			printk("Invalid FM_enable value %s\n", params[i]);
			return -EINVAL;
		}
	}
	printk("Fm enable = 0x%02x\n",en_val);
	pChip->SetRegVal(CMD_FMENABLE, en_val);
	pChip->fm_enable = en_val;

	return count;
}

/*!
* This method is used to show Freefall and Motion Detection enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	bool x_en;
	bool y_en;
	bool z_en;

	x_en = !!(pChip->fm_enable & 0x01);
	y_en = !!(pChip->fm_enable & 0x02);
	z_en = !!(pChip->fm_enable & 0x04);

	sprintf(buf, "%s,%s,%s\n", (x_en == 1)? "enable" : "disable",(y_en == 1)? "enable" : "disable",(z_en == 1)? "enable" : "disable");
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to store Freefall and Motion Detection wake on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid fm_wakeon_event value.
* return     : Length of string.
*/

ssize_t fm_wake_on_event_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("fm_wake_on_event enable\r\n");
		pChip->SetRegVal(CMD_FM_WAKE, 1);
		pChip->fm_wakeon_event = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("fm_wake_on_event disable\r\n");
		pChip->SetRegVal(CMD_FM_WAKE, 0);
		pChip->fm_wakeon_event = 0;
	}
	else
	{
		printk("Invalid fm_wakeon_event value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show Freefall and Motion Detection wake on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t fm_wake_on_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->fm_wakeon_event == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Transient Detection cut-off frequencies. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t trans_cut_off_frequencies_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT val = 0;
	FIXPOINT *freqList;
	int i = 4;

	if(strict_strtofp(buf, &val) < 0)
		return -EINVAL;

	freqList =  pChip->pTrans_cut_off_frequenciesList + (pChip->oversampling * 32);
       	freqList += (pChip->odr)* 4;

	while(i--)
	{
	    	if(*freqList == val)
			break;

	     	freqList++;
	}

	if(i < 0)
	{
		printk("Invalid !!!\r\n");
		return -EINVAL;
	}

	i = 3-i;
	pChip->SetRegVal(CMD_TRANS_CUTOFF, i);
	pChip->cutoff_freq_sel = i;

	printk("Setting cut off frequencies to %d (%d.%d)\r\n", i, FIXP_INT_PART(*freqList), FIXP_DEC_PART(*freqList));

	return count;
}

/*!
* This method is used to show Transient Detection cut-off frequencies. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_cut_off_frequencies_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int i=0,len =0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT * freqList = 0;
	FIXPOINT val;

	freqList =  pChip->pTrans_cut_off_frequenciesList + (pChip->oversampling * 32);
        freqList += (pChip->odr)* 4;

	sprintf(buf,"%d.%03d:", FIXP_INT_PART(*(freqList + pChip->cutoff_freq_sel)), FIXP_DEC_PART(*(freqList + pChip->cutoff_freq_sel)));

	len = strlen(buf);
	for(i = 0; i < 4; i++)
	{
		val = *(freqList + i);
		sprintf(&buf[len],"%d.%03d,", FIXP_INT_PART(val), FIXP_DEC_PART(val));
		len = strlen(buf);
	}

	len = strlen(buf);
	buf[len - 1] = '\n';
	buf[len] = '\0';
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Transient Detection threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid Transient Detection threshold value 
* return     : Length of string.
*/

ssize_t trans_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT val;
	FIXPOINT threshold = 0;

	if(strict_strtofp(buf, &val) < 0)
		return -EINVAL;

	threshold = (FIXPOINT)( (long)(val * 127)  / (8 * FLOAT_TO_FIXP(9.8)));
	
	if((threshold < 1) || (threshold > 127 ))
	{
		printk("Invalid Trans threshold value");
		return -EINVAL;
	}

	val = threshold;

	printk("%s: Setting transient threshold to %d\r\n", __func__, threshold);
	pChip->SetRegVal(CMD_TRANS_THS, val);
	pChip->trans_threshold = val;
	return count;
}

/*!
* This method is used to show Transient Detection threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT threshold;

	threshold = (pChip->trans_threshold *((1 * 8 * FLOAT_TO_FIXP(9.8)) / 127));
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum Transient Detection threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8)) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum Transient Detection threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8));
	if(pChip->oversampling == 1)
		threshold /= 2;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show Transient Detection threshold step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_threshold_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8)) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));
	ret = strlen(buf) + 1;

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show Transient Detection threshold number step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_threshold_num_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%d\n", 127);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Transient Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* ccount: Length of string.
* return -EINVAL: Invalid debounce count
* return     : Length of string.
*/

ssize_t trans_debounce_count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t ccount)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	char *params[2] = {NULL, NULL};
	unsigned int val[2];
	unsigned int count = pChip->trans_debounce_count;
	int func = -1;
		
	ParseString((char *)buf, strlen(buf), ';', params, 2);
	if(strict_strtoul(params[0], 10, (unsigned long *) &val[0])<0)
	{
		printk("invalid\n");
		return -EINVAL;
	}
	if((val[0] < 0) || (val[0] > 255 ))
	{
		printk("Invalid debounce count");
		return -EINVAL;
	}
	count = val[0];
	if(count < pChip->Debounce_Count_Min)
		count = pChip->Debounce_Count_Min;

	if(params[1] != NULL)	
	{
		if(!strncmp(params[1], "decr", strlen("decr")))
		{
			// Decrement
			func = 0;
		}
		else if(!strncmp(params[1], "set_zero", strlen("set_zero")))
		{
			// Set zero
			func = 1;
		}
	}

	printk("Setting trans debounce count to %d function; %s\r\n", val[0], (params[1] != NULL ? params[1] : "null"));

	pChip->SetRegVal(CMD_TRANS_DEBOUNCE, count);
	pChip->SetRegVal(CMD_TRANS_DEBOUNCE_MODE,func);

	pChip->trans_debounce_count = count;
	pChip->trans_debounce_function = func;


	return ccount;		
}

/*!
* This method is used to show Transient Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_debounce_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *(pChip->pTrans_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	DebounceCount = (pChip->trans_debounce_count * DebounceCount);

	sprintf(buf,"%d.%03d;%s \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount),(pChip->trans_debounce_function ? "set_zero": "decr"));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show mimimum Transient Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_debounce_count_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *(pChip->pTrans_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	DebounceCount =  pChip->Debounce_Count_Min * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum Transient Detection debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_debounce_count_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *(pChip->pTrans_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	DebounceCount =  pChip->Debounce_Count_Max * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show Transient Detection debounce time step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_debounce_time_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	FIXPOINT DebounceCount = 0; 
	DebounceCount = *(pChip->pTrans_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Transient Detection event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Value is outof range. 
* return     : Length of string.
*/

ssize_t trans_event_type_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	unsigned long val;

	strict_strtoul(buf, 10, &val);
	if(val < 0 || val > 255)
		return -EINVAL;
	pDev->trans_event_type = val;

	return count;
}

/*!
* This method is used to show Transient Detection event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_event_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", pDev->trans_event_type);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Transient Detection enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid Trans_enable value. 
* return     : Length of string.
*/

ssize_t trans_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	char *params[3];
	int en_val = 0;
	int i = 0;
	if( ParseString((char *)buf, strlen(buf), ',', params, 3) < 3)
		return -EINVAL;

	for(i = 0; i < 3; i++)
	{
		 if(!strncmp(params[i], "enable", strlen("enable")))
		{
			printk("Trans detection enable\r\n");
			en_val |= 0x1 << i;
		}

		else if(!strncmp(params[i], "disable", strlen("disable")))
		{
			printk("Trans detection disable\r\n");
			en_val &= ~(0x1 << i);
		}
		else
		{
			printk("Invalid Trans_enable value %s\n", params[i]);
			return -EINVAL;
		}
	}
	printk("reg value after enable = %d\n",en_val);
	pChip->SetRegVal(CMD_TRANS_ENABLE, en_val);
	pChip->trans_enable = en_val;

	return count;
}

/*!
* This method is used to show Transient Detection enable.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	bool x_en;
	bool y_en;
	bool z_en;

	x_en = !!(pChip->trans_enable & 0x01);
	y_en = !!(pChip->trans_enable & 0x02);
	z_en = !!(pChip->trans_enable & 0x04);
	sprintf(buf, "%d,%d,%d \n",x_en,y_en,z_en);
	sprintf(buf, "%s,%s,%s\n", (x_en == 1)? "enable" : "disable",(y_en == 1)? "enable" : "disable",(z_en == 1)? "enable" : "disable");
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to show Transient Detection wake on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid trans_wakeon_event value.
* return     : Length of string.
*/

ssize_t trans_wake_on_event_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	if(!strncmp(buf, "enable", count-1))
	{
		printk("trans_wake_on_event enable\r\n");
		pChip->SetRegVal(CMD_TRANS_WAKE, 1);
		pChip->trans_wakeon_event = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("trans_wake_on_event disable\r\n");
		pChip->SetRegVal(CMD_TRANS_WAKE, 0);
		pChip->trans_wakeon_event = 0;
	}
	else
	{
		printk("Invalid trans_wakeon_event value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show Transient Detection wake on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trans_wake_on_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->trans_wakeon_event == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store z_lock angle threshold.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	int zlock_angle = 0;
	int i=0;

	if(strict_strtofp(buf, &zlock_angle) < 0)
		return -EINVAL;
	zlock_angle = zlock_angle/1000;
	/* Check if threshold is within limits */
	if(zlock_angle < pChip->zlock_ths_min || zlock_angle > pChip->zlock_ths_max)
	{
		printk("%s:: Out of range [%d] > %d < [%d]\r\n", __func__, pChip->zlock_ths_min, zlock_angle, pChip->zlock_ths_max);
		return -EINVAL;
	}

	for(i = 0; pChip->pZ_lock_angle_thresholdList[i] != -1; i++)
	{
		if(zlock_angle == pChip->pZ_lock_angle_thresholdList[i])
			{
			// Set z lock angle threshold here
			printk("Setting z lock angle threshold to %d (%d)\r\n", i, pChip->pZ_lock_angle_thresholdList[i]);
			pChip->SetRegVal(CMD_Z_LOCK_ANGLE, i);
			pChip->zlock_angle_ths = i;
			return count;
			}
	}
	return -EINVAL;
}

/*!
* This method is used to show z_lock angle threshold.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	sprintf(buf, "%d\n", pChip->pZ_lock_angle_thresholdList[pChip->zlock_angle_ths]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum z_lock angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n",  pChip->zlock_ths_min);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum z_lock angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n",  pChip->zlock_ths_max);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show z_lock angle threshold values. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_values_show(struct device *dev,struct device_attribute *attr,char *buf)
{
	ssize_t ret = 0;
	int len = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pZ_lock_angle_thresholdList[i] != -1)
	{
		sprintf(&buf[len], "%d;",pChip->pZ_lock_angle_thresholdList[i]);
		len = strlen(buf);
		i++;

	}
	buf[len - 1] = '\0';
	len = strlen(buf);
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to show lock angle threshold number. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t z_lock_angle_threshold_num_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->zlock_angle_step);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store back front trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	int bf_trip;
	int i =0;

	if(strict_strtofp(buf, &bf_trip) < 0)
		return -EINVAL;

	bf_trip = bf_trip/1000;
	/* Check if value is valid */
	if(bf_trip < pChip->bf_trip_min || bf_trip > pChip->bf_trip_max)
	{
		printk("%s:: Out of range [%d] > %d < [%d]\r\n", __func__, pChip->bf_trip_min, bf_trip, pChip->bf_trip_max);
		return -EINVAL;
	}
	for(i = 0; pChip->pBack_front_trip_angle_thresholdList[i] != -1; i++)
	{
		if(bf_trip == pChip->pBack_front_trip_angle_thresholdList[i])
			{
				// Set back front trip angle threshold here
				printk("Setting back front trip angle threshold to %d (%d)\r\n", i, pChip->pBack_front_trip_angle_thresholdList[i]);
				pChip->SetRegVal(CMD_TRIP_ANGLE, i);
				pChip->bf_trip_angle = i;
				return count;
			}
	}
	return -EINVAL;
}

/*!
* This method is used to show back front trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->pBack_front_trip_angle_thresholdList[pChip->bf_trip_angle]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum back front trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->bf_trip_min);
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to show maximum back front trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->bf_trip_max);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show back front trip angle threshold.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d\n", pChip->bf_trip_angle_step);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show back front trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t back_front_trip_angle_threshold_num_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	sprintf(buf, "%d\n", pChip->bf_trip_step);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t trip_angle_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	int i = 0;
	int trip_angle;

	if(strict_strtofp(buf, &trip_angle) < 0)
		return -EINVAL;

	trip_angle = trip_angle/1000;
	/* Check if threshold is within limits */
	if(trip_angle < pChip->trip_angle_min || trip_angle > pChip->trip_angle_max)
	{
		printk("%s:: Out of range [%d] > %d < [%d]\r\n", __func__, pChip->trip_angle_min, trip_angle, pChip->trip_angle_max);
		return -EINVAL;
	}

	for(i = 0; pChip->pTrip_angle_thresholdList[i] != -1; i++)
	{
		if(trip_angle == pChip->pTrip_angle_thresholdList[i])
		{
			// Set resolution here
			printk("Setting trip angle threshold to %d (%d)\r\n", i, pChip->pTrip_angle_thresholdList[i]);
			pChip->SetRegVal(CMD_TRIP_ANGLE_TH, i);
			pChip->trip_angle_threshold = i;
			return count;
		}
	}
	return -EINVAL;
}

/*!
* This method is used to show trip angle threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d;", pChip->pTrip_angle_thresholdList[pChip->trip_angle_threshold]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show trip angle threshold values. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_threshold_values_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int len = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pTrip_angle_thresholdList[i] != -1)
	{
		sprintf(&buf[len], "%d;",pChip->pTrip_angle_thresholdList[i]);
		len = strlen(buf);
		i++;

	}
	buf[len - 1] = '\0';
	len = strlen(buf);
	ret = strlen(buf) + 1;

	return ret;
}

/*!
* This method is used to show trip angle threshold number steps. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_threshold_num_steps_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int i=0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	while(pChip->pTrip_angle_thresholdList[i] != -1)
	i++;
	sprintf(buf,"%d\n",i);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store trip angle hysteresis. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t trip_angle_hysteresis_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	int i = 0;
	int trip_angle_hys;

	if(strict_strtofp(buf, &trip_angle_hys) < 0)
		return -EINVAL;

	trip_angle_hys = trip_angle_hys/1000;
	/* Check if threshold is within limits */
	if(trip_angle_hys < pChip->trip_angle_hys_min || trip_angle_hys > pChip->trip_angle_hys_max)
	{
		printk("%s:: Out of range [%d] > %d < [%d]\r\n", __func__, pChip->trip_angle_hys_min, trip_angle_hys, pChip->trip_angle_hys_max);
		return -EINVAL;
	}

	for(i = 0; pChip->pTrip_angle_hysteresisList[i] != -1; i++)
	{
		if(trip_angle_hys == pChip->pTrip_angle_hysteresisList[i])
			{
				//Set trip angle hysteresis here
				printk("Setting trip angle hysteresis %d (%d)\r\n", i, pChip->pTrip_angle_hysteresisList[i]);
				pChip->SetRegVal(CMD_TRIP_ANGLE_HYS, i);
				pChip->trip_angle_hysteresis = i;
				return count;
			}
	}
	return -EINVAL;
}

/*!
* This method is used to show trip angle hysteresis. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_hysteresis_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%d;", pChip->pTrip_angle_hysteresisList[pChip->trip_angle_hysteresis]);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show trip anble hysteresis values. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_hysteresis_values_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int len = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	int i = 0;

	while(pChip->pTrip_angle_hysteresisList[i] != -1)
	{
		sprintf(&buf[len], "%d;",pChip->pTrip_angle_hysteresisList[i]);
		len = strlen(buf);
		i++;
	}

	buf[len - 1] = '\0';
	len = strlen(buf);
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show trip angle hysteresis number steps. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t trip_angle_hysteresis_num_steps_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	int i=0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	 
	while(pChip->pTrip_angle_hysteresisList[i] != -1)
	i++;
	sprintf(buf,"%d\n",i);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store ornt debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* ccount: Length of string.
* return -EINVAL: Invalid debounce count. 
* return     : Length of string.
*/

ssize_t ornt_debounce_count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t ccount)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	char *params[2] = {NULL, NULL};
	unsigned int val[2];
	unsigned int count = pChip->ornt_debounce_count;
	int func =0;

	ParseString((char *)buf, strlen(buf), ';', params, 2);
	if(strict_strtoul(params[0], 10, (unsigned long *)&val[0])<0)
	{
		printk("invalid\n");
		return -EINVAL;
	}
	if((val[0] < 0) || (val[0] > 255 ))
	{
		printk("Invalid debounce count");
		return -EINVAL;
	}
	count = val[0];
	if(count < pChip->Debounce_Count_Min)
		count = pChip->Debounce_Count_Min;

	if(params[1] != NULL)	
	{
		if(!strncmp(params[1], "decr", strlen("decr")))
		{
			// Decrement
			func = 0;
					}
		else if(!strncmp(params[1], "set_zero", strlen("set_zero")))
		{
			// Set zero
			func = 1;
		}
	}

	printk("Setting orientation debounce count to %d function; %s\r\n", val[0], (params[1] != NULL ? params[1] : "null"));

	pChip->SetRegVal(CMD_ORNT_DEBOUNCE, count);
	pChip->SetRegVal(CMD_ORNT_DEBOUNCE_MODE,func);

	pChip->ornt_debounce_count = count;
	pChip->ornt_debounce_function = func;
	
	return ccount;
}

/*!
* This method is used to show ornt debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_debounce_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pOrnt_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	DebounceCount = (pChip->ornt_debounce_count * DebounceCount);

	sprintf(buf,"%d.%03d;%s \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount),(pChip->ornt_debounce_function ? "set_zero": "decr"));
	
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum ornt debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_debounce_count_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pOrnt_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	DebounceCount =  pChip->Debounce_Count_Min * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum ornt debounce count. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_debounce_count_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT DebounceCount = 0; 

	DebounceCount = *( pChip->pOrnt_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	DebounceCount =  pChip->Debounce_Count_Max * DebounceCount ;
	
	sprintf(buf,"%d.%03d \n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show ornt debounce time step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_debounce_time_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	
	FIXPOINT DebounceCount = 0; 
	DebounceCount = *( pChip->pOrnt_debounce_countList + (4 * (pChip->odr) + pChip->oversampling)); 
	//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(DebounceCount), FIXP_DEC_PART(DebounceCount));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store ornt event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t ornt_event_type_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	unsigned long val;

	strict_strtoul(buf, 10, &val);
	if(val < 0 || val > 255)
		return -EINVAL;

	pDev->ornt_event_type = val;

	return count;
}

/*!
* This method is used to show ornt event type. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_event_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", pDev->ornt_event_type);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store ornt enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count:  Length of string.
* return -EINVAL: Invalid ornt_enable value. 
* return     : Length of string.
*/

ssize_t ornt_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("Orientation enable\r\n");
		pChip->SetRegVal(CMD_ORNT_ENABLE, 1);
		pChip->ornt_enable = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("Orientation disable\r\n");
		pChip->SetRegVal(CMD_ORNT_ENABLE, 0);
		pChip->ornt_enable = 0;
	}
	else
	{
		printk("Invalid ornt_enable value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show ornt enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->ornt_enable == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store Orientation Detection wake on event.  
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return -EINVAL: Invalid ornt_wakeon_event value. 
* return     : Length of string.
*/

ssize_t ornt_wake_on_event_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("ornt_wakeon_event enable\r\n");
		pChip->SetRegVal(CMD_ORNT_WAKE, 1);
		pChip->ornt_wakeon_event = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("ornt_wakeon_event disable\r\n");
		pChip->SetRegVal(CMD_ORNT_WAKE, 0);
		pChip->ornt_wakeon_event = 0;
	}
	else
	{
		printk("Invalid ornt_wakeon_event value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show Orientation Detection wake on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t ornt_wake_on_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->ornt_wakeon_event == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum sentap threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sentap_threshold_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8)) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum sentap threshold. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sentap_threshold_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8));
	if(pChip->oversampling == 1)
		threshold /= 2;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show sentap threshold step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sentap_threshold_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	FIXPOINT threshold;
	
	threshold = (1 * 8 * FLOAT_TO_FIXP(9.8)) / 127;
	sprintf(buf, "%d.%03d\n", FIXP_INT_PART(threshold), FIXP_DEC_PART(threshold));
	
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show sentap threshold num step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sentap_threshold_num_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%d\n", 127);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store low pass filter. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return -EINVAL  : Invalid Low Pass Filter value. 
* return     : Length of string.
*/

ssize_t low_pass_filter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("LPF enable\r\n");
		pChip->SetRegVal(CMD_LPF_ENABLE, 1);
		pChip->LPF = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("LPF disable\r\n");
		pChip->SetRegVal(CMD_LPF_ENABLE, 0);
		pChip->LPF = 0;
	}
	else
	{
		printk("Invalid LPF value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show low pass filter. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t low_pass_filter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->LPF == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;	
}

/*!
* This method is used to store pulse time limit.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string.
* return     : Length of string.
*/

ssize_t pulse_time_limit_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT val = 0;
	FIXPOINT PulseCount = 0;
	FIXPOINT PulseTimeStep = 0; 

	if(pChip->LPF == 1)
	{
		PulseTimeStep = *( pChip->pPulse_time_limitList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       				//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseTimeStep = *( pChip->pPulse_time_limitList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	if(strict_strtofp(buf, &val) < 0)
		return -EINVAL;

	PulseCount = (val/ PulseTimeStep);
	if((PulseCount <0) | (PulseCount > 255))
	return -EINVAL;

	pChip->SetRegVal(CMD_PULSE_LIMIT, PulseCount);
	pChip->pulse_time_limit = PulseCount;

	printk("Setting pulse count to %d \n Setting pulse limit to(%d.%d)\r\n",PulseCount , FIXP_INT_PART(val), FIXP_DEC_PART(val));

	return count;
}

/*!
* This method is used to show pulse time limit. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_time_limit_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	size_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT pulsetime  = 0;

	FIXPOINT PulseTimeLimit = 0; 
	if(pChip->LPF == 1)
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	pulsetime = (PulseTimeLimit * pChip->pulse_time_limit);
	sprintf(buf,"%d.%03d", FIXP_INT_PART(pulsetime), FIXP_DEC_PART(pulsetime));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum pulse time limit. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_time_limit_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT PulseTimeLimit = 0;

	if(pChip->LPF == 1)
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(PulseTimeLimit), FIXP_DEC_PART(PulseTimeLimit));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum pluse time limit. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_time_limit_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT max;
	FIXPOINT PulseTimeLimit = 0;

	if(pChip->LPF == 1)
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(PulseTimeLimit), FIXP_DEC_PART(PulseTimeLimit));
		
	max = 255 * PulseTimeLimit;

	sprintf(buf,"%d.%d\n",FIXP_INT_PART(max), FIXP_DEC_PART(max));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show pulse time limit setp. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_time_limit_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT PulseTimeLimit = 0;
	if(pChip->LPF == 1)
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseTimeLimit = *( pChip->pPulse_time_limitList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(PulseTimeLimit), FIXP_DEC_PART(PulseTimeLimit));
		
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store pulse latency time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return     : Length of string.
*/

ssize_t pulse_latency_time_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT val = 0;
	FIXPOINT PulseCount = 0;
	FIXPOINT pulse_latency = 0;

	if(pChip->LPF == 1)
	{
		pulse_latency = *( pChip->pPulse_latency_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		pulse_latency = *( pChip->pPulse_latency_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
       	}

	if(strict_strtofp(buf, &val) < 0)
		return -EINVAL;

	PulseCount = (val/ pulse_latency );
	if((PulseCount <0) |(PulseCount > 255))
	return -EINVAL;

	pChip->SetRegVal(CMD_PULSE_LATENCY, PulseCount);
	pChip->pulse_latency_time = PulseCount;

	printk("Setting pulse count to %d \n Setting pulse latency time to(%d.%d)\r\n",PulseCount , FIXP_INT_PART(val), FIXP_DEC_PART(val));

	return count;
}
  
/*!
* This method is used to show pulse latency time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_latency_time_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	size_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT pulselatency = 0;
	FIXPOINT pulse_latency_step = 0;

	if(pChip->LPF == 1)
	{
		pulse_latency_step = *( pChip->pPulse_latency_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		pulse_latency_step = *( pChip->pPulse_latency_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
       	}

	pulselatency = (pulse_latency_step * pChip->pulse_latency_time);
	sprintf(buf,"%d.%03d", FIXP_INT_PART(pulselatency), FIXP_DEC_PART(pulselatency));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum pulse latency time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_latency_time_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT Pulse_latency_time = 0;

	if(pChip->LPF == 1)
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(Pulse_latency_time), FIXP_DEC_PART(	Pulse_latency_time));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum pluse time limit. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_latency_time_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT max;
	FIXPOINT Pulse_latency_time = 0;

	if(pChip->LPF == 1)
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(Pulse_latency_time), FIXP_DEC_PART(	Pulse_latency_time));
		
	max = 255 * Pulse_latency_time;

	sprintf(buf,"%d.%d\n",FIXP_INT_PART(max), FIXP_DEC_PART(max));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show pulse time limit setp. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_latency_time_step_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT Pulse_latency_time = 0;

	if(pChip->LPF == 1)
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		Pulse_latency_time = *( pChip->pPulse_latency_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(Pulse_latency_time), FIXP_DEC_PART(	Pulse_latency_time));

	ret = strlen(buf) + 1;
	return ret;
}


/*!
* This method is used to store pulse window time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return     : Length of string.
*/

ssize_t pulse_window_time_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT val = 0;
	FIXPOINT PulseCount = 0;
	FIXPOINT PulseWindowStep = 0; 

	if(pChip->LPF == 1)
	{
		PulseWindowStep = *( pChip->pPulse_window_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseWindowStep  = *( pChip->pPulse_window_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
        }
	
	if(strict_strtofp(buf, &val) < 0)
		return -EINVAL;


	PulseCount = (val/ PulseWindowStep);
	if((PulseCount <0) | (PulseCount > 255))
	return -EINVAL;

	pChip->SetRegVal(CMD_PULSE_WINDOW, PulseCount);
	pChip->pulse_window_time = PulseCount;

	printk("Setting pulse count to %d \n Setting pulse window time to(%d.%d)\r\n",PulseCount , FIXP_INT_PART(val), FIXP_DEC_PART(val));

	return count;
}

/*!
* This method is used to show pulse window time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_window_time_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	size_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT pulsewindow  = 0;
	FIXPOINT PulseWindowStep = 0; 

	if(pChip->LPF == 1)
	{
		PulseWindowStep = *( pChip->pPulse_window_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		PulseWindowStep  = *( pChip->pPulse_window_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
	}
	pulsewindow = (PulseWindowStep * pChip->pulse_window_time);
	sprintf(buf,"%d.%03d", FIXP_INT_PART(pulsewindow), FIXP_DEC_PART(pulsewindow));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show minimum pulse window time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_window_time_min_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT pulse_window_time = 0;

	if(pChip->LPF == 1)
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(pulse_window_time), FIXP_DEC_PART(pulse_window_time));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show maximum pulse window time. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_window_time_max_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT max;
	FIXPOINT pulse_window_time = 0;

	if(pChip->LPF == 1)
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(pulse_window_time), FIXP_DEC_PART(pulse_window_time));

	max = 255 * pulse_window_time;

	sprintf(buf,"%d.%d\n",FIXP_INT_PART(max), FIXP_DEC_PART(max));
	
	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to show pulse window time step. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t pulse_window_time_step_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	FIXPOINT pulse_window_time = 0;

	if(pChip->LPF == 1)
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN1 + (4 * (pChip->odr) + pChip->oversampling)); 
       										//Formula =  BaseAddr+(Total Num of clmn * Current row num + Current clmn num)
	}
	else 
	{
		pulse_window_time = *( pChip->pPulse_window_timeList_LPF_EN0 + (4 * (pChip->odr) + pChip->oversampling)); 
     	}
	sprintf(buf,"%d.%d\n",FIXP_INT_PART(pulse_window_time), FIXP_DEC_PART(pulse_window_time));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store event type single. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return     : Length of string.
*/

ssize_t event_type_single_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	unsigned long val;

	strict_strtoul(buf, 10, &val);
	if(val < 0 || val > 255)
		return -EINVAL;

	pDev->event_type_single = val;

	return count;
}

/*!
* This method is used to show event type single. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t event_type_single_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", pDev->event_type_single);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store event type double. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return     : Length of string.
*/

ssize_t event_type_double_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	unsigned long val;

	strict_strtoul(buf, 10, &val);
	if(val < 0 || val > 255)
		return -EINVAL;

	pDev->event_type_double = val;

	return count;
}

/*!
* This method is used to show event type double. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t event_type_double_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);

	sprintf(buf, "%d\n", pDev->event_type_double);

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store threshold xyz.
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return -EINVAL  : Invalid threshold value. 
* return     : Length of string.
*/


ssize_t threshold_xyz_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	char *params[3];
	int i = 0;
	FIXPOINT val[3];
	FIXPOINT threshold = 0;

	ParseString((char *)buf, strlen(buf), ',', params, 3);
	for(i=0;i<3;i++)
	{
		if(strict_strtofp(params[i], &val[i]) < 0)
			return -EINVAL;

		threshold = (FIXPOINT)( (long)(val[i] * 127)  / (8 * FLOAT_TO_FIXP(9.8)));
		printk("%s: >>> Setting FM threshold to %d\r\n", __func__, threshold);
	
		if((threshold < 1) || (threshold > 127 ))
		{
			printk("Invalid threshold value");
			return -EINVAL;
		}

		threshold = INT_TO_FIXP(threshold);

		if(FIXP_DEC_PART(threshold) >= 500)
		 	threshold = FIXP_INT_PART(threshold) + 1;
		else
			 threshold = FIXP_INT_PART(threshold);

		printk("%s: Setting XYZ threshold to %d\r\n", __func__, threshold);

		val[i] = threshold;

		printk("Setting pulse threshold to %d \r\n", val[i]);
	}
	pChip->SetCalOffset(val);
	pChip->tap_threshold_x = val[0];	
	pChip->tap_threshold_y = val[1];	
	pChip->tap_threshold_z = val[2];
	return count;
}
	
/*!
* This method is used to show threshold xyz. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t threshold_xyz_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	FIXPOINT threshold_X;
	FIXPOINT threshold_Y;
	FIXPOINT threshold_Z;

	threshold_X = (pChip->tap_threshold_x *((1 * 8 * FLOAT_TO_FIXP(9.8)) / 127));
	threshold_Y = (pChip->tap_threshold_y *((1 * 8 * FLOAT_TO_FIXP(9.8)) / 127));
	threshold_Z = (pChip->tap_threshold_z *((1 * 8 * FLOAT_TO_FIXP(9.8)) / 127));

	sprintf(buf, "%d.%03d,%d.%03d,%d.%03d\n", FIXP_INT_PART(threshold_X), FIXP_DEC_PART(threshold_X),FIXP_INT_PART(threshold_Y), FIXP_DEC_PART(threshold_Y),
			FIXP_INT_PART(threshold_Z), FIXP_DEC_PART(threshold_Z));

	ret = strlen(buf) + 1;
	return ret;
}

/*!
* This method is used to store tap enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return -EINVAL: Invalid tap_enable value.
* return     : Length of string.
*/

ssize_t tap_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;
	char *params[3];
	int en_val = 0;
	int i = 0;

	if( ParseString((char *)buf, strlen(buf), ',', params, 3) < 3)
		return -EINVAL;

	for(i = 0; i < 3; i++)
	{
		if(!strncmp(params[i], "single", strlen("single")))
		{
			printk("Tap detection single\r\n");
			en_val |= 0x1 << (i * 2);
		}
		else if(!strncmp(params[i], "double", strlen("double")))
		{
			printk("Tap detection double\r\n");
			en_val |= 0x2 << (i * 2);
		}
		else if(!strncmp(params[i], "enable", strlen("enable")))
		{
			printk("Tap detection enable\r\n");
			en_val |= 0x3 << (i * 2);
		}

		else if(!strncmp(params[i], "disable", strlen("disable")))

		{
			printk("Tap detection disable\r\n");
			en_val &= ~(0x3 << ( i * 2));
		}
		else
		{
			printk("Invalid tap_enable value %s\n", params[i]);
			return -EINVAL;
		}
	}
	printk("Tap enable = 0x%02x\n",en_val);
	pChip->SetRegVal(CMD_TAP_ENABLE, en_val);
	pChip->tap_enable = en_val;

	return count;
}

/*!
* This method is used to show tap enable. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t tap_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	int val = 0;
	int i = 0;
	int len = 0;
	for(i = 0; i < 3; i++)
	{
		val = (pChip->tap_enable & (0x03 << (i * 2))) >> (i*2);
		switch(val)
		{
			case 0:
				sprintf(&buf[len], "%s,", "disable");
				break;
			case 1:
				sprintf(&buf[len], "%s,", "single");
				break;
			case 2:
				sprintf(&buf[len], "%s,", "double");
				break;
			case 3:
				sprintf(&buf[len], "%s,", "enable");
				break;
		}
		len = strlen(buf);
	}
	buf[len] = '\0';
	len = strlen(buf);
	return len;
}

/*!
* This method is used to store sentap wake-on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* count: Length of string. 
* return     : Length of string.
* return  -EINVAL   : Invalid sentap_wakeon_event value.
*/

ssize_t sentap_wake_on_event_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	if(!strncmp(buf, "enable", count-1))
	{
		printk("sentap_wakeon_event enable\r\n");
		pChip->SetRegVal(CMD_TAP_WAKE, 1);
		pChip->tap_wakeon_event = 1;
	}
	else if(!strncmp(buf, "disable", count-1))
	{
		printk("sentap_wakeon_event disable\r\n");
		pChip->SetRegVal(CMD_TAP_WAKE, 0);
		pChip->tap_wakeon_event = 0;
	}
	else
	{
		printk("Invalid sentap_wakeon_event value. (buf: %s count: %d)\n", buf, count);
		return -EINVAL;
	}
	return count;
}

/*!
* This method is used to show sentap wake-on event. 
* dev  : Pointer to device structure.
* attr : Pointer to device attributes.
* buf  : Pointer to buffer containing trigger set value.
* return     : Length of string.
*/

ssize_t sentap_wake_on_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
	struct ChipInfo_t *pChip = pDev->pChip;

	sprintf(buf, "%s\n", (pChip->tap_wakeon_event == 1)? "enable" : "disable");

	ret = strlen(buf) + 1;
	return ret;
}


/*!
  * This method is used to set polling time. 
  * dev          : Pointer to device structure.
  * attr         : Pointer to device attributes.
  * buf          : Pointer to buffer containing poll timer value to be set.
  * count        : Length of string  
  * return             : Length of string for valid poll timer value.  
  * return  -EINVAL   : Invalid poll timer value.
*/
ssize_t poll_time_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count)
{
        struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
        struct ChipInfo_t *pChip = pDev->pChip;
        unsigned long val = 0;

        strict_strtoul(buf, 10, &val);
       
	if (val <= 0) 
	{
		return -EINVAL;
	} 
	printk("poll time set to: %d\n", pChip->poll_time);

        pChip->poll_time = ( val / 10);


        return count;
}


/*!
  * This method is used to get poll time.
  * dev  : Pointer to device structure.
  * attr : Pointer to device attributes.
  * buf  : Pointer to buffer in which current data poll time value will be copied.
  * return     : Length of string.  
*/
ssize_t poll_time_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
        ssize_t ret = 0;

        struct mxc_mma_device_t *pDev = dev_get_drvdata(dev);
        struct ChipInfo_t *pChip = pDev->pChip;

        sprintf(buf, "%d\n", pChip->poll_time * 10);
        ret = strlen(buf) + 1;

        return ret;
}






/*!
* This method is used to Initialize Sysfs. 
* client   : Pointer to i2c_client structure.
* return -EPERM  : Device pointer is NULL.
* return  0      : Success. 
*/
struct class *sensor_class_obj = NULL;
int InitializeSysfs(struct i2c_client *client)
{
	int RetVal = 0;
	int i = 0;
	int Iterator = 0;
	int Instance = 0;
	struct mxc_mma_device_t *pDev = i2c_get_clientdata(client);
	struct ChipInfo_t *pChip = pDev->pChip;
	struct SysfsInfo_t *pSysfsInfo = pChip->pSysfsInfo;

	if(pDev == NULL)
	{
		printk("%s: pDev => NULL pointer\r\n", __func__);
		return -EPERM;
	}

	if(sensor_class_obj == NULL)
	{
		pDev->class = class_create(THIS_MODULE, "sensor");
		if (IS_ERR(pDev->class)) 
		{
			printk(KERN_ERR "Unable to create class for Mxc MMA\n");
			RetVal = PTR_ERR(pDev->class);
		}
		sensor_class_obj = pDev->class;
	}
		else
	{
		pDev->class = sensor_class_obj;
	}
	client->dev.class = pDev->class;

	pDev->sys_device = device_create(pDev->class, NULL, MKDEV(pDev->major, 0), pDev,"mma");

	if (IS_ERR(pDev->sys_device))
       	{
		printk(KERN_ERR "Unable to create class device for Mxc Ipu\n");
		RetVal = PTR_ERR(pDev->sys_device);
		return RetVal;
	}

	dev_set_drvdata( (struct device *)&pDev->sys_device, pDev);

	for(Iterator = 0; Iterator < pChip->SysfsInfoSize; Iterator++)
	{
		for(Instance = 0; Instance < pSysfsInfo[Iterator].Instance; Instance++)
		{		
			/* Create motion_detection device */
			if(pSysfsInfo[Iterator].grpName != NULL)
			{
				pDev->sys_motion_dev = device_create(pDev->class, pDev->sys_device, MKDEV(0, 0), pDev,
						"%s%d", pSysfsInfo[Iterator].grpName, Instance);
			}
			else
			{
				pDev->sys_motion_dev = pDev->sys_device;
			}

			for(i=0; i < pSysfsInfo[Iterator].TotalEntries; i++)
			{
				if(sysfs_create_file(&pDev->sys_motion_dev->kobj, &pSysfsInfo[Iterator].AttrEntry[i].attr) < 0)
				printk("%s sys file creation failed.\r\n", pSysfsInfo[Iterator].AttrEntry[i].attr.name);
			}
		}
	}
	return RetVal;	
}
EXPORT_SYMBOL(sensor_class_obj);

/*!
* This method is used to Deinitializesysfs 
* pdev     : Pointer to mxc_mma_device_t structure.
* return -EPERM  : Device pointer is NULL.
* return 0       :  Success.
*/

int DeInitializeSysfs(struct mxc_mma_device_t *pDev)
{
	if(pDev == NULL)
	{
		printk("%s: pDev => NULL pointer\r\n", __func__);
		return -EPERM;
	}

	device_destroy(pDev->class, MKDEV(pDev->major, 0));
	class_destroy(pDev->class);

	return 0;
}

