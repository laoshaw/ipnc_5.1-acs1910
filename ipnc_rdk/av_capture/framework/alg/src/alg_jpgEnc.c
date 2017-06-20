
#include <alg_priv.h>
#include <alg_util.h>
#include <alg_jpgEnc.h>
#include <osa_cmem.h>
#include <osa_file.h>

typedef struct {

  IMGENC1_Handle   hEncode;
  ALG_JpgEncCreate createPrm;
  char             algName[20];
  IMGENC1_Params  params;
  IMGENC1_DynamicParams dynamicParams;

  IMGENC1_Status  encStatus;

} ALG_JpgEncObj;

void *ALG_jpgEncCreate(ALG_JpgEncCreate *create)
{
  ALG_JpgEncObj *pObj;
  XDAS_Int32 status;

  if(gALG_hEngine==NULL)
    return NULL;

  pObj = OSA_memAlloc(sizeof(ALG_JpgEncObj));
  if(pObj==NULL)
    return NULL;

  memset(pObj, 0, sizeof(*pObj));

  strcpy(pObj->algName, ALG_VID_CODEC_JPEG_ENC_NAME);

  memcpy(&pObj->createPrm, create, sizeof(pObj->createPrm));

  pObj->params.size 			= sizeof(IMGENC1_Params);
  pObj->params.maxWidth 		= (create->offsetH>create->width)?create->offsetH:create->width;
  pObj->params.maxHeight		= (create->offsetV>create->height)?create->offsetV:create->height;
  pObj->params.maxScans  		= 1;
  pObj->params.dataEndianness 	= XDM_BYTE;

  if(create->encDataFormat==ALG_VID_DATA_FORMAT_YUV422)
    pObj->params.forceChromaFormat = XDM_YUV_422P;
  else
    pObj->params.forceChromaFormat = XDM_YUV_420P;

  pObj->hEncode = IMGENC1_create(gALG_hEngine, pObj->algName, &pObj->params);

  if (pObj->hEncode == NULL) {
    OSA_ERROR("Failed to open video encode algorithm: %s (0x%x)\n", pObj->algName, Engine_getLastError(gALG_hEngine));
    OSA_memFree(pObj);
    return NULL;
  }

  status = ALG_jpgEncSetQvalue(pObj, create->qValue);
  if(status!=OSA_SOK) {
    ALG_jpgEncDelete(pObj);
    return NULL;
  }

  return pObj;
}

int ALG_jpgEncSetQvalue(void *hndl, int qValue)
{
  ALG_JpgEncObj  *pObj = (ALG_JpgEncObj*)hndl;
  XDAS_Int32 status;

  if(pObj==NULL)
    return OSA_EFAIL;

  pObj->dynamicParams.size = sizeof(IMGENC1_DynamicParams);
  pObj->dynamicParams.numAU = XDM_DEFAULT;

  if(pObj->createPrm.dataFormat==ALG_VID_DATA_FORMAT_YUV422)
    pObj->dynamicParams.inputChromaFormat  = XDM_YUV_422ILE;
  else
    pObj->dynamicParams.inputChromaFormat  = XDM_YUV_420SP;

  qValue = checkMinMax((int)qValue, ALG_JPG_ENC_Q_VALUE_MIN, ALG_JPG_ENC_Q_VALUE_MAX);

  pObj->dynamicParams.inputWidth      = pObj->createPrm.width;
  pObj->dynamicParams.inputHeight     = pObj->createPrm.height;
  pObj->dynamicParams.captureWidth    = (pObj->createPrm.offsetH>pObj->createPrm.width)?pObj->createPrm.offsetH:pObj->createPrm.width;
  pObj->dynamicParams.generateHeader  = XDM_ENCODE_AU;
  pObj->dynamicParams.qValue          = qValue;

  /* Set video encoder dynamic parameters */
  pObj->encStatus.size = sizeof(pObj->encStatus);
  pObj->encStatus.data.buf = NULL;

  status = IMGENC1_control(pObj->hEncode, XDM_SETPARAMS, &pObj->dynamicParams, &pObj->encStatus);

  if (status != IMGENC1_EOK) {
    OSA_ERROR("XDM_SETPARAMS failed, status=%ld\n", status);
    return OSA_EFAIL;
  }

  pObj->encStatus.data.buf = NULL;

  status = IMGENC1_control(pObj->hEncode, XDM_GETBUFINFO, &pObj->dynamicParams, &pObj->encStatus);

  if (status != IMGENC1_EOK) {
      OSA_ERROR("XDM_GETBUFINFO failed, status=%ld\n", status);
      return OSA_EFAIL;
  }

  #ifdef ALG_DEBUG
  OSA_printf(" ALG: ImgEnc: XDM_GETBUFINFO: min in bufs:%ld buf(0):%ld buf(1):%ld\n",pObj->encStatus.bufInfo.minNumInBufs,pObj->encStatus.bufInfo.minInBufSize[0], pObj->encStatus.bufInfo.minInBufSize[1]);
  #endif

  return OSA_SOK;
}

int ALG_jpgEncRun(void *hndl, ALG_JpgEncRunPrm *prm, ALG_JpgEncRunStatus *runStatus)
{
  ALG_JpgEncObj  *pObj = (ALG_JpgEncObj*)hndl;
  XDM1_BufDesc    inBufDesc;
  XDM1_BufDesc    outBufDesc;
  XDAS_Int32      status;
  IMGENC1_InArgs  inArgs;
  IMGENC1_OutArgs outArgs;
  Uint32          bytesPerPixel, offset;

  runStatus->bytesGenerated = 0;

  if(pObj==NULL)
    return OSA_EFAIL;

  offset = pObj->createPrm.offsetH * pObj->createPrm.offsetV;

  if(prm->outDataMaxSize==0)
    prm->outDataMaxSize = pObj->createPrm.width*pObj->createPrm.height*2;

  prm->inStartY = OSA_floor(prm->inStartY, 2);
  prm->inStartX = OSA_floor(prm->inStartX, 2);

  inBufDesc.numBufs       = 1;

  bytesPerPixel = 1;
  if(pObj->createPrm.dataFormat==ALG_VID_DATA_FORMAT_YUV422) {
    bytesPerPixel = 2;
  }

  inBufDesc.descs[0].bufSize    = pObj->createPrm.offsetH*pObj->createPrm.offsetV*bytesPerPixel;
  inBufDesc.descs[0].buf        = (XDAS_Int8*)(prm->inAddr + prm->inStartY*pObj->createPrm.offsetH*bytesPerPixel + prm->inStartX*bytesPerPixel);
  inBufDesc.descs[0].accessMask = 0;

  if(pObj->createPrm.dataFormat==ALG_VID_DATA_FORMAT_YUV420) {
    inBufDesc.numBufs = 2;

    inBufDesc.descs[1].bufSize    = pObj->createPrm.offsetH*pObj->createPrm.offsetV/2;
    inBufDesc.descs[1].buf        = (XDAS_Int8*)(prm->inAddr + offset + prm->inStartY*pObj->createPrm.offsetH/2 + prm->inStartX);
    inBufDesc.descs[1].accessMask = 0;
  }

  if(prm->snapEnable) {
  	if(prm->snapMode==SNAPSHOT_YUV) {
  		OSA_fileWriteVideoData(prm->snapFilename, (Uint8 *)inBufDesc.descs[0].buf, (Uint8 *)inBufDesc.descs[1].buf,
  			pObj->createPrm.width, pObj->createPrm.height, pObj->createPrm.offsetH);
  	}
  	else {
		SnapshotFrameData(prm->snapFilename, (Uint8 *)inBufDesc.descs[0].buf, (Uint8 *)inBufDesc.descs[1].buf,
  			pObj->createPrm.width, pObj->createPrm.height, pObj->createPrm.offsetH);
	}
  }

  outBufDesc.numBufs              = 1;
  outBufDesc.descs[0].buf         = (XDAS_Int8*)prm->outAddr;
  outBufDesc.descs[0].bufSize     = prm->outDataMaxSize;
  outBufDesc.descs[0].accessMask  = 0;

  inArgs.size         = sizeof(IMGENC1_InArgs);
  outArgs.size        = sizeof(IMGENC1_OutArgs);

  status = IMGENC1_process(pObj->hEncode, &inBufDesc, &outBufDesc, &inArgs, &outArgs);

  if (status != IMGENC1_EOK)
    return OSA_EFAIL;

  runStatus->bytesGenerated = outArgs.bytesGenerated;

  return OSA_SOK;
}

int ALG_jpgEncDelete(void *hndl)
{
  ALG_JpgEncObj *pObj=(ALG_JpgEncObj *)hndl;

  if(pObj==NULL)
    return OSA_EFAIL;

  if(pObj->hEncode)
    IMGENC1_delete(pObj->hEncode);

  OSA_memFree(pObj);

  return OSA_SOK;
}

int SnapshotFrameData(char *fileName, Uint8 *Yaddr, Uint8 *UVaddr, Uint32 width, Uint32 height, Uint32 pitch)
{
  IMGENC1_Handle   		hEncode;
  IMGENC1_Params  		params;
  IMGENC1_DynamicParams dynamicParams;
  IMGENC1_Status  		encStatus;
  IMGENC1_InArgs  		inArgs;
  IMGENC1_OutArgs 		outArgs;
  XDM1_BufDesc    		inBufDesc;
  XDM1_BufDesc    		outBufDesc;
  XDAS_Int32 			status;

  params.size 				= sizeof(IMGENC1_Params);
  params.maxWidth  			= pitch;
  params.maxHeight 			= height;
  params.maxScans  			= 1;
  params.dataEndianness 	= XDM_BYTE;
  params.forceChromaFormat 	= XDM_YUV_420P;

  hEncode = IMGENC1_create(gALG_hEngine, ALG_VID_CODEC_JPEG_ENC_NAME, &params);
  if (hEncode == NULL) {
    OSA_ERROR("Failed to open snapshot algorithm: %s (0x%x)\n", ALG_VID_CODEC_JPEG_ENC_NAME, Engine_getLastError(gALG_hEngine));
    return OSA_EFAIL;
  }

  dynamicParams.size 				= sizeof(IMGENC1_DynamicParams);
  dynamicParams.numAU 				= XDM_DEFAULT;
  dynamicParams.inputChromaFormat  	= XDM_YUV_420SP;
  dynamicParams.inputWidth      	= width;
  dynamicParams.inputHeight     	= height;
  dynamicParams.captureWidth    	= pitch;
  dynamicParams.generateHeader  	= XDM_ENCODE_AU;
  dynamicParams.qValue          	= ALG_JPG_ENC_SNAPSHOT_QVAL;

  /* Set video encoder dynamic parameters */
  encStatus.size 		= sizeof(IMGENC1_Status);
  encStatus.data.buf 	= NULL;

  status = IMGENC1_control(hEncode, XDM_SETPARAMS, &dynamicParams, &encStatus);
  if (status != IMGENC1_EOK) {
    OSA_ERROR("XDM_SETPARAMS failed, status=%ld\n", status);
  	IMGENC1_delete(hEncode);
    return OSA_EFAIL;
  }

  encStatus.data.buf = NULL;

  status = IMGENC1_control(hEncode, XDM_GETBUFINFO, &dynamicParams, &encStatus);
  if (status != IMGENC1_EOK) {
    OSA_ERROR("XDM_GETBUFINFO failed, status=%ld\n", status);
  	IMGENC1_delete(hEncode);
    return OSA_EFAIL;
  }

  inBufDesc.numBufs       		= 2;

  inBufDesc.descs[0].bufSize    = pitch*height;
  inBufDesc.descs[0].buf        = (XDAS_Int8*)Yaddr;
  inBufDesc.descs[0].accessMask = 0;

  inBufDesc.descs[1].bufSize    = pitch*height/2;
  inBufDesc.descs[1].buf        = (XDAS_Int8*)UVaddr;
  inBufDesc.descs[1].accessMask = 0;

  outBufDesc.numBufs           		= 1;
  outBufDesc.descs[0].buf       	= (XDAS_Int8*)OSA_cmemAlloc((pitch*height), 32);
  outBufDesc.descs[0].bufSize   	= pitch*height;
  outBufDesc.descs[0].accessMask	= 0;

  if((inBufDesc.descs[0].buf==NULL)||(inBufDesc.descs[1].buf==NULL)||(outBufDesc.descs[0].buf == NULL)){
     OSA_ERROR("Snapshot Mode: XDM1_BufDesc failed\n");
  	 IMGENC1_delete(hEncode);
     if(outBufDesc.descs[0].buf!=NULL)
  		OSA_cmemFree(outBufDesc.descs[0].buf);

     return OSA_EFAIL;
  }

  inArgs.size	= sizeof(IMGENC1_InArgs);
  outArgs.size 	= sizeof(IMGENC1_OutArgs);

  status = IMGENC1_process(hEncode, &inBufDesc, &outBufDesc, &inArgs, &outArgs);
  if (status != IMGENC1_EOK)
    return OSA_EFAIL;

  OSA_fileWriteFile(fileName, outBufDesc.descs[0].buf, outArgs.bytesGenerated);

  OSA_cmemFree(outBufDesc.descs[0].buf);

  IMGENC1_delete(hEncode);

  return OSA_SOK;
}

