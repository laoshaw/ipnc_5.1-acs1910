
#include <avserver_dmva.h>
#include <avserver_ui.h>
//#define DMVA_EVT_MGR 1

#ifdef DMVA_EVT_MGR
#include "App_common.h"
#include "App_eventMgr.h"
extern EventMgr *gEvtMgr;
#endif

int     DEF_INCOLS;
#define DEF_INROWS          192

float   DEF_INCOLS_SCALE;
#define DEF_INROWS_SCALE    ((float)DEF_INROWS/100.0)

float   DEF_META_W_SCALE;
#define DEF_META_H_SCALE    ((float)100.0/DEF_INROWS)

/* Pre-processor directives */

#define ENABLE_EVT_RECORD

/* Global variables */

int gDMVA_ControlFps = 0;
int gObjectCount = 0;
int gDmvaMinEvtRecInt;
int gFrameSkipMaskInitDone = 0;
char gDMVA_metaDataBuf[VIDEO_BUF_METADATA_SIZE];
DmvaRoiCtrl_t gDmvaRoiCtrl;

/* Global structures */

ALG_DmvalCreateParams gParams;
DmvaEvt_t             gDmvaEvt;
DmvaFpsControl_t      gDMVA_fpsControl;

/* Function Definitions */
/*----------------------*/

/* Frame Skip Mask Init */

int VIDEO_dmvaFrameSkipMaskInit()
{
	gDMVA_fpsControl.numSensorFps = 0;

	// CASE 1 :sensor fps = 30
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].sensorFps           = 30;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[0]          = 15;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[0]    = 0x1; // 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[0] = 2;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[1]          = 10;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[1]    = 0x1; // 001 001 001 001 001 001 001 001 001 001
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[1] = 3;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[2]          = 5;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[2]    = 0x1; // 000001 000001 000001 000001 000001
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[2] = 6;

	gDMVA_fpsControl.numSensorFps ++;

	if(gDMVA_fpsControl.numSensorFps >= DMVA_MAX_NUM_SENSOR_FPS)
	{
		goto exit;
	}

	// CASE 2 :sensor fps = 24
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].sensorFps           = 24;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[0]          = 15;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[0]    = 0x6D;   // 01101101 01101101 01101101
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[0] = 8;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[1]          = 10;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[1]    = 0x552;  // 010101010010 010101010010
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[1] = 12;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[2]          = 5;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[2]    = 0x111110;    // 000100010001000100010000
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[2] = 24;

	gDMVA_fpsControl.numSensorFps ++;

	if(gDMVA_fpsControl.numSensorFps >= DMVA_MAX_NUM_SENSOR_FPS)
	{
		goto exit;
	}

	// CASE 3 :sensor fps = 20
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].sensorFps           = 20;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[0]          = 15;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[0]    = 0xD; // 1101 1101 1101 1101 1101
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[0] = 4;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[1]          = 10;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[1]    = 0x1; // 01 01 01 01 01 01 01 01 01 01
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[1] = 2;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[2]          = 5;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[2]    = 0x1; // 0001 0001 0001 0001 0001
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[2] = 4;

	gDMVA_fpsControl.numSensorFps ++;

	if(gDMVA_fpsControl.numSensorFps >= DMVA_MAX_NUM_SENSOR_FPS)
	{
		goto exit;
	}

	// CASE 4 :sensor fps = 15
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].sensorFps           = 15;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[0]          = 15;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[0]    = 0x1; // 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[0] = 1;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[1]          = 10;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[1]    = 0x5; // 101 101 101 101 101
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[1] = 3;

	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].dmvaFps[2]          = 5;
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMask[2]    = 0x1; // 001 001 001 001 001
	gDMVA_fpsControl.frameSkipMask[gDMVA_fpsControl.numSensorFps].frameSkipMaskLen[2] = 3;

	gDMVA_fpsControl.numSensorFps ++;

exit:
	gDMVA_fpsControl.curDmvaFps          = 0;
	gDMVA_fpsControl.curSkipFrameMask    = 0;
	gDMVA_fpsControl.curSkipFrameMaskLen = 0;

	gFrameSkipMaskInitDone = 1;

	return OSA_SOK;
}

/* Obtain the frame skip mask and the frame skip mask length based on sensor and DMVA fps */

int VIDEO_dmvaGetFrameSkipMask(int sensorFps,int dmvaFps)
{
	int i,j;

	for(i = 0;i < gDMVA_fpsControl.numSensorFps;i ++)
	{
		if(sensorFps == gDMVA_fpsControl.frameSkipMask[i].sensorFps)
		{
			for(j = 0;j < DMVA_MAX_NUM_FPS;j ++)
			{
				if(dmvaFps == gDMVA_fpsControl.frameSkipMask[i].dmvaFps[j])
				{
					gDMVA_fpsControl.curSkipFrameMask    = gDMVA_fpsControl.frameSkipMask[i].frameSkipMask[j];
					gDMVA_fpsControl.curSkipFrameMaskLen = gDMVA_fpsControl.frameSkipMask[i].frameSkipMaskLen[j];
					return OSA_SOK;
				}
			}
		}
	}

	gDMVA_fpsControl.curSkipFrameMask    = 0;
	gDMVA_fpsControl.curSkipFrameMaskLen = 0;

	return OSA_SOK;
}

/* This fn copies the video frame captured into DMVA specific set of buffers */
/* It is called from CAPTURE/VNF task */

int VIDEO_dmvaCopyRun(int tskId, int streamId, OSA_BufInfo *pBufInfo)
{
	VIDEO_CaptureStream *pCaptureStream;
	OSA_BufInfo *pDmvaBufInfo;
	VIDEO_BufHeader *pInBufHeader, *pOutBufHeader;

	DRV_FrameCopyPrm frameCopy;

	int status, dmvaBufId;

	if(gFrameSkipMaskInitDone == 0)
	{
		return OSA_SOK;
	}

	if(gDMVA_fpsControl.curDmvaFps != gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate)
	{
		gDMVA_fpsControl.curDmvaFps = gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate;

		VIDEO_dmvaGetFrameSkipMask(gAVSERVER_UI_ctrl.avserverConfig.sensorFps,
			gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate);

		OSA_printf("\nDMVA:cur sensor fps             = %d\n",gAVSERVER_UI_ctrl.avserverConfig.sensorFps);
		OSA_printf("DMVA:cur DMVA fps               = %d\n",gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate);
		OSA_printf("DMVA:cur skip frame mask        = 0x%X\n",gDMVA_fpsControl.curSkipFrameMask);
		OSA_printf("DMVA:cur skip frame mask length = %d\n\n",gDMVA_fpsControl.curSkipFrameMaskLen);

		if((gDMVA_fpsControl.curSkipFrameMask == 0) && (gDMVA_fpsControl.curSkipFrameMaskLen == 0))
		{
			gDMVA_fpsControl.numFrames2Skip   = (int)(gAVSERVER_UI_ctrl.avserverConfig.sensorFps/gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate);
		}
		else
		{
			gDMVA_fpsControl.numFrames2Skip = 0;
		}

		gDMVA_ControlFps = 0;
	}

	if((streamId < 0) || (streamId >= AVSERVER_MAX_STREAMS))
	{
		OSA_ERROR("Invalid streamId %d\n", streamId);
		return OSA_EFAIL;
	}

	// Save the address and resolution of the frame which
	// caused the DMVA evevnt.
	// Assuming that the this buffer is not overwritten untill
	// it it JPEG encoded by DMVA event task.
	if(streamId == 0)
	{
		gDmvaEvt.addr = pBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
	}

	if(streamId != gAVSERVER_config.dmvaAppConfig.captureStreamId)
	{
		return OSA_SOK;
	}

	pCaptureStream = &gVIDEO_ctrl.captureStream[streamId];

	if(pCaptureStream->dmvaCopyTsk != tskId)
	{
		return OSA_SOK;
	}

	// If code is uncommented the DMVA can be disable during Event Rec
	if(VIDEO_dmvaGetEvtRecStatus())
	{
		//return OSA_SOK;
	}

	if(gDMVA_fpsControl.numFrames2Skip == 0)
	{
		if(gDMVA_ControlFps == gDMVA_fpsControl.curSkipFrameMaskLen)
		{
			gDMVA_ControlFps = 0;
		}

		gDMVA_ControlFps ++;

		if((gDMVA_fpsControl.curSkipFrameMask & ((int)1 << (gDMVA_ControlFps - 1))) == 0)
		{
			return OSA_SOK;
		}
	}
	else
	{
		gDMVA_ControlFps ++;

		if(gDMVA_ControlFps < gDMVA_fpsControl.numFrames2Skip)
		{
			return OSA_SOK;
		}
		else
		{
			gDMVA_ControlFps = 0;
		}
	}

	status = OSA_bufGetEmpty(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, &dmvaBufId, OSA_TIMEOUT_NONE);

	if(status==OSA_SOK)
	{
		pDmvaBufInfo = OSA_bufGetBufInfo(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, dmvaBufId);

		OSA_assert(pDmvaBufInfo!=NULL);

		if(pDmvaBufInfo != NULL)
		{
			pInBufHeader = (VIDEO_BufHeader *)pBufInfo->virtAddr;
			pOutBufHeader = (VIDEO_BufHeader *)pDmvaBufInfo->virtAddr;

			OSA_assert(pInBufHeader!=NULL);

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
#ifdef AVSERVER_DEBUG_RUNNING
			OSA_printf(" DMVA: Copying to Buf %d \n", dmvaBufId);
#endif
#endif

			frameCopy.srcPhysAddr = pBufInfo->physAddr + VIDEO_BUF_HEADER_SIZE;
			frameCopy.srcVirtAddr = pBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
			frameCopy.dstPhysAddr = pDmvaBufInfo->physAddr + VIDEO_BUF_HEADER_SIZE;
			frameCopy.dstVirtAddr = pDmvaBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
			frameCopy.srcOffsetH  = pInBufHeader->offsetH;
			frameCopy.srcOffsetV  = pInBufHeader->offsetV;
			frameCopy.dstOffsetH  = gVIDEO_ctrl.dmvaAppStream.dmvaOffsetH;
			frameCopy.dstOffsetV  = gVIDEO_ctrl.dmvaAppStream.dmvaHeight;
			frameCopy.copyWidth   = gVIDEO_ctrl.dmvaAppStream.dmvaWidth;
			frameCopy.copyHeight  = gVIDEO_ctrl.dmvaAppStream.dmvaHeight;
			frameCopy.dataFormat  = gAVSERVER_config.captureYuvFormat;
			frameCopy.srcStartX   = pInBufHeader->startX;
			frameCopy.srcStartY   = pInBufHeader->startY;
			frameCopy.dstStartX   = 0;
			frameCopy.dstStartY   = 0;

			status = DRV_frameCopy(NULL, &frameCopy);

			if(status!=OSA_SOK)
			{
				OSA_ERROR("DRV_frameCopy(srcOffset:%dx%d dstOffset:%dx%d wxh:%dx%d)\n",
					frameCopy.srcOffsetH, frameCopy.srcOffsetV,
					frameCopy.dstOffsetH, frameCopy.dstOffsetV,
					frameCopy.copyWidth, frameCopy.copyHeight);
			}

			memcpy(pOutBufHeader, pInBufHeader, sizeof(VIDEO_BufHeader));
		}

		OSA_bufPutFull(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, dmvaBufId);
		OSA_tskSendMsg(&gVIDEO_ctrl.dmvaTsk, NULL, AVSERVER_CMD_NEW_DATA, NULL, 0);
	}

	return status;
}

/* This function is called from the videoSwosdThr thread */
/* It basically takes a copy of the DMVALouput structure */

int VIDEO_dmvalGetObjectStatus( DMVALout * dmvalResult)
{
	OSA_mutexLock(&gVIDEO_ctrl.dmvaAppStream.statusLock);

	if (gAVSERVER_config.dmvaAppConfig.dmvaEnable)
	{
		dmvalResult->modeResult = gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.output->modeResult;
		dmvalResult->numObjects = gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.output->numObjects;

		memcpy(dmvalResult->objMetadata, gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.output->objMetadata, sizeof(DMVALobjMeta)*dmvalResult->numObjects);
	}
	else
	{
		dmvalResult->modeResult = 0;
		dmvalResult->numObjects = 0;

		memcpy(dmvalResult->objMetadata, gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.output->objMetadata, sizeof(DMVALobjMeta)*dmvalResult->numObjects);

		// turn off all text overlap
		AVSERVER_swosdDmvaEnableText(0);
	}

	OSA_mutexUnlock(&gVIDEO_ctrl.dmvaAppStream.statusLock);

	return OSA_SOK;
}

/* This function is called from the videoSwosdThr thread */
/* It basically takes a copy of the polygon information */

int VIDEO_dmvalGetZoneStatus( DMVALzones * zones)
{
	OSA_mutexLock(&gVIDEO_ctrl.dmvaAppStream.statusLock);

	if (gAVSERVER_config.dmvaAppConfig.dmvaEnable)
	{
		zones->numZones = gVIDEO_ctrl.dmvaAppStream.zones.numZones;
		memcpy(zones->poly, gVIDEO_ctrl.dmvaAppStream.zones.poly, sizeof(DMVALpolygon)*zones->numZones);
	}
	else
	{
		zones->numZones = 0;
		memcpy(zones->poly, gVIDEO_ctrl.dmvaAppStream.zones.poly, sizeof(DMVALpolygon)*zones->numZones);
	}

	OSA_mutexUnlock(&gVIDEO_ctrl.dmvaAppStream.statusLock);

	return OSA_SOK;
}

/* This fn sets the status of zones to be used by the SWOSD thread */

int VIDEO_dmvalSetZoneStatus( int index, int numPolygons, DMVALpolygon polygons[])
{
	int i;
	OSA_mutexLock(&gVIDEO_ctrl.dmvaAppStream.statusLock);
	gVIDEO_ctrl.dmvaAppStream.zones.numZones = numPolygons;

	if(index == -1)
	{
		for(i=0;i<6;i++)
		{
			memset(&gVIDEO_ctrl.dmvaAppStream.zones.poly[i], 0, sizeof(DMVALpolygon));
		}
	}
	else
	{
		for(i=0;i<index;i++)
		{
			memset(&gVIDEO_ctrl.dmvaAppStream.zones.poly[i], 0, sizeof(DMVALpolygon));
		}

		for(i=index;i<(numPolygons+index);i++)
		{
			memcpy(&gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index], &polygons[i], sizeof(DMVALpolygon));
#if 0
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].numPoints = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].numPoints);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].valid = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].valid);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].polygonID = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].polygonID);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[0].x = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[0].x);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[0].y = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[0].y);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[1].x = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[1].x);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[1].y = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[1].y);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[2].x = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[2].x);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[2].y = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[2].y);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[3].x = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[3].x);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].pt[3].y = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].pt[3].y);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.ymin = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.ymin);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.xmin = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.xmin);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.width = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.width);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.height = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.height);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.boxArea = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.boxArea);
			printf("\n gVIDEO_ctrl.dmvaAppStream.zones.poly[%d].bBox.objArea = %d", i, gVIDEO_ctrl.dmvaAppStream.zones.poly[i-index].bBox.objArea);
#endif
		}
	}

	OSA_mutexUnlock(&gVIDEO_ctrl.dmvaAppStream.statusLock);

	return 0;
}


/* This fn reads the DMVA related params from the nand only once when the system is powered ON/restarted */

int VIDEO_dmvalTskGetParamFrmNand()
{
	int i,mid;
	int minX[4],maxX[4],minY[4],maxY[4];
	SysInfo *pSysInfo,sysInfo;

	// Zero init the DMVA roi structure
	memset(&gDmvaRoiCtrl,0x00,sizeof(DmvaRoiCtrl_t));

	/* Assumption: Sys Server writes to shared mem from nand before starting av_server process */
#if 0
	ShareMemInit(FILE_MSG_KEY);
	ShareMemRead(0,&sysInfo,sizeof(SysInfo));
	pSysInfo = &sysInfo;
#else
	mid = pShareMemInit(FILE_MSG_KEY);
	pSysInfo = shmat(mid,0,0);
	gAVSERVER_config.dmvaAppConfig.pSysInfo = (void*)pSysInfo;
#endif

	gAVSERVER_config.dmvaAppConfig.dmvaEnable                 = pSysInfo->dmva_config.dmvaenable_sys;
	gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate      = pSysInfo->dmva_config.dmvaexptalgoframerate_sys;
	gAVSERVER_config.dmvaAppConfig.dmvaexptalgodetectfreq     = pSysInfo->dmva_config.dmvaexptalgodetectfreq_sys;
	gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable    = pSysInfo->dmva_config.dmvaexptevtrecordenable_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEncRoiEnable        = pSysInfo->dmva_config.dmvaAppEncRoiEnable_sys;
	gAVSERVER_config.dmvaAppConfig.dmvaexptsmetatrackerenable = pSysInfo->dmva_config.dmvaexptSmetaTrackerEnable_sys;

	gParams.frameRate = gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate;
	gParams.detectionRate = gAVSERVER_config.dmvaAppConfig.dmvaexptalgodetectfreq;

	// MAIN page params
	gAVSERVER_config.dmvaAppParams.dmvaAppCamID                              = pSysInfo->dmva_config.dmvaAppCamID_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppGetSchedule                        = pSysInfo->dmva_config.dmvaAppGetSchedule_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgFD                              = pSysInfo->dmva_config.dmvaAppCfgFD_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc                  = pSysInfo->dmva_config.dmvaAppCfgCTDImdSmetaTzOc_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppDisplayOptions                     = pSysInfo->dmva_config.dmvaAppDisplayOptions_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventRecordingVAME                 = pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListActionPlaySendSearchTrash = pSysInfo->dmva_config.dmvaAppEventListActionPlaySendSearchTrash_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListSelectEvent               = pSysInfo->dmva_config.dmvaAppEventListSelectEvent_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListEventType                 = pSysInfo->dmva_config.dmvaAppEventListEventType_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListStartMonth                = pSysInfo->dmva_config.dmvaAppEventListStartMonth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListStartDay                  = pSysInfo->dmva_config.dmvaAppEventListStartDay_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListStartTime                 = pSysInfo->dmva_config.dmvaAppEventListStartTime_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListEndMonth                  = pSysInfo->dmva_config.dmvaAppEventListEndMonth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListEndDay                    = pSysInfo->dmva_config.dmvaAppEventListEndDay_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppEventListEndTime                   = pSysInfo->dmva_config.dmvaAppEventListEndTime_sys;

	// TZ page params
	gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity        = pSysInfo->dmva_config.dmvaAppTZSensitivity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth     = pSysInfo->dmva_config.dmvaAppTZPersonMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight    = pSysInfo->dmva_config.dmvaAppTZPersonMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth    = pSysInfo->dmva_config.dmvaAppTZVehicleMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight   = pSysInfo->dmva_config.dmvaAppTZVehicleMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth     = pSysInfo->dmva_config.dmvaAppTZPersonMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight    = pSysInfo->dmva_config.dmvaAppTZPersonMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth    = pSysInfo->dmva_config.dmvaAppTZVehicleMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight   = pSysInfo->dmva_config.dmvaAppTZVehicleMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection          = pSysInfo->dmva_config.dmvaAppTZDirection_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust         = pSysInfo->dmva_config.dmvaAppTZPresentAdjust_sys;

	gAVSERVER_config.dmvaAppParams.dmvaAppTZEnable             = pSysInfo->dmva_config.dmvaAppTZEnable_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides = pSysInfo->dmva_config.dmvaAppTZZone1_ROI_numSides_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides = pSysInfo->dmva_config.dmvaAppTZZone2_ROI_numSides_sys;

	for(i = 0;i < 16;i ++)
	{
		gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] = pSysInfo->dmva_config.dmvaAppTZZone1_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] = pSysInfo->dmva_config.dmvaAppTZZone1_y[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] = pSysInfo->dmva_config.dmvaAppTZZone2_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] = pSysInfo->dmva_config.dmvaAppTZZone2_y[i + 1];
	}

	// CTD page params
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv1        = pSysInfo->dmva_config.dmvaAppCfgTDEnv1_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv2        = pSysInfo->dmva_config.dmvaAppCfgTDEnv2_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity = pSysInfo->dmva_config.dmvaAppCfgTDSensitivity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime   = pSysInfo->dmva_config.dmvaAppCfgTDResetTime_sys;

	// OC page params
	gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity  = pSysInfo->dmva_config.dmvaAppOCSensitivity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth  = pSysInfo->dmva_config.dmvaAppOCObjectWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight = pSysInfo->dmva_config.dmvaAppOCObjectHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection    = pSysInfo->dmva_config.dmvaAppOCDirection_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCEnable       = pSysInfo->dmva_config.dmvaAppOCEnable_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight    = pSysInfo->dmva_config.dmvaAppOCLeftRight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppOCTopBottom    = pSysInfo->dmva_config.dmvaAppOCTopBottom_sys;

	// SMETA page params
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity          = pSysInfo->dmva_config.dmvaAppSMETASensitivity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPresentAdjust        = pSysInfo->dmva_config.dmvaAppSMETAPresentAdjust_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth       = pSysInfo->dmva_config.dmvaAppSMETAPersonMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight      = pSysInfo->dmva_config.dmvaAppSMETAPersonMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth       = pSysInfo->dmva_config.dmvaAppSMETAPersonMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight      = pSysInfo->dmva_config.dmvaAppSMETAPersonMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth      = pSysInfo->dmva_config.dmvaAppSMETAVehicleMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight     = pSysInfo->dmva_config.dmvaAppSMETAVehicleMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth      = pSysInfo->dmva_config.dmvaAppSMETAVehicleMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight     = pSysInfo->dmva_config.dmvaAppSMETAVehicleMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData  = pSysInfo->dmva_config.dmvaAppSMETAEnableStreamingData_sys;

	gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones             = pSysInfo->dmva_config.dmvaAppSMETANumZones_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides   = pSysInfo->dmva_config.dmvaAppSMETAZone1_ROI_numSides_sys;

	for(i = 0;i < 16;i ++)
	{
		gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] = pSysInfo->dmva_config.dmvaAppSMETAZone1_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] = pSysInfo->dmva_config.dmvaAppSMETAZone1_y[i + 1];
	}

	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamBB            = pSysInfo->dmva_config.dmvaAppSMETAStreamBB_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamVelocity      = pSysInfo->dmva_config.dmvaAppSMETAStreamVelocity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamHistogram     = pSysInfo->dmva_config.dmvaAppSMETAStreamHistogram_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamMoments       = pSysInfo->dmva_config.dmvaAppSMETAStreamMoments_sys;

	printf("gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity = %d\n", gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity);
	printf("gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPresentAdjust = %d\n", gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPresentAdjust);

	// IMD page params
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity      = pSysInfo->dmva_config.dmvaAppIMDSensitivity_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust    = pSysInfo->dmva_config.dmvaAppIMDPresentAdjust_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth   = pSysInfo->dmva_config.dmvaAppIMDPersonMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight  = pSysInfo->dmva_config.dmvaAppIMDPersonMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth  = pSysInfo->dmva_config.dmvaAppIMDVehicleMinWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight = pSysInfo->dmva_config.dmvaAppIMDVehicleMinHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth   = pSysInfo->dmva_config.dmvaAppIMDPersonMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight  = pSysInfo->dmva_config.dmvaAppIMDPersonMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth  = pSysInfo->dmva_config.dmvaAppIMDVehicleMaxWidth_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight = pSysInfo->dmva_config.dmvaAppIMDVehicleMaxHeight_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones         = pSysInfo->dmva_config.dmvaAppIMDNumZones_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable       = pSysInfo->dmva_config.dmvaAppIMDZoneEnable_sys;

	gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides = pSysInfo->dmva_config.dmvaAppIMDZone1_ROI_numSides_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides = pSysInfo->dmva_config.dmvaAppIMDZone2_ROI_numSides_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides = pSysInfo->dmva_config.dmvaAppIMDZone3_ROI_numSides_sys;
	gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides = pSysInfo->dmva_config.dmvaAppIMDZone4_ROI_numSides_sys;

	for(i = 0;i < 16;i ++)
	{
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone1_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone1_y[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone2_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone2_y[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone3_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone3_y[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone4_x[i + 1];
		gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] = pSysInfo->dmva_config.dmvaAppIMDZone4_y[i + 1];
	}

	// Setting up the DMVAL algorithm interface structure
	gParams.enableSaveLogFile = 0;
	gParams.detectMode        = gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc;  // Detect mode

	if(gParams.detectMode & DMVAL_DETECTMODE_TRIP)
	{
		gParams.direction              = gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection;
		gParams.dmvaAppTZPresentAdjust = gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust;
		gParams.sensitiveness          = gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight;

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];
			gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
			gParams.polygon[1].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];
			gParams.polygon[1].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];

			// find out min and max X and Y
			if(i == 0)
			{
				minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];
				minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
				minX[1] = maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];
				minY[1] = maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];
			}
			else
			{
				// zone 1
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] < minX[0])
						minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] > maxX[0])
						maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] < minY[0])
						minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] > maxY[0])
						maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
				}

				// zone 2
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] < minX[1])
						minX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] > maxX[1])
						maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] < minY[1])
						minY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] > maxY[1])
						maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];
				}
			}
		}

		gDmvaRoiCtrl.numRoi = 2;

		// zone 1
		gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = minX[0];
		gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = minY[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = maxX[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = maxY[0];

		// zone 2
		gDmvaRoiCtrl.dmvaRoi[1].topLeftX     = minX[1];
		gDmvaRoiCtrl.dmvaRoi[1].topLeftY     = minY[1];
		gDmvaRoiCtrl.dmvaRoi[1].bottomRightX = maxX[1];
		gDmvaRoiCtrl.dmvaRoi[1].bottomRightY = maxY[1];

		OSA_printf(" DMVA > Trip:zone 1 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",minX[0],maxX[0],minY[0],maxY[0]);
		OSA_printf(" DMVA > Trip:zone 2 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",minX[1],maxX[1],minY[1],maxY[1]);

		/* Scaling to Normal */
		gParams.minPersonSize[0]  = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0]  = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1]  = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1]  = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[1].pt[i].x = (S16) ceil(gParams.polygon[1].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[1].pt[i].y = (S16) ceil(gParams.polygon[1].pt[i].y * DEF_INROWS_SCALE);
		}
		/* Scaling to Normal Done */

		gParams.numPolygons = 2;

		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_TZ_1A;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides;

		gParams.polygon[1].polygonID = 2;
		gParams.polygon[1].valid     = 1;
		gParams.polygon[1].type      = DMVAL_TZ_1B;
		//gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[1].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides;

		pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys = 33;
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_IMD)
	{
		gParams.dmvaAppIMDPresentAdjust = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust;
		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight;

		/* Scaling to Normal  */
		gParams.minPersonSize[0]  = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0]  = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1]  = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1]  = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];
			gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
			gParams.polygon[1].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];
			gParams.polygon[1].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
			gParams.polygon[2].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];
			gParams.polygon[2].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
			gParams.polygon[3].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];
			gParams.polygon[3].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];

			/* Scaling to Normal  */
			gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[1].pt[i].x = (S16) ceil(gParams.polygon[1].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[1].pt[i].y = (S16) ceil(gParams.polygon[1].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[2].pt[i].x = (S16) ceil(gParams.polygon[2].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[2].pt[i].y = (S16) ceil(gParams.polygon[2].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[3].pt[i].x = (S16) ceil(gParams.polygon[3].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[3].pt[i].y = (S16) ceil(gParams.polygon[3].pt[i].y * DEF_INROWS_SCALE);

			// find out min and max X and Y
			if(i == 0)
			{
				minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];
				minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
				minX[1] = maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];
				minY[1] = maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
				minX[2] = maxX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];
				minY[2] = maxY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
				minX[3] = maxX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];
				minY[3] = maxY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];
			}
			else
			{
				// zone 1
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] < minX[0])
						minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] > maxX[0])
						maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] < minY[0])
						minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] > maxY[0])
						maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
				}

				// zone 2
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] < minX[1])
						minX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] > maxX[1])
						maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] < minY[1])
						minY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] > maxY[1])
						maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
				}

				// zone 3
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] < minX[2])
						minX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] > maxX[2])
						maxX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] < minY[2])
						minY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] > maxY[2])
						maxY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
				}

				// zone 4
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] < minX[3])
						minX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] > maxX[3])
						maxX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] < minY[3])
						minY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] > maxY[3])
						maxY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];
				}
			}
		}

		gDmvaRoiCtrl.numRoi = gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones;

		for(i = 0;i < gDmvaRoiCtrl.numRoi;i ++)
		{
			gDmvaRoiCtrl.dmvaRoi[i].topLeftX     = minX[i];
			gDmvaRoiCtrl.dmvaRoi[i].topLeftY     = minY[i];
			gDmvaRoiCtrl.dmvaRoi[i].bottomRightX = maxX[i];
			gDmvaRoiCtrl.dmvaRoi[i].bottomRightY = maxY[i];

			OSA_printf(" DMVA > IMD:zone %d - minX = %d,maxX = %d,minY = %d,maxY = %d\n",
				i,minX[i],maxX[i],minY[i],maxY[i]);
		}

		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones;

		gParams.polygon[0].polygonID = 1;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 1)
		{
			gParams.polygon[0].valid     = 1;
		}
		else
		{
			gParams.polygon[0].valid     = 0;
		}

		gParams.polygon[0].type      = DMVAL_IMD;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides;

		gParams.polygon[1].polygonID = 2;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 2)
		{
			gParams.polygon[1].valid     = 1;
		}
		else
		{
			gParams.polygon[1].valid     = 0;
		}

		gParams.polygon[1].type      = DMVAL_IMD;
		//gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[1].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides;

		gParams.polygon[2].polygonID = 3;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 4)
		{
			gParams.polygon[2].valid     = 1;
		}
		else
		{
			gParams.polygon[2].valid     = 0;
		}

		gParams.polygon[2].type      = DMVAL_IMD;
		//gParams.polygon[2].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[2].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides;

		gParams.polygon[3].polygonID = 4;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 8)
		{
			gParams.polygon[3].valid     = 1;
		}
		else
		{
			gParams.polygon[3].valid     = 0;
		}

		gParams.polygon[3].type      = DMVAL_IMD;
		//gParams.polygon[3].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[3].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides;
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_COUNT)
	{
		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity;

		gParams.direction     = gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 2) {
			gParams.direction = 1;
		}
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 1) {
			gParams.direction = 2;
		}

		gParams.orientation   = gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight;


		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight;

		// scaling to normal
		gParams.minPersonSize[0] = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1] = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);

		gDmvaRoiCtrl.numRoi = 1;

		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight == 1)
		{
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = 25;
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = 5;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = 75;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = 95;
		}
		else if(gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight == 0)
		{
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = 5;
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = 25;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = 95;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = 75;
		}

		OSA_printf(" DMVA > OC:zone 1 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX,gDmvaRoiCtrl.dmvaRoi[0].bottomRightX,
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY,gDmvaRoiCtrl.dmvaRoi[0].bottomRightY);

		pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys = 25;
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_TAMPER)
	{
		gParams.sensitivity = gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity;
		gParams.resetCounter  = (gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime * gDMVA_fpsControl.curDmvaFps);
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_SMETA)
	{
		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight;

		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones;

		// NOTE:currently only one zone is supported
		if(gParams.numPolygons > 1)
			gParams.numPolygons = 1;

		if (gParams.numPolygons  > 0)
		{
			for(i = 0;i < 16;i ++)
			{
				gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];
				gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];

				gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
				gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);

				// find out min and max X and Y
				if(i == 0)
				{
					minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];
					minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];
				}
				else
				{
					// zone 1
					if(i < gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides)
					{
						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] < minX[0])
							minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] > maxX[0])
							maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] < minY[0])
							minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] > maxY[0])
							maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];
					}
				}
			}
		}

		gDmvaRoiCtrl.numRoi = 1;

		// zone 1
		gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = minX[0];
		gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = minY[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = maxX[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = maxY[0];

		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_META;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides;

		/* Scaling to Normal  */
		gParams.minPersonSize[0] = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0] = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1] = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1] = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);
	}

	/* TZ/OC box min/max sizes in percentage */
	pSysInfo->dmva_config.dmvaTZOCMinWidthLimit_sys  = 6;
	pSysInfo->dmva_config.dmvaTZOCMinHeightLimit_sys = 9;
	pSysInfo->dmva_config.dmvaTZOCMaxWidthLimit_sys  = 25;

	/* IMD/SEMTA box min/max size limits in percentage */
	pSysInfo->dmva_config.dmvaIMDSMETAPeopleMinWidthLimit_sys  = 2;
	pSysInfo->dmva_config.dmvaIMDSMETAPeopleMinHeightLimit_sys = 7;

	if(gVIDEO_ctrl.dmvaAppStream.dmvaWidth == 288)
	{
	    pSysInfo->dmva_config.dmvaIMDSMETAPeopleMaxWidthLimit_sys  = 74;
	    pSysInfo->dmva_config.dmvaIMDSMETAPeopleMaxHeightLimit_sys = 74;
	}
	else
	{
	    pSysInfo->dmva_config.dmvaIMDSMETAPeopleMaxWidthLimit_sys  = 66;
	    pSysInfo->dmva_config.dmvaIMDSMETAPeopleMaxHeightLimit_sys = 66;
	}

	pSysInfo->dmva_config.dmvaIMDSMETAVehicleMinWidthLimit_sys  = 2;
	pSysInfo->dmva_config.dmvaIMDSMETAVehicleMinHeightLimit_sys = 7;

	if(gVIDEO_ctrl.dmvaAppStream.dmvaWidth == 288)
	{
	    pSysInfo->dmva_config.dmvaIMDSMETAVehicleMaxWidthLimit_sys  = 74;
	    pSysInfo->dmva_config.dmvaIMDSMETAVehicleMaxHeightLimit_sys = 66;
	}
	else
	{
	    pSysInfo->dmva_config.dmvaIMDSMETAVehicleMaxWidthLimit_sys  = 66;
	    pSysInfo->dmva_config.dmvaIMDSMETAVehicleMaxHeightLimit_sys = 60;
	}

	return OSA_SOK;
}

/* This fn reads the DMVA related params from the AV server global structure whenever the */
/* the new parameters are received from the gui */

int VIDEO_dmvalTskGetParamFrmAVServer()
{
	int i;
	int minX[4],maxX[4],minY[4],maxY[4];
	SysInfo *pSysInfo = gAVSERVER_config.dmvaAppConfig.pSysInfo;

	// Zero init the DMVA roi structure
	memset(&gDmvaRoiCtrl,0x00,sizeof(DmvaRoiCtrl_t));

	gParams.frameRate = gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate;
	gParams.detectionRate = gAVSERVER_config.dmvaAppConfig.dmvaexptalgodetectfreq;

	fprintf(stdout,"\n TEST DMVA FRATE=%d, DETFQ=%d, EVTRECEN=%d\n",
		gAVSERVER_config.dmvaAppConfig.dmvaexptalgoframerate,
		gAVSERVER_config.dmvaAppConfig.dmvaexptalgodetectfreq,
		gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable);

	// Setting up the DMVAL algorithm interface structure
	gParams.enableSaveLogFile = 0;
	gParams.detectMode        = gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc;  // Detect mode

	if(gParams.detectMode & DMVAL_DETECTMODE_TAMPER)
	{
		gParams.sensitivity   = gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity;
		gParams.resetCounter   = (gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime * gDMVA_fpsControl.curDmvaFps);
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_TRIP)
	{
		pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys = 33;

		gParams.direction = gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection;
		gParams.dmvaAppTZPresentAdjust     = gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust;
		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight;

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];
			gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
			gParams.polygon[1].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];
			gParams.polygon[1].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];

			// find out min and max X and Y
			if(i == 0)
			{
				minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];
				minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
				minX[1] = maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];
				minY[1] = maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];
			}
			else
			{
				// zone 1
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] < minX[0])
						minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] > maxX[0])
						maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] < minY[0])
						minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] > maxY[0])
						maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i];
				}

				// zone 2
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] < minX[1])
						minX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] > maxX[1])
						maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] < minY[1])
						minY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] > maxY[1])
						maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i];
				}
			}
		}

		gDmvaRoiCtrl.numRoi = 2;

		// zone 1
		gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = minX[0];
		gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = minY[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = maxX[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = maxY[0];

		// zone 2
		gDmvaRoiCtrl.dmvaRoi[1].topLeftX     = minX[1];
		gDmvaRoiCtrl.dmvaRoi[1].topLeftY     = minY[1];
		gDmvaRoiCtrl.dmvaRoi[1].bottomRightX = maxX[1];
		gDmvaRoiCtrl.dmvaRoi[1].bottomRightY = maxY[1];

		OSA_printf(" DMVA > Trip:zone 1 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",minX[0],maxX[0],minY[0],maxY[0]);
		OSA_printf(" DMVA > Trip:zone 2 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",minX[1],maxX[1],minY[1],maxY[1]);

		/* Scaling to Normal */
		gParams.minPersonSize[0] = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0] = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1] = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1] = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[1].pt[i].x = (S16) ceil(gParams.polygon[1].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[1].pt[i].y = (S16) ceil(gParams.polygon[1].pt[i].y * DEF_INROWS_SCALE);
		}
		/* Scaling to Normal Done */

		gParams.numPolygons = 2;

		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_TZ_1A;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides;

		gParams.polygon[1].polygonID = 2;
		gParams.polygon[1].valid     = 1;
		gParams.polygon[1].type      = DMVAL_TZ_1B;
		//gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[1].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides;
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_IMD)
	{
		gParams.dmvaAppIMDPresentAdjust     = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust;
		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight;

		/* Scaling to Normal  */
		gParams.minPersonSize[0]  = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0]  = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1]  = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1]  = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);

		for(i = 0;i < 16;i ++)
		{
			gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];
			gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
			gParams.polygon[1].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];
			gParams.polygon[1].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
			gParams.polygon[2].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];
			gParams.polygon[2].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
			gParams.polygon[3].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];
			gParams.polygon[3].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];

			gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[1].pt[i].x = (S16) ceil(gParams.polygon[1].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[1].pt[i].y = (S16) ceil(gParams.polygon[1].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[2].pt[i].x = (S16) ceil(gParams.polygon[2].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[2].pt[i].y = (S16) ceil(gParams.polygon[2].pt[i].y * DEF_INROWS_SCALE);
			gParams.polygon[3].pt[i].x = (S16) ceil(gParams.polygon[3].pt[i].x * DEF_INCOLS_SCALE);
			gParams.polygon[3].pt[i].y = (S16) ceil(gParams.polygon[3].pt[i].y * DEF_INROWS_SCALE);

			// find out min and max X and Y
			if(i == 0)
			{
				minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];
				minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
				minX[1] = maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];
				minY[1] = maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
				minX[2] = maxX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];
				minY[2] = maxY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
				minX[3] = maxX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];
				minY[3] = maxY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];
			}
			else
			{
				// zone 1
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] < minX[0])
						minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] > maxX[0])
						maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] < minY[0])
						minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] > maxY[0])
						maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i];
				}

				// zone 2
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] < minX[1])
						minX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] > maxX[1])
						maxX[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] < minY[1])
						minY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] > maxY[1])
						maxY[1] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i];
				}

				// zone 3
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] < minX[2])
						minX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] > maxX[2])
						maxX[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] < minY[2])
						minY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] > maxY[2])
						maxY[2] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i];
				}

				// zone 4
				if(i < gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides)
				{
					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] < minX[3])
						minX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] > maxX[3])
						maxX[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] < minY[3])
						minY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];

					if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] > maxY[3])
						maxY[3] = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i];
				}
			}
		}
		/* Scaling to Normal Done */

		gDmvaRoiCtrl.numRoi = gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones;

		for(i = 0;i < gDmvaRoiCtrl.numRoi;i ++)
		{
			gDmvaRoiCtrl.dmvaRoi[i].topLeftX     = minX[i];
			gDmvaRoiCtrl.dmvaRoi[i].topLeftY     = minY[i];
			gDmvaRoiCtrl.dmvaRoi[i].bottomRightX = maxX[i];
			gDmvaRoiCtrl.dmvaRoi[i].bottomRightY = maxY[i];

			OSA_printf(" DMVA > IMD:zone %d - minX = %d,maxX = %d,minY = %d,maxY = %d\n",
				i,minX[i],maxX[i],minY[i],maxY[i]);
		}

		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones;

		gParams.polygon[0].polygonID = 1;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 1)
		{
			gParams.polygon[0].valid     = 1;
		}
		else
		{
			gParams.polygon[0].valid     = 0;
		}

		gParams.polygon[0].type      = DMVAL_IMD;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides;

		gParams.polygon[1].polygonID = 2;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 2)
		{
			gParams.polygon[1].valid     = 1;
		}
		else
		{
			gParams.polygon[1].valid     = 0;
		}

		gParams.polygon[1].type      = DMVAL_IMD;
		//gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[1].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides;

		gParams.polygon[2].polygonID = 3;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 4)
		{
			gParams.polygon[2].valid     = 1;
		}
		else
		{
			gParams.polygon[2].valid     = 0;
		}

		gParams.polygon[2].type      = DMVAL_IMD;
		//gParams.polygon[2].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[2].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides;

		gParams.polygon[3].polygonID = 4;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 8)
		{
			gParams.polygon[3].valid     = 1;
		}
		else
		{
			gParams.polygon[3].valid     = 0;
		}

		gParams.polygon[3].type      = DMVAL_IMD;
		//gParams.polygon[3].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[3].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides;
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_COUNT)
	{
		pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys = 25;

		gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity;
		if(gParams.sensitiveness > 5)
		{
			gParams.sensitiveness = 3;//Moderate
		}

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight;

		if(gParams.minPersonSize[1] > pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys)
		{
		    gParams.minPersonSize[1]                             = pSysInfo->dmva_config.dmvaTZOCMaxHeightLimit_sys;
		    pSysInfo->dmva_config.dmvaAppOCObjectHeight_sys      = gParams.minPersonSize[1];
		    gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight = gParams.minPersonSize[1];
		}

		gParams.direction     = gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 2) {
			gParams.direction = 1;
		}
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 1) {
			gParams.direction = 2;
		}
		gParams.orientation = gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight;

		// scaling to normal
		gParams.minPersonSize[0] = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1] = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);

		gDmvaRoiCtrl.numRoi = 1;

		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight == 1)
		{
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = 25;
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = 5;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = 75;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = 95;
		}
		else if(gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight == 0)
		{
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = 5;
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = 25;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = 95;
			gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = 75;
		}

		OSA_printf(" DMVA > OC:zone 1 - minX = %d,maxX = %d,minY = %d,maxY = %d\n",
			gDmvaRoiCtrl.dmvaRoi[0].topLeftX,gDmvaRoiCtrl.dmvaRoi[0].bottomRightX,
			gDmvaRoiCtrl.dmvaRoi[0].topLeftY,gDmvaRoiCtrl.dmvaRoi[0].bottomRightY);
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_TAMPER)
	{
		gParams.sensitivity  = gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity;
		gParams.resetCounter = (gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime * gDMVA_fpsControl.curDmvaFps);
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_SMETA)
	{
		gParams.dmvaAppSMETAPresentAdjust  = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPresentAdjust;
		gParams.sensitiveness              = gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity;

		gParams.minPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth;
		gParams.minPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight;

		gParams.maxPersonSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth;
		gParams.maxPersonSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight;

		gParams.minVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth;
		gParams.minVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight;

		gParams.maxVehicleSize[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth;
		gParams.maxVehicleSize[1] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight;

		/* Scaling to Normal  */
		gParams.minPersonSize[0] = (S16) ceil(gParams.minPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.minVehicleSize[0] = (S16) ceil(gParams.minVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.maxPersonSize[0] = (S16) ceil(gParams.maxPersonSize[0] * DEF_INCOLS_SCALE);
		gParams.maxVehicleSize[0] = (S16) ceil(gParams.maxVehicleSize[0] * DEF_INCOLS_SCALE);
		gParams.minPersonSize[1] = (S16) ceil(gParams.minPersonSize[1] * DEF_INROWS_SCALE);
		gParams.minVehicleSize[1] = (S16) ceil(gParams.minVehicleSize[1] * DEF_INROWS_SCALE);
		gParams.maxPersonSize[1] = (S16) ceil(gParams.maxPersonSize[1] * DEF_INROWS_SCALE);
		gParams.maxVehicleSize[1] = (S16) ceil(gParams.maxVehicleSize[1] * DEF_INROWS_SCALE);

		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones;

		// NOTE:currently only one zone is supported
		if(gParams.numPolygons > 1)
			gParams.numPolygons = 1;

		if(gParams.numPolygons  > 0)
		{
			for(i = 0;i < 16;i ++)
			{
				gParams.polygon[0].pt[i].x = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];
				gParams.polygon[0].pt[i].y = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];

				gParams.polygon[0].pt[i].x = (S16) ceil(gParams.polygon[0].pt[i].x * DEF_INCOLS_SCALE);
				gParams.polygon[0].pt[i].y = (S16) ceil(gParams.polygon[0].pt[i].y * DEF_INROWS_SCALE);

				// find out min and max X and Y
				if(i == 0)
				{
					minX[0] = maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];
					minY[0] = maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];
				}
				else
				{
					// zone 1
					if(i < gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides)
					{
						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] < minX[0])
							minX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] > maxX[0])
							maxX[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] < minY[0])
							minY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];

						if(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] > maxY[0])
							maxY[0] = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i];
					}
				}
			}
		}

		gDmvaRoiCtrl.numRoi = gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones;

		// zone 1
		gDmvaRoiCtrl.dmvaRoi[0].topLeftX     = minX[0];
		gDmvaRoiCtrl.dmvaRoi[0].topLeftY     = minY[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightX = maxX[0];
		gDmvaRoiCtrl.dmvaRoi[0].bottomRightY = maxY[0];

		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_META;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;
		gParams.polygon[0].numPoints = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides;
	}

	return OSA_SOK;
}

/* DMVA task create function */

int VIDEO_dmvalTskCreate()
{
	int streamId;
	DMVALstatus libStatus;

	int                  size;
	int bytesMemBufAllocated = 0;

	DEF_INCOLS = gVIDEO_ctrl.dmvaAppStream.dmvaWidth;
	DEF_INCOLS_SCALE = (float)DEF_INCOLS/100.0;
	DEF_META_W_SCALE = (float)100.0/DEF_INCOLS;

	// set object count to zero (global variable to keep track of objects counted)
	gObjectCount = 0;

	// Assign default values for parameters
	gParams.sensitiveness = DMVAL_SENSITIVITY_MID;
	gParams.imgType       = DMVAL_IMG_YUV420_PLANARINTERLEAVED;
	gParams.dmvalHdl      = &gVIDEO_ctrl.dmvaAppStream.algDmvalHndl;

	gParams.minPersonSize[0]            = 16;
	gParams.minPersonSize[1]            = 16;
	gParams.maxPersonSize[0]            = 64;
	gParams.maxPersonSize[1]            = 64;

	gParams.inCols       = DEF_INCOLS;
	gParams.inRows       = DEF_INROWS;
	gParams.inStride     = DEF_INCOLS;
	gParams.resetCounter = 3;

	/* Assumption: Sys Server writes to shared mem from nand before starting av_server process */

	// Obtain these paramteres from GUI
	if(VIDEO_dmvalTskGetParamFrmNand() != OSA_SOK)
		OSA_printf("\nUnable to get parameters from GUI. Using Default Parameters.\n");

	// Initialize the handle. Zero out all fields
	memset( gParams.dmvalHdl, 0, sizeof(DMVALhandle));

	/////////////////////////////////////////////////////////////////////////////
	//
	// CREATE INSTANCE of DMVAL. The call populates handle variables
	// with the memory required for permanent and output buffers
	//
	/////////////////////////////////////////////////////////////////////////////

	libStatus = ALG_dmvalCreate( &gParams );

	bytesMemBufAllocated  = gParams.dmvalHdl->bytesMemBufOutput + gParams.dmvalHdl->bytesMemBufPermanent;

	OSA_printf("\n> DMVA APP: Application Memory Allocation Summary\n            ------------------------------------------");
	OSA_printf("\n %8d bytes for DMVAL instance.", gParams.dmvalHdl->bytesMemBufPermanent);
	OSA_printf("\n %8d bytes for DMVAL output.", gParams.dmvalHdl->bytesMemBufOutput);

	OSA_printf("\n ============================================");
	OSA_printf("\n Total Allocated = %d bytes\n", bytesMemBufAllocated);
	fflush(stdout);

	VIDEO_dmvaFrameSkipMaskInit();

	return OSA_SOK;
}

/* This fn sets the parameters in the DMVA algorithm */

int VIDEO_dmvalTskSetParams()
{
	int smetaTrackerEnable;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;
	DMVALstatus libStatus;
	S16 vehiclesInFOV, peopleInFOV;

	// turn off all text overlay
	AVSERVER_swosdDmvaText("", 0);

	// calculate max no of objects that can be supported in the meta data
	if(gAVSERVER_config.dmvaAppConfig.dmvaexptsmetatrackerenable == 1)
	{
		// velocity and obj id are also present
		gAVSERVER_config.dmvaAppParams.maxNoObj = (VIDEO_BUF_METADATA_SIZE - 28)/12;
	}
	else
	{
		// no velocity and obj id
		gAVSERVER_config.dmvaAppParams.maxNoObj = (VIDEO_BUF_METADATA_SIZE - 28)/8;
	}

	if (gParams.detectMode & DMVAL_DETECTMODE_TAMPER)
	{
		printf("\n> DMVA APP: Setting up DMVA-TAMPER parameters... ");
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_TAMPERSENSITIVITY, (short*) &(gParams.sensitivity), 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_TAMPERCOUNTER, (short*) &(gParams.resetCounter), 1);

		printf("\n> DMVA APP: DONE setting up DMVA-TAMPER parameters... \n");
	}

	if (gParams.detectMode & DMVAL_DETECTMODE_IMD)
	{
		S32 id;

		printf("\n> DMVA APP: Setting up DMVA-IMD parameters... ");

		for (id = 0; id < gParams.numPolygons; id++)
		{
			libStatus = DMVAL_setROI(dmvalHdl, &(gParams.polygon[id]));

			if (libStatus != DMVAL_NO_ERROR)
			{
				printf("\nError setting up ROIs for Intelligent Motion Detection!!!\n");
			}
		}

		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_SENSITIVITY, (short*) &(gParams.sensitiveness), 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINPERSONSIZE,  gParams.minPersonSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXPERSONSIZE,  gParams.maxPersonSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINVEHICLESIZE,  gParams.minVehicleSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXVEHICLESIZE,  gParams.maxVehicleSize, 2);

		//dmvaAppIMDPresentAdjust:
		// Bit 3: person in view
		// Bit 2: vehicle in view
		vehiclesInFOV = 0;
		if ((gParams.dmvaAppIMDPresentAdjust & 0x4) == 0x4)
			vehiclesInFOV = 1;

		peopleInFOV = 0;
		if ((gParams.dmvaAppIMDPresentAdjust & 0x8) == 0x8)
			peopleInFOV = 1;

		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_VEHICLES, &vehiclesInFOV, 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_PEOPLE, &peopleInFOV, 1);

		OSA_printf("\n> DMVA APP: DONE setting up DMVA-IMD parameters... \n");
	}

	if (gParams.detectMode & DMVAL_DETECTMODE_TRIP)
	{
		S32 id;
		S16 detectionInterval;

		printf("\n> DMVA APP: Setting up TZ parameters...");

		for(id = 0; id < gParams.numPolygons; id++)
			DMVAL_setROI(dmvalHdl, &(gParams.polygon[id]));

		//dmvaAppTZPresentAdjust:
		// Bit 3: person in view
		// Bit 2: vehicle in view
		vehiclesInFOV = 0;
		if ((gParams.dmvaAppTZPresentAdjust & 0x4) == 0x4)
			vehiclesInFOV = 1;

		peopleInFOV = 0;
		if ((gParams.dmvaAppTZPresentAdjust & 0x8) == 0x8)
			peopleInFOV = 1;

		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_SENSITIVITY,  &(gParams.sensitiveness), 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINPERSONSIZE,  gParams.minPersonSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXPERSONSIZE,  gParams.maxPersonSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINVEHICLESIZE,  gParams.minVehicleSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXVEHICLESIZE,  gParams.maxVehicleSize, 2);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_VEHICLES, &vehiclesInFOV, 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_PEOPLE, &peopleInFOV, 1);

		// allow for bi-directional trip-wire
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_TZDIRECTION, &(gParams.direction), 1);

		// declare what the detection interval is
		detectionInterval = (S16)(gParams.frameRate / gParams.detectionRate);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_DETECTIONINTERVAL, &(detectionInterval), 1);

		printf("\n> DMVA APP: DONE setting up DMVA-TZ parameters\n");
	}

	if(gParams.detectMode & DMVAL_DETECTMODE_COUNT)
	{
		S16 detectionInterval;
		printf("\n> DMVA APP: Setting up OC parameters...");

		// set object count to zero (global variable to keep track of objects counted)
		gObjectCount = 0;

		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_SENSITIVITY, &(gParams.sensitiveness), 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINPERSONSIZE,  gParams.minPersonSize, 2);

		// orientation 0: move up-down, orientation 1: move left-right
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_OCORIENTATION, &(gParams.orientation), 1);

		// allow for bi-directional counting
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_OCDIRECTION, &(gParams.direction), 1);

		// declare what the detection interval is
		detectionInterval = (S16)(gParams.frameRate / gParams.detectionRate);
		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_DETECTIONINTERVAL, &(detectionInterval), 1);

		printf("\n> DMVA APP: DONE setting up DMVA-OC parameters\n");

	}

	if(gParams.detectMode & DMVAL_DETECTMODE_SMETA)
	{
		int detectionInterval;

		printf("\n> DMVA APP: Setting up SMETA parameters...");

		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_SENSITIVITY,  &(gParams.sensitiveness), 1);

		smetaTrackerEnable = gAVSERVER_config.dmvaAppConfig.dmvaexptsmetatrackerenable;
		printf("\nTracker Enable Flag = %d\n", smetaTrackerEnable);

		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_SMD_TRACKER,  &smetaTrackerEnable, 1);

		detectionInterval = (S16)(gParams.frameRate / gParams.detectionRate);
		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_DETECTIONINTERVAL, &(detectionInterval), 1);

		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINPERSONSIZE,  gParams.minPersonSize, 2);
		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXPERSONSIZE,  gParams.maxPersonSize, 2);
		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MINVEHICLESIZE,  gParams.minVehicleSize, 2);
		libStatus = DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_MAXVEHICLESIZE,  gParams.maxVehicleSize, 2);

		// TODO
		////dmvaAppIMDPresentAdjust:
		//// Bit 3: person in view
		//// Bit 2: vehicle in view
		vehiclesInFOV = 0;
		if ((gParams.dmvaAppSMETAPresentAdjust & 0x4) == 4)
			vehiclesInFOV = 1;
		peopleInFOV = 0;
		if ((gParams.dmvaAppSMETAPresentAdjust & 0x8) == 8)
			peopleInFOV = 1;

		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_VEHICLES, &vehiclesInFOV, 1);
		DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_INVIEW_PEOPLE, &peopleInFOV, 1);

		if(gParams.numPolygons > 0)
			DMVAL_setROI(dmvalHdl, &(gParams.polygon[0]));

		printf("\n> DMVA APP: Done setting up SMETA parameters\n");
	}
#ifdef DMVA_EVT_MGR
	OSA_printf("\n> DMVA EVT: Creating DMVA Event Manager instance...\n");
	EVT_createEventManager(&gEvtMgr); // AYK - 0701
#endif
	return OSA_SOK;
}

/* DMVA task delete */

int VIDEO_dmvalTskDelete()
{
	DMVALhandle* dmvalHdl;

	dmvalHdl = (DMVALhandle *)(gParams.dmvalHdl);

	ALG_dmvalDelete(dmvalHdl);

	//shmdt(gAVSERVER_config.dmvaAppConfig.pSysInfo);

	return OSA_SOK;
}

/* This fn writes the result of the DMVA algorithm to a log file */
void
VIDEO_writeLogFile(DMVALhandle * handle,
				   FILE  *fp,
				   int  fNum,
				   char  *string)
{
	int i, n, r1, c1, r2, c2, numObj;
	DMVALout * target = handle->output;
	char * p = string;

	numObj = 0;
	//if (target->modeResult == DMVAL_ACTION)
	numObj = target->numObjects;

	n = sprintf(p, "%d: %d, ", fNum, (int)target->modeResult);
	p = p + n;
	n = sprintf(p, "%d, ", numObj);
	p = p + n;

	for (i=0; i < numObj; i++)
	{
		r1 = target->objMetadata[i].objBox.ymin;
		c1 = target->objMetadata[i].objBox.xmin;
		r2 = r1 + target->objMetadata[i].objBox.height - 1;
		c2 = c1 + target->objMetadata[i].objBox.width - 1;

		n = sprintf(p, "%d %d %d %d, ", c1, r1, c2, r2);
		p = p + n;
	}

	for (i=0; i < numObj; i++)
	{
		n = sprintf(p, "%d ", target->objMetadata[i].objID.data2);
		p = p + n;
	}

	sprintf(p, "\n");

	fprintf(fp, "%s", string);

	return;
}

/* Create the meta data */

int VIDEO_dmvalTskCreateMetaData()
{
	int numObj;
	int dmvaType  = 0;
	char zone[4];
	float Vx,Vy;

	int i;
	char *pMetaBufPtr;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;
	DMVALout *pOutput     = dmvalHdl->output;

	pMetaBufPtr = gDMVA_metaDataBuf;

	// zero init the meta data buffer
	DMVA_memset(gDMVA_metaDataBuf,0x00,VIDEO_BUF_METADATA_SIZE);

	// Write the METS fourcc
	*pMetaBufPtr++ = 'M';
	*pMetaBufPtr++ = 'E';
	*pMetaBufPtr++ = 'T';
	*pMetaBufPtr++ = 'S';

	// Write the OBJT fourcc
	*pMetaBufPtr++ = 'O';
	*pMetaBufPtr++ = 'B';
	*pMetaBufPtr++ = 'J';
	*pMetaBufPtr++ = 'T';

	// Write the no of objects
	if(pOutput->numObjects > gAVSERVER_config.dmvaAppParams.maxNoObj)
	{
		numObj = gAVSERVER_config.dmvaAppParams.maxNoObj;
	}
	else
	{
		numObj = pOutput->numObjects;
	}

	if(((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x10) != 0))
	{
		// insert no of objects
		*pMetaBufPtr++ = numObj;

		DMVA_memset(zone,0x00,4);

		// insert the OBJT meta data
		for(i = 0;i < numObj;i ++)
		{
			//if(pOutput->objMetadata[i].roiID != 0)
			{
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objBox.xmin * DEF_META_W_SCALE);    // X
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objBox.ymin * DEF_META_H_SCALE);    // Y
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objBox.width * DEF_META_W_SCALE);   // width
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objBox.height * DEF_META_H_SCALE);  // height

				switch(pOutput->objMetadata[i].roiID)
				{
					// roiID 1,2,3 and 4 are used for the IMD zones
				case 1:
					zone[0] = 1;
					break;
				case 2:
					zone[1] = 1;
					break;
				case 3:
					zone[2] = 1;
					break;
				case 4:
					zone[3] = 1;
					break;
				}//switch
			}//if
		}//for

	}
	else
	{
		// insert no of objects
		*pMetaBufPtr++ = 0;

		DMVA_memset(zone,0x00,4);

		// insert the OBJT meta data
		for(i = 0;i < numObj;i ++)
		{
			//if(pOutput->objMetadata[i].roiID != 0)
			{
				switch(pOutput->objMetadata[i].roiID)
				{
					// roiID 1,2,3 and 4 are used for the IMD zones
				case 1:
					zone[0] = 1;
					break;
				case 2:
					zone[1] = 1;
					break;
				case 3:
					zone[2] = 1;
					break;
				case 4:
					zone[3] = 1;
					break;
				}//switch
			}//if
		}//for
	}

	// Write the DMVA event type for zone highlighting
	if(pOutput->modeResult & DMVAL_DETECTOR_TRIPZONE)
	{
		// TRIP
		*pMetaBufPtr++ = 'T';
		*pMetaBufPtr++ = 'R';
		*pMetaBufPtr++ = 'I';
		*pMetaBufPtr++ = 'P';

		*pMetaBufPtr++ = 1;
	}

	if(pOutput->modeResult & DMVAL_DETECTOR_IMD)
	{
		// IMD
		*pMetaBufPtr++ = 'I';
		*pMetaBufPtr++ = 'M';
		*pMetaBufPtr++ = 'D';
		*pMetaBufPtr++ = 'T';

		// Write the zone highhlight status
		*pMetaBufPtr++ = zone[0];
		*pMetaBufPtr++ = zone[1];
		*pMetaBufPtr++ = zone[2];
		*pMetaBufPtr++ = zone[3];
	}

	if(pOutput->modeResult & DMVAL_DETECTOR_COUNTER)
	{
		// IMD
		*pMetaBufPtr++ = 'O';
		*pMetaBufPtr++ = 'B';
		*pMetaBufPtr++ = 'C';
		*pMetaBufPtr++ = 'T';

		*pMetaBufPtr++ = 1;
	}

	if(pOutput->modeResult & DMVAL_DETECTOR_TAMPER)
	{
		// IMD
		*pMetaBufPtr++ = 'C';
		*pMetaBufPtr++ = 'M';
		*pMetaBufPtr++ = 'T';
		*pMetaBufPtr++ = 'D';

		*pMetaBufPtr++ = 1;
	}

	// insert centroid and velocity
	// Write the CDVT fourcc
	*pMetaBufPtr++ = 'C';
	*pMetaBufPtr++ = 'D';
	*pMetaBufPtr++ = 'V';
	*pMetaBufPtr++ = 'T';

	if(((pOutput->modeResult & DMVAL_DETECTOR_SMETA) || (pOutput->modeResult & DMVAL_DETECTOR_IMD)) &&
		(((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x4) != 0) || ((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x1) != 0)))
	{
		// insert no of objects
		*pMetaBufPtr++ = numObj;

		for(i = 0;i < numObj;i ++)
		{
			// insert centroid
			if(((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x4) != 0) ||
			   ((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x1) != 0))
			{
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objCentroid.x * DEF_META_W_SCALE);    // X
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objCentroid.y * DEF_META_H_SCALE);    // Y
			}
			else
			{
				*pMetaBufPtr++ = 0;    // X
				*pMetaBufPtr++ = 0;    // Y
			}

			// insert velocity
			if(((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x1) != 0) &&
				(gAVSERVER_config.dmvaAppConfig.dmvaexptsmetatrackerenable == 1))
			{
				Vx = pOutput->objMetadata[i].objCentroid.x + (5*pOutput->objMetadata[i].objVelocity.x);
				Vy = pOutput->objMetadata[i].objCentroid.y + (5*pOutput->objMetadata[i].objVelocity.y);

				*pMetaBufPtr++ = (char)((float)Vx * DEF_META_W_SCALE);
				*pMetaBufPtr++ = (char)((float)Vy * DEF_META_H_SCALE);
			}
			else
			{
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objCentroid.x * DEF_META_W_SCALE);   // X
				*pMetaBufPtr++ = (char)((float)pOutput->objMetadata[i].objCentroid.y * DEF_META_H_SCALE);   // Y
			}
		}
	}
	else
	{
		// insert no of objects
		*pMetaBufPtr++ = 0;
	}

	// insert objectr id
	// Write the OBID fourcc
	*pMetaBufPtr++ = 'O';
	*pMetaBufPtr++ = 'B';
	*pMetaBufPtr++ = 'I';
	*pMetaBufPtr++ = 'D';

	if(((pOutput->modeResult & DMVAL_DETECTOR_SMETA) || (pOutput->modeResult & DMVAL_DETECTOR_IMD)) &&
		(gAVSERVER_config.dmvaAppConfig.dmvaexptsmetatrackerenable == 1) &&
		((gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData & 0x2) != 0))
	{
		// insert no of objects
		*pMetaBufPtr++ = numObj;

		for(i = 0;i < numObj;i ++)
		{
			*pMetaBufPtr++ = (char)(pOutput->objMetadata[i].objID.data2 >> 24);
			*pMetaBufPtr++ = (char)(pOutput->objMetadata[i].objID.data2 >> 16);
			*pMetaBufPtr++ = (char)(pOutput->objMetadata[i].objID.data2 >> 8);
			*pMetaBufPtr++ = (char)(pOutput->objMetadata[i].objID.data2);
		}
	}
	else
	{
		// insert no of objects
		*pMetaBufPtr++ = 0;
	}

	// Write METE fourCC at the end
	*pMetaBufPtr++ = 'M';
	*pMetaBufPtr++ = 'E';
	*pMetaBufPtr++ = 'T';
	*pMetaBufPtr++ = 'E';

	return (pMetaBufPtr - gDMVA_metaDataBuf);
}

/* Parse the meta data */

int VIDEO_dmvalTskParseMetaData()
{
	int i;
	int numObj;
	int zone1,zone2,zone3,zone4,onOffFlag;
	int objX,objY,objW,objH;

	char *pMetaBufPtr,fourCC[5];
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	pMetaBufPtr = gDMVA_metaDataBuf;

nextByte:
	// Search for METS fourCC
	if((*(pMetaBufPtr)     == 'M') &&
		(*(pMetaBufPtr + 1) == 'E') &&
		(*(pMetaBufPtr + 2) == 'T') &&
		(*(pMetaBufPtr + 3) == 'S'))
	{
		// META fourcc is found
		pMetaBufPtr += 4;

nextFourCC:
		// Find next fourcc
		memcpy(fourCC,pMetaBufPtr,4);
		fourCC[4] = '\0';

		if(strcmp(fourCC,"OBJT") == 0)
		{
			pMetaBufPtr += 4;

			numObj = *pMetaBufPtr++;

			for(i = 0;i < numObj;i ++)
			{
				objX = *pMetaBufPtr++;
				objY = *pMetaBufPtr++;
				objW = *pMetaBufPtr++;
				objH = *pMetaBufPtr++;

				OSA_printf(" DMVA META:Obj %d - x = %d,y = %d,W = %d,H = %d\n",i + 1,objX,objY,objW,objH);
			}

			goto nextFourCC;
		}
		else if(strcmp(fourCC,"IMDT") == 0)
		{
			pMetaBufPtr += 4;

			zone1 = *pMetaBufPtr++;
			zone2 = *pMetaBufPtr++;
			zone3 = *pMetaBufPtr++;
			zone4 = *pMetaBufPtr++;

			OSA_printf(" DMVA META:zone1 = %d,zone2 = %d,zone3 = %d,zone4 = %d\n",zone1,zone2,zone3,zone4);

			goto nextFourCC;
		}
		else if((strcmp(fourCC,"TRIP") == 0) ||
			(strcmp(fourCC,"OBCT") == 0) ||
			(strcmp(fourCC,"CMTD") == 0))
		{
			OSA_printf(" DMVA META:%s fourCC is found\n",fourCC);
			pMetaBufPtr += 4;

			onOffFlag = *pMetaBufPtr++;

			OSA_printf(" DMVA META:onOffFlag = %d\n",onOffFlag);

			goto nextFourCC;
		}
		else if(strcmp(fourCC,"METE") == 0)
		{
			goto exit;
		}
		else
		{
			OSA_printf(" DMVA META:undef fourCC is found\n");
			pMetaBufPtr += 4;
			goto nextFourCC;
		}
	}
	else
	{
		pMetaBufPtr ++;

		if((pMetaBufPtr - gDMVA_metaDataBuf) >= VIDEO_BUF_METADATA_SIZE)
		{
			OSA_printf(" DMVA META:Meta Data not found\n");
			goto exit;
		}

		goto nextByte;
	}

exit:
	return OSA_SOK;
}

/* Print the cur time */
void VIDEO_dmvalPrintCurTime()
{
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);

	OSA_printf("\nCUR TIME:\n");
	OSA_printf("cur year      = %d\n",p->tm_year);
	OSA_printf("cur year day  = %d\n",p->tm_yday);
	OSA_printf("cur week day  = %d\n",p->tm_wday);
	OSA_printf("cur hour      = %d\n",p->tm_hour);
	OSA_printf("cur min       = %d\n",p->tm_min);
	OSA_printf("cur sec       = %d\n",p->tm_sec);
}

/* Print DMVAL error messages */
void VIDEO_dmvalPrintErrMsg(DMVALstatus status)
{
	switch(status)
	{
	case DMVAL_WARNING_INITIALIZING:
		printf("\n> DMVA APP: DMVA Library is busy initializing....\n", status);
		break;
	case DMVAL_NO_ERROR:
		printf("\n> DMVA APP: No error reported.\n", status);
		break;
	case DMVAL_ERR_INSTANCE_CREATE_FAILED:
		printf("\n> DMVA APP: Error Creating DMVAL instance!\n", status);
		break;
	case DMVAL_ERR_INPUT_INVALID:
	case DMVAL_ERR_MEMORY_POINTER_NULL:
		printf("\n> DMVA APP: Error = Input invalid!\n", status);
		break;
	case DMVAL_ERR_INPUT_NEGATIVE:
		printf("\n> DMVA APP: DMVA Library Error = Negative Input parameter!\n", status);
		break;
	case DMVAL_ERR_INPUT_EXCEEDED_RANGE:
		printf("\n> DMVA APP: DMVA Library Error = Input parameter exceeded range!\n", status);
		break;
	case DMVAL_ERR_LIBRARY_INTERNAL_FAILURE:
		printf("\n> DMVA APP: DMVA Library experienced internal failure!\n", status);
		break;
	case DMVAL_WARNING_LOW_MEMORY:
	case DMVAL_ERR_MEMORY_EXCEEDED_BOUNDARY:
	case DMVAL_ERR_MEMORY_INSUFFICIENT:
		printf("\n> DMVA APP: DMVA Library Error = Out of memory!", status);
		break;
	}
}

/* DMVA task run */
int gIMDevtCount = 0;

int VIDEO_dmvalTskRun()
{
	int status=OSA_SOK, inBufId;
	int i,j,count;
	OSA_BufInfo *pBufInfo;
	SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;

	// DMVAL instance handle
	DMVALhandle *dmvalHdl;
	// DMVAL image
	DMVALimage inImage;
	DMVALout  * output;
	DMVALstatus libStatus;
	HPTimeStamp      timeStamp;
	char           * timeString;
	char             swosdString[20];
	static int       counterForTextMessage = 0;
	
	struct timeval start,end;
	int processInterval;

	dmvalHdl = gParams.dmvalHdl;
	output = dmvalHdl->output;

	status = OSA_bufGetFull(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, &inBufId, OSA_TIMEOUT_FOREVER);
	if(status!=OSA_SOK)
	{
		OSA_ERROR("OSA_bufGetFull()\n");
		return status;
	}

	pBufInfo = OSA_bufGetBufInfo(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, inBufId);
	OSA_assert(pBufInfo!=NULL);

	if(gAVSERVER_config.dmvaAppConfig.dmvaEnable && gVIDEO_ctrl.dmvaAppStream.dmvaWidth<=gVIDEO_ctrl.dmvaAppStream.dmvaOffsetH)
	{
		OSA_assert(pBufInfo->physAddr!=NULL);

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
#ifdef AVSERVER_DEBUG_RUNNING
		OSA_printf(" DMVA: InBuf %d \n", inBufId);
#endif
#endif
		// get time-stamp for current frame
		gettimeofday(&timeStamp.clock, NULL);

		inImage.pixelDepth  = DMVAL_PIXEL_U08;
		inImage.width       = gVIDEO_ctrl.dmvaAppStream.dmvaWidth;
		inImage.height      = gVIDEO_ctrl.dmvaAppStream.dmvaHeight;
		inImage.imageStride = gVIDEO_ctrl.dmvaAppStream.dmvaOffsetH;
		inImage.imageData   = pBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
		inImage.imageSize   = inImage.imageStride * inImage.height;
		inImage.type        = DMVAL_IMG_YUV420_PLANARINTERLEAVED;
		inImage.horzOffset  = 0;
		inImage.vertOffset  = 0;
		inImage.timeStamp   = timeStamp;

		OSA_prfBegin(&gAVSERVER_ctrl.dmvaPrf);

		//fprintf(stderr,"@@@@@@@@@@@@@@@@ DMVAL_process Enter @@@@@@@@@@@@@@@@@@@\n");
		gettimeofday (&start, NULL);
		
		OSA_mutexLock(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);
		libStatus = DMVAL_process(dmvalHdl, &inImage);
		if (libStatus != DMVAL_NO_ERROR)
		{
			VIDEO_dmvalPrintErrMsg(libStatus);

			if( (libStatus == DMVAL_ERR_MEMORY_INSUFFICIENT) ||
				(libStatus == DMVAL_ERR_LIBRARY_INTERNAL_FAILURE))
			{
				printf("\n\n> DMVA APP: Attempting to reset DMVA library...\n");
				fflush(stdout);

				// Reset all parameters
				DMVAL_initModule(dmvalHdl);

				// Re-assign parameters
				VIDEO_dmvalTskSetParams();
			}
		}
				
		gettimeofday (&end, NULL);
		processInterval = (end.tv_sec * 1000 + end.tv_usec/1000) - (start.tv_sec * 1000 + start.tv_usec/1000);
		//fprintf(stderr,"@@@@@@@@@@@@@@@@ DMVAL_process Exit - %d msec @@@@@@@@@@@@@@@@@@@\n",processInterval);

		OSA_mutexUnlock(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);

		OSA_prfEnd(&gAVSERVER_ctrl.dmvaPrf, 1);

		if(gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable == 2)
		{
			// increment the frame no for next event SNAP SHOT recording
			gDmvaMinEvtRecInt ++;

			if(gDmvaMinEvtRecInt >= DMVA_EVT_MIN_REC_INTERVAL)
			{
				VIDEO_dmvaSetEvtRecStatus(0);
			}
		}

		// turn off all text overlap
		if (counterForTextMessage == 0)
			AVSERVER_swosdDmvaText("", 0);
		else
			counterForTextMessage = counterForTextMessage - 1;


		// Turn on specific text based on detector output
		if(output->modeResult & DMVAL_DETECTOR_IMD)
		{
			AVSERVER_swosdDmvaEnableText(1);
			AVSERVER_swosdDmvaText("MOTION DETECTED", 15);
			counterForTextMessage = 3;
		}
		else
			gIMDevtCount = 0;


		if(output->modeResult & DMVAL_DETECTOR_TAMPER)
		{
			AVSERVER_swosdDmvaEnableText(1);

			if(output->modeResult & DMVAL_TAMPER_SCENECHANGE)
				AVSERVER_swosdDmvaText("TAMPER (SCENE MODIFIED)", 23);
			else if(output->modeResult & DMVAL_TAMPER_MOVEMENT)
				AVSERVER_swosdDmvaText("TAMPER (CAMERA MOVE)", 20);
			else if(output->modeResult & DMVAL_TAMPER_DARK)
				AVSERVER_swosdDmvaText("TAMPER (SCENE DARKEN)", 21);
			else if(output->modeResult & DMVAL_TAMPER_BRIGHT)
				AVSERVER_swosdDmvaText("TAMPER (SCENE BRIGHTEN)", 23);
			else if(output->modeResult & DMVAL_TAMPER_BLUR)
				AVSERVER_swosdDmvaText("TAMPER (CAMERA DEFOCUS)", 23);

			counterForTextMessage = 8;

		}

		if(output->modeResult & DMVAL_DETECTOR_TRIPZONE)
		{
			AVSERVER_swosdDmvaEnableText(1);
			AVSERVER_swosdDmvaText("TRIP ZONE", 9);
			counterForTextMessage = 10;
		}

		if(output->modeResult & DMVAL_DETECTOR_COUNTER)
		{
			AVSERVER_swosdDmvaEnableText(1);
			gObjectCount = gObjectCount + output->numObjects;

			sprintf(swosdString, "OBJ COUNTED %d",gObjectCount);
			AVSERVER_swosdDmvaText(swosdString, 16);
			counterForTextMessage = -1;
		}


#ifdef ENABLE_EVT_RECORD //To Enable/Disable EVT RECORDING

		if((gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable) &&
		   (pSysInfo->sdcard_config.schAviRun == 0))
		{
			if(VIDEO_dmvaGetEvtRecStatus() == 0)
			{
				if( (output->modeResult & DMVAL_DETECTOR_IMD)      ||
					(output->modeResult & DMVAL_DETECTOR_TAMPER)   ||
					(output->modeResult & DMVAL_DETECTOR_TRIPZONE) ||
					(output->modeResult & DMVAL_DETECTOR_COUNTER))
				{
					gDmvaEvt.modeResult = output->modeResult;

					// Send msg to DMVA Event task
					OSA_tskSendMsg(&gVIDEO_ctrl.dmvaEvtTsk,     // pPrcTo
						&gVIDEO_ctrl.dmvaTsk,        // pPrcFrom
						AVSERVER_CMD_DMVA_EVENT,     // cmd
						&gDmvaEvt,                   // pPrm
						0);                          // flags
				}
			}
		}

#endif

		// Create the meta data
		VIDEO_dmvalTskCreateMetaData();
		//VIDEO_dmvalTskParseMetaData();
	}

	OSA_bufPutEmpty(&gVIDEO_ctrl.dmvaAppStream.bufDmvaIn, inBufId);

	return status;
}

/* This fn prints the new TZ params received from the GUI */

int VIDEO_dmvaTskPrintNewTZParams()
{
	int i,numSides;

	fprintf(stderr,"\n Received New TZ Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity      = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection        = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust    = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZEnable           = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZEnable);

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i]);
	}

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i]);
	}

	return OSA_SOK;
}

/* This fn sets the new TZ params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetTZParams()
{
	int resetDmva = 0;
	int setROI = 0,i;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	if(gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc & DMVAL_DETECTMODE_TRIP)
	{
		// Set the TZ params only if we have received different params
		if(gParams.sensitiveness != gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity)
		{
			gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppTZSensitivity;
			DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_SENSITIVITY,&(gParams.sensitiveness),1);
		}

		if(gParams.direction != gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection)
		{
			gParams.direction = gAVSERVER_config.dmvaAppParams.dmvaAppTZDirection;
			DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_TZDIRECTION,&(gParams.direction),1);
		}

		//Need to activate this once Algo parameters support this
		if(gParams.dmvaAppTZPresentAdjust != gAVSERVER_config.dmvaAppParams.dmvaAppTZPresentAdjust)
		{
			resetDmva = 1;
		}

		// Not a dynamic parameter
		if((gParams.minPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMinHeight * DEF_INROWS_SCALE)) )
		{
			resetDmva = 1;
		}

		// Not a dynamic parameter
		if((gParams.maxPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZPersonMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		if((gParams.minVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMinHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// Not a dynamic parameter
		if((gParams.maxVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZVehicleMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// Set the zone 1 params
		gParams.numPolygons = 2;

		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_TZ_1A;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[0].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[0].numPoints;i ++)
			{
				if((gParams.polygon[0].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[0].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone1_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}

		// Set the zone 2 params
		gParams.polygon[1].polygonID = 2;
		gParams.polygon[1].valid     = 1;
		gParams.polygon[1].type      = DMVAL_TZ_1B;
		//gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[1].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[1].numPoints;i ++)
			{
				if((gParams.polygon[1].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[1].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppTZZone2_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}

		if(resetDmva)
		{
			// Read new parameters from NAND
			VIDEO_dmvalTskGetParamFrmAVServer();

			//CHECK: Key to set the new VA
			gParams.dmvalHdl->detectMode = gParams.detectMode;

			// Reset all parameters
			DMVAL_initModule(dmvalHdl);

			// Re-assign parameters
			VIDEO_dmvalTskSetParams();
		}
	}

	return OSA_SOK;
}

/* Print the new CTD params received from the GUI */

int VIDEO_dmvaTskPrintNewCTDParams()
{
	fprintf(stderr,"\n Received New CTD Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv1        = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv1);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv2        = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDEnv2);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime);

	return OSA_SOK;
}

/* This fn sets the new CTD params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetCTDParams()
{
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	if(gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc & DMVAL_DETECTMODE_TAMPER)
	{
		if(gParams.sensitivity != gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity)
		{
			gParams.sensitivity = gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDSensitivity;
			DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_TAMPERSENSITIVITY, (short*) &(gParams.sensitivity), 1);
		}

		if(gParams.resetCounter != (gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime * gDMVA_fpsControl.curDmvaFps))
		{
			gParams.resetCounter = (gAVSERVER_config.dmvaAppParams.dmvaAppCfgTDResetTime * gDMVA_fpsControl.curDmvaFps);
			DMVAL_setParameter(dmvalHdl, DMVAL_PARAM_TAMPERCOUNTER, (short*) &(gParams.resetCounter), 1);
		}
	}

	return OSA_SOK;
}

/* This fn prints the new OC params received from the GUI */

int VIDEO_dmvaTskPrintNewOCParams()
{
	fprintf(stderr,"\n Received New OC Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection    = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCEnable       = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCEnable);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight    = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppOCTopBottom    = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppOCTopBottom);

	return OSA_SOK;
}

/* This fn sets the new OC params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetOCParams()
{
	int resetDmva = 0;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	if(gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc & DMVAL_DETECTMODE_COUNT)
	{
		if(gParams.orientation != gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight)
		{
			gParams.orientation = gAVSERVER_config.dmvaAppParams.dmvaAppOCLeftRight;
			resetDmva = 1;
		}

		if(gParams.sensitiveness != gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity)
		{
			gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppOCSensitivity;
			if(gParams.sensitiveness > 5)
			{
				gParams.sensitiveness   = 3;//Moderate
			}

			DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_SENSITIVITY,&(gParams.sensitiveness),1);
		}

		//if(gParams.direction != gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection)
		//{
		gParams.direction     = gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection;
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 2) {
			gParams.direction = 1;
		}
		if(gAVSERVER_config.dmvaAppParams.dmvaAppOCDirection == 1) {
			gParams.direction = 2;
		}
		DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_OCDIRECTION,&(gParams.direction),1);
		//}

		// Not dynamic
		if ((gParams.minPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppOCObjectHeight* DEF_INROWS_SCALE)) )
		{
			resetDmva = 1;
		}

		if (resetDmva)
		{
			// Read new parameters from NAND
			VIDEO_dmvalTskGetParamFrmAVServer();

			//CHECK: Key to set the new VA
			gParams.dmvalHdl->detectMode = gParams.detectMode;

			// Reset all parameters
			DMVAL_initModule(dmvalHdl);

			// Re-assign parameters
			VIDEO_dmvalTskSetParams();
		}
	}

	return OSA_SOK;
}

/* This fn prints the new MAIN page params received from the GUI */

int VIDEO_dmvaTskPrintNewMAINParams()
{
	fprintf(stderr,"\n Received New MAIN Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc                  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc);

	return OSA_SOK;
}

/* This fn sets the new MAIN page params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetMAINParams()
{
	int resetDmva = 0;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	resetDmva = 1;

	if (resetDmva)
	{
		// Read new parameters from NAND
		VIDEO_dmvalTskGetParamFrmAVServer();

		//CHECK: Key to set the new VA
		gParams.dmvalHdl->detectMode = gParams.detectMode;

		// Reset all parameters
		DMVAL_initModule(dmvalHdl);

		// Re-assign parameters
		VIDEO_dmvalTskSetParams();
	}

	return OSA_SOK;
}

/* This fn prints the new SMETA params received from the GUI */

int VIDEO_dmvaTskPrintNewSMETAParams()
{
	int i,numSides;

	fprintf(stderr,"\n Received New SMETA Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");

#if 0

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity         = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAEnableStreamingData);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamBB            = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamBB);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamVelocity      = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamVelocity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamHistogram     = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamHistogram);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamMoments       = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAStreamMoments);

#endif

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity      = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones         = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones);

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i]);
	}

	return OSA_SOK;
}

/* This fn sets the new SMETA params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetSMETAParams()
{
	int i,resetDmva = 0,setROI = 0;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	if(gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc & DMVAL_DETECTMODE_SMETA)
	{
		// sensitivity
		if(gParams.sensitiveness != gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity)
		{
			gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppSMETASensitivity;
			DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_SENSITIVITY,(short*) &(gParams.sensitiveness),1);
		}

		// Person min size
		if((gParams.minPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMinHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// Person max size
		if((gParams.maxPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAPersonMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// vehicle min size
		if((gParams.minVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMinHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// vehicle max size
		if((gParams.maxVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAVehicleMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// Set SMETA zones
		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppSMETANumZones;

		// NOTE:currently only 1 zone is supported.
		// TBD :support more than 1 zones
		if(gParams.numPolygons > 1)
			gParams.numPolygons = 1;

		// ZONE 1
		gParams.polygon[0].polygonID = 1;
		gParams.polygon[0].valid     = 1;
		gParams.polygon[0].type      = DMVAL_META;
		//gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[0].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[0].numPoints;i ++)
			{
				if((gParams.polygon[0].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[0].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppSMETAZone1_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}
	}

	if(resetDmva)
	{
		// Read new parameters from NAND
		VIDEO_dmvalTskGetParamFrmAVServer();

		//CHECK: Key to set the new VA
		gParams.dmvalHdl->detectMode = gParams.detectMode;

		// Reset all parameters
		DMVAL_initModule(dmvalHdl);

		// Re-assign parameters
		VIDEO_dmvalTskSetParams();
	}

	return OSA_SOK;
}

/* This fn prints the new IMD params received from the GUI */

int VIDEO_dmvaTskPrintNewIMDParams()
{
	int i,numSides;

	fprintf(stderr,"\n Received New IMD Parameters\n");
	fprintf(stderr,"\n ----------------------------\n");
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity      = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust    = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth   = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth  = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones         = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones);
	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable       = %d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable);

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i]);
	}

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i]);
	}

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i]);
	}

	numSides = gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides;

	fprintf(stderr,"\n gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides = %d\n",numSides);

	for(i = 0;i < numSides;i ++)
	{
		fprintf(stderr," %3d,%3d\n",gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i],gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i]);
	}

	return OSA_SOK;
}

/* This fn sets the new IMD params received from the GUI into the DMVA algorithm */

int VIDEO_dmvaTskSetIMDParams()
{
	int resetDmva = 0;
	int polygonCount,i,setROI = 0;
	DMVALhandle *dmvalHdl = gParams.dmvalHdl;

	if(gAVSERVER_config.dmvaAppParams.dmvaAppCfgCTDImdSmetaTzOc & DMVAL_DETECTMODE_IMD)
	{

		//Need to activate this once Algo parameters support this
		if(gParams.dmvaAppIMDPresentAdjust != gAVSERVER_config.dmvaAppParams.dmvaAppIMDPresentAdjust)
		{
			resetDmva = 1;
		}

		if(gParams.sensitiveness != gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity)
		{
			gParams.sensitiveness = gAVSERVER_config.dmvaAppParams.dmvaAppIMDSensitivity;
			DMVAL_setParameter(dmvalHdl,DMVAL_PARAM_SENSITIVITY,(short*) &(gParams.sensitiveness),1);
		}

		if((gParams.minPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMinHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		if((gParams.maxPersonSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxPersonSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDPersonMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		if((gParams.minVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinWidth * DEF_INCOLS_SCALE)) ||
			(gParams.minVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMinHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		if((gParams.maxVehicleSize[0] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxWidth * DEF_INCOLS_SCALE)) ||
			(gParams.maxVehicleSize[1] != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDVehicleMaxHeight * DEF_INROWS_SCALE)))
		{
			resetDmva = 1;
		}

		// Set the IMD zones
		gParams.numPolygons = gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones;

		polygonCount = 0;

		// ZONE 1
		gParams.polygon[0].polygonID = 1;
//		gParams.polygon[0].valid     = 1;

        if(gParams.polygon[0].valid != (gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 1))
        {
		    resetDmva = 1;
		}

		gParams.polygon[0].type      = DMVAL_IMD;
//		gParams.polygon[0].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[0].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[0].numPoints;i ++)
			{
				if((gParams.polygon[0].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[0].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone1_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}

		polygonCount ++;

		if(polygonCount == gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones)
		{
			goto exit;
		}

		// ZONE 2
		gParams.polygon[1].polygonID = 2;
//		gParams.polygon[1].valid     = 1;

        if(gParams.polygon[1].valid != ((gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 2) >> 1))
        {
		    resetDmva = 1;
		}

		gParams.polygon[1].type      = DMVAL_IMD;
//		gParams.polygon[1].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[1].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[1].numPoints;i ++)
			{
				if((gParams.polygon[1].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[1].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone2_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}

		polygonCount ++;

		if(polygonCount == gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones)
		{
			goto exit;
		}

		// ZONE 3
		gParams.polygon[2].polygonID = 3;
//		gParams.polygon[2].valid     = 1;

        if(gParams.polygon[2].valid != ((gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 4) >> 2))
        {
		    resetDmva = 1;
		}

		gParams.polygon[2].type      = DMVAL_IMD;
		//gParams.polygon[2].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[2].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[2].numPoints;i ++)
			{
				if((gParams.polygon[2].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[2].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone3_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}

		polygonCount ++;

		if(polygonCount == gAVSERVER_config.dmvaAppParams.dmvaAppIMDNumZones)
		{
			goto exit;
		}

		// ZONE 4
		gParams.polygon[3].polygonID = 4;
//		gParams.polygon[3].valid     = 1;

        if(gParams.polygon[3].valid != ((gAVSERVER_config.dmvaAppParams.dmvaAppIMDZoneEnable & 8) >> 3))
        {
		    resetDmva = 1;
		}

		gParams.polygon[3].type      = DMVAL_IMD;
		//gParams.polygon[3].dir       = DMVAL_DIRECTION_UNSPECIFIED;

		if(gParams.polygon[3].numPoints == gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_ROI_numSides)
		{
			for(i = 0;i < gParams.polygon[3].numPoints;i ++)
			{
				if((gParams.polygon[3].pt[i].x != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_x_array[i] * DEF_INCOLS_SCALE)) ||
					(gParams.polygon[3].pt[i].y != (S16)ceil(gAVSERVER_config.dmvaAppParams.dmvaAppIMDZone4_y_array[i] * DEF_INROWS_SCALE)))
				{
					setROI = 1;
					break;
				}
			}
		}
		else
		{
			setROI = 1;
		}

		if(setROI == 1)
		{
			setROI = 0;
			resetDmva = 1;
		}
	}

exit:
	if(resetDmva)
	{
		// Read new parameters from NAND
		VIDEO_dmvalTskGetParamFrmAVServer();

		//CHECK: Key to set the new VA
		gParams.dmvalHdl->detectMode = gParams.detectMode;

		// Reset all parameters
		DMVAL_initModule(dmvalHdl);

		// Re-assign parameters
		VIDEO_dmvalTskSetParams();
	}

	return OSA_SOK;
}

/* Main fn of DMVA task */

int VIDEO_dmvaTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
	int status;
	Bool done=FALSE, ackMsg = FALSE;
	Uint16 cmd = OSA_msgGetCmd(pMsg);

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
	OSA_printf(" DMVA: Received CMD = 0x%04x\n", cmd);
#endif

	if(cmd!=AVSERVER_CMD_CREATE)
	{
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
		return OSA_SOK;
	}

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
	OSA_printf(" DMVA: Create...\n");
#endif

	status = VIDEO_dmvalTskCreate();
	status = VIDEO_dmvalTskSetParams();

	OSA_tskAckOrFreeMsg(pMsg, status);

	if(status!=OSA_SOK)
		return OSA_SOK;

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
	OSA_printf(" DMVA: Create...DONE\n");
#endif

	while(!done)
	{
		status = OSA_tskWaitMsg(pTsk, &pMsg);

		if(status!=OSA_SOK)
			break;

		cmd = OSA_msgGetCmd(pMsg);

		switch(cmd)
		{
		case AVSERVER_CMD_DELETE:
			done = TRUE;
			ackMsg = TRUE;
			break;

		case AVSERVER_CMD_NEW_DATA:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			VIDEO_dmvalTskRun();
			break;

		case AVSERVER_CMD_NEW_TZ_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			//VIDEO_dmvaTskPrintNewTZParams();
			VIDEO_dmvaTskSetTZParams();
			break;

		case AVSERVER_CMD_NEW_CTD_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			//VIDEO_dmvaTskPrintNewCTDParams();
			VIDEO_dmvaTskSetCTDParams();
			break;

		case AVSERVER_CMD_NEW_OC_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			//VIDEO_dmvaTskPrintNewOCParams();
			VIDEO_dmvaTskSetOCParams();
			break;

		case AVSERVER_CMD_NEW_MAIN_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			VIDEO_dmvaTskSetMAINParams();
			//VIDEO_dmvaTskPrintNewMAINParams();
			break;

		case AVSERVER_CMD_NEW_SMETA_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			//VIDEO_dmvaTskPrintNewSMETAParams();
			VIDEO_dmvaTskSetSMETAParams();
			break;

		case AVSERVER_CMD_NEW_IMD_PARAM:
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			//VIDEO_dmvaTskPrintNewIMDParams();
			VIDEO_dmvaTskSetIMDParams();
			break;

		default:
#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
			OSA_printf(" DMVA: Unknown CMD = 0x%04x\n", cmd);
#endif

			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			break;
		}
	}// while(!done)

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
	OSA_printf(" DMVA: Delete...\n");
#endif

	VIDEO_dmvalTskDelete();

	if(ackMsg)
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

#ifdef AVSERVER_DEBUG_VIDEO_DMVA_THR
	OSA_printf(" DMVA: Delete...DONE\n");
#endif

	if(gVIDEO_ctrl.dmvaAppStream.dmvaError)
	{
		OSA_printf(" DMVA: DMVA timeout happened while running !!!\n");
	}

	return OSA_SOK;
}

/* DMVA task create */

int VIDEO_dmvaCreate()
{
	int status;

	status = OSA_mutexCreate(&gVIDEO_ctrl.dmvaAppStream.statusLock);
	if(status!=OSA_SOK)
	{
		OSA_ERROR("OSA_mutexCreate()\n");
		return status;
	}

	status = OSA_mutexCreate(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);
	if(status!=OSA_SOK)
	{
		OSA_ERROR("OSA_mutexCreate()\n");
		return status;
	}

	status = OSA_tskCreate( &gVIDEO_ctrl.dmvaTsk, VIDEO_dmvaTskMain, VIDEO_DMVA_THR_PRI, VIDEO_DMVA_STACK_SIZE, 0);
	if(status!=OSA_SOK)
	{
		OSA_ERROR("OSA_tskCreate()\n");
		return status;
	}

	status = OSA_tskCreate( &gVIDEO_ctrl.dmvaEvtTsk, VIDEO_dmvaEvtTskMain, VIDEO_DMVA_EVT_THR_PRI, VIDEO_DMVA_EVT_STACK_SIZE, 0);
	if(status!=OSA_SOK)
	{
		OSA_ERROR("OSA_tskCreate()\n");
		return status;
	}

	gDmvaMinEvtRecInt = 0;

	return status;
}

/* DMVA task delete */

int VIDEO_dmvaDelete()
{
	int status;

	/*
	OSA_tskSendMsg(&gVIDEO_ctrl.dmvaEvtTsk,     // pPrcTo
	&gVIDEO_ctrl.dmvaTsk,        // pPrcFrom
	AVSERVER_CMD_DELETE,         // cmd
	NULL,                        // pPrm
	OSA_MBX_WAIT_ACK);           // flags
	*/

	status  = OSA_tskDelete( &gVIDEO_ctrl.dmvaTsk );
	status |= OSA_tskDelete( &gVIDEO_ctrl.dmvaEvtTsk );
	status |= OSA_mutexDelete(&gVIDEO_ctrl.dmvaAppStream.statusLock);
	status |= OSA_mutexDelete(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);

	return status;
}



/************************************************************************************************/

/* memset fn implemented using EDMA*/

int DMVA_memset(char *addr,char value,int size)
{
	int status;
	DRV_DmaChHndl memsetDmaHndl;
	DRV_DmaFill1D fill1D;

	if(size >= 1024)
	{
		// Open EDMA channel for memset
		status = DRV_dmaOpen(&memsetDmaHndl, DRV_DMA_MODE_NORMAL, 1);

		if(status != OSA_SOK)
		{
			OSA_printf("DMVA:memset DMA open failed\n");
		}

		fill1D.dstVirtAddr = (unsigned long)addr;
		fill1D.dstPhysAddr = (unsigned long)DRV_dmaGetPhysAddr((void *)addr);
		fill1D.size        = (size>>2) <<2;
		fill1D.fillValue   = (value << 24) | (value << 16) | (value << 8) | (value);

		DRV_dmaFill1D(&memsetDmaHndl,&fill1D,1);

		DRV_dmaClose(&memsetDmaHndl);

		if(size > fill1D.size)
		{
			memset(addr + fill1D.size,value,(size - fill1D.size));
		}
	}
	else
	{
		memset(addr,value,size);
	}

	return OSA_SOK;
}


/* END OF FILE videoDmvaAppThr.c */
