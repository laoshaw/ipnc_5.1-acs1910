ROOTDIR = ../
-include $(ROOTDIR)/Rules.make
TARGET = dm365mmap.ko

obj-y += dm365mmap.o

EXTRA_CFLAGS +=	-DUSE_UDEV=$(USE_UDEV) \
		-DMAX_POOLS=$(MAX_POOLS)

MAKE_ENV = ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX)


debug: EXTRA_CFLAGS += -D__DEBUG

.PHONY: clean debug release install default

default: release

docs:

release:
	make -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" 

debug:
	make $(INCLUDES) -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

install:
	install -d $(EXEC_DIR)

clean:
	-rm -rf *.o *mod* *.ko .cmem* .tmp*
