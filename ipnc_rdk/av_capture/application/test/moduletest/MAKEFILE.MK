
#### BUILD USING DMVA LIB
include $(AVCAPTURE_BUILD_DIR)/COMMON_HEADER.MK

LIBS=$(LIB_DIR)/moduletest.a $(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmval.a $(AVCAPTURE_BUILD_DIR)/../framework/alg/lib/dmvastub.a $(LIB_DIR)/alg.a $(LIB_DIR)/drv.a $(LIB_DIR)/alg.a $(LIB_DIR)/image_tune.a $(LIB_DIR)/osa.a $(LIB_DIR)/csl.a $(CMEM_LIB)
LIBS+=$(CODEC_LIBS)
LIBS+=$(DM365MM_LIB)

ifeq ($(IMGS_ID), IMGS_MC1208_2MP)
	LIBS+=$(VIM_LIB)
endif

include $(AVCAPTURE_BUILD_DIR)/COMMON_FOOTER.MK
