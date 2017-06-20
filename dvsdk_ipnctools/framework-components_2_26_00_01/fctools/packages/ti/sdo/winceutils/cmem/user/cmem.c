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
 *  ======== cmem.c ========
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "../cmem.h"

CMEM_AllocParams CMEM_DEFAULTPARAMS = {
    CMEM_POOL,          /* type */
    CMEM_NONCACHED,     /* flags */
    1                   /* alignment */
};

#ifdef __DEBUG
#define __D(fmt, ...) RETAILMSG(1, (L"CMEM Debug: " L##fmt, __VA_ARGS__))
#else
#define __D(fmt, ...) 
#endif

#define __E(fmt, ...) RETAILMSG(1, (L"CMEM Error: " L##fmt, __VA_ARGS__))

#ifdef PAGE_ALIGN
#undef PAGE_ALIGN
#endif

#define PAGE_ALIGN(x) (((x) + dwPageSize - 1) & ~(dwPageSize - 1))
#define PAGE_MASK (~(dwPageSize - 1))

struct block_struct {
    unsigned long addr;
    size_t size;
};

/* CMEM driver handle */
static HANDLE hCMEM = NULL;

static int ref_count = 0;
static DWORD dwPageSize = 0;

static void *allocFromHeap(int blockid, size_t size, CMEM_AllocParams *params);
static void *allocFromPool(int blockid, int poolid, CMEM_AllocParams *params);

static int validate_init()
{
    if (hCMEM == NULL) {
        __E("Either CMEM_init() was not called, or failed. You must\n");
        __E("    initialize CMEM before making CMEM API calls.\n");
        __E("    If CMEM_init() was called, check for earlier CMEM\n");
        __E("    failure messages.\n");
        return (FALSE);
    }
    return (TRUE);
}

int CMEM_init(void)
{
    SYSTEM_INFO  systemInfo;
    unsigned int version;

    __D("init: entered - ref_count %d, hCMEM %d\n", ref_count, hCMEM);

    if (hCMEM) {
        ref_count++;
        __D("init: CMK1: already opened, incremented ref_count %d\n",
                ref_count);
        return (0);
    }

    hCMEM = CreateFile(L"CMK1:",
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (hCMEM == INVALID_HANDLE_VALUE) {
        __E("init: Failed to open CMK1: '%d'\n", GetLastError());
        hCMEM = NULL;
        return (-1);
    }

    GetSystemInfo(&systemInfo);
    dwPageSize = systemInfo.dwPageSize;

    ref_count++;

    __D("init: successfully opened CMK1:, matching driver version...\n");

    version = CMEM_getVersion();
    if ((version & 0xffff0000) != (CMEM_VERSION & 0xffff0000)) {
        __E("init: major version mismatch between interface and driver.\n");
        __E("    needs driver version %#x, got %#x\n", CMEM_VERSION, version);

        CMEM_exit();

        return (-1);
    }
    else if ((version & 0x0000ffff) < (CMEM_VERSION & 0x0000ffff)) {
        __E("init: minor version mismatch between interface and driver.\n");
        __E("    needs driver minor version %#x or greater.\n",
                CMEM_VERSION & 0x0000ffff);
        __E("    got minor version %#x (full version %#x)\n",
                version & 0x0000ffff, version);

        CMEM_exit();

        return (-1);
    }

    __D("init: ... match good (%#x)\n", version);
    __D("init: exiting, returning success\n");

    return (0);
}

static void *allocFromPool(int blockid, int poolid, CMEM_AllocParams *params)
{
    union CMEM_AllocUnion allocDesc;
    unsigned int cmd;
    BOOL  rv;
    DWORD dwBytesReturned;

    allocDesc.alloc_pool_inparams.poolid = poolid;
    allocDesc.alloc_pool_inparams.blockid = blockid;
    allocDesc.alloc_pool_inparams.dwProcId = (DWORD)GetCurrentProcessId();
    cmd = CMEM_IOCALLOC | params->flags;
    rv = DeviceIoControl(hCMEM, cmd, &allocDesc, sizeof(allocDesc), NULL, 0,
            &dwBytesReturned, NULL);
    if (!rv) {
        __E("allocPool: ioctl %s failed from pool %d: %d\n",
                cmd == CMEM_IOCALLOC ? "CMEM_IOCALLOC" : "CMEM_IOCALLOCCACHED",
                poolid, rv);
        return NULL;
    }

    __D("allocPool: mmap succeeded, returning virt buffer %p\n",
            allocDesc.alloc_pool_outparams.virtp);

    return (PVOID)allocDesc.alloc_pool_outparams.virtp;
}

static void *getAndAllocFromPool(int blockid, size_t size,
        CMEM_AllocParams *params)
{
    int poolid;

    poolid = CMEM_getPool2(blockid, size);

    if (poolid == -1) {
        return (NULL);
    }
    if (poolid == -2) {
        /*
         *  No pool was big enough, but the heap is, and we're allowed to
         *  use the heap if a pool allocation fails.
         */
        return (allocFromHeap(blockid, size, params));
    }
    else {
        return (allocFromPool(blockid, poolid, params));
    }
}

static void *allocFromHeap(int blockid, size_t size, CMEM_AllocParams *params)
{
    union CMEM_AllocUnion allocDesc;
    unsigned int cmd;
    BOOL rv;
    DWORD dwBytesReturned;

    cmd = CMEM_IOCALLOCHEAP | params->flags;
    allocDesc.alloc_heap_inparams.size = size;
    allocDesc.alloc_heap_inparams.align = params->alignment == 0 ?
        1 : params->alignment;
    allocDesc.alloc_heap_inparams.blockid = blockid;
    allocDesc.alloc_heap_inparams.dwProcId = (DWORD)GetCurrentProcessId();

    rv = DeviceIoControl(hCMEM, cmd, &allocDesc, sizeof(allocDesc), NULL, 0,
            &dwBytesReturned, NULL);
    if (!rv) {
        __E("allocHeap: ioctl %s failed: %d\n",
                cmd == CMEM_IOCALLOCHEAP ?
                "CMEM_IOCALLOCHEAP" : "CMEM_IOCALLOCHEAPCACHED",
                rv);
        return NULL;
    }

    __D("allocHeap: mmap succeeded, returning virt buffer %p\n",
            allocDesc.virtp);

    return (PVOID)allocDesc.virtp;
}

/*
 *  Single interface into all flavors of alloc.
 *  Need to support:
 *    - cached vs. noncached
 *    - heap vs. pool
 *    - alignment w/ heap allocs
 */
void *alloc(int blockid, size_t size, CMEM_AllocParams *params)
{
    if (params == NULL) {
        params = &CMEM_DEFAULTPARAMS;
    }

    __D("alloc: entered w/ size %#x, params - type %s, flags %s, align %#x %s\n",
            size,
            params->type == CMEM_POOL ? L"POOL" : L"HEAP",
            params->flags & CMEM_CACHED ? L"CACHED" : L"NONCACHED",
            params->alignment,
            params == &CMEM_DEFAULTPARAMS ? L" (default)" : L"");

    if (!validate_init()) {
        return NULL;
    }

    if (params->type == CMEM_POOL) {
        return getAndAllocFromPool(blockid, size, params);
    }
    else {
        return allocFromHeap(blockid, size, params);
    }
}

void *CMEM_alloc(size_t size, CMEM_AllocParams *params)
{
    return alloc(0, size, params);
}

void *CMEM_alloc2(int blockid, size_t size, CMEM_AllocParams *params)
{
    return alloc(blockid, size, params);
}

static void *allocPool(int blockid, int poolid, CMEM_AllocParams *params)
{
    if (params == NULL) {
        params = &CMEM_DEFAULTPARAMS;
    }

    __D("allocPool: entered w/ poolid %d, params - flags %s%s\n", poolid,
            params->flags & CMEM_CACHED ? L"CACHED" : L"NONCACHED",
            params == &CMEM_DEFAULTPARAMS ? L" (default)" : L"");

    if (!validate_init()) {
        return NULL;
    }

    return allocFromPool(blockid, poolid, params);
}

void *CMEM_allocPool(int poolid, CMEM_AllocParams *params)
{
    return allocPool(0, poolid, params);
}

void *CMEM_allocPool2(int blockid, int poolid, CMEM_AllocParams *params)
{
    return allocPool(blockid, poolid, params);
}

int CMEM_free(void *ptr, CMEM_AllocParams *params)
{
    union CMEM_AllocUnion freeDesc;
    unsigned int cmd;
    size_t size;
    DWORD dwBytesReturned;

    if (params == NULL) {
        params = &CMEM_DEFAULTPARAMS;
    }

    __D("free: entered w/ ptr %p, params - type %s%s\n",
            ptr,
            params->type == CMEM_POOL ? L"POOL" : L"HEAP",
            params == &CMEM_DEFAULTPARAMS ? L" (default)" : L"");

    if (!validate_init()) {
        return -1;
    }

    freeDesc.virtp = (int)ptr;
    cmd = CMEM_IOCFREE | params->type;
    if (!DeviceIoControl(hCMEM, cmd, &freeDesc, sizeof(freeDesc), NULL, 0,
                &dwBytesReturned, NULL)) {
        __E("free: failed to free %#x\n", (unsigned int) ptr);
        return -1;
    }
    size = freeDesc.free_outparams.size;

    __D("free: ioctl CMEM_IOCFREE%s succeeded, size %#x\n",
            params->type == CMEM_POOL ? "POOL" : "HEAP", size);

    __D("free: munmap succeeded, returning 0\n");

    return 0;
}

static int getPoolFromBlock(int blockid, size_t size)
{
    union CMEM_AllocUnion poolDesc; 
    DWORD dwBytesReturned;

    if (!validate_init()) {
        return -1;
    }

    poolDesc.get_pool_inparams.size = size;
    poolDesc.get_pool_inparams.blockid = blockid;
    if (!DeviceIoControl(hCMEM, CMEM_IOCGETPOOL, &poolDesc, sizeof(poolDesc),
                NULL, 0, &dwBytesReturned, NULL)) {
        __E("getPool: Failed to get a pool fitting a size %d\n", size);
        return -1;
    }

    __D("getPool: exiting, ioctl CMEM_IOCGETPOOL succeeded, returning %d\n",
            poolDesc.poolid);

    return poolDesc.poolid;
}

int CMEM_getPool(size_t size)
{
    __D("getPool: entered w/ size %#x\n", size);

    return getPoolFromBlock(0, size);
}

int CMEM_getPool2(int blockid, size_t size)
{
    __D("getPool2: entered w/ size %#x\n", size);

    return getPoolFromBlock(blockid, size);
}

unsigned long CMEM_getPhys(void *ptr)
{
    union CMEM_AllocUnion getDesc;
    DWORD dwBytesReturned;

    __D("getPhys: entered w/ addr %p\n", ptr);

    if (!validate_init()) {
        return 0;
    }

    getDesc.virtp = (unsigned long)ptr;
    if (!DeviceIoControl(hCMEM, CMEM_IOCGETPHYS, &getDesc, sizeof(getDesc),
                NULL, 0, &dwBytesReturned, NULL)) {
        __E("getPhys: Failed to get physical address of %#x\n",
                (unsigned int) ptr);
        return 0;
    }

    __D("getPhys: exiting, ioctl CMEM_IOCGETPHYS succeeded, returning %#lx\n",
            getDesc.physp);

    return getDesc.physp;
}

int CMEM_cacheWb(void *ptr, size_t size)
{
    struct block_struct block;
    DWORD  dwBytesReturned;

    __D("cacheWb: entered w/ addr %p, size %#x\n", ptr, size);

    if (!validate_init()) {
        return -1;
    }

    block.addr = (unsigned long)ptr;
    block.size = size;
    if (!DeviceIoControl(hCMEM, CMEM_IOCCACHEWB, &block, sizeof(block), NULL,
                0, &dwBytesReturned, NULL)) {
        __E("cacheWb: Failed to writeback %#x\n", (unsigned int) ptr);

        return -1;
    }

    __D("cacheWb: exiting, ioctl CMEM_IOCCACHEWB succeeded, returning 0\n");

    return 0;
}

int CMEM_cacheWbInv(void *ptr, size_t size)
{
    struct block_struct block;
    DWORD dwBytesReturned;

    __D("cacheWbInv: entered w/ addr %p, size %#x\n", ptr, size);

    if (!validate_init()) {
        return -1;
    }

    block.addr = (unsigned long)ptr;
    block.size = size;
    if (!DeviceIoControl(hCMEM, CMEM_IOCCACHEWBINV, &block, sizeof(block),
                NULL, 0, &dwBytesReturned, NULL)) {
        __E("cacheWbInv: Failed to writeback & invalidate %#x\n",
                (unsigned int) ptr);

        return -1;
    }

    __D("cacheWbInv: exiting, ioctl CMEM_IOCCACHEWBINV succeeded, returning 0\n");

    return 0;
}

int CMEM_cacheInv(void *ptr, size_t size)
{
    struct block_struct block;
    DWORD  dwBytesReturned;

    __D("cacheInv: entered w/ addr %p, size %#x\n", ptr, size);

    if (!validate_init()) {
        return -1;
    }

    block.addr = (unsigned long)ptr;
    block.size = size;
    if (!DeviceIoControl(hCMEM, CMEM_IOCCACHEINV, &block, sizeof(block), NULL,
                0, &dwBytesReturned, NULL)) {
        __E("cacheInv: Failed to invalidate %#x\n", (unsigned int) ptr);

        return -1;
    }

    __D("cacheInv: exiting, ioctl CMEM_IOCCACHEINV succeeded, returning 0\n");

    return 0;
}

int CMEM_getVersion(void)
{
    unsigned int version;
    DWORD dwBytesReturned;

    __D("getVersion: entered\n");

    if (!validate_init()) {
        return -1;
    }

    if (!DeviceIoControl(hCMEM, CMEM_IOCGETVERSION, &version, sizeof(version),
                NULL, 0, &dwBytesReturned, NULL)) {
        __E("getVersion: Failed to retrieve version from driver\n");

        return -1;
    }

    __D("getVersion: exiting, ioctl CMEM_IOCGETVERSION returned %#x\n",
            version);

    return version;
}

static int getBlock(int blockid, unsigned long *pphys_base, size_t *psize)
{
    union CMEM_AllocUnion block;
    DWORD dwBytesReturned;

    __D("getBlock: entered\n");

    if (!validate_init()) {
        return -1;
    }

    block.blockid = blockid;
    if (!DeviceIoControl(hCMEM, CMEM_IOCGETBLOCK, &block, sizeof(block), NULL,
                0, &dwBytesReturned, NULL)) {
        __E("getBlock: Failed to retrieve memory block bounds for\n");
        __E("  block %d from driver\n", blockid);

        return -1;
    }

    *pphys_base = block.get_block_outparams.physp;
    *psize = block.get_block_outparams.size;

    __D("getVersion: exiting, ioctl CMEM_IOCGETBLOCK succeeded,\n");
    __D("  returning *pphys_base=0x%lx, *psize=0x%x\n", *pphys_base, *psize);

    return 0;
}

int CMEM_getBlock(unsigned long *pphys_base, size_t *psize)
{
    return getBlock(0, pphys_base, psize);
}

int CMEM_getBlockAttrs(int blockid, CMEM_BlockAttrs *pattrs)
{
    return getBlock(blockid, &pattrs->phys_base, &pattrs->size);
}

int CMEM_getNumBlocks(int *pnblocks)
{
    unsigned int numBlocks = 0;
    DWORD dwBytesReturned;

    __D("CMEM_getNumBlocks: entered\n");

    if (!validate_init()) {
        return (-1);
    }

    if (!DeviceIoControl(hCMEM, CMEM_IOCGETNUMBLOCKS, &numBlocks,
                sizeof(numBlocks), NULL, 0, &dwBytesReturned, NULL)) {
        __E("CMEM_getNumBlocks: Failed to retrieve version from driver\n");

        return (-1);
    }

    __D("CMEM_getNumBlocks: ioctl CMEM_IOCGETNUMBLOCKS returned %#x\n",
            numBlocks);

    *pnblocks = numBlocks;

    return (0);
}

int CMEM_exit(void)
{
    BOOL result = FALSE;

    __D("exit: entered - ref_count %d, hCMEM %d\n", ref_count, hCMEM);
    __D("exit: decrementing ref_count\n");

    ref_count--;
    if (ref_count == 0) {
        result = CloseHandle(hCMEM);

        __D("exit: ref_count == 0, closed CMEM (%s)\n", (!result) ? "failed"
                : "succeeded");
        hCMEM = NULL;
    }

    __D("exit: exiting, returning %d\n", result);

    return (result);
}

/*
 *  ======== CMEM_mapAddress ========
 */
int CMEM_mapAddress(unsigned long physAddr, unsigned long length,
        unsigned int flags, unsigned long *pVirtAddr)
{
    union CMEM_AllocUnion mapDesc;
    DWORD   dwBytesReturned;

    __D("CMEM_mapAddress: entered, addr %p, length %#x, flags %#x\n",
            physAddr, length, flags);

    if (!validate_init()) {
        return (-1);
    }

    /* Make sure address, length are multiples of page sise */
    if (physAddr & (dwPageSize - 1)) {
        __E("CMEM_mapAddress: Input address %#x not page aligned\n", physAddr);
        return (-2);
    }

    if (length & (dwPageSize - 1)) {
        __E("CMEM_mapAddress: Input length %#x not page aligned\n", length);
        return (-2);
    }

    mapDesc.map_inparams.physp = physAddr;
    mapDesc.map_inparams.size = (size_t)length;
    mapDesc.map_inparams.dwProcId = (DWORD)GetCurrentProcessId();

    if (!DeviceIoControl(hCMEM, CMEM_IOCMAPADDRESS, &mapDesc, sizeof(mapDesc),
                NULL, 0, &dwBytesReturned, NULL)) {
        __E("CMEM_mapAddress: Failed to map physical address %#x\n",
                (unsigned int)physAddr);
        return (-3);
    }

    __D("CMEM_mapAddress: ioctl CMEM_IOCMAPADDRESS succeeded, virtAddr=%#lx\n",
            mapDesc.virtp);
    *pVirtAddr = mapDesc.virtp;

    return (0);
}

/*
 *  ======== CMEM_unmapAddress ========
 */
int CMEM_unmapAddress(unsigned long virtAddr, unsigned long length)
{
    union CMEM_AllocUnion mapDesc;
    DWORD   dwBytesReturned;

    __D("CMEM_unmapAddress: entered, addr %p, length %#x\n", virtAddr, length);

    mapDesc.unmap_inparams.virtp = virtAddr;
    mapDesc.unmap_inparams.size = (size_t)length;
    mapDesc.unmap_inparams.dwProcId = (DWORD)GetCurrentProcessId();

    if (!DeviceIoControl(hCMEM, CMEM_IOCUNMAPADDRESS, &mapDesc,
                sizeof(mapDesc), NULL, 0, &dwBytesReturned, NULL)) {
        __E("CMEM_unmapAddress: Failed to unmap virtual address %#x\n",
                (unsigned int)virtAddr);
        return (-3);
    }
    return (0);
}

/*
 *  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:12; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

 */

