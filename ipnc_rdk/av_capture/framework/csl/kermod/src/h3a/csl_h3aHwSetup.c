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

#include <csl_h3a.h>

CSL_Status CSL_h3aHwSetup(CSL_H3aHandle hndl, CSL_H3aHwSetup * data)
{
  CSL_Status status = CSL_SOK;

  if (hndl == NULL || data == NULL)
    return CSL_EFAIL;

  if (data->inFrameConfig != NULL) {
    if (CSL_h3aInFrameConfig(hndl, data->inFrameConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }

  if (data->afConfig != NULL) {
    if (CSL_h3aAfSetConfig(hndl, data->afConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }

  if (data->aewbConfig != NULL) {
    if (CSL_h3aAewbSetConfig(hndl, data->aewbConfig) != CSL_SOK)
      status = CSL_EFAIL;
  }
  return status;
}
