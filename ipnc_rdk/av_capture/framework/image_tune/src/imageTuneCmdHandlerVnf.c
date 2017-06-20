#include <imageTunePriv.h>

int IMAGE_TUNE_CmdExecuteVnfPrm(IMAGE_TUNE_CmdInfo *cmdInfo, IMAGE_TUNE_CmdInfo *statusInfo)
{
  Uint32 reqdPrmSize;
  Uint32 prmSize;
  Uint32 tableSize;
  ALG_vnfParams *pPrm = &gIMAGE_TUNE_ctrl.curPrm.vnfPrm;

  prmSize = sizeof(*pPrm);
  reqdPrmSize =  prmSize;

  if(cmdInfo->prmSize != reqdPrmSize ) {
    #ifdef IMAGE_TUNE_CMD_HANDLER_DEBUG
    OSA_printf(" IMAGE TUNE SERVER: Received parameter size (%d != %d) mismatch ERROR for CMD 0x%04x\n", cmdInfo->prmSize, reqdPrmSize, cmdInfo->commandId);
    #endif
    return OSA_EFAIL;
  }

  memcpy( pPrm, cmdInfo->prm, prmSize);

  return OSA_SOK;
}
int IMAGE_TUNE_CmdExecuteTnf3Prm(IMAGE_TUNE_CmdInfo *cmdInfo, IMAGE_TUNE_CmdInfo *statusInfo)
{
  Uint32 reqdPrmSize;
  Uint32 prmSize;
  Uint32 tableSize;
  ALG_tnf3Params *pPrm = &gIMAGE_TUNE_ctrl.curPrm.tnf3Prm;

  OSA_printf ("In the TNF3 command handler ...\n");
  prmSize = sizeof(*pPrm);
  reqdPrmSize =  prmSize;

  if(cmdInfo->prmSize != reqdPrmSize ) {
    #ifdef IMAGE_TUNE_CMD_HANDLER_DEBUG
    OSA_printf(" IMAGE TUNE SERVER: Received parameter size (%d != %d) mismatch ERROR for CMD 0x%04x\n", cmdInfo->prmSize, reqdPrmSize, cmdInfo->commandId);
    #endif
    return OSA_EFAIL;
  }

#ifdef IMAGE_TUNE_CMD_HANDLER_DEBUG
  OSA_printf("param [0]\n",cmdInfo->prm[0]);
  OSA_printf("param [1]\n",cmdInfo->prm[1]);
  OSA_printf("param [2]\n",cmdInfo->prm[2]);
  OSA_printf("param [3]\n",cmdInfo->prm[2]);
#endif

  memcpy( pPrm, cmdInfo->prm, prmSize);

  return OSA_SOK;
}

