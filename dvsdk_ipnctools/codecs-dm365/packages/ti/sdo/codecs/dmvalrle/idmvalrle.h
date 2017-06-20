/*
//============================================================================
//    FILE NAME : idmvalrle.h
//    ALGORITHM : DMVALRLE
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the DMVALRLE code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IDMVALRLE_
#define _IDMVALRLE_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IDMVALRLE_GETSTATUS      XDM_GETSTATUS
#define IDMVALRLE_SETPARAMS      XDM_SETPARAMS
#define IDMVALRLE_RESET          XDM_RESET
#define IDMVALRLE_FLUSH          XDM_FLUSH
#define IDMVALRLE_SETDEFAULT     XDM_SETDEFAULT
#define IDMVALRLE_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IDMVALRLE_Handle ========
     *  This handle is used to reference all DMVALRLE instance objects
     */
    typedef struct IDMVALRLE_Obj *IDMVALRLE_Handle;

    /*
     *  ======== IDMVALRLE_Obj ========
     *  This structure must be the first field of all DMVALRLE instance objects
     */
    typedef struct IDMVALRLE_Obj {

        struct IDMVALRLE_Fxns *fxns;

    } IDMVALRLE_Obj;


    /*
    // ===========================================================================
    // IDMVALRLE_Params
    //
    // This structure defines the creation parameters for all DMVALRLE objects
    */
    typedef struct IDMVALRLE_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to DMVALRLE Encoder
        XDAS_Int32 subWindowHeight;     /* Height of the Subwindow       */
        XDAS_Int32 subWindowWidth;      /*  Width of the Subwindow     */
        XDAS_Int32 outputFormat;  /*  DMVALRLE Output Format     */
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_Int32 askIMCOPRes;    /*  Explicitly ask for IMCOP Resource     */
        XDAS_Int32 sysBaseAddr;

    } IDMVALRLE_Params;

    /*
    // ===========================================================================
    // IDMVALRLE_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IDMVALRLE_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;

    }IDMVALRLE_DynamicParams;
    /*
     *  ======== IDMVALRLE_PARAMS ========
     *  Default parameter values for DMVALRLE instance objects
     */
    extern IDMVALRLE_Params IDMVALRLE_PARAMS;

    /*
    // ===========================================================================
    // IDMVALRLE_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IDMVALRLE_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IDMVALRLE_Status;

    /*
     *  ======== IDMVALRLE_InArgs ========
     *  This structure defines the runtime input arguments for IDMVALRLE::process
     */

    typedef struct IDMVALRLE_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IDMVALRLE_InArgs;

    /*
     *  ======== IDMVALRLE_OutArgs ========
     *  This structure defines the run time output arguments for IDMVALRLE::process
     *  function.
     */

    typedef struct IDMVALRLE_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
		XDAS_UInt32 numOfTransitions;
		XDAS_Int16 *ptrEndOfRLEcontent;

    } IDMVALRLE_OutArgs;

    /*
     *  ======== IDMVALRLE_Cmd ========
     *  This structure defines the control commands for the IDMVALRLE module.
     */
    typedef IVIDENC1_Cmd IDMVALRLE_Cmd;

    /*
     *  ======== IDMVALRLE_Fxns ========
     *  This structure defines all of the operations on IDMVALRLE objects.
     */

    typedef struct IDMVALRLE_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IDMVALRLE extends IVIDENC object.

    } IDMVALRLE_Fxns;

    typedef enum {
        IDMVALRLE_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IDMVALRLE_ERROR;


    extern IDMVALRLE_Fxns DMVALRLE_TI_IDMVALRLE;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DMVALRLE_EDMA_CHANNELS 6 //3 AYK - 0813
#define NUM_DMVALRLE_EDMA_PARAMS   (NUM_DMVALRLE_EDMA_CHANNELS*2)

#endif    /* _IDMVALRLE_ */

/* ======================================================================== */
/* End of file : idmvalrle.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

