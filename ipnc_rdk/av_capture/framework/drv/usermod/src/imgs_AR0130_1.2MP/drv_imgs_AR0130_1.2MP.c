
#include "drv_imgs_AR0130_1.2MP.h"
#include <drv_gpio.h>

//#define FRAME_RATE_45FPS

#ifdef FRAME_RATE_45FPS
#define PLL_M        (74)  //pll_multiplier
#define ROW_TIME   ( 22.98)
#else
//#define PLL_M        (50)   //pll_multiplier
#define PLL_M        (37)   //pll_multiplier
//#define ROW_TIME    (32.64)
#define ROW_TIME    (33.62162)
#endif

//#define LINE_LENGTH (0x0672)
#define LINE_LENGTH (0x9b8)  // 0x07D0 =  2000    0x0672 = 1650(MIN)
#define INPUT_CLK    (24)  //EXTCLK

//#define PLL_pre_div  (2)   //pre_pll_clk_div
#define PLL_pre_div  (1)   //pre_pll_clk_div
#define PLL_P1       (2)   //vt_sys_clk_div
#define PLL_P2       (6)   //vt_pix_clk_div
#define OP_SYS_CLK_DIV (2) //OP_SYS_CLK_DIV


#define OUT_CLK ((INPUT_CLK * PLL_M) / (PLL_pre_div * PLL_P1 * PLL_P2) )//74.25MHz


DRV_ImgsObj gDRV_imgsObj;


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
  return "Aptina_AR0130_1.2MP";
}

int DRV_imgsSpecificSetting(void)
{
	return 0;
}

int DRV_imgsSet50_60Hz(Bool is50Hz)
{
 /*
 int fps;

  if( gDRV_imgsObj.curFrameTime.fps==30
    ||gDRV_imgsObj.curFrameTime.fps==25
    ) {

    if(is50Hz)
      fps = 25;
    else
      fps = 30;

    DRV_imgsSetFramerate(fps);
  }
  return 0;
*/
return 0;
}

int DRV_imgsSetFramerate(int fps)
{
/*
	Uint16 regAddr[4];
	Uint16 regValue[4];
	int i = 0;
	int j;
	int status = 0;

  switch(fps){
      case 30:
         i = 0;
		       regAddr[i] = AR0130_PLL_Multiplier; regValue[i] = 0x0031; i++;
		       regAddr[i] = AR0130_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++;
			regAddr[i] = AR0130_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++;
			regAddr[i] = AR0130_VT_PIX_CLK_DIV; regValue[i] = 0x0008; i++;
        break;
      case 24:
         i = 0;
		       regAddr[i] = AR0130_PLL_Multiplier; regValue[i] = 0x002D; i++;
		       regAddr[i] = AR0130_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++;
			regAddr[i] = AR0130_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++;
			regAddr[i] = AR0130_VT_PIX_CLK_DIV; regValue[i] = 0x000A; i++;
        break;
      case 15:
         i = 0;
		       regAddr[i] = AR0130_PLL_Multiplier; regValue[i] = 0x002D; i++;
		       regAddr[i] = AR0130_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++;
			regAddr[i] = AR0130_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++;
			regAddr[i] = AR0130_VT_PIX_CLK_DIV; regValue[i] = 0x0010; i++;
        break;
      case 8:
                 i = 0;
		       regAddr[i] = AR0130_PLL_Multiplier; regValue[i] = 0x002D; i++;
		       regAddr[i] = AR0130_Pre_PLL_CLK_DIV; regValue[i] = 0x0004; i++;
			regAddr[i] = AR0130_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++;
			regAddr[i] = AR0130_VT_PIX_CLK_DIV; regValue[i] = 0x000F; i++;
        break;
      default: 
         break;
       }
for(j = 0; j < i ; j++)
{
	status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		return status;
	}
	OSA_waitMsecs(1);
}
      OSA_waitMsecs(50);
    return status;
*/    
    return 0;
}

int DRV_imgsBinEnable(Bool enable)
{
/*
  Uint8 regAddr[4];
  Uint16 regValue[4];
  int i, col_bin, row_bin, status;

  if(!enable) {
    col_bin = 0;
    row_bin = 0;
  } else {
    col_bin = gDRV_imgsObj.curFrameTime.col_bin;
    row_bin = gDRV_imgsObj.curFrameTime.row_bin;
  }

  i=0;
  regAddr[i]  = ROW_ADDR_MODE;
  regValue[i] = (gDRV_imgsObj.curFrameTime.row_skip & 0x7 ) | ((row_bin & 0x3) << 4);
  i++;

  regAddr[i]  = COL_ADDR_MODE;
  regValue[i] = (gDRV_imgsObj.curFrameTime.col_skip & 0x7 ) | ((col_bin & 0x3) << 4);
  i++;

  status = DRV_i2cWrite16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  return status;
*/
return 0;
}

int DRV_imgsBinMode(int binMode)
{
/*
  Uint8 regAddr[4];
  Uint16 regValue[4];
  int i, status;

  i=0;
  regAddr[i]  = READ_MODE_2;
  regValue[i] = binMode;
  i++;


  status = DRV_i2cWrite16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  return status;
*/
return 0;
}
int DRV_imgsSetDgain(int dgain)
{
	  Uint16 regAddr[1];
	  Uint16 regValue[1];
	  int status=0;
	  regAddr[0] = AR0130_DGAIN_ADDR_A;
	  regValue[0] = dgain >> 5;
	  if (regValue[0] > 255)  regValue[0] = 255;

	  //OSA_printf("------set d gain = %d, reg=%d\n", dgain, regValue[0]);
	  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, 1);

	  if(status!=OSA_SOK) {
	    OSA_ERROR("DRV_i2c16Write16()\n");
	    return status;
	  }

	  return status;

}

int DRV_imgsSetAgain(int again, int setRegDirect)
{
    Uint16 regAddr[4];
    Uint16 regValue[4];
    int status=0;
    regAddr[0] = AR0130_FIRST_AGAIN_ADDR_A;
    regAddr[1] = AR0130_SECOND_AGAIN_ADDR_A;
    regAddr[2] = 0x3100;
    regAddr[3] = AR0130_DGAIN_ADDR_A;

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

    status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, 4);
    if(status!=OSA_SOK) {
        OSA_ERROR("DRV_i2c16Write16()\n");
        return status;
    }

    return status;
}


int DRV_imgsSetEshutter(Uint32 eshutterInUsec, int setRegDirect)
{

  int status;
  Uint16 regAddr;
  Uint16 regValue;
  regAddr =AR0130_COARSE_IT_TIME_A;
  regValue = (float)(eshutterInUsec/ROW_TIME);
//  regValue = 0x134; // 8333/27
  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }
// printf("eshutterInUsec:%d\n",eshutterInUsec);
  return status;

 
}
int AR0130_HDR_Enable(int enable)
{
   return 0;
}
//Read AGain & exposure
int DRV_imgsGetAgain(int *again)
{
/*
  Uint16 regAddr;
  Uint16 regValue;
  int status;

  regAddr = AR0130_GLOABL_GAIN;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Read16()\n");
    return status;
  }

  *again = regValue;
  return status;
 */
 return 0;
}

int DRV_imgsGetEshutter(Uint32 *eshutterInUsec)
{
/*
  int status;
  Uint16 regAddr;
  Uint16 regValue;

  regAddr = AR0130_COARSE_IT_TIME_A;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }

 *eshutterInUsec = regValue*ROW_TIME;

  return status;
  */
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
  printf("Read Sensor ID:0x%4x\n",regValue);
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
  Uint16  regAddr[500];
  Uint16 regValue[500];
  int status,i=0;
  int j;
  memset(regAddr, 0, 500*sizeof(Uint16));
  memset(regValue, 0,500*sizeof(Uint16));

  regAddr[i]  = 0x301A; regValue[i] = 0x0001 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x301A; regValue[i] = 0x10D8 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x3088; regValue[i] = 0x8000 	; i++ ; // SEQ_CTRL_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0225 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x5050 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2D26 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0828 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0D17 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0926 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0028 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0526 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0xA728 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0725 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x8080 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2917 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0525 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0040 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2702 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1616 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2706 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1736 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x26A6 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1703 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x26A4 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x171F 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2805 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2620 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2804 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2520 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2027 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0017 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1E25 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0020 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2117 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1028 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x051B 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1703 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2706 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1703 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1747 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2660 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x17AE 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2500 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x9027 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0026 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1828 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x002E 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2A28 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x081E 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0831 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1440 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x4014 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2020 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1410 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1034 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1014 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0020 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x4013 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1802 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1470 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x7004 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1470 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x7003 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1470 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x7017 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2002 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2002 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x5004 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2004 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x1400 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x5022 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0314 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0020 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0314 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x0050 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2C2C 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x3086; regValue[i] = 0x2C2C 	; i++ ; // SEQ_DATA_PORT
  regAddr[i]  = 0x309E; regValue[i] = 0x0000 	; i++ ; // RESERVED_MFR_309E
  regAddr[i]  = 0x30E4; regValue[i] = 0x6372 	; i++ ; // RESERVED_MFR_30E4
  regAddr[i]  = 0x30E2; regValue[i] = 0x7253 	; i++ ; // RESERVED_MFR_30E2
  regAddr[i]  = 0x30E0; regValue[i] = 0x5470 	; i++ ; // RESERVED_MFR_30E0
  regAddr[i]  = 0x30E6; regValue[i] = 0xC4CC 	; i++ ; // RESERVED_MFR_30E6
  regAddr[i]  = 0x30E8; regValue[i] = 0x8050 	; i++ ; // RESERVED_MFR_30E8
  regAddr[i]  = 0x3082; regValue[i] = 0x0029 	; i++ ; // OPERATION_MODE_CTRL
  regAddr[i]  = 0x301E; regValue[i] = 0x00C8 	; i++ ; // DATA_PEDESTAL
  regAddr[i]  = 0x3EDA; regValue[i] = 0x0F03 	; i++ ; // RESERVED_MFR_3EDA
  regAddr[i]  = 0x3EDE; regValue[i] = 0xC005 	; i++ ; // RESERVED_MFR_3EDE
  regAddr[i]  = 0x3ED8; regValue[i] = 0x09EF 	; i++ ; // RESERVED_MFR_3ED8
  regAddr[i]  = 0x3EE2; regValue[i] = 0xA46B 	; i++ ; // RESERVED_MFR_3EE2
  regAddr[i]  = 0x3EE0; regValue[i] = 0x047D 	; i++ ; // RESERVED_MFR_3EE0
  regAddr[i]  = 0x3EDC; regValue[i] = 0x0070 	; i++ ; // RESERVED_MFR_3EDC
  regAddr[i]  = 0x3044; regValue[i] = 0x0404 	; i++ ; // DARK_CONTROL
  regAddr[i]  = 0x3EE6; regValue[i] = 0x8303 	; i++ ; // RESERVED_MFR_3EE6
  regAddr[i]  = 0x3EE4; regValue[i] = 0xD208 	; i++ ; // DAC_LD_24_25
  regAddr[i]  = 0x3ED6; regValue[i] = 0x00BD 	; i++ ; // RESERVED_MFR_3ED6
  regAddr[i]  = 0x30B0; regValue[i] = 0x1300 	; i++ ; // DIGITAL_TEST
  regAddr[i]  = 0x30D4; regValue[i] = 0xE007 	; i++ ; // COLUMN_CORRECTION
  regAddr[i]  = 0x301A; regValue[i] = 0x10DC 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x301A; regValue[i] = 0x10D8 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x3044; regValue[i] = 0x0400 	; i++ ; // DARK_CONTROL
  regAddr[i]  = 0x3012; regValue[i] = 0x02A0 	; i++ ; // COARSE_INTEGRATION_TIME
  regAddr[i]  = 0x3032; regValue[i] = 0x0000 	; i++ ; // DIGITAL_BINNING
  regAddr[i]  = 0x3002; regValue[i] = 0x0002 	; i++ ; // Y_ADDR_START
  regAddr[i]  = 0x3004; regValue[i] = 0x0000 	; i++ ; // X_ADDR_START
  regAddr[i]  = 0x3006; regValue[i] = 0x03C1 	; i++ ; // Y_ADDR_END
  regAddr[i]  = 0x3008; regValue[i] = 0x04FF 	; i++ ; // X_ADDR_END
  regAddr[i]  = 0x300A; regValue[i] = 0x03DE 	; i++ ; // FRAME_LENGTH_LINES
  regAddr[i]  = 0x300C; regValue[i] = LINE_LENGTH 	; i++ ; // LINE_LENGTH_PCK
  regAddr[i]  = 0x301A; regValue[i] = 0x10D8 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x31D0; regValue[i] = 0x0001 	; i++ ; // HDR_COMP
  regAddr[i]  = 0x302C; regValue[i] = PLL_P1; 	; i++ ; // VT_SYS_CLK_DIV //0x0002
  regAddr[i]  = 0x302A; regValue[i] = PLL_P2; 	; i++ ; // VT_PIX_CLK_DIV //0x0004
  regAddr[i]  = 0x302E; regValue[i] = PLL_pre_div;	; i++ ; // PRE_PLL_CLK_DIV //0x0002
  regAddr[i]  = 0x3030; regValue[i] = PLL_M; 	; i++ ; // PLL_MULTIPLIER //0x002C
  regAddr[i]  = 0x30B0; regValue[i] = 0x1300 	; i++ ; // DIGITAL_TEST
  regAddr[i]  = 0x301A; regValue[i] = 0x10DC 	; i++ ; // RESET_REGISTER
  regAddr[i]  = 0x301A; regValue[i] = 0x10DC 	; i++ ; // RESET_REGISTER

for(j=0; j < i; j++)
{
   status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr[j], &regValue[j], 1);  
	if(status != OSA_SOK)
	{
	   printf("I2C write Error,index:%d\n",j);
	   return status;
	}
if(regAddr[j] == 0x301A )	
   OSA_waitMsecs(100);

   OSA_waitMsecs(1);	
}
printf("Finished Linear Mode Init with AR0130 rev2.\n");
return 0;
}

