# (c) Texas Instruments 

ifndef $(INCLUDES_MK)
INCLUDES_MK = 1

KERNEL_INC=-I$(KERNELDIR)/include -I$(KERNELDIR)/arch/arm/mach-davinci/include
CMEM_INC=-I$(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/include
COMMON_INC=-I../inc -I../priv 

OSA_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/osa/inc 
CSL_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/csl/inc
DRV_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/drv/inc
DMVAL_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/dmval/inc -I$(AVCAPTURE_BUILD_DIR)/../framework/alg/inc -I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv -I$(AVCAPTURE_BUILD_DIR)/../framework/dmval/priv 

ALG_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/inc
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/swosd
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/swosd
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/vstab
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/aewb
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/facetrack
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/priv/saldre
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ap/include
ifeq ($(IMGS_ID), IMGS_AR0331_3.1MP_WDR)
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ar0331wdr/include
else
ALG_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ti/include
endif

IMAGE_TUNE_INC=-I$(AVCAPTURE_BUILD_DIR)/../framework/image_tune/inc
IMAGE_TUNE_INC+=-I$(AVCAPTURE_BUILD_DIR)/../framework/image_tune/priv

AV_SERVER_INC=-I$(AVCAPTURE_BUILD_DIR)/../application/ipnc/av_server/inc

STREAM_INC=-I$(AVCAPTURE_BUILD_DIR)/../application/ipnc/av_server/src/stream/inc

FRAMEWORK_INC=$(CSL_INC) $(DRV_INC) $(ALG_INC) $(IMAGE_TUNE_INC) $(DMVAL_INC)

CMEM_LIB=$(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/lib/cmem.a470MV

DM365MM_LIB=$(DVSDK_BASE_DIR)/dm365mm-module_01_00_03/lib/libdm365mm.a

#VICP libraries
CODEC_LIBS=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/alg_facetrack_ti.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/alg_saldre_ti.a

CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/alg_config/linker.cmd 

#Algorithm libraries
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/alg_vstab_ti.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/alg_osd_ti.a

# 2A libaries
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ti/lib/alg_awb_ti.a 
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ti/lib/alg_flicker_detect_ti.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ap/lib/Appro_2a_lib.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/src/aewb_ap/lib/Appro_aew_control.a

#Face Recognition Libraries
ifeq ($(FACE_RECOG_MODE), ON)
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/fr_ti/alg_fr_ti.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/fr_ti/libeOkaoFr.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/fr_ti/libeOkaoPt.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/fr_ti/libeOkaoCo.a
endif

# DMVA libaries
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmval.a
CODEC_LIBS+=$(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmvastub.a

endif # ifndef $(INCLUDES_MK)
