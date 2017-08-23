#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drv_imgs_MC1208_2MP.h"
#include <drv_gpio.h>
#include "VIM_API_Release.h"
#include <fcntl.h>
#include "osa_file.h"
#include "drv_lenPWM.h"
#include <sys/msg.h>
#include <cmd_server.h>
#include "drv_ACS1910.h"
#include "drv_rtc.h"
#include <linux/spi/spidev.h>

DRV_ImgsObj gDRV_imgsObj;
//VIM_ATTRIBUTE_S gVIM_CurAttr;
static int vim_cmd_msqid = 0;
static int vim_ack_msqid = 0;
VF_AE_SHUTTER_MODE_E gVIMAEShutterMode;

//tACS1910Cfg ggACS1910_current_cfg;
//unsigned int gAEDlay_roi;

static pthread_t VIM_control_thread_id;
static int VIM_control_thread_run = 0;
static pthread_t VIM_roi_autoexp_thread_id;
static int VIM_roi_autoexp_thread_run = 0;

/***********************************************************
\brief 获取VIM模组的版本信息 
\param pVIM_GenInfo:版本信息数据指针 

\return 0:成功 其它:失败
***********************************************************/
static int DRV_FPGASPIWrite(unsigned short reg, unsigned short data)
{
    int status = OSA_SOK;

    fpga_spi_data spi_data;

    spi_data.wr = FPGA_SPI_WR_WRITE & 0x01;
    spi_data.reg_addr = reg & 0x7fff;
    spi_data.data = data;

    if(DRV_SPIWrite(&gDRV_imgsObj.spiHndl, &spi_data, sizeof(fpga_spi_data)) != sizeof(fpga_spi_data))
    {
        VI_DEBUG("DRV_SPIWrite FPGA error!\n");
        status = OSA_EFAIL;
    }
    return status;
}
/***********************************************************
\brief 获取VIM模组的版本信息 
\param pVIM_GenInfo:版本信息数据指针 

\return 0:成功 其它:失败
***********************************************************/
static int DRV_FPGASPIRead(unsigned short reg, unsigned short *data)
{
    int status = OSA_SOK;

    fpga_spi_data spi_data;
    fpga_spi_data spi_read;

    spi_data.wr = FPGA_SPI_WR_READ & 0x01;
    spi_data.reg_addr = reg & 0x7fff;
    spi_data.data = 0;

    VI_DEBUG("wr reg_addr: 0x%04X\n ", *(unsigned short *)&spi_data);

    if(DRV_SPIRead(&gDRV_imgsObj.spiHndl, &spi_data, sizeof(fpga_spi_data), &spi_read) != sizeof(fpga_spi_data))
    {
        VI_DEBUG("DRV_SPIRead FPGA error!\n");
        return OSA_EFAIL;
    }
    VI_DEBUG("spi_read.data = %d\n", spi_read.data);
    *data = spi_read.data;

    return status;
}
/***********************************************************
\brief 获取VIM模组的版本信息 
\param pVIM_GenInfo:版本信息数据指针 

\return 0:成功 其它:失败
***********************************************************/
static int DRV_GetVIMGenInfo(pVIM_GENERAL_INFO pVIM_GenInfo)
{
    int status = 0;

    status = VIM_GetGeneralAttribute(pVIM_GenInfo);
    if(status!=VIM_SUCCEED){
        OSA_ERROR("VIM GetGeneralAttribute\n");
        printf("status = %d\n", status);
        return OSA_EFAIL;
    }
#if 0
    VI_DEBUG("General Attribute flag                    = %x\n", pVIM_GenInfo->flag);
    VI_DEBUG("General Attribute Sensor_VID              = %d\n", pVIM_GenInfo->sensor_VID);
    VI_DEBUG("General Attribute Sensor_ID               = %d\n", pVIM_GenInfo->sensor_ID);
    VI_DEBUG("General Attribute Chip_Version            = %d\n", pVIM_GenInfo->chip_Version);
    VI_DEBUG("General Attribute Ini_Version             = %d\n", pVIM_GenInfo->Ini_Version);
    VI_DEBUG("General Attribute ProjectSpec_Version     = %d\n", pVIM_GenInfo->projectSpec_Version);
    VI_DEBUG("General Attribute Content_Version         = %d\n", pVIM_GenInfo->content_Version);
    VI_DEBUG("General Attribute date_info               = %02x %02x %02x %02x\n", pVIM_GenInfo->date_info[3], pVIM_GenInfo->date_info[2], pVIM_GenInfo->date_info[1], pVIM_GenInfo->date_info[0]);
    VI_DEBUG("General Attribute Firmware_version        = %d\n", pVIM_GenInfo->Firmware_Version);
#endif
    return status;
}

/***********************************************************
\brief 获取安装的VIM模组的版本信息，并更新存储在nand上的版本
       文件，以便通过网络查询相关信息
\param 

\return 0:成功 其它:失败
***********************************************************/
static int DRV_UpdateVIMGenInfoFile()
{
    char *file = "/mnt/nand/vim_general_info";
    FILE *fp;
    int ret;
    VIM_GENERAL_INFO VIM_GenInfo;

    ret = DRV_GetVIMGenInfo(&VIM_GenInfo);
    if(ret != VIM_SUCCEED)
    {
        VI_DEBUG("Get VIM GenInfo error!\n");
        fclose(fp);
        return OSA_EFAIL;
    }
    fp = fopen(file, "w"); 
    if(fp == NULL)
    {
        perror("create vim_general_info file error\n"); 
        return OSA_EFAIL;
    }
    else 
    {
        fseek(fp, 0, SEEK_SET);
        ret = fprintf(fp, "%04X,%X,%04X,%04X,%04X,%04X,%X,%X,%X,%X,%X,%08X\n", \
                VIM_GenInfo.flag, VIM_GenInfo.sensor_VID, VIM_GenInfo.sensor_ID, VIM_GenInfo.chip_Version,\
                VIM_GenInfo.Ini_Version, VIM_GenInfo.projectSpec_Version, VIM_GenInfo.content_Version,\
                VIM_GenInfo.date_info[3], VIM_GenInfo.date_info[2],VIM_GenInfo.date_info[1], VIM_GenInfo.date_info[0],\
                VIM_GenInfo.Firmware_Version);
        if(ret == 0)
        {
            perror("write vim info error\n");
            fclose(fp);
            return OSA_EFAIL;
        }
        fclose(fp);
    }

    return OSA_SOK; 
    
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
\brief  设置相机时间，设置系统时间以及ＲＴＣ的时间
       
\param  pVF_TIME_S:参数数据结构 

\return 0:成功 其它:失败
***********************************************************/
int DRV_SetTime(pVF_TIME_S ptime)
{
    Set_SysTime(ptime);
    RTC_SetDate(ptime);
}
/***********************************************************
\brief  设置相机ID
       
\param  pVF_CAMERA_ID_S:参数数据结构 

\return 0:成功 其它:失败
***********************************************************/
int DRV_SetCameraID(pVF_CAMERA_ID_S pCameraID)
{
    int ret = 0;
    FILE *fp;
    

    VI_DEBUG("Camera id = %d\n", pCameraID->id);
    VI_DEBUG("Camera name = %s\n", pCameraID->name);
    memcpy(&gACS1910_saved_cfg.SYSCfg.camera_id, pCameraID, sizeof(VF_CAMERA_ID_S));
    memcpy(&gACS1910_current_cfg.SYSCfg.camera_id, pCameraID, sizeof(VF_CAMERA_ID_S));
    fp = fopen(ACS1910_SAVED_CFG, "wb");
    if(fp == NULL)
    {
        VI_DEBUG(":");
        perror("open ACS1910_SAVED_CFG\n");
    }
    ret = fwrite(&gACS1910_saved_cfg, 1, sizeof(tACS1910Cfg), fp);
    if(ret != sizeof(tACS1910Cfg))
    {
        perror("save saved cfg error\n");
        return OSA_EFAIL;
    }
    fclose(fp);
    return 0; 
}

/***********************************************************
\brief 获取VIM当前的设置参数
       
\param pVIM_CurAttr: 参数数据结构

\return 0:成功 其它:失败
***********************************************************/
//貌似这个函数到最后正常阶段没啥用
int DRV_GetVIMAttr(pVIM_ATTRIBUTE_S pVIM_CurAttr)
{
   int status = VIM_SUCCEED;
   int i;
   UINT8 CurAttr[4096];

    status = VIM_GetCurrentAttribute(CurAttr);
    VI_DEBUG("sizeof VIM_ATTRIBUTE_S = %d\n", sizeof(VIM_ATTRIBUTE_S));
    if(status!=VIM_SUCCEED){
        OSA_ERROR("VIM GetCurrentAttribute");
        printf("status = %d\n", status);
        return status;
    }
    i = 0;
    pVIM_CurAttr->AEMode.AE_Shutter_Mode = CurAttr[i++];
    gVIMAEShutterMode = pVIM_CurAttr->AEMode.AE_Shutter_Mode;
    pVIM_CurAttr->AEMode.AE_MaxET_Mode = CurAttr[i++];
    pVIM_CurAttr->AEMode.Exposuretime = (CurAttr[i] << 24) + (CurAttr[i+1] << 16) + (CurAttr[i+2] << 8) + CurAttr[i+3];
    i = i+4;
    pVIM_CurAttr->AEMode.MaxET = (CurAttr[i] << 24) + (CurAttr[i+1] << 16) + (CurAttr[i+2] << 8) + CurAttr[i+3];
    i = i+4;
    pVIM_CurAttr->AEMode.DGain = CurAttr[i++];
    pVIM_CurAttr->AEMode.AGain = CurAttr[i++];
    pVIM_CurAttr->AEMode.MaxDGain = CurAttr[i++];
    pVIM_CurAttr->AEMode.MaxAGain = CurAttr[i++];
    pVIM_CurAttr->AEMode.DGainDeci = CurAttr[i++];
    pVIM_CurAttr->AEMode.AGainDeci = CurAttr[i++];
    pVIM_CurAttr->AWBMode = CurAttr[i++];
    pVIM_CurAttr->BrightnessCoeff = CurAttr[i++];
    pVIM_CurAttr->HueCoeff = CurAttr[i++];
    pVIM_CurAttr->ContrastCoeff = CurAttr[i++];
    pVIM_CurAttr->EdgeCoeff = CurAttr[i++];
    pVIM_CurAttr->SaturationCoeff = CurAttr[i++];
    pVIM_CurAttr->FlipMirrorMode = CurAttr[i++];
    pVIM_CurAttr->IrisMode = CurAttr[i++];
    pVIM_CurAttr->IRcutMode = CurAttr[i++];
    pVIM_CurAttr->ColorBlack = CurAttr[i++];
    pVIM_CurAttr->DRMode = CurAttr[i++];
    pVIM_CurAttr->DRLevel = CurAttr[i++];
    pVIM_CurAttr->ColorType = CurAttr[i++];
    pVIM_CurAttr->DenoiseMode = CurAttr[i++];
    pVIM_CurAttr->AEdelay = (CurAttr[i] << 24) + (CurAttr[i+1] << 16) + (CurAttr[i+2] << 8) + CurAttr[i+3];
    i = i+4;
    pVIM_CurAttr->EisMode = CurAttr[i++];
    pVIM_CurAttr->LdcMode = CurAttr[i++];
    pVIM_CurAttr->DssNum = CurAttr[i++];
    pVIM_CurAttr->DefogMode = CurAttr[i++];
    pVIM_CurAttr->IRC_TH3 = CurAttr[i++];
    pVIM_CurAttr->NR3DLevel = CurAttr[i++];
    pVIM_CurAttr->TmMode = CurAttr[i++];
    pVIM_CurAttr->AWBAERelation = CurAttr[i++];
    pVIM_CurAttr->AWBInterval = CurAttr[i] << 8 + CurAttr[i+1];
    i = i+2;
    pVIM_CurAttr->AWBDelay = CurAttr[i] << 8 + CurAttr[i+1];
    i = i+2;
    pVIM_CurAttr->MaxFrmRate = CurAttr[i];
#if 1
    VI_DEBUG("AE Shutter Mode                           = %d\n", pVIM_CurAttr->AEMode.AE_Shutter_Mode);
    VI_DEBUG("AE MaxET Mode                             = %d\n", pVIM_CurAttr->AEMode.AE_MaxET_Mode);
    VI_DEBUG("Exposuretime                              = %d\n", pVIM_CurAttr->AEMode.Exposuretime);
    VI_DEBUG("MaxET                                     = %d\n", pVIM_CurAttr->AEMode.MaxET);
    VI_DEBUG("DGain                                     = %d\n", pVIM_CurAttr->AEMode.DGain);
    VI_DEBUG("MaxDGain                                  = %d\n", pVIM_CurAttr->AEMode.MaxDGain);
    VI_DEBUG("DGainDeci                                 = %d\n", pVIM_CurAttr->AEMode.DGainDeci);
    VI_DEBUG("MaxFrmRate                                = %d\n", pVIM_CurAttr->MaxFrmRate);
    VI_DEBUG("FlipMirrorMode                            = %d\n", pVIM_CurAttr->FlipMirrorMode);
#endif
    VI_DEBUG("IrisMode = %x\n", pVIM_CurAttr->IrisMode);
     return status;
}

/***********************************************************
\brief 获取当前的曝光时间和增益
       响应cmd_server发送的获取曝光时间和增益的时候调用该函数 
\param pETGain: 曝光时间和增益结构指针

\return 0:成功 其它:失败
***********************************************************/
static int DRV_GetVIMETGain(pVF_AE_ETGain_S pETGain)
{
    int ret = 0;
    GLOBAL_GAIN_FORMAT_E gainFormat = Ten_Point_Six;
   
    ret = VIM_ISP_GetCurrentAEETGain(&pETGain->etus, &pETGain->gainValue, &gainFormat);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM GetETGain error = %d\n", ret);
    }
    VI_DEBUG("VIM Shutter = %d, VIM Gain = 0x%x %d.%d\n", pETGain->etus, pETGain->gainValue,  (pETGain->gainValue >> 6), (pETGain->gainValue & 0x3f));
    return ret;
}

/***********************************************************
\brief 设置相机的亮度、饱和度、对比度、锐度、色度
       
\param pBaseAttr:亮度、饱和度、对比度、锐度、色度的结构指针

\return 0:成功 其它:失败
***********************************************************/
static int DRV_SetVIMBaseAttr(pVF_BASE_ATTRIBUTE_S pBaseAttr)
{
    int ret = 0;
    //VI_DEBUG("BrightCoeff: %d\n", pBaseAttr->BrightnessCoeff);
    ret = VIM_ISP_SetBrightness(pBaseAttr->BrightnessCoeff);
    if(ret != VIM_SUCCEED)
    {
       OSA_ERROR("VIM SetBrightness error = %d\n", ret);
       return ret;
    }
    //VI_DEBUG("ContrastCoeff: %d\n", pBaseAttr->ContrastCoeff);
    ret = VIM_ISP_SetContrast(pBaseAttr->ContrastCoeff);
    if(ret != VIM_SUCCEED)
    {
       OSA_ERROR("VIM SetContrast error = %d\n", ret);
       return ret;
    }
    //VI_DEBUG("SaturationCoeff: %d\n", pBaseAttr->SaturationCoeff);
    ret = VIM_ISP_SetSaturation(pBaseAttr->SaturationCoeff);
    if(ret != VIM_SUCCEED)
    {
       OSA_ERROR("VIM SetSaturation error = %d\n", ret);
       return ret;
    }
    //VI_DEBUG("HueCoeff: %d\n", pBaseAttr->HueCoeff);
    ret = VIM_ISP_SetHue(pBaseAttr->HueCoeff);
    if(ret != VIM_SUCCEED)
    {
       OSA_ERROR("VIM SetHue error = %d\n", ret);
       return ret;
    }
    //VI_DEBUG("SharpnessCoeff: %d\n", pBaseAttr->SharpnessCoeff);
    ret = VIM_ISP_SetSharpness(pBaseAttr->SharpnessCoeff);
    if(ret != VIM_SUCCEED)
    {
       OSA_ERROR("VIM SetSharpness error = %d\n", ret);
       return ret;
    }
    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.BaseAttr), pBaseAttr, sizeof(VF_BASE_ATTRIBUTE_S));
    return ret;
}

static int DRV_SetVIMAEMode(pVF_AE_MODE_S pAEMode)
{
    int ret = OSA_SOK;
    AE_MODE_S VIMAEMode;

    if(pAEMode->AE_Shutter_Mode == VF_AE_ROI)
        VIMAEMode.AE_Shutter_Mode = (AE_SHUTTER_MODE_E)AE_MANUAL_VIM;
    else
        VIMAEMode.AE_Shutter_Mode = (AE_SHUTTER_MODE_E)pAEMode->AE_Shutter_Mode;
    VIMAEMode.AE_MaxET_Mode = (AE_MAXET_MODE_E)pAEMode->AE_MaxET_Mode;
    VIMAEMode.Exposuretime = pAEMode->Exposuretime;
    VIMAEMode.MaxET = pAEMode->MaxET;
    VIMAEMode.DGain = pAEMode->Gain;
    VIMAEMode.MaxDGain = pAEMode->MaxGain;
    VIMAEMode.DGainDeci = pAEMode->GainDeci;
    VI_DEBUG("AE_Shutter_Mode= %d\n", VIMAEMode.AE_Shutter_Mode);
    //VI_DEBUG("AE_MaxET_Mode = %d\n", VIMAEMode.AE_MaxET_Mode);
    VI_DEBUG("Exposuretime = %d\n", VIMAEMode.Exposuretime);
    VI_DEBUG("DGain = %d\n", VIMAEMode.DGain);
    //VI_DEBUG("MaxDGain = %d\n", VIMAEMode.MaxDGain);
    VI_DEBUG("DGainDeci = %d\n", VIMAEMode.DGainDeci);
    ret = VIM_ISP_SetAEMode(VIMAEMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AEMode error = %d\n", ret);
        return ret;
    }
    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AEMode), pAEMode, sizeof(VF_AE_MODE_S) );
    return ret;
}

static int DRV_SetVIMROI(pVF_AE_ROI_S pROI)
{
    int ret = OSA_SOK;
    int num = 0;
    unsigned short data = 0;



    num = pROI->ROI_No;

    VI_DEBUG("AERoi[%02d].ROI_No = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].ROI_No);
    VI_DEBUG("AERoi[%02d].onoff  = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].onoff);
    VI_DEBUG("AERoi[%02d].x      = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].x);
    VI_DEBUG("AERoi[%02d].y      = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].y);
    VI_DEBUG("AERoi[%02d].width  = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].width);
    VI_DEBUG("AERoi[%02d].height = %08x\n", num, gACS1910_current_cfg.ISPAllCfg.AERoi[num].height);
    VI_DEBUG("pROI->ROI_No       = %08x\n", pROI->ROI_No);
    VI_DEBUG("pROI->onoff        = %08x\n", pROI->onoff);
    VI_DEBUG("pROI->x            = %08x\n", pROI->x);
    VI_DEBUG("pROI->y            = %08x\n", pROI->y);
    VI_DEBUG("pROI->width        = %08x\n", pROI->width);
    VI_DEBUG("pROI->height       = %08x\n", pROI->height);

//    DRV_FPGASPIWrite(127, 0);
//    DRV_FPGASPIWrite(128, 0);
//    DRV_FPGASPIWrite(129, 1919);
//    DRV_FPGASPIWrite(130, 1079);
//    DRV_FPGASPIRead(0x20, &data);
//    DRV_FPGASPIRead(0x21, &data);
//    DRV_FPGASPIRead(0x22, &data);
//    DRV_FPGASPIRead(0x23, &data);
//    DRV_FPGASPIRead(0x24, &data);
//      DRV_FPGASPIRead(0x20, &data);
//      DRV_FPGASPIRead(0x21, &data);
//      DRV_FPGASPIRead(0x22, &data);
//      DRV_FPGASPIRead(0x23, &data);
//      DRV_FPGASPIRead(0x24, &data);
    
      DRV_FPGASPIRead(ROI0_X_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI0_Y_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI0_WIDTH_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI0_HEIGHT_FPGA_REG_ADDR, &data);

      DRV_FPGASPIRead(ROI1_X_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI1_Y_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI1_WIDTH_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI1_HEIGHT_FPGA_REG_ADDR, &data);

      DRV_FPGASPIRead(ROI2_X_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI2_Y_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI2_WIDTH_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI2_HEIGHT_FPGA_REG_ADDR, &data);

    //DRV_FPGASPIRead(37,&data);
      DRV_FPGASPIRead(ROI0_HISTOGRAM_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI1_HISTOGRAM_FPGA_REG_ADDR, &data);
      DRV_FPGASPIRead(ROI2_HISTOGRAM_FPGA_REG_ADDR, &data);
        VI_DEBUG("sizeof(VF_AE_MODE_S) = %d\n", sizeof(VF_AE_MODE_S));
    //DRV_FPGASPIRead(120, &data);

#if 0
    switch(num)
    {
        case 0:
            if(DRV_FPGASPIWrite(ROI_0_X_FPGA_REG_ADDR, pROI->x) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[0].x = pROI->x;
                VI_DEBUG("AERoi[0].x = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[0].x);
                DRV_FPGASPIRead(ROI_0_X_FPGA_REG_ADDR, &data);
                VI_DEBUG("ROI_0_X_FPGA_REG_ADDR = %d\n", data);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_0_Y_FPGA_REG_ADDR, pROI->y) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[0].y = pROI->y;
                VI_DEBUG("AERoi[0].y = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[0].y);
                DRV_FPGASPIRead(ROI_0_Y_FPGA_REG_ADDR, &data);
                VI_DEBUG("ROI_0_Y_FPGA_REG_ADDR = %d\n", data);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_0_WIDTH_FPGA_REG_ADDR, pROI->width) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[0].width = pROI->width;
                VI_DEBUG("AERoi[0].width = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[0].width);
                DRV_FPGASPIRead(ROI_0_WIDTH_FPGA_REG_ADDR, &data);
                VI_DEBUG("ROI_0_WIDTH_FPGA_REG_ADDR = %d\n", data);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_0_HEIGHT_FPGA_REG_ADDR, pROI->height) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[0].height = pROI->height;
                VI_DEBUG("AERoi[0].height = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[0].height);
                DRV_FPGASPIRead(ROI_0_HEIGHT_FPGA_REG_ADDR, &data);
                VI_DEBUG("ROI_0_HEIGHT_FPGA_REG_ADDR = %d\n", data);
            }
            else 
                return OSA_EFAIL;
            gACS1910_current_cfg.ISPAllCfg.AERoi[0].onoff = pROI->onoff;
            break;
        case 1:
            if(DRV_FPGASPIWrite(ROI_1_X_FPGA_REG_ADDR, pROI->x) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[1].x = pROI->x;
                VI_DEBUG("AERoi[1].x = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[1].x);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_1_Y_FPGA_REG_ADDR, pROI->y) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[1].y = pROI->y;
                VI_DEBUG("AERoi[1].y = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[1].y);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_1_WIDTH_FPGA_REG_ADDR, pROI->width) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[1].width = pROI->width;
                VI_DEBUG("AERoi[1].width = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[1].width);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_1_HEIGHT_FPGA_REG_ADDR, pROI->height) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[1].height = pROI->height;
                VI_DEBUG("AERoi[1].height= %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[1].height);
            }
            else 
                return OSA_EFAIL;
            gACS1910_current_cfg.ISPAllCfg.AERoi[1].onoff = pROI->onoff;
            break;
        case 2:
            if(DRV_FPGASPIWrite(ROI_2_X_FPGA_REG_ADDR, pROI->x) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[2].x = pROI->x;
                VI_DEBUG("AERoi[2].x = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[2].x);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_2_Y_FPGA_REG_ADDR, pROI->y) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[2].y = pROI->y;
                VI_DEBUG("AERoi[2].y = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[2].y);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_2_WIDTH_FPGA_REG_ADDR, pROI->width) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[2].width = pROI->width;
                VI_DEBUG("AERoi[2].width = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[2].width);
            }
            else 
                return OSA_EFAIL;
            if(DRV_FPGASPIWrite(ROI_2_HEIGHT_FPGA_REG_ADDR, pROI->height) == OSA_SOK)
            {
                gACS1910_current_cfg.ISPAllCfg.AERoi[2].height = pROI->height;
                VI_DEBUG("AERoi[2].height = %d\n", gACS1910_current_cfg.ISPAllCfg.AERoi[2].height);
            }
            else 
                return OSA_EFAIL;
            gACS1910_current_cfg.ISPAllCfg.AERoi[2].onoff = pROI->onoff;
            break;
        default:
            break;
    }
    //gACS1910_current_cfg.ISPAllCfg.AERoi[num].ROI_No = pROI->ROI_No;
    //gACS1910_current_cfg.ISPAllCfg.AERoi[num].x = pROI->x;
    //gACS1910_current_cfg.ISPAllCfg.AERoi[num].y = pROI->y;
    //gACS1910_current_cfg.ISPAllCfg.AERoi[num].width = pROI->width;
    //gACS1910_current_cfg.ISPAllCfg.AERoi[num].height = pROI->height;
#endif

    return ret;
}

static int DRV_SetVIMFlipMirror(VF_FLIP_MIRROR_MODE_E FlipMirrorMode)
{
    int ret;

    ret = VIM_ISP_SetFlipMirror((FLIP_MIRROR_MODE_E)FlipMirrorMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set FlipMirror error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.FlipMirrorMode = FlipMirrorMode;
    VI_DEBUG("gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.FlipMirrorMode = %d\n ", gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.FlipMirrorMode);
    return ret;
}

static int DRV_SetVIMAWBMode(pVF_AWB_MODE_S pAWBMode)
{
    int ret = OSA_SOK;

    VI_DEBUG("Set VIM AWB Mode\n");
    ret = VIM_ISP_SetAWBMode((AWB_MODE_E)pAWBMode->Mode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AWBMode error = %d\n", ret);
        return ret;
    }
    
    ret = VIM_ISP_SetAWBAERelation(pAWBMode->value);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AWBAERelation error = %d\n", ret);
        return ret;
    }
 
    ret = VIM_ISP_SetAWBCalcInterval(pAWBMode->interval);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AWBCalcInterval error = %d\n", ret);
        return ret;
    }

    ret = VIM_ISP_SetAWBUpdateSpeed(pAWBMode->speed);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AWBUpdateSpeed error = %d\n", ret);
        return ret;
    }

    ret = VIM_ISP_SetAWBDelay(pAWBMode->Delay);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set AWBDelay error = %d\n", ret);
        return ret;
    }

    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AWBMode), pAWBMode, sizeof(VF_AWB_MODE_S) );
    VI_DEBUG("gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AWBMode.Mode = %d\n ", gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AWBMode.Mode);
    return ret;
}

static int DRV_SetVIMIris(VF_IRIS_MODE_E IrisMode)
{
    int ret;

    VI_DEBUG("IrisMode = %x\n", IrisMode);
    ret = VIM_ISP_SetIris((IRIS_MODE_E)IrisMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set FlipMirror error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.IrisMode = IrisMode;
    return ret;
}

static int DRV_SetVIMIRCut(pVF_IRCUT_MODE_S pIRCutMode)
{
    int ret;

    ret = VIM_ISP_SetIrCut((IRCUT_MODE_E)pIRCutMode->Mode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set IRCutMode error = %d\n", ret);
        return ret;
    }
    ret = VIM_ISP_SetIrCut(pIRCutMode->Th);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set IRCut Th error = %d\n", ret);
        return ret;
    }
    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.IRCutMode), pIRCutMode, sizeof(VF_IRCUT_MODE_S));
    return ret;
}

static int DRV_SetVIMColorBlack(VF_COLORBLACK_MODE_E colorblack)
{
    int ret;

    ret = VIM_ISP_SetColorBlack((COLORBLACK_MODE_E)colorblack);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set ColorBlack error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.ColorBlackMode = colorblack;
    return ret;
}

static int DRV_SetVIMDRMode(pVF_DR_MODE_S pDRMode)
{
    int ret;

    ret = VIM_ISP_SetDRMode((DR_MODE_E)pDRMode->Mode, (DR_LEVEL_E)pDRMode->Level);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set IRDRMode error = %d\n", ret);
        return ret;
    }
    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.DRMode), pDRMode, sizeof(VF_DR_MODE_S));
}

static int DRV_SetVIMDeNoiseMode(pVF_DENOISE_MODE_S pDeNoiseMode)
{
    int ret;

    ret = VIM_ISP_SetDenoiseMode((DENOISE_MODE_E)pDeNoiseMode->Mode, (NR3D_LEVEL_E)pDeNoiseMode->Level);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set DeNoiseMode error = %d\n", ret);
        return ret;
    }
    memcpy(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.DeNoiseMode), pDeNoiseMode, sizeof(VF_DENOISE_MODE_S));
    return ret;
}

static int DRV_SetVIMEISEnable(VF_EIS_FLAG_E flag)
{
    int ret;

    VI_DEBUG("Set VIM EIS Enable: %d\n", flag);
    ret = VIM_ISP_SetEISEnable((EIS_FLAG_E)flag);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set EISEnable error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.EISFlag = flag;
    return ret;
}

static int DRV_SetVIMDefogMode(VF_DEFOG_MODE_E DefogMode)
{
    int ret;

    VI_DEBUG("Set VIM Defog Mode: %d\n", DefogMode);
    ret = VIM_ISP_SetDefogMode((DEFOG_MODE_E)DefogMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM Set DefogMode error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.DefogMode = DefogMode;
    return ret;
}

static int DRV_SetVIMMaxFrmRate(VF_MAXFRMRATE_E fps_mode)
{
    int ret;

    VI_DEBUG("fps_mode = %d\n", fps_mode);
    ret = VIM_VOUT_SetMaxFrmRate(VIDEOMODE_SEP_16BIT, (MAXFRMRATE_E)fps_mode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("VIM SetMaxFrmRate error = %d\n", ret);
        return ret;
    }
    gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.MaxFrmRate = fps_mode;
    return ret;
}


static int DRV_InitVIM(ptACS1910ISPNormalCfg ptACS1910_isp_normal_cfg)
{                      
    int ret = OSA_SOK;
    FILE *fp;
     
    ret = DRV_UpdateVIMGenInfoFile();
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_UpdateVIMGenInfoFile");
        return ret;
    }

    
    ret = DRV_SetVIMAEMode(&ptACS1910_isp_normal_cfg->AEMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMAEMode error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMAWBMode(&ptACS1910_isp_normal_cfg->AWBMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMAWBMode error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMBaseAttr(&ptACS1910_isp_normal_cfg->BaseAttr);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMBaseAttr error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMFlipMirror(ptACS1910_isp_normal_cfg->FlipMirrorMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMFlipMirror error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMIris(ptACS1910_isp_normal_cfg->IrisMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMIris error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMIRCut(&ptACS1910_isp_normal_cfg->IRCutMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMIRcut error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMColorBlack(ptACS1910_isp_normal_cfg->ColorBlackMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMColorBlack error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMDRMode(&ptACS1910_isp_normal_cfg->DRMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMDRMode error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMDeNoiseMode(&ptACS1910_isp_normal_cfg->DeNoiseMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMDeNoiseMode error = %d\n", ret);
        return ret;
    }  
    ret = DRV_SetVIMEISEnable(ptACS1910_isp_normal_cfg->EISFlag);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMEISEnable error = %d\n", ret);
        return ret;
    }
    ret = DRV_SetVIMDefogMode(ptACS1910_isp_normal_cfg->DefogMode);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMDefogMode error = %d\n", ret);
        return ret;
    }
    VI_DEBUG("ptACS1910_isp_cfg->MaxFrmRate = %d\n", ptACS1910_isp_normal_cfg->MaxFrmRate);
    ret = DRV_SetVIMMaxFrmRate(ptACS1910_isp_normal_cfg->MaxFrmRate);
    if(ret != VIM_SUCCEED)
    {
        OSA_ERROR("DRV_SetVIMMaxFrmRate error = %d\n", ret);
        return ret;
    }
 
}

static int VIM_control_msg_init()
{
    int retVal = OSA_SOK;
    
    VI_DEBUG("Get vim_msg_msqid!\n");

    vim_cmd_msqid = msgget((key_t)VIM_CMD_MSG_KEY, IPC_CREAT|0666);

    if(vim_cmd_msqid == 0)
    {
        msgctl(vim_cmd_msqid, IPC_RMID, 0);
        vim_cmd_msqid = msgget((key_t)VIM_CMD_MSG_KEY, IPC_CREAT|0666);
    }
    if(vim_cmd_msqid < 0)
    {
        perror("Get vim_cmd_msqid error!\n");
        retVal = OSA_EFAIL;
    }
    VI_DEBUG("Get vim_cmd_msqid %d done!\n", vim_cmd_msqid);

    vim_ack_msqid = msgget((key_t)VIM_ACK_MSG_KEY, IPC_CREAT|0666);

    if(vim_ack_msqid == 0)
    {
        msgctl(vim_ack_msqid, IPC_RMID, 0);
        vim_cmd_msqid = msgget((key_t)VIM_ACK_MSG_KEY, IPC_CREAT|0666);
    }
    if(vim_ack_msqid < 0)
    {
        perror("Get vim_ack_msqid error!\n");
        retVal = OSA_EFAIL;
    }
    VI_DEBUG("Get vim_ack_msqid %d done!\n\n", vim_ack_msqid);

    return retVal;
}

void VIM_control_thread()
{
    unsigned char rcv_msg_data[MSG_BUF_SIZE];
    unsigned char snd_msg_data[MSG_BUF_SIZE]; 
    tCmdServerMsg vim_control_rcv_msg;
    tCmdServerMsg vim_control_snd_msg;
    tACS1910ISPNormalCfg acs1910_isp_normal_cfg;
    int ret, i;
    VF_AE_ETGain_S ETGain;
    VIM_ATTRIBUTE_S vim_attr;
    unsigned int roi_no;

    VI_DEBUG("Hello VIM control thread!\n");

    while(VIM_control_thread_run)
    {
        VI_DEBUG("wait msg from cmd server!\n");
        msgrcv(vim_cmd_msqid, &vim_control_rcv_msg, MSG_BUF_SIZE, 0, 0);
        VI_DEBUG("receive msg type: 0x%lx\n", vim_control_rcv_msg.msg_type);
        vim_control_snd_msg.msg_type = vim_control_rcv_msg.msg_type;
        switch(vim_control_rcv_msg.msg_type)
        {
            case IP_CMD_ISP_SET_EXPOSURE:
                DRV_SetVIMAEMode((pVF_AE_MODE_S)vim_control_rcv_msg.msg_data);                
                break;
            case IP_CMD_ISP_GET_ETGAIN:
                ret = DRV_GetVIMETGain((pVF_AE_ETGain_S)vim_control_snd_msg.msg_data);
                //VI_DEBUG("ret = %d, vim_control_snd_msg: Shutter = %d, Gain = %d\n", 
                //       ret, ((pVF_AE_ETGain_S)vim_control_snd_msg.msg_data)->etus, ((pVF_AE_ETGain_S)vim_control_snd_msg.msg_data)->gainValue);
                if(ret != VIM_SUCCEED)
                    vim_control_snd_msg.msg_type = IP_CMD_ISP_GET_ERROR;
                else 
                {
                    msgsnd(vim_ack_msqid, &vim_control_snd_msg, MSG_BUF_SIZE, 0);
                }
                break;
            case IP_CMD_ISP_SET_BASE_ATTR:
                DRV_SetVIMBaseAttr((pVF_BASE_ATTRIBUTE_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_AE_ROI:
                DRV_SetVIMROI((pVF_AE_ROI_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_AWB:
                DRV_SetVIMAWBMode((pVF_AWB_MODE_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_FLIP_MIRROR:
                DRV_SetVIMFlipMirror(*(VF_FLIP_MIRROR_MODE_E *)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_IRIS:
                DRV_SetVIMIris(*(VF_IRIS_MODE_E *)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_IRCUT:
                DRV_SetVIMIRCut((pVF_IRCUT_MODE_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_COLOR_BLACK:
                DRV_SetVIMColorBlack(*(VF_COLORBLACK_MODE_E *)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_DR_MODE:
                DRV_SetVIMDRMode((pVF_DR_MODE_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_DENOISE:
                DRV_SetVIMDeNoiseMode((pVF_DENOISE_MODE_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_EIS:
                DRV_SetVIMEISEnable(*(VF_EIS_FLAG_E *)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_DEFOG:
                DRV_SetVIMDefogMode(*(VF_DEFOG_MODE_E *)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_ISP_SET_SAVE_ATTR:
                VI_DEBUG("save current cfg to saved cfg\n");
                save_current_cfg();
                break;
            case IP_CMD_ISP_SET_LOAD_DEFAULT:
                VI_DEBUG("load default cfg to current\n");
                memcpy(&(gACS1910_current_cfg.ISPAllCfg), &(gACS1910_default_cfg.ISPAllCfg), sizeof(tACS1910ISPAllCfg));
                DRV_InitVIM(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg));
                break;
            case IP_CMD_ISP_SET_LOAD_SAVED:
                VI_DEBUG("load saved cfg to current\n");
                memcpy(&(gACS1910_current_cfg.ISPAllCfg), &(gACS1910_saved_cfg.ISPAllCfg), sizeof(tACS1910ISPAllCfg));
                DRV_InitVIM(&(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg));
                break;
            case IP_CMD_ISP_GET_AE_ROI:
                roi_no = *(unsigned int *)vim_control_rcv_msg.msg_data;
                VI_DEBUG("get roi %d\n", roi_no);
                if(roi_no > (ROI_NO - 1))
                {
                    VI_DEBUG("roi_no error\n");
                    vim_control_snd_msg.msg_type = IP_CMD_ISP_GET_ERROR;
                    *(unsigned int *)vim_control_snd_msg.msg_data = IP_CMD_DATA_ERROR; 
                    msgsnd(vim_ack_msqid, &vim_control_snd_msg, MSG_BUF_SIZE, 0);
                }
                else
                {
                    VI_DEBUG("get roi %d data\n", roi_no);
                    memcpy(vim_control_snd_msg.msg_data, &(gACS1910_current_cfg.ISPAllCfg.AERoi[roi_no]), sizeof(VF_AE_ROI_S));
                    //for(i = 0; i < sizeof(VF_AE_ROI_S); i++)
                    //    VI_DEBUG("vim_control_snd_msg.msg_data[%02d] = %02x\n", i, vim_control_snd_msg.msg_data[i]);
                    msgsnd(vim_ack_msqid, &vim_control_snd_msg, MSG_BUF_SIZE, 0);
                }
                break;
            case IP_CMD_ISP_GET_CURRENT_ISP_ATTR:
                VI_DEBUG("get current isp attr!\n");
                memcpy(&(acs1910_isp_normal_cfg), &(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg), sizeof(tACS1910ISPAllCfg));
                VI_DEBUG("acs1910_isp_normal_cfg.AEMode.AE_Shutter_Mode = %x\n", acs1910_isp_normal_cfg.AEMode.AE_Shutter_Mode);
                if((acs1910_isp_normal_cfg.AEMode.AE_Shutter_Mode != VF_AE_MANUAL) && (acs1910_isp_normal_cfg.AEMode.AE_Shutter_Mode != VF_AE_ROI))
                {
                    VI_DEBUG("it is auto mode, need to read shutter and gain on time \n");
                    DRV_GetVIMETGain(&ETGain);
                    acs1910_isp_normal_cfg.AEMode.Exposuretime = ETGain.etus;
                    acs1910_isp_normal_cfg.AEMode.Gain =(unsigned char)( ETGain.gainValue  >> 6);
                    //VI_DEBUG("acs1910_isp_normal_cfg.AEmode.Gain = %d\n", acs1910_isp_normal_cfg.AEMode.Gain);
                    acs1910_isp_normal_cfg.AEMode.GainDeci =(unsigned char)(((( ETGain.gainValue & 0x3f) + 5)/10));
                    //VI_DEBUG("acs1910_isp_normal_cfg.AEmode.GainDeci = %d\n", acs1910_isp_normal_cfg.AEMode.GainDeci);
                }
                else 
                {
                    VI_DEBUG("it is manual mode\n");
                    DRV_GetVIMETGain(&ETGain);
                }
                VI_DEBUG("acs1910_isp_normal_cfg.AEMode.Exposuretime = %d\n", acs1910_isp_normal_cfg.AEMode.Exposuretime); 
                memcpy(vim_control_snd_msg.msg_data, &(acs1910_isp_normal_cfg), sizeof(tACS1910ISPNormalCfg));
                msgsnd(vim_ack_msqid, &vim_control_snd_msg, MSG_BUF_SIZE, 0);
                //for(i = 0; i < sizeof(tACS1910ISPNormalCfg); i++)
                //    VI_DEBUG("vim_control_snd_msg.msg_data[%02d] = %02x\n", i, vim_control_snd_msg.msg_data[i]);
                break;
            case IP_CMD_SYS_GET_SYS_CFG:
                VI_DEBUG("get sys cfg!\n");
                memcpy(vim_control_snd_msg.msg_data, &(gACS1910_current_cfg.SYSCfg), sizeof(tACS1910SYSCfg));
                msgsnd(vim_ack_msqid, &vim_control_snd_msg, MSG_BUF_SIZE, 0);
                break;
            case IP_CMD_SYS_SET_TIME:
                VI_DEBUG("set time!\n");
                DRV_SetTime((pVF_TIME_S)vim_control_rcv_msg.msg_data);
                break;
            case IP_CMD_SYS_SET_CAMERA_ID:
                VI_DEBUG("set camera id!\n");
                DRV_SetCameraID((pVF_CAMERA_ID_S)vim_control_rcv_msg.msg_data);
                break;
            default:
                break;
        }
    }
}

void VIM_roi_autoexp_thread()
{

    VI_DEBUG("Hello roi autoexp thread!\n");

    while(VIM_roi_autoexp_thread_run)
    {
        if(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AEMode.AE_Shutter_Mode == VF_AE_ROI)
        {
            VI_DEBUG("ROI autoexp\n");
            if(gACS1910_current_cfg.ISPAllCfg.AERoi[0].onoff == 1)
            {

            }
            usleep(1000000);
        }
        else 
        {
            sleep(1);
        }
    }
}


int VIM_control_thread_init()
{
    int retVal = 0;
    VI_DEBUG("Initialize VIM control thread\n");
   
    if((retVal = pthread_create(&VIM_control_thread_id, NULL, VIM_control_thread, NULL)) != 0)
    {
        perror("Create VIM control thread\n");
    }
    else 
    {
        VIM_control_thread_run = 1;
        VI_DEBUG("Create VIM control thread done!\n\n");
    }
    
    return retVal;
}

int VIM_roi_autoexp_thread_init()
{
    int retVal = 0;
    VI_DEBUG("Initialize roi autoexp_thread\n");

    if((retVal = pthread_create(&VIM_roi_autoexp_thread_id, NULL, VIM_roi_autoexp_thread, NULL)) != 0)
    {
        perror("Create ROI autoexp thread\n");
    }
    else 
    {
        VIM_roi_autoexp_thread_run = 1;
        VI_DEBUG("Create roi autoexp thread done!\n\n");
    }
}


int DRV_imgsOpen(DRV_ImgsConfig *config)
{
  int status;
  Uint16 width, height;
    VIM_ATTRIBUTE_S VIM_CurAttr;
    VIM_GENERAL_INFO VIM_GenInfo;
    DRV_SPICfg fpga_spi_cfg;
    unsigned char *fpga_spi_dev = "/dev/spidev0.0";

  memset(&gDRV_imgsObj, 0, sizeof(gDRV_imgsObj));

  DRV_imgGetWidthHeight(config->sensorMode, &width, &height);

  width+=IMGS_H_PAD;
  height+=IMGS_V_PAD;
  
  DRV_imgsCalcFrameTime(config->fps, width, height, config->binEnable, config->flipH, config->flipV);

#if 1
//start VIM initial
    VIM_SystemInit();
    status = VIM_OpenChannel(CHANNEL_I2C, 100, "/dev/i2c-1");
    printf("VIM open channel return %d\n", status);
    if(status!=VIM_SUCCEED) {
        OSA_ERROR("DRV_i2cOpen()\n");
        return OSA_EFAIL;
    }
    
    DRV_InitVIM(&(gACS1910_saved_cfg.ISPAllCfg.ISPNormalCfg));
    //status = VIM_UpdateAttribute();
    //if(status!=VIM_SUCCEED){
    //    OSA_ERROR("VIM UpdateAtrribute!\n");
    //    printf("status = %d\n", status);
    //    return OSA_EFAIL;
    //}
    status = DRV_GetVIMAttr(&VIM_CurAttr);
    if(status!=VIM_SUCCEED){
        OSA_ERROR("VIM GetCurAtrribute!\n");
        printf("status = %d\n", status);
        return OSA_EFAIL;
    }

    
//end VIM initial
#endif

    status = VIM_control_msg_init();
    if(status < 0)
        return OSA_EFAIL;
    status = VIM_control_thread_init(); 
    if(status < 0)
        return OSA_EFAIL;
    status = VIM_roi_autoexp_thread_init();
    if(status < 0)
        return OSA_EFAIL;


    fpga_spi_cfg.mode = SPI_MODE_0;
    fpga_spi_cfg.bits = 16;
    fpga_spi_cfg.speed = 1000000;
    fpga_spi_cfg.delay = 0;
    status = DRV_SPIOpen(&gDRV_imgsObj.spiHndl, fpga_spi_dev, &fpga_spi_cfg);
    VI_DEBUG("gDRV_imgsObj.spiHndl.fd = %d\n", gDRV_imgsObj.spiHndl.fd);
    if(status < 0)
        return OSA_EFAIL;

    DRV_FPGASPIWrite(32, 1);
    DRV_FPGASPIWrite(ROI0_X_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[0].x);
    DRV_FPGASPIWrite(ROI0_Y_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[0].y);
    DRV_FPGASPIWrite(ROI0_WIDTH_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[0].width);
    DRV_FPGASPIWrite(ROI0_HEIGHT_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[0].height);
    DRV_FPGASPIWrite(ROI1_X_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[1].x);
    DRV_FPGASPIWrite(ROI1_Y_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[1].y);
    DRV_FPGASPIWrite(ROI1_WIDTH_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[1].width);
    DRV_FPGASPIWrite(ROI1_HEIGHT_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[1].height);
    DRV_FPGASPIWrite(ROI2_X_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[2].x);
    DRV_FPGASPIWrite(ROI2_Y_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[2].y);
    DRV_FPGASPIWrite(ROI2_WIDTH_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[2].width);
    DRV_FPGASPIWrite(ROI2_HEIGHT_FPGA_REG_ADDR, gACS1910_saved_cfg.ISPAllCfg.AERoi[2].height);

    return OSA_SOK;
}

int DRV_imgsClose()
{
  int status;

  if(VIM_control_thread_run == 1)
  {
    VIM_control_thread_run = 0;
    pthread_join(VIM_control_thread_id, NULL);
  }

  if(VIM_roi_autoexp_thread_run == 1)
  {
    VIM_roi_autoexp_thread_run = 0;
    pthread_join(VIM_roi_autoexp_thread_id, NULL);
  }

  if(vim_cmd_msqid > 0)
    msgctl(vim_cmd_msqid, IPC_RMID, 0);
  if(vim_ack_msqid > 0)
    msgctl(vim_ack_msqid, IPC_RMID, 0);

  if(gDRV_imgsObj.spiHndl.fd > 0)
      close(gDRV_imgsObj.spiHndl.fd);

  status = DRV_imgsEnable(FALSE);
  //status |= DRV_i2cClose(&gDRV_imgsObj.i2cHndl);

  return status;
}

char* DRV_imgsGetImagerName()
{
  return "VS-MC1280-2MP";
}

int DRV_imgsSpecificSetting(void)
{	
	return 0;     
}


int DRV_imgsSet50_60Hz(Bool is50Hz)
{
  int fps;

	int status,i;
	Uint16 regAddr[10];

	Uint8 regValueSend[10]; 

return 0;

	if( is50Hz )
	{
		i = 0;
		regAddr[i] = TIMING_CONTROL_HTS_HIGHBYTE; 
		regValueSend[i] = 0x0B; 		i++;
		regAddr[i] = TIMING_CONTROL_HTS_LOWBYTE; 
		regValueSend[i] = 0x58; 		i++;
	}else{
		i = 0;
		regAddr[i] = TIMING_CONTROL_HTS_HIGHBYTE; 
		regValueSend[i] = 0x09; 		i++;
		regAddr[i] = TIMING_CONTROL_HTS_LOWBYTE; 
		regValueSend[i] = 0x74; 		i++;
	}

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValueSend, i);	
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2cWrite16()\n");
		return status;
	} 
	return status;


//  if( gDRV_imgsObj.curFrameTime.fps==30
//    ||gDRV_imgsObj.curFrameTime.fps==25
//    ) {
//
//    if(is50Hz)
//      fps = 25;
//    else
//      fps = 30;
//
//    DRV_imgsSetFramerate(fps);
//  }
  return 0;
}

int DRV_imgsSetFramerate(int fps)
{

	return 0;

}

int DRV_imgsBinEnable(Bool enable)
{

		return 0;


}

int DRV_imgsBinMode(int binMode)
{

		return 0;


}


int DRV_imgsSetDgain(int dgain)
{
    return OSA_SOK;
}

int DRV_imgsSetAgain(int again, int setRegDirect)
{

  Uint16 regAddr[8];
  Uint8 regValueSend[8]; 
  Uint16 regValue;
  int status,i;
  
return 0;

  if(setRegDirect) {
    regValue = again;
  } else {
    regValue = DRV_imgsCalcAgain(again);
  }

  i = 0;

	//regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0x01; 		i++;
  
  regAddr[i] = AEC_AGC_ADJ_L; regValueSend[i] = (regValue)&0xff; 	i++;
  regAddr[i] = AEC_AGC_ADJ_H; regValueSend[i] = 0; 								i++;

  //regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0x11; 		i++;
  //regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0xA1; 		i++;

  status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValueSend, i);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  return status;


}

int DRV_imgsSetEshutter(Uint32 eshutterInUsec, int setRegDirect)
{

  int status,i;
  Uint16 regAddr[20];
  int regValue=0; 
  Uint8 regValueSend[20]; 
  int regValue2=0; 
  
return 0;
  
  if(setRegDirect) {
    regValue = eshutterInUsec;
  } else  {
    DRV_imgsCalcSW(eshutterInUsec);  
    regValue = gDRV_imgsObj.curFrameTime.SW;
    regValue2 = regValue/16;
    if( regValue2 > 0x44a )
    {
	regValue2 = regValue2 - 0x44a;
    }else{
	regValue2 = 0;
    }
  }
  i = 0;
  regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0x00; 		i++;

//  regAddr[i] = PLL_CTRL01; regValueSend[i] = gDRV_imgsObj.curFrameTime.div_base;	i++;
  regAddr[i] = AEC_PK_EXPO1; regValueSend[i] = (regValue>>16)&0xff;	i++;
  regAddr[i] = AEC_PK_EXPO2; regValueSend[i] = (regValue>>8)&0xff; 	i++;  
  regAddr[i] = AEC_PK_EXPO3; regValueSend[i] = (regValue)&0xff; 		i++;

  regAddr[i] = AEC_PK_VTS_H; regValueSend[i] = (regValue2>>8)&0xff; 		i++;
  regAddr[i] = AEC_PK_VTS_L; regValueSend[i] = (regValue2)&0xff; 		i++;


  regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0x10; 		i++;
  regAddr[i] = GROUP_ACCESS; regValueSend[i] = 0xA0; 		i++;


  status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValueSend, i);  
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  } 
  return status;


}

#if 1 //Gang: read AGain & exposure
//return value is 8 times the true analog gain
int DRV_imgsGetAgain(int *again)
{
#if 0
  Uint8 regAddr;
  Uint16 regValue;
  int status;

  regAddr = GAIN_G1;

  status = DRV_i2cRead16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cRead16()\n");
    return status;
  }

  *again = regValue & 0x3f;

  if (regValue & 0x40)
  {
    *again <<= 1;
  }

  return status;
#else
	return 0;
#endif
}

int DRV_imgsGetEshutter(Uint32 *eshutterInUsec)
{
#if 0

  int status;
  Uint8 regAddr;
  Uint16 regValue;

  regAddr  = SHUTTER_WIDTH_L;

  status = DRV_i2cRead16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  *eshutterInUsec = DRV_imgsInvertSW(regValue);

  return status;
#else
	return 0;
#endif
}
#endif

int DRV_imgsSetDcSub(Uint32 dcSub, int setRegDirect)
{

  return 0;
}

int DRV_imgsEnable(Bool enable)
{
	int status=OSA_SOK;
#if 0 	
  Uint8 regAddr;
  Uint16 regValue;
  

  DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;

  if(enable) {
    status = DRV_imgsSetRegs();
    if(status!=OSA_SOK) {
      OSA_ERROR("DRV_imgsSetRegs()\n");
      return status;
    }
    DRV_imgsSpecificSetting();
  }
  if( (pFrame->f_pclk == (96*IMGS_MHZ))&&( pFrame->row_bin > 0 || pFrame->col_bin > 0))
  {
	  regAddr  = OUTPUT_CTRL;
	  regValue = 0x1f8C | ((enable & 0x1) << 1);
  }else{
	  regAddr  = OUTPUT_CTRL;
	  regValue = 0x1f80 | ((enable & 0x1) << 1);
  }

  status = DRV_i2cWrite16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }
#else
//if(enable) {
//    status = DRV_imgsSetRegs(); 
//  }

#endif
  OSA_waitMsecs(10);
  return status;
}

DRV_ImgsModeConfig      *DRV_imgsGetModeConfig(int sensorMode)
{
  return &gDRV_imgsObj.curModeConfig;
}

DRV_ImgsIsifConfig      *DRV_imgsGetIsifConfig(int sensorMode)
{
  return &gDRV_imgsIsifConfig_Common;
}

DRV_ImgsIpipeConfig     *DRV_imgsGetIpipeConfig(int sensorMode, int vnfDemoCfg)
{
  if(vnfDemoCfg)
  	return &gDRV_imgsIpipeConfig_Vnfdemo;
  else
  	return &gDRV_imgsIpipeConfig_Common;
}

DRV_ImgsH3aConfig       *DRV_imgsGetH3aConfig(int sensorMode, int aewbVendor)
{
  if(aewbVendor==1) {
  	printf("APPRO Setting Applied\n");
  	return &gDRV_imgsH3aConfig_Appro;
  }
  else if(aewbVendor==2) {
  	printf("TI Setting Applied\n");
  	return &gDRV_imgsH3aConfig_TI;
  }
  else {
  	printf("Common Setting Applied\n");
  	return &gDRV_imgsH3aConfig_Common;
  }
}

DRV_ImgsLdcConfig       *DRV_imgsGetLdcConfig(int sensorMode, Uint16 ldcInFrameWidth, Uint16 ldcInFrameHeight)
{
  OSA_printf(" DRV LDC: GetConfig %04x %dx%d\n", sensorMode, ldcInFrameWidth, ldcInFrameHeight);

  sensorMode &= 0xFF;

  switch(sensorMode) {
#ifdef IMGS_OV10630_1MP
//    case DRV_IMGS_SENSOR_MODE_752x512:
    case DRV_IMGS_SENSOR_MODE_720x480:
//    case DRV_IMGS_SENSOR_MODE_1620x1080:			//D1
#else
//    case DRV_IMGS_SENSOR_MODE_752x512:
    case DRV_IMGS_SENSOR_MODE_720x480:
    case DRV_IMGS_SENSOR_MODE_1620x1080:			//D1
#endif
      if(ldcInFrameWidth==864)
        return &gDRV_imgsLdcConfig_736x480_0_VS;

      if(ldcInFrameWidth==352)
        return &gDRV_imgsLdcConfig_736x480_1_VS;

      if(ldcInFrameWidth==736)
        return &gDRV_imgsLdcConfig_736x480_0;
        
      if(ldcInFrameWidth==752)
        return &gDRV_imgsLdcConfig_736x480_0;

      if(ldcInFrameWidth==288)
        return &gDRV_imgsLdcConfig_736x480_1;

      if(ldcInFrameWidth==768)
        return &gDRV_imgsLdcConfig_768x512_0;

      if(ldcInFrameWidth==320)
        return &gDRV_imgsLdcConfig_768x512_1;

      if(ldcInFrameWidth==928)
        return &gDRV_imgsLdcConfig_768x512_0_VS;

      if(ldcInFrameWidth==384)
        return &gDRV_imgsLdcConfig_768x512_1_VS;

      break;

    case DRV_IMGS_SENSOR_MODE_1280x720:
    case DRV_IMGS_SENSOR_MODE_1920x1080:			//720P / 1080P

      if(ldcInFrameWidth==1280)
        return &gDRV_imgsLdcConfig_1280x736_0;

      if(ldcInFrameWidth==320)
        return &gDRV_imgsLdcConfig_1280x736_1;

      if(ldcInFrameWidth==640)
        return &gDRV_imgsLdcConfig_1280x736_2;

      if(ldcInFrameWidth==1536)
        return &gDRV_imgsLdcConfig_1280x736_0_VS;

      if(ldcInFrameWidth==384)
        return &gDRV_imgsLdcConfig_1280x736_1_VS;

      if(ldcInFrameWidth==768)
        return &gDRV_imgsLdcConfig_1280x736_2_VS;

      if(ldcInFrameWidth==1312)
        return &gDRV_imgsLdcConfig_1312x768_0;

      if(ldcInFrameWidth==352)
        return &gDRV_imgsLdcConfig_1312x768_1;

      if(ldcInFrameWidth==672)
        return &gDRV_imgsLdcConfig_1312x768_2;

      if(ldcInFrameWidth==1600)
        return &gDRV_imgsLdcConfig_1312x768_0_VS;

      if(ldcInFrameWidth==448)
        return &gDRV_imgsLdcConfig_1312x768_1_VS;

      if(ldcInFrameWidth==832)
        return &gDRV_imgsLdcConfig_1312x768_2_VS;

      break;

  }

  return NULL;
}

int DRV_imgsReset()
{
  Uint16 regAddr[200];
  Uint8 regValue[200];
  int status, i;

	//注释掉的是1280x720的，用了1280x800取代它

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i=0;  
	regAddr[i] = OV10630_REG_0103;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_301b;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_301c;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_301a;regValue[i] = 0xff;  i++;
	//regAddr[i] = OV10630_REG_300c;regValue[i] = 0x61;  i++;
	regAddr[i] = OV10630_REG_3021;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_3011;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_6900;regValue[i] = 0x0c;  i++;
	regAddr[i] = OV10630_REG_6901;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_3033;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_3503;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_302d;regValue[i] = 0x2f;  i++;
	regAddr[i] = OV10630_REG_3025;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_3003;regValue[i] = 0x20;  i++;//0x14
	regAddr[i] = OV10630_REG_3004;regValue[i] = 0x21;  i++;//0x13
	regAddr[i] = OV10630_REG_3005;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_3006;regValue[i] = 0x91;  i++;
	regAddr[i] = OV10630_REG_3600;regValue[i] = 0x74;  i++;
	regAddr[i] = OV10630_REG_3601;regValue[i] = 0x2b;  i++;
	regAddr[i] = OV10630_REG_3612;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3611;regValue[i] = 0x67;  i++;
	regAddr[i] = OV10630_REG_3633;regValue[i] = 0xba;  i++;
	regAddr[i] = OV10630_REG_3602;regValue[i] = 0x2f;  i++;
	regAddr[i] = OV10630_REG_3603;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3630;regValue[i] = 0xa8;  i++;
	regAddr[i] = OV10630_REG_3631;regValue[i] = 0x16;  i++;
	regAddr[i] = OV10630_REG_3714;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_371d;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4300;regValue[i] = 0x38;  i++;//output yuv:ox38   raw:0xf8
	regAddr[i] = OV10630_REG_3007;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_3024;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_3020;regValue[i] = 0x0b;  i++;
	regAddr[i] = OV10630_REG_3702;regValue[i] = 0x10;  i++;//0x0d
	regAddr[i] = OV10630_REG_3703;regValue[i] = 0x24;  i++;//0x20
	regAddr[i] = OV10630_REG_3704;regValue[i] = 0x19;  i++;//0x15
	regAddr[i] = OV10630_REG_3709;regValue[i] = 0x28;  i++;
	regAddr[i] = OV10630_REG_370d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3712;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3713;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_3715;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_381d;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_381c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3824;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_3815;regValue[i] = 0x8c;  i++;
	regAddr[i] = OV10630_REG_3804;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_3805;regValue[i] = 0x1f;  i++;
	regAddr[i] = OV10630_REG_3800;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3801;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3806;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_3807;regValue[i] = 0x29;  i++;//0x01
	regAddr[i] = OV10630_REG_3802;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3803;regValue[i] = 0x06;  i++;//0x2e
	regAddr[i] = OV10630_REG_3808;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_3809;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_380a;regValue[i] = 0x03;  i++;//1280x800
	regAddr[i] = OV10630_REG_380b;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_380c;regValue[i] = 0x07;  i++;//0x06
	regAddr[i] = OV10630_REG_380d;regValue[i] = 0x71;  i++;//0xf6
	regAddr[i] = OV10630_REG_380e;regValue[i] = 0x03;  i++;//0x02
	regAddr[i] = OV10630_REG_380f;regValue[i] = 0x48;  i++;//0xec
	regAddr[i] = OV10630_REG_3811;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_381f;regValue[i] = 0x0c;  i++;
	regAddr[i] = OV10630_REG_3621;regValue[i] = 0x63;  i++;
	regAddr[i] = OV10630_REG_5005;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_56d5;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56d6;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_56d7;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56d8;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56d9;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56da;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_56db;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56dc;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56e8;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56e9;regValue[i] = 0x7f;  i++;
	regAddr[i] = OV10630_REG_56ea;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56eb;regValue[i] = 0x7f;  i++;
	regAddr[i] = OV10630_REG_5100;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5101;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5102;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5103;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5104;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5105;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5106;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5107;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5108;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5109;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510b;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_510f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5110;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5111;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5112;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5113;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5114;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5115;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5116;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5117;regValue[i] = 0x80;  i++;

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i = 0;
	regAddr[i] = OV10630_REG_5118;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5119;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511b;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_511f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_56d0;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5006;regValue[i] = 0x24;  i++;
	regAddr[i] = OV10630_REG_5608;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_52d7;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_528d;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_5293;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_52d3;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_5288;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_5289;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_52c8;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_52c9;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_52cd;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_5381;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5382;regValue[i] = 0xff;  i++;
	
	regAddr[i] = OV10630_REG_5300;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_5301;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5302;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5303;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_5304;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5305;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_5306;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5307;regValue[i] = 0x36;  i++;
	regAddr[i] = OV10630_REG_5308;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5309;regValue[i] = 0xd9;  i++;
	regAddr[i] = OV10630_REG_530a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_530b;regValue[i] = 0x0f;  i++;
	regAddr[i] = OV10630_REG_530c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_530d;regValue[i] = 0x2c;  i++;
	regAddr[i] = OV10630_REG_530e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_530f;regValue[i] = 0x59;  i++;
	regAddr[i] = OV10630_REG_5310;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5311;regValue[i] = 0x7b;  i++;
	regAddr[i] = OV10630_REG_5312;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5313;regValue[i] = 0x22;  i++;
	regAddr[i] = OV10630_REG_5314;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5315;regValue[i] = 0xd5;  i++;
	regAddr[i] = OV10630_REG_5316;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5317;regValue[i] = 0x13;  i++;
	regAddr[i] = OV10630_REG_5318;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5319;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_531a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_531b;regValue[i] = 0x26;  i++;
	regAddr[i] = OV10630_REG_531c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_531d;regValue[i] = 0xdc;  i++;
	regAddr[i] = OV10630_REG_531e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_531f;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_5320;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5321;regValue[i] = 0x24;  i++;
	regAddr[i] = OV10630_REG_5322;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5323;regValue[i] = 0x56;  i++;
	regAddr[i] = OV10630_REG_5324;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5325;regValue[i] = 0x85;  i++;
	regAddr[i] = OV10630_REG_5326;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5327;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_5609;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_560a;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_560b;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_560c;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_560d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_560e;regValue[i] = 0xfa;  i++;
	regAddr[i] = OV10630_REG_560f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5610;regValue[i] = 0xfa;  i++;
	regAddr[i] = OV10630_REG_5611;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_5612;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5613;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_5614;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_5615;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_5616;regValue[i] = 0x2c;  i++;
	regAddr[i] = OV10630_REG_5617;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_5618;regValue[i] = 0x2c;  i++;
	regAddr[i] = OV10630_REG_563b;regValue[i] = 0x01;  i++;

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i = 0;

	regAddr[i] = OV10630_REG_563c;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_563d;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_563e;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_563f;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_5640;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_5641;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_5642;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_5643;regValue[i] = 0x09;  i++;
	regAddr[i] = OV10630_REG_5644;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_5645;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_5646;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_5647;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_5651;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5652;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_521a;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_521b;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_521c;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_521d;regValue[i] = 0x0a;  i++;
	regAddr[i] = OV10630_REG_521e;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_521f;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_5220;regValue[i] = 0x16;  i++;
	regAddr[i] = OV10630_REG_5223;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_5225;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_5227;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_5229;regValue[i] = 0x0c;  i++;
	regAddr[i] = OV10630_REG_522b;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_522d;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_522f;regValue[i] = 0x1e;  i++;
	regAddr[i] = OV10630_REG_5241;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_5242;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_5243;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_5244;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_5245;regValue[i] = 0x0a;  i++;
	regAddr[i] = OV10630_REG_5246;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_5247;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_5248;regValue[i] = 0x16;  i++;
	regAddr[i] = OV10630_REG_524a;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_524c;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_524e;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_5250;regValue[i] = 0x0c;  i++;
	regAddr[i] = OV10630_REG_5252;regValue[i] = 0x12;  i++;
	regAddr[i] = OV10630_REG_5254;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_5256;regValue[i] = 0x1e;  i++;
	regAddr[i] = OV10630_REG_4605;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_4606;regValue[i] = 0x07;  i++;
	regAddr[i] = OV10630_REG_4607;regValue[i] = 0x71;  i++;
	regAddr[i] = OV10630_REG_460a;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_460b;regValue[i] = 0x70;  i++;
	regAddr[i] = OV10630_REG_460c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4620;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_4700;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_4701;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4702;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4708;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4004;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_4005;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_4001;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_4050;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_4051;regValue[i] = 0x22;  i++;
	regAddr[i] = OV10630_REG_4057;regValue[i] = 0x9c;  i++;
	regAddr[i] = OV10630_REG_405a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4202;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_3023;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_0100;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_6f0e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6f0f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_460e;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_460f;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4610;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4611;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4612;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4613;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_4605;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_4608;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_4609;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_6804;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6805;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_6806;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5120;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3510;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_3504;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6800;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6f0d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_5000;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_5001;regValue[i] = 0xbf;  i++;
	regAddr[i] = OV10630_REG_5002;regValue[i] = 0xfe;  i++;
	regAddr[i] = OV10630_REG_503d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c450;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c452;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c453;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c454;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c455;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c456;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c457;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c458;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c459;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c45b;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c45c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c45d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c45e;regValue[i] = 0x00;  i++;

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i = 0;

	regAddr[i] = OV10630_REG_c45f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c460;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c461;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c462;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c464;regValue[i] = 0x88;  i++;
	regAddr[i] = OV10630_REG_c465;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c466;regValue[i] = 0x8a;  i++;
	regAddr[i] = OV10630_REG_c467;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c468;regValue[i] = 0x86;  i++;
	regAddr[i] = OV10630_REG_c469;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c46a;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_c46b;regValue[i] = 0x50;  i++;
	regAddr[i] = OV10630_REG_c46c;regValue[i] = 0x30;  i++;
	regAddr[i] = OV10630_REG_c46d;regValue[i] = 0x28;  i++;
	regAddr[i] = OV10630_REG_c46e;regValue[i] = 0x60;  i++;
	regAddr[i] = OV10630_REG_c46f;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_c47c;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c47d;regValue[i] = 0x38;  i++;
	regAddr[i] = OV10630_REG_c47e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c47f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c480;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c481;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_c482;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c483;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_c484;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c485;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_c486;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c487;regValue[i] = 0x18;  i++;
	regAddr[i] = OV10630_REG_c488;regValue[i] = 0x34;  i++;//0x2e
	regAddr[i] = OV10630_REG_c489;regValue[i] = 0x70;  i++;//0x80
	regAddr[i] = OV10630_REG_c48a;regValue[i] = 0x34;  i++;//0x2e
	regAddr[i] = OV10630_REG_c48b;regValue[i] = 0x70;  i++;//0x80
	regAddr[i] = OV10630_REG_c48c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c48d;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c48e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c48f;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c490;regValue[i] = 0x07;  i++;
	regAddr[i] = OV10630_REG_c492;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_c493;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_c498;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c499;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c49a;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c49b;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c49c;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c49d;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c49e;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c49f;regValue[i] = 0x60;  i++;
	regAddr[i] = OV10630_REG_c4a0;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c4a1;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4a2;regValue[i] = 0x06;  i++;
	regAddr[i] = OV10630_REG_c4a3;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4a4;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4a5;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_c4a6;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4a7;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_c4a8;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4a9;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c4aa;regValue[i] = 0x0d;  i++;
	regAddr[i] = OV10630_REG_c4ab;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4ac;regValue[i] = 0x0f;  i++;
	regAddr[i] = OV10630_REG_c4ad;regValue[i] = 0xc0;  i++;
	regAddr[i] = OV10630_REG_c4b4;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4b5;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4b6;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4b7;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4ba;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4bb;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4be;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c4bf;regValue[i] = 0x33;  i++;
	regAddr[i] = OV10630_REG_c4c8;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_c4c9;regValue[i] = 0xd0;  i++;
	regAddr[i] = OV10630_REG_c4ca;regValue[i] = 0x0e;  i++;
	regAddr[i] = OV10630_REG_c4cb;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4cc;regValue[i] = 0x10;  i++;//0x0e
	regAddr[i] = OV10630_REG_c4cd;regValue[i] = 0x18;  i++;//0x51
	regAddr[i] = OV10630_REG_c4ce;regValue[i] = 0x10;  i++;//0x0e
	regAddr[i] = OV10630_REG_c4cf;regValue[i] = 0x18;  i++;//0x51
	regAddr[i] = OV10630_REG_c4d0;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c4d1;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c4e0;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c4e1;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c4e2;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4e4;regValue[i] = 0x10;  i++;
	regAddr[i] = OV10630_REG_c4e5;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_c4e6;regValue[i] = 0x30;  i++;
	regAddr[i] = OV10630_REG_c4e7;regValue[i] = 0x40;  i++;
	regAddr[i] = OV10630_REG_c4e8;regValue[i] = 0x50;  i++;
	regAddr[i] = OV10630_REG_c4e9;regValue[i] = 0x60;  i++;
	regAddr[i] = OV10630_REG_c4ea;regValue[i] = 0x70;  i++;
	regAddr[i] = OV10630_REG_c4eb;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c4ec;regValue[i] = 0x90;  i++;
	regAddr[i] = OV10630_REG_c4ed;regValue[i] = 0xa0;  i++;
	regAddr[i] = OV10630_REG_c4ee;regValue[i] = 0xb0;  i++;
	regAddr[i] = OV10630_REG_c4ef;regValue[i] = 0xc0;  i++;
	regAddr[i] = OV10630_REG_c4f0;regValue[i] = 0xd0;  i++;
	regAddr[i] = OV10630_REG_c4f1;regValue[i] = 0xe0;  i++;
	regAddr[i] = OV10630_REG_c4f2;regValue[i] = 0xf0;  i++;
	regAddr[i] = OV10630_REG_c4f3;regValue[i] = 0x80;  i++;

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i = 0;

	regAddr[i] = OV10630_REG_c4f4;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4f5;regValue[i] = 0x20;  i++;
	regAddr[i] = OV10630_REG_c4f6;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c4f7;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4f8;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c4f9;regValue[i] = 0x0b;  i++;
	regAddr[i] = OV10630_REG_c4fa;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c4fb;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4fc;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4fd;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c4fe;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c4ff;regValue[i] = 0x02;  i++;
	regAddr[i] = OV10630_REG_c500;regValue[i] = 0x68;  i++;
	regAddr[i] = OV10630_REG_c501;regValue[i] = 0x74;  i++;
	regAddr[i] = OV10630_REG_c502;regValue[i] = 0x70;  i++;
	regAddr[i] = OV10630_REG_c503;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c504;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_c505;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c506;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_c507;regValue[i] = 0x80;  i++;
	regAddr[i] = OV10630_REG_c508;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c509;regValue[i] = 0xc0;  i++;
	regAddr[i] = OV10630_REG_c50a;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c50b;regValue[i] = 0xa0;  i++;
	regAddr[i] = OV10630_REG_c50c;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c50d;regValue[i] = 0x2c;  i++;
	regAddr[i] = OV10630_REG_c50e;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c50f;regValue[i] = 0x0a;  i++;
	regAddr[i] = OV10630_REG_c510;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c511;regValue[i] = 0x01;  i++;//0x00
	regAddr[i] = OV10630_REG_c512;regValue[i] = 0x01;  i++;//0xe5
	regAddr[i] = OV10630_REG_c513;regValue[i] = 0x80;  i++;//0x14
	regAddr[i] = OV10630_REG_c514;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_c515;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c518;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_c519;regValue[i] = 0x48;  i++;
	regAddr[i] = OV10630_REG_c51a;regValue[i] = 0x07;  i++;
	regAddr[i] = OV10630_REG_c51b;regValue[i] = 0x70;  i++;
	regAddr[i] = OV10630_REG_c2e0;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c2e1;regValue[i] = 0x51;  i++;
	regAddr[i] = OV10630_REG_c2e2;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c2e3;regValue[i] = 0xd6;  i++;
	regAddr[i] = OV10630_REG_c2e4;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c2e5;regValue[i] = 0x5e;  i++;
	regAddr[i] = OV10630_REG_c2e9;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c2ea;regValue[i] = 0x7a;  i++;
	regAddr[i] = OV10630_REG_c2eb;regValue[i] = 0x90;  i++;
	regAddr[i] = OV10630_REG_c2ed;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c2ee;regValue[i] = 0x7a;  i++;
	regAddr[i] = OV10630_REG_c2ef;regValue[i] = 0x64;  i++;
	regAddr[i] = OV10630_REG_c308;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c309;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c30a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c30c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c30d;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c30e;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c30f;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c310;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c311;regValue[i] = 0x60;  i++;
	regAddr[i] = OV10630_REG_c312;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_c313;regValue[i] = 0x08;  i++;
	regAddr[i] = OV10630_REG_c314;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c315;regValue[i] = 0x7f;  i++;
	regAddr[i] = OV10630_REG_c316;regValue[i] = 0xff;  i++;
	regAddr[i] = OV10630_REG_c317;regValue[i] = 0x0b;  i++;
	regAddr[i] = OV10630_REG_c318;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c319;regValue[i] = 0x0c;  i++;
	regAddr[i] = OV10630_REG_c31a;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c31b;regValue[i] = 0xe0;  i++;
	regAddr[i] = OV10630_REG_c31c;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_c31d;regValue[i] = 0x14;  i++;
	

	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  
	
 memset(regAddr, 0, 200*sizeof(Uint16));  //Richard Added to update AWB Matrix
 memset(regValue, 0, 200*sizeof(Uint8));
 i = 0;

 regAddr[i] = OV10630_REG_5580;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_5581;regValue[i] = 0x52;  i++;
 regAddr[i] = OV10630_REG_5582;regValue[i] = 0x11;  i++;
 regAddr[i] = OV10630_REG_5583;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_5584;regValue[i] = 0x25;  i++;
 regAddr[i] = OV10630_REG_5585;regValue[i] = 0x24;  i++;
 regAddr[i] = OV10630_REG_5586;regValue[i] = 0x0d;  i++;
 regAddr[i] = OV10630_REG_5587;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_5588;regValue[i] = 0x0b;  i++;
 regAddr[i] = OV10630_REG_5589;regValue[i] = 0x74;  i++;
 regAddr[i] = OV10630_REG_558a;regValue[i] = 0x63;  i++;
 regAddr[i] = OV10630_REG_558b;regValue[i] = 0xa5;  i++;
 regAddr[i] = OV10630_REG_558c;regValue[i] = 0x96;  i++;
 regAddr[i] = OV10630_REG_558d;regValue[i] = 0x4f;  i++;
 regAddr[i] = OV10630_REG_558e;regValue[i] = 0x31;  i++;
 regAddr[i] = OV10630_REG_558f;regValue[i] = 0x65;  i++;
 regAddr[i] = OV10630_REG_5590;regValue[i] = 0x51;  i++;
 regAddr[i] = OV10630_REG_5591;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_5592;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_5593;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_5594;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_5595;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_5596;regValue[i] = 0x03;  i++;
 regAddr[i] = OV10630_REG_5597;regValue[i] = 0x02;  i++;
 regAddr[i] = OV10630_REG_559e;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_559f;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_55a0;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_55a1;regValue[i] = 0x10;  i++;
 regAddr[i] = OV10630_REG_55a2;regValue[i] = 0x76;  i++;
 regAddr[i] = OV10630_REG_55a3;regValue[i] = 0x61;  i++;
 regAddr[i] = OV10630_REG_55a4;regValue[i] = 0xb2;  i++;
 regAddr[i] = OV10630_REG_55a5;regValue[i] = 0xb9;  i++;
 regAddr[i] = OV10630_REG_55a6;regValue[i] = 0x38;  i++;
 regAddr[i] = OV10630_REG_55a7;regValue[i] = 0x36;  i++;
 regAddr[i] = OV10630_REG_55a8;regValue[i] = 0x64;  i++;
 regAddr[i] = OV10630_REG_55a9;regValue[i] = 0x4e;  i++;
 regAddr[i] = OV10630_REG_55aa;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_55ab;regValue[i] = 0x04;  i++;
 regAddr[i] = OV10630_REG_55ac;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_55ad;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_55ae;regValue[i] = 0xf0;  i++;
 regAddr[i] = OV10630_REG_c4b8;regValue[i] = 0x01;  i++;
 regAddr[i] = OV10630_REG_c4b9;regValue[i] = 0x01;  i++;
 regAddr[i] = OV10630_REG_c31e;regValue[i] = 0x00;  i++; //Richard: starting color matrix
 regAddr[i] = OV10630_REG_c31f;regValue[i] = 0xde;  i++;
 regAddr[i] = OV10630_REG_c320;regValue[i] = 0xFF;  i++;
 regAddr[i] = OV10630_REG_c321;regValue[i] = 0x39;  i++;
 regAddr[i] = OV10630_REG_c322;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c323;regValue[i] = 0xea;  i++;
 regAddr[i] = OV10630_REG_c324;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c325;regValue[i] = 0xe3;  i++;
 regAddr[i] = OV10630_REG_c326;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c327;regValue[i] = 0x52;  i++;
 regAddr[i] = OV10630_REG_c328;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c329;regValue[i] = 0xcb;  i++;
 regAddr[i] = OV10630_REG_c32a;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c32b;regValue[i] = 0xe4;  i++;
 regAddr[i] = OV10630_REG_c32c;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c32d;regValue[i] = 0xab;  i++;
 regAddr[i] = OV10630_REG_c32e;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c32f;regValue[i] = 0x71;  i++;
 regAddr[i] = OV10630_REG_c330;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c331;regValue[i] = 0xf9;  i++;
 regAddr[i] = OV10630_REG_c332;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c333;regValue[i] = 0xd9;  i++;
 regAddr[i] = OV10630_REG_c334;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c335;regValue[i] = 0x2e;  i++;
 regAddr[i] = OV10630_REG_c336;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c337;regValue[i] = 0xc8;  i++;
 regAddr[i] = OV10630_REG_c338;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c339;regValue[i] = 0x4a;  i++;
 regAddr[i] = OV10630_REG_c33a;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c33b;regValue[i] = 0xed;  i++;
 regAddr[i] = OV10630_REG_c33c;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c33d;regValue[i] = 0xe5;  i++;
 regAddr[i] = OV10630_REG_c33e;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c33f;regValue[i] = 0x53;  i++;
 regAddr[i] = OV10630_REG_c340;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c341;regValue[i] = 0xc7;  i++;
 regAddr[i] = OV10630_REG_c342;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c343;regValue[i] = 0xe8;  i++;
 regAddr[i] = OV10630_REG_c344;regValue[i] = 0xff;  i++;
 regAddr[i] = OV10630_REG_c345;regValue[i] = 0xa2;  i++;
 regAddr[i] = OV10630_REG_c346;regValue[i] = 0x00;  i++;
 regAddr[i] = OV10630_REG_c347;regValue[i] = 0x76;  i++;
 
 
 status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
 if(status!=OSA_SOK) {
  OSA_ERROR("DRV_i2c16Write16()\n");
  printf("10630 set error\n");
  return status;
 } //end awb & color modification

	memset(regAddr, 0, 200*sizeof(Uint16));
	memset(regValue, 0, 200*sizeof(Uint8));
	i = 0;

	
	regAddr[i] = OV10630_REG_c348;regValue[i] = 0x01;  i++;
	regAddr[i] = OV10630_REG_c349;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6700;regValue[i] = 0x04;  i++;
	regAddr[i] = OV10630_REG_6701;regValue[i] = 0x7b;  i++;
	regAddr[i] = OV10630_REG_6702;regValue[i] = 0xfd;  i++;
	regAddr[i] = OV10630_REG_6703;regValue[i] = 0xf9;  i++;
	regAddr[i] = OV10630_REG_6704;regValue[i] = 0x3d;  i++;
	regAddr[i] = OV10630_REG_6705;regValue[i] = 0x71;  i++;
	regAddr[i] = OV10630_REG_6706;regValue[i] = 0x71;  i++;
	regAddr[i] = OV10630_REG_6708;regValue[i] = 0x05;  i++;
	regAddr[i] = OV10630_REG_3822;regValue[i] = 0x50;  i++;
	regAddr[i] = OV10630_REG_6f06;regValue[i] = 0x6f;  i++;
	regAddr[i] = OV10630_REG_6f07;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6f0a;regValue[i] = 0x6f;  i++;
	regAddr[i] = OV10630_REG_6f0b;regValue[i] = 0x00;  i++;
	regAddr[i] = OV10630_REG_6f00;regValue[i] = 0x03;  i++;
	regAddr[i] = OV10630_REG_3042;regValue[i] = 0xf0;  i++;
	regAddr[i] = OV10630_REG_301b;regValue[i] = 0xf0;  i++;
	regAddr[i] = OV10630_REG_301c;regValue[i] = 0xf0;  i++;
	regAddr[i] = OV10630_REG_301a;regValue[i] = 0xf0;  i++;	
	
	status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);  
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		printf("10630 set error\n");
		return status;
	}  

	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! all 10630 set ok!\n");
return OSA_SOK;



	if(status==OSA_SOK) {

		i=0;
			 
		/*
			0x00 : 30fps, 
			0x10 : 15fps, 
			0x20 : 10fps
		*/
		regAddr[i]	= PLL_CTRL01;		regValue[i] = 0x00;	i++;

		status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
	}

	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2cWrite16()\n");
		return status;
	}

  return status;
}

int DRV_imgsCheckId()
{
  	int status;
  	Uint16  regAddr;
  	Uint8 regValueH;  
  	Uint8 regValueL;  
  	Uint16 regValue;  
  

  	regAddr = CHIP_VERSION_H;
  	regValueH = 0;



  	status = DRV_i2c16Read8(&gDRV_imgsObj.i2cHndl, &regAddr, &regValueH, 1);
  	if(status!=OSA_SOK) {
    	OSA_ERROR("DRV_i2cRead16()\n");
    	return status;  
      	}
	printf(" register addr:%x, and its value is %x\n", regAddr, regValueH);

   regAddr = CHIP_VERSION_L;
  	regValueL = 0;
  
  	status = DRV_i2c16Read8(&gDRV_imgsObj.i2cHndl, &regAddr, &regValueL, 1);
  	if(status!=OSA_SOK) 
	{
    	OSA_ERROR("DRV_i2cRead16()\n");
    	return status;  
  	}

	printf(" register addr:%x, and its value is %x\n", regAddr, regValueL);  

  regValue = (regValueH<<8)|regValueL;
    printf(" chip id is 0x%04x\n", regValue);

/* all registers' readout value are always zero, why????*/
  if(regValue!=IMGS_CHIP_ID)
  {
      printf("chip id is not right!\n");
    return OSA_EFAIL;  
  }
  return OSA_SOK;
}

int DRV_imgsSetRegs()
{
	Uint16 regAddr;
	Uint8 regValue;
    int i;
    int status;

#if 0	
	for(i = 0; i < REG_BUF_MAX_LEN; i = i + 2)
	{
		regAddr = gOv10635_reg_buf[i];
		regValue = (Uint8)(gOv10635_reg_buf[i + 1]);
		status = DRV_i2c16Write8(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  		if(status!=OSA_SOK) {
    			OSA_ERROR("DRV_i2c16Write8\n");
    			return status;
  		}
		readregValue[0] = 0;
		status = DRV_i2cRead8Addr16(&gDRV_imgsObj.i2cHndl, regAddr, readregValue, 1);
		OSA_printf("\r\nregAddr %x, regValue %x, readregValue %x, i %d\r\n", regAddr[0], regValue[0], readregValue[0], i);

		if((readregValue[0] != regValue[0]) && (i != 0))
		{
			i = i - 2;
			OSA_waitMsecs(10);
		}
	}
#endif	
	
	OSA_printf("\r ImgsSetRegs OK ..............\n");
	return status;
}

int DRV_imgsAPChipInit(int *pGioC1, int *pGioD1)
{
  #define PINMUX_BASE		(0x01C40000)
  #define PINMUX_LENGTH	(0x8C)
  #define PINMUX0_OFSET	(0x0000)
  #define PINMUX1_OFSET	(0x0004)
  #define PINMUX2_OFSET	(0x0008)
  #define PINMUX3_OFSET	(0x000C)
  #define PINMUX4_OFSET	(0x0010)
  #define IMGS_APCHIP_C1_GIO	(93)
  #define AP_CHIP_C1_PINMUX	(PINMUX_BASE+PINMUX0_OFSET)
  #define AP_CHIP_C1_MASK	(((unsigned int)0x3)<<14)
  #define AP_CHIP_C1_VAL 		(((unsigned int)0x1)<<14)	
  #define IMGS_APCHIP_D1_GIO	(90)
  #define AP_CHIP_D1_PINMUX	(PINMUX_BASE+PINMUX1_OFSET)
  #define AP_CHIP_D1_MASK	(((unsigned int)0x3)<<4)
  #define AP_CHIP_D1_VAL 		(((unsigned int)0x0)<<4)

	unsigned int *pPINMUXC1 = NULL;
	unsigned int *pPINMUXD1 = NULL;
	unsigned int val1, val2;
	unsigned char *pRegStart; 
	int ret = 0;

	if(gCSL_vpssHndl.regBaseVirtAddr==0 || pGioC1==0 || pGioD1==0){
		return -1;
	}
	pRegStart = (unsigned char *)gCSL_vpssHndl.regBaseVirtAddr;
	
	pPINMUXC1	= (unsigned int *)( pRegStart+ (AP_CHIP_C1_PINMUX-CSL_SYS_BASE_PHYS_ADDR));
	pPINMUXD1	= (unsigned int *)( pRegStart+ (AP_CHIP_D1_PINMUX-CSL_SYS_BASE_PHYS_ADDR));
	/* Enable C1 */
	*pPINMUXC1 = ((*pPINMUXC1 & (~AP_CHIP_C1_MASK)) | AP_CHIP_C1_VAL);
	val1 = (*pPINMUXC1 & AP_CHIP_C1_MASK);
	if(AP_CHIP_C1_VAL!= val1){
		OSA_ERROR("PINMUXC1 setting error(%X)!!!\n",val1);
		ret = -1;
		goto __DRV_imgs_APChipInit_End__;
	}
	/* Enable D1 */
	*pPINMUXD1 = (*pPINMUXD1 & (~AP_CHIP_D1_MASK)) | (AP_CHIP_D1_VAL);
	val2 = (*pPINMUXD1 & AP_CHIP_D1_MASK);
	if(AP_CHIP_D1_VAL != val2){
		OSA_ERROR("PINMUXD1 setting error(%X)!!!\n",val2);
		ret = -1;
		goto __DRV_imgs_APChipInit_End__;
	}

	*pGioC1 = IMGS_APCHIP_C1_GIO;
	*pGioD1 = IMGS_APCHIP_D1_GIO;

__DRV_imgs_APChipInit_End__:

	return ret;
}






