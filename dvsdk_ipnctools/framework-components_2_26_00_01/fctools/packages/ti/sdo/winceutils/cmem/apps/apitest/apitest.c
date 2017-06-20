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
 * apitest.c
 *
 * Tests basic API usage and memory allocation. Inserting CMEM with the
 * following options works on the DVEVM (if mem=120M):
 *
 * insmod cmemk.ko phys_start=0x87800000 phys_end=0x87F00000 pools=4xBUFSIZE phys_start_1=0x87F00000 phys_end_1=0x88000000 pools_1=4xBUFSIZE
 *
 * where BUFSIZE is the number of bytes you plan on passing as command
 * line parameter to apitest. If in doubt, use a larger number as BUFSIZE
 * denotes the maximum buffer you can allocate.
 */

#include <windows.h>
#include <kfuncs.h>

#include <ti/sdo/winceutils/cmem/cmem.h>

#define __D(fmt, ...) RETAILMSG(1, (L"CMEM Debug: " L##fmt, __VA_ARGS__))

#define NUMHEAPPTRS 0x1000
unsigned int *heap_ptrs[NUMHEAPPTRS];

int ARGC = 2;
char *ARGV[] = {
    "apitest_wince.exe",
    "1024"
};


void testHeap(int size, int block)
{
    int rv;
    int num_buffers;
    int i;
    CMEM_AllocParams params;

    __D("allocating heap buffers from CMEM memory block %d...\n", block);
    num_buffers = 0;
    params.type = CMEM_HEAP;
    params.flags = CMEM_NONCACHED;
    params.alignment = 0;
    while (num_buffers < NUMHEAPPTRS) {
        heap_ptrs[num_buffers] = CMEM_alloc2(block, size, &params);
        if (heap_ptrs[num_buffers] == NULL) {
            break;
        }
        num_buffers++;
    }
    __D("...done, %d allocated\n", num_buffers);

    __D("Press ENTER to continue (after 'cat /proc/cmem' if desired).\n");
    getchar();

    __D("freeing heap blocks...\n");
    for (i = 0; i < num_buffers; i++) {
        rv = CMEM_free(heap_ptrs[i], &params);
        if (rv < 0) {
            __D("error freeing blocks\n");
            break;
        }
        heap_ptrs[i] = NULL;
    }

    /* make sure we can still allocate num_buffers after freeing */
    __D("allocating %d heap blocks...\n", num_buffers);
    for (i = 0; i < num_buffers; i++) {
        heap_ptrs[i] = CMEM_alloc2(block, size, &params);
        if (heap_ptrs[i] == NULL) {
            __D("error re-allocating %d heap blocks\n", num_buffers);
            break;
        }
    }
    __D("...done, freeing heap blocks...\n");
    for (i = 0; i < num_buffers; i++) {
        rv = CMEM_free(heap_ptrs[i], &params);
        if (rv < 0) {
            __D("error freeing blocks\n");
            break;
        }
    }

    __D("...done\n");
}

void testCache(int size, int block)
{
    unsigned int *ptr1_nocache = NULL;
    unsigned int *ptr1_cache = NULL;
    unsigned int *ptr1_dma = NULL;
    unsigned int *ptr2 = NULL;
    unsigned long physp;
    unsigned long physp_dma;
    unsigned long physp_nocache;
    unsigned long physp_cache;
    int poolid;
    int i, j;
//    struct timeval start_tv, end_tv;
//    unsigned long diff;
    int foo, bar;
    CMEM_AllocParams params;

    __D("Allocating first noncached buffer.\n");

    /* First allocate a buffer from the pool that best fits */
    ptr1_nocache = CMEM_alloc(size, NULL);

    if (ptr1_nocache == NULL) {
        __D("Failed to allocate buffer of size %d\n", size);
        goto cleanup;
    }

    __D("Allocated buffer of size %d at address %#x.\n", size,
           (unsigned int) ptr1_nocache);

    /* Find out and print the physical address of this buffer */
    physp_nocache = CMEM_getPhys(ptr1_nocache);

    if (physp_nocache == 0) {
        __D("Failed to get physical address of buffer %#x\n",
                (unsigned int) ptr1_nocache);
        goto cleanup;
    }

    __D("Physical address of allocated buffer is %#x.\n",
           (unsigned int) physp_nocache);

    /* Write some data into this buffer */
    for (i=0; i < size / sizeof(int) ; i++) {
        ptr1_nocache[i] = 0xbeefbeef;
    }

    __D("Allocating first cached buffer.\n");

    /* First allocate a buffer from the pool that best fits */
    params = CMEM_DEFAULTPARAMS;
    params.flags = CMEM_CACHED;
    ptr1_cache = CMEM_alloc2(block, size, &params);

    if (ptr1_cache == NULL) {
        __D("Failed to allocate buffer of size %d\n", size);
        goto cleanup;
    }

    __D("Allocated buffer of size %d at address %#x.\n", size,
           (unsigned int) ptr1_cache);

    /* Find out and print the physical address of this buffer */
    physp_cache = CMEM_getPhys(ptr1_cache);

    if (physp_cache == 0) {
        __D("Failed to get physical address of buffer %#x\n",
                (unsigned int) ptr1_cache);
        goto cleanup;
    }

    __D("Physical address of allocated buffer is %#x.\n",
           (unsigned int) physp_cache);

    /* Write some data into this buffer */
    for (i = 0; i < size / sizeof(int); i++) {
        ptr1_cache[i] = 0x0dead1ce;
    }

    __D("Allocating noncached DMA source buffer.\n");

    /* Allocate a noncached buffer for the DMA source */
    ptr1_dma = CMEM_alloc(size, NULL);

    if (ptr1_dma == NULL) {
        __D("Failed to allocate buffer of size %d\n", size);
        goto cleanup;
    }

    __D("Allocated buffer of size %d at address %#x.\n", size,
           (unsigned int) ptr1_dma);

    /* Find out and print the physical address of this buffer */
    physp_dma = CMEM_getPhys(ptr1_dma);

    if (physp_dma == 0) {
        __D("Failed to get physical address of buffer %#x\n",
                (unsigned int) ptr1_dma);
        goto cleanup;
    }

    __D("Physical address of allocated buffer is %#x.\n",
           (unsigned int) physp_dma);

    /* Initialize DMA source buffer */
    for (i = 0; i < size / sizeof(int); i++) {
        ptr1_cache[i] = 0x0dead1ce;
    }

    /*
     * Measure the write performance of each buffer to check that one
     * is cached and the other isn't.
     */
    __D("Measuring R-M-W performance (cached should be quicker).\n");
    for (j = 0; j < 3; j++) {
        __D("R-M-W noncached buffer %lx\n", physp_nocache);
        //gettimeofday(&start_tv, NULL);
        for (i = 0; i < (size / sizeof(int)); i += 1) {
            ptr1_nocache[i] += 1;
        }
        //gettimeofday(&end_tv, NULL);
        //diff = end_tv.tv_usec - start_tv.tv_usec;
        //if (end_tv.tv_sec > start_tv.tv_sec) {
        //    diff += (end_tv.tv_sec - start_tv.tv_sec) * 1000000;
        //}
        //__D("  diff=%ld\n", diff);

        __D("R-M-W cached buffer %lx\n", physp_cache);
        //gettimeofday(&start_tv, NULL);
        for (i = 0; i < (size / sizeof(int)); i += 1) {
            ptr1_cache[i] += 1;
        }
        //gettimeofday(&end_tv, NULL);
        //diff = end_tv.tv_usec - start_tv.tv_usec;
        //if (end_tv.tv_sec > start_tv.tv_sec) {
        //    diff += (end_tv.tv_sec - start_tv.tv_sec) * 1000000;
        //}
        //__D("  diff=%ld\n", diff);
    }

    __D("Invalidate cached buffer %p\n", ptr1_cache);

    foo = *ptr1_cache;
    bar = foo;
    bar++;
    *ptr1_cache = bar;
    CMEM_cacheInv(ptr1_cache, size);
    __D("post-flush *ptr1_cache=0x%x\n", foo);
    __D("wrote 0x%x to *ptr1_cache\n", bar);
    __D("post-inv *ptr1_cache=0x%x\n", *ptr1_cache);

    __D("R-M-W cached buffer %lx\n", physp_cache);
    //gettimeofday(&start_tv, NULL);
    for (i = 0; i < (size / sizeof(int)); i += 1) {
        ptr1_cache[i] += 1;
    }
    //gettimeofday(&end_tv, NULL);
    //diff = end_tv.tv_usec - start_tv.tv_usec;
    //if (end_tv.tv_sec > start_tv.tv_sec) {
    //    diff += (end_tv.tv_sec - start_tv.tv_sec) * 1000000;
    //}
    //__D("  diff=%ld\n", diff);

    /*
     * Now allocate another buffer by first finding out which pool that fits
     * best, and then explicitly allocating from that pool. This gives more
     * control at the cost of an extra function call, but essentially does
     * the same thing as the above CMEM_alloc() call.
     */
    __D("Allocating second buffer.\n");

    poolid = CMEM_getPool(size);

    if (poolid == -1) {
        __D("Failed to get a pool which fits size %d\n", size);
        goto cleanup;
    }

    __D("Got a pool (%d) that fits the size %d\n", poolid, size);

    ptr2 = CMEM_allocPool(poolid, NULL);

    if (ptr2 == NULL) {
        __D("Failed to allocate buffer of size %d\n", size);
        goto cleanup;
    }

    __D("Allocated buffer of size %d at address %#x.\n", size,
           (unsigned int) ptr2);

    /* Find out and print the physical address of this buffer */
    physp = CMEM_getPhys(ptr2);

    if (physp == 0) {
        __D("Failed to get physical address of buffer %#x\n",
                (unsigned int) ptr2);
        goto cleanup;
    }

    __D("Physical address of allocated buffer is %#x.\n",
           (unsigned int) physp);

    /* Write some data into this buffer */
    for (i=0; i < size / sizeof(int); i++) {
        ptr2[i] = 0xfeebfeeb;
    }

    //__D("Inspect your memory map in /proc/%d/maps.\n", getpid());
    __D("Also look at your pool info under /proc/cmem\n");
    __D("Press ENTER to exit (after 'cat /proc/cmem' if desired).\n");
    getchar();

cleanup:
    if (ptr1_nocache != NULL) {
        if (CMEM_free(ptr1_nocache, NULL) < 0) {
            __D("Failed to free buffer at %#x\n",
                    (unsigned int) ptr1_nocache);
        }
        __D("Successfully freed buffer at %#x.\n",
               (unsigned int) ptr1_nocache);
    }

    if (ptr1_cache != NULL) {
        if (CMEM_free(ptr1_cache, &params) < 0) {
            __D("Failed to free buffer at %#x\n",
                    (unsigned int) ptr1_cache);
        }
        __D("Successfully freed buffer at %#x.\n",
               (unsigned int) ptr1_cache);
    }

    if (ptr1_dma != NULL) {
        if (CMEM_free(ptr1_dma, NULL) < 0) {
            __D("Failed to free buffer at %#x\n",
                    (unsigned int) ptr1_dma);
        }
        __D("Successfully freed buffer at %#x.\n",
               (unsigned int) ptr1_dma);
    }

    if (ptr2 != NULL) {
        if (CMEM_free(ptr2, NULL) < 0) {
            __D("Failed to free buffer at %#x\n",
                    (unsigned int) ptr2);
        }
        __D("Successfully freed buffer at %#x.\n",
               (unsigned int) ptr2);
    }
}

int main(int argc, char *argv[])
{
    int size;
    int version;
    CMEM_BlockAttrs attrs;

    DebugBreak();

    if (ARGC != 2) {
        __D("Usage: %s <Number of bytes to allocate>\n", ARGV[0]);
        exit(EXIT_FAILURE);
    }

    size = atoi(ARGV[1]);

    /* First initialize the CMEM module */
    if (CMEM_init() == -1) {
        __D("Failed to initialize CMEM\n");
        exit(EXIT_FAILURE);
    }

    __D("CMEM initialized.\n");

    version = CMEM_getVersion();
    if (version == -1) {
        __D("Failed to retrieve CMEM version\n");
    }
    __D("CMEM version = 0x%x\n", version);

    if (CMEM_getBlockAttrs(0, &attrs) == -1) {
        __D("Failed to retrieve CMEM memory block 0 bounds\n");
    }
    __D("CMEM memory block 0: phys start = 0x%lx, size = 0x%x\n",
           attrs.phys_base, attrs.size);

    if (CMEM_getBlockAttrs(1, &attrs) == -1) {
        __D("Failed to retrieve CMEM memory block 1 bounds\n");
    }
    __D("CMEM memory block 1: phys start = 0x%lx, size = 0x%x\n",
           attrs.phys_base, attrs.size);

    __D("main: Testing heap allocation in block 0...\n");
    testHeap(size, 0);
    __D("main: Testing heap allocation in block 1...\n");
    testHeap(size, 1);

    __D("main: Testing heap allocation with cache flags in block 0...\n");
    testCache(size, 0);
    __D("main: Testing heap allocation with cache flags in block 0...\n");
    testCache(size, 1);

    if (CMEM_exit() < 0) {
        __D("Failed to finalize the CMEM module\n");
    }

    exit(EXIT_SUCCESS);
}

/*
 *  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:11; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

 */

