
#include "drv_imgs_IMX104_1_3MP.h"
#include "imx104_reg_setting.h"
#include <drv_gpio.h>
#include <math.h>

#define __DRV_IMGS_IMX104_DEBUG__
#ifdef __DRV_IMGS_IMX104_DEBUG__
#define DB(fmt, args...) fprintf(stderr,fmt, ## args)
#else
#define DB(fmt, args...)
#endif
#define DBErr(fmt, args...) fprintf(stderr,fmt, ## args)

//#define AEWB_DEBUG

//#define IMX104_LVDS324  
//#define IMX104_SPI_TEST
#define IMX104_SPI_DRIVER_MODE 0

#if(IMX104_SPI_DRIVER_MODE==0)
//SPI0
#define SPI1_CS_GPO     (23)
#define SPI1_SCLK_GPO 	(24)
#define SPI1_SDO_GPO 	(22)
//#define SPI1_SDI_GPI  (37)   
#endif

#define PCLK 81000000
#define LINE_LENGTH 2400
#define FRAME_LENGTH 1125
#define ROW_TIME (LINE_LENGTH / PCLK)   //29.6us

DRV_ImgsObj gDRV_imgsObj;

Bool gbIsDDRIn = FALSE;
Bool gbIsAlawEnalbe = FALSE;
int gCurrsensorMode = DRV_IMGS_SENSOR_MODE_1920x1080;

int DRV_imgsGetDdrInMode(void)
{
    return (int) gbIsDDRIn;
}

void imx104_setgio(unsigned short addr, unsigned short reg)
{
    int i;
    unsigned char imx104_value;
    unsigned char spi_chipid;
    unsigned char page_addr;
    unsigned char regset[3];

    spi_chipid = (unsigned char) (addr >> 8);
    page_addr = (unsigned char) (addr & 0xFF);
    imx104_value = (unsigned char) reg;

    regset[0] = spi_chipid;
    regset[1] = page_addr;
    regset[2] = imx104_value;

#if(IMX104_SPI_DRIVER_MODE==0)

    // sclk low
    DRV_gpioClr(SPI1_SCLK_GPO);
    // CS low                      
    DRV_gpioClr(SPI1_CS_GPO);

/*spi_chipid*/
    for (i = 0; i < 8; i++)
    {
        if (spi_chipid & 0x01)
            DRV_gpioSet(SPI1_SDO_GPO);
        else
            DRV_gpioClr(SPI1_SDO_GPO);
        spi_chipid = spi_chipid >> 1;

        DRV_gpioSet(SPI1_SCLK_GPO);
        DRV_gpioClr(SPI1_SCLK_GPO);
    }
/*page address*/
    for (i = 0; i < 8; i++)
    {
        if (page_addr & 0x01)
            DRV_gpioSet(SPI1_SDO_GPO);
        else
            DRV_gpioClr(SPI1_SDO_GPO);
        page_addr = page_addr >> 1;

        DRV_gpioSet(SPI1_SCLK_GPO);
        DRV_gpioClr(SPI1_SCLK_GPO);
    }
/*data*/
    for (i = 0; i < 8; i++)
    {
        if (imx104_value & 0x01)
            DRV_gpioSet(SPI1_SDO_GPO);
        else
            DRV_gpioClr(SPI1_SDO_GPO);
        imx104_value = imx104_value >> 1;

        DRV_gpioSet(SPI1_SCLK_GPO);
        DRV_gpioClr(SPI1_SCLK_GPO);
    }

    // CS high                     
    DRV_gpioSet(SPI1_CS_GPO);
#else
    DRV_SPIWrite8(&gDRV_imgsObj.spiHndl, regset, 3);
#endif
}

int InitRegArrary()
{
    int i = 0;

#ifdef  IMX104_SPI_TEST
    do
    {
        imx104_setgio(IMX104_SETTING[0], 0x25);
    }
    while (1);
#endif

    OSA_printf("Current FRAME RATE SEL:%d\n", IMX104_LVDS_FPS);

    OSA_waitMsecs(200);
    imx104_setgio(0x0200, 0x01);        //standy  mode
    OSA_waitMsecs(200);

    for (i = 0; i < IMX104_REG_COUNT; i = i + 2)
    {

        imx104_setgio(IMX104_SETTING[i], IMX104_SETTING[i + 1]);

    }

/*these 3 regs need be changed for parallel CMOS SDR output from default lvds+324 setting*/
#ifndef IMX104_LVDS324
    imx104_setgio(0x0244, 1);
    imx104_setgio(0x0247, 3);
    imx104_setgio(0x0248, 0xc0);
#endif

    OSA_waitMsecs(200);
    imx104_setgio(0x0200, 0);   //cancel standy mode

    OSA_waitMsecs(200);
    imx104_setgio(0x0202, 0);   //start master mode

    return 0;

}

int DRV_imgsOpen(DRV_ImgsConfig * config)
{
    int status = 0;
    Uint16 width, height;
    int value32;

    memset(&gDRV_imgsObj, 0, sizeof(gDRV_imgsObj));

    DRV_imgGetWidthHeight(config->sensorMode, &width, &height);

    OSA_printf("sensor mode:%d;width:%d;height:%d;%s", config->sensorMode,
               width, height, __FUNCTION__);

    width += IMGS_H_PAD;
    height += IMGS_V_PAD;


#ifdef IMX104_LVDS324
    /*for lvds324 */
    status = DRV_i2cOpen(&gDRV_imgsObj.i2cHndl, IMGS_I2C_ADDR);
    if (status != OSA_SOK)
    {
        OSA_ERROR("DRV_i2cOpen()\n");
        return status;
    }
#endif

#if (0)
    DRV_imgsCalcFrameTime(config->fps, width, height, config->binEnable);
#else
    DRV_imgsCalcFrameTime(config->fps, width, height, config->binEnable);
    gDRV_imgsObj.curModeConfig.sensorDataWidth = width;
    gDRV_imgsObj.curModeConfig.sensorDataHeight = height - 10;  //That's the isif Vdint
    gDRV_imgsObj.curModeConfig.validStartX = IMGS_H_PAD / 2;
    gDRV_imgsObj.curModeConfig.validStartY = IMGS_V_PAD / 2;
    gDRV_imgsObj.curModeConfig.validWidth = width - IMGS_H_PAD;
    gDRV_imgsObj.curModeConfig.validHeight = height - IMGS_V_PAD;
    gDRV_imgsObj.curModeConfig.binEnable = 0;
#endif

    DB("############reset###########\n");
    /* Sensor reset */
    DRV_gpioSetMode(IMGS_RESET_GPIO, DRV_GPIO_DIR_OUT);
    DRV_gpioSet(IMGS_RESET_GPIO);
    DRV_gpioClr(IMGS_RESET_GPIO);
    OSA_waitMsecs(50);
    DRV_gpioSet(IMGS_RESET_GPIO);
    OSA_waitMsecs(50);
#if(IMX104_SPI_DRIVER_MODE==0)
//SPI0 GIO INILIZATION
    // init PINMUX3  GIO23 24 22
    status = CSL_gpioGetPinmux(&gCSL_gpioHndl, 3, &value32);
    if (status == OSA_SOK)
    {
        value32 &= 0xe1ffffff;
        //value32 &=~((3<<26)|(1<<28)|(1<<25));
        status = CSL_gpioSetPinmux(&gCSL_gpioHndl, 3, value32);
        OSA_printf("set pinmux3 = 0x%x\n", value32);
    }

    DRV_gpioSetMode(SPI1_CS_GPO, DRV_GPIO_DIR_OUT);
    DRV_gpioSetMode(SPI1_SCLK_GPO, DRV_GPIO_DIR_OUT);
    DRV_gpioSetMode(SPI1_SDO_GPO, DRV_GPIO_DIR_OUT);
#else

    status = DRV_SPIOpen(&gDRV_imgsObj.spiHndl, 0);     //devAddr unused in DRV_SPIOpen
    if (status != OSA_SOK)
    {
        OSA_ERROR("DRV_spiOpen()\n");
        return status;
    }
    DB("############status:%d###########\n", status);

#endif
    DB("############SET REG###########\n");
    status = DRV_imgsSetRegs(width, height, config->fps);
/*	
	gbIsDDRIn = config->bIsDdrIn;
	gbIsAlawEnalbe = config->bIsALawEnable;
*/
    return 0;
}

int DRV_imgsClose()
{
    int status;
    //DRV_imgs_DarkframeRelease();
    status = DRV_imgsEnable(FALSE);

    return status;
}

char *DRV_imgsGetImagerName()
{
    return "SONY_IMX104_1_3MP";
}

int DRV_imgsSpecificSetting(void)
{
    return OSA_SOK;
}


int DRV_imgsSet50_60Hz(Bool is50Hz)
{
    return 0;
}

int DRV_imgsSetFramerate(int fps)
{
    return OSA_SOK;
}

int DRV_imgsBinEnable(Bool enable)
{
    return OSA_EFAIL;
}

int DRV_imgsBinMode(int binMode)
{
    return OSA_SOK;
}

//return value is 8 times the true analog gain
int DRV_imgsGetAgain(int *again)
{
    /* Not support */
    return OSA_EFAIL;
}

int DRV_imgsGetEshutter(Uint32 * eshutterInUsec)
{
    /* Not support */
    return OSA_EFAIL;
}

int DRV_imgsSetOB(int Level)
{
    int status = OSA_SOK;
    return status;
}

int DRV_imgsSetDcSub(Uint32 dcSub, int setRegDirect)
{
    return OSA_SOK;
}

int DRV_imgsEnable(Bool enable)
{
    return OSA_SOK;
}

int DRV_imgsGetLightSensorMode(void)
{
    /* return Low light:1, Normal:0 */
    /* LS:Normal 1, low light 0 */
    return 0;
}

int DRV_imgs_IsDDRIn(void)
{
    return gbIsDDRIn;
}

int DRV_imgs_IsAlawEnable(void)
{
    return gbIsAlawEnalbe;
}

int DRV_imgsNDShutter(int bIsNight)
{
    return 0;
}

DRV_ImgsModeConfig *DRV_imgsGetModeConfig(int sensorMode)
{
    return &gDRV_imgsObj.curModeConfig;
}

DRV_ImgsIsifConfig *DRV_imgsGetIsifConfig(int sensorMode)
{
    return &gDRV_imgsIsifConfig_Common;
}

DRV_ImgsIpipeConfig *DRV_imgsGetIpipeConfig(int sensorMode, int vnfDemoCfg)
{
    if (vnfDemoCfg)
        return &gDRV_imgsIpipeConfig_Vnfdemo;
    else
        return &gDRV_imgsIpipeConfig_Common;
}

DRV_ImgsH3aConfig *DRV_imgsGetH3aConfig(int sensorMode, int aewbVendor)
{
    if (aewbVendor == 1)
    {
        printf("APPRO Setting Applied\n");
        return &gDRV_imgsH3aConfig_Appro;
    }
    else if (aewbVendor == 2)
    {
        printf("TI Setting Applied\n");
        return &gDRV_imgsH3aConfig_TI;
    }
    else
    {
        printf("Common Setting Applied\n");
        return &gDRV_imgsH3aConfig_Common;
    }
}

DRV_ImgsLdcConfig *DRV_imgsGetLdcConfig(int sensorMode,
                                        Uint16 ldcInFrameWidth,
                                        Uint16 ldcInFrameHeight)
{
    OSA_printf(" DRV LDC: GetConfig %04x %dx%d\n", sensorMode,
               ldcInFrameWidth, ldcInFrameHeight);

    sensorMode &= 0xFF;

    switch (sensorMode)
    {

    case DRV_IMGS_SENSOR_MODE_720x480:
    //case DRV_IMGS_SENSOR_MODE_1620x1080:

        if (ldcInFrameWidth == 864)
            return &gDRV_imgsLdcConfig_736x480_0_VS;

        if (ldcInFrameWidth == 352)
            return &gDRV_imgsLdcConfig_736x480_1_VS;

        if (ldcInFrameWidth == 736)
            return &gDRV_imgsLdcConfig_736x480_0;

        if (ldcInFrameWidth == 288)
            return &gDRV_imgsLdcConfig_736x480_1;

        if (ldcInFrameWidth == 768)
            return &gDRV_imgsLdcConfig_768x512_0;

        if (ldcInFrameWidth == 320)
            return &gDRV_imgsLdcConfig_768x512_1;

        if (ldcInFrameWidth == 928)
            return &gDRV_imgsLdcConfig_768x512_0_VS;

        if (ldcInFrameWidth == 384)
            return &gDRV_imgsLdcConfig_768x512_1_VS;

        break;

    case DRV_IMGS_SENSOR_MODE_1280x720:
    case DRV_IMGS_SENSOR_MODE_1920x1080:

        if (ldcInFrameWidth == 1280)
            return &gDRV_imgsLdcConfig_1280x736_0;

        if (ldcInFrameWidth == 320)
            return &gDRV_imgsLdcConfig_1280x736_1;

        if (ldcInFrameWidth == 640)
            return &gDRV_imgsLdcConfig_1280x736_2;

        if (ldcInFrameWidth == 1536)
            return &gDRV_imgsLdcConfig_1280x736_0_VS;

        if (ldcInFrameWidth == 384)
            return &gDRV_imgsLdcConfig_1280x736_1_VS;

        if (ldcInFrameWidth == 768)
            return &gDRV_imgsLdcConfig_1280x736_2_VS;

        if (ldcInFrameWidth == 1312)
            return &gDRV_imgsLdcConfig_1312x768_0;

        if (ldcInFrameWidth == 352)
            return &gDRV_imgsLdcConfig_1312x768_1;

        if (ldcInFrameWidth == 672)
            return &gDRV_imgsLdcConfig_1312x768_2;

        if (ldcInFrameWidth == 1600)
            return &gDRV_imgsLdcConfig_1312x768_0_VS;

        if (ldcInFrameWidth == 448)
            return &gDRV_imgsLdcConfig_1312x768_1_VS;

        if (ldcInFrameWidth == 832)
            return &gDRV_imgsLdcConfig_1312x768_2_VS;

        break;

    }

    return NULL;
}

int DRV_imgsReset()
{
    return OSA_SOK;
}

int DRV_imgsCheckId()
{
    return OSA_SOK;
}

int DRV_imgsSetDgain(int dgain)
{
    return OSA_SOK;
}

int DRV_imgsSetAgain(int again, int setRegDirect)
{
/*
   0 dB to 24 dB nalog Gain 24 dB (step pitch 0.3 dB) 
24.3 dB to 42 dB nalog Gain 24 dB + Digital Gain 0.3 to 18 dB (step pitch 0.3 dB)
*/
    float gain_times;
    double gain_db;
    unsigned int gain_reg;

//42db:125000
    if (again <= 1000) again = 1000;
    if (again >= 250000) again = 250000;

    gain_times = again / 1000.0;
    gain_db = 20.0 * log10(gain_times);
    gain_reg = (unsigned char) (gain_db / 0.3 + 0.5);    //for 0.3db/step
    if (gain_reg > 0xA0) gain_reg = 0xA0;

    imx104_setgio(0x0214, (gain_reg & 0xff));

#ifdef AEWB_DEBUG
    OSA_printf("gain value:%d,again:%d,gain_db:%f,gain_times:%f\n", gain_reg,
               again, gain_db, gain_times);
#endif

    return 0;
}

//set the shutter start line,setRegDirect = 0
int DRV_imgsSetEshutter(Uint32 eshutterInUsec, int setRegDirect)
{

    Uint32 SetValue, SUB_Control = 0, tempdata;
    Uint8 Send_HH, Send_H, Send_L;

    DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;


    if (setRegDirect)
    {
        // eshutterInUsec in line
        SetValue = eshutterInUsec;
    }
    else
    {
        // eshutterInUsec in Usec.
        DRV_imgsCalcSW(eshutterInUsec);
        SetValue = pFrame->SW;
    }

#ifdef AEWB_DEBUG
    OSA_printf("IMX104 ShutterInUsec:%d ; SetRegDirect:%d ; SetValue:%d \n",
               eshutterInUsec, setRegDirect, SetValue);
#endif

    if (SetValue < IMGS_SENSOR_LINEWIDTH)
    {
        SUB_Control = IMGS_SENSOR_LINEWIDTH - SetValue;
    }
    else
    {
        SUB_Control = 0;
    }
    Send_HH = (Uint8) (SUB_Control >> 16);
    Send_H = (Uint8) ((SUB_Control >> 8) & 0x00FF);
    Send_L = (Uint8) (SUB_Control & 0x00FF);

    imx104_setgio(0x0222, Send_HH);
    imx104_setgio(0x0221, Send_H);
    imx104_setgio(0x0220, Send_L);

    /* long exposure ,need re-set VMAX */
    if (SetValue < IMGS_SENSOR_LINEWIDTH)
    {
        tempdata = IMGS_SENSOR_LINEWIDTH;
    }
    else
    {
        tempdata = SetValue;
    }
    Send_HH = (Uint8) (tempdata >> 16);
    Send_H = (Uint8) ((tempdata >> 8) & 0x00FF);
    Send_L = (Uint8) (tempdata & 0x00FF);

    imx104_setgio(0x021A, Send_HH);
    imx104_setgio(0x0219, Send_H);
    imx104_setgio(0x0218, Send_L);

    return 0;

}


int DRV_imgsSetRegs(Uint16 width, Uint16 height, int fps)
{

#define IMX104_REG_ID3_SET(X,Y) imx104_setgio(0x0300|X,Y)
#define IMX104_REG_ID2_SET(X,Y) imx104_setgio(0x0200|X,Y)
#define IMX104_REG_ID4_SET(X,Y) imx104_setgio(0x0400|X,Y)
	/* ID2 */
	IMX104_REG_ID2_SET(0x00,0x01);// STANDBY
	IMX104_REG_ID2_SET(0x01,0x00);// REG NOT HOLD
	IMX104_REG_ID2_SET(0x02,0x01);// MASTER MODE STOP
	IMX104_REG_ID2_SET(0x03,0x01);// SW RESET START
	OSA_waitMsecs(50); 
	IMX104_REG_ID2_SET(0x03,0x00);// SW RESET STOP
	IMX104_REG_ID2_SET(0x04,0x10);// FIXED
	IMX104_REG_ID2_SET(0x05,0x01);// AD 12BIT, NO SHIFT
	IMX104_REG_ID2_SET(0x06,0x00);// DRIVE MODE = ALL PIXEL SCAN MODE
	IMX104_REG_ID2_SET(0x07,0x00);// NO H,V REVERSE , WIN MODE = ALL PIXEL
	IMX104_REG_ID2_SET(0x08,0x10);// FIXED
	IMX104_REG_ID2_SET(0x09,0x02);// FRAME RATE = 30 FPS
	IMX104_REG_ID2_SET(0x0A,0xF0);// 12BIT BLACK LEVEL[0:7]
	IMX104_REG_ID2_SET(0x0B,0x00);// 12BIT BLACK LEVEL[8]
	IMX104_REG_ID2_SET(0x0C,0x00);// FIXED
	IMX104_REG_ID2_SET(0x0D,0x20);// FIXED
	IMX104_REG_ID2_SET(0x0E,0x01);// FIXED
	IMX104_REG_ID2_SET(0x0F,0x01);// FIXED
	IMX104_REG_ID2_SET(0x10,0x39);// FIXED
	IMX104_REG_ID2_SET(0x11,0x00);// FIXED
	IMX104_REG_ID2_SET(0x12,0x50);// FIXED
	IMX104_REG_ID2_SET(0x13,0x00);// FIXED
	IMX104_REG_ID2_SET(0x14,0x00);// GAIN = 0 DB
	IMX104_REG_ID2_SET(0x15,0x00);// FIXED
	IMX104_REG_ID2_SET(0x16,0x08);// FIXED
	IMX104_REG_ID2_SET(0x17,0x01);// FIXED
	IMX104_REG_ID2_SET(0x18,0x44);// VMAX[0:7]
	IMX104_REG_ID2_SET(0x19,0x04);// VMAX[8:15]
	IMX104_REG_ID2_SET(0x1A,0x00);// VMAX[16]
	IMX104_REG_ID2_SET(0x1B,0xE4);// HMAX[0:7]
	IMX104_REG_ID2_SET(0x1C,0x0C);// HMAX[8:15]
	IMX104_REG_ID2_SET(0x1D,0xFF);// FIXED
	IMX104_REG_ID2_SET(0x1E,0x01);// FIXED
	IMX104_REG_ID2_SET(0x1F,0x00);// FIXED
	IMX104_REG_ID2_SET(0x20,0x00);// SUB CONTROL [0:7]
	IMX104_REG_ID2_SET(0x21,0x00);// SUB CONTROL [8:15]
	IMX104_REG_ID2_SET(0x22,0x00);// SUB CONTROL [16]
	IMX104_REG_ID2_SET(0x36,0x14);// VOPB SIZE
	IMX104_REG_ID2_SET(0x37,0x00);// FIXED
	IMX104_REG_ID2_SET(0x38,0x00);// WINPV[0:7] , V CROPPING POSITION
	IMX104_REG_ID2_SET(0x39,0x00);// WINPV[8:10]
	IMX104_REG_ID2_SET(0x3A,0x19);// WINWV[0:7] , V CROPPING SIZE
	IMX104_REG_ID2_SET(0x3B,0x04);// WINWV[8:10]
	IMX104_REG_ID2_SET(0x3C,0x00);// WINPH[0:7] , H CROPPING POSITION
	IMX104_REG_ID2_SET(0x3D,0x00);// WINPV[8:10]
	IMX104_REG_ID2_SET(0x3E,0x1C);// WINWH[0:7] , H CROPPING SIZE
	IMX104_REG_ID2_SET(0x3F,0x05);// WINWH[8:10]
	IMX104_REG_ID2_SET(0x40,0x00);// FIXED
	IMX104_REG_ID2_SET(0x41,0x00);// FIXED
	IMX104_REG_ID2_SET(0x42,0x00);// FIXED
	IMX104_REG_ID2_SET(0x43,0x00);// FIXED
	IMX104_REG_ID2_SET(0x44,0x01);// 12 BIT NUMBER,PARALLEL CMOS O/P
	IMX104_REG_ID2_SET(0x45,0x01);
	IMX104_REG_ID2_SET(0x46,0x00);// XVS PULSE WIDTH
	IMX104_REG_ID2_SET(0x47,0x08);// XHS PULSE MIN. O/P
	IMX104_REG_ID2_SET(0x49,0x00);// XVS AND XHS O/P FIXED AT HI
	IMX104_REG_ID2_SET(0x54,0x63);// FIXED
	IMX104_REG_ID2_SET(0x5B,0x01);// INCKSEL1
	IMX104_REG_ID2_SET(0x5C,0x20);// FIXED
	IMX104_REG_ID2_SET(0x5D,0x10);// INCKSEL2
	IMX104_REG_ID2_SET(0x5E,0x2C);// FIXED
	IMX104_REG_ID2_SET(0x5F,0x10);// INCKSEL3
	IMX104_REG_ID2_SET(0x60,0x00);// FIXED
	IMX104_REG_ID2_SET(0x61,0x21);// FIXED
	IMX104_REG_ID2_SET(0x62,0x08);// FIXED by SONY
	IMX104_REG_ID2_SET(0xA1,0x45);// FIXED
	IMX104_REG_ID2_SET(0xBF,0x1F);// FIXED



	/* ID3 */
	IMX104_REG_ID3_SET(0x12,0x00);
	IMX104_REG_ID3_SET(0x1D,0x07);
	IMX104_REG_ID3_SET(0x23,0x07);
	IMX104_REG_ID3_SET(0x26,0xDF);
	IMX104_REG_ID3_SET(0x47,0x87);
	

	/* ID4 */
	IMX104_REG_ID4_SET(0x03,0xCD);
	IMX104_REG_ID4_SET(0x07,0x4B);
	IMX104_REG_ID4_SET(0x09,0xE9);
	IMX104_REG_ID4_SET(0x13,0x1B);
	IMX104_REG_ID4_SET(0x15,0xED);
	IMX104_REG_ID4_SET(0x16,0x01);
	IMX104_REG_ID4_SET(0x18,0x09);
	IMX104_REG_ID4_SET(0x1A,0x19);
	IMX104_REG_ID4_SET(0x1B,0xA1);
	IMX104_REG_ID4_SET(0x1C,0x11);
	IMX104_REG_ID4_SET(0x27,0x00);
	IMX104_REG_ID4_SET(0x28,0x05);
	IMX104_REG_ID4_SET(0x29,0xEC);
	IMX104_REG_ID4_SET(0x2A,0x40);
	IMX104_REG_ID4_SET(0x2B,0x11);
	IMX104_REG_ID4_SET(0x2D,0x22);
	IMX104_REG_ID4_SET(0x2E,0x00);
	IMX104_REG_ID4_SET(0x2F,0x05);
	IMX104_REG_ID4_SET(0x31,0xEC);
	IMX104_REG_ID4_SET(0x32,0x40);
	IMX104_REG_ID4_SET(0x33,0x11);
	IMX104_REG_ID4_SET(0x35,0x23);
	IMX104_REG_ID4_SET(0x36,0xB0);
	IMX104_REG_ID4_SET(0x37,0x04);
	IMX104_REG_ID4_SET(0x39,0x24);
	IMX104_REG_ID4_SET(0x3A,0x30);
	IMX104_REG_ID4_SET(0x3B,0x04);
	IMX104_REG_ID4_SET(0x3C,0xED);
	IMX104_REG_ID4_SET(0x3D,0xC0);
	IMX104_REG_ID4_SET(0x3E,0x10);
	IMX104_REG_ID4_SET(0x40,0x44);
	IMX104_REG_ID4_SET(0x41,0xA0);
	IMX104_REG_ID4_SET(0x42,0x04);
	IMX104_REG_ID4_SET(0x43,0x0D);
	IMX104_REG_ID4_SET(0x44,0x31);
	IMX104_REG_ID4_SET(0x45,0x11);
	IMX104_REG_ID4_SET(0x47,0xEC);
	IMX104_REG_ID4_SET(0x48,0xD0);
	IMX104_REG_ID4_SET(0x49,0x1D);
	IMX104_REG_ID4_SET(0x55,0x03);
	IMX104_REG_ID4_SET(0x56,0x54);
	IMX104_REG_ID4_SET(0x57,0x60);
	IMX104_REG_ID4_SET(0x58,0x1F);
	IMX104_REG_ID4_SET(0x5A,0xA9);
	IMX104_REG_ID4_SET(0x5B,0x50);
	IMX104_REG_ID4_SET(0x5C,0x0A);
	IMX104_REG_ID4_SET(0x5D,0x25);
	IMX104_REG_ID4_SET(0x5E,0x11);
	IMX104_REG_ID4_SET(0x5F,0x12);
	IMX104_REG_ID4_SET(0x61,0x9B);
	IMX104_REG_ID4_SET(0x66,0xD0);
	IMX104_REG_ID4_SET(0x67,0x08);
	IMX104_REG_ID4_SET(0x6A,0x20);
	IMX104_REG_ID4_SET(0x6B,0x0A);
	IMX104_REG_ID4_SET(0x6E,0x20);
	IMX104_REG_ID4_SET(0x6F,0x0A);
	IMX104_REG_ID4_SET(0x72,0x20);
	IMX104_REG_ID4_SET(0x73,0x0A);
	IMX104_REG_ID4_SET(0x75,0xEC);
	IMX104_REG_ID4_SET(0x7D,0xA5);
	IMX104_REG_ID4_SET(0x7E,0x20);
	IMX104_REG_ID4_SET(0x7F,0x0A);
	IMX104_REG_ID4_SET(0x81,0xEF);
	IMX104_REG_ID4_SET(0x82,0xC0);
	IMX104_REG_ID4_SET(0x83,0x0E);
	IMX104_REG_ID4_SET(0x85,0xF6);
	IMX104_REG_ID4_SET(0x8A,0x60);
	IMX104_REG_ID4_SET(0x8B,0x1F);
	IMX104_REG_ID4_SET(0x8D,0xBB);
	IMX104_REG_ID4_SET(0x8E,0x90);
	IMX104_REG_ID4_SET(0x8F,0x0D);
	IMX104_REG_ID4_SET(0x90,0x39);
	IMX104_REG_ID4_SET(0x91,0xC1);
	IMX104_REG_ID4_SET(0x92,0x1D);
	IMX104_REG_ID4_SET(0x94,0xC9);
	IMX104_REG_ID4_SET(0x95,0x70);
	IMX104_REG_ID4_SET(0x96,0x0E);
	IMX104_REG_ID4_SET(0x97,0x47);
	IMX104_REG_ID4_SET(0x98,0xA1);
	IMX104_REG_ID4_SET(0x99,0x1E);
	IMX104_REG_ID4_SET(0x9B,0xC5);
	IMX104_REG_ID4_SET(0x9C,0xB0);
	IMX104_REG_ID4_SET(0x9D,0x0E);
	IMX104_REG_ID4_SET(0x9E,0x43);
	IMX104_REG_ID4_SET(0x9F,0xE1);
	IMX104_REG_ID4_SET(0xA0,0x1E);
	IMX104_REG_ID4_SET(0xA2,0xBB);
	IMX104_REG_ID4_SET(0xA3,0x10);
	IMX104_REG_ID4_SET(0xA4,0x0C);
	IMX104_REG_ID4_SET(0xA6,0xB3);
	IMX104_REG_ID4_SET(0xA7,0x30);
	IMX104_REG_ID4_SET(0xA8,0x0A);
	IMX104_REG_ID4_SET(0xA9,0x29);
	IMX104_REG_ID4_SET(0xAA,0x91);
	IMX104_REG_ID4_SET(0xAB,0x11);
	IMX104_REG_ID4_SET(0xAD,0xB4);
	IMX104_REG_ID4_SET(0xAE,0x40);
	IMX104_REG_ID4_SET(0xAF,0x0A);
	IMX104_REG_ID4_SET(0xB0,0x2A);
	IMX104_REG_ID4_SET(0xB1,0xA1);
	IMX104_REG_ID4_SET(0xB2,0x11);
	IMX104_REG_ID4_SET(0xB4,0xAB);
	IMX104_REG_ID4_SET(0xB5,0xB0);
	IMX104_REG_ID4_SET(0xB6,0x0B);
	IMX104_REG_ID4_SET(0xB7,0x21);
	IMX104_REG_ID4_SET(0xB8,0x11);
	IMX104_REG_ID4_SET(0xB9,0x13);
	IMX104_REG_ID4_SET(0xBB,0xAC);
	IMX104_REG_ID4_SET(0xBC,0xC0);
	IMX104_REG_ID4_SET(0xBD,0x0B);
	IMX104_REG_ID4_SET(0xBE,0x22);
	IMX104_REG_ID4_SET(0xBF,0x21);
	IMX104_REG_ID4_SET(0xC0,0x13);
	IMX104_REG_ID4_SET(0xC2,0xAD);
	IMX104_REG_ID4_SET(0xC3,0x10);
	IMX104_REG_ID4_SET(0xC4,0x0B);
	IMX104_REG_ID4_SET(0xC5,0x23);
	IMX104_REG_ID4_SET(0xC6,0x71);
	IMX104_REG_ID4_SET(0xC7,0x12);
	IMX104_REG_ID4_SET(0xC9,0xB5);
	IMX104_REG_ID4_SET(0xCA,0x90);
	IMX104_REG_ID4_SET(0xCB,0x0B);
	IMX104_REG_ID4_SET(0xCC,0x2B);
	IMX104_REG_ID4_SET(0xCD,0xF1);
	IMX104_REG_ID4_SET(0xCE,0x12);
	IMX104_REG_ID4_SET(0xD0,0xBB);
	IMX104_REG_ID4_SET(0xD1,0x10);
	IMX104_REG_ID4_SET(0xD2,0x0C);
	IMX104_REG_ID4_SET(0xD4,0xE7);
	IMX104_REG_ID4_SET(0xD5,0x90);
	IMX104_REG_ID4_SET(0xD6,0x0E);
	IMX104_REG_ID4_SET(0xD8,0x45);
	IMX104_REG_ID4_SET(0xD9,0x11);
	IMX104_REG_ID4_SET(0xDA,0x1F);
	IMX104_REG_ID4_SET(0xEB,0xA4);
	IMX104_REG_ID4_SET(0xEC,0x60);
	IMX104_REG_ID4_SET(0xED,0x1F);	
	
	#if 1
	// input 54Mhz
	IMX104_REG_ID2_SET(0x5B,0x1);	// INCLK1
	IMX104_REG_ID2_SET(0x5D,0x10);	// INCLK2
	IMX104_REG_ID2_SET(0x5F,0x10);	//INCLK3
	#else
	// input 37.125Mhz
	IMX104_REG_ID2_SET(0x5B,0x0);// INCLK1
	IMX104_REG_ID2_SET(0x5D,0x0);// INCLK2
	IMX104_REG_ID2_SET(0x5F,0x10);//INCLK3
	#endif

	IMX104_REG_ID2_SET(0x46,0x30);	// XVS PULSE WIDTH=8H
	IMX104_REG_ID2_SET(0x47,0x18);	// XHS PULSE =128 pixel
	IMX104_REG_ID2_SET(0x05,0x1);	// AD bit 12
	IMX104_REG_ID2_SET(0x06,0x00);	// all pixel 
	IMX104_REG_ID2_SET(0x09,0x2);	// FRSEL=30
	IMX104_REG_ID2_SET(0x18,0x44);
	IMX104_REG_ID2_SET(0x19,0x04); // V = 1092
	IMX104_REG_ID2_SET(0x1B,0xE0);
	IMX104_REG_ID2_SET(0x1C,0x0C); // H =3300-4 = 3296
	IMX104_REG_ID2_SET(0x44,0x1);	//ODBIT 12 & Parallel CMOS


	printf("######### Run in Normal Mode ###########\n");


	IMX104_REG_ID2_SET(0x00,0x00);// STANDBY CANCEL
	OSA_waitMsecs(50);	// wait for stable
	IMX104_REG_ID2_SET(0x02,0x00);// Master mode start
	IMX104_REG_ID2_SET(0x49,0x0a);// XVS AND XHS O/P start
	OSA_waitMsecs(50);	// wait for stable

	DRV_imgsSetEshutter(1092, 1);	// exposure 1 frame ( 1/30sec)
	DRV_imgsSetAgain(1000,0);	//1000=0db

    return 0;
}

int DRV_imgs_APChipInit(int *pGioC1, int *pGioD1)
{
    unsigned int *pPINMUXC1 = NULL;
    unsigned int *pPINMUXD1 = NULL;
    unsigned int val1, val2;
    unsigned char *pRegStart;
    int ret = 0;

    if (gCSL_vpssHndl.regBaseVirtAddr == 0 || pGioC1 == 0 || pGioD1 == 0)
    {
        return -1;
    }
    pRegStart = (unsigned char *) gCSL_vpssHndl.regBaseVirtAddr;

    pPINMUXC1 =
        (unsigned int *) (pRegStart + (AP_CHIP_C1_PINMUX - RegBaseVirtStart));
    pPINMUXD1 =
        (unsigned int *) (pRegStart + (AP_CHIP_D1_PINMUX - RegBaseVirtStart));
    /* Enable C1 */
    *pPINMUXC1 = ((*pPINMUXC1 & (~AP_CHIP_C1_MASK)) | AP_CHIP_C1_VAL);
    val1 = (*pPINMUXC1 & AP_CHIP_C1_MASK);
    if (AP_CHIP_C1_VAL != val1)
    {
        DBErr("PINMUXC1 setting error(%X)!!!\n", val1);
        ret = -1;
        goto __DRV_imgs_APChipInit_End__;
    }
    /* Enable D1 */
    *pPINMUXD1 = (*pPINMUXD1 & (~AP_CHIP_D1_MASK)) | (AP_CHIP_D1_VAL);
    val2 = (*pPINMUXD1 & AP_CHIP_D1_MASK);
    if (AP_CHIP_D1_VAL != val2)
    {
        DBErr("PINMUXD1 setting error(%X)!!!\n", val2);
        ret = -1;
        goto __DRV_imgs_APChipInit_End__;
    }

    *pGioC1 = IMGS_APCHIP_C1_GIO;
    *pGioD1 = IMGS_APCHIP_D1_GIO;

  __DRV_imgs_APChipInit_End__:

    return ret;
}
