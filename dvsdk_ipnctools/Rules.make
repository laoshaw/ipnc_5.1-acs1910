# Define target platform.
PLATFORM=dm368

# The installation directory of the DVSDK.
DVSDK_INSTALL_DIR=$(DVSDK_BASE_DIR)

# For backwards compatibility
DVEVM_INSTALL_DIR=$(DVSDK_INSTALL_DIR)

# Where the Codec Engine package is installed.
CE_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/codec-engine_2_26_02_11

# Where the codecs are installed.
CODEC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/codecs-dm365

# Where the DM365mm module is installed.
DM365MM_MODULE_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dm365mm-module_01_00_03

# Where the Framework Components package is installed.
FC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/framework-components_2_26_00_01

# Where the MFC Linux Utils package is installed.
LINUXUTILS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/linuxutils_2_26_02_05

# The directory to CMEM library
CMEM_INSTALL_DIR=$(LINUXUTILS_INSTALL_DIR)
CMEM_LIB_DIR := $(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/lib
CMEM_INC_DIR := $(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/include

# Where the XDAIS package is installed.
XDAIS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdais_6_26_01_03

# Where the RTSC tools package is installed.
XDC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdctools_3_16_03_36

# The directory that points to your kernel source directory.
LINUXKERNEL_INSTALL_DIR=$(KERNELDIR)

# The prefix to be added before the GNU compiler tools (optionally including # path), i.e. "arm_v5t_le-" or "/opt/bin/arm_v5t_le-".
CSTOOL_DIR=$(DVSDK_BASE_DIR)/linux-devkit/
CSTOOL_PREFIX=$(CSTOOL_DIR)/bin/arm-arago-linux-gnueabi-
MVTOOL_PREFIX=$(CSTOOL_PREFIX)

# Where temporary Linux headers and libs are installed.
LINUXLIBS_INSTALL_DIR=$(CSTOOL_DIR)/arm-arago-linux-gnueabi/usr

# Where to copy the resulting executables
EXEC_DIR=$(HOME)/install/$(PLATFORM)

# List all export needed
export CSTOOL_DIR
export CSTOOL_PREFIX
export CMEM_LIB_DIR
export CMEM_INC_DIR
