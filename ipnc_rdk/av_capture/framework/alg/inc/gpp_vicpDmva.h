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

#ifndef _GPP_CPISLIBDMVA_H
#define _GPP_CPISLIBDMVA_H

#ifdef __cplusplus
    extern "C" {
#endif

#include <tistdtypes.h>
#include "vicplib.h"
#include "vicpDmva.h"

Int32 GPP_CPIS_updateEWRMeanVar16(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_UpdateEWRMeanVar16Parms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the background update algorithm based on roi's width and roi's height) */
Int32 GPP_CPIS_getUpdateEWRMeanVar16blockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

Int32 GPP_CPIS_segmentFG(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_SegmentFGParms *params,
    CPIS_ExecType execType
);

Int32 GPP_CPIS_getSegmentFGblockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

Int32 GPP_CPIS_imDiffIntlv(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ImDiffIntlvParms *params,
    CPIS_ExecType execType
);

/* Return optimum block dimensions used by the image difference algorithm based on roi's width and roi's height) */
Int32 GPP_CPIS_getImDiffIntlvBlockDim(Uint32 roiWidth, Uint32 roiHeight, Uint32 *blockWidth, Uint32 *blockHeight);

Int32 GPP_CPIS_calcEdgeOffset(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_CalcEdgeOffsetParms *params,
    CPIS_ExecType execType
);

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
