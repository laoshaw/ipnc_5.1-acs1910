#ifndef __DRV_ROI_H
#define __DRV_ROI_H

#define ROI_DEBUG_MODE
#ifdef ROI_DEBUG_MODE
#define ROI_DEBUG(...) \
    do \
    { \
        fprintf(stderr, " DEBUG (%s|%s|%d): ", \
                __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    while(0)

#else
#define ROI_DEBUG(...)
#endif


#define DEFAULT_AVG_MAX 130
#define DEFAULT_AVG_MIN 110
#define DEFAULT_AVG_TARGET ((DEFAULT_AVG_MAX + DEFAULT_AVG_MIN) >> 1)

#define DEFAULT_SHUTTER_MAX 200000
#define DEFAULT_SHUTTER_MIN 36

void VIM_roi_autoexp_thread();
int VIM_roi_autoexp_thread_init();

#endif

