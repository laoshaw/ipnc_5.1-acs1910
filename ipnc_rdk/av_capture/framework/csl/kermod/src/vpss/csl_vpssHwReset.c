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

#include <csl_vpss.h>

CSL_Status CSL_vpssHwReset(CSL_VpssHandle hndl)
{
  if (hndl == NULL)
    return CSL_EFAIL;

  hndl->ispRegs->PCCR = CSL_ISP5_PCCR_RESETVAL;
  hndl->ispRegs->BCR = CSL_ISP5_BCR_RESETVAL;
  hndl->ispRegs->INTSTAT = CSL_ISP5_INTSTAT_RESETVAL;
  hndl->ispRegs->INTSEL1 = CSL_ISP5_INTSEL1_RESETVAL;
  hndl->ispRegs->INTSEL2 = CSL_ISP5_INTSEL2_RESETVAL;
  hndl->ispRegs->INTSEL3 = CSL_ISP5_INTSEL3_RESETVAL;
  hndl->ispRegs->EVTSEL = CSL_ISP5_EVTSEL_RESETVAL;

  return CSL_SOK;
}
