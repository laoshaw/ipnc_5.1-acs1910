/***********************************************************
@file     drv_ledPWM.c
@brief    
@version  V1.0
@date     2016-12-01
@author   pamsimochen
Copyright (c) 2016-2016 VIFOCUS
***********************************************************/

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <drv.h>
#include "drv_lenPWM.h"
#include <fcntl.h>
#include <termios.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <osa.h>
#include <errno.h> 
#include <drv_csl.h>
#include <drv_gpio.h>
#include "cmd_server.h"

static int fd_pwm0, fd_pwm1, fd_pwm2, fd_pwm3;

static int len_cmd_msqid = 0;
static int len_control_thread_run = 0;
static pthread_t len_control_thread_id;
static tCmdServerMsg len_control_rcv_msg;
static tCmdServerMsg len_control_snd_msg;

static void lenPWM_control_thread(void)
{
    VF_LEN_CONTROL_S len_control_data;
    
    VI_DEBUG("Hello len control thread!\n");
    while(len_control_thread_run)
    {
        VI_DEBUG("wait msg from cmd_server\n");
        msgrcv(len_cmd_msqid, &len_control_rcv_msg, MSG_BUF_SIZE, 0, 0);
        printf("\n");
        VI_DEBUG("receive msg type: 0x%lx\n\n", len_control_rcv_msg.msg_type);
        memcpy(&len_control_data, len_control_rcv_msg.msg_data, sizeof(VF_LEN_CONTROL_S));
        VI_DEBUG("len_control_data.Mode = %x, .speed = %x\n", len_control_data.Mode, len_control_data.speed);
        if(len_control_rcv_msg.msg_type == IP_CMD_LEN_CONTROL)
        {
            switch(len_control_data.Mode)
            {
                case VF_CONTROL_ZOOM_WIDE:
                    break;
                case VF_CONTROL_ZOOM_TELE:
                    break;
                case VF_CONTROL_FOCUS_FAR:
                    break;
                case VF_CONTROL_FOCUS_NEAR:
                    break;
                case VF_CONTROL_IRIS_LARGE:
                    break;
                case VF_CONTROL_IRIS_SMALL:
                    break;
                case VF_CONTROL_LEN_STOP:
                    break;
                default:
                    break;
            }
        }

    }
    VI_DEBUG("GoodBye len control thread!\n");

      
}
/***********************************************************
\brief 设置PWM的周期 
\param fd:pwm设备文件, per:周期 
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_set_period(int fd, int per)
{
    int retVal = 0;
    retVal = ioctl(fd, PWMIOC_SET_PERIOD, per);
    if(retVal < 0)
    {
        perror("Set period0 error\n");
    }
    return retVal;
}

/***********************************************************
\brief 设置PWM的占空比（p1out为高的时候）
\param fd:pwm设备文件, p1d: 占空比（p1out为高时） 
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_set_p1d(int fd, int p1d)
{
    int retVal = 0;
    retVal = ioctl(fd, PWMIOC_SET_DURATION, p1d);
    if(retVal < 0)
    {
        perror("Set duration error\n");
    }
    return retVal; 
}

/***********************************************************
\brief 设置PWM的模式
\param fd:pwm设备文件, mode:连续模式 or one-shot模式
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_set_mode(int fd, int mode)
{
    int retVal = 0;
    retVal = ioctl(fd, PWMIOC_SET_MODE, mode);
    if(retVal < 0)
    {
        perror("Set mode error\n");
    }
    return retVal;
}

/***********************************************************
\brief 设置PWM的模式
\param fd:pwm设备文件, p1out:第一个输出的状态，高电平 or 低电平
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_set_p1out(int fd, int p1out)
{
    int retVal = 0;
    retVal = ioctl(fd, PWMIOC_SET_FIRST_PHASE_STATE, 1);
    if(retVal < 0)
    {
        perror("Set p1out error\n");
    }
    return retVal;
}
/***********************************************************
\brief 启动pwm 
\param fd:pwm设备文件
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_start(int fd)
{
    int retVal = 0;
    retVal = ioctl(fd_pwm0, PWMIOC_START, 0);
    if(retVal < 0)
    {
        perror("Start pwm error\n");
    }
    return retVal;
}

/***********************************************************
\brief 初始化PWM参数 
\param fd:pwm设备文件, per:周期, p1d:占空比, mode:模式, 
       p1out:第一个输出高低电平
 
\return 0:成功 其它:失败
***********************************************************/
static int PWM_init(int fd, int per, int p1d, int mode, int p1out)
{
    int retVal; 
    retVal = PWM_set_period(fd, per);
    if(retVal < 0)
        return retVal;
    retVal = PWM_set_p1d(fd, p1d);
    if(retVal < 0)
        return retVal;
    retVal = PWM_set_mode(fd, mode);
    if(retVal < 0)
        return retVal;
    retVal = PWM_set_p1out(fd, p1out);
    if(retVal < 0)
        return retVal;
    return retVal;
  
}

/***********************************************************
\brief 初始化消息队列，用于接收cmd_server传过来的镜头控制命令 
\param fd:pwm设备文件
 
\return 0:成功 其它:失败
***********************************************************/
static int lenPWM_msg_init()
{
    int retVal = 0;
    typedef struct{
        unsigned int a;
        unsigned char b;
        unsigned char c;
        unsigned char d;
        unsigned char e:1;

    }test;

   VI_DEBUG("%d\n", sizeof(test)); 
    VI_DEBUG("Get len_cmd_msqid!\n");

    len_cmd_msqid = msgget((key_t)LEN_CMD_MSG_KEY, IPC_CREAT|0666);

    if(len_cmd_msqid == 0)
    {
        msgctl(len_cmd_msqid, IPC_RMID, 0);
        len_cmd_msqid = msgget((key_t)LEN_CMD_MSG_KEY, IPC_CREAT|0666);
    }
    if(len_cmd_msqid < 0)
    {
        perror("Get len_cmd_msqid error!\n");
        retVal = len_cmd_msqid;
    }
    VI_DEBUG("Get len_cmd_msqid %d done!\n\n", len_cmd_msqid);

    return retVal;
}

/***********************************************************
\brief 创建镜头控制进程，用于处理cmd_server发过来的命令
       并执行相应的操作
\param fd:pwm设备文件
 
\return 0:成功 其它:失败
***********************************************************/
int lenPWM_control_thread_init()
{
    int retVal = 0;
    VI_DEBUG("Initialize lenPWM control thread\n");
   
    if((retVal = pthread_create(&len_control_thread_id, NULL, lenPWM_control_thread, NULL)) != 0)
    {
        perror("Create lenPWM control thread\n");
    }
    else 
    {
        len_control_thread_run = 1;
        VI_DEBUG("Create lenPWM control thread done!\n\n");
    }
    
    return retVal;
}

int lenPWM_init(void)
{
    int retVal;

    //initial pwm 
    VI_DEBUG("Start intialize PWM\n");
    fd_pwm0 = open("/dev/davinci_pwm0", O_RDWR);
    if(fd_pwm0 < 0)
    {
        printf("Can't open /dev/davinci_pwm0\n");
        return OSA_EFAIL;
    }
    fd_pwm1 = open("/dev/davinci_pwm1", O_RDWR);
    if(fd_pwm1 < 0)
    {
        close(fd_pwm0);
        printf("Can't open /dev/davinci_pwm1\n");
        return OSA_EFAIL;
    }

    fd_pwm2 = open("/dev/davinci_pwm2", O_RDWR);
    if(fd_pwm2 < 0)
    {
        close(fd_pwm1);
        printf("Can't open /dev/davinci_pwm2\n");
        return OSA_EFAIL;
    }

    fd_pwm3 = open("/dev/davinci_pwm3", O_RDWR);
    if(fd_pwm3 < 0)
    {
        close(fd_pwm2);
        printf("Can't open /dev/davinci_pwm3\n");
        return OSA_EFAIL;
    }

    retVal = PWM_init(FOCUS_PWM, FOCUS_PWM_INIT_PER, FOCUS_PWM_INIT_P1D, PWM_CONTINUOUS_MODE, FOCUS_PWM_INIT_P1OUT);
    if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_init(ZOOM_PWM, ZOOM_PWM_INIT_PER, ZOOM_PWM_INIT_P1D, PWM_CONTINUOUS_MODE, ZOOM_PWM_INIT_P1OUT);
    if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_init(IRIS_PWM, IRIS_PWM_INIT_PER, IRIS_PWM_INIT_P1D, PWM_CONTINUOUS_MODE, IRIS_PWM_INIT_P1OUT);
    if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_init(IRCUT_PWM, IRCUT_PWM_INIT_PER, IRCUT_PWM_INIT_P1D, PWM_CONTINUOUS_MODE, IRCUT_PWM_INIT_P1OUT);
    if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_start(FOCUS_PWM); 
    if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_start(ZOOM_PWM);
     if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_start(IRIS_PWM);
      if(retVal < 0)
        goto pwm_init_error;

    retVal = PWM_start(IRCUT_PWM);
    if(retVal < 0)
        goto pwm_init_error;

    retVal = lenPWM_msg_init();
    if(retVal < 0)
        goto pwm_init_error;
    retVal = lenPWM_control_thread_init();
    if(retVal != 0)
        goto pwm_control_thread_init_error;

    VI_DEBUG("Intialize PWM done!\n\n");
    return 0;

pwm_control_thread_init_error:
    msgctl(len_cmd_msqid, IPC_RMID, 0);

pwm_init_error:
    close(fd_pwm0);
    close(fd_pwm1);
    close(fd_pwm2);
    close(fd_pwm3);
    VI_DEBUG("Intialize PWM error!\n\n");
    return retVal;
}

int ledPWM_exit(void)
{
    close(fd_pwm0);
    close(fd_pwm1);
    close(fd_pwm2);
    close(fd_pwm3);
    
    msgctl(len_cmd_msqid, IPC_RMID, 0);
    len_control_thread_run = 0;
    pthread_join(len_control_thread_id, NULL);

    return 0;
}
