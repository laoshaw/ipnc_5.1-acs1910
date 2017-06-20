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

ifneq ($(KERNELRELEASE),)
# call from kernel build system

EXTRA_CFLAGS += -I$(M)/../inc -I$(M)/../priv -I$(M)/../../inc -DKERNEL_LEVEL -DDEBUG -D$(BOARD_ID) -O3
                                                          
ifeq ($(IMGS_ID), IMGS_TVP514X)
EXTRA_CFLAGS += -DYUV_MODE
EXTRA_CFLAGS += -DYUV_MODE_INTERLACED 
endif

drv-objs := ../src/dev_drv.o ../src/dev_i2c.o ../src/dev_dma.o ../src/dev_dmaCopyFill.o ../src/dev_dmaDemux.o 
           
obj-y	:= drv.o 

else

PWD     := $(shell pwd)
SUBDIRS := build src 
KER_MODULE := drv_kernel

modules:
	@echo $(KER_MODULE): Building module
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(BUILD_TOOL_PREFIX) 
  
install:
	@echo $(KER_MODULE): Copying files to install directory
	-mkdir -p $(IPNC_EXEC_DIR)
	
clean:
	@echo $(KER_MODULE): Deleting temporary files
	for i in $(SUBDIRS) ; do cd ../$$i; rm -rf *.ko *.o *~ core .depend .*.cmd *.mod.c .tmp_versions; cd -; done  
	-rm -f $(IPNC_EXEC_DIR)/*.ko
	
depend:
	
endif



