/* 
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/*
 *  ======== cmem_test.c ========
 */

#include <windows.h>
#include <kfuncs.h>

#include <ti/sdo/winceutils/cmem/cmem.h>

#define __D(fmt, ...) RETAILMSG(1, (L"CMEM TEST Debug: " L##fmt, __VA_ARGS__))


/*
 *  ======== main ========
 */
void main()
{
    CMEM_BlockAttrs   blockAttrs;
    CMEM_AllocParams  allocParams;
    PVOID             heapPtr;
    PVOID             ptrPA;
    PVOID             poolBuf;
    int               numBlocks = 0;
    int               i;
    int               status;

    DebugBreak();

    /* Initialize CMEM driver */
    CMEM_init();

    /* Get the number of CMEM blocks */
    status = CMEM_getNumBlocks(&numBlocks);
    if (status != 0) {
        __D("CMEM_getNumBlocks failed!\n");
    }
    else {
        __D("Number of CMEM blocks: %#x.\n", (unsigned int)numBlocks);
    }

    for (i = 0; i < numBlocks; i++) {
        status = CMEM_getBlockAttrs(i, &blockAttrs);
        if (status != 0) {
            __D("CMEM_getBlockAttrs for block %#x failed!\n", (unsigned int)i);
        }
        else {
            __D("Block %#d address: %#x.\n", i,
                    (unsigned int)blockAttrs.phys_base);
            __D("Block %#d length: %#x.\n", i,
                    (unsigned int)blockAttrs.size);
        }
    }

    /* Allocate a buffer from CMEM pool */
    allocParams.type = CMEM_POOL;
    allocParams.flags = CMEM_NONCACHED;
    poolBuf = CMEM_allocPool(0, &allocParams);

    __D("Pool buffer address: %#x.\n", (unsigned int)poolBuf);

    /* Allocate buffer from CMEM heap */
    allocParams.type = CMEM_HEAP;
    heapPtr = CMEM_alloc(1024, &allocParams);

    __D("Heap buffer address: %#x.\n", (unsigned int)heapPtr);

    /*
     *  Determine physical addresses of buffer.
     */
    ptrPA = (PVOID)CMEM_getPhys(heapPtr);

    __D("ptrPA: 0x%x\n", ptrPA);

    CMEM_exit();
}
/*
 *  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:12; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

 */

