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

#include <csl_h3aIoctl.h>

CSL_Status CSL_h3aOpen(CSL_H3aHandle hndl)
{
  if (hndl == NULL)
    return CSL_EINVPARAMS;

  hndl->regs = CSL_sysGetBaseAddr(CSL_MODULE_H3A);

  if (hndl->regs == NULL)
    return CSL_EFAIL;

  return CSL_SOK;
}

CSL_Status CSL_h3aClose(CSL_H3aHandle hndl)
{
  return CSL_SOK;
}

CSL_Status CSL_h3aInit(CSL_H3aHandle hndl)
{
  int i;
  CSL_Status status = CSL_SOK;

  if (hndl == NULL)
    return CSL_EINVPARAMS;

  for (i = 0; i < CSL_H3A_MOD_MAX; i++) {
    hndl->outBufSwitchEnable[i] = FALSE;
    status |= CSL_bufCreate(&hndl->outBuf[i]);
  }

  status |= CSL_h3aOpen(hndl);

  return status;
}

CSL_Status CSL_h3aExit(CSL_H3aHandle hndl)
{
  int i;

  if (hndl == NULL)
    return CSL_EINVPARAMS;

  CSL_h3aClose(hndl);

  for (i = 0; i < CSL_H3A_MOD_MAX; i++) {
    hndl->outBufSwitchEnable[i] = FALSE;
    CSL_bufDelete(&hndl->outBuf[i]);
  }

  return CSL_SOK;
}
