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
 *  ======== cmem.h ========
 */

/**
 * @file    ti/sdo/winceutils/cmem/cmem.h
 *
 * @brief   Describes the interface to the contiguous memory allocator.
 *
 * The cmem user interface library wraps file system calls to an associated
 * kernel dll (cmemk.dll), which needs to be loaded in order for calls to
 * this library to succeed.
 *
 */
/**
 *  @defgroup   ti_sdo_winceutils_cmem_CMEM  Contiguous Memory Manager
 *
 *  This is the API for the Contiguous Memory Manager.
 */

#ifndef ti_sdo_winceutils_cmem_CMEM_H
#define ti_sdo_winceutils_cmem_CMEM_H

#include <windows.h>

#if defined (__cplusplus)
extern "C" {
#endif

/** @ingroup    ti_sdo_linuxutils_cmem_CMEM */
/*@{*/

#define CMEM_VERSION    0x02200000U

/* ioctl cmd "flavors" */
#define CMEM_WB                         0x0100
#define CMEM_INV                        0x0200
#define CMEM_HEAP                       0x0400  /**< operation applies to heap */
#define CMEM_POOL                       0x0000  /**< operation applies to a pool */
#define CMEM_CACHED                     0x0800  /**< allocated buffer is cached */
#define CMEM_NONCACHED                  0x0000  /**< allocated buffer is not cached */
#define CMEM_PHYS                       0x1000

/* supported "base" ioctl cmds for the driver. */
#define CMEM_IOCALLOC                   1
#define CMEM_IOCALLOCHEAP               2
#define CMEM_IOCFREE                    3
#define CMEM_IOCGETPHYS                 4
#define CMEM_IOCGETSIZE                 5
#define CMEM_IOCGETPOOL                 6
#define CMEM_IOCCACHE                   7
#define CMEM_IOCGETVERSION              8
#define CMEM_IOCGETBLOCK                9
#define CMEM_IOCREGUSER                 10
#define CMEM_IOCGETNUMBLOCKS            11
#define CMEM_IOCMAPADDRESS              12
#define CMEM_IOCUNMAPADDRESS            13

/*
 * New ioctl cmds should use integers greater than the largest current cmd
 * in order to not break backward compatibility.
 */

/* supported "flavors" to "base" ioctl cmds for the driver. */
#define CMEM_IOCCACHEWBINV              CMEM_IOCCACHE | CMEM_WB | CMEM_INV
#define CMEM_IOCCACHEWB                 CMEM_IOCCACHE | CMEM_WB
#define CMEM_IOCCACHEINV                CMEM_IOCCACHE | CMEM_INV
#define CMEM_IOCALLOCCACHED             CMEM_IOCALLOC | CMEM_CACHED
#define CMEM_IOCALLOCHEAPCACHED         CMEM_IOCALLOCHEAP | CMEM_CACHED
#define CMEM_IOCFREEHEAP                CMEM_IOCFREE | CMEM_HEAP
#define CMEM_IOCFREEPHYS                CMEM_IOCFREE | CMEM_PHYS
#define CMEM_IOCFREEHEAPPHYS            CMEM_IOCFREE | CMEM_HEAP | CMEM_PHYS

#define CMEM_IOCCMDMASK                 0x000000ff

/**
 * @brief Parameters for CMEM_alloc(), CMEM_alloc2(), CMEM_allocPool(),
 * CMEM_allocPool2(), CMEM_free().
 */
typedef struct CMEM_AllocParams {
    int type;           /**< either CMEM_HEAP or CMEM_POOL */
    int flags;          /**< either CMEM_CACHED or CMEM_NONCACHED */
    size_t alignment;   /**<
                         * only used for heap allocations, must be power of 2
                         */
} CMEM_AllocParams;

extern CMEM_AllocParams CMEM_DEFAULTPARAMS;

typedef struct CMEM_BlockAttrs {
    unsigned long phys_base;
    size_t size;
} CMEM_BlockAttrs;

/** @cond INTERNAL */

/**
 */
union CMEM_AllocUnion {
    struct {                    /**< */
        size_t size;
        size_t align;
        int blockid;
        DWORD dwProcId; // caller process ID - used to call VirtualCopyEx()
    } alloc_heap_inparams;      /**< */
    struct {                    /**< */
        int poolid;
        int blockid;
        DWORD dwProcId; // caller process ID - used to call VirtualCopyEx()
    } alloc_pool_inparams;      /**< */
    struct {                    /**< */
        int poolid;
        int blockid;
    } get_size_inparams;        /**< */
    struct {                    /**< */
        size_t size;
        int blockid;
    } get_pool_inparams;        /**< */
    struct {                    /**< */
        unsigned long physp;
        unsigned long virtp;
        size_t size;
    } alloc_pool_outparams;     /**< */
    struct {                    /**< */
        unsigned long physp;
        size_t size;
    } get_block_outparams;      /**< */
    struct {                    /**< */
        int poolid;
        size_t size;
    } free_outparams;           /**< */
    struct {                    /**< */
        unsigned long physp;
        size_t size;
        DWORD dwProcId; // caller process ID - used to call VirtualCopyEx()
    } map_inparams;             /**< */
    struct {                    /**< */
        unsigned long virtp;
        size_t size;
        DWORD dwProcId; // caller process ID - used for VirtualFreeEx
    } unmap_inparams;             /**< */
    unsigned long physp;
    unsigned long virtp;
    size_t size;
    int poolid;
    int blockid;
};

/** @endcond */

/**
 * @brief Initialize the CMEM module. Must be called before other API calls.
 *
 * @return 0 for success or -1 for failure.
 *
 * @sa CMEM_exit
 */
int CMEM_init(void);

/**
 * @brief Find the pool that best fits a given buffer size and has a buffer
 * available.
 *
 * @param   size    The buffer size for which a pool is needed.
 *
 * @return A poolid that can be passed to CMEM_allocPool(), or -1 for error.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_allocPool2()
 * @sa CMEM_free()
 * @sa CMEM_getPool2()
 */
int CMEM_getPool(size_t size);

/**
 * @brief Find the pool in memory block blockid that best fits a given
 * buffer size and has a buffer available.
 *
 * @param   blockid  Block number
 * @param   size     The buffer size for which a pool is needed.
 *
 * @return A poolid that can be passed to CMEM_allocPool2(), or -1 for error.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_allocPool2()
 * @sa CMEM_free()
 * @sa CMEM_getPool()
 */
int CMEM_getPool2(int blockid, size_t size);

/**
 * @brief Allocate memory from a specified pool.
 *
 * @param   poolid  The pool from which to allocate memory.
 * @param   params  Allocation parameters.
 *
 * @remarks @c params->type is ignored - a pool will always be used.
 * @remarks @c params->alignment is unused, since pool buffers are already
 *          aligned to specific boundaries.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_alloc2()
 * @sa CMEM_allocPool2()
 * @sa CMEM_free()
 */
void *CMEM_allocPool(int poolid, CMEM_AllocParams *params);

/**
 * @brief Allocate memory from a specified pool in a specified memory block.
 *
 * @param   blockid The memory block from which to allocate.
 * @param   poolid  The pool from which to allocate memory.
 * @param   params  Allocation parameters.
 *
 * @remarks @c params->type is ignored - a pool will always be used.
 * @remarks @c params->alignment is unused, since pool buffers are already
 *          aligned to specific boundaries.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_alloc2()
 * @sa CMEM_allocPool()
 * @sa CMEM_free()
 */
void *CMEM_allocPool2(int blockid, int poolid, CMEM_AllocParams *params);

/**
 * @brief Allocate memory of a specified size
 *
 * @param   size    The size of the buffer to allocate.
 * @param   params  Allocation parameters.
 *
 * @remarks Used to allocate memory from either a pool or the heap.
 *          If doing a pool allocation, the pool that best fits the requested
 *          size will be selected.  Use CMEM_allocPool() to allocate from a
 *          specific pool.
 *          Allocation will be cached or noncached, as specified by params.
 *          params->alignment valid only for heap allocation.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_allocPool2()
 * @sa CMEM_alloc2()
 * @sa CMEM_free()
 */
void *CMEM_alloc(size_t size, CMEM_AllocParams *params);

/**
 * @brief Allocate memory of a specified size from a specified memory block
 *
 * @param   blockid The memory block from which to allocate.
 * @param   size    The size of the buffer to allocate.
 * @param   params  Allocation parameters.
 *
 * @remarks Used to allocate memory from either a pool or the heap.
 *          If doing a pool allocation, the pool that best fits the requested
 *          size will be selected.  Use CMEM_allocPool() to allocate from a
 *          specific pool.
 *
 * @remarks Allocation will be cached or noncached, as specified by params.
 *          params->alignment valid only for heap allocation.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_allocPool2()
 * @sa CMEM_alloc()
 * @sa CMEM_free()
 */
void *CMEM_alloc2(int blockid, size_t size, CMEM_AllocParams *params);

/**
 * @brief Free a buffer previously allocated with
 *        CMEM_alloc()/ CMEM_allocPool().
 *
 * @param   ptr     The pointer to the buffer.
 * @param   params  Allocation parameters.
 *
 * @remarks Use the same CMEM_AllocParams as was used for the allocation.
 *          params->flags is "don't care".  params->alignment is "don't
 *          care".
 *
 * @return 0 for success or -1 for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_alloc2()
 * @sa CMEM_allocPool()
 * @sa CMEM_allocPool2()
 */
int CMEM_free(void *ptr, CMEM_AllocParams *params);

/**
 * @brief Get the physical address of a contiguous buffer.
 *
 * @param   ptr     The pointer to the buffer.
 *
 * @return The physical address of the buffer or 0 for failure.
 *
 * @pre Must have called CMEM_init()
 */
unsigned long CMEM_getPhys(void *ptr);

/**
 * @brief Do a cache writeback of the block pointed to by @c ptr/@c size
 *
 * @param   ptr     Pointer to block to writeback
 * @param   size    Size in bytes of block to writeback.
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheInv()
 * @sa CMEM_cacheWbInv()
 */
int CMEM_cacheWb(void *ptr, size_t size);

/**
 * @brief Do a cache invalidate of the block pointed to by @c ptr/@c size
 *
 * @param   ptr     Pointer to block to invalidate
 * @param   size    Size in bytes of block to invalidate
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheWb()
 * @sa CMEM_cacheWbInv()
 */
int CMEM_cacheInv(void *ptr, size_t size);

/**
 * @brief Do a cache writeback/invalidate of the block pointed to by
 *        @c ptr/@c size
 *
 * @param   ptr     Pointer to block to writeback/invalidate
 * @param   size    Size in bytes of block to writeback/invalidate
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheInv()
 * @sa CMEM_cacheWb()
 */
int CMEM_cacheWbInv(void *ptr, size_t size);

/**
 * @brief Retrieve version from CMEM driver.
 *
 * @return Installed CMEM driver's version number.
 *
 * @pre Must have called CMEM_init()
 */
int CMEM_getVersion(void);

/**
 * @brief Retrieve memory block bounds from CMEM driver
 *
 * @param   pphys_base   Pointer to storage for base physical address of
 *                       CMEM's memory block
 * @param   psize        Pointer to storage for size of CMEM's memory block
 *
 * @return Success (0) or failure (-1).
 *
 * @pre Must have called CMEM_init()
 *
 * @sa  CMEM_getBlockAttrs()
 * @sa  CMEM_getNumBlocks()
 */
int CMEM_getBlock(unsigned long *pphys_base, size_t *psize);

/**
 * @brief Retrieve extended memory block attributes from CMEM driver
 *
 * @param   blockid      Block number
 * @param   pattrs       Pointer to CMEM_BlockAttrs struct
 *
 * @return Success (0) or failure (-1).
 *
 * @remarks Currently this API returns the same values as CMEM_getBlock().
 *
 * @pre Must have called CMEM_init()
 *
 * @sa  CMEM_getBlock()
 * @sa  CMEM_getNumBlocks()
 */
int CMEM_getBlockAttrs(int blockid, CMEM_BlockAttrs *pattrs);

/**
 * @brief Retrieve number of blocks configured into CMEM driver
 *
 * @param   pnblocks     Pointer to storage for holding number of blocks
 * @param   pattrs       Pointer to CMEM_BlockAttrs struct
 *
 * @return Success (0) or failure (-1).
 *
 * @remarks Blocks are configured into CMEM through the Linux 'insmod'
 *          command.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa  CMEM_getBlock()
 * @sa  CMEM_getBlockAttrs()
 */
int CMEM_getNumBlocks(int *pnblocks);

/**
 * @brief Finalize the CMEM module.
 *
 * @return 0 for success or -1 for failure.
 *
 * @remarks After this function has been called, no other CMEM function may be
 *          called (unless CMEM is reinitialized).
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_init()
 */
int CMEM_exit(void);

/**
 * @brief Map a physical address to the calling process's virtual address
 *        space.
 *
 * @param   physAddr     The physical address to be mapped.
 * @param   length       The number of bytes to be mapped.
 * @param   flags        TBD.
 * @param   pVirtAddr    Location to store the virtual address of the mapped
 *                       region.
 *
 * @return 0 for success or <0 for failure.
 *
 * @pre Must have called CMEM_init()
 * @pre physAddr and length must be multiples of the page size.
 *
 * @sa CMEM_unmapAddress()
 */
int CMEM_mapAddress(unsigned long physAddr, unsigned long length,
        unsigned int flags, unsigned long *pVirtAddr);

/**
 * @brief Unmap a virtual address that was previously mapped to the calling
 *        process's virtual address space with CMEM_mapAddress().
 *
 * @param   virtAddr     The virtual address to be unmapped.
 * @param   length       The number of bytes to be unmapped.
 *
 * @return 0 for success or <0 for failure.
 *
 * @pre Must have called CMEM_mapAddress() to obtain virtAddr.
 *
 * @sa CMEM_mapAddress()
 */
int CMEM_unmapAddress(unsigned long virtAddr, unsigned long length);

/*@}*/

#if defined (__cplusplus)
}
#endif

#endif
/*
 *  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:11; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

 */

