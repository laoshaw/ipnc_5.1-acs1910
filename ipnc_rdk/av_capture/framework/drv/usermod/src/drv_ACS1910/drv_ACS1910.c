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
#include "drv_rtc.h"
#include <sys/stat.h>

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

static pthread_t check_default_set_thread_id;
static int check_default_set_thread_run = 0;
static int check_fpga_down = 1;

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

    //enable gio79 gio80 gio81 gio82 gio83 pwm3(gio85) pwm2(gio87) pwm1(gio91) pwm0(gio92)
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 1, &value32);
    VI_DEBUG("pinmux1 = 0x%08X\n", value32);
    value32 |= 0x0043080A;
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

    //enable gio37 gio33(SPI2_CS) gio32(SPI2_SCLK) gio31(SPI2_SMOI) gio30(SPI2_SIMO) gio28(reset vim) gio27(program_b) 
    //gio35(done)
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 4, &value32);
    VI_DEBUG("pinmux4 = 0x%08X\n", value32);
    value32 &= 0xFFCCC030;
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

    DRV_gpioSetMode(SYS_SET_DEFAULT_IO, DRV_GPIO_DIR_IN);

    DRV_gpioSetMode(FPGA_PROGRAM_B_IO, DRV_GPIO_DIR_IN);
    DRV_gpioSetMode(FPGA_DONE_IO, DRV_GPIO_DIR_IN);

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
    int fd;
    int ret;
    int need_create = 0;
    struct stat cfg_stat;

    need_create = 0;
    ret = stat(ACS1910_SAVED_CFG, &cfg_stat);
    if(ret != 0)
    {
        if(errno == ENOENT )
        {
            VI_DEBUG("saved cfg file is not exist\n");
        }
        need_create = 1;
    }
    else 
    {
        if(cfg_stat.st_size != sizeof(tACS1910Cfg))
            need_create = 1;
    }
    if(need_create == 1)
    {
        VI_DEBUG("create a new saved cfg\n");
        fp = fopen(ACS1910_SAVED_CFG, "wb");
        if(fp == NULL)
        {
           perror("create saved_cfg file error\n"); 
           return OSA_EFAIL;
        }
        else 
        {
            fd = fileno(fp);
            ret = fwrite(&gACS1910_default_cfg, 1, sizeof(tACS1910Cfg), fp);
            VI_DEBUG("write %d into cfg\n", ret);
            if(ret != sizeof(tACS1910Cfg))
            {
                perror("write saved_cfg file error\n");
                fclose(fp);
                return OSA_EFAIL;
            }
            fsync(fd);
            fclose(fp);
        }
    }

    return OSA_SOK; 
}
static int init_cfg(char *file)
{
    int ret;
    FILE *fp;
    int fd;

    
    fp = fopen(file, "rb");
    if(fp == NULL)
    {
        perror("open file error\n");
        return OSA_EFAIL;
    }
    fd = fileno(fp);
    VI_DEBUG("sizeof(tACS1910Cfg) = %d\n", sizeof(tACS1910Cfg));
    ret = fread(&gACS1910_saved_cfg, 1, sizeof(tACS1910Cfg), fp);
    if(ret != sizeof(tACS1910Cfg))
    {
        perror("read saved cfg error!\n");
        return OSA_EFAIL;
    }
    fsync(fd);
    fclose(fp);
    memcpy(&gACS1910_current_cfg, &gACS1910_saved_cfg, sizeof(tACS1910Cfg));

    return OSA_SOK;
}

int save_current_cfg()
{
   int ret;
   FILE *saved_fp;
   int fd;
#if 0
   saved_fp = fopen(ACS1910_SAVED_CFG, "wb+");
   if(saved_fp == NULL)
   {
       perror("open saved cfg file error\n");
       return OSA_EFAIL;
   }
   ret = fwrite(&gACS1910_current_cfg, 1, sizeof(tACS1910Cfg), saved_fp);
   if(ret != sizeof(tACS1910Cfg))
   {
       perror("wirte current cfg error\n");
       return OSA_EFAIL;
   }
   ret = fflush(saved_fp);
   ret = fclose(saved_fp);
   if(ret != 0)
   {
       perror("save current cfg error\n");
   }
   memcpy(&gACS1910_saved_cfg, &gACS1910_current_cfg, sizeof(tACS1910Cfg));
   VI_DEBUG("saved current cfg!\n");
   //system("cp /mnt/nand/acs1910_saved.cfg /mnt/nand/acs1910_saved.cfg.bak\n");

#endif
   VI_DEBUG("gACS1910_current_cfg.FlipMirrorMode = %d\n", gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.FlipMirrorMode);
   fd = open(ACS1910_SAVED_CFG, O_WRONLY | O_TRUNC);
   if(fd == -1)
   {
       perror("open saved cfg file error\n");
   }
   ret = write(fd, &gACS1910_current_cfg, sizeof(tACS1910Cfg));
   if(ret != sizeof(tACS1910Cfg))
   {
       perror("write current cfg error\n");
       close(fd);
       return OSA_EFAIL;
   }
   fsync(fd);
   close(fd);
   memcpy(&gACS1910_saved_cfg, &gACS1910_current_cfg, sizeof(tACS1910Cfg));
   VI_DEBUG("saved current_cfg!\n");

   return OSA_SOK;
}

void check_default_set_thread()
{
    int default_set = 1;
    int count = 0;
    FILE *fp;

    while(check_default_set_thread_run) 
    {
        sleep(1);
        default_set = DRV_gpioGet(SYS_SET_DEFAULT_IO);
        //VI_DEBUG("default_set = %d\n", default_set);
        if(default_set == 0)
            count++;
        if(default_set == 1)
            count = 0;
        //VI_DEBUG("count = %d\n", count);
        if(count == 3)
            check_default_set_thread_run = 0;
    }
    VI_DEBUG("Reset FPGA\n"); 
    default_set = 0;
    DRV_gpioSetMode(FPGA_PROGRAM_B_IO, DRV_GPIO_DIR_OUT); 
    DRV_gpioClr(FPGA_PROGRAM_B_IO);
    usleep(1000000);
    DRV_gpioSet(FPGA_PROGRAM_B_IO);
    DRV_gpioSetMode(FPGA_PROGRAM_B_IO, DRV_GPIO_DIR_IN);
    while(check_fpga_down)
    {
        sleep(1);
        default_set = DRV_gpioGet(FPGA_DONE_IO);
        VI_DEBUG("done is %d\n", default_set);
        if(default_set == 1)
            check_fpga_down = 0;
    }
    

    VI_DEBUG("rm -rf cfg file\n");
    //system("rm -rf /mnt/nand/acs1910_default.cfg");
    system("rm -rf /mnt/nand/acs1910_saved.cfg");
    system("rm -rf /mnt/nand/sysenv.cfg");
    system("reboot");
   
}

static int check_default_set_thread_init()
{
    int retVal = 0;
    VI_DEBUG("Initialize check_default_set thread\n");
   
    if((retVal = pthread_create(&check_default_set_thread_id, NULL, check_default_set_thread, NULL)) != 0)
    {
        perror("Create check_default_set thread\n");
    }
    else 
    {
        check_default_set_thread_run = 1;
        VI_DEBUG("Create check_default_set thread done!\n\n");
    }
    return retVal;
}

int Set_SysTime(pVF_TIME_S ptime)
{
   char time_date[32];

   struct tm t_tm;
   struct timeval tv;
   time_t timep;

   t_tm.tm_sec = ptime->second;
   t_tm.tm_min = ptime->minute;
   t_tm.tm_hour = ptime->hour;
   t_tm.tm_mday = ptime->day;
   t_tm.tm_mon = ptime->month - 1;
   t_tm.tm_year = ptime->year + 2000 - 1900;

   if(ptime->month == 1 || ptime->month == 2)
   {
       ptime->month += 12;
       ptime->year--;
   }
   t_tm.tm_wday = (ptime->day + 1 + 2*ptime->month + 3*(ptime->month + 1)/5 + ptime->year + ptime->year/4 - ptime->year/100 + ptime->year/400) % 7;
   VI_DEBUG("t_tm.tm_wday = %d\n", t_tm.tm_wday);

   timep = mktime(&t_tm);
   tv.tv_sec = timep;
   tv.tv_usec = 0;
   
   if(settimeofday(&tv, (struct timezone *)0) < 0)
   {
       perror("settimeofday");
       return -1;
   }
   return 0;
}

/***********************************************************
\brief 初始化ACS1910相关的硬件、消息等 
\param 
 
\return 0:成功 其它:失败
***********************************************************/
int DRV_ACS1910Init()
{
    int status = OSA_SOK;
    VF_TIME_S date_time;

    RTC_GetDate(&date_time);
    Set_SysTime(&date_time);

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

    status = check_default_set_thread_init();
    if(status != OSA_SOK)
    {
        VI_DEBUG("check_default_set_thread init error\n");
        return status;
    }

    return status;
}


int DRV_ACS1910Exit()
{
    int status = OSA_SOK;

    status = ledPWM_exit();
//    if(check_default_set_thread_run == 1)
    {
      check_default_set_thread_run = 0;
      pthread_join(check_default_set_thread_id, NULL);
    }
    check_fpga_down = 0;
}


