/*
 * linux/arch/arm/mach-exynos4/mach-tiny4412.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/lcd.h>
#include <linux/mmc/host.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/serial_core.h>
#include <linux/lcd.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/input/pixcir_ts.h>
#include <linux/gpio_event.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/platform_data/exynos_thermal.h>

#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>

#include <plat/backlight.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <plat/adc.h>
#include <plat/adc-core.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/mfc.h>
#include <plat/regs-serial.h>
#include <plat/sdhci.h>
#include <plat/regs-fb-v4.h>
#include <plat/fb.h>
#include <plat/pm.h>
#include <plat/hdmi.h>
#include <plat/ehci.h>
#include <plat/camport.h>
#include <plat/s3c64xx-spi.h>
#include <plat/fimg2d.h>

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
#include <mach/dwmci.h>
#endif

#include <mach/map.h>
#include <mach/exynos-ion.h>
#include <mach/regs-pmu.h>
#include <mach/ohci.h>
#include <mach/ppmu.h>
#include <mach/dev.h>
#include <mach/s3cfb.h>

#include <media/v4l2-mediabus.h>
#include <media/s5p_fimc.h>
#include <media/m5mols.h>
#include <plat/mipi_csis.h>
#include <plat/camport.h>
#include <media/exynos_fimc_is.h>
#include "common.h"
#include <media/exynos_flite.h>

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define TINY4412_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define TINY4412_ULCON_DEFAULT	S3C2410_LCON_CS8

#define TINY4412_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)


static struct s3c2410_uartcfg tiny4412_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= TINY4412_UCON_DEFAULT,
		.ulcon		= TINY4412_ULCON_DEFAULT,
		.ufcon		= TINY4412_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= TINY4412_UCON_DEFAULT,
		.ulcon		= TINY4412_ULCON_DEFAULT,
		.ufcon		= TINY4412_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= TINY4412_UCON_DEFAULT,
		.ulcon		= TINY4412_ULCON_DEFAULT,
		.ufcon		= TINY4412_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= TINY4412_UCON_DEFAULT,
		.ulcon		= TINY4412_ULCON_DEFAULT,
		.ufcon		= TINY4412_UFCON_DEFAULT,
	},
};

static struct s3c_sdhci_platdata tiny4412_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
};

static struct s3c_sdhci_platdata tiny4412_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
};


static struct s3c2410_platform_i2c tiny4412_i2c0_data __initdata = {
	.flags			= 0,
	.bus_num		= 0,
	.slave_addr		= 0x10,
	.frequency		= 200*1000,
	.sda_delay		= 100,
};

#ifdef CONFIG_SND_SOC_WM8960_TINY4412
#include <sound/wm8960.h>
static struct wm8960_data wm8960_pdata = {
	.capless	= 0,
	.dres		= WM8960_DRES_400R,
};
#endif

static struct i2c_board_info tiny4412_i2c_devs0[] __initdata = {
#ifdef CONFIG_SND_SOC_WM8960_TINY4412
	{
		I2C_BOARD_INFO("wm8960", 0x1a),
		.platform_data = &wm8960_pdata,
	},
#endif
};

#ifdef CONFIG_TOUCHSCREEN_FT5X0X
#include <plat/ft5x0x_touch.h>
static struct ft5x0x_i2c_platform_data ft5x0x_pdata = {
	.gpio_irq		= EXYNOS4_GPX1(6),
	.irq_cfg		= S3C_GPIO_SFN(0xf),
	.screen_max_x	= 800,
	.screen_max_y	= 1280,
	.pressure_max	= 255,
};
#endif

static struct s3c2410_platform_i2c tiny4412_i2c1_data __initdata = {
	.flags			= 0,
	.bus_num		= 1,
	.slave_addr		= 0x10,
	.frequency		= 200*1000,
	.sda_delay		= 100,
};

static struct i2c_board_info tiny4412_i2c_devs1[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
	{
		I2C_BOARD_INFO("ft5x0x_ts", (0x70 >> 1)),
		.platform_data = &ft5x0x_pdata,
	},
#endif
#ifdef CONFIG_SND_SOC_SAMSUNG_SMDK_WM8994 
	{ 
		I2C_BOARD_INFO("wm8994", 0x1a), 
	  	.platform_data  = &wm8994_platform_data,
	}
#endif
};

static struct i2c_board_info tiny4412_i2c_devs2[] __initdata = {
	/* nothing here yet */
};


static struct s5p_platform_mipi_csis mipi_csis_platdata = {
#ifdef CONFIG_VIDEO_S5K6A3
	.clk_rate	= 160000000UL,
	.lanes		= 1,
	.alignment	= 24,
	.hs_settle	= 12,
	.phy_enable	= s5p_csis_phy_enable,
#endif
#ifdef CONFIG_VIDEO_M5MOLS
	.clk_rate	= 166000000UL,
	.lanes		= 2,
	.alignment	= 32,
	.hs_settle	= 12,
	.phy_enable	= s5p_csis_phy_enable,
#endif
};
#define GPIO_CAM_LEVEL_EN(n)	EXYNOS4_GPX1(2)
#define GPIO_CAM_8M_ISP_INT	EXYNOS4_GPX3(3)	/* XEINT_27 */
#define GPIO_CAM_MEGA_nRST	EXYNOS4_GPX1(2) 
static int m5mols_set_power(struct device *dev, int on)
{
	gpio_set_value(EXYNOS4_GPX1(2), !on);
	gpio_set_value(EXYNOS4_GPX1(2), !!on);
	return 0;
}
static struct m5mols_platform_data m5mols_platdata = {
	.gpio_reset	= GPIO_CAM_MEGA_nRST,
	.reset_polarity	= 0,
	.set_power	= m5mols_set_power,
};
static struct i2c_board_info m5mols_board_info = {
	I2C_BOARD_INFO("M5MOLS", 0x1F),
	.platform_data = &m5mols_platdata,
};

#ifdef CONFIG_VIDEO_S5K6A3
static struct i2c_board_info s5k6a3_sensor_info = {
        .type = "S5K6A3",
};
#endif

#ifdef CONFIG_VIDEO_S5K6A3
static int tiny4412_cam1_reset(int dummy) 
{
        int err;

        /* Camera B */

        err = gpio_request(EXYNOS4_GPX1(0), "GPX1");
        if (err)
                printk(KERN_ERR "#### failed to request GPX1_0 ####\n");

        s3c_gpio_setpull(EXYNOS4_GPX1(0), S3C_GPIO_PULL_NONE);
        gpio_direction_output(EXYNOS4_GPX1(0), 0);
        gpio_direction_output(EXYNOS4_GPX1(0), 1);
        gpio_free(EXYNOS4_GPX1(0));

        return 0;
}

static struct s3c_platform_camera s5k6a3 = {
        .id             = CAMERA_CSI_D,
        .clk_name       = "sclk_cam1",
        .cam_power      = tiny4412_cam1_reset,
        .type           = CAM_TYPE_MIPI,
        .fmt            = MIPI_CSI_RAW10,
        .order422       = CAM_ORDER422_8BIT_YCBYCR,
        .pixelformat    = V4L2_PIX_FMT_UYVY,
        .line_length    = 1920,
        .width          = 1920,
        .height         = 1080,
        .window         = {
                .left   = 0,
                .top    = 0,
                .width  = 1920,
                .height = 1080,
        },
        .srclk_name     = "xusbxti",
        .clk_rate       = 24000000,
        .mipi_lanes     = 1,
        .mipi_settle    = 12,
        .mipi_align     = 24,

        .initialized    = 0,
        .flite_id       = FLITE_IDX_B,
        .use_isp        = true,
        .sensor_index   = 102,
	.type  		= CAM_TYPE_MIPI,
        .use_isp 	= true,
        .inv_pclk 	= 0,
        .inv_vsync 	= 0,
        .inv_href 	= 0,
        .inv_hsync 	= 0,
};


static struct s3c_platform_fimc fimc_plat = {
	.default_cam    = CAMERA_CSI_D,
	.camera         = {
			&s5k6a3,
	},
};
#endif
static struct s5p_fimc_isp_info tiny4412_camera_sensors[] = {
#ifdef CONFIG_VIDEO_S5K6A3
	{
                .board_info     = &s5k6a3_sensor_info,
                .clk_frequency  = 24000000UL,
                .bus_type       = FIMC_MIPI_CSI2,
		.i2c_bus_num    = 1,
                .mux_id         = 1, /* A-Port : 0, B-Port : 1 */
                .flite_id       = FLITE_IDX_B,
                .cam_power      = tiny4412_cam1_reset,
		.flags          = 0,
                .csi_data_align = 24,
                .use_isp        = true,
        },
#endif
#ifdef CONFIG_VIDEO_M5MOLS
	{
		.mux_id		= 0,
		.flags		= V4L2_MBUS_PCLK_SAMPLE_FALLING |
				  V4L2_MBUS_VSYNC_ACTIVE_LOW,
		.bus_type	= FIMC_MIPI_CSI2,
		.board_info	= &m5mols_board_info,
		.i2c_bus_num	= 4,
		.clk_frequency	= 24000000UL,
		.csi_data_align	= 32,
	},
#endif
};
static struct s5p_platform_fimc fimc_md_platdata = {
	.isp_info	= tiny4412_camera_sensors,
	.num_clients	= ARRAY_SIZE(tiny4412_camera_sensors),
#ifdef CONFIG_VIDEO_S5K6A3
	.fimc_plat	= &fimc_plat,
#endif
};

static struct gpio tiny4412_camera_gpios[] = {
	{ GPIO_CAM_8M_ISP_INT,	GPIOF_IN,            "8M_ISP_INT"  },
	{ GPIO_CAM_MEGA_nRST,	GPIOF_OUT_INIT_LOW,  "CAM_8M_NRST" },
};
static void __init tiny4412_camera_init(void)
{
	s3c_set_platdata(&mipi_csis_platdata, sizeof(mipi_csis_platdata),
			 &s5p_device_mipi_csis0);
	s3c_set_platdata(&mipi_csis_platdata, sizeof(mipi_csis_platdata),
                         &s5p_device_mipi_csis1);
	s3c_set_platdata(&fimc_md_platdata,  sizeof(fimc_md_platdata),
			 &s5p_device_fimc_md);
	

	if (gpio_request_array(tiny4412_camera_gpios,
			       ARRAY_SIZE(tiny4412_camera_gpios))) {
		pr_err("%s: GPIO request failed\n", __func__);
		return;
	}

	if (!s3c_gpio_cfgpin(GPIO_CAM_8M_ISP_INT, S3C_GPIO_SFN(0xf)))
	{
        	s3c_gpio_setpull(GPIO_CAM_8M_ISP_INT, S3C_GPIO_PULL_NONE);
		m5mols_board_info.irq = gpio_to_irq(GPIO_CAM_8M_ISP_INT);
	}
	else
		pr_err("Failed to configure 8M_ISP_INT GPIO\n");

	/* Free GPIOs controlled directly by the sensor drivers. */
	gpio_free(GPIO_CAM_MEGA_nRST);
	gpio_free(GPIO_CAM_8M_ISP_INT);

	if (exynos4_fimc_setup_gpio(S5P_CAMPORT_A))
		pr_err("Camera port A setup failed\n");
}

#ifdef CONFIG_MXC_MMA845X
#include <linux/mma845x.h>

static struct mxc_mma845x_platform_data mma845x_data = {
	.gpio_pin_get = NULL,
	.gpio_pin_put = NULL,
	.int1 = IRQ_EINT(25),	// ACCL_INT1 is gpio for MMA845X INT1
	.int2 = 0,				// ACCL_INT2 is gpio for MMA845X INT2
};
#endif

#ifdef CONFIG_SENSORS_MMA7660
#include <linux/mma7660.h>
static struct mma7660_platform_data mma7660_pdata = {
	.irq			= IRQ_EINT(25),
	.poll_interval	= 100,
	.input_fuzz		= 4,
	.input_flat		= 4,
};
#endif

static struct s3c2410_platform_i2c tiny4412_i2c3_data __initdata = {
	.flags			= 0,
	.bus_num		= 3,
	.slave_addr		= 0x10,
	.frequency		= 200*1000,
	.sda_delay		= 100,
};

static struct i2c_board_info tiny4412_i2c_devs3[] __initdata = {
#ifdef CONFIG_MXC_MMA845X
	{
		.type = "mma845x",
		.addr = 0x1D,		/*mma845x i2c slave address*/
		.platform_data = (void *)&mma845x_data,
	},
#endif
#ifdef CONFIG_SENSORS_MMA7660
	{
		I2C_BOARD_INFO("mma7660", 0x4c),
		.platform_data = &mma7660_pdata,
	},
#endif
};

/* I2C module and id for HDMIPHY */
static struct i2c_board_info tiny4412_i2c_hdmiphy[] __initdata = {
	{ I2C_BOARD_INFO("hdmiphy-exynos4412", 0x38), }
};

static void s5p_tv_setup(void)
{
	/* direct HPD to External Interrupt */
	WARN_ON(gpio_request_one(EXYNOS4_GPX3(7), GPIOF_IN, "hpd-plug"));
	s3c_gpio_cfgpin(EXYNOS4_GPX3(7), S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(EXYNOS4_GPX3(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPX3(7));
}

static struct s3c2410_platform_i2c tiny4412_i2c7_data __initdata = {
	.flags			= 0,
	.bus_num		= 7,
	.slave_addr		= 0x10,
	.frequency		= 200*1000,
	.sda_delay		= 100,
};

static struct i2c_board_info tiny4412_i2c_devs7[] __initdata = {
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
	},
};

static void __init tiny4412_touch_init(void)
{
	/* nothing here yet */
}

static struct samsung_bl_gpio_info tiny4412_bl_gpio_info = {
	.no = EXYNOS4_GPD0(1),
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data tiny4412_bl_data = {
	.pwm_id = 1,
	.pwm_period_ns  = 1000,
};

#ifdef CONFIG_FB_S3C

static struct s3c_fb_pd_win tiny4412_fb_win0 = {
	.xres			= 480,
	.yres			= 800,
	.virtual_x		= 480,
	.virtual_y		= 800 * CONFIG_FB_S3C_NR_BUFFERS,
	.max_bpp		= 32,
	.default_bpp	= 24,
	.width			= 66,
	.height			= 109,
};

static struct s3c_fb_pd_win tiny4412_fb_win1 = {
	.xres			= 480,
	.yres			= 800,
	.virtual_x		= 480,
	.virtual_y		= 800 * CONFIG_FB_S3C_NR_BUFFERS,
	.max_bpp		= 32,
	.default_bpp	= 24,
	.width			= 66,
	.height			= 109,
};

static struct s3c_fb_pd_win tiny4412_fb_win2 = {
	.xres			= 480,
	.yres			= 800,
	.virtual_x		= 480,
	.virtual_y		= 800 * CONFIG_FB_S3C_NR_BUFFERS,
	.max_bpp		= 32,
	.default_bpp	= 24,
	.width			= 66,
	.height			= 109,
};

static struct s3c_fb_pd_win tiny4412_fb_win3 = {
	.xres			= 480,
	.yres			= 800,
	.virtual_x		= 480,
	.virtual_y		= 800 * CONFIG_FB_S3C_NR_BUFFERS,
	.max_bpp		= 32,
	.default_bpp	= 24,
	.width			= 66,
	.height			= 109,
};

static struct s3c_fb_pd_win tiny4412_fb_win4 = {
	.xres			= 480,
	.yres			= 800,
	.virtual_x		= 480,
	.virtual_y		= 800 * CONFIG_FB_S3C_NR_BUFFERS,
	.max_bpp		= 32,
	.default_bpp	= 24,
	.width			= 66,
	.height			= 109,
};

static struct fb_videomode tiny4412_lcd_timing = {
	.left_margin	= 9,
	.right_margin	= 9,
	.upper_margin	= 5,
	.lower_margin	= 5,
	.hsync_len	= 2,
	.vsync_len	= 2,
	.xres		= 480,
	.yres		= 800,
};

static struct s3c_fb_platdata tiny4412_lcd0_pdata __initdata = {
	.win[0]		= &tiny4412_fb_win0,
	.win[1]		= &tiny4412_fb_win1,
	.win[2]		= &tiny4412_fb_win2,
	.win[3]		= &tiny4412_fb_win3,
	.win[4]		= &tiny4412_fb_win4,
	.vtiming	= &tiny4412_lcd_timing,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
	.setup_gpio	= exynos4_fimd0_gpio_setup_24bpp,
};

static void __init tiny4412_fb_init_pdata(struct s3c_fb_platdata *pd) {
	struct s3cfb_lcd *lcd;
	struct s3c_fb_pd_win *win;
	struct fb_videomode *mode = pd->vtiming;
	unsigned long val = 0;
	u64 pixclk = 1000000000000ULL;
	u32 div;
	int i;

	lcd = tiny4412_get_lcd();

	for (i = 0; i < S3C_FB_MAX_WIN; i++) {
		if (pd->win[i] == NULL)
			continue;

		win = pd->win[i];
		win->xres		= lcd->width;
		win->yres		= lcd->height;
		win->default_bpp= lcd->bpp ? : 24;
		win->virtual_x	= win->xres;
		win->virtual_y	= win->yres * CONFIG_FB_S3C_NR_BUFFERS;
		win->width		= lcd->p_width;
		win->height		= lcd->p_height;
	}

	mode->left_margin	= lcd->timing.h_bp;
	mode->right_margin	= lcd->timing.h_fp;
	mode->upper_margin	= lcd->timing.v_bp;
	mode->lower_margin	= lcd->timing.v_fp;
	mode->hsync_len		= lcd->timing.h_sw;
	mode->vsync_len		= lcd->timing.v_sw;
	mode->xres			= lcd->width;
	mode->yres			= lcd->height;

	/* calculates pixel clock */
	div  = mode->left_margin + mode->hsync_len + mode->right_margin +
		mode->xres;
	div *= mode->upper_margin + mode->vsync_len + mode->lower_margin +
		mode->yres;
	div *= lcd->freq ? : 60;

	do_div(pixclk, div);

	mode->pixclock		= pixclk + 386;

	/* initialize signal polarity of RGB interface */
	if (lcd->polarity.rise_vclk)
		val |= VIDCON1_INV_VCLK;
	if (lcd->polarity.inv_hsync)
		val |= VIDCON1_INV_HSYNC;
	if (lcd->polarity.inv_vsync)
		val |= VIDCON1_INV_VSYNC;
	if (lcd->polarity.inv_vden)
		val |= VIDCON1_INV_VDEN;

	pd->vidcon1 = val;
}
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI0
static struct s3c64xx_spi_csinfo spi0_csi[] = {
	[0] = {
		.line = EXYNOS4_GPB(1),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi0_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi0_csi[0],
	}
};
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI1
static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line = EXYNOS4_GPB(5),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi1_csi[0],
	}
};
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI2
static struct s3c64xx_spi_csinfo spi2_csi[] = {
	[0] = {
		.line = EXYNOS4_GPC1(2),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi2_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 2,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi2_csi[0],
	}
};
#endif

#ifdef CONFIG_LCD_LMS501KF03
static int lcd_power_on(struct lcd_device *ld, int enable)
{
	return 1;
}

static int reset_lcd(struct lcd_device *ld)
{
	int err = 0;
	err = gpio_request_one(EXYNOS4X12_GPM3(6),
			GPIOF_OUT_INIT_HIGH, "GPM3");
	if (err) {
		pr_err("failed to request GPM3 for lcd reset control\n");
		return err;
	}
	gpio_set_value(EXYNOS4X12_GPM3(6), 0);
	mdelay(1);
	gpio_set_value(EXYNOS4X12_GPM3(6), 1);
	gpio_free(EXYNOS4X12_GPM3(6));

	return 1;
}

static struct lcd_platform_data lms501kf03_platform_data = {
	.reset			= reset_lcd,
	.power_on		= lcd_power_on,
	.lcd_enabled	= 0,
	.reset_delay	= 100,  /* 100ms */
};

#define LCD_BUS_NUM		3
static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias		= "lms501kf03",
		.platform_data	= (void *)&lms501kf03_platform_data,
		.max_speed_hz	= 1200000,
		.bus_num		= LCD_BUS_NUM,
		.chip_select	= 0,
		.mode			= SPI_MODE_3,
		.controller_data	= (void *)EXYNOS4_GPB(5),
	}
};

static struct spi_gpio_platform_data lms501kf03_spi_gpio_data = {
	.sck	= EXYNOS4_GPB(4), /* DISPLAY_CLK */
	.mosi	= EXYNOS4_GPB(7), /* DISPLAY_SI */
	.miso	= SPI_GPIO_NO_MISO,
	.num_chipselect = 1,
};

static struct platform_device s3c_device_spi_gpio = {
	.name	= "spi_gpio",
	.id		= LCD_BUS_NUM,
	.dev	= {
		.parent			= &s5p_device_fimd0.dev,
		.platform_data	= &lms501kf03_spi_gpio_data,
	},
};
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
static void exynos_dwmci_cfg_gpio(int width)
{
	unsigned int gpio;

	for (gpio = EXYNOS4_GPK0(0); gpio < EXYNOS4_GPK0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	}

	switch (width) {
	case MMC_BUS_WIDTH_8:
		for (gpio = EXYNOS4_GPK1(3); gpio <= EXYNOS4_GPK1(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(4));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
	case MMC_BUS_WIDTH_4:
		for (gpio = EXYNOS4_GPK0(3); gpio <= EXYNOS4_GPK0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
		break;
	case MMC_BUS_WIDTH_1:
		gpio = EXYNOS4_GPK0(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	default:
		break;
	}
}

static struct dw_mci_board exynos_dwmci_pdata __initdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION | DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR |
				MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23,
	.fifo_depth		= 0x80,
	.detect_delay_ms	= 200,
	.hclk_name		= "dwmci",
	.cclk_name		= "sclk_dwmci",
	.cfg_gpio		= exynos_dwmci_cfg_gpio,
};
#endif

static int exynos_boot_dev;

#define is_bootfrom_emmc()	\
	((exynos_boot_dev == 0x6) || (exynos_boot_dev == 0x7))
#define is_bootfrom_sd()	\
	 (exynos_boot_dev == 0x3)

static void __init exynos_bootdev_init(void)
{
	u32 capboot = MMC_CAP2_BOOT_DEVICE;

	exynos_boot_dev = __raw_readl(S5P_INFORM3);

	if (is_bootfrom_emmc()) {
#if defined(CONFIG_EXYNOS4_DEV_DWMCI)
		exynos_dwmci_pdata.caps2 |= capboot;
#endif
	} else if (is_bootfrom_sd()) {
		tiny4412_hsmmc2_pdata.host_caps2 |= capboot;
	} else {
		/* oops...should never fly to here */
		printk(KERN_ERR "Unknown boot device\n");
	}
}

static void __init tiny4412_wifi_init(void)
{
	/* sdwifi (external): PDn --> RESETn */
#define SDWIFI_GPIO_PD		EXYNOS4_GPX1(5)
#define SDWIFI_GPIO_RESET	EXYNOS4_GPX1(4)
	int ret;

	ret = gpio_request(SDWIFI_GPIO_PD, "SDWIFI_PD");
	if (ret)
		printk(KERN_ERR "failed to request GPIO %d for wifi, %d\n",
				SDWIFI_GPIO_PD, ret);

	gpio_direction_output(SDWIFI_GPIO_PD, 1);
	udelay(10);
	gpio_free(SDWIFI_GPIO_PD);

	ret = gpio_request(SDWIFI_GPIO_RESET, "SDWIFI_RESET");
	if (ret)
		printk(KERN_ERR "failed to request GPIO %d for wifi, %d\n",
				SDWIFI_GPIO_RESET, ret);

	gpio_direction_output(SDWIFI_GPIO_RESET, 1);
	gpio_free(SDWIFI_GPIO_RESET);
}

#ifdef CONFIG_INPUT_GPIO
static struct gpio_event_direct_entry tiny4412_key_map[] = {
	{
		.gpio	= EXYNOS4_GPX3(2),
		.code	= KEY_MENU,
	}, {
		.gpio	= EXYNOS4_GPX3(3),
		.code	= KEY_HOME,
	}, {
		.gpio	= EXYNOS4_GPX3(4),
		.code	= KEY_BACK,
	}, {
		.gpio	= EXYNOS4_GPX3(5),
		.code	= 353,	/* DPAD_CENTER */
	},
};

static struct gpio_event_input_info tiny4412_key_info = {
	.info.func			= gpio_event_input_func,
	.info.no_suspend	= true,
	.debounce_time.tv64	= 20 * NSEC_PER_MSEC,
	.type				= EV_KEY,
	.keymap				= tiny4412_key_map,
	.keymap_size		= ARRAY_SIZE(tiny4412_key_map)
};

static struct gpio_event_info *tiny4412_input_info[] = {
	&tiny4412_key_info.info,
};

static struct gpio_event_platform_data tiny4412_input_data = {
	.names	= {
		"tiny4412-key",
		NULL,
	},
	.info		= tiny4412_input_info,
	.info_count	= ARRAY_SIZE(tiny4412_input_info),
};

static struct platform_device tiny4412_input_device = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id		= 0,
	.dev	= {
		.platform_data = &tiny4412_input_data,
	},
};
#endif

static struct platform_device tiny4412_device_1wire = {
	.name			= "tiny4412_1wire",
	.id				= -1,
	.num_resources	= 0,
};

static struct platform_device tiny4412_device_adc = {
	.name			= "tiny4412_adc",
	.id				= -1,
	.num_resources	= 0,
};

#ifdef CONFIG_SAMSUNG_DEV_KEYPAD
static uint32_t tiny4412_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(1, 3, KEY_1), KEY(1, 4, KEY_2), KEY(1, 5, KEY_3),
	KEY(1, 6, KEY_4), KEY(1, 7, KEY_5),
	KEY(2, 5, KEY_D), KEY(2, 6, KEY_A), KEY(2, 7, KEY_B),
	KEY(0, 7, KEY_E), KEY(0, 5, KEY_C),
	KEY(0, 6, KEY_DOWN), KEY(0, 3, KEY_MENU), KEY(0, 4, KEY_RIGHT)
};

static struct matrix_keymap_data tiny4412_keymap_data __initdata = {
	.keymap			= tiny4412_keymap,
	.keymap_size	= ARRAY_SIZE(tiny4412_keymap),
};

static struct samsung_keypad_platdata tiny4412_keypad_data __initdata = {
	.keymap_data	= &tiny4412_keymap_data,
	.rows			= 3,
	.cols			= 8,
};
#endif

/* Audio */
static struct platform_device tiny4412_audio = {
	.name		= "tiny4412-audio",
	.id			= -1,
};

/* USB OTG */
static struct s3c_hsotg_plat tiny4412_hsotg_pdata;

static struct platform_device exynos4_bus_devfreq = {
	.name		= "exynos4412-busfreq",
	.id			= 1,
};

/* USB EHCI */
static struct s5p_ehci_platdata tiny4412_ehci_pdata;

static void __init tiny4412_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &tiny4412_ehci_pdata;
	int err;

	s5p_ehci_set_platdata(pdata);

#define GPIO_USBH_RESET		EXYNOS4X12_GPM2(4)
	err = gpio_request_one(GPIO_USBH_RESET,
			GPIOF_OUT_INIT_HIGH, "USBH_RESET");
	if (err)
		pr_err("failed to request GPM2_4 for USB reset control\n");

	s3c_gpio_setpull(GPIO_USBH_RESET, S3C_GPIO_PULL_UP);
	gpio_set_value(GPIO_USBH_RESET, 0);
	mdelay(1);
	gpio_set_value(GPIO_USBH_RESET, 1);
	gpio_free(GPIO_USBH_RESET);
}

/* USB OHCI */
static struct exynos4_ohci_platdata tiny4412_ohci_pdata;

static void __init tiny4412_ohci_init(void)
{
	struct exynos4_ohci_platdata *pdata = &tiny4412_ohci_pdata;

	exynos4_ohci_set_platdata(pdata);
}

#ifdef CONFIG_BUSFREQ_OPP
/* BUSFREQ to control memory/bus*/
static struct device_domain busfreq;
#endif
static struct platform_device exynos4_busfreq = {
	.id = -1,
	.name = "exynos-busfreq",
};

#ifdef CONFIG_BATTERY_SAMSUNG
static struct platform_device samsung_device_battery = {
	.name	= "samsung-fake-battery",
	.id = -1,
};
#endif

static struct platform_device *tiny4412_devices[] __initdata = {
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	&exynos_device_dwmci,
#endif
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c3,
#ifdef CONFIG_VIDEO_M5MOLS
	&s3c_device_i2c4,
#endif
	&s3c_device_i2c7,
	&s3c_device_adc,
	&s3c_device_rtc,
	&s3c_device_wdt,
#ifdef CONFIG_TINY4412_BUZZER
	&s3c_device_timer[0],
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	&exynos_device_flite0,
	&exynos_device_flite1,
#endif
	&s5p_device_mipi_csis0,
	&s5p_device_mipi_csis1,
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc3,
	&s5p_device_fimc_md,
	&s5p_device_fimd0,
	&mali_gpu_device,
	&s5p_device_mfc,
	&s5p_device_mfc_l,
	&s5p_device_mfc_r,
	&s5p_device_jpeg,
#ifdef CONFIG_SAMSUNG_DEV_KEYPAD
	&samsung_device_keypad,
#endif
	&tiny4412_device_1wire,
	&tiny4412_device_adc,
#ifdef CONFIG_INPUT_GPIO
	&tiny4412_input_device,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&exynos4_device_fimc_is,
#endif
#ifdef CONFIG_LCD_LMS501KF03
	&s3c_device_spi_gpio,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI0
	&s3c64xx_device_spi0,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	&s3c64xx_device_spi1,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	&s3c64xx_device_spi2,
#endif
#ifdef CONFIG_ION_EXYNOS
	&exynos_device_ion,
#endif
	&s5p_device_i2c_hdmiphy,
	&s5p_device_hdmi,
	&s5p_device_mixer,
	&exynos4_bus_devfreq,
	&samsung_asoc_dma,
	&samsung_asoc_idma,
#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos4_device_i2s0,
#endif
#ifdef CONFIG_SND_SAMSUNG_PCM
	&exynos4_device_pcm0,
#endif
#ifdef CONFIG_SND_SAMSUNG_SPDIF
	&exynos4_device_spdif,
#endif
	&tiny4412_audio,
#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
	&s5p_device_fimg2d,
#endif
#ifdef CONFIG_EXYNOS_THERMAL
	&exynos_device_tmu,
#endif
	&s5p_device_ehci,
	&exynos4_device_ohci,
	&s5p_device_usbswitch,
#if defined CONFIG_SND_SAMSUNG_ALP
	&exynos_device_srp,
#endif
#ifdef CONFIG_BUSFREQ_OPP
	&exynos4_busfreq,
#endif
#ifdef CONFIG_BATTERY_SAMSUNG
	&samsung_device_battery,
#endif
};

static void __init tiny4412_map_io(void)
{
	clk_xusbxti.rate = 24000000;

	exynos_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(tiny4412_uartcfgs, ARRAY_SIZE(tiny4412_uartcfgs));
}

static void __init tiny4412_reserve(void)
{
	// HACK: This reserved memory will be used for FIMC-IS
	s5p_mfc_reserve_mem(0x43000000, 8<< 20, 0x51000000, 8 << 20);
}

static void tiny4412_pmu_wdt_init(void)
{
	unsigned int value;

	if (soc_is_exynos4212() || soc_is_exynos4412()) {
		value = __raw_readl(S5P_AUTOMATIC_WDT_RESET_DISABLE);
		value &= ~S5P_SYS_WDTRESET;
		__raw_writel(value, S5P_AUTOMATIC_WDT_RESET_DISABLE);
		value = __raw_readl(S5P_MASK_WDT_RESET_REQUEST);
		value &= ~S5P_SYS_WDTRESET;
		__raw_writel(value, S5P_MASK_WDT_RESET_REQUEST);
	}
}

static void tiny4412_rtc_wake_init(void)
{
#ifdef CONFIG_PM
	gic_arch_extn.irq_set_wake = s3c_irq_wake;
#endif
}

static struct s3c2410_platform_i2c tiny4412_i2c4_platdata __initdata = {
	.frequency	= 300 * 1000,
	.sda_delay	= 200,
};
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
static void __set_flite_camera_config(struct exynos_platform_flite *data,
                                        u32 active_index, u32 max_cam)
{       
        data->active_cam_index = active_index;
        data->num_clients = max_cam;
}

static void __init tiny4412_set_camera_flite_platdata(void)
{
        int flite0_cam_index = 0;
        int flite1_cam_index = 0;
#ifdef CONFIG_VIDEO_S5K6A3
#ifdef CONFIG_S5K6A3_CSI_C
        exynos_flite0_default_data.cam[flite0_cam_index++] = &s5k6a3;
#endif
#ifdef CONFIG_S5K6A3_CSI_D
        exynos_flite1_default_data.cam[flite1_cam_index++] = &s5k6a3;
#endif
#endif
        __set_flite_camera_config(&exynos_flite0_default_data, 0, flite0_cam_index);
        __set_flite_camera_config(&exynos_flite1_default_data, 0, flite1_cam_index);
}
#endif

/* usb phy0 mode */
static int uhost0 = 0;

static int __init tiny4412_setup_uhost(char *str)
{
	if (!strcasecmp(str, "y") || !strcmp(str, "1") ||
		!strcasecmp(str, "yes")) {
		printk("USB PHY0 configured as HOST mode\n");
		uhost0 = 1;
	}

	return 0;
}
early_param("uhost0", tiny4412_setup_uhost);

#ifdef CONFIG_USB_EXYNOS_SWITCH
static struct s5p_usbswitch_platdata tiny4412_usbswitch_pdata;

static void __init tiny4412_usbswitch_init(void)
{
	struct s5p_usbswitch_platdata *pdata = &tiny4412_usbswitch_pdata;
	int err;

	pdata->gpio_host_detect = EXYNOS4_GPX3(5); /* low active */
	err = gpio_request_one(pdata->gpio_host_detect, GPIOF_IN,
							"HOST_DETECT");
	if (err) {
		pr_err("failed to request gpio_host_detect\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_host_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_host_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_detect);

	pdata->gpio_device_detect = EXYNOS4_GPX3(4); /* high active */
	err = gpio_request_one(pdata->gpio_device_detect, GPIOF_IN,
							"DEVICE_DETECT");
	if (err) {
		pr_err("failed to request gpio_host_detect for\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_device_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_device_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_device_detect);

	pdata->gpio_host_vbus = EXYNOS4_GPL2(0);
	err = gpio_request_one(pdata->gpio_host_vbus, GPIOF_OUT_INIT_LOW,
							"HOST_VBUS_CONTROL");
	if (err) {
		pr_err("failed to request gpio_host_vbus\n");
		return;
	}

	s3c_gpio_setpull(pdata->gpio_host_vbus, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_vbus);

	s5p_usbswitch_set_platdata(pdata);
}
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.ip_ver			= IP_VER_G2D_4P,
	.hw_ver			= 0x41,
	.parent_clkname	= "mout_g2d0",
	.clkname		= "sclk_fimg2d",
	.gate_clkname	= "fimg2d",
	.clkrate		= 200 * MHZ,
};
#endif

static int __init exynos4_setup_clock(struct device *dev,
						const char *clock,
						const char *parent,
						unsigned long clk_rate)
{
	struct clk *clk_parent;
	struct clk *sclk;

	sclk = clk_get(dev, clock);
	if (IS_ERR(sclk)) {
		pr_err("Unable to get clock:%s.\n", clock);
		return PTR_ERR(sclk);
	}

	clk_parent = clk_get(NULL, parent);
	if (IS_ERR(clk_parent)) {
		clk_put(sclk);
		pr_err("Unable to get parent clock:%s of clock:%s.\n",
				parent, sclk->name);
		return PTR_ERR(clk_parent);
	}

	if (clk_set_parent(sclk, clk_parent)) {
		pr_err("Unable to set parent %s of clock %s.\n", parent, clock);
		clk_put(sclk);
		clk_put(clk_parent);
		return PTR_ERR(sclk);
	}

	if (clk_rate)
		if (clk_set_rate(sclk, clk_rate)) {
			pr_err("%s rate change failed: %lu\n", sclk->name,
				clk_rate);
			clk_put(sclk);
			clk_put(clk_parent);
			return PTR_ERR(sclk);
		}

	clk_put(sclk);
	clk_put(clk_parent);

	return 0;
}

static void initialize_prime_clocks(void)
{
	exynos4_setup_clock(&s5p_device_fimd0.dev, "sclk_fimd",
                                        "mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(&s5p_device_fimc0.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc1.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc2.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc3.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(NULL, "mout_mfc0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_mfc",
					"mout_mfc0", 220 * MHZ);

	exynos4_setup_clock(NULL, "mout_jpeg0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_jpeg",
					"mout_jpeg0", 176 * MHZ);

	exynos4_setup_clock(&s3c_device_hsmmc2.dev, "dout_mmc2",
					"mout_mpll_user", 100 * MHZ);
#ifdef CONFIG_SND_SAMSUNG_I2S_MASTER
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_epll", 400 * MHZ);
#else
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_mpll_user", 440 * MHZ);
#endif
#endif
}

static void initialize_non_prime_clocks(void)
{
	exynos4_setup_clock(&s5p_device_fimd0.dev, "sclk_fimd",
                                        "mout_mpll_user", 800 * MHZ);

	exynos4_setup_clock(&s5p_device_fimc0.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc1.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc2.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc3.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);

	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 160 * MHZ);

	exynos4_setup_clock(NULL, "mout_mfc0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_mfc",
					"mout_mfc0", 200 * MHZ);

	exynos4_setup_clock(NULL, "mout_jpeg0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_jpeg",
					"mout_jpeg0", 160 * MHZ);

	exynos4_setup_clock(&s3c_device_hsmmc2.dev, "dout_mmc2",
					"mout_mpll_user", 100 * MHZ);
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_mpll_user", 400 * MHZ);
#endif
}

static void __init tiny4412_machine_init(void)
{
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
	struct s3cfb_lcd *lcd = tiny4412_get_lcd();
	ft5x0x_pdata.screen_max_x = lcd->width;
	ft5x0x_pdata.screen_max_y = lcd->height;
#endif

	exynos_bootdev_init();

#ifdef CONFIG_S3C64XX_DEV_SPI0
	spi_register_board_info(spi0_board_info, ARRAY_SIZE(spi0_board_info));
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	spi_register_board_info(spi2_board_info, ARRAY_SIZE(spi2_board_info));
#endif

	if (samsung_pack() != EXYNOS4412_PACK_SCP) {
	}

	s3c_adc_set_platdata(NULL);
	s3c_adc_setname("samsung-adc-v4");

	s3c_i2c0_set_platdata(&tiny4412_i2c0_data);
	i2c_register_board_info(0, tiny4412_i2c_devs0,
			ARRAY_SIZE(tiny4412_i2c_devs0));

	s3c_i2c1_set_platdata(&tiny4412_i2c1_data);
	i2c_register_board_info(1, tiny4412_i2c_devs1,
			ARRAY_SIZE(tiny4412_i2c_devs1));

	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(2, tiny4412_i2c_devs2,
			ARRAY_SIZE(tiny4412_i2c_devs2));

	s3c_i2c3_set_platdata(&tiny4412_i2c3_data);
	i2c_register_board_info(3, tiny4412_i2c_devs3,
			ARRAY_SIZE(tiny4412_i2c_devs3));

	s3c_i2c4_set_platdata(NULL);

	tiny4412_rtc_wake_init();
	tiny4412_pmu_wdt_init();
	tiny4412_touch_init();
	tiny4412_wifi_init();

	s3c_i2c7_set_platdata(&tiny4412_i2c7_data);
	i2c_register_board_info(7, tiny4412_i2c_devs7,
			ARRAY_SIZE(tiny4412_i2c_devs7));

	s3c_hsotg_set_platdata(&tiny4412_hsotg_pdata);
#ifdef CONFIG_USB_EXYNOS_SWITCH
	tiny4412_usbswitch_init();
#endif
	samsung_bl_set(&tiny4412_bl_gpio_info, &tiny4412_bl_data);

#ifdef CONFIG_FB_S3C
	tiny4412_fb_init_pdata(&tiny4412_lcd0_pdata);
	s5p_fimd0_set_platdata(&tiny4412_lcd0_pdata);
#endif
#ifdef CONFIG_LCD_LMS501KF03
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif

#ifdef CONFIG_SAMSUNG_DEV_KEYPAD
	samsung_keypad_set_platdata(&tiny4412_keypad_data);
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos_dwmci_set_platdata(&exynos_dwmci_pdata);
#endif

	s3c_sdhci2_set_platdata(&tiny4412_hsmmc2_pdata);
	s3c_sdhci3_set_platdata(&tiny4412_hsmmc3_pdata);

#ifdef CONFIG_ION_EXYNOS
	exynos_ion_set_platdata();
#endif
	s5p_tv_setup();
	s5p_i2c_hdmiphy_set_platdata(NULL);
	s5p_hdmi_set_platdata(tiny4412_i2c_hdmiphy, NULL, 0);

#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
	s5p_fimg2d_set_platdata(&fimg2d_data);
#endif
#if defined(CONFIG_VIDEO_M5MOLS) || defined(CONFIG_VIDEO_S5K6A3)
	tiny4412_camera_init();
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	tiny4412_set_camera_flite_platdata();
	s3c_set_platdata(&exynos_flite0_default_data,
			sizeof(exynos_flite0_default_data), &exynos_device_flite0);
	s3c_set_platdata(&exynos_flite1_default_data,
			sizeof(exynos_flite1_default_data), &exynos_device_flite1);
#endif
	tiny4412_ehci_init();

#ifdef CONFIG_S3C64XX_DEV_SPI0
	s3c64xx_spi0_set_platdata(NULL, 0, 1);
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	s3c64xx_spi1_set_platdata(NULL, 0, 1);
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	s3c64xx_spi2_set_platdata(NULL, 0, 1);
#endif

	tiny4412_ohci_init();
	platform_add_devices(tiny4412_devices, ARRAY_SIZE(tiny4412_devices));
	if (!uhost0)
		platform_device_register(&s3c_device_usb_hsotg);

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	exynos4_fimc_is_set_platdata(NULL);
#endif

	if (soc_is_exynos4412()) {
		if ((samsung_rev() >= EXYNOS4412_REV_2_0))
			initialize_prime_clocks();
		else
			initialize_non_prime_clocks();
	}
#ifdef CONFIG_BUSFREQ_OPP
	dev_add(&busfreq, &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC0], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC1], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_CPU], &exynos4_busfreq.dev);
#endif
	set_tmu_platdata();
}

MACHINE_START(TINY4412, "TINY4412")
	/* Maintainer: FriendlyARM (www.arm9.net) */
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	/* Maintainer: Changhwan Youn <chaos.youn@samsung.com> */
	.atag_offset	= 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= tiny4412_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= tiny4412_machine_init,
	.init_late	= exynos_init_late,
	.timer		= &exynos4_timer,
	.restart	= exynos4_restart,
	.reserve	= &tiny4412_reserve,
MACHINE_END

