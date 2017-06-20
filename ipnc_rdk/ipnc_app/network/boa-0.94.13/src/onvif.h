#ifndef onvif_H
#define onvif_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipnc_ver.h"
#include "onvifH.h"

void midprint_soap(struct soap *soap);
void reboot_ipcam_onvif(void);
struct SOAP_ENV__Fault onvif_send_fault(char *sender,char *first,char *second);
void ByeHandler(int);
int udp_hello(int *);
int udp_probe(int *);

#define HOSTNAMELENGTH 80
#define NUM_LOG_PER_PAGE	20
#define SIZE_ALLOC      100
#define ALL  0
#define ANALYTICS 1
#define DEVICE 2
#define EVENTS 3
#define IMAGING 4
#define MEDIA 5
#define PANTILTZOOM 6
#define TRUE 1
#define FALSE 0
#define NUM_IN_CONEC 0
#define NUM_RELAY_CONEC 0
#define MAJOR 1
#define MINOR 0
#define DISCOVERABLE 0
#define NON-DISCOVERABLE 1
#define HTTP 0
#define HTTPS 1
#define RTSP 2
#define IPV4_NTP 0
#define IPV6_NTP 1
#define DNS_NTP 2
#define DATE_MANUAL 0
#define DATE_NTP 1
#define DEFAULT_VALUE 1
#define DEFAULT_SIZE 1
#define NUM_PROFILE 4
#define ANALYTICS_ENABLE 1
#define PTZ_ENABLE 1
#define METADATA_ENABLE 1
#define EXIST 1
#define NOT_EXIST 0
#define SUCCESS 0
#define DEFAULT_ENCODER_PROF {0,"", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "", "", 0, 0, 0}
#define DEFAULT_SOURCE_PROF {0,"", 0, "", "", 0, 0, 0, 0}
#define ETH_NAME "eth0"

#define COMMAND_LENGTH 40
#define MID_INFO_LENGTH 40
#define SMALL_INFO_LENGTH 20
#define IP_LENGTH 20
#define INFO_LENGTH 100
#define LARGE_INFO_LENGTH 1024
#define TERMINATE_THREAD 0
#define LINK_LOCAL_ADDR "169.254.166.174"
#define MACH_ADDR_LENGTH 6
#define DEFAULT_SCOPE "onvif://www.onvif.org"
#define ETH_NAME_LOCAL "eth0:avahi"

#define YEAR_IN_SEC (31556926)
#define MONTH_IN_SEC (2629744)
#define DAY_IN_SEC (86400)
#define HOURS_IN_SEC (3600)
#define MIN_IN_SEC (60)
#define MAX_EVENTS (3)
#define KEY_NOTIFY_SEM (1729)
#define MOTION_DETECTED_MASK (1) 	
#define NO_MOTION_MASK (2)	

#endif

