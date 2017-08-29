/* ===========================================================================
* @path $(IPNCPATH)\sys_adm\system_server
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
* @file system_contorl.c
* @brief Send commands to system server.
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system_control.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include "file_msg_drv.h"
#include <ApproDrvMsg.h>
#include "net_config.h"
#include "sd_config.h"
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <Appro_interface.h>
#include <sem_util.h>
#include <system_default.h>
#include <ipnc_ver.h>
#include <time.h>
#include <alarm_msg_drv.h>
#include <ipnc_gio_util.h>
#include <avi_mng.h>
#include <jpg_mng.h>
#include <schedule_mng.h>
#include <alarm_schedule.h>
#include <gio_alarmout.h>
#include <av_server_ctrl.h>
#include <signal.h>
#include <rtsp_ctrl.h>
#include <semaphore.h>

extern SemHandl_t gFORKSem;
//#define SYS_CTRL_DEBUG
//#define SYS_CTRL_ERROR

#ifdef SYS_CTRL_DEBUG
#define DBG(fmt, args...)	fprintf(stderr, fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#ifdef SYS_CTRL_ERROR
#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#else
#define __E(fmt, args...)
#endif

//#define WDT_RESET_TEST

unsigned char restart_flag = 0;
unsigned char alarmRestart_flag = 0;
unsigned char default_flag = 0;
static SemHandl_t hndlSysSem = NULL;
static int gPlatform = -1, network_en = 0, sdcard_en = 0, upnp_en = 0, dhcp_fail = 0;
static __u8 gResetbttn_Error = 0;
static SysInfo* gpSysInfo;
/* Onvif Semaphore id for motion detection */
int id_motion_sem;

int UpdateEnvFromFile(SysInfo* pSysInfo);
int Platform_check( void );
static unsigned char charToint(unsigned char *pChar);
static int	NcharToInt(char *pChar, int num );
/* Current recording file name in SD card */
extern char gSdFileName[];
extern SemHandl_t hSdFileSem;
extern void CleanSysMsg();
extern int gAS_Pause;

#define KEY (1492)
#define KEY_NOTIFY_SEM (1729)
//extern int alarmDurationTime;
extern int	gFileDispCount;

/**
* @brief check device version
*
* @param void
* @return device version
* @retval 0  old version
* @reval 1   new version
* @retval -1  Error
*/
int Check_device_version( void )
{
	int 	fd_proc = 0;
	char	data[5];
	int		result = 0;
	char 	dev_name[] = "/proc/device_version";

	fd_proc = open(dev_name, O_RDONLY);

	if( !fd_proc )
	{
		printf("Check_device_version : open device error !! \n");
		return -1;
	}
	result = read(fd_proc,  data, 1);
	if( result <= 0 )
	{
		printf("Check_device_version : read device error !!\n");
		close(fd_proc);
		return -1;
	}
	close(fd_proc);
	if( data[0] == '1')
	{
		printf("Check_device_version : DEVICE VER = 1 !!\n");
		return 1;
	}else{
		printf("Check_device_version : DEVICE VER = 0 !!\n");
		return 0;
	}

	return -1;
}

#ifdef WDT_RESET_TEST

#define WDT_RESET_COUNT		(58)
unsigned int wdt_reset_count = 0;

void WDT_RESET_CHECK(void)
{
	wdt_reset_count++;

	if(wdt_reset_count==WDT_RESET_COUNT) {
		wdt_reset_count = 0;
		SetWDTResetMsg(1);
	}
}

#endif

/**
* @brief If restart.
*
* @return restart flag.
*/

int IsRestart()
{
	int ret;
	SemWait(hndlSysSem);
	ret = restart_flag;
	SemRelease(hndlSysSem);
	return ret;
}

int IsAlarmRestart()
{
	int ret;
	SemWait(hndlSysSem);
	ret = alarmRestart_flag;
	SemRelease(hndlSysSem);
	return ret;
}

/**
* @brief Set to restart.
*
*/
void SetReloadFlag(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	pSysInfo->lan_config.reloadFlag = value;
}

void SetRestart()
{
	SetReloadFlag(1);

	SemWait(hndlSysSem);
	restart_flag = 1;
	alarmRestart_flag = 1;
	SemRelease(hndlSysSem);
}

/**
* @brief Clean restart flag.
*
*/
void CleanRestart()
{
	SemWait(hndlSysSem);
	restart_flag = 0;
	SemRelease(hndlSysSem);
}

void CleanAlarmRestart()
{
	SemWait(hndlSysSem);
	alarmRestart_flag = 0;
	SemRelease(hndlSysSem);
}

int IsResetDefault()
{
	int ret;
	SemWait(hndlSysSem);
	ret = default_flag;
	SemRelease(hndlSysSem);
	return ret;
}

void SetDefault()
{
	SetReloadFlag(1);

	SemWait(hndlSysSem);
	default_flag = 1;
	SemRelease(hndlSysSem);
}

void CleanSetDefault()
{
	SemWait(hndlSysSem);
	default_flag = 0;
	SemRelease(hndlSysSem);
}

/**
* @brief Check the running platform
*
* @return Platfprm check result
* @retval -1  Error
* @retval 0  EVM
* @retval 1  IPNC
*/
int Platform_check( void )
{
	FILE *pfile = NULL;
	char tempbuff[100];
	char filename[]="/proc/version";
	int	 ret = 0;
	char *pStr = NULL;

	pfile = fopen(filename,"r");
	if( pfile == NULL )
	{
		ret = -1;
		goto CHECK_END;
	}

	if( fread(tempbuff, sizeof(tempbuff),1,pfile) <= 0 )
	{
		ret = -1;
		goto CHECK_END;
	}

	tempbuff[sizeof(tempbuff)-1]='\0';

	/*Linux version 2.6.10_mvl401_IPNC_1.0.0.0 (root@localhost.localdomain) (gcc ve.....*/

	pStr = strstr(tempbuff,"IPNC");
	if( pStr != NULL )
	{
		ret = 1;
		goto CHECK_END;
	}else
		ret = 0;

	pStr = strstr(tempbuff,"EVM");
	if( pStr != NULL )
	{
		ret = 0;
		goto CHECK_END;
	}


CHECK_END:

	if( pStr )
	{
		fprintf(stderr,"%s \n",pStr);
	}
	if( pfile )
	{
		fclose(pfile);
	}
	return ret;
}
/**
* @brief Check if USB is inserted
*
* @retval 1 Inserted
* @retval 0 Not inserted
*/
int IsUsbInserted()
{
	int ret = 0;
	FILE *fp;
	fp = fopen(USB_PATH, "rb");
	if(fp){
		ret = 1;
		fclose(fp);
	}
	DBG("%s : %d\n", __func__, ret);
	return ret;
}
/**
* @brief Check if USB is connected
*
* @retval 1 Connect
* @retval 0 Not connect
*/
int IsUsbConnect()
{
	int ret = 0;
	FILE *fp;
	fp = fopen(USB_CONNECT_PATH, "rb");
	if(fp){
		ret = 1;
		fclose(fp);
	}
	DBG("%s : %d\n", __func__, ret);
	return ret;
}

/**
* @brief Start network
*
* @param pSysInfo [I] pointer to current system infomation
* @retval 0 Success
* @retval -1  Error
*/

int StartStaticNetwork(SysInfo *pSysInfo)
{
	int dhcp;
    //char syscmd[64];
	if(pSysInfo == NULL){
    	return -1;
    	}
    	if(net_get_hwaddr(ETH_NAME, pSysInfo->lan_config.net.MAC) != 0){
    		__E("Error on get MAC address\n");
        	return -1;
    	}
        //sprintf(syscmd, "ifconfig eth0 hw ether %02X:%02X:%02X:%02X:%02X:%02X\n", pSysInfo->lan_config.net.MAC[0] , pSysInfo->lan_config.net.MAC[1], pSysInfo->lan_config.net.MAC[2], pSysInfo->lan_config.net.MAC[3], pSysInfo->lan_config.net.MAC[4], pSysInfo->lan_config.net.MAC[5]); 

        //SYSTEM(syscmd);
    	SYSTEM("ifconfig eth0 up\n");
    	if(net_set_ifaddr(ETH_NAME, pSysInfo->lan_config.net.ip.s_addr) < 0){
    		__E("Error on Set ip\n");
        	return -1;
    	}
    	/* set net mask */
    	if(net_set_netmask(ETH_NAME, pSysInfo->lan_config.net.netmask.s_addr) < 0){
    		__E("Fail on set netmask\n");
        	return -1;
    	}
    	/* set gateway */
    	if(net_set_gateway(pSysInfo->lan_config.net.gateway.s_addr) < 0){
    		__E("Fail on set gateway\n");
        	return -1;
    	}
	return 0;

}

int StartDhcpNetwork(SysInfo *pSysInfo)
{
	int dhcp;
	if(pSysInfo == NULL){
		return -1;
	}
	if(net_get_hwaddr(ETH_NAME, pSysInfo->lan_config.net.MAC) != 0){
		 __E("Error on get MAC address\n");
		return -1;
	}
	SetDhcpEnable(1);
	if(dhcp_fail){
		if(net_get_hwaddr(ETH_NAME, pSysInfo->lan_config.net.MAC) != 0){
			__E("Error on get MAC address\n");
			return -1;
		}
		SYSTEM("ifconfig eth0 up\n");
		if(net_set_ifaddr(ETH_NAME, pSysInfo->lan_config.net.ip.s_addr) < 0){
			__E("Error on Set ip\n");
			return -1;
		}
		/* set net mask */
		if(net_set_netmask(ETH_NAME, pSysInfo->lan_config.net.netmask.s_addr) < 0){
			__E("Fail on set netmask\n");
			return -1;
		}
		/* set gateway */
		if(net_set_gateway(pSysInfo->lan_config.net.gateway.s_addr) < 0){
			__E("Fail on set gateway\n");
			return -1;
		}
	}
	/* set http port will be done by BOA */
	return 0;
}

/**
* @brief Restart all
*
*/
static void RestartAll()
{
	SysInfo *pSysInfo = GetSysInfo();
	StreamEnv_t tConfig;
	int nExtFunc = 0;
	ResetSchedule();
	gAS_Pause = 1;
	PauseSchedule();
	/* Force all recoding stop */
	AviStop();
	JpgStop(pSysInfo->sdcard_config.sdinsert ? 1 : 0);

	SYSTEM("killall -2 Appro_avi_save");
	SYSTEM("killall -9 audio-receiver");
	SYSTEM("killall -9 aplay");

	ApproInterfaceExit();
	SetRTSPStreamingOFF();

	SYSTEM("killall -2 av_server.out");
	SYSTEM("killall -9 boa");

	sleep(2);

	CleanSysMsg();
	tConfig.nDayNight = pSysInfo->lan_config.nDayNight;
	tConfig.nWhiteBalance = pSysInfo->lan_config.nWhiteBalance;
	tConfig.nBackLight = pSysInfo->lan_config.nBackLight;
	tConfig.nBrightness = pSysInfo->lan_config.nBrightness;
	tConfig.nContrast = pSysInfo->lan_config.nContrast;
	tConfig.nSaturation = pSysInfo->lan_config.nSaturation;
	tConfig.nSharpness = pSysInfo->lan_config.nSharpness;
	tConfig.nVideocodecmode = pSysInfo->lan_config.nVideocodecmode;
	tConfig.nVideocodecres = pSysInfo->lan_config.nVideocodecres;
	tConfig.nBinning = pSysInfo->lan_config.nBinning;
	tConfig.tStampEnable=pSysInfo->lan_config.tstampenable;
	tConfig.tStampFormat=pSysInfo->lan_config.tstampformat;
	tConfig.nJpegQuality = pSysInfo->lan_config.njpegquality;
	tConfig.nMirror = pSysInfo->lan_config.mirror;
	tConfig.encrypt = pSysInfo->lan_config.encryptEnable;
    tConfig.vsEnable = pSysInfo->advanceMode.vsEnable;
    tConfig.ldcEnable = pSysInfo->advanceMode.ldcEnable;
    tConfig.vnfEnable = pSysInfo->advanceMode.vnfEnable;
    tConfig.vnfMode = pSysInfo->advanceMode.vnfMode;
	tConfig.nFaceDetect = pSysInfo->face_config.fdetect;
	tConfig.nDemoCfg = pSysInfo->lan_config.democfg;
	tConfig.nAEWswitch = pSysInfo->lan_config.nAEWswitch;
	tConfig.nAEWtype = pSysInfo->lan_config.nAEWtype;
	tConfig.expPriority = pSysInfo->lan_config.expPriority;
	tConfig.nRateControl1 = pSysInfo->lan_config.nRateControl1;
	tConfig.nRateControl2 = pSysInfo->lan_config.nRateControl2;
	tConfig.nFrameRate1 = pSysInfo->lan_config.nFrameRate1;
	tConfig.nFrameRate2 = pSysInfo->lan_config.nFrameRate2;
	tConfig.nFrameRate3 = pSysInfo->lan_config.nFrameRate3;
	tConfig.nMEConfig1 = pSysInfo->codec_advconfig[0].meConfig;
	tConfig.nMEConfig2 = pSysInfo->codec_advconfig[1].meConfig;
	tConfig.nMEConfig3 = pSysInfo->codec_advconfig[2].meConfig;
	tConfig.imagesource = pSysInfo->lan_config.net.imagesource;
	tConfig.nDisplay = pSysInfo->lan_config.nLocalDisplay;
	tConfig.histogram = pSysInfo->lan_config.histogram;
	tConfig.dynRange = pSysInfo->advanceMode.dynRange;
	tConfig.nBitrate1 = pSysInfo->lan_config.nMpeg41bitrate;
	tConfig.nBitrate2 = pSysInfo->lan_config.nMpeg42bitrate;
	tConfig.audioenable = pSysInfo->audio_config.audioenable;
	tConfig.audiocodectype = pSysInfo->audio_config.codectype;
	tConfig.audiobitrate = pSysInfo->audio_config.bitrate;
	tConfig.audioinvolume = pSysInfo->audio_config.audioinvolume;
	tConfig.audiooutvolume = pSysInfo->audio_config.audiooutvolume;
	tConfig.audioSampleRate = pSysInfo->audio_config.samplerate;
	tConfig.audioalarm = pSysInfo->audio_config.alarmON;
	tConfig.audioalarmlevel = pSysInfo->audio_config.alarmlevel;

	StartStream(&tConfig);
	InitAV_Server(&tConfig);

	SemWait(gFORKSem);
	system(BOA_START_CMD);
	SemRelease(gFORKSem);

	nExtFunc = GetRTSPStreamOption();
	SetRTSPStreamingON(nExtFunc);
	RTSP_AudioRecvON();

	gAS_Pause = 0;
	ResumeSchedule();

	SendForceIFrameMsg(1); // Force IDR and Sync once streaming starts

	system(DROP_FILESYS_CMD);

	SetReloadFlag(0); //Send message to GUI that restart is Over

	CleanAlarmRestart(); // clean alarm restart flag
}

static int BootProc2()
{
	DBG("%s\n", __func__);
	if(network_en){
		/* start upnp */
		if(!upnp_en){
			SemWait(gFORKSem);
			system("./upnp-scanip &\n");
			SemRelease(gFORKSem);
			upnp_en = 1;
		}
	}
	return 0;
}

static int BootProc1()
{
	int dhcp;
	DBG("%s\n", __func__);

	if(!sdcard_en){
		CheckSDCard(1, gPlatform);
		sdcard_en = 1;
	}
	if(gPlatform == 0)
		dhcp = 1;
	else
		dhcp = ipnc_gio_read(GIO_DHCP);

	if ((!dhcp) || dhcp_fail ){
		/* set DNS */
		if(net_set_dns(inet_ntoa(gpSysInfo->lan_config.net.dns)) < 0){
			__E("Fail on set dns\n");
			return -1;
		}
	}

	ipnc_gio_write(GIO_LED_3, LED_OFF);
	WriteConfigFile();
	SYSTEM("cp /var/www/usbconf.exe /mnt/ramdisk\n");
	SYSTEM("cp /var/www/autorun.inf /mnt/ramdisk\n");
	SYSTEM("cp /tmp/NET.TXT /mnt/ramdisk\n");
	SYSTEM("sync\n");
	SYSTEM("rmmod g_file_storage.ko\n");
	SYSTEM("insmod g_file_storage.ko file=/dev/ram0 stall=0 removable=1 \n");

	network_en = 1;

	/* This is needed during upgrade */
	SYSTEM("cp /sbin/shutdown /tmp/");

	return 0;
}

/**
* @brief System server thread
*/
static void *PollingThread(void *arg){
	int dhcp = 1;
	__u8 isReset = 0;
	__u8 isAlarmReSet;
	__u8 alarmStatus;
	int durationTimer = 0;
	int GIO_now=-1,link_status=-1, old_network_en = network_en,temp = 1;
	int network_enable = 1;
	unsigned int sleep_time = 300000;

	/* Onvif - Semaphores Intialization */
	int id,retval;
	char init_sem = 0;
	struct sembuf operations[1];
	union semun{
		int val;
		struct semid_ds *buf;
		short *array;
	}argument;
	argument.val = 0;
	/* End-Onvif */
	in_addr_t ip;
	in_addr_t netmask;
	in_addr_t gateway;
	in_addr_t dns;
	SysInfo *pSysInfo = GetSysInfo();
	static int rstTimer = 0;
	int resetCodecDone = 0;
	__u8 bIsUsbModify = 0;
	int nExtFunc = 0;
	int	dev_ver = Check_device_version();

	while(1)
	{
		/* Below check is done to make sure semaphore is created after the Boot up.
		   "sem_start" is created in autorun.sh during bootup*/
		if (!(init_sem & 0x0f) && access("/tmp/sem_start", F_OK) != -1)
		{

			/* Onvif - The below semaphores are created for the Onvif Event operation */
			id_motion_sem = semget(KEY_NOTIFY_SEM, 1, 0666 | IPC_CREAT);
			if(id_motion_sem < 0)
			{
				fprintf(stderr, "Unable to obtain motion semaphore.\n");
			}
			id = semget(KEY, 1, 0666 | IPC_CREAT);
			if(id < 0)
			{
				fprintf(stderr, "Unable to obtain hello semaphore.\n");
			}
			if( semctl(id, 0, SETVAL, argument) < 0)
			{
				fprintf( stderr, "Cannot set semaphore value.\n");
			}
			operations[0].sem_num = 0;
			operations[0].sem_op = 1;
			operations[0].sem_flg = 0;
			init_sem |= 1;
			system("rm -f /tmp/sem_start");
			/* End-Onvif */
		}
		if(gPlatform == 1){
			dhcp = ipnc_gio_read(GIO_DHCP);
			if(!dhcp)
				dhcp_fail = 0;
			if(sdcard_en)
				CheckSDCard(0, gPlatform);
			if( dev_ver == 1 )
			{
				isAlarmReSet=ipnc_gio_read(GIO_ALARMRESET);
			}else{
				isAlarmReSet=ipnc_gio_read(GIO_ALARMRESET_OLD);
			}
			/** detect alarm reset*/
			if(isAlarmReSet==0){
				DBG("SendAlarmResetTrigger\n");
				SendAlarmResetTrigger();
			}else {
				/** detect extern alarm  */
				alarmStatus=ipnc_gio_read(GIO_ALARMIN);
				/** check if  gio equals to the trigger setting  */
				if(alarmStatus==pSysInfo->lan_config.giointype){
					DBG("SendAlarmInTrigger\n");
					SendAlarmInTrigger();
				}
			}
			/**Check ethernet  status  */
			if(network_en && !dhcp_fail)
				link_status=Check_Link_status(ETH_NAME);
			if (link_status!=-1)
			{
				/* Onvif - Signal resend for Hello operation when ethernet link comes back*/
				if (!temp && (temp^link_status))
				{
					/* Onvif - The below semaphores are created for the Onvif Hello operation */
					#if 0
					if(!(init_sem & 0xf0))
					{
						id = semget(KEY, 1, 0666 | IPC_CREAT);
						if(id < 0)
						{
							fprintf(stderr, "Unable to obtain hello semaphore.\n");
						}
						if( semctl(id, 0, SETVAL, argument) < 0)
						{
							fprintf( stderr, "Cannot set semaphore value.\n");
						}
						operations[0].sem_num = 0;
						operations[0].sem_op = 1;
						operations[0].sem_flg = 0;
						init_sem |= 0x10;
					}
					#endif
					retval = semop(id, operations, 1);
					if(retval != 0)
					{
						perror("Failed");
					}
				}
				temp = link_status;
				/* end-Onvif */
				if(GIO_now==link_status){}
				else{
					GIO_now=link_status;
					ipnc_gio_write(GIO_LED_3,!(GIO_now));
				}
			}
			if ((pSysInfo->lan_config.lostalarm==1) && (link_status==0)){
			   SendAlarmEthTrigger();
			   DBG("SendAlarmEthTrigger \n");
			}
			durationTimer = Get_AlarmDuration();
			if(durationTimer > 0){
				Sub_AlarmDuration();
			}else if(durationTimer == 0){
				Sub_AlarmDuration();
				ipnc_gio_write(GIO_ALARMOUT,!(pSysInfo->lan_config.gioouttype));
			}
			if (!gResetbttn_Error)
				isReset = ipnc_gio_read(GIO_RESET_TO_DEFAULT) ? 0 : 1;
		}
		if(dhcp_fail)
			dhcp = 0;

#ifdef _DELAY_FOR_NFS
		// Masked & Changed for boot-time reduction
		if(network_enable == 1 && network_en){
#else
		if(network_enable == 1){

#endif
			nExtFunc = GetRTSPStreamOption();
			SetRTSPStreamingON(nExtFunc);
			RTSP_AudioRecvON();
			network_enable = 0;
			sleep_time = 1000000;
		}
		old_network_en = network_en;
		if(IsUsbInserted() ){
			bIsUsbModify =1;
		} else if(bIsUsbModify == 1){
			SYSTEM("umount /mnt/ramdisk/\n");
			SYSTEM("mount -t vfat /dev/ram0 /mnt/ramdisk");
			bIsUsbModify = 2;
			UpdateEnvFromFile(pSysInfo);
		}
		if(!IsUsbConnect() && bIsUsbModify==2){
			SYSTEM("rmmod g_file_storage.ko");
			SYSTEM("insmod g_file_storage.ko file=/dev/ram0 stall=0 removable=1 ");
			bIsUsbModify = 0;
		}
		if(network_en && dhcp != pSysInfo->lan_config.net.dhcp_enable){
			SetDhcpEnable(dhcp);
			if(!dhcp ){
				if((ip = net_get_ifaddr(ETH_NAME)) !=
							pSysInfo->lan_config.net.ip.s_addr){
					if(net_set_ifaddr(ETH_NAME,
							pSysInfo->lan_config.net.ip.s_addr) < 0){
						__E("Error on Set ip\n");
					}
				}
				if((netmask = net_get_netmask(ETH_NAME)) !=
						pSysInfo->lan_config.net.netmask.s_addr){
					/* set net mask */
					if(net_set_netmask(ETH_NAME,
							pSysInfo->lan_config.net.netmask.s_addr) < 0){
						__E("Fail on set netmask\n");
					}
				}
				if((gateway = net_get_gateway()) !=
							pSysInfo->lan_config.net.gateway.s_addr){
					/* set gateway */
					if(net_set_gateway(
							pSysInfo->lan_config.net.gateway.s_addr) < 0){
						__E("Fail on set gateway\n");
					}
				}
				if((dns = net_get_dns()) !=
							pSysInfo->lan_config.net.dns.s_addr){
					/* set DNS */
					if(net_set_dns(
							inet_ntoa(pSysInfo->lan_config.net.dns)) < 0){
						__E("Fail on set dns\n");
					}
				}
			}
			WriteConfigFile();
			SYSTEM("cp /tmp/NET.TXT /mnt/ramdisk");
			SYSTEM("umount /mnt/ramdisk");
			SYSTEM("rmmod g_file_storage.ko");
			SYSTEM("sync");
			SYSTEM("insmod g_file_storage.ko file=/dev/ram0 stall=0 removable=1 ");
			SYSTEM("mount -t vfat /dev/ram0 /mnt/ramdisk");
#if 0
			system("umount /mnt/ramdisk/\n");
			system("mount -t vfat /dev/ram0 /mnt/ramdisk");
			system("rmmod g_file_storage.ko\n");
			system("insmod g_file_storage.ko file=/dev/ram0 stall=0 removable=1 \n");
#endif
		}
		if(IsRestart()){
			printf("\nChange in Config Data: Reloading New Usecase !!!!!\n");
			CleanRestart();
			RestartAll();
		}
		if(IsResetDefault()){
			printf("\nReset Config Data to Default!!\n");
			CleanSetDefault();
			fResetSysInfo();
			CheckSDCard(1, gPlatform);
			SetRestart();
		}
		if(pSysInfo->lan_config.alarmstatus)
		{
			if(resetCodecDone==0) {
				//printf("Reset Codec to I frame MSG Sent\n");
				SendForceIFrameMsg(1);
				resetCodecDone = 1;
			}
		}
		else
			resetCodecDone = 0;

		if (isReset && !gResetbttn_Error) {
			if (rstTimer++ == 5) {
				printf("\nReset SysInfo!!\n");
				fResetSysInfo();
				CheckSDCard(1, gPlatform);
				SetRestart();
			} else
				printf("\nReset Count: %d\n", rstTimer);
		}
		else if(rstTimer){
			rstTimer = 0;
			printf("\nReset Timer Released!!\n");
		}


#ifdef WDT_RESET_TEST
		WDT_RESET_CHECK();
#endif
		usleep(sleep_time);
	}

	return NULL;
}

/**
* @brief Initial system
*
* @retval 0 Success
* @retval -1 System initial error
*/
int SystemInit()
{
	pthread_t thread;
	StreamEnv_t tConfig;
	SysInfo *pSysInfo;
	int dhcp;
	gResetbttn_Error = ipnc_gio_read(GIO_RESET_TO_DEFAULT) ? 0 : 1;

	gPlatform = Platform_check();

	if(gPlatform == -1){
		__E("Platform_check fail\n");
		return -1;
	}

	if(AlarmDrvInit(ALARM_SYS_MSG) < 0){
        __E("AlarmDrvInit fail\n");
        return -1;
    }

	gpSysInfo = pSysInfo = GetSysInfo();
	if(pSysInfo == NULL){
		CleanupFileMsgDrv();
		return -1;
	}

	if(gPlatform == 0)
		dhcp = 1;
	else
		dhcp = ipnc_gio_read(GIO_DHCP);

	if (!dhcp){
		if(StartStaticNetwork(gpSysInfo)){
			__E("Network Init Fail\n");
			return -1;
		}
   	}

	if(sdcard_en)
		CheckSDCard(1, gPlatform);

	if((hndlSysSem = MakeSem()) == NULL){
		CleanupFileMsgDrv();
		pSysInfo = NULL;
		return -1;
	}
	if(RTSP_ControlInit() != 0){
		DestroySem(hndlSysSem);
		hndlSysSem = NULL;
		CleanupFileMsgDrv();
		pSysInfo = NULL;
		return -1;
	}
	pthread_create(&thread, NULL, PollingThread, NULL);
	tConfig.nDayNight = pSysInfo->lan_config.nDayNight;
	tConfig.nWhiteBalance = pSysInfo->lan_config.nWhiteBalance;
	tConfig.nBackLight = pSysInfo->lan_config.nBackLight;
	tConfig.nBrightness = pSysInfo->lan_config.nBrightness;
	tConfig.nContrast = pSysInfo->lan_config.nContrast;
	tConfig.nSaturation = pSysInfo->lan_config.nSaturation;
	tConfig.nSharpness = pSysInfo->lan_config.nSharpness;
	tConfig.nVideocodecmode = pSysInfo->lan_config.nVideocodecmode;
	tConfig.nVideocodecres = pSysInfo->lan_config.nVideocodecres;
	tConfig.nBinning = pSysInfo->lan_config.nBinning;
	tConfig.tStampEnable=pSysInfo->lan_config.tstampenable;
	tConfig.tStampFormat=pSysInfo->lan_config.tstampformat;
	tConfig.nMirror = pSysInfo->lan_config.mirror;
	tConfig.encrypt = pSysInfo->lan_config.encryptEnable;
    tConfig.vsEnable = pSysInfo->advanceMode.vsEnable;
    tConfig.ldcEnable = pSysInfo->advanceMode.ldcEnable;
    tConfig.vnfEnable = pSysInfo->advanceMode.vnfEnable;
    tConfig.vnfMode = pSysInfo->advanceMode.vnfMode;
	tConfig.nFaceDetect = pSysInfo->face_config.fdetect;
	tConfig.nDemoCfg = pSysInfo->lan_config.democfg;
	tConfig.nJpegQuality = pSysInfo->lan_config.njpegquality;
	tConfig.nAEWswitch = pSysInfo->lan_config.nAEWswitch;
	tConfig.nAEWtype = pSysInfo->lan_config.nAEWtype;
	fSetImageAEW(tConfig.nAEWswitch);
	fSetImageAEWType(tConfig.nAEWtype);
	tConfig.expPriority = pSysInfo->lan_config.expPriority;
	tConfig.nRateControl1 = pSysInfo->lan_config.nRateControl1;
	tConfig.nRateControl2 = pSysInfo->lan_config.nRateControl2;
	tConfig.nFrameRate1 = pSysInfo->lan_config.nFrameRate1;
	tConfig.nFrameRate2 = pSysInfo->lan_config.nFrameRate2;
	tConfig.nFrameRate3 = pSysInfo->lan_config.nFrameRate3;
	tConfig.nMEConfig1 = pSysInfo->codec_advconfig[0].meConfig;
	tConfig.nMEConfig2 = pSysInfo->codec_advconfig[1].meConfig;
	tConfig.nMEConfig3 = pSysInfo->codec_advconfig[2].meConfig;
	tConfig.nDisplay = pSysInfo->lan_config.nLocalDisplay;
	tConfig.imagesource = pSysInfo->lan_config.net.imagesource;
	tConfig.histogram = pSysInfo->lan_config.histogram;
	tConfig.dynRange = pSysInfo->advanceMode.dynRange;
	tConfig.nBitrate1 = pSysInfo->lan_config.nMpeg41bitrate;
	tConfig.nBitrate2 = pSysInfo->lan_config.nMpeg42bitrate;
	tConfig.audioenable = pSysInfo->audio_config.audioenable;
	tConfig.audiocodectype = pSysInfo->audio_config.codectype;
	tConfig.audiobitrate = pSysInfo->audio_config.bitrate;
	tConfig.audioinvolume = pSysInfo->audio_config.audioinvolume;
	tConfig.audiooutvolume = pSysInfo->audio_config.audiooutvolume;
	tConfig.audioSampleRate = pSysInfo->audio_config.samplerate;
	tConfig.audioalarm = pSysInfo->audio_config.alarmON;
	tConfig.audioalarmlevel = pSysInfo->audio_config.alarmlevel;

    tConfig.nVideocodecmode = CODEC_COMB_ACS1910_H264; 

	StartStream(&tConfig);
	ipnc_gio_write(GIO_LED_2, LED_ON);
	if(pSysInfo->lan_config.gioouttype==0){
		ipnc_gio_write(GIO_ALARMOUT, 1);
	}
	else{
		ipnc_gio_write(GIO_ALARMOUT, 0);
	}
	char hostname[255];
	if(gethostname(hostname,255)==0)
	{
		char temp[10]="(none)";
		if(strcmp(hostname,temp)==0) {
			int chip_id = pSysInfo->lan_config.chipConfig;
			if(chip_id==CHIP_DM365)
				sethostname("DM365_IPNC",15);
			else if((chip_id==CHIP_DM368)||(chip_id==CHIP_DM368E))
				sethostname("DM368_IPNC",15);
			else if(chip_id==CHIP_DM369)
				sethostname("DM369_IPNC",15);
			else if(chip_id==CHIP_DMVA2)
				sethostname("DMVA2_IPNC",15);
			else if(chip_id==CHIP_DMVA1)
				sethostname("DMVA1_IPNC",15);
			else
				sethostname("UNKNOWN_IPNC",15);
		}
	}

	changeTimeZone();
	InitAV_Server(&tConfig);
	RTSP_AudioRecvON();

	syncOnvif(pSysInfo);

	/* Autmatic upgrade to defult setting, when release upgrade happens */
	if(pSysInfo->relUniqueId!=REL_UNIQUE_ID)
		SetDefault();

	if (dhcp)
	{
		if(StartDhcpNetwork(gpSysInfo)){
			__E("Network Init Fail\n");
			return -1;
		}
	}

	SendForceIFrameMsg(1); // Force IDR and Sync once streaming starts

	system(DROP_FILESYS_CMD);

	SetReloadFlag(0); //Send message to GUI that restart is Over

	return 0;
}

/**
* @brief Clean up system message driver
*
*/
void SystemCleanup(void)
{
	RTSP_ControlExit();
	DestroySem(hndlSysSem);
	hndlSysSem = NULL;
	ApproInterfaceExit();
	CleanupFileMsgDrv();
	AlarmDrvExit();
}
/**
* @brief Get DHCP status
*
* @retval NULL for failed to get DHCP status
* @return DHCP status.
*/
int* GetDhcpEn()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.dhcp_enable;
}

/**
* @brief Get http port number
*
* @return Http prot nunber
* @retval NULL Failed to get http port
*/
unsigned short *GetHttpPort()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.http_port;
}

/**
* @brief Set netmask
*
* @return Function to set netmask
*/
int SetNetMask(in_addr_t net_mask)
{
	struct in_addr NetMask;
	NetMask.s_addr = net_mask;
	return fSetNetMask(NetMask);
}

/**
* @brief Get NTP server
*
* @return NTP server
* @retval NULL Fail.
*/
char *GetFqdn()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->lan_config.net.ntp_server;
}

/**
* @brief Set time format
* @param value Time format
* @retval 0 Success
* @retval -1 Fail
*/
int SetTimeFormat(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	if(value != pSysInfo->lan_config.net.time_format)
		return fSetTimeFormat(value);
	return 0;
}

#define ZONE_INFO "/usr/share/info/"
#define ZONE_LOCAL "/usr/share/info/localtime"
#define ZONE_POSIX "/usr/share/info/posixrules"
static char *TZfiles[][2] = {
	{"Etc/GMT+12", "Pacific/Kwajalein"},
	{"Etc/GMT+11", "Pacific/Midway"},
	{"Etc/GMT+10", "US/Hawaii"},
	{"Etc/GMT+9", "US/Alaska"},
	{"Etc/GMT+8", "US/Pacific"},
	{"Etc/GMT+7", "US/Mountain"},
	{"Etc/GMT+6", "US/Central"},
	{"Etc/GMT+5", "US/Eastern"},
	{"Etc/GMT+4", "Canada/Atlantic"},
	{"Etc/GMT+3", "America/Buenos_Aires"},
	{"Etc/GMT+2", "Etc/GMT+2"},
	{"Etc/GMT+1", "Atlantic/Azores"},
	{"Etc/GMT", "Europe/London"},
	{"Etc/GMT-1", "Europe/Berlin"},
	{"Etc/GMT-2", "Europe/Athens"},
	{"Etc/GMT-3", "Europe/Moscow"},
	{"Etc/GMT-4", "Asia/Muscat"},
	{"Etc/GMT-5", "Asia/Karachi"},
	{"Etc/GMT-6", "Asia/Dhaka"},
	{"Etc/GMT-7", "Asia/Bangkok"},
	{"Etc/GMT-8", "Asia/Taipei"},
	{"Etc/GMT-9", "Asia/Tokyo"},
	{"Etc/GMT-10", "Australia/Brisbane"},
	{"Etc/GMT-11", "Asia/Magadan"},
	{"Etc/GMT-12", "Pacific/Fiji"}};
/**
* @brief Change time zone
*/
int changeTimeZone()
{
	__u8 idx1, idx2;
	char tzcmd[100];
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	idx1 = pSysInfo->lan_config.net.ntp_timezone;
	idx2 = pSysInfo->lan_config.net.daylight_time;
	/*sprintf(tzcmd,"cp -f " ZONE_INFO "%s " ZONE_LOCAL, TZfiles[idx1][idx2]);
	system(tzcmd);
	sprintf(tzcmd,"cp -f " ZONE_INFO "%s " ZONE_POSIX, TZfiles[idx1][idx2]);
	system(tzcmd);
	sprintf(tzcmd,"ln -sf " ZONE_INFO "%s /etc/localtime\n", TZfiles[idx1][idx2]);
	system(tzcmd);*/
	sprintf(tzcmd,"cp -f " ZONE_INFO "%s /tmp/localtime\n", TZfiles[idx1][idx2]);
	SYSTEM(tzcmd);
	//printf("TIMEZONE DAYLIGHT DEBUG COMMAND: %s\n",tzcmd);
	dbg("changeTimeZone:\n %s\n", tzcmd);
	tzset();
	return 0;
}

/**
* @brief function to manipulate DST & TZ
* @param "__u8 sel": 0:setDST, 1:setTZ
* @retval 0 on success
* @retval -1 on fail
*/
int SetDSTandTZ(__u8 sel, __u8 value)
{
	int ret;
	static __u8 doneflag = 0, changeflag = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	/* sel == 0:setDST, 1:setTZ */
	if (sel == 0) {
		doneflag++;
		if(value != pSysInfo->lan_config.net.daylight_time) {
			changeflag = 1;
			ret = fSetDaylightTime(value);
		} else ret = 0;
	} else if (sel == 1) {
		doneflag++;
		if(value != pSysInfo->lan_config.net.ntp_timezone) {
			changeflag = 1;
			ret = fSetTimeZone(value);
		} else ret = 0;
	} else ret = 0;

	if (ret == -1)
		return ret;

	if (doneflag == 2) {
		doneflag = 0;
		if (changeflag) {
			changeflag = 0;
			ret = changeTimeZone();
		}
	}
	return ret;
}

/**
* @brief interface to set Daylight Saving Time
* @param "unsigned char value": DST ON/OFF
* @retval 0 on success
* @retval -1 on fail
*/
int SetDaylightTime(unsigned char value)
{
	return SetDSTandTZ(0, value);
}


/**
* @brief interface to set Timezone
* @param "unsigned char value": selected timezone
* @retval 0 on success
* @retval -1 on fail
*/
int SetTimeZone(unsigned char value)
{
	return SetDSTandTZ(1, value);
}

/**
* @brief Get device type
*
* @return device type
* @retval 0 failed to get device type
*/
unsigned short *GetDeviceType()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return 0;
	return &pSysInfo->DeviceType;
}

/**
* @brief Get current recording file name in SD card.
*
* @return current recording file name in SD card
* @retval NULL for failed to get.
*/
char *GetSdFileName()
{
	static char strFileName[30];
	if(gSdFileName == NULL)
		return NULL;
	SemWait(hSdFileSem);
	strcpy(strFileName, gSdFileName);
	SemRelease(hSdFileSem);
	return strFileName;
}

/**
* @brief Get title
*
* @return title
* @retval NULL for failed to get title
*/
char *GetTitle()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->lan_config.title;
}

/**
* @brief Get RegUsr
*
* @return RegUsr
* @retval NULL for failed to get title
*/
char *GetRegUsr()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return (char *)pSysInfo->lan_config.regusr;
}

/**
* @brief Get OSDText
*
* @return OSDText
* @retval NULL for failed to get title
*/
char *GetOSDText()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return (char *)pSysInfo->lan_config.osdtext;
}

/**
* @brief Get mac address
*
* @return mac address
* @retval NULL for failed to get mac address
*/
unsigned char *GetMac()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->lan_config.net.MAC;
}

/**
* @brief Get ip
*
* @return ip
* @retval NULL for failed to get ip
*/
in_addr_t* GetIp()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.ip.s_addr;
}

/**
* @brief Get gateway
*
* @return gateway
* @retval NULL for failed to get gateway
*/
in_addr_t* GetGateWay()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.gateway.s_addr;
}

/**
* @brief Get netmask
*
* @return netmask
* @retval NULL for failed to get netmask
*/
in_addr_t* GetNetmask()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.netmask.s_addr;
}

/**
* @brief Get server ip
*
* @return server ip
* @retval NULL for failed to get server ip
*/
char* GetFtpServerIp()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->ftp_config.servier_ip;
}

/**
* @brief Get ftp user name
*
* @return ftp user name
* @retval NULL for failed to get ftp user name
*/
char *GetFtpUsername()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->ftp_config.username;
}

/**
* @brief Get ftp password
*
* @return ftp password
* @retval NULL for failed to get password
*/
char *GetFtpPassword()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->ftp_config.password;
}

/**
* @brief Get ftp foldname
*
* @return ftp foldname
* @retval NULL for failed to get ftp foldname
*/
char *GetFtpFoldername()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->ftp_config.foldername;
}

/**
* @brief Get ftp image amount
*
* @return ftp image amount
* @retval NULL for failed to get ftp image amount
*/
int *GetFtpImageAmount()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->ftp_config.image_acount;
}
/**
* @brief Get FTP port
* @retval NULL Fail.
* @return FTP port
*/
unsigned short *GetFtpPort()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->ftp_config.port;
}

/**
* @brief Get FTP pid
*
* @return FTP pid
* @retval NULL for failed to get ftp pid
*/
int *GetFtpPid()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->ftp_config.pid;
}

/**
* @brief Get SMTP server ip
*
* @return SMTP server ip
* @retval NULL for failed to get SMTP server ip
*/
char* GetSmtpServerIp()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.servier_ip;
}

/**
* @brief Get SMTP port
* @retval NULL Fail.
* @return SMTP port
*/
unsigned short *GetSmtpServerPort()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->smtp_config.server_port;
}

/**
* @brief Get SMTP authentication
*
* @return SMTP authentication
* @retval NULL for failed to get SMTP authentication
*/
unsigned char *GetSmtpAuthentication()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->smtp_config.authentication;
}

/**
* @brief Get SMTP user name
*
* @return SMTP user name
* @retval NULL for failed to get SMTP user name
*/
char *GetsmtpUsername()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.username;
}

/**
* @brief Get SMTP password
*
* @return SMTP password
* @retval NULL for failed to get SMTP password
*/
char *GetSmtpPassword()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.password;
}

/**
* @brief Get SMTP sender's email
*
* @return SMTP sender's email
* @retval NULL for failed to get SMTP sender's email
*/
char *GetSmtpSenderEmail()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.sender_email;
}

/**
* @brief Get SMTP receiver's email
*
* @return SMTP receiver's email
* @retval NULL for failed to get SMTP reveiver's email
*/
char *GetSmtpReceiverEmail()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.receiver_email;
}

/**
* @brief Get SMTP cc
*
* @return SMTP cc
* @retval NULL for failed to get SMTP cc
*/
char *GetSmtpCC()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.CC;
}

/**
* @brief Get SMTP mail subject
*
* @return SMTP mail subject
* @retval NULL for failed to get SMTP mail subject
*/
char *GetSmtpSubject()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.subject;
}

/**
* @brief Get SMTP mail text
*
* @return SMTP mail text
* @retval NULL for failed to get SMTP mail text
*/
char *GetSmtpText()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return pSysInfo->smtp_config.text;
}

/**
* @brief Get SMTP attachments
*
* @return SMTP attachments
* @retval NULL for failed to get SMTP attachments
*/
unsigned char *GetSmtpAttachments()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->smtp_config.attachments;
}

/**
* @brief Get SMTP view
*
* @return SMTP view
* @retval NULL for failed to get SMTP view
*/
unsigned char *GetSmtpView()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->smtp_config.view;
}

/**
* @brief Set IP
*
* @param ip IP value
* @return function to set IP
*/
int SetIp(in_addr_t ip)
{
	struct in_addr Ip;
	int ret;
	Ip.s_addr = ip;
	ret = fSetIp(Ip);
	WriteConfigFile();
	SYSTEM("cp /tmp/NET.TXT /mnt/ramdisk");
	SYSTEM("sync");
	return ret;
}

/**
* @brief Set http port
*
* @param port port number
* @return function to set http port
*/
int SetHttpPort(unsigned short port)
{
	int ret;
	/* 0 ~ 1024 to be reserved */
	if(port == 80 || port > 1024){
		SYSTEM("killall -9 boa\n");
		ret = fSetHttpPort(port);
		SemWait(gFORKSem);
		ret |= system(BOA_START_CMD);
		SemRelease(gFORKSem);

	} else
		ret = -1;
	return ret;
}

/**
* @brief Set title
*
* @param *buf title value
* @param length buffer length
* @return function to set title
*/
int SetTitle(void * buf, int length)
{
	return fSetTitle(buf, length);
}

/**
* @brief Set RegUsrName
*
* @param *buf RegUsrName value
* @param length buffer length
* @return function to set RegUsrName
*/
int SetRegUsrName(void * buf, int length)
{
	return fSetRegUsr(buf, length);
}

/**
* @brief Set gateway
*
* @param gateway gateway value
* @return function to set getaway
*/
int SetGateway(in_addr_t gateway)
{
	struct in_addr Gateway;
	Gateway.s_addr = gateway;
	return fSetGateway(Gateway);
}

/**
* @brief Set dhcp status
*
* @param enable determine on or off
* @return function to set dhcp status
* @retval -1 error on set ip
*/
int SetDhcpEnable(int enable)
{
	int count = 0;
	if(enable){
		do{
			net_enable_dhcpcd();
		}while(-1 == (int)net_get_netmask(ETH_NAME) && ++count < 3);
		if(count >= 3){
			dhcp_fail = 1;
			__E("dhcp fail\n");
			enable = 0;
		}
	} else {
		SysInfo *pSysInfo = GetSysInfo();
		net_disable_dhcpcd();
		if(net_set_ifaddr(ETH_NAME, pSysInfo->lan_config.net.ip.s_addr) < 0){
			__E("Error on Set ip\n");
			return -1;
		}
	}

	return fSetDhcpEnable(enable);
}

/**
* @brief Set ftp server ip
* @param ip server ip
* @return function to set iftp server p
*/
int SetFtpFqdn(void * buf, int length)
{
	return fSetFtpFqdn(buf, length);
}

/**
* @brief Set ftp user name
*
* @param *buf user name
* @param length buffer length
* @return function to set ip
*/
int SetFtpUsername(void * buf, int length)
{
	return fSetFtpUsername(buf, length);
}

/**
* @brief Set ftp password
*
* @param *buf password
* @param length buffer length
* @return function to set ftp password
*/
int SetFtpPassword(void * buf, int length)
{
	return fSetFtpPassword(buf, length);
}

/**
* @brief Set ftp foldname
*
* @param *buf foldname
* @param length buffer length
* @return function to set ftp foldname
*/
int SetFtpFoldname(void * buf, int length)
{
	return fSetFtpFoldname(buf, length);
}

/**
* @brief Set ftp image count
*
* @param imageacount ftp image count
* @return function to set ftp image count
*/
int SetFtpImageacount(int imageacount)
{
	return fSetFtpImageacount(imageacount);
}

int SetFtpPort(unsigned short port)
{
	return fSetFtpPort(port);
}

/**
* @brief Set ftp pid
*
* @param pid ftp pid
* @return function to set ftp pid
*/
int SetFtpPid(int pid)
{
	return fSetFtpPid(pid);
}

int SetSmtpServerIp(void *buf, int length)
{
	return fSetSmtpServerIp(buf, length);
}

int SetSmtpServerPort(unsigned short port)
{
	return fSetSmtpServerPort(port);
}

/**
* @brief Set SMTP authentication
*
* @param authentication SMTP authentication
* @return function to set SMTP authentication
*/
int SetSmtpAuthentication(unsigned char authentication)
{
	return fSetSmtpAuthentication(authentication);
}

/**
* @brief Set SMTP user name
*
* @param *buf user name
* @param length buffer length
* @return function to set SMTP user name
*/
int SetSmtpUsername(void * buf, int length)
{
	return fSetSmtpUsername(buf,length);
}

/**
* @brief Set SMTP password
*
* @param *buf password
* @param length buffer length
* @return function to set SMTP password
*/
int SetSmtpPassword(void * buf, int length)
{
	return fSetSmtpPassword(buf,length);
}

/**
* @brief Set SMTP user name
*
* @param *buf user name
* @param length buffer length
* @return function to set SMTP user name
*/
int SetSmtpSenderEmail(void * buf, int length)
{
	return fSetSmtpSenderEmail(buf,length);
}

/**
* @brief Set SMTP receiver email
*
* @param *buf receiver email
* @param length buffer length
* @return function to set SMTP receiver email
*/
int SetSmtpReceiverEmail(void * buf, int length)
{
	return fSetSmtpReceiverEmail(buf,length);
}

/**
* @brief Set SMTP cc
*
* @param *buf cc
* @param length buffer length
* @return function to set SMTP cc
*/
int SetSmtpCC(void * buf, int length)
{
	return fSetSmtpCC(buf,length);
}

/**
* @brief Set SMTP mail subject
*
* @param *buf subject
* @param length buffer length
* @return function to set SMTP  mail subject
*/
int SetSmtpSubject(void * buf, int length)
{
	return fSetSmtpSubject(buf,length);
}

/**
* @brief Set SMTP text
*
* @param *buf text
* @param length buffer length
* @return function to set SMTP text
*/
int SetSmtpText(void * buf, int length)
{
	return fSetSmtpText(buf,length);
}

/**
* @brief Set SMTP attachments
*
* @param attachments SMTP attachments
* @return function to set SMTP attachments
*/
int SetSmtpAttachments(unsigned char attachments)
{
	if(attachments<=1)
		attachments=1;
	if(attachments >= 20)
	    attachments = 20;

	return fSetSmtpAttachments(attachments);
}

/**
* @brief Set SMTP view
*
* @param view SMTP view
* @return function to set SMTP view
*/
int SetSmtpView(unsigned char view)
{
	return fSetSmtpView(view);
}

/**
* @brief Get DNS
*
* @return DNS IP
* @retval NULL for failed to get DNS IP
*/
in_addr_t* GetDns()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.dns.s_addr;
}
/**
* @brief Set DNS IP
*
* @param ip DNS IP
* @return function to set DNS IP
*/
int SetDns(in_addr_t ip)
{
	struct in_addr Ip;
	Ip.s_addr = ip;
	return fSetDns(Ip);
}

/**
* @brief Set SNTP server
*
* @param *buf SNTP server
* @param length buffer length
* @return function to set SNTP server
*/
int SetSntpServer(void * buf, int length)
{
	return fSetSntpServer(buf, length);
}


/**
* @brief Set Rate Control
*
* @param "unsigned char value": 0: OFF, 1: VBR, 2: CBR
* @return SUCCESS: 0, FAIL: -1
*/
int SetRateControl(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	if (value != pSysInfo->lan_config.nRateControl) {
		if ((ret = fSetRateControl(value)) == 0) {
			SetRestart();
			return 1;
		}
		return ret;
	}

	return 0;
}

/**
* @brief Set backlight status
*
* @param value on or off of backlight
* @return function to set backlight status
*/
int SetBlc(unsigned char value)
{
	return fSetBlc(value);
}

/**
* @brief Set one avi file length
*
* @param value avi file length.
*
* @return function to set avi file length
*/
int SetAVIDuration(unsigned char value)
{
	return fSetAVIDuration(value);
}
/**
* @brief Set avi file format
*
* @param value avi file format.
*
* @return function to set avi file format
*/
int SetAVIFormat(unsigned char value)
{
	return fSetAVIFormat(value);
}
/**
* @brief Set file format to save via FTP
*
* @param value file format.
*
* @return function to set file format
*/
int SetFTPFileFormat(unsigned char value)
{
	return fSetFTPFileFormat(value);
}
/**
* @brief Set file format to save into SD card
*
* @param value file format.
*
* @return function to set file format
*/
int SetSDFileFormat(unsigned char value)
{
	return fSetSDFileFormat(value);
}
/**
* @brief Set file format to save via SMTP
*
* @param value file format.
*
* @return function to set file format
*/
int SetAttFileFormat(unsigned char value)
{
	return fSetAttFileFormat(value);
}

/**
* @brief SMTP attatched file numbers
*
* @param value SMTP attatched file numbers
* @return function to set SMTP attatched file numbers
*/
int SetASmtpAttach(unsigned char value)
{
	return fSetASmtpAttach(value);
}

/**
* @brief enable record - upload via ftp
*
* @param value value  of enable record - upload via ftp
* @return function to set enable record - upload via ftp
*/
int SetRftpenable(unsigned char value)
{
	// While disabling FTP recording,if previous instance of FTP process is blocked
	// then kill the FTP process
	if(value == 0)
	{
	    if(gFileDispCount)
	    {
		    SYSTEM("killall -9 quftp\n");
		}
	}

	return fSetRftpenable(value);
}

/**
* @brief enable record - save into SD card
*
* @param value value of enable record - save into SD card
* @return function to set enable record - save into SD card
*/
int SetSdReEnable(unsigned char value)
{
	return fSetSdReEnable(value);
}

int SetVideoCodecType(unsigned char mode, unsigned char res)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	switch(mode)
	{
		case 9:
			pSysInfo->lan_config.codectype1=MPEG4_CODEC;
			pSysInfo->lan_config.codectype2=MJPEG_CODEC;
			pSysInfo->lan_config.codectype3=MPEG4_CODEC;
			break;
		case 8:
			pSysInfo->lan_config.codectype1=H264_CODEC;
			pSysInfo->lan_config.codectype2=MJPEG_CODEC;
			pSysInfo->lan_config.codectype3=H264_CODEC;
			break;
		case 7:
			pSysInfo->lan_config.codectype1=H264_CODEC;
			pSysInfo->lan_config.codectype2=MPEG4_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 6:
			pSysInfo->lan_config.codectype1=MPEG4_CODEC;
			pSysInfo->lan_config.codectype2=MPEG4_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 5:
			pSysInfo->lan_config.codectype1=H264_CODEC;
			pSysInfo->lan_config.codectype2=H264_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 4:
			pSysInfo->lan_config.codectype1=MPEG4_CODEC;
			pSysInfo->lan_config.codectype2=MJPEG_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 3:
			pSysInfo->lan_config.codectype1=H264_CODEC;
			pSysInfo->lan_config.codectype2=MJPEG_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 2:
			pSysInfo->lan_config.codectype1=(res&1)?MJPEG_CODEC:H264_CODEC; //if odd then JPEG
			pSysInfo->lan_config.codectype2=NO_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 1:
			pSysInfo->lan_config.codectype1=MPEG4_CODEC;
			pSysInfo->lan_config.codectype2=NO_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
		case 0:
		default:
			pSysInfo->lan_config.codectype1=H264_CODEC;
			pSysInfo->lan_config.codectype2=NO_CODEC;
			pSysInfo->lan_config.codectype3=NO_CODEC;
			break;
	}

	return 0;
}

//value 0:H.264 1:MPEG4 2:MJPEG
int SetVideoCodecMode(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nStreamType==0) //"SINGLESTREAM"
		value = value;
	if (pSysInfo->lan_config.nStreamType==1) //"DUALSTREAM"
		value = value+3;
	if (pSysInfo->lan_config.nStreamType==2) //"TRISTREAM"
		value = value+8;

	if(value != pSysInfo->lan_config.nVideocodecmode) {
		if ((ret = fSetVideoCodecMode(value)) == 0)
			SetRestart();
		return ret;
	}

	return 0;
}

/*
These three functions SetVideoMode, SetVideoCodecCombo and SetVideoCodecRes
will set nVideocodecmode and nVideocodecres varibale.
*/
int resmodecount=0;
#define RESMODE_MAX_VARIABLE (3)

void SetVideoCodecCfg(void)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	resmodecount = 0;

	SetVideoCodecMode(pSysInfo->lan_config.nVideocombo);

	SetVideoCodecType(pSysInfo->lan_config.nVideocodecmode, pSysInfo->lan_config.nVideocodecres);

	/* For codec mode not support JPEG ,change the format to AVI */
	if((pSysInfo->lan_config.codectype1 != MJPEG_CODEC)	&&
	   (pSysInfo->lan_config.codectype2 != MJPEG_CODEC)	&&
	   (pSysInfo->lan_config.codectype3 != MJPEG_CODEC))
	{
		SetFTPFileFormat(0);
		SetSDFileFormat(0);
		SetAttFileFormat(0);
	}

}

int SetVideoMode(unsigned char value)
{
	int ret = 0;
	resmodecount++;

	ret = fSetVideoMode(value);

	if(resmodecount == RESMODE_MAX_VARIABLE)
		SetVideoCodecCfg();

	return ret;
}

int SetVideoCodecCombo(unsigned char value)
{
	int ret = 0;
	resmodecount++;

	ret = fSetVideoCodecCombo(value);

	if(resmodecount == RESMODE_MAX_VARIABLE)
		SetVideoCodecCfg();

	return ret;
}

int SetVideoCodecRes(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	resmodecount++;

	if(value != pSysInfo->lan_config.nVideocodecres) {
		ret = fSetVideoCodecRes(value);
		SetRestart();
	}

	if(resmodecount == RESMODE_MAX_VARIABLE)
		SetVideoCodecCfg();

	return ret;
}

/**
* @brief Set image format
*
* @param value image format
* @return function to set image format
*/
int SetImageFormat(unsigned char value)
{
	return fSetImageFormat(value);
}

/**
* @brief Set resolution
*
* @param value resolution
* @return function to set resolution
*/
int SetResolution(unsigned char value)
{
	return fSetResolution(value);
}

/**
* @brief Set MPEG4 resolution
*
* @param value MPEG4 resolution
* @return function to set MPEG4 resolution
*/
int SetMPEG4Res(unsigned char value)
{
	return fSetMPEG4Res(value);
}

/**
* @brief Set schedule
*
* @param *buf schdule
* @param length buffer length
* @return function to set schedule
* @retval -1 failed to set schedule
*/
int SetSchedule(void * buf, int length)
{
	struct tm *pCurTime = schedGetCurTime();
	Schedule_t schedule;
	int index,bStatus,nDay,nSHour,nSMin,nSSec,nDHour,nDMin,nDSec;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(sscanf(buf, "%02d%1d%02d%02d%02d%02d%02d%02d%02d", &index, &bStatus,
			&nDay, &nSHour,	&nSMin, &nSSec,	&nDHour, &nDMin, &nDSec) != 9)
		return -1;

	schedule.bStatus = bStatus;
	schedule.nDay = nDay;
	schedule.tStart.nHour = nSHour;
	schedule.tStart.nMin = nSMin;
	schedule.tStart.nSec = nSSec;
	schedule.tDuration.nHour = nDHour;
	schedule.tDuration.nMin = nDMin;
	schedule.tDuration.nSec = nDSec;

	//printf("SCHEDULE DEBUG VALUE: index: %d status: %d day: %d hr: %d min: %d sec: %d hr: %d min: %d sec: %d\n",
	//	index, bStatus,	nDay, nSHour, nSMin, nSSec,	nDHour, nDMin, nDSec);

	//printf("SCHEDULE DEBUG VALUE: CurYear: %d CurMon: %d CurDay: %d\n",
	//	pCurTime->tm_year, pCurTime->tm_mon, pCurTime->tm_yday);

	return fSetSchedule(index, &schedule, pCurTime->tm_yday, pCurTime->tm_year);
}

/**
* @brief Set MPEG4-2 resolution
*
* @param value MPEG4-2 resolution
* @return function to set MPEG4-2 resolution
*/
int SetMPEG42Res(unsigned char value)
{
	return fSetMPEG42Res(value);
}

/**
* @brief Get dhcp config
*
* @return dhcp config information
* @retval NULL for failed to get dhcp config information
*/
int* GetDhcpCinfig()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.dhcp_config;
}

/**
* @brief Get UPNP port
*
* @return UPNP port number
* @retval NULL for failed to get UPNP port
*/
/**unsigned short* GetUpnpPort()
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return NULL;
	return &pSysInfo->lan_config.net.upnpport;
}*/


/**
* @brief delete schedule
*
* @param bEnable
* @return
*/
int DelSchedule(int bEnable)
{
	int i;
	Schedule_t schedule = {
		0, 1,
		{0, 0, 0},
		{0, 0, 0}
	};


	if(bEnable == 1){
		for(i = 0;i < SCHDULE_NUM;i++)
			if(fSetSchedule(i, &schedule, 0, 0))
				return -1;
	}

	return 0;
}

/**
* @brief Set lost ethernet alarm
*
* @param value lost ethernet alarm value
* @return function to set alarm enable - lost ethernet
*/
int SetLostAlarm(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nAlarmEnable==0)
		value = 0;

	return fSetLostAlarm(value);
}
/**
* @brief Set alarm enable - upload via SD card
*
* @param value alarm enable value
* @return function to set alarm enable - upload via SD card
*/
int SetSDAlarmEnable(unsigned char value)
{
	return fSetSDAlarmEnable(value);
}

/**
* @brief Set alarm enable - upload via ftp
*
* @param value alarm enable value
* @return function to set alarm enable - upload via ftp
*/
int SetFTPAlarmEnable(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	int ret;

	if(value == pSysInfo->lan_config.bAFtpEnable)
	{
	    return 0;
	}

	ret = fSetFTPAlarmEnable(value);

	// While disabling FTP recording,if previous instance of FTP process is blocked
	// then kill the FTP process
	if(value == 0)
	{
	    if(gFileDispCount)
	    {
		    SYSTEM("killall -9 quftp\n");
		}
	}

	return ret;
}

/**
* @brief Set alarm enable - upload via smtp
*
* @param value alarm enable value
* @return function to set alarm enable - upload via smtp
*/
int SetSMTPAlarmEnable(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	int ret;

	if(value == pSysInfo->lan_config.bASmtpEnable)
	{
	    return 0;
	}

	ret = fSetSMTPAlarmEnable(value);

	// While disabling SMTP recording,if previous instance of SMTP process is blocked
	// then kill the SMTP process
	if(value == 0)
	{
	    if(gFileDispCount)
	    {
		    SYSTEM("killall -9 esmtp\n");
		}
	}

	return ret;
}

/**
* @brief Set alarm duration
*
* @param value alarm duration
* @return function to set alarm duration
*/
int SetAlarmDuration(unsigned char value)
{
	return fSetAlarmDuration(value);
}

/**
* @brief .specific Convert charactor to integer
* @param *pChar
* @return unsigned char
*/
static unsigned char charToint(unsigned char *pChar)
{
	unsigned char ret;

	if( (*pChar>=0x30)&& (*pChar<=0x39) )
	{
		ret = (unsigned char)*pChar - 0x30;
		return ret;
	} else if( (*pChar>=0x41) && (*pChar<=0x46)  )
	{
		ret = (unsigned char)*pChar - 0x37;
		return ret;
	}
	return 0; /* to make compiler happy. */
}
/**
* @brief .Convert charactors to integer
* @param *pChar
* @param num
* @return int
*/
static int	NcharToInt(char *pChar, int num )
{
	int	ret = 0;
	int cnt =0;

	for( cnt = 0;cnt <num;cnt++ )
	{
		ret += (charToint((unsigned char *)(pChar+cnt))<<((num-cnt-1)*4));

	}

	return ret;
}

/**
* @brief Set motion detection on or off
*
* @param value status of motion detection
* @return function to set status of motion detection
*/

int motioncount = 0;
#define MOTION_MAX_VARIABLE (4)

void SetMotionDetectParam(void)
{
	ApproMotionPrm tMotionPrm;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	if(pSysInfo->lan_config.nVideocodecmode==2) //switch OFF for Megapixel Mode
		fSetMotionEnable(0);

	motioncount = 0;

	tMotionPrm.bMotionEnable = (int)pSysInfo->motion_config.motionenable;
	tMotionPrm.MotionBlock = NcharToInt((char *)pSysInfo->motion_config.motionblock, 3);
	tMotionPrm.MotionLevel = (int)pSysInfo->motion_config.motionlevel;
	tMotionPrm.bMotionCEnale = (int)pSysInfo->motion_config.motioncenable;
	tMotionPrm.MotionCValue = (int)pSysInfo->motion_config.motioncvalue;

	ApproSetMotion(&tMotionPrm);
}

int SetMotionDetect(unsigned char value)
{
	int ret;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nAlarmEnable==0)
		value = 0;

    if(value==0)
		RemoveSchedule(MOTION_SCHEDULE);

	ret = fSetMotionEnable(value);

	SetMotionDetectParam();

	return ret;
}

int SetMotionDetectBlock(char *pValue,int len)
{
	int ret = 0;

	motioncount++;

	ret = fSetMotionBlock(pValue, len);

	if(motioncount == MOTION_MAX_VARIABLE)
		SetMotionDetectParam();

	return ret;
}

int SetMotionDetLevel(unsigned char value)
{
	int ret = 0;

	motioncount++;

	ret = fSetMotionLevel(value);

	if(motioncount == MOTION_MAX_VARIABLE)
		SetMotionDetectParam();

	return ret;
}

int SetMotionDetCEnable(unsigned char value)
{
	int ret = 0;

	motioncount++;

	ret = fSetMotionCEnable(value);

	if(motioncount == MOTION_MAX_VARIABLE)
		SetMotionDetectParam();

	return ret;
}

int SetMotionDetCValue(unsigned char value)
{
	int ret = 0;

	motioncount++;

	value = CheckValueBound(value, 1, 100);

	ret = fSetMotionCValue(value);

	if(motioncount == MOTION_MAX_VARIABLE)
		SetMotionDetectParam();

	return ret;
}

/**
* @brief Set image setting to default
*
* @param value default enable
* @return
*/
int SetImageParaDefault(unsigned char value)
{
	if (value == 1)
	{
		SetCamWhiteBalance(WHITEBALANCE_DEFAULT);
		SetBinning(BINNING_DEFAULT);
		SetBlc(BACKLIGHT_CONTROL_DEFAULT);
		SetCamBacklight(BACKLIGHT_VALUE_DEFAULT);
		SetCamBrightness(BRIGHTNESS_DEFAULT);
		SetCamContrast(CONTRAST_DEFAULT);
		SetCamSaturation(SATURATION_DEFAULT);
		SetCamSharpness(SHARPNESS_DEFAULT);
		SetCamDayNight(DAY_NIGHT_DEFAULT);
	}
	else __E ("SysCtrl: SetImageDefault called with invalid param\n");
	return 0;
}
/**
* @brief Set GIO trigger alarm on or off.
*
* @param value GIO trigger status
*/
int SetGIOInEnable(unsigned char value)
{
	return fSetGIOInEnable(value);
}
/**
* @brief Set GIO trigger setting(high or low)
*
* @param value GIO trigger setting
*/
int SetGIOInType(unsigned char value)
{
	return fSetGIOInType(value);
}
/**
* @brief Set alarm trigger GIO on or off.
*
* @param value Alarm trigger status
*/
int SetGIOOutEnable(unsigned char value)
{
	return fSetGIOOutEnable(value);
}
/**
* @brief Set alarm trigger GIO setting(high or low).
*
* @param value Alarm trigger setting
*/
int SetGIOOutType(unsigned char value)
{
	if(value==0){
		printf("\n1SetGIOOutType=%d\n",value);
		ipnc_gio_write(GIO_ALARMOUT,1);
	}
	else{
		printf("\n2SetGIOOutType=%d\n",value);
		ipnc_gio_write(GIO_ALARMOUT,0);
	}

	return fSetGIOOutType(value);
}
/**
* @brief Add user
*
* @param *acout user account
* @retval 0: success
* @retval -1: failed to add user
*/
int AddUser(Acount_t *acout)
{
         int i;
        SysInfo *pSysInfo = GetSysInfo();
        Acount_t admin_acount = ADMIN_ACOUT;
        if(pSysInfo == NULL)
                return -1;
        for(i = 0;i < ACOUNT_NUM;i++){
                /* Skip exist and not matched acount */
                if(strlen(pSysInfo->acounts[i].user) &&
                                strcmp(acout->user, pSysInfo->acounts[i].user))
                        continue;
                if(i == 0){
                        /* For admin, only password can be changed. */
                        memcpy(&admin_acount.password, acout->password,
                                sizeof(admin_acount.password));
                        fSetAcount(0, &admin_acount);
                }else
                        fSetAcount(i, acout);

                return 0;
        }
        __E("Account max reached, add user fail\n");
        return -1;
}
/**
* @brief Delete user
*
* @param *strUserName User name
* @retval 0: success
* @retval -1: failed to delete user
*/
int DelUser(char *strUserName)
{
        int i, target_i = -1;
        Acount_t none_acount = NONE_ACOUT;
        SysInfo *pSysInfo = GetSysInfo();
        if(pSysInfo == NULL)
                return -1;
        /* Never delete admin */
        for(i = 1;i < ACOUNT_NUM;i++){
                if(strcmp(strUserName, pSysInfo->acounts[i].user))
                        continue;
                target_i = i;
                break;
        }
        if(target_i == -1)
                return -1;
        /* copy last acount to replace with target one */
        for(i = 1;i < ACOUNT_NUM && strlen(pSysInfo->acounts[i].user);i++);
        if(target_i != i - 1)
                fSetAcount(target_i, &pSysInfo->acounts[i - 1]);
        fSetAcount(i - 1, &none_acount);
        return 0;
}
/**
* @brief Check if user login
* @param nIndex index
* @param strId user ID
* @param pIpStr Login IP
* @retval 0 SUCCESS
*/
time_t aLastLoginTime[ACOUNT_NUM] = ACOUNT_TIME_DEFAULT;
int CheckLogin(int nIndex, char* strId, char *pIpStr)
{
	LogEntry_t tLog;
	time_t rawtime;
	time ( &rawtime );
	if((rawtime - aLastLoginTime[nIndex]) < LOGIN_TIMEOUT) {
		DBG("LOG DEBUG MSG: timediff: %d %d\n", (rawtime - aLastLoginTime[nIndex]), LOGIN_TIMEOUT);
		return 0;
	}
	aLastLoginTime[nIndex] = rawtime;
	sprintf(tLog.event, "%s login on %s", strId, pIpStr);
	memcpy(&tLog.time, localtime(&rawtime), sizeof(struct tm));
	return fSetSysLog(&tLog);
}
/**
* @brief User login
* @param *pPrm login data
* @retval -1 Error
* @retval 0 Success
*/
int DoLogin(login_data_t* pPrm)
{
	int i;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	for(i = 0;i < ACOUNT_NUM;i++){
		if(strcmp(pPrm->id, pSysInfo->acounts[i].user))
			continue;
		if(strcmp(pPrm->password, pSysInfo->acounts[i].password) == 0){
			CheckLogin(i, pPrm->id, pPrm->remote_ip);
			return 0;
		}
	}

	return -1;
}

/**
* @brief Get user authority
* @param *user user
* @retval -1 Error
* @return User authority
*/
int GetUserAuthority(char* user)
{
	int i;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	for(i = 0;i < ACOUNT_NUM;i++){
		if(strcmp(user, pSysInfo->acounts[i].user) == 0)
			return pSysInfo->acounts[i].authority;
	}
	return -1;
}



#define CONFIG_FILE "/tmp/NET.TXT"
/**
* @brief Write config data to file
*/
int WriteConfigFile(void)
{
	int ret = 0;
	FILE *fp;
	SysInfo *pSysInfo = GetSysInfo();
	//char stringbuffer[128];
	char *filename = CONFIG_FILE;
	if ((fp = fopen(filename, "wb")) == NULL) {
		__E("Can't create config file\n");
		ret = -1;
	} else {
		//fwrite(stringbuffer, 1, sizeof(stringbuffer), fp)
		struct in_addr ip;
		__u8 mac[6];
		ip.s_addr = net_get_ifaddr(ETH_NAME);
		fprintf(fp,"LocalIP=\"%s\"\n",inet_ntoa(ip));
		ip.s_addr = net_get_netmask(ETH_NAME);
		fprintf(fp,"NetMask=\"%s\"\n",inet_ntoa(ip));
		ip.s_addr = net_get_gateway();
		fprintf(fp,"Gateway=\"%s\"\n",inet_ntoa(ip));
		ip.s_addr = net_get_dns();
		fprintf(fp,"DNS=\"%s\"\n",inet_ntoa(ip));
		fprintf(fp,"DHCPConfig=%d\n",pSysInfo->lan_config.net.dhcp_config);
		fprintf(fp,"DHCPEnable=%d\n",pSysInfo->lan_config.net.dhcp_enable);
		fprintf(fp,"HTTPPort=%d\n",pSysInfo->lan_config.net.http_port);
		fprintf(fp,"Title=\"%s\"\n",pSysInfo->lan_config.title);
		//__u8 *mac = pSysInfo->lan_config.net.MAC;
		net_get_hwaddr(ETH_NAME, mac);
		fprintf(fp,"MAC=\"%02x:%02x:%02x:%02x:%02x:%02x\"\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		fprintf(fp,"PPPoEAccount=\"\"\n");
		fprintf(fp,"PPPoEPassword=\"\"\n");
		fprintf(fp,"PPPoEEnable=%d\n\n",pSysInfo->lan_config.net.servicepppoe);
		fclose(fp);
	}
	return ret;
}
/**
* @brief Get IP from file
*
* @param pLocalIp[OUT] where IP will return
* @param pFile[IN]	 Pointer to a opened file
* @return 0:SUCCESS	-1:FAIL
*/
int ReadLocalIp(struct in_addr* pLocalIp, FILE* pFile)
{
	const char sTarget[] = "LocalIP=\"";
	int i;
	char strIp[16], ch;
	unsigned long ip;
	for(i = 0;i < strlen(sTarget);i++)
		if(getc(pFile) != sTarget[i]){
			__E("File format error\n");
			return -1;
		}
	for(i = 0;i < sizeof(strIp);i++){
		if((ch = getc(pFile)) != '"')
			strIp[i] = ch;
		else {
			ch = getc(pFile);
			if(ch == '\n'){
				break;
			}else if(ch == '\r'){
				getc(pFile);
				getc(pFile);
				break;
			}else
				return -1;
		}
	}
	if(i == sizeof(strIp))
		return -1;
	/* parse ip */
	strIp[i] = '\0';
	ip = inet_addr(strIp);
	memcpy(pLocalIp, &ip, 4);
	return 0;
}
/**
* @brief Get NetMask from file
*
* @param pNetMask [O] where NetMask will return
* @param pFile [I] pointer to a opened file
* @retval 0:SUCCESS
* @retval -1:FAIL
*/
int ReadNetMask(struct in_addr* pNetMask, FILE* pFile)
{
	const char sTarget[] = "NetMask=\"";
	int i;
	char strIp[16], ch;
	unsigned long ip;
	for(i = 0;i < strlen(sTarget);i++)
		if(getc(pFile) != sTarget[i]){
			__E("File format error\n");
			return -1;
		}
	for(i = 0;i < sizeof(strIp);i++){
		if((ch = getc(pFile)) != '"')
			strIp[i] = ch;
		else {
			ch = getc(pFile);
			if(ch == '\n'){
				break;
			}else if(ch == '\r'){
				getc(pFile);
				getc(pFile);
				break;
			}else
				return -1;
		}
	}
	if(i == sizeof(strIp))
		return -1;
	/* parse ip */
	strIp[i] = '\0';
	ip = inet_addr(strIp);
	memcpy(pNetMask, &ip, 4);
	return 0;
}
/**
* @brief Get Gateway from file
*
* @param pGateway [O] where Gateway will return
* @param pFile [I] pointer to a opened file
* @retval 0:SUCCESS
* @retval -1:FAIL
*/
int ReadGateway(struct in_addr* pGateway, FILE* pFile)
{
	const char sTarget[] = "Gateway=\"";
	int i;
	char strIp[16], ch;
	unsigned long ip;
	for(i = 0;i < strlen(sTarget);i++)
		if(getc(pFile) != sTarget[i]){
			__E("File format error\n");
			return -1;
		}
	for(i = 0;i < sizeof(strIp);i++){
		if((ch = getc(pFile)) != '"')
			strIp[i] = ch;
		else {
			ch = getc(pFile);
			if(ch == '\n'){
				break;
			}else if(ch == '\r'){
				getc(pFile);
				getc(pFile);
				break;
			}else
				return -1;
		}
	}
	if(i == sizeof(strIp))
		return -1;
	/* parse ip */
	strIp[i] = '\0';
	ip = inet_addr(strIp);
	memcpy(pGateway, &ip, 4);
	return 0;
}
/**
* @brief Get DNS from file
* @param pDNS [O] where DNS will return
* @param pFile [I] pointer to a opened file
* @retval 0:SUCCESS
* @retval -1:FAIL
*/
int ReadDNS(struct in_addr* pDNS, FILE* pFile)
{
	const char sTarget[] = "DNS=\"";
	int i;
	char strIp[16], ch;
	unsigned long ip;
	for(i = 0;i < strlen(sTarget);i++)
		if(getc(pFile) != sTarget[i]){
			__E("File format error\n");
			return -1;
		}
	for(i = 0;i < sizeof(strIp);i++){
		if((ch = getc(pFile)) != '"')
			strIp[i] = ch;
		else {
			ch = getc(pFile);
			if(ch == '\n'){
				break;
			}else if(ch == '\r'){
				getc(pFile);
				getc(pFile);
				break;
			}else
				return -1;
		}
	}
	if(i == sizeof(strIp))
		return -1;
	/* parse ip */
	strIp[i] = '\0';
	ip = inet_addr(strIp);
	memcpy(pDNS, &ip, 4);
	return 0;
}
/**
* @brief Get new config from USB file and set it to system.
* @param pSysInfo [I] pointer to current system infomation
* @retval 0:SUCCESS
* @retval -1:FAIL
*/
int UpdateEnvFromFile(SysInfo* pSysInfo)
{
	int ret = 0;
	FILE *fp;
	char filename[] = USB_CFG_FILE;
	struct in_addr ip;
	struct in_addr netmask;
	struct in_addr gateway;
	struct in_addr dns;
	if(pSysInfo->lan_config.net.dhcp_enable){
		DBG("DHCP enable\n");
		return ret;
	}
	if ((fp = fopen(filename, "rb")) == NULL) {
		__E("Can't open config file\n");
		ret = -1;
	} else {
		if(ReadLocalIp(&ip, fp) == 0 && ReadNetMask(&netmask, fp) == 0 &&
				ReadGateway(&gateway, fp) == 0 && ReadDNS(&dns, fp) == 0){
			if(ip.s_addr != pSysInfo->lan_config.net.ip.s_addr){
				DBG("IP change\n");
				net_set_ifaddr(ETH_NAME, ip.s_addr);
				SetIp(ip.s_addr);
			}
			if(netmask.s_addr != pSysInfo->lan_config.net.netmask.s_addr){
				DBG("NetMask change\n");
				net_set_netmask(ETH_NAME, netmask.s_addr);
				SetNetMask(netmask.s_addr);
			}
			if(gateway.s_addr != pSysInfo->lan_config.net.gateway.s_addr){
				DBG("Gateway change\n");
				net_set_gateway(gateway.s_addr);
				SetGateway(gateway.s_addr);
			}
			if(dns.s_addr != pSysInfo->lan_config.net.dns.s_addr){
				DBG("DNS change\n");
				net_set_dns(inet_ntoa(dns));
				SetDns(dns.s_addr);
			}
			DBG("LocalIP=\"%s\"\n",inet_ntoa(pSysInfo->lan_config.net.ip));
			DBG("NetMask=\"%s\"\n",
				inet_ntoa(pSysInfo->lan_config.net.netmask));
			DBG("Gateway=\"%s\"\n",
				inet_ntoa(pSysInfo->lan_config.net.gateway));
			DBG("DNS=\"%s\"\n",inet_ntoa(pSysInfo->lan_config.net.dns));
		} else {
			__E("Unexcept error\n");
			ret = -1;
		}
		fclose(fp);
	}
	return ret;
}
void ScheduleStop()
{
	int i;
	SysInfo *pSysInfo = GetSysInfo();

	ResetSchedule();
	PauseSchedule();
	sleep(1);

    // stop DMVA event recording
	pSysInfo->dmva_config.sdUmountFlag = 1;

    for(i = 0;i < 20;i ++)
    {
	    if(pSysInfo->sdcard_config.schAviRun == 0)
	    {
	        break;
		}
	    else
	    {
			fprintf(stderr,"Waiting for recording to stop - %d\n",i);
	        sleep(1);
		}
	}

	fprintf(stderr,"Recording stopped\n");
}

void ScheduleResume()
{
	SysInfo *pSysInfo = GetSysInfo();

	// Enable DMVA event recording
    pSysInfo->dmva_config.sdUmountFlag = 0;
    ResumeSchedule();
    fprintf(stderr,"Resuming the recording\n");
}

int SetNetMulticast(unsigned char value)
{
	int ret = 0, nExtFunc = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	if(value != pSysInfo->lan_config.net.multicast_enable) {
		ret = fSetNetMulticast(value);
		SetRTSPStreamingOFF();
		nExtFunc = GetRTSPStreamOption();
		SetRTSPStreamingON(nExtFunc);
	}

	return ret;
}

int DoBootProc(int value)
{
	int ret = 0;
	switch(value){
		case 1:
			ret = BootProc1();
			break;
		case 2:
			ret = BootProc2();
			break;
		default:
			ret = -1;
			break;
	}
	return ret;
}

int SetHttpsPort(unsigned char value)
{
	return fSetHttpsPort(value);
}

int SetRS485Port(unsigned char value)
{
	return fSetRS485Port(value);
}

int SetAlarmEnable(unsigned char value)
{
     return fSetAlarmEnable(value);
}

int Avs_SetExtAlarm(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nAlarmEnable==0)
		value = 0;

     return fSetExtAlarm(value);
}

int Avs_SetAudioAlarm(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nAlarmEnable==0)
		value = 0;

	if(pSysInfo->audio_config.audioON==0)
		value = 0;

	if(pSysInfo->audio_config.audiomode==1)
		value = 0;

    if (value==0)
		RemoveSchedule(AUDIO_SCHEDULE);

    SetAudioAlarmFlag(value);

    return fSetAudioAlarm(value);
}

int Avs_SetAlarmAudioPlay(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->audio_config.audiomode==0)
		value = 0;
	if(pSysInfo->audio_config.audioON==0)
		value = 0;

     return fSetAlarmAudioPlay(value);
}

int Avs_SetAlarmAudioFile(unsigned char value)
{
     return fSetAlarmAudioFile(value);
}
int Avs_SetScheduleRepeatEnable(unsigned char value)
{
     return fSetScheduleRepeatEnable(value);
}

int Avs_SetScheduleNumWeeks(unsigned char value)
{
     return fSetScheduleNumWeeks(value);
}

int Avs_SetScheduleInfiniteEnable(unsigned char value)
{
     return fSetScheduleInfiniteEnable(value);
}

int SetAlarmStorage(unsigned char value)
{
     return fSetAlarmStorage(value);
}

int SetRecordStorage(unsigned char value)
{
     return fSetRecordStorage(value);
}

int SetSystemDefault(unsigned char value)
{
	SetDefault();
    return 0;
}

int SetSysDefaultDateTime(void)
{
	struct tm *tmnow;
	time_t tnow;
	static int timeboundReq = 1;

	if(timeboundReq == 0)
		return 0;

	time(&tnow);
	tmnow = localtime(&tnow);

	/* if Date is before Jan 1st, reset to Jan 1st */
	if((tmnow->tm_year<112)||((tmnow->tm_year==112)&&(tmnow->tm_mon<0)))
	{
		tmnow->tm_year 	= 112;
		tmnow->tm_mon 	= 0;
		tmnow->tm_mday 	= 1;
		tmnow->tm_hour 	= 0;
		tmnow->tm_min 	= 0;
		tmnow->tm_sec 	= 0;

		if ((tnow = mktime(tmnow)) < 0)
			return -1;

		stime(&tnow);
		SYSTEM("hwclock -uw");

		usleep(100000);
		printf("Date & Time Reset to Jan 1, 2012, 00:00:00AM \n");
	}

	return 0;
}

/* Add Media Profile */
int AddMediaProfile(media_profile *profile)
{
	int i=0;
	SysInfo* pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
	{
		return -1;
	}
	for(i=0;i<MAX_PROF;i++)
	{
		if((strlen(pSysInfo->Oprofile[i].profiletoken)!=0))
			continue;
		if(i == 0)
		{
			return -1;
		}
		else
		{
			fsetmediaprofile(i,profile);
		}
		return 0;
	}
	__E("Profile account max reached....failed to create new profile\n");
	return -1;
}

int SetImagingSettings(Imaging_Conf_tmp *imaging)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetimaging(imaging);
	return 0;
}
/* Add Video Source Configuration */
int Addvideosourceconfiguration(video_source_conf *videosourceconf)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddvideosourceconfiguration(i,videosourceconf);
	return 0;
}
/* Set Video Source Configuration */
int Setvideosourceconfiguration(video_source_conf *setvideosourceconf)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetvideosourceconfiguration(setvideosourceconf);
	return 0;
}
int Addmetadataconfiguration(Metadata_configuration_tmp *metadata)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddmetadataconfiguration(metadata);
	return 0;
}
int addaudiosourceconfiguration(Add_AudioSource_Configuration_tmp *addaudiosource)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddaudiosourceconfiguration(addaudiosource);
	return 0;
}
int SetMetadataConf(Metadata_configuration_tmp *metadata)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetmetadata(metadata);
	return 0;
}
int Setaudiosourceconfiguration(Add_AudioSource_Configuration_tmp *setaudiosource)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetaudiosourceconfiguration(setaudiosource);
	return 0;
}
int addaudioencoderconfiguration(Add_AudioEncoder_Configuration_tmp *addaudioencoder)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddaudioencoderconfiguration(addaudioencoder);
	return 0;
}
int Setaudioencoderconfiguration(Add_AudioEncoder_Configuration_tmp *setaudioencoder)
{
	int i=0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetaudioencoderconfiguration(setaudioencoder);
	return 0;
}
/* Add Video Encoder Configuratio */
int Addvideoencoderconfiguration(video_encoder_conf *videoencoderconf)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddvideoencoderconfiguration(videoencoderconf);
	return 0;
}
/* Set Video ENcoder Configuration */
int Setvideoencoderconfiguration(video_encoder_conf *setvideoencoderconf)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetvideoencoderconfiguration(setvideoencoderconf);
	return 0;
}

int Delvideoencoderconfiguration(video_encoder_conf *setvideoencoderconf)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fdelvideoencoderconfiguration(setvideoencoderconf);
	return 0;
}
/* Delete Profile */
int DeleteProfile(int index)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fdelprofile(index);
	return 0;
}
/** delete audio source */
int RemoveAudioSource(int index)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fremoveaudiosource(index);
	return 0;
}
int RemoveAudioEncoder(int index)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fremoveaudioencoder(index);
	return 0;
}
/* Set Discovery */
int SetDiscovery(unsigned char mode)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetdiscoverymode(mode);
	return 0;
}
/* Add Scopes */
int AddScope(Onvif_Scopes_t onvif_scopes_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	faddscopes(onvif_scopes_tmp);
	return 0;
}
/* Delete Scopes */
int DelScope(Onvif_Scopes_t onvif_scopes_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fdelscopes(onvif_scopes_tmp);
	return 0;
}
int SetAnalytics(video_analytics_conf video_analytics_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetvideoanalytics(video_analytics_tmp);
	return 0;
}
int SetVideoOutput(video_output_conf video_output_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetvideooutput(video_output_tmp);
	return 0;
}
int SetRelay(relay_conf relay_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetrelay(relay_tmp);
	return 0;
}
int SetRelaylogicalstate(relay_conf relay_tmp)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsetrelaylogicalstate(relay_tmp);
	return 0;
}


int set_hostname(hostname_tmp *host)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;
	fsethostname(host);
	return 0;
}

int syncOnvif(SysInfo *pSysInfo)
{
    if(pSysInfo == NULL)
        return NULL;

#if ONVIF_LOG
	//Resolution/Frame_rate/Encoding interval/bitrate/rate_Control/GOV length/
	//Brightness/Color saturation/Contrast/Sharpness/Backlight/
	printf("syncOnvif %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		pSysInfo->lan_config.title,				pSysInfo->lan_config.hostname,
		pSysInfo->lan_config.nWhiteBalance,		pSysInfo->lan_config.nDayNight,
		pSysInfo->lan_config.nBacklightControl,	pSysInfo->lan_config.nBackLight,
		pSysInfo->lan_config.nBrightness,		pSysInfo->lan_config.nContrast,
		pSysInfo->lan_config.nSaturation,		pSysInfo->lan_config.nSharpness,
		pSysInfo->lan_config.nStreamType,		pSysInfo->lan_config.nVideocodecmode,
		pSysInfo->lan_config.nVideocombo,		pSysInfo->lan_config.nVideocodecres,
		pSysInfo->lan_config.nRateControl,		pSysInfo->lan_config.nFrameRate1,
		pSysInfo->lan_config.nFrameRate2,		pSysInfo->lan_config.nFrameRate3,
		pSysInfo->lan_config.njpegquality,		pSysInfo->lan_config.nMpeg41bitrate,
		pSysInfo->lan_config.nMpeg42bitrate,	pSysInfo->lan_config.nRateControl1,
		pSysInfo->lan_config.nRateControl2,		pSysInfo->lan_config.codectype1,
		pSysInfo->lan_config.codectype2,		pSysInfo->lan_config.codectype3,
		pSysInfo->lan_config.frameRateVal1,		pSysInfo->lan_config.frameRateVal2,
		pSysInfo->lan_config.frameRateVal3);
#endif

	//Oprofile
	pSysInfo->nprofile = pSysInfo->lan_config.nStreamType + 1;
	printf("sycOnvif Profile %d\n",  pSysInfo->nprofile);

	// JPEG = 0, MPEG4 = 1, H264 = 2
	pSysInfo->Oprofile[0].AESC.Vencoder = 2 - pSysInfo->lan_config.nVideocombo;
	pSysInfo->Oprofile[0].AESC.frameratelimit = pSysInfo->lan_config.frameRateVal1/1000;
    pSysInfo->Oprofile[0].AESC.bitratelimit = pSysInfo->lan_config.nMpeg41bitrate;
    pSysInfo->Oprofile[0].AESC.encodinginterval = pSysInfo->lan_config.nRateControl1;

	strcpy(pSysInfo->Oprofile[1].profilename, "Default_Profile1");
	strcpy(pSysInfo->Oprofile[1].profiletoken, "Default_Token1");
	strcpy(pSysInfo->Oprofile[1].AESC.VEname, "DE_Name1");
	strcpy(pSysInfo->Oprofile[1].AESC.VEtoken, "DE_Token1");
	pSysInfo->Oprofile[1].AESC.Vencoder = 2 - pSysInfo->lan_config.codectype2;
	pSysInfo->Oprofile[1].AESC.Rwidth = pSysInfo->stream_config[1].width;
	pSysInfo->Oprofile[1].AESC.Rheight = pSysInfo->stream_config[1].height;
	pSysInfo->Oprofile[1].AESC.frameratelimit = pSysInfo->lan_config.frameRateVal2/1000;
	pSysInfo->Oprofile[1].AESC.bitratelimit = pSysInfo->lan_config.nMpeg42bitrate;//njpegquality
	pSysInfo->Oprofile[1].AESC.encodinginterval = pSysInfo->lan_config.nRateControl2;
	pSysInfo->Oprofile[1].AESC.Mpeggovlength = pSysInfo->codec_advconfig[1].ipRatio ;
	pSysInfo->Oprofile[1].AESC.Mpegprofile = 0;
	pSysInfo->Oprofile[1].AESC.H264govlength = pSysInfo->codec_advconfig[1].ipRatio ;
	pSysInfo->Oprofile[1].AESC.H264profile = 3;

	strcpy(pSysInfo->Oprofile[2].profilename, "Default_Profile2");
	strcpy(pSysInfo->Oprofile[2].profiletoken, "Default_Token2");
	strcpy(pSysInfo->Oprofile[2].AESC.VEname, "DE_Name2");
	strcpy(pSysInfo->Oprofile[2].AESC.VEtoken, "DE_Token2");
	pSysInfo->Oprofile[2].AESC.Vencoder = 2 - pSysInfo->lan_config.codectype3;
	pSysInfo->Oprofile[2].AESC.Rwidth = pSysInfo->stream_config[2].width;
	pSysInfo->Oprofile[2].AESC.Rheight = pSysInfo->stream_config[2].height;
	pSysInfo->Oprofile[2].AESC.frameratelimit = pSysInfo->lan_config.frameRateVal3/1000;
	pSysInfo->Oprofile[2].AESC.bitratelimit = pSysInfo->lan_config.nMpeg42bitrate;
	pSysInfo->Oprofile[2].AESC.encodinginterval = pSysInfo->lan_config.nRateControl2;
	pSysInfo->Oprofile[2].AESC.Mpeggovlength = pSysInfo->codec_advconfig[2].ipRatio ;
	pSysInfo->Oprofile[2].AESC.Mpegprofile = 0;
	pSysInfo->Oprofile[2].AESC.H264govlength = pSysInfo->codec_advconfig[2].ipRatio ;
	pSysInfo->Oprofile[2].AESC.H264profile = 3;

	pSysInfo->imaging_conf[0].Brightness = pSysInfo->lan_config.nBrightness;
	pSysInfo->imaging_conf[0].Saturation = pSysInfo->lan_config.nSaturation;
	pSysInfo->imaging_conf[0].Contrast = pSysInfo->lan_config.nContrast;
	pSysInfo->imaging_conf[0].Sharpness = pSysInfo->lan_config.nSharpness;
	pSysInfo->imaging_conf[0].BacklightCompensationMode = pSysInfo->lan_config.nBacklightControl;
	pSysInfo->imaging_conf[0].BackLightLevel = pSysInfo->lan_config.nBackLight;

	int i = 0;
	for( i = 0; i <=  pSysInfo->lan_config.nStreamType ; i++ )
	{
#if ONVIF_LOG
		printf("syscOnvif AESC[%d] %s %s %d %f %d %d %d %d %d %d %d %d %d %d\n",i,
			pSysInfo->Oprofile[i].AESC.VEname, pSysInfo->Oprofile[i].AESC.VEtoken,
			pSysInfo->Oprofile[i].AESC.VEusercount, pSysInfo->Oprofile[i].AESC.VEquality,
			pSysInfo->Oprofile[i].AESC.Vencoder, pSysInfo->Oprofile[i].AESC.Rwidth,
			pSysInfo->Oprofile[i].AESC.Rheight, pSysInfo->Oprofile[i].AESC.frameratelimit,
			pSysInfo->Oprofile[i].AESC.encodinginterval, pSysInfo->Oprofile[i].AESC.bitratelimit,
			pSysInfo->Oprofile[i].AESC.Mpeggovlength, pSysInfo->Oprofile[i].AESC.Mpegprofile,
			pSysInfo->Oprofile[i].AESC.H264govlength, pSysInfo->Oprofile[i].AESC.H264profile);

		printf("syscOnvif AVSC[%d] %s %d %s %s %d %d %d %d \n",i,
			pSysInfo->Oprofile[i].AVSC.Vname,		pSysInfo->Oprofile[i].AVSC.Vcount,
			pSysInfo->Oprofile[i].AVSC.Vtoken,		pSysInfo->Oprofile[i].AVSC.Vsourcetoken,
			pSysInfo->Oprofile[i].AVSC.windowx,	pSysInfo->Oprofile[i].AVSC.windowy,
			pSysInfo->Oprofile[i].AVSC.windowwidth,	pSysInfo->Oprofile[i].AVSC.windowheight);
#endif

		if(i != 0)
		{
			memcpy(&pSysInfo->Oprofile[i].AVSC , &pSysInfo->Oprofile[0].AVSC, sizeof(pSysInfo->Oprofile[0].AVSC));
		}
	}

#if ONVIF_LOG
	//imaging_conf
	printf("syscOnvif Imaging_conf: %s %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %f %f %f %d %f %d %f %d %f %f\n",
			pSysInfo->imaging_conf[0].Isourcetoken,		pSysInfo->imaging_conf[0].BacklightCompensationMode,
			pSysInfo->imaging_conf[0].BackLightLevel,	pSysInfo->imaging_conf[0].Brightness,
			pSysInfo->imaging_conf[0].Saturation,		pSysInfo->imaging_conf[0].Contrast,
			pSysInfo->imaging_conf[0].ExposureMode,		pSysInfo->imaging_conf[0].ExposurePriority,
			pSysInfo->imaging_conf[0].bottom,			pSysInfo->imaging_conf[0].top,
			pSysInfo->imaging_conf[0].right,			pSysInfo->imaging_conf[0].left,
			pSysInfo->imaging_conf[0].MinExposureTime,	pSysInfo->imaging_conf[0].MaxExposureTime,
			pSysInfo->imaging_conf[0].MinGain,			pSysInfo->imaging_conf[0].MaxGain,
			pSysInfo->imaging_conf[0].MinIris,			pSysInfo->imaging_conf[0].MaxIris,
			pSysInfo->imaging_conf[0].ExposureTime,		pSysInfo->imaging_conf[0].Gain,
			pSysInfo->imaging_conf[0].Iris,				pSysInfo->imaging_conf[0].AutoFocusMode,
			pSysInfo->imaging_conf[0].DefaultSpeed,		pSysInfo->imaging_conf[0].NearLimit,
			pSysInfo->imaging_conf[0].FarLimit,			pSysInfo->imaging_conf[0].IrCutFilterMode,
			pSysInfo->imaging_conf[0].Sharpness,		pSysInfo->imaging_conf[0].WideDynamicMode,
			pSysInfo->imaging_conf[0].WideDMLevel,		pSysInfo->imaging_conf[0].WhiteBalanceMode,
			pSysInfo->imaging_conf[0].CrGain,			pSysInfo->imaging_conf[0].CbGain
		 );

	printf("syscOnvif Imaging Conf: %s %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %f %f %f %d %f %d %f %d %f %f\n",
			pSysInfo->imaging_conf[1].Isourcetoken,		pSysInfo->imaging_conf[1].BacklightCompensationMode,
			pSysInfo->imaging_conf[1].BackLightLevel,	pSysInfo->imaging_conf[1].Brightness,
			pSysInfo->imaging_conf[1].Saturation,		pSysInfo->imaging_conf[1].Contrast,
			pSysInfo->imaging_conf[1].ExposureMode,		pSysInfo->imaging_conf[1].ExposurePriority,
			pSysInfo->imaging_conf[1].bottom,			pSysInfo->imaging_conf[1].top,
			pSysInfo->imaging_conf[1].right,			pSysInfo->imaging_conf[1].left,
			pSysInfo->imaging_conf[1].MinExposureTime,	pSysInfo->imaging_conf[1].MaxExposureTime,
			pSysInfo->imaging_conf[1].MinGain,			pSysInfo->imaging_conf[1].MaxGain,
			pSysInfo->imaging_conf[1].MinIris,			pSysInfo->imaging_conf[1].MaxIris,
			pSysInfo->imaging_conf[1].ExposureTime,		pSysInfo->imaging_conf[1].Gain,
			pSysInfo->imaging_conf[1].Iris,				pSysInfo->imaging_conf[1].AutoFocusMode,
			pSysInfo->imaging_conf[1].DefaultSpeed,		pSysInfo->imaging_conf[1].NearLimit,
			pSysInfo->imaging_conf[1].FarLimit,			pSysInfo->imaging_conf[1].IrCutFilterMode,
			pSysInfo->imaging_conf[1].Sharpness,		pSysInfo->imaging_conf[1].WideDynamicMode,
			pSysInfo->imaging_conf[1].WideDMLevel,		pSysInfo->imaging_conf[1].WhiteBalanceMode,
			pSysInfo->imaging_conf[1].CrGain,			pSysInfo->imaging_conf[1].CbGain
		 );
#endif


}

