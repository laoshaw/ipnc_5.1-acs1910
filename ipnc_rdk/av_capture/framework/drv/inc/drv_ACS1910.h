/***********************************************************
@file     drv_ACS1910.h
@brief    ACS1910相关数据
@version  V1.0
@date     2017-08-01
@author   pamsimochen
Copyright (c) 2017-2019 VIFOCUS
***********************************************************/

#ifndef _DRV_ACS1910_H_
#define _DRV_ACS1910_H_ 

#include "cmd_server.h"

#define FOCUS_PWM_IO    (92) 
#define FOCUS_PWM_NO    0
#define FOCUS_A_IO      (44)
#define FOCUS_B_IO      (26)
#define ZOOM_PWM_IO     (91)
#define ZOOM_PWM_NO     1
#define ZOOM_A_IO       (37)
#define ZOOM_B_IO       (45)
#define IRIS_PWM_IO     (87)
#define IRIS_PWM_NO     2
#define IRIS_A_IO       (82)
#define IRIS_B_IO       (81)
#define IRCUT_PWM_IO    (85)
#define IRCUT_PWM_IO    3
#define IRCUT_A_IO     (80)
#define IRCUT_B_IO     (79)

#define FOCUS_WIPPER_ADC_CH (3)
#define ZOOM_WIPPER_ADC_CH  (0)
#define IRIS_WIPPER_ADC_CH  (4)

#define SPI0_SCS0_IO        (29)
#define SPI0_SOMI_IO        (23)
#define SPI0_SIMO_IO        (22)
#define SPI0_SCLK_IO        (24)

#define SPI2_SCS0_IO        (33)
#define SPI2_SOMI_IO        (31)
#define SPI2_SIMO_IO        (30)
#define SPI2_SCLK_IO        (32)

#define ACS1910_DEFAULT_CFG "/mnt/nand/acs1910_default.cfg"
#define ACS1910_SAVED_CFG "/mnt/nand/acs1910_saved.cfg"

#define DEFAULT_EXPOSURE_TIME 9984

#define DEFAULT_ROI1_X 0
#define DEFAULT_ROI1_Y 0
#define DEFAULT_ROI1_WIDTH 1919
#define DEFAULT_ROI1_HEIGHT 1079
#define DEFAULT_ROI2_X 0
#define DEFAULT_ROI2_Y 0
#define DEFAULT_ROI2_WIDTH 1919
#define DEFAULT_ROI2_HEIGHT 1079
#define DEFAULT_ROI3_X 0
#define DEFAULT_ROI3_Y 0
#define DEFAULT_ROI3_WIDTH 1919
#define DEFAULT_ROI3_HEIGHT 1079
#define ROI_NO 3
#define DEFAULT_ROI_1 {DEFAULT_ROI1_X, DEFAULT_ROI1_Y, DEFAULT_ROI1_WIDTH, DEFAULT_ROI1_HEIGHT}
#define DEFAULT_ROI_2 {DEFAULT_ROI2_X, DEFAULT_ROI2_Y, DEFAULT_ROI2_WIDTH, DEFAULT_ROI2_HEIGHT}
#define DEFAULT_ROI_3 {DEFAULT_ROI3_X, DEFAULT_ROI3_Y, DEFAULT_ROI3_WIDTH, DEFAULT_ROI3_HEIGHT}
#define DEFAULT_AE_ROI {DEFAULT_ROI_1, DEFAULT_ROI_2, DEFAULT_ROI_3}

#define DEFAULT_AWB_INTERVAL 0
#define DEFAULT_AWB_SPEED 16
#define DEFAULT_AWB_AWBDELAY 10

#define DEFAULT_BRIGHTNESS 50 
#define DEFAULT_CONTRAST 50
#define DEFAULT_SATURATION 50
#define DEFAULT_HUE 50
#define DEFAULT_SHARPNESS 50

#define DEFAULT_IRCUT_TH 16

typedef struct ACS1910_CFG{
    VF_AE_MODE_S AEMode;
    VF_AE_ROI_S AERoi[ROI_NO];
    VF_AWB_MODE_S AWBMode;
    VF_BASE_ATTRIBUTE_S BaseAttr;
    VF_IRCUT_MODE_S IRCutMode;
    VF_DR_MODE_S DRMode;
    VF_FLIP_MIRROR_MODE_E FlipMirrorMode;
    VF_IRIS_MODE_E IrisMode;
    VF_COLORBLACK_MODE_E ColorBlackMode;
    VF_DENOISE_MODE_S DeNoiseMode;
    VF_EIS_FLAG_E EISFlag;
    VF_DEFOG_MODE_E DefogMode;
    VF_MAXFRMRATE_E MaxFrmRate;
    //unsigned int AEDelay;
}tACS1910Cfg, *ptACS1910Cfg;

int DRV_ACS1910Init();

#endif


