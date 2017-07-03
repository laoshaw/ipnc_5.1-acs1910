#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <osa_cmem.h>
#include <osa_file.h>
#include <avserver_ui.h>
#include <mem_mng.h>
#include <alarm_msg_drv.h>

AVSERVER_UI_Ctrl gAVSERVER_UI_ctrl;

int UI_rtspStreamStart()
{
  char opt[32] = "./wis-streamer";

  OSA_printf(" AVSERVER UI: Starting Streaming Server...\n");

  if(!gAVSERVER_UI_config.audioEnable) {
  	strcat(opt, " -s");
  }
  else {
  	if(gAVSERVER_UI_config.audioCodeType == ALG_AUD_CODEC_AAC) {
  		strcat(opt, " -a");
	}
  }

  if((gAVSERVER_UI_config.codecEncodePreset[0]&ALG_VID_ENC_SVC_MASK) == ALG_VID_ENC_PRESET_SVC) {
  	strcat(opt, " -v");
  }

  strcat(opt, " -h");
  strcat(opt, "&\n");

  OSA_waitMsecs(2000);
  system(opt);
  OSA_waitMsecs(2000);

  OSA_printf(" AVSERVER UI: Starting Streaming Server...DONE\n");

  return 0;
}

int UI_rtspStreamStop()
{
  OSA_printf(" AVSERVER UI: Stopping Streaming Server...\n");

  system("killall -2 wis-streamer\n");
  OSA_waitMsecs(2000);
  system("killall -9 wis-streamer\n");
  OSA_waitMsecs(2000);

  OSA_printf(" AVSERVER UI: Stopping Streaming Server...DONE\n");

  return 0;
}

int UI_setDone(Bool isDone)
{
  gAVSERVER_UI_ctrl.quit = 1;
  return 0;
}

int UI_isDone()
{
  return gAVSERVER_UI_ctrl.quit;
}

int UI_start()
{
  int status;

  DM365MM_init();

  memset(&gAVSERVER_UI_ctrl, 0, sizeof(gAVSERVER_UI_ctrl));

  OSA_attachSignalHandler(SIGINT, UI_signalHandler);

  gAVSERVER_UI_ctrl.mode = gAVSERVER_UI_config.mode;

  OSA_printf("\nAVSERVER UI: Initializing.\n")
  status = AVSERVER_init();
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_init()\n");
    return status;
  }

  OSA_getHostName(gAVSERVER_UI_ctrl.hostName, sizeof(gAVSERVER_UI_ctrl.hostName));

  UI_setConfig( &gAVSERVER_UI_ctrl.avserverConfig);

  gAVSERVER_UI_ctrl.ldcEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].ldcEnable;
  gAVSERVER_UI_ctrl.tnfEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].tnfEnable;
  gAVSERVER_UI_ctrl.snfEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].snfEnable;
  gAVSERVER_UI_ctrl.vsEnable       = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].vsEnable;
  gAVSERVER_UI_ctrl.fdEnable       = gAVSERVER_UI_ctrl.avserverConfig.faceDetectConfig.fdEnable;
  gAVSERVER_UI_ctrl.dmvaEnable     = gAVSERVER_UI_ctrl.avserverConfig.dmvaAppConfig.dmvaEnable;
  gAVSERVER_UI_ctrl.fileSaveEnable = gAVSERVER_UI_ctrl.avserverConfig.encodeConfig[0].fileSaveEnable;
  gAVSERVER_UI_ctrl.aewbEnable     = gAVSERVER_UI_ctrl.avserverConfig.aewb_config.aewbEnable;
  gAVSERVER_UI_ctrl.afEnable       = gAVSERVER_UI_ctrl.avserverConfig.aewb_config.afEnable;
  gAVSERVER_UI_ctrl.saldreEnable   = gAVSERVER_UI_ctrl.avserverConfig.aewb_config.saldreEnable;
  gAVSERVER_UI_ctrl.aewbType       = gAVSERVER_UI_ctrl.avserverConfig.aewb_config.aewbType;
  gAVSERVER_UI_ctrl.aewbVendor     = gAVSERVER_UI_ctrl.avserverConfig.aewb_config.aewbVendor;
  gAVSERVER_UI_ctrl.winmodeEnable  = gAVSERVER_UI_ctrl.avserverConfig.winmodeEnable;
  gAVSERVER_UI_ctrl.vnfDemoEnable  = gAVSERVER_UI_ctrl.avserverConfig.vnfDemoEnable;
  gAVSERVER_UI_ctrl.swosdEnable    = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].swosdConfig.swosdEnable;

  if(gAVSERVER_UI_config.rtspEnable)
    UI_rtspStreamStart();

  status = AVSERVER_start(&gAVSERVER_UI_ctrl.avserverConfig);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_start()\n");
    AVSERVER_exit();
    return status;
  }

  return status;
}

int UI_stop()
{
  OSA_printf("AVSERVER UI: Stoping.....\n")
  AVSERVER_stop();

  if(gAVSERVER_UI_config.rtspEnable)
    UI_rtspStreamStop();

  OSA_printf("AVSERVER UI: Exiting.....\n")
  AVSERVER_exit();

  DM365MM_exit();

  OSA_printf("AVSERVER UI: Closed Successfully\n\n")

  return OSA_SOK;
}

void UI_signalHandler(int signum)
{
  UI_setDone(TRUE);
}

void UI_changeMasterPriority(Bool flag)
{
    unsigned int pRegAddr;
    volatile unsigned int *pReg;

    pRegAddr	= DM365MM_mmap(0x01c40000, 0x40);

    pReg 		= (unsigned int *)(pRegAddr+0x3c);	// SYSTEM.MSTPRI0
    *pReg 		= (flag == TRUE) ? 0x00440022:0x00440011;

    OSA_printf("SYSTEM.MSTPRI0 value changed to %08x\n", *pReg);

    DM365MM_ummap(pRegAddr, 0x40);
}

void UI_setConfig(AVSERVER_Config *config)
{
  int i, k, numEncodes;
  int platform = gAVSERVER_UI_config.platform_id;

  numEncodes = gAVSERVER_UI_config.numEncodes;

  memset(config, 0, sizeof(*config));

  config->resizerClkdivN 	= 50;
  config->displayEnable  	= FALSE;

  #ifdef YUV_MODE
  config->captureRawInMode  = AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
  #else
  config->captureRawInMode	= gAVSERVER_UI_config.saldreEnable?AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN:AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
  #endif

  #ifdef YUV_MODE_INTERLACED
  config->deiTskEnable 	= TRUE;
  config->displayEnable = TRUE;
  #else
  config->deiTskEnable = FALSE;
  #endif

  config->aewb_config.aewbEnable 	= gAVSERVER_UI_config.aewbEnable;
  config->aewb_config.saldreEnable  = gAVSERVER_UI_config.saldreEnable;
  config->aewb_config.saldreState 	= FALSE; //switch ON after saldre Create
  config->aewb_config.saldreLevel	= SALDRE_MED;
  config->aewb_config.saldreMode	= SALDRE_LOCAL;
  config->aewb_config.afEnable 		= gAVSERVER_UI_config.afEnable;
  config->aewb_config.autoFocusVal	= 0;
  config->aewb_config.aewbType   	= gAVSERVER_UI_config.aewbType;
  #ifdef YUV_MODE
  config->aewb_config.aewbVendor    = AEWB_NONE;
  #else
  config->aewb_config.aewbVendor 	= gAVSERVER_UI_config.aewbVendor;
  #endif
  config->aewb_config.reduceShutter = 100;
  config->aewb_config.aewbPriority  = AEWB_FRAMERATE_PRIORITY;
  config->aewb_config.flickerType  	= gAVSERVER_UI_config.flickerType;

  config->fxdROIEnable 					= FALSE;
  config->faceDetectConfig.fdROIEnable 	= FALSE;

  config->snap_config.snapEnable	= FALSE;
  config->snap_config.snapLocation	= FALSE;
  strcpy(config->snap_config.snapName, "snap");

  config->faceDetectConfig.fdTracker			= FALSE;
  config->faceDetectConfig.privacyMaskEnable 	= FALSE;
  config->faceDetectConfig.frIdentify 			= FALSE;
  config->faceDetectConfig.frRegUsr 			= FALSE;
  config->faceDetectConfig.frDelUsr 			= FALSE;

  for(i=0;i<AVSERVER_MAX_STREAMS;i++) {
    config->captureConfig[i].dmaInterval = 0x20;

	config->captureConfig[i].mirrUpdate = FALSE;
    config->captureConfig[i].flipH = gAVSERVER_UI_config.flipH;
    config->captureConfig[i].flipV = gAVSERVER_UI_config.flipV;

	config->captureConfig[i].vnfPrmUpdate = FALSE;

    config->captureConfig[i].swosdConfig.swosdEnable 	= gAVSERVER_UI_config.swosdEnable;
    config->captureConfig[i].swosdConfig.swosdType 		= -1;
    config->captureConfig[i].swosdConfig.swosdLogoStringUpdate = OSD_YES;
    strcpy(config->captureConfig[i].swosdConfig.swosdDispString , "EVALUATION");
    config->captureConfig[i].swosdConfig.swosdDmvaStringUpdate = OSD_YES;
	strcpy(config->captureConfig[i].swosdConfig.swosdDmvaDispString , "");
    config->captureConfig[i].swosdConfig.swosdDateTimeUpdate = OSD_YES;
  }

  for(i=0;i<numEncodes;i++) {
    config->encodeConfig[i].rateControlType = gAVSERVER_UI_config.codecRateControlType[i];
    config->encodeConfig[i].encodePreset = gAVSERVER_UI_config.codecEncodePreset[i];
  }

  config->vnfDemoEnable	= gAVSERVER_UI_config.vnfDemoEnable;

  config->audioTskEnable  				= gAVSERVER_UI_config.audioEnable;
  config->audioConfig.codecType        	= gAVSERVER_UI_config.audioCodeType;
  config->audioConfig.samplingRate  	= gAVSERVER_UI_config.audioSampleRate;
  config->audioConfig.bitRate           = gAVSERVER_UI_config.audioBitRate;
  config->audioConfig.audioUpdate		= FALSE;
  config->audioConfig.alarmLevel 		= 100;
  config->audioConfig.audioAlarm 		= FALSE;
  config->audioConfig.fileSaveEnable    = FALSE;

  // Encryption
  config->encryptEnable = gAVSERVER_UI_config.encryptEnable;

  if(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1) {
	UI_changeMasterPriority(TRUE);
  }
  else {
	UI_changeMasterPriority(FALSE);
  }

  config->dmvaAppConfig.dmvaEnable = FALSE;

  if(platform == PLATFORM_DM365) {
    OSA_printf("\nCONFIGURING AVSERVER FOR DM365 .....\n");
  }
  else if(platform == PLATFORM_DM368) {
  	OSA_printf("\nCONFIGURING AVSERVER FOR DM368 .....\n");
  }
  else if(platform == PLATFORM_DM368E) {
  	OSA_printf("\nCONFIGURING AVSERVER FOR DM368E .....\n");
  }
  else if(platform == PLATFORM_DM369) {
    OSA_printf("\nCONFIGURING AVSERVER FOR DM369 .....\n");
  }
  else if(platform == PLATFORM_DMVA1) {
    OSA_printf("\nCONFIGURING AVSERVER FOR DMVA1 .....\n");
	config->dmvaAppConfig.dmvaEnable = gAVSERVER_UI_config.dmvaEnable;
  }
  else if(platform == PLATFORM_DMVA2) {
  	OSA_printf("\nCONFIGURING AVSERVER FOR DMVA2 .....\n");
	config->dmvaAppConfig.dmvaEnable = gAVSERVER_UI_config.dmvaEnable;
  }
  else {
    OSA_printf("\nUNKNOWN PLATFORM: CONFIGURING FOR DEFAULT VALUES .....\n");
    platform = PLATFORM_DM368;
  }

  switch(gAVSERVER_UI_config.mode) {

////////////////////add by pamsimochen
        case AVSERVER_UI_CAPTURE_MODE_ACS1910:

            gAVSERVER_UI_config.ldcEnable 		= FALSE;
		    gAVSERVER_UI_config.vsEnable  		= FALSE;
		    gAVSERVER_UI_config.snfEnable 		= FALSE;
		    gAVSERVER_UI_config.tnfEnable 		= FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_ACS1910;
			config->sensorFps 					= DRV_IMGS_FPS_ACS1910;
  			config->aewb_config.aewbBinEnable	= FALSE;
  			config->aewb_config.reduceShutter	= 200;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= gAVSERVER_UI_config.encryptEnable;

			config->captureSingleResize 		= FALSE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

	  		config->numCaptureStream		  	= 1;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 					= numEncodes;

			config->faceDetectConfig.captureStreamId 	= 1;
			config->faceDetectConfig.fdEnable 		 	= FALSE;
			config->faceDetectConfig.fdTracker 		 	= FALSE;
			config->dmvaAppConfig.captureStreamId       = 1;
			config->dmvaAppConfig.dmvaEnable            = FALSE;

	  		config->displayConfig.captureStreamId    	= 0;
			config->displayConfig.width 			 	= 720;
			config->displayConfig.height			 	= 480;
			config->displayConfig.expandH            	= TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width				= DRV_IMGS_WIDTH_ACS1910; //1280
			config->captureConfig[i].height 			= DRV_IMGS_HEIGHT_ACS1910; //800
			config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable			= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable			= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask		= 0x3FFFFFFF;

			i=0;
			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= ALIGN_ENCODE(DRV_IMGS_ENCODE_WIDTH_ACS1910);//ALIGN_ENCODE(1280);
			config->encodeConfig[i].cropHeight					= ALIGN_ENCODE(DRV_IMGS_ENCODE_HEIGHT_ACS1910);//ALIGN_ENCODE(720);
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];

			break;
////////////////////end add
//
		case AVSERVER_UI_CAPTURE_MODE_D1:

			#ifdef YUV_MODE_INTERLACED
			config->sensorMode          = gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_720x480:DRV_IMGS_SENSOR_MODE_720x480;
			#else
 			config->sensorMode			= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1920x1080:DRV_IMGS_SENSOR_MODE_720x480;
			#endif

			config->sensorFps 					= 30;
  			config->aewb_config.aewbBinEnable	= TRUE;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= gAVSERVER_UI_config.encryptEnable;

			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;
			#ifdef YUV_MODE_INTERLACED
			config->numCaptureStream        = 1;
			config->captureSingleResize   = TRUE;
			#else
			config->numCaptureStream		= 2;
			config->captureSingleResize   = FALSE;
			#endif

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;
			config->dmvaAppConfig.captureStreamId    = 1;

			config->displayConfig.captureStreamId 	 = 0;
			config->displayConfig.width 			 = 720;
			config->displayConfig.height			 = 480;
      		config->displayConfig.expandH		     = TRUE;

			i=0;

			k=0;
			#ifdef YUV_MODE_INTERLACED
			config->captureConfig[i].width						= gAVSERVER_UI_config.yuvModeWidth;
			config->captureConfig[i].height                     = gAVSERVER_UI_config.yuvModeHeight;
			#else
			config->captureConfig[i].width						= 720;
			config->captureConfig[i].height 					= 512;
			#endif
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

		    if(numEncodes>0)
			  config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]		= 0;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width						= 288;
			config->captureConfig[i].height 					= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]		= 1;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(480);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(288);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_D1_D1:

		    if((platform == PLATFORM_DM365)||(platform == PLATFORM_DMVA1))
		    {
			    if(gAVSERVER_UI_config.flipH || gAVSERVER_UI_config.flipV)
			    {
					if(gAVSERVER_UI_config.vsEnable)
						config->resizerClkdivN = 80;
					else if(gAVSERVER_UI_config.ldcEnable)
						     config->resizerClkdivN = 90;
					     else
						     config->resizerClkdivN = 80;
				}
				else
					config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
			}
		  	else if(platform == PLATFORM_DMVA2)
		  	{
			    if(gAVSERVER_UI_config.flipH||gAVSERVER_UI_config.flipV)
			    {
				    if(gAVSERVER_UI_config.vsEnable)
						config->resizerClkdivN = 100;
					else if(gAVSERVER_UI_config.ldcEnable||gAVSERVER_UI_config.snfEnable||gAVSERVER_UI_config.tnfEnable)
						     config->resizerClkdivN = 100;
					     else
						     config->resizerClkdivN = 80;
				}
				else
					config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
		    }
			else
			{
				if(gAVSERVER_UI_config.flipH||gAVSERVER_UI_config.flipV)
				{
					if(gAVSERVER_UI_config.vsEnable)
						config->resizerClkdivN = 100;
					else if(gAVSERVER_UI_config.ldcEnable||gAVSERVER_UI_config.snfEnable||gAVSERVER_UI_config.tnfEnable)
						config->resizerClkdivN = 100;
					else
						config->resizerClkdivN = 80;
				}
				else
					config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
			}

 			config->sensorMode					= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_720x480:DRV_IMGS_SENSOR_MODE_720x480;
			config->sensorFps 					= 30;
  			config->aewb_config.aewbBinEnable	= TRUE;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= gAVSERVER_UI_config.encryptEnable;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 3;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 2;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;
			config->dmvaAppConfig.captureStreamId    = 2;

			config->displayConfig.captureStreamId 	 = 0;
			config->displayConfig.width 			 = 720;
			config->displayConfig.height			 = 480;
      		config->displayConfig.expandH		     = TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width					= 720;
			config->captureConfig[i].height 				= 480;
			config->captureConfig[i].ldcEnable				= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable				= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable				= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 				= gAVSERVER_UI_config.vsEnable;

		    if(numEncodes>0)
			  config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width					= 720;
			config->captureConfig[i].height 				= 480;
			if(gAVSERVER_UI_config.demomode)
			{
				config->captureConfig[i].ldcEnable			= FALSE;
				config->captureConfig[i].snfEnable			= FALSE;
				config->captureConfig[i].tnfEnable			= FALSE;
				config->captureConfig[i].vsEnable 			= FALSE;
			}
			else {
				config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
				config->captureConfig[i].snfEnable			= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
				config->captureConfig[i].tnfEnable			= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
				config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;
			}

      		if(numEncodes>1)
  		   		config->captureConfig[i].numEncodes 		= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width					= 288;
			config->captureConfig[i].height 				= 192;
			config->captureConfig[i].ldcEnable				= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable				= (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable				= (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].vsEnable 				= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>2)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]= 2;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(480);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(480);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 2;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(288);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

	default:
    case AVSERVER_UI_CAPTURE_MODE_720P:
	case AVSERVER_UI_CAPTURE_MODE_720P_720P:
	case AVSERVER_UI_CAPTURE_MODE_720P_720P_30:

      config->sensorMode          		= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1920x1080:DRV_IMGS_SENSOR_MODE_1280x720;
      config->sensorFps           		= 30;
  	  config->aewb_config.aewbBinEnable	= TRUE;

      config->vstabTskEnable      = gAVSERVER_UI_config.vsEnable;
      config->ldcTskEnable        = gAVSERVER_UI_config.ldcEnable;
      config->vnfTskEnable        = (gAVSERVER_UI_config.snfEnable | gAVSERVER_UI_config.tnfEnable);
      config->encryptTskEnable    = gAVSERVER_UI_config.encryptEnable;

      config->captureSingleResize = FALSE;
      config->captureYuvFormat    = DRV_DATA_FORMAT_YUV420;

      config->numCaptureStream    = 2;

      if(numEncodes > config->numCaptureStream)
        numEncodes = config->numCaptureStream;

      config->numEncodeStream     = numEncodes;

        VI_DEBUG("config->numEncodeStream = %d\n", numEncodes);

      config->faceDetectConfig.captureStreamId = 1;
      config->faceDetectConfig.fdEnable        = gAVSERVER_UI_config.fdEnable;
      config->dmvaAppConfig.captureStreamId    = 1;

      config->displayConfig.captureStreamId    = 0;
      config->displayConfig.width              = 720;
      config->displayConfig.height             = 480;
      config->displayConfig.expandH		       = TRUE;

      i=0;

      k=0;
      config->captureConfig[i].width              = 1280;
      config->captureConfig[i].height             = 736;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = gAVSERVER_UI_config.snfEnable;
      config->captureConfig[i].tnfEnable          = gAVSERVER_UI_config.tnfEnable;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>0)
        config->captureConfig[i].numEncodes       = ((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))?2:1;

      config->captureConfig[i].encodeStreamId[k++]= 0;
      if((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))
      	config->captureConfig[i].encodeStreamId[k++]= 1;

      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      k=0;
      config->captureConfig[i].width              = 320;
      config->captureConfig[i].height             = 192;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
	  config->captureConfig[i].snfEnable		  = (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
	  config->captureConfig[i].tnfEnable		  = (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>1)
        config->captureConfig[i].numEncodes       = ((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))?0:1;

      config->captureConfig[i].encodeStreamId[k++]= 1;

      config->captureConfig[i].frameSkipMask      = (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)?0x1AAAAAAA:0x3FFFFFFF;
      i++;

      i=0;

      config->encodeConfig[i].captureStreamId          	= 0;
      config->encodeConfig[i].cropWidth                	= ALIGN_ENCODE(1280);
      config->encodeConfig[i].cropHeight               	= ALIGN_ENCODE(720);
      config->encodeConfig[i].frameRateBase			   	= config->sensorFps*1000;
      config->encodeConfig[i].frameSkipMask            	= 0x3FFFFFFF;
      config->encodeConfig[i].codecType                	= gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             	= gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            	= gAVSERVER_UI_config.encryptEnable;
      config->encodeConfig[i].fileSaveEnable           	= FALSE;
      config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
      config->encodeConfig[i].qValue                   	= gAVSERVER_UI_config.codecBitrate[i];
      i++;

	  if((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30)) {
        config->encodeConfig[i].captureStreamId         = 0;
      	config->encodeConfig[i].cropWidth               = ALIGN_ENCODE(1280);
      	config->encodeConfig[i].cropHeight              = ALIGN_ENCODE(720);
      	config->encodeConfig[i].frameRateBase		 	= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)?(config->sensorFps*1000)>>1:(config->sensorFps*1000);
	  }
	  else {
        config->encodeConfig[i].captureStreamId         = 1;
      	config->encodeConfig[i].cropWidth               = ALIGN_ENCODE(320);
      	config->encodeConfig[i].cropHeight              = ALIGN_ENCODE(192);
      	config->encodeConfig[i].frameRateBase		 	= (config->sensorFps*1000);
  	  }
      config->encodeConfig[i].frameSkipMask             = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                	= gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             	= gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            	= gAVSERVER_UI_config.encryptEnable;
      config->encodeConfig[i].fileSaveEnable           	= FALSE;
      config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
      config->encodeConfig[i].qValue                   	= gAVSERVER_UI_config.codecBitrate[i];
      i++;

      break;

    case AVSERVER_UI_CAPTURE_MODE_720P_VGA:
    case AVSERVER_UI_CAPTURE_MODE_720P_VGA_30:

	  if(platform==PLATFORM_DM365)
		config->resizerClkdivN = 50;
	  else if(platform==PLATFORM_DMVA1)
		config->resizerClkdivN = 80;
	  else
		config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;

      config->sensorMode          		= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1280x720:DRV_IMGS_SENSOR_MODE_1280x720;
      config->sensorFps           		= 30;
  	  config->aewb_config.aewbBinEnable	= TRUE;

      config->vstabTskEnable      = gAVSERVER_UI_config.vsEnable;
      config->ldcTskEnable        = gAVSERVER_UI_config.ldcEnable;
      config->vnfTskEnable        = gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
      config->encryptTskEnable    = gAVSERVER_UI_config.encryptEnable;

      config->captureRawInMode	  = AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
      config->captureSingleResize = FALSE;
      config->captureYuvFormat    = DRV_DATA_FORMAT_YUV420;

      config->numCaptureStream    = 3;

      if(numEncodes > config->numCaptureStream)
        numEncodes = config->numCaptureStream;

      config->numEncodeStream     = numEncodes;

      config->faceDetectConfig.captureStreamId = 2;
      config->faceDetectConfig.fdEnable        = gAVSERVER_UI_config.fdEnable;
      config->dmvaAppConfig.captureStreamId    = 2;

      config->displayConfig.captureStreamId    = 1;
      config->displayConfig.width              = 720;
      config->displayConfig.height             = 480;
	  config->displayConfig.expandH		       = TRUE;

      i=0;

      k=0;
      config->captureConfig[i].width              = 1280;
      config->captureConfig[i].height             = 736;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = gAVSERVER_UI_config.snfEnable;
      config->captureConfig[i].tnfEnable          = gAVSERVER_UI_config.tnfEnable;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>0)
        config->captureConfig[i].numEncodes         = 1;

      config->captureConfig[i].encodeStreamId[k++]= 0;
      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      k=0;
      config->captureConfig[i].width            = 640;
      config->captureConfig[i].height           = 352;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
      config->captureConfig[i].tnfEnable          = (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>1)
        config->captureConfig[i].numEncodes       = 1;

      config->captureConfig[i].encodeStreamId[k++]= 1;

      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
	      config->captureConfig[i].frameSkipMask      = 0x1AAAAAAA;
	    else
	    	config->captureConfig[i].frameSkipMask    = 0x3FFFFFFF;

      i++;

      k=0;
      config->captureConfig[i].width              = 320;
      config->captureConfig[i].height             = 192;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
	  config->captureConfig[i].snfEnable		  = (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
	  config->captureConfig[i].tnfEnable		  = (numEncodes>2)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>2)
        config->captureConfig[i].numEncodes       = 1;

      config->captureConfig[i].encodeStreamId[k++]= 2;

      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
	      config->captureConfig[i].frameSkipMask      = 0x1AAAAAAA;
	    else
 	      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      i=0;

      config->encodeConfig[i].captureStreamId          = 0;
      config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(1280);
      config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(720);
      config->encodeConfig[i].frameRateBase			   = config->sensorFps*1000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = gAVSERVER_UI_config.encryptEnable;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      config->encodeConfig[i].captureStreamId          = 1;
	  config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(640);
   	  config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(352);
      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
      	config->encodeConfig[i].frameRateBase		   = (config->sensorFps*1000)>>1;
      else
      	config->encodeConfig[i].frameRateBase		   = config->sensorFps*1000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = gAVSERVER_UI_config.encryptEnable;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      config->encodeConfig[i].captureStreamId          = 2;
      config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(320);
      config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(192);
      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
      	config->encodeConfig[i].frameRateBase		 	= (config->sensorFps*1000)>>1;
      else
      	config->encodeConfig[i].frameRateBase		 	= config->sensorFps*1000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = gAVSERVER_UI_config.encryptEnable;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      break;

		case AVSERVER_UI_CAPTURE_MODE_SXVGA:

      		gAVSERVER_UI_config.ldcEnable = FALSE;
      		gAVSERVER_UI_config.vsEnable  = FALSE;

			config->sensorMode					= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1600x1200:DRV_IMGS_SENSOR_MODE_1280x960;
			config->sensorFps 					= 30;
  			config->aewb_config.aewbBinEnable	= FALSE;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= gAVSERVER_UI_config.encryptEnable;

			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;
			config->dmvaAppConfig.captureStreamId    = 1;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 			 = 720;
			config->displayConfig.height			 = 480;
			config->displayConfig.expandH            = TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width							= 1280;
			config->captureConfig[i].height 						= 960;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width							= 256;
			config->captureConfig[i].height 						= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(1280);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(960);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_1080P:
		case AVSERVER_UI_CAPTURE_MODE_1080P_D1:

			gAVSERVER_UI_config.ldcEnable = FALSE;
			gAVSERVER_UI_config.vsEnable  = FALSE;

			config->sensorMode = DRV_IMGS_SENSOR_MODE_1920x1080;
			config->sensorFps  = ((platform == PLATFORM_DM365) || (platform == PLATFORM_DMVA1))?24:30;

			if((platform == PLATFORM_DMVA1)||(platform == PLATFORM_DMVA2))
			{
				gAVSERVER_UI_config.snfEnable = FALSE;
				gAVSERVER_UI_config.tnfEnable = FALSE;
			}

  			config->aewb_config.aewbBinEnable	= FALSE;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= gAVSERVER_UI_config.encryptEnable;

			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 		 = (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?FALSE:gAVSERVER_UI_config.fdEnable;
		    config->dmvaAppConfig.captureStreamId    = 1;

			if(gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_1080P_D1)
				config->dmvaAppConfig.dmvaEnable = FALSE;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 			 = 720;
			config->displayConfig.height			 = 480;
			config->displayConfig.expandH            = TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width						= 1920;
			config->captureConfig[i].height 					= 1080;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;
			config->captureConfig[i].dmaInterval				= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?0x40:0x20;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]		= 0;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width			= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?720:320;
			config->captureConfig[i].height 		= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?480:192;
			config->captureConfig[i].ldcEnable		= gAVSERVER_UI_config.ldcEnable;
	  		config->captureConfig[i].snfEnable		= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
	  		config->captureConfig[i].tnfEnable		= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 		= gAVSERVER_UI_config.vsEnable;
			config->captureConfig[i].dmaInterval	= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?0x40:0x20;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask		= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= 1920;
			config->encodeConfig[i].cropHeight					= 1080;
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable	= FALSE;
			config->encodeConfig[i].qValue						= gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 	= 1;
			config->encodeConfig[i].cropWidth 					= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?720:320;
			config->encodeConfig[i].cropHeight					= (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_1080P_D1)?480:192;
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue						= gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_2_MEGA:

      		gAVSERVER_UI_config.ldcEnable = FALSE;
      		gAVSERVER_UI_config.vsEnable  = FALSE;
      		gAVSERVER_UI_config.snfEnable = FALSE;
      		gAVSERVER_UI_config.tnfEnable = FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_1600x1200;
			config->sensorFps 					= ((platform == PLATFORM_DM365) || (platform == PLATFORM_DMVA1))?24:30;
  			config->aewb_config.aewbBinEnable	= FALSE;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= gAVSERVER_UI_config.encryptEnable;

			config->captureSingleResize 		= FALSE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream			= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 			= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;
			config->dmvaAppConfig.captureStreamId    = 1;
			config->dmvaAppConfig.dmvaEnable         = FALSE;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 			 = 720;
			config->displayConfig.height			 = 480;
			config->displayConfig.expandH            = TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width						= 1600;
			config->captureConfig[i].height 					= 1200;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width						= 256;
			config->captureConfig[i].height 					= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 		= 1;

			config->captureConfig[i].encodeStreamId[k++]	= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= ALIGN_ENCODE(1600);
			config->encodeConfig[i].cropHeight					= ALIGN_ENCODE(1200);
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 	= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 	= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 	= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 	= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 	= 1;
			config->encodeConfig[i].cropWidth 				 	= ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight				 	= ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 	= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 	= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 	= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 	= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 	= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_3_MEGA:

		   gAVSERVER_UI_config.ldcEnable = FALSE;
		   gAVSERVER_UI_config.vsEnable  = FALSE;
		   gAVSERVER_UI_config.snfEnable = FALSE;
		   gAVSERVER_UI_config.tnfEnable = FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_2048x1536;
			config->sensorFps 					= ((platform == PLATFORM_DM365) || (platform == PLATFORM_DMVA1))?15:20;
  			config->aewb_config.aewbBinEnable	= FALSE;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= FALSE;

			config->captureSingleResize 		= FALSE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream			= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 					= numEncodes;

			config->faceDetectConfig.captureStreamId 	= 1;
			config->faceDetectConfig.fdEnable 			= gAVSERVER_UI_config.fdEnable;
			config->dmvaAppConfig.captureStreamId       = 1;
			config->dmvaAppConfig.dmvaEnable            = FALSE;

			config->displayConfig.captureStreamId 	 	= 1;
			config->displayConfig.width 				= 720;
			config->displayConfig.height				= 480;
			config->displayConfig.expandH            	= TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width				= 2048;
			config->captureConfig[i].height 			= 1536;
			config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable			= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable			= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 		= 1;

			config->captureConfig[i].encodeStreamId[k++]	= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width					= 256;
			config->captureConfig[i].height 				= 192;
			config->captureConfig[i].ldcEnable				= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable				= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable				= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 				= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 		= 1;

			config->captureConfig[i].encodeStreamId[k++]	= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(2048);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(1536);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_5_MEGA:

		    gAVSERVER_UI_config.ldcEnable 		= FALSE;
		    gAVSERVER_UI_config.vsEnable  		= FALSE;
		    gAVSERVER_UI_config.snfEnable 		= FALSE;
		    gAVSERVER_UI_config.tnfEnable 		= FALSE;

			config->resizerClkdivN				= 60;
			config->sensorMode					= DRV_IMGS_SENSOR_MODE_2592x1920;
			config->sensorFps 					= 10;
  			config->aewb_config.aewbBinEnable	= FALSE;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= FALSE;

      		config->captureRawInMode			= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
			config->captureSingleResize 		= TRUE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

	  		config->numCaptureStream		  	= 1;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 					= numEncodes;

			config->faceDetectConfig.captureStreamId 	= 1;
			config->faceDetectConfig.fdEnable 		 	= FALSE;
			config->faceDetectConfig.fdTracker 		 	= FALSE;
			config->dmvaAppConfig.captureStreamId       = 1;
			config->dmvaAppConfig.dmvaEnable            = FALSE;

	  		config->displayConfig.captureStreamId    	= 0;
			config->displayConfig.width 			 	= 720;
			config->displayConfig.height			 	= 480;
			config->displayConfig.expandH            	= TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width				= 2592;
			config->captureConfig[i].height 			= 1920;
			config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable			= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable			= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask		= 0x3FFFFFFF;

			i=0;
			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= ALIGN_ENCODE(2592);
			config->encodeConfig[i].cropHeight					= ALIGN_ENCODE(1920);
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];

			break;

		case AVSERVER_UI_CAPTURE_MODE_720P_MAX:

		    gAVSERVER_UI_config.ldcEnable 		= FALSE;
		    gAVSERVER_UI_config.vsEnable  		= FALSE;
		    gAVSERVER_UI_config.snfEnable 		= FALSE;
		    gAVSERVER_UI_config.tnfEnable 		= FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_1280x720;
			config->sensorFps 					= ((platform == PLATFORM_DM365) || (platform == PLATFORM_DMVA1))?45:60;
  			config->aewb_config.aewbBinEnable	= FALSE;
  			config->aewb_config.reduceShutter	= 200;
			config->aewb_config.saldreEnable 	= FALSE;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= gAVSERVER_UI_config.encryptEnable;

			config->captureSingleResize 		= FALSE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

	  		config->numCaptureStream		  	= 1;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 					= numEncodes;

			config->faceDetectConfig.captureStreamId 	= 1;
			config->faceDetectConfig.fdEnable 		 	= FALSE;
			config->faceDetectConfig.fdTracker 		 	= FALSE;
			config->dmvaAppConfig.captureStreamId       = 1;
			config->dmvaAppConfig.dmvaEnable            = FALSE;

	  		config->displayConfig.captureStreamId    	= 0;
			config->displayConfig.width 			 	= 720;
			config->displayConfig.height			 	= 480;
			config->displayConfig.expandH            	= TRUE;

			i=0;

			k=0;
			config->captureConfig[i].width				= 1280;
			config->captureConfig[i].height 			= 720;
			config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable			= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable			= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask		= 0x3FFFFFFF;

			i=0;
			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= ALIGN_ENCODE(1280);
			config->encodeConfig[i].cropHeight					= ALIGN_ENCODE(720);
			config->encodeConfig[i].frameRateBase				= config->sensorFps*1000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= gAVSERVER_UI_config.encryptEnable;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];

			break;
  }

  {
	  if(config->numEncodeStream == 1) {
	  	if(config->encodeConfig[0].codecType == ALG_VID_CODEC_MJPEG) //check megapixel JPEG or not
		 	config->streamConfig.mem_layou_set = MEM_LAYOUT_2;
		else if(config->sensorMode == DRV_IMGS_SENSOR_MODE_2048x1536) //check 3MP H264 or not
		 	config->streamConfig.mem_layou_set = MEM_LAYOUT_1;
		else if(config->sensorMode == DRV_IMGS_SENSOR_MODE_2592x1920) //check 5MP H264 or not
		 	config->streamConfig.mem_layou_set = MEM_LAYOUT_1;
		else
		 	config->streamConfig.mem_layou_set = MEM_LAYOUT_0;
	  }
	  else if(config->numEncodeStream == 2) {
		if(gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_D1_D1) {
			if(config->encodeConfig[1].codecType == ALG_VID_CODEC_MJPEG) //check second stream JPEG or not
				config->streamConfig.mem_layou_set = MEM_LAYOUT_4;
			else
				config->streamConfig.mem_layou_set = MEM_LAYOUT_3;
		}
		else {
			if(config->encodeConfig[1].codecType == ALG_VID_CODEC_MJPEG) //check second stream JPEG or not
				config->streamConfig.mem_layou_set = MEM_LAYOUT_6;
			else
				config->streamConfig.mem_layou_set = MEM_LAYOUT_5;
		}
      }
      else {
	  	config->streamConfig.mem_layou_set = MEM_LAYOUT_7;  //set default memory layout
	  }
  }

  config->alarmEnable = (AlarmDrvInit(ALARM_AVSERVER_MSG) != 0) ? 0:1;

  if(config->vstabTskEnable)
    config->sensorMode |= DRV_IMGS_SENSOR_MODE_VSTAB;

}
