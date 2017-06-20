#include <alg_priv.h>
#include <alg_aes.h>
#include <osa_cmem.h>
#include <osa_file.h>

#include <ti/sdo/codecs/aes/iaes.h>

typedef struct {

  VIDENC1_Handle hEncode;
  ALG_AesCreate createPrm;
  char    algName[20];

  VIDENC1_Status encStatus;
  VIDENC1_Params params;
  IAES_Params extParams;
  IAES_DynamicParams extDynParams;

} ALG_AesObj;

void *ALG_aesCreate(ALG_AesCreate *create)
{
	ALG_AesObj *pObj;
	unsigned char KEYs[32] =
	{
	    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
	};

	int k;

	if (gALG_hEngine == NULL)
		return NULL;

	pObj = OSA_memAlloc(sizeof(ALG_AesObj));
	if (pObj == NULL)
		return NULL;

	memset(pObj, 0, sizeof(ALG_AesObj));

	strcpy(pObj->algName, ALG_VID_ADV_FEATURE_AES_NAME);

	memcpy(&pObj->createPrm, create, sizeof(pObj->createPrm));

	pObj->params.size      = sizeof(IAES_Params);
	pObj->params.maxWidth  = create->dataSize;
	pObj->extDynParams.videncDynamicParams.inputWidth  = pObj->params.maxWidth;

	// Set the create time extended parameters
	pObj->extParams.videncParams = pObj->params;

	if(create->pAesParams==NULL) {

		pObj->extParams.AES_bigEndian = 1;	/*  AES_bigEndian... 0: Little Endian, 1: Big Endian, key is always little endian */
		pObj->extParams.AES_keySize   = 0;	/*  AES_keySize... 0: 128 bit, 1: 192 bit, 2:256 bit */
		pObj->extParams.AES_encrypt   = 1;	/*  AES_encrypt... 1: Encrypt, 0: Decrypt     */
		pObj->extParams.q_num         = 3;	/*  EDMA Queue Number     */
		pObj->extParams.askIMCOPRes   = 1;

		for(k = 0;k < 32;k++) {
			pObj->extParams.roundKey[k]     = KEYs[k];
		}
	}
	else {

		pObj->extParams.AES_bigEndian   = create->pAesParams->AES_bigEndian;
		pObj->extParams.AES_keySize     = create->pAesParams->AES_keySize;
		pObj->extParams.AES_encrypt     = create->pAesParams->AES_encrypt;
		pObj->extParams.q_num           = create->pAesParams->q_num;
		pObj->extParams.askIMCOPRes     = create->askIMCOPRes;

		for(k = 0;k < 32;k++) {
			pObj->extParams.roundKey[k]     = create->pAesParams->roundKey[k];
		}
	}

	pObj->extParams.sysBaseAddr = create->sysBaseAddr;

	/* Create video encoder instance */
	pObj->hEncode = VIDENC1_create(gALG_hEngine, pObj->algName, (VIDENC1_Params *) & pObj->extParams);

	if (pObj->hEncode == NULL)
	{
		OSA_ERROR("Failed to open video encode algorithm: %s (0x%x)\n", pObj->algName, Engine_getLastError(gALG_hEngine));
		OSA_memFree(pObj);
		return NULL;
	}
	else
	{
		OSA_printf("\n Exit Create successfully  \n");
	}

	return pObj;
}

Int32 ALG_aesRun(void *hndl, ALG_AesRunPrm * prm, ALG_AesStatus * runStatus)
{
	ALG_AesObj *pObj = (ALG_AesObj *) hndl;
	IVIDEO1_BufDescIn inBufDesc;
	XDM_BufDesc outBufDesc;
	XDAS_Int32 status;
	VIDENC1_InArgs inArgs;
	IAES_OutArgs outArgs;
	XDAS_Int8 *outbufs[2];
	XDAS_Int32 outBufSizeArray[2], dataSize;

	if (pObj == NULL)
		return OSA_EFAIL;

	dataSize = pObj->createPrm.dataSize;

	/* Input buffers */
	inBufDesc.numBufs = 1;
	inBufDesc.bufDesc[0].bufSize = dataSize;
	inBufDesc.bufDesc[0].buf = (XDAS_Int8 *) (prm->inAddr);
	inBufDesc.bufDesc[0].accessMask = 0;

	/* Output buffers */
	outBufDesc.numBufs = 1;
	outbufs[0] = (XDAS_Int8*)prm->outAddr;
	outBufSizeArray[0]  = dataSize;
	outBufDesc.bufSizes = outBufSizeArray;
	outBufDesc.bufs     = outbufs;

	inArgs.size = sizeof(VIDENC1_InArgs);
	outArgs.videncOutArgs.size = sizeof(IAES_OutArgs);

	OSA_mutexLock(&vnf_imcop_lock);
	OSA_mutexLock(&vicp_imcop_lock);

	/* Set Dynamic Params */

	/* Call the control function */
	status = VIDENC1_control((VIDENC1_Handle)pObj->hEncode, XDM_SETPARAMS, (IVIDENC1_DynamicParams *)&pObj->extDynParams, (IVIDENC1_Status *)&status);
	if (status != VIDENC1_EOK)
	{
		OSA_ERROR(" ALG:AES: Dynamic Parameter updation failed !!!\n");
		return OSA_EFAIL;
	}

	status = VIDENC1_process(pObj->hEncode, &inBufDesc, &outBufDesc, &inArgs, (IVIDENC1_OutArgs *) & outArgs);

	OSA_mutexUnlock(&vicp_imcop_lock);
	OSA_mutexUnlock(&vnf_imcop_lock);

	runStatus->outDataSize  = outArgs.outDataSize;
	if (status != VIDENC1_EOK)
	{
		return OSA_EFAIL;
	}

	return OSA_SOK;
}

Int32 ALG_aesDelete(void *hndl)
{
	ALG_AesObj *pObj = (ALG_AesObj *) hndl;

	if (pObj == NULL)
		return OSA_EFAIL;

	if (pObj->hEncode)
		VIDENC1_delete(pObj->hEncode);

	OSA_memFree(pObj);

	return OSA_SOK;
}

