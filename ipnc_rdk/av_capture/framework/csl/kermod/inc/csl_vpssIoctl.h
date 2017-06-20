/*
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CSL_VPSS_IOCTL_
#define _CSL_VPSS_IOCTL_

#include <csl_vpss.h>

#define CSL_VPSS_CMD_HW_SETUP                     (0x00)  ///< prm: [I ] CSL_VpssHwSetup *
#define CSL_VPSS_CMD_HW_RESET                     (0x01)  ///< prm: [  ] NONE
#define CSL_VPSS_CMD_SET_CLK_CONFIG               (0x02)  ///< prm: [I ] CSL_VpssClkConfig *
#define CSL_VPSS_CMD_SET_INT_EVT_CONFIG           (0x03)  ///< prm: [I ] CSL_VpssIntEvtConfig *
#define CSL_VPSS_CMD_SET_PRI_CONFIG               (0x04)  ///< prm: [I ] CSL_VpssPriConfig *
#define CSL_VPSS_CMD_SET_WBL_SEL_CONFIG           (0x05)  ///< prm: [I ] CSL_VpssWblSelConfig *
#define CSL_VPSS_CMD_SET_CLKDIV                   (0x06)  ///< prm: [I ] CSL_VpssClkDiv *
#define CSL_VPSS_CMD_GET_CLKDIV                   (0x07)  ///< prm: [ O] CSL_VpssClkDiv *

CSL_Status CSL_vpssInit(CSL_VpssHandle hndl);
CSL_Status CSL_vpssExit(CSL_VpssHandle hndl);
CSL_Status CSL_vpssHwControl(CSL_VpssHandle hndl, Uint32 cmd, void *prm);

#endif /* _CSL_VPSS_IOCTL_ */
