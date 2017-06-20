SUBDIRS = util gui

.PHONY: $(SUBDIRS) $(DEBUGSUBDIRS) $(RELEASESUBDIRS) $(INSTALLSUBDIRS) \
	$(CLEANSUBDIRS)

all: 
	@echo "do nothing!!!"

install:
	rm -rf $(TARGET_FS)/var/www
	cp -aR -f gui/. $(TARGET_FS)/var/www/
	cp -aR -f util/. $(TARGET_FS)/var/www/
	cp -aR -f etc/ $(TARGET_FS)/
	cp -aR -f alarm_wav/. $(IPNC_EXEC_DIR)/
	rm -f $(TARGET_FS)/var/www/tvdevicedesc.xml
	ln -s /tmp/tvdevicedesc.xml $(TARGET_FS)/var/www/tvdevicedesc.xml
	
clean:
	@echo "do nothing!!!"

