/*******************************************************************************
 *  All rights reserved, Copyright (C) vimicro LIMITED 2011                                                     
 * -----------------------------------------------------------------------------
 * [File Name]: 			API/VIM_API_Video.h
 * [Description]: 			video
 *				
 * [Author]: 				
 * [Date Of Creation]: 		2012-06-07 10:49
 * [Platform]: 				Independent Of Platform
 * [Note]: 
 *
 * Modification History : 
 * ------------------------------------------------------------------------------
 * Date					Author				Modifications
 * ------------------------------------------------------------------------------
 * 2012-06-07			Wu Pingping			Created
 *******************************************************************************/
/*******************************************************************************
 * This source code has been made available to you by VIMICRO on an
 * AS-IS basis. Anyone receiving this source code is licensed under VIMICRO
 * copyrights to use it in any way he or she deems fit, including copying it,
 * modifying it, compiling it, and redistributing it either with or without
 * modifications. Any person who transfers this source code or any derivative
 * work must include the VIMICRO copyright notice and this paragraph in
 * the transferred software.
 *******************************************************************************/
#ifndef _VIM_API_VIDEO_H_
#define _VIM_API_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_MAX_NUM 0x10
#define OSD_CHN_NUM 0x10
#define ALIGN_ORDER(val, size)  ((val)%(size))?((val)/(size)+1)*(size):(val)

#define SN_LEN 32

typedef enum
{
	VIM_P2I_EN	   = 0x0	,
	VIM_OSD_SET				,
	VIM_CLIP_TP				,
}VIM_VOUT_CMD_E;

typedef struct
{
	UINT32 x	;
	UINT32 y	;
}VIM_VOUT_POSITION_S;

typedef struct
{
	UINT32 w	;
	UINT32 h	;
}VIM_VOUT_SIZE_S;

typedef struct
{
	UINT8  cropen				;
	UINT8  dsen					;
	//VIM_VOUT_SIZE_S srcsize		;
	VIM_VOUT_SIZE_S cropsize		;
	VIM_VOUT_POSITION_S croppos	;
	VIM_VOUT_SIZE_S dssize		;
}VIM_VOUT_CROPDSCFG_S;

typedef struct
{
	UINT16  y		;
	UINT16  u		;
	UINT16  v		;
}VIM_VOUT_COLOR_S;

typedef	enum{
	VIM_IPP_TO_DCVO = 0,
	VIM_SIF_DCVO,
	VIM_DDR_TO_DCVO,
}VIM_VOUT_DCVOSRC_E;

typedef	enum{
	VIM_ISP_TO_IPP = 0,
	VIM_SIF_TO_IPP,
}VIM_VOUT_IPPSRC_E;
/*typedef struct
{
	UINT8  y0		;
	UINT8  y1		;
	UINT8  u		;
	UINT8  v		;
}VIM_VOUT_COLOR_S;*/

typedef enum
{
	OSD_COLOR_BG	 = 0x0	,//background
	OSD_COLOR_IDX0			,
	OSD_COLOR_IDX1			,
	OSD_COLOR_IDX2			,
}VIM_VOUT_COLOR_IDX_E;

typedef struct
{
	UINT8  fillmode				;//0:masaic,1:fixed color
	UINT8  blocksize			;//0:16x16,1:32x32,it will take effect when fillmode is mosaic(0).
	UINT8  order				;//pvc coordinate order,0:clockwise,1:countclockwise
	VIM_VOUT_COLOR_S fixedcolor	;//it is valid when when fillmode is fixed color(1).
}VIM_VOUT_PVCPARAM_S;

typedef enum
{
	NONASCIILIB	   = 0x0	,
	HALFWIDTHASCII			,
	FULLWIDTHASCII			,
}VIM_VOUT_FONTMODE_E;

typedef enum
{
	FONT_HORIZONTAL	   = 0x0	,
	FONT_VERTICAL				,
}VIM_VOUT_FONTDIR_E;

typedef struct
{
	UINT8	*filename		;
	VIM_VOUT_FONTMODE_E	mode;
	UINT8	width			;
	UINT8	height			;
}VIM_VOUT_FONTINFO_S;

typedef struct
{
	UINT8	*srcfontname			;// the source font library filename(including path).
	UINT8	*dstfontname			;// the convertrd font library filename(including path).
	UINT8	width					;
	UINT8	height					;
	VIM_VOUT_FONTDIR_E	dir			;
	VIM_VOUT_COLOR_IDX_E dotcolor	;//the font dot color
	VIM_VOUT_COLOR_IDX_E foilcolor	;//the foil color,(outline)
	VIM_VOUT_COLOR_IDX_E bgcolor	;//the other region point color
}VIM_VOUT_FONTCONVERTPARAM_S;

typedef enum
{
	YUV_NO_ADJUST	= 0x0	,
	YUV_64_940	   			,
	YUV_4_1019				,	
}VIM_VOUT_YUVADJUSTMODE_E;

typedef enum
{
	HIGH_ACTIVE	   = 0x0	,
	LOW_ACTIVE				,
}VIM_VOUT_SYNCPOLARITY_E;

typedef enum
{
	OSD_FROM_SRAM	   = 0x0	,
	OSD_FROM_DDR				,
}VIM_VOUT_OSDSRC_E;

typedef enum
{
	VIDEO_8BIT	   = 0x0	,
	VIDEO_10BIT				,
}VIM_VOUT_BITSWIDTH_E;

typedef enum
{
	P2I_BYPASS		= 0x0	,	
	P2I_AVERAGE				,
	P2I_ABSTRACT			,
}VIM_VOUT_P2IMODE_E;

typedef struct
{
	VIM_VOUT_COLOR_S	color0;
	VIM_VOUT_COLOR_S	color1;
	VIM_VOUT_COLOR_S	color2;
	UINT8				alphaval;
	VIM_VOUT_OSDSRC_E		src;
}VIM_VOUT_OSDCFG_S;

typedef struct
{
	UINT8	en				;
	UINT8	upscale			;
	UINT8	interval		;//the pixels between the two characters
	VIM_VOUT_COLOR_IDX_E	intvcoloridx;//set the interval color index(0-3)
	UINT16  posx			;//the osd start x
	UINT16  posy			;//the osd start y
	UINT8	width			;//the font library width
	UINT8	height			;//the font library height
	UINT32	charfmt			;//the corresponding bit indicates full width or half width 
	UINT8	*str			;
}VIM_VOUT_OSDCHNINFO_S;

typedef struct
{
	UINT16 uppermarginf0;	// pixels from vsync to the video for the top filed or frame
	UINT16 lowermarginf0;	// pixels from the video to vsync for the top filed or frame
	UINT16 uppermarginf1;	// pixels from vsync to the video for the bottom filed
	UINT16 lowermarginf1;	// pixels from the video to vsync for the bottom filed
	UINT16 heightf0;		//height for the top filed or frame
	UINT16 heightf1;		//height for the bottom filed
	UINT16 leftmargin;		// pixels from hsync to the video 
	UINT16 rightmargin;		// pixels from the video to hsync  
	UINT16 width;			//video width
}VIM_VOUT_DCVOCFGINFO_S;

typedef	enum {
	VIDEO_EMB_8BIT = 0,		//  multiplexed 8-bit with embedded sync, bt656 timing
	VIDEO_EMB_10BIT,		//  multiplexed 10-bit with embedded sync, bt656 timing
	VIDEO_EMB_16BIT,		//  demultiplexed 16-bit with embedded sync, bt1120 timing 
	VIDEO_EMB_20BIT,		//  demultiplexed 20-bit with embedded sync, bt1120 timing
	VIDEO_SEP_8BIT,			//  multiplexed 8-bit with separate sync 
	VIDEO_SEP_10BIT,		//  multiplexed 10-bit with separate sync 
	VIDEO_SEP_16BIT,		//  demultiplexed 16-bit with separate sync 
	VIDEO_SEP_20BIT,		//  demultiplexed 20-bit with separate sync 
	VIDEO_MODE_END,			
}VIM_VOUT_VIDEOMODE_E;

typedef	enum {
	VIDEOMODE_EMB_16BIT = 2, 
	VIDEOMODE_SEP_16BIT = 6,
	VIDEOMODE_BT1120_FRAME = 16   // standard bt1120 frame mode output, with 74.25MHz clock
}VIM_VOUT_VIDEOMODE_16BIT_E;

typedef enum
{
	VIDEO_720x480i60 = 0,		// 720x480i 60Hz
	VIDEO_720x576i50,			// 720x576i 50Hz
	VIDEO_960x480i60,			// 960x480i 60Hz
	VIDEO_960x576i50,			// 960x576i 50Hz
	VIDEO_720x480p60,			// 720x480p 60Hz
	VIDEO_720x576p50,			// 720x576p 50Hz
	VIDEO_1280x720p60,			// 1280x720p 60Hz
	VIDEO_1280x720p50,			// 1280x720p 50Hz
	VIDEO_1920x1080i60,			// 1920x1080i 60Hz
	VIDEO_1920x1080i50,			// 1920x1080i 50Hz
	VIDEO_1920x1080p60,			// 1920x1080p 60Hz
	VIDEO_1920x1080p50,			// 1920x1080p 50Hz
	VIDEO_640x480p60,			// 640x480p 60Hz
	VIDEO_800x600p60,			// 800x600p 60Hz
	VIDEO_1024x768p60,			// 1024x768p 60Hz
	VIDEO_1280x1024p60,			// 1280x1024p 60Hz
	VIDEO_1440x900p60,			// 1440x900p 60Hz
	VIDEO_AUTODETECT,				// 1440x900p 60Hz
	VIDEO_FORMAT_END			// format end
}VIM_VOUT_VIDEOFORMAT_E;

typedef enum _SNR_FMT_
{
	SNR_BP,
	SNR_YUV,
}VIM_SIF_SNRFMT_E;


typedef enum TSIFCmdEenum
{
	SIF_GET_TOTAL_FMTNUM =0X0 ,	//获取当前sensor支持的分辨率的种类 和当前分辨率索引
	SIF_GET_FMT_BYINDEX,		//按照索引获取某种分辨率的详细信息
	SIF_SET_FMT_BYINDEX,		//按照索引设置sensor输出某种分辨率
	SIF_GET_TPSENSOR_FMTNUM ,
	SIF_GET_TPSENSORFMT_BYINDEX,
	SIF_SET_TPSENSORFMT_BYINDEX,
	SIF_SET_SENSOR_REG, //sensor reg read
	SIF_GET_SENSOR_REG, //sensor reg write
}VIM_SIF_CMDTYPE_E;

typedef enum VIM_ISP_CMD_E
{
	AE_MODE = 0x0,
	AWB_MODE,
	BRIGHTNESS,
	HUE,
	CONTRAST,
	SATURATION,
	SHARPNESS,
	FLIPMIRROR_MODE,
	IRCUT_MODE,
	IRIS_MODE,
	COLORBLACK_MODE,
	WDR_MODE,
	COLORTYPE_MODE,
	DENOISE_MODE,
	AE_DELAY,
	EIS_MODE,
	LDC_MODE,
	DSS_MODE,
	DEFOG_MODE,
	IRCUT_TH,
	NR3D_LEVEL,
	AWB_AE_RELATION,
	AWB_INTERVAL,
	AWB_DELAY,
	AWB_PARAUPDTSPD,	
	AE_MANUAL_TMMODE,
	MAXFRMRATE,
	TriggerEPACPhase,
	SifFlashSnr,
	DownSizer_Mode,
	SVACDecreaseDetails_LEVEL,
	NR2D,
	NR3D,
	AF_MODE,
	MWB_RGBGain,
	MWB_CCM,
	IRLED_POWER,
	IRLED_NEAR,
	IRLED_FAR,
	IRCUT_AUTO_TYPE,
	TEC_MODE,
	TEC_LEVEL,
	LED_SET,
	SET_DAC,
	SHUTTER_MODE,
	SN_SET,
	AF_MOTOR_DUTY_SET,
	FLASHTRIG,
	FLASHTRIGT,
	FLASHTRIGETRATIO,
	BRIGHTNESS_RETURN,

	ISP_CMD_NUM,

	FLASHTRIGETRATIO_GET=0xe5,
	FLASHTRIGT_GET=0xe6,
	AF_MOTOR_DUTY_GET=0xe7,
	GET_TEC_ALARM_STATE=0xe8,
	SN_GET=0xe9,
	GET_TMP_TOTAL_710_VAL=0xea,
	GET_TMP_TOTAL_SNR_VAL=0xeb,
	GET_SHUTTER_MODE=0xec,
	GET_LED_STATUS=0xed,
	GET_TEC_MODE=0xee,
	GET_TEC_LEVEL=0xef,
	GET_TMP_710_VAL=0xf0,
	GET_TMP_SNR_VAL=0xf1,
	GET_LDR_VALUE=0xf2,
	GET_IRLED_POWER_STATE=0xf3,
	GET_IRLED_NEAR_STATE=0xf4,
	GET_IRLED_FAR_STATE=0xf5,
	GET_CURRENT_WB_MODE=0xf6,
	GET_CURRENT_MWB_GAIN_STATE=0xf7,
	GET_CURRENT_MWB_CCM_STATE=0xf8,
	GET_CURRENT_IRCUT_STATE=0xf9,
	Get_SifFlashSnr=0xfa,
	GET_CURRENT_AE_ET_GAIN = 0xfb,
	GET_GENERAL_DATA = 0xfc,
	GET_CURRENT_DATA = 0xfd,
	DEFAULT_DATA = 0xfe,
	UPDATA_TOTA = 0xff,
}ISP_CMD_E;

typedef enum VIM_DDR_CMD_E
{
    DDR_STAT = 0x7,
}DDR_CMD_E;

//AWB depend on AE
typedef enum VIM_AWB_AE_RELATION_E
{
	AWB_WITHOUT_AE_STABLE = 0x0,
	AWB_WITH_AE_STABLE,
}AWB_AE_RELATION_E;

typedef enum VIM_AE_MANUAL_TMMODE_E
{
    TM_OFF=0x0,
    TM_ON,
    TM_STOP,
}AE_MANUAL_TMMODE_E;

//AE
typedef enum VIM_AE_SHUTTER_MODE_E
{
	AE_50Hz_Auto = 0,
	AE_60Hz_Auto,
	AE_50Hz_Indoor,
	AE_60Hz_Indoor,
	AE_MANUAL_VIM = 0xf,
	AE_INVALID = 0xff
}AE_SHUTTER_MODE_E;

typedef enum VIM_AE_SPEED_MODE_E
{
	AE_S5 = 0,
	AE_S10,
	AE_S25,
	AE_S30,
	AE_S33,
	AE_S40,
	AE_S50,
	AE_S60,
	AE_S100,
	AE_S120,
	AE_S250,
	AE_S500,
	AE_S1000,
	AE_S5000,
	AE_S10000,
	AE_MAXET_MANUAL,
	AE_MAXET_INVALID = 0xff
}AE_MAXET_MODE_E;

typedef enum VIM_AE_YTARGET_LEVEL_E
{	
	AE_LEVEL_NEG_3 = -3,
	AE_LEVEL_NEG_2,
	AE_LEVEL_NEG_1,
	AE_LEVEL_0 = 0,
	AE_LEVEL_POS_1,
	AE_LEVEL_POS_2,
	AE_LEVEL_POS_3,
}AE_YTARGET_LEVEL_E;

typedef struct VIM_AE_MODE_S
{
	AE_SHUTTER_MODE_E AE_Shutter_Mode;
	AE_MAXET_MODE_E AE_MaxET_Mode;	//在50Hz auto和60Hz auto模式中，所包含的模式参数
	UINT32 Exposuretime;	//Shutter manual 模式下配置参数，以微秒为单位的时间值
	UINT32 MaxET;	//Speed manual模式下配置参数，以微秒为单位的时间值
	UINT8 DGain;	//manual模式下的配置参数，digital gain值
	UINT8 AGain;	//manual模式下的配置参数，sensor gain值
	UINT8 MaxDGain;	//50Hz 60Hz auto模式下的配置参数，max sensor gain值
	UINT8 MaxAGain;	//50Hz 60Hz auto模式下的配置参数，max digital gain值
	UINT8 DGainDeci;	//manual模式下的配置参数，global gain值
	UINT8 AGainDeci;    //manual模式下的配置参数，sensor gain值	
}AE_MODE_S;

//FLIP & MIRROR
typedef enum VIM_FLIP_MIRROR_MODE_E
{
	VIM_FLIP_MIRROR_Normal,     //正常
	VIM_FLIP_MIRROR_Flip,       //上下翻转
	VIM_FLIP_MIRROR_Mirror,     //左右翻转
	VIM_FLIP_MIRROR_FlipMirror, //旋转
}FLIP_MIRROR_MODE_E;

//AWB
typedef enum VIM_AWB_MODE_E
{
	AWB_AUTO_1 = 0x0,
	AWB_FLUORESCENT_LAMP = 0x10,
	AWB_INCANDESCENT_LAMP = 0x11,
	AWB_OUTDOOR_SUN = 0x12,
	AWB_OUTDOOR_CLOUDY = 0x13,
	AWB_OUTDOOR_DUSK = 0x14,
	AWB_INDOOR_OFFICE = 0x15,
	AWB_DISABLE = 0x1f,
	AWB_AUTO_2 = 0x20,
	AWB_AUTO_3 = 0x21
}AWB_MODE_E;

//MWB
typedef struct VIM_MWB_RGBGAIN_S
{
	UINT16 RGain;
	UINT16 BGain;
	UINT16 GrGain;
	UINT16 GbGain;
	SINT16 RGainOffset;
	SINT16 BGainOffset;
	SINT16 GrGainOffset;
	SINT16 GbGainOffset;
}MWB_RGBGAIN_S,*pMWB_RGBGAIN_S;

typedef struct VIM_MWB_CCM_S
{
	SINT16 Colmat11;
	SINT16 Colmat12;
	SINT16 Colmat13;
	SINT16 Colmat21;
	SINT16 Colmat22;
	SINT16 Colmat23;
	SINT16 Colmat31;
	SINT16 Colmat32;
	SINT16 Colmat33;
	SINT8 ColmatOffsetR;
	SINT8 ColmatOffsetG;
	SINT8 ColmatOffsetB;
}MWB_CCM_S,*pMWB_CCM_S;

//IR_CUT
typedef enum VIM_IRCUT_MODE_E
{
	IRCUT_AUTO = 0x0,
	IRCUT_MANUAL_DAY,
	IRCUT_MANUAL_NIGHT,
}IRCUT_MODE_E;

typedef enum
{
	IRC_Unblock = 0,
	IRC_Block
}IRC_STATE,*pIRC_STATE;

//Iris
typedef enum VIM_IRIS_MODE_E
{
	IRIS_AUTO_FIXED = 0x0,
	IRIS_AUTO_IRIS,
	IRIS_MANUAL_IRIS_LENS,
	IRIS_AUTO_IRIS_FORCE,
	IRIS_FORCECLOSE,
}IRIS_MODE_E;
//color&gray
typedef enum VIM_COLORBLACK_MODE_E
{
	COLORBLACK_AUTO = 0x0,
	COLORBLACK_MANUAL_COLOR,
	COLORBLACK_MANUAL_BLACK,
}COLORBLACK_MODE_E;
//WDR
typedef enum VIM_DR_MODE_E
{
    DRMODE_DEBUG = 0x01,
    DRMODE_BACKLIGHT=2,
    DRMODE_OE=4,
    DRMODE_WDR=8,
    DRMODE_NORMAL=16,
} DR_MODE_E;    
typedef enum VIM_DR_LEVEL_E
{
	DR_LEVEL_LOW = 0,
	DR_LEVEL_MID,
	DR_LEVEL_HIGH,
} DR_LEVEL_E;
//corlor type
typedef enum VIM_COLOR_TYPE_E
{
	BT601 = 0x0,
	BT709,
}COLOR_TYPE_E;
//denoise
typedef enum VIM_DENOISE_MODE_E
{
	DENOISE_CLOSE = 0x0,
	DENOISE_2D,
	DENOISE_3D,
}DENOISE_MODE_E;

//NR3D
typedef enum VIM_NR3D_LEVEL_E
{
	NR3D_LEVEL_LOW=0,
	NR3D_LEVEL_MID,
	NR3D_LEVEL_HIGH,
}NR3D_LEVEL_E;

//EIS
typedef enum VIM_EIS_FLAG_E
{
	EIS_DISABLE = 0x0,
	EIS_ENABLE
}EIS_FLAG_E;
typedef enum VIM_EIS_MODE_E
{
	EIS_CLOSE = 0x0,
	EIS_UPSCALE,
	EIS_EMBEDED,
}EIS_MODE_E;
//LDC
typedef enum VIM_LDC_MODE_E
{
	LDC_CLOSE = 0x0,
	LDC_SINGLE,
	LDC_2X2,
}LDC_MODE_E;
//DSS
typedef enum VIM_DSS_NUM_E
{
	DSS_CLOSE = 0x0,
	DSS_NUM_2 = 2,
	DSS_NUM_4 = 4,
	DSS_NUM_8 = 8,
	DSS_NUM_16 = 16,
	DSS_NUM_32 = 32,
	DSS_NUM_64 = 64,
	DSS_NUM_128 = 128,
}DSS_NUM_E;
//defog
typedef enum VIM_DEFOG_MODE_E
{
	DEFOG_CLOSE = 0x0,
	DEFOG_LOW,
	DEFOG_MID,
	DEFOG_HIGH,
}DEFOG_MODE_E;

typedef enum _UV_SEL_
{
	Y_HIGH,
	Y_LOW,
}VIM_UV_SEL_E;

typedef enum __VIM_DDR_STAT_E
{
    DDR_ERR = 0x0,
    DDR_READY    
}DDR_MODE_E;

typedef enum _MAXFRMRATE_E
{
    FPS25 = 0x0,
    FPS30,
    FPS50,
    FPS60
}MAXFRMRATE_E;

typedef enum DownSizer_Mode_E
{
	DS_Deft=0x0,
	DS_Max1080P=0x1,
	DS_Max720P=0x2,
	DS_MaxCustmDef=0x03,
}DownSizer_Mode_E;

typedef struct VIM_DS_Size_CustmDef_S
{
	UINT16 DS_Size_Width;
	UINT16 DS_Size_Height;
}DS_Size_CustmDef_S;

//SVAC decrease details
typedef enum VIM_SVAC_DECREASE_DETAILS_LEVEL_E
{
	SVAC_DECREASE_DETAILS_LEVEL_0=0,
	SVAC_DECREASE_DETAILS_LEVEL_1,
	SVAC_DECREASE_DETAILS_LEVEL_2,
	SVAC_DECREASE_DETAILS_LEVEL_3,
	SVAC_DECREASE_DETAILS_LEVEL_4,
	SVAC_DECREASE_DETAILS_LEVEL_5,
	SVAC_DECREASE_DETAILS_LEVEL_6,
	SVAC_DECREASE_DETAILS_LEVEL_7,
	SVAC_DECREASE_DETAILS_LEVEL_8,
	SVAC_DECREASE_DETAILS_LEVEL_9,
	SVAC_DECREASE_DETAILS_LEVEL_10,
}SVAC_DECREASE_DETAILS_LEVEL_E;

//tecooler mode
typedef enum VIM_TEC_MODE_E
{
	TEC_AUTO_OFF = 0x0,
	TEC_AUTO_ON,
}TEC_MODE_E;

//tecooler manual level
typedef enum VIM_TEC_LEVEL_E
{
	TEC_LEVEL_0 = 0x0,
	TEC_LEVEL_1,
	TEC_LEVEL_2,
	TEC_LEVEL_3,
	TEC_LEVEL_4,
	TEC_LEVEL_5,
	TEC_LEVEL_6,
	TEC_LEVEL_7,
	TEC_LEVEL_8,
	TEC_LEVEL_9,
	TEC_LEVEL_10,
	TEC_LEVEL_11,
}TEC_LEVEL_E;

typedef enum VIM_IRCUT_AUTO_TYPE_E
{
         IRCUT_AUTO_CAMERA = 0x0, //相机控制IRCUT
         IRCUT_AUTO_LDR,          //光敏控制IRCUT和IRLED
         IRCUT_AUTO_SOFT,                  //相机控制IRCUT和IRLED
}IRCUT_AUTO_TYPE_E;

typedef enum SHUTTER_STATE
{
	RS_MODE = 0,
	GS_MODE,
	Factory_enterRS,
	Factory_setRScfg,
	Factory_quitRS,
	Factory_enterGS,
	Factory_quitGS,
}VIM_SHUTTER_STATE;

#pragma pack(1)
typedef struct __VIM_ATTRIBUTE__
{
    AE_MODE_S AEMode;
    AWB_MODE_E AWBMode;
    UINT8 BrightnessCoeff;
    UINT8 HueCoeff;
    UINT8 ContrastCoeff;
    UINT8 EdgeCoeff;
    UINT8 SaturationCoeff;
    FLIP_MIRROR_MODE_E FlipMirrorMode;
    IRIS_MODE_E IrisMode;
    IRCUT_MODE_E IRcutMode;
    COLORBLACK_MODE_E ColorBlack;
    DR_MODE_E DRMode;
    DR_LEVEL_E DRLevel;
    COLOR_TYPE_E ColorType;
    DENOISE_MODE_E DenoiseMode;
    UINT32 AEdelay;
    EIS_MODE_E EisMode;
    LDC_MODE_E LdcMode;
    DSS_NUM_E DssNum;
    DEFOG_MODE_E DefogMode;
    UINT8 IRC_TH3;
	NR3D_LEVEL_E NR3DLevel;
	AE_MANUAL_TMMODE_E TmMode;
	AWB_AE_RELATION_E AWBAERelation;
	UINT16 AWBInterval;
    UINT16 AWBDelay;    
    MAXFRMRATE_E MaxFrmRate;
	UINT16 ACPhase;
	DownSizer_Mode_E DownSizer_Mode;
	DS_Size_CustmDef_S DSSizeCustmDef;
	SVAC_DECREASE_DETAILS_LEVEL_E SVACDecreaseDetailsLevel;
	UINT8 NR2D_Coeff;
	UINT8 NR3D_Coeff;
	MWB_RGBGAIN_S MWB_RGBGain;
	MWB_CCM_S MWB_CCM;
	IRCUT_AUTO_TYPE_E IRcutAutoType;
	SINT16 AF_ZoomIndex;
	TEC_MODE_E TECMode;
	TEC_LEVEL_E TECLevel;
	VIM_SHUTTER_STATE ShutterMode;
}VIM_ATTRIBUTE_S, *pVIM_ATTRIBUTE_S;
#pragma pack()

typedef struct __VIM_GENERAL_INFO__{
    UINT16 flag;
    UINT8 sensor_VID;
    UINT16 sensor_ID;
    UINT16 chip_Version;
    UINT16 Ini_Version;
    UINT16 projectSpec_Version;
    UINT8 content_Version;
    UINT8 date_info[4];    
    UINT32 Firmware_Version;
}VIM_GENERAL_INFO, *pVIM_GENERAL_INFO;

typedef enum VIM_GLOBAL_GAIN_FORMAT_E
{
	Ten_Point_Six = 0,
	Nine_Point_Seven,
	Eight_Point_Eight,
	Seven_Point_Nine,
}GLOBAL_GAIN_FORMAT_E;

typedef struct __VIM_CURRENT_AE__{
    UINT32 etus;
    UINT16 gainValue;
    GLOBAL_GAIN_FORMAT_E gainFormat;
}VIM_CURRENT_AE, *pVIM_CURRENT_AE;

typedef enum PADC_IRLED_CTRL
{
	LED_OFF = 0,
	LED_ON,
	LED_UNKNOWN
}VIM_PADC_IRLED_CTRL;

typedef enum
{
	LED_CLOSE = 0,
	LED_OPEN
}LED_STATE;


//void VIM_ISP_SetBl(VIM_ISP_BLACKLEVEL bl);
//void VIM_ISP_SetSc(VIM_ISP_SEAMCORRECTION sc);
//void VIM_ISP_SetLinear(VIM_ISP_LINEARIZATION lin);
//void VIM_ISP_SetGlobalGain(VIM_ISP_GLOBALGAIN gg);
//void VIM_ISP_SetRawRGBGain(VIM_ISP_RAWRGBGAIN rrg);
//void VIM_ISP_SetPaaCfa(VIM_ISP_PAA_CFA pc);
//
//void VIM_ISP_SetClipPoint(VIM_ISP_CLIPPOINT cp);
//void VIM_ISP_SetCCM(VIM_ISP_CCM ccm);
//void VIM_ISP_SetCE(VIM_ISP_CE ce);
//void VIM_ISP_SetCAC(VIM_ISP_CAC cac);
//void VIM_ISP_SetGamma(VIM_ISP_GAMMA gamma);
//void VIM_ISP_SetEE(VIM_ISP_EE ee);

//isp
VIM_RESULT VIM_ISP_SetAEMode(AE_MODE_S AEMode);
VIM_RESULT VIM_ISP_SetIris(IRIS_MODE_E IrisMode);
VIM_RESULT VIM_ISP_SetIrCut(IRCUT_MODE_E IRcutMode);
VIM_RESULT VIM_ISP_SetIrCutTh(UINT32 IRcutTh);
VIM_RESULT VIM_ISP_SetAWBMode(AWB_MODE_E AWBMode);
VIM_RESULT VIM_ISP_SetBrightness(UINT32 BrightnessCoeff);
VIM_RESULT VIM_ISP_SetHue(UINT32 HueCoeff);
VIM_RESULT VIM_ISP_SetContrast(UINT32 ContrastCoeff);
VIM_RESULT VIM_ISP_SetSaturation(UINT32 SaturationCoeff);
VIM_RESULT VIM_ISP_SetSharpness(UINT32 EdgeCoeff);
VIM_RESULT VIM_ISP_SetFlipMirror(FLIP_MIRROR_MODE_E FlipMirrorMode);
VIM_RESULT VIM_ISP_SetColorType(COLOR_TYPE_E ColorType);
VIM_RESULT VIM_ISP_SetDenoiseMode(DENOISE_MODE_E DenoiseMode, NR3D_LEVEL_E DenoiseLevel);
VIM_RESULT VIM_ISP_SetDRMode(DR_MODE_E DRMode, DR_LEVEL_E DRLevel);
VIM_RESULT VIM_ISP_SetAEDelay(UINT32 AEDelay);
VIM_RESULT VIM_ISP_SetEISMode(EIS_MODE_E EisMode);
VIM_RESULT VIM_ISP_SetLDCMode(LDC_MODE_E LdcMode);
VIM_RESULT VIM_ISP_SetDSSMode(DSS_NUM_E DssNum);
VIM_RESULT VIM_ISP_SetDefogMode(DEFOG_MODE_E DefogMode);
VIM_RESULT VIM_ISP_SetEISEnable(EIS_FLAG_E EisMode);
VIM_RESULT VIM_ISP_SetAeManualTM(AE_MANUAL_TMMODE_E TmMode);
VIM_RESULT VIM_ISP_SetColorBlack(COLORBLACK_MODE_E ColorBlack);
VIM_RESULT VIM_ISP_GetCurrentAEETGain(UINT32 *etus, UINT16 *gainValue, UINT8 *gainFormat);
VIM_RESULT VIM_ISP_SetAWBAERelation(AWB_AE_RELATION_E value);
VIM_RESULT VIM_ISP_SetAWBCalcInterval(UINT16 interval);
VIM_RESULT VIM_ISP_SetAWBUpdateSpeed(UINT8 speed);
VIM_RESULT VIM_ISP_SetAWBDelay(UINT16 AWBDelay);
VIM_RESULT VIM_ISP_SetTriggerEPACPhase(UINT16 acphase);
VIM_RESULT VIM_ISP_SetMaxFrmRate(MAXFRMRATE_E fps_mode);

//vout
void VIM_VOUT_SetPath(VIM_VOUT_IPPSRC_E eippsrc, VIM_VOUT_DCVOSRC_E edcvosrc);
void VIM_VOUT_SetHighlight(UINT16 winen, UINT8 yvalue, UINT8 ythrd);
void VIM_VOUT_SetYUVAdjustMode(VIM_VOUT_YUVADJUSTMODE_E emode);
void VIM_VOUT_EnableHammingEnc();
void VIM_VOUT_DisableHammingEnc();
VIM_RESULT VIM_VOUT_SetCropDs(VIM_VOUT_CROPDSCFG_S scfg);

VIM_RESULT VIM_VOUT_SetP2iMode(VIM_VOUT_P2IMODE_E emode, VIM_VOUT_BITSWIDTH_E ebitswidth);
void VIM_VOUT_CancelP2i(VIM_VOUT_P2IMODE_E emode);
VIM_RESULT VIM_VOUT_GetP2iImage(UINT16 *imageWidth, UINT16 *imageHeight);
VIM_RESULT VIM_VOUT_SetP2iImage(UINT16 imageWidth, UINT16 imageHeight);
VIM_RESULT VIM_VOUT_SetFrameDrop(UINT8 inrate, UINT8 outrate);
void VIM_VOUT_CancelFrameDrop();

VIM_RESULT VIM_VOUT_SetDcvoOutput(VIM_VOUT_VIDEOFORMAT_E efmt, VIM_VOUT_VIDEOMODE_E emode); 
VIM_RESULT VIM_VOUT_SetPureColorTestPattern(VIM_VOUT_VIDEOFORMAT_E efmt, VIM_VOUT_VIDEOMODE_E emode, VIM_VOUT_COLOR_S scolor);
VIM_RESULT VIM_VOUT_SetColorBarTestPattern(VIM_VOUT_VIDEOFORMAT_E efmt, VIM_VOUT_VIDEOMODE_E emode);
VIM_RESULT VIM_VOUT_SetDcvoOutputMode(VIM_VOUT_VIDEOMODE_16BIT_E emode);

VIM_RESULT VIM_VOUT_SetPvc(char *filePath, VIM_VOUT_PVCPARAM_S sparam);
void VIM_VOUT_CancelPvc();

VIM_RESULT VIM_VOUT_SetOsdFromHost(VIM_VOUT_FONTINFO_S sfinfo[FONT_MAX_NUM], VIM_VOUT_OSDCHNINFO_S schninfo[OSD_CHN_NUM], VIM_VOUT_OSDCFG_S sosdcfg);
VIM_RESULT VIM_VOUT_SetOsdFromFlash(VIM_VOUT_OSDCHNINFO_S schninfo[OSD_CHN_NUM], VIM_VOUT_OSDCFG_S sosdcfg);
void VIM_VOUT_CancelOsd();

VIM_RESULT VIM_VOUT_ConvertFont(VIM_VOUT_FONTCONVERTPARAM_S sparam);
VIM_RESULT VIM_VOUT_BurnFontToFlash(VIM_VOUT_FONTINFO_S sfinfo[FONT_MAX_NUM]);
VIM_RESULT VIM_VOUT_UvSelection_set(VIM_UV_SEL_E mode);
VIM_RESULT VIM_VOUT_UvSelection_get(UINT8* value);

VIM_RESULT VIM_VOUT_SetMaxFrmRate(VIM_VOUT_VIDEOMODE_16BIT_E emode, MAXFRMRATE_E fps_mode);

//SIF
VIM_RESULT VIM_SIF_GetResolutionNum(UINT8* resnum,UINT8* currentindex);
VIM_RESULT VIM_SIF_GetResolution(UINT8 index,UINT8* yuvbp,UINT32* width,UINT32* height);
VIM_RESULT VIM_SIF_SetResolution(UINT8 index);

////Attribute
VIM_RESULT VIM_UpdateAttribute(void);
VIM_RESULT VIM_GetCurrentAttribute(UINT8* ptr);
VIM_RESULT VIM_SetDefaultAttribute(void);
VIM_RESULT VIM_GetGeneralAttribute(pVIM_GENERAL_INFO pGeneral_info);

//ddr
VIM_RESULT VIM_GetDDRStat(UINT8 * ptr);

//nr
VIM_RESULT VIM_ISP_SetNR2D(UINT32 NR2D_Coeff);
VIM_RESULT VIM_ISP_SetNR3D(UINT32 NR3D_Coeff);

//manual WB
VIM_RESULT VIM_ISP_SetMWB_RGBGain(MWB_RGBGAIN_S MWB_RGBGain_val);
VIM_RESULT VIM_ISP_SetMWB_CCM(MWB_CCM_S MWB_CCM_val);
VIM_RESULT VIM_ISP_GetCurrentMWBGain(pMWB_RGBGAIN_S pMWB_RGBGain_val);
VIM_RESULT VIM_ISP_GetCurrentMWBCCM(pMWB_CCM_S pMWB_CCM_val);
VIM_RESULT VIM_ISP_GetCurrentWBMode(UINT8 *pWBMode);

//ircut
VIM_RESULT VIM_ISP_GetCurrentIRCUT(UINT8 *pIRCState);

//IR LED
VIM_RESULT VIM_GPIO_IRLED_Power_Set(VIM_PADC_IRLED_CTRL val);
VIM_RESULT VIM_GPIO_IRLED_Near_Set(VIM_PADC_IRLED_CTRL val);
VIM_RESULT VIM_GPIO_IRLED_Far_Set(VIM_PADC_IRLED_CTRL val);
VIM_RESULT VIM_GPIO_IRLED_Power_Get(UINT8 *pIRPowerState);
VIM_RESULT VIM_GPIO_IRLED_Near_Get(UINT8 *pIRNearState);
VIM_RESULT VIM_GPIO_IRLED_Far_Get(UINT8 *pIRFarState);
VIM_RESULT VIM_LDR_Val_Get(UINT8 *pLDRval);

//Temperature
VIM_RESULT VIM_TMP_710_Val_Get(SINT16 *pTMPval);
VIM_RESULT VIM_TMP_SNR_Val_Get(SINT16 *pTMPval);
VIM_RESULT VIM_TMP_TOTAL_710_Val_Get(SINT16 *pTMPval);
VIM_RESULT VIM_TMP_TOTAL_SNR_Val_Get(SINT16 *pTMPval);

//ircut auto type select
VIM_RESULT VIM_IRCUT_AutoType_Set(IRCUT_AUTO_TYPE_E val);

//tecooler
VIM_RESULT VIM_ISP_Set_TEC_Mode(TEC_MODE_E TECMode);
VIM_RESULT VIM_ISP_Set_TEC_Level(TEC_LEVEL_E level);
VIM_RESULT VIM_ISP_Get_TEC_Mode(UINT8 *pTECMode);
VIM_RESULT VIM_ISP_Get_TEC_Level(UINT8 *plevel);

//LED control
VIM_RESULT VIM_ISP_Set_LED_State(LED_STATE LEDState);
VIM_RESULT VIM_ISP_Get_LED_State(UINT8 *pLEDState);

//shutter mode
VIM_RESULT VIM_ISP_ShutterMode_Set(VIM_SHUTTER_STATE ShutterState);
VIM_RESULT VIM_ISP_ShutterMode_Get(UINT8 *pShutterState);

VIM_RESULT VIM_ISP_WriteDac(UINT8 channel,UINT8 hval,UINT8 lval);

//serial number
VIM_RESULT VIM_ISP_SN_SET(UINT8 *pSN_buf);
VIM_RESULT VIM_ISP_SN_GET(UINT8 *pSN_buf);

//tec alarm
VIM_RESULT VIM_ISP_GET_TEC_ALARM_STATE(UINT8 *pTecAlarmState);

//AF MOTOR DUTY
VIM_RESULT	VIM_AF_MotorDuty_Set(UINT8 Duty);
VIM_RESULT	VIM_AF_MotorDuty_Get(UINT8* pDuty);

//GS Flash Trigger
//0-1
VIM_RESULT	VIM_GS_FlashTrig_Set(UINT8 FlashTrig_Flag);
//FlashTrigTime: us
VIM_RESULT	VIM_GS_FlashTrigTime_Set(UINT16 FlashTrigTime);	
VIM_RESULT	VIM_GS_FlashTrigTime_Get(UINT16* pFlashTrigTime);
//FlashTrigEtRatio: (denominator<<8) + numerator
VIM_RESULT	VIM_GS_FlashTrigEtRatio_Set(UINT16 FlashTrigEtRatio);	
VIM_RESULT	VIM_GS_FlashTrigEtRatio_Get(UINT16* pFlashTrigEtRatio);

VIM_RESULT VIM_ISP_Brightness_Return(UINT8 BrightnessCoeff);



// debug function
VIM_RESULT VIM_Debug_1(UINT32 Set_Value, UINT32 *value);
VIM_RESULT VIM_Debug_2(UINT32 Set_Value, UINT32 value);
VIM_RESULT VIM_Debug_3(void);
VIM_RESULT VIM_Debug_4(void);
VIM_RESULT VIM_Debug_5(void);
VIM_RESULT VIM_Debug_6(void);
VIM_RESULT VIM_Debug_7(void);
VIM_RESULT VIM_Debug_8(void);
VIM_RESULT VIM_Debug_9(void);
VIM_RESULT VIM_Debug_10(void);


//add 20150504: for AF-----------------------------------------------------start
void VIM_ISP_SetAfHStart(UINT8* pStart);
void VIM_ISP_GetAfHStart(UINT8* pStart);
void VIM_ISP_SetAfVStart(UINT8* pStart);
void VIM_ISP_GetAfVStart(UINT8* pStart);
void VIM_ISP_SetAfWinWt(UINT8* pWeightMux);
void VIM_ISP_GetAfWinWt(UINT8* pWeightMux);
void VIM_ISPAuto_SetAfBpSubFilterSel(UINT8 FilterSel);
void VIM_ISPAuto_SetAfBpSubRatioSel(UINT8 subRatio);
void VIM_ISPAuto_SetAfStatWinUnitSel(UINT8 WinUnitSel);
void VIM_ISPAuto_GetAfStatWinUnitSel(UINT8* WinUnitSel);
void VIM_ISPAuto_SetAfOffset(UINT16 Hoffset,UINT16 Voffset);
void VIM_ISPAuto_GetAfOffset(UINT16* Hoffset,UINT16* Voffset);

UINT32 VIM_ISPAuto_GetEdgeBpWinSum(UINT8 i);
void VIM_ISP_GetAfBpWinSum(UINT32* pWinSum);
UINT32 VIM_ISPAuto_GetEdgeSubSum(void);

void VIM_ISPAuto_SetAfStatEn(UINT8 Enable);
void VIM_ISPAuto_SetAfBpStatEn(UINT8 Enable);
void VIM_ISPAuto_AfUpdate(void);
UINT32 VIM_ISPAuto_GetEdgeSum(void);
UINT32 VIM_ISPAuto_GetEdgeSumBP(void);
//add 20150504: for AF--------------------------------------------end


////////////////////////////////////////
//xierong 20141013 add for tools request start

#define	LSNUMMAX 10

typedef struct VIM_ADAPT_EN
{
	UINT8 NRRAW_AdaptEn;//0x20003000
	UINT8 NR2D_AdaptEn;
	UINT8 NR3D_AdaptEn;
	UINT8 TM_AdaptEn;
	UINT8 Gamma_AdaptEn;//0x20003004
	UINT8 Saturation_AdaptEn;
	UINT8 CE_AdaptEn;
	UINT8 EE_AdaptEn;
	UINT8 EEPropty_AdaptEn;
	UINT8 EEFilt_AdaptEn;
	UINT8 UvNr_AdaptEn;
	UINT8 DustNr_AdaptEn;
	UINT8 ADPD_AdaptEn;
	UINT8 PAA_AdaptEn;//0x2000300d
	UINT8 Contrast_AdaptEn;//0x2000300e
	UINT8 CCM1x_AdaptEn;//0x2000300f
}ADAPT_En, *PADAPT_EN;
typedef struct _RGBGAMMA_PARAM
{
	UINT16 RGBGAMMA_DAY[17];//0x20003020
	UINT16 RGBGAMMA_NIGHT1[17];//0x20003042
	UINT16 RGBGAMMA_NIGHT2[17];//0x20003064
}RGBGAMMA_PARAM, *PRGBGAMMA_PARAM;

typedef struct _AE_WR_PARAM
{
	UINT8	AEWrVal[32];
	UINT8	AEWrFlg[32];
	UINT8	ETWrNum;
	UINT8	FLETBatchNum;
	UINT8	ETBatchNum;
	UINT8	SGWrNum;
	UINT8	SGBatchNum;	
}AE_WR_PARAM, *PAE_WR_PARAM;
typedef struct _AE_PARAM//0x200030f0
{
	UINT8	ET_IncreaseSGDflt;//0x200030f0
	UINT8	SGmin;
	UINT16	SGmax1;
	UINT8	MaxDGLimit;
	UINT8	MaxAGLimit;
	UINT8	SGDelta;
	UINT8	NeedAdjustSG;
	UINT8	AE_PLL1;
	UINT8	AE_PLL2;
	UINT16	OverheadTime;
	UINT16	RowPixelNum;
	UINT16	MaxFRRowNum25;
	UINT16	MaxFRFrmLen25;
	UINT16	MaxFRRowNum30;
	UINT16	MaxFRFrmLen30;
	UINT8	ETRegValDelta;
	UINT8	ETRegValRatio;
	UINT32	FrmLenOR;
	UINT8	ChNum;
	UINT8	AddrWidth;
	UINT8	RegWidth;
	AE_WR_PARAM AEWrSt[2];
	UINT32	AEInitET;
	UINT16	AEInitDG;
	UINT16	AEInitSG;
	UINT8	YtargetLowNorm;
	UINT8	YtargetHighNorm;
	UINT8	MaxBrightYtTimes;
	UINT32	YtETTH;
	UINT16	YtDGTH;
	UINT16	YtSGTH;
	UINT16	YtDGTH_MaxET;
	UINT8	YtSGTH_MaxET;
	UINT8	YtTHDelta;
	UINT8	YtRatioN_Night;
	UINT8	YtRatioD_Night;
	UINT8	IncSpeed;
	UINT8	DecSpeed;
	UINT8	GainDelayFL;
	UINT8	GainDelayNFL;
	UINT8	SENSORMode;//0:WDR for WDR sensor, 1:WDR for normal sensor
	UINT8	AEStableTH;//0-255,frame numbers
	UINT16	WDR_MINETLINE;
	UINT16	WDR_MAXETLINE;
	UINT8	WDRMaxAGLimit;	
	UINT8	WDRHighRatioTH;
	UINT8	WDRLowRatioTH;
	UINT8	WDRHPTHHigh;
	UINT8	WDRHPTHHigher;
	UINT8	WDRHPTHLow;
	UINT8	WDRHPTHLower;
	UINT8	WDRHPTHStepLim;
	UINT8	WDRLPTHHigh;
	UINT8	WDRLPTHLow;
	UINT8	WDRIncStepLimit;
	UINT8	WDRDecStep;
	UINT16	BLHighRatioTH;
	UINT16	BLLowRatioTH;
	UINT8	BLLPTHLow;
	UINT8	BLLPTHLower;
	UINT8	BLLPTHHigh;
	UINT8	BLLPTHHigher;
	UINT8	BLHPTHLow;
	UINT8	BLHPTHHigh;
	UINT8	BLIncStep;
	UINT8	DSSEn;
	UINT8	YtRatioN_SH;
	UINT8	YtRatioD_SH;
	UINT8	YtargetHigherTH;
	UINT8	YtargetLowerTH;
	UINT16	YmMinL;
	UINT16	YmMinH;
	UINT32    READPIN_ETusThr_MaxFrmRate25;
	UINT32    READPIN_ETusThr_MaxFrmRate50;
	UINT16    READPIN_ETlineThr_MaxFrmRate25;
	UINT16    READPIN_ETlineThr_MaxFrmRate30;
	UINT16    READPIN_ETlineThr_MaxFrmRate50;
	UINT16    READPIN_ETlineThr_MaxFrmRate60;
	UINT8	YmMaxTH;
	UINT8  WDRYmMinRatioN_Night;
	UINT8  WDRYmMinRatioD_Night;  
}AE_PARAM, *PAE_PARAM;

typedef struct _IRIS_PARAM
{
	UINT16	IrisCAETime;//0X200031f0
	UINT16	IrisOAETime;
	UINT8	IrisRstDelay;
	UINT8	Iris_Ratio_Open;	
	UINT16	Iris_Ratio_Close;						
	UINT8	Iris_Ratio_Reset;
	//......
}IRIS_PARAM, *PIRIS_PARAM;

typedef struct _IRC_PARAM
{
	UINT32	MaxUnbLux;//0X20003210
	UINT32	MinUnbLux;
	UINT8	IRCDelay1;
	UINT8	IRCDelay2;
	//UINT8	IRCDelay3;
	UINT8	LDRState; //光敏电阻状态0:close;1:open;2:broken
	//	UINT8	LDRUsefulTH;
	UINT8     LDRU2BLightRatio; //光敏电阻控制IRC 时切日片的光照与切夜片光照的倍数关系
	UINT8	IRLightType;//红外补光灯类型0:一个灯，1:两个灯	
	UINT16	LDRMaxUnbLux; //光敏控制IRC时切夜片最大光照
	UINT16	LDRMinUnbLux;//光敏控制IRC时切夜片最小光照*10
	UINT8	IRCU2BCalcDelay;//若IR cut正处于日片切夜片过程中，即IR cut刚切成夜片，计算夜片切回日片的阈值的延时，也就是AE稳定后延时多久再计算切回日片的门限，单位为帧
}IRC_PARAM, *PIRC_PARAM;

typedef struct _ADAPT_PARAM
{
	UINT16 NRRAW_GAINStep1;//0x20003230
	UINT16 NRRAW_GAINStep2;	
	UINT16 NRRAW_NoiseTH11;
	UINT16 NRRAW_NoiseTH12;
	UINT16 NRRAW_TimexGain1;		
	UINT16 NRRAW_TimexGain2;		
	UINT16 NRRAW_NoiseTH21;
	UINT16 NRRAW_NoiseTH22;
	UINT16 NRRAW_TimeTH;	
	UINT8 NRRAW_YtRef;	

	UINT16 NR2D_GAINStep1;		
	UINT16 NR2D_GAINStep2;		
	UINT8 NR2D_NoiseTH1;
	UINT8 NR2D_NoiseTH2;
	UINT8 NR2D_Ratio;

	UINT16 NR3D_GAINStep1;		
	UINT16 NR3D_GAINStep2;	
	UINT8 NR3D_MThL1;	
	UINT8 NR3D_MThH1;	
	UINT8 NR3D_MThL2;	
	UINT8 NR3D_MThH2;	
	UINT8 NR3D_MThH2_MID;	
	UINT8 NR3D_MThL2_HIGH;	
	UINT8 NR3D_MThH2_HIGH;	

	UINT16 TM_Gain;	
	UINT16 TM_MaxET;
	UINT8 TM_Div1[12];	
	UINT8 TM_Div2[12];	
	UINT8 TM_Max1[12];	
	UINT8 TM_Min1[12];	
	UINT8 TM_Max2[12];	
	UINT8 TM_Min2[12];	
	UINT8 TM_Max3[12];	
	UINT8 TM_Min3[12];	
	UINT8 TM_HistYIdx[12];
	
	UINT16 Saturation_GAINStep1;	
	UINT16 Saturation_GAINStep2;	
	UINT16 Saturation_GAINStep3;	
	UINT16 Saturation_GAINStep4;
	UINT8 Saturation_TH1;			
	UINT8 Saturation_TH2;
	UINT8 Saturation_TH3;

	UINT16 CE_GAINStep1;	
	UINT16 CE_GAINStep2;	
	UINT16 CE_GAINStep3;	
	UINT8 CE_MLP1;	
	UINT8 CE_MHP1;	
	UINT8 CE_MLP2;	
	UINT8 CE_MHP2;	
	UINT8 CE_MLP3;	
	UINT8 CE_MHP3;	
	UINT8 CE_MLP4;	
	UINT8 CE_MHP4;

	UINT16 EE_Times;
	UINT16 EE_GAINStep1;		
	UINT16 EE_GAINStep2;	
	UINT8 EE_Adaptth_TH1;		
	UINT8 EE_Adaptth_TH2;		
	UINT16 EE_GAINStep3;	
	UINT16 EE_GAINStep4;	
	UINT16 EE_Cof_Edge_TH1;		
	UINT16 EE_Cof_Edge_TH2;
	UINT16 EE_GAINStep5;		
	UINT16 EE_GAINStep6;
	UINT8 Sharp_TH1;	
	UINT8 Sharp_TH2;	
	UINT16 EE_FiltGain1;	
	UINT16 EE_FiltGain2;	
	SINT8 EE_Filter1[15];	
	SINT8 EE_Filter2[15];	
	
	UINT16 UvNr_GAINStep1;	
	UINT16 UvNr_GAINStep2;	
	UINT8 UvNr_TH1;	
	UINT8 UvNr_TH2;	

	UINT16 DustNR_GAINStep1;
	UINT16 DustNR_GAINStep2;
	UINT8 DustNR_TH1;	
	UINT8 DustNR_TH2;	

	UINT16 ADPD_TimesxGain1;		
	UINT16 ADPD_TimesxGain2;
	UINT16 ADPD_TH1;			
	UINT16 ADPD_TH2;		

	UINT16 PAAGainStep;	
	UINT8 PAAMode1;	
	UINT8 PAAMode2;
	UINT16 PAA_GAINStep1;			
	UINT16 PAA_GAINStep2;			
	UINT16 PAA_TH1;				
	UINT16 PAA_TH2;	

	UINT8 ShortETLine_En;
	UINT8 ShortETLine1;	
	UINT8 ShortETLine2;	

	UINT16 ADPD_TH3;	
	UINT16 ADPD_GainTH;
	UINT8  ADPD_YTH;

	UINT8  SC_AdaptEn;
	UINT16 SC_Times1;
	UINT16 SC_Times2;
	UINT16 SC_Times3;
	UINT16 SC_Gain1_TH1;			
	UINT16 SC_Gain2_TH1;			
	UINT16 SC_Gain1_TH2;			
	UINT16 SC_Gain2_TH2;			
	UINT16 SC_Gain1_TH3;			
	UINT16 SC_Gain2_TH3;	

	UINT8 AntiBleedingDis;
	UINT8 AntiBleedDisTH1;
	UINT8 AntiBleedDisTH2;
	UINT16 Antibleed_GainTH;
	
	UINT16 Saturation_Times1;
	UINT16 Saturation_Times2;
	UINT8 Saturation_TH4;

	UINT16 Contrast_Times1;
	UINT16 Contrast_Times2;	
	UINT8 Contrast_TH1;
	UINT8 Contrast_TH2;

	UINT8 TM_Div1_YTSH[4];
	UINT8 TM_Div2_YTSH[4];
	UINT8 TM_Max1_YTSH[4];
	UINT8 TM_Min1_YTSH[4];
	UINT8 TM_Max2_YTSH[4];
	UINT8 TM_Min2_YTSH[4];
	UINT8 TM_Max3_YTSH[4];
	UINT8 TM_Min3_YTSH[4];
	UINT8 TM_HistYIdx_YTSH[4];
	
	UINT8 SCBlackLevel_tmpare_En;
	UINT32 SCBlackLevel_Times1;// 1/250s
	UINT32 SCBlackLevel_Times2;// 1/25s
	UINT32 SCBlackLevel_Times3;// 1/2s
	UINT16 SensorTempareture_Th1;// 1/250s
	UINT16 SensorTempareture_Th2;// 1/250s
	UINT16 SensorTempareture_Th3;// 1/25s
	UINT16 SensorTempareture_Th4;// 1/25s
	UINT16 SensorTempareture_Th5;// 1/25s
	UINT16 SensorTempareture_Th6;// 1/2s
	UINT16 SensorTempareture_Th7;// 1/2s

	UINT16 NRRAW_GAINStep3;
	UINT16 NRRAW_TimexGain3;
	UINT16 NRRAW_NoiseTH13;
	UINT16 NRRAW_NoiseTH23;

	UINT16 Saturation_Times3;
	UINT8  Saturation_TH5;

	UINT16 SensorTempareture_Th8;// 1/2s
	UINT8 SCBlackLevel_Offset1;// 1/250s
	UINT8 SCBlackLevel_Offset2;// 1/250s
	UINT8 SCBlackLevel_Offset3;// 1/25s
	UINT8 SCBlackLevel_Offset4;// 1/25s
	UINT8 SCBlackLevel_Offset5;// 1/25s
	UINT8 SCBlackLevel_Offset6;// 1/2s
	UINT8 SCBlackLevel_Offset7;// 1/2s
	UINT8 SCBlackLevel_Offset8;// 1/2s

	UINT16 SVACDecreaseDetails_RawNR_Step;
	UINT8 SVACDecreaseDetails_Sharp_Step;
	UINT8 SVACDecreaseDetails_AdaptTH_Step;

	UINT16 HighSatCCM_TimesTH;	
	UINT16 HighSatCCM_TimesxGain1;	
	UINT16 HighSatCCM_TimesxGain2;	
	UINT16 HighSatCCM_TimesxGain3;	
	UINT16 HighSatCCM_TimesxGain4;	
	UINT8 HighSatCCM_TH1;			
	UINT8 HighSatCCM_TH2;	
	UINT8 HighSatCCM_TH3;			
	UINT8 HighSatCCM_TH4;

	UINT16 Contrast_GAINStep1;
	UINT16 Contrast_GAINStep2;	
	UINT8 Contrast_TH3;
	UINT8 Contrast_TH4;

	UINT16 NR2D_TimeTH;
	UINT16 NR2D_TimexGain1;	
	UINT16 NR2D_TimexGain2;	
	UINT8 NR2D_HighLight_Th;

	UINT16 NR3D_TimeTH;
	UINT32 NR3D_TimexGain1;	
	UINT32 NR3D_TimexGain2;
	UINT32 NR3D_TimexGain3;
	UINT16 NR3D_GAINStep3;
	UINT8 NR3D_HighLight_MThL;
	UINT8 NR3D_HighLight_MThH;
	UINT8 NR3D_Ratio;
	UINT16 NR3D_MaxEtTH1;
	UINT16 NR3D_MaxEtTH2;
	UINT8 NR3D_MaxEtRatio;
	UINT8 NR3D_AttributeRatioN;
	UINT8 NR3D_AttributeRatioD;
	UINT8 NR3D_40ms_MaxMThLRatioN;
	UINT8 NR3D_40ms_MaxMThLRatioD;
	UINT8 NR3D_WDR_MaxMThLRatioN;
	UINT8 NR3D_WDR_MaxMThLRatioD;

	UINT8 NRRAWwith3D_MThH_Step1;
	UINT8 NRRAWwith3D_MThH_Step2;
	UINT8 NRRAWwith3D_MThH_Step3;
	UINT8 NRRAWwith3D_MThH_Step4;
	UINT8 NRRAWwith3D_Ratio1;
	UINT8 NRRAWwith3D_Ratio2;
	UINT8 NRRAWwith3D_Ratio3;

	UINT8 NRRAWwith3D_MThH_Step5;
	UINT8 NRRAWwith3D_MThH_Step6;
	UINT8 NRRAWwith3D_MThH_Step7;
	UINT8 NRRAWwith3D_MThH_Step8;
	UINT8 NRRAWwith3D_Ratio4;
	UINT8 NRRAWwith3D_Ratio5;
	UINT8 NRRAWwith3D_Ratio6;

	UINT16 ADPD_TH4;

	UINT8 AWB1_Saturation_TH1;			
	UINT8 AWB1_Saturation_TH2;
	UINT8 AWB1_Saturation_TH3;
	UINT8 AWB1_Saturation_TH4;
	UINT8 AWB1_Saturation_TH5;

	UINT16 Enable_1xCCM_MaxETTH;
	UINT8 Enable_1xCCM_MaxETRatio;
	UINT16 Enable_1xCCM_Gain;
	UINT16 Disable_1xCCM_Gain;
	UINT8 SaturationRegValue_1xCCM;
	UINT16 WDR_BL_Enable_1xCCM_Gain;
	UINT16 WDR_BL_Disable_1xCCM_Gain;
	UINT8 WDR_BL_SaturationRegValue_1xCCM;

	UINT8 BL_Ymean1;
	UINT8 BL_Ymean2;
	UINT8 CE_MLP5;
	UINT8 CE_MHP5;
	UINT8 CE_MLP6;
	SINT16 BL_base;
	UINT8 BL1;
	UINT8 BL2;
	
	UINT8 MAXGain;
}ADAPT_PARAM, *PADAPT_PARAM;

typedef struct _WDRMODE_PARAM
{
	UINT8 YtRatioN_Day[10];//0x20003440
	UINT8 YtRatioD_Day[10];
	UINT8 YtDelta_Day[10];

	UINT8 WDRTM_Div1[3];
	UINT8 WDRTM_Div2[3];
	UINT8 WDRTM_Max1[3];
	UINT8 WDRTM_Min1[3];
	UINT8 WDRTM_Max2[3];
	UINT8 WDRTM_Min2[3];
	UINT8 WDRTM_Max3[3];
	UINT8 WDRTM_Min3[3];
	UINT8 WDRTM_HistYIdx[3];

	UINT16 WDRGAMMA_LOW[17];
	UINT16 WDRGAMMA_MID[17];
	UINT16 WDRGAMMA_HIGH[17];

	UINT16 WDRDPDThd;

	UINT16 WDRDGLimit;
	
	UINT16 WDRRawNRThLimL1;
	UINT16 WDRRawNRThLimH1;
	UINT16 WDRRawNRThLimL2;
	UINT16 WDRRawNRThLimH2;
	UINT16 WDRRawNRThLimL3;
	UINT16 WDRRawNRThLimH3;



	UINT8 WDRMID_SatuPara;
	UINT8 WDRHIGH_SatuPara;

	UINT8 BLTM_Div1[3];
	UINT8 BLTM_Div2[3];
	UINT8 BLTM_Max1[3];
	UINT8 BLTM_Min1[3];
	UINT8 BLTM_Max2[3];
	UINT8 BLTM_Min2[3];
	UINT8 BLTM_Max3[3];
	UINT8 BLTM_Min3[3];
	UINT8 BLTM_HistYIdx[3];

	UINT8 WDRLOW_SatuPara;
	UINT8 WDRMinAGLimit;

	UINT8 WDR3DNRMThLLim1; 
	UINT8 WDR3DNRMThLLim2; 
	UINT8 WDR3DNRMThLLim3;

	UINT16 WDR_BL_TM_Gain;
	UINT16 WDRDGLimit2;
	UINT16 WDRDGLimit3;
	UINT8 WDR_BL_TM_Div1[12];
	UINT8 WDR_BL_TM_Div2[12];
	UINT8 WDR_BL_TM_Max1[12];
	UINT8 WDR_BL_TM_Min1[12];
	UINT8 WDR_BL_TM_Max2[12];
	UINT8 WDR_BL_TM_Min2[12];
	UINT8 WDR_BL_TM_Max3[12];
	UINT8 WDR_BL_TM_Min3[12];
	UINT8 WDR_BL_TM_HistYIdx[12];

	UINT16	WDRYmMinL1;
	UINT16	WDRYmMinH1;
	UINT16	WDRYmMinL2;
	UINT16	WDRYmMinH2;
	UINT16	WDRYmMinL3;
	UINT16	WDRYmMinH3;

	UINT8  YmMaxTH1;
	UINT8  YmMaxTH2;
	UINT8  YmMaxTH3;
}WDRMODE_PARAM,*PWDRMODE_PARAM;

typedef struct _CSSDEF_PARAM
{
    UINT8 SaturationRegDeft;//0x200035c0
    UINT8 ContrastRegDeft;
    UINT8 EdgeRegDeft;
}CSSDEF_PARAM, *PCSSDEF_PARAM;

typedef struct _AWB_PARAM 
{
	UINT16	RGainMin;//0x200035e0
	UINT16	RGainMax;
	UINT16	BGainMin;
	UINT16	BGainMax;
	UINT8	ParaUpdtSpd;
	UINT16	GainDiffThrLock;
	UINT16	GainDiffThrUnLock;
	UINT8	CCMAdjEn;
	UINT8	LSNum;
	UINT16  	RGainList_HighSat[LSNUMMAX];	
	UINT16  	BGainList_HighSat[LSNUMMAX];
	UINT32	GainTarVarThdLow;
	UINT32	GainTarVarThdHigh;
	SINT16	CCMCoefList_HighSat[LSNUMMAX][9];
    	UINT16  	RGainInit;
    	UINT16  	BGainInit;
    	SINT16   	CCMCoefInit[9];
    	UINT8   	GainTarStbCntThd;
    	UINT32  	GainDistanceThd;
    	UINT8   	CCMNormEn;
    	UINT8   	CCMSatEn;
	UINT16	YBotNorm;
	UINT16	YBotLow;
	UINT16  	GainFactor;
	UINT16	RGainTrigger;
	UINT16	BGainTrigger;
	SINT16	CCMCoefTrigger[9];
	UINT8 Ybottom_AdaptEn;
	UINT16 Ybottom_TH1;
	UINT16 Ybottom_TH2;
	UINT8 YellowLight1;
	UINT8 YellowLight2;
	UINT8 YellowLight_CCM;
	UINT8 YellowLight_En;
	UINT8 YellowLight3;
	SINT16 CCMCoefList_NormalSat[LSNUMMAX][9];
	UINT8 RBGain_Limit_En;
	UINT8 RBGain_Limit_GainTh;
	
//add on 20151225:
	UINT8 V4_IQNUM;// =13, max 16
	SINT16 V4_IQTable[16][4];
	UINT8 V4_IQGrayFlag[16];//= {1,1,1,1,1,   1,1,1,0,0,   0,0,1};
	UINT8 V4_RgainMod[16];//= {100,100,100,100,100,   100,100,100,95,72,    75, 95, 100};
	UINT8 V4_BgainMod[16];//= {100,100,100,100,100,   100,100,100,90,135,   110, 90, 100};
// CWF
	UINT8 V4_CWFIQIndex;// = 3;
	UINT16 V4_CWFETThrlow;// = 2; // ms
	UINT16 V4_CWFETThrhigh;// = 6; // ms
	UINT8 V4_CWFRgainmod;// = 95;
	UINT8 V4_CWFBgainmod;// = 95;

	UINT8 V4_WINHORNUM;//	=10, max 16
	UINT8 V4_WINVERNUM;//	=6, max 8

	UINT8 V4_WinSizePer;// = 60; // 1/100
	
	UINT16 V4_RGBTop;// = 50000;
	UINT16 V4_u16YBot;// = 16;

	UINT8 V4_PixelCntThrPer;// = 10; // 1/100

// AWB switch threshold of ET x TotalGain
// ET: unit ms
// TotalGain: x1.0 = 64
	UINT32 V4_u32ETxGainThrLow;// = 5120; // 40ms, x2.0
	UINT32 V4_u32ETxGainThrHigh;// = 10240; //  40ms, x4.0

// motion detection threshold
	UINT16 V4_u16MDRGBThr;// = 256;
	UINT8 V4_u8MDRGBThrPer;// = 20;	// 1/100
	UINT8 V4_u8MDPixelCntThrPer;// = 50; // 1/100

//recover from MWB
	SINT16 RGainOffset;
	SINT16 BGainOffset;
	SINT16 GrGainOffset;
	SINT16 GbGainOffset;

// 2016-4-15
	UINT8 V4_IQExpand_En;
	UINT8 V4_IQExpand_PixelCntThrPer;
	SINT16 V4_IQExpand_IExp;
	SINT16 V4_IQExpand_QExp;

	UINT16  RGainList_NormalSat[LSNUMMAX];	
	UINT16  BGainList_NormalSat[LSNUMMAX];
	//BlueEdgeRemove add by YYF
	UINT8 	BlueEdgeRemoveEn;
	UINT16	BlueEdgeRemoveTG_0x40_TH1;
	UINT16	BlueEdgeRemoveTG_0x40_TH2;
	UINT16	BlueEdgeRemoveTG_0x40_TH3;
	UINT16	BlueEdgeRemoveEnd;
	UINT16	BlueEdgeJudgeHistbin;
	UINT16	BlueEdgeJudgeHistbinPixels;
	UINT16	BlueEdgeRemoveSat_Gain;
	UINT16	BlueEdgeRemoveSat_Gain1;
	UINT16	BlueEdgeRemoveSat_Gain2;
	UINT8	BlueEdgeRemoveSat;
	UINT8	BlueEdgeRemoveSat1;
	UINT8	BlueEdgeRemoveSat2;
} AWB_PARAM, *PAWB_PARAM;


typedef enum
{
	NRRAW_ADAPTEN = 0x20003000,	//0x3000
	NR2D_ADAPTEN,
	NR3D_ADAPTEN,
	TM_ADAPTEN,
	GAMMA_ADAPTEN,//0x3004
	SATURATION_ADAPTEN,
	CE_ADAPTEN,
	EE_ADAPTEN,
	EEPROPTY_ADAPTEN,	
	EEFILT_ADAPTEN,
	UVNR_ADAPTEN,
	DUSTNR_ADAPTEN,
	ADPD_ADAPTEN,
	PAA_ADAPTEN,//0x300D
	CONTRAST_ADAPTEN,//0x300e
	CCM1x_AdaptEn//0x2000300f
}ADAPT_EN_ADDR;

typedef enum
{
	RGBGAMMA_DAY = 0x20003020,	//[17];//0x3020
	RGBGAMMA_NIGHT1 = 0x20003042,	//[17];//0x3042
	RGBGAMMA_NIGHT2 = 0x20003064,	//[17];//0x3064
}RGBGAMMA_ADDR;

typedef enum
{
	ET_IncreaseSGDflt=0x200030f0,
	SGmin,
	SGmax1,
	MaxDGLimit,
	MaxAGLimit,
	SGDelta,
	NeedAdjustSG,
	AE_PLL1,
	AE_PLL2,
	OverheadTime,
	RowPixelNum,
	MaxFRRowNum25,
	MaxFRFrmLen25,
	MaxFRRowNum30,
	MaxFRFrmLen30,
	ETRegValDelta,
	ETRegValRatio,
	FrmLenOR,
	ChNum,
	AddrWidth,
	RegWidth,
	AEWrSt,//=0x200030f0+31=0x2000310f
	AEInitET,
	AEInitDG,
	AEInitSG,
	YtargetLowNorm,
	YtargetHighNorm,
	MaxBrightYtTimes,
	YtETTH,
	YtDGTH,
	YtSGTH,
	YtDGTH_MaxET,
	YtSGTH_MaxET,
	YtTHDelta,
	YtRatioN_Night,
	YtRatioD_Night,
	IncSpeed,
	DecSpeed,
	GainDelayFL,
	GainDelayNFL,
	SENSORMode,//0:WDR for WDR sensor, 1:WDR for normal sensor
	AEStableTH,//0-255,frame numbers
	WDR_MINETLINE,
	WDR_MAXETLINE,
	WDRMaxAGLimit,
	WDRHighRatioTH,
	WDRLowRatioTH,
	WDRHPTHHigh,
	WDRHPTHHigher,
	WDRHPTHLow,
	WDRHPTHLower,
	WDRHPTHStepLim,
	WDRLPTHHigh,
	WDRLPTHLow,
	WDRIncStepLimit,
	WDRDecStep,
	BLHighRatioTH,
	BLLowRatioTH,
	BLLPTHLow,
	BLLPTHLower,
	BLLPTHHigh,
	BLLPTHHigher,
	BLHPTHLow,
	BLHPTHHigh,
	BLIncStep,
	DSSEn,
	YtRatioN_SH,
	YtRatioD_SH,
	YtargetHigherTH,
	YtargetLowerTH,
	YmMinL,
	YmMinH,
	READPIN_ETusThr_MaxFrmRate25,
	READPIN_ETusThr_MaxFrmRate50,
	READPIN_ETlineThr_MaxFrmRate25,
	READPIN_ETlineThr_MaxFrmRate30,
	READPIN_ETlineThr_MaxFrmRate50,
	READPIN_ETlineThr_MaxFrmRate60,
	YmMaxTH,
	WDRYmMinRatioN_Night,
	WDRYmMinRatioD_Night,
}AE_ADDR;

typedef enum
{
	IrisCAETime=0X200031f0,
	IrisOAETime,
	IrisRstDelay,
	Iris_Ratio_Open,	
	Iris_Ratio_Close,					
	Iris_Ratio_Reset,
	//......
}IRIS_ADDR;

typedef enum
{
	MaxUnbLux=0X20003210,
	MinUnbLux,
	IRCDelay1,
	IRCDelay2,
	IRCDelay3,
	LDRState, //光敏电阻状态0:close;1:open;2:broken
	//LDRUsefulTH,
	LDRU2BLightRatio, 
	IRLightType,
	LDRMaxUnbLux,
	LDRMinUnbLux,
	IRCU2BCalcDelay,
}IRC_ADDR;

typedef enum
{
 	NRRAW_GAINStep1=0x20003230,	
 	NRRAW_GAINStep2,		
 	NRRAW_NoiseTH11,	
 	NRRAW_NoiseTH12,	
 	NRRAW_TimexGain1,		
	NRRAW_TimexGain2,		
 	NRRAW_NoiseTH21,	
 	NRRAW_NoiseTH22,	
 	NRRAW_TimeTH,	
	NRRAW_YtRef,

 	NR2D_GAINStep1,		
	NR2D_GAINStep2,	
	NR2D_NoiseTH1,	
 	NR2D_NoiseTH2,
	NR2D_Ratio,

	NR3D_GAINStep1,		
 	NR3D_GAINStep2,		
 	NR3D_MThL1,
 	NR3D_MThH1,
 	NR3D_MThL2,
 	NR3D_MThH2,
 	NR3D_MThH2_MID,	
 	NR3D_MThL2_HIGH,	
 	NR3D_MThH2_HIGH,	

 	TM_Gain,
 	TM_MaxET,	
 	TM_Div1,
	TM_Div2,
 	TM_Max1,
	TM_Min1,	
 	TM_Max2,
 	TM_Min2,
 	TM_Max3,	
 	TM_Min3,
 	TM_HistYIdx,
	
 	Saturation_GAINStep1,
 	Saturation_GAINStep2,
	Saturation_GAINStep3,	
	Saturation_GAINStep4,	
	Saturation_TH1,				
	Saturation_TH2,	
	Saturation_TH3,	

 	CE_GAINStep1,	
 	CE_GAINStep2,	
	CE_GAINStep3,	
	CE_MLP1,	
	CE_MHP1,
	CE_MLP2,
	CE_MHP2,
	CE_MLP3,
	CE_MHP3,	
	CE_MLP4,
	CE_MHP4,

	EE_Times,	
	EE_GAINStep1,		
	EE_GAINStep2,		
	EE_Adaptth_TH1,		
	EE_Adaptth_TH2,		
	EE_GAINStep3,	
	EE_GAINStep4,	
	EE_Cof_Edge_TH1,	
	EE_Cof_Edge_TH2,
	EE_GAINStep5,		
	EE_GAINStep6,	
	Sharp_TH1,	
	Sharp_TH2,	
	EE_FiltGain1,
	EE_FiltGain2,	
	EE_Filter1,	
	EE_Filter2,	
	
	UvNr_GAINStep1,
	UvNr_GAINStep2,	
	UvNr_TH1,	
	UvNr_TH2,	

	DustNR_GAINStep1,
	DustNR_GAINStep2,
	DustNR_TH1,	
	DustNR_TH2,	

	ADPD_TimesxGain1,	
	ADPD_TimesxGain2,	
	ADPD_TH1,			
	ADPD_TH2,			

	PAAGainStep,	
	PAAMode1,	
	PAAMode2,	
	PAA_GAINStep1,		
	PAA_GAINStep2,			
	PAA_TH1,				
	PAA_TH2,	

	ShortETLine_En,
	ShortETLine1,	
	ShortETLine2,

	ADPD_TH3,	
	ADPD_GainTH,
	ADPD_YTH,
	
	SC_AdaptEn,
	SC_Times1,
	SC_Times2,
	SC_Times3,
	SC_Gain1_TH1,			
	SC_Gain2_TH1,			
	SC_Gain1_TH2,			
	SC_Gain2_TH2,			
	SC_Gain1_TH3,			
	SC_Gain2_TH3,	

	AntiBleedingDis,
	AntiBleedDisTH1,
	AntiBleedDisTH2,
	Antibleed_GainTH,
	
	Saturation_Times1,
	Saturation_Times2,
	Saturation_TH4,

	Contrast_Times1,
	Contrast_Times2,	
	Contrast_TH1,
	Contrast_TH2,

	TM_Div1_YTSH,
	TM_Div2_YTSH,
	TM_Max1_YTSH,
	TM_Min1_YTSH,
	TM_Max2_YTSH,
	TM_Min2_YTSH,
	TM_Max3_YTSH,
	TM_Min3_YTSH,
	TM_HistYIdx_YTSH,
	
	SCBlackLevel_tmpare_En,
	SCBlackLevel_Times1,// 1/250s
	SCBlackLevel_Times2,// 1/25s
	SCBlackLevel_Times3,// 1/2s
	SensorTempareture_Th1,// 1/250s
	SensorTempareture_Th2,// 1/250s
	SensorTempareture_Th3,// 1/25s
	SensorTempareture_Th4,// 1/25s
	SensorTempareture_Th5,// 1/25s
	SensorTempareture_Th6,// 1/2s
	SensorTempareture_Th7,// 1/2s

	NRRAW_GAINStep3,
	NRRAW_TimexGain3,
	NRRAW_NoiseTH13,
	NRRAW_NoiseTH23,

	
	Saturation_Times3,
	Saturation_TH5,

	SensorTempareture_Th8,// 1/2s
	SCBlackLevel_Offset1,// 1/250s
	SCBlackLevel_Offset2,// 1/250s
	SCBlackLevel_Offset3,// 1/25s
	SCBlackLevel_Offset4,// 1/25s
	SCBlackLevel_Offset5,// 1/25s
	SCBlackLevel_Offset6,// 1/2s
	SCBlackLevel_Offset7,// 1/2s
	SCBlackLevel_Offset8,// 1/2s

	SVACDecreaseDetails_RawNR_Step,
	SVACDecreaseDetails_Sharp_Step,
	SVACDecreaseDetails_AdaptTH_Step,

	HighSatCCM_TimesTH,	
	HighSatCCM_TimesxGain1,	
	HighSatCCM_TimesxGain2,	
	HighSatCCM_TimesxGain3,	
	HighSatCCM_TimesxGain4,	
	HighSatCCM_TH1,			
	HighSatCCM_TH2,	
	HighSatCCM_TH3,			
	HighSatCCM_TH4,

	Contrast_GAINStep1,
	Contrast_GAINStep2,	
	Contrast_TH3,
	Contrast_TH4,

	NR2D_TimeTH,
	NR2D_TimexGain1,	
	NR2D_TimexGain2,	
	NR2D_HighLight_Th,

	NR3D_TimeTH,
	NR3D_TimexGain1,	
	NR3D_TimexGain2,
	NR3D_TimexGain3,
	NR3D_GAINStep3,
	NR3D_HighLight_MThL,
	NR3D_HighLight_MThH,
	NR3D_Ratio,
	NR3D_MaxEtTH1,
	NR3D_MaxEtTH2,
	NR3D_MaxEtRatio,
	NR3D_AttributeRatioN,
	NR3D_AttributeRatioD,
	NR3D_40ms_MaxMThLRatioN,
	NR3D_40ms_MaxMThLRatioD,
	NR3D_WDR_MaxMThLRatioN,
	NR3D_WDR_MaxMThLRatioD,

	NRRAWwith3D_MThH_Step1,
	NRRAWwith3D_MThH_Step2,
	NRRAWwith3D_MThH_Step3,
	NRRAWwith3D_MThH_Step4,
	NRRAWwith3D_Ratio1,
	NRRAWwith3D_Ratio2,
	NRRAWwith3D_Ratio3,

	NRRAWwith3D_MThH_Step5,
	NRRAWwith3D_MThH_Step6,
	NRRAWwith3D_MThH_Step7,
	NRRAWwith3D_MThH_Step8,
	NRRAWwith3D_Ratio4,
	NRRAWwith3D_Ratio5,
	NRRAWwith3D_Ratio6,

	ADPD_TH4,
	
	AWB1_Saturation_TH1,			
	AWB1_Saturation_TH2,
	AWB1_Saturation_TH3,
	AWB1_Saturation_TH4,
	AWB1_Saturation_TH5,

	Enable_1xCCM_MaxETTH,
	Enable_1xCCM_MaxETRatio,
	Enable_1xCCM_Gain,
	Disable_1xCCM_Gain,
	SaturationRegValue_1xCCM,
	WDR_BL_Enable_1xCCM_Gain,
	WDR_BL_Disable_1xCCM_Gain,
	WDR_BL_SaturationRegValue_1xCCM,

	BL_Ymean1,
	BL_Ymean2,
	CE_MLP5,
	CE_MHP5,
	CE_MLP6,
	BL_base,
	BL1,
	BL2,
	
	MAXGain,
}ADAPT_ADDR;

typedef enum
{
	YtRatioN_Day=0x20003440,
	YtRatioD_Day,
	YtDelta_Day,

	WDRTM_Div1,
	WDRTM_Div2,
	WDRTM_Max1,
	WDRTM_Min1,
	WDRTM_Max2,
	WDRTM_Min2,
	WDRTM_Max3,
	WDRTM_Min3,
	WDRTM_HistYIdx,

	WDRGAMMA_LOW,
	WDRGAMMA_MID,
	WDRGAMMA_HIGH,

	WDRDPDThd,

	WDRDGLimit,
	
	WDRRawNRThLimL1,
	WDRRawNRThLimH1,
	WDRRawNRThLimL2,
	WDRRawNRThLimH2,
	WDRRawNRThLimL3,
	WDRRawNRThLimH3,

	WDRMID_SatuPara,
	WDRHIGH_SatuPara,

	BLTM_Div1,
	BLTM_Div2,
	BLTM_Max1,
	BLTM_Min1,
	BLTM_Max2,
	BLTM_Min2,
	BLTM_Max3,
	BLTM_Min3,
	BLTM_HistYIdx,

	WDRLOW_SatuPara,
	WDRMinAGLimit,

	WDR3DNRMThLLim1, 
	WDR3DNRMThLLim2,
	WDR3DNRMThLLim3,

	WDR_BL_TM_Gain,
	WDRDGLimit2,
	WDRDGLimit3,
	WDR_BL_TM_Div1,
	WDR_BL_TM_Div2,
	WDR_BL_TM_Max1,
	WDR_BL_TM_Min1,
	WDR_BL_TM_Max2,
	WDR_BL_TM_Min2,
	WDR_BL_TM_Max3,
	WDR_BL_TM_Min3,
	WDR_BL_TM_HistYIdx,

	WDRYmMinL1,
	WDRYmMinH1,
	WDRYmMinL2,
	WDRYmMinH2,
	WDRYmMinL3,
	WDRYmMinH3,

	YmMaxTH1,
	YmMaxTH2,
	YmMaxTH3,
}WDRMODE_ADDR;

typedef enum
{
    SaturationRegDeft=0x200035c0,
    ContrastRegDeft,
   EdgeRegDeft,
}CSSDEF_ADDR;

typedef enum
{
	RGainMin=0x200035e0,
	RGainMax,
	BGainMin,
	BGainMax,
	ParaUpdtSpd,
	GainDiffThrLock,
	GainDiffThrUnLock,
	CCMAdjEn,
	LSNum,
	RGainList_HighSat,	
	BGainList_HighSat,
	GainTarVarThdLow,
	GainTarVarThdHigh,
	CCMCoefList_HighSat,
    	RGainInit,
    	BGainInit,
    	CCMCoefInit,
    	GainTarStbCntThd,
    	GainDistanceThd,
    	CCMNormEn,
    	CCMSatEn,
	YBotNorm,
	YBotLow,
	GainFactor,
	RGainTrigger,
	BGainTrigger,
	CCMCoefTrigger,
	Ybottom_AdaptEn,
	Ybottom_TH1,
	Ybottom_TH2,
	YellowLight1,
	YellowLight2,
	YellowLight_CCM,
	YellowLight_En,
	YellowLight3,
	CCMCoefList_NormalSat,
	RBGain_Limit_En,
	RBGain_Limit_GainTh,
	
//add on 20151225:
	V4_IQNUM,
	V4_IQTable,
	V4_IQGrayFlag,
	V4_RgainMod,
	V4_BgainMod,
// CWF
	V4_CWFIQIndex,
	V4_CWFETThrlow,
	V4_CWFETThrhigh,
	V4_CWFRgainmod,
	V4_CWFBgainmod,

	V4_WINHORNUM,
	V4_WINVERNUM,

	V4_WinSizePer,
	
	V4_RGBTop,
	V4_u16YBot,

	V4_PixelCntThrPer,

// AWB switch threshold of ET x TotalGain
	V4_u32ETxGainThrLow,
	V4_u32ETxGainThrHigh,

// motion detection threshold
	V4_u16MDRGBThr,
	V4_u8MDRGBThrPer,
	V4_u8MDPixelCntThrPer,

//recover from MWB
	RGainOffset,
	BGainOffset,
	GrGainOffset,
	GbGainOffset,

// 2016-4-15
	V4_IQExpand_En,
	V4_IQExpand_PixelCntThrPer,
	V4_IQExpand_IExp,
	V4_IQExpand_QExp,

	RGainList_NormalSat,
	BGainList_NormalSat,
	//BlueEdgeRemove add by YYF
	BlueEdgeRemoveEn,
	BlueEdgeRemoveTG_0x40_TH1,
	BlueEdgeRemoveTG_0x40_TH2,
	BlueEdgeRemoveTG_0x40_TH3,
	BlueEdgeRemoveEnd,
	BlueEdgeJudgeHistbin,
	BlueEdgeJudgeHistbinPixels,
	BlueEdgeRemoveSat_Gain,
	BlueEdgeRemoveSat_Gain1,
	BlueEdgeRemoveSat_Gain2,
	BlueEdgeRemoveSat,
	BlueEdgeRemoveSat1,
	BlueEdgeRemoveSat2,
} AWB_ADDR;

typedef struct _AF_MODE_CMD
{
	UINT8	afcmd;
	UINT32	step;
	UINT8 	cntFlag;
}AF_MODE_CMD;

typedef enum
{
    AUTO=0x00,
    FOCUS = 0x10,
   ZOOM = 0x20,
   ZOOM_ABS = 0x21,
   RESET = 0x30,
   INVALID = 0xff,
}AF_CMD;

VIM_RESULT VIM_ISP_Set_Adapt_En(PADAPT_EN adapt_en);
VIM_RESULT VIM_ISP_Get_Adapt_En(PADAPT_EN adapt_en);
VIM_RESULT VIM_ISP_Set_Gamma_Adapten(UINT8 enable);
VIM_RESULT VIM_ISP_Get_Gamma_Adapten(UINT8* enable);
VIM_RESULT VIM_ISP_Set_RGBGamma(UINT8 flag,PRGBGAMMA_PARAM rgbgamma);
VIM_RESULT VIM_ISP_Get_RGBGamma(UINT8 flag,PRGBGAMMA_PARAM rgbgamma);
#if 0
VIM_RESULT VIM_ISP_Set_AEWrSt(PAE_WR_PARAM aewr);
VIM_RESULT VIM_ISP_Get_AEWrSt(PAE_WR_PARAM aewr);
#endif
VIM_RESULT VIM_ISP_Set_AE(PAE_PARAM ae);
VIM_RESULT VIM_ISP_Get_AE(PAE_PARAM ae);
VIM_RESULT VIM_ISP_Set_IRIS(PIRIS_PARAM iris);
VIM_RESULT VIM_ISP_Get_IRIS(PIRIS_PARAM iris);
VIM_RESULT VIM_ISP_Set_IRC(PIRC_PARAM irc);
VIM_RESULT VIM_ISP_Get_IRC(PIRC_PARAM irc);
VIM_RESULT VIM_ISP_Set_ADAPT(PADAPT_PARAM adapt);
VIM_RESULT VIM_ISP_Get_ADAPT(PADAPT_PARAM adapt);
VIM_RESULT VIM_ISP_Set_WDRMODE(PWDRMODE_PARAM wdrmode);
VIM_RESULT VIM_ISP_Get_WDRMODE(PWDRMODE_PARAM wdrmode);
VIM_RESULT VIM_ISP_Set_CSSDEF(PCSSDEF_PARAM cssdef);
VIM_RESULT VIM_ISP_Get_CSSDEF(PCSSDEF_PARAM cssdef);
VIM_RESULT VIM_ISP_Set_AWB(PAWB_PARAM awb);
VIM_RESULT VIM_ISP_Get_AWB(PAWB_PARAM awb);
UINT16  lsb2msb_uint16(UINT16 A);
UINT32  lsb2msb_uint32(UINT32 A);


//xierong 20141013 add for tools request end

#ifdef __cplusplus
}
#endif

#endif
