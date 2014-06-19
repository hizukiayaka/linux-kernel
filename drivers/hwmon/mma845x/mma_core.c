/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_core.c
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: MMA845X driver module implementation, implementation for driver 
 			  registrationd, interrupt handling and sysfs layer initialization. 
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
#include <mach/hardware.h>
#ifdef CONFIG_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <linux/slab.h>
#include <linux/mma845x.h>

#include "mma_regs.h"
#include "mma845x.h"
/*
 * macro define
 */

/*! Default I2C slave address for MMA845x chipsets */
#define MMA845X_I2C_ADDR	0x1d
#define DEVICE_NAME		"mma"

/*! Macro defination to enable interrupt 2. */
#define ENABLE_INT2

/*
 * extern declarations 
 */
extern const struct file_operations mma_fops;
extern int      InitializeInputInterface(struct mxc_mma_device_t *pDev);
extern int      DeInitializeInputInterface(struct mxc_mma_device_t *pDev);
extern int      ReportEvent(struct mxc_mma_device_t *pDev, int type,
			    void *buff);
extern int      InitializeSysfs(struct i2c_client *pDev);
extern int      DeInitializeSysfs(struct mxc_mma_device_t *pDev);

/*
 * Function prototypes 
 */
static int      mma845x_probe(struct i2c_client *client,
			      const struct i2c_device_id *id);
static int      mma845x_remove(struct i2c_client *client);
static int      mma845x_suspend(struct i2c_client *client,
				pm_message_t state);
static int      mma845x_resume(struct i2c_client *client);
static int      IdentifyChipset(struct i2c_client *client);

/*
 * extern variables
 */

extern struct ChipInfo_t mma8451Chip;
extern struct ChipInfo_t mma8452Chip;
extern struct ChipInfo_t mma8453Chip;

/*
 * Global variables
 */

/*! Handle to interrupt service thread 	*/
static struct task_struct *hIstThread;	
/*! Semaphore to signal event to IST	*/
static struct semaphore chip_ready;	
/*! Chip type used to differentiate between various chipsets */
static int      ChipType = MMA845X;
/*! Pointer to platform data structure. This is initialized to platform structure declared in board file */
static struct mxc_mma845x_platform_data *plat_data;
/*! Global pointer to access ChipInfo_t to be used within this file */
static struct ChipInfo_t *gpChip = NULL;
/*! Global pointer to access device structure to be used within this file */
struct mxc_mma_device_t *gpDev = NULL;		
/*! Timer to handle stall condition for driver */
static struct timer_list stall_timer;		
/*! Flag for device suspend state */
static int      IsSuspended = 0;		
/*! Flag to notify termination of IST */
static int      done = 0;

/*! Variable to determine where the device is in poll mode or interrupted mode */
static int poll_mode = 0;

static const struct i2c_device_id mma845x_id[] = {
    {MODULE_NAME, 0},
    {},
};

MODULE_DEVICE_TABLE(i2c, mma845x_id);

/*! I2C driver structure */
static struct i2c_driver i2c_mma845x_driver = {
    .driver = {
	       .name = MODULE_NAME,
	       },
    .probe     = mma845x_probe,
    .remove    = mma845x_remove,
    .suspend   = mma845x_suspend,
    .resume    = mma845x_resume,
    .id_table  = mma845x_id,
};

/*! Chip table - Array containing pointer to supported chip info structure */
static struct ChipInfo_t *ChipTable[] = {
    &mma8451Chip,
    &mma8452Chip,
    &mma8453Chip,
};

/*!
* This function returns the current state of acclerometer reading events 
* return 1       : Enabled
* return 0       : Disabled
* return -ENOMEM : Driver not initialized
*/
int GetAcclerometerFlag(void)
{
    if (gpDev == NULL)
	return -ENOMEM;
    return gpDev->aflag;
}

/*!
* This function sets state of acclerometer reading events 
* AcclFlag   : 0 - Disable, 1 - Enable
* return 0         : Success
* return -ENOMEM   : Driver not initialized
*/
int SetAcclerometerFlag(int AcclFlag)
{
    if (gpDev == NULL)
	return -ENOMEM;

    gpDev->aflag = AcclFlag;
    /*! \todo Enable/Disable interrupt here */

    return 0;
}

#ifdef CONFIG_EARLYSUSPEND
/*!
* This function implements early suspend handling for device 
* early suspend handle
* return NONE
*/
static void mma845x_early_suspend(struct early_suspend *h)
{
    int             mask = 0;
    int             ret = 0;
    printk("%s: Early suspend called\r\n", __func__);

    IsSuspended = 1;
    gpChip->DisableInt(0xff);

    /*
     * Clear all interrupts 
     */
    ret = gpChip->GetIntSrc();
    if (ret) {
	printk("%s:: Clearing pending interrupts... \r\n", __func__);
	up(&chip_ready);
    }

    mask |= (gpChip->fm_wakeon_event) ? INT_EN_FF_MT : 0;
    mask |= (gpChip->ornt_wakeon_event) ? INT_EN_LNDPRT : 0;
    mask |= (gpChip->trans_wakeon_event) ? INT_EN_TRANS : 0;
    mask |= (gpChip->tap_wakeon_event) ? INT_EN_PULSE : 0;
    gpChip->EnableInt(mask);
}

/*!
* This function implements late resume handling for device 
* early suspend handle
* return NONE
*/
static void mma845x_late_resume(struct early_suspend *h)
{
    int mask = 0;
    printk("%s: Late resume called\r\n", __func__);

    if (!poll_mode) {
    	mask |= (gpChip->enablefifo) ? INT_EN_FIFO : INT_EN_DRDY;
    }
    mask |= (gpChip->fm_enable) ? INT_EN_FF_MT : 0;
    mask |= (gpChip->ornt_enable) ? INT_EN_LNDPRT : 0;
    mask |= (gpChip->trans_enable) ? INT_EN_TRANS : 0;
    mask |= (gpChip->tap_enable) ? INT_EN_PULSE : 0;

    gpChip->EnableInt(mask);

    if (IsSuspended == 1)
	IsSuspended = 0;
}

static struct early_suspend mma845x_early_suspend_desc = {
    .level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
    .suspend = mma845x_early_suspend,
    .resume = mma845x_late_resume,
};
#endif


/*!
* This function implements interrupt handler routine 
* irq : Interrupt number
* dev_id
* return IRQ_RETVAL
*/
static irqreturn_t mma845x_interrupt(int irq, void *dev_id)
{

    up(&chip_ready);
    return IRQ_RETVAL(1);
}

/*!
* This routine implements a call back for stall timer 
* data : Pointer to device data structure
* return NONE
*/
static void stall_timer_fn(unsigned long data)
{
    up(&chip_ready);
}

/*!
* This function implements Interrupt service thread 
* data : Pointer to device data structure
* return 0   : Success

    This thread is responsible to handle all the activities that needs to be done after an interrupt is raised
    by the hardware. This thread waits on a semaphore "chip_ready" if there is no action to be perfomed. 
    The "chip_ready" semaphore is released in following conditions:
    	- if there is interrupt from hardware
	- stall timer has expired
	- thread is to be terminated (during driver exit)

    The thread performs following activities in case of interrupt is raised:
    	- Read the interrupt source from device, this will clear the HW interrupt
	- Update the Accelerometer fifo "UpdateAcclFiFo", in case of DRDY interrupt
	- Report the event to user
*/
static int IntServiceThread(void *data)
{
    wait_queue_t    wait;
    int             ret = 0;
    struct mxc_mma_device_t *pDev = (struct mxc_mma_device_t *) data;
    struct ChipInfo_t *pChip = pDev->pChip;
    char buff[256];

    init_waitqueue_entry(&wait, current);

    mod_timer(&stall_timer, jiffies + (HZ));
    while (!done) {

	do {
	    ret = down_interruptible(&chip_ready);
	} while (ret == -EINTR);

	if (!poll_mode) {
		ret = pChip->GetIntSrc();
	} 
	else 
		ret = SRC_DRDY;


	if (SRC_DRDY & ret) {
	    pChip->Read(ACCL_DATA, (void *) buff);

	    UpdateAcclFiFo(buff);
	    if (!IsSuspended) {
		if (pDev->aflag == 1)
		    ReportEvent(pDev, ACCL_DATA, buff);
	    }
	}

	if (SRC_FIFO & ret) {
	    	int             count = 0,
			i = 0;
	    	char *pBuff = (char *) buff;
	    	printk(KERN_DEBUG "\t[FIFO] [%ld]\r\n", jiffies);
	    	count = pChip->Read(FIFO_DATA, (void *) buff);

	    	for (i = 0; i < count; i++) {
			UpdateAcclFiFo(pBuff + (i * sizeof(AcclData_t)));
			if (!IsSuspended) {
		    		if (pDev->aflag == 1)
					ReportEvent(pDev, ACCL_DATA,
				    	pBuff + (i * sizeof(AcclData_t)));
		}
	    }
	}

	/*
	 * Orientation 
	 */
	if (SRC_LNDPRT & ret) {
	    char           *pBuff = (char *) buff;
	    pChip->Read(ACCL_LNDPRT, (void *) buff);
	    if (!IsSuspended)
		ReportEvent(pDev, ACCL_LNDPRT, pBuff);
	}

	/*
	 * Motion / Freefall interrupt 
	 */
	if (SRC_FF_MT & ret) {
	    pChip->Read(ACCL_FF_MT, (void *) buff);
	    if (!IsSuspended)
		ReportEvent(pDev, ACCL_FF_MT, buff);
	}

	if (SRC_FF_MT_2 & ret) {
	    pChip->Read(ACCL_FF_MT_2, (void *) buff);
	    if (!IsSuspended)
		ReportEvent(pDev, ACCL_FF_MT_2, buff);
	}

	/*
	 * Motion / Freefall interrupt 
	 */
	if (SRC_PULSE & ret) {
	    pChip->Read(ACCL_PULSE, (void *) buff);
	    if (!IsSuspended)
		ReportEvent(pDev, ACCL_PULSE, buff);
	}

	if (SRC_TRANS & ret) {
	    pChip->Read(ACCL_TRANS, (void *) buff);
	    if (!IsSuspended)
		ReportEvent(pDev, ACCL_TRANS, buff);
	}
	if (!IsSuspended) {
		if (!poll_mode) {
		    mod_timer(&stall_timer, jiffies + (HZ));
		}
		else 
		    mod_timer(&stall_timer, jiffies + pChip->poll_time);
	}
	
    }
    return 0;
}

/*!
* This function Identify the chip connected on bus and associate client driver for the chipset 
* client   : i2c_client pointer for i2c bus attached to device
* return chip_id : Chip id of identified device
* return -1      : Device not identified
*/
static int IdentifyChipset(struct i2c_client *client)
{
    int             retVal = 0;
    int             ChipIdentified = 0;

    retVal = i2c_smbus_read_byte_data(client, REG(WHO_AM_I));
    switch (retVal) {
    case ID_MMA8451:
	{
	    printk("%s:: Found MMA8451 chipset with chip ID 0x%02x\r\n",
		   __func__, retVal);
	    ChipIdentified = 1;
	}
	break;

    case ID_MMA8452:
	{
	    printk("%s:: Found MMA8452 chipset with chip ID 0x%02x\r\n",
		   __func__, retVal);
	    ChipIdentified = 1;
	}
	break;

    case ID_MMA8453:
	{
	    printk("%s:: Found MMA8453 chipset with chip ID 0x%02x\r\n",
		   __func__, retVal);
	    ChipIdentified = 1;
	}
	break;

    default:
	{
	    printk("%s:: Not a valid MMA845X chipset. Chip ID 0x%02x\r\n",
		   __func__, retVal);
	    ChipIdentified = 0;
	}
	break;
    }
 
    if (!ChipIdentified) {
	return -1;
    }

    return retVal;
}

/*!
* This function intiailized chipset with default values 
* pChip    : Pointer to ChipInfo for associated chipset
* return 0       : Success
* return -ENOMEM : NULL pChip pointer
*/
static int SetDefaultVal(struct ChipInfo_t *pChip)
{

    if (pChip == NULL) {
	printk("%s: NULL pointer\r\n", __func__);
	return -ENOMEM;
    }

    pChip->fm_wakeon_event = 0;
    pChip->ornt_wakeon_event = 0;
    pChip->trans_wakeon_event = 0;
    pChip->tap_wakeon_event = 0;

    switch (pChip->ChipId) {
    case ID_MMA8451:
	{
	    printk("Inialize default orientation values for MMA8451 chip \n");
	    pChip->odr = 0x06;
	    pChip->ornt_enable = 0x00;
	    pChip->zlock_angle_ths = 0x4;
	    pChip->bf_trip_angle = 0x01;
	    pChip->trip_angle_threshold = 0x09;
	    pChip->trip_angle_hysteresis = 0X03;
	    pChip->fm_enable = 0x00;
	    pChip->fm_threshold_logic = 0x01;
	    pChip->fm_threshold = 0x08;
	    pChip->fm_debounce_count = 0x30;
	    pChip->trans_enable = 0x00;
	    pChip->trans_threshold = 0x10;
	    pChip->trans_debounce_count = 0x0A;
	    pChip->tap_threshold_x = 0x20;
	    pChip->tap_threshold_y = 0x20;
	    pChip->tap_threshold_z = 0x40;
	    pChip->pulse_time_limit = 0x18;
	    pChip->pulse_time_step = INT_TO_FIXP(5);
	    pChip->pulse_latency_time = 0x28;
	    pChip->pulse_latency_step = INT_TO_FIXP(10);
	    pChip->pulse_window_time = 0x3c;
	    pChip->pulse_window_step = INT_TO_FIXP(10);
	    pChip->tap_enable = 0x0;

	}
	break;

    case ID_MMA8452:
	{
	    printk
		("Inialize default orientation values for MMA8452 chip \n");
	    pChip->odr = 0x06;
	    pChip->ornt_enable = 0x00;
	    pChip->zlock_angle_ths = 0x0;
	    pChip->bf_trip_angle = 0x0;
	    pChip->trip_angle_threshold = 0x0;
	    pChip->trip_angle_hysteresis = 0X0;
	    pChip->fm_enable = 0x00;
	    pChip->fm_threshold_logic = 0x01;
	    pChip->fm_threshold = 0x08;
	    pChip->fm_debounce_count = 0x30;
	    pChip->trans_enable = 0x00;
	    pChip->trans_threshold = 0x10;
	    pChip->trans_debounce_count = 0x0A;
	    pChip->tap_threshold_x = 0x20;
	    pChip->tap_threshold_y = 0x20;
	    pChip->tap_threshold_z = 0x40;
	    pChip->pulse_time_limit = 0x18;
	    pChip->pulse_latency_time = 0x28;
	    pChip->pulse_window_time = 0x3c;
	    pChip->tap_enable = 0x0;
	}
	break;

    case ID_MMA8453:
	{
	    printk
		("Inialize default orientation values for MMA8453 chip \n");

	    pChip->odr = 0x06;
	    pChip->ornt_enable = 0x00;
	    pChip->zlock_angle_ths = 0x0;
	    pChip->bf_trip_angle = 0x0;
	    pChip->trip_angle_threshold = 0x0;
	    pChip->trip_angle_hysteresis = 0X0;
	    pChip->fm_enable = 0x00;
	    pChip->fm_threshold_logic = 0x01;
	    pChip->fm_threshold = 0x08;
	    pChip->fm_debounce_count = 0x30;
	    pChip->trans_enable = 0x00;
	    pChip->trans_threshold = 0x10;
	    pChip->trans_debounce_count = 0x0A;
	    pChip->tap_threshold_x = 0x20;
	    pChip->tap_threshold_y = 0x20;
	    pChip->tap_threshold_z = 0x40;
	    pChip->pulse_time_limit = 0x18;
	    pChip->pulse_latency_time = 0x28;
	    pChip->pulse_window_time = 0x3c;
	    pChip->tap_enable = 0x0;
	}
	break;
           default:
	{
	    printk("Invalid chip \n");
	}
    }

    return 0;
}

/*!
* This function implements probe routine for driver
* client     : pointer to i2c client
* id         : i2c device id
* return 0         : Success
* return -ENOMEM   : Memory allocation for pDev failed
* return -ENODEV   : Platform data not found
*/
static int mma845x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0,
    i = 0;
    struct ChipInfo_t *pChip = NULL;
    struct mxc_mma_device_t *pDev = NULL;
    int             mask = 0;
   
    plat_data =
	(struct mxc_mma845x_platform_data *) client->dev.platform_data;
    if (plat_data == NULL) {
	dev_err(&client->dev, "lack of platform data!\n");
	return -ENODEV;
    }

    pDev = kzalloc(sizeof(struct mxc_mma_device_t), GFP_KERNEL);
    if (!pDev) {
	pDev = NULL;
	return -ENOMEM;
    }

     printk(KERN_INFO "\r\nProbing Module: %s %s\r\n", MODULE_NAME, DRIVER_VERSION);
     printk(KERN_INFO "Freescale Android 2.3 Release: %s\r\n", ANDROID_RELEASE); 
     printk(KERN_INFO "Build Date: %s [%s]\r\n", __DATE__, __TIME__);
    
    i2c_set_clientdata(client, pDev);

    /*
     * bind the right device to the driver 
     */
    ret = IdentifyChipset(client);
    if (ret < 0) {
	 printk(KERN_INFO "%s:: Unable to identify device.\r\n", __func__);
	return -EIO;
    }

    /*
     * Associate chip layer 
     */
    for (i = 0; i < sizeof(ChipTable) / sizeof(ChipTable[0]); i++) {
	if (ChipTable[i]->ChipId == ret) {
	    pChip = ChipTable[i];
	    pChip->ChipId = ret;
	    pChip->client = client;
	    pChip->ChipType = ChipType;
	    break;
	}
    }

    if (i >= (sizeof(ChipTable) / sizeof(ChipTable[0]))) {
	 printk(KERN_INFO "Chipset not supported by MMA driver\r\n");
	return -ENOMEM;
    }

    gpChip = pChip;
    gpDev = pDev;
    SetDefaultVal(pChip);

    /*
     * Inialize default event codes 
     */
    pDev->fm_event_type = 0x25;
    pDev->ornt_event_type = 0x26;
    pDev->trans_event_type = 0x27;
    pDev->event_type_single = 0x28;
    pDev->event_type_double = 0x29;

    pDev->aflag = 1;
    /*
     * Initialize chipset 
     */
    pChip->Init(pChip);

    pDev->pChip = pChip;
    pDev->version = 1;

    /*
     * configure gpio as input for interrupt monitor 
     */

    if(plat_data->gpio_pin_get)
	    plat_data->gpio_pin_get();

    /*
     * Register character device 
     */
    pDev->major = register_chrdev(0, "mma", &mma_fops);
    if (ret < 0) {
	 printk(KERN_INFO "%s:: Unable to register device\r\n", __func__);
	goto error_disable_power;
    }

    strcpy(pDev->devname, "mma");
    printk(KERN_INFO "%s:: Registered char dev \"%s\" @ %d\r\n", __func__,
	pDev->devname, pDev->major);

    /*
     * Initialize input layer 
     */
    InitializeInputInterface(pDev);

    /*
     * Create sysfs entries 
     */
    InitializeSysfs( (struct i2c_client *)client);

    /*
     * Initialize semaphore for interrupt
     */
    sema_init(&chip_ready, 0);

    setup_timer(&stall_timer, stall_timer_fn, 0);
    hIstThread = kthread_run(IntServiceThread, pDev, "mxc845x_ist");
    if (IS_ERR(hIstThread)) {
	 printk(KERN_INFO "Error creating mxc845x ist.\n");
	goto error_free_irq1;
    }
    if (plat_data->int1 > 0)  {   
	    irq_set_irq_type(plat_data->int1, IRQF_TRIGGER_FALLING);

	    ret = request_irq(plat_data->int1, mma845x_interrupt,
			      IRQF_TRIGGER_FALLING, DEVICE_NAME, pDev);

	    if (ret) {
		dev_err(&client->dev, "request_irq(%d) returned error %d\n",
			plat_data->int1, ret);
		goto error_disable_power;
	    }
    }
    else {
		gpChip->DisableInt(INT_EN_FIFO | INT_EN_DRDY);
		poll_mode = 1;
	}
    if (plat_data->int2 > 0){
    	 printk(KERN_INFO "%s:: Configuring interrupt IRQ [%d]\r\n", __func__,
	   plat_data->int2);

   	 irq_set_irq_type(plat_data->int2, IRQF_TRIGGER_FALLING);
   	 ret = request_irq(plat_data->int2, mma845x_interrupt,
		      IRQF_TRIGGER_FALLING, DEVICE_NAME, pDev);
     if (ret) {
		dev_err(&client->dev, "request_irq(%d) returned error %d\n",
		plat_data->int2, ret);
		goto error_free_irq1;
     }
   }
#ifdef CONFIG_EARLYSUSPEND
    register_early_suspend(&mma845x_early_suspend_desc);
#endif

    if (!poll_mode) {
	    mask |= (gpChip->enablefifo) ? INT_EN_FIFO : INT_EN_DRDY;
   }
    mask |= (gpChip->fm_enable) ? INT_EN_FF_MT : 0;
    mask |= (gpChip->ornt_enable) ? INT_EN_LNDPRT : 0;
    mask |= (gpChip->trans_enable) ? INT_EN_TRANS : 0;
    mask |= (gpChip->tap_enable) ? INT_EN_PULSE : 0;

    gpChip->EnableInt(mask);
    pChip->SetMode(0x01);

    dev_info(&client->dev, "mma845x device is probed successfully.\n");

    return 0;

  error_free_irq1:
    free_irq(plat_data->int1, plat_data);
  error_disable_power:

    return ret;
}

/*!
* This function implements "remove" routine for driver
* client    	: pointer to i2c client
* return 0 		: Success
* return -ENODEV 	: Device data is not initialized
*/
static int mma845x_remove(struct i2c_client *client)
{
    struct mxc_mma_device_t *pDev = i2c_get_clientdata(client);
    printk(KERN_INFO "%s:: Enter\n", __func__);

    if(pDev == NULL)
	    return -ENODEV;
    /*
     * Remove sysfs entries 
     */
    DeInitializeSysfs(pDev);

    /*
     * unegister character device 
     */
    printk(KERN_INFO "%s:: Unregister char interface\n", __func__);
    unregister_chrdev(pDev->major, pDev->devname);

    /*
     * DeInitialize input layer 
     */
    printk(KERN_INFO "%s:: Unregister input interface....\n", __func__);
    DeInitializeInputInterface(pDev);

    /*
     * DeInit chipset 
     */
    printk(KERN_INFO "%s:: DeInit chipset\n", __func__);
    if(plat_data->gpio_pin_put)
	plat_data->gpio_pin_put();

    /*
     * DeRegister interrupt 
     */
   
    if (plat_data->int1 > 0){
   		 printk(KERN_INFO "%s:: Freeing IRQs\n", __func__);
    	free_irq(plat_data->int1, pDev);
    }

//#ifdef ENABLE_INT2
    if (plat_data->int2 > 0)
   		 free_irq(plat_data->int2, pDev);
//#endif
    /*
     * Stop IST 
     */
    printk(KERN_INFO "%s:: Stopping thread....\n", __func__);
    done = 1;
    up(&chip_ready);
    kthread_stop(hIstThread);

//    wake_lock_destroy(&pDev->mxc_wake_lock);

    /*
     * Release device 
     */
    kfree(pDev);
    pDev = NULL;
    return 0;
}

/*!
* This function implements "suspend" routine for driver
* client 	: pointer to i2c client
* state 	: power manager state
* return 0 		: Success
*/
static int mma845x_suspend(struct i2c_client *client, pm_message_t state)
{
     printk(KERN_INFO "%s:: Enter____ STATE [%d]\n", __func__, state.event);
    if (plat_data->int2 > 0)
    	enable_irq_wake(plat_data->int2);
    return 0;
}

/*!
* This function implements "resume" routine for driver
* client : pointer to i2c client
* return 0     : Success
*/
static int mma845x_resume(struct i2c_client *client)
{  
    printk(KERN_INFO "%s:: Enter\n", __func__);
    IsSuspended = 0;

    return 0;
}

/*!
* This function implements "init" routine for driver
* return 0 : Success
*/
static int __init init_mma845x(void)
{
    /*
     * register driver 
     */
    printk(KERN_INFO "add mma i2c driver\n");
    return i2c_add_driver(&i2c_mma845x_driver);
}

/*!
* This function implements "exit" routine for driver
*/
static void __exit exit_mma845x(void)
{
    printk(KERN_INFO "del mma i2c driver.\n");
#ifdef CONFIG_EARLYSUSPEND
    // Register early_suspend and late_wakeup handlers
    unregister_early_suspend(&mma845x_early_suspend_desc);
#endif
    return i2c_del_driver(&i2c_mma845x_driver);
}

module_init(init_mma845x);
module_exit(exit_mma845x);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("MMA845x sensor driver");
MODULE_LICENSE("GPL");
