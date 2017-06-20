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
 *  ======== cmemk.c ========
 */

/*
 * The following macros control version-dependent code:
 * USE_CACHE_VOID_ARG - #define if dmac functions take "void *" parameters,
 *    otherwise unsigned long is used
 * USE_CLASS_SIMPLE - #define if Linux version contains class_simple,
 *    otherwise class is used (Linux supports one or the other, not both)
 */
#define USE_CACHE_VOID_ARG

#include <windows.h>
//#include <kfuncs.h>

#include <strsafe.h>
#include <linklist.h>
#include <ceddk.h>
#include "../cmem.h"

/*
 * Poor man's config params
 */

/*
 * NOCACHE means physical memory block is ioremap()'ed as noncached
 */
#define NOCACHE

// HACK - figure out what MAX_POOLS really is and set it
#ifndef MAX_POOLS
#define MAX_POOLS (128)
#endif // MAX_POOLS

/*
 * USE_MMAPSEM means acquire/release current->mm->mmap_sem around calls
 * to dma_[flush/clean/inv]_range.
 */
//#define USE_MMAPSEM

/*
 * CHECK_FOR_ALLOCATED_BUFFER means ensure that the passed addr/size block
 * is actually an allocated, CMEM-defined buffer.
 */
//#define CHECK_FOR_ALLOCATED_BUFFER

DWORD gdwPageSize;

/* HEAP_ALIGN is used in place of sizeof(HeapMem_Header) */
#define HEAP_ALIGN gdwPageSize

// WINCE's implementation of PAGE_ALIGN is basically a floor function.
// Linux's implementation is ceiling function!
#ifdef PAGE_ALIGN
#undef PAGE_ALIGN
#endif // PAGE_ALIGN
#define PAGE_ALIGN(x) (((x) + gdwPageSize - 1) & ~(gdwPageSize - 1))

#define PAGE_MASK (~(gdwPageSize - 1))

#define CMEMK_REG_KEY TEXT("Drivers\\BuiltIn\\CMEMK")
#define CMEMK_REG_KEY_NUM_POOLS_0 TEXT("NumPools0")
#define CMEMK_REG_KEY_NUM_POOLS_1 TEXT("NumPools1")
#define CMEMK_REG_KEY_NUM_BUFFERS TEXT("NumBuffers_Pool")
#define CMEMK_REG_KEY_POOL_SIZE TEXT("PoolSize_Pool")
#define CMEMK_REG_KEY_BLOCK TEXT("Block")
#define CMEMK_REG_KEY_PHYSICAL_START_0 TEXT("PhysicalStart0")
#define CMEMK_REG_KEY_PHYSICAL_END_0 TEXT("PhysicalEnd0")
#define CMEMK_REG_KEY_PHYSICAL_START_1 TEXT("PhysicalStart1")
#define CMEMK_REG_KEY_PHYSICAL_END_1 TEXT("PhysicalEnd1")
#define CMEMK_REG_KEY_USE_HEAP TEXT("UseHeapIfPoolUnavailable")
#define CMEMK_MUTEX_NAME TEXT("CMEMK_MUTEX")

#ifdef __DEBUG

/*
 *  Return the base address of an instance of a structure of a specified type,
 *  given the address (ptr) of a member within the containing structure.
 *  For WinCE, we can probably just use CONTAINING_RECORD. list_entry is a
 *  Linux macro.
 */
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define __D(fmt, ...) RETAILMSG(1, (L"CMEMK Debug: " L##fmt, __VA_ARGS__))

#else
#define __D(fmt, ...)
#endif

#define __E(fmt, ...) RETAILMSG(1, (L"CMEMK Error: " L##fmt, __VA_ARGS__))

#define MAXTYPE(T) ((T) (((T)1 << ((sizeof(T) * 8) - 1) ^ ((T) -1))))

/*
 * Change here for supporting more than 2 blocks.  Also change all
 * NBLOCKS-based arrays to have NBLOCKS-worth of initialization values.
 */
#define NBLOCKS 2

enum MemAllocType {
    MMMAPIOSPACE = 0,
    ALLOCPHYSMEM
};

static unsigned long real_block_virtp[NBLOCKS] = {0, 0};
static unsigned long real_block_start[NBLOCKS] = {0, 0};
static unsigned long block_virtp[NBLOCKS] = {0, 0};
static unsigned long block_virtoff[NBLOCKS] = {0, 0};
static unsigned long block_virtend[NBLOCKS] = {0, 0};
static unsigned long block_start[NBLOCKS] = {0, 0};
static unsigned long block_end[NBLOCKS] = {0, 0};
static unsigned int block_avail_size[NBLOCKS] = {0, 0};
static unsigned int total_num_buffers[NBLOCKS] = {0, 0};
static unsigned int mem_alloc_type[NBLOCKS] = {MMMAPIOSPACE, MMMAPIOSPACE};

static int cmem_major;
static struct proc_dir_entry *cmem_proc_entry;
static LONG reference_count = 0;
static unsigned int version = CMEM_VERSION;

static int numBlocks = 0;

static char *phys_start = NULL;
static char *phys_end = NULL;

static int npools[NBLOCKS] = {0, 0};

static char *pools[MAX_POOLS] = {
    NULL
};

/* cut-and-paste below as part of adding support for more than 2 blocks */
static char *phys_start_1 = NULL;
static char *phys_end_1 = NULL;

static char *pools_1[MAX_POOLS] = {
    NULL
};

static int allowOverlap = 0;

static int useHeapIfPoolUnavailable = 0;

static HANDLE cmem_mutex = NULL;

/* Describes a pool buffer */
typedef struct pool_buffer {
    LIST_ENTRY element;
    int id;
    unsigned long physp;
    int flags;                 /* CMEM_CACHED or CMEM_NONCACHED */
    unsigned long kvirtp;      /* used only for heap-based allocs */
    size_t size;               /* used only for heap-based allocs */
    PVOID pUsrVA; /* user space virtual address that we mapped this entry to */
    DWORD dwUsrProcId; /* User space process ID that pUsrVA is mapped to */
} pool_buffer;

/* Describes a pool */
typedef struct pool_object {
    LIST_ENTRY freelist;
    LIST_ENTRY busylist;
    unsigned int numbufs;
    unsigned int size;
    unsigned int reqsize;
} pool_object;

pool_object p_objs[NBLOCKS][MAX_POOLS];


/*
 *  NOTE: The following implementation of a heap is taken from the
 *  DSP/BIOS 6.0 source tree (avala-f15/src/ti/sysbios/heaps).  Changes
 *  are necessary due to the fact that CMEM is not built in an XDC
 *  build environment, and therefore XDC types and helper APIs (e.g.,
 *  Assert) are not available.  However, these changes were kept to a
 *  minimum.
 *
 *  The changes include:
 *      - renaming XDC types to standard C types
 *      - replacing sizeof(HeapMem_Header) w/ HEAP_ALIGN throughout
 *
 *  As merged with CMEM, the heap becomes a distinguished "pool" and
 *  is sometimes treated specially, and at other times can be treated
 *  as a normal pool instance.
 */

/*
 * HeapMem compatibility stuff
 */
typedef struct HeapMem_Header {
    struct HeapMem_Header *next;
    unsigned long size;
} HeapMem_Header;

/*
 *  Describes a buffer obtained by mapping a physical address to a user
 *  space virtual address with CMEM_mapAddress(). We need to keep track of
 *  the kernel virtual address for unmapping the buffer.
 */
typedef struct Mapped_buffer {
    LIST_ENTRY element;
    int id;
    unsigned long physp;
    int flags;                 /* CMEM_CACHED or CMEM_NONCACHED */
    unsigned long kvirtp;      /* Kernel virtual address of mapped buffer */
    size_t size;               /* Size of mapped buffer */
    PVOID pUsrVA;              /* user virtual address buffer is mapped to */
    DWORD dwUsrProcId;         /* User process ID that pUsrVA is mapped to */
} Mapped_buffer;

static LIST_ENTRY mappedBufferList;

#define ALLOCRUN 0
#define DRYRUN 1

void *HeapMem_alloc(int bi, size_t size, size_t align, int dryrun);
void HeapMem_free(int bi, void *block, size_t size);

static unsigned long mapAddress(unsigned long physAddr, size_t size,
        DWORD dwProcId);
static int unmapAddress(unsigned long virtAddr, size_t size, DWORD dwProcId);


/*
 * Heap configuration stuff
 */
static unsigned long heap_size[NBLOCKS] = {0, 0};
static unsigned long heap_virtp[NBLOCKS] = {0, 0};
static int heap_pool[NBLOCKS] = {-1, -1};
static HeapMem_Header heap_head[NBLOCKS] = {
    {
        NULL,   /* next */
        0       /* size */
    },
/* cut-and-paste below as part of adding support for more than 2 blocks */
    {
        NULL,   /* next */
        0       /* size */
    }
/* cut-and-paste above as part of adding support for more than 2 blocks */
};

typedef struct deviceState {
    BOOL initialized;
    ULONG openCount;
} DEVICE_STATE, *PDEVICE_STATE;

DEVICE_STATE CMKDevice;

/*
 *  ======== HeapMem_alloc ========
 *  HeapMem is implemented such that all of the memory and blocks it works
 *  with have an alignment that is a multiple of HEAP_ALIGN and have a size
 *  which is a multiple of HEAP_ALIGN. Maintaining this requirement
 *  throughout the implementation ensures that there are never any odd
 *  alignments or odd block sizes to deal with.
 *
 *  Specifically:
 *  The buffer managed by HeapMem:
 *    1. Is aligned on a multiple of HEAP_ALIGN
 *    2. Has an adjusted size that is a multiple of HEAP_ALIGN
 *  All blocks on the freelist:
 *    1. Are aligned on a multiple of HEAP_ALIGN
 *    2. Have a size that is a multiple of HEAP_ALIGN
 *  All allocated blocks:
 *    1. Are aligned on a multiple of HEAP_ALIGN
 *    2. Have a size that is a multiple of HEAP_ALIGN
 *
 */
void *HeapMem_alloc(int bi, size_t reqSize, size_t reqAlign, int dryrun)
{
    HeapMem_Header *prevHeader, *newHeader, *curHeader;
    char *allocAddr;
    size_t curSize, adjSize;
    size_t remainSize; /* free memory after allocated memory      */
    size_t adjAlign, offset;
//    long key;

#if 0
    /* Assert that requested align is a power of 2 */
    Assert_isTrue(((reqAlign & (reqAlign - 1)) == 0), HeapMem_A_align);

    /* Assert that requested block size is non-zero */
    Assert_isTrue((reqSize != 0), HeapMem_A_zeroBlock);
#endif

    adjSize = reqSize;

    /* Make size requested a multiple of HEAP_ALIGN */
    if ((offset = (adjSize & (HEAP_ALIGN - 1))) != 0) {
        adjSize = adjSize + (HEAP_ALIGN - offset);
    }

    /*
     *  Make sure the alignment is at least as large as HEAP_ALIGN.
     *  Note: adjAlign must be a power of 2 (by function constraint) and
     *  HEAP_ALIGN is also a power of 2,
     */
    adjAlign = reqAlign;
    if (adjAlign & (HEAP_ALIGN - 1)) {
        /* adjAlign is less than HEAP_ALIGN */
        adjAlign = HEAP_ALIGN;
    }

    /*
     *  NOTE: We don't need a "gate" here since this function is called
     *  after acquiring a mutex.
     */

    /*
     *  The block will be allocated from curHeader. Maintain a pointer to
     *  prevHeader so prevHeader->next can be updated after the alloc.
     */
    prevHeader  = &heap_head[bi];
    curHeader = prevHeader->next;

    /* Loop over the free list. */
    while (curHeader != NULL) {

        curSize = curHeader->size;

        /*
         *  Determine the offset from the beginning to make sure
         *  the alignment request is honored.
         */
        offset = (unsigned long)curHeader & (adjAlign - 1);
        if (offset) {
            offset = adjAlign - offset;
        }

#if 0
        /* Internal Assert that offset is a multiple of HEAP_ALIGN */
        Assert_isTrue(((offset & (HEAP_ALIGN - 1)) == 0), NULL);
#endif

        /* big enough? */
        if (curSize >= (adjSize + offset)) {

            /* Set the pointer that will be returned. Alloc from front */
            allocAddr = (char *)((unsigned long)curHeader + offset);

            if (dryrun) {
                return ((void *)allocAddr);
            }

            /*
             *  Determine the remaining memory after the allocated block.
             *  Note: this cannot be negative because of above comparison.
             */
            remainSize = curSize - adjSize - offset;

#if 0
            /* Internal Assert that remainSize is a multiple of HEAP_ALIGN */
            Assert_isTrue(((remainSize & (HEAP_ALIGN - 1)) == 0), NULL);
#endif

            /*
             *  If there is memory at the beginning (due to alignment
             *  requirements), maintain it in the list.
             *
             *  offset and remainSize must be multiples of
             *  HEAP_ALIGN. Therefore the address of the newHeader
             *  below must be a multiple of the HEAP_ALIGN, thus
             *  maintaining the requirement.
             */
            if (offset) {

                /* Adjust the curHeader size accordingly */
                curHeader->size = offset;

                /*
                 *  If there is remaining memory, add into the free list.
                 *  Note: no need to coalesce and we have HeapMem locked so
                 *        it is safe.
                 */
                if (remainSize) {
                    newHeader = (HeapMem_Header *)
                        ((unsigned long)allocAddr + adjSize);
                    newHeader->next = curHeader->next;
                    newHeader->size = remainSize;
                    curHeader->next = newHeader;
                }
            }
            else {
                /*
                 *  If there is any remaining, link it in,
                 *  else point to the next free block.
                 *  Note: no need to coalesce and we have HeapMem locked so
                 *        it is safe.
                 */
                if (remainSize) {
                    newHeader = (HeapMem_Header *)
                        ((unsigned long)allocAddr + adjSize);
                    newHeader->next  = curHeader->next;
                    newHeader->size  = remainSize;
                    prevHeader->next = newHeader;
                }
                else {
                    prevHeader->next = curHeader->next;
                }
            }

            /* Success, return the allocated memory */
            return ((void *)allocAddr);
        }
        else {
            prevHeader = curHeader;
            curHeader = curHeader->next;
        }
    }

    return (NULL);
}

/*
 *  ======== HeapMem_free ========
 */
void HeapMem_free(int bi, void *addr, size_t size)
{
//    long key;
    HeapMem_Header *curHeader, *newHeader, *nextHeader;
    size_t offset;

    /* obj->head never changes, doesn't need Gate protection. */
    curHeader = &heap_head[bi];

    /* Restore size to actual allocated size */
    if ((offset = size & (HEAP_ALIGN - 1)) != 0) {
        size += HEAP_ALIGN - offset;
    }

/*
 * We don't need to enter the "gate" since this function is called
 * with it held already.
 */
//    key = Gate_enterModule();

    newHeader = (HeapMem_Header *)addr;
    nextHeader = curHeader->next;

    /* Go down freelist and find right place for buf */
    while (nextHeader != NULL && nextHeader < newHeader) {
        curHeader = nextHeader;
        nextHeader = nextHeader->next;
    }

    newHeader->next = nextHeader;
    newHeader->size = size;
    curHeader->next = newHeader;

    /* Join contiguous free blocks */
    /* Join with upper block */
    if ((nextHeader != NULL) &&
            (((unsigned long)newHeader + size) == (unsigned long)nextHeader)) {
        newHeader->next = nextHeader->next;
        newHeader->size += nextHeader->size;
    }

    /*
     *  Join with lower block. Make sure to check to see if not the
     *  first block.
     */
    if ((curHeader != &heap_head[bi]) &&
            (((unsigned long)curHeader + curHeader->size) ==
                    (unsigned long)newHeader)) {
        curHeader->next = newHeader->next;
        curHeader->size += newHeader->size;
    }

/*
 * See above comment on Gate_enterModule for an explanation of why we
 * don't use the "gate".
 */
//    Gate_leaveModule(key);

}

/*
 *  ======== get_phys ========
 *
 *  Translate virtual address to physical address. First check the CMEM
 *  blocks, in case the buffer was allocated by CMEM. If the address is
 *  one that was not returned by CMEM, use LockPages() to determine the
 *  physical address.
 */
static unsigned long get_phys(unsigned long virtp)
{
    unsigned long bufStart;
    unsigned long offset;
    unsigned long physp = 0;
    DWORD         dwPFN[2];
    int           bi;
    BOOL          fRet;

    /* For CMEM block kernel addresses */
    for (bi = 0; bi < NBLOCKS; bi++) {
        if (virtp >= block_virtp[bi] && virtp < block_virtend[bi]) {
            physp = virtp - block_virtoff[bi];
            __D("get_phys: block_virtoff[%d] translated kernel %#lx to %#lx\n",
                    bi, virtp, physp);
            return (physp);
        }
    }

    __D("get_phys(): Requested a VA to PA mapping for an address that CMEM did not allocate!\r\n");
    __D(" Perhaps it's in the list of busy entries?\r\n");

    offset = virtp % PAGE_SIZE; /* Offset from the beginning of a page */
    bufStart = virtp - offset;

    /*
     *  We will only query for one page, since we don't know the size of the
     *  buffer we're doing the get_phys() for.
     */
    fRet = LockPages((LPVOID)bufStart, PAGE_SIZE, dwPFN, LOCKFLAG_QUERY_ONLY);
    if (fRet) {
        physp = (dwPFN[0] << UserKInfo[KINX_PFN_SHIFT]) + offset;

        /* Don't call UnlockPages() since we only did a query */
    }
    else {
        __E("get_phys(): LockPages(0x%x) failed!\n");
    }

    return (physp);
}

/* Allocates space from the top "highmem" contiguous buffer for pool buffer. */
static unsigned long alloc_pool_buffer(int bi, unsigned int size)
{
    unsigned long virtp;

    __D("alloc_pool_buffer: Called for size %u\n", size);

    if (size <= block_avail_size[bi]) {
        __D("alloc_pool_buffer: Fits req %u < avail: %u\n",
                size, block_avail_size[bi]);

        block_avail_size[bi] -= size;
        virtp = block_virtp[bi] + block_avail_size[bi];

        __D("alloc_pool_buffer: new available block size is %d\n",
                block_avail_size[bi]);
        __D("alloc_pool_buffer: returning allocated buffer at %#lx\n", virtp);

        return virtp;
    }

    __E("Failed to find a big enough free block\n");

    return 0;
}


#ifdef __DEBUG
/* Only for debug */
/*
 *  ======== dump_lists ========
 */
static void dump_lists(int bi, int idx)
{
    LIST_ENTRY *freelistp = &p_objs[bi][idx].freelist;
    LIST_ENTRY *busylistp = &p_objs[bi][idx].busylist;
    LIST_ENTRY *e;
    struct pool_buffer *entry;

    if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
        return;
    }

    __D("Busylist for pool %d:\n", idx);
    for (e = busylistp->Flink; e != busylistp; e = e->Flink) {

        entry = list_entry(e, struct pool_buffer, element);

        __D("Busy: Buffer with id %d and physical address %#lx\n",
                entry->id, entry->physp);
    }

    __D("Freelist for pool %d:\n", idx);
    for (e = freelistp->Flink; e != freelistp; e = e->Flink) {

        entry = list_entry(e, struct pool_buffer, element);

        __D("Free: Buffer with id %d and physical address %#lx\n",
                entry->id, entry->physp);
    }

    ReleaseMutex(cmem_mutex);
}
#endif

/*
 *  ======== find_busy_entry ========
 *  find_busy_entry looks for an allocated pool buffer with addr. If
 *  bSearchByVA is FALSE, addr is a physical address, otherwise addr is a
 *  virtual address.
 *  
 *
 *  Should be called with the cmem_mutex held.
 */
static struct pool_buffer *find_busy_entry(unsigned long addr, int *poolp,
        LIST_ENTRY **ep, int *bip, BOOL bSearchByVA)
{
    LIST_ENTRY *busylistp;
    LIST_ENTRY *e;
    struct pool_buffer *entry;
    unsigned long physAddr;
    int num_pools;
    int i;
    int bi;


    /* Convert virtual address to physical address */
    physAddr = (bSearchByVA) ? get_phys(addr) : addr;

    for (bi = 0; bi < NBLOCKS; bi++) {
        num_pools = npools[bi];
        if (heap_pool[bi] != -1) {
            num_pools++;
        }

        for (i = 0; i < num_pools; i++) {
            busylistp = &p_objs[bi][i].busylist;

            for (e = busylistp->Flink; e != busylistp; e = e->Flink) {
                entry = CONTAINING_RECORD(e, struct pool_buffer, element);

                if (entry->physp == physAddr) {
                    if (poolp) {
                        *poolp = i;
                    }
                    if (ep) {
                        *ep = e;
                    }
                    if (bip) {
                        *bip = bi;
                    }

                    return (entry);
                }
            }
        }
    }

    return (NULL);
}

#define SHOW_BUSY_BANNER (1 << 0)
#define SHOW_PREV_FREE_BANNER (1 << 1)
#define SHOW_LAST_FREE_BANNER (1 << 2)
#define BUSY_ENTRY (1 << 3)
#define FREE_ENTRY (1 << 4)


/* Allocate a contiguous memory pool. */
static int alloc_pool(int bi, int idx, int num, int reqsize,
        unsigned long *virtpRet)
{
    struct pool_buffer *entry;
    LIST_ENTRY *freelistp = &p_objs[bi][idx].freelist;
    LIST_ENTRY *busylistp = &p_objs[bi][idx].busylist;
    int size = PAGE_ALIGN(reqsize);
    unsigned long virtp;
    int i;

    __D("Allocating %d buffers of size %d (requested %d)\n",
            num, size, reqsize);

    p_objs[bi][idx].reqsize = reqsize;
    p_objs[bi][idx].numbufs = num;
    p_objs[bi][idx].size = size;

    InitializeListHead(freelistp);
    InitializeListHead(busylistp);

    for (i=0; i < num; i++) {
        entry = (pool_buffer*)malloc(sizeof(struct pool_buffer));

        if (!entry) {
            __E("alloc_pool failed to malloc pool_buffer struct");
            //return -ENOMEM;
            return -1;
        }

        virtp = alloc_pool_buffer(bi, size);

        if (virtp == 0) {
            __E("alloc_pool failed to get contiguous area of size %d\n", size);
            //return -ENOMEM;
            return -1;
        }

        entry->id = i;
        entry->physp = get_phys(virtp);
        entry->kvirtp = virtp;

        if (virtpRet) {
            *virtpRet++ = virtp;
        }

        __D("Allocated buffer %d, virtual %#lx and physical %#lx and size %d\n",
                entry->id, virtp, entry->physp, size);

        //list_add_tail(&entry->element, freelistp);
        InsertTailList(freelistp, &entry->element);
    }

#ifdef __DEBUG
    dump_lists(bi, idx);
#endif

    return 0;
}


struct block_struct {
    unsigned long addr;
    size_t size;
};


/*
 *  ======== CMK_Init ========
 */
__declspec(dllexport)
DWORD
CMK_Init(PVOID Context)
{
    int bi;
    int i;
    int err;
    int pool_size;
    int pool_num_buffers;
    unsigned long length;
    HeapMem_Header *header;
    SYSTEM_INFO systemInfo;
    HKEY hKey;
    DWORD dwDataSize;

//    __D("** CMEM kernel module built: " __DATE__ " " __TIME__ "\n" );
//    DebugBreak();

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CMEMK_REG_KEY, 0, 0, &hKey) !=
            ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key!\n");
        return 0;
    }

    dwDataSize = sizeof(block_start[0]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_PHYSICAL_START_0, NULL, NULL,
                (LPBYTE)&block_start[0], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_PHYSICAL_START_0);
        block_start[0] = 0;
    }

    dwDataSize = sizeof(block_start[1]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_PHYSICAL_START_1, NULL, NULL,
                (LPBYTE)&block_start[1], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_PHYSICAL_START_1);
        block_start[1] = 0;
    }

    dwDataSize = sizeof(block_end[0]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_PHYSICAL_END_0, NULL, NULL,
                (LPBYTE)&block_end[0], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_PHYSICAL_END_0);
        block_end[0] = 0;
    }

    dwDataSize = sizeof(block_end[1]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_PHYSICAL_END_1, NULL, NULL,
                (LPBYTE)&block_end[1], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_PHYSICAL_END_1);
        block_end[1] = 0;
    }


    dwDataSize = sizeof(npools[0]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_NUM_POOLS_0, NULL, NULL,
                (LPBYTE)&npools[0], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_NUM_POOLS_0);
        npools[0] = 0;
    }

    dwDataSize = sizeof(npools[1]);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_NUM_POOLS_1, NULL, NULL,
                (LPBYTE)&npools[1], &dwDataSize) != ERROR_SUCCESS) {
        __E("CMEMK: cannot open registry key: %s!\n",
                CMEMK_REG_KEY_NUM_POOLS_1);
        npools[1] = 0;
    }

    dwDataSize = sizeof(useHeapIfPoolUnavailable);
    if (RegQueryValueEx(hKey, CMEMK_REG_KEY_USE_HEAP, NULL, NULL,
                (LPBYTE)&useHeapIfPoolUnavailable,
                &dwDataSize) != ERROR_SUCCESS) {
        __D("CMEMK: Warning: Cannot open registry key: %s\n",
                CMEMK_REG_KEY_USE_HEAP);
        useHeapIfPoolUnavailable = 0;
    }

    if (npools[0] > MAX_POOLS) {
        __E("Too many pools specified (%d) for Block 0, only %d supported.\n",
                npools[0], MAX_POOLS);
        goto fail_after_create;
    }

/* cut-and-paste below as part of adding support for more than 2 blocks */
    if (npools[1] > MAX_POOLS) {
        __E("Too many pools specified (%d) for Block 1, only %d supported.\n",
                npools[1], MAX_POOLS);
        goto fail_after_create;
    }

/* cut-and-paste above as part of adding support for more than 2 blocks */

    cmem_mutex = CreateMutex(NULL, FALSE, CMEMK_MUTEX_NAME);

    GetSystemInfo(&systemInfo);
    gdwPageSize = systemInfo.dwPageSize;

/* cut-and-paste above as part of adding support for more than 2 blocks */

    for (bi = 0; bi < NBLOCKS; bi++) {

/* cut-and-paste below as part of adding support for more than 2 blocks */
        if (bi == 1 && (!block_start[1] || !block_end[1])) {
            break;
        }

/* cut-and-paste above as part of adding support for more than 2 blocks */

        /* Get the start and end of CMEM memory */
        block_start[bi] = PAGE_ALIGN(block_start[bi]);
        block_end[bi] = PAGE_ALIGN(block_end[bi]);
        length = block_end[bi] - block_start[bi];

        if (length < 0) {
            __E("Negative length of physical memory (%ld)\n", length);
            err = 0;
            goto fail_after_create;
        }

        block_avail_size[bi] = length;
        numBlocks = bi + 1;

        // TODO: Can't do this in WINCE....figure out some other way!
        /* attempt to determine the end of Linux kernel memory */
        //phys_end_kernel = virt_to_phys((void *)PAGE_OFFSET) +
        //  (num_physpages << PAGE_SHIFT);

        //if (phys_end_kernel > block_start[bi])
        //{
        //  if (allowOverlap == 0)
        //  {
        //      __E("CMEM phys_start (%#lx) overlaps kernel (%#lx -> %#lx)\n",
        //          block_start[bi], virt_to_phys((void *)PAGE_OFFSET), phys_end_kernel);
        //      err = 0;
        //      goto fail_after_create;
        //  }
        //  else
        //  {
        //      printk("CMEM Range Overlaps Kernel Physical - allowing overlap\n");
        //      printk("CMEM phys_start (%#lx) overlaps kernel (%#lx -> %#lx)\n",
        //          block_start[bi], virt_to_phys((void *)PAGE_OFFSET), phys_end_kernel);
        //  }
        //}

        /* Initialize the top memory chunk in which to put the pools */
        //#ifdef NOCACHE
        //    block_virtp[bi] = (unsigned long) ioremap_nocache(block_start[bi], length);
        //#else
        //    block_virtp[bi] = (unsigned long) ioremap_cached(block_start[bi], length);
        //#endif
        if (block_start[bi]) {
            // physical address specified in registry (hole was punched in
            // address table in config.bib) - use it directly
            // HACKHACKHACK!!!! - This is a big security risk.  Either remove
            // this feature or add a check on the allowable physical address
            // range.
            //
            PHYSICAL_ADDRESS pa = {block_start[bi], 0};
#ifdef NOCACHE
            block_virtp[bi] = (unsigned long)MmMapIoSpace(pa, length, FALSE);
#else
            block_virtp[bi] = (unsigned long)MmMapIoSpace(pa, length, TRUE);
#endif // NOCACHE
            mem_alloc_type[bi] = MMMAPIOSPACE;
        }
        else {
            // user did not specify a fixed physical address - let
            // AllocPhysMem() decide where to allocate memory. Memory
            // allocated this way is always uncached.
            //
            // block_virtp[bi] = (unsigned long)AllocPhysMem(length,
            //         PAGE_READWRITE, gdwPageSize, 0, &block_start[bi]);
            // block_virtp[bi] = (unsigned long)AllocPhysMem(length,
            //         PAGE_READWRITE, 0, 0, &block_start[bi]);
#define PHYS_ALLOC_MEM_ALIGN 0x10000
            // hack: dsplink like the physical address which is 64k aligned,
            // but AllocPhysMem seems to only care if the virtual address
            // is aligned, so we have to adjust the physical address manually.
            //
            real_block_virtp[bi] = (unsigned long)AllocPhysMem(length +
                    PHYS_ALLOC_MEM_ALIGN, PAGE_READWRITE, PHYS_ALLOC_MEM_ALIGN,
                    0, &real_block_start[bi]);
            block_start[bi] = (real_block_start[bi] &
                    (PHYS_ALLOC_MEM_ALIGN - 1)) ?
                (real_block_start[bi] & ~(PHYS_ALLOC_MEM_ALIGN - 1)) +
                    PHYS_ALLOC_MEM_ALIGN
                : real_block_start[bi];

            block_virtp[bi] = real_block_virtp[bi] +
                (block_start[bi] - real_block_start[bi]);

            block_end[bi] += block_start[bi];
            mem_alloc_type[bi] = ALLOCPHYSMEM;
        }

        if (block_virtp[bi] == 0) {
#ifdef NOCACHE
            __E("Failed to allocate contiguous memory (non-cached) (%#lx, %ld)\n",
                    block_start[bi], length);
#else
            __E("Failed to allocate contiguous memory (cached) (%#lx, %ld)\n",
                    block_start[bi], length);
#endif
            err = 0;
            goto fail_after_create;
        }
        else {
#ifdef NOCACHE
            __D("Allocate contiguous memory (non-cached)(%#lx, %ld)=%#lx\n",
                    block_start[bi], length, block_virtp[bi]);
#else
            __D("allocate contiguous memory (cached)(%#lx, %ld)=%#lx\n",
                    block_start[bi], length, block_virtp[bi]);
#endif
        }

        block_virtoff[bi] = block_virtp[bi] - block_start[bi];
        block_virtend[bi] = block_virtp[bi] + length;

        memset((PUCHAR)block_virtp[bi], 0, length);

        /* Parse and allocate the pools */
        for (i = 0; i < npools[bi]; i++) {
            TCHAR tszRegKey[256];
            dwDataSize = sizeof(pool_num_buffers);
            if (StringCchPrintf(tszRegKey, 256, TEXT("%s%d_%s%d"),
                        CMEMK_REG_KEY_BLOCK, bi, CMEMK_REG_KEY_NUM_BUFFERS, i)
                    != S_OK) {
                RETAILMSG(1, (TEXT("ERROR: CMEMK init(): StringCchPrintf (NUM_BUFFERS) failed!!!\r\n")));
            }
            if (RegQueryValueEx(hKey, tszRegKey, NULL, NULL,
                        (LPBYTE)&pool_num_buffers, &dwDataSize) !=
                    ERROR_SUCCESS) {
                __E("CMEMK: cannot open registry key: %s!\n", tszRegKey);
                pool_num_buffers = 0;
            }

            if (StringCchPrintf(tszRegKey, 256, TEXT("%s%d_%s%d"),
                        CMEMK_REG_KEY_BLOCK, bi, CMEMK_REG_KEY_POOL_SIZE, i)
                    != S_OK) {
                RETAILMSG(1, (TEXT("ERROR: CMEMK init(): StringCchPrintf (POOL_SIZE) failed!!!\r\n")));
            }
            dwDataSize = sizeof(pool_size);
            if (RegQueryValueEx(hKey, tszRegKey, NULL, NULL,
                        (LPBYTE)&pool_size, &dwDataSize) != ERROR_SUCCESS) {
                __E("CMEMK: cannot open registry key: %s!\n", tszRegKey);
                pool_size = 0;
            }

            if (alloc_pool(bi, i, pool_num_buffers, pool_size, NULL) < 0) {
                __E("Failed to alloc pool of size %d and number of buffers %d\n",
                        pool_size, pool_num_buffers);
                err = 0;
                goto fail_after_ioremap;
            }

            total_num_buffers[bi] += pool_num_buffers;
        }

        /* use whatever is left for the heap */
        heap_size[bi] = block_avail_size[bi] & PAGE_MASK;
        heap_pool[bi] = npools[bi];
        if (heap_size[bi] > 0) {
            err = alloc_pool(bi, heap_pool[bi], 1, heap_size[bi],
                    &heap_virtp[bi]);
            if (err < 0) {
                __E("Failed to alloc heap of size %#lx\n", heap_size[bi]);
                goto fail_after_ioremap;
            }
            __D("allocated heap buffer %#lx of size %#lx\n", heap_virtp[bi],
                    heap_size[bi]);
            header = (HeapMem_Header *)heap_virtp[bi];
            heap_head[bi].next = header;
            heap_head[bi].size = heap_size[bi];
            header->next = NULL;
            header->size = heap_size[bi];
        }
        else {
            __D("no remaining memory for heap, no heap created for memory block %d\n", bi);
            heap_head[bi].next = NULL;
            heap_head[bi].next = 0;
        }

        __D("cmem initialized %d pools between %#lx and %#lx\n", npools[bi],
                block_start[bi], block_end[bi]);
    }

    /* Initialize list of mapped buffers */
    InitializeListHead(&mappedBufferList);

    __D("Successfully initialized module\n");

    RegCloseKey(hKey);
    CMKDevice.openCount = 0;
    CMKDevice.initialized = TRUE;

    return (DWORD)&CMKDevice;


  fail_after_ioremap:
    for (bi = 0; bi < NBLOCKS; bi++) {
        if (mem_alloc_type[bi] == MMMAPIOSPACE) {
            MmUnmapIoSpace((PVOID)block_virtp[bi], block_avail_size[bi]);
        }
        else if (mem_alloc_type[bi] == ALLOCPHYSMEM) {
            FreePhysMem((LPVOID)block_virtp[bi]);
        }
    }

  fail_after_create:
//fail_after_reg:
    RegCloseKey(hKey);
    return 0;
}

/*
 *  ======== CMK_Deinit ========
 */
__declspec(dllexport)
BOOL
CMK_Deinit(DWORD dwContext)
{
    DEVICE_STATE *pCMKDevice = (DEVICE_STATE *)dwContext;
    LIST_ENTRY *freelistp;
    LIST_ENTRY *busylistp;
    LIST_ENTRY *e;
    struct pool_buffer *entry;
    int num_pools;
    int bi;
    int i;

    __D("In cmem_exit()\n");

    for (bi = 0; bi < NBLOCKS; bi++) {
        num_pools = npools[bi];
        if (!num_pools) {
            continue;
        }

        if (heap_pool[bi] != -1) {
            num_pools++;
        }

        /* Free the pool structures and empty the lists. */
        for (i=0; i<num_pools; i++) {
            __D("Freeing memory associated with pool %d\n", i);

            freelistp = &p_objs[bi][i].freelist;
            busylistp = &p_objs[bi][i].busylist;

            e = busylistp->Flink;
            while (e != busylistp) {
                entry = CONTAINING_RECORD(e, struct pool_buffer, element);
                __D("Warning: Freeing busy entry %d at %#lx\n",
                        entry->id, entry->physp);
                e = e->Flink;
                free(entry);
            }

            e = freelistp->Flink;
            while (e != freelistp) {
                entry = CONTAINING_RECORD(e, struct pool_buffer, element);
                __D("Freeing free entry %d at %#lx\n", entry->id, entry->physp);
                e = e->Flink;
                free(entry);
            }
        }

        //iounmap((void *) block_virtp[bi]);
        if (mem_alloc_type[bi] == MMMAPIOSPACE) {
            MmUnmapIoSpace((PVOID)block_virtp[bi], block_avail_size[bi]);
        }
        else if (mem_alloc_type[bi] == ALLOCPHYSMEM) {
            FreePhysMem((LPVOID)block_virtp[bi]);
        }
    }
    CloseHandle(cmem_mutex);

    pCMKDevice->initialized = FALSE;
    if (pCMKDevice->openCount != 0) {
        RETAILMSG(1, (L"CMEMK: warning, deinitialized with open clients\r\n"));
    }

    return TRUE;
}

/*
 *  ======== CMK_Open ========
 */
__declspec(dllexport)
DWORD
CMK_Open(DWORD Context, DWORD Access, DWORD ShareMode)
{
    DEVICE_STATE *pCMKDevice = (DEVICE_STATE *)Context;

    InterlockedIncrement(&pCMKDevice->openCount);

    return (Context);
}


/*
 *  ======== CMK_Close ========
 */
__declspec(dllexport)
BOOL
CMK_Close(DWORD dwContext)
{
    DEVICE_STATE *pCMKDevice = (DEVICE_STATE *)dwContext;

    if (pCMKDevice->openCount < 1) {
        RETAILMSG(1, (TEXT("CMEMK: CMK_Close() - openCount < 1!\r\n")));
        return FALSE;
    }

    InterlockedDecrement(&pCMKDevice->openCount);

    return (TRUE);
}


__declspec(dllexport)
DWORD
CMK_Read(
    DWORD  dwContext,
    LPVOID pBuf,
    DWORD  Len
    )
{
    return  0;
}

__declspec(dllexport)
DWORD
CMK_Write(
    DWORD  dwContext,
    LPVOID pBuf,
    DWORD  Len
    )
{
    return  0;
}

__declspec(dllexport)
ULONG
CMK_Seek(
    PVOID Context,
    LONG Position,
    DWORD Type
    )
{
    return 0;
}


__declspec(dllexport)
BOOL
CMK_IOControl(
    DWORD  dwContext,
    DWORD  Ioctl,
    PUCHAR pInBuf,
    DWORD  InBufLen,
    PUCHAR pOutBuf,
    DWORD  OutBufLen,
    PDWORD pdwBytesTransferred
    )
{

    LIST_ENTRY *freelistp = NULL;
    LIST_ENTRY *busylistp = NULL;
    LIST_ENTRY *e = NULL;
    struct pool_buffer *entry;
    unsigned long physp;
    unsigned long virtp, virtp_end;
    size_t reqsize, size, align;
    int delta = MAXTYPE(int);
    int pool = -1;
    int i;
    int bi;
    struct block_struct block;
    struct block_struct *pBlock;
    union CMEM_AllocUnion allocDesc;
    //PUCHAR pInBufSafe = NULL;
    union CMEM_AllocUnion *pAllocDesc;
    PVOID pUsrVA;
    DWORD dwCallerProcId;
    DWORD cacheFlag;

    switch (Ioctl & CMEM_IOCCMDMASK) {
        case CMEM_IOCALLOCHEAP:
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            pAllocDesc = (union CMEM_AllocUnion*)pInBuf;

            size = pAllocDesc->alloc_heap_inparams.size;
            align = pAllocDesc->alloc_heap_inparams.align;
            bi = pAllocDesc->alloc_heap_inparams.blockid;
            dwCallerProcId = pAllocDesc->alloc_heap_inparams.dwProcId;

            __D("ALLOCHEAP%s ioctl received on heap pool for block %d\n",
                    Ioctl & CMEM_CACHED ? "CACHED" : "", bi);

            if (bi >= NBLOCKS) {
                __E("ioctl: invalid block id %d, must be < %d\n",
                        bi, NBLOCKS);
                return FALSE;
            }

            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                return FALSE;
            }

            virtp = (unsigned long)HeapMem_alloc(bi, size, align, ALLOCRUN);
            if (virtp == (unsigned long)NULL) {
                __E("ioctl: failed to allocate heap buffer of size %#x\n",
                        size);
                ReleaseMutex(cmem_mutex);
                return FALSE;
            }

            entry = (pool_buffer*)malloc(sizeof(struct pool_buffer));

            cacheFlag = (Ioctl & CMEM_CACHED) ? 0 : PAGE_NOCACHE;

            pUsrVA = VirtualAllocEx((HANDLE)dwCallerProcId, NULL, size,
                    MEM_RESERVE, PAGE_READWRITE | cacheFlag);

            if (!entry || !pUsrVA ||
                    !VirtualCopyEx((HANDLE)dwCallerProcId, pUsrVA,
                            (HANDLE)GetCurrentProcessId(), (PVOID)virtp, size,
                            PAGE_READWRITE | cacheFlag)) {
                __E("ioctl: failed to malloc pool_buffer struct for heap");
                HeapMem_free(bi, (void *)virtp, size);
                ReleaseMutex(cmem_mutex);
                return FALSE;
            }

            physp = get_phys(virtp);

            entry->id = heap_pool[bi];
            entry->physp = physp;
            entry->kvirtp = virtp;
            entry->size = size;
            entry->flags = Ioctl & ~CMEM_IOCCMDMASK;
            entry->pUsrVA = pUsrVA;
            entry->dwUsrProcId = dwCallerProcId;

            busylistp = &p_objs[bi][heap_pool[bi]].busylist;
            //list_add_tail(&entry->element, busylistp);
            InsertTailList(busylistp, &entry->element);

            ReleaseMutex(cmem_mutex);

            pAllocDesc->physp = physp;
            pAllocDesc->virtp = (unsigned long)pUsrVA;

            __D("ALLOCHEAP%s: allocated %#x size buffer at %#lx (phys address)\n",
                    Ioctl & CMEM_CACHED ? "CACHED" : "", entry->size,
                    entry->physp);

            break;

            /*
             * argp contains a pointer to an alloc descriptor coming in, and the
             * physical address and size of the allocated buffer when returning.
             */
        case CMEM_IOCALLOC:
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            pAllocDesc = (union CMEM_AllocUnion*)pInBuf;

            pool = pAllocDesc->alloc_pool_inparams.poolid;
            bi = pAllocDesc->alloc_pool_inparams.blockid;
            dwCallerProcId = pAllocDesc->alloc_pool_inparams.dwProcId;

            __D("ALLOC%s ioctl received on pool %d for memory block %d\n",
                    Ioctl & CMEM_CACHED ? "CACHED" : "", pool, bi);

            if (bi >= NBLOCKS) {
                __E("ioctl: invalid block id %d, must be < %d\n",
                        bi, NBLOCKS);
                return FALSE;
            }

            if (pool >= npools[bi] || pool < 0) {
                __E("ALLOC%s: invalid pool (%d) passed.\n",
                        Ioctl & CMEM_CACHED ? "CACHED" : "", pool);
                return FALSE;
            }

            freelistp = &p_objs[bi][pool].freelist;
            busylistp = &p_objs[bi][pool].busylist;

            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                return FALSE;
            }

            e = freelistp->Flink;
            if (e == freelistp) {
                __E("ALLOC%s: No free buffers available for pool %d\n",
                        Ioctl & CMEM_CACHED ? "CACHED" : "", pool);
                ReleaseMutex(cmem_mutex);
                return FALSE;
            }
            entry = CONTAINING_RECORD(e, struct pool_buffer, element);

            cacheFlag = (Ioctl & CMEM_CACHED) ? 0 : PAGE_NOCACHE;

            pAllocDesc->alloc_pool_outparams.physp = entry->physp;
            pAllocDesc->alloc_pool_outparams.size = p_objs[bi][pool].size;

            pUsrVA = VirtualAllocEx((HANDLE)dwCallerProcId, NULL,
                    p_objs[bi][pool].size, MEM_RESERVE,
                    PAGE_READWRITE | cacheFlag);

            if (!pUsrVA || !VirtualCopyEx((HANDLE)dwCallerProcId, pUsrVA,
                        (HANDLE)GetCurrentProcessId(), (PVOID)entry->kvirtp,
                        p_objs[bi][pool].size,
                        PAGE_READWRITE | cacheFlag)) {
                __E("ALLOC%s: VirtualCopyEx() failed for pool %d\n",
                        Ioctl & CMEM_CACHED ? "CACHED" : "", pool);
                ReleaseMutex(cmem_mutex);
                return FALSE;
            }
            pAllocDesc->alloc_pool_outparams.virtp = (unsigned long)pUsrVA;
            entry->pUsrVA = pUsrVA;
            entry->dwUsrProcId = dwCallerProcId;
            entry->flags = Ioctl & ~CMEM_IOCCMDMASK;

            RemoveEntryList(e);
            InitializeListHead(e);

            InsertHeadList(busylistp, e);

            ReleaseMutex(cmem_mutex);

            __D("ALLOC%s: allocated a buffer at %#lx (phys address)\n",
                    Ioctl & CMEM_CACHED ? "CACHED" : "", entry->physp);

#ifdef __DEBUG
            dump_lists(bi, pool);
#endif
            break;

            /*
             * argp contains either the user virtual address or the physical
             * address of the buffer to free coming in, and contains the pool
             * where it was freed from and the size of the block on return.
             */
        case CMEM_IOCFREE:
            __D("FREE%s%s ioctl received.\n",
                    Ioctl & CMEM_HEAP ? "HEAP" : "",
                    Ioctl & CMEM_PHYS ? "PHYS" : "");

            if (InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }

            allocDesc = *(union CMEM_AllocUnion*)pInBuf;

            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                return FALSE;
            }

            if (Ioctl & CMEM_PHYS) {
                // user passed in a physical address
                entry = find_busy_entry(allocDesc.virtp, &pool, &e, &bi, FALSE);
            }
            else {
                // user passed in a virtual address
                entry = find_busy_entry(allocDesc.virtp, &pool, &e, &bi, TRUE);
            }

            if (entry) {
                if (!VirtualFreeEx((HANDLE)entry->dwUsrProcId, entry->pUsrVA,
                            0, MEM_RELEASE)) {
                    __E("FREE%s%s: Failed to free buffer %d from pool %d - error:%d\n",
                            Ioctl & CMEM_HEAP ? "HEAP" : "",
                            Ioctl & CMEM_PHYS ? "PHYS" : "",
                            entry->id, pool, GetLastError());
                }

                if (Ioctl & CMEM_HEAP) {
                    HeapMem_free(bi, (void *)entry->kvirtp, entry->size);
                    RemoveEntryList(e);
                    free(entry);
                }
                else {
                    RemoveEntryList(e);
                    InitializeListHead(e);
                    InsertHeadList(&p_objs[bi][pool].freelist, e);
                }

                __D("FREE%s%s: Successfully freed buffer %d from pool %d\n",
                        Ioctl & CMEM_HEAP ? "HEAP" : "",
                        Ioctl & CMEM_PHYS ? "PHYS" : "", entry->id, pool);
            }

            ReleaseMutex(cmem_mutex);

            if (!entry) {
                __E("Failed to free memory at %#lx\n", allocDesc.virtp);
                return FALSE;
            }

#ifdef __DEBUG
            dump_lists(bi, pool);
#endif
            if (Ioctl & CMEM_HEAP) {
                allocDesc.free_outparams.size = entry->size;
            }
            else {
                allocDesc.free_outparams.size = p_objs[bi][pool].size;
            }
            allocDesc.free_outparams.poolid = pool;
            if (!pInBuf || InBufLen < sizeof(allocDesc)) {
                return FALSE;
            }
            *(union CMEM_AllocUnion*)pInBuf = allocDesc;

            __D("FREE%s%s: returning size %d, poolid %d\n",
                    Ioctl & CMEM_HEAP ? "HEAP" : "",
                    Ioctl & CMEM_PHYS ? "PHYS" : "",
                    allocDesc.free_outparams.size,
                    allocDesc.free_outparams.poolid);

            break;

       /*
        * argp contains the user virtual address of the buffer to translate
        * coming in, and the translated physical address on return.
        */
        case CMEM_IOCGETPHYS:
            __D("GETPHYS ioctl received.\n");
            //if (get_user(virtp, argp))
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            virtp = ((union CMEM_AllocUnion*)pInBuf)->virtp;
            __D("CMK_IOControl case CMEM_IOCGETPHYS: virtp = %#lx.\n", virtp);

            physp = get_phys(virtp);

            if (physp == 0) {
                // user passed in a user space virtual address, maybe?
                if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                    return FALSE;
                }
                entry = find_busy_entry(virtp, &pool, &e, &bi, TRUE);

                if (entry) {
                    physp = entry->physp;
                }
                else {
                    ReleaseMutex(cmem_mutex);
                    __E("GETPHYS: Failed to convert virtual %#lx to physical.\n",
                            virtp);
                    return FALSE;
                }
                ReleaseMutex(cmem_mutex);
            }

            ((union CMEM_AllocUnion*)pInBuf)->physp = physp;

            __D("GETPHYS: returning %#lx\n", physp);
            break;

            /*
             * argp contains the pool to query for size coming in, and the size
             * of the pool on return.
             */
        case CMEM_IOCGETSIZE:
            __D("GETSIZE ioctl received\n");

            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            pAllocDesc = (union CMEM_AllocUnion*)pInBuf;

            pool = pAllocDesc->get_size_inparams.poolid;
            bi = pAllocDesc->get_size_inparams.blockid;

            if (bi >= NBLOCKS) {
                __E("ioctl: invalid block id %d, must be < %d\n",
                        bi, NBLOCKS);
                return FALSE;
            }

            if (pool >= npools[bi] || pool < 0) {
                __E("GETSIZE: invalid pool (%d) passed.\n", pool);
                return FALSE;
            }

            if (InBufLen < sizeof(p_objs[bi][pool].size)) {
                return FALSE;
            }
            *(unsigned long *)pInBuf = p_objs[bi][pool].size;

            __D("GETSIZE returning %d\n", p_objs[bi][pool].size);
            break;

            /*
             * argp contains the requested pool buffers size coming in, and the
             * pool id (index) on return.
             */
        case CMEM_IOCGETPOOL:
            __D("GETPOOL ioctl received.\n");

            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            pAllocDesc = (union CMEM_AllocUnion*)pInBuf;

            reqsize = pAllocDesc->get_pool_inparams.size;
            bi = pAllocDesc->get_pool_inparams.blockid;

            if (bi >= NBLOCKS) {
                __E("ioctl: invalid block id %d, must be < %d\n",
                        bi, NBLOCKS);
                return FALSE;
            }

            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                return FALSE;
            }

            __D("GETPOOL: Trying to find a pool to fit size %d\n", reqsize);
            for (i = 0; i < npools[bi]; i++) {
                size = p_objs[bi][i].size;
                freelistp = &p_objs[bi][i].freelist;

                __D("GETPOOL: size (%d) > reqsize (%d)?\n", size, reqsize);
                if (size >= reqsize) {
                    __D("GETPOOL: delta (%d) < olddelta (%d)?\n",
                            size - reqsize, delta);
                    if ((size - reqsize) < (unsigned int)delta) {
                        if (!IsListEmpty(freelistp)) {
                            delta = size - reqsize;
                            __D("GETPOOL: Found a best fit delta %d\n", delta);
                            pool = i;
                        }
                    }
                }
            }

            if (pool == -1 && heap_pool[bi] != -1) {
                if (useHeapIfPoolUnavailable) {
                    /* no pool buffer available, try heap */
                    virtp = (unsigned long)HeapMem_alloc(bi, reqsize,
                            HEAP_ALIGN, DRYRUN);
                    if (virtp != (unsigned long)NULL) {
                        /*
                         * Indicate heap pool with magic negative value.
                         * -1 indicates no pool and no heap.
                         * -2 indicates no pool but heap available and allowed.
                         */
                        pool = -2;

                        __D("GETPOOL: no pool-based buffer available, returning heap \"pool\" instead (due to config override)\n");
                    }
                }
            }

            ReleaseMutex(cmem_mutex);

            if (pool == -1) {
                __E("Failed to find a pool which fits %d\n", reqsize);
                return FALSE;
            }

            if (InBufLen < sizeof(pool)) {
                return FALSE;
            }
            ((union CMEM_AllocUnion*)pInBuf)->poolid = pool;
            __D("GETPOOL: returning %d\n", pool);
            break;

        case CMEM_IOCCACHE:
            __D("CACHE%s%s ioctl received.\n",
                    Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "");

            if (!pInBuf || InBufLen < sizeof(block)) {
                return FALSE;
            }
            pBlock = (struct block_struct*)pInBuf;

            virtp = pBlock->addr;
            virtp_end = virtp + pBlock->size;

#ifdef CHECK_FOR_ALLOCATED_BUFFER
            physp = get_phys(virtp);
            if (physp == 0) {
                __E("CACHE%s%s: Failed to convert virtual %#lx to physical\n",
                        Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "",
                        virtp);
                return FALSE;
            }

            __D("CACHE%s%s: translated %#lx user virtual to %#lx physical\n",
                    Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "",
                    virtp, physp);

            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                return FALSE;
            }
            entry = find_busy_entry(physp, &pool, &e, &bi, FALSE);
            ReleaseMutex(cmem_mutex);
            if (!entry) {
                __E("CACHE%s%s: Failed to find allocated buffer at virtual %#lx\n",
                        Ioctl & CMEM_WB ? "WB" : "",
                        Ioctl & CMEM_INV ? "INV" : "", virtp);
                return FALSE;
            }
            if (!(entry->flags & CMEM_CACHED)) {
                __E("CACHE%s%s: virtual buffer %#lx not cached\n",
                        Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "",
                        virtp);
                return FALSE;
            }
#endif

#ifdef USE_MMAPSEM
            __D("CACHE%s%s: acquiring mmap_sem ...\n",
                    Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "");
            down_write(&current->mm->mmap_sem);
#endif

            switch (Ioctl) {
                case CMEM_IOCCACHEWB:
                    CacheRangeFlush((LPVOID)virtp,
                            (DWORD)virtp_end - (DWORD)virtp,
                            CACHE_SYNC_WRITEBACK);
                    __D("CACHEWB: cleaned user virtual %#lx->%#lx\n",
                            virtp, virtp_end);

                    break;

                case CMEM_IOCCACHEINV:
                    CacheRangeFlush((LPVOID)virtp,
                            (DWORD)virtp_end - (DWORD)virtp,
                            CACHE_SYNC_DISCARD);
                    __D("CACHEINV: invalidated user virtual %#lx->%#lx\n",
                            virtp, virtp_end);

                    break;

                case CMEM_IOCCACHEWBINV:
                    CacheRangeFlush((LPVOID)virtp,
                            (DWORD)virtp_end - (DWORD)virtp,
                            CACHE_SYNC_DISCARD);
                    __D("CACHEWBINV: flushed user virtual %#lx->%#lx\n",
                            virtp, virtp_end);

                    break;
            }

#ifdef USE_MMAPSEM
            __D("CACHE%s%s: releasing mmap_sem ...\n",
                    Ioctl & CMEM_WB ? "WB" : "", Ioctl & CMEM_INV ? "INV" : "");
            up_write(&current->mm->mmap_sem);
#endif
            break;

        case CMEM_IOCGETVERSION:
            __D("GETVERSION ioctl received, returning %#x.\n", version);

            if (!pInBuf || InBufLen < sizeof(version)) {
                return FALSE;
            }
            *(unsigned int*)pInBuf = version;

            break;

        case CMEM_IOCGETNUMBLOCKS:
            __D("GETNUMBLOCKS ioctl received, returning %#x.\n", numBlocks);

            if (!pInBuf || InBufLen < sizeof(numBlocks)) {
                return FALSE;
            }
            *(unsigned int*)pInBuf = numBlocks;

            break;

        case CMEM_IOCGETBLOCK:
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return FALSE;
            }
            pAllocDesc = (union CMEM_AllocUnion*)pInBuf;

            bi = pAllocDesc->blockid;

            pAllocDesc->get_block_outparams.physp = block_start[bi];
            pAllocDesc->get_block_outparams.size = block_end[bi] -
                block_start[bi];

            __D("GETBLOCK ioctl received, returning phys base 0x%lx, size 0x%x.\n",
                    pAllocDesc->get_block_outparams.physp,
                    pAllocDesc->get_block_outparams.size);

            //if (copy_to_user(argp, &allocDesc, sizeof(allocDesc)))
            //{
            //  return FALSE;
            //}

            break;

       /*
        * argp contains the physical address and size of the region
        * to map, and the mapped physical address on return.
        */
        case CMEM_IOCMAPADDRESS:
            __D("MAPADDRESS ioctl received.\n");
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return (FALSE);
            }
            pAllocDesc = (union CMEM_AllocUnion *)pInBuf;
            physp = pAllocDesc->map_inparams.physp;
            size = pAllocDesc->map_inparams.size;
            dwCallerProcId = pAllocDesc->map_inparams.dwProcId;

            __D("MAPADDRESS: physp = %#x.\n", physp);

            /* Acquire mutex since we will be updating mapped buffer list */
            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                __E("MAPADDRESS: WaitForSingleObject failed!\n");
                return (FALSE);
            }

            virtp = mapAddress(physp, size, dwCallerProcId);

            ReleaseMutex(cmem_mutex);

            if (virtp == 0) {
                __E("MAPADDRESS: Failed to map physical address %#lx\n",
                        physp);
                return (FALSE);
            }

            pAllocDesc->virtp = virtp;

            __D("MAPADDRESS: returning %#lx\n", virtp);
            break;

       /*
        * argp contains the physical address and size of the region
        * to map, and the mapped physical address on return.
        */
        case CMEM_IOCUNMAPADDRESS:
            __D("UNMAPADDRESS ioctl received.\n");
            if (!pInBuf || InBufLen < sizeof(union CMEM_AllocUnion)) {
                return (FALSE);
            }

            pAllocDesc = (union CMEM_AllocUnion *)pInBuf;
            virtp = pAllocDesc->unmap_inparams.virtp;
            size = pAllocDesc->unmap_inparams.size;
            dwCallerProcId = pAllocDesc->unmap_inparams.dwProcId;
            __D("UNMAPADDRESS: virtp = %#lx.\n", virtp);
            __D("UNMAPADDRESS: size = %#lx.\n", size);

            /* Acquire mutex since we will be updating mapped buffer list */
            if (WaitForSingleObject(cmem_mutex, INFINITE) != WAIT_OBJECT_0) {
                __E("MAPADDRESS: WaitForSingleObject failed!\n");
                return (FALSE);
            }

            if (unmapAddress(virtp, size, dwCallerProcId) != 0) {
                __E("UNMAPADDRESS: Failed to unmap virtual address %#lx\n",
                        virtp);
                ReleaseMutex(cmem_mutex);
                return (FALSE);
            }

            ReleaseMutex(cmem_mutex);

            break;

        default:
            __E("Unknown ioctl received.\n");
            return FALSE;
    }

    return TRUE;
}

__declspec(dllexport)
VOID
CMK_PowerDown(DWORD dwContext)
{
}

__declspec(dllexport)
VOID
CMK_PowerUp(DWORD dwContext)
{
}

/*
 *  ======== mapAddress ========
 */
static unsigned long mapAddress(unsigned long physAddr, size_t size,
        DWORD dwProcId)
{
    PHYSICAL_ADDRESS    pa;
    PVOID               pv;
    PVOID               pUsrVA;
    Mapped_buffer      *entry;
    BOOL                retVal;


    entry = (Mapped_buffer *)malloc(sizeof(Mapped_buffer));
    if (entry == NULL) {
        __E("mapAddress: Failed to allocate Mapped_buffer list entry\n");
        return (0);
    }

    /* Reserve a block of virtual memory for the calling process. */
    pUsrVA = VirtualAllocEx((HANDLE)dwProcId, NULL, size, MEM_RESERVE,
            PAGE_READWRITE | PAGE_NOCACHE);

    if (pUsrVA == NULL) {
        __E("mapAddress: VirtualAllocEx failed for size %#lx\n", size);
        return (0);
    }

    /* Map physAddr to kernal virtual address, non-cached */
    pa.QuadPart = physAddr;
    pv = MmMapIoSpace(pa, size, FALSE);

    if (pv == NULL) {
        __E("mapAddress: MmMapIoSpace failed for address %#lx\n", physAddr);
        VirtualFreeEx((HANDLE)dwProcId, pUsrVA, 0, MEM_RELEASE);
        return (0);
    }

    /* Map the kernel virtual address to caller's virtual address */
    retVal = VirtualCopyEx((HANDLE)dwProcId, pUsrVA,
            (HANDLE)GetCurrentProcessId(), pv, size,
            PAGE_READWRITE | PAGE_NOCACHE);

    if (retVal == FALSE) {
        __E("mapAddress: VirtualCopyEx failed\n");
        free(entry);
        MmUnmapIoSpace(pv, size);        
        VirtualFreeEx((HANDLE)dwProcId, pUsrVA, 0, MEM_RELEASE);
        return (0);
    }

    /* Add mapped buffer info to list */
    entry->id = 0;
    entry->physp = physAddr;
    entry->flags = 0;       /* ?? */
    entry->kvirtp = (PVOID)pv;
    entry->size = size;
    entry->pUsrVA = pUsrVA;
    entry->dwUsrProcId = dwProcId;

    InsertTailList(&mappedBufferList, &entry->element);

    return ((unsigned long)pUsrVA);
}

/*
 *  ======== unmapAddress ========
 */
static int unmapAddress(unsigned long pUsrVA, size_t size, DWORD dwProcId)
{
    Mapped_buffer      *entry;
    LIST_ENTRY         *listp;
    LIST_ENTRY         *e;


    listp = &mappedBufferList;

    for (e = listp->Flink; e != listp; e = e->Flink) {
        entry = CONTAINING_RECORD(e, struct Mapped_buffer, element);

        if ((entry->pUsrVA == pUsrVA) && (entry->dwUsrProcId == dwProcId)) {
            break;
        }
    }

    if (e == listp) {
        __E("unmapAddress: Failed to find mapped buffer!\n");
        return (-1);
    }

    MmUnmapIoSpace((PVOID)entry->kvirtp, size);

    if (!VirtualFreeEx((HANDLE)dwProcId, (LPVOID)pUsrVA, 0, MEM_RELEASE)) {
        __E("unmapAddress: VirtualFreeEx failed\n");
        return (-2);
    }

    RemoveEntryList(e);
    free(entry);

    return (0);
}
/*
 *  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:12; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

 */

