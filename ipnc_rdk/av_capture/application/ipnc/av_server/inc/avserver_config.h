#ifndef _AV_SERVER_CONFIG_H_
#define _AV_SERVER_CONFIG_H_

#include <osa.h>

#define AVSERVER_MAX_STREAMS 		(4)
#define AVSERVER_MAX_FRAMERATE 		(30)

#define AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN        (0)
#define AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN         (1)

#define MAX_STRING_LENGTH 24

#define MAX_DMVA_STRING_LENGTH 256

typedef struct {

  Uint32 samplingRate;
  Uint32 bitRate;
  Uint16 codecType;
  Bool   audioUpdate;

  Bool   audioAlarm;
  Uint16 alarmLevel;

  Bool   fileSaveEnable;

} AUDIO_Config;

typedef struct {

  Uint16 captureStreamId;
  Bool   fdEnable;
  Bool   fdTracker;
  Bool   frIdentify;         /* ANR - FR */
  Bool   frRegUsr;
  Bool   frDelUsr;
  Bool 	 privacyMaskEnable;
  Bool   fdROIEnable;

  Uint16 startX;
  Uint16 startY;
  Uint16 width;
  Uint16 height;
  Uint16 fdconflevel;
  Uint16 fddirection;
  Uint16 frconflevel;
  Uint16 frdatabase;
  Uint16 maskoption;

} VIDEO_FaceDetectConfig;

typedef struct {

  /* Trip Zone Parameters */
  Uint16 dmvaAppTZSensitivity; /* 1 to 5: Low to High */
  Int16 dmvaAppTZPersonMinWidth;
  Int16 dmvaAppTZPersonMinHeight;
  Int16 dmvaAppTZVehicleMinWidth;   // AYK - 0223 - changed to Int16,can take -1
  Int16 dmvaAppTZVehicleMinHeight;  // AYK - 0223 - changed to Int16,can take -1
  Int16 dmvaAppTZPersonMaxWidth;
  Int16 dmvaAppTZPersonMaxHeight;
  Int16 dmvaAppTZVehicleMaxWidth;   // AYK - 0223 - changed to Int16,can take -1
  Int16 dmvaAppTZVehicleMaxHeight;  // AYK - 0223 - changed to Int16,can take -1
  Uint16 dmvaAppTZDirection; /* 4: Bi-direction, 2: Zone 1 to Zone 2, 1: Zone 2 to Zone 1 */
  Uint16 dmvaAppTZPresentAdjust;
  Uint16 dmvaAppTZEnable;

  /* Trip Zone Parameters */
  Uint16 dmvaAppTZZone1_ROI_numSides;
  Uint16 dmvaAppTZZone2_ROI_numSides;
  Uint16 dmvaAppTZZone1_x_array[16];
  Uint16 dmvaAppTZZone1_y_array[16];
  Uint16 dmvaAppTZZone2_x_array[16];
  Uint16 dmvaAppTZZone2_y_array[16];


  /* DMVA Video Analytics Main Page */
  Uint16 dmvaAppCamID;
  Uint16 dmvaAppGetSchedule;
  Uint16 dmvaAppCfgFD;
  Uint16 dmvaAppCfgCTDImdSmetaTzOc; /* Bit 4: OC, Bit 3: TZ, Bit 2: SMETA, Bit 1: IMD, Bit 0: CTD */
  Uint16 dmvaAppDisplayOptions; /* Bit 3: Object Bounding Box, Bit 2: Zones, Bit 1: TimeStamp, Bit 0: MetaData Overlay*/
  Uint16 dmvaAppEventRecordingVAME;/* Bit 3: Video Stream, Bit 2: Audio Stream, Bit 1: MetaData Stream, Bit 0: Event Log */
  Uint16 dmvaAppEventListActionPlaySendSearchTrash;/* Bit 3: Play, Bit 2: Send Email, Bit 1: Search, Bit 0: Trash*/
  Uint16 dmvaAppEventListSelectEvent;
  Uint16 dmvaAppEventListArchiveFlag;
  Uint16 dmvaAppEventListEventType;

  /* Camera Tamper Detect Parameters */
  Uint16 dmvaAppCfgTDEnv1;/* Detect Lights On to Off */
  Uint16 dmvaAppCfgTDEnv2;/* Detect Lights On to Off */
  Uint16 dmvaAppCfgTDSensitivity;/* 1 to 5: Low to High */
  Uint16 dmvaAppCfgTDResetTime; /* Number of seconds */

  /* Object Counting Parameters */
  Uint16 dmvaAppOCSensitivity; /* 1 to 5: Low to High */
  Uint16 dmvaAppOCObjectWidth;
  Uint16 dmvaAppOCObjectHeight;
  Uint16 dmvaAppOCDirection;/* 4: Bi-direction, 2: Zone 1 to Zone 2, 1: Zone 2 to Zone 1 */
  Uint16 dmvaAppOCEnable;
  Uint16 dmvaAppOCLeftRight;
  Uint16 dmvaAppOCTopBottom;

  /* Streaming MOS Parameters */
  Uint16 dmvaAppSMETASensitivity; /* 1 to 5: Low to High */
  Uint16 dmvaAppSMETAEnableStreamingData;
  Uint16 dmvaAppSMETAStreamBB;
  Uint16 dmvaAppSMETAStreamVelocity;
  Uint16 dmvaAppSMETAStreamHistogram;
  Uint16 dmvaAppSMETAStreamMoments;
  Uint16 dmvaAppSMETAPresentAdjust; /* 8: Person present, 4: Present vehicle, 2: View Person, 1: View vehicle */
  Int16 dmvaAppSMETAPersonMinWidth;
  Int16 dmvaAppSMETAPersonMinHeight;
  Int16 dmvaAppSMETAVehicleMinWidth;
  Int16 dmvaAppSMETAVehicleMinHeight;
  Int16 dmvaAppSMETAPersonMaxWidth;
  Int16 dmvaAppSMETAPersonMaxHeight;
  Int16 dmvaAppSMETAVehicleMaxWidth;
  Int16 dmvaAppSMETAVehicleMaxHeight;
  Uint16 dmvaAppSMETANumZones; /* 1 to 4 */
  Uint16 dmvaAppSMETAZone1_ROI_numSides;
  Uint16 dmvaAppSMETAZone1_x_array[16];
  Uint16 dmvaAppSMETAZone1_y_array[16];

  /* IMD Parameters */
  Uint16 dmvaAppIMDSensitivity; /* 1 to 5: Low to High */
  Uint16 dmvaAppIMDPresentAdjust; /* 8: Person present, 4: Present vehicle, 2: View Person, 1: View vehicle */
  Int16 dmvaAppIMDPersonMinWidth;
  Int16 dmvaAppIMDPersonMinHeight;
  Int16 dmvaAppIMDVehicleMinWidth;
  Int16 dmvaAppIMDVehicleMinHeight;
  Int16 dmvaAppIMDPersonMaxWidth;
  Int16 dmvaAppIMDPersonMaxHeight;
  Int16 dmvaAppIMDVehicleMaxWidth;
  Int16 dmvaAppIMDVehicleMaxHeight;
  Uint16 dmvaAppIMDNumZones; /* 1 to 4 */
  Uint16 dmvaAppIMDZoneEnable; /* Bit 3: Zone 3 enable, Bit 2: Zone 2 enable, Bit 1: Zone 1 enable, Bit 0: Zone 0 enable*/

  Uint16 dmvaAppIMDZone1_ROI_numSides;
  Uint16 dmvaAppIMDZone2_ROI_numSides;
  Uint16 dmvaAppIMDZone3_ROI_numSides;
  Uint16 dmvaAppIMDZone4_ROI_numSides;
  Uint16 dmvaAppIMDZone1_x_array[16];
  Uint16 dmvaAppIMDZone1_y_array[16];
  Uint16 dmvaAppIMDZone2_x_array[16];
  Uint16 dmvaAppIMDZone2_y_array[16];
  Uint16 dmvaAppIMDZone3_x_array[16];
  Uint16 dmvaAppIMDZone3_y_array[16];
  Uint16 dmvaAppIMDZone4_x_array[16];
  Uint16 dmvaAppIMDZone4_y_array[16];

  /* IMD Parameters */
  char   dmvaAppIMDZone_1_Label[MAX_DMVA_STRING_LENGTH];//TO BE ADDED
  char   dmvaAppIMDZone_2_Label[MAX_DMVA_STRING_LENGTH];//TO BE ADDED
  char   dmvaAppIMDZone_3_Label[MAX_DMVA_STRING_LENGTH];//TO BE ADDED
  char   dmvaAppIMDZone_4_Label[MAX_DMVA_STRING_LENGTH];//TO BE ADDED

  /* DMVA Video Analytics Main Page */
  Uint16     dmvaAppEventListStartMonth;//TO BE ADDED
  Uint16       dmvaAppEventListStartDay;//TO BE ADDED
  Uint16      dmvaAppEventListStartTime;//TO BE ADDED
  Uint16       dmvaAppEventListEndMonth;//TO BE ADDED
  Uint16         dmvaAppEventListEndDay;//TO BE ADDED
  Uint16        dmvaAppEventListEndTime;//TO BE ADDED
  char      dmvaAppEventListTimeStamp[MAX_DMVA_STRING_LENGTH];//TO BE ADDED
  char   dmvaAppEventListEventDetails[MAX_DMVA_STRING_LENGTH];//TO BE ADDED

  Uint16 dmvaAppEncRoiEnable;
  Uint16 maxNoObj;

} VIDEO_DmvaAppParameters;

typedef struct {

  Uint16   captureStreamId;
  Bool     dmvaEnable;
  Uint16   dmvaexptalgoframerate;
  Uint16   dmvaexptalgodetectfreq;
  Uint16   dmvaexptevtrecordenable;
  Uint16   dmvaexptsmetatrackerenable;
  void     *pSysInfo;

} VIDEO_DmvaAppConfig;

typedef struct {

  Uint16 captureStreamId;
  Uint16 width;
  Uint16 height;
  Uint16 expandH;
  Uint16 expandV;

} VIDEO_DisplayConfig;

typedef enum{                       //Defined by Rajiv:  SWOSD Test

SWOSD_BASIC = 0,
SWOSD_DETAIL,
SWOSD_COMPLETE

}SWOSD_Type ;

typedef enum{
	SWOSD_FMT_TOP_LEFT = 0,
	SWOSD_FMT_TOP_RIGHT,
	SWOSD_FMT_TOP_NONE
}SWOSD_TOP_POS_FMT;

typedef enum{
	SWOSD_FMT_BOTTOM_LEFT = 0,
	SWOSD_FMT_BOTTOM_RIGHT,
	SWOSD_FMT_BOTTOM_NONE
}SWOSD_BOTTOM_POS_FMT;

typedef enum{
	SWOSD_FMT_YMD = 0,
	SWOSD_FMT_MDY,
	SWOSD_FMT_DMY
}SWOSD_DATE_FMT;

typedef enum{
	SWOSD_FMT_12HR = 0,
	SWOSD_FMT_24HR
}SWOSD_TIME_FMT;

typedef enum{
	AEWB_NONE = 0,
	AEWB_APPRO,
	AEWB_TI
}AEWB_VENDOR;

typedef enum{
	AEWB_OFF = 0,
	AEWB_AE,
	AEWB_AWB,
	AEWB_AEWB
}AEWB_TYPE;

typedef enum{
	AEWB_FRAMERATE_PRIORITY = 0,
	AEWB_FRAMERATE_EXPOSURE
}AEWB_PRIORITY;

typedef enum{
	FLICKER_NTSC = 0,
	FLICKER_PAL,
	FLICKER_NONE
}FLICKER_TYPE;

typedef enum{
	SALDRE_LOW = 0,
	SALDRE_MED,
	SALDRE_HIGH
}SALDRE_LEVEL;

typedef enum{
	SALDRE_OFF = 0,
	SALDRE_GLOBAL,
	SALDRE_LOCAL,
	SALDRE_ADAPTIVE
}SALDRE_MODE;

typedef struct{

	Bool swosdEnable;
	Bool swosdLogoStringUpdate;
	Bool swosdDmvaStringUpdate;
	SWOSD_Type  swosdType;
	Bool swosdDateEnable;
	char swosdDateFormat;
	char swosdDatePos;
	Bool swosdTimeEnable;
	char swosdTimeFormat;
	char swosdTimePos;
	Bool swosdDateTimeUpdate;
	Bool swosdTextEnable;
	Bool swosdDmvaTextEnable;
	char swosdTextPos;
	char swosdDispString[MAX_STRING_LENGTH];
	char swosdDmvaDispString[MAX_STRING_LENGTH];
	Bool swosdLogoEnable;
	char swosdLogoPos;
	Bool swosdDetailEnable;
	Bool swosdHistEnable;
	Bool swosdFREnable[8];
} VIDEO_SWOSDConfig;

typedef struct {

  Uint16 width;
  Uint16 height;
  Uint16 dmaInterval;
  Uint16 vnfStrength;
  Bool   ldcEnable;
  Bool   snfEnable;
  Bool   tnfEnable;
  Bool   vsEnable;
  Bool   flipH;
  Bool   flipV;
  Bool   mirrUpdate;
  Bool   vnfPrmUpdate;

  int numEncodes;
  int encodeStreamId[AVSERVER_MAX_STREAMS];

  Uint32 frameSkipMask;
  VIDEO_SWOSDConfig  swosdConfig;

} VIDEO_CaptureConfig;

typedef struct {
	Uint16 startx;
	Uint16 starty;
	Uint16 width;
	Uint16 height;
} REGION_roi;

typedef struct {

  Uint16 captureStreamId;
  Uint16 cropWidth;
  Uint16 cropHeight;
  Uint32 frameRateBase;
  Uint32 frameSkipMask;
  Uint16 codecType;
  Uint32 codecBitrate;
  Bool   encryptEnable;
  Bool   fileSaveEnable;
  Bool   motionVectorOutputEnable;
  Int16  qValue;
  Uint16 rateControlType;

  Int16  	 numROI;
  REGION_roi prmROI[AVSERVER_MAX_STREAMS];

  Uint16 newCodecPrm;
  Uint16 ipRatio;
  Uint16 fIframe;
  Uint16 qpInit;
  Uint16 qpMin;
  Uint16 qpMax;
  Uint16 encodePreset;
  Uint16 packetSize;
  Uint16 resetCodec;

} VIDEO_EncodeConfig;

typedef struct {

  int mem_layou_set;

} STREAM_Config;

typedef struct {
  Uint16 snapLocation;
  Uint16 snapEnable;
  char snapName[MAX_STRING_LENGTH];
} SNAP_config;

typedef struct {
  Bool   aewbEnable;
  Bool   aewbBinEnable;
  Bool   afEnable;
  Bool   saldreEnable;
  Bool   saldreState;
  int    saldreMode;
  int    saldreLevel;
  int    aewbVendor;
  int    aewbType;
  int    aewbPriority;
  int    autoFocusVal;
  int    reduceShutter;
  int    flickerType;
} AEWB_config;

typedef struct {

  Uint16 sensorMode;
  Uint16 sensorFps;

  Bool   audioTskEnable;
  Bool   vstabTskEnable;
  Bool   ldcTskEnable;
  Bool   vnfTskEnable;
  Bool   encryptTskEnable;
  Bool   displayEnable;
  Bool   alarmEnable;
  Bool   deiTskEnable ;

  Bool   winmodeEnable;
  Bool	 fxdROIEnable;
  Bool	 vnfDemoEnable;

  Bool   captureSingleResize;
  Uint16 captureRawInMode;
  Uint16 resizerClkdivN;
  Uint16 captureYuvFormat;
  Uint16 numCaptureStream;
  Uint16 numEncodeStream;

  VIDEO_CaptureConfig     captureConfig[AVSERVER_MAX_STREAMS];
  VIDEO_EncodeConfig      encodeConfig[AVSERVER_MAX_STREAMS];
  VIDEO_FaceDetectConfig  faceDetectConfig;
  VIDEO_DmvaAppConfig     dmvaAppConfig;
  VIDEO_DmvaAppParameters dmvaAppParams;
  VIDEO_DisplayConfig     displayConfig;
  AUDIO_Config            audioConfig;
  STREAM_Config			  streamConfig;
  SNAP_config			  snap_config;
  AEWB_config			  aewb_config;

  Bool encryptEnable;

} AVSERVER_Config;

#endif  /*  _AV_SERVER_CONFIG_H_  */

