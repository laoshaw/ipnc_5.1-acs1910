
include $(AVCAPTURE_BUILD_DIR)/COMMON_HEADER.MK

INCLUDE+=$(CSL_INC)
INCLUDE+=$(IMAGE_TUNE_INC)
INCLUDE+=$(DRV_INC)
INCLUDE+=$(ALG_INC)
INCLUDE+=$(AV_SERVER_INC)
INCLUDE+=$(STREAM_INC)
INCLUDE+=-I$(PUBLIC_INCLUDE_DIR)
INCLUDE+=-I$(DVSDK_BASE_DIR)/dm365mm-module_01_00_03/Inc
INCLUDE+=-I$(IPNC_DIR)/multimedia/avi_save/armffmpeg
INCLUDE+=-I$(IPNC_DIR)/multimedia/avi_save/armffmpeg/libavformat
INCLUDE+=-I$(IPNC_DIR)/multimedia/avi_save/armffmpeg/libswscale

include $(AVCAPTURE_BUILD_DIR)/COMMON_FOOTER.MK


