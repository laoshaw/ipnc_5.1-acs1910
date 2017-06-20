SUBDIRS = root-filesys

all: 
	@echo "do nothing!!!"

root/.authenticate/ca:
	mkdir root/.authenticate/ca

install: root/.authenticate/ca
	cp -aR -f --remove-destination etc/ $(TARGET_FS)/
	rm $(TARGET_FS)/etc/init.d/rcS_*
ifeq ($(SYSTEM_PLATFORM), EVM)
	cp etc/init.d/rcS_evm $(TARGET_FS)/etc/init.d/rcS
else
ifeq ($(HARDWARE_CFG), dm365)
	cp etc/init.d/rcS_dm365_ipnc $(TARGET_FS)/etc/init.d/rcS
else
	cp etc/init.d/rcS_dm368_ipnc $(TARGET_FS)/etc/init.d/rcS
endif
endif
	rm $(TARGET_FS)/etc/fw_env*
ifeq ($(HARDWARE_CFG), dm365)
	cp etc/fw_env_dm365.config $(TARGET_FS)/etc/fw_env.config	
else
	cp etc/fw_env_dm368.config $(TARGET_FS)/etc/fw_env.config
endif
	cp -aR -f --remove-destination opt/ $(TARGET_FS)/
#	sudo cp -aR -f --remove-destination root/ $(TARGET_FS)/
	cp -aR -f --remove-destination usr/ $(TARGET_FS)/
	rm -f $(TARGET_FS)/etc/resolv.conf
	rm -f $(TARGET_FS)/usr/local/etc/esmtprc
	rm -f $(TARGET_FS)/home/root/.esmtprc
	rm -f $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.cache
	rm -f $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.info
	rm -f $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.info.old
	ln -s /tmp/resolv.conf $(TARGET_FS)/etc/resolv.conf
	ln -s /tmp/.esmtprc $(TARGET_FS)/usr/local/etc/esmtprc
	ln -s /tmp/.esmtprc $(TARGET_FS)/home/root/.esmtprc
	ln -s /tmp/dhcpcdhcpcd-eth0.cache $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.cache
	ln -s /tmp/dhcpcdhcpcd-eth0.info $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.info
	ln -s /tmp/dhcpcdhcpcd-eth0.info.old $(TARGET_FS)/etc/dhcpcdhcpcd-eth0.info.old
#	sudo chown -R root:root $(TARGET_FS)/root/.authenticate
#	sudo chmod -R 0600 $(TARGET_FS)/root/.authenticate
#	sudo chmod 0700 $(TARGET_FS)/root/.authenticate

clean:
	@echo "do nothing!!!"

