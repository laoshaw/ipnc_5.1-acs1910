/*
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <csl_dma.h>
#include <csl_sysDrv.h>
#include <mach/edma.h>

#define DMA_PARAM_NO_LINK          (0xFFFF)
#define DMA_PARAM_OPT_SYNCDIM_AB   (1<<2)
#define DMA_PARAM_OPT_TCINTEN      (1<<20)

CSL_Status CSL_ipipeBscBufSwitch(CSL_IpipeHandle hndl);

typedef void (*CSL_DmaCallback)(int, u16, void*);

typedef struct {

  int chId;
  struct edmacc_param dmaParam;
  CSL_DmaCallback callback;
  int countDmaStart;
  int countDmaComplete;

} CSL_DmaObj;

CSL_DmaObj gCSL_dmaObj[CSL_DMA_MAX];

static void CSL_dmaBscCallback(int channel, u16 ch_status, void *data)
{
  edma_stop( channel );

  gCSL_dmaObj[CSL_DMA_IPIPE_BSC].countDmaComplete++;

  CSL_ipipeBscBufSwitch(&gCSL_drvIpipeObj);
}

static void CSL_dmaHistoCallback(int channel, u16 ch_status, void *data)
{
  edma_stop( channel );
}


int CSL_dmaCreate(int id)
{
  int tcc;
  CSL_DmaObj *pObj;
  Uint32 srcPhysAddr;
  Uint32 size;


  if(id<0 || id>=CSL_DMA_MAX)
    return -1;

  pObj = &gCSL_dmaObj[id];

  memset(pObj, 0, sizeof(*pObj));

  switch(id) {
    case CSL_DMA_IPIPE_BSC:
      pObj->callback = CSL_dmaBscCallback;
      size = 0x4000;
      srcPhysAddr = CSL_IPIPE_BSC_TBL_0_PHYS_ADDR;
      break;
    case CSL_DMA_IPIPE_HISTO:
      pObj->callback = CSL_dmaHistoCallback;
      size = 0x2000;
      srcPhysAddr = CSL_IPIPE_HST_TBL_0_PHYS_ADDR;
      break;
    default:
      return -1;
  }

  pObj->countDmaComplete = 0;
  pObj->countDmaStart    = 0;


  pObj->chId=14;
  pObj->chId = edma_alloc_channel(EDMA_CHANNEL_ANY, pObj->callback, pObj, EVENTQ_1);
  if (pObj->chId < 0)
  {
    printk( KERN_ERR "Unable to request DMA (channel=%d).\n", pObj->chId );
    return -EFAULT;
  }

  CSL_dmaSetPrm(id, 0, srcPhysAddr, 4, size/4);

  return 0;
}

int CSL_dmaDelete(int id)
{
  if(id>=CSL_DMA_MAX)
    return -1;

  edma_stop( gCSL_dmaObj[id].chId );
  edma_free_channel( gCSL_dmaObj[id].chId );

  return 0;
}

int CSL_dmaInit()
{
  int i;
  int status=0;

  for(i=0; i<CSL_DMA_MAX; i++)
    status |= CSL_dmaCreate(i);

  return status;
}

int CSL_dmaExit()
{
  int i;
  int status=0;

  for(i=0; i<CSL_DMA_MAX; i++)
    status |= CSL_dmaDelete(i);

  return status;
}

int CSL_dmaSetDstAddr(int id, Uint32 dstPhysAddr)
{
  if(id>=CSL_DMA_MAX)
    return -1;

  gCSL_dmaObj[id].dmaParam.dst = (uint32_t) dstPhysAddr;

  edma_write_slot( gCSL_dmaObj[id].chId, &gCSL_dmaObj[id].dmaParam );

  return 0;
}

int CSL_dmaStart(int id)
{
  if(id>=CSL_DMA_MAX)
    return -1;

  gCSL_dmaObj[id].countDmaStart++;

  edma_start(gCSL_dmaObj[id].chId);

  return 0;
}

int CSL_dmaPrintCount(int id)
{
  printk( KERN_INFO " CSL DMA: CH %d: start=%d, complete=%d\n", gCSL_dmaObj[id].chId, gCSL_dmaObj[id].countDmaStart, gCSL_dmaObj[id].countDmaComplete);

  return 0;
}

int CSL_dmaClearCount(int id)
{
  gCSL_dmaObj[id].countDmaComplete = 0;
  gCSL_dmaObj[id].countDmaStart    = 0;

  return 0;
}

int CSL_dmaStop(int id)
{
  if(id>=CSL_DMA_MAX)
    return -1;

  edma_stop(gCSL_dmaObj[id].chId);

  return 0;
}

int CSL_dmaSetPrm(int id, Uint32 dstPhysAddr, Uint32 srcPhysAddr, Uint16 aCnt, Uint16 bCnt)
{
  if(id>=CSL_DMA_MAX)
    return -1;

  gCSL_dmaObj[id].dmaParam.src = (uint32_t) srcPhysAddr;
  gCSL_dmaObj[id].dmaParam.dst = (uint32_t) dstPhysAddr;
  gCSL_dmaObj[id].dmaParam.opt = DMA_PARAM_OPT_TCINTEN | DMA_PARAM_OPT_SYNCDIM_AB | ((gCSL_dmaObj[id].chId&0x3f)<<12);
  gCSL_dmaObj[id].dmaParam.link_bcntrld = DMA_PARAM_NO_LINK;
  gCSL_dmaObj[id].dmaParam.src_dst_cidx = 0;
  gCSL_dmaObj[id].dmaParam.ccnt = 1;
  gCSL_dmaObj[id].dmaParam.src_dst_bidx = (aCnt<<16) | (aCnt);
  gCSL_dmaObj[id].dmaParam.a_b_cnt = (uint32_t)(bCnt<<16)|aCnt;

  edma_write_slot( gCSL_dmaObj[id].chId, &gCSL_dmaObj[id].dmaParam );

  return 0;
}

