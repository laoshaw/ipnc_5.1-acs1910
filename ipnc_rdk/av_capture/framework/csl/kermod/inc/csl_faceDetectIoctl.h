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

#ifndef _CSL_FACE_DETECT_IOCTL_
#define _CSL_FACE_DETECT_IOCTL_

#include <csl_faceDetect.h>

#define CSL_FACE_DETECT_CMD_HW_SETUP                     (0x00) ///< prm: [I ] CSL_FaceDetectHwSetup *
#define CSL_FACE_DETECT_CMD_DISABLE                      (0x01) ///< prm: [  ] NONE
#define CSL_FACE_DETECT_CMD_GET_STATUS                   (0x02) ///< prm: [I ] CSL_FaceDetectGetStatusPrm
#define CSL_FACE_DETECT_CMD_INT_ENABLE                   (0x03) ///< prm: [I ] Bool32
#define CSL_FACE_DETECT_CMD_INT_CLEAR                    (0x04) ///< prm: [  ] NONE
#define CSL_FACE_DETECT_CMD_INT_WAIT                     (0x05) ///< prm: [  ] NONE


typedef struct {

  CSL_FaceDetectFaceStatus *faceList;
  Uint16 *numFaces;

} CSL_FaceDetectGetStatusPrm;


CSL_Status CSL_faceDetectInit(CSL_FaceDetectHandle hndl);
CSL_Status CSL_faceDetectExit(CSL_FaceDetectHandle hndl);
CSL_Status CSL_faceDetectHwControl(CSL_FaceDetectHandle hndl, Uint32 cmd, void *prm);

#endif /* _CSL_FACE_DETECT_IOCTL_ */
