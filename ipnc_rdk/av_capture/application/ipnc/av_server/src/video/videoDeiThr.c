#include <avserver.h>
#include <alg_dei.h>
#include <sys/time.h>

//#define DUMP_IN_FRAMES
//#define DUMP_OUT_FRAMES

#if defined(DUMP_IN_FRAMES) || defined(DUMP_OUT_FRAMES)
    #define MAX_FRAME_DUMP_CNT		(1000)
    FILE *inFile,*outFile;
	Uint32 gInDumpFrameCnt,gOutDumpFrameCnt;
	Uint32 gInFormat,gOutFormat;
#endif

int pastBufInfo;

int VIDEO_deiTskCreate(Int32 sysRegBase)
{
	ALG_DeiCreate createPrm;
	Uint32 bufSize;
	int status = OSA_EFAIL;

	/* Allocate memory to store previous frame */
	bufSize     = (OSA_align(gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataWidth,32) * gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight * 2 );
	pastBufInfo = OSA_cmemAlloc(bufSize, 32);

	createPrm.width        = OSA_align(gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataWidth,32);
	createPrm.height       = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight;
	createPrm.inFormat     = DRV_DATA_FORMAT_YUV422;
	createPrm.outFormat    = gAVSERVER_config.captureYuvFormat;
	createPrm.threshold[0] = 4;
	createPrm.threshold[1] = 20;
	createPrm.sysBaseAddr  = sysRegBase;

	status = ALG_deiCreate(0, &createPrm);

	if(status == OSA_EFAIL)
	{
		OSA_ERROR("ALG_deiCreate()\n");
		return OSA_EFAIL;
	}

#ifdef DUMP_IN_FRAMES
    gInDumpFrameCnt = 0;
	gInFormat       = createPrm.inFormat;

    inFile = fopen("deiIn.yuv","wb");
    if(inFile == NULL)
    {
        printf("DEI >> Error opening input file\n");
    }
#endif

#ifdef DUMP_OUT_FRAMES
    gOutDumpFrameCnt = 0;
	gOutFormat       = createPrm.outFormat;

    outFile = fopen("deiOut.yuv","wb");
    if(outFile == NULL)
    {
        printf("DEI >> Error opening output file\n");
    }
#endif

  return OSA_SOK;
}

int VIDEO_deiTskDelete()
{
	ALG_deiDelete(0);

	OSA_cmemFree(pastBufInfo);

	return OSA_SOK;
}

int VIDEO_deiTskRun(int streamId)
{
	static int putfull = 0,putempty = 0,getfull =0,getempty =0;
	int status = OSA_EFAIL, inBufId, outBufId;
	OSA_BufInfo *pInBufInfo, *pOutBufInfo;
	VIDEO_BufHeader *pOutBufHeader;
	ALG_DeiRun deiPrm;
	ALG_DeiStatus deiStatus;
	DRV_FrameCopyPrm frameCopy;
	struct timeval curTime;

#if defined(DUMP_IN_FRAMES) || defined(DUMP_OUT_FRAMES)
  Uint32 writeSize;
#endif

	pInBufInfo = AVSERVER_bufGetFull( VIDEO_TSK_DEI, streamId, &inBufId, OSA_TIMEOUT_FOREVER);

	OSA_assert(pInBufInfo != NULL);

	if(pInBufInfo != NULL)
	{
		/* Get the current time */
		gettimeofday(&curTime, NULL);
	
		pOutBufInfo = AVSERVER_bufGetEmpty( VIDEO_TSK_DEI, streamId, &outBufId, OSA_TIMEOUT_FOREVER);
		OSA_assert(pOutBufInfo!=NULL);

		if(pOutBufInfo != NULL)
		{
			pOutBufHeader = (VIDEO_BufHeader*)pOutBufInfo->virtAddr;

			OSA_assert(pOutBufHeader != NULL);
			
			/* Put the timestamp */
			pOutBufHeader->timestamp = (curTime.tv_sec * 1000) + (curTime.tv_usec + 500)/1000;

#ifdef DUMP_IN_FRAMES
		    if(inFile != NULL)
		    {
			    if(gInDumpFrameCnt < MAX_FRAME_DUMP_CNT)
			    {
				    if(gInFormat == DRV_DATA_FORMAT_YUV422)
					{
						writeSize = ((gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH >> 1) * gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight * 2);
					}
					else
					{
                        writeSize = ((gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH >> 1) * gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight * 3)/2;
					}

				    if(fwrite((pInBufInfo->virtAddr),1,writeSize,inFile) != writeSize)
				    {
					    printf("DEI >> Error writing to input file\n");
				    }
			    }
			    else if(gInDumpFrameCnt == MAX_FRAME_DUMP_CNT)
				     {
					     fclose(inFile);
				     }

			    gInDumpFrameCnt ++;
		    }
#endif

			deiPrm.inAddr     = pInBufInfo->virtAddr;
			deiPrm.outAddr    = pOutBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
			deiPrm.inAddrPast = pastBufInfo;

			OSA_prfBegin(&gAVSERVER_ctrl.deiPrf);

			status = ALG_deiRun(0, &deiPrm ,&deiStatus );

			if(status != OSA_SOK)
			{
				OSA_ERROR("ALG_deiRun()\n");
			}

			OSA_prfEnd(&gAVSERVER_ctrl.deiPrf, 1);

#ifdef DUMP_OUT_FRAMES
		    if(outFile != NULL)
		    {
			    if(gOutDumpFrameCnt < MAX_FRAME_DUMP_CNT)
			    {
				    if(gOutFormat == DRV_DATA_FORMAT_YUV422)
					{
					    writeSize = ((gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH >> 1) * deiStatus.outHeight * 2);
					}
					else
					{
						writeSize = ((gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH >> 1) * deiStatus.outHeight * 3)/2;
					}

				    if(fwrite((pOutBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE),1,writeSize,outFile) != writeSize)
				    {
					    printf("DEI >> Error writing to output file\n");
				    }
			    }
			    else if(gOutDumpFrameCnt == MAX_FRAME_DUMP_CNT)
				     {
					     fclose(outFile);
				     }

			    gOutDumpFrameCnt ++;
		    }
#endif

			pOutBufHeader->startX  = 0;
			pOutBufHeader->startY  = 0;
			pOutBufHeader->width   = deiStatus.outWidth;
			pOutBufHeader->height  = deiStatus.outHeight;
			pOutBufHeader->offsetH = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH/2;
			pOutBufHeader->offsetV = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight;

			/* Copy the current frame intp previous frame buffer*/
			frameCopy.srcPhysAddr = pInBufInfo->physAddr;
			frameCopy.srcVirtAddr = pInBufInfo->virtAddr;
			frameCopy.dstPhysAddr = CMEM_getPhys(pastBufInfo);
			frameCopy.dstVirtAddr = pastBufInfo;
			frameCopy.srcOffsetH  = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH/2;
			frameCopy.srcOffsetV  = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight;
			frameCopy.dstOffsetH  = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH/2;
			frameCopy.dstOffsetV  = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight;
			frameCopy.copyWidth   = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataOffsetH/2;
			frameCopy.copyHeight  = gVIDEO_ctrl.captureInfo.isifInfo.ddrOutDataHeight;
			frameCopy.srcStartX   = 0;
			frameCopy.srcStartY   = 0;
			frameCopy.dstStartX   = 0;
			frameCopy.dstStartY   = 0;
			frameCopy.dataFormat  = DRV_DATA_FORMAT_YUV422;

			status = DRV_frameCopy(NULL, &frameCopy);

			if(status != OSA_SOK)
			{
				OSA_ERROR("DRV_frameCopy()\n");
			}

			VIDEO_fdCopyRun(VIDEO_TSK_DEI, streamId, pOutBufInfo);
			VIDEO_swosdRun(VIDEO_TSK_DEI, streamId, pOutBufInfo);

			AVSERVER_bufPutFull( VIDEO_TSK_DEI, streamId, outBufId);

			VIDEO_displayCopyRun(VIDEO_TSK_DEI, streamId, pOutBufInfo);
			VIDEO_dmvaCopyRun(VIDEO_TSK_DEI, streamId, pOutBufInfo);
		}

		AVSERVER_bufPutEmpty(VIDEO_TSK_DEI, streamId, inBufId);
	}

	return status;
}

int VIDEO_deiTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
	int status, streamId;
	Bool done = FALSE, ackMsg = FALSE;
	Uint16 cmd = OSA_msgGetCmd(pMsg);
	Int32 sysRegBase;

#ifdef AVSERVER_DEBUG_VIDEO_DEI_THR
	OSA_printf(" DEI: Recevied CMD = 0x%04x\n", cmd);
#endif

	if(cmd != AVSERVER_CMD_CREATE)
	{
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
		return OSA_SOK;
	}

#ifdef AVSERVER_DEBUG_VIDEO_DEI_THR
	OSA_printf(" DEI: Create...\n");
#endif

	DM365MM_init();
	sysRegBase = DM365MM_mmap(0x01C40000,0x4000);

	status = VIDEO_deiTskCreate(sysRegBase);

	OSA_tskAckOrFreeMsg(pMsg, status);

	if(status!=OSA_SOK)
	{
		OSA_ERROR("VIDEO_deiTskCreate()\n");
		return status;
	}

	while(!done)
	{
		status = OSA_tskWaitMsg(pTsk, &pMsg);

		if(status != OSA_SOK)
		{
			break;
		}

		cmd = OSA_msgGetCmd(pMsg);

		switch(cmd)
		{
			case AVSERVER_CMD_DELETE:
				done   = TRUE;
				ackMsg = TRUE;
				break;

			case AVSERVER_CMD_NEW_DATA:
				streamId = (int)OSA_msgGetPrm(pMsg);
				OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
				status = VIDEO_deiTskRun(streamId);
				break;

			default:

#ifdef AVSERVER_DEBUG_VIDEO_DEI_THR
				OSA_printf(" DEI: Unknown CMD = 0x%04x\n", cmd);
#endif
				OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
				break;
		}
	}

#ifdef AVSERVER_DEBUG_VIDEO_DEI_THR
	OSA_printf(" DEI: Delete...\n");
#endif

	VIDEO_deiTskDelete();

	DM365MM_ummap(sysRegBase,0x4000);
	DM365MM_exit();

	if(ackMsg)
	{
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
	}

#ifdef AVSERVER_DEBUG_VIDEO_DEI_THR
	OSA_printf(" DEI: Delete...DONE\n");
#endif

	return OSA_SOK;
}

int VIDEO_deiCreate()
{
	int status;

	status = OSA_tskCreate(&gVIDEO_ctrl.deiTsk, VIDEO_deiTskMain, VIDEO_DEI_THR_PRI, VIDEO_DEI_STACK_SIZE, 0);

	if(status != OSA_SOK)
	{
		OSA_ERROR("OSA_tskCreate()\n");
		return status;
	}

	return status;
}

int VIDEO_deiDelete()
{
	int status;

	status = OSA_tskDelete(&gVIDEO_ctrl.deiTsk);

	return status;
}
