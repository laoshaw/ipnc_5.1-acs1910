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

Dh_Stream_OSD_Config g_h264_osd_cfg = {0}; ///<M3��OSD�����õ���ȫ�ֱ���

//extern SWOSD_config_t gSWOSD_config;  //add by gomo 2012-12-18

//------------------------------------------------------------------------
/**
\brief ��ʱ����Ϣ��ʽ��Ϊ�ַ���
\param time1 ������
\param time2 ʱ�������
\param timeformat ʱ���ַ�����ʽ 0:YYYY/MM/DD; 1:MM/DD/YYYY; 2:DD/MM/YYYY
\param timestr ʱ���ַ���

\return 0:�ɹ����� ����:ʧ�ܷ��� 
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
    msecond = (time2 & 0x000003FF) * 10;//Ϊ֡��Ϣ���������ԣ�A8��ȡʱ��ʱ������10
    
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
\brief ��ʱ������ý�֡��Ϣ
\param YUV���ݵ�ַ
\param img_w ͼ���
\param img_h ͼ���
\param timestr ��������ַ��� 

\return 0:�ɹ����� ����:ʧ�ܷ��� 
*/
//------------------------------------------------------------------------
int dh_set_frame_timestamp(unsigned int yuv_data_addr, 
                            unsigned int img_w, unsigned int img_h)
{
    int time1 = 0; ///<������
    int time2 = 0; ///<ʱ�������

    time1 = ((gSWOSD_config.dateTimeInfo.year & 0xFFFF) << 16) |
                ((gSWOSD_config.dateTimeInfo.month & 0xFF) << 8) |
                (gSWOSD_config.dateTimeInfo.mday & 0xFF);
    time2 = ((gSWOSD_config.dateTimeInfo.hours & 0xFF) << 26) |
                ((gSWOSD_config.dateTimeInfo.min & 0xFF) << 20)|
                ((gSWOSD_config.dateTimeInfo.sec & 0xFF) << 14)|
                (gSWOSD_config.dateTimeInfo.mesec & 0x3FF);

    unsigned char *ptr = (unsigned char *)yuv_data_addr + img_w * img_h * 3 / 2;

    *(int *)(ptr + 4 + 12 * 8 + 4) = time1; //ʱ���֡��Ϣ�ǵ�13���͵�14��
    *(int *)(ptr + 4 + 13 * 8 + 4) = time2;
  
   // printf("time1=0x%x, time2=0x%x\n", time1, time2);

    return 0;
}
#endif //#ifdef RTC_COTEX_A8 //modify by gomo 2012-12-19

//add by lhb 2013-5-3

//------------------------------------------------------------------------
/**
\brief ��av_server��������osd�ṹ�帳ֵ��M3ȫ�ֱ���
\param osd_cfg OSD��ؽṹ�� @sa Dh_Stream_OSD_Config

\return 0:�ɹ����� ����:ʧ�ܷ��� 
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
\brief ͨ��֡��Ϣ��ȡʱ��
\param yuv_data_addr YUV���ݵ�ַ

\return 0:�ɹ����� ����:ʧ�ܷ��� 
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
    
    int time1 = 0; ///<������
    int time2 = 0; ///<ʱ�������
    
    unsigned char *buffer 
                = (unsigned char *)yuv_data_addr + img_w * img_h * 3 / 2;

    time1 = 0x12345678;//*(int *)(buffer + 4 + 12 * 8 + 4); //ʱ���֡��Ϣ�ǵ�13���͵�14��
    time2 = 0x12345678;//*(int *)(buffer + 4 + 13 * 8 + 4);

    dh_time2str(time1, time2, g_h264_osd_cfg.timeformat, timestr);

    return 0;
}

//------------------------------------------------------------------------
/**
\brief ����ȫ�ֱ���g_h264_osd_cfg�е���Ϣʵ��OSD����
\param yuv_data_addr YUV���ݵ�ַ
\param img_w YUV���ݿ��
\param img_h YUV���ݸ߶�

\return 0:�ɹ����� ����:ʧ�ܷ��� 
*/
//------------------------------------------------------------------------

char *wday[] = {"������", "����һ", "���ڶ�", "������", "������", "������", "������"};

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
    int osd_flag = 0; //�Ƿ���ӱ�־����osd_flag=1ʱ����һ�е���
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


 //   if(img_w > RES_720P_WIDTH)//����ֱ��ʴ���720P���ֿ�������32�ţ�������16��
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
    //func_��ȡʱ����Ϣ(yuv_data_addr); //ͨ��YUV�����е�֡��Ϣ��ȡʱ����Ϣ
    //dh_get_time_from_frameinfo(yuv_data_addr, img_w, img_h, timestr);
	//g_h264_osd_cfg.addrtext = "�к��Ӽ�";
    //���е���osd��Ϣ������ʱ�䡢�ص㡢����ȡ�ÿѭ��һ�ε���һ�����ݲ�����
    //i<5�������ܹ���5��osd��Ϣ��ÿ����ϢҪ����
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
            sprintf((char *)osd_buffer, "%s", "ʱ��: ");
            snprintf((char *)(osd_buffer+strlen("ʱ��: ")), DH_TEXT_LEN * 2,
                "%s", timestr);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.addren && (i == 1))
        {
            sprintf((char *)osd_buffer, "%s", "λ��: ");
            snprintf((char *)(osd_buffer+strlen("λ��: ")), DH_TEXT_LEN * 2, 
                "%s", g_h264_osd_cfg.addrtext);
            osd_flag = 1;
        }
        else if(g_h264_osd_cfg.directionen && (i == 2))
        {
            sprintf((char *)osd_buffer, "%s", "���: ");
            snprintf((char *)(osd_buffer+strlen("���: ")), DH_TEXT_LEN * 2,
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
            ret = DH_Osd(osd_buffer,        //osd�������ݣ���g_h264_osd_cfg���
                (unsigned char *)yuv_data_addr, //YUV���ݵ�ַ
                hzk, asc,                       //�ֿ�����
                font_w, font_h,                 //�����С
                start_x, start_y,               //�ַ�������ʼλ��
                img_w, img_h,                   //YUV���ݿ��
                DH_OSD_YUV_420,                 //YUV���ݸ�ʽYUV420
                &pOsdParm);                     //�ַ�������ɫ
            
            start_y = start_y + font_h;         //ѭ�����ӻ���
		//void *memset(void *s, int c, size_t n);
		memset(osd_buffer, '\0', DH_TEXT_LEN * 2);
			
            osd_flag = 0;
        }
    }
    
    return ret;
}



