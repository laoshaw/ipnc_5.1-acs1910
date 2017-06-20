include $(AVCAPTURE_BUILD_DIR)/COMMON_HEADER.MK

include $(AVCAPTURE_BUILD_DIR)/COMMON_FOOTER.MK

install:DO_COPY
	echo "install sensor_ctrl"
	
	
all: DO_COPY
	echo "all sensor_ctrl"
	
	
clean:
	echo "clean sensor_ctrl"
	

depend:DO_COPY
	echo "depend sensor_ctrl"
	
DO_COPY:
	cp --remove-destination sensor_ctrl.o.prebuild $(OBJ_DIR)/sensor_ctrl.o
	
