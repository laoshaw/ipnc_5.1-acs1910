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

#include <csl_faceDetect.h>
#include <csl_sysDrv.h>

CSL_IntRet_t CSL_faceDetectIsr(int intId, void *prm, void *reserved)
{
  CSL_FaceDetectHandle hndl = &gCSL_drvFaceDetectObj;

  CSL_sysFlagSet(&hndl->intFlag);

  return CSL_INT_RET_SOK;
}

CSL_Status CSL_faceDetectIntEnable(CSL_FaceDetectHandle hndl, Bool32 enable)
{
  CSL_Status status;

  if (hndl == NULL)
    return CSL_EFAIL;

  status = CSL_sysIntEnable(CSL_SYS_INT_FD, enable);

  return status;
}

CSL_Status CSL_faceDetectIntClear(CSL_FaceDetectHandle hndl)
{
  CSL_Status status;

  if (hndl == NULL)
    return CSL_EFAIL;

  CSL_sysIntClear(CSL_SYS_INT_FD);

  status = CSL_sysFlagClear(&hndl->intFlag);

  return status;
}

CSL_Status CSL_faceDetectIntWait(CSL_FaceDetectHandle hndl, int timeout)
{
  CSL_Status status;

  if (hndl == NULL)
    return CSL_EFAIL;

  status = CSL_sysFlagWait(&hndl->intFlag, timeout);

  return status;
}
