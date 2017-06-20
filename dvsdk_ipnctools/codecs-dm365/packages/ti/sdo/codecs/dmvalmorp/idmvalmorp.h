/*
//============================================================================
//    FILE NAME : idmvalmorp.h
//    ALGORITHM : DMVALMORP
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the DMVALMORP code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IDMVALMORP_
#define _IDMVALMORP_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IDMVALMORP_GETSTATUS      XDM_GETSTATUS
#define IDMVALMORP_SETPARAMS      XDM_SETPARAMS
#define IDMVALMORP_RESET          XDM_RESET
#define IDMVALMORP_FLUSH          XDM_FLUSH
#define IDMVALMORP_SETDEFAULT     XDM_SETDEFAULT
#define IDMVALMORP_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IDMVALMORP_Handle ========
     *  This handle is used to reference all DMVALMORP instance objects
     */
    typedef struct IDMVALMORP_Obj *IDMVALMORP_Handle;

    /*
     *  ======== IDMVALMORP_Obj ========
     *  This structure must be the first field of all DMVALMORP instance objects
     */
    typedef struct IDMVALMORP_Obj {

        struct IDMVALMORP_Fxns *fxns;

    } IDMVALMORP_Obj;


    /*
    // ===========================================================================
    // IDMVALMORP_Params
    //
    // This structure defines the creation parameters for all DMVALMORP objects
    */
    typedef struct IDMVALMORP_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to DMVALMORP Encoder
        XDAS_Int32 subWindowHeight;     /* Height of the Subwindow       */
        XDAS_Int32 subWindowWidth;      /*  Width of the Subwindow     */
        XDAS_UInt8 MorphFunc;
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_Int32 askIMCOPRes;    /*  Explicitly ask for IMCOP Resource     */
        XDAS_Int32 sysBaseAddr;

    } IDMVALMORP_Params;

    /*
    // ===========================================================================
    // IDMVALMORP_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IDMVALMORP_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;
        XDAS_UInt8 MorphFunc;

    }IDMVALMORP_DynamicParams;
    /*
     *  ======== IDMVALMORP_PARAMS ========
     *  Default parameter values for DMVALMORP instance objects
     */
    extern IDMVALMORP_Params IDMVALMORP_PARAMS;

    /*
    // ===========================================================================
    // IDMVALMORP_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IDMVALMORP_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IDMVALMORP_Status;

    /*
     *  ======== IDMVALMORP_InArgs ========
     *  This structure defines the runtime input arguments for IDMVALMORP::process
     */

    typedef struct IDMVALMORP_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IDMVALMORP_InArgs;

    /*
     *  ======== IDMVALMORP_OutArgs ========
     *  This structure defines the run time output arguments for IDMVALMORP::process
     *  function.
     */

    typedef struct IDMVALMORP_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
    } IDMVALMORP_OutArgs;

    /*
     *  ======== IDMVALMORP_Cmd ========
     *  This structure defines the control commands for the IDMVALMORP module.
     */
    typedef IVIDENC1_Cmd IDMVALMORP_Cmd;

    /*
     *  ======== IDMVALMORP_Fxns ========
     *  This structure defines all of the operations on IDMVALMORP objects.
     */

    typedef struct IDMVALMORP_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IDMVALMORP extends IVIDENC object.

    } IDMVALMORP_Fxns;

    typedef enum {
        IDMVALMORP_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IDMVALMORP_ERROR;


    extern IDMVALMORP_Fxns DMVALMORP_TI_IDMVALMORP;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DMVALMORP_EDMA_CHANNELS 4 //3 AYK - 0813
#define NUM_DMVALMORP_EDMA_PARAMS   (NUM_DMVALMORP_EDMA_CHANNELS*2)

#endif    /* _IDMVALMORP_ */

/* ======================================================================== */
/* End of file : idmvalmorp.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

