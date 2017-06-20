#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <onvif.h>
#include <sys_env_type.h>
#include <sysctrl.h>
#include "para_list.h"
#include <web_translate.h>
#include <system_default.h>
#include <file_list.h>
#include "DeviceBinding.nsmap"
#include "onvifH.h"
#include <sys/sem.h>
#include <sys/types.h>
#include "onvifface.h"
#include "onvifStub.h"
#include <pthread.h>

int _true = 1;
int _false = 0;
int tempdhcp_flag;
int tempdnssearchdomainsize;
int temprtspportsize = 5;
char domainname[MID_INFO_LENGTH] = "ipnc";
extern char *TZname[];
long t_out; /*global variable for termination time*/
long time_out; /*global variable for termination time*/
int canSubscribe_pullmessages = 1;
char eventstatus[3] = "";
extern int midhnewsock;
extern struct SOCKADDR onvif_remote_addr;
int isValidIp4 (char *str);
int startonvif(struct soap*);
unsigned long iptol(char *ip);
int canSubscribe = 1;
int _DateTimeType = DATE_MANUAL;
typedef struct
{
	char *user_id;
	char *password;
	int authority;
}add_user;

/**
 * @brief explodeitem
 * @This function splits the given string based on a delimiter.
    'list' is an array which holds the split strings
  */

void explodeitem(char *in, char dl, char list[][100])
{
	int i = 0;
	int j = 0;
	int k = 0;
	int len = strlen(in);
	for(i = 0; i < len; i++)
	{
		if(in[i] == dl)
		{

			*( *(list + j) + k) = 0;
			j++;
			k = 0;
		}
		else
		{
			*( *(list + j) + k) = in[i];
			k++;
		}
	}
	*(*(list + j) + k) = 0;
	j++;
	**(list + j) = 0;
}

/**
 * @brief notify_thr
 * @This thread sends the notify message when an event occurs.
 * (This thread is initialized for every 'Subscribe' request)
 * Operations Done
 *	1. Loop till 'Termination Time' (Sent as a part of Subscription Request)
 *	2. Intialize appropriate event handling mechanism
 * 	3. Wait for events
 *	4. Send Notify Packet whenever an event Occurs
 */
void *notify_thr(void *arg)
{
	/* soap structure */
	struct soap *soap_notify;
	int sem_value = 0;
	union semun{
                int val;
                struct semid_ds *buf;
                short *array;
        }argument;
	int id;
	time_t current;
	time_t time_tm;
	int value = 1;
	int ret;
	int temp_sock; /* DUP variable */
	canSubscribe = 0;

	/* Saving current Time */
	current = time(NULL);

	/* Get semaphore descriptor */
	id = semget(KEY_NOTIFY_SEM, 1, 0666);
	if(id < 0)
	{
		ONVIF_DBG( "Unable to obtain semaphore.");
        }

	/* Initialize Motion Detection */
	ret = ControlSystemData(SFIELD_SET_MOTIONENABLE, (void *)&value, sizeof(value));
	if (ret < 0)
	{
		ONVIF_DBG("Failed to enable motion.");
	}

	/* Ignore any previous motion */
	argument.val = 0;
	semctl(id, 0, SETVAL, argument);

	/* TODO:: Send Initialization Notification */

	temp_sock = *((int *)arg);
	/* soap initialization */
	soap_notify = soap_new();
	soap_init(soap_notify);

	/* soap binding */
	soap_notify->master = temp_sock;
	soap_notify->socket = temp_sock;
	soap_notify->errmode = 0;
	soap_notify->bind_flags = 1;
	soap_notify->error = SOAP_OK;
	soap_notify->keep_alive = 0;
	memset((void *)&soap_notify->peer, 0, sizeof(soap_notify->peer));
	soap_notify->peer = onvif_remote_addr;

	if (soap_valid_socket(soap_notify->socket))
	{
		soap_notify->ip = ntohl(soap_notify->peer.sin_addr.s_addr);
		soap_notify->port = (int)ntohs(soap_notify->peer.sin_port);
		soap_notify->keep_alive = (((soap_notify->imode | soap_notify->omode) & SOAP_IO_KEEPALIVE) != 0);
	}
	/* Loop till Termination Time */
	while(t_out > 0)
	{
		/* semaphore get value */
		/* Semaphore value set in proc_alarm.c when motion is detected */
		sem_value = semctl(id, 0, GETVAL, 0);

		/* Check if any motion is detected */
		if(sem_value & MOTION_DETECTED_MASK)
		{
			/* Send Change Notification */
			ret = soap_serve___ns6__Notify(soap_notify);
			argument.val = 0;
			semctl(id, 0, SETVAL, argument);
		}
		else if(sem_value & NO_MOTION_MASK)
		{
			argument.val = 0;
			semctl(id, 0, SETVAL, argument);
			/* TODO:: Send Delete Notification */
		}

	       	time_tm = time(NULL);
		if((time_tm - current) > t_out)
		{
			t_out = 0;
		}
	}
	soap_dealloc(soap_notify, NULL);
	soap_destroy(soap_notify);
	soap_end(soap_notify);
	free(soap_notify);
	canSubscribe = 1;
	pthread_exit(NULL);
}


/**
 * @brief eventtracker_thr
 * @This thread records the events within the specified termination time.
 * (This thread is initialized for every 'CreatePullPointSubscription' request)
 * Operations Done
 *	1. Loop till 'Termination Time' (Sent as a part of Subscription Request)
 *	2. Intialize appropriate event handling mechanism
 * 	3. Wait for events
 *	4. Send Notify Packet whenever an event Occurs
 */

void *eventtracker_thr(void *arg)
{
	int sem_value = 0;
	union semun{
                int val;
                struct semid_ds *buf;
                short *array;
        }argument;
	int id;
	time_t current;
	time_t time_tm;
	int value = 1;
	int ret;

	time_out = *(long *) arg;
	canSubscribe_pullmessages = 0;

	/* Saving current Time */
	current = time(NULL);

	/* Get semaphore descriptor */
	id = semget(KEY_NOTIFY_SEM, 1, 0666);
	if(id < 0)
	{
		ONVIF_DBG( "Unable to obtain semaphore.");
        }

	/* Initialize Motion Detection */
	ret = ControlSystemData(SFIELD_SET_MOTIONENABLE, (void *)&value, sizeof(value));
	if (ret < 0)
	{
		//ONVIF_DBG("Failed to enable motion.");
	}

	/* Ignore any previous motion */
	argument.val = 0;
	semctl(id, 0, SETVAL, argument);

	/* TODO:: Send Initialization Notification */

	/* Loop till Termination Time */
	while(time_out > 0)
	{
		/* semaphore get value */
		/* Semaphore value set in proc_alarm.c when motion is detected */
		sem_value = semctl(id, 0, GETVAL, 0);
		/* Check if any motion is detected */
		if(sem_value & MOTION_DETECTED_MASK)
		{
			argument.val = 0;
			semctl(id, 0, SETVAL, argument);
			eventstatus[0] |= MOTION_DETECTED_MASK;
		}
		else if(sem_value & NO_MOTION_MASK)
		{
			argument.val = 0;
			semctl(id, 0, SETVAL, argument);
		}

	       	time_tm = time(NULL);
		if((time_tm - current) > time_out)
		{
			time_out = 0;
		}
	}
	canSubscribe_pullmessages = 1;
	pthread_exit(NULL);
	return NULL;
}

/* @brief Parse the scope URI and return the scope parameter */
char *strparser(char *str)
{
	int i = 0;
	int j = 0;
	char *retstr = malloc(sizeof(char) * INFO_LENGTH);
	while(i < 3)
	{
		if(*str == '/')
		{
			i++;
		}
		str++;
	}
	while(*str != '/' && *str != '\0')
	{
		*(retstr + j) = *str;
		j++;
		str++;
	}
	*(retstr + j) = '\0';
	if(((strcmp(retstr, "type") == 0)) || (strcmp(retstr, "name") == 0) || (strcmp(retstr, "location") == 0) || (strcmp(retstr, "hardware") == 0))
	{
		return retstr;
	}
	else
	{
		return NULL;
	}
}

/*
 *  @brief Parsing the string for Motion and Facedetection parameters
 *  @return Parameters of Motion and FD parameters
 */
char *anyparser(char *str)
{
	char *retstr = malloc(sizeof(char)* INFO_LENGTH);
	while(*str != '>')
	{
                str++;
	}
        if(strstr(str,"tns1:VideoAnalytics/tns1:MotionDetection/") != NULL ||  strstr(str,"tns1:VideoAnalytics/tns1:FaceDetection/") != NULL)
        {
                return str;
        }
	else if(strstr(str,"@Name") != NULL)
	{
		str = strstr(str,"@Name=");
		while(*str != '"')
		{
			str ++;

		}
		str++;
		int i=0;
		while(*str != ']')
		{
			*(retstr+i) = *str;
			str ++;
			i++;
		}
		*(retstr + (--i)) = '\0';
		return retstr;
	}
        else
	{
		return NULL;
	}

}

long epoch_convert_switch(int value, char convert, int time)
{
	long seconds = 0;
	switch(convert)
	{
		case 'Y': seconds = value * YEAR_IN_SEC ;
			  break;
		case 'M':
			  if(time == 1)
			  {
				  seconds = value * MIN_IN_SEC;
			  }
			  else
			  {
				  seconds = value * MONTH_IN_SEC;
			  }
			  break;
		case 'D': seconds = value * DAY_IN_SEC;
			  break;
		case 'H': seconds = value * HOURS_IN_SEC;
			  break;
		case 'S': seconds = value;
			  break;
	}
	return seconds;
}

/*
 *  @brief Converts XML period format (PnYnMnDTnHnMnS) to long data type
 */

long periodtol(char *ptr)
{
	char buff[10] = "0";
	char convert;
	int i = 0;
	int value = 0;
	int time = 0;
	int minus = 0;
	long cumulative = 0;
	if(*ptr == '-')
	{
		ptr++;
		minus = 1;
	}
	while(*ptr != '\0')
	{
		if(*ptr != 'P' || *ptr != 'T' || *ptr >= '0' || *ptr <= '9')
		{
			return -1;
		}
		if(*ptr == 'P' || *ptr == 'T')
		{
			ptr++;
			if(*ptr == 'T')
			{
				time = 1;
				ptr++;
			}
		}
		else
		{
			if(*ptr >= '0' && *ptr <= '9')
			{
				buff[i] = *ptr;
				i++;
				ptr++;
			}
			else
			{
				buff[i] = 0;
				value = atoi(buff);
				memset(buff, 0, sizeof(buff));
				i = 0;
				convert = *ptr;
				ptr++;
				cumulative = cumulative + epoch_convert_switch(value, convert, time);
			}
		}
	}
	if(minus == 1)
	{
		return -cumulative;
	}
	else
	{
		return cumulative;
	}
}

unsigned long iptol(char *ip)
{
	unsigned char o1,o2,o3,o4; /* The 4 ocets */
	char tmp[13] = "000000000000\0";
	short i = 11; /* Current Index in tmp */
	short j = (strlen(ip) - 1);
	do
	{
		if ((ip[--j] == '.'))
		{
			i -= (i % 3);
		}
		else
		{
			tmp[--i] = ip[j];
		}
	}while (i > -1);

	o1 = (tmp[0] * 100) + (tmp[1] * 10) + tmp[2];
	o2 = (tmp[3] * 100) + (tmp[4] * 10) + tmp[5];
	o3 = (tmp[6] * 100) + (tmp[7] * 10) + tmp[8];
	o4 = (tmp[9] * 100) + (tmp[10] * 10) + tmp[11];
	return (o1 * 16777216) + (o2 * 65536) + (o3 * 256) + o4;
}

/* @brief Check if IP is valid */
int isValidIp4 (char *str)
{
	int segs = 0;   /* Segment count. */
	int chcnt = 0;  /* Character count within segment. */
	int accum = 0;  /* Accumulator for segment. */
	/* Catch NULL pointer. */
	if (str == NULL) return 0;
	/* Process every character in string. */
	while (*str != '\0')
	{
		/* Segment changeover. */
		if (*str == '.')
		{
			/* Must have some digits in segment. */
			if (chcnt == 0) return 0;
			/* Limit number of segments. */
			if (++segs == 4) return 0;
			/* Reset segment values and restart loop. */
			chcnt = accum = 0;
			str++;
			continue;
		}

		/* Check numeric. */
		if ((*str < '0') || (*str > '9')) return 0;
		/* Accumulate and check segment. */
		if ((accum = accum * 10 + *str - '0') > 255) return 0;
		/* Advance other segment specific stuff and continue loop. */
		chcnt++;
		str++;
	}
	/* Check enough segments and enough characters in last segment. */
	if (segs != 3) return 0;
	if (chcnt == 0) return 0;
	/* Address okay. */
	return 1;
}

/* @brief Check if a hostname is valid */
int isValidHostname (char *str)
{
	/* Catch NULL pointer. */
	if (str == NULL)
	{
		return 0;
	}
	/* Process every character in string. */
	while (*str != '\0')
	{
		if ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9') || (*str == '.') || (*str == '-') )
		{
			str++;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

/* Not Used For Now */
int __ns2__Hello(struct soap *soap, struct d__HelloType *ns1__Hello, struct d__ResolveType *ns1__HelloResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

/* Not Used For Now */
int __ns2__Bye(struct soap *soap, struct d__ByeType *ns1__Bye, struct d__ResolveType *ns1__ByeResponse)
{
	return SOAP_OK;
}

/**
 * @brief Probe
 * @This operation sends the ProbeMatch response for probe request.
 *
 * @param Request        : d__ProbeType structure contains the data requesting for device information.
 * @param Response       : d__ProbeMatchType structure contains the device information.
 * @return returns       : On success - SOAP_OK.
 : On failure - SOAP_FAULT.
 */

int __ns3__Probe(struct soap *soap, struct d__ProbeType *ns1__Probe, struct d__ProbeMatchesType *ns1__ProbeResponse)
{
	char macaddr[MACH_ADDR_LENGTH];
	char _IPAddr[INFO_LENGTH];
	char _HwId[LARGE_INFO_LENGTH];
	SysInfo *oSysInfo = GetSysInfo();
	int i = 0;
	int flag = 0;
	char list[20][100];
	int len = 0;
	int index = 0;
	if(ns1__Probe->Scopes != NULL)
	{
		if(strcmp(ns1__Probe->Scopes->__item, "") == 0)
		{
			flag = 1;
		}
		else
		{
			explodeitem(ns1__Probe->Scopes->__item, ' ', list);
			for(i = 0; **(list + i) != 0; i++)
			{
				len = strlen("onvif://www.onvif.org/type");
				if(!strncmp(list[i], "onvif://www.onvif.org/type", len))
				{
					if(list[i][len] == 0)
					{
						flag = 1;
					}
					else
					{
						index = 0;
						while(oSysInfo->onvif_scopes.type[index][0])
						{
							if(!strcmp((*(list + i) + len), oSysInfo->onvif_scopes.type[index]))
							{
								flag = 1;
								break;
							}
							index++;
						}
					}
				}
				len = strlen("onvif://www.onvif.org/location");
				if(!strncmp(list[i], "onvif://www.onvif.org/location", len))
				{
					if(list[i][len] == 0)
					{
						flag = 1;
					}
					else
					{
						index = 0;
						while(oSysInfo->onvif_scopes.location[index][0])
						{
							if(!strcmp((*(list + i) + len), oSysInfo->onvif_scopes.location[index]))
							{
								flag = 1;
								break;
							}
							index++;
						}
					}
				}
				len = strlen("onvif://www.onvif.org/hardware");
				if(!strncmp(list[i], "onvif://www.onvif.org/hardware", len))
				{
					if(list[i][len] == 0)
					{
						flag = 1;
					}
					else if(!strcmp((*(list + i) + len), oSysInfo->onvif_scopes.hardware))
					{
						flag = 1;
					}
				}
				len = strlen("onvif://www.onvif.org/name");
				if(!strncmp(list[i], "onvif://www.onvif.org/name", len))
				{
					if(list[i][len] == 0)
					{
						flag = 1;
					}
					else if(!strcmp((*(list + i) + len), oSysInfo->onvif_scopes.name))
					{
						flag = 1;
					}
				}

			}
		}
	}
	else
	{
		flag = 1;
	}


	if(flag == 1)
	{
		ns1__ProbeResponse->ProbeMatch = (struct d__ProbeMatchType *)soap_malloc(soap, sizeof(struct d__ProbeMatchType));
		ns1__ProbeResponse->ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		ns1__ProbeResponse->ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		ns1__ProbeResponse->ProbeMatch->Scopes = (struct d__ScopesType*)soap_malloc(soap,sizeof(struct d__ScopesType));
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceProperties = (struct wsa__ReferencePropertiesType*)soap_malloc(soap,sizeof(struct wsa__ReferencePropertiesType));
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceParameters = (struct wsa__ReferenceParametersType*)soap_malloc(soap,sizeof(struct wsa__ReferenceParametersType));
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ServiceName = (struct wsa__ServiceNameType*)soap_malloc(soap,sizeof(struct wsa__ServiceNameType));
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.PortType = (char **)soap_malloc(soap, sizeof(char *) * SMALL_INFO_LENGTH);
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__any = (char **)soap_malloc(soap, sizeof(char*) * SMALL_INFO_LENGTH);
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__anyAttribute = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		NET_IPV4 ip;
		net_get_hwaddr(ETH_NAME, macaddr);
		sprintf(_HwId,"urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
		ip.int32 = net_get_ifaddr(ETH_NAME);
		sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/device_service", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
		ns1__ProbeResponse->__sizeProbeMatch = 1;
		ns1__ProbeResponse->ProbeMatch->Scopes->__item =(char *)soap_malloc(soap, LARGE_INFO_LENGTH);
		memset(ns1__ProbeResponse->ProbeMatch->Scopes->__item,0,sizeof(ns1__ProbeResponse->ProbeMatch->Scopes->__item));
		while(strstr(oSysInfo->onvif_scopes.type[i], DEFAULT_SCOPE) != NULL)
		{
			strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, oSysInfo->onvif_scopes.type[i]);
			strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, " ");
			i++;
		}
		i = 0;
		strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, oSysInfo->onvif_scopes.name);
		strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, " ");
		strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, oSysInfo->onvif_scopes.hardware);
		strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, " ");
		while(strstr(oSysInfo->onvif_scopes.location[i], DEFAULT_SCOPE) != NULL)
		{
			strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, oSysInfo->onvif_scopes.location[i]);
			strcat(ns1__ProbeResponse->ProbeMatch->Scopes->__item, " ");
			i++;
		}

		ns1__ProbeResponse->ProbeMatch->Scopes->MatchBy = NULL;
		strcpy(ns1__ProbeResponse->ProbeMatch->XAddrs, _IPAddr);
		//strcpy(ns1__ProbeResponse->ProbeMatch->Types, ns1__Probe->Types);
		strcpy(ns1__ProbeResponse->ProbeMatch->Types, "dn:NetworkVideoTransmitter");
		ns1__ProbeResponse->ProbeMatch->MetadataVersion = 1;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceProperties->__size = 0;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceProperties->__any = NULL;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceParameters->__size = 0;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ReferenceParameters->__any = NULL;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.PortType[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
		strcpy(ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.PortType[0], "ttl");
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ServiceName->__item = NULL;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ServiceName->PortName = NULL;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.ServiceName->__anyAttribute = NULL;
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__any[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
		strcpy(ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__any[0], "Any");
		strcpy(ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__anyAttribute, "Attribute");
		ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.__size = 0;
		strcpy(ns1__ProbeResponse->ProbeMatch->wsa__EndpointReference.Address, _HwId);
	}
	else
	{
		onvif_fault(soap,"ter:InvArgs","ter:InvArgs");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

int __ns4__GetSupportedRules(struct soap *soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns4__CreateRules(struct soap *soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns4__DeleteRules(struct soap *soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns4__GetRules(struct soap *soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns4__ModifyRules(struct soap *soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns5__GetSupportedAnalyticsModules(struct soap *soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns5__CreateAnalyticsModules(struct soap *soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns5__DeleteAnalyticsModules(struct soap *soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __ns5__GetAnalyticsModules(struct soap *soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
{
	onvif_fault(soap,"ter:Action", "ter:AnalyticsModuleNotSupported ");
	return SOAP_FAULT;
}

int __ns5__ModifyAnalyticsModules(struct soap *soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse)
{
	onvif_fault(soap,"ter:Action", "ter:AnalyticsModuleNotSupported ");
	return SOAP_FAULT;
}

int __tad__DeleteAnalyticsEngineControl(struct soap *soap, struct _tad__DeleteAnalyticsEngineControl *tad__DeleteAnalyticsEngineControl, struct _tad__DeleteAnalyticsEngineControlResponse *tad__DeleteAnalyticsEngineControlResponse)
{
	onvif_fault(soap,"ter:Action", "ter:AnalyticsModuleNotSupported ");
	return SOAP_FAULT;
}

int __tad__CreateAnalyticsEngineControl(struct soap *soap, struct _tad__CreateAnalyticsEngineControl *tad__CreateAnalyticsEngineControl, struct _tad__CreateAnalyticsEngineControlResponse *tad__CreateAnalyticsEngineControlResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__SetAnalyticsEngineControl(struct soap *soap, struct _tad__SetAnalyticsEngineControl *tad__SetAnalyticsEngineControl, struct _tad__SetAnalyticsEngineControlResponse *tad__SetAnalyticsEngineControlResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsEngineControl(struct soap *soap, struct _tad__GetAnalyticsEngineControl *tad__GetAnalyticsEngineControl, struct _tad__GetAnalyticsEngineControlResponse *tad__GetAnalyticsEngineControlResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsEngineControls(struct soap *soap, struct _tad__GetAnalyticsEngineControls *tad__GetAnalyticsEngineControls, struct _tad__GetAnalyticsEngineControlsResponse *tad__GetAnalyticsEngineControlsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsEngine(struct soap *soap, struct _tad__GetAnalyticsEngine *tad__GetAnalyticsEngine, struct _tad__GetAnalyticsEngineResponse *tad__GetAnalyticsEngineResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;

}

int __tad__GetAnalyticsEngines(struct soap *soap, struct _tad__GetAnalyticsEngines *tad__GetAnalyticsEngines, struct _tad__GetAnalyticsEnginesResponse *tad__GetAnalyticsEnginesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__SetVideoAnalyticsConfiguration(struct soap *soap, struct _tad__SetVideoAnalyticsConfiguration *tad__SetVideoAnalyticsConfiguration, struct _tad__SetVideoAnalyticsConfigurationResponse *tad__SetVideoAnalyticsConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__SetAnalyticsEngineInput(struct soap *soap, struct _tad__SetAnalyticsEngineInput *tad__SetAnalyticsEngineInput, struct _tad__SetAnalyticsEngineInputResponse *tad__SetAnalyticsEngineInputResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsEngineInput(struct soap *soap, struct _tad__GetAnalyticsEngineInput *tad__GetAnalyticsEngineInput, struct _tad__GetAnalyticsEngineInputResponse *tad__GetAnalyticsEngineInputResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsEngineInputs(struct soap *soap, struct _tad__GetAnalyticsEngineInputs *tad__GetAnalyticsEngineInputs, struct _tad__GetAnalyticsEngineInputsResponse *tad__GetAnalyticsEngineInputsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:AnalyticsEngineNotSupported ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsDeviceStreamUri(struct soap *soap, struct _tad__GetAnalyticsDeviceStreamUri *tad__GetAnalyticsDeviceStreamUri, struct _tad__GetAnalyticsDeviceStreamUriResponse *tad__GetAnalyticsDeviceStreamUriResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoAnalyticsEngineControl  ");
	return SOAP_FAULT;
}

int __tad__GetVideoAnalyticsConfiguration(struct soap *soap, struct _tad__GetVideoAnalyticsConfiguration *tad__GetVideoAnalyticsConfiguration, struct _tad__GetVideoAnalyticsConfigurationResponse *tad__GetVideoAnalyticsConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig ");
	return SOAP_FAULT;
}

int __tad__CreateAnalyticsEngineInputs(struct soap *soap, struct _tad__CreateAnalyticsEngineInputs *tad__CreateAnalyticsEngineInputs, struct _tad__CreateAnalyticsEngineInputsResponse *tad__CreateAnalyticsEngineInputsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:invalidConfig ");
	return SOAP_FAULT;
}

int __tad__DeleteAnalyticsEngineInputs(struct soap *soap, struct _tad__DeleteAnalyticsEngineInputs *tad__DeleteAnalyticsEngineInputs, struct _tad__DeleteAnalyticsEngineInputsResponse *tad__DeleteAnalyticsEngineInputsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:invalidConfig ");
	return SOAP_FAULT;
}

int __tad__GetAnalyticsState(struct soap *soap, struct _tad__GetAnalyticsState *tad__GetAnalyticsState, struct _tad__GetAnalyticsStateResponse *tad__GetAnalyticsStateResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidConfig");
	return SOAP_FAULT;
}

/**
 * @brief Get Device Information (Onvif Core Spec.Ver 2.0- Section 8.3.1)
 * @This operation gets the following device information:
    -Manufacturer,
    -Model,
    -Firmware version,
    -Serial Number,
    -Hardware Id
 *
 * @param Request	: _tds__GetDeviceInformation structure contains the request. In this function, the request is an empty message.
 * @param Response	: _tds__GetDeviceInformationResponse structure contains the response.
 * @return 		: On success - SOAP_OK
   On failure - There is no command specific faults returned.
 */
int __tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char mac[MACH_ADDR_LENGTH];

	char *_Manufacturer = (char *) malloc(sizeof(char) * LARGE_INFO_LENGTH);
	char *_Model = (char *) malloc(sizeof(char) * LARGE_INFO_LENGTH);
	char *_FirmwareVersion =  (char *) malloc(sizeof(char) * LARGE_INFO_LENGTH);
	char *_SerialNumber = (char *) malloc(sizeof(char) * LARGE_INFO_LENGTH);
	char *_HardwareId = (char *) malloc(sizeof(char) * LARGE_INFO_LENGTH);

	strcpy(_Manufacturer,oSysInfo->lan_config.title);
	strcpy(_Model,oSysInfo->lan_config.modelname);

	switch(oSysInfo->lan_config.chipConfig)
	{
		case CHIP_DM365:
			strcpy(_FirmwareVersion,DM365_APP_VERSION);
			break;
		default:
		case CHIP_DM368:
		case CHIP_DM368E:
			strcpy(_FirmwareVersion,DM368_APP_VERSION);
			break;
		case CHIP_DM369:
			strcpy(_FirmwareVersion,DM369_APP_VERSION);
			break;
		case CHIP_DMVA1:
			strcpy(_FirmwareVersion,DMVA1_APP_VERSION);
			break;
		case CHIP_DMVA2:
			strcpy(_FirmwareVersion,DMVA2_APP_VERSION);
			break;
	}

	net_get_hwaddr(ETH_NAME, mac);

	sprintf(_SerialNumber,"%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	sprintf(_HardwareId,"1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	tds__GetDeviceInformationResponse->Manufacturer = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	tds__GetDeviceInformationResponse->Model = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	tds__GetDeviceInformationResponse->FirmwareVersion = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	tds__GetDeviceInformationResponse->SerialNumber = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	tds__GetDeviceInformationResponse->HardwareId = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);

	strcpy(tds__GetDeviceInformationResponse->Manufacturer, _Manufacturer);
	strcpy(tds__GetDeviceInformationResponse->Model, _Model);
	strcpy(tds__GetDeviceInformationResponse->FirmwareVersion, _FirmwareVersion);
	strcpy(tds__GetDeviceInformationResponse->SerialNumber, _SerialNumber);
	strcpy(tds__GetDeviceInformationResponse->HardwareId, _HardwareId);

	free(_Manufacturer);
	free(_Model);
	free(_FirmwareVersion);
	free(_SerialNumber);
	free(_HardwareId);

	return SOAP_OK;
}


/**
 * @brief Setting Date and Time of IPNC.(Onvif Core Spec.Ver 2.0- Section 8.3.7)
 * @This operation sets the following device information.
    -System date,
    -Time
 *
 * @param Request	: _tds__SetSystemDateAndTime structure contains the date and time information of the device.
 * @param Response	: _tds___tds__SetSystemDateAndTimeResponse structure contains dummy response.
 * @return returns       : On success - SOAP_OK.
    On failure - SOAP_FAULT.
 */
int __tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
	char value[INFO_LENGTH];
	int ret;
	int _Year;
	int _Month;
	int _Day;
	int _Hour;
	int _Minute;
	int _Second;
	int _DaylightSavings;
	char *_TZ;
	_DateTimeType = tds__SetSystemDateAndTime->DateTimeType; //Manual = 0, NTP = 1
	/* Time Zone */
	if(tds__SetSystemDateAndTime->TimeZone)
	{
		_TZ  = tds__SetSystemDateAndTime->TimeZone->TZ;
		ret = oset_timezone(_TZ);
		if(ret == 100)
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidTimeZone");
			return SOAP_FAULT;
		}
		ControlSystemData(SFIELD_SET_TIMEZONE, (void *)&ret, sizeof(ret));
	}

	/* DayLight */
	if(tds__SetSystemDateAndTime->DaylightSavings)
	{
		_DaylightSavings = tds__SetSystemDateAndTime->DaylightSavings;
		ControlSystemData(SFIELD_SET_DAYLIGHT, (void *)&_DaylightSavings, sizeof(_DaylightSavings));//DayLight
	}

	if(_DateTimeType == 1) // NTP
	{
		ControlSystemData(SFIELD_GET_SNTP_FQDN, (void *)value, sizeof(value)); //get sntp ip
		run_ntpclient(value);
		return SOAP_OK;
	}

	if(tds__SetSystemDateAndTime->UTCDateTime)
	{
		_Year = tds__SetSystemDateAndTime->UTCDateTime->Date->Year;
		_Month = tds__SetSystemDateAndTime->UTCDateTime->Date->Month;
		_Day = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;
		_Hour = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
		_Minute = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
		_Second = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;

		if((_Hour>24) || (_Minute>60) || (_Second>60) || (_Month>12) || (_Day>31))
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidDateTime");
			return SOAP_FAULT;
		}
		sys_set_date(_Year, _Month, _Day);
		sys_set_time(_Hour, _Minute, _Second);
	}

	return SOAP_OK;
}


/**
 * @brief Getting Date and Time of IPNC.
 * @This Operation gets the following information from the device:
   -System Date
   -Day light saving
   -Time Zone
   -Time
 * @param Request	:_tds__GetSystemDateAndTime structure contains requests.In this function, the request is an empty message.
 * @param Response	:_tds___tds__GetSystemDateAndTimeResponse sturcture variable members contains the values for Date and Time of IPNC.
 * @return               :On success -  SOAP_OK.
 :On failure -  No command specific faults
 */
int __tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
	/* UTC - Coordinated Universal Time */
	time(&current_time);
	SysInfo* oSysInfo = GetSysInfo();
	time_t tnow;
	time_t tnow_utc;
	struct tm *tmnow;
	struct tm *tmnow_utc;

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime = (struct tt__SystemDateTime*)soap_malloc(soap, sizeof(struct tt__SystemDateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = _DateTimeType;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = oSysInfo->lan_config.net.daylight_time;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->Extension = NULL;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone = (struct tt__TimeZone*)soap_malloc(soap, sizeof(struct tt__TimeZone));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = oget_timezone(oSysInfo->lan_config.net.ntp_timezone);//"GMT+05";//"NZST-12NZDT,M10.1.0/2,M3.3.0/3";//timezone;POSIX 1003.1

	time(&tnow);
	tmnow = localtime(&tnow);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime = (struct tt__DateTime*)soap_malloc(soap, sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time = (struct tt__Time*)soap_malloc(soap, sizeof(struct tt__Time));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour = tmnow->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute = tmnow->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second = tmnow->tm_sec;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date = (struct tt__Date*)soap_malloc(soap, sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year =  tmnow->tm_year + 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month =  tmnow->tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day = tmnow->tm_mday;

	tnow_utc = tnow - (oSysInfo->lan_config.net.ntp_timezone - 12) * 3600;
	tmnow_utc = localtime(&tnow_utc);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime = (struct tt__DateTime*)soap_malloc(soap, sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time = (struct tt__Time*)soap_malloc(soap, sizeof(struct tt__Time));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour = tmnow_utc->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute = tmnow_utc->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second = tmnow_utc->tm_sec;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date = (struct tt__Date*)soap_malloc(soap, sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year = tmnow_utc->tm_year + 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month = tmnow_utc->tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day = tmnow_utc->tm_mday;
	return SOAP_OK;
}

/**
 * @brief Setting System Factory Default of  IPNC.
 * @This operation reloads parameters of a device to their factory default values.
 * @param Request	: _tds__GetSystemFactoryDefault structure contains request for the following types of factory default to perform:
			 "Hard": All parameters are set to their factory default value
			 "Soft": All parameters except device vendor specific
 			parameters are set to their factory default values
 * @param Response	: _tds__GetSystemDateAndTimeResponse structure variable members contains dummy response  .
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults
 */
int __tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
	__u8 value;
	value = DEFAULT_VALUE;
	ControlSystemData(SFIELD_SET_DEFAULT, &value, sizeof(value));
	//"Hard": All parameters are set to their factory default value
	//"Soft": All parameters except device vendor specific parameters are set to their factory default values
	return SOAP_OK;
}

/**
 * @brief Updating firmware for IPNC.
 * @This operation upgrades a device firmware version
 * @param Request	: _tds__UpgradeSystemFirmware structure contains the firmware used for the upgrade.
 * @param Response	: _tds__UpgradeSystemFirmwareResponse structure variable members contains dummy response.
 * @return               : On success - SOAP_OK.
 On failure - The firmware was invalid, i.e., not supported by this device.
 */
int __tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidFirmware");
	return SOAP_FAULT;
}

/**
 * @brief Reboot the IPNC.
 * @This function reboot the system.
 * @param Request	: _tds__SystemReboot structure contains dummy request.
 * @param Response	: _tds__SystemReboot structure contains the following message:
   			   "Rebooting".
 * @return               : On success - SOAP_OK.
 : On failure - No command specific faults.
 */
int __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	tds__SystemRebootResponse->Message = "Rebooting";
	return SOAP_OK;
}

/**
 * @brief Restore System.
 * @This operation restores the system backup configuration files(s) previously retrieved from a device.

 * @param Request	: _tds__RestoreSystem structure contains the system backup file(s).
 * @param Response	: _tds__RestoreSystem structure dummy response.in this function response message is empity.
 * @return               : On success - SOAP_OK.
 : On failure - The backup file(s) are invalid.
 */
int __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:RestoreNotSupported");
	return SOAP_FAULT;
}
/**
 * @brief Get System backup
 * @This operation gets the  system backup configuration files(s) previously retrieved from adevice.
 * @param Request       :_tds__GetSystemBackup structure contains request for system backup.in this function message is empity.
 * @param Response      :_tds__GetSystemBackupResponse structure contains the system backup configuration files(s).
 * @return              : On success - SOAP_OK.
 : On failure - No command specific faults.

 */
int __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:BackupNotSupported");
	return SOAP_FAULT;
}


/**
 * @brief  Get System log from a device.
 * @This operation gets a system log from a device.

 * @param Request	: _tds__GetSystemLog structure contains the type of system log to retrieve. The types of supported log information is defined in two different types:
 "System": The system log
 "Access": The client access log

 * @param Response	: _tds__GetSystemLogResponse structure variable members contains the requested system log information.
 * @return               : On success - SOAP_OK on success.
 : On failure - There is no access log information available or
 There is no system log information available

 */
int __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
	LogEntry_t tLog;
	struct tm* ptm;
	int logNum=0;
	int logIndex = 0;
	char logBuffer[4096];
	char timeStampBuffer[32];

	tds__GetSystemLogResponse->SystemLog = (struct tt__SystemLog*)soap_malloc(soap, sizeof(struct tt__SystemLog));
	tds__GetSystemLogResponse->SystemLog->Binary = (struct tt__AttachmentData*)soap_malloc(soap, sizeof(struct tt__AttachmentData));
	//	System = 0, Access = 1
	if(tds__GetSystemLog->LogType == 0)
	{
		int i,iRet;
		tds__GetSystemLogResponse->SystemLog->Binary->xmime__contentType = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
		strcpy(tds__GetSystemLogResponse->SystemLog->Binary->xmime__contentType, "System");
		GetSysLog(0, logIndex, &tLog);
		ptm = &tLog.time;

		if(strstr(tLog.event, "login") != NULL)
		{
			logNum++;
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.__ptr = NULL; //string
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.__size = NUM_LOG_PER_PAGE; //int
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.id = (char *)&logNum;	/*string optional element of type xsd:string */
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.type = (char *) soap_malloc(soap, sizeof(char) * MID_INFO_LENGTH);
			sprintf(tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.type, "%s", tLog.event);
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.options = (char *) soap_malloc(soap, sizeof(char) * MID_INFO_LENGTH);
			sprintf(tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.options, "%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		}
		logIndex = 0;
		memset(logBuffer, 0 ,sizeof(logBuffer));
		for(i = 0; i < NUM_LOG_PER_PAGE; i++)
		{
			iRet = GetSysLog(0, logIndex, &tLog);
			if(iRet == -1)
				break;
			ptm = &tLog.time;

			strncat(logBuffer, tLog.event, strlen(tLog.event) );
			sprintf(timeStampBuffer, " %d-%02d-%02d %02d:%02d:%02d \n",
				ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,ptm->tm_hour, ptm->tm_min, ptm->tm_sec  );
			strncat(logBuffer, timeStampBuffer, strlen(timeStampBuffer));

			logIndex++;
		}
		tds__GetSystemLogResponse->SystemLog->String = (char *) soap_malloc(soap, sizeof(char) * strlen(logBuffer));
		strcpy( tds__GetSystemLogResponse->SystemLog->String, logBuffer );

	}

	else if(tds__GetSystemLog->LogType == 1)
	{
		int i,iRet;
		tds__GetSystemLogResponse->SystemLog->Binary->xmime__contentType = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
		strcpy(tds__GetSystemLogResponse->SystemLog->Binary->xmime__contentType, "Access");
		iRet = GetSysLog(0, logIndex, &tLog);
		ptm = &tLog.time;

		if(strstr(tLog.event, "login") != NULL)
		{
			logNum++;
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.__ptr = NULL; //string
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.__size = NUM_LOG_PER_PAGE; //int
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.id = (char *)&logNum;	/*string optional element of type xsd:string */
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.type = (char *) soap_malloc(soap, sizeof(char) * MID_INFO_LENGTH);
			sprintf(tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.type,"%s",tLog.event);
			tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.options = (char *) soap_malloc(soap, sizeof(char) * MID_INFO_LENGTH);
			sprintf(tds__GetSystemLogResponse->SystemLog->Binary->xop__Include.options,"%d-%02d-%02d %02d:%02d:%02d",ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		}
		logIndex = 0;

		memset(logBuffer, 0 ,sizeof(logBuffer));
		for(i = 0; i < NUM_LOG_PER_PAGE; i++)
		{
			iRet = GetSysLog(0, logIndex, &tLog);
			if(iRet == -1)
				break;
			ptm = &tLog.time;

			strncat(logBuffer, tLog.event, strlen(tLog.event) );
			sprintf(timeStampBuffer, " %d-%02d-%02d %02d:%02d:%02d \n",
				ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,ptm->tm_hour, ptm->tm_min, ptm->tm_sec  );
			strncat(logBuffer, timeStampBuffer, strlen(timeStampBuffer));

			logIndex++;
		}
		tds__GetSystemLogResponse->SystemLog->String = (char *) soap_malloc(soap, sizeof(char) * strlen(logBuffer));
		strcpy( tds__GetSystemLogResponse->SystemLog->String, logBuffer );
	}

	return SOAP_OK;
}

/**
  @brief Get System Support Information
  @This operation gets arbitrary device diagnostics information from a device.
  @param request             :_tds__GetSystemSupportInformation structure contains the request.in this function request is an empity message.
  @param response            :_tds__GetSystemSupportInformationResponse strucure contains the support information. The device can choose if it wants to return the support
  information as binary data or as a common string.
  @return                    :On success - SOAP_OK
  :On failure - There is no support information available.

 */
int __tds__GetSystemSupportInformation(struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:SupportInformationUnavailable");
	return SOAP_FAULT;
}

/**
 * @brief Get Scopes of the device
 * @This operation requests the scope parameters of a device.

 *
 * @param Request	: _tds__GetScopes structure varialbles members contains an empty message.
 * @param Response	: _tds__GetScopesResponse structure variable members ontains a list of URIs
 defining the device scopes.
 * @return               : On success - SOAP_OK.
 : On failure - Scope list is empty.
 */
int __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int i=0;
	int num_scope=0;
	int num_type=0;
	int num_loc=0;
	int _size=0;
	while(strlen(oSysInfo->onvif_scopes.type[num_type])!=0)
	{
		num_type++;
	}
	while(strlen(oSysInfo->onvif_scopes.location[num_loc])!=0)
	{
		num_loc++;
	}
	_size = (num_type + num_loc + 2);
	tds__GetScopesResponse->__sizeScopes = _size;
	tds__GetScopesResponse->Scopes = (struct tt__Scope*)soap_malloc(soap, (_size *sizeof(struct tt__Scope)));
	while(strstr(oSysInfo->onvif_scopes.type[i],DEFAULT_SCOPE) != NULL)
	{
		tds__GetScopesResponse->Scopes[num_scope].ScopeDef = 1; // Fixed = 0, Configurable = 1
		tds__GetScopesResponse->Scopes[num_scope].ScopeItem = (char *)soap_malloc(soap, (sizeof(char) * INFO_LENGTH));
		strcpy(tds__GetScopesResponse->Scopes[num_scope].ScopeItem, oSysInfo->onvif_scopes.type[i]);
		i++;
		num_scope++;
	}
	tds__GetScopesResponse->Scopes[num_scope].ScopeDef = 0; // Fixed = 0, Configurable = 1
	tds__GetScopesResponse->Scopes[num_scope].ScopeItem = (char *)soap_malloc(soap, (sizeof(char) * INFO_LENGTH));
	strcpy(tds__GetScopesResponse->Scopes[num_scope].ScopeItem, oSysInfo->onvif_scopes.hardware);
	num_scope++;
	tds__GetScopesResponse->Scopes[num_scope].ScopeDef = 0; // Fixed = 0, Configurable = 1
	tds__GetScopesResponse->Scopes[num_scope].ScopeItem = (char *)soap_malloc(soap, (sizeof(char) * INFO_LENGTH));
	strcpy(tds__GetScopesResponse->Scopes[num_scope].ScopeItem, oSysInfo->onvif_scopes.name);
	num_scope++;
	i=0;
	while(strstr(oSysInfo->onvif_scopes.location[i], DEFAULT_SCOPE) != NULL)
	{
		tds__GetScopesResponse->Scopes[num_scope].ScopeDef = 1; // Fixed = 0, Configurable = 1
		tds__GetScopesResponse->Scopes[num_scope].ScopeItem = (char *)soap_malloc(soap, (sizeof(char) * INFO_LENGTH));
		strcpy(tds__GetScopesResponse->Scopes[num_scope].ScopeItem, oSysInfo->onvif_scopes.location[i]);
		i++;
		num_scope++;
	}
	return SOAP_OK;
}

/**
 * @brief Set scopes of device
 * @This operation sets the scope parameters of a device. The scope parameters are used in the
 device discovery to match probe messages.
 *
 * @param Request	: _tds__SetScopes structure contains a list of URIs defining the device scope.
 * @param Response	: _tds__SetScopesResponse structure contains an empty message.
 * @return               : On success SOAP_OK.
 : On failure - Scope parameter overwrites fixed device scope setting, command rejected. or
 The requested scope list exceeds the supported number of scopes.

 */
int __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
	int i;
	int err = 0;
	int size;
	SysInfo* oSysInfo = GetSysInfo();
	char *_Model = oSysInfo->lan_config.modelname;

	char mac[MACH_ADDR_LENGTH];
	char _HardwareId[LARGE_INFO_LENGTH];
	char _HardwareId1[LARGE_INFO_LENGTH];
	char _Model1[LARGE_INFO_LENGTH];
	net_get_hwaddr(ETH_NAME, mac);
	sprintf(_HardwareId,"1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	sprintf(_HardwareId1, "onvif://www.onvif.org/hardware/%s", _HardwareId);
	sprintf(_Model1, "onvif://www.onvif.org/name/%s", _Model);

	size = tds__SetScopes->__sizeScopes;
	for(i=0; i<size; i++)
	{
		if(strcmp(tds__SetScopes->Scopes[i], "onvif://www.onvif.org/type/Network_Video_Transmitter") || strcmp(tds__SetScopes->Scopes[i], _HardwareId1) || strcmp(tds__SetScopes->Scopes[i], _Model1) || strcmp(tds__SetScopes->Scopes[i], "onvif://www.onvif.org/type/Network_Video_Analytic") || strcmp(tds__SetScopes->Scopes[i], "onvif://www.onvif.org/type/Network_Video_Storage"))
		{
			err = 1;
		}
		else
		{
			err = 0;
		}

	}
	if(err == 1)
	{
		onvif_fault(soap,"ter:OperationProhibited", "ter:ScopeOverwrite");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}
/**
 * @brief Add Scopes of the device
 * @This operation adds new configurable scope parameters to a device.

 *
 * @param Request        : _tds__AddScopes structure  contains a list of URIs to be added to the existing configurable scope list.

 * @param Response       : _tds__AddScopesResponse structure contains an empty message.

 * @return               : On success - SOAP_OK.
 : On failure - Appropriate fault code.

 */

int __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	Onvif_Scopes_t onvif_scopes_t;
	int size =  tds__AddScopes->__sizeScopeItem;
	char checker[INFO_LENGTH];
	char *check = NULL;
	int i;
	int num_type = 0;
	int num_location = 0;
	int num_loc;
	int num_tp;
	for(i = 0; i < size; i++)
	{
	        check = strparser(tds__AddScopes->ScopeItem[i]);
		if(check == NULL)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:TooManyScopes");
			free(check);
			return SOAP_FAULT;
		}
		strcpy(checker, check);
		free(check);
		num_tp = 0;
		num_loc = 0;
		if(strcmp(checker, "type") == 0)
		{
			while(strstr(oSysInfo->onvif_scopes.type[num_tp], DEFAULT_SCOPE) != NULL)
			{
				if(strcmp(oSysInfo->onvif_scopes.type[num_tp], tds__AddScopes->ScopeItem[i]) == 0)
				{
					onvif_fault(soap,"ter:OperationProhibited", "ter:ScopeOverwrite");
					return SOAP_FAULT;
				}
				num_tp++;
			}
			if(num_tp < 3)
			{
				strcpy(onvif_scopes_t.type_t[num_type], tds__AddScopes->ScopeItem[i]);
			}
			else
			{
				onvif_fault(soap,"ter:OperationProhibited", "ter:TooManyScopes");
				return SOAP_FAULT;
			}
			num_type++;
		}
		else if(strcmp(checker, "name") == 0)
		{
            		onvif_fault(soap,"ter:OperationProhibited", "ter:InValArgs");
			return SOAP_FAULT;
		}
		else if(strcmp(checker, "hardware") == 0)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:InValArgs");
			return SOAP_FAULT;
		}
		else if(strcmp(checker, "location") == 0)
		{
			while(strstr(oSysInfo->onvif_scopes.location[num_loc], DEFAULT_SCOPE) != NULL)
			{
				if(strcmp(oSysInfo->onvif_scopes.location[num_loc], tds__AddScopes->ScopeItem[i]) == 0)
				{
					onvif_fault(soap,"ter:OperationProhibited", "ter:ScopeOverwrite");
					return SOAP_FAULT;
				}
				num_loc++;
			}
			if(num_loc < 3)
			{
				strcpy(onvif_scopes_t.location_t[num_location], tds__AddScopes->ScopeItem[i]);
			}
			else
			{
				onvif_fault(soap,"ter:OperationProhibited", "ter:TooManyScopes");
				return SOAP_FAULT;
			}
			num_location++;
		}
		else
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:InValArgs");
			return SOAP_FAULT;
		}
	}
	ControlSystemData(SFIELD_ADD_SCOPE, &onvif_scopes_t, sizeof(onvif_scopes_t));
	return SOAP_OK;
}

/**
 * @brief Remove Scopes of the device
 * @ This operation deletes scope-configurable scope parameters from a device.
 *
 * @param Request        : _tds__RemoveScopes structure  contains a list of URIs to be removed to the existing configurable scope list.

 * @param Response       : _tds__RemoveScopesResponse structure The scope response message contains a list of URIs that has been removed from the device scope.
 * @return               : On success - SOAP_OK.
 : On failure - Appropriate fault code.

 */

int __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	char checker[INFO_LENGTH];
	char *check = NULL;
	int i = 0;
	int num_tp = 0;
	int num_loc = 0;
	int num_type = 0;
	int num_location = 0;
	int LOC_MAX = 0;
	int TYPE_MAX = 0;
	Onvif_Scopes_t onvif_scopes_t;
	int size = tds__RemoveScopes->__sizeScopeItem;
	while(strstr(oSysInfo->onvif_scopes.type[num_tp], DEFAULT_SCOPE) != NULL)
	{
		TYPE_MAX++;
		num_tp++;
	}
	while(strstr(oSysInfo->onvif_scopes.location[num_loc], DEFAULT_SCOPE) != NULL)
	{
		LOC_MAX++;
		num_loc++;
	}
	num_tp = 0;
	num_loc= 0;
	for(i = 0; i < size; i++)
	{
		check = strparser(tds__RemoveScopes->ScopeItem[i]);
		if(check != NULL)
		{
			strcpy(checker, check);
			free(check);
		}
		else
		{
			continue;
		}
		if(strcmp(checker, "type") == 0)
		{
			while(strstr(oSysInfo->onvif_scopes.type[num_type], DEFAULT_SCOPE) != NULL)
			{
				if(strcmp(oSysInfo->onvif_scopes.type[num_type], tds__RemoveScopes->ScopeItem[i]) == 0)
				{
					strcpy(onvif_scopes_t.type_t[num_tp], tds__RemoveScopes->ScopeItem[i]);
					break;
				}
				num_type++;
			}
			if(num_type >= TYPE_MAX)
			{
				onvif_fault(soap,"ter:InvalidArgs", "ter:ScopeNotExist");
				return SOAP_FAULT;
			}
			num_tp++;
		}
		else if(strcmp(checker, "name") == 0)
		{

			onvif_fault(soap,"ter:OperationProhibited", "ter:InvalidArgs");
			return SOAP_FAULT;
		}
		else if(strcmp(checker, "hardware") == 0)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:InvalidArgs");
			return SOAP_FAULT;
		}
		else if(strcmp(checker, "location") == 0)
		{
			while(strstr(oSysInfo->onvif_scopes.location[num_location], DEFAULT_SCOPE) != NULL)
			{
				if(strcmp(oSysInfo->onvif_scopes.location[num_location], tds__RemoveScopes->ScopeItem[i]) == 0)
				{
					strcpy(onvif_scopes_t.location_t[num_loc], tds__RemoveScopes->ScopeItem[i]);
					break;
				}
				num_location++;
			}
			if(num_location >= LOC_MAX)
			{
				onvif_fault(soap,"ter:InvalidArgs", "ter:ScopesNotExist");
				return SOAP_FAULT;
			}
			num_loc++;
		}
		else
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:InvalidArgs");
			return SOAP_FAULT;
		}
	}
	ControlSystemData(SFIELD_DEL_SCOPE, &onvif_scopes_t, sizeof(onvif_scopes_t));
	tds__RemoveScopesResponse->__sizeScopeItem = size;
	tds__RemoveScopesResponse->ScopeItem = (char **)soap_malloc(soap,sizeof(char *) * INFO_LENGTH);
	for(i = 0; i < size; i++)
	{
		tds__RemoveScopesResponse->ScopeItem[i] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__RemoveScopesResponse->ScopeItem[i], tds__RemoveScopes->ScopeItem[i]);
	}

	return SOAP_OK;
}


/**
 * @brief Get Discovery Mode of device
 * @This operation gets the discovery mode of a device.
 *
 * @param Request	:_tds__GetDiscoveryMode structures contains an empty message.
 * @param Response	:_tds__iGetDiscoveryModeResponse members contains the current discovery mode setting, i.e. discoverable or non-discoverable. Discovery mode is always On.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults
 */
int __tds__GetDiscoveryMode(struct soap *soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	tds__GetDiscoveryModeResponse->DiscoveryMode =  oSysInfo->lan_config.net.discovery_mode;
	return SOAP_OK;
}

/**
 * @brief Set Discovery Mode of device
 * @This operation sets the discovery mode operation of a device.
 *
 * @param Request        :_tds__SetDiscoveryMode contains an empty message.
 * @param Response       :_tds__SetDiscoveryModeResponse contains the current discovery mode setting, i.e. discoverable or non-discoverable.

 * @return               : On success - SOAP_OK.
 On failure - No command specific faults.
 */

int __tds__SetDiscoveryMode(struct soap *soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
	__u8  _mode = tds__SetDiscoveryMode->DiscoveryMode;
	if(_mode == 0 || _mode == 1)
	{
		ControlSystemData(SFIELD_DIS_MODE,&_mode, sizeof(_mode));
	}
	else
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidArgVal");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

int __tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{

	onvif_fault(soap,"ter:InvalidArgVal", "ter:GetRemoteDiscoveryModeNotSupported");
	return SOAP_FAULT;
}

int __tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:SetRemoteDiscoveryModeNotSupported");
	return SOAP_FAULT;
}

int __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:GetDiscoveryProxyNotSupported");
	return SOAP_FAULT;
}

int __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:SetDiscoveryProxyNotSupported");
	return SOAP_FAULT;
}

int __tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:GetEndPointRefNotSupported");
	return SOAP_FAULT;
}

int __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:RemoteUserNotSupported");
	return SOAP_FAULT;
}

int __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:RemoteUserNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Users Information
 * @This operation lists the registered users and the following corresponding credentials on a device:
 -User name
 -User level
 *
 * @param Request	:_tds__GetUsers structures contains an empty message.
 * @param Response	:_tds__GetUsersResponse contains list of users and corresponding credentials.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults

 */
int __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int userIndex = 0;
	int i;
	int NEW_ACOUNT=0;
	//User Level :- {onv__UserLevel__Administrator = 0, onv__UserLevel__Operator = 1, onv__UserLevel__User = 2, onv__UserLevel__Anonymous = 3, onv__UserLevel__Extended = 4}
	//IPNC authority = 0 "Admin"; 1 "Operator"; 2 "Viewer"; 3 "Superuser";
	//Response
	for(i = 0; i < ACOUNT_NUM; i++)
	{
		if (strlen(oSysInfo->acounts[NEW_ACOUNT].user) == 0)
		{
			break;
		}
		NEW_ACOUNT++;
	}

	tds__GetUsersResponse->__sizeUser = NEW_ACOUNT;
	tds__GetUsersResponse->User = (struct tt__User*)soap_malloc(soap, (NEW_ACOUNT * sizeof(struct tt__User)));
	for(i = 0; i < NEW_ACOUNT; i++)
	{
		tds__GetUsersResponse->User[userIndex].Username = oSysInfo->acounts[userIndex].user;
		tds__GetUsersResponse->User[userIndex].Password = NULL;
		tds__GetUsersResponse->User[userIndex].UserLevel = oSysInfo->acounts[userIndex].authority;
		tds__GetUsersResponse->User[userIndex].Extension = NULL;
		if (strlen(tds__GetUsersResponse->User[userIndex].Username) == 0)
		{
			break;
		}
		userIndex++;
	}
	return SOAP_OK;
}

/**
 * @brief Create Users on device
 * @This operation creates new device users and corresponding credentials on a device for the user token profile.
 *
 * @param Request	:_tds__CreateUsers structures contains the following  user parameters element for a new user:
			 -Username
			 -Password
			 -UserLevel
 * @param Response	:_tds__CreateUsersResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 On failure - Appropriate fault code(ONVIF Core Spec. - Ver. 2.00 8.4.4)
 */
int __tds__CreateUsers(struct soap *soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{

	SysInfo* oSysInfo = GetSysInfo();
	add_user_t account;
	int size = tds__CreateUsers->__sizeUser;
	int i;
	int j;
	int NEW_ACOUNT = 0;

	for(i = 0; i < ACOUNT_NUM; i++)
	{
		if (strlen(oSysInfo->acounts[NEW_ACOUNT].user) == 0)
		{
			break;
		}
		NEW_ACOUNT++;
	}

	if(size > (ACOUNT_NUM - NEW_ACOUNT))
	{
		onvif_fault(soap,"ter:OperationProhibited","ter:TooManyUsers");
		return SOAP_FAULT;
	}

	for(i = 0; i < size; i++)
	{
		if(strlen(tds__CreateUsers->User[i].Username) > USER_LEN)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:UsernameTooLong");
			return SOAP_FAULT;
		}
		if(strlen(tds__CreateUsers->User[i].Password) > PASSWORD_LEN)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:PasswordTooLong");
			return SOAP_FAULT;
		}
		if(tds__CreateUsers->User[i].UserLevel > 2 || tds__CreateUsers->User[i].UserLevel < 0)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:NotAllowedUserLevel");
			return SOAP_FAULT;
		}

	}

	/* check username already exist */
	if (strcmp(tds__CreateUsers->User[0].Username, "admin") != 0)
	{
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < ACOUNT_NUM; j++)
			{
				if (strcmp(tds__CreateUsers->User[i].Username, oSysInfo->acounts[j].user) == 0)
				{
					onvif_fault(soap,"ter:OperationProhibited", "ter:UsernameClash");
					return SOAP_FAULT;
				}
			}
		}
	}

	/* Save user */
	for(i = 0; i < size; i++)
	{
		strcpy(account.user_id, tds__CreateUsers->User[i].Username);
		strcpy(account.password, tds__CreateUsers->User[i].Password);
		account.authority = tds__CreateUsers->User[i].UserLevel;
		ControlSystemData(SFIELD_ADD_USER, (void *)&account, sizeof(account));
	}

	return SOAP_OK;
}

/**
 * @brief Delete User from device
 * @This operation creates new device users and corresponding credentials on a device for the user token profile.
 *
 * @param Request	: _tds__DeleteUsers structures contains the name of the user or users to be deleted.
 * @param Response	: _tds__DeleteUsersResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 On failure - Username NOT recognized. or
 Username may not be deleted
 */
int __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char newuser[SMALL_INFO_LENGTH];
	int size = tds__DeleteUsers->__sizeUsername;
	int i = 0;
	int j = 0;
	int NUM_ACOUNT = 0;

	for(i = 0; i < ACOUNT_NUM; i++)
	{
		if (strlen(oSysInfo->acounts[NUM_ACOUNT].user) == 0)
		{
			break;
		}
		NUM_ACOUNT++;
	}

	/* check username already exist */
	for(i = 0; i < size; i++)
	{
		for(j = 0; j <= NUM_ACOUNT; j++)
		{
			if (strcmp(tds__DeleteUsers->Username[i],oSysInfo->acounts[j].user) == 0)
			{
				if(oSysInfo->acounts[j].authority == 0 && strcmp(tds__DeleteUsers->Username[i], "admin") == 0)
				{
					onvif_fault(soap,"ter:InvalidArgVal", "ter:FixedUser");
					return SOAP_FAULT;
				}
				break;
			}

			if(j == NUM_ACOUNT)
			{
				return SOAP_FAULT;
			}
		}
	}

	/* Delete user */
	for(i = 0; i < size; i++)
	{
		strcpy(newuser, tds__DeleteUsers->Username[i]);
		if(strlen(newuser) == 0)
		{
			onvif_fault(soap, "ter:OperationProhibited", "ter:AnonymousNotAllowed");
			return SOAP_FAULT;
		}
		ControlSystemData(SFIELD_DEL_USER, (void *)newuser, strlen(newuser) + 1);
	}
	return SOAP_OK;
}

/**
 * @brief Set User On device
 * @This operation updates the following settings for one or several users on a device for the user token profile.
 -Username
 -Password
 -Userlevel
 *
 * @param Request	: _tds__SetUsers structures contains a list of users and corresponding parameters to be updated.
 * @param Response	: _tds__SetUsersResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 On failure - Appropriate fault code(ONVIF Core Spec. - Ver. 2.00 8.4.6).
 */
int __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	Acount_t   account;
	int size = tds__SetUser->__sizeUser;
	int i;
	int j;
	int NEW_ACOUNT = 0;
	//User Level :- {onv__UserLevel__Administrator = 0, onv__UserLevel__Operator = 1, onv__UserLevel__User = 2, onv__UserLevel__Anonymous = 3, onv__UserLevel__Extended = 4}
	//IPNC authority = 0 "Admin"; 1 "Operator"; 2 "Viewer"; 3 "Superuser";
	//Response
	for(i = 0; i < ACOUNT_NUM; i++)
	{
		if (strlen(oSysInfo->acounts[NEW_ACOUNT].user) == 0)
		{
			break;
		}
		NEW_ACOUNT++;
	}

	/* check username already exist */
	int flag = 0;

	//if (strcmp(tds__SetUser->User[0].Username, "admin") != 0)
	{
		for(i = 0; i < size; i++)
		{
			flag = 0;
			for(j = 0; j < ACOUNT_NUM; j++)
			{
				if(strcmp(tds__SetUser->User[i].Username,oSysInfo->acounts[j].user) == 0)
				{
					if(tds__SetUser->User[i].UserLevel == 0  && strcmp(tds__SetUser->User[i].Username, "admin") == 0)
					{
						onvif_fault(soap,"ter:InvalidArgVal", "ter:FixedUser");
						return SOAP_FAULT;
					}
					flag = 1;
				}
			}
			if(flag == 0)
			{
				onvif_fault(soap,"ter:InvalidArgVal", "ter:UsernameMissing");
				return SOAP_FAULT;
			}
		}
	}

	/* Save user */
	if(flag == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:UsernameMissing");
		return SOAP_FAULT;
	}

	if(size > (ACOUNT_NUM - NEW_ACOUNT))
	{
		onvif_fault(soap,"ter:OperationProhibited", "ter:TooManyUsers");
		return SOAP_FAULT;
	}

	for(i=0;i<size;i++)
	{
		if(strlen(tds__SetUser->User[i].Username) > USER_LEN)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:UsernameTooLong");
			return SOAP_FAULT;
		}

		if(strlen(tds__SetUser->User[i].Password) > PASSWORD_LEN)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:PasswordTooLong");
			return SOAP_FAULT;
		}
		if(strlen(tds__SetUser->User[i].Password) ==  PASSWORD_ZERO)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:PasswordTooWeak");
			return SOAP_FAULT;
		}

		if(tds__SetUser->User[i].UserLevel > 2 || tds__SetUser->User[i].UserLevel < 0)
		{
			onvif_fault(soap,"ter:OperationProhibited", "ter:NotAllowedUserLevel");
			return SOAP_FAULT;
		}
	}

	for(i = 0; i < size; i++)
	{
		strcpy(account.user, tds__SetUser->User[i].Username);
		strcpy(account.password, tds__SetUser->User[i].Password);
		account.authority = tds__SetUser->User[i].UserLevel;
		ControlSystemData(SFIELD_ADD_USER, (void *)&account, sizeof(account));
	}
	return SOAP_OK;
}


/**
 * @brief Get wsl Url
 * @This operation request a URL that can be used to retrieve the complete schema and WSDL definitions of a device.
 *
 * @param Request	: _tds__GetWsdlUrl structures contains an empty message.
 * @param Response	: _tds__GetWsdlUrlResponse structure contains the requested URL.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults
 */
int __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
	tds__GetWsdlUrlResponse->WsdlUrl = "http://www.onvif.org/Documents/Specifications.aspx";
	return SOAP_OK;
}


/**
 * @brief Get Capabilities of device
 * @This message contains a request for device capabilities.
 *
 * @param Request	: _tds__GetCapabilities structures contains a request for device capabilities.
 			  The client can either ask for all capabilities or just the capabilities for a particular service category.
			  All = 0, Analytics = 1, Device = 2, Events = 3, Imaging = 4, Media = 5, PTZ = 6
 * @param Response	: _tds__GetCapabilitiesResponse contains the requested device capabilities using a hierarchical XML capability structure.
 * @return               : On success - SOAP_OK.
 On failure - The requested WSDL service category is not supported by the device.
 */

int __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	int _Category;
	char _IPv4Address[LARGE_INFO_LENGTH];

	if(tds__GetCapabilities->Category == NULL)
	{
		tds__GetCapabilities->Category=(int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilities->Category = ALL;
		_Category = ALL;
	}
	else
	{
		_Category = *tds__GetCapabilities->Category;
	}
	sprintf(_IPv4Address, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	tds__GetCapabilitiesResponse->Capabilities = (struct tt__Capabilities*)soap_malloc(soap, sizeof(struct tt__Capabilities));
	tds__GetCapabilitiesResponse->Capabilities->Analytics = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Device = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Events = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Imaging = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Media = NULL;
	tds__GetCapabilitiesResponse->Capabilities->PTZ = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension = (struct tt__CapabilitiesExtension*)soap_malloc(soap, sizeof(struct tt__CapabilitiesExtension));
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO = (struct tt__DeviceIOCapabilities*)soap_malloc(soap, sizeof(struct tt__DeviceIOCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr,_IPv4Address);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoOutputs = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioSources = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioOutputs = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->RelayOutputs = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Display = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Recording = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Search = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Replay = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions = NULL;
	if((_Category == ALL) || (_Category == ANALYTICS))
	{
		tds__GetCapabilitiesResponse->Capabilities->Analytics = (struct tt__AnalyticsCapabilities*)soap_malloc(soap, sizeof(struct tt__AnalyticsCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, _IPv4Address);
		tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = FALSE;
	}
	if((_Category == ALL) || (_Category == DEVICE))
	{
		tds__GetCapabilitiesResponse->Capabilities->Device = (struct tt__DeviceCapabilities*)soap_malloc(soap, sizeof(struct tt__DeviceCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, _IPv4Address);
		tds__GetCapabilitiesResponse->Capabilities->Device->Extension = NULL;

		tds__GetCapabilitiesResponse->Capabilities->Device->Network = (struct tt__NetworkCapabilities*)soap_malloc(soap, sizeof(struct tt__NetworkCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = (int *)soap_malloc(soap, sizeof(int));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = (int *)soap_malloc(soap, sizeof(int));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = (int *)soap_malloc(soap, sizeof(int));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = _false;
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = _false;
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = _false;
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = _false;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension = (struct tt__NetworkCapabilitiesExtension*)soap_malloc(soap, sizeof(struct tt__NetworkCapabilitiesExtension));
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = _false;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Extension = NULL;

		tds__GetCapabilitiesResponse->Capabilities->Device->System = (struct tt__SystemCapabilities*)soap_malloc(soap, sizeof(struct tt__SystemCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging = TRUE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade = TRUE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = TRUE;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions = (struct tt__OnvifVersion*)soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major = MAJOR;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor = MINOR;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension = (struct tt__SystemCapabilitiesExtension*)soap_malloc(soap, sizeof(struct tt__SystemCapabilitiesExtension));
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = _false;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = _true;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = _true;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = _true;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->Extension = NULL;

		tds__GetCapabilitiesResponse->Capabilities->Device->IO = (struct tt__IOCapabilities*)soap_malloc(soap, sizeof(struct tt__IOCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = &_false;
		tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = &_true;
		tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension = NULL;

		tds__GetCapabilitiesResponse->Capabilities->Device->Security = (struct tt__SecurityCapabilities*)soap_malloc(soap, sizeof(struct tt__SecurityCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension = NULL;

	}
	if((_Category == ALL) || (_Category == EVENTS))
	{
		tds__GetCapabilitiesResponse->Capabilities->Events = (struct tt__EventCapabilities*)soap_malloc(soap, sizeof(struct tt__EventCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, _IPv4Address);
		tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = FALSE;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport = FALSE;
	}
	if((_Category == ALL) || (_Category == IMAGING))
	{
		tds__GetCapabilitiesResponse->Capabilities->Imaging = (struct tt__ImagingCapabilities*)soap_malloc(soap, sizeof(struct tt__ImagingCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, _IPv4Address);
	}
	if((_Category == ALL) || (_Category == MEDIA))
	{
		tds__GetCapabilitiesResponse->Capabilities->Media = (struct tt__MediaCapabilities*)soap_malloc(soap, sizeof(struct tt__MediaCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, _IPv4Address);
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = (struct tt__RealTimeStreamingCapabilities*)soap_malloc(soap, sizeof(struct tt__RealTimeStreamingCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = _false;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = _true;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = _true;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->Extension = NULL;
		tds__GetCapabilitiesResponse->Capabilities->Media->Extension = NULL;
	}
	if((_Category == ALL) || (_Category == PANTILTZOOM))
	{
		tds__GetCapabilitiesResponse->Capabilities->PTZ = (struct tt__PTZCapabilities*)soap_malloc(soap, sizeof(struct tt__PTZCapabilities));
		tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, _IPv4Address);
	}
	return SOAP_OK;
}

/**
 * @brief Get Services
 * @This message contains a request for GetServices.
 *
 * @param Request	: _tds__GetServices structure contains a request for GetServices.
			  Returns a collection of device services and their available capabilities
 * @param Response	: _tds__GetServicesResponse contains the requested device capabilities.
 * @return               : On success - SOAP_OK.
 */

int  __tds__GetServices(struct soap* soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{

	char _IPAddr[INFO_LENGTH];
	int i = 0;
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	tds__GetServicesResponse->__sizeService = 1;
	if(!tds__GetServices->IncludeCapability)
	{
        tds__GetServicesResponse->Service = (struct tds__Service *)soap_malloc(soap, sizeof(struct tds__Service));
		tds__GetServicesResponse->__sizeService = 1;
		tds__GetServicesResponse->Service[0].XAddr = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		tds__GetServicesResponse->Service[0].Namespace = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetServicesResponse->Service[0].Namespace, "http://www.onvif.org/ver10/events/wsdl");
		strcpy(tds__GetServicesResponse->Service[0].XAddr, _IPAddr);
		tds__GetServicesResponse->Service[0].Capabilities = NULL;
		tds__GetServicesResponse->Service[0].Version = (struct ns2__OnvifVersion *)soap_malloc(soap, sizeof(struct ns2__OnvifVersion));
		tds__GetServicesResponse->Service[0].Version->Major = 0;
		tds__GetServicesResponse->Service[0].Version->Minor = 3;
		tds__GetServicesResponse->Service[0].__any = (char **)soap_malloc(soap, 2*sizeof(char *));
		tds__GetServicesResponse->Service[0].__any[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetServicesResponse->Service[0].__any[0],"Face Detection ");
		tds__GetServicesResponse->Service[0].__any[1] = (char *)soap_malloc(soap,sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetServicesResponse->Service[0].__any[1],"Motion Detection ");
		tds__GetServicesResponse->Service[0].__size = NULL;
		tds__GetServicesResponse->Service[0].__anyAttribute = NULL;
		return SOAP_OK;
	}
	else
	{
		tds__GetServicesResponse->Service = (struct tds__Service *)soap_malloc(soap, sizeof(struct tds__Service));
		tds__GetServicesResponse->__sizeService = 1;
		tds__GetServicesResponse->Service[0].Namespace = NULL;
		tds__GetServicesResponse->Service[0].XAddr = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetServicesResponse->Service[0].XAddr, _IPAddr);
		tds__GetServicesResponse->Service[0].Capabilities = NULL;
		tds__GetServicesResponse->Service[0].Version = (struct ns2__OnvifVersion *)soap_malloc(soap, sizeof(struct ns2__OnvifVersion));
		tds__GetServicesResponse->Service[0].Version->Major = 0;
		tds__GetServicesResponse->Service[0].Version->Minor = 3;
		tds__GetServicesResponse->Service[0].__size = NULL;
		tds__GetServicesResponse->Service[0].__any = NULL;
		tds__GetServicesResponse->Service[0].__anyAttribute = NULL;

	return SOAP_OK;
	}
}

/**
 * @brief Get Service Capabilities
 * @This message contains a request for GetServiceCapabilities.
 *
 * @param Request	: _tds__GetServiceCapabilities structure contains a request for GetServiceCapabilities.
			  Returns a capailities of device services
 * @param Response	: _tds__GetServicesResponse contains the requested device capabilities.
 * @return               : On success - SOAP_OK.
 */


int  __tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{


	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	char _IPv4Address[LARGE_INFO_LENGTH];
	sprintf(_IPv4Address, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	tds__GetServiceCapabilitiesResponse->Capabilities = (struct tds__DeviceServiceCapabilities *)soap_malloc(soap, sizeof(struct tds__DeviceServiceCapabilities));
	/* NETWORK */
	tds__GetServiceCapabilitiesResponse->Capabilities->Network = (struct tds__NetworkCapabilities *)soap_malloc(soap, sizeof(struct tds__NetworkCapabilities));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP= (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP = (int *)soap_malloc(soap, sizeof(int));
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP= &_false;
	*tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP= 3;


	/* SYSTEM */
	tds__GetServiceCapabilitiesResponse->Capabilities->System = (struct tds__SystemCapabilities *)soap_malloc(soap, sizeof(struct tds__SystemCapabilities));
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryResolve = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->RemoteDiscovery = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade = &_true;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging = &_true;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup = &_false;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade = &_true;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging = &_true;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation = &_true;


	/* SECURITY */
	tds__GetServiceCapabilitiesResponse->Capabilities->Security = (struct tds__SecurityCapabilities *)soap_malloc(soap, sizeof(struct tds__SecurityCapabilities));
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e0 = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e1 = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e2 = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->OnboardKeyGeneration = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->AccessPolicyConfig = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->Dot1X = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->X_x002e509Token = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SAMLToken = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->KerberosToken = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RELToken = FALSE;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SupportedEAPMethods = NULL;
	tds__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	tds__GetServiceCapabilitiesResponse->Capabilities->__any  = NULL;
	return SOAP_OK;
}

/**
	SetHostnameFromDHCP
**/
int __tds__SetHostnameFromDHCP(struct soap* soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{

	return SOAP_OK;
}
/**
 * @brief Get Host Name
 * @This operation is used by an endpoint to get the hostname from a device.
 *
 "FromDHCP" True if the hostname is obtained via DHCP
 "Name" The host name.In case of DHCP the host name has been obtained from the DHCP server.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults
 */
int __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _Hostname[LARGE_INFO_LENGTH];
	strcpy(_Hostname, oSysInfo->lan_config.hostname);
	tds__GetHostnameResponse->HostnameInformation = (struct tt__HostnameInformation *)soap_malloc(soap, sizeof(struct tt__HostnameInformation));
	//host name is not from dhcp, so fix this as FALSE
	tds__GetHostnameResponse->HostnameInformation->FromDHCP = FALSE;//oSysInfo->lan_config.net.dhcp_enable;
	tds__GetHostnameResponse->HostnameInformation->Name = (char *) soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	tds__GetHostnameResponse->HostnameInformation->Extension = NULL;
	strcpy(tds__GetHostnameResponse->HostnameInformation->Name, _Hostname);
	return SOAP_OK;
}

/**
 * @brief Set Host Name
 * @This operation is used to set the hostname on device.
 *
 * @param Request	: _tds__SetHostname structures contain host name.
 * @param Response	: _tds__SetHostnameResponse structure contain response. in this its an empity message.
 * @return               : On success - SOAP_OK.
 On failure - The requested hostname cannot be accepted by the device.
 */
int __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	int ret;
	hostname_tmp hostname_tm;

	strcpy(hostname_tm.hostname_t, tds__SetHostname->Name);
	ret = checkhostname(hostname_tm.hostname_t);
	if(ret != 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidHostname");
		return SOAP_FAULT;
	}
	ControlSystemData(SFIELD_SET_HOSTNAME, (void *)&hostname_tm, sizeof(hostname_tm));

	return SOAP_OK;
}

/**
 * @brief Get DNS
 * @This operation gets the following DNS settings from a device:
 -FromDHCP
 -Size Search Domain
 -SearchDomain
 -Size DNS From DHCP
 -Size DNS Manual
 *
 * @param Request	: _tds__GetDNS structures contains an an empty message.
 * @param Response	: _tds__GetDNSResponse structure contains DNS credentials.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults.
 */
int __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	struct sockaddr_in dns_ip;
	char _dns[LARGE_INFO_LENGTH];
	SysInfo* oSysInfo = GetSysInfo();
	if(oSysInfo->lan_config.net.dns.s_addr != net_get_dns())
	{
		oSysInfo->lan_config.net.dns.s_addr = net_get_dns();
	}
	dns_ip.sin_addr.s_addr = oSysInfo->lan_config.net.dns.s_addr;
	strcpy(_dns, inet_ntoa(dns_ip.sin_addr));

	tds__GetDNSResponse->DNSInformation = (struct tt__DNSInformation *)soap_malloc(soap, sizeof(struct tt__DNSInformation));
	tds__GetDNSResponse->DNSInformation->FromDHCP = oSysInfo->lan_config.net.dhcp_enable;
	tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 1;
	tds__GetDNSResponse->DNSInformation->SearchDomain = (char **)soap_malloc(soap, sizeof(char *));
	*tds__GetDNSResponse->DNSInformation->SearchDomain = domainname;
	tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 0;
	tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 0;
	tds__GetDNSResponse->DNSInformation->Extension = NULL;

	if((tds__GetDNSResponse->DNSInformation->FromDHCP) == 1)
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 1;
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->Type = 0;
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(*tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address, _dns);
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv6Address = NULL;
	}
	else
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 1;
		tds__GetDNSResponse->DNSInformation->DNSManual = ((struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress)));
		tds__GetDNSResponse->DNSInformation->DNSManual->Type = 0;
		tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
		strcpy(*tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address, _dns);
		tds__GetDNSResponse->DNSInformation->DNSManual->IPv6Address = NULL;
	}
	return SOAP_OK;
}

/**
 * @brief Set DNS
 * @brief This operation sets  the following DNS settings on a device.
 -FromDHCP
 -SearchDomain
 -DNSManual
 *
 * @param Request	: _tds__SetDNS structures contains DNS credentials.
 * @param Response	: _tds__SetDNSResponse structure contains response. in this operation its an empity message.

 * @return               : On success - SOAP_OK.
 On failure - The suggested IPv6 address is invalid. or
 The suggested IPv4 address is invalid.

 */
int __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
	struct sockaddr_in dns_ip;
	int sys_dhcp = oget_dhcpstatus();
	int value = tds__SetDNS->FromDHCP;
	char _IPv4Address[SMALL_INFO_LENGTH];

	if(tds__SetDNS->SearchDomain)
	{
		strcpy(domainname, *(tds__SetDNS->SearchDomain));
	}
	if (sys_dhcp != value)
	{
		oset_dhcpstatus(value);
		if(value == 1)
		{
			return SOAP_OK;
		}
	}
	if(tds__SetDNS->__sizeSearchDomain)
	{
		tempdnssearchdomainsize = tds__SetDNS->__sizeSearchDomain;
	}
	else
	{
		tempdnssearchdomainsize = 0;
	}

	if(tds__SetDNS->DNSManual == NULL)
	{
		return SOAP_OK;
	}

	if(tds__SetDNS->DNSManual->Type != 0)
	{
		onvif_fault(soap,"ter:NotSupported", "ter:InvalidIPv6Address");
		return SOAP_FAULT;
	}

	strcpy(_IPv4Address, *tds__SetDNS->DNSManual->IPv4Address);
	if(isValidIp4(_IPv4Address) == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
		return SOAP_FAULT;
	}
	inet_aton(_IPv4Address, &dns_ip.sin_addr);
	ControlSystemData(SFIELD_SET_DNS, (void *)&dns_ip.sin_addr.s_addr, sizeof(dns_ip.sin_addr.s_addr));
	ControlSystemData(SFIELD_SET_DHCPC_ENABLE, (void *)&value, sizeof(value));

	return SOAP_OK;
}


/**
 * @brief Get NTP
 * @This operation gets the following NTP settings from a device:
 -Type (IPv4Address/IPv6Address)
 -IPv4Address
 -IPv6Address
 -DNSname
 *
 * @param Request	:_tds__GetNTP structures contain an empty message.
 * @param Response	:_tds__GetNTPResponse structure contains NTP credentials.

 * @return               : On success - SOAP_OK.
 : On failure - No command specific faults

 */
int __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _NTPhostname[MID_INFO_LENGTH];
	struct hostent *host;
	struct in_addr ip;
	char *_IPv4Address;
	char *_NTP = oSysInfo->lan_config.net.ntp_server;
	int dhcp_status = oSysInfo->lan_config.net.dhcp_enable;
	ControlSystemData(SFIELD_GET_SNTP_FQDN, (void *)_NTPhostname, sizeof(_NTPhostname));
	host = gethostbyname(_NTPhostname);
	if(host != NULL)
	{
		memcpy((char *)&ip, host->h_addr_list[0], sizeof(ip));
		_IPv4Address = inet_ntoa(ip);
	} else {
		_IPv4Address = "140.112.126.36"; //NTP_SERVER_DEFAULT  "tw.pool.ntp.org"
	}
	tds__GetNTPResponse->NTPInformation = (struct tt__NTPInformation*)soap_malloc(soap, sizeof(struct tt__NTPInformation));
	tds__GetNTPResponse->NTPInformation->FromDHCP = dhcp_status;
	if(dhcp_status == 1) //FromDHCP
	{
		tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP = 1;
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP = (struct tt__NetworkHost*)soap_malloc(soap, sizeof(struct tt__NetworkHost));
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->Type = IPV4_NTP;
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv4Address[0], _IPv4Address);
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->IPv6Address = NULL;
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->DNSname = NULL;
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP->Extension = NULL;

		tds__GetNTPResponse->NTPInformation->__sizeNTPManual = 0;
		tds__GetNTPResponse->NTPInformation->NTPManual = NULL;

	}
	else // NTPManual || Static
	{
		tds__GetNTPResponse->NTPInformation->__sizeNTPManual = 1;
		tds__GetNTPResponse->NTPInformation->NTPManual = ((struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress)));
		tds__GetNTPResponse->NTPInformation->NTPManual->Type = IPV4_NTP;
		tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address[0], _IPv4Address);
		tds__GetNTPResponse->NTPInformation->NTPManual->IPv6Address = NULL;
		tds__GetNTPResponse->NTPInformation->NTPManual->DNSname = NULL;
		tds__GetNTPResponse->NTPInformation->NTPManual->Extension = NULL;

		tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP = 0;
		tds__GetNTPResponse->NTPInformation->NTPFromDHCP = NULL;

	}
	return SOAP_OK;
}

/**
 * @brief Set NTP
 * @This operation sets the following  NTP settings on a device:
 -FromDHCP
 -NTPManual
 *
 * @param Request	: _tds__SetNTP structures contains
 "FromDHCP": True if the NTP servers are obtained via DHCP.
 "NTPManual": A list of manually given NTP servers when they not are obtained via DHCP.
 * @param Response:      :_tds__SetNTPResponse structurecontains an empty message.
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 8.2.6)
 */
int __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
	struct sockaddr_in sa;
	char host[LARGE_INFO_LENGTH];
	char service[SMALL_INFO_LENGTH];
	int s;
	int value = tds__SetNTP->FromDHCP;

	if (value == TRUE)
	{
		onvif_fault(soap,"ter:NotSupported", "ter:SetDHCPNotAllowed");
		return SOAP_FAULT;
	}
	if (tds__SetNTP->NTPManual->IPv6Address != NULL)
	{
		onvif_fault(soap,"ter:NotSupported", "ter:IPv6AddressNotAllowed");
                return SOAP_FAULT;
	}
	if(tds__SetNTP->NTPManual->IPv4Address != NULL)
	{
		if(isValidIp4(tds__SetNTP->NTPManual->IPv4Address[0]) == 0)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
			return SOAP_FAULT;
		}

		sa.sin_family = AF_INET;
		inet_aton(tds__SetNTP->NTPManual->IPv4Address[0], &sa.sin_addr.s_addr);
		s = getnameinfo(&sa, sizeof(sa), host, sizeof(host), service, sizeof(service), NI_NAMEREQD);
		if(isValidHostname(host) == 0)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidDnsName");
			return SOAP_FAULT;
		}
		if(tds__SetNTP->NTPManual)
		{
			ControlSystemData(SFIELD_SET_SNTP_SERVER, (void *)host, strlen(host));
		}
	}
	return SOAP_OK;
}

int __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:DynamicDNSNotSupported");
	return SOAP_FAULT;
}

int __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:DynamicDNSNotSupported");
	return SOAP_FAULT;
}


/**
 * @brief Get Network Interfaces
 * @This operation gets the following network interface configuration from a device:
 -Network interface token
 -Name
 -Hardware Id
 -MTU
 -IpAddress
 *
 * @param Request	: _tds__GetNetworkInterfaces structures contains an empty message.
 * @param Response	: _tds__GetNetworkInterfacesResponse structure contains an array of device network interfaces.
 * @return               : On success - SOAP_OK.
 : On failure - No command specific faults!

 */
int __tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	char mac[MACH_ADDR_LENGTH];
	char _mac[LARGE_INFO_LENGTH];
	char _IPAddr[LARGE_INFO_LENGTH];
	char _IPAddr_local[LARGE_INFO_LENGTH];
	NET_IPV4 ip;
	NET_IPV4 ip_local;

	ip.int32 = net_get_ifaddr(ETH_NAME);
	ip_local.int32 = net_get_ifaddr(ETH_NAME_LOCAL);
	net_get_hwaddr(ETH_NAME, mac);
	sprintf(_mac, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	sprintf(_IPAddr, "%d.%d.%d.%d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	sprintf(_IPAddr_local, "%d.%d.%d.%d", ip_local.str[0], ip_local.str[1], ip_local.str[2], ip_local.str[3]);
	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces = (struct tt__NetworkInterface *)soap_malloc(soap, sizeof(struct tt__NetworkInterface));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->token = ETH_NAME;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled = TRUE;

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info = (struct tt__NetworkInterfaceInfo *)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceInfo));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name = ETH_NAME;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress = (char *)soap_malloc(soap, LARGE_INFO_LENGTH);
	strcpy(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress,_mac);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = (int *)soap_malloc(soap, sizeof(int));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU[0] = 1500;

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link = NULL;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4 = (struct tt__IPv4NetworkInterface *)soap_malloc(soap, sizeof(struct tt__IPv4NetworkInterface));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled = TRUE;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config = (struct tt__IPv4Configuration *)soap_malloc(soap, sizeof(struct tt__IPv4Configuration));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address = (char *)soap_malloc(soap, sizeof(char) * MID_INFO_LENGTH);
	strcpy(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address,_IPAddr);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->Address = _IPAddr_local;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP = (struct tt__PrefixedIPv4Address *)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->Address = (char *)soap_malloc(soap, LARGE_INFO_LENGTH);
	strcpy(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->Address, _IPAddr);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP->PrefixLength = 24;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP = FALSE;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6 = NULL;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Set Network Interface
 * @This operation sets the network interface configuration on a device.
 *
 * @param Request	: _tds__SetNetworkInterfaces structures contains:
 -InterfaceToken
 -NetworkInterface
 * @param Response	: _tds__SetNetworkInterfacesResponse structure contains:
 "RebootNeeded": An indication if a reboot is needed in case of changes in the network settings.

 * @return               : On success - SOAP_OK.
 : On failure - an Appropriate fault code(ONVIF Core Spec. - Ver. 2.00 8.2.10)
 */
int __tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
	struct in_addr ipaddr, sys_ip;
	int value;
	int ret;
	SysInfo* oSysInfo = GetSysInfo();
	int local_dhcp_status = oSysInfo->lan_config.net.dhcp_enable;

	if(tds__SetNetworkInterfaces->InterfaceToken != NULL)
	{
		if(strcmp(tds__SetNetworkInterfaces->InterfaceToken, ETH_NAME))
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidNetworkInterface");
			return SOAP_FAULT;
		}
	}


	if(tds__SetNetworkInterfaces->NetworkInterface->MTU != NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:SettingMTUNotSupported");
		return SOAP_FAULT;
	}
	if(tds__SetNetworkInterfaces->NetworkInterface->Link != NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:SettingLinkValuesNotSupported");
		return SOAP_FAULT;
	}
	if(tds__SetNetworkInterfaces->NetworkInterface->IPv6 != NULL)
	{
		if(tds__SetNetworkInterfaces->NetworkInterface->IPv6->Enabled == 1)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:IPv6NotSupported");
			return SOAP_FAULT;
		}
	}
	if(tds__SetNetworkInterfaces->NetworkInterface->IPv4 == NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:IPv4ValuesMissing");
		return SOAP_FAULT;
	}
	if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Enabled != NULL)
	{
		if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual != NULL)
		{
			if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address != NULL)
			{
				if(isValidIp4(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address) == 0) // Check IP address
				{
					onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidIPv4Address");
					return SOAP_FAULT;
				}
				if (ipv4_str_to_num(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address, &ipaddr) == 0)
				{
					return SOAP_FAULT;
				}
			}
			if(tds__SetNetworkInterfaces->InterfaceToken != NULL)
			{
				if ((sys_ip.s_addr = net_get_ifaddr(tds__SetNetworkInterfaces->InterfaceToken)) == -1)
				{
					return SOAP_FAULT;
				}
				if (sys_ip.s_addr != ipaddr.s_addr)
				{
					if (net_set_ifaddr(tds__SetNetworkInterfaces->InterfaceToken, ipaddr.s_addr) < 0)
					{
						return SOAP_FAULT;
					}
					ret = ControlSystemData(SFIELD_SET_IP, (void *)&ipaddr.s_addr, sizeof(ipaddr.s_addr));
				}
			}
		}
		else
		{
			if(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP != NULL)
			{
				if(*(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP))
				{
					if(local_dhcp_status != 1)
					{
						net_enable_dhcpcd();
					}
					value = 1;
				}
				else
				{
					if(local_dhcp_status != 0)
					{
						net_disable_dhcpcd();
					}
					value = 0;
				}
			}
			if(local_dhcp_status != value)
			{
				ControlSystemData(SFIELD_SET_DHCPC_ENABLE, (void *)&value, sizeof(value));
			}
		}
	}
	tds__SetNetworkInterfacesResponse->RebootNeeded = TRUE;
	return SOAP_OK;
}

/**
 * @brief Get Network Protocol
 * @This operation gets defined network protocols from a device.
 *
 * @param Request	:_tds__GetNetworkProtocols structure contains an empty message.
 * @param Response	:_tds__GetNetworkProtocols structure members contains an array of defined protocols supported by the device.
 * @return               : On success - SOAP_OK.
 : On failure - No command specific faults

 */
int __tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
	int _http_port = 80;
	int _https_port = 81;
	int _h2641_port = 8556;
	int _h2642_port = 8557;
	int _mpeg41_port = 8553;
	int _mpeg42_port = 8554;
	int _mjpeg_port = 8555;

	tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = 2;
	tds__GetNetworkProtocolsResponse->NetworkProtocols = (struct tt__NetworkProtocol *)soap_malloc(soap,tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols * sizeof(struct tt__NetworkProtocol));

	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Name = HTTP;	// HTTP = 0, HTTPS = 1, RTSP = 2
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Enabled = 1;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].__sizePort = 1;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = (int *)soap_malloc(soap, sizeof(int));
	*tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = _http_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Extension = NULL;

	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Name = RTSP;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Enabled = 1;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].__sizePort = 5;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port = (int *)soap_malloc(soap, tds__GetNetworkProtocolsResponse->NetworkProtocols[1].__sizePort * sizeof(int));
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port[0] = _mpeg41_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port[1] = _mpeg42_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port[2] = _mjpeg_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port[3] = _h2641_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port[4] = _h2642_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Set Network Protocol
 * @This operation configures defined network protocols on a device.
 *
 *  @param Request	:_tds__SetNetworkProtocols structure contains values to be configure one or more defined network protocols supported by the device.
 *  @param Response	:_tds__SetNetworkProtocolsResponse structure contains members an empty message.
 *  @return              : On success - SOAP_OK.
 : On failure - The supplied network service is not supported.

 */

int __tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
	if(tds__SetNetworkProtocols->NetworkProtocols->Name == HTTP || tds__SetNetworkProtocols->NetworkProtocols->Name == HTTPS || tds__SetNetworkProtocols->NetworkProtocols->Name == RTSP) // HTTP = 0
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ServiceNotSupported");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

/**
 * @brief Get Network Default Gateway
 * @This operation gets the default gateway settings from a device.
 *
 * @param Request	: _tds__GetNetworkDefaultGateway structure contains an empty message.
 * @param Response	:_tds__GetNetworkDefaultGatewayResponse structure contains:
 -IPv4Address	: The default IPv4 gateway address(es).
 -IPv6Address	: The default IPv6 gateway address(es).

 * @return               : On success - SOAP_OK.
 On failure - No command specific faults!

 */
int __tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	struct sockaddr_in gateway_ip;
	SysInfo* oSysInfo = GetSysInfo();
	char _GatewayAddress[LARGE_INFO_LENGTH];
	gateway_ip.sin_addr.s_addr = oSysInfo->lan_config.net.gateway.s_addr;
	strcpy(_GatewayAddress, inet_ntoa(gateway_ip.sin_addr));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway = ((struct tt__NetworkGateway *)soap_malloc(soap,sizeof(struct tt__NetworkGateway*)));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv6Address = 0;
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0] = (char *)soap_malloc(soap, INFO_LENGTH);
	strcpy(*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address, _GatewayAddress);
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv6Address = NULL;
	return SOAP_OK;
}

/**
 * @brief Set Network Default gateway
 * @This operation sets the default gateway settings on a device.
 *
 * @param Request	: _tds__SetNetworkDefaultGateway structure contains:
 -IPv4Address	: The default IPv4 gateway address(es).
 -IPv6Address	: The default IPv6 gateway address(es).
 * @param Response	: _tds__SetNetworkDefaultGatewayResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 On failure - The supplied gateway address was invalid.

 */
int __tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
	struct in_addr ipaddr;
	char _IPv4Address[LARGE_INFO_LENGTH];
	if(tds__SetNetworkDefaultGateway->__sizeIPv6Address)
	{
		onvif_fault(soap,"ter:NotSupported", "ter:InvalidIPv6Address");
		return SOAP_FAULT;
	}
	if(!tds__SetNetworkDefaultGateway->__sizeIPv4Address)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidGatewayAddress");
		return SOAP_FAULT;
	}
	strcpy(_IPv4Address, *tds__SetNetworkDefaultGateway->IPv4Address);
	if(isValidIp4(_IPv4Address) == 0) // Check IP address
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidGatewayAddress");
		return SOAP_FAULT;
	}
	ipv4_str_to_num(_IPv4Address, &ipaddr);
	ControlSystemData(SFIELD_SET_GATEWAY, (void *)&ipaddr.s_addr, sizeof(ipaddr.s_addr));
	return SOAP_OK;
}

/**
 * @brief Get Zero Configuration.
 * @This operation gets the zero-configuration from a device.
 *
 * @param Request        : _tds__GetZeroConfiguration contains an empty message.
 * @param Response       : _tds__GetZeroConfiguration contains:
 "InterfaceToken" The token of the network interface
 "Enabled" If zero configuration is enabled or not.
 "Addresses" The IPv4 zero configuration address(es).
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults!

 */
int __tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	char _IPAddr[LARGE_INFO_LENGTH];
	NET_IPV4 ip;

	ip.int32 = net_get_ifaddr(ETH_NAME_LOCAL);
	sprintf(_IPAddr, "%d.%d.%d.%d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

	tds__GetZeroConfigurationResponse->ZeroConfiguration = (struct tt__NetworkZeroConfiguration *)soap_malloc(soap, sizeof(struct tt__NetworkZeroConfiguration));

	tds__GetZeroConfigurationResponse->ZeroConfiguration->InterfaceToken = ETH_NAME;
	tds__GetZeroConfigurationResponse->ZeroConfiguration->Enabled = 1;
	tds__GetZeroConfigurationResponse->ZeroConfiguration->__sizeAddresses = 1;
	tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses = (char **)soap_malloc(soap, sizeof(char *));
	tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[1] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[0], _IPAddr);
	strcpy(tds__GetZeroConfigurationResponse->ZeroConfiguration->Addresses[1], LINK_LOCAL_ADDR);
	tds__GetZeroConfigurationResponse->ZeroConfiguration->Extension = NULL;
	return SOAP_OK;
}

int __tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(tds__SetZeroConfiguration->InterfaceToken);
	if(ip.int32 == -1)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidNetworkInterface");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

int __tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:IPAddressFilterNotSupported");
	return SOAP_FAULT;
}

int __tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:IPAddressFilterNotSupported");
	return SOAP_FAULT;
}

int __tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:IPAddressFilterNotSupported");
	return SOAP_FAULT;
}

int __tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:IPAddressFilterNotSupported");
	return SOAP_FAULT;
}

int __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AccessPolicyNotSupported");
	return SOAP_FAULT;
}

int __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AccessPolicyNotSupported");
	return SOAP_FAULT;
}

int __tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetPkcs10Request(struct soap *soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:Pkcs10NotSupported");
	return SOAP_FAULT;
}

int __tds__LoadCertificates(struct soap *soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:RelayNotSupported");
	return SOAP_FAULT;
}

int __tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{

	SysInfo *oSysInfo = GetSysInfo();
	relay_conf relays_conf_in;
	int i;
	int Ptoken_exist = NOT_EXIST;
	int ret;
	if(tds__SetRelayOutputSettings->RelayOutputToken != NULL)
	{
		for(i = 0;i < oSysInfo->nrelays; i++)
		{
			if(strcmp(tds__SetRelayOutputSettings->RelayOutputToken, oSysInfo->onvif_relay[i].onvif_relay_in.token) == 0)
			{
				Ptoken_exist = EXIST;
				break;
			}
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:RelayToken");
		return SOAP_FAULT;
	}

	relays_conf_in.position = i;
	strcpy(relays_conf_in.relay_conf_in_t.token_t, tds__SetRelayOutputSettings->RelayOutputToken);
	relays_conf_in.relay_conf_in_t.relaymode_t = tds__SetRelayOutputSettings->Properties->Mode;
	relays_conf_in.relay_conf_in_t.delaytime_t = tds__SetRelayOutputSettings->Properties->DelayTime;
	relays_conf_in.relay_conf_in_t.idlestate_t = tds__SetRelayOutputSettings->Properties->IdleState;
	ret = ControlSystemData(SFIELD_SET_RELAYS_CONF, (void *)&relays_conf_in, sizeof(relays_conf_in));

	return SOAP_OK;
}

int __tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	SysInfo *oSysInfo  = GetSysInfo();
	int i = 0;
	relay_conf  relay;
	int token_exist = NOT_EXIST;
	for(i = 0 ;i< oSysInfo->nrelays; i++)
	{
		if(strcmp(tds__SetRelayOutputState->RelayOutputToken, oSysInfo->onvif_relay[i].onvif_relay_in.token) == 0)
		{
			token_exist = EXIST;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal ", "ter:RelayToken");
		return SOAP_FAULT;
	}
	relay.logicalstate_t = tds__SetRelayOutputState->LogicalState ;
	ControlSystemData(SFIELD_SET_RELAY_LOGICALSTATE, (void*) &relay, sizeof(relay));
	return SOAP_OK;


}

int __tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AuxiliaryDataNotSupported");
	return SOAP_FAULT;
}

int __tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__LoadCertificateWithPrivateKey(struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__GetCertificateInformation(struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:CertificateNotSupported");
	return SOAP_FAULT;
}

int __tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:EAPMethodNotSupported");
	return SOAP_FAULT;
}

int __tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:EAPMethodNotSupported");
	return SOAP_FAULT;
}

int __tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:EAPMethodNotSupported");
	return SOAP_FAULT;
}

int __tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:EAPMethodNotSupported");
	return SOAP_FAULT;
}

int __tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:EAPMethodNotSupported");
	return SOAP_FAULT;
}

int __tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:InvalidDot11");
	return SOAP_FAULT;
}

int __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:InvalidDot11");
	return SOAP_FAULT;
}

int __tds__ScanAvailableDot11Networks(struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:InvalidDot11");
	return SOAP_FAULT;
}

int __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:SystemUrisNotSupported");
	return SOAP_FAULT;
}

int __tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:FirmwareUpgradeNotSupported");
	return SOAP_FAULT;
}

int __tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:SystemRestoreNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief This operation requests the imaging setting for a video source on the device.
 *
 * @param Request	: _timg__GetImagingSettings structure variable members contains a
 reference to the VideoSource for which the ImagingSettings should be requested.
 * @param Response	: _timg__GetImagingSettingsResponse structure variables members contains the
 ImagingSettings for the VideoSource thatwas requested.
 * @return returns SOAP_OK on success.
 */

int __timg__GetImagingSettings(struct soap *soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int i = 0,flag = 0;
	int token_exist = 0;
	int imaging_exist = 0;
	float _Brightness;
	float _Saturation;
	float _Contrast;
	float _Sharpness;
	float _WideDynamicRange;
	int _Backlight;
	float _BacklightCompensationLevel;
	int _WhiteBalance;

	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(timg__GetImagingSettings->VideoSourceToken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken) == 0)
		{
			token_exist = EXIST;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(timg__GetImagingSettings->VideoSourceToken, oSysInfo->imaging_conf[i].Isourcetoken) == 0)
		{
			imaging_exist = EXIST;
			break;
		}
	}
	if(!imaging_exist)
	{
		onvif_fault(soap, "ter:ActionNotSupported", "ter:NoImagingForSource");
		return SOAP_FAULT;
	}
	_Brightness = (float)(oSysInfo->imaging_conf[i].Brightness );
	_Saturation = (float)(oSysInfo->imaging_conf[i].Saturation );
	_Contrast = (float)(oSysInfo->imaging_conf[i].Contrast );
	_Sharpness = (float)(oSysInfo->imaging_conf[i].Sharpness );
	_WideDynamicRange = oSysInfo->imaging_conf[i].WideDynamicMode;
	_Backlight = oSysInfo->imaging_conf[i].BacklightCompensationMode;
	_BacklightCompensationLevel = oSysInfo->imaging_conf[i].BackLightLevel;
	_WhiteBalance = oSysInfo->imaging_conf[i].WhiteBalanceMode;

	timg__GetImagingSettingsResponse->ImagingSettings = (struct tt__ImagingSettings20 *)soap_malloc(soap, sizeof(struct tt__ImagingSettings20));
	timg__GetImagingSettingsResponse->ImagingSettings->Brightness = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness = _Brightness;
	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = _Saturation;
	timg__GetImagingSettingsResponse->ImagingSettings->Contrast = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast = _Contrast;
	timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = (int *)soap_malloc(soap, sizeof(int));
	*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = oSysInfo->imaging_conf[i].IrCutFilterMode; // dummy //{onv__IrCutFilterMode__ON = 0, onv__IrCutFilterMode__OFF = 1, onv__IrCutFilterMode__AUTO = 2}
	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = _Sharpness;
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation = (struct tt__BacklightCompensation20 *)soap_malloc(soap, sizeof(struct tt__BacklightCompensation20));
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = _Backlight;//Streaming_onvif->BacklightCompensationMode;  //{onv__BacklightCompensationMode__OFF = 0, onv__BacklightCompensationMode__ON = 1}
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level =(float *) soap_malloc(soap, sizeof(float));//float
	*timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level =_BacklightCompensationLevel;//float
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->Focus = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange = (struct tt__WideDynamicRange20 *)soap_malloc(soap, sizeof(struct tt__WideDynamicRange20));
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode = _WideDynamicRange;   //{onv__WideDynamicMode__OFF = 0, onv__WideDynamicMode__ON = 1}
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance = (struct tt__WhiteBalance20 *)soap_malloc(soap, sizeof(struct tt__WhiteBalance20));
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Mode = _WhiteBalance;	//{onv__WhiteBalanceMode__AUTO = 0, onv__WhiteBalanceMode__MANUAL = 1}
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->Extension = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain = (float *) soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CrGain = oSysInfo->imaging_conf[i].CrGain;
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain = (float *) soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance->CbGain = oSysInfo->imaging_conf[i].CbGain;
	timg__GetImagingSettingsResponse->ImagingSettings->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Set Imaging Settings
 * @This operation set the following imaging setting for a video source on the device:
 -Brightness
 -ColorSaturation
 -Contrast
 -Sharpness
 *
 * @param Request	: _timg__SetImagingSettings structure contains a reference to the VideoSource and ImagingSettings that should be set.
 * @param Response	: _timg__SetImagingSettingsResponse structure contains an empity message

 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 10.1.2)
 */
int __timg__SetImagingSettings(struct soap *soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	Imaging_Conf_tmp  imaging_conf_tmp;
	memset(&imaging_conf_tmp,0,sizeof(imaging_conf_tmp));
	int i,token_exist = 0;
	int imaging_exist;
	__u8 *_brightness;
	__u8 *_saturation;
	__u8 *_contrast;
	__u8 *_sharpness;
	__u8 *_pBacklightCompensationMode;
	float *_pBacklightCompensationModeLevel;
	int _br;
	int _sa;
	int _co;
	int _sh;
	int	_blc_mode;
	int	_blc_level;
	for(i = 0; i< oSysInfo->nprofile ;i++)
	{
		if(strcmp(timg__SetImagingSettings->VideoSourceToken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken) == 0)
		{
			token_exist = EXIST;
			imaging_conf_tmp.position = i;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	for(i = 0; i< oSysInfo->nprofile; i++)
	{
		if(strcmp(timg__SetImagingSettings->VideoSourceToken, oSysInfo->imaging_conf[i].Isourcetoken) == 0)
		{
			imaging_exist = EXIST;
			break;
		}
	}
	if(!imaging_exist)
	{
		onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
		return SOAP_FAULT;
	}
	strcpy(imaging_conf_tmp.imaging_conf_in.Isourcetoken_t, timg__SetImagingSettings->VideoSourceToken);
	_brightness = (unsigned char *)malloc(sizeof(unsigned char));
	_saturation = (unsigned char *)malloc(sizeof(unsigned char));
	_contrast = (unsigned char *)malloc(sizeof(unsigned char));
	_sharpness = (unsigned char *)malloc(sizeof(unsigned char));
	_pBacklightCompensationMode = (unsigned char *)malloc(sizeof(unsigned char));
	_pBacklightCompensationModeLevel = (float *)malloc(sizeof(float));
	if(timg__SetImagingSettings->ImagingSettings->Brightness != NULL)
	{
		*_brightness = (unsigned char)*timg__SetImagingSettings->ImagingSettings->Brightness;
		_br = *timg__SetImagingSettings->ImagingSettings->Brightness;
		imaging_conf_tmp.imaging_conf_in.Brightness_t = _br;
	}
	if(timg__SetImagingSettings->ImagingSettings->ColorSaturation != NULL)
	{
		*_saturation = (unsigned char)*timg__SetImagingSettings->ImagingSettings->ColorSaturation;
		_sa = *timg__SetImagingSettings->ImagingSettings->ColorSaturation;
		imaging_conf_tmp.imaging_conf_in.Saturation_t = *_saturation;
	}
	if(timg__SetImagingSettings->ImagingSettings->Contrast != NULL)
	{
		*_contrast = (unsigned char)*timg__SetImagingSettings->ImagingSettings->Contrast;
		_co = *timg__SetImagingSettings->ImagingSettings->Contrast;
		imaging_conf_tmp.imaging_conf_in.Contrast_t = _co;

	}
	if(timg__SetImagingSettings->ImagingSettings->Sharpness != NULL)
	{
		*_sharpness = (unsigned char)*timg__SetImagingSettings->ImagingSettings->Sharpness;
		_sh = *timg__SetImagingSettings->ImagingSettings->Sharpness;
		imaging_conf_tmp.imaging_conf_in.Sharpness_t = _sh;
	}
	if(timg__SetImagingSettings->ImagingSettings->BacklightCompensation != NULL)
	{
		*_pBacklightCompensationMode =  timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode;
		*_pBacklightCompensationModeLevel = (float)*timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level;


		_blc_mode = imaging_conf_tmp.imaging_conf_in.BacklightCompensationMode_t
			= timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode;

		if(timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level != NULL)
		{
			_blc_level = imaging_conf_tmp.imaging_conf_in.BackLightLevel_t
				= *timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->Exposure != NULL)
	{
		imaging_conf_tmp.imaging_conf_in.ExposureMode_t = timg__SetImagingSettings->ImagingSettings->Exposure->Mode;
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Priority != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.ExposurePriority_t = *timg__SetImagingSettings->ImagingSettings->Exposure->Priority;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Window != NULL)
		{
			if(timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom != NULL)
			{
				imaging_conf_tmp.imaging_conf_in.bottom_t = *timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom;
			}
			if(timg__SetImagingSettings->ImagingSettings->Exposure->Window->top != NULL)
			{
				imaging_conf_tmp.imaging_conf_in.top_t    = *timg__SetImagingSettings->ImagingSettings->Exposure->Window->top;
			}
			if(timg__SetImagingSettings->ImagingSettings->Exposure->Window->right != NULL)
			{
				imaging_conf_tmp.imaging_conf_in.right_t  = *timg__SetImagingSettings->ImagingSettings->Exposure->Window->right;
			}
			if(timg__SetImagingSettings->ImagingSettings->Exposure->Window->left != NULL)
			{
				imaging_conf_tmp.imaging_conf_in.left_t  = *timg__SetImagingSettings->ImagingSettings->Exposure->Window->left;
			}
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MinExposureTime_t = *timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MaxExposureTime_t = *timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinGain != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MinGain_t         = *timg__SetImagingSettings->ImagingSettings->Exposure->MinGain;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MaxGain_t         = *timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MinIris != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MinIris_t         = *timg__SetImagingSettings->ImagingSettings->Exposure->MinIris;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.MaxIris_t         = *timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.ExposureTime_t    = *timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Gain != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.Gain_t            = *timg__SetImagingSettings->ImagingSettings->Exposure->Gain;
		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Iris != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.Iris_t            = *timg__SetImagingSettings->ImagingSettings->Exposure->Iris;
		}

	}
	if(timg__SetImagingSettings->ImagingSettings->Focus != NULL)
	{
		imaging_conf_tmp.imaging_conf_in.AutoFocusMode_t = timg__SetImagingSettings->ImagingSettings->Focus->AutoFocusMode;
		if(timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.DefaultSpeed_t = *timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed;
		}
		if(timg__SetImagingSettings->ImagingSettings->Focus->NearLimit != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.NearLimit_t    = *timg__SetImagingSettings->ImagingSettings->Focus->NearLimit;
		}
		if(timg__SetImagingSettings->ImagingSettings->Focus->FarLimit != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.FarLimit_t     = *timg__SetImagingSettings->ImagingSettings->Focus->FarLimit;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->IrCutFilter != NULL)
	{
		imaging_conf_tmp.imaging_conf_in.IrCutFilterMode_t = *timg__SetImagingSettings->ImagingSettings->IrCutFilter;
	}
	if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange != 0)
	{
		imaging_conf_tmp.imaging_conf_in.WideDynamicMode_t  = timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode;
		if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level != NULL)
		{
			if(timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level != NULL)
			{
				imaging_conf_tmp.imaging_conf_in.WideDMLevel_t = *timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level;
			}
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->WhiteBalance != 0)
	{
		imaging_conf_tmp.imaging_conf_in.WhiteBalanceMode_t = timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode;
		if(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.CrGain_t = *timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain;
		}
		if(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain != NULL)
		{
			imaging_conf_tmp.imaging_conf_in.CbGain_t = *timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain;
		}
	}

	if(timg__SetImagingSettings->ImagingSettings->Brightness != NULL)
	{
		if(_br < 256 && _br > 0)
		{
			ControlSystemData(SFIELD_SET_BRIGHTNESS, (void *)_brightness, sizeof(*_brightness));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->ColorSaturation != NULL)
	{
		if(_sa < 256 && _sa > 0)
		{
			ControlSystemData(SFIELD_SET_SATURATION, (void *)_saturation, sizeof(*_saturation));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->Contrast != NULL)
	{
		if(_co < 256 && _co > 0)
		{
			ControlSystemData(SFIELD_SET_CONTRAST, (void *)_contrast, sizeof(*_contrast));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->Sharpness != NULL)
	{
		if(_sh < 256 && _sh > 0)
		{
			ControlSystemData(SFIELD_SET_SHARPNESS, (void *)_sharpness, sizeof(*_sharpness));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	{
		if(_blc_mode < 2 && _blc_mode >= 0)
		{
			ControlSystemData(SFIELD_SET_BLC, (void *)_pBacklightCompensationMode, sizeof(*_pBacklightCompensationMode));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level != NULL)
	{
		if(_blc_level < 3 && _blc_level >= 0)
		{
			ControlSystemData(SFIELD_SET_BACKLIGHT, (void *)_pBacklightCompensationModeLevel, sizeof(*_pBacklightCompensationModeLevel));
		}
		else
		{
			onvif_fault(soap,"ter:Action", "ter:ValueExceeded");
			free(_brightness);
			free(_saturation);
			free(_contrast);
			free(_sharpness);
			free(_pBacklightCompensationMode);
			free(_pBacklightCompensationModeLevel);
			return SOAP_FAULT;
		}
	}
	for(i = 0; i< oSysInfo->nprofile ;i++)
	{
		if(strcmp(timg__SetImagingSettings->VideoSourceToken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken) == 0)
		{
			token_exist = EXIST;
			imaging_conf_tmp.position = i;
		}

		if(token_exist)
		{
			ControlSystemData(SFIELD_SET_IMAGING, (void *) &imaging_conf_tmp, sizeof(imaging_conf_tmp));
			break;
		}
	}
	free(_brightness);
	free(_saturation);
	free(_contrast);
	free(_sharpness);
	free(_pBacklightCompensationMode);
	free(_pBacklightCompensationModeLevel);
	return SOAP_OK;
}


int __timg__GetOptions(struct soap *soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i = 0, token_exist;
	int imaging_exist = 0;
	int index = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(timg__GetOptions->VideoSourceToken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken) == 0)
		{
			token_exist = EXIST;
			index = i;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(timg__GetOptions->VideoSourceToken, oSysInfo->imaging_conf[i].Isourcetoken) == 0)
		{
			imaging_exist = EXIST;
			break;
		}
	}
	if(!imaging_exist)
	{
		onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
		return SOAP_FAULT;
	}
	timg__GetOptionsResponse->ImagingOptions =  (struct tt__ImagingOptions20 *)soap_malloc(soap, sizeof(struct tt__ImagingOptions20));
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation = (struct tt__BacklightCompensationOptions20 *)soap_malloc(soap, sizeof(struct tt__BacklightCompensationOptions20));
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->__sizeMode = 1;

	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = (int *) soap_malloc(soap, sizeof(int));
	*timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = _false;
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Level = NULL;
	timg__GetOptionsResponse->ImagingOptions->Brightness = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Contrast = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Brightness->Min = 1;
	timg__GetOptionsResponse->ImagingOptions->Brightness->Max = 255;
	timg__GetOptionsResponse->ImagingOptions->Contrast->Min = 1;
	timg__GetOptionsResponse->ImagingOptions->Contrast->Max = 255;
	timg__GetOptionsResponse->ImagingOptions->Sharpness = (struct tt__FloatRange *)soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Min = 1;
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Max = 225;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min = 1;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max = 255;
	timg__GetOptionsResponse->ImagingOptions->__sizeIrCutFilterModes = 0;
	timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes = (int *)soap_malloc(soap, sizeof(int));
	*timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes = 2;
	timg__GetOptionsResponse->ImagingOptions->Exposure = NULL;
	timg__GetOptionsResponse->ImagingOptions->Focus = NULL;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange = NULL;
	timg__GetOptionsResponse->ImagingOptions->WhiteBalance = NULL;
	timg__GetOptionsResponse->ImagingOptions->Extension = NULL;
	return SOAP_OK;
}

int __timg__Move(struct soap *soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource ");
	return SOAP_FAULT;
}

int __timg__Stop(struct soap *soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

int __timg__GetStatus(struct soap *soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

int __timg__GetMoveOptions(struct soap *soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

int __tls__GetLayout(struct soap *soap, struct _tls__GetLayout *tls__GetLayout, struct _tls__GetLayoutResponse *tls__GetLayoutResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__SetLayout(struct soap *soap, struct _tls__SetLayout *tls__SetLayout, struct _tls__SetLayoutResponse *tls__SetLayoutResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__GetDisplayOptions(struct soap *soap, struct _tls__GetDisplayOptions *tls__GetDisplayOptions, struct _tls__GetDisplayOptionsResponse *tls__GetDisplayOptionsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__GetPaneConfigurations(struct soap *soap, struct _tls__GetPaneConfigurations *tls__GetPaneConfigurations, struct _tls__GetPaneConfigurationsResponse *tls__GetPaneConfigurationsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__GetPaneConfiguration(struct soap *soap, struct _tls__GetPaneConfiguration *tls__GetPaneConfiguration, struct _tls__GetPaneConfigurationResponse *tls__GetPaneConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__SetPaneConfigurations(struct soap *soap, struct _tls__SetPaneConfigurations *tls__SetPaneConfigurations, struct _tls__SetPaneConfigurationsResponse *tls__SetPaneConfigurationsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__SetPaneConfiguration(struct soap *soap, struct _tls__SetPaneConfiguration *tls__SetPaneConfiguration, struct _tls__SetPaneConfigurationResponse *tls__SetPaneConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__CreatePaneConfiguration(struct soap *soap, struct _tls__CreatePaneConfiguration *tls__CreatePaneConfiguration, struct _tls__CreatePaneConfigurationResponse *tls__CreatePaneConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

int __tls__DeletePaneConfiguration(struct soap *soap, struct _tls__DeletePaneConfiguration *tls__DeletePaneConfiguration, struct _tls__DeletePaneConfigurationResponse *tls__DeletePaneConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

/**
 * @brief Get Audio Source
 * @This operation lists all available audio sources of the device.
 *
 * @param Request	: _tmd__GetAudioSource structure contains empty response.
 * @param Response	: _tmd__GetAudioSourceResponse structure contains list of structures describing all available audio sources of the device.
 * @return               : On success - SOAP_OK.
 : On failure - NVT does not support audio.

 */
int __tmd__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	int i;
	SysInfo *oSysInfo = GetSysInfo();
	trt__GetAudioSourcesResponse->__sizeAudioSources = oSysInfo->nprofile;
	trt__GetAudioSourcesResponse->AudioSources = (struct tt__AudioSource *)soap_malloc(soap, sizeof(struct tt__AudioSource)* oSysInfo->nprofile);
	for(i=0; i< oSysInfo->nprofile; i++)
	{
		trt__GetAudioSourcesResponse->AudioSources[i].token = (char *)soap_malloc(soap,  sizeof(char) * INFO_LENGTH);
		strcpy(trt__GetAudioSourcesResponse->AudioSources[i].token, oSysInfo->Oprofile[i].AASC.Aname);
		trt__GetAudioSourcesResponse->AudioSources[i].Channels = 1;

	}

	return SOAP_OK;


}

int __tmd__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioOutputNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Video Source
 * @This operation lists all available video sources for the device.
 *
 * @param Request	: _trt__GetVideoSources structure contains an empty message.
 * @param Response	: _trt__GetVideoSourcesResponse structure contains a list of structures describing all available video sources of the device.
 * @return               : On success SOAP_OK.
 : On failure Audio or Audio Outputs are not supported by the NVT.

 */
int __tmd__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int flag = 0, j =0;
	float _Framerate;
	float _Brightness;
	float _Saturation;
	float _Contrast;
	float _Sharpness;
	float _WideDynamicRange;
	int _Backlight;
	float _BacklightCompensationLevel;
	int _WhiteBalance;
	int height;
	int size = 0;//oSysInfo->nprofile;
	int i = 0;
	int n = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		flag = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vsourcetoken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken)==0)
			{
				flag = EXIST;
				break;
			}
		}
		if(!flag)
		{
			size++;
		}
	}

	trt__GetVideoSourcesResponse->__sizeVideoSources = size;
	trt__GetVideoSourcesResponse->VideoSources = (struct tt__VideoSource *)soap_malloc(soap, sizeof(struct tt__VideoSource) * size);

	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		flag = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vsourcetoken, oSysInfo->Oprofile[i].AVSC.Vsourcetoken)==0)
			{
				flag = EXIST;
				break;
			}
		}

		if(!flag)
		{
			_Framerate = oget_framerate(0);
			_Brightness = (float)(oSysInfo->imaging_conf[i].Brightness * 100) / 255;
			_Saturation = (float)(oSysInfo->imaging_conf[i].Saturation * 100) / 255;
			_Contrast = (float)(oSysInfo->imaging_conf[i].Contrast * 100) / 255;
			_Sharpness = (float)(oSysInfo->imaging_conf[i].Sharpness * 100) / 255;
			_WideDynamicRange = oSysInfo->imaging_conf[i].WideDynamicMode;
			_Backlight = oSysInfo->imaging_conf[i].BacklightCompensationMode;
			_BacklightCompensationLevel = oSysInfo->imaging_conf[i].BackLightLevel;
			_WhiteBalance = oSysInfo->imaging_conf[i].WhiteBalanceMode;
			height = oget_resolutionheight(0);

			//		for(i=0; i< size; i++)
			trt__GetVideoSourcesResponse->VideoSources[n].token = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;//"IPNC_VideoSource";
			trt__GetVideoSourcesResponse->VideoSources[n].Framerate = _Framerate;
			trt__GetVideoSourcesResponse->VideoSources[n].Resolution = (struct tt__VideoResolution*)soap_malloc(soap, sizeof(struct tt__VideoResolution));
			trt__GetVideoSourcesResponse->VideoSources[n].Resolution->Width = oSysInfo->Oprofile[i].AVSC.windowwidth;//width;
			trt__GetVideoSourcesResponse->VideoSources[n].Resolution->Height = oSysInfo->Oprofile[i].AVSC.windowheight;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging =(struct tt__ImagingSettings*)soap_malloc(soap, sizeof(struct tt__ImagingSettings));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Brightness = (float*)soap_malloc(soap,sizeof(float));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Brightness[0] = _Brightness;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->ColorSaturation = (float*)soap_malloc(soap,sizeof(float));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->ColorSaturation[0] = _Saturation;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Contrast = (float*)soap_malloc(soap,sizeof(float));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Contrast[0] = _Contrast;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter = (int *)soap_malloc(soap,sizeof(int));
			*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter = oSysInfo->imaging_conf[i].IrCutFilterMode; // dummy //{onv__IrCutFilterMode__ON = 0, onv__IrCutFilterMode__OFF = 1, onv__IrCutFilterMode__AUTO = 2}
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Sharpness = (float*)soap_malloc(soap,sizeof(float));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Sharpness[0] = _Sharpness;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation = (struct tt__BacklightCompensation*)soap_malloc(soap, sizeof(struct tt__BacklightCompensation));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation->Mode = _Backlight;//Streaming_onvif->BacklightCompensationMode;  //{onv__BacklightCompensationMode__OFF = 0, onv__BacklightCompensationMode__ON = 1}
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation->Level = _BacklightCompensationLevel;//float
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure = NULL;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus = NULL;
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WideDynamicRange = (struct tt__WideDynamicRange*)soap_malloc(soap, sizeof(struct tt__WideDynamicRange));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WideDynamicRange->Mode = _WideDynamicRange;   //{onv__WideDynamicMode__OFF = 0, onv__WideDynamicMode__ON = 1}
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WideDynamicRange->Level = oSysInfo->imaging_conf[i].WideDynamicMode;// dummy float
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WhiteBalance = (struct tt__WhiteBalance*)soap_malloc(soap, sizeof(struct tt__WhiteBalance));
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WhiteBalance->Mode = _WhiteBalance;	//{onv__WhiteBalanceMode__AUTO = 0, onv__WhiteBalanceMode__MANUAL = 1}
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WhiteBalance->CrGain = oSysInfo->imaging_conf[i].CrGain; // dummy
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WhiteBalance->CbGain = oSysInfo->imaging_conf[i].CbGain; // dummy
			trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Extension = NULL;
			trt__GetVideoSourcesResponse->VideoSources[n].Extension = NULL;
			n++;
		}
	}
	return SOAP_OK;

}

int __tmd__GetVideoOutputs(struct soap *soap, struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs, struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:VideoOutputNotSupported");
	return SOAP_FAULT;
}

int __tmd__GetVideoSourceConfiguration(struct soap *soap, struct _tmd__GetVideoSourceConfiguration *tmd__GetVideoSourceConfiguration, struct _tmd__GetVideoSourceConfigurationResponse *tmd__GetVideoSourceConfigurationResponse)
{
	return SOAP_OK;
}

/**
 * @brief Get Video Output Configuration
 * @This operation gets the video output configuration.
 *
 * @param Request	: _tmd__GetVideoOutputConfiguration structure contains an empty message.
 * @param Response	: _tmd__GetVideoOutputConfigurationResponse structure contains a list of structures describing all available video output configurations.
 * @return               : On success SOAP_OK.
 */

int __tmd__GetVideoOutputConfiguration(struct soap *soap, struct _tmd__GetVideoOutputConfiguration *tmd__GetVideoOutputConfiguration, struct _tmd__GetVideoOutputConfigurationResponse *tmd__GetVideoOutputConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int Ptoken_exist = NOT_EXIST;

	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(tmd__GetVideoOutputConfiguration->VideoOutputToken, oSysInfo->Oprofile[i].AVOC.name) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	tmd__GetVideoOutputConfigurationResponse->VideoOutputConfiguration = (struct tt__VideoOutputConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoOutputConfiguration));

	tmd__GetVideoOutputConfigurationResponse->VideoOutputConfiguration->Name = oSysInfo->Oprofile[0].AVOC.name;
	tmd__GetVideoOutputConfigurationResponse->VideoOutputConfiguration->UseCount = oSysInfo->Oprofile[0].AVOC.usecount;
	tmd__GetVideoOutputConfigurationResponse->VideoOutputConfiguration->token = oSysInfo->Oprofile[0].AVOC.token;
	return SOAP_OK;
}

/**
 * @brief Get Audio Source Configuration
 * @This operation lists the configuration of an Audio Input.
 *
 * @param Request	: _tmd__GetAudioSourceConfiguration structure contains the token of an AudioSource.
 * @param Response	: _tmd__GetAudioSourceConfigurationResponse structure variables members contains the requested AudioSourceConfiguration with the matching token.
 * @return               : On success SOAP_OK.
 : On failure AudioSourceToken does not exist.

 */
int __tmd__GetAudioSourceConfiguration(struct soap *soap, struct _tmd__GetAudioSourceConfiguration *tmd__GetAudioSourceConfiguration, struct _tmd__GetAudioSourceConfigurationResponse *tmd__GetAudioSourceConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoAudioSource");
	return SOAP_FAULT;
}

int __tmd__GetAudioOutputConfiguration(struct soap *soap, struct _tmd__GetAudioOutputConfiguration *tmd__GetAudioOutputConfiguration, struct _tmd__GetAudioOutputConfigurationResponse *tmd__GetAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioOutputNotSupported");
	return SOAP_FAULT;
}

int __tmd__SetVideoSourceConfiguration(struct soap *soap, struct _tmd__SetVideoSourceConfiguration *tmd__SetVideoSourceConfiguration, struct _tmd__SetVideoSourceConfigurationResponse *tmd__SetVideoSourceConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
	return SOAP_FAULT;
}
/**
 * @brief Set Video Output Configuration
 * @This operation modifies an video output configuration.
 *
 * @param Request	: _tmd__SetVideoOutputConfiguration structure contains the credentials to be set for VideoOutput.
 * @param Response	: _tmd__SetVideoOutputConfigurationResponse structure contains an empty message.
 * @return               :  On success - SOAP_OK.
 On failure - The requested VideoOutput does not exist. or
 */
int __tmd__SetVideoOutputConfiguration(struct soap *soap, struct _tmd__SetVideoOutputConfiguration *tmd__SetVideoOutputConfiguration, struct _tmd__SetVideoOutputConfigurationResponse *tmd__SetVideoOutputConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	video_output_conf set_video_output;
	int i = 0;
	int Ptoken_exist = NOT_EXIST;
	int ret;

	if(tmd__SetVideoOutputConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile; i++)
		{
			if(strcmp(tmd__SetVideoOutputConfiguration->Configuration->token, oSysInfo->Oprofile[i].AVOC.token) == 0)
			{
				Ptoken_exist = EXIST;
				break;
			}
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}

	if(tmd__SetVideoOutputConfiguration->Configuration->token != NULL)
	{
		set_video_output.position = i;
		strcpy(set_video_output.video_output_conf_t.name, tmd__SetVideoOutputConfiguration->Configuration->Name);
		set_video_output.video_output_conf_t.usecount = tmd__SetVideoOutputConfiguration->Configuration->UseCount;
		strcpy(set_video_output.video_output_conf_t.token, tmd__SetVideoOutputConfiguration->Configuration->token);
	}
	ret = ControlSystemData(SFIELD_SET_VIDEOOUTPUT_CONF, (void *)&set_video_output, sizeof(set_video_output));
	if(ret != SUCCESS)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

/**
 * @brief Set Audio Source Configuration
 * @This operation modifies an audio source configuration.
 *
 * @param Request	: _tmd__SetAudioSourceConfiguration structure contains the credentials to be set for AudioSource.
 * @param Response	: _tmd__SetAudioSourceConfigurationResponse structure contains an empty message.
 * @return               :  On success - SOAP_OK.
 On failure - The requested AudioSource does not exist. or
 The configuration parameters are not possible to set.

 */

int __tmd__SetAudioSourceConfiguration(struct soap *soap, struct _tmd__SetAudioSourceConfiguration *tmd__SetAudioSourceConfiguration, struct _tmd__SetAudioSourceConfigurationResponse *tmd__SetAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

int __tmd__SetAudioOutputConfiguration(struct soap *soap, struct _tmd__SetAudioOutputConfiguration *tmd__SetAudioOutputConfiguration, struct _tmd__SetAudioOutputConfigurationResponse *tmd__SetAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioOutputNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Video Source ConfigurationOptions
 * @This operation returns the available options when the video source parameters are reconfigured If a video source configuration is specified, the options shall concern that
 particular configuration.
 *
 * @param Request	:_tmd__GetVideoSourceConfigurationOptions structure contains optional tokens of a video source configuration and a media profile.
 * @param Response	:_tmd__GetVideoSourceConfigurationOptionsResponse structure contains the video configuration options.
 * @return               : On success - SOAP_OK.
 : On failure - The requested Video Input does not exist

 */

int __tmd__GetVideoSourceConfigurationOptions(struct soap *soap, struct _tmd__GetVideoSourceConfigurationOptions *tmd__GetVideoSourceConfigurationOptions, struct _tmd__GetVideoSourceConfigurationOptionsResponse *tmd__GetVideoSourceConfigurationOptionsResponse)
{
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions = (struct tt__VideoSourceConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfigurationOptions));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange = (struct tt__IntRectangleRange *)soap_malloc(soap, sizeof(struct tt__IntRectangleRange));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->XRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->XRange->Min = 100;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->XRange->Max = 1000;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->YRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->YRange->Min = 100;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->YRange->Max = 1000;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->WidthRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->WidthRange->Min = 720;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->WidthRange->Max = 2592;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->HeightRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->HeightRange->Min = 480;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->BoundsRange->HeightRange->Max = 1920;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->__sizeVideoSourceTokensAvailable = 3;
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable = (char **)soap_malloc(soap, sizeof(char) * 3);
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[0], "JPEG");
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[1] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[1], "MPEG4");
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[2] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->VideoSourceTokensAvailable[2], "H264");
	tmd__GetVideoSourceConfigurationOptionsResponse->VideoSourceConfigurationOptions->Extension = NULL;
	return SOAP_OK;
}

int __tmd__GetVideoOutputConfigurationOptions(struct soap *soap, struct _tmd__GetVideoOutputConfigurationOptions *tmd__GetVideoOutputConfigurationOptions, struct _tmd__GetVideoOutputConfigurationOptionsResponse *tmd__GetVideoOutputConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoOutput");
	return SOAP_FAULT;
}

/**
 * @brief Get Audio source Configuration options
 * @This operation returns the available options when the audio source parameters are reconfigured.
 *
 * @param Request	: _tmd__GetAudioSourceConfigurationOptions structure contains optional tokens of an audio source configuration and a media profile.
 * @param Response	:_tmd__GetAudioSourceConfigurationOptionsResponse structure contains the audio configuration options.
 * @return               : On success - SOAP_OK.
 : On failure an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.7.4).
 */

int __tmd__GetAudioSourceConfigurationOptions(struct soap *soap, struct _tmd__GetAudioSourceConfigurationOptions *tmd__GetAudioSourceConfigurationOptions, struct _tmd__GetAudioSourceConfigurationOptionsResponse *tmd__GetAudioSourceConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported ", "ter:AudioNotSupported");
	return SOAP_FAULT;
}

int __tmd__GetAudioOutputConfigurationOptions(struct soap *soap, struct _tmd__GetAudioOutputConfigurationOptions *tmd__GetAudioOutputConfigurationOptions, struct _tmd__GetAudioOutputConfigurationOptionsResponse *tmd__GetAudioOutputConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoAudioOutput");
	return SOAP_FAULT;
}
/**
 * @brief Get Relay outputs
 * @This operation returns all the available relay outputs.
 *
 * @param Response	:_tmd__GetRelayOptionsResponse structure contains the relay tokens created in the camera along with their properties.
 * @return               : On success - SOAP_OK.
 */

int __tmd__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;

	tds__GetRelayOutputsResponse->__sizeRelayOutputs = oSysInfo->nrelays;
	tds__GetRelayOutputsResponse->RelayOutputs = (struct tt__RelayOutput *)soap_malloc(soap, sizeof(struct tt__RelayOutput) * oSysInfo->nrelays);
	for(i = 0; i < oSysInfo->nrelays; i++)
	{
		tds__GetRelayOutputsResponse->RelayOutputs[i].token = oSysInfo->onvif_relay[i].onvif_relay_in.token;

		tds__GetRelayOutputsResponse->RelayOutputs[i].Properties = (struct tt__RelayOutputSettings *)soap_malloc(soap, sizeof(struct tt__RelayOutputSettings));
		tds__GetRelayOutputsResponse->RelayOutputs[i].Properties->Mode = oSysInfo->onvif_relay[i].onvif_relay_in.relaymode;
		tds__GetRelayOutputsResponse->RelayOutputs[i].Properties->DelayTime = 1000; // 1 Second
		tds__GetRelayOutputsResponse->RelayOutputs[i].Properties->IdleState = oSysInfo->onvif_relay[i].onvif_relay_in.idlestate;
	}
	return SOAP_OK;
}
/**
 * @brief Set Relay Output
 * @This operation sets the relays along with the token name and respective properties.
 *
 * @param Request	:_tmd__SetRelayOutputSettings structure contains the relay tokens and properties.
 * @return               : On success - SOAP_OK.
 */


int __tmd__SetRelayOutputSettings(struct soap *soap, struct _tmd__SetRelayOutputSettings *tmd__SetRelayOutputSettings, struct _tmd__SetRelayOutputSettingsResponse *tmd__SetRelayOutputSettingsResponse)
{
	return SOAP_OK;

}
/**
 * @brief Set Relay Output state
 * @This operation sets the particualar relay referenced by relay token. Resetting the paritular GIO has to be completed in this function based on the relay configs set by the client. Accordingly setting the GIO pins has to be implemented in this function.
 *
 * @param Request	:_tmd__SetRelayOutputSettings structure contains the relay token and the state to which relay has to be set for.
 * @return               : On success - SOAP_OK.

 */

int __tmd__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	SysInfo *oSysInfo  = GetSysInfo();
	int i = 0;
	relay_conf  relay;
	int token_exist = NOT_EXIST;
	for(i = 0 ;i < oSysInfo->nrelays; i++)
	{
		if(strcmp(tds__SetRelayOutputState->RelayOutputToken, oSysInfo->onvif_relay[i].onvif_relay_in.token) == 0)
		{
			token_exist = EXIST;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal ", "ter:RelayToken");
		return SOAP_FAULT;
	}

	relay.logicalstate_t = tds__SetRelayOutputState->LogicalState ;
	ControlSystemData(SFIELD_SET_RELAY_LOGICALSTATE, (void*) &relay, sizeof(relay));
	return SOAP_OK;
}

int __tptz__GetConfigurations(struct soap *soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse)
{
	tptz__GetConfigurationsResponse->__sizePTZConfiguration = 1;
	tptz__GetConfigurationsResponse->PTZConfiguration = (struct tt__PTZConfiguration *)soap_malloc(soap, sizeof(struct tt__PTZConfiguration));
	tptz__GetConfigurationsResponse->PTZConfiguration->Name = "ptz_default";
	tptz__GetConfigurationsResponse->PTZConfiguration->UseCount = 1;
	tptz__GetConfigurationsResponse->PTZConfiguration->token = "ptz_default_token";
	tptz__GetConfigurationsResponse->PTZConfiguration->NodeToken = "ptz_default_node_token";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultAbsoluteZoomPositionSpace = "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultRelativePanTiltTranslationSpace = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultRelativeZoomTranslationSpace = "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultContinuousZoomVelocitySpace = "http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultPTZSpeed = NULL ;
	//tptz__GetConfigurationsResponse->PTZConfiguration->DefaultPTZSpeed = (struct tt__PTZSpeed *)soap_malloc(soap, sizeof(stuct tt__PTZSpeed)) ;
	tptz__GetConfigurationsResponse->PTZConfiguration->DefaultPTZTimeout= NULL ;
	tptz__GetConfigurationsResponse->PTZConfiguration->PanTiltLimits = NULL ;
	tptz__GetConfigurationsResponse->PTZConfiguration->ZoomLimits = NULL ;
	tptz__GetConfigurationsResponse->PTZConfiguration->Extension = NULL ;
	//onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_OK;
}

int __tptz__GetPresets(struct soap *soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoPTZProfile");
	return SOAP_FAULT;
}

int __tptz__SetPreset(struct soap *soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__RemovePreset(struct soap *soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GotoPreset(struct soap *soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GetStatus(struct soap *soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

int __tptz__GetConfiguration(struct soap *soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GetNodes(struct soap *soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GetNode(struct soap *soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__SetConfiguration(struct soap *soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GetConfigurationOptions(struct soap *soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__GotoHomePosition(struct soap *soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__SetHomePosition(struct soap *soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__ContinuousMove(struct soap *soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__RelativeMove(struct soap *soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__SendAuxiliaryCommand(struct soap *soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__AbsoluteMove(struct soap *soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tptz__Stop(struct soap *soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

int __trc__CreateRecording(struct soap *soap, struct _trc__CreateRecording *trc__CreateRecording, struct _trc__CreateRecordingResponse *trc__CreateRecordingResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__DeleteRecording(struct soap *soap, struct _trc__DeleteRecording *trc__DeleteRecording, struct _trc__DeleteRecordingResponse *trc__DeleteRecordingResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__GetRecordings(struct soap *soap, struct _trc__GetRecordings *trc__GetRecordings, struct _trc__GetRecordingsResponse *trc__GetRecordingsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__SetRecordingConfiguration(struct soap *soap, struct _trc__SetRecordingConfiguration *trc__SetRecordingConfiguration, struct _trc__SetRecordingConfigurationResponse *trc__SetRecordingConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__GetRecordingConfiguration(struct soap *soap, struct _trc__GetRecordingConfiguration *trc__GetRecordingConfiguration, struct _trc__GetRecordingConfigurationResponse *trc__GetRecordingConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__CreateTrack(struct soap *soap, struct _trc__CreateTrack *trc__CreateTrack, struct _trc__CreateTrackResponse *trc__CreateTrackResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__DeleteTrack(struct soap *soap, struct _trc__DeleteTrack *trc__DeleteTrack, struct _trc__DeleteTrackResponse *trc__DeleteTrackResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__GetTrackConfiguration(struct soap *soap, struct _trc__GetTrackConfiguration *trc__GetTrackConfiguration, struct _trc__GetTrackConfigurationResponse *trc__GetTrackConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__SetTrackConfiguration(struct soap *soap, struct _trc__SetTrackConfiguration *trc__SetTrackConfiguration, struct _trc__SetTrackConfigurationResponse *trc__SetTrackConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NotImplemented");
	return SOAP_FAULT;
}

int __trc__CreateRecordingJob(struct soap *soap, struct _trc__CreateRecordingJob *trc__CreateRecordingJob, struct _trc__CreateRecordingJobResponse *trc__CreateRecordingJobResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__DeleteRecordingJob(struct soap *soap, struct _trc__DeleteRecordingJob *trc__DeleteRecordingJob, struct _trc__DeleteRecordingJobResponse *trc__DeleteRecordingJobResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__GetRecordingJobs(struct soap *soap, struct _trc__GetRecordingJobs *trc__GetRecordingJobs, struct _trc__GetRecordingJobsResponse *trc__GetRecordingJobsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__SetRecordingJobConfiguration(struct soap *soap, struct _trc__SetRecordingJobConfiguration *trc__SetRecordingJobConfiguration, struct _trc__SetRecordingJobConfigurationResponse *trc__SetRecordingJobConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__GetRecordingJobConfiguration(struct soap *soap, struct _trc__GetRecordingJobConfiguration *trc__GetRecordingJobConfiguration, struct _trc__GetRecordingJobConfigurationResponse *trc__GetRecordingJobConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__SetRecordingJobMode(struct soap *soap, struct _trc__SetRecordingJobMode *trc__SetRecordingJobMode, struct _trc__SetRecordingJobModeResponse *trc__SetRecordingJobModeResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trc__GetRecordingJobState(struct soap *soap, struct _trc__GetRecordingJobState *trc__GetRecordingJobState, struct _trc__GetRecordingJobStateResponse *trc__GetRecordingJobStateResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoRecordingJob");
	return SOAP_FAULT;
}

int __trp__GetReplayUri(struct soap *soap, struct _trp__GetReplayUri *trp__GetReplayUri, struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
	return SOAP_FAULT;
}

int __trp__GetReplayConfiguration(struct soap *soap, struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration, struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReplayNotSupported");
	return SOAP_FAULT;
}

int __trp__SetReplayConfiguration(struct soap *soap, struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration, struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReplayNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Video Source
 * @This operation lists all available video sources for the device.
 *
 * @param Request	: _trt__GetVideoSources structure contains an empty message.
 * @param Response	: _trt__GetVideoSourcesResponse structure contains a list of structures describing all available video sources of the device.
 * @return               : On success - SOAP_OK.
 On failure - No specific fault codes.
 */
int __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	trt__GetVideoSourcesResponse->__sizeVideoSources = 1;
	trt__GetVideoSourcesResponse->VideoSources = (struct tt__VideoSource *)soap_malloc(soap, sizeof(struct tt__VideoSource));
	trt__GetVideoSourcesResponse->VideoSources->token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);	/* required attribute of type onv:ReferenceToken */
	strcpy(trt__GetVideoSourcesResponse->VideoSources->token, "IPNC_Video_Source0");	/* required attribute of type onv:ReferenceToken */
	trt__GetVideoSourcesResponse->VideoSources->Framerate = 1;  /* required element of type xsd:float */
	trt__GetVideoSourcesResponse->VideoSources->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution)); // required element of type onv:VideoResolution
	trt__GetVideoSourcesResponse->VideoSources->Resolution->Width = 640;
	trt__GetVideoSourcesResponse->VideoSources->Resolution->Height = 480;
	trt__GetVideoSourcesResponse->VideoSources->Imaging = NULL;
	trt__GetVideoSourcesResponse->VideoSources->Extension = NULL;
	return SOAP_OK;
}

int __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	int i;
	SysInfo *oSysInfo = GetSysInfo();
	trt__GetAudioSourcesResponse->__sizeAudioSources = oSysInfo->nprofile;
	for(i=0; i< oSysInfo->nprofile; i++)
	{
		trt__GetAudioSourcesResponse->AudioSources = (struct tt__AudioSource *)soap_malloc(soap, sizeof(struct tt__AudioSource));
		trt__GetAudioSourcesResponse->AudioSources->token = (char *)soap_malloc(soap,  sizeof(char)*INFO_LENGTH);
		strcpy(trt__GetAudioSourcesResponse->AudioSources->token, oSysInfo->Oprofile[i].AASC.Aname);
		trt__GetAudioSourcesResponse->AudioSources->Channels = 1;
	}
	return SOAP_OK;

}

int __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioOutputNotSupported");
	return SOAP_FAULT;
}
/**
 * @brief Delete Profile
 * @This operation delete a media profile from device.
 * @param request           : _trt__DeleteProfile structure Contains a ProfileToken that indicates what media profile to delete.
 @param response          : _trt__DeleteProfileResponse structure contains an empity message.

 @return                   : On success - SOAP_OK.
 : On failure - The requested profile token ProfileToken does not exist. or
 The fixed Profile cannot be deleted.
 */

int __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
	int ret;
	int index;
	int Ptoken_exist = NOT_EXIST;
	SysInfo *pSysInfo = GetSysInfo();
	/* check whether profile already exist or not */
	for(index = 0; index < MAX_PROF; index++)
	{
		if(strcmp(trt__DeleteProfile->ProfileToken, pSysInfo->Oprofile[index].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	ret = ControlSystemData(SFIELD_DEL_PROFILE, (void *)&index, sizeof(index));
	return SOAP_OK;
}

/**
 * @brief Create Profile
 * @This operation creates a new empty media profile.
 *
 * @param Request	:_trt__CreateProfile structure contains  the friendly Name of the Profile to create as well as an optional Token
 parameter, specifying the unique identifier of the new media profile.
 @param Response	:_trt__CreateProfileResponse structure contains an empty Profile structure with no configuration entities.

 * @return               : On success - SOAP_OK.
 : On failure - A profile with the token ProfileToken already exists. or
 The maximum number of supported profiles has been reached.

 */

int __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	int i=0,ret,num;
	SysInfo *pSysInfo = GetSysInfo();
	media_profile profile_acount;
	if(trt__CreateProfile->Token == NULL)
	{
		//pay attention that here if Token is empty will use the same as Name!!
		trt__CreateProfile->Token = trt__CreateProfile->Name;
	}
	/* check whether profile already exist or not */
	for(i = 0; i < pSysInfo->nprofile; i++)
	{

		if(strcmp(trt__CreateProfile->Token, pSysInfo->Oprofile[i].profiletoken)==0){

			onvif_fault(soap,"ter:InvalidArgVal", "ter:ProfileExists");
			return SOAP_FAULT;
		}
	}

	/* check the limit on number of profiles */
	num = pSysInfo->nprofile;		// total number of profiles existing in the memory
	num = num + 1;
	if(num >= MAX_PROF)
	{
		onvif_fault(soap,"ter:Action ", "ter:MaxNVTProfiles");
		return SOAP_FAULT;
	}

	/* save profile */

	strcpy(profile_acount.profile_name, trt__CreateProfile->Name);
	strcpy(profile_acount.profile_token, trt__CreateProfile->Token);

	ret=ControlSystemData(SFIELD_ADD_PROFILE, (void *)&profile_acount, sizeof(media_profile));

	if(ret == -1)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:AddProfileFailed");
		return SOAP_FAULT;
	}

	trt__CreateProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
	trt__CreateProfileResponse->Profile->Name = pSysInfo->Oprofile[num - 1].profilename;
	trt__CreateProfileResponse->Profile->token = pSysInfo->Oprofile[num - 1].profiletoken;
	trt__CreateProfileResponse->Profile->fixed = 0;
	trt__CreateProfileResponse->Profile->VideoSourceConfiguration = NULL;
	trt__CreateProfileResponse->Profile->AudioSourceConfiguration = NULL;
	trt__CreateProfileResponse->Profile->VideoEncoderConfiguration = NULL;
	trt__CreateProfileResponse->Profile->AudioEncoderConfiguration = NULL;
	trt__CreateProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
	trt__CreateProfileResponse->Profile->PTZConfiguration = NULL;
	trt__CreateProfileResponse->Profile->MetadataConfiguration = NULL;
	trt__CreateProfileResponse->Profile->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Get Profile
 * @This operation fetches the whole profile configuration.
 *
 * @param Request	:_trt__GetProfile structure contains the token to the requested profile.
 * @param Response	:_trt__GetProfileResponse structure contains the Profile indicated by the Token parameter.
 * @return               : On success -  SOAP_OK.
 : On failure - The requested profile token ProfileToken does not exist.
 */

int __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	char _IPAddr[LARGE_INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "%3d.%3d.%3d.%3d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	int i;
	int Ptoken_status = NOT_EXIST;

	if((trt__GetProfile->ProfileToken) == NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidInputToken");
		return SOAP_FAULT;
	}
	if(strcmp(trt__GetProfile->ProfileToken, "") == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidInputToken");
		return SOAP_FAULT;
	}

	/* Check if ProfileToken Exist or Not */
	for(i = 0; i < MAX_PROF; i++)
	{
		if(strcmp(trt__GetProfile->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_status = EXIST;
			break;
		}
	}
	if(!Ptoken_status)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
	trt__GetProfileResponse->Profile->Name = oSysInfo->Oprofile[i].profilename;
	trt__GetProfileResponse->Profile->token = oSysInfo->Oprofile[i].profiletoken;
	trt__GetProfileResponse->Profile->fixed = (int *)soap_malloc(soap, sizeof(int)); //xsd__boolean__false_ = 0, xsd__boolean__true_ = 1
	*trt__GetProfileResponse->Profile->fixed = _false; //xsd__boolean__false_ = 0, xsd__boolean__true_ = 1
	/* VideoSourceConfiguration */
	trt__GetProfileResponse->Profile->VideoSourceConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = oSysInfo->Oprofile[i].AVSC.Vname;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = oSysInfo->Oprofile[i].AVSC.Vcount;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = oSysInfo->Oprofile[i].AVSC.Vtoken;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = oSysInfo->Oprofile[i].AVSC.windowy;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = oSysInfo->Oprofile[i].AVSC.windowx;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width  = oSysInfo->Oprofile[i].AVSC.windowwidth;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = oSysInfo->Oprofile[i].AVSC.windowheight;
	/* AudioSourceConfiguration */
	trt__GetProfileResponse->Profile->AudioSourceConfiguration = NULL;
	/*VideoEncoderConfiguration */
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name =oSysInfo->Oprofile[i].AESC.VEname;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = oSysInfo->Oprofile[i].AESC.VEusercount;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = oSysInfo->Oprofile[i].AESC.VEtoken;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = oSysInfo->Oprofile[i].AESC.Vencoder;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width = oSysInfo->Oprofile[i].AESC.Rwidth;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height = oSysInfo->Oprofile[i].AESC.Rheight;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = oSysInfo->Oprofile[i].AESC.VEquality;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = oSysInfo->Oprofile[i].AESC.frameratelimit;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = oSysInfo->Oprofile[i].AESC.encodinginterval;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit = oSysInfo->Oprofile[i].AESC.bitratelimit;

	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4->GovLength = oSysInfo->Oprofile[i].AESC.Mpeggovlength;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4->Mpeg4Profile = 0;//SP = 0, ASP = 1;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength = oSysInfo->Oprofile[i].AESC.H264govlength;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = 3;//Baseline = 0, Main = 1, High = 3
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Port = oSysInfo->Oprofile[i].AESC.Multicast_Conf.port;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->TTL = oSysInfo->Oprofile[i].AESC.Multicast_Conf.ttl;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->AutoStart = oSysInfo->Oprofile[i].AESC.Multicast_Conf.autostart;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = oSysInfo->Oprofile[i].AESC.Multicast_Conf.nIPType;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	strcpy(*trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv6Address = NULL;

	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = 720000;//oSysInfo->Oprofile[i].AESC.sessiontimeout;
	/* AudioEncoderConfiguration */
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration = NULL;
	/* VideoAnalyticsConfiguration */
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->Name, "video_analytics_config");
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->UseCount = 1;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->token , "video_analytics_config");
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 0;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = NULL;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration = (struct tt__RuleEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = 0;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = NULL;
	trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
	/* PTZConfiguration */
	trt__GetProfileResponse->Profile->PTZConfiguration = NULL;
	/* MetadataConfiguration */
	trt__GetProfileResponse->Profile->MetadataConfiguration = (struct tt__MetadataConfiguration *)soap_malloc(soap, sizeof(struct tt__MetadataConfiguration));
	trt__GetProfileResponse->Profile->MetadataConfiguration->Name = oSysInfo->Oprofile[i].MDC.MDname;// "metadata_config";
	trt__GetProfileResponse->Profile->MetadataConfiguration->UseCount = oSysInfo->Oprofile[i].MDC.MDusecount;//1;
	trt__GetProfileResponse->Profile->MetadataConfiguration->token = oSysInfo->Oprofile[i].MDC.MDtoken;//"metadata_config";
	trt__GetProfileResponse->Profile->MetadataConfiguration->Analytics = oSysInfo->Oprofile[i].MDC.MDanalytics;//0;
	trt__GetProfileResponse->Profile->MetadataConfiguration->SessionTimeout = 720000;
	trt__GetProfileResponse->Profile->MetadataConfiguration->PTZStatus = NULL;
	trt__GetProfileResponse->Profile->MetadataConfiguration->Events = NULL;
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Port = oSysInfo->Oprofile[i].MDC.VMC.port;
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->TTL = oSysInfo->Oprofile[i].MDC.VMC.ttl;
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->AutoStart = oSysInfo->Oprofile[i].MDC.VMC.autostart;//false
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->Type = oSysInfo->Oprofile[i].MDC.VMC.nIPType;
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(*trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetProfileResponse->Profile->MetadataConfiguration->Multicast->Address->IPv6Address = NULL;
	trt__GetProfileResponse->Profile->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Get Profiles
 * @Any endpoint can ask for the existing media profiles of an NVT using the GetProfiles command.
 *
 * @param Request	:_trt__GetProfile structure contains an empty message.
 * @param Response	:_trt__GetProfileResponse structure contains contains a configuration of all available profiles.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults!

 */

int __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int i;
	char _IPAddr[LARGE_INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "%d.%d.%d.%d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);


	trt__GetProfilesResponse->Profiles =(struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile) * oSysInfo->nprofile);
	trt__GetProfilesResponse->__sizeProfiles = oSysInfo->nprofile;

	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		trt__GetProfilesResponse->Profiles[i].Name = oSysInfo->Oprofile[i].profilename;
		trt__GetProfilesResponse->Profiles[i].token = oSysInfo->Oprofile[i].profiletoken;
		trt__GetProfilesResponse->Profiles[i].fixed = (int *)soap_malloc(soap, sizeof(int)); //false_ = 0, true_ = 1
		*trt__GetProfilesResponse->Profiles[i].fixed = _false; //false_ = 0, true_ = 1

		/* VideoSourceConfiguration */
		if(strcmp(oSysInfo->Oprofile[i].AVSC.Vtoken, "") != 0 && oSysInfo->Oprofile[i].AVSC.Vtoken != NULL)
		{
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration =
				(struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));

			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Name = oSysInfo->Oprofile[i].AVSC.Vname;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->UseCount = oSysInfo->Oprofile[i].AVSC.Vcount;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token = oSysInfo->Oprofile[i].AVSC.Vtoken;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->y = oSysInfo->Oprofile[i].AVSC.windowy;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->x = oSysInfo->Oprofile[i].AVSC.windowx;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->width  = oSysInfo->Oprofile[i].AVSC.windowwidth;
			trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->height = oSysInfo->Oprofile[i].AVSC.windowheight;
		} else {
			 trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration = NULL;
		}
		/* AudioSourceConfiguration */
		trt__GetProfilesResponse->Profiles[i].AudioSourceConfiguration = NULL;
		/* VideoEncoderConfiguration */
		if(strcmp(oSysInfo->Oprofile[i].AESC.VEtoken, "") != 0 && oSysInfo->Oprofile[i].AESC.VEtoken != NULL)
		{
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration =
				(struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));

		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name = oSysInfo->Oprofile[i].AESC.VEname;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->UseCount = oSysInfo->Oprofile[i].AESC.VEusercount;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token = oSysInfo->Oprofile[i].AESC.VEtoken;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Encoding = oSysInfo->Oprofile[i].AESC.Vencoder;//JPEG = 0, MPEG4 = 1, H264 = 2;

		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));

		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Width = oSysInfo->Oprofile[i].AESC.Rwidth;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Resolution->Height = oSysInfo->Oprofile[i].AESC.Rheight;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Quality = oSysInfo->Oprofile[i].AESC.VEquality;

		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->FrameRateLimit = oSysInfo->Oprofile[i].AESC.frameratelimit;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->EncodingInterval = oSysInfo->Oprofile[i].AESC.encodinginterval;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->RateControl->BitrateLimit = oSysInfo->Oprofile[i].AESC.bitratelimit;

		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
		if(i == 0)
		{
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->MPEG4->GovLength = oSysInfo->Oprofile[i].AESC.Mpeggovlength;
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->MPEG4->Mpeg4Profile = 0; //SP = 0, ASP = 1
		}
		else
		{
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->MPEG4->GovLength = oSysInfo->Oprofile[i].AESC.Mpeggovlength;
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->MPEG4->Mpeg4Profile = oSysInfo->Oprofile[i].AESC.Mpegprofile; //SP = 0, ASP = 1
		}
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
		if(i == 0)
		{
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->GovLength = oSysInfo->Oprofile[i].AESC.H264govlength;
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->H264Profile = 3;//Baseline = 0, Main = 1, High = 3
		}
		else
		{
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->GovLength = oSysInfo->Oprofile[i].AESC.H264govlength;
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->H264->H264Profile = 3; //Baseline = 0, Main = 1, High = 3
		}
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast =
					(struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Port = oSysInfo->Oprofile[i].AESC.Multicast_Conf.port;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->TTL = oSysInfo->Oprofile[i].AESC.Multicast_Conf.ttl;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->AutoStart = oSysInfo->Oprofile[i].AESC.Multicast_Conf.autostart;//false
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->Type = oSysInfo->Oprofile[i].AESC.Multicast_Conf.nIPType;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(*trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->IPv4Address, _IPAddr);
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Multicast->Address->IPv6Address = NULL;
		trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->SessionTimeout = 720000;
		} else {
			trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration = NULL;
		}
		/* AudioEncoderConfiguration */
		trt__GetProfilesResponse->Profiles[i].AudioEncoderConfiguration =NULL;
		/* VideoAnalyticsConfiguration */
		if(strcmp(oSysInfo->Oprofile[i].AVAC.VAtoken, "") != 0 && oSysInfo->Oprofile[i].AVAC.VAtoken!= NULL)
		{
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration =
				(struct tt__VideoAnalyticsConfiguration *) soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
			strcpy(trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->Name, "video_analytics_config");
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->UseCount = 1;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
			strcpy(trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->token, "video_analytics_config");//"video_analytics_config"
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration =
				(struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 0;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = NULL;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->RuleEngineConfiguration =
				(struct tt__RuleEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = 0;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = NULL;
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration->RuleEngineConfiguration->Extension = NULL;
		} else {
			trt__GetProfilesResponse->Profiles[i].VideoAnalyticsConfiguration = NULL;
		}

		/* PTZConfiguration */
		trt__GetProfilesResponse->Profiles[i].PTZConfiguration = NULL;
		/* MetadataConfiguration */
#if METADATA_ENABLE
		if(strcmp(oSysInfo->Oprofile[i].MDC.MDtoken, "") != 0 && oSysInfo->Oprofile[i].MDC.MDtoken != NULL)
		{
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration = (struct tt__MetadataConfiguration *)soap_malloc(soap,sizeof(struct tt__MetadataConfiguration));
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Name = oSysInfo->Oprofile[i].MDC.MDname;// "metadata_config";
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->UseCount = oSysInfo->Oprofile[i].MDC.MDusecount;//1;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->token = oSysInfo->Oprofile[i].MDC.MDtoken;//"metadata_config";
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Analytics = oSysInfo->Oprofile[i].MDC.MDanalytics;//0;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->SessionTimeout = oSysInfo->Oprofile[i].MDC.sessiontimeout;//720000;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->PTZStatus = NULL;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Events = NULL;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast =
					(struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Port = oSysInfo->Oprofile[i].MDC.VMC.port;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->TTL = oSysInfo->Oprofile[i].MDC.VMC.ttl;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->AutoStart = oSysInfo->Oprofile[i].MDC.VMC.autostart;//false
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address->Type = oSysInfo->Oprofile[i].MDC.VMC.nIPType;
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
			strcpy(*trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address->IPv4Address, _IPAddr);
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration->Multicast->Address->IPv6Address = NULL;
		} else
			trt__GetProfilesResponse->Profiles[i].MetadataConfiguration = NULL;
#endif
		trt__GetProfilesResponse->Profiles[i].Extension = NULL;
	}
	return SOAP_OK;
}

/**
 * @brief Add Video Encoder Configuration
 * @This operation adds a VideoEncoderConfiguration to an existing media profile. If a configuration exists in the media profile, it will be replaced.
 *
 * @param Request	:_trt__AddVideoEncoderConfiguration structure contains a reference to the VideoEncoderConfiguration to add and the Profile where it shall be added.
 * @param Response	:_trt__AddVideoEncoderConfigurationResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 : On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.2.5).
 */

int __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
	int Ptoken_exist = NOT_EXIST;
	int i;
	video_encoder_conf video_encoder_t;

  	memset(&video_encoder_t, 0, sizeof(video_encoder_t));
	SysInfo *oSysInfo = GetSysInfo();
	for(i = 1; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__AddVideoEncoderConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			video_encoder_t.position = i;
			break;
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	else if((Ptoken_exist))
	{
		video_encoder_t.video_encoder_conf_in_t.VEusercount_t = 1;
		strcpy(video_encoder_t.video_encoder_conf_in_t.VEtoken_t, trt__AddVideoEncoderConfiguration->ConfigurationToken);
		video_encoder_t.position = i;


		char _IPAddr[INFO_LENGTH];
		NET_IPV4 ip;
    	ip.int32 = net_get_ifaddr(ETH_NAME);
		sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

		// the same logic as in GetCompatibleVideoEncoderConfigurations
		int numberofstream = oget_numberofstream();
		struct tt__VideoEncoderConfiguration * Configurations =
			(struct tt__VideoEncoderConfiguration *)soap_malloc(soap, numberofstream * sizeof(struct tt__VideoEncoderConfiguration));
		int i = 0;
		for(i = 0; i < numberofstream; i++)
		{
			int bitrate = oget_bitrate(i);
			int ratecontrol = oget_ratecontrol(i);
			int framerate = oget_framerate(i);
			int width = oget_resolutionwidth(i);
			int height = oget_resolutionheight(i);
			int govlength = oget_ipratio(i);

			//fprintf(stderr, "__trt__GetCompatibleVideoEncoderConfigurations [%d] %d %d %d %d %d %d\n",i, bitrate, framerate, ratecontrol, width, height, govlength);
			if(i == 0) {
				Configurations[i].Name = "DE_Name1"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
				Configurations[i].token = "DE_Token1"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
			} else if(i == 1) {
				Configurations[i].Name = "DE_Name2"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
				Configurations[i].token = "DE_Token2"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
			} else if(i == 2) {
				Configurations[i].Name = "DE_Name3"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
				Configurations[i].token = "DE_Token3"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
			} else {
				Configurations[i].Name = "DE_Name"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
				Configurations[i].token = "DE_Token"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
			}
			Configurations[i].UseCount = 1;
			// {tt__VideoEncoding__JPEG = 0, onv__VideoEncoding__MPEG4 = 1, tt__VideoEncoding__H264 = 2}
			Configurations[i].Encoding = oget_encoding(i);
			Configurations[i].Quality = oSysInfo->Oprofile[0].AESC.VEquality; // float
			Configurations[i].SessionTimeout = 60;
			Configurations[i].Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
			Configurations[i].Resolution->Width = width;
			Configurations[i].Resolution->Height = height;
			Configurations[i].RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
			Configurations[i].RateControl->FrameRateLimit = framerate;
			Configurations[i].RateControl->EncodingInterval = ratecontrol; //0:vbr 1:cbr
			Configurations[i].RateControl->BitrateLimit = bitrate;
			Configurations[i].MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
			Configurations[i].MPEG4->GovLength = govlength;
			Configurations[i].MPEG4->Mpeg4Profile = 0;//{onv__Mpeg4Profile__SP = 0, onv__Mpeg4Profile__ASP = 1}
			Configurations[i].H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
			Configurations[i].H264->GovLength = govlength;
			Configurations[i].H264->H264Profile = 0;//Baseline = 0, Main = 1, High = 3
			Configurations[i].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			Configurations[i].Multicast->Port = 80;
			Configurations[i].Multicast->TTL = 1;
			Configurations[i].Multicast->AutoStart = 0;
			Configurations[i].Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
			Configurations[i].Multicast->Address->Type = 0;
			Configurations[i].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
			Configurations[i].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
			strcpy(Configurations[i].Multicast->Address->IPv4Address[0], _IPAddr);
			Configurations[i].Multicast->Address->IPv6Address = NULL;

			if(strcmp(video_encoder_t.video_encoder_conf_in_t.VEtoken_t, Configurations[i].token ) == 0)
			{
				strcpy(video_encoder_t.video_encoder_conf_in_t.VEname_t, Configurations[i].Name);
				video_encoder_t.video_encoder_conf_in_t.VEquality_t = Configurations[i].Quality;
				video_encoder_t.video_encoder_conf_in_t.Vencoder_t = Configurations[i].Encoding;             //video encoder for jpeg, mpeg, h.264
				video_encoder_t.video_encoder_conf_in_t.Rwidth_t = Configurations[i].Resolution->Width;               //resolution width
				video_encoder_t.video_encoder_conf_in_t.Rheight_t = Configurations[i].Resolution->Height;                //resolution height
				video_encoder_t.video_encoder_conf_in_t.frameratelimit_t = Configurations[i].RateControl->FrameRateLimit;        //frameratelimit
				video_encoder_t.video_encoder_conf_in_t.encodinginterval_t = Configurations[i].RateControl->EncodingInterval;       //encodinginterval
				video_encoder_t.video_encoder_conf_in_t.bitratelimit_t = Configurations[i].RateControl->BitrateLimit;            //bitratelimit
				video_encoder_t.video_encoder_conf_in_t.Mpeggovlength_t = Configurations[i].MPEG4->GovLength ;              //mpeg GoVLength
				video_encoder_t.video_encoder_conf_in_t.Mpegprofile_t =  Configurations[i].MPEG4->Mpeg4Profile;                 //mpegprofile SP/ASP
				video_encoder_t.video_encoder_conf_in_t.H264govlength_t = Configurations[i].H264->GovLength;               //H264 govlength
				video_encoder_t.video_encoder_conf_in_t.H264profile_t = Configurations[i].H264->H264Profile;                 //H264 profile  baseline/main/extended/high
				video_encoder_t.video_encoder_conf_in_t.sessiontimeout_t = Configurations[i].SessionTimeout;
				video_encoder_t.video_encoder_conf_in_t.Multicast_Conf_t.nIPType_t =  Configurations[i].Multicast->Address->Type;
				strcpy(video_encoder_t.video_encoder_conf_in_t.Multicast_Conf_t.IPv4Addr_t,  Configurations[i].Multicast->Address->IPv4Address);
				video_encoder_t.video_encoder_conf_in_t.Multicast_Conf_t.port_t  =  Configurations[i].Multicast->Port;
				video_encoder_t.video_encoder_conf_in_t.Multicast_Conf_t.ttl_t  =  Configurations[i].Multicast->TTL;
				video_encoder_t.video_encoder_conf_in_t.Multicast_Conf_t.autostart_t =  Configurations[i].Multicast->AutoStart;
			}
		}
		ControlSystemData(SFIELD_ADD_VIDEOENCODER_CONF, (void *)&video_encoder_t, sizeof(video_encoder_t));
	}
        return SOAP_OK;
}


/**
 * @brief Add VIdeo Source Configuration
 * @This operation adds a VideoEncoderConfiguration to an existing media profile.
 *
 * @param Request	:_trt__AddVideoSourceConfiguration structure contains a reference to the VideoEncoderConfiguration to add and the Profile where it shall be added.
 * @param Response	:_trt__AddVideoSourceConfigurationResponse structure contains an empty message.
 * @return               : On success - SOAP_OK.
 : On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.2.4).
 */

int __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
	int Ptoken_exist=NOT_EXIST;
	int i = 0, j = 0;
	int num_tokens = 0;
	int flg = 0;
	video_source_conf video_source_t;
	memset(&video_source_t, 0, sizeof(video_source_t));
	SysInfo *oSysInfo = GetSysInfo();

	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__AddVideoSourceConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			video_source_t.position = i;
			break;
		}
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	else if((Ptoken_exist))
	{
		int k = 0, found = 0;
		for(k = 0; k < oSysInfo->nprofile; k++)
		{
			if(strcmp( oSysInfo->Oprofile[k].AVSC.Vtoken, trt__AddVideoSourceConfiguration->ConfigurationToken) == 0)
			{
				strcpy(video_source_t.video_source_conf_in_t.Vname_t, oSysInfo->Oprofile[k].AVSC.Vname);
				video_source_t.video_source_conf_in_t.Vcount_t = oSysInfo->Oprofile[k].AVSC.Vcount + 1;
				oSysInfo->Oprofile[k].AVSC.Vcount ++;
				strcpy(video_source_t.video_source_conf_in_t.Vtoken_t, oSysInfo->Oprofile[k].AVSC.Vtoken);
				strcpy(video_source_t.video_source_conf_in_t.Vsourcetoken_t, oSysInfo->Oprofile[k].AVSC.Vsourcetoken);
				video_source_t.video_source_conf_in_t.windowx_t = oSysInfo->Oprofile[k].AVSC.windowx;
				video_source_t.video_source_conf_in_t.windowy_t = oSysInfo->Oprofile[k].AVSC.windowy;
				video_source_t.video_source_conf_in_t.windowwidth_t = oSysInfo->Oprofile[k].AVSC.windowwidth;
				video_source_t.video_source_conf_in_t.windowheight_t = oSysInfo->Oprofile[k].AVSC.windowheight;
				found = 1;
			}
		}
		if(found == 0)
		{
			video_source_t.video_source_conf_in_t.Vcount_t = num_tokens;
			strcpy(video_source_t.video_source_conf_in_t.Vtoken_t, trt__AddVideoSourceConfiguration->ConfigurationToken);
		}
	}

	ControlSystemData(SFIELD_ADD_VIDEOSOURCE_CONF, (void *)&video_source_t, sizeof(video_source_t));
	return SOAP_OK;
}

int __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
/*	onvif_fault(soap,"ter:ActionNotSupported","ter:AudioIn/OutNotSupported"); */
	SysInfo* oSysInfo = GetSysInfo();
	char _Token[SMALL_INFO_LENGTH];
	int Ptoken_exist = NOT_EXIST;
	int i;
	Add_AudioEncoder_Configuration_tmp add_audio_encoder;
	memset(&add_audio_encoder, 0, sizeof(add_audio_encoder));
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__AddAudioEncoderConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			add_audio_encoder.position = i;
			break;
		}
	}
	__u8 _Encoding;
	strcpy(_Token, trt__AddAudioEncoderConfiguration->ConfigurationToken);

	if(strcmp(_Token,"G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap,"ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	else if((Ptoken_exist))
	{
		strcpy(add_audio_encoder.Add_AudioEncoder_in.AEtoken_t, trt__AddAudioEncoderConfiguration->ConfigurationToken);
	}
	ControlSystemData(SFIELD_ADD_AUDIOENCODER_CONF, (void *)&add_audio_encoder, sizeof(add_audio_encoder));
	return SOAP_OK;

}

int __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
/*	onvif_fault(soap,"ter:ActionNotSupported","ter:AudioIn/OutNotSupported"); */
	SysInfo* oSysInfo = GetSysInfo();
	char _Token[SMALL_INFO_LENGTH];
	int Ptoken_exist = NOT_EXIST;
	int i;
	int flag = 0;
	Add_AudioSource_Configuration_tmp add_audio_source;
	memset(&add_audio_source, 0, sizeof(add_audio_source));
	if(trt__AddAudioSourceConfiguration->ProfileToken != NULL)
	{
		for(i = 0; i < oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__AddAudioSourceConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
			{
				Ptoken_exist = EXIST;
				flag = 1;
				add_audio_source.position = i;
				break;
			}
		}
	}
	__u8 _Encoding;
	strcpy(_Token, trt__AddAudioSourceConfiguration->ConfigurationToken);

	if(strcmp(_Token,"G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap,"ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	else if((Ptoken_exist))
	{
		strcpy(add_audio_source.Add_AudioSource_in.Atoken_t, trt__AddAudioSourceConfiguration->ConfigurationToken);
	}
	ControlSystemData(SFIELD_ADD_AUDIOSOURCE_CONF, (void *)&add_audio_source, sizeof(add_audio_source));
	return SOAP_OK;

}

int __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
	return SOAP_OK;
}

int __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
	int Ptoken_exist = NOT_EXIST;
	int i;
	Metadata_configuration_tmp metadata;
	memset(&metadata, 0, sizeof(metadata));
	SysInfo *oSysInfo = GetSysInfo();
	for(i=0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__AddMetadataConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			metadata.position = i;
			break;
		}
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	strcpy(metadata.MDC_in.MDtoken_t, trt__AddMetadataConfiguration->ConfigurationToken);
	ControlSystemData(SFIELD_ADD_METADATA, (void *)&metadata, sizeof(metadata));
	return SOAP_OK;
}

int __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Remove Video Encoder Configuration
 * @This operation removes a VideoEncoderConfiguration from an existing media profile. If the media profile does not contain a VideoEncoderConfiguration, the operation has no
 effect.
 * @params request            : _trt__RemoveVideoEncoderConfiguration structure Contains a reference to the media profile from which the VideoEncoderConfiguration shall be
 removed.
 * @params response           : _trt__RemoveVideoEncoderConfigurationResponse contains an empty message.
 * @return                    : On success - SOAP_OK.
 : On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.2.14)

 */
int __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	video_encoder_conf set_video_encoder_conf;
	memset(&set_video_encoder_conf, 0, sizeof(video_encoder_conf));
	int i;
	int ret;
	int Ptoken_exist = NOT_EXIST;
	for(i = 0; i < MAX_PROF; i++)
	{
		if(strcmp(trt__RemoveVideoEncoderConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken)==0)
		{
			Ptoken_exist = EXIST;
			set_video_encoder_conf.position = i;
			ret = ControlSystemData(SFIELD_DEL_VIDEOENCODER_CONF, (void *)&set_video_encoder_conf, sizeof(set_video_encoder_conf));
			if(ret != SUCCESS)
			{
				onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
				return SOAP_FAULT;
			}
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

/**
 * @brief Remove Video Source Configuration
 * @This operation removes a VideoSourceConfiguration from an existing media profile.
 * @params   request       :_trt__RemoveVideoSourceConfiguration structure Contains a reference to the media profile from which the VideoSourceConfiguration shall be removed.
 @params   response      :_trt__RemoveVideoSourceConfigurationResponse structure contains an empity message.
 @return                 : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.2.13)

 */
int __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int ret;
	int Ptoken_exist = NOT_EXIST;
	video_source_conf set_video_source_conf;
	memset(&set_video_source_conf, 0, sizeof(video_source_conf));
	for(i = 0; i < MAX_PROF; i++)
	{
		if(strcmp(trt__RemoveVideoSourceConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			set_video_source_conf.position = i;
			set_video_source_conf.video_source_conf_in_t.Vcount_t = 0;
			ret = ControlSystemData(SFIELD_SET_VIDEOSOURCE_CONF, (void *)&set_video_source_conf, sizeof(set_video_source_conf));
			if(ret != SUCCESS)
			{
				onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
				return SOAP_FAULT;
			}
		}
	}

	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

int __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
//	onvif_fault(soap,"ter:ActionNotSupported","ter:AudioIn/OutNotSupported");
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int profile_exist = 0;
	int index = 0;
	Add_AudioEncoder_Configuration_tmp remove_audio_encoder;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__RemoveAudioEncoderConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			profile_exist = EXIST;
			index = i;
			break;
		}
	}
	if(!profile_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	remove_audio_encoder.position = index;
	strcpy(remove_audio_encoder.Add_AudioEncoder_in.AEtoken_t, trt__RemoveAudioEncoderConfiguration->ProfileToken);
	ControlSystemData(SFIELD_DEL_AUDIO_ENCODER, (void *)&remove_audio_encoder, sizeof(remove_audio_encoder));
	return SOAP_OK;
}

int __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
/*	onvif_fault(soap,"ter:ActionNotSupported","ter:AudioIn/OutNotSupported"); */
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int profile_exist = 0;
	int index = 0;
	Add_AudioSource_Configuration_tmp remove_audio_source;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__RemoveAudioSourceConfiguration->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			profile_exist = EXIST;
			index = i;
			break;
		}
	}
	if(!profile_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	remove_audio_source.position = index;
	strcpy(remove_audio_source.Add_AudioSource_in.Atoken_t, trt__RemoveAudioSourceConfiguration->ProfileToken);
	ControlSystemData(SFIELD_DEL_AUDIO_SOURCE,(void *)&remove_audio_source,sizeof(remove_audio_source));
	return SOAP_OK;
}

int __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
	return SOAP_FAULT;
}

int __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
	return SOAP_FAULT;
}

int __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Source Source Configurations
 * @This operation lists the video source configurations of a VideoSource.
 *
 * @param Request	: _trt__GetVideoSourceConfigurations structure contains the token of the video input.
 * @param Response	: _trt__GetVideoSourceConfigurationsResponse structure contains the requested VideoSourceConfiguration with the matching token.
 * @return               : On success - SOAP_OK.
 : On failure No command specific faults!

 */
int __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int num_tokens = 0;
	int num = 0;
	int j;
	int flag = NOT_EXIST;
	int k = 0;
	int use = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		flag = 0;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vtoken, oSysInfo->Oprofile[i].AVSC.Vtoken)==0)
			{
				flag = 1;
				break;
			}
		}
		if(!flag)
		{
			num_tokens++;
		}


	}

	trt__GetVideoSourceConfigurationsResponse->Configurations = (struct tt__VideoSourceConfiguration*)soap_malloc(soap,(num_tokens * sizeof(struct tt__VideoSourceConfiguration)));
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		flag = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vtoken, oSysInfo->Oprofile[i].AVSC.Vtoken)==0)
			{
				flag = EXIST;
				break;
			}
		}

		if(!flag)
		{
			use = 0;
			for(k = 0; k < oSysInfo->nprofile; k++)
			{
				if(strcmp(oSysInfo->Oprofile[i].AVSC.Vtoken, oSysInfo->Oprofile[k].AVSC.Vtoken) == 0)
				{
					use++;
				}
			}
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Name = oSysInfo->Oprofile[i].AVSC.Vname;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].UseCount = use;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].token = oSysInfo->Oprofile[i].AVSC.Vtoken;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Bounds->x = oSysInfo->Oprofile[i].AVSC.windowx;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Bounds->y = oSysInfo->Oprofile[i].AVSC.windowy;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Bounds->width = oSysInfo->Oprofile[i].AVSC.windowwidth;
			trt__GetVideoSourceConfigurationsResponse->Configurations[num].Bounds->height = oSysInfo->Oprofile[i].AVSC.windowheight;
			num++;
		}
	}
	trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = num;
	return SOAP_OK;
}

/**
 * @brief Get Video Encoder Configuration
 * @This operation lists all existing video encoder configurations of an NVT.
 *
 * @param Request	: _trt__GetVideoEncoderConfigurations structure contains an empty message.
 * @param Response	:_trt__GetVideoEncoderConfigurationsResponse structure contains a list of all existing video encoder configurations in the NVT.
 * @return               : On success - SOAP_OK.
 : On failure - The requested configuration indicated with ConfigurationToken does not exist.

 */
int __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
	SysInfo *pSysInfo = GetSysInfo();
	int i;
	int j;
	int flag = 0;
	int num_tokens = 0;
	int num = 0;
	int use = 0;
	int k = 0;
	char _IPAddr[LARGE_INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "%3d.%3d.%3d.%3d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);


	for(i = 0; i < pSysInfo->nprofile;i++)
	{
		flag  = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(pSysInfo->Oprofile[i].AESC.VEtoken, pSysInfo->Oprofile[j].AESC.VEtoken) == 0)
			{
				flag = EXIST;
				break;
			}
		}
		if(!flag)
		{
			num_tokens++;
		}
	}
	trt__GetVideoEncoderConfigurationsResponse->Configurations = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, (num_tokens * sizeof(struct tt__VideoEncoderConfiguration)));

	for(i = 0; i < pSysInfo->nprofile; i++)
	{
		for(j = 0; j< i; j++)
		{
			if(strcmp(pSysInfo->Oprofile[i].AESC.VEtoken, pSysInfo->Oprofile[j].AESC.VEtoken) == 0)
			{
				flag = EXIST;
				break;
			}
		}
		if(!flag)
		{
			use = 0;
			for(k = 0; k < pSysInfo->nprofile; k++)
			{
				if(strcmp(pSysInfo->Oprofile[i].AVSC.Vtoken, pSysInfo->Oprofile[k].AVSC.Vtoken) == 0)
				{
					use++;
				}
			}

			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Name = pSysInfo->Oprofile[i].AESC.VEname;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].UseCount = use;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].token = pSysInfo->Oprofile[i].AESC.VEtoken;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Encoding = pSysInfo->Oprofile[i].AESC.Vencoder;// JPEG = 0, MPEG4 = 1, H264 = 2
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Quality = pSysInfo->Oprofile[i].AESC.VEquality;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].SessionTimeout = 720000;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Resolution->Width = pSysInfo->Oprofile[i].AESC.Rwidth;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Resolution->Height = pSysInfo->Oprofile[i].AESC.Rheight;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].RateControl->FrameRateLimit = pSysInfo->Oprofile[i].AESC.frameratelimit;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].RateControl->EncodingInterval = pSysInfo->Oprofile[i].AESC.encodinginterval; // dummy
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].RateControl->BitrateLimit = pSysInfo->Oprofile[i].AESC.bitratelimit;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].MPEG4->GovLength = pSysInfo->Oprofile[i].AESC.Mpeggovlength;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].MPEG4->Mpeg4Profile = pSysInfo->Oprofile[i].AESC.Mpegprofile;//{tt__Mpeg4Profile__SP = 0, tt__Mpeg4Profile__ASP = 1}
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].H264->GovLength = pSysInfo->Oprofile[i].AESC.H264govlength;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].H264->H264Profile = pSysInfo->Oprofile[i].AESC.H264profile;//Baseline = 0, Main = 1, High = 3;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Port = pSysInfo->Oprofile[i].AESC.Multicast_Conf.port;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->TTL = pSysInfo->Oprofile[i].AESC.Multicast_Conf.ttl;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->AutoStart = pSysInfo->Oprofile[i].AESC.Multicast_Conf.autostart;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address->Type = pSysInfo->Oprofile[i].AESC.Multicast_Conf.nIPType;
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
			strcpy(*trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address, _IPAddr);
			trt__GetVideoEncoderConfigurationsResponse->Configurations[num].Multicast->Address->IPv6Address = NULL;
			num++;
			flag = 0;
		}
	}
	trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = num_tokens; // JPEG | MPEG4 | H264
	return SOAP_OK;
}

/**
 * @brief Get Audio Source Configuration
 * @This operation lists all existing audio source configurations of an NVT.
 *
 * @param Request	:_trt__GetAudioSourceConfigurations structure contains an empty message.
 * @param Response	:_trt__GetAudioSourceConfigurationsResponse structure contains a list of all existing audio source configurations in the NVT.
 * @return               : On success -  SOAP_OK.
 : On failure - NVT does not support audio.

 */
int __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int flag = 0;
	int flg = 0;
	int j;
	int num_token = 0;
	int index = 0;

	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AASC.Atoken, oSysInfo->Oprofile[i].AASC.Atoken)==0)
			{
				flg = 1;
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	trt__GetAudioSourceConfigurationsResponse->__sizeConfigurations = num_token;


	/* AudioSourceConfiguration */
	trt__GetAudioSourceConfigurationsResponse->Configurations = (struct tt__AudioSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration)* num_token);
	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		flag = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AASC.Atoken, oSysInfo->Oprofile[i].AASC.Atoken)==0)
			{
				flag = EXIST;
				break;
			}
		}

		if(!flag)
		{
			trt__GetAudioSourceConfigurationsResponse->Configurations[index].Name = oSysInfo->Oprofile[i].AASC.Aname;//"audio_encoder_config";
			trt__GetAudioSourceConfigurationsResponse->Configurations[index].UseCount = 1;
			trt__GetAudioSourceConfigurationsResponse->Configurations[index].token = oSysInfo->Oprofile[i].AASC.Atoken;//"audio_encoder_config";
			trt__GetAudioSourceConfigurationsResponse->Configurations[index].SourceToken = oSysInfo->Oprofile[i].AASC.Asourcetoken;//"audio_encoder_config";
			index++;
		}

	}
		return SOAP_OK;
}

/**
 * @brief Get Audio Encoder Configuration
 * @This operation lists all existing device audio encoder configurations.
 *
 * @param Request	: _trt__GetAudioEncoderConfigurations structure contains an empty message.
 * @param Response	: _trt__GetAudioEncoderConfigurationsResponse structure contains contains a list of all existing audio encoder configurations in the NVT.
 * @return               : On success - SOAP_OK.
 On failure - NVT does not support audio.

 */
int __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _IPAddr[INFO_LENGTH];
	int _Bitrate = 0;
	int _SampleRate = 0;
	int _Encoding = 0;
	int i;
	int num_G711 = 0;
	int num_AAC = 0;
	int num_G726 = 0;
	int num_profile = oSysInfo->nprofile;
	int flag = 0,flg = 0;
	int j,num_token = 0;
	int index = 0;
	/* Read Settings */
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	for(i = 0 ;i < oSysInfo->nprofile; i++)
	{
		if(strcmp("G711", oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			num_G711 ++;
		}
	}
	for(i = 0 ;i < oSysInfo->nprofile; i++)
	{
		if(strcmp("G726", oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			num_G726 ++;
		}
	}
	for(i = 0 ;i < oSysInfo->nprofile; i++)
	{
		if(strcmp("AAC", oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			num_AAC ++;
		}
	}

	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AAEC.AEtoken, oSysInfo->Oprofile[i].AAEC.AEtoken)==0)
			{
				flg = 1;
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	trt__GetAudioEncoderConfigurationsResponse->__sizeConfigurations = num_token;//num_profile;
	trt__GetAudioEncoderConfigurationsResponse->Configurations = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap,num_token*sizeof(struct tt__AudioEncoderConfiguration));


	/* AudioSourceConfiguration */
	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		flag = NOT_EXIST;
		for(j = 0; j < i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AAEC.AEtoken, oSysInfo->Oprofile[i].AAEC.AEtoken)==0)
			{
				flag = EXIST;
				break;
			}
		}

		if(!flag)
		{
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Name = oSysInfo->Oprofile[i].AAEC.AEname;
			if(strcmp(oSysInfo->Oprofile[i].AAEC.AEtoken, "G711") == 0)
			{
				trt__GetAudioEncoderConfigurationsResponse->Configurations[index].UseCount = num_G711;
			}
			else if(strcmp(oSysInfo->Oprofile[i].AAEC.AEtoken, "ACC") == 0)
			{
				trt__GetAudioEncoderConfigurationsResponse->Configurations[index].UseCount = num_AAC;
			}
			else
			{
				trt__GetAudioEncoderConfigurationsResponse->Configurations[index].UseCount = num_G726;
			}
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].token = oSysInfo->Oprofile[i].AAEC.AEtoken;//"G711";
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Encoding = oSysInfo->Oprofile[i].AAEC.AEencoding;//{tt__AudioEncoding__G711 = 0, tt__AudioEncoding__G726 = 1, tt__AudioEncoding__AAC = 2}
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Bitrate = oSysInfo->Oprofile[i].AAEC.bitrate;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].SampleRate = oSysInfo->Oprofile[i].AAEC.samplerate;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].SessionTimeout = 60;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Port = 80;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->TTL = 1;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->AutoStart = 0;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address->Type = 0;
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
			strcpy(*trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address->IPv4Address, _IPAddr);
			trt__GetAudioEncoderConfigurationsResponse->Configurations[index].Multicast->Address->IPv6Address = NULL;
			index++;
		}

	}
	return SOAP_OK;
}
/**
 * @brief Get Video Analytics Configurations
 * @This operation lists all video analytics configurations of a device.
 *
 * @param Request	:_trt__GetVideoAnalyticsConfigurations structure contains message is empty.
 * @param Response	:_trt__GetVideoAnalyticsConfigurationsResponse structure contains a list of all existing video analytics configurations in the device.
 * @return               : On success - SOAP_OK.
 : On failure - Device does not support video analytics.
 */
int __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations=(struct tt__VideoAnalyticsConfiguration*)soap_malloc(soap, 2 * sizeof(struct tt__VideoAnalyticsConfiguration));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].Name, "Face Detection");
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].token, "Face Detection");
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->__sizeAnalyticsModule = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, sizeof(struct tt__Config));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem ));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = (struct tt__ItemListExtension *)soap_malloc(soap, sizeof(struct tt__ItemListExtension));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[0].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;

	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].Name, "Motion Detecion");
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].UseCount = 1;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].token, "Motion Detection");
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->__sizeAnalyticsModule = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, sizeof(struct tt__Config));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters = (struct tt__ItemList *)soap_malloc(soap, sizeof(struct tt__ItemList));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 0;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem ));
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name = NULL;
	trt__GetVideoAnalyticsConfigurationsResponse->Configurations[1].AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Get MetaData Configurations
 * @This operation lists all existing metadata configurations.
 *
 * @param Request	:_trt__GetVideoAnalyticsConfigurations structure contains message is empty.
 * @param Response	:_trt__GetVideoAnalyticsConfigurationsResponse structure contains a list of all existing metadata configurations in the device.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults!
 */
int __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i = 0;
	int token_exist = 0;
	int j = 0;
	int num_token = 0;
	int index = 0;
	int num = 0;
	char _IPAddr[INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		token_exist = NOT_EXIST;
		for(j = 0; j< i ; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].MDC.MDtoken, oSysInfo->Oprofile[i].MDC.MDtoken) == 0)
			{
				token_exist = EXIST;
				break;
			}

		}
		if(!token_exist)
		{
			num_token++;
		}
	}
	trt__GetMetadataConfigurationsResponse->__sizeConfigurations = num_token;

	trt__GetMetadataConfigurationsResponse->Configurations=(struct tt__MetadataConfiguration *)soap_malloc(soap, num_token * sizeof(struct tt__MetadataConfiguration));
	for(i = 0; i< oSysInfo->nprofile; i++)
	{
		token_exist = NOT_EXIST;
		for(j = 0; j< i ; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].MDC.MDtoken, oSysInfo->Oprofile[i].MDC.MDtoken) == 0)
			{
				token_exist = EXIST;
			}

		}
		if(!token_exist)
		{
			trt__GetMetadataConfigurationsResponse->Configurations[num].Name = oSysInfo->Oprofile[i].MDC.MDname;//"Face Detection";
			trt__GetMetadataConfigurationsResponse->Configurations[num].UseCount = oSysInfo->Oprofile[i].MDC.MDusecount;//1;
			trt__GetMetadataConfigurationsResponse->Configurations[num].token = oSysInfo->Oprofile[i].MDC.MDtoken;//"Face Detection";
			trt__GetMetadataConfigurationsResponse->Configurations[num].SessionTimeout = oSysInfo->Oprofile[i].MDC.sessiontimeout;
			trt__GetMetadataConfigurationsResponse->Configurations[num].PTZStatus = NULL;// (struct tt__PTZFilter*)soap_malloc(soap,sizeof(struct tt__PTZFilter));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Events = NULL;// (struct tt__EventSubscription*)soap_malloc(soap,sizeof(struct tt__EventSubscription));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Analytics = NULL;//(int *)soap_malloc(soap,sizeof(int));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Analytics = (int *)soap_malloc(soap, sizeof(int));
			*trt__GetMetadataConfigurationsResponse->Configurations[num].Analytics = 1;
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address->Type = oSysInfo->Oprofile[i].MDC.VMC.nIPType;
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
			strcpy(*trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address->IPv4Address, _IPAddr);
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Address->IPv6Address = NULL;
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->Port = oSysInfo->Oprofile[i].MDC.VMC.port;
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->TTL = oSysInfo->Oprofile[i].MDC.VMC.ttl;
			trt__GetMetadataConfigurationsResponse->Configurations[num].Multicast->AutoStart = oSysInfo->Oprofile[i].MDC.VMC.autostart;
			num ++;
		}
	}
	return SOAP_OK;
}

int __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Video Source Configuration
 * @This operation lists the video source configurations of a VideoSource.
 *
 * @param Request	:_trt__GetVideoSourceConfiguration structure contains token of the video input.
 * @param Response	:_trt__GetVideoSourceConfigurationResponse structure contains requested VideoSourceConfiguration with the matching token.
 * @return               : On success -  SOAP_OK.
 : On failure - No specific fault codes.

 */
int __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int i;
	int flag = 0;
	int j;
	int num_tokens = 0;
	int Ptoken_exist = NOT_EXIST;
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetVideoSourceConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].AVSC.Vtoken)==0)
		{
			num_tokens++;
		}
	}
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetVideoSourceConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetVideoSourceConfigurationResponse->Configuration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
	trt__GetVideoSourceConfigurationResponse->Configuration->Name = oSysInfo->Oprofile[i].AVSC.Vname;
	trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = num_tokens;//oSysInfo->Oprofile[i].AVSC.Vcount;
	trt__GetVideoSourceConfigurationResponse->Configuration->token = oSysInfo->Oprofile[i].AVSC.Vtoken;
	trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x = oSysInfo->Oprofile[i].AVSC.windowx;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y = oSysInfo->Oprofile[i].AVSC.windowy;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width = oSysInfo->Oprofile[i].AVSC.windowwidth;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = oSysInfo->Oprofile[i].AVSC.windowheight;
	return SOAP_OK;
}

/**
 * @brief Get Video Encoder Configuration
 * @This operation lists all existing video encoder configurations of an NVT.
 *
 * @param Request	:_trt__GetVideoEncoderConfiguration structure contains an empty message.
 * @param Response	:_trt__GetVideoEncoderConfigurationResponse structure contains a list of all existing video encoder configurations in the NVT.
 * @return               : On success - SOAP_OK.
 On failure - The requested configuration indicated with ConfigurationToken does not exist.
 */
int __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int i;
	int num_tokens = 0;
	NET_IPV4 ip;
	char _IPAddr[INFO_LENGTH];
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "%d.%d.%d.%d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	int Ptoken_exist = NOT_EXIST;
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].AESC.VEtoken) == 0)
		{
			num_tokens++;
		}
	}

	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].AESC.VEtoken) == 0)
		{

			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	trt__GetVideoEncoderConfigurationResponse->Configuration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Name =oSysInfo->Oprofile[i].AESC.VEname;
	trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = num_tokens;//oSysInfo->Oprofile[i].AESC.VEusercount;
	trt__GetVideoEncoderConfigurationResponse->Configuration->token = oSysInfo->Oprofile[i].AESC.VEtoken;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = oSysInfo->Oprofile[i].AESC.Vencoder;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width = oSysInfo->Oprofile[i].AESC.Rwidth;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height = oSysInfo->Oprofile[i].AESC.Rheight;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Quality = oSysInfo->Oprofile[i].AESC.VEquality;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit = oSysInfo->Oprofile[i].AESC.frameratelimit;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = oSysInfo->Oprofile[i].AESC.encodinginterval;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit = oSysInfo->Oprofile[i].AESC.bitratelimit;
	trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4->GovLength = oSysInfo->Oprofile[i].AESC.Mpeggovlength;
	trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4->Mpeg4Profile = oSysInfo->Oprofile[i].AESC.Mpegprofile;//SP = 0, ASP = 1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength = oSysInfo->Oprofile[i].AESC.H264govlength;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = oSysInfo->Oprofile[i].AESC.H264profile;//Baseline = 0, Main = 1, High = 3
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Port = oSysInfo->Oprofile[i].AESC.Multicast_Conf.port;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->TTL = oSysInfo->Oprofile[i].AESC.Multicast_Conf.ttl;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->AutoStart = oSysInfo->Oprofile[i].AESC.Multicast_Conf.autostart;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = oSysInfo->Oprofile[i].AESC.Multicast_Conf.nIPType;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(*trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address,_IPAddr);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv6Address = NULL;
	trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = oSysInfo->Oprofile[i].AESC.sessiontimeout;
	return SOAP_OK;
}

/**
 * @brief Get Audio Source Configuration
 * @An AudioSourceConfiguration contains a reference to an AudioSource that is to be used for input in a media profile. This operation lists all existing audio source configurations of an NVT
 *
 * @param Request	:_trt__GetAudioSourceConfiguration structure variable members contains empty message.
 * @param Response	:_trt__GetAudioSourceConfigurationResponse structure contains a list of structures describing all available audio sources of the device.
 * @return               : On success - SOAP_OK.
 On failure - The requested configuration indicated with ConfigurationToken does not exist. or
 NVT does not support audio.
 */
int __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int flag = NOT_EXIST;
	char *_Name;
	int use = 0;
	int k = 0;

	if(trt__GetAudioSourceConfiguration->ConfigurationToken != NULL)
	{
		_Name = trt__GetAudioSourceConfiguration->ConfigurationToken;
	}

	for(i = 0;i < oSysInfo->nprofile; i++)
	{
		if(strcmp(_Name, oSysInfo->Oprofile[i].AASC.Atoken) == 0)
		{
			flag = EXIST;
			break;
		}
	}
	if(flag)
	{
		for(k = 0; k < oSysInfo->nprofile; k++)
		{
			if(strcmp(_Name, oSysInfo->Oprofile[k].AASC.Atoken) == 0)
			{
				use++;
			}
		}
		trt__GetAudioSourceConfigurationResponse->Configuration = (struct tt__AudioSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
		trt__GetAudioSourceConfigurationResponse->Configuration->Name = oSysInfo->Oprofile[i].AASC.Aname;
		trt__GetAudioSourceConfigurationResponse->Configuration->UseCount = use;
		trt__GetAudioSourceConfigurationResponse->Configuration->token = oSysInfo->Oprofile[i].AASC.Atoken;
		trt__GetAudioSourceConfigurationResponse->Configuration->SourceToken = oSysInfo->Oprofile[i].AASC.Asourcetoken;
	}
	else
	{
		onvif_fault(soap,"ter:ProfileNotExist", "ter:ProfileNotExist");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

/**
 * @brief Get Audio Encoder Configuration
 * @brief This operation lists all existing device audio encoder configurations.
 *
 * @param Request	:_trt__GetAudioEncoderConfiguration structure variable members contains an empty message.
 * @param Response	:_trt__GetAudioEncoderConfigurationResponse structure contains a list of all existing audio encoder configurations in the NVT.
 * @return               : On success - SOAP_OK.
 On failure - The configuration does not exist.
 NVT does not support audio.
 */
int __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
	/* Read Audio */
	SysInfo* oSysInfo = (SysInfo *)GetSysInfo();
	int _Bitrate = 0;
	int _SampleRate = 0;
	int i = 0,token_exist = 0;
	int _Encoding = oSysInfo->audio_config.codectype;
	char _IPAddr[INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	char *name = (char *)malloc(SMALL_INFO_LENGTH);
	int k = 0;
	int use = 0;

	if(oSysInfo->audio_config.bitrate == 0)
		_Bitrate = 64;
	else if(oSysInfo->audio_config.bitrate == 1)
		_Bitrate = 128;

	if(oSysInfo->audio_config.samplerate == 0)
		_SampleRate = 8;
	else if(oSysInfo->audio_config.samplerate == 1)
		_SampleRate = 16;
	/* Read Audio */
	/* Read Settings */
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	/* Read Settings */
	if(_Encoding==0)
	{
		strcpy(name, "G711");
	}
	if(_Encoding==1)
	{
		strcpy(name, "AAC");
		_Encoding = 2;
	}
	for(i = 0; i <=  oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetAudioEncoderConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			token_exist = EXIST;
			break;

		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	use = 0;
	for(k = 0; k < oSysInfo->nprofile; k++)
	{

		if(strcmp(trt__GetAudioEncoderConfiguration->ConfigurationToken, oSysInfo->Oprofile[k].AAEC.AEtoken) == 0)
		{
			use++;
		}
	}
	trt__GetAudioEncoderConfigurationResponse->Configuration = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
	trt__GetAudioEncoderConfigurationResponse->Configuration->Name = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetAudioEncoderConfigurationResponse->Configuration->Name, oSysInfo->Oprofile[i].AAEC.AEname);
	trt__GetAudioEncoderConfigurationResponse->Configuration->UseCount = use;
	trt__GetAudioEncoderConfigurationResponse->Configuration->token = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetAudioEncoderConfigurationResponse->Configuration->token, oSysInfo->Oprofile[i].AAEC.AEtoken);
	trt__GetAudioEncoderConfigurationResponse->Configuration->Encoding = oSysInfo->Oprofile[i].AAEC.AEencoding;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Bitrate = oSysInfo->Oprofile[i].AAEC.bitrate;
	trt__GetAudioEncoderConfigurationResponse->Configuration->SampleRate = oSysInfo->Oprofile[i].AAEC.samplerate;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address = (struct tt__IPAddress*)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->Type = 0;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(*trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv6Address = NULL;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Port = 80;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->TTL = 1;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->AutoStart = 0;
	trt__GetAudioEncoderConfigurationResponse->Configuration->SessionTimeout = 60;
	free(name);
	return SOAP_OK;
}

int __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int Ptoken_exist = NOT_EXIST;

	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetVideoAnalyticsConfiguration->ConfigurationToken,oSysInfo->Oprofile[i].AVAC.VAtoken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->Name = oSysInfo->Oprofile[0].AVAC.VAname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->UseCount =oSysInfo->Oprofile[0].AVAC.VAusercount;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->token = oSysInfo->Oprofile[0].AVAC.VAtoken;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = oSysInfo->Oprofile[0].AVAC.VAE.VAMsize;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = oSysInfo->Oprofile[0].AVAC.VAE.VAsizeitem;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name = oSysInfo->Oprofile[0].AVAC.VAE.VASname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value = oSysInfo->Oprofile[0].AVAC.VAE.VASvalue;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = oSysInfo->Oprofile[0].AVAC.VAE.VAEsize;

	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name = oSysInfo->Oprofile[0].AVAC.VAE.VAEname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Name = oSysInfo->Oprofile[0].AVAC.VAE.Aconfigname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Type = oSysInfo->Oprofile[0].AVAC.VAE.Aconfigtype;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->AnalyticsEngineConfiguration->Extension = NULL;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->__sizeRule = oSysInfo->Oprofile[0].AVAC.VRE.VARsize;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->__sizeSimpleItem = oSysInfo->Oprofile[0].AVAC.VRE.VARSsizeitem;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->SimpleItem->Name = oSysInfo->Oprofile[0].AVAC.VRE.VARSname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->SimpleItem->Value = oSysInfo->Oprofile[0].AVAC.VRE.VARSvalue;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->__sizeElementItem = oSysInfo->Oprofile[0].AVAC.VRE.VAREsize;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->ElementItem->Name = oSysInfo->Oprofile[0].AVAC.VRE.VAREname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Parameters->Extension = NULL;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Name = oSysInfo->Oprofile[0].AVAC.VRE.Rconfigname;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Rule->Type = oSysInfo->Oprofile[0].AVAC.VRE.Rconfigtype;
	trt__GetVideoAnalyticsConfigurationResponse->Configuration->RuleEngineConfiguration->Extension = NULL;
	return SOAP_OK;
}

/**
 * @brief Get Meta Data Configuration
 * @This operation lists all video analytics configurations of a device.
 *
 * @param Request	:_trt__GetAudioEncoderConfiguration structure contains an empty message.
 * @param Response	:_trt__GetAudioEncoderConfigurationResponse structure contains a list of all existing video analytics configurations in the device.
 * @return               : On success - SOAP_OK.
 On failure - The requested configuration indicated with ConfigurationToken does not exist

 */
int __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	char _name[LARGE_INFO_LENGTH];
	int i = 0,token_exist = 0;
	char _IPAddr[INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);

	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
	for(i = 0; i< oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetMetadataConfiguration->ConfigurationToken, oSysInfo->Oprofile[i].MDC.MDtoken ) == 0)
		{
			token_exist = EXIST;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}


	trt__GetMetadataConfigurationResponse->Configuration = (struct tt__MetadataConfiguration *)soap_malloc(soap, sizeof(struct tt__MetadataConfiguration));
	trt__GetMetadataConfigurationResponse->Configuration[0].Name = oSysInfo->Oprofile[i].MDC.MDname;//"Face Detection";
	trt__GetMetadataConfigurationResponse->Configuration[0].UseCount = oSysInfo->Oprofile[i].MDC.MDusecount;//1;
	trt__GetMetadataConfigurationResponse->Configuration[0].token = oSysInfo->Oprofile[i].MDC.MDtoken;//"Face Detection";
	trt__GetMetadataConfigurationResponse->Configuration[0].PTZStatus = NULL;// (struct tt__PTZFilter*)soap_malloc(soap,sizeof(struct tt__PTZFilter));
	trt__GetMetadataConfigurationResponse->Configuration[0].Events = NULL;// (struct tt__EventSubscription*)soap_malloc(soap,sizeof(struct tt__EventSubscription));
	trt__GetMetadataConfigurationResponse->Configuration[0].Analytics = NULL;//(int *)soap_malloc(soap,sizeof(int));
	trt__GetMetadataConfigurationResponse->Configuration[0].Analytics = (int *)soap_malloc(soap, sizeof(int));
	*trt__GetMetadataConfigurationResponse->Configuration[0].Analytics = 1;
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address->Type = oSysInfo->Oprofile[i].MDC.VMC.nIPType;
		trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(*trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address->IPv4Address, _IPAddr);
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Address->IPv6Address = NULL;
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->Port = oSysInfo->Oprofile[i].MDC.VMC.port;
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->TTL = oSysInfo->Oprofile[i].MDC.VMC.ttl;
	trt__GetMetadataConfigurationResponse->Configuration[0].Multicast->AutoStart = oSysInfo->Oprofile[i].MDC.VMC.autostart;

	return SOAP_OK;
}

int __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Compatible Video Encoder Configurations
 * @brief This operation lists all the video encoder configurations of the NVT that are compatible with a certain media profile.
 *
 * @param Request	:_trt__GetCompatibleVideoEncoderConfigurations structure contains the token of an existing media profile.
 * @param Response	:_trt__GetCompatibleVideoEncoderConfigurationsResponse structure contains a list of video encoder configurations that are compatible with the given medi                            a profile.
 * @return               : On success -  SOAP_OK.
 : On failure - The requested profile token ProfileToken does not exist.
 */
int __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
	char _IPAddr[INFO_LENGTH];
	int i = 0;
	int flag = 0;
	int Ptoken_exist = 0;
	SysInfo *oSysInfo = GetSysInfo();
	/* Read Settings */
	NET_IPV4 ip;
	int numberofstream = oget_numberofstream();


	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

	/* VideoEncoderConfiguration */
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = numberofstream;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations =
			(struct tt__VideoEncoderConfiguration *)soap_malloc(soap, numberofstream * sizeof(struct tt__VideoEncoderConfiguration));
	for(i = 0; i < numberofstream; i++)
	{
		int bitrate = oget_bitrate(i);
		int ratecontrol = oget_ratecontrol(i);
		int framerate = oget_framerate(i);
		int width = oget_resolutionwidth(i);
		int height = oget_resolutionheight(i);
		int govlength = oget_ipratio(i);

		//fprintf(stderr, "__trt__GetCompatibleVideoEncoderConfigurations [%d] %d %d %d %d %d %d\n",i, bitrate, framerate, ratecontrol, width, height, govlength);
		if(i == 0) {
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Name = "DE_Name1"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].token = "DE_Token1"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
		} else if(i == 1) {
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Name = "DE_Name2"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].token = "DE_Token2"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
		} else if(i == 2) {
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Name = "DE_Name3"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].token = "DE_Token3"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
		} else {
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Name = "DE_Name"; //oSysInfo->Oprofile[i].AESC.VEname;//"VideoEncoderConfiguration";
			trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].token = "DE_Token"; //oSysInfo->Oprofile[i].AESC.VEtoken;//"VideoEncoderConfiguration";
		}
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].UseCount = 1;
		// {tt__VideoEncoding__JPEG = 0, onv__VideoEncoding__MPEG4 = 1, tt__VideoEncoding__H264 = 2}
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Encoding = oget_encoding(i);
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Quality = oSysInfo->Oprofile[0].AESC.VEquality; // float
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].SessionTimeout = 60;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Resolution->Width = width;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Resolution->Height = height;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].RateControl->FrameRateLimit = framerate;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].RateControl->EncodingInterval = ratecontrol; //0:vbr 1:cbr
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].RateControl->BitrateLimit = bitrate;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].MPEG4 = (struct tt__Mpeg4Configuration *)soap_malloc(soap, sizeof(struct tt__Mpeg4Configuration));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].MPEG4->GovLength = govlength;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].MPEG4->Mpeg4Profile = 0;//{onv__Mpeg4Profile__SP = 0, onv__Mpeg4Profile__ASP = 1}
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].H264->GovLength = govlength;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].H264->H264Profile = 0;//Baseline = 0, Main = 1, High = 3
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Port = 80;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->TTL = 1;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->AutoStart = 0;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address->Type = 0;
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
		strcpy(*trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address->IPv4Address, _IPAddr);
		trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations[i].Multicast->Address->IPv6Address = NULL;
	}
	return SOAP_OK;
}

/**
 * @brief Get Compatible Video Source Configurations
 * @This operation requests all the video source configurations of the NVT that are compatible with a certain media profile.
 *
 * @param Request	:_trt__GetCompatibleVideoSourceConfigurations structure contains the token of an existing media profile.
 * @param Response	:_trt__GetCompatibleVideoSourceConfigurationsResponse structure contains a list of video source configurations that are compatible with the media profile.
 * @return               : On success - SOAP_OK.
 On failure - The requested profile token ProfileToken does not exist.
 */
int __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
//	trt__GetCompatibleVideoSourceConfigurations->ProfileToken;
	int i=0;
	int Ptoken_exist = NOT_EXIST;
	SysInfo *oSysInfo = GetSysInfo();
	for(i=0;i<=oSysInfo->nprofile;i++)
	{
		if(strcmp(trt__GetCompatibleVideoSourceConfigurations->ProfileToken,oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations = 1; //MPEG4 | H264 | JPEG
	/* VideoSourceConfiguration */
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations = (struct tt__VideoSourceConfiguration *)soap_malloc(soap, 3 * sizeof(struct tt__VideoSourceConfiguration));
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Name = oSysInfo->Oprofile[i].AVSC.Vname;//"JPEG";
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].token = oSysInfo->Oprofile[i].AVSC.Vtoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->x = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->y = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->width = 1600;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[0].Bounds->height = 1200;

	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Name = oSysInfo->Oprofile[i].AVSC.Vname;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].UseCount = 1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].token = oSysInfo->Oprofile[i].AVSC.Vtoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Bounds->x = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Bounds->y = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Bounds->width = 1280;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[1].Bounds->height = 720;

	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Name = oSysInfo->Oprofile[i].AVSC.Vname;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].UseCount = 1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].token = oSysInfo->Oprofile[i].AVSC.Vtoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].SourceToken = oSysInfo->Oprofile[i].AVSC.Vsourcetoken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Bounds->x = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Bounds->y = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Bounds->width = 1280;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations[2].Bounds->height = 720;
	return SOAP_OK;
}

/**
 * @brief Get Compatible Audio Encoder Cofigurations
 * @This operation requests all audio encoder configurations of the NVT that are compatible with a certain media profile.
 *
 * @param Request	:_trt__GetCompatibleAudioEncoderConfigurations structure contains the token of an existing media profile.
 * @param Response	:_trt__GetCompatibleAudioEncoderConfigurationsResponse structure contains a list of audio encoder configurations that are compatible with the given
 media profile.
 * @return               : On success - SOAP_OK.
 On failure - The requested profile token ProfileToken does not exist. or
 NVT does not support audio.
 */
int __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _IPAddr[INFO_LENGTH];
	NET_IPV4 ip;
	int _Bitrate = 0;
	int _SampleRate = 0;
	int _Encoding = oSysInfo->audio_config.codectype;
	int i,profile_exist = 0,index = 0;
	for(i = 0 ;i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetCompatibleAudioEncoderConfigurations->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
		{
			profile_exist = EXIST;
			index = i;
			break;
		}
	}
	if(!profile_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	/* Read Settings */
	ip.int32 = net_get_ifaddr(ETH_NAME);
	sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/services", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

	/* Read Audio*/
	if(oSysInfo->audio_config.bitrate == 0)
	{
		_Bitrate = 64;
	}
	else if(oSysInfo->audio_config.bitrate == 1)
	{
		_Bitrate = 128;
	}

	if(oSysInfo->audio_config.samplerate == 0)
	{
		_SampleRate = 8;
	}
	else if(oSysInfo->audio_config.samplerate == 1)
	{
		_SampleRate = 16;
	}
	trt__GetCompatibleAudioEncoderConfigurationsResponse->__sizeConfigurations = 1;

	/* AudioEncoderConfiguration */
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Name = oSysInfo->Oprofile[0].AAEC.AEname;//"audio_encoder_config";
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->UseCount = 1;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->token = oSysInfo->Oprofile[0].AAEC.AEtoken;//"audio_encoder_config";
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Encoding = _Encoding;//{tt__AudioEncoding__G711 = 0, tt__AudioEncoding__G726 = 1, tt__AudioEncoding__AAC = 2}
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Bitrate = _Bitrate;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->SampleRate = _SampleRate;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->SessionTimeout = 60;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Port = 80;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->TTL = 1;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->AutoStart = 0;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address  = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address->Type = 0;
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address = (char **)soap_malloc(soap, sizeof(char *));
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(*trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetCompatibleAudioEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv6Address = NULL;
	return SOAP_OK;
}

/**
 * @brief Get Compatible Audio Source Configurations
 * @This operation requests all audio encoder configurations of the NVT that are compatible with a certain media profile.
 *
 * @param Request	:_trt__GetCompatibleAudioSourceConfigurations structure contains the token of an existing media profile.
 * @param Response	:_trt__GetCompatibleAudioSourceConfigurationsResponse structure contains list of audio encoder configurations that are compatible with the given media
 profile.
 * @return               : On success - SOAP_OK.
 On failure - The requested profile token ProfileToken does not exist. or
 NVT does not support audio.
 */
int __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
	trt__GetCompatibleAudioSourceConfigurationsResponse->__sizeConfigurations = 2;

	/* AudioSourceConfiguration */
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations = (struct tt__AudioSourceConfiguration*)soap_malloc(soap, 2 * sizeof(struct tt__AudioSourceConfiguration));
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);;
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken, "AAC");
	return SOAP_OK;
}

/**
 * @brief Get Compatible Video Analytics Configuratons
 * @This operation requests all video analytic configurations of the device that are compatible with a certain media profile.
 *
 * @param Request	:_trt__GetCompatibleVideoAnalyticsConfigurations structure contains the token of an existing media profile.
 * @param Response	:_trt__GetCompatibleVideoAnalyticsConfigurationsResponse structure contains a list of video analytics configurations that are compatible  with the
 given media profile.
 * @return               : On success - SOAP_OK.
 On failure - The requested profile token ProfileToken does not exist. or
 The device does not support video analytics.
 */
int __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
	char _name[LARGE_INFO_LENGTH];
	strcpy(_name, trt__GetCompatibleVideoAnalyticsConfigurations->ProfileToken);

	if(!strcmp(_name, "Face Detection"))
	{
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse-> __sizeConfigurations = 1;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations = (struct tt__VideoAnalyticsConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->Name = _name;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->UseCount = 1;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->UseCount = _name;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration = (struct tt__AnalyticsEngineConfiguration *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 1;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule = (struct tt__Config *)soap_malloc(soap, sizeof(struct tt__Config));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters = (struct tt__ItemList *)soap_malloc(soap, sizeof(struct tt__ItemList));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 0;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name = NULL;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value = NULL;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 0;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = NULL;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = (struct tt__ItemListExtension *)soap_malloc(soap, sizeof(struct tt__ItemListExtension));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->Extension = NULL;
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->Extension = (struct tt__AnalyticsEngineConfigurationExtension *)soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfigurationExtension));
		trt__GetCompatibleVideoAnalyticsConfigurationsResponse->Configurations->AnalyticsEngineConfiguration->Extension = NULL;
	}
	else
	{
		/* if ProfileToken does not exist */
		onvif_fault(soap,"ter:ProfileToken Mismatch", "ter:ProfileToken Mismatch");
		return SOAP_FAULT;
	}
	return SOAP_OK;
}

/**
 * @brief Get Compatible Metadata Configurations.
 * @This operation requests all the metadata configurations of the device that are compatible with a certain media profile.
 *
 * @param Request        :_trt__GetCompatibleMetadataConfigurations structure contains the token of an existing media profile.
 * @param Response       :_trt__GetCompatibleMetadataConfigurationsResponse structure contains a list of metadata configurations that are compatible with the
 given media profile.

 * @return               : On success - SOAP_OK.
 On failure - The requested profile token ProfileToken does not exist.
 */

int __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	trt__GetCompatibleMetadataConfigurationsResponse->__sizeConfigurations = 1;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations = (struct tt__MetadataConfiguration *)soap_malloc(soap, 2 * sizeof(struct tt__MetadataConfiguration));
	if((oSysInfo->Oprofile[1].profiletoken != NULL) && (oSysInfo->Oprofile[1].profilename != NULL))
	{
		trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Name = oSysInfo->Oprofile[1].profilename;
		trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].token = oSysInfo->Oprofile[1].profiletoken;
	}
	else
	{
		trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Name = oSysInfo->Oprofile[0].profilename;
		trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].token = oSysInfo->Oprofile[0].profiletoken;
	}
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].PTZStatus = (struct tt__PTZFilter *)soap_malloc(soap, sizeof(struct tt__PTZFilter));
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].PTZStatus = NULL;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Events = (struct tt__EventSubscription *)soap_malloc(soap, sizeof(struct tt__EventSubscription));
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Events = NULL;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Analytics = (int *)soap_malloc(soap, sizeof(int));
	*trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Analytics = 1;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->Address = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->Address->Type = 0;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->Address->IPv4Address = NULL;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->Address->IPv6Address = NULL;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->Port = 0;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->TTL = 0;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].Multicast->AutoStart = 0;
	trt__GetCompatibleMetadataConfigurationsResponse->Configurations[0].SessionTimeout = 720000;
	return SOAP_OK;
}

int __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Set Video Source Configuration
 * @This operation modifies a video input configuration.
 *
 * @param Request	:_trt__SetVideoSourceConfiguration structure contains the configuration element contains the modified VideoSource configuration.
 * @param Response	:_trt__SetVideoSourceConfigurationResponse structure members contains message is empty.
 * @return               : On success - SOAP_OK.
 On failure - The requested VideoSource does not exist or
 The configuration parameters are not possible to set.
 */

int __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	video_source_conf set_video_source_t;
	int i;
	int j = 0;
	int flg = 0;
	int Ptoken_exist = NOT_EXIST;
	int ret;
	int num_token = 0;
	__u8 codec_combo;
	__u8 codec_res;
	__u8 mode = 1;
	 int _width;
	 int _height;
	 if(trt__SetVideoSourceConfiguration->Configuration->Bounds != NULL)
	 {
		 _width = trt__SetVideoSourceConfiguration->Configuration->Bounds->width;
		 _height = trt__SetVideoSourceConfiguration->Configuration->Bounds->height;
	 }
	for(i = 0; i < oSysInfo->nprofile ; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vtoken, oSysInfo->Oprofile[i].AVSC.Vtoken)==0);
			{
				flg = 1;
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}

	if(trt__SetVideoSourceConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__SetVideoSourceConfiguration->Configuration->token, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
			{
				Ptoken_exist = EXIST;
				break;
			}
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}

	/* check for width and height */
	if(!((_width >= 320 && _width <= 1920) && (_height >= 192 && _height <= 1080)))
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(trt__SetVideoSourceConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i< oSysInfo->nprofile; i++)
		{
			Ptoken_exist = NOT_EXIST;
			if(strcmp(trt__SetVideoSourceConfiguration->Configuration->token, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
			{
				Ptoken_exist = EXIST;
			}

			if(Ptoken_exist)
			{

				set_video_source_t.position = i;
				strcpy(set_video_source_t.video_source_conf_in_t.Vtoken_t, trt__SetVideoSourceConfiguration->Configuration->token);
				strcpy(set_video_source_t.video_source_conf_in_t.Vname_t, trt__SetVideoSourceConfiguration->Configuration->Name);
				strcpy(set_video_source_t.video_source_conf_in_t.Vsourcetoken_t, trt__SetVideoSourceConfiguration->Configuration->SourceToken);
				set_video_source_t.video_source_conf_in_t.Vcount_t = trt__SetVideoSourceConfiguration->Configuration->UseCount;
				if(trt__SetVideoSourceConfiguration->Configuration->Bounds != NULL)
				{
					set_video_source_t.video_source_conf_in_t.windowx_t = trt__SetVideoSourceConfiguration->Configuration->Bounds->x;
					set_video_source_t.video_source_conf_in_t.windowy_t = trt__SetVideoSourceConfiguration->Configuration->Bounds->y;
					set_video_source_t.video_source_conf_in_t.windowwidth_t = _width;
					set_video_source_t.video_source_conf_in_t.windowheight_t = _height;
				}

				ret = ControlSystemData(SFIELD_SET_VIDEOSOURCE_CONF, (void *)&set_video_source_t, sizeof(set_video_source_t));
			}
		}
	}
	if(ret != SUCCESS)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(oSysInfo->Oprofile[i].AESC.Vencoder == 0)
	{
		codec_combo = 2;
		if((_width == 1600) && (_height == 1200)) // 2MP = 1600x1200
		{
			codec_res = 1;
		}
		else if((_width == 2048) && (_height == 1536)) // 3MP = 2048x1536
		{
			codec_res = 3;
		}
		else if(((_width == 2592) || (_height == 2560)) && (_height == 1920)) // 5MP = 2592x1920
		{
			codec_res = 5;
		}
		else
		{
			codec_res = 1;
		}
	}
	else if(oSysInfo->Oprofile[i].AESC.Vencoder == 1)
	{
		codec_combo = 1;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
		{
			codec_res = 0;
		}
		else if((_width == 720) && (_height == 480)) // D1 = 720x480
		{
			codec_res = 1;
		}
		else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
		{
			codec_res = 2;
		}
		else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
		{
			codec_res = 3;
		}
		else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
		{
			codec_res = 4;
		}
		else codec_res = 0;
	}
	else if(oSysInfo->Oprofile[i].AESC.Vencoder == 2)
	{
		codec_combo = 0;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
		{
			codec_res = 0;
		}
		else if((_width == 720) && (_height == 480)) // D1 = 720x480
		{
			codec_res = 1;
		}
		else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
		{
			codec_res = 2;
		}
		else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
		{
			codec_res = 3;
		}
		else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
		{
			codec_res = 4;
		}
	}
	else
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:UnabletoSetParams");
		return SOAP_FAULT;
	}

	ControlSystemData(SFIELD_SET_VIDEO_MODE, (void *)&mode, sizeof(mode));
	if(oSysInfo->lan_config.nVideocombo != codec_combo)
	{
		ControlSystemData(SFIELD_SET_VIDEOCODECCOMBO, (void *)&codec_combo, sizeof(codec_combo));
	}
	if(oSysInfo->lan_config.nVideocodecres != codec_res)
	{
		ControlSystemData(SFIELD_SET_VIDEOCODECRES, (void *)&codec_res, sizeof(codec_res));
	}


	return SOAP_OK;
}


/**
 * @brief Set Video Encoder Configuration
 * @This operation modifies a video encoder configuration.
 *
 * @param Request	:_trt__SetVideoEncoderConfiguration structure contains the configuration element contains the modified video encoder configuration.
 * @param Response	:_trt__SetVideoEncoderConfigurationResponse structure contains message is empty.
 * @return               : On success - SOAP_OK.
 On failure - The configuration does not exist.or
 The configuration parameters are not possible to set.
 */
int __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
        int i=0;
	int Ptoken_exist = NOT_EXIST;
	int ret;
        __u8 codec_combo = 0;
        __u8 codec_res = 0;
        __u8 mode = 1;
        __u8 encodinginterval = 0;
        __u8 frameratelimit = 0;
        int _width = 0;
        int _height = 0;
        int bitrate = 0;
	int govlength = 0;

	video_encoder_conf set_video_encoder_conf;
	if(trt__SetVideoEncoderConfiguration->Configuration != NULL)
	{
		if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
		{
			_width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
			_height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
		}
	}
	if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
	{
		encodinginterval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
		bitrate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
                bitrate *= 1000;
		frameratelimit = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
	}
	for(i = 0; i <= oSysInfo->nprofile; i++)
        {
		if(strcmp(trt__SetVideoEncoderConfiguration->Configuration->token, oSysInfo->Oprofile[i].AESC.VEtoken)==0)
              	{
	      		Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}
	if( ( (_width % 2) != 0 || (_height % 2) != 0 ) || ((_width * _height) % 16 != 0))
         {
 		onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
                return SOAP_FAULT;
         }


        if(trt__SetVideoEncoderConfiguration->Configuration != NULL)
        {
		strcpy(set_video_encoder_conf.video_encoder_conf_in_t.VEname_t, trt__SetVideoEncoderConfiguration->Configuration->Name);
		strcpy(set_video_encoder_conf.video_encoder_conf_in_t.VEtoken_t, trt__SetVideoEncoderConfiguration->Configuration->token);
		set_video_encoder_conf.video_encoder_conf_in_t.VEusercount_t = trt__SetVideoEncoderConfiguration->Configuration->UseCount;
		set_video_encoder_conf.video_encoder_conf_in_t.VEquality_t = trt__SetVideoEncoderConfiguration->Configuration->Quality;
		set_video_encoder_conf.video_encoder_conf_in_t.Vencoder_t = trt__SetVideoEncoderConfiguration->Configuration->Encoding;
        if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
        {
		set_video_encoder_conf.video_encoder_conf_in_t.Rwidth_t = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
		set_video_encoder_conf.video_encoder_conf_in_t.Rheight_t = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
        }
		if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
		{
			set_video_encoder_conf.video_encoder_conf_in_t.frameratelimit_t = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
			set_video_encoder_conf.video_encoder_conf_in_t.encodinginterval_t = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
			set_video_encoder_conf.video_encoder_conf_in_t.bitratelimit_t = bitrate;
		}
		else
		{
			set_video_encoder_conf.video_encoder_conf_in_t.frameratelimit_t = 0;//trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
			set_video_encoder_conf.video_encoder_conf_in_t.encodinginterval_t = 0;//trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
			set_video_encoder_conf.video_encoder_conf_in_t.bitratelimit_t = 0;//bitrate;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
		{
			set_video_encoder_conf.video_encoder_conf_in_t.Mpeggovlength_t = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->GovLength;
			set_video_encoder_conf.video_encoder_conf_in_t.Mpegprofile_t  = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->Mpeg4Profile;
		}
		else
		{
			set_video_encoder_conf.video_encoder_conf_in_t.Mpeggovlength_t =  oSysInfo->Oprofile[i].AESC.Mpeggovlength;
			set_video_encoder_conf.video_encoder_conf_in_t.Mpegprofile_t = oSysInfo->Oprofile[i].AESC.Mpegprofile;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
		{
			set_video_encoder_conf.video_encoder_conf_in_t.H264govlength_t = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
			set_video_encoder_conf.video_encoder_conf_in_t.H264profile_t = trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile;
		}
		else
		{
			set_video_encoder_conf.video_encoder_conf_in_t.H264govlength_t =  oSysInfo->Oprofile[i].AESC.H264govlength;
			set_video_encoder_conf.video_encoder_conf_in_t.H264profile_t = oSysInfo->Oprofile[i].AESC.H264profile;
		}

		set_video_encoder_conf.video_encoder_conf_in_t.sessiontimeout_t = trt__SetVideoEncoderConfiguration->Configuration->SessionTimeout;
		if(trt__SetVideoEncoderConfiguration->Configuration->Multicast != NULL)
		{
			set_video_encoder_conf.video_encoder_conf_in_t.Multicast_Conf_t.nIPType_t = trt__SetVideoEncoderConfiguration->Configuration->Multicast->Address->Type;
			strcpy(set_video_encoder_conf.video_encoder_conf_in_t.Multicast_Conf_t.IPv4Addr_t, *trt__SetVideoEncoderConfiguration->Configuration->Multicast->Address->IPv4Address);
			set_video_encoder_conf.video_encoder_conf_in_t.Multicast_Conf_t.port_t = trt__SetVideoEncoderConfiguration->Configuration->Multicast->Port;
			set_video_encoder_conf.video_encoder_conf_in_t.Multicast_Conf_t.ttl_t = trt__SetVideoEncoderConfiguration->Configuration->Multicast->TTL;
			set_video_encoder_conf.video_encoder_conf_in_t.Multicast_Conf_t.autostart_t = trt__SetVideoEncoderConfiguration->Configuration->Multicast->AutoStart;
		}
	}
	for(i = 0 ;i < oSysInfo->nprofile; i++)
	{
		Ptoken_exist = NOT_EXIST;
		if(strcmp(trt__SetVideoEncoderConfiguration->Configuration->token, oSysInfo->Oprofile[i].AESC.VEtoken)==0)
		{
			Ptoken_exist = EXIST;
		}
		if(Ptoken_exist)
		{
			set_video_encoder_conf.position = i;
			ret = ControlSystemData(SFIELD_SET_VIDEOENCODER_CONF, (void *)&set_video_encoder_conf, sizeof(set_video_encoder_conf));
		}
	}
	if(ret != SUCCESS)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;

        }
	if (trt__SetVideoEncoderConfiguration->Configuration->Encoding == 0)
	{
		codec_combo = 2;
		if((_width == 1600) && (_height == 1200)) // 2MP = 1600x1200
                {
                        codec_res = 1;
                }
                else if((_width == 2048) && (_height == 1536)) // 3MP = 2048x1536
                {
                        codec_res = 3;
                }
                else if(((_width == 2592) || (_height == 2560)) && (_height == 1920)) // 5MP = 2592x1920
                {
                        codec_res = 5;
                }
                else
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;
		}
	}
	else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == 1)
	{
		codec_combo = 1;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
                {
                        codec_res = 0;
                }
                else if((_width == 720) && (_height == 480)) // D1 = 720x480
                {
                        codec_res = 1;
                }
                else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
                {
                        codec_res = 2;
                }
                else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
                {
                        codec_res = 3;
                }
                else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
                {
                        codec_res = 4;
                }
                else
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
		{
                	govlength = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->GovLength;
		}
        	if((govlength<0) || (govlength>30))
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;
		}

	}
	else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == 2)
	{
			codec_combo = 0;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
                {
			codec_combo = 0;
                        codec_res = 0;
                }
                else if((_width == 720) && (_height == 480)) // D1 = 720x480
                {
			codec_combo = 0;
                        codec_res = 1;
                }
                else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
                {
			codec_combo = 0;
                        codec_res = 2;
                }
                else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
                {
			codec_combo = 0;
                        codec_res = 3;
                }
                else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
                {
			codec_combo = 0;
                        codec_res = 4;
                }
		else if((_width == 1600) && (_height == 1200)) // 2MP 1600x1200
		{
			codec_combo = 2;
                        codec_res = 0;
		}
		else if((_width == 2048) && (_height == 1536)) // 3MP 2048x1536
		{
			codec_combo = 2;
                        codec_res = 2;
		}
		else if((_width == 2592) && (_height == 1920)) // 5MP 2592x1920
		{
			codec_combo = 2;
                        codec_res = 4;
		}
		else
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
			return SOAP_FAULT;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
		{
                	govlength = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
		}
        	if((govlength<0) || (govlength>30))
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
			return SOAP_FAULT;
		}

        }

        ControlSystemData(SFIELD_SET_VIDEO_MODE, (void *)&mode, sizeof(mode));
        ControlSystemData(SFIELD_SET_VIDEOCODECCOMBO, (void *)&codec_combo, sizeof(codec_combo));
        ret = ControlSystemData(SFIELD_SET_VIDEOCODECRES, (void *)&codec_res, sizeof(codec_res));
	    ControlSystemData(SFIELD_SET_MPEG41_BITRATE, (void *)&bitrate, sizeof(bitrate));
        oset_framerate(codec_combo,codec_res,frameratelimit);
        return SOAP_OK;

}


/**
 * @brief Set Audio Source Configuration
 * @This operation modifies an audio source configuration.

 * @param Request	:_trt__SetAudioSourceConfiguration structure configuration element contains the modified audio source configuration.
 * @param Response	:_trt__SetAudioSourceConfigurationResponse structure contains message is empty.
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.7.5)
 */
int __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _Token[SMALL_INFO_LENGTH];
	char _Name[SMALL_INFO_LENGTH];
	char _SourceToken[SMALL_INFO_LENGTH];
	int i;
	int flag = 0;
	Add_AudioSource_Configuration_tmp Add_Audio_Conf;
	__u8 _Encoding;

	strcpy(_SourceToken, trt__SetAudioSourceConfiguration->Configuration->SourceToken);
	strcpy(_Token, trt__SetAudioSourceConfiguration->Configuration->token);
	strcpy(_Name, trt__SetAudioSourceConfiguration->Configuration->Name);

	if(strcmp(_Token,"G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap,"ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(_SourceToken, oSysInfo->Oprofile[i].AASC.Asourcetoken) == 0)
		{
			flag = EXIST;
			break;
		}
	}
	if(!flag)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(strcmp(_SourceToken, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(oSysInfo->audio_config.codectype != _Encoding)
	{
		ControlSystemData(SFIELD_SET_AUDIO_ENCODE, (void *)&_Encoding, sizeof(_Encoding));
	}
	strcpy(Add_Audio_Conf.Add_AudioSource_in.Aname_t, _Name);
	strcpy(Add_Audio_Conf.Add_AudioSource_in.Atoken_t, _Token);
	strcpy(Add_Audio_Conf.Add_AudioSource_in.Asourcetoken_t, _SourceToken);
	flag = NOT_EXIST;
	for(i = 0; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(_Token, oSysInfo->Oprofile[i].AASC.Atoken) == 0)
		{
			flag = EXIST;
			Add_Audio_Conf.position = i;
			break;
		}

		if(flag)
		{
			ControlSystemData(SFIELD_SET_AUDIO_SOURCE, (void *)&Add_Audio_Conf, sizeof(Add_Audio_Conf));
		}
	}
	return SOAP_OK;
}

/**
 * @brief Set Audio Encoder Configuration
 * @This operation modifies an audio encoder configuration.
 *
 * @param Request	:_trt__SetAudioEncoderConfiguration structure contains the configuration element contains the modified audio encoderconfiguration.
 * @param Response	:_trt__SetAudioEncoderConfigurationResponse structure contains message is empty.
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.8.5)
 */
int __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	__u8 _Bitrate = trt__SetAudioEncoderConfiguration->Configuration->Bitrate;
	__u8 _SampleRate = trt__SetAudioEncoderConfiguration->Configuration->SampleRate;
	__u8 _Encoding = trt__SetAudioEncoderConfiguration->Configuration->Encoding;//enum ns5__AudioEncoding {ns5__AudioEncoding__G711 = 0, ns5__AudioEncoding__G726 = 1, ns5__AudioEncoding__AAC = 2}
	__u8 _Enable = 1;
	__u8 _Mode = 2;//0 - Mic, 1-Speaker ,2-Both
	Add_AudioEncoder_Configuration_tmp add_audio_encoder;
	int i;
	int flag = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__SetAudioEncoderConfiguration->Configuration->token, oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			flag = EXIST;
			break;
		}
	}
	if(!flag)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	if(_Encoding == 0 || _Encoding == 1) //G711 || G726
	{
		_Encoding = 0;
		if(_Bitrate == 64)
		{
			_Bitrate = 0;
		}
		else if(_Bitrate == 128)
		{
			_Bitrate = 1;
		}
		else
		{
			onvif_fault(soap, "ter:Action", "ter:Bitrate is not valid");
			return SOAP_FAULT;
		}
	}
	else if(_Encoding == 2) //AAC LC
	{
		_Encoding = 1;
		if(_Bitrate == 64)
		{
			_Bitrate = 0;
		}
		else
		{
			onvif_fault(soap, "ter:Action", "ter:Bitrate is not valid");
			return SOAP_FAULT;
		}
	}

	if(_SampleRate == 8)
	{
		_SampleRate = 0;
	}
	else if(_SampleRate == 16)
	{
		_SampleRate = 1;
	}
	else
	{
		onvif_fault(soap, "ter:Action", "ter:SampleRate is not valid");
		return SOAP_FAULT;
	}
	strcpy(add_audio_encoder.Add_AudioEncoder_in.AEname_t, trt__SetAudioEncoderConfiguration->Configuration->Name);
	strcpy(add_audio_encoder.Add_AudioEncoder_in.AEtoken_t, trt__SetAudioEncoderConfiguration->Configuration->token);
	add_audio_encoder.Add_AudioEncoder_in.AEusercount_t = trt__SetAudioEncoderConfiguration->Configuration->UseCount;
	add_audio_encoder.Add_AudioEncoder_in.bitrate_t = trt__SetAudioEncoderConfiguration->Configuration->Bitrate;
	add_audio_encoder.Add_AudioEncoder_in.samplerate_t = trt__SetAudioEncoderConfiguration->Configuration->SampleRate;
	add_audio_encoder.Add_AudioEncoder_in.AEencoding_t = trt__SetAudioEncoderConfiguration->Configuration->Encoding;
	add_audio_encoder.multicast_in.nIPType_t = trt__SetAudioEncoderConfiguration->Configuration->Multicast->Address->Type;
	strcpy(add_audio_encoder.multicast_in.IPv4Addr_t, *trt__SetAudioEncoderConfiguration->Configuration->Multicast->Address->IPv4Address);
	add_audio_encoder.multicast_in.port_t = trt__SetAudioEncoderConfiguration->Configuration->Multicast->Port;
	add_audio_encoder.multicast_in.ttl_t = trt__SetAudioEncoderConfiguration->Configuration->Multicast->TTL;
	add_audio_encoder.multicast_in.autostart_t = trt__SetAudioEncoderConfiguration->Configuration->Multicast->AutoStart;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__SetAudioEncoderConfiguration->Configuration->token, oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
		{
			flag = EXIST;
		}
		if(flag)
		{

			add_audio_encoder.position = i;
			ControlSystemData(SFIELD_SET_AUDIOENCODER_CONF, (void *)&add_audio_encoder, sizeof(add_audio_encoder));
		}
	}

	if(oSysInfo->audio_config.audioenable != _Enable)
	{
		ControlSystemData(SFIELD_SET_AUDIOENABLE, (void *)&_Enable, sizeof(_Enable));
		ControlSystemData(SFIELD_SET_AUDIOMODE, (void *)&_Mode, sizeof(_Mode));
	}
	if(oSysInfo->audio_config.codectype != _Encoding)
	{
		ControlSystemData(SFIELD_SET_AUDIO_ENCODE, (void *)&_Encoding, sizeof(_Encoding));
	}

	if(oSysInfo->audio_config.bitrate != _Bitrate)
	{
		ControlSystemData(SFIELD_SET_AUDIO_BITRATE, (void *)&_Bitrate, sizeof(_Bitrate));
	}

	if(oSysInfo->audio_config.samplerate != _SampleRate)
	{
		ControlSystemData(SFIELD_SET_AUDIO_SAMPLERATE, (void *)&_SampleRate, sizeof(_SampleRate));
	}
	return SOAP_OK;
}

int __trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	int Ptoken_exist;
	video_analytics_conf video_analytics;
	int i = 0;
	for(i = 1; i <= oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__SetVideoAnalyticsConfiguration->Configuration->token, oSysInfo->Oprofile[1].AVAC.VAtoken)==0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}

	video_analytics.position = i;
	if(trt__SetVideoAnalyticsConfiguration->Configuration != NULL)
	{
		strcpy(video_analytics.video_analytics_conf_t.VAname_t, trt__SetVideoAnalyticsConfiguration->Configuration->Name);
		strcpy(video_analytics.video_analytics_conf_t.VAtoken_t, trt__SetVideoAnalyticsConfiguration->Configuration->token);
		video_analytics.video_analytics_conf_t.VAusercount_t = trt__SetVideoAnalyticsConfiguration->Configuration->UseCount;
		if(trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration != NULL)
		{
			video_analytics.video_analytics_conf_t.VAMsize_t = trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->__sizeAnalyticsModule;
			if(trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule != NULL)
			{
				if(trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters != NULL)
				{
					video_analytics.video_analytics_conf_t.VAsizeitem_t = trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem;
					if(trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem != NULL)
					{
						strcpy(video_analytics.video_analytics_conf_t.VASname_t, trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name);
						strcpy(video_analytics.video_analytics_conf_t.VASvalue_t, trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value);
					}
					video_analytics.video_analytics_conf_t.VAEsize_t =trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem;
					if(trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem != NULL);
					{
						strcpy(video_analytics.video_analytics_conf_t.VAEname_t, trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name);
					}

					strcpy(video_analytics.video_analytics_conf_t.Aconfigname_t, trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Name);
					strcpy(video_analytics.video_analytics_conf_t.Aconfigtype_t, trt__SetVideoAnalyticsConfiguration->Configuration->AnalyticsEngineConfiguration->AnalyticsModule->Type);
				}
			}
		}

		if(trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration != NULL)
		{
			video_analytics.video_analytics_conf_t.VARsize_t = trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->__sizeRule;
			if(trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule != NULL)
			{
				if(trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters != NULL)
				{
					video_analytics.video_analytics_conf_t.VARsize_t = trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->__sizeSimpleItem;
					if(trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->SimpleItem != NULL)
					{
						strcpy(video_analytics.video_analytics_conf_t.VARSname_t, trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->SimpleItem->Name);
						strcpy(video_analytics.video_analytics_conf_t.VARSvalue_t, trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->SimpleItem->Value);
					}
					video_analytics.video_analytics_conf_t.VAREsize_t =trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->__sizeElementItem;
					if(trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->ElementItem != NULL);
					{
						strcpy(video_analytics.video_analytics_conf_t.VAREname_t, trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Parameters->ElementItem->Name);
					}

					strcpy(video_analytics.video_analytics_conf_t.Rconfigname_t, trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Name);
					strcpy(video_analytics.video_analytics_conf_t.Rconfigtype_t, trt__SetVideoAnalyticsConfiguration->Configuration->RuleEngineConfiguration->Rule->Type);
				}
			}
		}
	}

	ControlSystemData(SFIELD_SET_VIDEOANALYTICS_CONF, (void *)&video_analytics, sizeof(video_analytics));
	return SOAP_OK;
}

int __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	Metadata_configuration_tmp metadata;
	int i;
	int ptoken_exist = NOT_EXIST;
	int ret = 0;
	if(trt__SetMetadataConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__SetMetadataConfiguration->Configuration->token, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
			{
				ptoken_exist = EXIST;
				break;
			}
		}
	}
	/* if ConfigurationToken does not exist */
	if(!ptoken_exist)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}

	for(i = 0;i <= oSysInfo->nprofile; i++)
	{
		ptoken_exist = NOT_EXIST;
		if(strcmp(trt__SetMetadataConfiguration->Configuration->token, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
		{
			ptoken_exist = EXIST;

		}

	if(ptoken_exist)
		{
		metadata.position = i;
		strcpy(metadata.MDC_in.MDname_t,trt__SetMetadataConfiguration->Configuration->Name);
		strcpy(metadata.MDC_in.MDtoken_t,trt__SetMetadataConfiguration->Configuration->token);
		metadata.MDC_in.MDusecount_t = trt__SetMetadataConfiguration->Configuration->UseCount;
		metadata.MDC_in.MDanalytics_t = *trt__SetMetadataConfiguration->Configuration->Analytics;
		ret = periodtol(trt__SetMetadataConfiguration->Configuration->SessionTimeout);
		if(ret == -1)
		{
			onvif_fault(soap, "ter:InvalidArgVal", "ter:ConfigModify");
			return SOAP_FAULT;
		}
		metadata.MDC_in.sessiontimeout_t = trt__SetMetadataConfiguration->Configuration->SessionTimeout;
		metadata.MDC_in.VMC_t.nIPType_t = trt__SetMetadataConfiguration->Configuration->Multicast->Address->Type;
		if(trt__SetMetadataConfiguration->Configuration->Multicast->Address->IPv4Address != NULL)
		{
			strcpy(metadata.MDC_in.VMC_t.IPv4Addr_t,*trt__SetMetadataConfiguration->Configuration->Multicast->Address->IPv4Address);
		}
		metadata.MDC_in.VMC_t.port_t = trt__SetMetadataConfiguration->Configuration->Multicast->Port;
		metadata.MDC_in.VMC_t.ttl_t  = trt__SetMetadataConfiguration->Configuration->Multicast->TTL;
		metadata.MDC_in.VMC_t.autostart_t = trt__SetMetadataConfiguration->Configuration->Multicast->AutoStart;
		ControlSystemData(SFIELD_SET_METADATA, (void*)&metadata, sizeof(metadata));
	}
	}
	return SOAP_FAULT;
}

int __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

/**
 * @brief Get Video Source Configuration
 * @This operation returns the available options when the video source parameters are reconfigured If a video source configuration is specified,the options shall concern that
 particular configuration.
 *
 * @param Request	:_trt__GetVideoSourceConfigurationOptions structure contains optional tokens of a video source configuration and a media profile.
 * @param Response	:_trt__GetVideoSourceConfigurationOptionsResponse structure contains the video configuration options.
 * @return               : On success - SOAP_OK.
 On failure - The requested Video Input does not exist
 */
int __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{


	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int num_token = 0;
	int j;
	int flag = NOT_EXIST;
	int flg = 0;
	int index = 0;
	int num = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].AVSC.Vtoken,oSysInfo->Oprofile[i].AVSC.Vtoken) == 0);
			{
				flg = 1;
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	if(trt__GetVideoSourceConfigurationOptions->ConfigurationToken != NULL)
	{
	for(i = 0; i <= oSysInfo->nprofile ; i++)
	{
		if(strcmp(trt__GetVideoSourceConfigurationOptions->ConfigurationToken,oSysInfo->Oprofile[i].AVSC.Vtoken) == 0);
			{
				flag = EXIST;
				index = j;
				break;
			}
	}
	}

	if(trt__GetVideoSourceConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__GetVideoSourceConfigurationOptions->ProfileToken, oSysInfo->Oprofile[i].profiletoken)==0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}

	if(!flag)
	{
		return SOAP_FAULT;
	}
	else
	{
		trt__GetVideoSourceConfigurationOptionsResponse->Options = (struct tt__VideoSourceConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfigurationOptions));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange = (struct tt__IntRectangleRange *)soap_malloc(soap, sizeof(struct tt__IntRectangleRange));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min = 320;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max = 1920;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min = 192;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max = 1080;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable = (char **)soap_malloc(soap, sizeof(char *) * 1);
		if(trt__GetVideoSourceConfigurationOptions->ProfileToken == NULL && trt__GetVideoSourceConfigurationOptions->ConfigurationToken == NULL)
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = num_token;
			for(i = 0;i < oSysInfo->nprofile; i++)
			{
				for(j = 0; j < i; j++)
				{
					if(strcmp(oSysInfo->Oprofile[j].AVSC.Vtoken, oSysInfo->Oprofile[i].AVSC.Vtoken) == 0)
					{
						flg = 1;
						break;
					}
				}
				if(flg == 0)
				{

				trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
				strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num], oSysInfo->Oprofile[i].AVSC.Vsourcetoken);
				num++;
				}
				flg = 0;
			}
		}
		else
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = 1;
			trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
			strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], oSysInfo->Oprofile[index].AVSC.Vsourcetoken);
		}
	}


	return SOAP_OK;
}

/**
 * @brief Get Video Encoder COnfiguration Options.
 * @This operation returns the available options when the video encoder parameters are reconfigured.
 *
 * @param Request	:_trt__GetVideoEncoderConfigurationOptions structure contains  optional tokens of a video encoder configuration and a media profile.
 * @param Response	:_trt__GetVideoEncoderConfigurationOptionsResponse structure contains the video configuration options.
 * @return               : On success - SOAP_OK.
 On failure - No command specific faults!
 */

int __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
	int i;
	char JPEG_profile;
	char MPEG4_profile;
	char H264_profile;
	SysInfo *oSysInfo = GetSysInfo();
	trt__GetVideoEncoderConfigurationOptionsResponse->Options  = (struct tt__VideoEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG  = NULL;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 = NULL;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264  = NULL;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension  = NULL;

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension = (struct                              tt__VideoEncoderOptionsExtension *)soap_malloc(soap, sizeof(struct tt__VideoEncoderOptionsExtension));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->JPEG  = NULL;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264  = NULL;
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4  = NULL;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 10; //dummy
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 100; //dummy

	char PToken[SMALL_INFO_LENGTH];
	char CToken[SMALL_INFO_LENGTH];

	if(trt__GetVideoEncoderConfigurationOptions->ProfileToken)
	{
		strcpy(PToken, trt__GetVideoEncoderConfigurationOptions->ProfileToken);
	}
	if(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken)
	{
		strcpy(CToken, trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);
	}

	for(i = 0; i < MAX_PROF; i++)
	{
		if((strcmp(PToken, oSysInfo->Oprofile[i].profiletoken) == 0) || (strcmp(CToken, oSysInfo->Oprofile[i].AESC.VEtoken) == 0))
		{
			if(oSysInfo->Oprofile[i].AESC.Vencoder == 0)
			{
				JPEG_profile = EXIST;
			}
			else if(oSysInfo->Oprofile[i].AESC.Vencoder == 1)
			{
				MPEG4_profile = EXIST;
			}
			else if(oSysInfo->Oprofile[i].AESC.Vencoder == 2)
			{
				H264_profile = EXIST;
			}
		}
	}
/*
	if(JPEG_profile)
	{
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG  = (struct tt__JpegOptions *)soap_malloc(soap, sizeof(struct tt__JpegOptions));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->__sizeResolutionsAvailable = 3; // 2MP(1600x1200) | 3MP(2048x1536) | 5MP(2592x1920)
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable  = (struct tt__VideoResolution *)soap_malloc(soap, 3 * sizeof(struct tt__VideoResolution));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[0].Width  = 1600;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[0].Height = 1200;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[1].Width  = 2048;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[1].Height = 1536;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[2].Width  = 2592;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[2].Height = 1920;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange  = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Min = 8; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Max = 30; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange  = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Min = 0; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Max = 3; //dummy
	}
    */

	if(MPEG4_profile)
	{
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4  = (struct tt__Mpeg4Options *)soap_malloc(soap, sizeof(struct tt__Mpeg4Options));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 = (struct                   tt__Mpeg4Options *)soap_malloc(soap, sizeof(struct tt__Mpeg4Options));

		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->__sizeResolutionsAvailable = 2;	//720(1280x720) : D1(720x480) : SXVGA(1280x960) : 1080(1920x1080) : 720MAX60(1280x720)
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable  = (struct tt__VideoResolution *)soap_malloc(soap, 2*sizeof(struct tt__VideoResolution));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[0].Width  = 1280;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[0].Height = 720;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[1].Width  = 720;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[1].Height = 480;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->GovLengthRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->GovLengthRange->Min = 0; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->GovLengthRange->Max = 30; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->FrameRateRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->FrameRateRange->Min = 8; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->FrameRateRange->Max = 30; //dummy
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->EncodingIntervalRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->EncodingIntervalRange->Min = 0; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->EncodingIntervalRange->Max = 3; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->__sizeMpeg4ProfilesSupported = 1;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->Mpeg4ProfilesSupported = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->Mpeg4ProfilesSupported = 0; //SP = 0, ASP = 1
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4  = (struct tt__Mpeg4Options2 *)soap_malloc(soap, sizeof(struct tt__Mpeg4Options2));


		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->ResolutionsAvailable  = NULL;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->GovLengthRange  = NULL;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->FrameRateRange  = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->EncodingIntervalRange  = NULL;
	//	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->Mpeg4ProfilesSupported = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->BitrateRange  = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->BitrateRange->Min = 64; //dummy
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->MPEG4->BitrateRange->Max = 12000; //dummy
	}


	if(H264_profile)
	{
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264  = (struct tt__H264Options *)soap_malloc(soap, sizeof(struct tt__H264Options));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 2;	//720(1280x720) : D1(720x480) : SXVGA(1280x960) : 1080(1920x1080) : 720MAX60(1280x720)
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable  = (struct tt__VideoResolution *)soap_malloc(soap,2* sizeof(struct tt__VideoResolution));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Width  = 1280;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[0].Height = 720;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Width  = 720;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[1].Height = 480;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 1; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = 30; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 8; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = 30; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange  = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 1; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 3; //dummy
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported = 3;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported = (int *)soap_malloc(soap, 3 * sizeof(int));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[0] = 0; //Baseline = 0, Main = 1, High = 3}
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[1] = 1;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[2] = 3;
		//ProfileIDC = 100 # Profile IDC (66=baseline, 77=main,
		//Z:\DM_365\IPNetCam\ipnc\av_capture\framework\alg\src\alg_vidEnc.c=pObj->params_h264.profileIdc = 100;

        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264  = (struct tt__H264Options2 *)soap_malloc(soap, sizeof(struct tt__H264Options2));
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable  = NULL;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange  = NULL;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange  = NULL;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange  = NULL;

        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange  = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min = 64; //dummy
        trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max = 12000; //dummy
	}
	return SOAP_OK;
}

/**
 * @brief Get Audio Source Configuration Options
 * @This operation returns the available options when the audio source parameters are reconfigured. If an audio source configuration is specified, the options shall concern that
 particular configuration.
 *
 * @param Request	:_trt__GetAudioSourceConfigurationOptions structure contains optional tokens of an audio source configuration and a media profile.
 * @param Response	:_trt__GetAudioSourceConfigurationOptionsResponse structure contains the audio configuration options.
 * @return               : On success - SOAP_OK.
 On failure - NVT does not support audio.
 */
int __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int token_exist = 0, profile_exist = 0;
	int i = 0;
	int index = 0;
	int _Encoding;
	char _Token[SMALL_INFO_LENGTH] = "";

	/* Response */
	if(trt__GetAudioSourceConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i < oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__GetAudioSourceConfigurationOptions->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
			{
				profile_exist = EXIST;
				index = i;
				break;
			}
		}
		if(!profile_exist)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
			return SOAP_FAULT;
		}
	}

	if(trt__GetAudioSourceConfigurationOptions->ConfigurationToken != NULL)
	{
		strcpy(_Token,trt__GetAudioSourceConfigurationOptions->ConfigurationToken);
	}

	if(_Token[0] == 0 || strcmp(_Token, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap, "ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	trt__GetAudioSourceConfigurationOptionsResponse->Options = (struct tt__AudioSourceConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__AudioSourceConfigurationOptions));
	trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable = 3;
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable = (char **)soap_malloc(soap, sizeof(char *) * trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable);

	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0], "G711");
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);;
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1], "G726");
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2], "AAC");
	trt__GetAudioSourceConfigurationOptionsResponse->Options->Extension = NULL;
	return SOAP_OK;
}

/**
 * @Get Audio Encoder COnfiguration Options.
 * @This operation returns the available options when the audio encoder parameters are reconfigured.
 *
 * @param Request	:_trt__GetAudioEncoderConfigurationOptions structure contains optional tokens of an audio encoder configuration and a media profile.
 * @param Response	:_trt__GetAudioEncoderConfigurationOptionsResponse structure contains This message contains the audio configuration options.
 * @return               : On success - SOAP_OK.
 On failure - NVT does not support audio encoder.

 */
int __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	int _Bitrate1 = 64;
	int _Samplerate1 = 8;
	int token_exist = 0, profile_exist = 0;
	int i = 0;
	int index = 0;
	int _Encoding;
	char *_Token = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	if(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken != NULL)
	{
		strcpy(_Token, trt__GetAudioEncoderConfigurationOptions->ConfigurationToken);
	}
	/* Response */
	if(trt__GetAudioEncoderConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i < oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__GetAudioEncoderConfigurationOptions->ProfileToken, oSysInfo->Oprofile[i].profiletoken) == 0)
			{
				profile_exist = EXIST;
				index = i;
				break;
			}
		}
	if(!profile_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	}
	if(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken != NULL)
	{
		for(i = 0; i < oSysInfo->nprofile; i++)
		{
			if(strcmp(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken, oSysInfo->Oprofile[i].AAEC.AEtoken) == 0)
			{
				token_exist = EXIST;
				break;
			}
		}
	}

	if(strcmp(_Token, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{
		onvif_fault(soap, "ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}
	trt__GetAudioEncoderConfigurationOptionsResponse->Options = (struct tt__AudioEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOptions));

	if(token_exist)
	{
		if(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken != NULL || trt__GetAudioEncoderConfigurationOptions->ProfileToken != NULL)
		{
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = 1;
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options = (struct tt__AudioEncoderConfigurationOption *)soap_malloc(soap,sizeof(struct tt__AudioEncoderConfigurationOption));
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].Encoding = oSysInfo->Oprofile[index].AAEC.AEencoding; //G711
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->__sizeItems = 1;
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = (int *)soap_malloc(soap,sizeof(int));
			*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = _Bitrate1;
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->__sizeItems = 1;
			trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = (int *)soap_malloc(soap, sizeof(int));
			*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = _Samplerate1;
		}
	}
	else if(strcmp(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken,"G711") == 0)
	{
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options = (struct tt__AudioEncoderConfigurationOption *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOption));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].Encoding = 0;  // "G711";
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = (int*)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = _Bitrate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList = (struct tt__IntList*)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = (int*)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = _Samplerate1;
	}
	else if(strcmp(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken,"G726") == 0)
	{
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options = (struct tt__AudioEncoderConfigurationOption *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOption));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].Encoding = 1;//"G726";
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = _Bitrate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = _Samplerate1;

	}
	else if(strcmp(trt__GetAudioEncoderConfigurationOptions->ConfigurationToken,"AAC") == 0)
	{
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options = (struct tt__AudioEncoderConfigurationOption *)soap_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOption));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].Encoding = 2;//"AAC";
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = _Bitrate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = _Samplerate1;
	}
	else
	{
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = 2;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options = (struct tt__AudioEncoderConfigurationOption *)soap_malloc(soap, 3 * sizeof(struct tt__AudioEncoderConfigurationOption));

		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].Encoding = 0; //G711
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].BitrateList->Items = _Bitrate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[0].SampleRateList->Items = _Samplerate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].Encoding = 2;//AAC
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].BitrateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].BitrateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].BitrateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].BitrateList->Items = _Bitrate1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].SampleRateList = (struct tt__IntList *)soap_malloc(soap, sizeof(struct tt__IntList));
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].SampleRateList->__sizeItems = 1;
		trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].SampleRateList->Items = (int *)soap_malloc(soap, sizeof(int));
		*trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options[1].SampleRateList->Items = _Samplerate1;
	}
	return SOAP_OK;
}

/**
 * @Get Metadata Configuration Options.
 * @This operation returns the available options for changing the metadata configuration.
 *
 * @param Request        :_trt__GetMetadataConfigurationOptions structure contains optional tokens of a metadata configuration and a media profile.
 * @param Response       :_trt__GetMetadataConfigurationOptionsResponse structure contains the metadata configuration options..
 * @return               : On success - SOAP_OK.
 On failure - The requested profile token does not exist.
 The requested configuration does not exist.


 */

int __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{

	SysInfo *oSysInfo = GetSysInfo();
	int i;
	int num_token = 0;
	int j;
	int flag = NOT_EXIST;
	int flg = 0;
	int index = 0;
	int num = 0;
	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(oSysInfo->Oprofile[j].MDC.MDtoken, oSysInfo->Oprofile[i].MDC.MDtoken)==0);
			{
				flg = 1;
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	if(trt__GetMetadataConfigurationOptions->ConfigurationToken != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile ; i++)
		{
			if(strcmp(trt__GetMetadataConfigurationOptions->ConfigurationToken, oSysInfo->Oprofile[i].MDC.MDtoken)==0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}

	if(trt__GetMetadataConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i <= oSysInfo->nprofile ; i++)
		{
			if(strcmp(trt__GetMetadataConfigurationOptions->ProfileToken, oSysInfo->Oprofile[i].profiletoken)==0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}

	if(!flag)
	{
		return SOAP_FAULT;
	}
	else
	{
		trt__GetMetadataConfigurationOptionsResponse->Options = (struct tt__MetadataConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__MetadataConfigurationOptions));
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions = (struct tt__PTZStatusFilterOptions *)soap_malloc(soap, sizeof(struct tt__PTZStatusFilterOptions));
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltStatusSupported = 0;
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomStatusSupported = 0;

		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltPositionSupported = (char *)soap_malloc(soap, sizeof(int));
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->PanTiltPositionSupported[0] = 0;
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomPositionSupported = (char *)soap_malloc(soap, sizeof(int));
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->ZoomPositionSupported[0] = 0;
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->Extension = (struct tt__PTZStatusFilterOptionsExtension *)soap_malloc(soap,sizeof(struct tt__PTZStatusFilterOptionsExtension));
		trt__GetMetadataConfigurationOptionsResponse->Options->PTZStatusFilterOptions->Extension = NULL;
	}

	return SOAP_OK;
}

int __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported");
	return SOAP_FAULT;
}

int __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
	trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->TotalNumber = 3;
	return SOAP_OK;
}

/**
 * @brief Get Stream Uri
 * @This operation requests a URI that can be used to initiate a live media stream using RTSP as the control protocol.
 *
 * @param Request	:_trt__GetStreamUri structure variable members contains two parts. StreamType defines if a unicast or multicast media stream is requested.
 * @param Response	:_trt__GetStreamUriResponse structure variables members contains the stable Uri to be used for requesting the media stream as well as parameters defining                           the lifetime of the Uri.
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.1)
 */
int __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{

	/* Read Settings */
	SysInfo* oSysInfo = GetSysInfo();
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);
	int i = 0;
	char _IPAddr[INFO_LENGTH];

	for(i = 0; i < oSysInfo->nprofile; i++)
	{
		if(strcmp(trt__GetStreamUri->ProfileToken, oSysInfo->Oprofile[i].profiletoken) != 0)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(i == oSysInfo->nprofile)
        {
                onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
                return SOAP_FAULT;
        }
        else
        {
                if(oSysInfo->Oprofile[i].AESC.Vencoder == 0)
                {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8555/PSIA/Streaming/channels/2?videoCodecType=MJPEG", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
                }
                else if(oSysInfo->Oprofile[i].AESC.Vencoder == 1)
                {
					if(strcmp(oSysInfo->Oprofile[i].AESC.VEtoken, "DE_Token1") == 0) {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8553/PSIA/Streaming/channels/2?videoCodecType=MPEG4", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
					}  else if (strcmp(oSysInfo->Oprofile[i].AESC.VEtoken, "DE_Token2") == 0) {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8554/PSIA/Streaming/channels/2?videoCodecType=MPEG4", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
                    } else {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8553/PSIA/Streaming/channels/2?videoCodecType=MPEG4", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
                    }

                }
                else if(oSysInfo->Oprofile[i].AESC.Vencoder == 2)
                {
					if(strcmp(oSysInfo->Oprofile[i].AESC.VEtoken, "DE_Token1") == 0) {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8557/PSIA/Streaming/channels/2?videoCodecType=H.264", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
					} else if (strcmp(oSysInfo->Oprofile[i].AESC.VEtoken, "DE_Token2") == 0) {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8556/PSIA/Streaming/channels/2?videoCodecType=H.264", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
                    } else {
                        sprintf(_IPAddr, "rtsp://%d.%d.%d.%d:8557/PSIA/Streaming/channels/2?videoCodecType=H.264", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);
					}
                }
        }

        if(trt__GetStreamUri->StreamSetup != NULL)
        {
                if(trt__GetStreamUri->StreamSetup->Stream == 1)
                {
                        onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidStreamSetup");
                        return SOAP_FAULT;
                }
        }

	/* Response */
	trt__GetStreamUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
	trt__GetStreamUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
        strcpy(trt__GetStreamUriResponse->MediaUri->Uri,_IPAddr);
	trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = 0;
	trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = 0;
	trt__GetStreamUriResponse->MediaUri->Timeout = 200;
	return SOAP_OK;
}

int __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
	return SOAP_FAULT;
}

int __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
	return SOAP_FAULT;
}
/* Events */
/**
 * @brief Creating PullPoint Subscription
 * @This operation creates a PullPoint subscription
 *
 * @param Request	:_ns1__CreatePullPointSubscription structure members contains the initial termination time along with the information regarding the type of event to monitor and the video information.
 * @param Response	:_ns1__CreatePullPointSubscriptionResponse structure will contain the endpoint reference which acts as the identifier
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.1)

TODO: At present faults haven't been addressed and the input structure doesn't contain the message filters.
 */

int __ns9__CreatePullPointSubscription(struct soap *soap, struct _ns1__CreatePullPointSubscription *ns1__CreatePullPointSubscription, struct _ns1__CreatePullPointSubscriptionResponse *ns1__CreatePullPointSubscriptionResponse)
{
	SysInfo *oSysInfo = GetSysInfo();
	pthread_t subscription_thread;
	int i=0;
	NET_IPV4 ip;
	char macaddr[MACH_ADDR_LENGTH];
	char _HwId[LARGE_INFO_LENGTH];
	char *temp1;
	char *temp2;
	time_t tnow = time(NULL);
	long time_out = periodtol(ns1__CreatePullPointSubscription->InitialTerminationTime);
	temp1 = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	temp2 = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	if(ns1__CreatePullPointSubscription->Filter != NULL)
	{
		if(ns1__CreatePullPointSubscription->Filter->__size == 1)
		{

			if(ns1__CreatePullPointSubscription->Filter->__any[0] != NULL)
			{
				if((temp1 = anyparser(ns1__CreatePullPointSubscription->Filter->__any[0])) == NULL)
				{
				 	onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidFilter");
					return SOAP_FAULT;
				}
			}
		}
		else
		{
			if(ns1__CreatePullPointSubscription->Filter != NULL)
        		{
				if(ns1__CreatePullPointSubscription->Filter->__any[1] != NULL)
				{
					temp2 = anyparser(ns1__CreatePullPointSubscription->Filter->__any[1]);
						if((temp2 = anyparser(ns1__CreatePullPointSubscription->Filter->__any[1])) == NULL)
						{
						        onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidFilter");
							return SOAP_FAULT;
						}

				}
			for(i = 0; i <= oSysInfo->nprofile; i++)
			{
				if(strcmp(temp2, oSysInfo->Oprofile[i].AESC.VEtoken)!=0)
				{
				        onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidMessageContentExpression");
					return SOAP_FAULT;
				}
			}

        		}
		}
	}
	net_get_hwaddr(ETH_NAME, macaddr);
	sprintf(_HwId,"urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	ns1__CreatePullPointSubscriptionResponse->SubscriptionReference.Address = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
	strcpy(ns1__CreatePullPointSubscriptionResponse->SubscriptionReference.Address, _HwId);
	ns1__CreatePullPointSubscriptionResponse->__any = NULL;
	ns1__CreatePullPointSubscriptionResponse->ns6__CurrentTime = tnow;
	ns1__CreatePullPointSubscriptionResponse->ns6__TerminationTime = tnow + time_out;
	if(canSubscribe_pullmessages)
	{
		if(pthread_create(&subscription_thread, NULL, eventtracker_thr, (void *)&time_out) != 0)
		{
			ONVIF_DBG("Error: Eventtracker thread creation failed");
		}
	}
	return SOAP_OK;
}

/**
 * @brief Getting Event properties
 * @This operation gets the event properties supported in the camera
 *
 * @param Response	:_ns1__GetEventPropertiesResponse structure will contain all the xml schema information along with the events supported by the camera
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.1)

TODO: At present faults haven't been addressed and the output structure doesn't contain the message filters.
 */
int __ns9__GetEventProperties(struct soap *soap, struct _ns1__GetEventProperties *ns1__GetEventProperties, struct _ns1__GetEventPropertiesResponse *ns1__GetEventPropertiesResponse)
{
        ns1__GetEventPropertiesResponse->__sizeTopicNamespaceLocation = 1;
        ns1__GetEventPropertiesResponse->TopicNamespaceLocation = (char **)soap_malloc(soap, sizeof(char *));
        ns1__GetEventPropertiesResponse->TopicNamespaceLocation[0] = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
        strcpy(ns1__GetEventPropertiesResponse->TopicNamespaceLocation[0], "http://www.onvif.org/onvif/ver10/topics/topicns.xml");
        ns1__GetEventPropertiesResponse->ns6__FixedTopicSet = 1;
        ns1__GetEventPropertiesResponse->ns7__TopicSet = (struct ns7__TopicSetType *)soap_malloc(soap, sizeof(struct ns7__TopicSetType));
        ns1__GetEventPropertiesResponse->ns7__TopicSet->documentation = (struct ns7__Documentation *)soap_malloc(soap,sizeof(struct ns7__Documentation));
        ns1__GetEventPropertiesResponse->ns7__TopicSet->documentation->__size = 0;
        ns1__GetEventPropertiesResponse->ns7__TopicSet->documentation->__any = NULL;
        ns1__GetEventPropertiesResponse->ns7__TopicSet->documentation->__mixed = NULL;
        ns1__GetEventPropertiesResponse->ns7__TopicSet->__size = 1;
        ns1__GetEventPropertiesResponse->ns7__TopicSet->__any = (char **)soap_malloc(soap, sizeof(char *) * ns1__GetEventPropertiesResponse->ns7__TopicSet->__size);
        ns1__GetEventPropertiesResponse->ns7__TopicSet->__any[0] = (char*)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
        strcpy(ns1__GetEventPropertiesResponse->ns7__TopicSet->__any[0],"<tns1:VideoAnalytics><tns1:MotionDetection><tns1:Motion wstop:topic=\"true\"><tt:MessageDescription><tt:Source><tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/></tt:Source><tt:Data><tt:SimpleItemDescription Name=\"ObjectId\" Type=\"tt:ObjectRefType\"/></tt:Data></tt:MessageDescription></tns1:Motion></tns1:MotionDetection></tns1:VideoAnalytics>");
        ns1__GetEventPropertiesResponse->__sizeTopicExpressionDialect = 2;
        ns1__GetEventPropertiesResponse->ns6__TopicExpressionDialect = (char **)soap_malloc(soap, sizeof(char *) * 2);
        ns1__GetEventPropertiesResponse->ns6__TopicExpressionDialect[0] = (char *)soap_malloc(soap, INFO_LENGTH);
        ns1__GetEventPropertiesResponse->ns6__TopicExpressionDialect[1] = (char *)soap_malloc(soap, INFO_LENGTH);
        strcpy(ns1__GetEventPropertiesResponse->ns6__TopicExpressionDialect[0], "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
        strcpy(ns1__GetEventPropertiesResponse->ns6__TopicExpressionDialect[1], "http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");
        ns1__GetEventPropertiesResponse->__sizeMessageContentFilterDialect = 1;
        ns1__GetEventPropertiesResponse->MessageContentFilterDialect = (char **)soap_malloc(soap, sizeof(char *));
        ns1__GetEventPropertiesResponse->MessageContentFilterDialect[0] = (char *)soap_malloc(soap, sizeof(char) * LARGE_INFO_LENGTH);
        strcpy(ns1__GetEventPropertiesResponse->MessageContentFilterDialect[0], "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");
        ns1__GetEventPropertiesResponse->__sizeProducerPropertiesFilterDialect = 0;
        ns1__GetEventPropertiesResponse->ProducerPropertiesFilterDialect = NULL;
        ns1__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation = 1;
        ns1__GetEventPropertiesResponse->MessageContentSchemaLocation = (char **)soap_malloc(soap, sizeof(char *));
        ns1__GetEventPropertiesResponse->MessageContentSchemaLocation[0] = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
        strcpy(ns1__GetEventPropertiesResponse->MessageContentSchemaLocation[0], "http://www.onvif.org/ver10/schema/onvif.xsd");
        ns1__GetEventPropertiesResponse->__size = 0;
        ns1__GetEventPropertiesResponse->__any = NULL;
        return SOAP_OK;



}
/**
 * @brief Pull Messages
 * @This operation sends a termination time and the event is checked for the duration of termination time.
 *
 * @param Response	:_ns1__PullMessagesResponse structure will contain the information related to the event which has occured
 * @return               : On success - SOAP_OK.
 On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.1)

TODO: At present faults haven't been addressed and the output structure doesn't contain the message filters.
 */

int __ns8__PullMessages(struct soap *soap, struct _ns1__PullMessages *ns1__PullMessages, struct _ns1__PullMessagesResponse *ns1__PullMessagesResponse)
{
	char macaddr[MACH_ADDR_LENGTH];
	char _HwId[LARGE_INFO_LENGTH];
	SysInfo *oSysInfo = GetSysInfo();
	NET_IPV4 ip;
	time_t tnow = time(NULL);
	net_get_hwaddr(ETH_NAME, macaddr);
	sprintf(_HwId,"urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	ns1__PullMessagesResponse->__sizeNotificationMessage = 1;
	ns1__PullMessagesResponse->CurrentTime = tnow;
	ns1__PullMessagesResponse->TerminationTime = tnow + ns1__PullMessages->Timeout;
	ns1__PullMessagesResponse->ns6__NotificationMessage = (struct ns6__NotificationMessageHolderType *) soap_malloc(soap, sizeof(struct ns6__NotificationMessageHolderType));
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference = (struct wsa5__EndpointReferenceType *)soap_malloc(soap, sizeof(struct wsa5__EndpointReferenceType));
	ns1__PullMessagesResponse->ns6__NotificationMessage->Topic = (struct ns6__TopicExpressionType *)soap_malloc(soap, sizeof(struct ns6__TopicExpressionType));
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->Address = (char *) soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->Address, _HwId);
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->ReferenceParameters = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->Metadata = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->__size = 0;
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->__any = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->SubscriptionReference->__anyAttribute = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference = (struct wsa5__EndpointReferenceType *)soap_malloc(soap, sizeof(struct wsa5__EndpointReferenceType));
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->Address = (char *) soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->Address, _HwId);
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->ReferenceParameters = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->Metadata = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->__size = 0;
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->__any = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->ProducerReference->__anyAttribute = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->Topic->__any = "tnsvendor:VideoAnalytics/MotionDetection/Detected";
	ns1__PullMessagesResponse->ns6__NotificationMessage->Topic->Dialect = "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
	ns1__PullMessagesResponse->ns6__NotificationMessage->Topic->__anyAttribute = NULL;
	ns1__PullMessagesResponse->ns6__NotificationMessage->Topic->__mixed = NULL;
	if(eventstatus[0] & MOTION_DETECTED_MASK)
	{
		ns1__PullMessagesResponse->ns6__NotificationMessage->Message.__any = "<tt:Message UtcTime=\"2008-10-10T12:24:57.321\"><tt:Source><tt:SimpleItem Name=\"VideoSourceConfigurationToken\"Value=\"1\"/><tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"2\"/><tt:SimpleItem Value=\"Motion\" Name=\"MotionDetection\"/></tt:Source><tt:Data><tt:SimpleItem Name=\"window\" Value=\"1\" /></tt:Data></tt:Message>";
	}
	else
	{
		ns1__PullMessagesResponse->ns6__NotificationMessage->Message.__any = NULL;
	}
	return SOAP_OK;
}

int __ns8__SetSynchronizationPoint(struct soap *soap, struct _ns1__SetSynchronizationPoint *ns1__SetSynchronizationPoint, struct _ns1__SetSynchronizationPointResponse *ns1__SetSynchronizationPointResponse)
{
	return SOAP_OK;
}

int __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
	return SOAP_OK;
}

/**
 * @brief Get Snapshot Uri.
 * @A Network client uses the GetSnapshotUri command to obtain a JPEG snhapshot from the NVT.
 *
 * @param Request       :_trt__GetSnapshotUri structure contains ProfileToken element indicates the media profile to use and will define the source and dimensions of the snapshot.
 * @param Response      :_trt__GetSnapshotUriResponse structure contains a stable Uri to be used for  acquiring a snapshot in JPEG format as well as parameters defining the
 lifetime of the Uri.
 * @return              : On success - SOAP_OK.
 : On failure - an appropriate fault code(ONVIF Core Spec. - Ver. 2.00 11.16.1)

 */

int __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	SysInfo* oSysInfo = GetSysInfo();
	char _IPAddr[INFO_LENGTH];
	NET_IPV4 ip;
	ip.int32 = net_get_ifaddr(ETH_NAME);

    if(ControlSystemData(SFIELD_SET_CLICK_SNAP_FILENAME, (void *)oSysInfo->lan_config.nClickSnapFilename,
            strlen(oSysInfo->lan_config.nClickSnapFilename)) < 0)
    {
        fprintf(stderr, "[onvif] Snapshot Image(%s) Save failure\n", oSysInfo->lan_config.nClickSnapFilename);
    }

	sprintf(_IPAddr, "http://%d.%d.%d.%d/sdget.htm?FILE=%s.jpg", ip.str[0], ip.str[1], ip.str[2], ip.str[3], oSysInfo->lan_config.nClickSnapFilename);
	trt__GetSnapshotUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
	trt__GetSnapshotUriResponse->MediaUri->Uri = (char *) soap_malloc(soap, sizeof(char) * INFO_LENGTH);	// dummy
	strcpy(trt__GetSnapshotUriResponse->MediaUri->Uri, _IPAddr);	// dummy
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = 0;
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot = 0;
	trt__GetSnapshotUriResponse->MediaUri->Timeout = 720000;
	return SOAP_OK;
}

int __trv__GetReceivers(struct soap *soap, struct _trv__GetReceivers *trv__GetReceivers, struct _trv__GetReceiversResponse *trv__GetReceiversResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__GetReceiver(struct soap *soap, struct _trv__GetReceiver *trv__GetReceiver, struct _trv__GetReceiverResponse *trv__GetReceiverResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__CreateReceiver(struct soap *soap, struct _trv__CreateReceiver *trv__CreateReceiver, struct _trv__CreateReceiverResponse *trv__CreateReceiverResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__DeleteReceiver(struct soap *soap, struct _trv__DeleteReceiver *trv__DeleteReceiver, struct _trv__DeleteReceiverResponse *trv__DeleteReceiverResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__ConfigureReceiver(struct soap *soap, struct _trv__ConfigureReceiver *trv__ConfigureReceiver, struct _trv__ConfigureReceiverResponse *trv__ConfigureReceiverResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__SetReceiverMode(struct soap *soap, struct _trv__SetReceiverMode *trv__SetReceiverMode, struct _trv__SetReceiverModeResponse *trv__SetReceiverModeResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __trv__GetReceiverState(struct soap *soap, struct _trv__GetReceiverState *trv__GetReceiverState, struct _trv__GetReceiverStateResponse *trv__GetReceiverStateResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:ReceiversNotSupported");
	return SOAP_FAULT;
}

int __tse__GetRecordingSummary(struct soap *soap, struct _tse__GetRecordingSummary *tse__GetRecordingSummary, struct _tse__GetRecordingSummaryResponse *tse__GetRecordingSummaryResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:RecordingSummaryNotSupported");
	return SOAP_FAULT;
}

int __tse__GetRecordingInformation(struct soap *soap, struct _tse__GetRecordingInformation *tse__GetRecordingInformation, struct _tse__GetRecordingInformationResponse *tse__GetRecordingInformationResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:RecordingSummaryNotSupported");
	return SOAP_FAULT;
}

int __tse__GetMediaAttributes(struct soap *soap, struct _tse__GetMediaAttributes *tse__GetMediaAttributes, struct _tse__GetMediaAttributesResponse *tse__GetMediaAttributesResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidToken");
	return SOAP_FAULT;
}

int __tse__FindRecordings(struct soap *soap, struct _tse__FindRecordings *tse__FindRecordings, struct _tse__FindRecordingsResponse *tse__FindRecordingsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:RecordingNotSupported");
	return SOAP_FAULT;
}

int __tse__GetRecordingSearchResults(struct soap *soap, struct _tse__GetRecordingSearchResults *tse__GetRecordingSearchResults, struct _tse__GetRecordingSearchResultsResponse *tse__GetRecordingSearchResultsResponse)
{
	onvif_fault(soap,"ter:ActionNotSupported", "ter:RecordingNotSupported");
	return SOAP_FAULT;
}

int __tse__FindEvents(struct soap *soap, struct _tse__FindEvents *tse__FindEvents, struct _tse__FindEventsResponse *tse__FindEventsResponse)
{
	onvif_fault(soap,"ter:Action", "ter:ResourceProblem");
	return SOAP_FAULT;
}

int __tse__GetEventSearchResults(struct soap *soap, struct _tse__GetEventSearchResults *tse__GetEventSearchResults, struct _tse__GetEventSearchResultsResponse *tse__GetEventSearchResultsResponse)
{
	onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidToken");
	return SOAP_FAULT;
}

int __tse__FindPTZPosition(struct soap *soap, struct _tse__FindPTZPosition *tse__FindPTZPosition, struct _tse__FindPTZPositionResponse *tse__FindPTZPositionResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tse__GetPTZPositionSearchResults(struct soap *soap, struct _tse__GetPTZPositionSearchResults *tse__GetPTZPositionSearchResults, struct _tse__GetPTZPositionSearchResultsResponse *tse__GetPTZPositionSearchResultsResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

int __tse__GetSearchState(struct soap *soap, struct _tse__GetSearchState *tse__GetSearchState, struct _tse__GetSearchStateResponse *tse__GetSearchStateResponse)
{
	onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidToken");
	return SOAP_FAULT;
}

int __tse__EndSearch(struct soap *soap, struct _tse__EndSearch *tse__EndSearch, struct _tse__EndSearchResponse *tse__EndSearchResponse)
{
	onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidToken");
	return SOAP_FAULT;
}

int __tse__FindMetadata(struct soap *soap, struct _tse__FindMetadata *tse__FindMetadata, struct _tse__FindMetadataResponse *tse__FindMetadataResponse)
{
	onvif_fault(soap, "ter:Action", "ter:ResourceProblem");
	return SOAP_FAULT;
}

int __tse__GetMetadataSearchResults(struct soap *soap, struct _tse__GetMetadataSearchResults *tse__GetMetadataSearchResults, struct _tse__GetMetadataSearchResultsResponse *tse__GetMetadataSearchResultsResponse)
{
	onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidToken");
        return SOAP_FAULT;
}

/**
 * @brief Renew a Subscription for Events
 * @ Renew request is sent to renew a Subscription for Event Notification
 * @param Request : _ns6__Renew structure containing new termination time
 * @return : On success - SOAP_OK.
 */
int __ns10__Renew(struct soap* soap, struct _ns6__Renew *ns6__Renew, struct _ns6__RenewResponse *ns6__RenewResponse)
{
	time_t time_tm;
	time_t termtime_tm;
	time_t timeout;

	timeout = periodtol(ns6__Renew->TerminationTime);
	time_out += timeout;
	t_out += timeout;
	time_tm = time(NULL);
	termtime_tm = time_tm + timeout;
	ns6__RenewResponse->TerminationTime = termtime_tm;
	ns6__RenewResponse->CurrentTime = (time_t *) soap_malloc(soap, sizeof(time_t));
	ns6__RenewResponse->CurrentTime[0] = time_tm;
	ns6__RenewResponse->__size = 0;
	ns6__RenewResponse->__any = NULL;
	return SOAP_OK;
}

/**
 * @brief Notify Events
 * @ Send Notification whenever a subscribed event occurs
 * @param Response : _ns6__Notify with appropriate event information
 * @return : On success - SOAP_OK.
 */
int __ns12__Notify(struct soap* soap, struct _ns6__Notify *ns6__Notify)
{
	char macaddr[MACH_ADDR_LENGTH];
	char _HwId[LARGE_INFO_LENGTH];

	ns6__Notify->__sizeNotificationMessage = 1;
	ns6__Notify->NotificationMessage = (struct ns6__NotificationMessageHolderType *)malloc(sizeof(struct ns6__NotificationMessageHolderType));

	ns6__Notify->NotificationMessage->SubscriptionReference	= (struct wsa5__EndpointReferenceType *)malloc(sizeof(struct wsa5__EndpointReferenceType));

	net_get_hwaddr(ETH_NAME, macaddr);
	sprintf(_HwId,"urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	ns6__Notify->NotificationMessage->SubscriptionReference->Address = (char *) soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(ns6__Notify->NotificationMessage->SubscriptionReference->Address, _HwId);
	ns6__Notify->NotificationMessage->SubscriptionReference->ReferenceParameters = NULL;

	ns6__Notify->NotificationMessage->SubscriptionReference->Metadata = NULL;
	ns6__Notify->NotificationMessage->SubscriptionReference->__size = 0;
	ns6__Notify->NotificationMessage->SubscriptionReference->__any = NULL;
	ns6__Notify->NotificationMessage->SubscriptionReference->__anyAttribute = NULL;
	ns6__Notify->NotificationMessage->Topic = NULL;
	ns6__Notify->NotificationMessage->ProducerReference = NULL;

	return SOAP_OK;
}

/**
 * @brief Subscribe Events
 * @ Subscribe request is sent to Subscribe for Event Notification
 * @param Request : _ns6__Subscribe structure containing intial termination time
 * @param Response : _ns6__SubscribeResponse with appropriate endpoint reference
 * @return : On success - SOAP_OK.
 */
int __ns11__Subscribe(struct soap* soap, struct _ns6__Subscribe *ns6__Subscribe, struct _ns6__SubscribeResponse *ns6__SubscribeResponse)
{
	char macaddr[MACH_ADDR_LENGTH];
	char _HwId[LARGE_INFO_LENGTH];
	char initial_time[SMALL_INFO_LENGTH] = "PT10S";
	time_t time_tm;
	time_t t_time;

	#ifdef NOTIFY_ENABLED
	int temp_sock;
	#endif

	time_tm = time(NULL);
	net_get_hwaddr(ETH_NAME, macaddr);
	#ifdef NOTIFY_ENABLED
	temp_sock = dup(midhnewsock);

	/* Check if previous Subscription is terminated */
	if(canSubscribe)
	{
		/* Notification thread Initialization */
		if(pthread_create(&notify_thread, NULL, notify_thr, (void *)&temp_sock) != 0)
		{
			ONVIF_DBG("Error: Notify thread creation failed");
		}
	}
	pthread_join(notify_thread, NULL);
	#endif

	if(ns6__Subscribe->Filter != NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidFilterFault");
		return SOAP_FAULT;
	}

	if(ns6__Subscribe->InitialTerminationTime != NULL)
	{
		t_time = periodtol(ns6__Subscribe->InitialTerminationTime);
	}
	else
	{
		t_time = periodtol(initial_time);
	}
	sprintf(_HwId,"urn:uuid:1419d68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	ns6__SubscribeResponse->SubscriptionReference.Address = (char *) soap_malloc(soap, sizeof(char) * INFO_LENGTH);
	strcpy(ns6__SubscribeResponse->SubscriptionReference.Address, _HwId);
	ns6__SubscribeResponse->SubscriptionReference.ReferenceParameters = NULL;
	ns6__SubscribeResponse->SubscriptionReference.Metadata = NULL;
	ns6__SubscribeResponse->SubscriptionReference.__size = 0;
	ns6__SubscribeResponse->SubscriptionReference.__any = NULL;
	ns6__SubscribeResponse->SubscriptionReference.__anyAttribute = NULL;
	ns6__SubscribeResponse->CurrentTime = (time_t *) soap_malloc(soap, sizeof(time_t));
	ns6__SubscribeResponse->CurrentTime[0] = time_tm;
	t_time += time_tm;
	ns6__SubscribeResponse->TerminationTime = (time_t *) soap_malloc(soap, sizeof(time_t));
	ns6__SubscribeResponse->TerminationTime[0] = t_time;
	ns6__SubscribeResponse->__size = 0;
	ns6__SubscribeResponse->__any = NULL;

	return SOAP_OK;
}

/**
 * @brief Unsubscribe Events
 * @ Unsubscribe request is sent to terminate any Subscription done for Event Notification
 *
 * @return : On success - SOAP_OK.
 */
int __ns10__Unsubscribe(struct soap* soap, struct _ns6__Unsubscribe *ns6__Unsubscribe, struct _ns6__UnsubscribeResponse *ns6__UnsubscribeResponse)
{
	/* Set termination time to zero, in order to make the subscription void */
	time_out = TERMINATE_THREAD;
	t_out = TERMINATE_THREAD;
	ns6__UnsubscribeResponse->__size = 0;
	ns6__UnsubscribeResponse->__any = NULL;
	return SOAP_OK;
}


