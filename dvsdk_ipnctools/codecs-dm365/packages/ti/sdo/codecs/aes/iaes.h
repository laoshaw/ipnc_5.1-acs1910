/*
//============================================================================
//    FILE NAME : iaes.h
//    ALGORITHM : AES
//    VENDOR    : TI
//    TARGET Proc: DM365
//    PURPOSE   : This is the top level driver file that exercises the AES code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2009 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
#ifndef _IAES_
#define _IAES_

//#include <xdc/std.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividenc1.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//!< control method commands
#define IAES_GETSTATUS      XDM_GETSTATUS
#define IAES_SETPARAMS      XDM_SETPARAMS
#define IAES_RESET          XDM_RESET
#define IAES_FLUSH          XDM_FLUSH
#define IAES_SETDEFAULT     XDM_SETDEFAULT
#define IAES_GETBUFINFO     XDM_GETBUFINFO


    /*
     *  ======== IAES_Handle ========
     *  This handle is used to reference all AES instance objects
     */
    typedef struct IAES_Obj *IAES_Handle;

    /*
     *  ======== IAES_Obj ========
     *  This structure must be the first field of all AES instance objects
     */
    typedef struct IAES_Obj {

        struct IAES_Fxns *fxns;

    } IAES_Obj;


    /*
    // ===========================================================================
    // IAES_Params
    //
    // This structure defines the creation parameters for all AES objects
    */
    typedef struct IAES_Params {

        IVIDENC1_Params videncParams; /* must be followed for all video encoders */

        // CreateParams specific to AES Encoder
		XDAS_Int32 AES_bigEndian; 		/*  AES_bigEndian... 0: Little Endian, 1: Big Endian, key is always little endian */
        XDAS_Int32 AES_keySize;         /*  AES_keySize... 0: 128 bit, 1: 192 bit, 2:256 bit */
        XDAS_Int32 AES_encrypt;         /*  AES_encrypt... 1: Encrypt, 0: Decrypt     */
        XDAS_Int32 q_num;               /*  EDMA Queue Number     */
        XDAS_UInt8 roundKey[32];        /*  ROUND KEY     */
        XDAS_Int32 askIMCOPRes;    /*  Explicitly ask for IMCOP Resource     */		
        XDAS_Int32 sysBaseAddr;

    } IAES_Params;

    /*
    // ===========================================================================
    // IAES_DynamicParams
    // This structure defines the parameters that can be modified run time
    */

    typedef struct IAES_DynamicParams
    {
        IVIDENC1_DynamicParams videncDynamicParams;

    }IAES_DynamicParams;
    /*
     *  ======== IAES_PARAMS ========
     *  Default parameter values for AES instance objects
     */
    extern IAES_Params IAES_PARAMS;

    /*
    // ===========================================================================
    // IAES_Status
    //
    // Status structure defines the parameters that can be changed or read
    // during real-time operation of the alogrithm.
    */

    typedef struct IAES_Status
    {

        IVIDENC1_Status  videncStatus;	/* must be followed for video encoder */

        /* TODO : add on  algorithm & vendor specific status here */

    } IAES_Status;

    /*
     *  ======== IAES_InArgs ========
     *  This structure defines the runtime input arguments for IAES::process
     */

    typedef struct IAES_InArgs
    {

        IVIDENC1_InArgs videncInArgs;

    } IAES_InArgs;

    /*
     *  ======== IAES_OutArgs ========
     *  This structure defines the run time output arguments for IAES::process
     *  function.
     */

    typedef struct IAES_OutArgs
    {

        IVIDENC1_OutArgs videncOutArgs;
		XDAS_Int32 outDataSize;

    } IAES_OutArgs;

    /*
     *  ======== IAES_Cmd ========
     *  This structure defines the control commands for the IAES module.
     */
    typedef IVIDENC1_Cmd IAES_Cmd;

    /*
     *  ======== IAES_Fxns ========
     *  This structure defines all of the operations on IAES objects.
     */

    typedef struct IAES_Fxns
    {

        IVIDENC1_Fxns	ividenc;    // IAES extends IVIDENC object.

    } IAES_Fxns;

    typedef enum {
        IAES_INVALID_PARAMETER  = 1            /* Invalid Image Width*/
    } IAES_ERROR;


    extern IAES_Fxns AES_TI_IAES;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define NUM_AES_EDMA_CHANNELS 2 
#define NUM_AES_EDMA_PARAMS   (NUM_AES_EDMA_CHANNELS*2)

#endif    /* _IAES_ */

/* ======================================================================== */
/* End of file : iaes.h                                               */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

