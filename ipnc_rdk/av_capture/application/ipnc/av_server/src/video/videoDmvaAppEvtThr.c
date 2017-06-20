
#include <avserver_dmva.h>

///////////////////////////////////////////////////////////////////////////////
// PRE-PROCESSOR DEFINITIONS
//#define META_DATA_FILE
//#define DMVA_EVT_MGR
#define SNAP_SHOT
//#define DBG_MSG

#ifdef DBG_MSG
    #define PRINTF(fmt,args...)    fprintf(stderr,fmt,##args)
#else
    #define PRINTF(fmt,args...)
#endif
///////////////////////////////////////////////////////////////////////////////

#ifdef DMVA_EVT_MGR
#include <sys/stat.h>
#include "App_common.h"
#include "App_eventMgr.h"
#include "App_time.h"
#include "App_prototypes.h"
#endif
///////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
int VIDEO_dmvalEvtAviExit();
int VIDEO_dmvalEvtAviCreate(char *fileName,int interval,int streamId,int audEnable,int bitRate);
void SendDMVAAlarmEventDetect(int EvtType,char *fileName);
unsigned char VIDEO_CheckSDCard();
/* Global variables */
///////////////////////////////////////////////////////////////////////////////
// DMVA Event Manager specific prototypes and variables
#ifdef DMVA_EVT_MGR
int EVT_createEventManager(EventMgr **evtMgr); // AYK - 0701
EventMgr  *gEvtMgr;
extern U08 lucidaFont6x10_576x10[5760];
extern TIME gCPUcountsPerSec;
#endif
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// GLOBALS (LOCAL & EXTERNAL DEFINITIONS)
char    gOutRootFilename[100];
int     gEventRecordingStatus = 0;  // AYK - 0215
int     gDmvaEvtSnapAddr;
void    *gAlgEncHndl;

char strFileName[128];


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

/* This fn sets the status of the event recording */

int VIDEO_dmvaSetEvtRecStatus(int evtRec)
{
    gEventRecordingStatus = evtRec;
    gDmvaMinEvtRecInt     = 0;
    return 0;
}

/* This fn gets the status of the event recording */

int VIDEO_dmvaGetEvtRecStatus()
{
    return (gEventRecordingStatus);
}

/* DMVA Event task create */

int VIDEO_dmvalEvtTskCreate(int checkCard)
{
    int status;
    ALG_VidEncCreate createPrm;
    SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;

#ifdef SNAP_SHOT

    // allocate memory for output buffer
    gDmvaEvtSnapAddr = (int) OSA_cmemAlloc(100 * 1024,32);

    // Create an instance of JPEG codec
    createPrm.codec      = ALG_VID_CODEC_MJPEG;
    createPrm.dataFormat = DRV_DATA_FORMAT_YUV420;
    createPrm.width      = gAVSERVER_config.encodeConfig[0].cropWidth;
    createPrm.height     = gAVSERVER_config.encodeConfig[0].cropHeight;
    createPrm.offsetH    = gVIDEO_ctrl.captureStream[gAVSERVER_config.encodeConfig[0].captureStreamId].captureOutOffsetH;
    createPrm.offsetV    = gVIDEO_ctrl.captureStream[gAVSERVER_config.encodeConfig[0].captureStreamId].captureOutOffsetV;
    createPrm.qValue     = 75;

    gAlgEncHndl = ALG_vidEncCreate(&createPrm);



    if(gAlgEncHndl == NULL)
    {
        OSA_ERROR("ALG_vidEncCreate");
        return OSA_EFAIL;
    }

    PRINTF("\n> DMVA EVT: Created JPEG encoder instance for event recording...\n");
#endif

    pSysInfo->sdcard_config.schAviRun = 0;
    pSysInfo->mmcqdPid                = 0;

    return OSA_SOK;
}

/* DMVA Event task delete */

int VIDEO_dmvalEvtTskDelete()
{
#ifdef DMVA_EVT_MGR
    EventMgr  *evtMgr = gEvtMgr;
#endif

    VIDEO_dmvalEvtAviExit();

    // AYK - 0428
    // Free the memory
    OSA_cmemFree((void*)gDmvaEvtSnapAddr);

#ifdef SNAP_SHOT
    // Delete the JPEG encoder
    ALG_vidEncDelete(gAlgEncHndl);
#endif

#ifdef DMVA_EVT_MGR
  if(evtMgr == NULL)
  {
      PRINTF("\n> DMVA EVT: No memory allocated...\n");
  }
  else
  {
      PRINTF("\n> DMVA EVT: Liberating memory allocated to evtMgr ...\n");
      OSA_cmemFree((void*)(evtMgr->roi));
      OSA_cmemFree((void*)(evtMgr->snapshotImgBuffer));
      OSA_cmemFree((void*)evtMgr);
  }
#endif

    return OSA_SOK;
}

/* Get AVI file name */

int VIDEO_dmvalEvtGetFileName(char *pFileName,char *pEvtStr,int checkCard)
{
    time_t tCurrentTime;
    struct tm *tmnow;
    time(&tCurrentTime);
    tmnow = localtime(&tCurrentTime);

    if(checkCard == 0)
    {
        return sprintf(pFileName, "/%s_%04d%02d%02d%02d%02d%02d",pEvtStr,
                       tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, tmnow->tm_hour,
                       tmnow->tm_min, tmnow->tm_sec);
    }
    else
    {
        return sprintf(pFileName, "%s/%s_%04d%02d%02d%02d%02d%02d",SD_PATH,pEvtStr,
                       tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, tmnow->tm_hour,
                       tmnow->tm_min, tmnow->tm_sec);
    }
}

/* DMVA Event Record function */
const int audio_bitrate_aac[2][3] = {{24000,36000,48000},{32000,48000,64000}};
const int audio_bitrate_g711[2][1] = {{64000},{128000}};

int VIDEO_dmvalEvtRecord(char *pFileName)
{
    int ret = OSA_SOK;
    int frameRate,audEnable,streamId,recInterval;
    struct timeval start,end;
    char cmd[128];
    int bitRate;
    SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;

    if(pSysInfo->sdcard_config.schAviRun == 1)
    {
	    return -1;
	}

    sprintf(strFileName,"%s.avi",pFileName);

    // Record the event for DMVA_EVT_REC_INTERVAL
    frameRate = 30;

    // Record the first stream
    switch(gAVSERVER_config.encodeConfig[0].codecType)
    {
	    case ALG_VID_CODEC_MPEG4:
	        streamId = 0;
	        break;
	    case ALG_VID_CODEC_MJPEG:
	        streamId = 2;
	        break;
	    case ALG_VID_CODEC_H264:
	        streamId = 3;
	        break;
	}

    // Audio Codec and Bitrate
	if((gAVSERVER_config.dmvaAppParams.dmvaAppEventRecordingVAME & 0x4) == 0x00)
	{
	    audEnable = 0;
		bitRate   = 0;
	}
	else if(pSysInfo->audio_config.codectype == 0)
	      {
			  // G711 Audio
		      audEnable = 1;
		      bitRate   = audio_bitrate_g711[pSysInfo->audio_config.samplerate][pSysInfo->audio_config.bitrate];
	      }
	      else
	      {
			  // AAC audio
		      audEnable = 2;
		      bitRate   = audio_bitrate_aac[pSysInfo->audio_config.samplerate][pSysInfo->audio_config.bitrate];
	      }

    PRINTF(" DMVA EVT:Recording event(%s) ...\n",strFileName);

    gettimeofday (&start, NULL);

    pSysInfo->sdcard_config.schAviRun = 1;
    ret = VIDEO_dmvalEvtAviCreate(strFileName,DMVA_EVT_REC_INTERVAL,streamId,audEnable,bitRate);
    pSysInfo->sdcard_config.schAviRun = 0;

    gettimeofday (&end, NULL);
    recInterval = (end.tv_sec * 1000 + end.tv_usec/1000) - (start.tv_sec * 1000 + start.tv_usec/1000);

	if(ret != 0)
	{
	    // record is not successful so remove the file
	    PRINTF(" DMVA EVT:Record is unsuccessful so deleting the file\n");
		sprintf(cmd,"rm -f /mnt/mmc/ipnc/%s\n",strFileName);
		system(cmd);
	}

    PRINTF("\n> DMVA EVT: Recording event VIDEO...");
    PRINTF("\n>           FILE   = %s", strFileName);
    PRINTF("\n>           FORMAT = %d    0 - MPEG4: 1 - MPEG4CIF: 2 - MJPEG: 3 - H264: 4 - H264CIF", streamId);

    if(audEnable)
    {
        PRINTF("\n>           AUDIO  = ENABLED");
    }
    else
    {
        PRINTF("\n>           AUDIO  = DISABLED");
    }

    if(ret == -1)
    {
        PRINTF("\n>           LENGTH = 0 msecs");
        PRINTF("\n>           STATUS = ERROR!!!!\n\n");
    }
    else
    {
        PRINTF("\n>           LENGTH = %d msecs", recInterval);
        PRINTF("\n>           STATUS = Succeeded.\n\n");
    }

	return ret;
}

/* DMVA Event snap shot */

int VIDEO_dmvaEvtSnapShot(char *pFileName,int inAddr)
{
    int status,jpgInterval;
    ALG_VidEncRunPrm encPrm;
    ALG_VidEncRunStatus encStatus;
    FILE *fp;
    struct timeval start,end;
    SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;

    sprintf(strFileName,"%s.jpg",pFileName);

  #ifdef SNAP_SHOT
    pSysInfo->sdcard_config.schAviRun = 1;

    // JPEG encode the frame
    encPrm.inStartX       = 0;
    encPrm.inStartY       = 0;
    encPrm.inAddr         = (U08 *)inAddr;
    encPrm.outAddr        = (U08 *)gDmvaEvtSnapAddr;
    encPrm.outDataMaxSize = 0;

    gettimeofday (&start, NULL);

    OSA_mutexLock(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);

    status = ALG_vidEncRun(gAlgEncHndl, &encPrm, &encStatus);

    OSA_mutexUnlock(&gVIDEO_ctrl.dmvaAppStream.snapShotLock);

    if(status != OSA_SOK)
    {
        OSA_ERROR("ALG_vidEncRun()\n");
        encStatus.bytesGenerated = 0;
    }
    else
    {
        // Write the JPEG file to the SD card
        if(VIDEO_CheckSDCard())
        {
            fp = fopen(strFileName,"wb");

            if(fp == NULL)
            {
                OSA_ERROR("file open\n");
            }
            else
            {
                PRINTF("\n> DMVA EVT: writing %s file\n", strFileName);

                if(fwrite((U08 *)gDmvaEvtSnapAddr,1,encStatus.bytesGenerated,fp) != encStatus.bytesGenerated)
                {
                    OSA_ERROR("file write\n");
                }

                fclose(fp);
            }
        }
    }

    gettimeofday (&end, NULL);

    jpgInterval = (end.tv_sec * 1000 + end.tv_usec/1000) - (start.tv_sec * 1000 + start.tv_usec/1000);

    PRINTF("\n> DMVA EVT: Snap shot(%d bytes) create time = %d msec\n",encStatus.bytesGenerated,jpgInterval);

    pSysInfo->sdcard_config.schAviRun = 0;
#endif

    return OSA_SOK;
}
















#ifdef DMVA_EVT_MGR
int
EVT_createEventManager(EventMgr **pEvtMgr)
{
    S16 e, r, s;
    U08 v;
    S32 size;
    U08 *buffer;
    S32 total = 0;
    S32 numZones = gVIDEO_ctrl.dmvaAppStream.zones.numZones; // AYK - 0701
    EventMgr  *evtMgr = *pEvtMgr;                            // AYK - 0701

    // AYK - 0701
    // gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmWidth and gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmHeight
    // are not initialized.

    //  S32 imgSize = gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmWidth *
    //          gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmHeight;

    S32 imgSize = gVIDEO_ctrl.dmvaAppStream.dmvaWidth *
                  gVIDEO_ctrl.dmvaAppStream.dmvaHeight;

    // YUV 420 image size is width * height * 1.5

    imgSize += (imgSize >> 1);

    ///////////////////////////////////////////////////////////////////////////
    // Allocate memory for event manager & related resources, if necessary
    ///////////////////////////////////////////////////////////////////////////
    if(evtMgr == NULL)
    {
        size   = sizeof(EventMgr);
        evtMgr = (EventMgr*)OSA_cmemAllocCached(size, 32);

        if(evtMgr == NULL)
        {
            OSA_ERROR("\n> DMVA EVT: Error allocating bytes for DMVA Event Manager!!\n");
            evtMgr = NULL;

            return OSA_SOK;
        }
        else
        {
            PRINTF("\n> DMVA EVT: Allocated %d bytes for DMVA Event Manager @ 0x%x", size, (unsigned int)evtMgr);
            total += size;
        }

        numZones    = gVIDEO_ctrl.dmvaAppStream.zones.numZones;
        size        = numZones * sizeof(ROI);
        evtMgr->roi = (ROI *)OSA_cmemAllocCached(size, 32);

        if(evtMgr->roi == NULL)
        {
            OSA_ERROR("\n> DMVA EVT: DMVA Event Manager error allocating bytes zones!!\n");
            OSA_cmemFree((void*)evtMgr);
            gEvtMgr = evtMgr = NULL;

            return OSA_SOK;
        }
        else
        {
            PRINTF("\n> DMVA EVT: DMVA Event Manager allocated %d bytes for %d zones @ 0x%x",
                        size,
                        gVIDEO_ctrl.dmvaAppStream.zones.numZones,
                        (unsigned int)evtMgr->roi);
                        total += size;
        }

        size                      = imgSize * EVTMGR_MAX_EVENTS_IN_MEM * EVTMGR_MAX_SNAPSHOTS_PER_EVENT;
        evtMgr->snapshotImgBuffer = (U08 *) OSA_cmemAllocCached(size, 32);
        buffer                    = (U08 *) evtMgr->snapshotImgBuffer;

        if(buffer == NULL)
        {
            OSA_ERROR("\n> DMVA EVT: DMVA Event Manager error allocating bytes zones!!\n");
            OSA_cmemFree((void*)(evtMgr->roi));
            OSA_cmemFree((void*)evtMgr);
            gEvtMgr = evtMgr = NULL;

            return OSA_SOK;
        }
        else
        {
            PRINTF("\n> DMVA EVT: DMVA Event Manager allocated %d bytes for %d snapshots @ 0x%x",
                      size,
                      EVTMGR_MAX_EVENTS_IN_MEM * EVTMGR_MAX_SNAPSHOTS_PER_EVENT,
                      (unsigned int)buffer);
                      total += size;
        }

        PRINTF("\n> DMVA EVT: DMVA Event Manager assigning the %d bytes allocated \n>           for storing %d events in memory with %d snapshots per event.",
                    total,
                    EVTMGR_MAX_EVENTS_IN_MEM,
                    EVTMGR_MAX_SNAPSHOTS_PER_EVENT);
    }
    else
    {
        PRINTF("\n> DMVA EVT: DMVA Event Manager already created...\n");
        buffer = (U08 *) evtMgr->snapshotImgBuffer; // AYK - 0701
    }

    ///////////////////////////////////////////////////////////////////////////
    // Initialize variables
    ///////////////////////////////////////////////////////////////////////////
    evtMgr->numROI                    = numZones;
    evtMgr->enableSaveEventSnapshot   = gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable;
    evtMgr->enableSaveEventList       =  0;
    evtMgr->totEvents                 =  0;
    evtMgr->newestEvent               = -1;
    evtMgr->newestROI                 = -1;
    evtMgr->oldestEvent               =  0;
    evtMgr->memEvents                 =  0;
    evtMgr->numActiveEvents_multiFrm  =  0;
    evtMgr->numActiveEvents_singleFrm =  0;

    // AYK - 0701
    evtMgr->curFrm.width       = gVIDEO_ctrl.dmvaAppStream.dmvaWidth;    // gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmWidth;
    evtMgr->curFrm.imageStride = gVIDEO_ctrl.dmvaAppStream.dmvaOffsetH;  // gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmWidth;
    evtMgr->curFrm.height      = gVIDEO_ctrl.dmvaAppStream.dmvaHeight;   // gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.inputFrmHeight;
    evtMgr->curFrm.horzOffset  = 0;
    evtMgr->curFrm.vertOffset  = 0;
    evtMgr->curFrm.type        = DMVAL_IMG_YUV420_PLANARINTERLEAVED;
    evtMgr->curFrm.pixelDepth  = DMVAL_PIXEL_U08;
    evtMgr->curFrm.imageSize   = imgSize; // imgSize + (imgSize >> 1); // AYK - 0701
    evtMgr->curFrm.imageData   = NULL;

    if (evtMgr->enableSaveEventSnapshot)
    {
        PRINTF("\n> DMVA EVT: DMVA Event Manager set up to save snapshots.");
    }

    v=1;
    for (e=0; e < EVTMGR_MAX_EVENTS_IN_MEM; e++)
    {
        evtMgr->events[e].eventStatus = 0;

        for(s=0; s < EVTMGR_MAX_SNAPSHOTS_PER_EVENT; s++)
        {
            evtMgr->events[e].snapshot[s].imageData = buffer;
            memset(buffer, v++, imgSize);
            buffer += imgSize;
        }
    }

    // Initialize zone-based array structure
    for (r = 0; r < evtMgr->numROI; r++)
    {
        evtMgr->roi[r].status         = 0;
        evtMgr->roi[r].numObjects     = 0;
        evtMgr->roi[r].motionDetected = 0;
        evtMgr->roi[r].polyPixelArea  = 0;
        evtMgr->roi[r].roiType        = DMVAL_NO_ROI;

        if(gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.detectMode & DMVAL_DETECTMODE_IMD)
        {
            evtMgr->roi[r].roiType |= DMVAL_IMD;
        }
    }

    PRINTF("\n> DMVA EVT: DMVA Event Manager installed.\n");
    *pEvtMgr = evtMgr; // AYK - 0701

    return OSA_SOK;
}

int
EVT_startEvent(EventMgr   * evtMgr,
               DMVALimage * priorFrame,
               HPTimeStamp  timeStamp,
               S16      roiID)
{
    S16 evtIndex = roiID + 1;

    // Check to ensure that the ROI is defined
    if((roiID < -1) || (roiID >= DMVAL_MAX_POLYGONS))
    {
        PRINTF("\n> DMVA EVT: Illegal ROI(polygon) identifier...");fflush(stdout);
        return -1;
    }

    // Check to see if this ROI recently experienced activity; If so, bridge
    // events if they occur within some specified time (hard-cooded to 3 sec)
    if(evtMgr->newestROI == evtIndex)
    {
        if((timeStamp.clock.tv_sec - evtMgr->events[evtIndex].timeStamp_end.clock.tv_sec) <= 3)
        {
            evtMgr->numActiveEvents_multiFrm += 1;

            // Skip initialization; Flow will call EVT_updateEvent() next
            return 0;
        }
    }

    evtMgr->newestEvent += 1;
    evtMgr->newestROI  = evtIndex;

    // Initialize Event Record
    evtMgr->events[evtIndex].eventID.data1           = evtMgr->totEvents;
    evtMgr->events[evtIndex].eventStatus             = 1;
    evtMgr->events[evtIndex].polygonID               = roiID + 1;
    evtMgr->events[evtIndex].numFramesObserved       = 0;
    evtMgr->events[evtIndex].numRecordedObservations = 0;
    evtMgr->events[evtIndex].numObservations2Skip    = 0;
    evtMgr->events[evtIndex].numSnapshots            = 0;
    evtMgr->events[evtIndex].maxObjectsDetected      = 0;
    evtMgr->events[evtIndex].evtType                 = DMVAL_DETECTOR_NONE;
    evtMgr->events[evtIndex].objType                 = APP_UNKNOWN_OBJ;
    evtMgr->events[evtIndex].snapshotMaxObjArea      = 0;
    evtMgr->events[evtIndex].timeStamp_start         = timeStamp;
    evtMgr->events[evtIndex].timeStamp_end           = timeStamp;
    evtMgr->events[evtIndex].elapsedmsec_duration    = 0;

    // Update event manager variables
    evtMgr->totEvents   += 1;
    evtMgr->numActiveEvents_multiFrm += 1;

    // If available, capture snapshot of the previous frame (just before event
    // became a qualified event)
    if(priorFrame)
    {
        DMVALimage *snapshot = &(evtMgr->events[evtIndex].snapshot[0]);

        snapshot->height      = priorFrame->height;
        snapshot->width       = priorFrame->width;
        snapshot->pixelDepth  = priorFrame->pixelDepth;
        snapshot->imageSize   = priorFrame->imageSize;
        snapshot->imageStride = priorFrame->imageStride;
        snapshot->timeStamp   = priorFrame->timeStamp;

        memcpy(snapshot->imageData, priorFrame->imageData, priorFrame->imageSize);
        evtMgr->events[evtIndex].numSnapshots = 1;
    }

    PRINTF("\n> DMVA EVT: Event #%d started in zone (%d).\n",
                evtMgr->events[evtIndex].eventID.data1,
                evtMgr->events[evtIndex].polygonID);

    return 0;
}


int
EVT_updateEvent(EventMgr    * evtMgr,
                S16       roiID,
                HPTimeStamp   timeStamp,
                DMVALimage    * capturedFrame,
                DMVALdetectorResult     eType,
                ObjTyp      oType,
                DMVALdirection  dir)
{
    S16     i;
    S16     obs;
    S16     num;
    S16     obsIndx;
    S16     evtIndex  = roiID + 1;
    S16     numObj    = evtMgr->roi[roiID].numObjects;
    U32     * objArea = evtMgr->roi[roiID].objArea;
    DMVALbox  * objBBox = evtMgr->roi[roiID].objBBox;
    DMVALpoint  * objCent = evtMgr->roi[roiID].objCent;

    // Update the observation count here
    evtMgr->events[evtIndex].numFramesObserved += 1;

    // Sub-sample observations each time the upper bound to store them is approached
    if(evtMgr->events[evtIndex].numRecordedObservations >= EVTMGR_MAX_OBSERVATIONS_PER_EVENT)
    {
        evtMgr->events[evtIndex].numObservations2Skip++;
        obsIndx = 1;

        for (obs=2; obs < EVTMGR_MAX_OBSERVATIONS_PER_EVENT; obs+=2)
        {
            num = evtMgr->events[evtIndex].numObjects[obsIndx] = evtMgr->events[evtIndex].numObjects[obs];

            for (i=0; i < num; i++)
            {
                evtMgr->events[evtIndex].objArea[obsIndx][i] = evtMgr->events[evtIndex].objArea[obs][i];
                evtMgr->events[evtIndex].objBBox[obsIndx][i] = evtMgr->events[evtIndex].objBBox[obs][i];
                evtMgr->events[evtIndex].objCent[obsIndx][i] = evtMgr->events[evtIndex].objCent[obs][i];
            }

            obsIndx++;
        }

        evtMgr->events[evtIndex].numRecordedObservations >>= 1; // divid by 2
    }

    // Identify the next placeholder in the observation array
    obsIndx = evtMgr->events[evtIndex].numRecordedObservations;

    // Fill-in observation structure for this ROI
    evtMgr->events[evtIndex].numObjects[obsIndx]  = numObj;
    evtMgr->events[evtIndex].maxObjectsDetected   = MAX(numObj, evtMgr->events[evtIndex].maxObjectsDetected);
    evtMgr->events[evtIndex].timeStamp_end        = timeStamp;
    evtMgr->events[evtIndex].evtType              |= eType;
    evtMgr->events[evtIndex].objType              |= oType;
    evtMgr->events[evtIndex].direction            |= dir;

    // Fill-in observation structure for each object/target observed within this ROI
    evtMgr->events[evtIndex].objAreaTotal = 0;
    for (i=0; i < numObj; i++)
    {
        evtMgr->events[evtIndex].objArea[obsIndx][i] = objArea[i];
        evtMgr->events[evtIndex].objBBox[obsIndx][i] = objBBox[i];
        evtMgr->events[evtIndex].objCent[obsIndx][i] = objCent[i];
        evtMgr->events[evtIndex].objAreaTotal   += objArea[i];
    }

    if(capturedFrame)
    {
        DMVALimage *snapshot = &(evtMgr->events[evtIndex].snapshot[1]);;

        // Take snapshot when combined size of objects is a maximum
        if(evtMgr->events[evtIndex].objAreaTotal > evtMgr->events[evtIndex].snapshotMaxObjArea)
        {
            if(evtMgr->events[evtIndex].snapshotMaxObjArea == 0)
                evtMgr->events[evtIndex].numSnapshots = 2;

            evtMgr->events[evtIndex].snapshotMaxObjArea = evtMgr->events[evtIndex].objAreaTotal;

            snapshot->height      = capturedFrame->height;
            snapshot->width       = capturedFrame->width;
            snapshot->pixelDepth  = capturedFrame->pixelDepth;
            snapshot->imageSize   = capturedFrame->imageSize;
            snapshot->imageStride = capturedFrame->imageStride;
            snapshot->timeStamp   = capturedFrame->timeStamp;

            memcpy(snapshot->imageData, capturedFrame->imageData, capturedFrame->imageSize);
        }
    }

    if (evtMgr->events[evtIndex].numObservations2Skip)
    {
        if((evtMgr->events[evtIndex].numFramesObserved % (evtMgr->events[evtIndex].numObservations2Skip+1)) == 0 )
            evtMgr->events[evtIndex].numRecordedObservations++;
    }
    else
        evtMgr->events[evtIndex].numRecordedObservations++;

    PRINTF("\n> DMVA EVT: Event #%d observed %d times in zone (%d).\n",
    evtMgr->events[evtIndex].eventID.data1,
    evtMgr->events[evtIndex].numFramesObserved,
    evtMgr->events[evtIndex].polygonID);

    return 0;
}


int
EVT_evtMgrReport(EventMgr *evtMgr,
                 S16    roiID,
                 S08    *outDirPath,
                 S08    *outRootFilename)
{
    S16 i;
    U16 smFontImgWidth;
    U16 smFontCharWidth;
    U16 smFontCharHeight;
    U16 smFontDisplayXoffset;
    U16 smFontDisplayYoffset;
    U32 msec;
    U32 sub;
    U32 sec;
    U32 min;
    U32 hrs;
    S08 * str;
    S08 fullPath[256];
    S08 filename[MAX_CHARS_IN_FILENAME],evtStr[5];
    struct tm   *dateTime;
    DMVALimage *snapshot;
    U08 * smFontImage;
    S16 outputRows;
    FILE * filePtr;
    S16 evtIndex = roiID + 1;
  //  ColorFrm  * imgConversionBuf = &(evtMgr->imgConversionBuf);
    int status;
    int EvtType;
    SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;
    long long int sdFreeSpaceKB = 0;

    smFontImage          = lucidaFont6x10_576x10;
    smFontImgWidth       = 576;
    smFontCharWidth      = EVTMGR_FONT_CHAR_WIDTH;
    smFontCharHeight     = EVTMGR_FONT_CHAR_HEIGHT;
    smFontDisplayXoffset = 0;
    smFontDisplayYoffset = 0;

    ///////////////////////////////////////////////////
    // FORMAT STATUS WINDOW
    str = evtMgr->events[evtIndex].eventStatusStr;

    // Format DURATION
    // identify hours, minutes, and seconds
    msec = (U32) (evtMgr->events[evtIndex].elapsedmsec_duration);
    sec  = msec / 1000;
    sub  = msec - (sec * 1000);
    min  = sec / 60;
    sec -= 60 * min;

    if (min < 59)
        hrs = 0;
    else
    {
        hrs  = min / 60;
        min -= 60 * hrs;
    }

    // Generate snapshots
    for(i=0; i < evtMgr->events[evtIndex].numSnapshots; i++)
    {
        ///////////////////////////////////////////////////
        // FORMAT TITLE WINDOW ABOVE RIGHT DISPLAY WINDOW
        sprintf(str, "Event #%d: ROI %d (%d of %d snapshots)",
        evtMgr->events[evtIndex].eventID.data1,
        evtMgr->events[evtIndex].polygonID,
        i + 1,
        evtMgr->events[evtIndex].numSnapshots);

        // Format START TIME
        // identify hours, minutes, and seconds
        dateTime = localtime((const time_t *) &(evtMgr->events[evtIndex].timeStamp_start.clock.tv_sec) ); // convert calendar time to local
        sprintf(str,   "%s\nStart Time = %s",  str, asctime(dateTime));

        // Format string to describe event duration
        sprintf(str, "%sDuration   = %02d:%02d:%02d.%03d %4d frms",
                str,
                hrs,
                min,
                sec,
                sub,
                evtMgr->events[evtIndex].numFramesObserved);

        // Format event type(s)
        sprintf(str, "%s\nEvent Type = ", str);
        if(evtMgr->events[evtIndex].evtType == DMVAL_DETECTOR_NONE)
            sprintf(str, "%sunknown",str);
        else
        {
            if(evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_TAMPER)
            {
                EvtType = 3;
                strcpy(evtStr,"CTRD");
                evtStr[4] = '\0';
                sprintf(str, "%sTAMPER DETECTED ",str);

                if(evtMgr->events[evtIndex].evtType & DMVAL_TAMPER_SCENECHANGE)
                    sprintf(str, "%s:Scene Modified ",str);

                if(evtMgr->events[evtIndex].evtType & DMVAL_TAMPER_BRIGHT)
                    sprintf(str, "%s:Bright ",str);

                if(evtMgr->events[evtIndex].evtType & DMVAL_TAMPER_DARK)
                    sprintf(str, "%s:Dark ",str);

                if(evtMgr->events[evtIndex].evtType & DMVAL_TAMPER_MOVEMENT)
                    sprintf(str, "%s:Movement ",str);

                if(evtMgr->events[evtIndex].evtType & DMVAL_TAMPER_BLUR)
                    sprintf(str, "%s:Blur ",str);
            }

            if(evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_IMD)
            {
                EvtType = 0;
                strcpy(evtStr,"IMDT");
                evtStr[4] = '\0';
                sprintf(str, "%sMotion Detected ",str);
            }

            if((evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_TAMPER) &&
               (evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_IMD))
            {
                EvtType = 3;
                strcpy(evtStr,"CTRD");
                evtStr[4] = '\0';
                sprintf(str, "%sTAMPER DETECTED (IMD Canceled)",str);
            }

            if(evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_TRIPZONE)
            {
                EvtType = 1;
                strcpy(evtStr,"TRIP");
                evtStr[4] = '\0';
                sprintf(str, "%sTrip Zone",str);
            }

            if(evtMgr->events[evtIndex].evtType & DMVAL_DETECTOR_COUNTER)
            {
                EvtType = 2;
                strcpy(evtStr,"OBCT");
                evtStr[4] = '\0';
                sprintf(str, "%sCount Incremented ",str);
            }
        }

        // Update log file
        if((i == 10000000) && evtMgr->enableSaveEventList)
        {
            // Create or verify date-based directory and mjpeg subdirectory
            if((status = APP_createDirectory(evtMgr->events[evtIndex].timeStamp_start,fullPath, outDirPath, NULL)) != 0)
                return status;

            sprintf(filename, "%s%s_evtList_%d%02d%02d.log",
                    fullPath,
                    outRootFilename,
                    dateTime->tm_year + 1900,
                    dateTime->tm_mon + 1,
                    dateTime->tm_mday);

            if((filePtr = fopen(filename,"a+")) == NULL)
            {
                PRINTF("\n> DMVA EVT: Couldn't read or append latest record to Event List file!!!\n");
                return -1;
            }

            PRINTF("\n\n%s", str);

            fclose(filePtr);
        }

        // copy
        if(gAVSERVER_config.dmvaAppParams.dmvaAppEventRecordingVAME & 0x8)
        {
            char    fileName[128];
			int ret = 0;

            VIDEO_dmvaSetEvtRecStatus(1);

            // Get the filename without extension
            VIDEO_dmvalEvtGetFileName(fileName,evtStr, 1);

            if(gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable == 2)
            {
                // Take the snap shot of the event
				ret = VIDEO_dmvaEvtSnapShot(fileName, (int) (evtMgr->curFrm.imageData));
            }
            else if(gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable == 1)
                 {
                     if (VIDEO_CheckSDCard() == 0)
                     {
                         PRINTF("\n> DMVA EVT: Can't record... SD card is not inserted!!!\n");
                         ret = -1;
                     }
                     else
                     {
						 // check if the SD card is inserted and mounted
						 if((pSysInfo->sdcard_config.sdinsert == 0x03) && // 0x01 - Inserted,0x02 - Mounted,0x03 - Inserted and Mounted
						    (pSysInfo->dmva_config.sdUmountFlag == 0))
						 {
							 // check for space on SD card
							 sdFreeSpaceKB = GetDiskfreeSpace("/mnt/mmc");

							 if(sdFreeSpaceKB > DMVA_MIN_SIZE_REC_KB)
							 {
								 // Record the event
								 PRINTF("\n> DMVA EVT: Free space on SD card = %ld KB\n",sdFreeSpaceKB);
								 ret = VIDEO_dmvalEvtRecord(fileName);
							 }
							 else
							 {
								 PRINTF("\n> DMVA EVT: Can't record... SD card free space(%ld KB) is less!!!\n",sdFreeSpaceKB);
								 ret = -1;
							 }
						 }
						 else
						 {
							 PRINTF("\n> DMVA EVT: Can't record... SD card is not mounted!!!\n");
							 ret = -1;
						 }
                     }

				     VIDEO_dmvaSetEvtRecStatus(0);
                 }

			if(ret == 0)
			{
                //ALARM STATUS UPDATE TO LOG IN THE EVENT
                // strip '/mnt/mmc/ipnc'
                strncpy(fileName,strFileName + 14,DMVA_EVT_FILENAME_LENGTH);
                fileName[DMVA_EVT_FILENAME_LENGTH] = '\0';

                SendDMVAAlarmEventDetect(EvtType, fileName);
            }
		}
#if 0

    if (evtMgr->enableSaveEventSnapshot)
    {
      // Identify the snapshot to annotate
      snapshot = &(evtMgr->events[evtIndex].snapshot[i]);

      // Convert to YUV444
      APP_convertYUV420toYUV444(snapshot->imageData,
        imgConversionBuf->frame_Y,
        imgConversionBuf->frame_U,
        imgConversionBuf->frame_V,
        snapshot->width,
        snapshot->height);

      // Clear luma component below image content (annotation area)
      memset(&(imgConversionBuf->frame_Y[snapshot->width * snapshot->height]), 0, snapshot->width * EVTMGR_FONT_CHAR_HEIGHT * EVTMGR_NUM_STATUS_ROWS);
      outputRows = snapshot->height + EVTMGR_FONT_CHAR_HEIGHT * EVTMGR_NUM_STATUS_ROWS;

      // Convert to full-color RGB
      APP_convertYUV444toRGB888(imgConversionBuf->frame_R,
        imgConversionBuf->frame_G,
        imgConversionBuf->frame_B,
        imgConversionBuf->frame_Y,
        imgConversionBuf->frame_U,
        imgConversionBuf->frame_V,
        snapshot->width,
        outputRows);

      // Annotate bottom of snapshot with label used in log file
      APP_drawStringRGB(str,
        smFontImage,
        smFontImgWidth,
        smFontCharWidth,
        smFontCharHeight,
        (255 << 16) | (255 << 8) | 255,
        imgConversionBuf->frame_R,
        imgConversionBuf->frame_G,
        imgConversionBuf->frame_B,
        snapshot->width,
        outputRows,
        smFontDisplayXoffset,
        snapshot->height);

      // Create or verify date-based directory and mjpeg subdirectory
      APP_createDirectory(snapshot->timeStamp,
        fullPath,
        outDirPath,
        NULL);

      // Define the full FILL PATH to store this event snapshot
      sprintf(filename, "%s%s_%0d%02d%02d_evt%04d-%d.jpg",
        fullPath,
        outRootFilename,
        dateTime->tm_year + 1900,
        dateTime->tm_mon + 1,
        dateTime->tm_mday,
        evtMgr->events[evtIndex].eventID.data1,
        i);

      // Write Snapshot to disk

      APP_writeFilePPM(filename,
        imgConversionBuf->frame_R,
        imgConversionBuf->frame_G,
        imgConversionBuf->frame_B,
        outputRows,
        snapshot->width,
        NULL,
        255);

    }
#endif
  }

  // Print log label to standard output
  if (evtMgr->enableSaveEventList)
    PRINTF("\n\n%s", str);

  PRINTF("\n> DMVA EVT: %s", str);

  return 0;
}


int
EVT_endEvent(EventMgr   * evtMgr,
            DMVALimage   * previousFrame,
            DMVALimage   * capturedFrame,
            S08      * outDirPath,
            S08      * outRootFilename,
            S16        roiID,
            HPTimeStamp  timeStamp,
            DMVALdetectorResult      eType,
            ObjTyp     oType)
{
    TIME duration;
    S16 evtIndex = roiID + 1;

    // If available, save snapshot that is time-aligned with final observation
    if((capturedFrame != NULL) && (previousFrame != NULL))
    {
        DMVALimage *snapshot = &(evtMgr->events[evtIndex].snapshot[2]);
        DMVALimage  *frame;

        // If our stored frame is the same as the previously acquired frame,
        // take the newest instead so that we don't replicate images
        if((evtMgr->events[evtIndex].snapshot->timeStamp.clock.tv_sec == previousFrame->timeStamp.clock.tv_sec) &&
           (evtMgr->events[evtIndex].snapshot->timeStamp.clock.tv_usec == previousFrame->timeStamp.clock.tv_sec))
            frame = capturedFrame;
        else
            frame = previousFrame;

        snapshot->height      = frame->height;
        snapshot->width       = frame->width;
        snapshot->pixelDepth  = frame->pixelDepth;
        snapshot->imageSize   = frame->imageSize;
        snapshot->imageStride = frame->imageStride;
        snapshot->timeStamp   = frame->timeStamp;

        memcpy(snapshot->imageData, frame->imageData, frame->imageSize);
        evtMgr->events[evtIndex].numSnapshots = 3;
    }

    evtMgr->events[evtIndex].evtType       |= eType;
    evtMgr->events[evtIndex].objType       |= oType;
    evtMgr->events[evtIndex].timeStamp_end = timeStamp;
    evtMgr->events[evtIndex].eventStatus   = 0;

    // Calculate event duration in seconds
    duration  = (timeStamp.clock.tv_sec) - (evtMgr->events[evtIndex].timeStamp_start.clock.tv_sec);
    duration *= 1000000;  // duration in microseconds

    if (duration == 0)
        duration = timeStamp.clock.tv_usec - evtMgr->events[evtIndex].timeStamp_start.clock.tv_usec;
    else
        duration += timeStamp.clock.tv_usec + (1000000 - evtMgr->events[evtIndex].timeStamp_start.clock.tv_usec);

    duration /= 1000; // duration in milliseconds
    evtMgr->events[evtIndex].elapsedmsec_duration = duration;

    // There isn't enough evidence to suggest that an event is happening, so
    // report event record
    EVT_evtMgrReport(evtMgr, roiID, outDirPath, outRootFilename);

    evtMgr->numActiveEvents_multiFrm -= 1;

    return 0;
}


int
EVT_addEvent(EventMgr   * evtMgr,
             HPTimeStamp  timeStamp,
             S16      roiID,
             DMVALimage   * priorFrame,
             DMVALimage   * capturedFrame,
             S08      * outDirPath,
             S08      * outRootFilename,
             DMVALdetectorResult      eType,
             ObjTyp     oType,
             DMVALdirection dir)
{
    S16     i;
    S16     e;
    S16     obsIndx;
    S16     evtIndex  = roiID + 1;
    S16     numObj    = evtMgr->roi[roiID].numObjects;
    U32     * objArea = evtMgr->roi[roiID].objArea;
    DMVALbox  * objBBox = evtMgr->roi[roiID].objBBox;
    DMVALpoint  * objCent = evtMgr->roi[roiID].objCent;
    DMVALimage  * snapshot;

    // Check to ensure that the ROI is defined
    if((roiID < -1) || (roiID >= DMVAL_MAX_POLYGONS))
    {
        PRINTF("\n> DMVA EVT: Illegal polygon identifier...");fflush(stdout);
        return -1;
    }

    // if this is a tamper event, de-activate any existing events that may have
    // been enabled
    if((eType == DMVAL_DETECTOR_TAMPER) && evtMgr->numActiveEvents_multiFrm)
    {
        // check all possible ROIs for active event records
        for(e=0; e < EVTMGR_MAX_EVENTS_IN_MEM; e++)
        {
            // if the event is active, delete the occurrence
          if(evtMgr->events[e].eventStatus)
          {
              // Terminate IMD event record
              if(evtMgr->events[e].evtType & DMVAL_DETECTOR_IMD)
              {
                  S16 r = e - 1;

                  evtMgr->roi[r].status = 0;
              }

              EVT_endEvent(evtMgr,
                           priorFrame,
                           capturedFrame,
                           outDirPath,
                           outRootFilename,
                           evtMgr->events[e].polygonID - 1,
                           timeStamp,
                           eType,
                           oType);
            }
        }
    }

    evtMgr->newestROI = evtIndex;

    // Otherwise, initialize a new Event Record
    evtMgr->events[evtIndex].eventID.data1           = evtMgr->totEvents;
    evtMgr->events[evtIndex].polygonID               = roiID + 1;
    evtMgr->events[evtIndex].numFramesObserved       = 1;
    evtMgr->events[evtIndex].numRecordedObservations = 1;
    evtMgr->events[evtIndex].numObservations2Skip    = 0;
    evtMgr->events[evtIndex].numSnapshots            = 0;
    evtMgr->events[evtIndex].maxObjectsDetected      = 0;
    evtMgr->events[evtIndex].evtType                 = DMVAL_DETECTOR_NONE;
    evtMgr->events[evtIndex].objType                 = APP_UNKNOWN_OBJ;
    evtMgr->events[evtIndex].snapshotMaxObjArea      = 0;
    evtMgr->events[evtIndex].timeStamp_start         = timeStamp;
    evtMgr->events[evtIndex].timeStamp_end           = timeStamp;
    evtMgr->events[evtIndex].elapsedmsec_duration    = 0;

    // Update event manager variables
    evtMgr->totEvents   += 1;

    // Increment single-frame active event count
    evtMgr->numActiveEvents_singleFrm += 1;

    // Identify the next placeholder in the observation array
    obsIndx = evtMgr->events[evtIndex].numRecordedObservations;

    // If available, capture snapshot of the previous frame (just before event
    // became a qualified event)
    if(priorFrame)
    {
        DMVALimage *snapshot = &(evtMgr->events[evtIndex].snapshot[0]);

        snapshot->height      = priorFrame->height;
        snapshot->width       = priorFrame->width;
        snapshot->pixelDepth  = priorFrame->pixelDepth;
        snapshot->imageSize   = priorFrame->imageSize;
        snapshot->imageStride = priorFrame->imageStride;
        snapshot->timeStamp   = priorFrame->timeStamp;

        memcpy(snapshot->imageData, priorFrame->imageData, priorFrame->imageSize);
        evtMgr->events[evtIndex].numSnapshots++;
    }

    // Update the observation count here
    evtMgr->events[evtIndex].numFramesObserved += 1;

    // Fill-in observation structure for this ROI
    evtMgr->events[evtIndex].numObjects[obsIndx] = numObj;
    evtMgr->events[evtIndex].maxObjectsDetected  = MAX(numObj, evtMgr->events[evtIndex].maxObjectsDetected);
    evtMgr->events[evtIndex].evtType             |= eType;
    evtMgr->events[evtIndex].objType             |= oType;
    evtMgr->events[evtIndex].direction           |= dir;

    if(eType != DMVAL_DETECTOR_TAMPER)
    {
        // Fill-in observation structure for each object/target observed within this ROI
        evtMgr->events[evtIndex].objAreaTotal = 0;
        for (i=0; i < numObj; i++)
        {
            evtMgr->events[evtIndex].objArea[obsIndx][i] = objArea[i];
            evtMgr->events[evtIndex].objBBox[obsIndx][i] = objBBox[i];
            evtMgr->events[evtIndex].objCent[obsIndx][i] = objCent[i];
            evtMgr->events[evtIndex].objAreaTotal        += objArea[i];
        }
    }

    // If available, take snapshot when combined size of objects is a maximum
    if(capturedFrame)
    {
        snapshot = &(evtMgr->events[evtIndex].snapshot[evtMgr->events[evtIndex].numSnapshots]);

        snapshot->height      = capturedFrame->height;
        snapshot->width       = capturedFrame->width;
        snapshot->pixelDepth  = capturedFrame->pixelDepth;
        snapshot->imageSize   = capturedFrame->imageSize;
        snapshot->imageStride = capturedFrame->imageStride;
        snapshot->timeStamp   = capturedFrame->timeStamp;

        memcpy(snapshot->imageData, capturedFrame->imageData, capturedFrame->imageSize);
        evtMgr->events[evtIndex].numSnapshots++;
    }

    EVT_evtMgrReport(evtMgr, roiID, outDirPath, outRootFilename);

    return 0;
}


int
EVT_processDMVALoutput(DMVALhandle  * dmvalHdl,
                       EventMgr   * evtMgr)
{
    U16 r;
    U16 o;
    U16 i;
    DMVALout  * out   = gVIDEO_ctrl.dmvaAppStream.algDmvalHndl.output; // dmvalHdl->output;
    DMVALimage  * curFrame  = &(evtMgr->curFrm);  // &(app->cirFrmBufYUV420[app->indexNewestFrm]);
    DMVALimage  * prevFrame = NULL;  // &(app->cirFrmBufYUV420[app->indexPreviousFrm]);
    HPTimeStamp timeStamp   = curFrame->timeStamp;
    EvtTyp    eType   = APP_UNKNOWN_EVT;
    ObjTyp    oType   = APP_UNKNOWN_OBJ;

    ///////////////////////////////////////////////////////////////////////////
    // Initialize output for this frame
    ///////////////////////////////////////////////////////////////////////////
    for (r = 0; r < evtMgr->numROI; r++)
    {
        evtMgr->roi[r].numObjects  = 0;
        evtMgr->roi[r].dirObserved = DMVAL_DIRECTION_UNSPECIFIED;
    }

    // Clear any single-frame event notifications
    evtMgr->numActiveEvents_singleFrm = 0;

    // Don't bother if DMVAL is disabled or initializing
    if(out->modeResult < DMVAL_DISABLED)
    {
        // Loop thru all objects associated with event detection in this frame
        for(o = 0; o < out->numObjects; o++)
        {
            // select the appropriate ROI associated with object #o
            //if (out->modeResult & DMVAL_DETECTOR_IMD)
            if(out->objMetadata[o].result & DMVAL_DETECTOR_IMD)
            {
                r = out->objMetadata[o].roiID - 1;
                evtMgr->roi[r].motionDetected = 1;
            }
            else
                r = out->objMetadata[o].roiID;

            i = evtMgr->roi[r].numObjects;
            evtMgr->roi[r].dirObserved   |= out->objMetadata[o].dir;
            evtMgr->roi[r].objArea[i]     = out->objMetadata[o].area;
            evtMgr->roi[r].objBBox[i]     = out->objMetadata[o].objBox;
            evtMgr->roi[r].objCent[i]     = out->objMetadata[o].objCentroid;
            evtMgr->roi[r].objDir[i]      = out->objMetadata[o].dir;
            evtMgr->roi[r].objVelocity[i] = out->objMetadata[o].objVelocity;
            evtMgr->roi[r].numObjects++;
        }
    }

    curFrame = &(evtMgr->curFrm);
    curFrame->timeStamp = timeStamp;
    PRINTF("_____$");
    ///////////////////////////////////////////////////////////////////////////
    // Check for tamper event
    ///////////////////////////////////////////////////////////////////////////
    if(out->modeResult & DMVAL_DETECTOR_TAMPER)
    {
        int direction = 0;

        EVT_addEvent(evtMgr,
                    timeStamp,
                    0,
                    NULL,
                    curFrame,
                    SD_PATH,
                    gOutRootFilename,
                    out->modeResult,
                    oType,
                    direction);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Check this code if we are configured for IMD
    ///////////////////////////////////////////////////////////////////////////
    if(dmvalHdl->detectMode & DMVAL_DETECTMODE_IMD)
    {
        // For each region of interest, update the event status
        for(r = 0; r < evtMgr->numROI; r++)
        {
            // If motion was detected within the ROI, e.g. IMD event took place in
            // the frame
            if(evtMgr->roi[r].motionDetected)
            {
                if(evtMgr->roi[r].status == 0)
                {
                    // change status to indicate start of an active event
                    evtMgr->roi[r].status = 1;

                    EVT_startEvent(evtMgr,
                                  prevFrame,
                                  timeStamp,
                                  r);
                }

                // Update event variables & statistics
                EVT_updateEvent(evtMgr,
                                r,
                                timeStamp,
                                curFrame,
                                DMVAL_DETECTOR_IMD,
                                oType,
                                DMVAL_DIRECTION_UNSPECIFIED);

                //  clear flag
                evtMgr->roi[r].motionDetected = 0;
        }
        else
        {
            // There isn't enough evidence to suggest that an event is happening, so
            // close event record
            if(evtMgr->roi[r].status)
            {
                evtMgr->roi[r].status = 0;

                EVT_endEvent(evtMgr,
                            prevFrame,
                            curFrame,
                            SD_PATH,
                            gOutRootFilename,
                            r,
                            timeStamp,
                            DMVAL_DETECTOR_IMD,
                            oType);
            }
        }

      } // END OF FOR LOOP
  }

    ///////////////////////////////////////////////////////////////////////////
    // Check for trip zone
    ///////////////////////////////////////////////////////////////////////////
    if(out->modeResult & DMVAL_DETECTOR_TRIPZONE)
    {
        int direction = 0;
        r   = 0;
        eType = DMVAL_DETECTOR_TRIPZONE | DMVAL_DETECTOR_TRIPZONE;  // Hack

        EVT_addEvent(evtMgr,
                     timeStamp,
                     0,
                     NULL,
                     curFrame,
                     SD_PATH,
                     gOutRootFilename,
                     out->modeResult,
                     oType,
                     direction);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Check for counting
    ///////////////////////////////////////////////////////////////////////////
    if(out->modeResult & DMVAL_DETECTOR_COUNTER)
    {
        int direction = 0;
        r   = 0;
        eType = DMVAL_DETECTOR_COUNTER | DMVAL_DETECTOR_COUNTER; // Hack

        EVT_addEvent(evtMgr,
                     timeStamp,
                     0,
                     NULL,
                     curFrame,
                     SD_PATH,
                     gOutRootFilename,
                     out->modeResult,
                     oType,
                     direction);
    }

    return 0;
}
#endif




/////////////////////////////////////////////////////////////////////
// DMVA event task main function
/////////////////////////////////////////////////////////////////////
int VIDEO_dmvaEvtTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
    char    fileName[128],evtStr[6];
    int     status;
    int     checkCard;
    int     EvtType;
	int         ret;
    DmvaEvt_t *pDmvaEvt;

    U16     cmd   = OSA_msgGetCmd(pMsg);
    Bool    done  = FALSE;
    Bool    ackMsg  = FALSE;
  #ifdef DMVA_EVT_MGR
    HPTimeStamp currentTime;
  #endif
    SysInfo *pSysInfo = (SysInfo*)gAVSERVER_config.dmvaAppConfig.pSysInfo;
    long long int sdFreeSpaceKB = 0;

    if(cmd != AVSERVER_CMD_CREATE)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        return OSA_SOK;
    }

    // 0 - recorded at temp location and then to SD card
    // 1 - recorded directly on SD card
    checkCard = 1;

    // Create a directory on the SD card and create an instance of the JPEG
    // codec
    status = VIDEO_dmvalEvtTskCreate(checkCard);

    OSA_tskAckOrFreeMsg(pMsg, status);

    if(status!=OSA_SOK)
        return OSA_SOK;

    PRINTF("\n> DMVA EVT: Created Event-handling task.\n");

    while(!done)
    {
        // wait for a cmd from DMVA task
        status = OSA_tskWaitMsg(pTsk, &pMsg);

        if(status!=OSA_SOK)
            break;

        cmd = OSA_msgGetCmd(pMsg);

        switch(cmd)
        {
            case AVSERVER_CMD_DELETE:
                PRINTF("\n> DMVA EVT: AVSERVER_CMD_DELETE cmd received. Killing task...\n");
                done = TRUE;
                ackMsg = TRUE;
                break;

            case AVSERVER_CMD_DMVA_EVENT:
                // Send acknowledgement that msg was received
                OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

                // If video recording has been disabled, leave....
                if((gAVSERVER_config.dmvaAppParams.dmvaAppEventRecordingVAME & 0x8) == 0)
                    break;

                // Get the DMVA event type
                pDmvaEvt = OSA_msgGetPrm(pMsg);

                if(pDmvaEvt == NULL)
                    break;

                if(pDmvaEvt->modeResult & DMVAL_DETECTOR_IMD)
                {
                    EvtType = 0;
                    sprintf(evtStr,"IMDT");
                    evtStr[4] = '\0';
				//PRINTF("\n> DMVA EVT: DMVAL_DETECTOR_IMD event occured\n");
                }

                if(pDmvaEvt->modeResult & DMVAL_DETECTOR_TRIPZONE)
                {
                    EvtType = 1;
                    sprintf(evtStr,"TRIP");
                    evtStr[4] = '\0';
				//PRINTF("\n> DMVA EVT: DMVAL_DETECTOR_TRIPZONE event occured\n");
                }

                if(pDmvaEvt->modeResult & DMVAL_DETECTOR_COUNTER)
                {
                    EvtType = 2;
                    sprintf(evtStr,"OBCT");
                    evtStr[4] = '\0';
				//PRINTF("\n> DMVA EVT: DMVAL_DETECTOR_COUNTER event occured\n");
                }

                if(pDmvaEvt->modeResult & DMVAL_DETECTOR_TAMPER)
                {
                    EvtType = 3;
                    sprintf(evtStr,"CTRD");
                    evtStr[4] = '\0';
				//PRINTF("\n> DMVA EVT: DMVAL_DETECTOR_TAMPER event occured\n");
                }

#ifdef DMVA_EVT_MGR
                // Get current time down to the msec
                PRINTF("\n> DMVA EVT: CALLING APP_getTime_HiPrecisionClock...");
                APP_getTime_HiPrecisionClock(&currentTime);

                PRINTF("DONE!\n> DMVA EVT: gEvtMgr @ %d", (int)gEvtMgr);
                gEvtMgr->curFrm.timeStamp = currentTime;
                gEvtMgr->curFrm.imageData = (U08 *) pDmvaEvt->addr;
                PRINTF("\n> DMVA EVT: CALLING EVT_processDMVALoutput...");fflush(stdout);

                EVT_processDMVALoutput(&(gVIDEO_ctrl.dmvaAppStream.algDmvalHndl), gEvtMgr);

                // Flush any other messages
                OSA_tskFlushMsg(pTsk);
                break;
#else
                if((pDmvaEvt->modeResult & DMVAL_DETECTOR_IMD)      ||
                   (pDmvaEvt->modeResult & DMVAL_DETECTOR_TAMPER)   ||
                   (pDmvaEvt->modeResult & DMVAL_DETECTOR_TRIPZONE) ||
                   (pDmvaEvt->modeResult & DMVAL_DETECTOR_COUNTER))
                {
                    VIDEO_dmvaSetEvtRecStatus(1);

                    // Get the filename without extension
                    VIDEO_dmvalEvtGetFileName(fileName,evtStr,checkCard);

                    if(gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable == 2)
                    {
                        // Take the snap shot of the event
                        ret = VIDEO_dmvaEvtSnapShot(fileName,pDmvaEvt->addr);
                    }
                    else if(gAVSERVER_config.dmvaAppConfig.dmvaexptevtrecordenable == 1)
                         {
                             if (VIDEO_CheckSDCard() == 0)
                             {
                                 PRINTF("\n> DMVA EVT: Can't record... SD card is not inserted!!!\n");
                                 ret = -1;
                             }
                             else
                             {
								 // check if the SD card is inserted and mounted
								 if((pSysInfo->sdcard_config.sdinsert == 0x03) && // 0x01 - Inserted,0x02 - Mounted,0x03 - Inserted and Mounted
								    (pSysInfo->dmva_config.sdUmountFlag == 0))
								 {
                                     // check for space on SD card
                                     sdFreeSpaceKB = GetDiskfreeSpace("/mnt/mmc");

                                     if(sdFreeSpaceKB > DMVA_MIN_SIZE_REC_KB)
                                     {
                                         // Record the event
                                         PRINTF("\n> DMVA EVT: Free space on SD card = %ld KB\n",sdFreeSpaceKB);
						                 ret = VIDEO_dmvalEvtRecord(fileName);
									 }
									 else
									 {
									     PRINTF("\n> DMVA EVT: Can't record... SD card free space(%ld KB) is less!!!\n",sdFreeSpaceKB);
									     ret = -1;
									 }
								 }
								 else
								 {
									 PRINTF("\n> DMVA EVT: Can't record... SD card is not mounted!!!\n");
									 ret = -1;
								 }
                             }

						     VIDEO_dmvaSetEvtRecStatus(0);
                         }

                    if(ret == 0)
                    {
                        //ALARM STATUS UPDATE TO LOG IN THE EVENT
                        // strip '/mnt/mmc/ipnc'
                        strncpy(fileName,strFileName + 14,DMVA_EVT_FILENAME_LENGTH);
                        fileName[DMVA_EVT_FILENAME_LENGTH] = '\0';

                        SendDMVAAlarmEventDetect(EvtType,fileName);
					}
                }

                break;
#endif
            default:
                PRINTF("\n> DMVA EVT: Unknown CMD = 0x%04x\n", cmd);
                OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
                break;
        }// switch(cmd)
    }

    PRINTF("\n> DMVA EVT: Delete...\n");

    VIDEO_dmvalEvtTskDelete();

    if(ackMsg)
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

    PRINTF("\n> DMVA EVT: Delete...DONE\n");

    if(gVIDEO_ctrl.dmvaAppStream.dmvaError)
    {
        PRINTF("\n> DMVA EVT: DMVA timeout happened while running !!!\n");
    }

    return OSA_SOK;
}
