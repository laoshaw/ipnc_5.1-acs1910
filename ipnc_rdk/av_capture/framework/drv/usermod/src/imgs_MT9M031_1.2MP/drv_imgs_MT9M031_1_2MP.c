
#include "drv_imgs_MT9M031_1_2MP.h"
#include "drv_imgs_MT9M031_1_2MP_REG.h"
#include <drv_gpio.h>

DRV_ImgsObj gDRV_imgsObj;

//720P@60FPS
#define LINE_LENGTH (0x672)  // 0x0672 = 1650
#define ROW_TIME (LINE_LENGTH / OUT_CLK)  //22.22us

int DRV_imgsOpen(DRV_ImgsConfig *config)
{
  int status, retry=10;
  Uint16 width, height;

  memset(&gDRV_imgsObj, 0, sizeof(gDRV_imgsObj));

  DRV_imgGetWidthHeight(config->sensorMode, &width, &height);

  width+=IMGS_H_PAD;
  height+=IMGS_V_PAD;

  DRV_imgsCalcFrameTime(config->fps, width, height, config->binEnable);

  status = DRV_i2cOpen(&gDRV_imgsObj.i2cHndl, IMGS_I2C_ADDR);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cOpen()\n");
    return status;
  }

  #ifdef BOARD_AP_IPNC
  DRV_gpioSetMode(IMGS_RESET_GPIO, DRV_GPIO_DIR_OUT);
  DRV_gpioSet(IMGS_RESET_GPIO);
  DRV_gpioClr(IMGS_RESET_GPIO);
  OSA_waitMsecs(5);
  DRV_gpioSet(IMGS_RESET_GPIO);
  OSA_waitMsecs(5);
  #endif

  do {
    status = DRV_imgsCheckId();
    if(status==OSA_SOK)
      break;
    OSA_waitMsecs(5);
  } while(retry--);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_imgsCheckId()\n");
    DRV_i2cClose(&gDRV_imgsObj.i2cHndl);
    return status;
  }

  return 0;
}

int DRV_imgsClose()
{
  int status;

  status = DRV_imgsEnable(FALSE);
  status |= DRV_i2cClose(&gDRV_imgsObj.i2cHndl);

  return status;
}

char* DRV_imgsGetImagerName()
{
  return "MICRON_MT9M031_1_2MP";
}

int DRV_imgsSpecificSetting(void)
{

return 0;

}

int DRV_imgsSet50_60Hz(Bool is50Hz)
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
#if 0
int DRV_imgsSetAgain(int again, int setRegDirect)
{

  Uint16 regAddr[2];
  Uint16 regValue[2];
  int status=0;
  regAddr[0] = MT9M031_DIGITAL_TEST;
  regAddr[1] = MT9M031_DAC_LD_24_25;

  if (again >= 10000)
  {
    regValue[0] = 0x30;
    regValue[1] = 0xD308;
  }
  else if (again >= 8000)
  {
    regValue[0] = 0x30;
    regValue[1] = 0xD208;
  }
  else if (again >= 5000)
  {
      regValue[0] = 0x20;
      regValue[1] = 0xD308;
  }
  else if (again >= 4000)
  {
      regValue[0] = 0x20;
      regValue[1] = 0xD208;
  }
  else if (again >= 2500)
  {
      regValue[0] = 0x10;
      regValue[1] = 0xD308;
  }
  else if (again >= 2000)
  {
      regValue[0] = 0x10;
      regValue[1] = 0xD208;
  }
  else if (again >= 1250)
  {
      regValue[0] = 0x00;
      regValue[1] = 0xD308;
  }
  else
  {
      regValue[0] = 0x00;
      regValue[1] = 0xD208;
  }

  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }

  return status;
}
#else
int DRV_imgsSetAgain(int again, int setRegDirect)
{
    Uint16 regAddr[4];
    Uint16 regValue[4];
    int status=0;
    regAddr[0] = MT9M031_DIGITAL_TEST;
    regAddr[1] = MT9M031_DAC_LD_24_25;
    regAddr[2] = MT9M031_AE_CTRL_REG;
    regAddr[3] = MT9M031_GLOABL_GAIN;

    if (again >= 28800)
    {
        regValue[0] = 0x30;
        regValue[1] = 0xD308;
        regValue[2] = 4;
        regValue[3] = (int)(again/(28800.0/32));
    }
    else if (again >= 23040)
    {
        regValue[0] = 0x30;
        regValue[1] = 0xD208;
        regValue[2] = 4;
        regValue[3] = (int)(again/(23040.0/32));
    }
    else if (again >= 11520)
    {
        regValue[0] = 0x20;
        regValue[1] = 0xD208;
        regValue[2] = 4;
        regValue[3] = (int)(again/(11520.0/32));
    }
    else if (again >= 5760)
    {
        regValue[0] = 0x10;
        regValue[1] = 0xD208;
        regValue[2] = 4;
        regValue[3] = (int)(again/(5760.0/32));
    }
    else if (again >= 2880)
    {
        regValue[0] = 0x00;
        regValue[1] = 0xD208;
        regValue[2] = 4;
        regValue[3] = (int)(again/(2880.0/32));
    }
    else if (again >= 2000)
    {
        regValue[0] = 0x10;
        regValue[1] = 0xD208;
        regValue[2] = 0;
        regValue[3] = (int)(again/(2000.0/32));
    }
    else
    {
        regValue[0] = 0x00;
        regValue[1] = 0xD208;
        regValue[2] = 0;
        regValue[3] = (int)(again/(1000.0/32));
    }
    if (regValue[3] > 255)  regValue[3] = 255;
    //OSA_printf("------set a gain = %d, reg=%d, %d\n", again, regValue[0], (regValue[1]&0x0f00)>>8);
#if 0
    if(setRegDirect) {
        regValue = again;
    } else {
        regValue = DRV_imgsCalcAgain(again);
    }
#endif
#if 1
    status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, 4);

    if(status!=OSA_SOK) {
        OSA_ERROR("DRV_i2c16Write16()\n");
        return status;
    }
#endif

    return status;
}
#endif
int DRV_imgsSetDgain(int dgain)
{
	return 0;
}
int DRV_imgsSetEshutter(Uint32 eshutterInUsec, int setRegDirect)
{
  int status=0;
  Uint16 regAddr;
  Uint16 regValue;

  regAddr = MT9M031_COARSE_IT_TIME_A;
  printf("row time = %d, exposure time = %d\n", ROW_TIME, eshutterInUsec);//xiangdong
  regValue = eshutterInUsec/ROW_TIME;
  
  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }

  return status;
}
//Read AGain & exposure
//return value is 8 times the true analog gain
int DRV_imgsGetAgain(int *again)
{
return 0;
}

int DRV_imgsGetEshutter(Uint32 *eshutterInUsec)
{
return 0;
}

int DRV_imgsSetDcSub(Uint32 dcSub, int setRegDirect)
{

  return 0;
}

int DRV_imgsEnable(Bool enable)
{
  Uint8 regAddr;
  Uint16 regValue;
  int status=OSA_SOK;

  DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;

  if(enable) {
    status = DRV_imgsSetRegs();
    if(status!=OSA_SOK) {
      OSA_ERROR("DRV_imgsSetRegs()\n");
      return status;
    }
  }
  OSA_waitMsecs(1);

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
  	return &gDRV_imgsH3aConfig_Appro;
  }
  else if(aewbVendor==2) {
  	return &gDRV_imgsH3aConfig_TI;
  }
  else {
  	return &gDRV_imgsH3aConfig_Common;
  }
}

DRV_ImgsLdcConfig       *DRV_imgsGetLdcConfig(int sensorMode, Uint16 ldcInFrameWidth, Uint16 ldcInFrameHeight)
{
  sensorMode &= 0xFF;

  switch(sensorMode) {

    case DRV_IMGS_SENSOR_MODE_720x480:

      if(ldcInFrameWidth==864)
        return &gDRV_imgsLdcConfig_736x480_0_VS;

      if(ldcInFrameWidth==352)
        return &gDRV_imgsLdcConfig_736x480_1_VS;

      if(ldcInFrameWidth==736)
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
  return 0;
}

int DRV_imgsCheckId()
{
  int status;
  Uint16 regAddr;
  Uint16 regValue;

  regAddr = MT9M031_CHIP_VERSION;
  regValue = 0;
  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Read16()\n");
    return status;
  }
OSA_printf("READ MT9M031 CHIP ID = 0x%x",regValue);
  if(regValue!=IMGS_CHIP_ID)
    return OSA_EFAIL;

  return OSA_SOK;
}

int DRV_imgsSetRegs()
{

  int status,i=0;
  int j;
for(j=0; j < sizeof(MT9M031_Parallel)/sizeof(unsigned short); j=j+2)
{
   status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &MT9M031_Parallel[j], &MT9M031_Parallel[j+1], 1);  
	if(status != OSA_SOK)
	{
	   printf("I2C write Error,index:%d\n",j);
	   return status;
	}
if(MT9M031_Parallel[j] == 0x301A )	
   OSA_waitMsecs(200);
else
   OSA_waitMsecs(1);	
}
OSA_printf("Finished Parallel Mode Init with MT9M031 rev2,index j:%d\n",j);

  return status;
}


