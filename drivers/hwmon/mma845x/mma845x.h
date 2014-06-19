/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma845x.h
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA8451/MMA8452/MMA8452 sensor register and some marco 
			  definitions 
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
 
#ifndef __MMA_REGS_845X_H__
#define __MMA_REGS_845X_H__

/* Macros for chip id */
#define ID_MMA8451	(0x1A)
#define ID_MMA8452	(0x2A)
#define ID_MMA8453	(0x3A)


enum {
	REG845X_STATUS = 0x00,
	REG845X_OUT_X_MSB,
	REG845X_OUT_X_LSB,
	REG845X_OUT_Y_MSB,
	REG845X_OUT_Y_LSB,
	REG845X_OUT_Z_MSB,
	REG845X_OUT_Z_LSB,
	REG845X_RSVD1,
	REG845X_RSVD2,
	REG845X_F_SETUP,
	REG845X_TRIG_CFG,
	REG845X_SYSMOD,
	REG845X_INT_SOURCE,
	REG845X_WHO_AM_I,
	REG845X_XYZ_DATA_CFG,
	REG845X_HP_FILTER_CUTOFF,
	REG845X_PL_STATUS,
	REG845X_PL_CFG,
	REG845X_PL_COUNT,
	REG845X_PL_BF_ZCOMP,
	REG845X_PL_THS_REG,
	REG845X_FF_MT_CFG,
	REG845X_FF_MT_SRC,
	REG845X_FF_MT_THS,
	REG845X_FF_MT_COUNT,
	REG845X_RSVD3,
	REG845X_RSVD4,
	REG845X_RSVD5,
	REG845X_RSVD6,
	REG845X_TRANSIENT_CFG,
	REG845X_TRANSIENT_SRC,
	REG845X_TRANSIENT_THS,
	REG845X_TRANSIENT_COUNT,
	REG845X_PULSE_CFG,
	REG845X_PULSE_SRC,
	REG845X_PULSE_THSX,
	REG845X_PULSE_THSY,
	REG845X_PULSE_THSZ,
	REG845X_PULSE_TMLT,
	REG845X_PULSE_LTCY,
	REG845X_PULSE_WIND,
	REG845X_ASLP_COUNT,
	REG845X_CTRL_REG1,
	REG845X_CTRL_REG2,
	REG845X_CTRL_REG3,
	REG845X_CTRL_REG4,
	REG845X_CTRL_REG5,
	REG845X_OFF_X,
	REG845X_OFF_Y,
	REG845X_OFF_Z,
	REG845X_RSVD7,
};

/* Interrupt Macros */
#define _BIT(x)		(0x01 << x)

#define INT_EN_ASLP	_BIT(7)		// Auto-SLEEP/WAKE
#define INT_EN_FIFO	_BIT(6)		// FIFO interrupt 
#define INT_EN_TRANS	_BIT(5)		// Transient interrupt
#define INT_EN_LNDPRT 	_BIT(4)		// Landscape/portrait (orientation)
#define INT_EN_PULSE 	_BIT(3)		// Pulse
#define INT_EN_FF_MT 	_BIT(2)		// Freefall/motion interrupt
#define INT_EN_DRDY 	_BIT(0)		// Dataready interrupt

/* FIFO Modes */
#define FMODE_DISABLE	(0x00 << 6)
#define FMODE_CIRCULAR	(0x01 << 6)
#define FMODE_FILL	(0x02 << 6)
#define FMODE_EVENT	(0x03 << 6)

/* FIFO status flags */
#define F_OVR		_BIT(7)
#define F_WMRK		_BIT(6)
#define F_COUNT		(0x3f)

#define FMODE_WATERMARK(x)	(x & 0x3f)

#endif // __MXC_REGS_845X_H__
