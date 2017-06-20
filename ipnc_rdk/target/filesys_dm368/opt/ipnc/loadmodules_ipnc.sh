#!/bin/sh

#./av_capture_load.sh

umount /mnt/ramdisk 2>/dev/null
sleep 1
mkdosfs -v /dev/ram0 2>/dev/null
mount -t vfat /dev/ram0 /mnt/ramdisk 2>/dev/null


