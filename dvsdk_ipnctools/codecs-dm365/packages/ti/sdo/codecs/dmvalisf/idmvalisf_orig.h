/*
//============================================================================
//    FILE NAME : idmvalisf.h
//    ALGORITHM : DMVALISF
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the DMVALISF code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IDMVALISF_
#define _IDMVALISF_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IDMVALISF_GETSTATUS      XDM_GETSTATUS
#define IDMVALISF_SETPARAMS      XDM_SETPARAMS
#define IDMVALISF_RESET          XDM_RESET
#define IDMVALISF_FLUSH          XDM_FLUSH
#define IDMVALISF_SETDEFAULT     XDM_SETDEFAULT
#define IDMVALISF_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IDMVALISF_Handle ========
     *  This handle is used to reference all DMVALISF instance objects
     */
    typedef struct IDMVALISF_Obj *IDMVALISF_Handle;

    /*
     *  ======== IDMVALISF_Obj ========
     *  This structure must be the first field of all DMVALISF instance objects
     */
    typedef struct IDMVALISF_Obj {

        struct IDMVALISF_Fxns *fxns;

    } IDMVALISF_Obj;


    /*
    // ===========================================================================
    // IDMVALISF_Params
    //
    // This structure defines the creation parameters for all DMVALISF objects
    */
    typedef struct IDMVALISF_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to DMVALISF Encoder
        XDAS_Int32 subWindowHeight;     /* Height of the Subwindow       */
        XDAS_Int32 subWindowWidth;      /*  Width of the Subwindow     */
//        XDAS_Int32 outputFormat;  /*  DMVALISF Output Format     */
        XDAS_Int32 imageDiffThreshold;    /* General value: 0 */
        XDAS_Int32 basicThreshold_8;      /* threshold for number of motion pixels */
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_Int32 askIMCOPRes;           /*  Explicitly ask for IMCOP Resource     */

    } IDMVALISF_Params;

    /*
    // ===========================================================================
    // IDMVALISF_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IDMVALISF_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;
        XDAS_Int32 imageDiffThreshold;    /* General value: 0 */
        XDAS_Int32 basicThreshold_8;      /* threshold for number of motion pixels */

    }IDMVALISF_DynamicParams;
    /*
     *  ======== IDMVALISF_PARAMS ========
     *  Default parameter values for DMVALISF instance objects
     */
    extern IDMVALISF_Params IDMVALISF_PARAMS;

    /*
    // ===========================================================================
    // IDMVALISF_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IDMVALISF_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IDMVALISF_Status;

    /*
     *  ======== IDMVALISF_InArgs ========
     *  This structure defines the runtime input arguments for IDMVALISF::process
     */

    typedef struct IDMVALISF_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IDMVALISF_InArgs;

    /*
     *  ======== IDMVALISF_OutArgs ========
     *  This structure defines the run time output arguments for IDMVALISF::process
     *  function.
     */

    typedef struct IDMVALISF_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
    } IDMVALISF_OutArgs;

    /*
     *  ======== IDMVALISF_Cmd ========
     *  This structure defines the control commands for the IDMVALISF module.
     */
    typedef IVIDENC1_Cmd IDMVALISF_Cmd;

    /*
     *  ======== IDMVALISF_Fxns ========
     *  This structure defines all of the operations on IDMVALISF objects.
     */

    typedef struct IDMVALISF_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IDMVALISF extends IVIDENC object.

    } IDMVALISF_Fxns;

    typedef enum {
        IDMVALISF_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IDMVALISF_ERROR;


    extern IDMVALISF_Fxns DMVALISF_TI_IDMVALISF;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DMVALISF_EDMA_CHANNELS 12 //3 AYK - 0813
#define NUM_DMVALISF_EDMA_PARAMS   (NUM_DMVALISF_EDMA_CHANNELS*2)

#endif    /* _IDMVALISF_ */

/* ======================================================================== */
/* End of file : idmvalisf.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

