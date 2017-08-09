#ifndef _CMD_SERVER_H
#define _CMD_SERVER_H

#define VI_DEBUG_MODE

#ifdef VI_DEBUG_MODE
#define VI_DEBUG(...) \
    do \
    { \
        fprintf(stderr, " DEBUG (%s|%s|%d): ", \
                __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    while(0)

#else
#define VI_DEBUG(...)
#endif

#define USE_MESSAGE 0
#define CMD_NONE_MSG_KEY 0x1001
#define LEN_CMD_MSG_KEY 0x5000000
#define VIM_CMD_MSG_KEY 0x7000000
#define SYS_CMD_MSG_KEY 0x8000000

#define MSG_BUF_SIZE 256

#define IP_CMD_ISP_SET_EXPOSURE 0x01 
#define IP_CMD_ISP_SET_AE_DELAY 0x02
#define IP_CMD_ISP_GET_ETGAIN 0x03
#define IP_CMD_ISP_SET_AE_ROI 0x05
#define IP_CMD_ISP_SET_AWB 0x06
#define IP_CMD_ISP_SET_BASE_ATTR 0x07
#define IP_CMD_ISP_SET_FLIP_MIRROR 0x08
#define IP_CMD_ISP_SET_IRIS 0x09
#define IP_CMD_ISP_SET_IRCUT 0x0A
#define IP_CMD_ISP_SET_COLOR_BLACK 0x0B
#define IP_CMD_ISP_SET_DR_MODE 0x0C
#define IP_CMD_ISP_SET_DENOISE 0x0D
#define IP_CMD_ISP_SET_EIS 0x0E
#define IP_CMD_ISP_SET_DEFOG 0x0F
#define IP_CMD_ISP_SET_SAVE_ATTR 0x10
#define IP_CMD_ISP_SET_DEFAULT_ATTR 0x11
#define IP_CMD_ISP_GET_CURRENT_ATTR 0x12
#define IP_CMD_SYS_SET_CAMERA_IP 0x13
#define IP_CMD_SYS_SET_TIME 0x14
#define IP_CMD_SYS_GET_TIME 0x15
#define IP_CMD_LEN_CONTROL 0x36

#define IP_CMD_ISP_GET_ERROR 0xFF

typedef struct _cmd_server_msg_t
{
	long int msg_type;
	unsigned char msg_data[MSG_BUF_SIZE];
}tCmdServerMsg, *ptCmdServerMSg;


//Exposure 
typedef enum VF_AE_SPEED_MODE_E
{
    VF_AE_S5 = 0, 
    VF_AE_S10, 
    VF_AE_S25, //默认值
    VF_AE_S30, 
    VF_AE_S33, 
    VF_AE_S40, 
    VF_AE_S50, 
    VF_AE_S60, 
    VF_AE_S100, 
    VF_AE_S120, 
    VF_AE_S250, 
    VF_AE_S500, 
    VF_AE_S1000, 
    VF_AE_S5000, 
    VF_AE_S10000, 
    VF_AE_MAXET_MANUAL, 
    VF_AE_MAXET_INVALID = 0xff 
}VF_AE_MAXET_MODE_E;

typedef enum VF_AE_SHUTTER_MODE_E 
{ 
    VF_AE_50Hz_Auto=0, //默认值
    VF_AE_60Hz_Auto, 
    VF_AE_50Hz_Indoor, 
    VF_AE_60Hz_Indoor, 
    VF_AE_ROI,
    VF_AE_MANUAL = 0xf, 
    VF_AE_INVALID = 0xff 
}VF_AE_SHUTTER_MODE_E;

typedef struct VF_AE_MODE_S
{
    VF_AE_SHUTTER_MODE_E AE_Shutter_Mode;
    VF_AE_MAXET_MODE_E AE_MaxET_Mode; 
    unsigned int Exposuretime; 
    unsigned int MaxET; 
    unsigned char Gain; 
    unsigned char MaxGain; 
    unsigned char GainDeci; 
}VF_AE_MODE_S, *pVF_AE_MODE_S;


typedef struct VF_AE_ETGain_S
{
    unsigned int etus;
    unsigned short gainValue;
}VF_AE_ETGain_S, *pVF_AE_ETGain_S;

typedef struct VF_AE_ROI_S
{
	unsigned int ROI_No;
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
}VF_AE_ROI_S, *pVF_AE_ROI_S;


//White Balance
typedef enum VF_AWB_MODE_E 
{ 
    VF_AWB_AUTO = 0x0, //默认值
    VF_AWB_FLUORESCENT_LAMP = 0x10, 
    VF_AWB_INCANDESCENT_LAMP = 0x11, 
    VF_AWB_OUTDOOR_SUN = 0x12, 
    VF_AWB_OUTDOOR_CLOUDY = 0x13, 
    VF_AWB_OUTDOOR_DUSK = 0x14, 
    VF_AWB_INDOOR_OFFICE = 0x15, 
    VF_AWB_DISABLE = 0x1f, 
}VF_AWB_MODE_E;

typedef enum VF_AWB_AE_RELATION_E 
{ 
    VF_AWB_WITHOUT_AE_STABLE = 0x0, 
    VF_AWB_WITH_AE_STABLE //默认值
}VF_AWB_AE_RELATION_E;

typedef struct VF_AWB_MODE_S
{
    VF_AWB_MODE_E Mode;
    VF_AWB_AE_RELATION_E value;
    unsigned short interval;
    unsigned char speed;
    unsigned short Delay;
}VF_AWB_MODE_S, *pVF_AWB_MODE_S;

//Base attribute
typedef struct VF_BASE_ATTRIBUTE_S
{
    unsigned int BrightnessCoeff;
    unsigned int ContrastCoeff;
    unsigned int SaturationCoeff;
    unsigned int HueCoeff;
    unsigned int SharpnessCoeff;
}VF_BASE_ATTRIBUTE_S, *pVF_BASE_ATTRIBUTE_S;

//Flip and Mirror
typedef enum VF_FLIP_MIRROR_MODE_E 
{ 
    VF_FLIP_MIRROR_Normal, //默认值
    VF_FLIP_MIRROR_Flip, 
    VF_FLIP_MIRROR_Mirror,
    VF_FLIP_MIRROR_FlipMirror 
}VF_FLIP_MIRROR_MODE_E;

//IRIS
typedef enum VF_IRIS_MODE_E 
{ 
    VF_IRIS_AUTO_FIXED = 0x0, 
    VF_IRIS_AUTO_IRIS, //默认值
    VF_IRIS_MANUAL_IRIS_LENS, 
    VF_IRIS_AUTO_IRIS_FORCE, 
}VF_IRIS_MODE_E;

//IRCUT
typedef enum VF_IRCUT_MODE_E 
{ 
    VF_IRCUT_AUTO = 0x0, //默认值
    VF_IRCUT_MANUAL_DAY, 
    VF_IRCUT_MANUAL_NIGHT, 
}VF_IRCUT_MODE_E;

typedef struct VF_IRCUT_MODE_S 
{ 
    VF_IRCUT_MODE_E Mode;
    unsigned int Th;
}VF_IRCUT_MODE_S, *pVF_IRCUT_MODE_S;

//Color Black
typedef enum VF_COLORBLACK_MODE_E 
{ 
    VF_COLORBLACK_AUTO = 0x0, //默认值
    VF_COLORBLACK_MANUAL_COLOR,
    VF_COLORBLACK_MUNUAL_BLACK, 
}VF_COLORBLACK_MODE_E;

//DR
typedef enum VF_DR_MODE_E 
{ 
    VF_DRMODE_DEBUG = 0x01, 
    VF_DRMODE_BACKLIGHT=2, 
    VF_DRMODE_OE=4, 
    VF_DRMODE_WDR=8, 
    VF_DRMODE_NORMAL=16, //默认值
}VF_DR_MODE_E;

typedef enum VF_DR_LEVEL_E 
{ 
    VF_DR_LEVEL_LOW = 0, //默认值
    VF_DR_LEVEL_MID, 
    VF_DR_LEVEL_HIGH, 
}VF_DR_LEVEL_E;

typedef struct VF_DR_MODE_S 
{ 
    VF_DR_MODE_E Mode;
    VF_DR_LEVEL_E Level;
}VF_DR_MODE_S, *pVF_DR_MODE_S;

//DENOISE
typedef enum VF_DENOISE_MODE_E 
{ 
    VF_DENOISE_CLOSE = 0x0, 
    VF_DENOISE_2D, //默认值
    VF_DENOISE_3D, 
}VF_DENOISE_MODE_E;

typedef enum VF_NR3D_LEVEL_E 
{ 
    VF_NR3D_LEVEL_LOW=0, //默认值
    VF_NR3D_LEVEL_MID, 
    VF_NR3D_LEVEL_HIGH, 
}VF_NR3D_LEVEL_E;

typedef struct VF_DENOISE_MODE_S 
{ 
    VF_DENOISE_MODE_E Mode;
    VF_NR3D_LEVEL_E Level;
}VF_DENOISE_MODE_S, *pVF_DENOISE_MODE_S;

//EIS
typedef enum VF_EIS_FLAG_E 
{
    VF_EIS_DISABLE = 0x0, //默认值
    VF_EIS_ENABLE 
}VF_EIS_FLAG_E;

//DEFOG
typedef enum VF_DEFOG_MODE_E 
{ 
    VF_DEFOG_CLOSE = 0x0, //默认值
    VF_DEFOG_LOW, 
    VF_DEFOG_MID, 
    VF_DEFOG_HIGH, 
}VF_DEFOG_MODE_E;

//Reserved
typedef enum VF_COLOR_TYPE_E
{ 
    VF_BT601 = 0x0, 
    VF_BT709, 
}VF_COLOR_TYPE_E;

typedef enum VF_EIS_MODE_E
{
	VF_EIS_CLOSE = 0x0,
	VF_EIS_UPSCALE,
	VF_EIS_EMBEDED,
}VF_EIS_MODE_E;

typedef enum VF_LDC_MODE_E
{
	VF_LDC_CLOSE = 0x0,
	VF_LDC_SINGLE,
	VF_LDC_2X2,
}VF_LDC_MODE_E;

typedef enum VF_DSS_NUM_E
{
	VF_DSS_CLOSE = 0x0,
	VF_DSS_NUM_2 = 2,
	VF_DSS_NUM_4 = 4,
	VF_DSS_NUM_8 = 8,
	VF_DSS_NUM_16 = 16,
	VF_DSS_NUM_32 = 32,
	VF_DSS_NUM_64 = 64,
	VF_DSS_NUM_128 = 128,
}VF_DSS_NUM_E;

typedef enum VF_AE_MANUAL_TMMODE_E
{
    VF_TM_OFF=0x0,
    VF_TM_ON,
    VF_TM_STOP,
}VF_AE_MANUAL_TMMODE_E;

typedef enum VF_MAXFRMRATE_E
{
    VF_FPS25 = 0x0,
    VF_FPS30
}VF_MAXFRMRATE_E;

//Attributes
typedef struct __VF_ATTRIBUTE__ 
{ 
    VF_AE_MODE_S AEMode; 
    VF_AWB_MODE_E AWBMode; 
    unsigned char BrightnessCoeff; 
    unsigned char HueCoeff; 
    unsigned char ContrastCoeff; 
    unsigned char EdgeCoeff; 
    unsigned char SaturationCoeff; 
    VF_FLIP_MIRROR_MODE_E FlipMirrorMode; 
    VF_IRIS_MODE_E IrisMode; 
    VF_IRCUT_MODE_E IRcutMode; 
    VF_COLORBLACK_MODE_E ColorBlack; 
    VF_DR_MODE_E DRMode; 
    VF_DR_LEVEL_E DRLevel; 
    VF_DENOISE_MODE_E DenoiseMode; 
    unsigned int AEdelay; 
    VF_DEFOG_MODE_E DefogMode; 
    unsigned char IRC_TH2; 
    VF_NR3D_LEVEL_E NR3DLevel; 
    VF_AWB_AE_RELATION_E AWBAERelation; 
    unsigned short AWBInterval; 
    unsigned short AWBDelay; 
    VF_MAXFRMRATE_E MaxFrmRate; 
}VF_ATTRIBUTE_S, *pVF_ATTRIBUTE_S;

//LEN
typedef enum CONTROL_MODE_E 
{ 
    VF_CONTROL_ZOOM_WIDE=0, //默认值
    VF_CONTROL_ZOOM_TELE, 
    VF_CONTROL_FOCUS_FAR,
    VF_CONTROL_FOCUS_NEAR,
    VF_CONTROL_IRIS_LARGE,
    VF_CONTROL_IRIS_SMALL,
    VF_CONTROL_LEN_STOP
}VF_LEN_CONTROL_E;

typedef struct VF_LEN_CONTROL_S 
{ 
    VF_LEN_CONTROL_E Mode;
    unsigned char speed;
}VF_LEN_CONTROL_S, *pVF_LEN_CONTROL_S;







#endif
