
#include <avserver.h>
#include <alg_aes.h>

#define DATASIZE_PER_MULTI_PASS 	(16000)

#define DECRYPTION_ENABLE

#ifdef DECRYPTION_ENABLE
	Uint8 *gEncryptInterBuf;
#endif

Int32 VIDEO_encryptTskCreate(Uint32 sysRegBase)	
{	
	Uint8 k;
	ALG_aesParams aesParams;
	ALG_AesCreate createPrm;
	Uint8 KEYs[32] =
	{
	    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
	};	
		
	aesParams.AES_bigEndian = 0;	/*  AES_bigEndian... 0: Little Endian, 1: Big Endian, key is always little endian */
	aesParams.AES_keySize   = 0;	/*  AES_keySize... 0: 128 bit, 1: 192 bit, 2:256 bit */
	aesParams.AES_encrypt   = 1;	/*  AES_encrypt... 1: Encrypt, 0: Decrypt     */
	aesParams.q_num         = 3;	/*  EDMA Queue Number     */
	
	for(k = 0;k < 32;k++) 
	{
	    aesParams.roundKey[k] = KEYs[k];
	}	
	
	createPrm.dataSize    = DATASIZE_PER_MULTI_PASS;
	createPrm.pAesParams  = &aesParams;
	createPrm.askIMCOPRes = 0;
	createPrm.sysBaseAddr = sysRegBase; 

	/* Create Encryption instance */
	gVIDEO_ctrl.algEncryptHndl = ALG_aesCreate(&createPrm);
	
	if(gVIDEO_ctrl.algEncryptHndl == NULL)
	{
		OSA_ERROR("ALG_aesCreate for Encryption\n");
		return OSA_EFAIL;
	}
	
#ifdef DECRYPTION_ENABLE	
	/* Create Decryption instance */
	createPrm.pAesParams->AES_encrypt = 0;   
	gVIDEO_ctrl.algDecryptHndl= ALG_aesCreate(&createPrm);
	
	if(gVIDEO_ctrl.algEncryptHndl == NULL)
	{
		OSA_ERROR("ALG_aesCreate for Decryption\n");
		return OSA_EFAIL;
	}	
	
	/* Allocate memory for intermediate buffer */
	gEncryptInterBuf = OSA_cmemAlloc(DATASIZE_PER_MULTI_PASS,32);
	if(gEncryptInterBuf == NULL)
	{
		OSA_ERROR("OSA_cmemAlloc for gEncryptInterBuf\n");
		return OSA_EFAIL;	
	}
#endif

	return OSA_SOK;
}

Int32 VIDEO_encryptTskDelete()
{  
    /* Delete Encryption instance */
	ALG_aesDelete(gVIDEO_ctrl.algEncryptHndl);
	
#ifdef DECRYPTION_ENABLE		
	/* Delete Encryption Instance */
    ALG_aesDelete(gVIDEO_ctrl.algDecryptHndl);
	
	/* Free intermediate buffer */
	if(gEncryptInterBuf)
		OSA_cmemFree(gEncryptInterBuf);
#endif	
    
	return OSA_SOK;
}

Int32 VIDEO_encryptTskRun(Int32 streamId)
{
	Int32 status=OSA_EFAIL, inBufId, outBufId;
	OSA_BufInfo *pInBufInfo, *pOutBufInfo;
	VIDEO_BufHeader *pInBufHeader, *pOutBufHeader;
	ALG_AesRunPrm runPrm;
	ALG_AesStatus runStatus;
	Uint8  *inaddr, *outaddr;
	Uint32 numPasses;
	Uint32 i;
		  
	pInBufInfo = AVSERVER_bufGetFull( VIDEO_TSK_ENCRYPT, streamId, &inBufId, OSA_TIMEOUT_FOREVER);
  
	OSA_assert(pInBufInfo != NULL);
  
	if(pInBufInfo!=NULL) {
  
		pOutBufInfo = AVSERVER_bufGetEmpty( VIDEO_TSK_ENCRYPT, streamId, &outBufId, OSA_TIMEOUT_FOREVER);  
	
		OSA_assert(pOutBufInfo != NULL);    

		if(pOutBufInfo!=NULL) {
  
			pInBufHeader  = (VIDEO_BufHeader*)pInBufInfo->virtAddr;
			pOutBufHeader = (VIDEO_BufHeader*)pOutBufInfo->virtAddr;
      
			OSA_assert(pInBufHeader != NULL);
			OSA_assert(pOutBufHeader != NULL);                          

			if(pInBufHeader->encFrameSize > 0) {
      
				numPasses = (pInBufHeader->encFrameSize)/DATASIZE_PER_MULTI_PASS;
				if(pInBufHeader->encFrameSize%DATASIZE_PER_MULTI_PASS)
					numPasses ++; 
						
				inaddr  = pInBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
				outaddr = pOutBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE;
				
				for(i = 0; i < numPasses; i++)
				{			
					runPrm.inAddr  = inaddr;
					runPrm.outAddr = outaddr;
			
#ifdef DECRYPTION_ENABLE
					runPrm.outAddr = gEncryptInterBuf;			
#endif									
					OSA_prfBegin(&gAVSERVER_ctrl.encryptPrf[streamId]);
					status = ALG_aesRun(gVIDEO_ctrl.algEncryptHndl, &runPrm, &runStatus);
					OSA_prfEnd(&gAVSERVER_ctrl.encryptPrf[streamId], 1);  

					if(status != OSA_SOK) {
						OSA_ERROR("ALG_encryptRun(%d)\n", streamId);
						pOutBufHeader->encFrameSize = 0;          
					}

#ifdef DECRYPTION_ENABLE		
					runPrm.inAddr  = gEncryptInterBuf;
					runPrm.outAddr = outaddr;
					
					OSA_prfBegin(&gAVSERVER_ctrl.decryptPrf[streamId]);
					status = ALG_aesRun(gVIDEO_ctrl.algDecryptHndl, &runPrm, &runStatus); 
					OSA_prfEnd(&gAVSERVER_ctrl.decryptPrf[streamId], 1);  

					if(status != OSA_SOK) {
						OSA_ERROR("ALG_encryptRun(%d)\n", streamId);
						pOutBufHeader->encFrameSize = 0;          
					}
			
					if(memcmp(inaddr,outaddr,DATASIZE_PER_MULTI_PASS) != 0)
					{
						OSA_ERROR("Input and Decrypted data don't match for stream %d,frame size = %u\n",streamId,pInBufHeader->encFrameSize);			
					}			
#endif			
					inaddr  += DATASIZE_PER_MULTI_PASS;
					outaddr += DATASIZE_PER_MULTI_PASS;
				}			
			}      

			memcpy(pOutBufHeader, pInBufHeader, sizeof(*pOutBufHeader));
      
			AVSERVER_bufPutFull( VIDEO_TSK_ENCRYPT, streamId, outBufId);          
		}

		AVSERVER_bufPutEmpty( VIDEO_TSK_ENCRYPT, streamId, inBufId);  
	}
  
  return status;
}

Int32 VIDEO_encryptTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
	Int32 status = OSA_SOK, streamId;
	Bool done = FALSE, ackMsg = FALSE;
	Uint16 cmd = OSA_msgGetCmd(pMsg);
	Uint32 sysRegBase;
  
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
	OSA_printf(" ENCRYPT: Recevied CMD = 0x%04x\n", cmd);
#endif  
  
	if(cmd != AVSERVER_CMD_CREATE) {
	    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
		return OSA_SOK;
	}

	DM365MM_init();
	sysRegBase = DM365MM_mmap(0x01C40000,0x4000); 	
	
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
	OSA_printf(" ENCRYPT: Create\n");
#endif		
	
	status = VIDEO_encryptTskCreate(sysRegBase);

	OSA_tskAckOrFreeMsg(pMsg, status);  
	
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
	OSA_printf(" ENCRYPT: Create...DONE\n");
#endif	
    
	while(!done) {
    
		status = OSA_tskWaitMsg(pTsk, &pMsg);
    
		if(status != OSA_SOK) 
			break;

		cmd = OSA_msgGetCmd(pMsg);
   
		switch(cmd) {
			case AVSERVER_CMD_DELETE:
				done = TRUE;
				ackMsg = TRUE;
				break;

			case AVSERVER_CMD_NEW_DATA:
				streamId = (int)OSA_msgGetPrm(pMsg);
        
				OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        
				VIDEO_encryptTskRun(streamId); 
		
				break;
			default:
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
				OSA_printf(" ENCRYPT: Unknown CMD = 0x%04x\n", cmd);
#endif  
      
				OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);      
				break;
		}
	}
  
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
	OSA_printf(" ENCRYPT: Delete...\n");
#endif  
  
	VIDEO_encryptTskDelete();
    
	DM365MM_ummap(sysRegBase,0x4000);
	DM365MM_exit();    	
	
	if(ackMsg)
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
		
#ifdef AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
	OSA_printf(" ENCRYPT: Delete...DONE\n");
#endif  

	return OSA_SOK;
}

Int32 VIDEO_encryptCreate()
{
	Int32 status;
  
	status = OSA_tskCreate( &gVIDEO_ctrl.encryptTsk, VIDEO_encryptTskMain, VIDEO_ENCRYPT_THR_PRI, VIDEO_ENCRYPT_STACK_SIZE, 0);
	if(status!=OSA_SOK) {
		OSA_ERROR("OSA_tskCreate()\n");
		return status;
	}
  
	return status;
}

Int32 VIDEO_encryptDelete()
{
	Int32 status;
  
	status = OSA_tskDelete( &gVIDEO_ctrl.encryptTsk );
  
	return status;
}
