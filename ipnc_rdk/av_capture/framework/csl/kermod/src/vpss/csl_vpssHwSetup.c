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

CSL_Status CSL_vpssHwSetup(CSL_VpssHandle hndl, CSL_VpssHwSetup * data)
{
  CSL_Status status = CSL_SOK;

  if (hndl == NULL || data == NULL)
    return CSL_EFAIL;

  if (data->wblSelConfig != NULL) {
    if (CSL_vpssSetWblSelConfig(hndl, data->wblSelConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }

  if (data->priConfig != NULL) {
    if (CSL_vpssSetPriConfig(hndl, data->priConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }

  if (data->intEvtConfig != NULL) {
    if (CSL_vpssSetIntEvtConfig(hndl, data->intEvtConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }

  if (data->clkConfig != NULL) {
    if (CSL_vpssSetClkConfig(hndl, data->clkConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }
  return status;
}
