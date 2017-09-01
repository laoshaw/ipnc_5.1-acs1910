//------------------------------------------------------------------------
/**
\file     dh_h264_osd.h
\brief    该模块为swosdLink_tsk.c服务，定义了OSD叠加用到的全局变量，
          该全局变量也是osd结构体，叠加信息全都保存在该全局变量中
\version  v1.0.0.0
\date     2012-10-19
\author   fengxin

Copyright (c) 2011-2012 China Daheng Group, Inc. 
Beijing Image Vision Technology Branch and all right reserved.
*/
//------------------------------------------------------------------------
#ifndef __DH_H264_OSD_H
#define __DH_H264_OSD_H

#include <asm/types.h>
#include "dh_osd.h"
//#include <dh_cmd_def.h>
//#include "dh_system_default.h"
//add by lhb 2013-5-3
//#include <mcfw/src_bios6/links_m3vpss/camera/dh_white_balance.h>
///osd字符叠加结构体

#define DH_TEXT_LEN 20

typedef struct __Dh_Stream_OSD_Config   ///<start fp: 1196
{
    __u32   enable;                         ///<fp:0，使能OSD功能，总开关，默认：0；0，关闭；1，开启
    __u32   timeen;                         ///<fp:4，OSD是否叠加时间，默认：0；0，关闭；1，开启
    __u32   timeformat;                     ///<fp:8，OSD叠加时间格式，默认：0；0: YYYY/MM/DD; 1:MM/DD/YYYY; 2:DD/MM/YYYY；
    __u32   addren;                         ///<fp:12，OSD是否叠加地点，默认：0；Bit0表示使能
    __u8    addrtext[DH_TEXT_LEN*2];        ///<fp:16，OSD地点信息，默认“\0”
    __u32   directionen;                    ///<fp:80，OSD是否叠加方向，默认：0；0，关闭；1，开启
    __u8    directiontext[DH_TEXT_LEN*2];   ///<fp:84，OSD方向信息，默认“\0”
    __u32   userdef1en;                     ///<fp:148，OSD用户自定义信息1使能，默认：0；0，关闭；1，开启
    __u8    userdef1text[DH_TEXT_LEN*2];    ///<fp:152，OSD车道号信息，默认“\0”
    __u32   userdef2en;                     ///<fp:216，OSD用户自定义信息2使能，默认：0；0，关闭；1，开启
    __u8    userdef2text[DH_TEXT_LEN*2];    ///<fp:220，OSD车道号信息，默认“\0”
    __u32   reserved[64];                   //fp:284，reserved
}Dh_Stream_OSD_Config;                  ///total len 540

extern Dh_Stream_OSD_Config g_h264_osd_cfg;

///将av_server传递来的osd结构体赋值给M3全局变量
int dh_h264_osd_setparam(Dh_Stream_OSD_Config *osd_cfg);

///根据全局变量g_h264_osd_cfg中的信息实现OSD叠加
int dh_h264_osd_apply(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h);

///把时间戳设置进帧信息，同时生存叠加字符串
int dh_set_frame_timestamp(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h);

///把白平衡分量设置进帧信息
//int dh_set_frame_whitebalance(unsigned int yuv_data_addr, 
 //                           awb_ratio_st wb_param, 
 //                           unsigned int img_w, unsigned int img_h);


#endif //__DH_H264_OSD_H

