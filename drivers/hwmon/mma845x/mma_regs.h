/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_regs.h
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA845X  declarations and defines required for MMA845X driver
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
#ifndef __MMA_REGS_H__
#define __MMA_REGS_H__

#include <linux/wakelock.h>

#define DRIVER_VERSION	"Ver. 1.0"
#define VENDOR_NAME	"Freescale Semiconductors" 

/* Macros for fixed point arithematic */
#define FP_SHIFT 3     // shifts to produce a fixed-point number
#define FP_SCALE 1000  // scaling factor
#define PRECISION		1000

#define INT_TO_FIXP(n) ((FIXPOINT)((n * FP_SCALE)))
#define FLOAT_TO_FIXP(n) ((FIXPOINT)((float)n * FP_SCALE))
#define FIXP_INT_PART(n) (n / FP_SCALE)
#define FIXP_DEC_PART(n) (n % FP_SCALE)

typedef unsigned int FIXPOINT;

enum {
       MMA845X = 0x51,
       MMA8450,
       MMA7660FC,
};

#define ANDROID_RELEASE	"10.2"
#define MODULE_NAME	"mma845x"

#define REG(x)	(REG845X_##x)

/* Data types */
#define ACCL_DATA	0x01
#define ACCL_LNDPRT	0x02
#define FIFO_DATA	0x03
#define ACCL_FF_MT	0x04
#define ACCL_PULSE	0x05
#define ACCL_TRANS	0x06
#define WAKE_UP		0x07
#define ACCL_FF_MT_2	0x08

/* Interrupt source macros */
#define SRC_ASLP	_BIT(7)			// Auto-SLEEP/WAKE
#define SRC_FIFO	_BIT(6)			// FIFO interrupt 
#define SRC_TRANS	_BIT(5)			// Transient interrupt
#define SRC_LNDPRT 	_BIT(4)			// Landscape/portrait (orientation)
#define SRC_PULSE 	_BIT(3)			// Pulse
#define SRC_FF_MT 	_BIT(2)			// Freefall/motion interrupt
#define SRC_FF_MT_2	_BIT(1)			// Freefall/motion 2 interrupt
#define SRC_DRDY 	_BIT(0)			// Dataready interrupt

/* Command codes for registers */
enum {
	CMD_ODR = 0x00,
	CMD_SAMPLE,
	CMD_RANGE,
	CMD_Z_LOCK_ANGLE,
	CMD_TRIP_ANGLE,
	CMD_TRIP_ANGLE_TH,
	CMD_TRIP_ANGLE_TH_P_L,
	CMD_TRIP_ANGLE_TH_L_P,
	CMD_TRIP_ANGLE_HYS,
	CMD_MODE,
	CMD_WAKEUP,
	CMD_RESOLUTION,
	CMD_FMENABLE,
	CMD_FM_WAKE,
	CMD_TRANS_ENABLE,
	CMD_ORNT_ENABLE,
	CMD_ORNT_WAKE,
	CMD_TAP_ENABLE,
	CMD_FM_THS,
	CMD_FM_DEBOUNCE,
	CMD_FM_DEBOUNCE_MODE,
	CMD_ORNT_DEBOUNCE,
	CMD_ORNT_DEBOUNCE_MODE,
	CMD_TRANS_DEBOUNCE,
	CMD_TRANS_DEBOUNCE_MODE,
	CMD_TRANS_CUTOFF,
	CMD_TRANS_THS,
	CMD_TRANS_WAKE,
	CMD_LPF_ENABLE,
	CMD_PULSE_LIMIT,
	CMD_PULSE_LATENCY,
	CMD_PULSE_WINDOW,
	CMD_TAP_WAKE,
	CMD_1G_LOCKOUT_THS,
	CMD_FMENABLE_2,
	CMD_FM_DEBOUNCE_2,
	CMD_FM_THS_2,
	CMD_FM_WAKE_2,
	CMD_TAP_THS ,
	CMD_TAP_DEBOUNCE,
	CMD_FILT_ENABLE,
	CMD_ENFIFO,
	CMD_FIFO_TH,
};


/* sysfs entries table */
struct SysfsInfo_t {
	char 	* grpName;			// sysfs group name;
						// NULL is treated as ungrouped
	struct 	device_attribute *AttrEntry;	// Pointer to attribute table
	int 	TotalEntries;			// Number of attributes
	int 	Instance;			// No. of instances for group
};

struct ChipInfo_t{
	int id;
	int ChipId;
	int ChipType;
	char name[16];
	int devtype;

	/* Parameters */
	unsigned int maxres;			// Maximum resolution
	unsigned int maxrange;			// Max. accleration
	int enablefifo;
	int fifo_threshold;
	int min_fifo_th;
	int max_fifo_th;

	FIXPOINT * pOdrList;
	char 	 ** pSamplingList;
	int * pSupportedResolution;
	int * pSupportedRange;
	FIXPOINT * pCalibration_offsetList;
	FIXPOINT * pZ_lock_angle_thresholdList;
	int * pBack_front_trip_angle_thresholdList;
	int * pTrip_angle_thresholdList;
	FIXPOINT * pTrip_angle_hysteresisList;
	FIXPOINT * pPulse_time_limitList_LPF_EN0;	
	FIXPOINT * pMax_pulse_time_limitList_LPF_EN0;
	FIXPOINT * pPulse_time_limitList_LPF_EN1;
	FIXPOINT * pMax_pulse_time_limitList_LPF_EN1;
	FIXPOINT * pPulse_latency_timeList_LPF_EN0;
	FIXPOINT * pPulse_latency_timeList_LPF_EN1;
	FIXPOINT * pPulse_window_timeList_LPF_EN0;
	FIXPOINT * pMax_pulse_window_timeList_LPF_EN0;
	FIXPOINT * pPulse_window_timeList_LPF_EN1;
	FIXPOINT * pMax_pulse_window_timeList_LPF_EN1;
	FIXPOINT * pFm_debounce_countList;
	FIXPOINT * pTrans_debounce_countList;
	FIXPOINT * pOrnt_debounce_countList;
	FIXPOINT * pTrans_cut_off_frequenciesList;
	int * pNominal_powerList;
	int Debounce_Count_Min;
	int Debounce_Count_Max;

	/* current Values*/
	int state;
	int odr;
	int oversampling;
	int wakeup;
	int resolution;
	int range;
	int value;
	int calibration_offset;
	int xCalOffset;
	int yCalOffset;
	int zCalOffset;
	int zlock_ths_min;
	int zlock_ths_max;
	int zlock_angle_step;
	int zlock_angle_step_size;
	int zlock_angle_ths;
	int bf_trip_min;
	int bf_trip_max;
	int bf_trip_angle;
	int bf_trip_angle_step;
	int bf_trip_step;
	int trip_angle_min;
	int trip_angle_max;
	int trip_angle_threshold;
	int trip_angle_hysteresis;
	int trip_angle_hys_min;
	int trip_angle_hys_max;
	int pulse_time_limit;
	FIXPOINT pulse_time_step;
	int max_pulse_time_limit;
	int pulse_latency_time;
	int pulse_latency_step;
	int pulse_window_time;
	int pulse_window_step;
	int max_pulse_window_time;
	int tap_threshold_x;
	int tap_threshold_y;
	int tap_threshold_z;
	int fm_debounce_count;
        int fm_debounce_function;
	int fm_debounce_time_step;
	int fm_threshold;
	int fm_threshold_logic;
	int trans_threshold;   
	int trans_debounce_count;
	int trans_debounce_function;
	int ornt_debounce_count;
	int ornt_debounce_function;
	int cut_off_frequency;
	int LPF;
	int cutoff_freq_sel;
	int fm_wakeon_event;
	int trans_wakeon_event;
	int ornt_wakeon_event;
	int tap_wakeon_event;
	int fm_enable;
	int ornt_enable;
	int trans_enable;
	int tap_enable;
      	// current values for MMA7660FC
	int sleep_count;
	int interrupt_set;
	int tap_detect;
	int sample_rate;
	int tap_debounce_count;
	int tap_debounce_step;
	int tap_threshold;
	int tap_ths_min;
	int tap_ths_max;
	int tap_ths_step;
	int tap_ths_step_size;
	int tilt_debounce_filter;


	int poll_time;
	
	struct i2c_client * client;
	struct SysfsInfo_t *pSysfsInfo;
	int SysfsInfoSize;

	/* Function pointers */
	int (*Init)( struct ChipInfo_t * );
	int (*Read)( int, void * );
	int (*SetMode)( int );
	int (*GetMode)( void );
	int (*EnableInt)( int );
	int (*DisableInt)( int );
	int (*GetIntSrc)( void );
	int (*SetRegVal)(int cmd, int val);
	int (*SetCalOffset)(int val[]);
	int (*SetThXYZ)(FIXPOINT val[]);
};

typedef struct {
	short x;
	short y;
	short z;
}AcclData_t, *pAcclData_t;

struct mxc_mma_device_t{
	struct ChipInfo_t *pChip;

	/* Character (FIFO) layer members */
	int major;
	char devname[32];
	int version;

	/* Control information */

	int aflag;	
	int fm_event_type;
	int trans_event_type;
	int ornt_event_type;
	int event_type_single;
	int event_type_double;

	/* Kernel object */
	struct kobject *kobj;

	/* Input layer members */
	struct input_dev *inp1;
	struct input_dev *inp2;

	struct class *class;
	struct device *sys_device;   		// Common entries
	struct device *sys_motion_dev;		// Motion entries
	struct device *sys_ort_dev;   		// Orientation entries
	struct device *sys_trans_dev;   	// Transient entries
	struct device *sys_tap_dev;   		// Tap entries

	struct wake_lock mxc_wake_lock;

};

/* Function prototypes */
int UpdateAcclFiFo(void * buff);
unsigned int FindMin(int *array,int inc,int offset,int count);
unsigned int FindMax(int *array,int inc,int offset,int count);
int GetAcclerometerFlag(void);
int SetAcclerometerFlag(int AcclFlag);

#endif // __MXC_REGS_H__
