#
# Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as 
# published by the Free Software Foundation version 2.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any
# kind, whether express or implied; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

rmmod ./drv.ko

rm -f /dev/dev_i2c
rm -f /dev/dev_dma
