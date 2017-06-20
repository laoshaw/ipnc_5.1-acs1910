/*
//============================================================================
//    FILE NAME : iiprun.h
//    ALGORITHM : IPRUN
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the XDM interface file for IP_run
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IIPRUN_
#define _IIPRUN_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#include "IP_run.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#ifndef ALG_IPRUN_NAME

#define ALG_IPRUN_NAME "iprun"

#endif

//!< control method commands
#define IIPRUN_GETSTATUS      XDM_GETSTATUS
#define IIPRUN_SETPARAMS      XDM_SETPARAMS
#define IIPRUN_RESET          XDM_RESET
#define IIPRUN_FLUSH          XDM_FLUSH
#define IIPRUN_SETDEFAULT     XDM_SETDEFAULT
#define IIPRUN_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IIPRUN_Handle ========
     *  This handle is used to reference all DMVALMORP instance objects
     */
    typedef struct IIPRUN_Obj *IIPRUN_Handle;

    /*
     *  ======== IIPRUN_Obj ========
     *  This structure must be the first field of all DMVALMORP instance objects
     */
    typedef struct IIPRUN_Obj {

        struct IIPRUN_Fxns *fxns;

    } IIPRUN_Obj;


    /*
    // ===========================================================================
    // IIPRUN_Params
    //
    // This structure defines the creation parameters for all DMVALMORP objects
    */
    typedef struct IIPRUN_Params {
        IVIDENC1_Params videncParams; /* must be followed for all video encoders */
        IP_RUN_InitParams iprunParams;
    } IIPRUN_Params;

    /*
    // ===========================================================================
    // IIPRUN_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IIPRUN_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;
        
    }IIPRUN_DynamicParams;
    /*
     *  ======== IIPRUN_PARAMS ========
     *  Default parameter values for DMVALMORP instance objects
     */
    extern IIPRUN_Params IIPRUN_PARAMS;

    /*
    // ===========================================================================
    // IIPRUN_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IIPRUN_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IIPRUN_Status;

    /*
     *  ======== IIPRUN_InArgs ========
     *  This structure defines the runtime input arguments for IIPRUN::process
     */

    typedef struct IIPRUN_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IIPRUN_InArgs;

    /*
     *  ======== IIPRUN_OutArgs ========
     *  This structure defines the run time output arguments for IIPRUN::process
     *  function.
     */

    typedef struct IIPRUN_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

    } IIPRUN_OutArgs;

    /*
     *  ======== IIPRUN_Cmd ========
     *  This structure defines the control commands for the IIPRUN module.
     */
    typedef IVIDENC1_Cmd IIPRUN_Cmd;

    /*
     *  ======== IIPRUN_Fxns ========
     *  This structure defines all of the operations on IIPRUN objects.
     */

    typedef struct IIPRUN_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IIPRUN extends IVIDENC object.

    } IIPRUN_Fxns;

    
    extern IIPRUN_Fxns IPRUN_TI_IIPRUN;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DMVALMORP_EDMA_CHANNELS 4 //3 AYK - 0813
#define NUM_DMVALMORP_EDMA_PARAMS   (NUM_DMVALMORP_EDMA_CHANNELS*2)

#endif    /* _IIPRUN_ */

/* ======================================================================== */
/* End of file : idmvalmorp.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

