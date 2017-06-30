#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codec-engine_2_26_02_11/examples;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codecs-dm365/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codec-engine_2_26_02_11/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/xdais_6_26_01_03/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/framework-components_2_26_00_01/packages;/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/linuxutils_2_26_02_05/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/framework-components_2_26_00_01/fctools/packages
override XDCROOT = /home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/xdctools_3_16_03_36
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codec-engine_2_26_02_11/examples;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codecs-dm365/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/codec-engine_2_26_02_11/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/xdais_6_26_01_03/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/framework-components_2_26_00_01/packages;/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/linuxutils_2_26_02_05/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/framework-components_2_26_00_01/fctools/packages;/home/pamsimochen/arable_land/acs1910/dvsdk_ipnctools/xdctools_3_16_03_36/packages;..
HOSTOS = Linux
endif
