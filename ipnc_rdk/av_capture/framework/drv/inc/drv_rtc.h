#ifndef __DRV_RTC_H
#define __DRV_RTC_H 


#define RTC_READ_ADDR 0xA3 
#define RTC_WRITE_ADDR 0xA2

#define RTC_CONTROL_STATUS1_REG 0x00
#define RTC_CONTROL_STATUS2_REG 0x01 
#define RTC_VL_SECONDS_REG 0x02 
#define RTC_MINUTES_REG 0x03
#define RTC_HOURS_REG 0x04
#define RTC_DAYS_REG 0x05
#define RTC_WEEKDAYS_REG 0x06
#define RTC_CENT_MONTHS_REG 0x07
#define RTC_YEARS_REG 0x08


int RTC_GetDate(pVF_TIME_S ptime);
int RTC_SetDate(pVF_TIME_S ptime);
#endif
