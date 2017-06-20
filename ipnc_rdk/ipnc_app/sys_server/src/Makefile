# Makefile
#

TARGET = system_server

C_FLAGS += -Wall -O3

CPP_FLAGS += -I$(PUBLIC_INCLUDE_DIR) \
		-I../inc \
		-D_GNU_SOURCE \
		-D_LARGEFILE64_SOURCE \
		-D_FILE_OFFSET_BITS=64 

ifeq ($(FILESYS_MODE), NFS)
CPP_FLAGS += -D_DELAY_FOR_NFS
endif

ifeq ($(SENSOR_BOARD_VENDOR), LIMG)
CPP_FLAGS += -D_ONLY_TI2A
endif

ifeq ($(AES_MODULE_MODE), ON)
CPP_FLAGS += -DENCRYPT_ON
endif

ifeq ($(IPNC_DEVICE), DMVA1)
CPP_FLAGS += -D_DMVAx_
else ifeq ($(IPNC_DEVICE), DMVA2)
CPP_FLAGS += -D_DMVAx_
else ifeq ($(IPNC_DEVICE), DM369)
CPP_FLAGS += -D_DM369_
endif

CPP_FLAGS += $(XDC_FLAGS)
                
LD_FLAGS += -lfreetype -lpthread -s

COMPILE.c = $(BUILD_TOOL_PREFIX)gcc $(C_FLAGS) $(CPP_FLAGS) -c
LINK.c = $(BUILD_TOOL_PREFIX)gcc $(LD_FLAGS)

RELTARGET = release/$(TARGET)

RELCFLAGS = -fno-strict-aliasing

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h) $(PUBLIC_INCLUDE_DIR)/*.h

RELOBJFILES = $(SOURCES:%.c=release/%.o)

RELLDFLAGS =

.PHONY: clean release install

all:    release

install: release
	install -d $(IPNC_EXEC_DIR)
	install $(RELTARGET) $(IPNC_EXEC_DIR)
	ln -sf /tmp/localtime $(TARGET_FS)/usr/share/info/localtime
	ln -sf /tmp/localtime $(TARGET_FS)/usr/share/info/posixrules
	ln -sf /tmp/localtime $(TARGET_FS)/etc/localtime

release:    $(RELTARGET)

$(RELTARGET):   $(RELOBJFILES) $(APP_LIB_DIR)/file_msg_drv.a $(APP_LIB_DIR)/Appro_interface.a $(APP_LIB_DIR)/share_mem.a $(APP_LIB_DIR)/alarm_msg_drv.a $(APP_LIB_DIR)/ipnc_gio_util.a $(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/lib/cmem.a470MV
	$(LINK.c) -o $@ $^ $(RELLDFLAGS)


$(RELOBJFILES): release/%.o: %.c $(HEADERS)
	@mkdir -p release
	$(COMPILE.c) $(RELCFLAGS) -o $@ $<

clean:
	-$(RM) -rf release *.d
