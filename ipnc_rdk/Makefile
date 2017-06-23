# ============================================================================
# Copyright (c) Texas Instruments Inc 2013
#
# Use of this software is controlled by the terms and conditions found in the
# license agreement under which this software has been supplied or provided.
# ============================================================================

include Rules.make
include $(AVCAPTURE_BUILD_DIR)/COMMON_HEADER.MK

app:
	make dvsdk
	make depend
	make libs
	make -fMAKEFILE.MK -C$(AVCAPTURE_BUILD_DIR)/../application/test exe
#ifeq ($(SYSTEM_PLATFORM), IPNC)		
	make -fMAKEFILE.MK -C$(AVCAPTURE_BUILD_DIR)/../application/ipnc/av_server exe								
#endif
	make install
	@echo 
	@echo "--------------------------------------------------------------"
	@echo "******** Build completed for following configuration *********"
	@echo "--------------------------------------------------------------"
	@echo "Hardware Platform      : $(IPNC_DEVICE) $(SYSTEM_PLATFORM)"
	@echo "Sensor used            : $(IMGS_ID)"
	@echo "Filesystem Mode        : $(FILESYS_MODE)"
	@echo "Sensor Board Vendor    : $(SENSOR_BOARD_VENDOR)"
	@echo "Build Mode             : $(APP_BUILD_CFG)"
	@echo "--------------------------------------------------------------"
	@echo 
	@echo "------------------------------------------------------------------------------------------------------------------"
	@echo "Please run < make lsp > if you have changed anything in csl/drv kernel modules and use the new built image"
	@echo "------------------------------------------------------------------------------------------------------------------"
	
libs:
	make -fMAKEFILE.MK -C$(AVCAPTURE_BUILD_DIR)/../framework $(MAKE_TARGET)
	make -fMAKEFILE.MK -C$(AVCAPTURE_BUILD_DIR)/../application/test $(MAKE_TARGET)
#ifeq ($(SYSTEM_PLATFORM), IPNC)		
	make -fMAKEFILE.MK -C$(AVCAPTURE_BUILD_DIR)/../application/ipnc $(MAKE_TARGET)
#endif

all:
	make dvsdkclean
	make clean
	make dvsdk
	make app

clean:
	make libs MAKE_TARGET=clean
	-rm -f $(EXE_BASE_DIR)/$(APP_BUILD_CFG)/*.out
	-rm -f $(IPNC_EXEC_DIR)/*.out
	
depend:
	-mkdir -p $(EXE_BASE_DIR)
	-mkdir -p $(IPNC_EXEC_DIR)	
	make libs MAKE_TARGET=depend  

install:
	make libs MAKE_TARGET=install  
	cp $(EXE_BASE_DIR)/$(APP_BUILD_CFG)/av_server.out $(IPNC_EXEC_DIR)
	cp $(EXE_BASE_DIR)/$(APP_BUILD_CFG)/i2crw.out $(IPNC_EXEC_DIR)
	cp $(EXE_BASE_DIR)/$(APP_BUILD_CFG)/regrw.out $(IPNC_EXEC_DIR)

dvsdkbuild:
	make -C$(DVSDK_BASE_DIR) $(DVSDK_TARGET)
	
dvsdk:
	make dvsdkbuild $(DVSDK_ARGS) DVSDK_TARGET=
	@echo
	@echo "WARNING:"
	@echo "Please run make lsp if you have changed cmem/edma/irqk/dm365mmap and use the new built uImage!!!!!"

dvsdkclean:
	make dvsdkbuild DVSDK_TARGET=clean
	
dvsdkall:	
	make dvsdkclean
	make dvsdk
		
lspbuild:
	make -C$(KERNELDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) $(MAKE_TARGET)
	
lspcfg:
ifeq ($(SYSTEM_PLATFORM), IPNC)
	make lspbuild MAKE_TARGET=davinci_$(HARDWARE_CFG)_$(SYSTEM_CFG)_$(BUILD_CFG)_defconfig
endif	
ifeq ($(SYSTEM_PLATFORM), EVM)
	make lspbuild MAKE_TARGET=davinci_dm36x_$(SYSTEM_CFG)_$(BUILD_CFG)_defconfig
endif	

psccfg:
	make -C$(KERNELDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) menuconfig
pscbuild:
	make -C$(KERNELDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) uImage
	cp $(KERNELDIR)/arch/arm/boot/uImage $(TFTP_HOME)/uImage_$(SYSTEM_CFG)_$(HARDWARE_CFG)
psclsp:
	#make psccfg
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/cmem/src/module/cmemk.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/edma/src/module/edmak.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/irq/src/module/irqk.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/dm365mm-module_01_00_03/module/dm365mmap.o $(KERNELDIR)/drivers/char
	cp $(AVCAPTURE_BUILD_DIR)/../framework/drv/kermod/build/drv.o $(KERNELDIR)/drivers/char
	cp $(AVCAPTURE_BUILD_DIR)/../framework/csl/kermod/build/csl.o $(KERNELDIR)/drivers/char
	make lspbuild MAKE_TARGET=uImage
	make lspbuild MAKE_TARGET=modules
	cp $(KERNELDIR)/arch/arm/boot/uImage $(TFTP_HOME)/uImage_$(SYSTEM_CFG)_$(HARDWARE_CFG)
	find $(KERNELDIR) -name "*.ko" -exec cp '{}' $(IPNC_EXEC_DIR)/modules \;
	cp $(KERNELDIR)/drivers/usb/gadget/g_file_storage.ko $(IPNC_EXEC_DIR)			
	
lsp:
	make lspcfg
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/cmem/src/module/cmemk.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/edma/src/module/edmak.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/irq/src/module/irqk.o $(KERNELDIR)/drivers/char
	cp $(DVSDK_BASE_DIR)/dm365mm-module_01_00_03/module/dm365mmap.o $(KERNELDIR)/drivers/char
	cp $(AVCAPTURE_BUILD_DIR)/../framework/drv/kermod/build/drv.o $(KERNELDIR)/drivers/char
	cp $(AVCAPTURE_BUILD_DIR)/../framework/csl/kermod/build/csl.o $(KERNELDIR)/drivers/char
	make lspbuild MAKE_TARGET=uImage
	make lspbuild MAKE_TARGET=modules
	cp $(KERNELDIR)/arch/arm/boot/uImage $(TFTP_HOME)/uImage_$(SYSTEM_CFG)_$(HARDWARE_CFG)
	find $(KERNELDIR) -name "*.ko" -exec cp '{}' $(IPNC_EXEC_DIR)/modules \;
	cp $(KERNELDIR)/drivers/usb/gadget/g_file_storage.ko $(IPNC_EXEC_DIR)		

lspall:
	make lspclean
	make lsp
	
lspclean:	
	make lspbuild MAKE_TARGET=distclean
	rm -f $(IPNC_EXEC_DIR)/modules/*.ko

ubootbuild:
	make -C$(UBOOTDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) $(MAKE_TARGET)

ubootbin:
	make ubootclean
ifeq ($(SYSTEM_PLATFORM), IPNC)
	make ubootbuild MAKE_TARGET=davinci_$(HARDWARE_CFG)_$(SYSTEM_CFG)_config
endif	
	make ubootbuild MAKE_TARGET=
ifeq ($(SYSTEM_PLATFORM), IPNC)
	cp $(UBOOTDIR)/u-boot-1.3.4-$(HARDWARE_CFG)_$(SYSTEM_CFG).bin $(TFTP_HOME)/u-boot-1.3.4-$(HARDWARE_CFG)_$(SYSTEM_CFG).bin
endif	

ubootclean:
	make ubootbuild MAKE_TARGET=distclean
	
uboot:
	make ubootbin
	
sysall:
ifeq ($(SYSTEM_PLATFORM), IPNC)
	make uboot
endif
	make lspcfg
	make -C $(KERNELDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) prepare
	make -C $(KERNELDIR) ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX) modules_prepare
	make dvsdkall
	make all
	make lspall
	make
	
sysclean:
	make ubootclean
	make lspclean	
	make dvsdkclean
	make clean

sys:
ifeq ($(SYSTEM_PLATFORM), IPNC)
	make uboot
endif
	make dvsdk
	make 	
	make lsp
	make

ubifs:
ifeq ($(HARDWARE_CFG), dm365) 
	$(UBIFS_PATH)/mkfs.ubifs -r  $(TARGET_FS) -m 512 -e 15360 -c 1700 -o ubifs.img
	$(UBIFS_PATH)/ubinize -o $(SYSTEM_CFG)_$(HARDWARE_CFG)_ubifs -m 512 -p 16KiB  $(HARDWARE_CFG)_ubinize.cfg
else
	$(UBIFS_PATH)/mkfs.ubifs -r  $(TARGET_FS) -m 2048 -e 126976 -c 313 -o ubifs.img
	$(UBIFS_PATH)/ubinize -o $(SYSTEM_CFG)_$(HARDWARE_CFG)_ubifs -m 2048 -p 128KiB  $(HARDWARE_CFG)_ubinize.cfg
endif
	mv $(SYSTEM_CFG)_$(HARDWARE_CFG)_ubifs $(TFTP_HOME)
	rm ubifs.img

binaries:
	make sysall
	make all
	make
	make ubifs

.PHONY : exe libs all clean depend install dvsdkbuild dvsdk dvsdkclean dvsdkall lspbuild lspcfg lsp lspall lspclean nfsreset sysall sysclean sys ipnc ipncbuild ipncclean ipncall ramdisk
