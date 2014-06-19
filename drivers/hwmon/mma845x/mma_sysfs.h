/******************** (C) COPYRIGHT 2012 Freescale Semiconductor, Inc. *************
 *
 * File Name		: mma_sysfs.h
 * Authors		: Rick Zhang(rick.zhang@freescale.com)
 			  Rick is willing to be considered the contact and update points 
 			  for the driver
 * Version		: V.1.0.0
 * Date			: 2012/Mar/15
 * Description		: the declartion of MMA845X sysfs interfaces
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
#include <linux/device.h>
#include <linux/i2c.h>
#include "mma_regs.h"
#include <mach/hardware.h>
#include "mma845x.h"

#define DECLARE_ATTR(_name, _mode, _show, _store)		\
{                                                               \
	.attr   = { .name = __stringify(_name), .mode = _mode,	\
		    /*.owner = THIS_MODULE*/ },  			\
	.show   = _show,                                        \
	.store  = _store,                                       \
}

/* COMMON Attributes */
extern ssize_t name_show(struct device *dev,struct device_attribute *attr,
	       			char *buf);

extern ssize_t vendor_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t devid_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t version_show(struct device *dev,struct device_attribute *attr,
	       			char *buf);

extern ssize_t type_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t max_range_show(struct device *dev,struct device_attribute *attr,
	       			char *buf);

extern ssize_t max_res_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t nominal_power_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t operation_mode_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t operation_mode_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t odr_show(struct device *dev,struct device_attribute *attr, 
				char *buf);

extern ssize_t odr_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t supported_odr_show(struct device *dev,struct device_attribute *attr,
	       			char *buf);

extern ssize_t oversampling_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t oversampling_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t oversampling_values_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t auto_wakeup_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t auto_wakeup_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t resolutions_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t resolution_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t resolution_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t range_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t range_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t range_low_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t range_high_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t precision_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t sample_rate_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t value_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t calibration_offset_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t calibration_offset_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t calibration_offset_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fifo_enable_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fifo_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fifo_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fifo_threshold_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fifo_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fifo_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

/* Motion attributes */

extern ssize_t fm_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fm_threshold_show(struct device *dev, struct device_attribute *attr,
				char *buf);

extern ssize_t fm_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_threshold_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_threshold_num_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_debounce_count_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t ccount);

extern ssize_t fm_debounce_count_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_debounce_count_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_debounce_count_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_debounce_time_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_event_type_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fm_event_type_show(struct device *dev,struct device_attribute *attr, char *buf);

extern ssize_t fm_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fm_enable_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t fm_wake_on_event_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t fm_wake_on_event_show(struct device *dev,struct device_attribute *attr,
				char *buf);

/* Transient attributes */

extern ssize_t trans_cut_off_frequencies_show(struct device *dev, struct device_attribute *attr,
				char *buf);

extern ssize_t trans_cut_off_frequencies_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trans_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trans_threshold_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_threshold_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_threshold_num_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_debounce_count_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t ccount);

extern ssize_t trans_debounce_count_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_debounce_count_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_debounce_count_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_debounce_time_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_event_type_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trans_event_type_show(struct device *dev,struct device_attribute *attr,	
				char *buf);

extern ssize_t trans_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trans_enable_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trans_wake_on_event_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trans_wake_on_event_show(struct device *dev,struct device_attribute *attr,
				char *buf);

/* Orientation attributes */

extern ssize_t z_lock_angle_threshold_show(struct device *dev, struct device_attribute *attr,
				char *buf);

extern ssize_t z_lock_angle_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t z_lock_angle_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t z_lock_angle_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t z_lock_angle_threshold_values_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t z_lock_angle_threshold_num_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t back_front_trip_angle_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t back_front_trip_angle_threshold_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t back_front_trip_angle_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t back_front_trip_angle_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t back_front_trip_angle_threshold_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t back_front_trip_angle_threshold_num_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_threshold_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trip_angle_threshold_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_threshold_values_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_threshold_num_steps_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_hysteresis_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t trip_angle_hysteresis_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_hysteresis_values_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t trip_angle_hysteresis_num_steps_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_debounce_count_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t ccount);

extern ssize_t ornt_debounce_count_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_debounce_count_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_debounce_count_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_debounce_time_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_event_type_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t ornt_event_type_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t ornt_enable_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t ornt_wake_on_event_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t ornt_wake_on_event_show(struct device *dev,struct device_attribute *attr,
				char *buf);

/* Tap attributes */

extern ssize_t sentap_threshold_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t sentap_threshold_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t sentap_threshold_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t sentap_threshold_num_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t low_pass_filter_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t low_pass_filter_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_time_limit_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t pulse_time_limit_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_time_limit_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_time_limit_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_time_limit_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_latency_time_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t pulse_latency_time_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_latency_time_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_latency_time_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_latency_time_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_window_time_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t pulse_window_time_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_window_time_min_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_window_time_max_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t pulse_window_time_step_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t event_type_single_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t event_type_single_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t event_type_double_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t event_type_double_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t threshold_xyz_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t threshold_xyz_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t tap_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t tap_enable_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t sentap_wake_on_event_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);

extern ssize_t sentap_wake_on_event_show(struct device *dev,struct device_attribute *attr,
				char *buf);

extern ssize_t poll_time_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count);

extern ssize_t poll_time_show(struct device *dev,
                struct device_attribute *attr, char *buf);

