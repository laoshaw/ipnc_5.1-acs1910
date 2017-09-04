/***********************************************************
@file     drv_rtc.c
@brief    实现PCF8563的读写，实现实时时钟的功能 

@version  V1.0
@date     2017-08-22
@author   pamsimochen
Copyright (c) 2017-2019 VIFOCUS 
***********************************************************/

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <drv_csl.h>
#include <drv_gpio.h>
#include "drv.h"
#include "drv_ACS1910.h"
#include "cmd_server.h"
#include "VIM_API_Release.h"
#include "drv_rtc.h"
#include "drv_i2c.h"


int RTC_GetSeconds(DRV_I2cHndl *hndl, unsigned char *sec)
{
    unsigned char regAddr = RTC_VL_SECONDS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get senconds i2c read error\n");
        return ret; 
    }
    *sec = (((regVal >> 4) & 0x7) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal = 0x%x, sec = %d\n", regVal, *sec);

    return ret;
}

int RTC_GetMinutes(DRV_I2cHndl *hndl, unsigned char *min)
{
    unsigned char regAddr = RTC_MINUTES_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get minutes i2c read error\n");
        return ret; 
    }
    *min = (((regVal >> 4) & 0x7) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal = 0x%x, min = %d\n", regVal, *min);

    return ret;
}

int RTC_GetHours(DRV_I2cHndl *hndl, unsigned char *h)
{
    unsigned char regAddr = RTC_HOURS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get hours i2c read error\n");
        return ret; 
    }
    *h = (((regVal >> 4) & 0x3) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal = 0x%x, hours = %d\n", regVal, *h);

    return ret;
}

int RTC_GetDays(DRV_I2cHndl *hndl, unsigned char *d)
{
    unsigned char regAddr = RTC_DAYS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get days i2c read error\n");
        return ret; 
    }
    *d = (((regVal >> 4) & 0x3) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal = 0x%x, days = %d\n", regVal, *d);

    return ret;
}

int RTC_GetWeekdays(DRV_I2cHndl *hndl, unsigned char *week)
{
    unsigned char regAddr = RTC_WEEKDAYS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get week i2c read error\n");
        return ret; 
    }
    *week = (regVal & 0x3);
    VI_DEBUG("regVal = 0x%x, week = %d\n", regVal, *week);

    return ret;
}

int RTC_GetMonths(DRV_I2cHndl *hndl, unsigned char *mon)
{
    unsigned char regAddr = RTC_CENT_MONTHS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get mon i2c read error\n");
        return ret; 
    }
    *mon = (((regVal >> 4) & 0x1) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal >> 4 = %x, regVal >> 4 & 0x1 = %x, regVal = 0x%x, mon = %d\n", regVal >> 4, ((regVal >> 4) & 0x1 )*10, regVal, *mon);

    return ret;
}

int RTC_GetYears(DRV_I2cHndl *hndl, unsigned char *y)
{
    unsigned char regAddr = RTC_YEARS_REG;
    unsigned char regVal;
    int ret = 0; 
    
    ret = DRV_i2cRead8(hndl, &regAddr, &regVal, 1);
    VI_DEBUG("ret = %d\n", ret); 
    if(ret < 0)
    {
        printf("Get years i2c read error\n");
        return ret; 
    }
    *y = (((regVal >> 4) & 0xf) * 10) + (regVal & 0xf);
    VI_DEBUG("regVal = 0x%x, y = %d\n", regVal, *y);

    return ret;
}

int RTC_SetSeconds(DRV_I2cHndl *hndl, unsigned char sec)
{
    unsigned char regAddr = RTC_VL_SECONDS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = ((((sec / 10) << 4) & 0x70) + (sec % 10)) & 0x7f;
    VI_DEBUG("sec = %d, regVal = %x\n", sec, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set seconds i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetMinutes(DRV_I2cHndl *hndl, unsigned char min)
{
    unsigned char regAddr = RTC_MINUTES_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = (((((min / 10) << 4)) & 0x70) + (min % 10)) & 0x7f;
    VI_DEBUG("min = %d, regVal = %x\n", min, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set minutes i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetHours(DRV_I2cHndl *hndl, unsigned char h)
{
    unsigned char regAddr = RTC_HOURS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = (((((h / 10) << 4)) & 0x30) + (h % 10)) & 0x3f;
    VI_DEBUG("h = %d, regVal = %x\n", h, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set hours i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetDays(DRV_I2cHndl *hndl, unsigned char d)
{
    unsigned char regAddr = RTC_DAYS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = ((((d / 10) << 4) & 0x30) + (d % 10)) & 0x3f;
    VI_DEBUG("d = %d, regVal = %x\n", d, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set day i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetWeekDays(DRV_I2cHndl *hndl, unsigned char week)
{
    unsigned char regAddr = RTC_WEEKDAYS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = week & 0x7;
    VI_DEBUG("week = %d, regVal = %x\n", week, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set week i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetMonths(DRV_I2cHndl *hndl, unsigned char m)
{
    unsigned char regAddr = RTC_CENT_MONTHS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = ((((m / 10) << 4) & 0x10) + (m % 10)) & 0x1f;
    VI_DEBUG("m = %d, regVal = %x\n", m, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set months i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_SetYears(DRV_I2cHndl *hndl, unsigned char y)
{
    unsigned char regAddr = RTC_YEARS_REG;
    unsigned char regVal;
    int ret = 0;

    regVal = ((((y / 10) << 4) & 0xf0) + (y % 10)) & 0xff;
    VI_DEBUG("y = %d, regVal = %x\n", y, regVal);
    ret = DRV_i2cWrite8(hndl, &regAddr, &regVal, 1);
    if(ret < 0)
    {
        printf("Set years i2c read error\n");
        return ret; 
    } 
    return ret;
}

int RTC_GetDate(pVF_TIME_S ptime)
{
    int ret = 0;
    DRV_I2cHndl hndl;

    sem_wait(&vim_sem);
    DRV_i2cOpen(&hndl, RTC_READ_ADDR);
    
    RTC_GetSeconds(&hndl, &ptime->second);
    RTC_GetMinutes(&hndl, &ptime->minute);
    RTC_GetHours(&hndl, &ptime->hour);
    RTC_GetDays(&hndl, &ptime->day);
    RTC_GetMonths(&hndl, &ptime->month);
    RTC_GetYears(&hndl, &ptime->year);
    
    VI_DEBUG("date is: %04d-%02d-%02d %02d:%02d:%02d\n", ptime->year + 2000, ptime->month, ptime->day, ptime->hour, ptime->minute, ptime->second);

    DRV_i2cClose(&hndl);
    sem_post(&vim_sem);

    return ret;
}

int RTC_SetDate(pVF_TIME_S ptime)
{
    int ret = 0;
    DRV_I2cHndl hndl;
    unsigned char week;

    sem_wait(&vim_sem);
    DRV_i2cOpen(&hndl, RTC_WRITE_ADDR);

    RTC_SetSeconds(&hndl, ptime->second);
    RTC_SetMinutes(&hndl, ptime->minute);
    RTC_SetHours(&hndl, ptime->hour);
    RTC_SetDays(&hndl, ptime->day);
    week = (ptime->day + 1 + 2*ptime->month + 3*(ptime->month + 1)/5 + ptime->year + ptime->year/4 - ptime->year/100 + ptime->year/400) % 7;
    RTC_SetWeekDays(&hndl, week);
    RTC_SetMonths(&hndl, ptime->month);
    RTC_SetYears(&hndl, ptime->year);

    DRV_i2cClose(&hndl);
    sem_post(&vim_sem);
}
