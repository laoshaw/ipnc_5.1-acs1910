//------------------------------------------------------------------------
/**
\file     dh_h264_osd.h
\brief    ��ģ��ΪswosdLink_tsk.c���񣬶�����OSD�����õ���ȫ�ֱ�����
          ��ȫ�ֱ���Ҳ��osd�ṹ�壬������Ϣȫ�������ڸ�ȫ�ֱ�����
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
///osd�ַ����ӽṹ��

#define DH_TEXT_LEN 20

typedef struct __Dh_Stream_OSD_Config   ///<start fp: 1196
{
    __u32   enable;                         ///<fp:0��ʹ��OSD���ܣ��ܿ��أ�Ĭ�ϣ�0��0���رգ�1������
    __u32   timeen;                         ///<fp:4��OSD�Ƿ����ʱ�䣬Ĭ�ϣ�0��0���رգ�1������
    __u32   timeformat;                     ///<fp:8��OSD����ʱ���ʽ��Ĭ�ϣ�0��0: YYYY/MM/DD; 1:MM/DD/YYYY; 2:DD/MM/YYYY��
    __u32   addren;                         ///<fp:12��OSD�Ƿ���ӵص㣬Ĭ�ϣ�0��Bit0��ʾʹ��
    __u8    addrtext[DH_TEXT_LEN*2];        ///<fp:16��OSD�ص���Ϣ��Ĭ�ϡ�\0��
    __u32   directionen;                    ///<fp:80��OSD�Ƿ���ӷ���Ĭ�ϣ�0��0���رգ�1������
    __u8    directiontext[DH_TEXT_LEN*2];   ///<fp:84��OSD������Ϣ��Ĭ�ϡ�\0��
    __u32   userdef1en;                     ///<fp:148��OSD�û��Զ�����Ϣ1ʹ�ܣ�Ĭ�ϣ�0��0���رգ�1������
    __u8    userdef1text[DH_TEXT_LEN*2];    ///<fp:152��OSD��������Ϣ��Ĭ�ϡ�\0��
    __u32   userdef2en;                     ///<fp:216��OSD�û��Զ�����Ϣ2ʹ�ܣ�Ĭ�ϣ�0��0���رգ�1������
    __u8    userdef2text[DH_TEXT_LEN*2];    ///<fp:220��OSD��������Ϣ��Ĭ�ϡ�\0��
    __u32   reserved[64];                   //fp:284��reserved
}Dh_Stream_OSD_Config;                  ///total len 540

extern Dh_Stream_OSD_Config g_h264_osd_cfg;

///��av_server��������osd�ṹ�帳ֵ��M3ȫ�ֱ���
int dh_h264_osd_setparam(Dh_Stream_OSD_Config *osd_cfg);

///����ȫ�ֱ���g_h264_osd_cfg�е���Ϣʵ��OSD����
int dh_h264_osd_apply(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h);

///��ʱ������ý�֡��Ϣ��ͬʱ��������ַ���
int dh_set_frame_timestamp(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h);

///�Ѱ�ƽ��������ý�֡��Ϣ
//int dh_set_frame_whitebalance(unsigned int yuv_data_addr, 
 //                           awb_ratio_st wb_param, 
 //                           unsigned int img_w, unsigned int img_h);


#endif //__DH_H264_OSD_H

