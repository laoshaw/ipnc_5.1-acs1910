
#include "drv_imgs_AR0330_3.1MP.h"
#include <drv_gpio.h>
#include "drv_imgs_AR0330_REG.h"

DRV_ImgsObj gDRV_imgsObj;

 // for 1080p60, change PLL_P1 TO 1, OP_SYS_CLK_DIV to 1, the pixel clock will be 148.5Mhz
#define INPUT_CLK    24  //EXTCLK
#define PLL_M        74  //pll_multiplier
#define PLL_pre_div  4   //pre_pll_clk_div
#define PLL_P1       2   //vt_sys_clk_div
#define PLL_P2       6   //vt_pix_clk_div
#define OP_SYS_CLK_DIV 2 //OP_SYS_CLK_DIV

#define OUT_CLK ((INPUT_CLK * PLL_M) / (PLL_pre_div * PLL_P1 * PLL_P2) )//74.25MHz

#define LINE_LENGTH (0x3e8) //1000,1080p 
#define ROW_TIME    29.6297 //Xiangdong //27     // (2*LINE_LENGTH / OUT_CLK)//26.9us,two paths readout,
   

//#define AEWB_DEBUG

Uint16 gGain_VAL=0;

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
  OSA_waitMsecs(50);
  DRV_gpioSet(IMGS_RESET_GPIO);
  OSA_waitMsecs(50);
  #endif
  do {
    status = DRV_imgsCheckId();
    if(status==OSA_SOK)
      break;
    OSA_waitMsecs(10);
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
  return "Aptina_AR0330_3.1MP";
}

int DRV_imgsSpecificSetting(void)
{
	return 0;
}

int DRV_imgsSet50_60Hz(Bool is50Hz)
{
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

/*Current Gain setting,only for Context A*/
int DRV_imgsSetAgain(int again, int setRegDirect)
{
    Uint16 regAddr[2];
    Uint16 regValue[2];

    int status=0;
    regAddr[0] = AR0331_ANALOG_GAIN;
    regAddr[1] = AR0331_GLOBAL_GAIN;

    if (again > 127938) again = 127938;
    if (again < 1000) again = 1000;

    if (again >= 8000)
    {
        regValue[0] = 0x30;//x8
        regValue[1] = again*128/8000;
    }
    else if (again >= 5333)
    {
        regValue[0] = 0x28;//5.3
        regValue[1] = again*128/5333;
    }
    else if (again >= 4000)
    {
        regValue[0] = 0x20;//4
        regValue[1] = again*128/4000;
    }
    else if (again >= 2666)
    {
        regValue[0] = 0x18;//2.6
        regValue[1] = again*128/2666;
    }
    else if (again >= 2000)
    {
        regValue[0] = 0x10;//2
        regValue[1] = again*128/2000;
    }
    else if (again >= 1333)
    {
        regValue[0] = 0x08;//1.3
        regValue[1] = again*128/1333;
    }
    else
    {
        regValue[0] = 0x00;
        regValue[1] = again*128/1000;
    }
    //printf("analog:%d\n",regValue);
    status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, 2);  
    if(status!=OSA_SOK) {
        OSA_ERROR("I2C write error\n");
        return status;
    }
    OSA_waitMsecs(10);
    return 0;
}

int DRV_imgsSetEshutter(Uint32 eshutterInUsec, int setRegDirect)
{

  int status;
  Uint16 regAddr;
  Uint16 regValue;
  regAddr =AR0331_COARSE_IT_TIME_A;
  regValue = (float)(eshutterInUsec/ROW_TIME);
//  regValue = 0x134; // 8333/27
  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }
  
#ifdef AEWB_DEBUG
OSA_printf("eshutterInUsec:%d\n",eshutterInUsec);
#endif

  return status;
 
}
int AR0331_HDR_Enable(int enable)
{
   return 0;
}
//Read AGain & exposure
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
  int status=OSA_SOK;
  if(enable) {
    status = DRV_imgsSetRegs();
    if(status!=OSA_SOK) {
      OSA_ERROR("DRV_imgsSetRegs()\n");
      return status;
    }
  }
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
  Uint16  regAddr;
  Uint16 regValue;

  regAddr = CHIP_VERSION_ADDR;
  regValue = 0;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  printf("Read Sensor ID==AR0330:0x%4x\n",regValue);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cRead16()\n");
    return status;
  }
  if(regValue!=IMGS_CHIP_ID)
    return OSA_EFAIL;
  
  return OSA_SOK;
}

int DRV_imgsSetRegs()
 {
 
  int status,i=0;
  int j;
for(j=0; j < sizeof(AR0330_Parallel)/sizeof(unsigned short); j=j+2)
{
   status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &AR0330_Parallel[j], &AR0330_Parallel[j+1], 1);  
	if(status != OSA_SOK)
	{
	   printf("I2C write Error,index:%d\n",j);
	   return status;
	}
if(AR0330_Parallel[j] == 0x301A )	
   OSA_waitMsecs(200);
else
   OSA_waitMsecs(1);	
}
OSA_printf("Finished Parallel Mode Init with AR0330 rev2,index j:%d\n",j);
return 0;
}

