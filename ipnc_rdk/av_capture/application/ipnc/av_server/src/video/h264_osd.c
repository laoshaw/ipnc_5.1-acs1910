#include <stdio.h>
#include <string.h>
#include <time.h>
#include "h264_osd.h"
#include "dh_osd.h"
#include "asc32.h"
#include "hzk32.h"
#include "asc16.h"
#include "hzk16.h"
//#include "cntrl_SignalingInterface.h"
//#include <dh_err_cmd_def.h>
//#include <dh_system_default.h>
//#include <mcfw/src_bios6/links_m3vpss/alg/sw_osd/swosd_priv.h>

//add by lhb 2013-5-3
//#include <mcfw/src_bios6/links_m3vpss/camera/dh_white_balance.h> 

Dh_Stream_OSD_Config g_h264_osd_cfg = {0}; ///<M3中OSD叠加用到的全局变量

//extern SWOSD_config_t gSWOSD_config;  //add by gomo 2012-12-18

//------------------------------------------------------------------------
/**
\brief 将时间信息格式化为字符串
\param time1 年月日
\param time2 时分秒毫秒
\param timeformat 时间字符串格式 0:YYYY/MM/DD; 1:MM/DD/YYYY; 2:DD/MM/YYYY
\param timestr 时间字符串

\return 0:成功返回 其它:失败返回 
*/
//------------------------------------------------------------------------
int dh_time2str(unsigned int time1, unsigned int time2, 
            int timeformat, unsigned char *timestr)
{
    unsigned int year = 0;
    unsigned int month = 0;
    unsigned int day = 0;
    unsigned int hour = 0;
    unsigned int minite = 0;
    unsigned int second = 0;
    unsigned int msecond = 0;

    if(timestr == NULL)
    {
        return -1;//STATUS_POINTER_IS_NULL;
    }
    if(timeformat > 2 || timeformat < 0)
    {
        return -1;//STATUS_XB_PARAM_RANGE_ERROR;
    }
    
    year = ((time1 & 0xffff0000) >> 16);
    month = ((time1 & 0x0000ff00) >> 8);
    day = time1 & 0x000000ff;

    hour = ((time2 & 0xFC000000) >> 26);
    minite = ((time2 & 0x03F00000) >> 20);
    second = ((time2 & 0x000FC000) >> 14);
    msecond = (time2 & 0x000003FF) * 10;//为帧信息解析兼容性，A8获取时间时除以了10
    
	switch(timeformat)
    {
        case 0:
            sprintf((char *)timestr,"%04d-%02d-%02d %02d:%02d:%02d:%03d", 
                    year, month, day, hour, minite, second, msecond);
            break;
        case 1:
            sprintf((char *)timestr,"%02d-%02d-%04d %02d:%02d:%02d:%03d", 
                month, day, year, hour, minite, second, msecond);
            break;
        case 2:
            sprintf((char *)timestr,"%02d-%02d-%04d %02d:%02d:%02d:%03d", 
                day, month, year, hour, minite, second, msecond);
            break;
        default:
            break;
    }
    
	return 0;
}

#ifdef RTC_COTEX_A8 //modify by gomo 2012-12-19
//------------------------------------------------------------------------
/**
\brief 把时间戳设置进帧信息
\param YUV数据地址
\param img_w 图像宽
\param img_h 图像高
\param timestr 生存叠加字符串 

\return 0:成功返回 其它:失败返回 
*/
//------------------------------------------------------------------------
int dh_set_frame_timestamp(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h)
{
    int time1 = 0; ///<年月日
    int time2 = 0; ///<时分秒毫秒

    time1 = ((gSWOSD_config.dateTimeInfo.year & 0xFFFF) << 16) |
                ((gSWOSD_config.dateTimeInfo.month & 0xFF) << 8) |
                (gSWOSD_config.dateTimeInfo.mday & 0xFF);
    time2 = ((gSWOSD_config.dateTimeInfo.hours & 0xFF) << 26) |
                ((gSWOSD_config.dateTimeInfo.min & 0xFF) << 20)|
                ((gSWOSD_config.dateTimeInfo.sec & 0xFF) << 14)|
                (gSWOSD_config.dateTimeInfo.mesec & 0x3FF);

    unsigned char *ptr = (unsigned char *)yuv_data_addr + img_w * img_h * 3 / 2;

    *(int *)(ptr + 4 + 12 * 8 + 4) = time1; //时间戳帧信息是第13个和第14个
    *(int *)(ptr + 4 + 13 * 8 + 4) = time2;
  
   // printf("time1=0x%x, time2=0x%x\n", time1, time2);

    return 0;
}
#endif //#ifdef RTC_COTEX_A8 //modify by gomo 2012-12-19

//add by lhb 2013-5-3

//------------------------------------------------------------------------
/**
\brief 将av_server传递来的osd结构体赋值给M3全局变量
\param osd_cfg OSD相关结构体 @sa Dh_Stream_OSD_Config

\return 0:成功返回 其它:失败返回 
*/
//------------------------------------------------------------------------
int dh_h264_osd_setparam(Dh_Stream_OSD_Config *osd_cfg)
{
    if(osd_cfg == NULL)
    {
        return -1;//STATUS_POINTER_IS_NULL;
    }
    memcpy(&g_h264_osd_cfg, osd_cfg, sizeof(Dh_Stream_OSD_Config)); 

    return 0;
}

//------------------------------------------------------------------------
/**
\brief 通过帧信息获取时间
\param yuv_data_addr YUV数据地址

\return 0:成功返回 其它:失败返回 
*/
//------------------------------------------------------------------------

int dh_get_time_from_frameinfo(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h,
                            unsigned char *timestr)
{
    if(timestr == NULL)
    {
        return -1;//STATUS_POINTER_IS_NULL;
    }
    
    int time1 = 0; ///<年月日
    int time2 = 0; ///<时分秒毫秒
    
    unsigned char *buffer 
                = (unsigned char *)yuv_data_addr + img_w * img_h * 3 / 2;

    time1 = 0x12345678;//*(int *)(buffer + 4 + 12 * 8 + 4); //时间戳帧信息是第13个和第14个
    time2 = 0x12345678;//*(int *)(buffer + 4 + 13 * 8 + 4);

    dh_time2str(time1, time2, g_h264_osd_cfg.timeformat, timestr);

    return 0;
}

//------------------------------------------------------------------------
/**
\brief 根据全局变量g_h264_osd_cfg中的信息实现OSD叠加
\param yuv_data_addr YUV数据地址
\param img_w YUV数据宽度
\param img_h YUV数据高度

\return 0:成功返回 其它:失败返回 
*/
//------------------------------------------------------------------------

char *wday[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

int dh_h264_osd_apply(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h)
{
    int ret = 0;
    int i = 0;
    int start_x = 88;//DH_OSD_START_X;
    int start_y = 0;//DH_OSD_START_Y;
    int font_w = 32;//DH_OSD_FONT_32;
    int font_h = 32;//DH_OSD_FONT_32;
    //unsigned int timedata = 0x7dc0a0a; //2012-10-10
    unsigned char timestr[41] = {0}; 
    unsigned char osd_buffer[41] = {0};//MAX_STRING_LEN] = {0};
    int osd_flag = 0; //是否叠加标志，当osd_flag=1时进行一行叠加
    unsigned char *hzk = NULL;
    unsigned char *asc = NULL;
    time_t b_time;
    struct tm *date_time;
    

    DH_OsdParm pOsdParm = {
        .osd_y = 255,
    	.osd_u = 0x80,
    	.osd_v = 0x80,
    	.osd_outline = 0
    };


 //   if(img_w > RES_720P_WIDTH)//如果分辨率大于720P，字库字体用32号，否则用16号
  //  {
        hzk = g_hzk32;
        asc = g_asc32;
        font_w = 32;//DH_OSD_FONT_32;
        font_h = 32;//DH_OSD_FONT_32;
//    }
//    else
/*
    {
        hzk = g_hzk16;
        asc = g_asc16;
        font_w = 16;//DH_OSD_FONT_16;
        font_h = 16;//DH_OSD_FONT_16;
    }
   */ 
    //func_获取时间信息(yuv_data_addr); //通过YUV数据中的帧信息获取时间信息
    //dh_get_time_from_frameinfo(yuv_data_addr, img_w, img_h, timestr);
	//g_h264_osd_cfg.addrtext = "中航视嘉";
    //分行叠加osd信息，包括时间、地点、方向等。每循环一次叠加一个内容并换行
    //i<5是由于总共有5中osd信息，每种信息要换行
    b_time = time(NULL);
    date_time = localtime(&b_time);
    date_time->tm_year = date_time->tm_year + 1900;
    sprintf(timestr, "%d-%02d-%02d %s %02d:%02d:%02d", 
            date_time->tm_year, date_time->tm_mon + 1, date_time->tm_mday, wday[date_time->tm_wday], 
            date_time->tm_hour, date_time->tm_min, date_time->tm_sec);

    
    for(i = 0; i < 5; i++)
    {
        if(g_h264_osd_cfg.timeen && (i == 0))
        {
            sprintf((char *)osd_buffer, "%s", "时间: ");
            snprintf((char *)(osd_buffer+strlen("时间: ")), DH_TEXT_LEN * 2,
                "%s", timestr);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.addren && (i == 1))
        {
            sprintf((char *)osd_buffer, "%s", "位置: ");
            snprintf((char *)(osd_buffer+strlen("位置: ")), DH_TEXT_LEN * 2, 
                "%s", g_h264_osd_cfg.addrtext);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.directionen && (i == 2))
        {
            sprintf((char *)osd_buffer, "%s", "编号: ");
            snprintf((char *)(osd_buffer+strlen("编号: ")), DH_TEXT_LEN * 2,
                "%s", g_h264_osd_cfg.directiontext);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.userdef1en && (i == 3))
        {
            snprintf((char *)osd_buffer, DH_TEXT_LEN * 2,
                "%s", g_h264_osd_cfg.userdef1text);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.userdef2en && (i == 4))
        {
            snprintf((char *)osd_buffer, DH_TEXT_LEN * 2,
                "%s", g_h264_osd_cfg.userdef2text);
            osd_flag = 1;
        }

        if(osd_flag == 1)
        {
            ret = DH_Osd(osd_buffer,        //osd叠加内容，从g_h264_osd_cfg获得
                (unsigned char *)yuv_data_addr, //YUV数据地址
                hzk, asc,                       //字库数组
                font_w, font_h,                 //字体大小
                start_x, start_y,               //字符叠加起始位置
                img_w, img_h,                   //YUV数据宽高
                DH_OSD_YUV_420,                 //YUV数据格式YUV420
                &pOsdParm);                     //字符叠加颜色
            
            start_y = start_y + font_h;         //循环叠加换行
		//void *memset(void *s, int c, size_t n);
		memset(osd_buffer, '\0', DH_TEXT_LEN * 2);
			
            osd_flag = 0;
        }
    }
    
    return ret;
}



