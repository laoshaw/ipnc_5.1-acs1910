
include $(AVCAPTURE_BUILD_DIR)/COMMON_HEADER.MK
LIBS=$(LIB_DIR)/av_server.a $(LIB_DIR)/alg.a  $(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmval.a $(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmvastub.a $(LIB_DIR)/alg.a $(LIB_DIR)/drv.a $(LIB_DIR)/image_tune.a $(LIB_DIR)/osa.a $(LIB_DIR)/csl.a $(CMEM_LIB)


#### BUILD USING DMVA SOURCE
#include $(BASE_DIR)/COMMON_HEADER.MK
#LIBS=$(LIB_DIR)/av_server.a $(LIB_DIR)/dmval.a $(LIB_DIR)/dmvastub.a $(LIB_DIR)/alg.a $(LIB_DIR)/drv.a $(LIB_DIR)/image_tune.a $(LIB_DIR)/osa.a $(LIB_DIR)/csl.a $(CMEM_LIB)

# AVI format
CODEC_LIBS+=$(IPNC_DIR)/multimedia/avi_save/lib/libavdevice.a
CODEC_LIBS+=$(IPNC_DIR)/multimedia/avi_save/lib/libavformat.a
CODEC_LIBS+=$(IPNC_DIR)/multimedia/avi_save/lib/libavcodec.a
CODEC_LIBS+=$(IPNC_DIR)/multimedia/avi_save/lib/libavutil.a
CODEC_LIBS+=$(IPNC_DIR)/multimedia/avi_save/lib/libx264.a

CODEC_LIBS+=$(APP_LIB_DIR)/file_msg_drv.a
CODEC_LIBS+=$(APP_LIB_DIR)/Appro_interface.a
CODEC_LIBS+=$(APP_LIB_DIR)/share_mem.a
CODEC_LIBS+=$(APP_LIB_DIR)/ipnc_gio_util.a

LIBS+=$(CODEC_LIBS) 
LIBS+=$(APP_LIB_DIR)/msg_util.a
LIBS+=$(APP_LIB_DIR)/alarm_msg_drv.a
LIBS+=$(DM365MM_LIB)
ifeq ($(IMGS_ID), IMGS_MC1208_2MP)
	LIBS+=$(VIM_LIB)
endif

INCLUDE+=$(CSL_INC)
INCLUDE+=$(IMAGE_TUNE_INC)
INCLUDE+=$(DRV_INC)
INCLUDE+=$(ALG_INC)
INCLUDE+=$(DMVAL_INC)	
INCLUDE+=$(AV_SERVER_INC)
INCLUDE+=$(STREAM_INC)
INCLUDE+=-I$(PUBLIC_INCLUDE_DIR)

include $(AVCAPTURE_BUILD_DIR)/COMMON_FOOTER.MK


