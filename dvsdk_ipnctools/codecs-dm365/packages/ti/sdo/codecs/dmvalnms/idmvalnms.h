/*
//============================================================================
//    FILE NAME : idmvalnms.h
//    ALGORITHM : DMVALNMS
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the DMVALNMS code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IDMVALNMS_
#define _IDMVALNMS_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IDMVALNMS_GETSTATUS      XDM_GETSTATUS
#define IDMVALNMS_SETPARAMS      XDM_SETPARAMS
#define IDMVALNMS_RESET          XDM_RESET
#define IDMVALNMS_FLUSH          XDM_FLUSH
#define IDMVALNMS_SETDEFAULT     XDM_SETDEFAULT
#define IDMVALNMS_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IDMVALNMS_Handle ========
     *  This handle is used to reference all DMVALNMS instance objects
     */
    typedef struct IDMVALNMS_Obj *IDMVALNMS_Handle;

    /*
     *  ======== IDMVALNMS_Obj ========
     *  This structure must be the first field of all DMVALNMS instance objects
     */
    typedef struct IDMVALNMS_Obj {

        struct IDMVALNMS_Fxns *fxns;

    } IDMVALNMS_Obj;


    /*
    // ===========================================================================
    // IDMVALNMS_Params
    //
    // This structure defines the creation parameters for all DMVALNMS objects
    */
    typedef struct IDMVALNMS_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to DMVALNMS Encoder
        XDAS_Int32 subWindowHeight;     /* Height of the Subwindow       */
        XDAS_Int32 subWindowWidth;      /*  Width of the Subwindow     */
        XDAS_Int32 outputFormat;  /*  DMVALNMS Output Format     */
        XDAS_Int32 filterWidth;
        XDAS_Int32 threshold;
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_Int32 askIMCOPRes;    /*  Explicitly ask for IMCOP Resource     */
        XDAS_Int32 sysBaseAddr;

    } IDMVALNMS_Params;

    /*
    // ===========================================================================
    // IDMVALNMS_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IDMVALNMS_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;
        XDAS_Int32 filterWidth;    /* filter width 13,15,17 */
        XDAS_Int32 threshold;      /* threshold  */

    }IDMVALNMS_DynamicParams;
    /*
     *  ======== IDMVALNMS_PARAMS ========
     *  Default parameter values for DMVALNMS instance objects
     */
    extern IDMVALNMS_Params IDMVALNMS_PARAMS;

    /*
    // ===========================================================================
    // IDMVALNMS_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IDMVALNMS_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IDMVALNMS_Status;

    /*
     *  ======== IDMVALNMS_InArgs ========
     *  This structure defines the runtime input arguments for IDMVALNMS::process
     */

    typedef struct IDMVALNMS_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IDMVALNMS_InArgs;

    /*
     *  ======== IDMVALNMS_OutArgs ========
     *  This structure defines the run time output arguments for IDMVALNMS::process
     *  function.
     */

    typedef struct IDMVALNMS_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
    } IDMVALNMS_OutArgs;

    /*
     *  ======== IDMVALNMS_Cmd ========
     *  This structure defines the control commands for the IDMVALNMS module.
     */
    typedef IVIDENC1_Cmd IDMVALNMS_Cmd;

    /*
     *  ======== IDMVALNMS_Fxns ========
     *  This structure defines all of the operations on IDMVALNMS objects.
     */

    typedef struct IDMVALNMS_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IDMVALNMS extends IVIDENC object.

    } IDMVALNMS_Fxns;

    typedef enum {
        IDMVALNMS_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IDMVALNMS_ERROR;


    extern IDMVALNMS_Fxns DMVALNMS_TI_IDMVALNMS;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DMVALNMS_EDMA_CHANNELS 6 //3 AYK - 0813
#define NUM_DMVALNMS_EDMA_PARAMS   (NUM_DMVALNMS_EDMA_CHANNELS*2)

#endif    /* _IDMVALNMS_ */

/* ======================================================================== */
/* End of file : idmvalnms.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

