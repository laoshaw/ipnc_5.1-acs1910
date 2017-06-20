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

#include <csl_sys.h>
#include <linux/semaphore.h>

CSL_Status CSL_sysMutexCreate(CSL_SysMutex * hndl)
{
  struct semaphore *pSem;

  if (hndl == NULL)
    return CSL_EFAIL;

  hndl->mutex = NULL;

  pSem = CSL_sysMemAlloc(sizeof(*pSem));

  if (pSem == NULL)
    return CSL_EFAIL;

  sema_init(pSem,1);

  hndl->mutex = pSem;

  return CSL_SOK;
}

CSL_Status CSL_sysMutexLock(CSL_SysMutex * hndl, Uint32 timeout)
{
  struct semaphore *pSem;

  if (hndl == NULL)
    return CSL_EFAIL;

  pSem = (struct semaphore *) hndl->mutex;

  if (pSem == NULL)
    return CSL_EFAIL;

  if (timeout == CSL_SYS_TIMEOUT_NONE) {

    if (down_trylock(pSem))
      return CSL_EFAIL;

  } else {

    if (down_interruptible(pSem))
      return CSL_EFAIL;

  }

  return CSL_SOK;
}

CSL_Status CSL_sysMutexUnlock(CSL_SysMutex * hndl)
{
  struct semaphore *pSem;

  if (hndl == NULL)
    return CSL_EFAIL;

  pSem = (struct semaphore *) hndl->mutex;

  if (pSem == NULL)
    return CSL_EFAIL;

  up(pSem);

  return CSL_SOK;
}

CSL_Status CSL_sysMutexDelete(CSL_SysMutex * hndl)
{
  if (hndl == NULL)
    return CSL_EFAIL;

  if (hndl->mutex != NULL)
    CSL_sysMemFree(hndl->mutex);
  return CSL_SOK;
}
