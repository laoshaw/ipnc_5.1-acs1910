/* ===========================================================================
* @path $(IPNCPATH)\include
*
* @desc
* .
* Copyright (c) Appro Photoelectron Inc.  2008
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
*
* =========================================================================== */
#ifndef __SYSTEM_DEFAULT_H__
#define __SYSTEM_DEFAULT_H__
/* Macro */
#define IPV4(a,b,c,d)		((d<<24)|(c<<16)|(b<<8)|(a))

#define SYS_FILE			"/mnt/nand/sysenv.cfg"
#define SYS_DMVATZ_FILE     "/mnt/nand/sysenv_tz.cfg"
#define SYS_DMVAIMD_FILE    "/mnt/nand/sysenv_imd.cfg"
#define SYS_DMVAOC_FILE     "/mnt/nand/sysenv_oc.cfg"
#define SYS_DMVASMETA_FILE  "/mnt/nand/sysenv_smeta.cfg"
#define LOG_FILE			"/mnt/nand/logfile.log"
#define DMVA_LOG_FILE		"/mnt/nand/dmvalogfile.log"
#define SYS_LOG_FILE 		"/mnt/nand/systemlog.log"
#define ACCESS_LOG_FILE 	"/mnt/nand/accesslog.log"
#define SUPPORT_REPORT 	    "/var/www/SupportReport.zip"
#define SD_PATH				"/mnt/mmc/ipnc"
#define TEMP_FOLDER			"/tmp"

#define TITLE_DEFAULT				"TI_IPNC"
#define HOST_DEFAULT                "IPNC-RDK"
#define MODELNAME_DEFAULT			"IPNC_RDK_DM36x"
#define IP_DEFAULT					IPV4(192,168,  8,81)
#define NETMASK_DEFAULT				IPV4(255,255,255,  0)
#define GATEWAY_DEAFULT				IPV4(192,168, 8,1)
#define DNS_DEFAULT					IPV4(192,168, 8,1)
#define HTTP_PORT_DEFAULT			80
#define HTTPS_PORT_DEFAULT			81
#define DHCP_ENABLE_DEFAULT			0
#define NTP_SERVER_DEFAULT			"192.168.1.168"
#define NTP_TIMEZONE_DEFAULT		20
#define NTP_FREQUENCY_DEFAULT		0
#define TIME_FORMAT_DEFAULT			0
#define DAYLIGHT_DEFAULT			0
#define DEVICE_TYPE_DEFAULT			0x1687
#define MAC_DEFAULT					{0x12,0x34,0x56,0x78,0x9a,0xbc}
#define FTP_SERVER_IP_DEFAULT		"192.168.1.1"
#define FTP_USERNAME_DEFAULT		"ftpuser"
#define FTP_PASSWORD_DEFAULT		"9999"
#define FTP_FOLDERNAME_DEFAULT		"default_folder"
#define FTP_IMAGEAMOUNT_DEFAULT 	5
#define FTP_PID_DEFAULT				0
#define SMTP_SERVER_IP_DEFAULT		"192.168.1.1"
#define SMTP_SERVER_PORT_DEFAULT	25
#define SMTP_AUTHENTICATION_DEFAULT	0
#define SMTP_USERNAME_DEFAULT		"smtpuser"
#define SMTP_PASSWORD_DEFAULT		"9999"
#define SMTP_SNEDER_EMAIL_DEFAULT	"user@domain.com"
#define SMTP_RECEIVER_EMAIL_DEFAULT	"user@domain.com"
#define SMTP_CC_DEFAULT				""
#define SMTP_SUBJECT_DEFAULT		"TI"
#define SMTP_TEXT_DEFAULT			"ti mail."
#define SMTP_ATTACHMENTS_DEFAULT	2
#define SMTP_VIEW_DEFAULT			0
#define MPEG4_RESOLUTION_DEF		0
#define MPEG4_RESOLUTION2_DEF		0
#define LIVE_RESOLUTION_DEF			0
#define MPEG4QUALITY_DEF			0
#define JPEGQUALITY_DEF				80
#define MIRROR_DEF					0
#define ENCRYPT_DEF					0
#define M41SFEAT_DEF 				0
#define M42SFEAT_DEF				0
#define JPGSFEAT_DEF				0
#define FDETECT_DEF 				0
#define REGUSR_DEF					"User1"
#define DEMOCFG_DEF					0
#define	OSDSTREAM_DEF				0
#define	OSDWINNUM_DEF				0
#define	OSDWIN_DEF					0
#define	HISTOGRAM_DEF				0
#define	VS_DEFAULT					0
#define	LDC_DEFAULT					0
#define	VNF_DEFAULT					0
#define	VNF_MODE_DEFAULT			2
#define	VNF_STRENGTH_DEFAULT		0
#define DYNRANGE_DEF				0
#define DYNRANGESTR_DEF				1
#define	OSDTEXT_DEF					"IPNC-RDK"
#define	DUMMY_DEF					0
#define FTP_PORT_DEFAULT			21
#define AEW_SWITCH_DEFAULT			2
#define AEW_TYPE_DEFAULT			3
#define SUPPORTMPEG4_DEFAULT		1
#define IMAGEFORMAT_DEFAULT			1
#define IMAGESOURCE_DEFAULT			0
#define MAX_FRAMERATE_DEFAULT		30000
#define DEFAULTSTORAGE_DEFAULT		1
#define DEFAULTCARDGETHTM_DEFAULT	"sdget.htm"
#define BRANDURL_DEFAULT			"www.ti.com"
#define BRANDNAME_DEFAULT			"TEXAS INSTRUMENTS INC"
#define SUPPORTTSTAMP_DEFAULT		1
#define	MPEG41XSIZE_DEFAULT			1280
#define	MPEG41YSIZE_DEFAULT			720
#define	MPEG42XSIZE_DEFAULT			352
#define	MPEG42YSIZE_DEFAULT			192
#define JPEGXSIZE_DEFAULT			640
#define JPEGYSIZE_DEFAULT			352
#define SUPPORTMOTION_DEFAULT		2
#define SUPPORTWIRELESS_DEFAULT		0
#define SERVICEFTPCLIENT_DEFAULT	1
#define SERVICESMTPCLIENT_DEFAULT	1
#define SERVICEPPPOE_DEFAULT		0
#define SERVICESNTPCLIENT_DEFAULT	1
#define SERVICEDDNSCLIENT_DEFAULT	0
#define SUPPORTMASKAREA_DEFAULT		0
#define MAXCHANNEL_DEFAULT			1
#define SUPPORTRS485_DEFAULT		0
#define SUPPORTRS232_DEFAULT		1
#define LAYOUTNUM_DEFAULT			0
#define SUPPORTMUI_DEFAULT			0
#define MUI_DEFAULT					0
#define SUPPORTSEQUENCE_DEFAULT		0
#define QUADMODESELECT_DEFAULT		-1
#define SERVICEIPFILTER_DEFAULT		0
#define OEMFLAG0_DEFAULT			1
#define	SUPPORTDNCONTROL_DEFAULT	0
#define SUPPORTAVC_DEFAULT			0
#define SUPPORTAUDIO_DEFAULT		1
#define SUPPORTPTZPAGE_DEFAULT		1
#define WHITEBALANCE_DEFAULT		0	/* auto white balance */
#define DAY_NIGHT_DEFAULT			1	/* Default: Day mode */
#define LOCALDISPLAY_DEFAULT		0
#define BINNING_DEFAULT				0
#define BACKLIGHT_CONTROL_DEFAULT	1
#define BACKLIGHT_VALUE_DEFAULT		1
#define BRIGHTNESS_DEFAULT			128
#define CONTRAST_DEFAULT			128
#define SATURATION_DEFAULT			128
#define SHARPNESS_DEFAULT			128
#define STREAMTYPE_DEFAULT			0
#define VCODECMODE_DEFAULT			0
#define VCODECCOMBO_DEFAULT			0
#define VCODECRES_DEFAULT			0
#define SUPPORTSTREAM1_DEFAULT		0
#define SUPPORTSTREAM2_DEFAULT		0
#define SUPPORTSTREAM3_DEFAULT		0
#define SUPPORTSTREAM4_DEFAULT		0
#define SUPPORTSTREAM5_DEFAULT		1
#define SUPPORTSTREAM6_DEFAULT		0
#define RATECONTROL_DEFAULT			1		/* Set to CVBR */
#define FRAMERATE1_DEFAULT			0
#define FRAMERATE2_DEFAULT			0
#define FRAMERATE3_DEFAULT			0
#define MPEG41BITRATE_DEFAULT		4000000
#define MPEG42BITRATE_DEFAULT		512000
#define DHCP_CONFIG_DEFAULT			0
#define DISCOVERY_MODE              0
#define NO_SCHEDULE					{0,1,{0,0,0},{0,0,0}}
#define SCHEDULE_DEFAULT			{NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE,NO_SCHEDULE}
#define LOSTALARM_DEFAULT			0
#define SDAENABLE_DEFAULT			0
#define ALARMDURATION_DEFAULT		1
#define AFTPENABLE_DEFAULT			0
#define ASMTPENABLE_DEFAULT			0
#define AVIDURATION_DEFAULT			0
#define AVIFORMAT_DEFAULT			0
#define FTPFILEFORMAT_DEFAULT		0
#define SDFILEFORMAT_DEFAULT		0
#define ATTFILEFORMAT_DEFAULT		0

#define ASMTPATTACH_DEFAULT			0
#define RFTPENABLE_DEFAULT			0
#define SD_INSERT_DEFAULT			0
#define SDRENABLE_DEFAULT			0
#define AUDIOENABLE_DEFAULT			0
#define MOTIONENABLE_DEFAULT		0
#define MOTIONCENABLE_DEFAULT		0
#define MOTIONLEVEL_DEFAULT			1
#define MOTIONCVALUE_DEFAULT		50
#define MOTIONBLOCK_DEFAULT			"000"

#define PTZZOOMIN_DEFAULT			"0"	//"zoomin"
#define PTZZOOMOUT_DEFAULT			"0"	//"zoomout"
#define PTZPANUP_DEFAULT			"0"	//"up"
#define PTZPANDOWN_DEFAULT			"0"	//"down"
#define PTZPANLEFT_DEFAULT			"0"	//"left"
#define PTZPANRIGHT_DEFAULT			"0"	//"right"

//GIO
#define	GIOINENABLE_DEFAULT		0
#define	GIOINTYPE_DEFAULT		0
#define	GIOOUTENABLE_DEFAULT	0
#define	GIOOUTTYPE_DEFAULT		0

#define DATEFORMAT_DEFAULT 			2
#define TSTAMPENABLE_DEFAULT 		0
#define TSTAMPFORMAT_DEFAULT 		1
#define AUDIOINVOLUME_DEFAULT 		2
#define ALARMSTATUS_DEFAULT			0
#define MULTICAST_ENABLE_DEFAULT	0

/* authority AUTHORITY_NONE = 9 */
#define NONE_ACOUT				{"", "", 9}
#define ADMIN_ACOUT				{"admin", "9999", 0}
#define ACOUNT_DEFAULT			{ADMIN_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, \
							 	NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT, NONE_ACOUT}
#define ACOUNT_TIME_DEFAULT		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define CUR_LOG_DEFAULT			{"", {0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define CUR_DMVA_LOG_DEFAULT	{"", "", "",0,0,{0, 0, 0, 0, 0, 0, 0, 0, 0}}

/* Onvif */
#define COM_PROF                        "", "", 0
#define VSC_PROF                        "", 0, "", "", 0, 0, 0, 0
#define ASC_PROF                        "", 0, "", ""
#define VEC_PROF                        "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 720000
#define MULC_PROF                       0, "", "", 0, 0, 720000
#define AEC_PROF                        "", "", 0, 0, 0, 0
#define VAC_PROF                        "", "", 0,
#define VAENGINE                        0, 0, "", "", 0, "", "", ""
#define VRENGINE                        0, 0, "", "", 0, "", "", ""
#define VOC_PROF			"",0,""
#define PTZ_PROF                        "", "", 0, "", "", "", "", "", "", "", 0, 0, "", 0, "", 0, "", 0, 0, 0, 0, "", "", "", 0, 0, 0, 0
#define EXT_PROF                        "", 0, "", "", "", 0, "", 0, ""
#define MDC_PROF			"" , 0, "", 0, 720000
#define NON_PROF                        {COM_PROF,{VSC_PROF},{ASC_PROF},{VEC_PROF,{MULC_PROF}},{AEC_PROF},{VAC_PROF{VAENGINE},{VRENGINE}},{VOC_PROF},PTZ_PROF,EXT_PROF,\
					{MDC_PROF,{MULC_PROF}}}
#define COM_PROFD                        "Default_Profile", "Default_Token", 1
#define VSC_PROFD                        "DVS_Name", 1, "DVS_Token", "DVS_SourceToken", 0, 0, 1280, 720
#define ASC_PROFD                        "G711", 1, "G711", "G711"
#define VEC_PROFD                        "DE_Name", "DE_Token", 1, 100, 2, 1280, 720, 30, 1, 5000, 30, 0, 30, 3, 720000
#define MULC_PROFD                       0, "", "", 0, 0, 720000
#define AEC_PROFD                        "DAE_Name", "G711", 0, 0, 0, 0
#define VAC_PROFD                        "video_analytics", "video_analytics", 1
#define VAENGINED                         0, 0, "", "", 0, "", "", ""
#define VRENGINED                         0, 0, "", "", 0, "", "", ""
#define VOC_PROFD			 "video_output", 1, "video_output"
#define PTZ_PROFD                        "", "", 0, "", "", "", "", "", "", "", 0, 0, "", 0, "", 0, "", 0, 0, 0, 0, "", "", "", 0, 0, 0, 0
#define EXT_PROFD                        "", 0, "", "", "", 0, "", 0, ""
#define MDC_PROFD			"MD_name" , 1, "MD_token", 0, 720000
#define DEFAULT_PROF                     {COM_PROFD,{VSC_PROFD},{ASC_PROFD},{VEC_PROFD,{MULC_PROFD}},{AEC_PROFD},{VAC_PROFD,{VAENGINED},{VRENGINED}},{VOC_PROFD},\
					PTZ_PROFD,EXT_PROFD,{MDC_PROFD,{MULC_PROFD}}}
#define ONVIF_PROFILE                   {DEFAULT_PROF, NON_PROF, NON_PROF, NON_PROF}
#define ONVIF_SCOPES			{{"onvif://www.onvif.org/type/NetworkVideoTransmitter"},"onvif://www.onvif.org/name/DM36x_IPNC",{"onvif://www.onvif.org/location"},"onvif://www.onvif.org/hardware/00E00ADD0000"}
#define RELAYS_PROFD			0 ,{"alarm", 0, 0, 0}
#define RELAY_NON_PROF			0 ,{"", 0, 0, 0}
#define RELAYS_CONFIG			{RELAYS_PROFD, RELAY_NON_PROF, RELAY_NON_PROF, RELAY_NON_PROF}
#define IMAGING_CONFD			"DVS_SourceToken" ,0, 1, 128, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 1, 0, 50, 50
#define IMAGING_CONFG			"" ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define IMAGING_CONF			{IMAGING_CONFD,IMAGING_CONFG,IMAGING_CONFG,IMAGING_CONFG}
/* End-Onvif */
#endif   /* __SYSTEM_DEFAULT_H__ */
