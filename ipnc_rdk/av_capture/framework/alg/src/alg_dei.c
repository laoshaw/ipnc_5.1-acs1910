
#include <alg_priv.h>
#include <ti/sdo/codecs/dei/idei.h>
#include <alg_dei.h>

#define DEI_2_0

#define DEI_DBG

#ifdef DEI_DBG
    #define DEI_PRINTF(fmt, args...)   fprintf(stderr, fmt, ##args)
#else
    #define DEI_PRINTF(fmt, args...)
#endif

#define DEI_ERROR(fmt, args...)        fprintf(stderr, fmt, ##args)

typedef struct
{
    VIDENC1_Handle hDei;
    VIDENC1_Params params;
    IDEI_Params extParams;
    IDEI_DynamicParams extDynParams;

}DEI_Instance_t;

typedef struct
{
    DEI_Instance_t 	deiInstance[2];

}DEI_Ctrl_t;


DEI_Ctrl_t gDEI_CTRL;

Int32 ALG_deiCreate(Uint32 deiId,ALG_DeiCreate *pCreate)
{
#ifdef DEI_2_0
	gDEI_CTRL.deiInstance[deiId].params.maxHeight = pCreate->height;
	gDEI_CTRL.deiInstance[deiId].params.maxWidth  = pCreate->width;

    gDEI_CTRL.deiInstance[deiId].extParams.videncParams.size = sizeof(IDEI_Params);
	gDEI_CTRL.deiInstance[deiId].extParams.frameHeight       = pCreate->height;
	gDEI_CTRL.deiInstance[deiId].extParams.frameWidth        = pCreate->width;
	gDEI_CTRL.deiInstance[deiId].extParams.inLineOffset      = pCreate->width;
	gDEI_CTRL.deiInstance[deiId].extParams.outLineOffset     = pCreate->width;
	gDEI_CTRL.deiInstance[deiId].extParams.threshold_low     = pCreate->threshold[0];
	gDEI_CTRL.deiInstance[deiId].extParams.threshold_high    = pCreate->threshold[1];

	if(pCreate->inFormat == DRV_DATA_FORMAT_YUV422)
	{
	    gDEI_CTRL.deiInstance[deiId].extParams.inputFormat   = XDM_YUV_422ILE;
	}
	else
	{
		gDEI_CTRL.deiInstance[deiId].extParams.inputFormat   = XDM_YUV_420SP;
	}

	if(pCreate->outFormat == DRV_DATA_FORMAT_YUV422)
	{
	    gDEI_CTRL.deiInstance[deiId].extParams.outputFormat  = XDM_YUV_422ILE;
	}
	else
	{
		gDEI_CTRL.deiInstance[deiId].extParams.outputFormat  = XDM_YUV_420SP;
	}

	gDEI_CTRL.deiInstance[deiId].extParams.q_num             = 1;
	gDEI_CTRL.deiInstance[deiId].extParams.askIMCOPRes       = 0;
	gDEI_CTRL.deiInstance[deiId].extParams.sysBaseAddr       = pCreate->sysBaseAddr;
#else
	gDEI_CTRL.deiInstance[deiId].params.size              = sizeof(IDEI_Params);
	gDEI_CTRL.deiInstance[deiId].params.maxHeight         = pCreate->height;
	gDEI_CTRL.deiInstance[deiId].params.maxWidth          = pCreate->width;

	if(pCreate->inFormat == DRV_DATA_FORMAT_YUV422)
	{
		gDEI_CTRL.deiInstance[deiId].params.inputChromaFormat = XDM_YUV_422ILE;
	}
	else
	{
		gDEI_CTRL.deiInstance[deiId].params.inputChromaFormat = XDM_YUV_420SP;
	}

	gDEI_CTRL.deiInstance[deiId].extParams.videncParams    = gDEI_CTRL.deiInstance[deiId].params;

	if(pCreate->outFormat == DRV_DATA_FORMAT_YUV422)
	{
		gDEI_CTRL.deiInstance[deiId].extParams.outputFormat    = XDM_YUV_422ILE;
	}
	else
	{
		gDEI_CTRL.deiInstance[deiId].extParams.outputFormat    = XDM_YUV_420SP;
	}

	gDEI_CTRL.deiInstance[deiId].extParams.subWindowWidth  = pCreate->width;
	gDEI_CTRL.deiInstance[deiId].extParams.subWindowHeight = pCreate->height;
	gDEI_CTRL.deiInstance[deiId].extParams.threshold       = pCreate->threshold[0];
	gDEI_CTRL.deiInstance[deiId].extParams.q_num           = 1;
	gDEI_CTRL.deiInstance[deiId].extParams.askIMCOPRes     = 0;
	gDEI_CTRL.deiInstance[deiId].extParams.sysBaseAddr     = pCreate->sysBaseAddr;
#endif

	// Create DEI algorithm
	gDEI_CTRL.deiInstance[deiId].hDei = VIDENC1_create(gALG_hEngine,
												       ALG_VID_ADV_FEATURE_DEI_NAME,
												       (VIDENC1_Params *)&gDEI_CTRL.deiInstance[deiId].extParams);

	if(gDEI_CTRL.deiInstance[deiId].hDei == NULL)
	{
		DEI_ERROR(" %d - DEI Failed to open algorithm: %s (0x%x)\n",deiId,ALG_VID_ADV_FEATURE_DEI_NAME,Engine_getLastError(gALG_hEngine));
		return OSA_EFAIL;
	}

	return OSA_SOK;
}

Int32 ALG_deiRun(Uint32 deiId,ALG_DeiRun *pRun,ALG_DeiStatus *pStatus)
{
	XDAS_Int32 status;
    IVIDEO1_BufDescIn inBufDesc;
    XDM_BufDesc outBufDesc;
    VIDENC1_InArgs inArgs;
    IDEI_OutArgs outArgs;
    XDAS_Int32 inOffset,outOffset;
    XDAS_Int32 outBufSizeArray[2], bufferSize;
    XDAS_Int8 *outbufs[2];

	gDEI_CTRL.deiInstance[deiId].extDynParams.videncDynamicParams.inputHeight = gDEI_CTRL.deiInstance[deiId].params.maxHeight;
	gDEI_CTRL.deiInstance[deiId].extDynParams.videncDynamicParams.inputWidth  = gDEI_CTRL.deiInstance[deiId].params.maxWidth;

    inOffset = gDEI_CTRL.deiInstance[deiId].params.maxWidth * gDEI_CTRL.deiInstance[deiId].params.maxHeight;

	// Provide some gap between Luma and Chroma planes
	inOffset += (DEI_Y_UV__GAP * gDEI_CTRL.deiInstance[deiId].params.maxWidth);
	outOffset = inOffset;

	// input buffers
	inBufDesc.frameWidth  = OSA_align(gDEI_CTRL.deiInstance[deiId].params.maxWidth, 16);    // Not used
	inBufDesc.frameHeight = OSA_align(gDEI_CTRL.deiInstance[deiId].params.maxHeight, 16);   // Not used
	inBufDesc.framePitch  = gDEI_CTRL.deiInstance[deiId].params.maxWidth;

	bufferSize = (gDEI_CTRL.deiInstance[deiId].params.maxWidth * gDEI_CTRL.deiInstance[deiId].params.maxHeight);

	inBufDesc.numBufs = 4;

	inBufDesc.bufDesc[0].bufSize    = bufferSize;
	inBufDesc.bufDesc[0].buf        = (XDAS_Int8 *)pRun->inAddr;
	inBufDesc.bufDesc[0].accessMask = 0;

	inBufDesc.bufDesc[1].bufSize    = bufferSize/2;
	inBufDesc.bufDesc[1].buf        = (XDAS_Int8 *)(pRun->inAddr + inOffset);
	inBufDesc.bufDesc[1].accessMask = 0;

	inBufDesc.bufDesc[2].bufSize    = bufferSize;
	inBufDesc.bufDesc[2].buf        = (XDAS_Int8 *)pRun->inAddrPast;
	inBufDesc.bufDesc[2].accessMask = 0;

	inBufDesc.bufDesc[3].bufSize    = bufferSize/2;
	inBufDesc.bufDesc[3].buf        = (XDAS_Int8 *)(pRun->inAddrPast + inOffset);
	inBufDesc.bufDesc[3].accessMask = 0;

	// output buffers
	outBufDesc.numBufs  = 2;
	outbufs[0]          = (XDAS_Int8*)pRun->outAddr;
	outbufs[1]          = (XDAS_Int8*)(pRun->outAddr + outOffset);
	outBufSizeArray[0]  = outOffset;
	outBufSizeArray[1]  = outOffset/2;

	outBufDesc.bufSizes = outBufSizeArray;
	outBufDesc.bufs     = outbufs;

	inArgs.size = sizeof(VIDENC1_InArgs);
	outArgs.videncOutArgs.size = sizeof(IDEI_OutArgs);

#ifndef DEI_2_0
    status = VIDENC1_control((VIDENC1_Handle)gDEI_CTRL.deiInstance[deiId].hDei,
                             XDM_SETPARAMS,
                             (IVIDENC1_DynamicParams *)&gDEI_CTRL.deiInstance[deiId].extDynParams,
                             (IVIDENC1_Status *)&status);

	if (status != VIDENC1_EOK)
	{
		DEI_ERROR(" %d - DEI: Dynamic Parameter updation failed !!!\n",deiId);
		return OSA_EFAIL;
	}
#endif
	
    status = VIDENC1_process((VIDENC1_Handle)gDEI_CTRL.deiInstance[deiId].hDei,
                             &inBufDesc,
                             &outBufDesc,
                             &inArgs,
                             (IVIDENC1_OutArgs *)&outArgs);

	if (status != VIDENC1_EOK)
	{
		DEI_ERROR(" %d - DEI: Process failed !!!\n",deiId);
		pStatus->outWidth  = 0;
		pStatus->outHeight = 0;
		return OSA_EFAIL;
	}

#ifdef DEI_2_0
	pStatus->outWidth  = outArgs.outWidth;
	pStatus->outHeight = outArgs.outHeight;
#else
	pStatus->outWidth  = gDEI_CTRL.deiInstance[deiId].extParams.subWindowWidth;
	pStatus->outHeight = gDEI_CTRL.deiInstance[deiId].extParams.subWindowHeight;
#endif	
	
    return OSA_SOK;
}

Int32 ALG_deiDelete(Uint32 deiId)
{
	VIDENC1_delete(gDEI_CTRL.deiInstance[deiId].hDei);
    return OSA_SOK;
}
