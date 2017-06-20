/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  VICP Signal Processing Library                                          */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*                                                                          */
/*     NAME                                                                 */
/*        vicpDmva.h -- Signal Processing Library Interface header file     */
/*                                                                          */
/*     DESCRIPTION                                                          */
/*        This file includes the definitions and the interfaces supported   */
/*        by the Library                                                    */
/*                                                                          */
/*     REV                                                                  */
/*        version 0.0.1:  22nd Sep                                          */
/*        Initial version                                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#ifndef _CPISLIBDMVA_H
#define _CPISLIBDMVA_H

#ifdef __cplusplus
    extern "C" {
#endif

#include <tistdtypes.h>

/* update exponentially weighted mean and variance API params */
typedef struct {
    Uint16 weightMean;
    Uint16 weightVar;
} CPIS_UpdateEWRMeanVar16Parms;

typedef struct {
    Int16 thresholdCameraNoiseSQ15_0;
    Int16 thresholdVarFactorS7_8;
    Int16 thresholdGlobalS15_0;
    Uint8 motionHistImgDecay;
    Uint8 enableCompensation;
} CPIS_SegmentFGParms;

typedef struct {
    Uint16 motionThreshold;
} CPIS_ImDiffIntlvParms;

typedef struct {
    Uint32 sumModelPixel;
    Uint32 sumCurFrmPixel;
    Int32 sumStableBGPixel;
    Uint32 sumStableBGPixelCnt;
    Uint32 sumBGSubCnt;
    Uint32 sumIFDCnt;
    Int32 sumStableBGPriorPixel;
    Uint32 sumStableBGPriorPixelCnt;
    Uint32 sumBGSubPriorCnt;
} CPIS_SegmentFG_stat;

/* Sum-of-absolute differences API params */
typedef struct {
    Uint16 qShift;
    Int16 *templatePtr; /* Pointer to template */
    Int16 loadTemplate; /* Enable(1)/Disable(0) pre-loading of template into coefficient memory */
    CPIS_Size templateRoiSize; /* Size of the template's ROI */
    CPIS_Size templateSize;     /* Size of the template */
    Int16 templateStartOfst; /* Offset from location pointed by templatePtr to the first data point of the template */
} CPIS_CalcEdgeOffsetParms;

typedef struct {
    Uint16 dummy; /* Really there is no specific parameters needed for this function */
} CPIS_CountNumONpixelsParms;

/* Sum-of-absolute differences API params */
typedef struct {
    Uint16 qShift;
    Uint16 edgeThreshold;
    Uint8 onEdgeVal;
    Uint8 offEdgeVal;
} CPIS_CalcSobelEdgeMapParms;

typedef struct {
    Uint16 motionT;
    Uint16 bgSubT;
} CPIS_TamperImageScanParms;

typedef struct {
    Uint32 totalPos;
    Uint32 truePos;
    Uint32 fgCount;
    Uint32 sumPixelValue;
} CPIS_TamperImageScan_stat;

Int32 CPIS_updateEWRMeanVar16(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_UpdateEWRMeanVar16Parms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the background update algorithm based on roi's width and roi's height) */
Int32 CPIS_getUpdateEWRMeanVar16blockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

Int32 CPIS_segmentFG(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_SegmentFGParms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the segment foreground algorithm based on roi's width and roi's height) */
Int32 CPIS_getSegmentFGblockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);


Int32 CPIS_imDiffIntlv(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ImDiffIntlvParms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the image difference algorithm based on roi's width and roi's height) */
Int32 CPIS_getImDiffIntlvBlockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

Int32 CPIS_calcEdgeOffset(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_CalcEdgeOffsetParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_setCalcEdgeOffsetTemplateOffset(CPIS_Handle *handle, Uint16 templateStartOfst);

Int32 CPIS_tamperImageScan(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_TamperImageScanParms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the segment foreground algorithm based on roi's width and roi's height) */
Int32 CPIS_getTamperImageScanBlockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

#ifdef __cplusplus
 }
#endif

#endif /* #define _CPISLIBCUST_H */

/* ======================================================================== */
/*                       End of file                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
