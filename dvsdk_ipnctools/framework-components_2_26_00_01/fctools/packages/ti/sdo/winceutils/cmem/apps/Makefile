# 
# 

#
# ======== Makefile ========
#
SUBDIRS = apitest translate insmod_rmmod multi_process

DEBUGSUBDIRS = $(addsuffix .debug, $(SUBDIRS))
RELEASESUBDIRS = $(addsuffix .release, $(SUBDIRS))
CLEANSUBDIRS = $(addsuffix .clean, $(SUBDIRS))
INSTALLSUBDIRS = $(addsuffix .install, $(SUBDIRS))


.PHONY: $(SUBDIRS) $(DEBUGSUBDIRS) $(RELEASESUBDIRS) $(INSTALLSUBDIRS) \
	$(CLEANSUBDIRS)

all:	$(SUBDIRS)

$(SUBDIRS):
	@echo
	@echo Making all in subdirectory $@...
	@$(MAKE) -C $@

install: $(INSTALLSUBDIRS)

$(DEBUGSUBDIRS):
	@echo
	@echo Executing make debug in subdirectory $(basename $@)...
	@cd $(basename $@) ; $(MAKE) debug

debug:	$(DEBUGSUBDIRS)

$(RELEASESUBDIRS):
	@echo
	@echo Executing make release in subdirectory $(basename $@)...
	@cd $(basename $@) ; $(MAKE) release

release:	$(RELEASESUBDIRS)

$(INSTALLSUBDIRS):
	@echo
	@echo Executing make install in subdirectory $(basename $@)...
	@cd $(basename $@) ; $(MAKE) install

clean:	$(CLEANSUBDIRS)

$(CLEANSUBDIRS):
	@cd $(basename $@) ; $(MAKE) clean
#
#  @(#) ti.sdo.winceutils.cmem; 1, 0, 0,47; 11-9-2010 15:33:11; /db/atree/library/trees/winceutils/winceutils-b01x/src/ xlibrary

#

