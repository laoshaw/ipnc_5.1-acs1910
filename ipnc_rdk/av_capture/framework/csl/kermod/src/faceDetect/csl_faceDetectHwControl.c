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

#include <csl_faceDetectIoctl.h>

CSL_Status CSL_faceDetectHwControl(CSL_FaceDetectHandle hndl, Uint32 cmd, void *prm)
{
  CSL_Status status = CSL_SOK;

  static CSL_FaceDetectHwSetup data;
  static CSL_FaceDetectGetStatusPrm getStatusPrm;
  static CSL_FaceDetectFaceStatus faceList[CSL_FACE_DETECT_MAX_FACES];
  Uint16 numFaces;

  switch (cmd) {

  case CSL_FACE_DETECT_CMD_HW_SETUP:

    status = CSL_copyFromUser(&data, prm, sizeof(data));

    if (status == CSL_SOK)
      status = CSL_faceDetectHwSetup(hndl, &data);

    break;

  case CSL_FACE_DETECT_CMD_DISABLE:

    status = CSL_faceDetectDisable(hndl);
    break;

  case CSL_FACE_DETECT_CMD_GET_STATUS:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&getStatusPrm, prm, sizeof(getStatusPrm));

    if (status == CSL_SOK)
      status = CSL_faceDetectGetStatus(hndl, &faceList[0], &numFaces);

    status |= CSL_copyToUser(getStatusPrm.faceList, &faceList[0], sizeof(faceList));

    status |= CSL_putUser(numFaces, getStatusPrm.numFaces);

    break;

  case CSL_FACE_DETECT_CMD_INT_ENABLE:

    status = CSL_faceDetectIntEnable(hndl, (Bool32) prm);
    break;

  case CSL_FACE_DETECT_CMD_INT_CLEAR:

    status = CSL_faceDetectIntClear(hndl);
    break;

  case CSL_FACE_DETECT_CMD_INT_WAIT:

    status = CSL_faceDetectIntWait(hndl, (int)prm);
    break;

  default:
    status = CSL_EFAIL;
    break;
  }

  return status;
}
