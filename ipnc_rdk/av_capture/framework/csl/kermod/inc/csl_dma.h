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

#ifndef _CSL_DMA_H
#define _CSL_DMA_H

#include <csl_soc.h>

#define CSL_DMA_IPIPE_BSC     0
#define CSL_DMA_IPIPE_HISTO   1
#define CSL_DMA_MAX           1 //2 - For DMVAx there is shortage of EDMA channels
                                //    Therefore reducing CSL DMA channels by 1

int CSL_dmaInit(void);
int CSL_dmaExit(void);
int CSL_dmaStart(int id);
int CSL_dmaStop(int id);
int CSL_dmaSetDstAddr(int id, Uint32 dstPhysAddr);
int CSL_dmaSetPrm(int id, Uint32 dstPhysAddr, Uint32 srcPhysAddr, Uint16 aCnt, Uint16 bCnt);
int CSL_dmaPrintCount(int id);
int CSL_dmaClearCount(int id);

#endif
