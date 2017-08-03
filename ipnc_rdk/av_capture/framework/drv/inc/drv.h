
#ifndef _DRV_H_
#define _DRV_H_

#include <osa.h>

//#define DRV_BUF_CLEAR

//#define DRV_SHOW_INFO
//#define SENSOR_REG_DUMP

#define DRV_REFERENCE_CLK_HZ     (24000000.0)

#define DRV_DATA_FORMAT_RAW      0
#define DRV_DATA_FORMAT_YUV422   1
#define DRV_DATA_FORMAT_YUV420   2

#define DRV_DATA_FORMAT_INTERLACED    (0x0100)

#define VI_DEBUG_MODE

#ifdef VI_DEBUG_MODE
#define VI_DEBUG(...) \
    do \
    { \
        fprintf(stderr, " DEBUG (%s|%s|%d): ", \
                __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    while(0)

#else
#define VI_DEBUG(...)
#endif


typedef struct {

  Uint32 armClkHz;
  Uint32 ddrClkHz;
  Uint32 vpssClkHz;
  Uint32 imcopClkHz;

} DRV_ClkHz;

int DRV_init();
int DRV_exit();

int DRV_regDump(Uint32 regAddr, Uint32 numRegs, Uint32 numPrintCols);
int DRV_regWrite(Uint32 regAddr, Uint32 regValue);

void DRV_imgWidthHeightPad(int sensorMode, Uint16 *width, Uint16 *height);
void DRV_imgGetWidthHeight(int sensorMode, Uint16 *width, Uint16 *height);
void DRV_imgSetWidthHeightYuvInterlacedMode(Uint16 width,Uint16 height);

int DRV_clkGetClkHz(DRV_ClkHz *clk, Bool printResults);

int DRV_vpssEnableBoxcar(Bool enable);

#endif

