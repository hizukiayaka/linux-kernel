/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma845x.c
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA8451/MMA8452/MMA8452 accelerometer sensor API
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
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/input-polldev.h>
#include <linux/hwmon.h>
#include <linux/regulator/consumer.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#ifdef CONFIG_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "mma_regs.h"
#include "mma845x.h"
#include "mma_sysfs.h"

/*Declarations*/
static int InitializeChip(struct ChipInfo_t *pChipInfo);
static int ReadChip( int type, void *data);
static int EnableInterrupt( int mask );
static int DisableInterrupt( int mask );
static int SetChipMode( int mode );
static int GetChipMode( void );
static int GetIntSource( void );

extern int ParseString(char *Data, int Length, char Delimiter, char *Tokens[], int TokenCnt);
extern FIXPOINT strict_strtofp(const char *buf, FIXPOINT *val); 
/*macro define*/
#define MMA845X_I2C_ADDR	0x1d

static const FIXPOINT OdrList[] = {
				INT_TO_FIXP(800),
				INT_TO_FIXP(400),
				INT_TO_FIXP(200),
				INT_TO_FIXP(100),
				INT_TO_FIXP(50),
				FLOAT_TO_FIXP(12.5),
				FLOAT_TO_FIXP(6.25),
				FLOAT_TO_FIXP(1.56),
				-1,
				};

const char * SamplingList[] = {
				"normal",
				"low_noise",
				"high_resolution",
				"low_power",
				NULL,
				};

static const int SupportedResolution[] 		= {14, 8, -1};     // MMA8451Q

static const FIXPOINT SupportedRange[] 		= {INT_TO_FIXP(2), INT_TO_FIXP(4), INT_TO_FIXP(8) , -1};

const int Z_lock_angle_thresholdList[] 		= {14, 18, 21, 25, 29, 33, 37, 42, -1};

const int Back_front_trip_angle_thresholdList[] = {80, 75, 70, 65, -1};

const int Trip_angle_thresholdList[]		= {15, 20, 30, 35, 40, 45, 55, 60, 70, 75, -1};

const int Trip_angle_thresholdValues[] 		= {0x07, 0x09, 0x0c, 0x0d, 0x0f, 0x10, 0x13, 0x14, 0x17, 0x19, -1};

const int Trip_angle_hysteresisList[] 		= {0, 4, 7, 11, 14, 17, 21, 24 , -1};

const FIXPOINT Fm_debounce_countList[8][4]      = {
		//Normal		//Low Noise Low Power	//High Resolution	//Low power
	{ 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  ,	FLOAT_TO_FIXP(1.25)	}, //ODR : 800
	{	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5) 	}, //ODR : 400
	{	INT_TO_FIXP(5)	     , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(5)		}, //ODR : 200
	{	INT_TO_FIXP(10)      , 	INT_TO_FIXP(10)      , 	FLOAT_TO_FIXP(2.5)   ,	INT_TO_FIXP(10) 	}, //ODR : 100
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(20)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(20)		}, //ODR : 50
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(80)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(80)		}, //ODR : 12.5
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(160)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(160)	}, //ODR : 6.25
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(640)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(640)	}, //ODR : 1.56
};

const FIXPOINT Trans_debounce_countList[8][4]   = {
		//Normal		//Low Noise Low Power	//High Resolution	//Low power
	{  	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) 	},
	{ 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5) 	},
	{  	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(5)		},
	{	INT_TO_FIXP(10)      , 	INT_TO_FIXP(10)      , 	FLOAT_TO_FIXP(2.5)   ,	INT_TO_FIXP(10) 	},
	{ 	INT_TO_FIXP(20)      , 	INT_TO_FIXP(20)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(20)		},	 
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(80)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(80) 	},
	{ 	INT_TO_FIXP(20)      , 	INT_TO_FIXP(160)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(160)	},
	{	INT_TO_FIXP(20)      , 	INT_TO_FIXP(640)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(640) 	},
};

const FIXPOINT Ornt_debounce_countList[8][4]   = {
	{	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) 	},
	{	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5) 	},
	{	INT_TO_FIXP(5)      , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(5)		},
	{	INT_TO_FIXP(10)     , 	INT_TO_FIXP(10)      , 	FLOAT_TO_FIXP(2.5)   ,	INT_TO_FIXP(10) 	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(20)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(20)		},	 
	{ 	INT_TO_FIXP(20)     , 	INT_TO_FIXP(80)      , 	FLOAT_TO_FIXP(2.5)   , 	INT_TO_FIXP(80) 	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(160)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(160)	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(640)     , 	FLOAT_TO_FIXP(1.25)  , 	INT_TO_FIXP(640) 	},
};

const FIXPOINT Pulse_time_limitList_LPF_EN0[8][4] = {
	{	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(0.625) 	},
	{	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(1.25) 	},
	{	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(0.625) , 	FLOAT_TO_FIXP(2.5) 	},
	{	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(0.625) , 	INT_TO_FIXP(5)		},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(0.625) , 	INT_TO_FIXP(10) 	},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(20)      , 	FLOAT_TO_FIXP(0.625) , 	INT_TO_FIXP(40) 	},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(40)      , 	FLOAT_TO_FIXP(0.625) , 	INT_TO_FIXP(80) 	},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(160)     , 	FLOAT_TO_FIXP(0.625) , 	INT_TO_FIXP(320) 	},
};

const FIXPOINT Max_pulse_time_limitList_LPF_EN0[8][4] = {
	{	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.159) 	},
	{	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(0.319) 	},
	{	FLOAT_TO_FIXP(0.319) ,  FLOAT_TO_FIXP(0.319) ,  FLOAT_TO_FIXP(0.159) ,  FLOAT_TO_FIXP(0.638) 	},
	{	FLOAT_TO_FIXP(0.638) ,  FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.159) ,  FLOAT_TO_FIXP(1.28)	},
	{	FLOAT_TO_FIXP(1.28)  ,	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(2.55)	},
	{	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(10.2) 	},
	{	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(10.2)  , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(20.4)	},
	{	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(40.8)  , 	FLOAT_TO_FIXP(0.159) , 	FLOAT_TO_FIXP(81.6) 	},
};


const FIXPOINT Pulse_time_limitList_LPF_EN1[8][4] = {
	{ 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25) 	},
	{	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5) 	},
	{	INT_TO_FIXP(5)      , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(2.5)  , 	INT_TO_FIXP(5)		},
	{ 	INT_TO_FIXP(10)     , 	INT_TO_FIXP(10)      , 	FLOAT_TO_FIXP(2.5)  ,	INT_TO_FIXP(10) 	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(20)      , 	FLOAT_TO_FIXP(2.5)  , 	INT_TO_FIXP(20)		},	 
	{ 	INT_TO_FIXP(20)     , 	INT_TO_FIXP(80)      , 	FLOAT_TO_FIXP(2.5)  , 	INT_TO_FIXP(80) 	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(160)     , 	FLOAT_TO_FIXP(2.5)  , 	INT_TO_FIXP(160)	},
	{ 	INT_TO_FIXP(20)     , 	INT_TO_FIXP(640)     , 	FLOAT_TO_FIXP(2.5)  , 	INT_TO_FIXP(640)	},
};

const FIXPOINT Max_pulse_time_limitList_LPF_EN1[8][4] = {
	{	FLOAT_TO_FIXP(0.319) , 	FLOAT_TO_FIXP(0.319) , 	FLOAT_TO_FIXP(0.319) , 	FLOAT_TO_FIXP(0.319) 	},
	{	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) 	},
	{	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(1.28)  , 	FLOAT_TO_FIXP(0.638) ,  FLOAT_TO_FIXP(1.28) 	},
	{	FLOAT_TO_FIXP(2.55)  , 	FLOAT_TO_FIXP(2.55)  , 	FLOAT_TO_FIXP(0.638) ,  FLOAT_TO_FIXP(2.55)	},
	{	FLOAT_TO_FIXP(5.1)   ,	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(5.1)	},
	{	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(20.4)  , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(20.4) 	},
	{	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(40.8)  , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(40.8)	},
	{	FLOAT_TO_FIXP(5.1)   , 	INT_TO_FIXP(163)     ,  FLOAT_TO_FIXP(0.638) , 	INT_TO_FIXP(163) 	},
};

const FIXPOINT Pulse_latency_timeList_LPF_EN0[8][4] = {
	{	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25) 	},
	{	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(2.5)	},
	{	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(1.25) , 	INT_TO_FIXP(5) 		},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5)      , 	FLOAT_TO_FIXP(1.25) , 	INT_TO_FIXP(10) 	},
	{ 	INT_TO_FIXP(10)      , 	INT_TO_FIXP(10)     ,	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(20)		},
	{	INT_TO_FIXP(10)      , 	INT_TO_FIXP(40)     , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(80) 	},
	{ 	INT_TO_FIXP(10)      , 	INT_TO_FIXP(80)     , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(160) 	},
	{	INT_TO_FIXP(10)      , 	INT_TO_FIXP(320)    , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(640)  	},
};	


const FIXPOINT Pulse_latency_timeList_LPF_EN1[8][4] = {
	{	FLOAT_TO_FIXP(2.5) , 	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5) 	},
	{	INT_TO_FIXP(5)     , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(5) 		},
	{	INT_TO_FIXP(10)    , 	INT_TO_FIXP(10)     ,  	INT_TO_FIXP(5)      ,	INT_TO_FIXP(10)  	},
	{ 	INT_TO_FIXP(20)    , 	INT_TO_FIXP(20)     , 	INT_TO_FIXP(5)      ,	INT_TO_FIXP(20)  	},
	{	INT_TO_FIXP(40)    , 	INT_TO_FIXP(40)     , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(40) 	},
	{	INT_TO_FIXP(40)    , 	INT_TO_FIXP(160)    , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(160) 	},
	{	INT_TO_FIXP(40)    ,  	INT_TO_FIXP(320)    , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(320)	},
	{	INT_TO_FIXP(40)    , 	INT_TO_FIXP(1280)   , 	INT_TO_FIXP(5)      , 	INT_TO_FIXP(1280) 	},
};


const FIXPOINT Pulse_window_timeList_LPF_EN0[8][4] = {
	{	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(1.25)	},
	{	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25)  , 	FLOAT_TO_FIXP(1.25) , 	FLOAT_TO_FIXP(2.5)	},
	{	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(1.25) , 	INT_TO_FIXP(5) 		},
	{	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5)       , 	FLOAT_TO_FIXP(1.25) , 	INT_TO_FIXP(10) 	},
	{	INT_TO_FIXP(10)      , 	INT_TO_FIXP(10)      ,	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(20)		},
	{	INT_TO_FIXP(10)      ,  INT_TO_FIXP(40)      , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(80) 	},
	{	INT_TO_FIXP(10)      ,  INT_TO_FIXP(80)      , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(160) 	},
	{	INT_TO_FIXP(10)      ,  INT_TO_FIXP(320)     , 	FLOAT_TO_FIXP(1.25) ,  	INT_TO_FIXP(640) 	},
};

const FIXPOINT Max_pulse_window_timeList_LPF_EN0[8][4] = {
	{	FLOAT_TO_FIXP(0.318) , 	FLOAT_TO_FIXP(0.318) , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(0.318)	},
	{	FLOAT_TO_FIXP(0.318) , 	FLOAT_TO_FIXP(0.318) , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(0.638) 	},
	{	FLOAT_TO_FIXP(0.638) ,  FLOAT_TO_FIXP(0.638) ,  FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(1.276) 	},
	{	FLOAT_TO_FIXP(1.276) ,  FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(2.56)	},
	{	FLOAT_TO_FIXP(2.56)  ,	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(5.1)	},
	{	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(10.2)  , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(20.4)	},
	{	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(20.4)  , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(40.8)	},
	{	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(81.6)  , 	FLOAT_TO_FIXP(0.318) ,  FLOAT_TO_FIXP(163.2) 	},
};

const FIXPOINT Pulse_window_timeList_LPF_EN1[8][4] = {
	{	FLOAT_TO_FIXP(2.5)  , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5)   , 	FLOAT_TO_FIXP(2.5) 	},
	{	INT_TO_FIXP(5)      , 	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5)       , 	INT_TO_FIXP(5) 		},
	{	INT_TO_FIXP(10)     , 	INT_TO_FIXP(10)      , 	INT_TO_FIXP(5)       ,	INT_TO_FIXP(10)  	},
	{	INT_TO_FIXP(20)     , 	INT_TO_FIXP(20)      , 	INT_TO_FIXP(5)       ,	INT_TO_FIXP(20)  	},
	{	INT_TO_FIXP(40)     , 	INT_TO_FIXP(40)      , 	INT_TO_FIXP(5)       , 	INT_TO_FIXP(40) 	},
	{ 	INT_TO_FIXP(40)     , 	INT_TO_FIXP(160)     ,	INT_TO_FIXP(5)       , 	INT_TO_FIXP(160) 	},
	{	INT_TO_FIXP(40)     , 	INT_TO_FIXP(320)     , 	INT_TO_FIXP(5)       , 	INT_TO_FIXP(320)	},
	{	INT_TO_FIXP(40)     , 	INT_TO_FIXP(1280)    , 	INT_TO_FIXP(5)       , 	INT_TO_FIXP(1280) 	},
};

const FIXPOINT Max_pulse_window_timeList_LPF_EN1[8][4] = {
	{	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) , 	FLOAT_TO_FIXP(0.638) 	},
	{	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(1.276) 	},
	{	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(2.56)  , 	FLOAT_TO_FIXP(1.276) ,  FLOAT_TO_FIXP(2.56) 	},
	{	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(5.1)   , 	FLOAT_TO_FIXP(1.276) ,  FLOAT_TO_FIXP(5.1)	},
	{	FLOAT_TO_FIXP(10.1)  ,	FLOAT_TO_FIXP(10.2)  , 	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(10.2)	},
	{	FLOAT_TO_FIXP(10.1)  , 	FLOAT_TO_FIXP(40.8)  , 	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(40.8) 	},
	{	FLOAT_TO_FIXP(10.1)  , 	FLOAT_TO_FIXP(81.6)  , 	FLOAT_TO_FIXP(1.276) , 	FLOAT_TO_FIXP(81.6)	},
	{	FLOAT_TO_FIXP(10.1)  , 	INT_TO_FIXP(326)     , 	FLOAT_TO_FIXP(1.276) ,	INT_TO_FIXP(326) 	},
};

const FIXPOINT Trans_cut_off_frequenciesList[4][8][4] = {
	{	// Normal Mode
		{	INT_TO_FIXP(16)	   ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)     ,	INT_TO_FIXP(2)		},  
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)     ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(8)     ,	INT_TO_FIXP(4)    ,	INT_TO_FIXP(2)     ,	INT_TO_FIXP(1)		},
		{	INT_TO_FIXP(4)     ,	INT_TO_FIXP(2)    ,	INT_TO_FIXP(1)     ,	FLOAT_TO_FIXP(0.5)	},
		{	INT_TO_FIXP(2)     ,	INT_TO_FIXP(1)    ,	FLOAT_TO_FIXP(0.5) ,	FLOAT_TO_FIXP(0.25)	},
		{	INT_TO_FIXP(2)     ,	INT_TO_FIXP(1)    ,	FLOAT_TO_FIXP(0.5) ,	FLOAT_TO_FIXP(0.25)	},
		{	INT_TO_FIXP(2)     ,	INT_TO_FIXP(1)    ,	FLOAT_TO_FIXP(0.5) ,	FLOAT_TO_FIXP(0.25)	},
		{	INT_TO_FIXP(2)     ,	INT_TO_FIXP(1)    ,	FLOAT_TO_FIXP(0.5) ,	FLOAT_TO_FIXP(0.25)	},
	},
	{
		//  Low Noise Low Power
		{	INT_TO_FIXP(16)     ,	INT_TO_FIXP(8)      ,	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)     ,	INT_TO_FIXP(8)      ,	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(8)      ,	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)		},
		{	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)      ,	FLOAT_TO_FIXP(0.5)	},
		{	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)      ,	FLOAT_TO_FIXP(0.5)  ,	FLOAT_TO_FIXP(0.25)	},
		{	FLOAT_TO_FIXP(0.5)  ,	FLOAT_TO_FIXP(0.25) ,	FLOAT_TO_FIXP(0.125),	FLOAT_TO_FIXP(0.063)	},
		{	FLOAT_TO_FIXP(0.25) ,	FLOAT_TO_FIXP(0.125),	FLOAT_TO_FIXP(0.063),	FLOAT_TO_FIXP(0.031)	},
		{	FLOAT_TO_FIXP(0.063),	FLOAT_TO_FIXP(0.031),	FLOAT_TO_FIXP(0.016),	FLOAT_TO_FIXP(0.008)	},
	},

	{
		// Hi Resolution
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    ,	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(16)    ,	INT_TO_FIXP(8)    , 	INT_TO_FIXP(4)   ,	INT_TO_FIXP(2)		},
	},
	{
		//  Low Power
		{	INT_TO_FIXP(16)     ,	INT_TO_FIXP(8)      ,	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)		},
		{	INT_TO_FIXP(8)      ,	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)		},
		{	INT_TO_FIXP(4)      ,	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)      ,	FLOAT_TO_FIXP(0.5)	},
		{	INT_TO_FIXP(2)      ,	INT_TO_FIXP(1)      ,	FLOAT_TO_FIXP(0.5)  ,	FLOAT_TO_FIXP(0.25)	},
		{	INT_TO_FIXP(1)      ,	FLOAT_TO_FIXP(0.5)  ,	FLOAT_TO_FIXP(0.25) ,	FLOAT_TO_FIXP(0.125)	},
		{	FLOAT_TO_FIXP(0.25) ,	FLOAT_TO_FIXP(0.125),	FLOAT_TO_FIXP(0.063),	FLOAT_TO_FIXP(0.031)	},
		{	FLOAT_TO_FIXP(0.125),	FLOAT_TO_FIXP(0.063),	FLOAT_TO_FIXP(0.031),	FLOAT_TO_FIXP(0.016)	},
		{	FLOAT_TO_FIXP(0.031),	FLOAT_TO_FIXP(0.016),	FLOAT_TO_FIXP(0.008),	FLOAT_TO_FIXP(0.004)	},

	},


};

static const int Nominal_powerList[] = {242 ,242 ,123 ,64 ,34 ,34 ,34 ,34,-1}; 

// MMA8452Q

static const int MMA8452Q_SupportedResolution[]  = {12 , 8 , -1};

static const int MMA8452Q_Z_lock_angle_thresholdList[] = {29, -1};

static const int MMA8452Q_Back_front_trip_angle_thresholdList[] = {75, -1};

static const int MMA8452Q_Trip_angle_thresholdList[] = {45, -1};

static const int MMA8452Q_Trip_angle_hysteresisList[] = {14, -1};


// MMA8453Q

static const int MMA8453Q_SupportedResolution[] = {10 , 8 , -1};

/*definition*/
static int ChipType = MMA845X;
static struct i2c_client * pClient = NULL;
static struct ChipInfo_t * pChip = NULL;

static int InitMotionModule(struct ChipInfo_t *pChip);
static int InitTransientDetectionModule(struct ChipInfo_t *pChip);
static int InitOrientationModule(struct ChipInfo_t *pChip);
static int InitTapDetectionModule(struct ChipInfo_t *pChip);

/*!
* This method is used to write a 8 bit hardware register of MMA845x.
* regaddr  : address of hardware register.
* startpos : position of the bit from which data will be written.
* bitlengh : no of bits to be written.
* value    : data to be written. 	
* return 0       : For successful write, -EINVAL : For write failure  
*/ 

static int WriteRegValue(int regaddr , int startpos , int bitlength , int value)
{
	char ret = 0;
	if(value < (0x01 << bitlength)) 
	{
		ret = i2c_smbus_read_byte_data(pClient, regaddr);
		ret &= ~(((0x01<<bitlength)-1)<<startpos);               					// to reset the required bits
		ret |= value<<startpos;
		i2c_smbus_write_byte_data(pClient, regaddr, ret);
		ret = i2c_smbus_read_byte_data(pClient, regaddr);
		return 0;
	}
	else
       	{
		printk("\t Invalid Value \r\n");
		return -EINVAL;
	}
}

/*!
* This method is used to set the calibration offset value.
* val[]  : integer array value to write in X, Y, Z offset register.
*/

int SetCalOffset(int val[])
{
	// Put device in standby
	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

         // Set Calibration offset
	i2c_smbus_write_byte_data(pClient, (REG845X_OFF_X), val[0]);
	i2c_smbus_write_byte_data(pClient, (REG845X_OFF_Y), val[1]);
	i2c_smbus_write_byte_data(pClient, (REG845X_OFF_Z), val[2]);

	 // Put device to original state
	(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

	return 0;
}

/*!
* This method is used to set the calibration offset value.
* val[]  : fixpoint array of value to write in threshold X,Y,Z register.
*/
int SetThXYZ(FIXPOINT val[])
{
	// Put device in standby
	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

	i2c_smbus_write_byte_data(pClient, (REG845X_PULSE_THSX), val[0]);
	i2c_smbus_write_byte_data(pClient, (REG845X_PULSE_THSY), val[1]);
	i2c_smbus_write_byte_data(pClient, (REG845X_PULSE_THSZ), val[2]);

	// Put device to original state
	(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

	return 0;
}

/*!
* This method is used to find minimum value from array.
* array  : Pointer to array.
* inc      : Count by which pointer increments in array.
* offset   : Start position in array.
* count    : Total number of elements in array. 	
* return min     : Returns minimum value from array.  
*/ 

unsigned int FindMin(int *array,int inc,int offset,int count)
{
	int i = 0;
	unsigned int max = *(array+offset);
	unsigned int min = max,curval = min;
	for(i=0;i<count;)
	{
	   curval = *(array+offset+i);
           min = ( curval < min ? curval : min );
		i=i+inc;   
	}
	return min;
}
/*!
* This method is used to find maximum value from array.
* array  : Pointer to array.
* inc      : Count by which pointer increments in array.
* offset   : Start position in array.
* count    : Total number of elements in array. 	
* return min     : Returns maximum value from array.  
*/ 

unsigned int FindMax(int *array,int inc,int offset,int count)
{
	int i = 0;
	unsigned int max = *(array+offset);
	unsigned int min = max,curval = min;
	for(i=0;i<count;)
	{
	   	curval = *(array+offset+i);
           	max = ( curval > min ? curval : min );
		i=i+inc;   
	}
	return max;
}
/*!
* This method is used to set different settings of MMA845X. 
* cmd  : contains setting to be applied to MMA845X.
* val  : contains data required to apply settings specified in cmd.
* return 0   : For successful write, -EINVAL : For write failure  
*/

int SetRegVal(int cmd, int val)
{
	int ret = 0;
	int data = -1;
	switch(cmd)
	{
		case CMD_MODE :
				data = (val == 0) ? 0 : 1;
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1 , data);
	
				data = -1;
				if(val)
				{
					ret = 0;
					if(val == 1)
						data = 1;
					else if(val == 2)
						data = 0;
					else
						ret = -EINVAL;
					if(data >= 0) {
						// Put device in standby
						WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);
	
						WriteRegValue((REG845X_CTRL_REG2) , 2, 1 , data);
						
						// Put device to Active state
						WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
					}

				}
				break;

		case CMD_ODR:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);
	
				WriteRegValue((REG845X_CTRL_REG1) , 3, 3, val);
			
				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_RESOLUTION:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG1) , 1, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_SAMPLE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG2) , 0, 2, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_WAKEUP:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				// This is system wakeup
				WriteRegValue((REG845X_CTRL_REG2) , 2, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_RANGE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_XYZ_DATA_CFG) , 0, 2 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_FM_THS:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_FF_MT_THS) , 0 , 7 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
	
				break;
		
		case CMD_FM_DEBOUNCE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_FF_MT_COUNT) , 0 , 8 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
	
				break;

		case CMD_FM_DEBOUNCE_MODE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_FF_MT_THS) , 7 , 1 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
	
				break;

		case CMD_FMENABLE:
			{
				int latch = 0;
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);
	
				WriteRegValue((REG845X_FF_MT_CFG) , 3, 3, val);
				latch = ((val == 0 )? 0 : 1);
				WriteRegValue((REG845X_FF_MT_CFG) , 7, 1,latch);
				WriteRegValue((REG845X_FF_MT_CFG) , 6, 1, pChip->fm_threshold_logic);
				WriteRegValue((REG845X_CTRL_REG4) , 2, 1, 1);
	
				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
				
				break;
			}

		case CMD_FM_WAKE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG3) , 3, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRANS_CUTOFF:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_HP_FILTER_CUTOFF) , 0, 2, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;
			
		case CMD_TRANS_THS:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_TRANSIENT_THS) , 0, 7, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRANS_DEBOUNCE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_TRANSIENT_COUNT) , 0 , 8 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRANS_DEBOUNCE_MODE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_TRANSIENT_THS) , 7 , 1 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRANS_ENABLE:
			{
				int latch = 0;
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_TRANSIENT_CFG) , 1, 3, val);
				latch = ((val == 0 )? 0 : 1);
				WriteRegValue((REG845X_TRANSIENT_CFG) , 4, 1,latch);

				WriteRegValue((REG845X_CTRL_REG4) , 5, 1, 1);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
			}
			break;

		case CMD_TRANS_WAKE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG3) , 6, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_Z_LOCK_ANGLE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_BF_ZCOMP) , 0 , 3 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
	
				break;

		case CMD_TRIP_ANGLE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_BF_ZCOMP) , 6 , 2 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRIP_ANGLE_TH:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_THS_REG) , 3 , 5 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TRIP_ANGLE_HYS:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_THS_REG) , 0 , 3 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_ORNT_DEBOUNCE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_COUNT) , 0 , 8 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_ORNT_DEBOUNCE_MODE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_CFG) , 7 , 1 , val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_ORNT_ENABLE:
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PL_CFG) , 6, 1, val);
				WriteRegValue((REG845X_CTRL_REG4) , 4, 1, 1);
			
				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_ORNT_WAKE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG3) , 5, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
	
				break;

		case CMD_LPF_ENABLE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_HP_FILTER_CUTOFF) , 4, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_PULSE_LIMIT:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PULSE_TMLT) , 0, 8, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_PULSE_LATENCY:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PULSE_LTCY) , 0, 8, val);
		
				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_PULSE_WINDOW:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PULSE_WIND) , 0, 8, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);

				break;

		case CMD_TAP_ENABLE:
			{
				int latch = 0;
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_PULSE_CFG) , 0, 6, val);
	
				latch = ((val == 0 )? 0 : 1);
				WriteRegValue((REG845X_PULSE_CFG) , 6, 1,latch);

				WriteRegValue((REG845X_CTRL_REG4) , 3, 1, 1);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
			}
				break;

		case CMD_TAP_WAKE:
				// Put device in standby
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue((REG845X_CTRL_REG3) , 4, 1, val);

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);
				break;

		case CMD_ENFIFO:
			if(pChip->ChipId == ID_MMA8451)
			{
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);

				WriteRegValue(REG845X_F_SETUP, 0, 6, FMODE_WATERMARK(pChip->fifo_threshold));
				WriteRegValue(REG845X_F_SETUP, 6, 2, val);
				if(val == 1)
				{
					WriteRegValue(REG845X_CTRL_REG4, 6, 1, 1);
					WriteRegValue(REG845X_CTRL_REG4, 0, 1, 0);
				}
				else
				{
					WriteRegValue(REG845X_CTRL_REG4, 6, 1, 0);
					WriteRegValue(REG845X_CTRL_REG4, 0, 1, 1);
				}

				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);		

				return 0;
			}
			else
			{
				return -EINVAL;
			}
			break;


		case CMD_FIFO_TH:
			if(pChip->ChipId == ID_MMA8451)
			{
				// Put device in standby state
				WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0);
				WriteRegValue(REG845X_F_SETUP, 0, 6, FMODE_WATERMARK(val));
				// Put device to original state
				(pChip->state == 0) ? 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 0) : 	WriteRegValue((REG845X_CTRL_REG1) , 0, 1, 1);						
				return 0;
			}
			else
			{
				return -EINVAL;
			}
			break;
		default:
			printk("%s: DEFAULT... unhandled case [%d]\r\n", __func__, cmd);
			return -EINVAL;

	}
	return 0;
}

/*!
* This method is used to set initialize Orientation module in MMA845X chip with default settings.
* pChipInfo  : Chip structure of MMA845x.
* return 0 	      : For successful chip initialization, -EINVAL : For failure  
*/

static int InitOrientationModule(struct ChipInfo_t *pChip)
{
	int ret = 0;

	WriteRegValue((REG845X_PL_CFG) , 6, 1 , pChip->ornt_enable);
	ret = i2c_smbus_read_byte_data(pClient, (REG845X_PL_CFG));  	 

	WriteRegValue((REG845X_PL_BF_ZCOMP) , 6, 2 , pChip->bf_trip_angle);
	WriteRegValue((REG845X_PL_BF_ZCOMP) , 0, 3 , pChip->zlock_angle_ths);				//PL_BF_ZCOMP = 0x82
	
	/* Threshold */
	WriteRegValue((REG845X_PL_THS_REG) , 3, 5 , pChip->trip_angle_threshold);
	WriteRegValue((REG845X_PL_THS_REG) , 0, 3 , pChip->trip_angle_hysteresis);

	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), 0x10);

	return 0;
}

/*!
* This method is used to set initialize Motion module in MMA845X chip with default settings.
* pChipInfo  : Chip structure of MMA845x.
* return 0 	      : For successful chip initialization, -EINVAL : For failure  
*/

static int InitMotionModule(struct ChipInfo_t *pChip)
{
	bool latch;

	/* Set condition for motion detection */	
	WriteRegValue((REG845X_FF_MT_CFG) , 3, 3 , pChip->fm_enable); 					 // X Y Enable
	latch = ((pChip->fm_enable > 0 )? 1 : 0);
	WriteRegValue((REG845X_FF_MT_CFG) , 7, 1 , latch);
        WriteRegValue((REG845X_FF_MT_CFG) , 6, 1 , pChip->fm_threshold_logic);

	/* Set threshold value */
	WriteRegValue((REG845X_FF_MT_THS) , 0, 7 , pChip->fm_threshold); 

	/* Debounce */
	WriteRegValue((REG845X_FF_MT_COUNT) , 0, 8 , pChip->fm_debounce_count); 

	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), 0x04);

	return 0;
}

/*!
* This method is used to set initialize Tap detection module in MMA845X chip with default settings.
* pChipInfo  : Chip structure of MMA845x.
* return 0 	      : For successful chip initialization, -EINVAL : For failure  
*/

static int InitTapDetectionModule(struct ChipInfo_t *pChip)
{
	/* Set pulse detection configuration */
	bool latch;
	WriteRegValue((REG845X_PULSE_CFG) , 0, 6 , pChip->tap_enable);  				// X Y Enable
	latch = ((pChip->tap_enable > 0 )? 1 : 0);
	WriteRegValue((REG845X_PULSE_CFG) , 6, 1 , latch);  						//0x7F
	
	/* Set thresholds */
	WriteRegValue((REG845X_PULSE_THSX) , 0, 7 , pChip->tap_threshold_x); 				//0x20

	WriteRegValue((REG845X_PULSE_THSY) , 0, 7 , pChip->tap_threshold_y); 				//0x20

	WriteRegValue((REG845X_PULSE_THSZ) , 0, 7 , pChip->tap_threshold_z);				//0x40

	/* Set time limit */
	WriteRegValue((REG845X_PULSE_TMLT) , 0, 8 , pChip->pulse_time_limit); 				// 0x0

	WriteRegValue((REG845X_PULSE_LTCY) , 0, 8 , pChip->pulse_latency_time);				//0x0
      
	WriteRegValue((REG845X_PULSE_WIND) , 0, 8 , pChip->pulse_window_time);				//0x0

	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), 0x08);

	return 0;
}
/*!
* This method is used to set initialize Transient module in MMA845X chip with default settings.
* pChipInfo  : Chip structure of MMA845x.
* return 0 	      : For successful chip initialization, -EINVAL : For failure  
*/

static int InitTransientDetectionModule(struct ChipInfo_t *pChip)
{
	/* Set thresholds */
	bool latch;
	WriteRegValue((REG845X_TRANSIENT_CFG) , 1, 3 , pChip->trans_enable);  				// X Y Enable
	latch = ((pChip->trans_enable > 0 )? 1 : 0);
	WriteRegValue((REG845X_TRANSIENT_CFG) , 4, 1 , latch);						//0x16

	WriteRegValue((REG845X_TRANSIENT_THS) , 0, 7 , pChip->trans_threshold);  			//0x10

	WriteRegValue((REG845X_TRANSIENT_COUNT) , 0, 8 , pChip->trans_debounce_count); 			//0xA
	
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), 0x20);

	return 0;
}

/*!
* This method is used to set initialize MMA845X chip with default settings
* pChipInfo  : Chip structure of MMA845x.
* return 0 	      : For successful chip initialization, -EINVAL : For failure  
*/

static int InitializeChip(struct ChipInfo_t *pChipInfo)
{
	unsigned char ret = 0;
	int i = 0;
	
	if(pChipInfo == NULL)
	{
		printk("%s:: NULL chip pointer\r\n", __func__);
		return -ENOMEM;
	}

	pChip = pChipInfo;
	pClient = pChip->client;
	ChipType = pChip->ChipType;


	/* Setup interrupt pin as open drain */
	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG3));
	ret |= (1 << 0);
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG3), ret);

	/* Reset accelerometer chipset */
	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG2));
	ret |= (0x01 << 6);
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG2), ret);

	i = 100;
	do {
		ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG2));
		i--;
	}while (i && (ret & (0x01 << 6)));

	if(i == 0)
		printk("Accelerometer device reset failed\r\n");

	// Put device in Standby
	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG1));
	ret &= ~(0x03);
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), ret);

	/*
	 **  Configure sensor data for:
	 **    - store current XYZ data (ie., no FIFO)
	 **    - generate data ready at ODR rate
	 **    - enable event flags for new XYZ data
	 **
	 **  XYZ Data Event Flag Enable
	 */

	ret = 0x00;
	i2c_smbus_write_byte_data(pClient, (REG845X_XYZ_DATA_CFG), ret);

	if(pChip->enablefifo)
	{
		printk("%s:: FIFO enabled\r\n", __func__);
		i2c_smbus_write_byte_data(pClient, (REG845X_F_SETUP), (FMODE_CIRCULAR | FMODE_WATERMARK(pChip->fifo_threshold)));
	}

	/*
	 **  Configure sensor for:
	 **    - Sleep Mode Poll Rate of 1.56Hz
	 **    - System Output Data Rate of 200Hz (5ms)
	 **    - Full Scale of +/-8g
	 */
	ret = 0xf4; 
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), ret);

	/* Enable orientation functionality */
	InitOrientationModule(pChipInfo);

	/* Enable motion/freefall functionality */
	InitMotionModule(pChipInfo);

	/* Enable Tap detection functionality */
	InitTapDetectionModule(pChipInfo);

	/* Enable Transient detection functionality */
	InitTransientDetectionModule(pChipInfo);

	return 0;
}

/*!
* This method is used to read output sample data from MMA845X.
* type : data type(FIFO or normal. FIFO mode not supported).
* data : pointer to a buffer in which sample data to be read.
* return 0   : For successful chip initialization, -EINVAL : For failure  
*/

static int ReadChip( int type, void *data)
{
	int ret = 0;
	unsigned char *buff;
	int cnt;
	int i = 0;
	short *ps;
	short *pd;

	switch(type)
	{
		case ACCL_DATA:
			{
				buff = (unsigned char *)data;
				ret = i2c_smbus_read_i2c_block_data(pClient, (REG845X_OUT_X_MSB), 6, (u8 *) buff);

				ps = (short *)buff;
				pd = (short *)buff;
				for(i = 0; i < 3; i++)
				{

					*pd =  (short)__be16_to_cpu(*ps) >> 2;
					pd++;
					ps++;	
				}
			}
			break;

		case FIFO_DATA:
			{
				/* Clear OVRFLAG if any */
				ret  = (int)i2c_smbus_read_byte_data(pClient, (REG845X_STATUS));
#ifdef DEBUG				
				if(ret & F_OVR)
				{
					printk("[OVRF] ");
				}

				if(ret & F_WMRK)
				{
					printk("[WMRK] ");
				}
#endif
				if(ret & F_COUNT)
				{
					cnt = (ret & F_COUNT);
#ifdef DEBUG					
					printk("CNT: [%d] bytes availble", cnt);
#endif
					buff = (unsigned char *)data;
					ret = i2c_smbus_read_i2c_block_data(pClient, (REG845X_OUT_X_MSB), cnt * 6, (u8 *) buff);
#ifdef DEBUG					
					printk("(%d) bytes read \n", ret);
#endif

					buff = (unsigned char *)data;

#ifdef DEBUG						
					for(i = 0; i < (cnt * 6); i++)
					{
						if(i%6 == 0)
							addr = (REG845X_OUT_X_MSB);
						printk("\r\n REG [%02x] VAL [%02x]", addr, *buff);
						addr++;
						buff++;
					}
#endif
			
					ps = (unsigned short *)data;
					pd = (unsigned short *)data;
					for(i = 0; i < 3; i++)
					{
						*pd =  (unsigned short)__be16_to_cpu(*ps) >> 2;
						pd++;
						ps++;	
					}
					ret = cnt;
				}

				printk("\r\n");
			}
			break;
		
		case ACCL_LNDPRT:
			{
				u16 tmpBuf[4];
				u32 *p32;
				ret = i2c_smbus_read_i2c_block_data(pClient, (REG845X_OUT_X_MSB), 6, (u8 *) tmpBuf);

				ps = (unsigned short *)tmpBuf;
				pd = (unsigned short *)tmpBuf;

				for(i = 0; i < 3; i++)
				{
					*pd++ = __be16_to_cpu(*ps++);	
				}

				p32 = (u32 *)data;

				// YAW
				*p32 = tmpBuf[1] << 16 | tmpBuf[2] ;						// Y | Z
				p32++;

				//PITCH
				*p32 = tmpBuf[0] << 16 | tmpBuf[1] ;						// X | Y
				p32++;

				//ROLL
				*p32 = tmpBuf[1] << 16 | tmpBuf[0] ;						// Y | X
				p32++;

				buff = (unsigned char *)p32;
				ret = i2c_smbus_read_byte_data(pClient, (REG845X_PL_STATUS));

				*buff = 0;

				*buff |= ( (!!(ret & (0x01 << 7)) << 5) |					// Change
					   (!!(ret & (0x01 << 6)) << 4) |					// Lockout
					   ((ret & (0x03 << 1)))	|					// Sensor position
					   ((ret & (0x01 << 0)))						// Back/front position
						);
#ifdef DEBUG
				printk("Orientation: \r\n");
				printk("\t%s \r\n", (*buff & (0x01<<5)) ? "No change" : "Change");
				printk("\tLockout %s \r\n", (*buff & (0x01<<4)) ? "detected" : "not detected");
				switch((*buff & 0x06) >> 1)
				{
					case 0:
						printk("\tPortrait up \r\n");
						break;
					case 1:
						printk("\tPortrait down \r\n");
						break;
					case 2:
						printk("\tLandscape right \r\n");
						break;
					case 3:
						printk("\tLandscape left \r\n");
						break;
				}
				printk("\t%s \r\n", (*buff & (0x01<<0)) ? "Back" : "Front");
#endif
			}
			break;	

		case ACCL_FF_MT:
			{
				u8 	val = 0;
				buff = data;
				
				val = i2c_smbus_read_byte_data(pClient, (REG845X_FF_MT_SRC));

				*buff = val;
				*buff |= (0x00 << 6);								// Motion : Set this bit as per functionality 

#ifdef DEBUG			
				printk("Motion/Freefall: ");
				printk("Event %s \r\n", (*buff & (0x01<<7)) ? "active" : "inactive");
				if(*buff & (0x1 << 5))
				{
					printk("\t Z%s ", (*buff & (0x01<<4)) ? "-" : "+");
				}
				
				if(*buff & (0x1 << 3))
				{
					printk("Y%s ", (*buff & (0x01<<2)) ? "-" : "+");
				}
				
				if(*buff & (0x1 << 1))
				{
					printk("X%s ", (*buff & (0x01<<0)) ? "-" : "+");
				}

				printk("\r\n");
#endif

			}
			break;

		case ACCL_PULSE:
			{
				
				u8 	val = 0;

				buff = data;
				val = i2c_smbus_read_byte_data(pClient, (REG845X_PULSE_SRC));
			
				*buff = ((!!(val & (0x01 << 7)) << 7) |						// Valid
					 (!!(val & (0x01 << 3)) << 6) |						// 0-Single tap  1-Double tap
					 (!!(val & (0x01 << 6)) << 5) |						// Z-axis
					 (!!(val & (0x01 << 2)) << 4) |						// Z-direction
					 (!!(val & (0x01 << 5)) << 3) |						// Y-axis
					 (!!(val & (0x01 << 1)) << 2) |						// Y-direction
					 (!!(val & (0x01 << 4)) << 1) |						// X-axis
					 (!!(val & (0x01 << 0)) << 0) 						// X-direction
					 );
#ifdef DEBUG
				printk("Pulse ");
				printk("Event %s ", (*buff & (0x01<<7)) ? "active" : "inactive");
				printk("(%s tap) ", (*buff & (0x01<<3)) ? "Double" : "Single");

				if(*buff & (0x1 << 6))
					printk("Z%s ", (*buff & (0x01<<2)) ? "-" : "+");
				
				if(*buff & (0x1 << 5))
					printk("Y%s ", (*buff & (0x01<<1)) ? "-" : "+");
				
				if(*buff & (0x1 << 4))
					printk("X%s ", (*buff & (0x01<<0)) ? "-" : "+");
					
				printk("\r\n");
#endif

			}
			break;

		case ACCL_TRANS:
			{
				u8 	val = 0;
				
				buff = data;
				val = i2c_smbus_read_byte_data(pClient, (REG845X_TRANSIENT_SRC));
				
				*buff = val;
#ifdef DEBUG
				printk("Transient ");
				printk("Event %s ", (*buff & (0x01<<6)) ? "active" : "inactive");

				if(*buff & (0x1 << 5))
					printk("Z%s ", (*buff & (0x01<<4)) ? "-" : "+");
				
				if(*buff & (0x1 << 3))
					printk("Y%s ", (*buff & (0x01<<2)) ? "-" : "+");
				
				if(*buff & (0x1 << 1))
					printk("X%s ", (*buff & (0x01<<0)) ? "-" : "+");
					
				printk("\r\n");
#endif

			}
			break;

	}
	return ret;
}

/*!
* This method is used to enable interrupt.
* return 0   : Id successfully enabled interrupt , -ENOMEM : if chip pointer is NULL.  
*/

static int EnableInterrupt( int mask )
{
	int ret = 0;
	int prev = 0;
	struct i2c_client * pClient;

	if(pChip == NULL)
	{
		printk("%s:: NULL chip pointer\r\n", __func__);
		return -ENOMEM;
	}

	pClient = pChip->client;

	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG1));
	printk("Mode [0x%x]\r\n", ret & 0x03);
	prev = ret;
	ret &= ~0x01;
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), ret);

	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG4));
	ret = mask;
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), ret);
	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG4));
	printk("Interrupt Mask [0x%x]\r\n", ret);

	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), prev);

	return 0;
}

/*!
* This method is used to disable interrupt.
* return 0   : Id successfully disabled interrupt , -ENOMEM : if chip pointer is NULL.  
*/

static int DisableInterrupt( int mask )
{
	int ret = 0;
	int prev = 0;
	struct i2c_client * pClient;

	if(pChip == NULL)
	{
		printk("%s:: NULL chip pointer\r\n", __func__);
		return -ENOMEM;
	}

	pClient = pChip->client;

	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG1));
	prev = ret;
	ret &= ~0x01;
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), ret);

	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG4));
	ret &= ~mask;
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG4), ret);

	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), prev);

	return 0;
}

/*!
* This method is used to set chip mode as active or standby
* mode : mode to be set.
* return 0   : For successful mode initialization, -ENOMEM : if chip pointer is NULL.  
*/

static int SetChipMode( int mode )
{
	int ret = 0;
	struct i2c_client * pClient;

	if(pChip == NULL)
	{
		printk("%s:: NULL chip pointer\r\n", __func__);
		return -ENOMEM;
	}

	pClient = pChip->client;

	ret = i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG1));
	ret &= ~0x03;
	ret |= mode;
	i2c_smbus_write_byte_data(pClient, (REG845X_CTRL_REG1), ret);
	return 0;
}

/*!
* This method is used to read operation mode
* return 0 : mode set, -ENOMEM : if chip pointer is NULL.  
*/

static int GetChipMode( void )
{
	struct i2c_client * pClient;

	if(pChip == NULL)
	{
		printk("%s:: NULL chip pointer\r\n", __func__);
		return -ENOMEM;
	}

	pClient = pChip->client;
	ChipType = pChip->ChipType;

	return (i2c_smbus_read_byte_data(pClient, (REG845X_CTRL_REG1)) & 0x03);
}

/*!
* This method is used to get interrupt source.
* return interrupt source read from register. 
*/

static int GetIntSource( void )
{
	return (i2c_smbus_read_byte_data(pClient, (REG845X_INT_SOURCE)));
}
 
/* Sysfs info */
static struct device_attribute common_attributes[] = {
	DECLARE_ATTR(name, 0444, name_show, NULL),
	DECLARE_ATTR(vendor, 0444, vendor_show, NULL),
	DECLARE_ATTR(device_id, 0444, devid_show, NULL),
	DECLARE_ATTR(version, 0444, version_show, NULL),
	DECLARE_ATTR(type, 0444, type_show, NULL),
	DECLARE_ATTR(max_range, 0444, max_range_show, NULL),
	DECLARE_ATTR(max_resolution, 0444, max_res_show, NULL),
	DECLARE_ATTR(nominal_power, 0444, nominal_power_show, NULL),
	DECLARE_ATTR(operation_mode, 0666, operation_mode_show, operation_mode_store),
	DECLARE_ATTR(odr, 0666, odr_show, odr_store),
	DECLARE_ATTR(supported_odr, 0444, supported_odr_show, NULL),
	DECLARE_ATTR(oversampling, 0666, oversampling_show, oversampling_store),
	DECLARE_ATTR(oversampling_values, 0444, oversampling_values_show, NULL),
	DECLARE_ATTR(auto_wakeup, 0666, auto_wakeup_show, auto_wakeup_store),
	DECLARE_ATTR(resolutions, 0444, resolutions_show,  NULL),
	DECLARE_ATTR(resolution, 0666, resolution_show, resolution_store),
	DECLARE_ATTR(range, 0666, range_show, range_store),
	DECLARE_ATTR(range_low, 0444, range_low_show,  NULL),
	DECLARE_ATTR(range_high, 0444, range_high_show,  NULL),
	DECLARE_ATTR(precision, 0444, precision_show,  NULL),
	DECLARE_ATTR(sample_rate, 0444, sample_rate_show,  NULL),
	DECLARE_ATTR(fifo_enable, 0666, fifo_enable_show,  fifo_enable_store),
	DECLARE_ATTR(fifo_threshold, 0666, fifo_threshold_show,  fifo_threshold_store),
	DECLARE_ATTR(fifo_threshold_min, 0444, fifo_threshold_min_show, NULL),
	DECLARE_ATTR(fifo_threshold_max, 0444, fifo_threshold_max_show, NULL),
	DECLARE_ATTR(value, 0444, value_show,  NULL),
	DECLARE_ATTR(calibration_offset, 0666, calibration_offset_show, calibration_offset_store),
	DECLARE_ATTR(poll_time, 0666, poll_time_show, poll_time_store),
};

static struct device_attribute motion_attributes[] = {
	DECLARE_ATTR(threshold, 0666, fm_threshold_show, fm_threshold_store),
	DECLARE_ATTR(threshold_min, 0444, fm_threshold_min_show,  NULL),
	DECLARE_ATTR(threshold_max, 0444, fm_threshold_max_show,  NULL),
	DECLARE_ATTR(threshold_step, 0444, fm_threshold_step_show,  NULL),
	DECLARE_ATTR(threshold_num_step, 0444, fm_threshold_num_step_show,  NULL),
	DECLARE_ATTR(debounce_count, 0666, fm_debounce_count_show, fm_debounce_count_store),
	DECLARE_ATTR(debounce_count_min, 0444, fm_debounce_count_min_show,  NULL),
	DECLARE_ATTR(debounce_count_max, 0444, fm_debounce_count_max_show,  NULL),
	DECLARE_ATTR(debounce_time_step, 0444, fm_debounce_time_step_show,  NULL),
	DECLARE_ATTR(event_type, 0666, fm_event_type_show, fm_event_type_store),
	DECLARE_ATTR(enable, 0666, fm_enable_show, fm_enable_store),
	DECLARE_ATTR(wake_on_event, 0666, fm_wake_on_event_show, fm_wake_on_event_store),
};

static struct device_attribute transient_attributes[] = {
	DECLARE_ATTR(cut_off_frequencies, 0666, trans_cut_off_frequencies_show, trans_cut_off_frequencies_store),
	DECLARE_ATTR(threshold, 0666, trans_threshold_show, trans_threshold_store),
	DECLARE_ATTR(threshold_min, 0444, trans_threshold_min_show,  NULL),
	DECLARE_ATTR(threshold_max, 0444, trans_threshold_max_show,  NULL),
	DECLARE_ATTR(threshold_step, 0444, trans_threshold_step_show,  NULL),
	DECLARE_ATTR(threshold_num_step, 0444, trans_threshold_num_step_show,  NULL),
	DECLARE_ATTR(debounce_count, 0666, trans_debounce_count_show, trans_debounce_count_store),
	DECLARE_ATTR(debounce_count_min, 0444, trans_debounce_count_min_show,  NULL),
	DECLARE_ATTR(debounce_count_max, 0444, trans_debounce_count_max_show,  NULL),
	DECLARE_ATTR(debounce_time_step, 0444, trans_debounce_time_step_show,  NULL),
	DECLARE_ATTR(event_type, 0666, trans_event_type_show, trans_event_type_store),
	DECLARE_ATTR(enable, 0666, trans_enable_show, trans_enable_store),
	DECLARE_ATTR(wake_on_event, 0666, trans_wake_on_event_show, trans_wake_on_event_store),
};

static struct device_attribute orientation_attributes[] = {
	DECLARE_ATTR(z_lock_angle_threshold_min, 0444, z_lock_angle_threshold_min_show,  NULL),
	DECLARE_ATTR(z_lock_angle_threshold, 0666, z_lock_angle_threshold_show, z_lock_angle_threshold_store),
	DECLARE_ATTR(z_lock_angle_threshold_max, 0444, z_lock_angle_threshold_max_show,  NULL),
	DECLARE_ATTR(z_lock_angle_threshold_values, 0444, z_lock_angle_threshold_values_show,  NULL),
	DECLARE_ATTR(z_lock_angle_threshold_num_step, 0444, z_lock_angle_threshold_num_step_show,  NULL),
	DECLARE_ATTR(back_front_trip_angle_threshold, 0666, back_front_trip_angle_threshold_show, back_front_trip_angle_threshold_store),
	DECLARE_ATTR(back_front_trip_angle_threshold_min, 0444, back_front_trip_angle_threshold_min_show,  NULL),
	DECLARE_ATTR(back_front_trip_angle_threshold_max, 0444, back_front_trip_angle_threshold_max_show,  NULL),
	DECLARE_ATTR(back_front_trip_angle_threshold_step, 0444, back_front_trip_angle_threshold_step_show,  NULL),
	DECLARE_ATTR(back_front_trip_angle_threshold_num_step, 0444, back_front_trip_angle_threshold_num_step_show,  NULL),
	DECLARE_ATTR(trip_angle_threshold, 0666, trip_angle_threshold_show, trip_angle_threshold_store),
	DECLARE_ATTR(trip_angle_threshold_values, 0444, trip_angle_threshold_values_show,  NULL),
	DECLARE_ATTR(trip_angle_threshold_num_steps, 0444, trip_angle_threshold_num_steps_show,  NULL),
	DECLARE_ATTR(trip_angle_hysteresis, 0666, trip_angle_hysteresis_show, trip_angle_hysteresis_store),
	DECLARE_ATTR(trip_angle_hysteresis_values, 0444, trip_angle_hysteresis_values_show,  NULL),
	DECLARE_ATTR(trip_angle_hysteresis_num_steps, 0444, trip_angle_hysteresis_num_steps_show,  NULL),
	DECLARE_ATTR(debounce_count, 0666, ornt_debounce_count_show, ornt_debounce_count_store),
	DECLARE_ATTR(debounce_count_min, 0444, ornt_debounce_count_min_show,  NULL),
	DECLARE_ATTR(debounce_count_max, 0444, ornt_debounce_count_max_show,  NULL),
	DECLARE_ATTR(debounce_time_step, 0444, ornt_debounce_time_step_show,  NULL),
	DECLARE_ATTR(event_type, 0666, ornt_event_type_show, ornt_event_type_store),
	DECLARE_ATTR(enable, 0666, ornt_enable_show, ornt_enable_store),
	DECLARE_ATTR(wake_on_event, 0666, ornt_wake_on_event_show, ornt_wake_on_event_store), 
};

static struct device_attribute tap_attributes[] = {
	DECLARE_ATTR(threshold_min, 0444, sentap_threshold_min_show,  NULL),
	DECLARE_ATTR(threshold_max, 0444, sentap_threshold_max_show,  NULL),
	DECLARE_ATTR(threshold_step, 0444, sentap_threshold_step_show,  NULL),
	DECLARE_ATTR(threshold_num_step, 0444, sentap_threshold_num_step_show,  NULL),
	DECLARE_ATTR(low_pass_filter, 0666, low_pass_filter_show, low_pass_filter_store), 
	DECLARE_ATTR(pulse_time_limit, 0666, pulse_time_limit_show, pulse_time_limit_store), 
	DECLARE_ATTR(pulse_time_limit_min, 0444, pulse_time_limit_min_show,  NULL),
	DECLARE_ATTR(pulse_time_limit_max, 0444, pulse_time_limit_max_show,  NULL),
	DECLARE_ATTR(pulse_time_limit_step, 0444, pulse_time_limit_step_show,  NULL),
	DECLARE_ATTR(pulse_latency_time, 0666, pulse_latency_time_show, pulse_latency_time_store),
    DECLARE_ATTR(pulse_latency_time_min, 0444, pulse_latency_time_min_show,  NULL),
	DECLARE_ATTR(pulse_latency_time_max, 0444, pulse_latency_time_max_show,  NULL),
	DECLARE_ATTR(pulse_latency_time_step, 0444, pulse_latency_time_step_show,  NULL),
	DECLARE_ATTR(pulse_window_time, 0666, pulse_window_time_show, pulse_window_time_store), 
	DECLARE_ATTR(pulse_window_time_min, 0444, pulse_window_time_min_show,  NULL),
	DECLARE_ATTR(pulse_window_time_max, 0444, pulse_window_time_max_show,  NULL),
	DECLARE_ATTR(pulse_window_time_step, 0444, pulse_window_time_step_show,  NULL),
	DECLARE_ATTR(event_type_single, 0666, event_type_single_show, event_type_single_store), 
	DECLARE_ATTR(event_type_double, 0666, event_type_double_show, event_type_double_store), 
	DECLARE_ATTR(threshold_xyz, 0666, threshold_xyz_show, threshold_xyz_store), 
	DECLARE_ATTR(enable, 0666, tap_enable_show, tap_enable_store),
	DECLARE_ATTR(wake_on_event, 0666, sentap_wake_on_event_show, sentap_wake_on_event_store),
};

static struct SysfsInfo_t SysfsInfo[] = {
	{NULL, &common_attributes[0], (sizeof(common_attributes)/sizeof(common_attributes[0])), 1},
	{"motion_detection", &motion_attributes[0], (sizeof(motion_attributes)/sizeof(motion_attributes[0])), 1},
	{"orientation_detection", &orientation_attributes[0], (sizeof(orientation_attributes)/sizeof(orientation_attributes[0])), 1},
	{"transient_detection", &transient_attributes[0], (sizeof(transient_attributes)/sizeof(transient_attributes[0])), 1},
	{"tap_detection", tap_attributes, (sizeof(tap_attributes)/sizeof(tap_attributes[0])), 1},
};

struct ChipInfo_t mma8451Chip = {
	.id = MMA845X,
	.ChipId = ID_MMA8451,
	.Init = InitializeChip,
	.Read = ReadChip,
	.EnableInt = EnableInterrupt,
	.DisableInt = DisableInterrupt,
	.name = "MMA8451Q",
	.devtype = 1,
	.maxrange = FLOAT_TO_FIXP(156.96), 						// (4 * 9.81)
	.maxres = FLOAT_TO_FIXP(0.25),							// @ 2G-14bit
	.Debounce_Count_Min = 0,
	.Debounce_Count_Max = 255,
	.zlock_ths_min = 14,
	.zlock_ths_max = 42,
	.zlock_angle_step = 8,
	.zlock_angle_step_size = 4,
	.bf_trip_min = 65,
	.bf_trip_max = 80,
	.bf_trip_angle_step = 5,
	.bf_trip_step = 4,
	.trip_angle_min = 15,
	.trip_angle_max = 75,
	.trip_angle_hys_min = 0,
	.trip_angle_hys_max = 24,
	.poll_time = 5,	

	.enablefifo = 0,
	.fifo_threshold = 20,
	.min_fifo_th = 1,
	.max_fifo_th = 32,
	.state = 2,
	.pOdrList = (FIXPOINT *)OdrList,
	.pSamplingList = (char **)SamplingList,
	.pSupportedResolution = (int *)SupportedResolution,
	.pSupportedRange = (int *)SupportedRange,
	.pZ_lock_angle_thresholdList = (int *)Z_lock_angle_thresholdList,
	.pBack_front_trip_angle_thresholdList = (int *)Back_front_trip_angle_thresholdList,
	.pTrip_angle_thresholdList = (int *)Trip_angle_thresholdList,
	.pTrip_angle_hysteresisList = (int *)Trip_angle_hysteresisList,
	.pPulse_time_limitList_LPF_EN0 = (FIXPOINT *)Pulse_time_limitList_LPF_EN0,
	.pMax_pulse_time_limitList_LPF_EN0 = (FIXPOINT *)Max_pulse_time_limitList_LPF_EN0,
	.pPulse_time_limitList_LPF_EN1 = (FIXPOINT *)Pulse_time_limitList_LPF_EN1,
	.pMax_pulse_time_limitList_LPF_EN1= (FIXPOINT *)Max_pulse_time_limitList_LPF_EN1,
	.pPulse_latency_timeList_LPF_EN0 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN0,
	.pPulse_latency_timeList_LPF_EN1 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN1,
	.pPulse_window_timeList_LPF_EN0 = (FIXPOINT *)Pulse_window_timeList_LPF_EN0,
	.pMax_pulse_window_timeList_LPF_EN0 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN0,
	.pPulse_window_timeList_LPF_EN1 = (FIXPOINT *)Pulse_window_timeList_LPF_EN1,
	.pMax_pulse_window_timeList_LPF_EN1 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN1,
	.pFm_debounce_countList = (FIXPOINT *)Fm_debounce_countList,
	.pTrans_debounce_countList = (FIXPOINT *)Trans_debounce_countList,
	.pOrnt_debounce_countList = (FIXPOINT *)Ornt_debounce_countList,
	.pTrans_cut_off_frequenciesList = (FIXPOINT *)Trans_cut_off_frequenciesList,
	.pNominal_powerList = (int *)Nominal_powerList,

	.pSysfsInfo = SysfsInfo,
	.SysfsInfoSize = sizeof(SysfsInfo)/sizeof(SysfsInfo[0]),

	.SetMode = SetChipMode,
	.GetMode = GetChipMode,
	.GetIntSrc = GetIntSource,
	.SetRegVal = SetRegVal,
	.SetCalOffset = SetCalOffset,
	.SetThXYZ = SetThXYZ,
};

struct ChipInfo_t mma8452Chip = {
	.id = MMA845X,
	.ChipId = ID_MMA8452,
	.Init = InitializeChip,
	.Read = ReadChip,
	.EnableInt = EnableInterrupt,
	.DisableInt = DisableInterrupt,
	.name = "MMA8452Q",
	.devtype = 1,
	.maxrange = FLOAT_TO_FIXP(156.96), 	// (16 * 9.81)
	.maxres = FLOAT_TO_FIXP(0.98),		// @ 2G-12bit
	.Debounce_Count_Min = 0,
	.Debounce_Count_Max = 255,
	.zlock_ths_min = 29,
	.zlock_ths_max = 29,
	.zlock_angle_step = 1,
	.zlock_angle_step_size = 0,
	.bf_trip_min = 75,
	.bf_trip_max = 75,
	.bf_trip_angle_step = 0,
	.bf_trip_step = 1,
	.poll_time = 5,	

	.enablefifo = 0,
	.fifo_threshold = 0,
	.min_fifo_th = 0,
	.max_fifo_th = 0,
	.state = 2,

	.pOdrList = (FIXPOINT *)OdrList,
	.pSamplingList = (char **)SamplingList,
	.pSupportedResolution = (int *)MMA8452Q_SupportedResolution,
	.pSupportedRange = (FIXPOINT *)SupportedRange,
	.pZ_lock_angle_thresholdList = (int *)MMA8452Q_Z_lock_angle_thresholdList ,
	.pBack_front_trip_angle_thresholdList = (int *)MMA8452Q_Back_front_trip_angle_thresholdList,
	.pTrip_angle_thresholdList = (int *)MMA8452Q_Trip_angle_thresholdList,
	.pTrip_angle_hysteresisList = (int *)MMA8452Q_Trip_angle_hysteresisList,
	.pPulse_time_limitList_LPF_EN0 = (FIXPOINT *)Pulse_time_limitList_LPF_EN0,
	.pMax_pulse_time_limitList_LPF_EN0 = (FIXPOINT *)Max_pulse_time_limitList_LPF_EN0,
	.pPulse_time_limitList_LPF_EN1 = (FIXPOINT *)Pulse_time_limitList_LPF_EN1,
	.pMax_pulse_time_limitList_LPF_EN1= (FIXPOINT *)Max_pulse_time_limitList_LPF_EN1,
	.pPulse_latency_timeList_LPF_EN0 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN0,
	.pPulse_latency_timeList_LPF_EN1 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN1,
	.pPulse_window_timeList_LPF_EN0 = (FIXPOINT *)Pulse_window_timeList_LPF_EN0,
	.pMax_pulse_window_timeList_LPF_EN0 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN0,
	.pPulse_window_timeList_LPF_EN1 = (FIXPOINT *)Pulse_window_timeList_LPF_EN1,
	.pMax_pulse_window_timeList_LPF_EN1 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN1,
	.pFm_debounce_countList = (FIXPOINT *)Fm_debounce_countList,
	.pTrans_debounce_countList = (FIXPOINT *)Trans_debounce_countList,
	.pOrnt_debounce_countList = (FIXPOINT *)Ornt_debounce_countList,
	.pTrans_cut_off_frequenciesList = (FIXPOINT *)Trans_cut_off_frequenciesList,
	.pNominal_powerList = (FIXPOINT *)Nominal_powerList,

	.pSysfsInfo = SysfsInfo,
	.SysfsInfoSize = sizeof(SysfsInfo)/sizeof(SysfsInfo[0]),

	.SetMode = SetChipMode,
	.GetMode = GetChipMode,
	.GetIntSrc = GetIntSource,
	.SetRegVal = SetRegVal,
	.SetCalOffset = SetCalOffset,
	.SetThXYZ = SetThXYZ,
};

struct ChipInfo_t mma8453Chip = {
	.id = MMA845X,
	.ChipId = ID_MMA8453,
	.Init = InitializeChip,
	.Read = ReadChip,
	.EnableInt = EnableInterrupt,
	.DisableInt = DisableInterrupt,
	.name = "MMA8453Q",
	.devtype = 1,
	.maxrange = FLOAT_TO_FIXP(156.96),	// (16 * 9.81)
	.maxres = FLOAT_TO_FIXP(3.9),		// @ 2G-10bit
	.Debounce_Count_Min = 0,
	.Debounce_Count_Max = 255,
	.zlock_ths_min = 29,
	.zlock_ths_max = 29,
	.zlock_angle_step = 1,
	.zlock_angle_step_size = 0,
	.bf_trip_min = 75,
	.bf_trip_max = 75,
	.bf_trip_angle_step = 0,
	.bf_trip_step = 1,
	.poll_time = 5,	

	.enablefifo = 0,
	.fifo_threshold = 0,
	.min_fifo_th = 0,
	.max_fifo_th = 0,
	.state = 2,

	.pOdrList = (FIXPOINT *)OdrList,
	.pSamplingList = (char **)SamplingList,
	.pSupportedResolution = (int *)MMA8453Q_SupportedResolution,
	.pSupportedRange = (int *)SupportedRange,
	.pZ_lock_angle_thresholdList = (int *)MMA8452Q_Z_lock_angle_thresholdList ,
	.pBack_front_trip_angle_thresholdList = (int *)MMA8452Q_Back_front_trip_angle_thresholdList,
	.pTrip_angle_thresholdList = (int *)MMA8452Q_Trip_angle_thresholdList,
	.pTrip_angle_hysteresisList = (int *)MMA8452Q_Trip_angle_hysteresisList,
	.pPulse_time_limitList_LPF_EN0 = (FIXPOINT *)Pulse_time_limitList_LPF_EN0,
	.pMax_pulse_time_limitList_LPF_EN0 = (FIXPOINT *)Max_pulse_time_limitList_LPF_EN0,
	.pPulse_time_limitList_LPF_EN1 = (FIXPOINT *)Pulse_time_limitList_LPF_EN1,
	.pMax_pulse_time_limitList_LPF_EN1= (FIXPOINT *)Max_pulse_time_limitList_LPF_EN1,
	.pPulse_latency_timeList_LPF_EN0 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN0,
	.pPulse_latency_timeList_LPF_EN1 = (FIXPOINT *)Pulse_latency_timeList_LPF_EN1,
	.pPulse_window_timeList_LPF_EN0 = (FIXPOINT *)Pulse_window_timeList_LPF_EN0,
	.pMax_pulse_window_timeList_LPF_EN0 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN0,
	.pPulse_window_timeList_LPF_EN1 = (FIXPOINT *)Pulse_window_timeList_LPF_EN1,
	.pMax_pulse_window_timeList_LPF_EN1 = (FIXPOINT *)Max_pulse_window_timeList_LPF_EN1,
	.pFm_debounce_countList = (FIXPOINT *)Fm_debounce_countList,
	.pTrans_debounce_countList = (FIXPOINT *)Trans_debounce_countList,
	.pOrnt_debounce_countList = (FIXPOINT *)Ornt_debounce_countList,
	.pTrans_cut_off_frequenciesList = (FIXPOINT *)Trans_cut_off_frequenciesList,
	.pNominal_powerList = (FIXPOINT *)Nominal_powerList,
	
	.pSysfsInfo = SysfsInfo,
	.SysfsInfoSize = sizeof(SysfsInfo)/sizeof(SysfsInfo[0]),

	.SetMode = SetChipMode,
	.GetMode = GetChipMode,
	.GetIntSrc = GetIntSource,
	.SetRegVal = SetRegVal,
	.SetCalOffset = SetCalOffset,
	.SetThXYZ = SetThXYZ,
};

