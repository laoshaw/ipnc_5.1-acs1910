/** ==========================================================================
 *  @file   mpeg4vdec_ti_arm926testapp.h
 *
 *  @path   $(PROJDIR)\Inc
 *
 *  @desc   This is the top level client header file that drives the mp4
 *          (Simple Profile) Video Encoder Call using XDM Interface
 *
 *  ==========================================================================
 *  Copyright (c) Texas Instruments Inc 2006, 2007
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied
 * =========================================================================*/

/* -------------------- compilation control switches ---------------------- */

#ifndef _TESTAPP_DECODER_
#define _TESTAPP_DECODER_

#define ONELINE_CONFIG
#undef  ONELINE_CONFIG

#define XDM_RESET_TST
#undef  XDM_RESET_TST

#define TST_FRAME_MODE
#undef  TST_FRAME_MODE

/****************************************************************************
*   INCLUDE FILES
*****************************************************************************/
/* -------------------- system and platform files ------------------------- */
/* ------------------------- program files -------------------------------- */
#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/ivideo.h>
#include <ti/xdais/dm/ividdec2.h>
#include <imp4vdec.h>

/****************************************************************************
*   EXTERNAL REFERENCES NOTE : only use if not found in header file
*****************************************************************************/
/* ------------------------ data declarations ----------------------------- */
/* ----------------------- function prototypes ---------------------------- */

/****************************************************************************
*   PUBLIC DECLARATIONS Defined here, used elsewhere
*****************************************************************************/
/* ----------------------- data declarations ------------------------------ */

/** ==========================================================================
 *  @name   _sTokenMapping/sTokenMapping
 *
 *  @desc   Token Mapping structure for parsing codec specific
 *          configuration file
 *
 *  @field  *tokenName
 *          String name in the configuration file
 *
 *  @field  *place
 *          Place Holder for the data
 *
 *  @see    None
 *  ==========================================================================
 */
typedef struct _sTokenMapping
{
  xdc_Char *tokenName;
  XDAS_Void *place;
} sTokenMapping;

/* ---------------------- function prototypes ----------------------------- */

/* ===========================================================================
 *  @func   TestApp_EnableCache()
 *
 *  @desc   Enable cache settings for system
 *.
 *  @param  None.
 *
 *  @return None
 *
 *  @Notes: None.
 *
 * ===========================================================================
 */
XDAS_Void   TestApp_EnableCache(void);

/* ===========================================================================
 *  @func   TestApp_ReadByteStream()
 *
 *  @desc   Reads Input Data from FILE
 *.
 *  @param  XDAS_Int8 *.
 *          Buffer Descriptor for input buffers
 *
 *  @return Status of the operation ( PASS / FAIL )
 *
 *  @Notes: None.
 *
 * ===========================================================================
 */
#ifndef TST_FRAME_MODE
XDAS_Int32  TestApp_ReadByteStream(xdc_Char *);
#else
XDAS_Int32  TestApp_ReadByteStream(xdc_Char *,FILE      *,XDAS_Int32);
#endif

/* ===========================================================================
 *  @func   TestApp_WriteOutputData()
 *
 *  @desc   Writes Output Data into FILE
 *.
 *  @param  fOutFile *.
 *          Output File Handle
 *
 *  @param  XDAS_Int32 display_marker:
 *          Index into the display buffer array which is marked for
 *          display.
 *
 *  @param  IVIDDEC2_OutArgs *outArgs:
 *          OutArgs structure containing display buffer array and thier
 *          properties like picture pitch, heigth and width.
 *
 *  @param  XDAS_Int32  height:
 *          Height of the displayed picture in pixels.
 *
 *  @param  XDAS_Int32  width:
 *          Width of the displayed picture in pixels.
 *
 *  @return None.
 *
 *  @note   None
 * ===========================================================================
 */
XDAS_Void   TestApp_WriteOutputData(
                                    FILE             *fOutFile,
                                    XDAS_Int32       display_marker,
                                    IMP4HDVICPDEC_OutArgs *outArgs,
                                    XDAS_Int32       height,
                                    XDAS_Int32       width
                                   );


/* ===========================================================================
 *  @func   TestApp_CompareOutputData()
 *
 *  @desc   Currently this file dosen't have this feature.Need to add
 *
 *  @note   None
 * ===========================================================================
 */
XDAS_Int32 TestApp_CompareOutputData
(
    FILE             * fOutFile,
    XDAS_Int32         display_marker,
    IMP4HDVICPDEC_OutArgs * outArgs,
    XDAS_Int32         height,
    XDAS_Int32         width
);

/* ===========================================================================
 *  @func   TestApp_SetInitParams()
 *
 *  @desc   Initalizes the init parameter structure with proper values
 *
 *  @param  IVIDDEC2_Params *param
 *          Init parameter structure..
 *
 *  @return None
 *
 *  @note   None
 * ===========================================================================
 */
XDAS_Void   TestApp_SetInitParams(IVIDDEC2_Params *Params);

/* ===========================================================================
 *  @func   TestApp_SetDynamicParams()
 *
 *  @desc   Initalizes the Dynamic parameter structure with proper values
 *
 *  @param  IMP4HDVICPDEC_DynamicParams *dynamicParams
 *          Dynamic parameter structure..
 *
 *  @return None
 *
 *  @note   None
 * ===========================================================================
 */
XDAS_Void   TestApp_SetDynamicParams(IMP4HDVICPDEC_DynamicParams *DynamicParams);


/****************************************************************************
*   PRIVATE DECLARATIONS Defined here, used only here
*****************************************************************************/
/* ---------------------- data declarations ------------------------------- */
/* --------------------- function prototypes ------------------------------ */

/* ---------------------------- macros ------------------------------------ */
/* Maximum width in pixels supported for PAD D1 standard is 720             */
#define IMAGE_WIDTH            1920//864//1920
/* Maximum height in pixels supported for PAD D1 standard is 576            */
#define IMAGE_HEIGHT           1088//576//1088
/* Buffer size required for CB and CR for 4:2:0 formate is half
 * of picture size in pixels
 */
#define CHROMA_BUFFER_SIZE     IMAGE_WIDTH * IMAGE_HEIGHT >> 1

/* It is not possible to read the entire bitsream into to single buffer
 * in case of large bitstream size. To hanlde this situation, initially
 * some part of bit stream can be read into small buffer then the buffer
 * can be filled with the following bitstream data after decoding a frame.
 * Currently we have allocated 128k bytes for this purpose this size can be
 * modified by the user for the application requirements.
 */

#define INPUT_BUFFER_SIZE      0x200000
//#define INPUT_BUFFER_SIZE      30 * 1024

/* The number of characters that can be parsed by the config file parser.
 */
#define STRING_SIZE            256

#endif //_TESTAPP_DECODER_






/*!
 *! Revision History
 *! ================
 *! 08-Oct-2007   Kumar    : Added the support for frame mode input handling.
 *! 28-Aug-2007   Kumar    : Incorporated code review comments.
 *! 01-Jul-2006   Prashanth: Created.
 *!
 */
