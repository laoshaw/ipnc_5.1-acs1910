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
#include "drv_lenPWM.h"
#include <fcntl.h>
#include <termios.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <osa.h>
#include <errno.h> 
#include <drv_csl.h>
#include <drv_gpio.h>

static int control_led_pwm = 0;
static pthread_t control_led_pwm_thread_id;
static int fd_pwm0, fd_pwm1, fd_pwm2, fd_pwm3;

#define ZOOM_PWM    fd_pwm0
#define FOCUS_PWM   fd_pwm1
#define IRIS_PWM    fd_pwm3

static void control_led_pwm_thread(void)
{
}

int PWM_init(void)
{
    int retVal;
    int dev_fp;
    unsigned long phyAddr = 0x01c22000;
    unsigned int value32;
    
    int ret;

    fd_pwm0 = open("/dev/davinci_pwm0", O_RDWR);
    if(fd_pwm0 < 0)
    {
        printf("Can't open /dev/davinci_pwm0\n");
        return OSA_EFAIL;
    }
#if 0
    retVal = ioctl(fd_pwm0, 9, 0x4ff );
    if(retVal < 0)
    {
        printf("errno=%d\n",errno);
        printf("Set period error\n");


        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm0, PWMIOC_SET_DURATION, 0xff);
    if(retVal < 0)
    {
        printf("Set duration error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm0, PWMIOC_SET_MODE, PWM_CONTINUOUS_MODE);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm0, PWMIOC_SET_FIRST_PHASE_STATE, 1);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm0, PWMIOC_START, 0);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
#endif
#if 0
    fd_pwm1 = open("/dev/davinci_pwm1", O_RDWR);
    if(fd_pwm1 < 0)
    {
        printf("Can't open /dev/davinci_pwm1\n");
        return OSA_EFAIL;
    }

    retVal = ioctl(fd_pwm1, PWMIOC_SET_PERIOD, 0x4ff);
    if(retVal < 0)
    {
        printf("Set period error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm1, PWMIOC_SET_DURATION, 0xff);
    if(retVal < 0)
    {
        printf("Set duration error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm1, PWMIOC_SET_MODE, PWM_CONTINUOUS_MODE);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm1, PWMIOC_SET_FIRST_PHASE_STATE, 1);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm1, PWMIOC_START, 0);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
#endif
#if 1

    fd_pwm2 = open("/dev/davinci_pwm2", O_RDWR);
    if(fd_pwm2 < 0)
    {
        printf("Can't open /dev/davinci_pwm2\n");
        return OSA_EFAIL;
    }
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 1, &value32);
    printf("pinmux1 = 0x%08X\n", value32);
    value32 |= 0x00020000;  
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 1, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 1, &value32);
    printf("pinmux1 = 0x%08X\n", value32);

    CSL_gpioGetPinmux(&gCSL_gpioHndl, 0, &value32);
    printf("pinmux0 = 0x%08X\n", value32);
    value32 &= 0xFFF3FFFF;  
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 0, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 0, &value32);
    printf("pinmux0 = 0x%08X\n", value32);

    CSL_gpioGetPinmux(&gCSL_gpioHndl, 4, &value32);
    printf("pinmux4 = 0x%08X\n", value32);
    value32 &= 0xFFCFFFFF;  
    CSL_gpioSetPinmux(&gCSL_gpioHndl, 4, value32);
    CSL_gpioGetPinmux(&gCSL_gpioHndl, 4, &value32);
    printf("pinmux4 = 0x%08X\n", value32);
    


    DRV_gpioSetMode(82, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(82);
    DRV_gpioSetMode(81, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(81);
    DRV_gpioSetMode(44, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(44);
     DRV_gpioSetMode(45,DRV_GPIO_DIR_OUT);
    DRV_gpioClr(45);
      DRV_gpioSetMode(37, DRV_GPIO_DIR_OUT);
    DRV_gpioClr(37);
     DRV_gpioSetMode(79,DRV_GPIO_DIR_OUT);
    DRV_gpioClr(79);
     DRV_gpioSetMode(80,DRV_GPIO_DIR_OUT);
    DRV_gpioClr(80);
 
    retVal = ioctl(fd_pwm2, PWMIOC_SET_PERIOD, 0x4ff);
    if(retVal < 0)
    {
        printf("Set period error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm2, PWMIOC_SET_DURATION, 0xff);
    if(retVal < 0)
    {
        printf("Set duration error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm2, PWMIOC_SET_MODE, PWM_CONTINUOUS_MODE);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm2, PWMIOC_SET_FIRST_PHASE_STATE, 1);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm2, PWMIOC_START, 0);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }

while(1)
{
    //DRV_gpioSet(81);
    DRV_gpioClr(82);

    DRV_gpioClr(44);
    DRV_gpioClr(45);
    DRV_gpioClr(37);
    DRV_gpioClr(79);
    DRV_gpioClr(80);
    sleep(1);
    //DRV_gpioClr(81);
    DRV_gpioSet(44);
    DRV_gpioSet(45);
    DRV_gpioSet(37);
    DRV_gpioSet(79);
    DRV_gpioSet(80);
    DRV_gpioSet(82);
    sleep(1);
}
#endif
#if 0
    fd_pwm3 = open("/dev/davinci_pwm3", O_RDWR);
    if(fd_pwm3 < 0)
    {
        printf("Can't open /dev/davinci_pwm3\n");
        return OSA_EFAIL;
    }
    retVal = ioctl(fd_pwm3, PWMIOC_SET_PERIOD, 0x4ff);
    if(retVal < 0)
    {
        printf("Set period error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm3, PWMIOC_SET_DURATION, 0xff);
    if(retVal < 0)
    {
        printf("Set duration error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm3, PWMIOC_SET_MODE, PWM_CONTINUOUS_MODE);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm3, PWMIOC_SET_FIRST_PHASE_STATE, 1);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
    retVal = ioctl(fd_pwm3, PWMIOC_START, 0);
    if(retVal < 0)
    {
        printf("Set mode error\n");
        control_led_pwm = 0;
        
    }
#endif
    
#if 0
    while(1)
    {
        sleep(2);
    }
#endif
#if 0
    {//test adc
        int fd,ret,i,times=10;
        int adc_data[6];

        memset(adc_data,0,sizeof(adc_data));
        fd = open("/dev/adc_device", O_RDWR);        
        if (fd < 0) {
            printf("Can't open /dev/adc_device\n");
            return -1;
        }
        while(1)
        {
            ret= read(fd,adc_data,sizeof(adc_data)); 

     // printf("adc value is:");
            for(i=0;i<6;i++)
            {
                printf("CH%d:%4d ",i,adc_data[i]);
            }
            printf("\n");
            sleep(1);
        }
 
        close(fd);
    }//end test adc
#endif

#if 0
    if ((dev_fp=open("/dev/mem",O_RDWR|O_SYNC))==-1)
	{
		VI_DEBUG("dev_fp Fail!! \n");
		return ;
	}
	pwm0_base_addr = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE,MAP_SHARED, dev_fp, phyAddr);
    pwm0_pid_addr = (unsigned int *)pwm0_base_addr;
    printf("pwm0_base_addr : 0x%08X = 0x%08x\n", pwm0_pid_addr, *pwm0_pid_addr );

    //retVal = DM365MM_init(); 
    //if(retVal != 0)
    //{
    //    VI_DEBUG("DM365MM_init() error!\n");
    //    return retVal;
    //}
    //pwm0_base_addr = (unsigned int *)DM365MM_mmap(PWM0_BASE_ADDR, PWM_REG_SIZE*4);
    pwm0_pid_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_PID_OFFSET);
    pwm0_pcr_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_PCR_OFFSET);
    pwm0_cfg_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_CFG_OFFSET);
    pwm0_start_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_START_OFFSET);
    pwm0_rpt_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_RPT_OFFSET);
    pwm0_per_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_PER_OFFSET);
    pwm0_ph1d_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_PH1D_OFFSET);

    pwm1_base_addr = (unsigned int *)((char *)pwm0_base_addr + PWM_REG_SIZE);
    pwm1_pid_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_PID_OFFSET);
    pwm1_pcr_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_PCR_OFFSET);
    pwm1_cfg_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_CFG_OFFSET);
    pwm1_start_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_START_OFFSET);
    pwm1_rpt_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_RPT_OFFSET);
    pwm1_per_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_PER_OFFSET);
    pwm1_ph1d_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_PH1D_OFFSET);

   
    pwm2_base_addr = (unsigned int *)((char *)pwm1_base_addr + PWM_REG_SIZE);
    pwm2_pid_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_PID_OFFSET);
    pwm2_pcr_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_PCR_OFFSET);
    pwm2_cfg_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_CFG_OFFSET);
    pwm2_start_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_START_OFFSET);
    pwm2_rpt_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_RPT_OFFSET);
    pwm2_per_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_PER_OFFSET);
    pwm2_ph1d_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_PH1D_OFFSET);


    pwm3_base_addr = (unsigned int *)((char *)pwm2_base_addr + PWM_REG_SIZE);
    pwm3_pid_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_PID_OFFSET);
    pwm3_pcr_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_PCR_OFFSET);
    pwm3_cfg_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_CFG_OFFSET);
    pwm3_start_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_START_OFFSET);
    pwm3_rpt_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_RPT_OFFSET);
    pwm3_per_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_PER_OFFSET);
    pwm3_ph1d_addr = (unsigned int *)((char *)pwm3_base_addr + PWM_PH1D_OFFSET);

    *pwm0_per_addr = 0x4ff;
    *pwm0_ph1d_addr = 0x0ff;
    *pwm0_cfg_addr = (PWM_CFG_INTEN_DISABLE << PWM_CFG_INTEN_OFFSET)
                       + (PWM_CFG_INACTOUT_HIGH << PWM_CFG_INACTOUT_OFFSET)
                       + (PWM_CFG_P1OUT_HIGH << PWM_CFG_P1OUT_OFFSET)
                       + (PWM_CFG_MODE_CONTINUOUS << PWM_CFG_MODE_OFFSET);
    *pwm1_per_addr = 0x4ff;
    *pwm1_ph1d_addr = 0x0ff;
   // *pwm1_cfg_addr = (PWM_CFG_INTEN_DISABLE << PWM_CFG_INTEN_OFFSET)
   //                    + (PWM_CFG_INACTOUT_HIGH << PWM_CFG_INACTOUT_OFFSET)
   //                    + (PWM_CFG_P1OUT_HIGH << PWM_CFG_P1OUT_OFFSET)
   //                    + (PWM_CFG_MODE_CONTINUOUS << PWM_CFG_MODE_OFFSET);
    *pwm2_per_addr = 0x4ff;
    *pwm2_ph1d_addr = 0x0ff;
   // *pwm2_cfg_addr = (PWM_CFG_INTEN_DISABLE << PWM_CFG_INTEN_OFFSET)
   //                    + (PWM_CFG_INACTOUT_HIGH << PWM_CFG_INACTOUT_OFFSET)
   //                    + (PWM_CFG_P1OUT_HIGH << PWM_CFG_P1OUT_OFFSET)
   //                    + (PWM_CFG_MODE_CONTINUOUS << PWM_CFG_MODE_OFFSET);
    *pwm3_per_addr = 0x4ff;
    *pwm3_ph1d_addr = 0x0ff;
   // *pwm3_cfg_addr = (PWM_CFG_INTEN_DISABLE << PWM_CFG_INTEN_OFFSET)
   //                    + (PWM_CFG_INACTOUT_HIGH << PWM_CFG_INACTOUT_OFFSET)
   //                    + (PWM_CFG_P1OUT_HIGH << PWM_CFG_P1OUT_OFFSET)
   //                    + (PWM_CFG_MODE_CONTINUOUS << PWM_CFG_MODE_OFFSET);
    VI_DEBUG("pwm0_per_reg 0x%08X = 0x%08X\n", pwm0_per_addr, *pwm0_per_addr);
    VI_DEBUG("pwm0_ph1d_reg 0x%08X = 0x%08X\n",pwm0_ph1d_addr,  *pwm0_ph1d_addr);
    VI_DEBUG("pwm0_cfg_reg 0x%08X = 0x%08X\n", pwm0_cfg_addr, *pwm0_cfg_addr);
    VI_DEBUG("pwm1_per_reg 0x%08X = 0x%08X\n", pwm1_per_addr, *pwm1_per_addr);
    VI_DEBUG("pwm1_ph1d_reg 0x%08X = 0x%08X\n", pwm1_ph1d_addr, *pwm1_ph1d_addr);
    VI_DEBUG("pwm1_cfg_reg 0x%08X = 0x%08X\n", pwm1_cfg_addr, *pwm1_cfg_addr);
    VI_DEBUG("pwm2_per_reg 0x%08X = 0x%08X\n", pwm2_per_addr, *pwm2_per_addr);
    VI_DEBUG("pwm2_ph1d_reg 0x%08X = 0x%08X\n", pwm2_ph1d_addr, *pwm2_ph1d_addr);
    VI_DEBUG("pwm2_cfg_reg 0x%08X = 0x%08X\n", pwm2_cfg_addr, *pwm2_cfg_addr);
    VI_DEBUG("pwm3_per_reg 0x%08X = 0x%08X\n", pwm3_per_addr, *pwm3_per_addr);
    VI_DEBUG("pwm3_ph1d_reg 0x%08X = 0x%08X\n", pwm3_ph1d_addr, *pwm3_ph1d_addr);
    VI_DEBUG("pwm3_cfg_reg 0x%08X = 0x%08X\n", pwm3_cfg_addr, *pwm3_cfg_addr);
     //VI_DEBUG("pwm3_base_addr = 0x%08X\n", pwm3_base_addr);

    //VI_DEBUG("pwm0_pid = 0x%08X\n", *pwm0_pid_addr);
    //VI_DEBUG("pwm1_pid = 0x%08X\n", *pwm1_pid_addr);
    //VI_DEBUG("pwm2_pid = 0x%08X\n", *pwm2_pid_addr);
    //VI_DEBUG("pwm3_pid = 0x%08X\n", *pwm3_pid_addr);

   // control_led_pwm = 1;
   // if(pthread_create(&control_led_pwm_thread_id, NULL, control_led_pwm_thread, NULL))
   // {
   //     printf("create telemetry_thread err\n");
   //     return -1;
   // } 
#endif
    return 0;
}

int ledPWM_exit(void)
{
    //close(fd_pwm0);
    control_led_pwm = 0;
    pthread_join(control_led_pwm_thread_id, NULL);
    return 0;
}
