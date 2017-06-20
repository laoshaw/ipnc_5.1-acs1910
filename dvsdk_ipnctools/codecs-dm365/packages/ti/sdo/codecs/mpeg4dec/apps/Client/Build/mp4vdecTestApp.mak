#Environment variable settings
XDAIS_INSTALL_DIR = $(FC_INSTALL_DIR)/fctools

#XDC related settings
#XDC_GLOBAL_PATH1 = C:/CCStudio_v3.3/xdctools_3_10_02
CONFIG = mp4vdecTestApp
XDCTARGET = ti.targets.arm.Arm9t
XDCPLATFORM = ti.platforms.evmDM365
#XDCPLATFORM = ti.platforms.evmDM355
XDCPATH = ^;$(XDAIS_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/examples;$(XDCROOT)/packages;

export XDCPATH
export XDAIS_INSTALL_DIR

#Makefile variable settings for project compilation
CGTOOLS = C:/CCStudio_v3.3/tms470/cgtools
CC = $(CGTOOLS)/bin/cl470
LNK = $(CGTOOLS)/bin/lnk470
RTS = $(CGTOOLS)/lib/rts32earm9.lib

#Compilation related options
CFLAGS = -o3 -oi200 -mc -mf -mv5e  --abi=ti_arm9_abi --code_state=32 --endian=little
CFLAGS_1 =  -oi200 -md -me -mf -ml -mv5e  --abi=ti_arm9_abi --code_state=32 --endian=little
CFLAGS_XDC = -@./$(CONFIG)/compiler.opt
INCLUDES = -I../Test/Inc -I../../Inc  -I../../../include 
INCLUDES += -I$(FC_INSTALL_DIR)/packages 
INCLUDES += -I$(FC_INSTALL_DIR)/fctools/packages
INCLUDES += -IC:/CCStudio_v3.3/tms470/cgtools/include


DEFS = 
OUTFILE =  
LIBS = -c -w -x -l$(RTS)  -l"../../Lib/mp4vdecAlg.lib"
CMDFILE = -l=./mp4vdecApp.cmd

OBJECTS = buffermanager.obj  TestAppConfigParser.obj mpeg4DecTestApp.obj alg_control.obj alg_create.obj alg_malloc.obj idma3_create.obj vicp_framework.obj testapp_arm926intc.obj testapp_inthandler.obj testapp_intvecs.obj testapp_mmutable.obj lockmp.obj mylinuxutils.obj 

all: testAppObj
	if not exist .\Out\
		md Out
	$(LNK) $(LIBS) $(CMDFILE) $(OBJECTS) ./$(CONFIG)/linker.cmd $(RTS) -o./Out/mp4vdecApp.out
	del *.obj
  
testAppObj: $(CONFIG)/linker.cmd $(CONFIG)/compiler.opt $(OBJECTS)

alg_control.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/alg_control.c
alg_create.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/alg_create.c 
alg_malloc.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/alg_malloc.c 
buffermanager.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/buffermanager.c 
idma3_create.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/idma3_create.c 
mpeg4DecTestApp.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/mpeg4DecTestApp.c 
TestAppConfigParser.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/TestAppConfigParser.c 
testapp_arm926intc.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/ccs/testapp_arm926intc.c 
testapp_inthandler.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS_1) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/ccs/testapp_inthandler.asm 
testapp_intvecs.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS_1) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/ccs/testapp_intvecs.asm
testapp_mmutable.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS_1) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/ccs/testapp_mmutable.asm
vicp_framework.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/vicp_framework.c
mylinuxutils.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/mylinuxutils.c
lockmp.obj:
	$(CC)     $(CFLAGS_XDC) $(CFLAGS) $(OUTFILE)  $(INCLUDES) $(DEFS)  ../Test/Src/lockmp.c


$(CONFIG)/linker.cmd $(CONFIG)/compiler.opt : $(CONFIG).cfg
	xs xdc.tools.configuro -b $(CONFIG).bld -c $(CGTOOLS) -t $(XDCTARGET) -p $(XDCPLATFORM) $<

clean:
	del /Q *.obj $(CONFIG)\ Out\
