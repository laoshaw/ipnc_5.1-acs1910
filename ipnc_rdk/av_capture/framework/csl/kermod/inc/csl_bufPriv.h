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

#ifndef _CSL_BUF_PRIV_H
#define _CSL_BUF_PRIV_H

#include <csl_sys.h>
#include <csl_buf.h>

typedef struct {

  CSL_BufInfo bufInfo[CSL_BUF_NUM_MAX];

  Uint32  emptyIdx;
  Uint32  fullIdx;

  Uint32  numBuf;

  CSL_SysFlag emptyFlag;
  CSL_SysFlag fullFlag;

} CSL_BufHndl;

CSL_Status CSL_bufCreate(CSL_BufHndl * hndl);
CSL_Status CSL_bufDelete(CSL_BufHndl * hndl);
CSL_Status CSL_bufInit(CSL_BufHndl * hndl, CSL_BufInit * bufInit);

Bool32    CSL_bufIsEmpty(CSL_BufHndl * hndl, Uint32 minBuf);
CSL_Status CSL_bufGetEmpty(CSL_BufHndl * hndl, CSL_BufInfo * buf, Uint32 minBuf, Uint32 timeout);
CSL_Status CSL_bufPutFull(CSL_BufHndl * hndl, CSL_BufInfo * buf);

Bool32    CSL_bufIsFull(CSL_BufHndl * hndl, Uint32 minBuf);
CSL_Status CSL_bufGetFull(CSL_BufHndl * hndl, CSL_BufInfo * buf, Uint32 minBuf, Uint32 timeout);
CSL_Status CSL_bufPutEmpty(CSL_BufHndl * hndl, CSL_BufInfo * buf);

CSL_Status CSL_bufSwitchFull(CSL_BufHndl * hndl, CSL_BufInfo * buf, Uint32 minBuf, Uint32 timestamp, Uint32 count);

#endif
