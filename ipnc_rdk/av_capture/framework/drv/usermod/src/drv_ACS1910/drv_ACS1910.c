/***********************************************************
@file     drv_ACS1910.c
@brief    实现ACS1910相关的控制功能
          主要是接收来自cmd_server的消息进行各类操作
@version  V1.0
@date     2017-08-01
@author   pamsimochen
Copyright (c) 2017-2019 VIFOCUS 
***********************************************************/

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <drv_csl.h>
#include <drv_gpio.h>
#include "drv.h"
#include "drv_ACS1910.h"
#include "cmd_server.h"
#include "VIM_API_Release.h"
#if 1
tACS1910Cfg gACS1910_default_cfg = {
    {//ISPallCfg
        //VF_AE_ROI_S AERoi;
        DEFAULT_AE_ROI,
        {//ISPNormalCfg
            {//VF_AE_MODE_S AEMode;
                VF_AE_50Hz_Auto,
                VF_AE_S5,
                DEFAULT_EXPOSURE_TIME,
                32,
                1,
                255,
                0
            },
            //VF_AE_AWB_MODE_S AEAWBMode;
            {
                VF_AWB_AUTO,
                VF_AWB_WITH_AE_STABLE,
                DEFAULT_AWB_INTERVAL,
                DEFAULT_AWB_SPEED,
                DEFAULT_AWB_AWBDELAY 
            },
            //VF_BASE_ATTRIBUTES BaseAttr;
            {
                DEFAULT_BRIGHTNESS,
                DEFAULT_CONTRAST,
                DEFAULT_SATURATION, 
                DEFAULT_HUE, 
                DEFAULT_SHARPNESS, 
            },
            //VF_IRCUT_MODE_S IRCutMode;
            {
                VF_IRCUT_AUTO,
                DEFAULT_IRCUT_TH
            },
            //VF_DR_MODE_S DRMode;
            {
                VF_DRMODE_NORMAL,
                VF_DR_LEVEL_LOW
            },
            //VF_FLIP_MIRROR_MODE_E FlipMirrorMode;
            VF_FLIP_MIRROR_FlipMirror,
            //VF_IRIS_MODE_E IrisMode;
            VF_IRIS_AUTO_IRIS,
            //VF_COLORBLACK_MODE_E ColorBlackMode;
            VF_COLORBLACK_AUTO,
            //VF_DENOISE_MODE_S DeNoiseMode;
            {
                VF_DENOISE_2D,
                VF_NR3D_LEVEL_LOW
            },
            //VF_EIS_FLAG_E EISFlag;
            VF_EIS_DISABLE,
            //VF_DEFOG_MODE_E DefogMode;
            VF_DEFOG_CLOSE,
            //VF_MAXFRMRATE_E MaxFrmRate
            VF_FPS25
        }//ISPNormalCfg
    },//ISPAllCfg
    {//SYSCfg
        {//camera id
            1,
            DEFAULT_CAMERA_NAME
        },
        {//osd_onoff
            0,
            0,
            0
        }
    },
    {//LenCfg
        5,
        5,
        5
    }
};

tACS1910Cfg gACS1910_saved_cfg;
tACS1910Cfg gACS1910_current_cfg;
#endif
/***********************************************************
\brief 初始化ACS1910相关的GIO Mux以及初始状态 
\param 
 
\return
***********************************************************/
static void acs1910_gpio_init()
{
    unsigned int value32;
    //enable gio44 gio45
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 0, &value32);
    VI_DEBUG("pinmux0 = 0x%08X\n", value32);
    value32 &= 0xFFF3FFFF;
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 0, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 0, &value32);
    VI_DEBUG("pinmux0 = 0x%08X\n\n", value32);

    //enable gio79 gio80 gio81 gio82 pwm3(gio85) pwm2(gio87) pwm1(gio91) pwm0(gio92)
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 1, &value32);
    VI_DEBUG("pinmux1 = 0x%08X\n", value32);
    value32 |= 0x0042080A;
    value32 &= 0xFFC3BBFA;
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 1, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 1, &value32);
    VI_DEBUG("pinmux1 = 0x%08X\n\n", value32);

    //enable gio26
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 3, &value32);
    VI_DEBUG("pinmux3 = 0x%08X\n", value32);
    value32 &= 0x7FFFFFFF;
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 3, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 3, &value32);
    VI_DEBUG("pinmux3 = 0x%08X\n\n", value32);

    //enable gio37 gio33(SPI2_CS) gio32(SPI2_SCLK) gio31(SPI2_SMOI) gio30(SPI2_SIMO) gio28(reset vim) 
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 4, &value32);
    VI_DEBUG("pinmux4 = 0x%08X\n", value32);
    value32 &= 0xFFCFC033;
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 4, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 4, &value32);
    VI_DEBUG("pinmux4 = 0x%08X\n\n", value32);

    DRV_gpioSetMode(FOCUS_A_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(FOCUS_A_IO);//Disable FOCUS PWM Out
    DRV_gpioSetMode(FOCUS_B_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(FOCUS_B_IO);//Disable FOCUS PWM Out
    
    DRV_gpioSetMode(ZOOM_A_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(ZOOM_A_IO);//Disable ZOOM PWM Out
    DRV_gpioSetMode(ZOOM_B_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(ZOOM_B_IO);//Disable ZOOM PWM Out

    DRV_gpioSetMode(IRIS_A_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(IRIS_A_IO);//Disable IRIS PWM Out
    DRV_gpioSetMode(IRIS_B_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(IRIS_B_IO);//Disable IRIS PWM Out

    DRV_gpioSetMode(IRCUT_A_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(IRCUT_A_IO);//Disable IRCUT PWM Out
    DRV_gpioSetMode(IRCUT_B_IO, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(IRCUT_B_IO);//Disable IRCUT PWM Out
 
    //Disable SPI2 when not update fpga
    DRV_gpioSetMode(SPI2_SCS0_IO, DRV_GPIO_DIR_IN);
    DRV_gpioSetMode(SPI2_SCLK_IO, DRV_GPIO_DIR_IN);
    DRV_gpioSetMode(SPI2_SIMO_IO, DRV_GPIO_DIR_IN);
    DRV_gpioSetMode(SPI2_SOMI_IO, DRV_GPIO_DIR_IN);

    //DRV_gpioSetMode(VIM_RESET_IO, DRV_GPIO_DIR_OUT);
    //DRV_gpioClr(VIM_RESET_IO);
    //sleep(1);
    //DRV_gpioSet(VIM_RESET_IO);
    //sleep(5);
    VI_DEBUG("ACS1910 GPIO Initial OK!\n\n");

}

static int check_cfg_file()
{
    FILE *fp;
    int ret;

    //check default cfg, if there is no then create one
    fp = fopen(ACS1910_DEFAULT_CFG, "rb"); 
    if(fp == NULL)
    {
        perror("check_cfg_file open error!\n");
        fp = fopen(ACS1910_DEFAULT_CFG, "wb");
        if(fp == NULL)
        {
           perror("create default_cfg file error\n"); 
           return OSA_EFAIL;
        }
        else 
        {
            ret = fwrite(&gACS1910_default_cfg, 1, sizeof(tACS1910Cfg), fp);
            VI_DEBUG("write %d into cfg\n", ret);
            if(ret != sizeof(tACS1910Cfg))
            {
                perror("write default_cfg file error\n");
                fclose(fp);
                return OSA_EFAIL;
            }
            fclose(fp);
        }
    }
    else 
    {
        VI_DEBUG("There is a default cfg file\n");
        fclose(fp);
    }
    //check saved cfg, if there is no then create one equal the default one 
    fp = fopen(ACS1910_SAVED_CFG, "rb");
    if(fp == NULL)
    {
        perror("check_cfg_file open error!\n");
        fp = fopen(ACS1910_SAVED_CFG, "wb");
        if(fp == NULL)
        {
           perror("create default_cfg file error\n"); 
           return OSA_EFAIL;
        }
        else 
        {
            ret = fwrite(&gACS1910_default_cfg, 1, sizeof(tACS1910Cfg), fp);
            VI_DEBUG("write %d into cfg\n", ret);
            if(ret != sizeof(tACS1910Cfg))
            {
                perror("write default_cfg file error\n");
                fclose(fp);
                return OSA_EFAIL;
            }
            fclose(fp);
        }
    }
    else 
    {
        VI_DEBUG("There is a saved cfg file\n");
        fclose(fp);
    }
    return OSA_SOK; 
    
}
static int init_cfg(char *file)
{
    int ret;
    FILE *fp;

    
    fp = fopen(file, "rb");
    if(fp == NULL)
    {
        perror("open file error\n");
        return OSA_EFAIL;
    }
    VI_DEBUG("sizeof(tACS1910Cfg) = %d\n", sizeof(tACS1910Cfg));
    ret = fread(&gACS1910_saved_cfg, 1, sizeof(tACS1910Cfg), fp);
    if(ret != sizeof(tACS1910Cfg))
    {
        perror("read saved cfg error!\n");
        return OSA_EFAIL;
    }
    fclose(fp);
    memcpy(&gACS1910_current_cfg, &gACS1910_saved_cfg, sizeof(tACS1910Cfg));

    return OSA_SOK;
}

int save_current_cfg()
{
   int ret;
   FILE *fp;

   fp = fopen(ACS1910_SAVED_CFG, "w");
   if(fp == NULL)
   {
       perror("open saved cfg file error\n");
       return OSA_EFAIL;
   }
   ret = fwrite(&gACS1910_current_cfg, 1, sizeof(tACS1910Cfg), fp);
   if(ret != sizeof(tACS1910Cfg))
   {
       perror("save current cfg error\n");
       return OSA_EFAIL;
   }
   fclose(fp);
   memcpy(&gACS1910_saved_cfg, &gACS1910_current_cfg, sizeof(tACS1910Cfg));

   return OSA_SOK;
}

/***********************************************************
\brief 初始化ACS1910相关的硬件、消息等 
\param 
 
\return 0:成功 其它:失败
***********************************************************/
int DRV_ACS1910Init()
{
    int status = OSA_SOK;

    status = check_cfg_file();
    if(status != OSA_SOK)
    {
        VI_DEBUG("check cfg file error!\n");
        return status;
    }

    status = init_cfg(ACS1910_SAVED_CFG);
    if(status != OSA_SOK)
    {
        VI_DEBUG("init cfg error!\n");
        return status;
    }


    acs1910_gpio_init();

    status = lenPWM_init();
    if(status != OSA_SOK)
    {
        VI_DEBUG("lenPWM init error!\n");
        return status;
    }

    return status;
}


int DRV_ACS1910Exit()
{
    int status = OSA_SOK;

    status = ledPWM_exit();

}


