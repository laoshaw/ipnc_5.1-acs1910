#ifndef _AV_SERVER_DEBUG_H_
#define _AV_SERVER_DEBUG_H_

//  #define AVSERVER_DEBUG_API
//  #define AVSERVER_DEBUG_MAIN_THR
//  #define AVSERVER_DEBUG_RUNNING

//  #define AVSERVER_DEBUG_VIDEO_CAPTURE_THR
//  #define AVSERVER_DEBUG_VIDEO_LDC_THR
//  #define AVSERVER_DEBUG_VIDEO_VNF_THR
//  #define AVSERVER_DEBUG_VIDEO_RESIZE_THR
//  #define AVSERVER_DEBUG_VIDEO_ENCODE_THR
//  #define AVSERVER_DEBUG_VIDEO_ENCRYPT_THR
//  #define AVSERVER_DEBUG_VIDEO_STREAM_THR
//  #define AVSERVER_DEBUG_VIDEO_2A_THR
//  #define AVSERVER_DEBUG_VIDEO_VS_THR
//  #define AVSERVER_DEBUG_VIDEO_SWOSD_THR
//  #define AVSERVER_DEBUG_VIDEO_FD_THR
//  #define AVSERVER_DEBUG_VIDEO_DISPLAY_THR
//  #define AVSERVER_DEBUG_VIDEO_SWOSD
//  #define AVSERVER_DEBUG_VIDEO_MOTION_THR
//	#define AVSERVER_DEBUG_VIDEO_ENCRYPT_THR

//  #define AVSERVER_DEBUG_AUDIO_THR

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

#endif  /*  _AV_SERVER_DEBUG_H_  */

