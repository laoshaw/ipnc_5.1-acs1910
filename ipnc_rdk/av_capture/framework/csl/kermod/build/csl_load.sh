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

#!/bin/sh

module="csl"


# invoke insmod with all arguments we got
insmod ./$module.ko || exit 1

# retrieve major number
major=$(awk '$2=="csl" {print $1}' /proc/devices)

rm -f /dev/${module}[0-9]
mknod /dev/${module}0 c $major 0
mknod /dev/${module}1 c $major 1
mknod /dev/${module}2 c $major 2
mknod /dev/${module}3 c $major 3
mknod /dev/${module}4 c $major 4
mknod /dev/${module}5 c $major 5
mknod /dev/${module}6 c $major 6
mknod /dev/${module}7 c $major 7
mknod /dev/${module}8 c $major 8
mknod /dev/${module}9 c $major 9






