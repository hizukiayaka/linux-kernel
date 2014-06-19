/*
 * Samsung FIMC driver
 *
 * Copyright (c) 2011-2012 Samsung Electronics Co., Ltd.
 *
 * Ritesh Kumar Solanki <r.solanki@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundiation. either version 2 of the License,
 * or (at your option) any later version
 */

#ifndef SAMSUNG_FIMC_IS_H
#define SAMSUNG_FIMC_IS_H

//Forword declarations

const struct v4l2_ctrl_ops fimc_ctrl_ops;

struct v4l2_ctrl_config test_mode = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_S_SCENARIO_MODE,
        .name = "V4L2_CID_IS_S_SCENARIO_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};
struct v4l2_ctrl_config scene_mode_cfg = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_S_SCENARIO_MODE,
        .name = "V4L2_CID_IS_S_SCENARIO_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_s_fmt_scenario = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_S_FORMAT_SCENARIO,
        .name = "V4L2_CID_IS_S_FORMAT_SCENARIO",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config s_m_normal = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_SHOT_MODE_NORMAL,
        .name = "V4L2_CID_IS_CAMERA_SHOT_MODE_NORMAL",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 0,
        .max = 2,
        .step = 1,
        .def = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config frame_rate  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_FRAME_RATE ,
        .name = "V4L2_CID_CAMERA_FRAME_RATE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 120,
        .step = 1,
        .def = 20,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config pos_x = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_OBJECT_POSITION_X,
        .name = "V4L2_CID_CAMERA_OBJECT_POSITION_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 270,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_pos_x = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_OBJECT_POSITION_X ,
        .name = "V4L2_CID_IS_CAMERA_OBJECT_POSITION_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 270,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config  pos_y= {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_OBJECT_POSITION_Y,
        .name = "V4L2_CID_CAMERA_OBJECT_POSITION_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 270,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config  is_pos_y = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_OBJECT_POSITION_Y,
        .name = "V4L2_CID_IS_CAMERA_OBJECT_POSITION_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 270,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};
 
struct v4l2_ctrl_config  win_pos_x = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_METERING_POSITION_X,
        .name = "V4L2_CID_IS_CAMERA_METERING_POSITION_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 800,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config  win_pos_y = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_METERING_POSITION_Y,
        .name = "V4L2_CID_IS_CAMERA_METERING_POSITION_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 480,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config  win_width = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_METERING_WINDOW_X,
        .name = "V4L2_CID_IS_CAMERA_METERING_WINDOW_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 40,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};


struct v4l2_ctrl_config  win_height = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_METERING_WINDOW_Y,
        .name = "V4L2_CID_IS_CAMERA_METERING_WINDOW_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 40,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};


//	ctrls->focus_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_FOCUS_MODE, 0, 256, 1, 0);


struct v4l2_ctrl_config focus_mode  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_FOCUS_MODE,
        .name = "V4L2_CID_CAMERA_FOCUS_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 256,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->s_focus_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_SET_AUTO_FOCUS, 0, 1, 1, 0);

struct v4l2_ctrl_config s_focus_mode  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_SET_AUTO_FOCUS,
        .name = "V4L2_CID_CAMERA_SET_AUTO_FOCUS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->af_start_stop = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_TOUCH_AF_START_STOP, 0, 1, 1, 0);

struct v4l2_ctrl_config af_start_stop  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_TOUCH_AF_START_STOP,
        .name = "V4L2_CID_CAMERA_TOUCH_AF_START_STOP",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->caf_start_stop = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_CAF_START_STOP, 0, 1, 1, 0);

struct v4l2_ctrl_config caf_start_stop  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_CAF_START_STOP,
        .name = "V4L2_CID_CAMERA_CAF_START_STOP",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->aeawb_l_ul = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK, 0, 3, 1, 0);

struct v4l2_ctrl_config aeawb_l_ul  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK,
        .name = "V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 3,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->flash_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_FLASH_MODE, 0, 4, 1, 0);

struct v4l2_ctrl_config flash_mode  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_FLASH_MODE,
        .name = "V4L2_CID_CAMERA_FLASH_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->awb_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_AWB_MODE, 0, 4, 1, 0);

struct v4l2_ctrl_config awb_mode  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_AWB_MODE,
        .name = "V4L2_CID_IS_CAMERA_AWB_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->white_balance = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_WHITE_BALANCE, 0, 5, 1, 0);

struct v4l2_ctrl_config white_balance  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_WHITE_BALANCE,
        .name = "V4L2_CID_CAMERA_WHITE_BALANCE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 5,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->c_effect = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_EFFECT, 0, 7, 1, 0);

struct v4l2_ctrl_config c_effect  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_EFFECT,
        .name = "V4L2_CID_CAMERA_EFFECT",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 7,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->c_i_effect = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_IMAGE_EFFECT, 0, 10, 1, 0);

struct v4l2_ctrl_config  c_i_effect = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_IMAGE_EFFECT,
        .name = "V4L2_CID_IS_CAMERA_IMAGE_EFFECT",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 10,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_iso = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_ISO, 0, 6, 1, 0);

struct v4l2_ctrl_config is_cam_iso  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_ISO,
        .name = "V4L2_CID_IS_CAMERA_ISO",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 6,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_iso = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_ISO, 0, 9, 1, 0);

struct v4l2_ctrl_config cam_iso  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_ISO ,
        .name = "V4L2_CID_CAMERA_ISO",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 9,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_contrast= v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_CONTRAST, 0, 4, 1, 0);

struct v4l2_ctrl_config cam_contrast = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_CONTRAST,
        .name = "V4L2_CID_CAMERA_CONTRAST",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_contrast = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_CONTRAST, 0, 5, 1, 0);

struct v4l2_ctrl_config is_cam_contrast  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_CONTRAST,
        .name = "V4L2_CID_IS_CAMERA_CONTRAST",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 5,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_saturation = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_SATURATION, 0, 4, 1, 0);

struct v4l2_ctrl_config is_cam_saturation  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_SATURATION,
        .name = "V4L2_CID_IS_CAMERA_SATURATION",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_saturation = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_SATURATION, 0, 4, 1, 0);

struct v4l2_ctrl_config cam_saturation  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_SATURATION,
        .name = "V4L2_CID_CAMERA_SATURATION",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_sharpness = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_SHARPNESS, 0, 4, 1, 0);

struct v4l2_ctrl_config is_cam_sharpness = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_SHARPNESS,
        .name = "V4L2_CID_IS_CAMERA_SHARPNESS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_sharpness = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_SHARPNESS, 0, 4, 1, 0);

struct v4l2_ctrl_config cam_sharpness  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_SHARPNESS,
        .name = "V4L2_CID_CAMERA_SHARPNESS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_exposure = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_EXPOSURE, 0, 8, 1, 0);

struct v4l2_ctrl_config is_cam_exposure  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_EXPOSURE,
        .name = "V4L2_CID_IS_CAMERA_EXPOSURE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 8,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_brightess = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_BRIGHTNESS, -4, 4, 1, 0);

struct v4l2_ctrl_config cam_brightess  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_BRIGHTNESS,
        .name = "V4L2_CID_CAMERA_BRIGHTNESS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -4,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_brightness = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_BRIGHTNESS, 0, 4, 1, 0);

struct v4l2_ctrl_config  is_cam_brightness  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_BRIGHTNESS ,
        .name = "V4L2_CID_IS_CAMERA_BRIGHTNESS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_hue = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_HUE, 0, 4, 1, 0);

struct v4l2_ctrl_config is_cam_hue   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_HUE,
        .name = "V4L2_CID_IS_CAMERA_HUE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 4,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_metering = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_METERING, 0, 3, 1, 0);

struct v4l2_ctrl_config cam_metering  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_METERING,
        .name = "V4L2_CID_CAMERA_METERING",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 3,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cam_metering = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_METERING, 0, 3, 1, 0);

struct v4l2_ctrl_config is_cam_metering  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_METERING ,
        .name = "V4L2_CID_IS_CAMERA_METERING",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 3,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->anti_banding = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_ANTI_BANDING, 0, 3, 1, 0);

struct v4l2_ctrl_config anti_banding   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_ANTI_BANDING,
        .name = "V4L2_CID_CAMERA_ANTI_BANDING",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 3,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->afc_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CAMERA_AFC_MODE, 0, 3, 1, 0);


struct v4l2_ctrl_config  afc_mode = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CAMERA_AFC_MODE,
        .name = "V4L2_CID_IS_CAMERA_AFC_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 3,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->s_max_face_num = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_FD_SET_MAX_FACE_NUMBER, 0, 6, 1, 0);

struct v4l2_ctrl_config s_max_face_num  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_SET_MAX_FACE_NUMBER,
        .name = "V4L2_CID_IS_FD_SET_MAX_FACE_NUMBER",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 6,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->s_roll_angle = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_FD_SET_ROLL_ANGLE, 0, 5, 1, 0);

struct v4l2_ctrl_config s_roll_angle  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_SET_ROLL_ANGLE,
        .name = "V4L2_CID_IS_FD_SET_ROLL_ANGLE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 5,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->s_data_addr = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_FD_SET_DATA_ADDRESS, 1, 0xfffffff, 1, 0);


struct v4l2_ctrl_config s_data_addr   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_SET_DATA_ADDRESS,
        .name = "V4L2_CID_IS_FD_SET_DATA_ADDRESS",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 0xfffffff,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_set_isp = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_SET_ISP, 0, 1, 1, 0);

struct v4l2_ctrl_config is_set_isp = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_SET_ISP,
        .name = "V4L2_CID_IS_SET_ISP",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_set_drc = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_SET_DRC, 0, 1, 1, 0);


struct v4l2_ctrl_config is_set_drc  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_SET_DRC,
        .name = "V4L2_CID_IS_SET_DRC",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cmd_isp = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CMD_ISP, 0, 1, 1, 0);

struct v4l2_ctrl_config is_cmd_isp   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CMD_ISP,
        .name = "V4L2_CID_IS_CMD_ISP",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cmd_drc = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CMD_DRC, 0, 1, 1, 0);

struct v4l2_ctrl_config is_cmd_drc  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CMD_DRC,
        .name = "V4L2_CID_IS_CMD_DRC",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->is_cmd_fd = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_CMD_FD, 0, 1, 1, 0);

struct v4l2_ctrl_config is_cmd_fd  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_CMD_FD,
        .name = "V4L2_CID_IS_CMD_FD",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 1,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};


//	ctrls->is_s_frame_no = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_SET_FRAME_NUMBER, 1, 1000, 1, 0);


struct v4l2_ctrl_config is_s_frame_no = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_SET_FRAME_NUMBER,
        .name = "V4L2_CID_IS_SET_FRAME_NUMBER",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 1000,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

//	ctrls->cam_scene_mode = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_CAMERA_SCENE_MODE, 0, 14, 1, 0);

struct v4l2_ctrl_config cam_scene_mode  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_CAMERA_SCENE_MODE ,
        .name = "V4L2_CID_CAMERA_SCENE_MODE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = -1,
        .max = 14,
        .step = 1,
        .def = -1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};


//	ctrls->is_zoom = v4l2_ctrl_new_std(handler, &fimc_ctrl_ops,
//					V4L2_CID_IS_ZOOM, 1, 100, 1, 0);

struct v4l2_ctrl_config is_zoom   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_ZOOM,
        .name = "V4L2_CID_IS_ZOOM",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_fcount   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_COUNT,
        .name = "V4L2_CID_IS_FD_GET_FACE_COUNT",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_fnum  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_FRAME_NUMBER,
        .name = "V4L2_CID_IS_FD_GET_FACE_FRAME_NUMBER",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 0,
        .max = 5,
        .step = 1,
        .def = 0,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_fconfidence  = {
        .ops = &fimc_ctrl_ops,
        .id =V4L2_CID_IS_FD_GET_FACE_CONFIDENCE ,
        .name = "V4L2_CID_IS_FD_GET_FACE_CONFIDENCE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_slevel   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_SMILE_LEVEL,
        .name = "V4L2_CID_IS_FD_GET_FACE_SMILE_LEVEL",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_blevel   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_BLINK_LEVEL,
        .name = "V4L2_CID_IS_FD_GET_FACE_BLINK_LEVEL",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_topleft_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_TOPLEFT_X,
        .name = "V4L2_CID_IS_FD_GET_FACE_TOPLEFT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_topleft_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_TOPLEFT_Y,
        .name = "V4L2_CID_IS_FD_GET_FACE_TOPLEFT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_bottom_right_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_X,
        .name = "V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_bottom_right_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_Y,
        .name = "V4L2_CID_IS_FD_GET_FACE_BOTTOMRIGHT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_le_tleft_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_X,
        .name = "V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_le_tleft_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_Y,
        .name = "V4L2_CID_IS_FD_GET_LEFT_EYE_TOPLEFT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_le_b_right_x  = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_X,
        .name = "V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_le_b_right_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_Y,
        .name = "V4L2_CID_IS_FD_GET_LEFT_EYE_BOTTOMRIGHT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_re_tleft_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_X,
        .name = "V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_re_tleft_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_Y,
        .name = "V4L2_CID_IS_FD_GET_RIGHT_EYE_TOPLEFT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_re_b_right_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_X,
        .name = "V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_re_b_right_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_Y,
        .name = "V4L2_CID_IS_FD_GET_RIGHT_EYE_BOTTOMRIGHT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};
struct v4l2_ctrl_config is_ext_m_tleft_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_X,
        .name = "V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_m_tleft_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_Y,
        .name = "V4L2_CID_IS_FD_GET_MOUTH_TOPLEFT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};
struct v4l2_ctrl_config is_ext_m_b_right_x   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_X,
        .name = "V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_m_b_right_y   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_Y,
        .name = "V4L2_CID_IS_FD_GET_MOUTH_BOTTOMRIGHT_Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_g_angle   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_ANGLE,
        .name = "V4L2_CID_IS_FD_GET_ANGLE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_g_yaw_angle   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_YAW_ANGLE,
        .name = "V4L2_CID_IS_FD_GET_YAW_ANGLE",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

struct v4l2_ctrl_config is_ext_g_next   = {
        .ops = &fimc_ctrl_ops,
        .id = V4L2_CID_IS_FD_GET_NEXT,
        .name = "V4L2_CID_IS_FD_GET_NEXT",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = 0,
        .min = 1,
        .max = 100,
        .step = 1,
        .def = 1,
        .menu_skip_mask = 0,
        .qmenu = NULL,
        .qmenu_int = NULL,
        .is_private = 0,
};

#endif
