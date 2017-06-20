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

insmod ./drv.ko

if [ ! -f /dev/dev_i2c ]
then
    major=$(awk '$2=="dev_i2c" {print $1}' /proc/devices)
    mknod /dev/dev_i2c c ${major} 0
fi

if [ ! -f /dev/dev_dma ]
then
    major=$(awk '$2=="dev_dma" {print $1}' /proc/devices)
    mknod /dev/dev_dma c ${major} 0
fi

