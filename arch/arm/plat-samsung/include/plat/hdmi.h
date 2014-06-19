/* linux/arch/arm/plat-samsung/include/plat/hdmi.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __PLAT_SAMSUNG_HDMI_H
#define __PLAT_SAMSUNG_HDMI_H __FILE__

extern void s5p_hdmi_set_platdata(struct i2c_board_info *hdmiphy_info,
				struct i2c_board_info *mhl_info, int mhl_bus);

extern void hdmi_hpd_cfg_gpio(bool hpd_to_hdmi);
extern int hdmi_hpd_read_gpio(void);
#endif /* __PLAT_SAMSUNG_HDMI_H */
