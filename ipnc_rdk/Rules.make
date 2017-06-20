# ============================================================================
# Copyright (c) Texas Instruments Inc 2013
#
# Use of this software is controlled by the terms and conditions found in the
# license agreement under which this software has been supplied or provided.
# ============================================================================

# Default build environment, set to linux
ifeq ($(OS), )
  OS := Linux
endif

#####################################################################################
#CONFIGURATION
#####################################################################################

# Select Hardware Platform: EVM or IPNC
#SYSTEM_PLATFORM := EVM
SYSTEM_PLATFORM := IPNC

# IPNC Platform Device needs to selected
#IPNC_DEVICE := DM365
IPNC_DEVICE := DM368
#IPNC_DEVICE := DM369
#IPNC_DEVICE := DMVA1
#IPNC_DEVICE := DMVA2

# Filesystem mode to selected: NAND or NFS
#FILESYS_MODE := NFS
FILESYS_MODE := UBIFS

# Build configuration : debug or release
#APP_BUILD_CFG    := debug
APP_BUILD_CFG   := release

# UBIFS mtd-utils mode
MTD_UTILS_MODE := 64bit
#MTD_UTILS_MODE := 32bit

# Sensor Selected for the build, based on SYSTEM_PLATFORM
# IPNC and EVM platform have different set of Sensor supported
ifeq ($(SYSTEM_PLATFORM), IPNC)
#IMGS_ID := IMGS_MICRON_MT9P031_5MP
#IMGS_ID := IMGS_MICRON_MT9D131_2MP
IMGS_ID := IMGS_OV10630_1MP
#IMGS_ID := IMGS_MICRON_MT9M031_1.2MP
#IMGS_ID := IMGS_MICRON_MT9M034_1.2MP
#IMGS_ID := IMGS_OV2715_2MP
#IMGS_ID := IMGS_SONY_IMX035_1_3MP
#IMGS_ID := IMGS_SONY_IMX036_3MP
#IMGS_ID := IMGS_SONY_IMX104_1_3MP
#IMGS_ID := IMGS_SONY_IMX136_2_3MP
#IMGS_ID := IMGS_SONY_IMX122_2_3MP
#IMGS_ID := IMGS_AR0130_1.2MP
#IMGS_ID := IMGS_AR0330_3.1MP
#IMGS_ID := IMGS_AR0331_3.1MP
#IMGS_ID := IMGS_AR0331_3.1MP_WDR
#IMGS_ID := IMGS_PANASONIC_MN34041_2.1MP
endif
ifeq ($(SYSTEM_PLATFORM), EVM)
IMGS_ID := IMGS_TVP514X
#IMGS_ID := IMGS_MICRON_MT9P031_5MP
endif

# This flag will determine whether any 2A modules is required in build or not
#AEWB_BUILD_MODE := no
AEWB_BUILD_MODE := yes

# WiFi hardware mode flag: ON or OFF
# ON:if WiFi Hardware module is present else this needs to be set to OFF
#WIFI_MODULE_MODE := ON
WIFI_MODULE_MODE := OFF

# AES Encyrption mode flag: ON or OFF
# ON:if AES Encyprtion module is present else this needs to be set to OFF
#AES_MODULE_MODE := ON
AES_MODULE_MODE := OFF

# Face Recognition flag: ON or OFF
# ON:if Face Recognitio module is present else this needs to be set to OFF
#FACE_RECOG_MODE := ON
FACE_RECOG_MODE := OFF

# Sensor Board Vendor: Appro or Leopard Imaging
# APPRO:if Appro Sensor Board is used or LIMG: if Leopard Imaging Board are used
SENSOR_BOARD_VENDOR := APPRO
#SENSOR_BOARD_VENDOR := LIMG

# Build warnings will be treated as errors for av_capture folder
TREAT_WARNINGS_AS_ERROR := no
#TREAT_WARNINGS_AS_ERROR := yes

################################################################################
# Combinations checks for the options set above are done here 
################################################################################

ifeq ($(IPNC_DEVICE), DM368)
HARDWARE_CFG := dm368
else ifeq ($(IPNC_DEVICE), DM369)
HARDWARE_CFG := dm368
else ifeq ($(IPNC_DEVICE), DMVA2)
HARDWARE_CFG := dm368
else ifeq ($(IPNC_DEVICE), DMVA1)
HARDWARE_CFG := dm365
else
HARDWARE_CFG := dm365
endif

ifeq ($(FILESYS_MODE), UBIFS)
BUILD_CFG := ubifs
else
BUILD_CFG := nfs
endif

ifeq ($(SYSTEM_PLATFORM), EVM)
SYSTEM_CFG := evm
else
SYSTEM_CFG := ipnc
endif

ifeq ($(AEWB_BUILD_MODE), yes)
AEWB_ID := AEWB_ENABLE
else
AEWB_ID := AEWB_NONE
endif

ifeq ($(SYSTEM_PLATFORM), IPNC)
BOARD_ID := BOARD_AP_IPNC
else
BOARD_ID := BOARD_TI_EVM
endif

DVSDK_ARGS=
ifeq ($(SYSTEM_CFG), ipnc)
ifeq ($(HARDWARE_CFG), dm368)
DVSDK_ARGS=USE_UDEV=1
endif
endif

################################################################################
# All build tools paths are set in this section
################################################################################

BASE_INSTALL_DIR  	:= $(shell pwd)/..

# Defining the install base directory for IPNC RDK
IPNC_INSTALL_DIR   	:= $(BASE_INSTALL_DIR)/ipnc_rdk
DVSDK_BASE_DIR 	  	:= $(BASE_INSTALL_DIR)/dvsdk_ipnctools

# Defining the build tools directory for IPNC RDK
BUILD_TOOL_DIR 	  	:= $(DVSDK_BASE_DIR)/linux-devkit
BUILD_TOOL_PREFIX   	:= $(BUILD_TOOL_DIR)/bin/arm-arago-linux-gnueabi-

# The directory that points to the Linux Support Package
PSP_BASE_DIR 	  	:= $(DVSDK_BASE_DIR)/ipnc_psp_03_21_00_04
KERNELDIR 		:= $(PSP_BASE_DIR)/kernel
UBOOTDIR 		:= $(PSP_BASE_DIR)/u-boot

# The directory that points to the AVCAPTURE module folder
AVCAPTURE_BASE_DIR	:= $(IPNC_INSTALL_DIR)/av_capture
AVCAPTURE_BUILD_DIR	:= $(AVCAPTURE_BASE_DIR)/build
			
# The directory that points to where filesystem is mounted
FILESYS_BASE_DIR  	:= $(IPNC_INSTALL_DIR)/target
TARGET_FS 		:= $(FILESYS_BASE_DIR)/filesys_$(HARDWARE_CFG)
UBIFS_PATH      	:= $(FILESYS_BASE_DIR)/mtd-utils/$(MTD_UTILS_MODE)

# The directory that points to the tftp directory
# This will have generated binaries likeu-boot, uImage, ubifs etc
# And also other binary files can be kept here
# This will all files needed that are needed for burning to NAND
TFTP_HOME 		:= $(IPNC_INSTALL_DIR)/tftp/$(IPNC_DEVICE)

# Where to copy the resulting executables and data to (when executing 'make
# install') in a proper file structure. This IPNC_EXEC_DIR should either be visible
# from the target, or you will have to copy this (whole) directory onto the
# target filesystem.
IPNC_EXEC_DIR		:= $(TARGET_FS)/opt/ipnc

# The directory that points to the IPNC Application package
IPNC_DIR		:= $(IPNC_INSTALL_DIR)/ipnc_app

# The directory to IPNC application interface modules to AVCAPTURE
IPNC_INTERFACE_DIR	:= $(IPNC_DIR)/interface
PUBLIC_INCLUDE_DIR	:= $(IPNC_INTERFACE_DIR)/inc
APP_LIB_DIR		:= $(IPNC_INTERFACE_DIR)/lib

include $(DVSDK_BASE_DIR)/Rules.make

################################################################################
# All EXPORT symbols are defined here
################################################################################
export BUILD_TOOL_DIR
export IPNC_DIR
export PUBLIC_INCLUDE_DIR
export APP_LIB_DIR
export TARGET_FS
export SYSTEM_PLATFORM
export HARDWARE_CFG
export IPNC_DEVICE
export LINUXKERNEL_INSTALL_DIR
export IPNC_EXEC_DIR
export SQUASHFS_PATH
export KERNELDIR
export UBOOTDIR
export AVCAPTURE_BUILD_DIR
export APP_BUILD_CFG
export DVSDK_BASE_DIR
export CMEM_INSTALL_DIR
export BUILD_TOOL_PREFIX
export BOARD_ID
export IMGS_ID
export SENSOR_BOARD_VENDOR
export AEWB_ID
export FILESYS_MODE
export TFTP_HOME
export WIFI_MODULE_MODE
export AES_MODULE_MODE
export FACE_RECOG_MODE
export TREAT_WARNINGS_AS_ERROR
export DVSDK_ARGS
