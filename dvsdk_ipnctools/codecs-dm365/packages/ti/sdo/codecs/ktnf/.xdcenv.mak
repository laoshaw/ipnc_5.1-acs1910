#
_XDCBUILDCOUNT = 1
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = D:/CCS_3.3/codec_engine_2_22/packages;D:/CCS_3.3/xdais_6_22_01/packages;D:/CCS_3.3/ceutils-v1.07/packages;
override XDCROOT = D:/CCS_3.3/xdctools_3_10_02
override XDCBUILDCFG = D:/CCS_3.3/xdctools_3_10_02/etc/config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = D:/CCS_3.3/codec_engine_2_22/packages;D:/CCS_3.3/xdais_6_22_01/packages;D:/CCS_3.3/ceutils-v1.07/packages;D:/CCS_3.3/xdctools_3_10_02/packages;../../../..
HOSTOS = Windows
endif
