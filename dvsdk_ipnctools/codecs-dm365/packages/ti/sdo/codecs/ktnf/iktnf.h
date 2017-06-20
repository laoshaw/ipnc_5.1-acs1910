/*
//============================================================================
//    FILE NAME : iktnf.h
//    ALGORITHM : KTNF
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the KTNF
//               Encoder code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IKTNF_
#define _IKTNF_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IKTNF_GETSTATUS      XDM_GETSTATUS
#define IKTNF_SETPARAMS      XDM_SETPARAMS
#define IKTNF_RESET          XDM_RESET
#define IKTNF_FLUSH          XDM_FLUSH
#define IKTNF_SETDEFAULT     XDM_SETDEFAULT
#define IKTNF_GETBUFINFO     XDM_GETBUFINFO

typedef enum {
	IKTNF_NO_NF     = 0,
	IKTNF_ONLY_KNF  = 1,
	IKTNF_ONLY_TNF  = 2,
	IKTNF_KNF_TNF   = 3,
	IKTNF_KTNF      = 4,
	IKTNF_FAST_TNF  = 5,
	IKTNF_TNF3      = 6,
	IKTNF_TNF3_ONLY = 7,
	IKTNF_MAX

} IKTNF_Type;

    /*
     *  ======== IKTNF_Handle ========
     *  This handle is used to reference all KTNF instance objects
     */
    typedef struct IKTNF_Obj *IKTNF_Handle;

    /*
     *  ======== IKTNF_Obj ========
     *  This structure must be the first field of all KTNF instance objects
     */
    typedef struct IKTNF_Obj {

        struct IKTNF_Fxns *fxns;

    } IKTNF_Obj;


    /*
    // ===========================================================================
    // IKTNF_Params
    //
    // This structure defines the creation parameters for all KTNF objects
    */
    typedef struct IKTNF_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to KTNF Encoder
        XDAS_Int32 subWindowHeight;     /* Height of the Subwindow       */
        XDAS_Int32 subWindowWidth;      /*  Width of the Subwindow     */
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_Int32 askIMCOPRes;    /*  Explicitly ask for IMCOP Resource     */
        XDAS_Int32 sysBaseAddr;

		IKTNF_Type EnableNSF;

		/* Some HWNF parameters exposed */
		/* Offset: U10Q2 (bits 0-9), Slope: S9Q10 (bits 0-8) */
	    XDAS_Int32   nsf_thr00;//color 0 level 1 offset
	    XDAS_Int32   nsf_thr01;//color 0 level 1 slope
	    XDAS_Int32   nsf_thr02;//color 0 level 2 offset
	    XDAS_Int32   nsf_thr03;//color 0 level 2 slope
	    XDAS_Int32   nsf_thr04;//color 0 level 3 offset
	    XDAS_Int32   nsf_thr05;//color 0 level 3 slope
	    XDAS_Int32   nsf_thr10;//color 1 level 1 offset
	    XDAS_Int32   nsf_thr11;//color 1 level 1 slope
	    XDAS_Int32   nsf_thr12;//color 1 level 2 offset
	    XDAS_Int32   nsf_thr13;//color 1 level 2 slope
	    XDAS_Int32   nsf_thr14;//color 1 level 3 offset
	    XDAS_Int32   nsf_thr15;//color 1 level 3 slope
	    XDAS_Int32   nsf_thr20;//color 2 level 1 offset
	    XDAS_Int32   nsf_thr21;//color 2 level 1 slope
	    XDAS_Int32   nsf_thr22;//color 2 level 2 offset
	    XDAS_Int32   nsf_thr23;//color 2 level 2 slope
	    XDAS_Int32   nsf_thr24;//color 2 level 3 offset
	    XDAS_Int32   nsf_thr25;//color 2 level 3 slope
	    XDAS_Int32   nsf_sft_slope;//nsf sft slope
	    XDAS_Int32   nsf_ee_l1_slope;//nsf
	    XDAS_Int32   nsf_ee_l1_thr1;//nsf
	    XDAS_Int32   nsf_ee_l1_thr2;//nsf
	    XDAS_Int32   nsf_ee_l1_ofst2;//nsf
	    XDAS_Int32   nsf_ee_l2_slope;//nsf
	    XDAS_Int32   nsf_ee_l2_thr1;//nsf
	    XDAS_Int32   nsf_ee_l2_thr2;//nsf
	    XDAS_Int32   nsf_ee_l2_ofst2;//nsf
	    XDAS_Int32   nsf_ee_l3_slope;//nsf
	    XDAS_Int32   nsf_ee_l3_thr1;//nsf
	    XDAS_Int32   nsf_ee_l3_thr2;//nsf
	    XDAS_Int32   nsf_ee_l3_ofst2;//nsf


		XDAS_Int32 TNF_A0;		/* Control Param: 0 to 255  */
		XDAS_Int32 TNF_TM;		/* Control Param: 1 to 255  */
		XDAS_Int32 TNFLuma;		/* TNF applied on Luma Only */

		//Place holder for the TNF3 Parameters
		XDAS_Int32 tnf3TS;
		XDAS_Int32 unDiffScaleFactor;
		XDAS_Int32 unMotionThreshold;
		XDAS_Int32 unStrengthOfTNF3;

    } IKTNF_Params;

    /*
    // ===========================================================================
    // IKTNF_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IKTNF_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;

		IKTNF_Type EnableNSF;
		/* Some HWNF parameters exposed */
		/* Offset: U10Q2 (bits 0-9), Slope: S9Q10 (bits 0-8) */
	    XDAS_Int32   nsf_thr00;//color 0 level 1 offset
	    XDAS_Int32   nsf_thr01;//color 0 level 1 slope
	    XDAS_Int32   nsf_thr02;//color 0 level 2 offset
	    XDAS_Int32   nsf_thr03;//color 0 level 2 slope
	    XDAS_Int32   nsf_thr04;//color 0 level 3 offset
	    XDAS_Int32   nsf_thr05;//color 0 level 3 slope
	    XDAS_Int32   nsf_thr10;//color 1 level 1 offset
	    XDAS_Int32   nsf_thr11;//color 1 level 1 slope
	    XDAS_Int32   nsf_thr12;//color 1 level 2 offset
	    XDAS_Int32   nsf_thr13;//color 1 level 2 slope
	    XDAS_Int32   nsf_thr14;//color 1 level 3 offset
	    XDAS_Int32   nsf_thr15;//color 1 level 3 slope
	    XDAS_Int32   nsf_thr20;//color 2 level 1 offset
	    XDAS_Int32   nsf_thr21;//color 2 level 1 slope
	    XDAS_Int32   nsf_thr22;//color 2 level 2 offset
	    XDAS_Int32   nsf_thr23;//color 2 level 2 slope
	    XDAS_Int32   nsf_thr24;//color 2 level 3 offset
	    XDAS_Int32   nsf_thr25;//color 2 level 3 slope
	    XDAS_Int32   nsf_sft_slope;//nsf sft slope
	    XDAS_Int32   nsf_ee_l1_slope;//nsf
	    XDAS_Int32   nsf_ee_l1_thr1;//nsf
	    XDAS_Int32   nsf_ee_l1_thr2;//nsf
	    XDAS_Int32   nsf_ee_l1_ofst2;//nsf
	    XDAS_Int32   nsf_ee_l2_slope;//nsf
	    XDAS_Int32   nsf_ee_l2_thr1;//nsf
	    XDAS_Int32   nsf_ee_l2_thr2;//nsf
	    XDAS_Int32   nsf_ee_l2_ofst2;//nsf
	    XDAS_Int32   nsf_ee_l3_slope;//nsf
	    XDAS_Int32   nsf_ee_l3_thr1;//nsf
	    XDAS_Int32   nsf_ee_l3_thr2;//nsf
	    XDAS_Int32   nsf_ee_l3_ofst2;//nsf


		XDAS_Int32 TNF_A0;		/* Control Param: 0 to 255  */
		XDAS_Int32 TNF_TM;		/* Control Param: 1 to 255  */
		XDAS_Int32 TNFLuma;		/* TNF applied on Luma Only */

		//Place holder for the TNF3 Parameters
		XDAS_Int32 tnf3TS;
		XDAS_Int32 unDiffScaleFactor;
		XDAS_Int32 unMotionThreshold;
		XDAS_Int32 unStrengthOfTNF3;

    }IKTNF_DynamicParams;
    /*
     *  ======== IKTNF_PARAMS ========
     *  Default parameter values for KTNF instance objects
     */
    extern IKTNF_Params IKTNF_PARAMS;

    /*
    // ===========================================================================
    // IKTNF_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IKTNF_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */
		XDAS_Int32 outPitch;

    } IKTNF_Status;

    /*
     *  ======== IKTNF_InArgs ========
     *  This structure defines the runtime input arguments for IKTNF::process
     */

    typedef struct IKTNF_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IKTNF_InArgs;

    /*
     *  ======== IKTNF_OutArgs ========
     *  This structure defines the run time output arguments for IKTNF::process
     *  function.
     */

    typedef struct IKTNF_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;

        XDAS_Int32 outWidth;
        XDAS_Int32 outHeight;
        XDAS_Int32 skipOutputLumaRows;
        XDAS_Int32 skipOutputChromaRows;
        XDAS_Int32 outPitch;
    } IKTNF_OutArgs;

    /*
     *  ======== IKTNF_Cmd ========
     *  This structure defines the control commands for the IKTNF module.
     */
    typedef IVIDENC1_Cmd IKTNF_Cmd;

    /*
     *  ======== IKTNF_Fxns ========
     *  This structure defines all of the operations on IKTNF objects.
     */

    typedef struct IKTNF_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IKTNF extends IVIDENC object.

    } IKTNF_Fxns;

    typedef enum {
        IKTNF_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IKTNF_ERROR;


    extern IKTNF_Fxns KTNF_TI_IKTNF;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_NF_EDMA_CHANNELS 6
#define NUM_NF_EDMA_PARAMS   (NUM_NF_EDMA_CHANNELS*2)

#endif    /* _IKTNF_ */

/* ======================================================================== */
/* End of file : iktnf.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

