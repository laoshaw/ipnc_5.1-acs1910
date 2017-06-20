/* ===========================================================================
* @file proc_alarm.c
*
* @path $(IPNCPATH)\sys_server\src\
*
* @desc
* .
* Copyright (c) Appro Photoelectron Inc.  2008
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
*
* =========================================================================== */
/**
* @file proc_alarm.c
* @brief Functions to handle alarm situations.
*/
#include "proc_alarm.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <Appro_interface.h>
#include <file_msg_drv.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <alarm_schedule.h>
#include <ipnc_gio_util.h>
#include <file_dispatcher.h>
#include <avi_mng.h>
#include <jpg_mng.h>
#include <sys/time.h>
#include <alarm_msg_drv.h>
#include <gio_alarmout.h>
#include <semaphore.h>
int SYSTEM(char *arg);

#define STATUS_ALARM		(1<<0)
#define STATUS_SCHEDULE		(1<<1)
#define STATUS_SD			(1<<2)
#define STATUS_FTP			(1<<3)
#define STATUS_SMTP			(1<<4)

#ifdef DEBUG
#define DBG(fmt, args...) fprintf(stdout, "%s: debug: " fmt, __FILE__, ##args)
#else
#define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stdout, "%s: error: " fmt, __FILE__, ##args)
#define MOTION_DETECTED (1)
#define KEY (1729)
extern int id_motion_sem;
//static time_t tLastScheduleTime = 0;
static int gIsMotion = 0;
static int gIsExt = 0;
static int gIsEthernet = 0;
static int gSchAviRun = 0, gSchJpgRun = 0;
extern SysInfo *pAlarmSysInfo;
extern char gbSD_WriteProtect;
extern int GetfreememInfo( void );
extern volatile SemHandl_t hGIOSem;
extern volatile SemHandl_t audioSem;
int alarmDurationTime = 0;
int audio_count =0;
int gInsideAudPlayTh = 0;
/**
* @brief Get alarm duration
* @return alarm duration
*/
int Get_AlarmDuration(void)
{
	int durationTime = -1;
	if(hGIOSem){
		SemWait(hGIOSem);
		durationTime = alarmDurationTime;
		SemRelease(hGIOSem);
	}
	return durationTime;
}
/**
* @brief Set alarm duration
* @return 0 on success.
*/
int Set_AlarmDuration(int durationTime)
{
	int alarm_count = 0;
	if(durationTime == 0)
	{
		alarm_count = 10;
	}else if(durationTime == 1){
		alarm_count = 30;
	}else if(durationTime == 2){
		alarm_count = 60;
	}else if(durationTime == 3){
		alarm_count = 300;
	}else if(durationTime == 4){
		alarm_count = 600;
	}else{
		alarm_count = -1;
	}
	SemWait(hGIOSem);
	alarmDurationTime = alarm_count;
	DBG("Set_AlarmDuration = %d\n",alarmDurationTime);
	SemRelease(hGIOSem);
	return 0;
}

int Sub_AlarmDuration(void)
{
	if(hGIOSem){
		SemWait(hGIOSem);
		alarmDurationTime --;
		DBG("Sub_AlarmDuration = %d\n",alarmDurationTime);
		SemRelease(hGIOSem);
	}
	return 0;
}
/**
* @brief Set alarm audio count
* @param alarmDuration [i] alarm duration
* @return 0 on success.
*/
int setAudioCount(int alarmDuration)
{
	int alarm_count;
	if(alarmDuration == 0){
		alarm_count = 2;
	}else if(alarmDuration == 1){
		alarm_count = 6;
	}else if(alarmDuration == 2){
		alarm_count = 12;
	}else if(alarmDuration == 3){
		alarm_count = 60;
	}else if(alarmDuration == 4){
		alarm_count = 120;
	}else if(alarmDuration == -1){
		alarm_count = 0;
	}else{
		alarm_count = 1999;
	}
	audio_count = alarm_count;
	DBG("setAudioCount=%d\n",audio_count);
	return 0;
}
/**
* @brief Get alarm audio count
* @return alarm audio count
*/
int getAudioCount(void)
{
	int count=0;
	count = audio_count;
	DBG("getAudioCount=%d\n",audio_count);
	return count;
}

int subAudioCount(void)
{
	audio_count --;
	DBG("subAudioCount=%d\n",audio_count);
	return 0;
}

/**
* @brief Notify AV server that the audio alarm is starting
* @return alarm audio count
*/

void NotifyAVServerStart(int audMsgQid)
{
    AudMsgBuf_t audMsgBuf;

    audMsgBuf.mtype = 1;
    audMsgBuf.msg   = AUD_ALARM_START;

    msgsnd(audMsgQid, &audMsgBuf, sizeof(audMsgBuf) - sizeof(long), IPC_NOWAIT);
    msgrcv(audMsgQid, &audMsgBuf, sizeof(audMsgBuf) - sizeof(long), 2, 0);
}

/**
* @brief Notify AV server that the audio alarm is stopped
* @return alarm audio count
*/

void NotifyAVServerEnd(int audMsgQid)
{
    AudMsgBuf_t audMsgBuf;

    audMsgBuf.mtype = 1;
    audMsgBuf.msg   = AUD_ALARM_END;

    msgsnd(audMsgQid, &audMsgBuf, sizeof(audMsgBuf) - sizeof(long), IPC_NOWAIT);
    msgrcv(audMsgQid, &audMsgBuf, sizeof(audMsgBuf) - sizeof(long), 2, 0);
}

/**
* @brief Save an AVI file.
* @return 0 on success. Otherwise meets fail.
*/
int AlarmAudio_Play(int audMsgQid)
{
	char cmd[280]="";
	int ret=-1,nFree;
	int count=0;
	SysInfo *pSysInfo = pAlarmSysInfo;
	if(pSysInfo == NULL)
		return -1;

    if(pSysInfo->lan_config.nAlarmAudioPlay == 0)
        return 0;

	nFree = GetfreememInfo();
	if(nFree < 0)
	{
	    sleep(1);
		return -1;
	}
	else
	{
		DBG("Alarm Audio play check free Memory = %ld Kbyte \n", (long)nFree);
	}

    SemWait(audioSem);
    gInsideAudPlayTh = 1;
	SemRelease(audioSem);

    SemWait(audioSem);
	count = getAudioCount();
	SemRelease(audioSem);

	WaitAlarmStatusUpdated();

	while((count > 0) && (IsAlarmRestart() == 0)){

		if(pSysInfo->lan_config.alarmstatus == 0){

			SemWait(audioSem);
			setAudioCount(-1);
			count = 0;
			SemRelease(audioSem);

			break;
		}

		if(pSysInfo->lan_config.nAlarmAudioPlay) {
			if(pSysInfo->audio_config.samplerate == 1) {
				if(pSysInfo->lan_config.nAlarmAudioFile == 0)
					sprintf(cmd, "aplay -f U8 alarm_1_16K.wav -r 16000");
				else if(pSysInfo->lan_config.nAlarmAudioFile == 1)
					sprintf(cmd, "aplay -f U8 alarm_2_16K.wav -r 16000");
				else
					break;
			}
			else {
				if(pSysInfo->lan_config.nAlarmAudioFile == 0)
					sprintf(cmd, "aplay -f U8 alarm_1_8K.wav -r 8000");
				else if(pSysInfo->lan_config.nAlarmAudioFile == 1)
					sprintf(cmd, "aplay -f U8 alarm_2_8K.wav -r 8000");
				else
					break;
			}

			NotifyAVServerStart(audMsgQid);
			ret = SYSTEM(cmd);
			NotifyAVServerEnd(audMsgQid);
		}


		SemWait(audioSem);
		subAudioCount();
		count = getAudioCount();
		SemRelease(audioSem);
	}

    SemWait(audioSem);
    gInsideAudPlayTh = 0;
	SemRelease(audioSem);

	return ret;
}

/**
* @brief Get alarm flag.
* @param hSem [i] Semaphore.
* @param bClean[i] Cleam flag.
* @return Alarm flag.
* @retval -1 Failed to get alarm flag.
*/
int GetAlarmFlag(SemHandl_t hSem, int bClean)
{
	int ret;
	if(SemWait(hSem))
		return -1;
	ret = gIsEthernet | gIsExt | gIsMotion;
	if(bClean)
		gIsEthernet = gIsExt = gIsMotion = 0;
	if(SemRelease(hSem))
		return -1;
	return ret;
}
/**
* @brief Set ethernet alarm.
* @param enable Ethernet alarm status.
* @param hSem [i] Semaphore.
* @retval 0 Set ethernet alarm sucess.
* @retval -1 Failed to set alarm status.
*/
int SetEthAlarm(int enable, SemHandl_t hSem)
{
	if(SemWait(hSem))
		return -1;
	gIsEthernet= enable;
	if(SemRelease(hSem))
		return -1;
	return 0;
}
/**
* @brief Get Ethernet alarm .
* @param hSem [i] Semaphore.
* @param bClean[i] Cleam flag.
* @return Ethernet alarm status.
* @retval -1 Failed to get ethernet alarm status.
*/
int GetEthAlarm(SemHandl_t hSem, int bClean)
{
	int ret;
	if(SemWait(hSem))
		return -1;
	ret = gIsEthernet;
	if(bClean)
		gIsEthernet = 0;
	if(SemRelease(hSem))
		return -1;
	return ret;
}
/**
* @brief Set external alarm.
* @param enable [i] External alarm status.
* @param hSem [i] Semaphore.
* @retval 0 Success.
* @retval -1 Failed to set alarm status.
*/
int SetExtAlarm(int enable, SemHandl_t hSem)
{
	if(SemWait(hSem))
		return -1;
	gIsExt= enable;
	if(SemRelease(hSem))
		return -1;
	return 0;
}
/**
* @brief Get external alarm status.
* @param hSem [i] Semaphore.
* @param bClean[i] Cleam flag.
* @return External alarm status.
* @retval -1 Failed to get alarm status.
*/
int GetExtAlarm(SemHandl_t hSem, int bClean)
{
	int ret;
	if(SemWait(hSem))
		return -1;
	ret = gIsExt;
	if(bClean)
		gIsExt = 0;
	if(SemRelease(hSem))
		return -1;
	return ret;
}
/**
* @brief Set moion alarm.
* @param enable [i] Motion alarm status.
* @param hSem [i] Semaphore.
* @retval 0 Success.
* @retval -1 Failed to set alarm status.
*/
int SetMotionAlarm(int enable, SemHandl_t hSem)
{
	if(SemWait(hSem))
		return -1;
	gIsMotion = enable;
	if(SemRelease(hSem))
		return -1;
	return 0;
}
/**
* @brief Get motion alarm status.
* @param hSem [i] Semaphore.
* @param bClean[i] Cleam flag.
* @return Motion alarm status.
* @retval -1 Failed to get motion alarm status.
*/
int GetMotionAlarm(SemHandl_t hSem, int bClean)
{
	int ret;
	if(SemWait(hSem))
		return -1;
	ret = gIsMotion;
	if(bClean)
		gIsMotion = 0;
	if(SemRelease(hSem))
		return -1;
	return ret;
}
/**
* @brief Alarm processing thread.
*/
void *ProcAlarmThread(void *arg)
{
	int iCurTime;
	void *status = PROC_SUCESS;
	time_t tCurrentTime;
	struct tm *tmnow;
	AlarmParam_t* pEvironment = (AlarmParam_t*)arg;
	unsigned char enableIn,enableOut=0,trigger;
	LogEntry_t tLog;
	DmvaLogEntry_t tDmvaLog;
	SysInfo *pSysInfo = pAlarmSysInfo;
	
	/* Onvif - Semaphores Intialization */
	int retval;
	union semun{
		int val;
		struct semid_ds *buf;
		short *array;		
	}argument;
	/* End-Onvif */	

	DBG("QID: %d\n", pEvironment ->msg_id);
	DBG("From: %d\n", pEvironment ->msg_buf.src);
	DBG("pSysInfo: %p\n", pSysInfo);
	DBG("Index: %d\n", pEvironment ->msg_buf.index);
		
	if((iCurTime = time(&tCurrentTime)) == -1){
		ERR("Fail on get current time\n");
		status =PROC_FAIL;
		goto ALARM_EXIT;
	}
	tmnow = localtime(&tCurrentTime);
	memcpy(&tLog.time, tmnow, sizeof(struct tm));
	memcpy(&tDmvaLog.time, tmnow, sizeof(struct tm));
	switch(pEvironment->msg_buf.event){
		case ALARM_EVENT_AUDIO_TRIG:
		{
			int alarmDuration;
			DBG("ALARM_EVENT_AUDIO_TRIG \n");

			enableOut     = (pSysInfo->lan_config.nExtAlarm & pSysInfo->lan_config.giooutenable);
			trigger       = pSysInfo->lan_config.gioouttype;
			alarmDuration = pSysInfo->lan_config.nAlarmDuration;

			if((pSysInfo->audio_config.alarmON == 0)||
			   (pSysInfo->lan_config.alarmstatus & FLG_UI_AUDIO)){
				break;
			}

			AddSchedule(AUDIO_SCHEDULE, tmnow, pSysInfo);

			if(enableOut == 1){
				ipnc_gio_write(GIO_ALARMOUT,trigger);
				Set_AlarmDuration(alarmDuration);
			}

			sprintf(tLog.event, "Audio Alarm");
			fSetSysLog(&tLog);

            // Audio on alarm
            SemWait(audioSem);
            setAudioCount(alarmDuration);
            SemRelease(audioSem);

            if(gInsideAudPlayTh == 0)
            {
			    AlarmAudio_Play(pEvironment->audMsg_id);
			}

			break;
		}
		case ALARM_EVENT_MOTION_DETECT:
		{
			int alarmDuration;
			DBG("ALARM_EVENT_MOTION_DETECT \n");
			/* Onvif - The below semaphores are created for the Onvif Event operation.
				   When motion happens, semaphore value is set. This value will 
				   be used in tcp_notify thread for sending notifications */
			argument.val = MOTION_DETECTED; 
			if( semctl(id_motion_sem, 0, SETVAL, argument) < 0)
			{
				fprintf( stderr, "Cannot set semaphore value.\n");
			}
			else
			{
				fprintf(stderr, "Semaphore %d initialized.\n", KEY);
			}
			/* End-Onvif */

			enableOut     = (pSysInfo->lan_config.nExtAlarm & pSysInfo->lan_config.giooutenable);
			trigger       = pSysInfo->lan_config.gioouttype;
			alarmDuration = pSysInfo->lan_config.nAlarmDuration;

			if((pSysInfo->motion_config.motionenable == 0)||
			   (pSysInfo->lan_config.alarmstatus & FLG_UI_MOTION)){
				break;
			}

			SetJpegStartSerial(pEvironment->msg_buf.serial);
			AddSchedule(MOTION_SCHEDULE, tmnow, pSysInfo);

			if(enableOut == 1){
				ipnc_gio_write(GIO_ALARMOUT,trigger);
				Set_AlarmDuration(alarmDuration);
			}

			sprintf(tLog.event, "Motion Alarm");
			fSetSysLog(&tLog);

			// Audio on alarm
            SemWait(audioSem);
            setAudioCount(alarmDuration);
            SemRelease(audioSem);

            if(gInsideAudPlayTh == 0)
            {
			    AlarmAudio_Play(pEvironment->audMsg_id);
			}

			break;
		}
		case ALARM_EVENT_ALARMRESET_TRIG:
		{
			DBG("ALARM_EVENT_ALARMRESET_TRIG \n");
			ResetSchedule();
			Set_AlarmDuration(0);
			setAudioCount(-1);
			break;
		}
		case ALARM_EVENT_ALARMIN_TRIG:
		{
			int alarmDuration;
			DBG("ALARM_EVENT_ALARMIN_TRIG \n");


            enableIn      = (pSysInfo->lan_config.nExtAlarm & pSysInfo->lan_config.gioinenable);
			enableOut     = (pSysInfo->lan_config.nExtAlarm & pSysInfo->lan_config.giooutenable);
			trigger       = pSysInfo->lan_config.gioouttype;
			alarmDuration = pSysInfo->lan_config.nAlarmDuration;

			/** check if turn gio alarm on   */
			if((enableIn == 0)||
			   (pSysInfo->lan_config.alarmstatus & FLG_UI_EXT)){
				break;
			}

			AddSchedule(EXT_SCHEDULE, tmnow, pSysInfo);

			if(enableOut == 1){
				ipnc_gio_write(GIO_ALARMOUT,trigger);
				Set_AlarmDuration(alarmDuration);
			}

			sprintf(tLog.event, "External Alarm");
			fSetSysLog(&tLog);

			// Audio on alarm
            SemWait(audioSem);
            setAudioCount(alarmDuration);
            SemRelease(audioSem);

            if(gInsideAudPlayTh == 0)
            {
			    AlarmAudio_Play(pEvironment->audMsg_id);
			}

			break;
		}
		case ALARM_EVENT_ALARMETH_TRIG:
		{
			int alarmDuration;
			DBG("ALARM_EVENT_ALARMETH_TRIG \n");

			enableOut     = (pSysInfo->lan_config.nExtAlarm&pSysInfo->lan_config.giooutenable);
			trigger       = pSysInfo->lan_config.gioouttype;
			alarmDuration = pSysInfo->lan_config.nAlarmDuration;

			/**  check if turn Ethernet alarm on */
			if((pSysInfo->lan_config.lostalarm == 0)||
			   (pSysInfo->lan_config.alarmstatus & FLG_UI_ETH)){
				break;
			}

			AddSchedule(ETH_SCHEDULE, tmnow, pSysInfo);

			if(enableOut == 1){
				ipnc_gio_write(GIO_ALARMOUT,trigger);
				Set_AlarmDuration(alarmDuration);
			}

			sprintf(tLog.event, "Ethernet Alarm");
			fSetSysLog(&tLog);

			// Audio on alarm
            SemWait(audioSem);
            setAudioCount(alarmDuration);
            SemRelease(audioSem);

            if(gInsideAudPlayTh == 0)
            {
			    AlarmAudio_Play(pEvironment->audMsg_id);
			}

			break;
		}
		case ALARM_EVENT_SCHEDULE:
		{
			int nflag, bIsAviRun;
			int isFTPAVI,isSDAVI;
			int supportJPG,supportH264,supportH264cif,supportMpeg4,supportMpeg4cif,supportAVI;
			DBG("ALARM_EVENT_SCHEDULE \n");
			// While DMVA event recording is underway,wait for its completion
			// before starting scheduled recording
			while(pSysInfo->sdcard_config.schAviRun == 1)
			{
				sleep(1); // sleep for 1 sec
			}
			isFTPAVI=pSysInfo->ftp_config.ftpfileformat;
			isSDAVI=pSysInfo->sdcard_config.sdfileformat;
			supportJPG = pSysInfo->lan_config.Supportstream1;
			supportMpeg4 = pSysInfo->lan_config.Supportstream2;
			supportMpeg4cif = pSysInfo->lan_config.Supportstream3;
			supportH264 = pSysInfo->lan_config.Supportstream5;
			supportH264cif = pSysInfo->lan_config.Supportstream6;
			supportAVI = supportH264|supportH264cif|supportMpeg4|supportMpeg4cif;
			bIsAviRun = 0;
			sprintf(tLog.event, "Schedule Record");
			nflag =0;
			if(pSysInfo->sdcard_config.sdinsert<=1){
				if(supportAVI&&pSysInfo->sdcard_config.sdrenable&&isSDAVI==0&&
						!gbSD_WriteProtect){
					nflag |= AVI_TO_SD;
					bIsAviRun = 1;
				}
				if(supportAVI&&pSysInfo->ftp_config.rftpenable&&isFTPAVI==0){
					nflag |= AVI_TO_FTP;
					bIsAviRun = 1;
				}
				if(bIsAviRun){
					AviRun(AVI_MODE1, nflag);
					SemWait(pEvironment->hAlarmSem);
					gSchAviRun = 1;
					SemRelease(pEvironment->hAlarmSem);
				}
			} else {
				/* AVI no stop case */
				if(supportAVI&&pSysInfo->sdcard_config.sdrenable&&isSDAVI==0&&
						!gbSD_WriteProtect){
					AviRun(AVI_MODE2, AVI_TO_SD);
					SemWait(pEvironment->hAlarmSem);
					gSchAviRun = 1;
					SemRelease(pEvironment->hAlarmSem);
				}
			}
			nflag =0;
			if(isSDAVI==1&&pSysInfo->sdcard_config.sdrenable &&
					!gbSD_WriteProtect)
				nflag |= JPG_TO_SD;
			/* Megapixel JPEG mode */
			if(!supportAVI&&pSysInfo->sdcard_config.sdrenable&&!gbSD_WriteProtect)
				nflag |= JPG_TO_SD;
			if(pSysInfo->sdcard_config.sdinsert<=1){
				if(isFTPAVI==1 &&pSysInfo->ftp_config.rftpenable)
					nflag |= JPG_TO_FTP;
				/* Megapixel JPEG mode*/
				if(!supportAVI&&pSysInfo->ftp_config.rftpenable)
					nflag |= JPG_TO_FTP;
				if(nflag){
					JpgRun(nflag, 0 );
					SemWait(pEvironment->hAlarmSem);
					gSchJpgRun = 1;
					SemRelease(pEvironment->hAlarmSem);
				}
			}
			else{
				if(supportJPG&&pSysInfo->ftp_config.rftpenable)
					nflag |= JPG_TO_FTP;
				if(nflag){
					JpgRun(nflag, 1 );
					SemWait(pEvironment->hAlarmSem);
					gSchJpgRun = 1;
					SemRelease(pEvironment->hAlarmSem);
				}
			}
			fSetSysLog(&tLog);
			break;
		}
		case ALARM_EVENT_SCHEDULE_END:
		{
			int isFTPAVI;
			int supportJPG=0,supportH264=0,supportH264cif=0,supportMpeg4=0,supportMpeg4cif=0,supportAVI=0;
			DBG("ALARM_EVENT_SCHEDULE_END\n");
			isFTPAVI=pSysInfo->ftp_config.ftpfileformat;
			isFTPAVI=pSysInfo->ftp_config.ftpfileformat;
			supportJPG = pSysInfo->lan_config.Supportstream1;
			supportMpeg4 = pSysInfo->lan_config.Supportstream2;
			supportMpeg4cif = pSysInfo->lan_config.Supportstream3;
			supportH264 = pSysInfo->lan_config.Supportstream5;
			supportH264cif = pSysInfo->lan_config.Supportstream6;
			supportAVI = supportH264|supportH264cif|supportMpeg4|supportMpeg4cif;
			if(pSysInfo->sdcard_config.sdinsert==3){
				/* AVI no stop case */
				SemWait(pEvironment->hAlarmSem);
				if(!gbSD_WriteProtect && gSchAviRun){
					AviStop();
					gSchAviRun = 0;
				}
				if(gSchJpgRun){
					JpgStop(1);
					gSchJpgRun = 0;
				}
				SemRelease(pEvironment->hAlarmSem);
			} else {
				SemWait(pEvironment->hAlarmSem);
				if(gSchAviRun){
					AviStop();
					gSchAviRun = 0;
				}
				if(gSchJpgRun){
					JpgStop(0);
					gSchJpgRun = 0;
				}
				SemRelease(pEvironment->hAlarmSem);
			}
			break;
		}
		case ALARM_DMVAEVENT_TRIP_ZONE:
		{
			DBG("ALARM_DMVAEVENT_TRIP_ZONE \n");

			ShareMemRead(pEvironment->msg_buf.alarmaudio_duration,tDmvaLog.eventListFileName,pEvironment->msg_buf.alarmaudio_enable);
			tDmvaLog.eventListFileName[pEvironment->msg_buf.alarmaudio_enable] = '\0';

            // Get the time stamp from the filename(DMVA_yyyymmddhhmmss.avi)
            char timeStr[5];
            char eventRecType[10],*charPtr;

            // Get Year
            strncpy(timeStr,tDmvaLog.eventListFileName + 5,4);
            timeStr[4] = '\0';
            tDmvaLog.time.tm_year = atoi(timeStr);

            // Get Month
            strncpy(timeStr,tDmvaLog.eventListFileName + 9,2);
            timeStr[2] = '\0';
            tDmvaLog.time.tm_mon = atoi(timeStr);

            // Get Day
            strncpy(timeStr,tDmvaLog.eventListFileName + 11,2);
            timeStr[2] = '\0';
            tDmvaLog.time.tm_mday = atoi(timeStr);

            // Get Hour
            strncpy(timeStr,tDmvaLog.eventListFileName + 13,2);
            timeStr[2] = '\0';
            tDmvaLog.time.tm_hour = atoi(timeStr);

            // Get Minute
            strncpy(timeStr,tDmvaLog.eventListFileName + 15,2);
            timeStr[2] = '\0';
            tDmvaLog.time.tm_min = atoi(timeStr);

            // Get Second
            strncpy(timeStr,tDmvaLog.eventListFileName + 17,2);
            timeStr[2] = '\0';
            tDmvaLog.time.tm_sec = atoi(timeStr);

			sprintf(tDmvaLog.eventListTimeStamp_sys, "%d-%02d-%02d %02d:%02d:%02d",
					tDmvaLog.time.tm_year, tDmvaLog.time.tm_mon, tDmvaLog.time.tm_mday,
					tDmvaLog.time.tm_hour, tDmvaLog.time.tm_min, tDmvaLog.time.tm_sec);

			tDmvaLog.eventListArchiveFlag_sys = 0x1;/* Currently supports only Video Archive */
			tDmvaLog.eventListEventType_sys = pEvironment->msg_buf.serial;/* 0: IMD, 1: TZ, 2: COUNT, 3: Tamper, 4: FD */
			switch(pEvironment->msg_buf.serial) {
				case 0:
					sprintf(tDmvaLog.eventListEventDetails_sys, "IMD EVENT ");
					break;
				case 1:
					sprintf(tDmvaLog.eventListEventDetails_sys, "TRIP ZONE EVENT ");
					break;
				case 2:
					sprintf(tDmvaLog.eventListEventDetails_sys, "OBJECT COUNT EVENT ");
					break;
				case 3:
					sprintf(tDmvaLog.eventListEventDetails_sys, "CAMERA TAMPER EVENT ");
					break;
				default:
					sprintf(tDmvaLog.eventListEventDetails_sys, "DMVA EVENT");
					break;
			};

            eventRecType[0] = '\0';
            charPtr         = eventRecType;

            if((pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys & 0x4) == 0x4)
            {
			    // audio is recorded
			    sprintf(charPtr,"AUD ");
			    charPtr += 4;
			}

            if((pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys & 0x2) == 0x2)
            {
			    // meta data is recorded
			    sprintf(charPtr,"MET ");
			}

            if(strlen(eventRecType) > 0)
            {
			    strcat(tDmvaLog.eventListEventDetails_sys,eventRecType);
			}

			fSetDmvaSysLog(&tDmvaLog);

			break;
		}
		default:
			ERR("Unknown Event\n");
			sprintf(tLog.event, "Unknown Event");
			fSetSysLog(&tLog);
			status = PROC_FAIL;
			break;
	}

ALARM_EXIT:
	FREE(pEvironment);
	DBG("Free evironment sucess\n");
	pthread_exit(status);
	return status;
}
