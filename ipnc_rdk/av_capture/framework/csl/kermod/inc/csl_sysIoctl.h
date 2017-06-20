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

#ifndef _CSL_SYS_IOCTL_
#define _CSL_SYS_IOCTL_

#include <csl_soc.h>
#include <linux/ioctl.h>

#define CSL_IOCTL_CMD_MAKE(cmd)     ( _IO( CSL_DRV_MAGICNUM, cmd) )
#define CSL_IOCTL_CMD_GET(cmd)      ( _IOC_NR(cmd) )
#define CSL_IOCTL_CMD_IS_VALID(cmd) ( (_IOC_TYPE(cmd) == CSL_DRV_MAGICNUM ) ? 1 : 0)

#endif /* _CSL_LDC_IOCTL_ */
