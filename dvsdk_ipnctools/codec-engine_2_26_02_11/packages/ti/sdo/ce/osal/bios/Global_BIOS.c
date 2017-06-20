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
 *  ======== Global_BIOS.c ========
 */
#include <xdc/std.h>

#include <stdarg.h>
#include <string.h>

#include <ti/bios/include/std.h>
#include <ti/bios/include/sys.h>
#include <ti/bios/include/trc.h>
#include <ti/bios/utils/Load.h>
#include <ti/sdo/utils/trace/gt.h>

#include <ti/sdo/ce/osal/Trace.h>
#include <ti/sdo/ce/osal/Global.h>
#include <ti/sdo/ce/osal/Memory.h>

#define GTBIOSNAME "ti.bios"

typedef struct ExitFxnElem {
    Fxn                 fxn;
    struct ExitFxnElem *next;
} ExitFxnElem;

static ExitFxnElem *exitFxnList      = NULL;
static Bool         doRegisterAtExit = TRUE;

static GT_Mask curTrace;

extern Void ti_sdo_ce_osal_bios_init();

extern Int Global_getLoad(Int * window);

extern String Global_buildInfo[];  /* generated by Global.xdt */
static Bool   buildInfoPrinted = FALSE;

/*
 *  ======== Global_abort ========
 */
Void Global_abort(String fmt, ...)
{
    va_list vargs;

    Trace_print("\n======== Global_abort =============\n");

    va_start(vargs, fmt);
    Trace_vprint(fmt, vargs);
    va_end(vargs);

    SYS_abort("aborted.");
}

/*
 *  ======== Global_atexit ========
 *  TODO: this code is nearly the same as in Global_noOS.c; refactor
 */
Bool Global_atexit(Fxn fxn)
{
    ExitFxnElem *exitFxnElem = (ExitFxnElem *)Memory_alloc(sizeof(ExitFxnElem),
            NULL);

    GT_1trace(curTrace, GT_ENTER, "Global_atexit> enter (fxn=0x%x)\n", fxn);

    if (exitFxnElem == NULL) {
        return FALSE;
    }

    exitFxnElem->fxn  = fxn;
    exitFxnElem->next = exitFxnList;
    exitFxnList       = exitFxnElem;

    /* register with the real system exit only once in the lifetime */
    if (doRegisterAtExit) {
        SYS_atexit((Fxn)Global_exit);
        doRegisterAtExit = FALSE;
    }

    return (TRUE);
}

/*
 *  ======== Global_exit ========
 *  TODO: this code is nearly the same as in Global_noOS.c; refactor
 */
Void Global_exit()
{
    ExitFxnElem *old;

    GT_0trace(curTrace, GT_ENTER, "Global_exit> enter\n");

    while (exitFxnList != NULL) {
        GT_1trace(curTrace, GT_2CLASS, "Global_exit> "
            "calling function *0x%x()...\n", exitFxnList->fxn);
        exitFxnList->fxn();
        old = exitFxnList;      /* TODO:H needs protection from preemption? */
        exitFxnList = exitFxnList->next;
        Memory_free(old, sizeof(ExitFxnElem), NULL);
    }

    GT_0trace(curTrace, GT_ENTER, "Global_exit> return\n" );
}

/*
 *  ======== Global_getCpuLoad ========
 */
Int Global_getCpuLoad(Void)
{
    Int load;
    Int window = -1;   /* initialize to invalid value */ 

    /* call the appropriate load monitor (either Load or PWRM) */
    load = Global_getLoad(&window);

    /* if using Load module will report a "window" value to trace ... */
    if (window != -1) {
        GT_2trace(curTrace, GT_ENTER,
            "Global_getCpuLoad: window = 0x%x, load = %d\n", window, load);
    }

    /* else, no change to window, then don't report it to trace ... */
    else {
        GT_1trace(curTrace, GT_ENTER,
            "Global_getCpuLoad: load = %d\n", load);
    }

    return (load);
}

/*
 *  ======== Global_getenv ========
 */
String Global_getenv(String name)
{
    return (NULL);
}

/*
 *  ======== Global_getProcessId ========
 */
UInt32 Global_getProcessId(Void)
{
    return (0);
}

/*
 *  ======== Global_init ========
 */
Void Global_init(Void)
{
    static Bool curInit = FALSE;

    if (curInit != TRUE) {
        curInit = TRUE;

        GT_init();
        Load_init();
        Trace_init();

        GT_create(&curTrace, Global_GTNAME);

        /* Call the generated init function */
        ti_sdo_ce_osal_bios_init();
    }

    SYS->PUTCFXN = (Fxn)Trace_putc;
}

/*
 *  ======== Global_setSpecialTrace ========
 *  Reads any special traces and takes appropriate action
 */
Void Global_setSpecialTrace(String mask)
{
    static GT_Mask trcTrace;
    static Bool    trcTraceInitialized = FALSE;
    UInt32         trcMask, i;

    GT_1trace(curTrace, GT_ENTER, "Global_setSpecialTrace> "
        "enter(mask='%s')\n", mask );

    /* don't do anything if the mask doesn't mention ti.bios explicitly */
    if (!strstr(mask, GTBIOSNAME)) {
        goto setSpecialTrace_return;
    }

    GT_0trace(curTrace, GT_2CLASS, "Global_setSpecialTrace> "
        "checking if initialized...\n" );

    if (trcTraceInitialized == FALSE) {
        Bool  maskExistsAlready;
        UInt8 mask;

        /* first check if the ti.bios mask has been set in GT already */
        maskExistsAlready = GT_getModNameFlags( GTBIOSNAME, &mask );
        trcMask = mask;

        GT_2trace( curTrace, GT_2CLASS, "Global_setSpecialTrace> "
            "not initialized yet; existsAlready=%d, trcMask=0x%x\n",
            maskExistsAlready, trcMask );

        /* now create the GT_Mask object for ti.bios */
        GT_create( &trcTrace, GTBIOSNAME );

        /* if the mask didn't exist, set it to the default */
        if (maskExistsAlready == FALSE) {
            GT_set( GTBIOSNAME "=" );
        }

        trcTraceInitialized = TRUE;
    }

    trcMask = GT_query( trcTrace, 0xFF );

    GT_1trace( curTrace, GT_2CLASS, "Global_setSpecialTrace> "
        "query returned trcMask=0x%x\n", trcMask );

    /* mapping 0-7 trace levels of "ti.bios" to TRC:
     *   - all cleared
     * 0 - TRC_LOGCLK
     * 1 - TRC_LOGPRD
     * 2 - TRC_LOGSWI
     * 3 - TRC_LOGTSK
     * 4 - TRC_STSHWI
     * 5 - TRC_STSPRD | TRC_STSSWI | TRC_STSTSK
     * 6 - TRC_USER0
     * 7 - TRC_USER1
     */

    TRC_disable(~0);

    if (trcMask & 0x01) {
        TRC_enable( TRC_LOGCLK );
    }
    if (trcMask & 0x02) {
        TRC_enable( TRC_LOGPRD );
    }
    if (trcMask & 0x04) {
        TRC_enable( TRC_LOGSWI );
    }
    if (trcMask & 0x08) {
        TRC_enable( TRC_LOGTSK );
    }
    if (trcMask & 0x10) {
        TRC_enable( TRC_STSHWI );
    }
    if (trcMask & 0x20) {
        TRC_enable( TRC_STSPRD | TRC_STSSWI | TRC_STSTSK );
    }
    if (trcMask & 0x40) {
        TRC_enable( TRC_USER0 );
    }
    if (trcMask & 0x80) {
        TRC_enable( TRC_USER1 );
    }
    if (trcMask != 0) {
        TRC_enable( TRC_GBLTARG | TRC_GBLHOST );
    }

setSpecialTrace_return:

    /* we have to sneak dumping the build info trace here, as we're sure
     * this function is called when DSP trace is set, but we want to do it
     * only once.
     */
    if (GT_query( curTrace, GT_4CLASS )) {
        if (buildInfoPrinted != TRUE) {
            GT_0trace(curTrace, GT_4CLASS, "Global_setSpecialTrace> "
                "This program was built with the following packages:\n" );
            for (i = 0; Global_buildInfo[i] != NULL; i++) {
                GT_1trace(curTrace, GT_4CLASS, "%s\n", Global_buildInfo[i]);
            }
            buildInfoPrinted = TRUE;
        }
    }

    GT_0trace(curTrace, GT_ENTER, "Global_setSpecialTrace> "
        "return\n" );
}
/*
 *  @(#) ti.sdo.ce.osal.bios; 2, 0, 1,182; 12-2-2010 21:24:42; /db/atree/library/trees/ce/ce-r11x/src/ xlibrary

 */

