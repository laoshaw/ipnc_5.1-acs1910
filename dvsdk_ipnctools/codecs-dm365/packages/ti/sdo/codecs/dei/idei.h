/*
//============================================================================
//    FILE NAME : idei.h
//    ALGORITHM : DE-Interlacer
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the DEI code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IDEI_
#define _IDEI_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IDEI_GETSTATUS      XDM_GETSTATUS
#define IDEI_SETPARAMS      XDM_SETPARAMS
#define IDEI_RESET          XDM_RESET
#define IDEI_FLUSH          XDM_FLUSH
#define IDEI_SETDEFAULT     XDM_SETDEFAULT
#define IDEI_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IDEI_Handle ========
     *  This handle is used to reference all DEI instance objects
     */
    typedef struct IDEI_Obj *IDEI_Handle;

    /*
     *  ======== IDEI_Obj ========
     *  This structure must be the first field of all DEI instance objects
     */
    typedef struct IDEI_Obj {

        struct IDEI_Fxns *fxns;

    } IDEI_Obj;


    /*
    // ===========================================================================
    // IDEI_Params
    //
    // This structure defines the creation parameters for all DEI objects
    */
    typedef struct IDEI_Params {

        IVIDENC1_Params videncParams; 		/* must be followed for all video encoders */

        // CreateParams specific to DEI Encoder
        XDAS_Int32 frameHeight;     		/* Height of the frame */
        XDAS_Int32 frameWidth;      		/* Width of the frame */
		XDAS_Int32 inLineOffset;           	/* Input Line Offset */ 		
		XDAS_Int32 outLineOffset;          	/* Output Line Offset */ 		
        XDAS_Int32 threshold_low;           /* DE-Interlacer Low Threshold */
        XDAS_Int32 threshold_high;          /* DE-Interlacer High Threshold */		
		XDAS_Int32 inputFormat;  			/* DE-Interlacer Input Format */
        XDAS_Int32 outputFormat;  			/* DE-Interlacer Output Format */
		
        XDAS_Int32 q_num;               	/* EDMA Queue Number */
        XDAS_Int32 askIMCOPRes;    			/* Explicitly ask for IMCOP Resource */
        XDAS_Int32 sysBaseAddr;

    } IDEI_Params;

    /*
    // ===========================================================================
    // IDEI_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IDEI_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;
		
        XDAS_Int32 frameHeight;     		/* Height of the frame */
        XDAS_Int32 frameWidth;      		/* Width of the frame */
		XDAS_Int32 inLineOffset;           	/* Input Line Offset */ 		
		XDAS_Int32 outLineOffset;          	/* Output Line Offset */ 		
        XDAS_Int32 threshold_low;           /* DE-Interlacer Low Threshold */
        XDAS_Int32 threshold_high;          /* DE-Interlacer High Threshold */		
		XDAS_Int32 inputFormat;  			/* DE-Interlacer Input Format */
        XDAS_Int32 outputFormat;  			/* DE-Interlacer Output Format */		

    }IDEI_DynamicParams;
    /*
     *  ======== IDEI_PARAMS ========
     *  Default parameter values for DEI instance objects
     */
    extern IDEI_Params IDEI_PARAMS;

    /*
    // ===========================================================================
    // IDEI_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IDEI_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IDEI_Status;

    /*
     *  ======== IDEI_InArgs ========
     *  This structure defines the runtime input arguments for IDEI::process
     */

    typedef struct IDEI_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IDEI_InArgs;

    /*
     *  ======== IDEI_OutArgs ========
     *  This structure defines the run time output arguments for IDEI::process
     *  function.
     */

    typedef struct IDEI_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
		XDAS_Int32 appliedTh_h;
		XDAS_Int32 appliedTh_l;
		
    } IDEI_OutArgs;

    /*
     *  ======== IDEI_Cmd ========
     *  This structure defines the control commands for the IDEI module.
     */
    typedef IVIDENC1_Cmd IDEI_Cmd;

    /*
     *  ======== IDEI_Fxns ========
     *  This structure defines all of the operations on IDEI objects.
     */

    typedef struct IDEI_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IDEI extends IVIDENC object.

    } IDEI_Fxns;

    typedef enum {
        IDEI_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IDEI_ERROR;


    extern IDEI_Fxns DEI_TI_IDEI;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_DEI_EDMA_CHANNELS 	(6) 	
#define NUM_DEI_EDMA_PARAMS   	(NUM_DEI_EDMA_CHANNELS * 2)

#endif    /* _IDEI_ */

/* ======================================================================== */
/* End of file : idei.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

