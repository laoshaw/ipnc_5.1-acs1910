/***********************************************************
@file     cmd_server.c
@brief    实现与远程socket的通信
          接收病解析来自socket的指令
@version  V1.0
@date     2017-08-01
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
#include <sys/msg.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <net/if.h>
#include <sys/time.h>
#include "version.h"

#include "drv_ACS1910.h"
#include "cmd_server.h"
#include "file_msg_drv.h"
#include "sys_msg_drv.h"
#include "sem_util.h"    

int SYSTEM(char *arg);

unsigned char client_id[CMD_PACK_HEADER_SIZE] = {CLIENT_ID_DATA}; //'VFU'0x01
unsigned char client_heart_beat[HEART_BEAT_SIZE] = {CLIENT_ID_DATA, 0x00, 0x00, 0x00, 0x44};

unsigned char server_id[CMD_PACK_HEADER_SIZE] = {SERVER_ID_DATA}; //'VFD'0x01
unsigned char server_ok[SERVER_OK_SIZE] = {SERVER_ID_DATA, 0x00, 0x00, 0x00,0x00};
unsigned char server_heart_beat[HEART_BEAT_SIZE] = {SERVER_ID_DATA, 0x00, 0x00, 0x00, 0x55};
unsigned char server_error[SERVER_ERROR_SIZE] = {SERVER_ERROR_DATA, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};

static int g_cmd_server_run = 1;
static int len_cmd_msqid;
static int len_ack_msqid;
static int vim_cmd_msqid;
static int vim_ack_msqid;
//static int sys_cmd_msqid;
//static int sys_ack_msqid; 
static SysInfo acs1910_ipnc_sysinfo; 
char version[32];

static int cmd_server_setip(pVF_CAMERA_NETINFO_S camera_ip, int mac)
{
    int ret = 0;
    FILE *fp;
    int fd;
    unsigned long magic_num = MAGIC_NUM;
    int i;
    char syscmd[64];
    
    memcpy(&(acs1910_ipnc_sysinfo.lan_config.net.ip), &(camera_ip->ipaddr), sizeof(struct in_addr)); 
    memcpy(&(acs1910_ipnc_sysinfo.lan_config.net.netmask), &(camera_ip->netmask), sizeof(struct in_addr)); 
    memcpy(&(acs1910_ipnc_sysinfo.lan_config.net.gateway), &(camera_ip->gateway), sizeof(struct in_addr)); 
    if(mac == 1)
    {
        memcpy(acs1910_ipnc_sysinfo.lan_config.net.MAC, camera_ip->MAC, 6);
        VI_DEBUG("acs1910_ipnc_sysinfo.lan_config.net.Mac:");
        for(i = 0; i < 6; i++)
            printf("%02X ", acs1910_ipnc_sysinfo.lan_config.net.MAC[i]);
        printf("\n ");

        VI_DEBUG("camera_ip.Mac:");
        for(i = 0; i < 6; i++)
            printf("%02X ", camera_ip->MAC[i]);
        printf("\n ");
    }

    //sprintf(syscmd, "ifconfig eth0 hw ether %02X:%02X:%02X:%02X:%02X:%02X", acs1910_ipnc_sysinfo.lan_config.net.MAC[0] , acs1910_ipnc_sysinfo.lan_config.net.MAC[1], acs1910_ipnc_sysinfo.lan_config.net.MAC[2], acs1910_ipnc_sysinfo.lan_config.net.MAC[3], acs1910_ipnc_sysinfo.lan_config.net.MAC[4], acs1910_ipnc_sysinfo.lan_config.net.MAC[5]); 
   // VI_DEBUG("syscmd = %s\n", syscmd);

    fp = fopen("/mnt/nand/sysenv.cfg", "wb");
    if(fp == NULL)
    {
        VI_DEBUG("open sysenv.cfg error\n");
        perror("open sysenv.cfg error\n");
        ret = -1;
    }
    else if(fwrite(&magic_num, 1, sizeof(magic_num), fp) != sizeof(magic_num))
    {
        VI_DEBUG("write magic_num error\n");
        perror("write magic_num error\n");
        ret = -1;
    }
    else if(fwrite(&acs1910_ipnc_sysinfo, 1, sizeof(acs1910_ipnc_sysinfo), fp) != sizeof(acs1910_ipnc_sysinfo))
    {
        VI_DEBUG("write sysinfo error\n");
        perror("write sysinfo error\n");
        ret = -1;
    }
    fd = fileno(fp);
    fsync(fd);
    fclose(fp);
    //sleep(1);
    SYSTEM("reboot\n");
    return ret;

}

int cmd_server_getmac(unsigned char *mac)
{
	struct ifreq ifr;
	int skfd;
    char *ifname = "eth0";

	if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket error");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("net_get_hwaddr: ioctl SIOCGIFHWADDR");
		close(skfd);
		return -1;
	}
	close(skfd);

	memcpy(mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
}	

void cmd_server_stop(void)
{
    VI_DEBUG("cmd_server_stop now!\n");
    g_cmd_server_run = 0;
}

static int cmd_server_msg_init(key_t key)
{
    int msqid;

    VI_DEBUG("key = 0x%x\n", key);
    msqid = msgget((key_t)key, IPC_CREAT|666);
    if(msqid == 0)
    {
        VI_DEBUG("RMID the id=0 queue!!\n");
        msgctl(msqid, IPC_RMID, 0);
        msqid = msgget((key_t)key, IPC_CREAT|0666);
    }

    if(msqid < 0)
    {
        perror("Get msqid error!\n");
    }

    return msqid;
}

static int cmd_server_init(void)
{
    int ret = 0;
    unsigned long magic_num;
    FILE *fp;
    char *ip_a;
         
    signal(SIGINT, cmd_server_stop);

    VI_DEBUG("Intialize msg in cmd_server!\n");
     
    VI_DEBUG("sizeof SysInfo = %d\n", sizeof(SysInfo)); 

    fp = fopen("/mnt/nand/sysenv.cfg", "rb");
    if(fp == NULL)
        perror("open SYS_ENV_FILE error\n");
    ret = fread(&magic_num, 1, sizeof(magic_num), fp);
    VI_DEBUG("ret = %d magic_num = %x\n", ret , magic_num);
    ret = fread(&acs1910_ipnc_sysinfo, 1, sizeof(acs1910_ipnc_sysinfo), fp);
    //ip_a = inet_ntoa(acs1910_ipnc_sysinfo.lan_config.net.ip);
    //VI_DEBUG("ip_a = %s\n", ip_a);
    //ip_a = inet_ntoa(acs1910_ipnc_sysinfo.lan_config.net.netmask);
    //VI_DEBUG("ip_a = %s\n", ip_a);
    fclose(fp);

    ret = 0;
    if((len_cmd_msqid = cmd_server_msg_init((key_t)LEN_CMD_MSG_KEY)) < 0)
        return -1;
    VI_DEBUG("Get len_cmd_msqid %d done!\n", len_cmd_msqid);

    if((len_ack_msqid = cmd_server_msg_init((key_t)LEN_ACK_MSG_KEY)) < 0)
        return -1;
    VI_DEBUG("Get len_ack_msqid %d done!\n", len_ack_msqid);

    if((vim_cmd_msqid = cmd_server_msg_init((key_t)VIM_CMD_MSG_KEY)) < 0)
        return -1;
    VI_DEBUG("Get vim_cmd_msqid %d done!\n", vim_cmd_msqid);

    if((vim_ack_msqid = cmd_server_msg_init((key_t)VIM_ACK_MSG_KEY)) < 0)
        return -1;
    VI_DEBUG("Get vim_ack_msqid %d done!\n", vim_ack_msqid);

//    if((sys_cmd_msqid = cmd_server_msg_init((key_t)SYS_CMD_MSG_KEY)) < 0)
//        return -1;
//    VI_DEBUG("Get sys_cmd_msqid %d done!\n", sys_cmd_msqid);
//
//    if((sys_ack_msqid = cmd_server_msg_init((key_t)SYS_ACK_MSG_KEY)) < 0)
//        return -1;
//    VI_DEBUG("Get sys_ack_msqid %d done!\n", sys_ack_msqid);
     
    VI_DEBUG("Intialize msg in cmd_server done!\n\n");

    return ret;
}


/***********************************************************
@brief 清除与系统其它进程通信的工作环境
***********************************************************/
static void cmd_server_exit(void)
{
    
//    CleanupFileMsgDrv();
//    SysDrvExit();
//    
//    sem_destroy(&emif_sem);
//    msgctl(ch_id, IPC_RMID, 0);
    //msgctl(len_cmd_msqid, IPC_RMID, 0);
    //

    VI_DEBUG("cmd_server_exit success\n");
}

static unsigned char calc_check_code(unsigned char *buf, int size)
{
    unsigned char check_code = 0;
    int i;

    //VI_DEBUG("xxxxx check_code = %x\n", check_code);
    for(i = 0; i < size; i++)
    {
    //    VI_DEBUG("buf[%d] = %x\n", i, *buf);
        check_code ^= *(buf++);
    //    VI_DEBUG("check_code = %x\n", check_code);
    }
    //VI_DEBUG("xxxxx check_code = %x\n", check_code);
    return check_code;
}

static int parse_cmd(unsigned char *recv, unsigned char *send)
{
    int ret = 0;
    tCmdServerMsg cmd_server_snd_msg;
    tCmdServerMsg cmd_server_rcv_msg;
    VF_CAMERA_NETINFO_S camera_ip_s;
    VF_TIME_S camera_time_s;
    VF_SYS_VER_S sys_ver;
    int data_len = 0;
    int send_len = 0;
    int i;
    char *ip_a;

    cmd_server_snd_msg.msg_type = recv[CMD_PACK_MSG_OFFSET];
    VI_DEBUG("cmd msg type = %02x\n", cmd_server_snd_msg.msg_type);

    data_len = (recv[CMD_PACK_DATA_LENGTH_OFFSET] << 8) | recv[CMD_PACK_DATA_LENGTH_OFFSET+1];
    //VI_DEBUG("data_len = %d\n", data_len);
    
    memcpy(cmd_server_snd_msg.msg_data, &recv[CMD_PACK_DATA_OFFSET], data_len);
    //for(i = 0; i < data_len; i++)
    //{
    //    VI_DEBUG("cmd_server_snd_msg.msg_data[%02d]: %02x\n", i, cmd_server_snd_msg.msg_data[i]);
    //}
    switch(recv[CMD_PACK_MSG_OFFSET])
    {
        case IP_CMD_ISP_SET_EXPOSURE: 
            if(data_len != sizeof(VF_AE_MODE_S))
            {
                VI_DEBUG("SET_EXPOSURE cmd msg error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            break;
        case IP_CMD_ISP_SET_AE_DELAY:
            if(data_len != sizeof(int))
            {
                VI_DEBUG("SET_AE_DELAY cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            break;
        case IP_CMD_ISP_GET_ETGAIN:
            if(data_len != 0)
            {
                VI_DEBUG("GET_ETGAIN cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR ;
            }
            else 
            {

                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_ack_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                VI_DEBUG("recv from vim control thread\n");
                if(cmd_server_rcv_msg.msg_type == IP_CMD_ISP_GET_ERROR)
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(unsigned int) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(unsigned int);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_ISP_GET_ETGAIN; 
                    send[CMD_PACK_MSG_OFFSET - 1] = 0xFF;
                    send[CMD_PACK_DATA_OFFSET] = IP_CMD_DATA_ERROR;
                    send[CMD_PACK_DATA_OFFSET + 1] = IP_CMD_DATA_ERROR >> 8;
                    send[CMD_PACK_DATA_OFFSET + 2] = IP_CMD_DATA_ERROR >> 16;
                    send[CMD_PACK_DATA_OFFSET + 3] = IP_CMD_DATA_ERROR >> 24;
                } 
                else 
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(VF_AE_ETGain_S) >> 8);
                    //VI_DEBUG("sizeof(VF_AE_ETGain_S) = %02x\n", sizeof(VF_AE_ETGain_S));
                    //VI_DEBUG("send[%02d] = %02x\n", CMD_PACK_DATA_LENGTH_OFFSET, send[CMD_PACK_DATA_LENGTH_OFFSET]); 
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(VF_AE_ETGain_S);
                    //VI_DEBUG("send[%02d] = %02x\n", CMD_PACK_DATA_LENGTH_OFFSET + 1, send[CMD_PACK_DATA_LENGTH_OFFSET + 1]);
                    send[CMD_PACK_MSG_OFFSET] = cmd_server_rcv_msg.msg_type;
                    VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                    VI_DEBUG("copy to send buf\n");
                    memcpy(&send[CMD_PACK_DATA_OFFSET], &cmd_server_rcv_msg.msg_data, sizeof(VF_AE_ETGain_S));
                    VI_DEBUG("cmd have done\n");
                }
                ret = 1;
            }
            break;
        case IP_CMD_ISP_SET_AE_ROI:
            if(data_len != sizeof(VF_AE_ROI_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }            
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_AWB:
            if(data_len != sizeof(VF_AWB_MODE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_BASE_ATTR:
            if(data_len != sizeof(VF_BASE_ATTRIBUTE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_FLIP_MIRROR:
            if(data_len != sizeof(VF_FLIP_MIRROR_MODE_E))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_IRIS:
            if(data_len != sizeof(VF_IRIS_MODE_E))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_IRCUT:
            if(data_len != sizeof(VF_IRCUT_MODE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_COLOR_BLACK:
            if(data_len != sizeof(VF_COLORBLACK_MODE_E))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_DR_MODE:
            if(data_len != sizeof(VF_DR_MODE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_DENOISE:
            if(data_len != sizeof(VF_DENOISE_MODE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_EIS:
            if(data_len != sizeof(VF_EIS_FLAG_E))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
//                for(i = 0; i < data_len; i++)
//                {
//                    VI_DEBUG("cmd_server_snd_msg.msg_data[%02d]: %02x\n", i, cmd_server_snd_msg.msg_data[i]);
//                }
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_DEFOG:
            if(data_len != sizeof(VF_DEFOG_MODE_E))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                for(i = 0; i < data_len; i++)
                {
                    VI_DEBUG("cmd_server_snd_msg.msg_data[%02d]: %02x\n", i, cmd_server_snd_msg.msg_data[i]);
                }
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_SAVE_ATTR:
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_LOAD_DEFAULT:
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_LOAD_SAVED:
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;        
        case IP_CMD_ISP_GET_CURRENT_ISP_ATTR:
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_ack_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                VI_DEBUG("recv from vim control thread\n");
                if(cmd_server_rcv_msg.msg_type == IP_CMD_ISP_GET_ERROR)
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(unsigned int) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(unsigned int);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_ISP_GET_CURRENT_ISP_ATTR;
                    send[CMD_PACK_MSG_OFFSET - 1] = 0xFF;
                    send[CMD_PACK_DATA_OFFSET] = IP_CMD_DATA_ERROR;
                    send[CMD_PACK_DATA_OFFSET + 1] = IP_CMD_DATA_ERROR >> 8;
                    send[CMD_PACK_DATA_OFFSET + 2] = IP_CMD_DATA_ERROR >> 16;
                    send[CMD_PACK_DATA_OFFSET + 3] = IP_CMD_DATA_ERROR >> 24;
                }
                else
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(tACS1910ISPNormalCfg) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(tACS1910ISPNormalCfg);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_ISP_GET_CURRENT_ISP_ATTR;
                    VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                    memcpy(&send[CMD_PACK_DATA_OFFSET], &cmd_server_rcv_msg.msg_data, sizeof(tACS1910ISPNormalCfg));
                    VI_DEBUG("cmd have done\n");
                }
                ret = 1;
            }
            break;
        case IP_CMD_ISP_GET_AE_ROI:
            if(data_len != sizeof(unsigned int))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_ack_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                VI_DEBUG("recv from vim control thread\n");
                if(cmd_server_rcv_msg.msg_type == IP_CMD_ISP_GET_ERROR)
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(unsigned int) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(unsigned int);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_ISP_GET_AE_ROI;
                    send[CMD_PACK_MSG_OFFSET - 1] = 0xFF;
                    send[CMD_PACK_DATA_OFFSET] = IP_CMD_DATA_ERROR;
                    send[CMD_PACK_DATA_OFFSET + 1] = IP_CMD_DATA_ERROR >> 8;
                    send[CMD_PACK_DATA_OFFSET + 2] = IP_CMD_DATA_ERROR >> 16;
                    send[CMD_PACK_DATA_OFFSET + 3] = IP_CMD_DATA_ERROR >> 24;
                }
                else
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(VF_AE_ROI_S) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(VF_AE_ROI_S);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_ISP_GET_AE_ROI;
                    VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                    memcpy(&send[CMD_PACK_DATA_OFFSET], &cmd_server_rcv_msg.msg_data, sizeof(VF_AE_ROI_S));
                    VI_DEBUG("cmd have done\n");
                }
                ret = 1;
            }
            break;
        case IP_CMD_SYS_SET_CAMERA_IP:
            if(data_len != sizeof(camera_ip_s))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            {
                memcpy(&camera_ip_s, &recv[CMD_PACK_DATA_OFFSET], data_len);
                VI_DEBUG("camera_ip_s.ipaddr = %x\n", camera_ip_s.ipaddr);
                VI_DEBUG("camera_ip_s.netmask = %x\n", camera_ip_s.netmask);
                VI_DEBUG("camera_ip_s.gateway = %x\n", camera_ip_s.gateway);
                ip_a = inet_ntoa(camera_ip_s.ipaddr);
                VI_DEBUG("ipaddr = %s\n", ip_a);
                ip_a = inet_ntoa(camera_ip_s.netmask);
                VI_DEBUG("ipaddr = %s\n", ip_a);
                ret = cmd_server_setip(&camera_ip_s, 0);
            }
            break;
        case IP_CMD_SYS_SET_NET:
            if(data_len != sizeof(camera_ip_s))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                memcpy(&camera_ip_s, &recv[CMD_PACK_DATA_OFFSET], data_len);
                VI_DEBUG("camera_ip_s.ipaddr = %x\n", camera_ip_s.ipaddr);
                VI_DEBUG("camera_ip_s.netmask = %x\n", camera_ip_s.netmask);
                VI_DEBUG("camera_ip_s.gateway = %x\n", camera_ip_s.gateway);
                ip_a = inet_ntoa(camera_ip_s.ipaddr);
                VI_DEBUG("ipaddr = %s\n", ip_a);
                ip_a = inet_ntoa(camera_ip_s.netmask);
                VI_DEBUG("ipaddr = %s\n", ip_a);
                ret = cmd_server_setip(&camera_ip_s, 0);
            }
            break;
        case IP_CMD_SYS_GET_NET:
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                memcpy(&(camera_ip_s.ipaddr), &(acs1910_ipnc_sysinfo.lan_config.net.ip), sizeof(struct in_addr)); 
                memcpy(&(camera_ip_s.netmask), &(acs1910_ipnc_sysinfo.lan_config.net.netmask), sizeof(struct in_addr)); 
                memcpy(&(camera_ip_s.gateway), &(acs1910_ipnc_sysinfo.lan_config.net.gateway), sizeof(struct in_addr)); 
                cmd_server_getmac(camera_ip_s.MAC);
                ip_a = inet_ntoa(camera_ip_s.ipaddr);
                VI_DEBUG("ipaddr = %s\n", ip_a);
                ip_a = inet_ntoa(camera_ip_s.gateway);
                VI_DEBUG("gateway = %s\n", ip_a);
                send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(camera_ip_s) >> 8);
                send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(camera_ip_s);
                send[CMD_PACK_MSG_OFFSET] = IP_CMD_SYS_GET_NET;
                VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                memcpy(&send[CMD_PACK_DATA_OFFSET], &camera_ip_s, sizeof(camera_ip_s));
                VI_DEBUG("cmd have done\n");
                ret = 1;
            }
            break;
        case IP_CMD_SYS_SET_TIME:
            VI_DEBUG("set time\n");
            if(data_len != sizeof(VF_TIME_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_SYS_SET_CAMERA_ID:
            VI_DEBUG("set camera id\n");
            if(data_len != sizeof(VF_CAMERA_ID_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_SYS_SET_OSD:
            VI_DEBUG("set osd\n");
            break;
        case IP_CMD_SYS_GET_VER:
            VI_DEBUG("get ver\n");
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_ack_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                VI_DEBUG("recv from vim control thread\n");
                if(cmd_server_rcv_msg.msg_type == IP_CMD_ISP_GET_ERROR)
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(unsigned int) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(unsigned int);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_SYS_GET_SYS_CFG;
                    send[CMD_PACK_MSG_OFFSET - 1] = 0xFF;
                    send[CMD_PACK_DATA_OFFSET] = IP_CMD_DATA_ERROR;
                    send[CMD_PACK_DATA_OFFSET + 1] = IP_CMD_DATA_ERROR >> 8;
                    send[CMD_PACK_DATA_OFFSET + 2] = IP_CMD_DATA_ERROR >> 16;
                    send[CMD_PACK_DATA_OFFSET + 3] = IP_CMD_DATA_ERROR >> 24;
                }
                else
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(VF_SYS_VER_S) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(VF_SYS_VER_S);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_SYS_GET_VER;
                    VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                    memcpy(&sys_ver, &cmd_server_rcv_msg.msg_data, sizeof(VF_SYS_VER_S));
                    strcpy(sys_ver.dsp_ver, version);
                    //sprintf(sys_ver.dsp_ver, "%s", VERSION_NUMBER);
                    VI_DEBUG("dsp_ver = %s\n", sys_ver.dsp_ver);
                    VI_DEBUG("fpga_ver = %s\n", sys_ver.fpga_ver);
                    VI_DEBUG("sensor_ver = %s\n", sys_ver.sensor_ver);
                    memcpy(&send[CMD_PACK_DATA_OFFSET], &sys_ver, sizeof(VF_SYS_VER_S));
                    VI_DEBUG("cmd have done\n");
                }
                ret = 1;  
            }
            break;
        case IP_CMD_SYS_GET_SYS_CFG:
            VI_DEBUG("get sys cfg\n");
            if(data_len != 0)
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_ack_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                VI_DEBUG("recv from vim control thread\n");
                if(cmd_server_rcv_msg.msg_type == IP_CMD_ISP_GET_ERROR)
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(unsigned int) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(unsigned int);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_SYS_GET_SYS_CFG;
                    send[CMD_PACK_MSG_OFFSET - 1] = 0xFF;
                    send[CMD_PACK_DATA_OFFSET] = IP_CMD_DATA_ERROR;
                    send[CMD_PACK_DATA_OFFSET + 1] = IP_CMD_DATA_ERROR >> 8;
                    send[CMD_PACK_DATA_OFFSET + 2] = IP_CMD_DATA_ERROR >> 16;
                    send[CMD_PACK_DATA_OFFSET + 3] = IP_CMD_DATA_ERROR >> 24;
                }
                else
                {
                    send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(tACS1910SYSCfg) >> 8);
                    send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(tACS1910SYSCfg);
                    send[CMD_PACK_MSG_OFFSET] = IP_CMD_SYS_GET_SYS_CFG;
                    VI_DEBUG("MSG : %02x\n", send[CMD_PACK_MSG_OFFSET]);
                    memcpy(&send[CMD_PACK_DATA_OFFSET], &cmd_server_rcv_msg.msg_data, sizeof(tACS1910SYSCfg));
                    VI_DEBUG("cmd have done\n");
                }
                ret = 1;
            }
            break;
        case IP_CMD_LEN_CONTROL:
            if(data_len != sizeof(VF_LEN_CONTROL_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else
            {
                if((((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_ZOOM_TELE) && 
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_ZOOM_WIDE) && 
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_FOCUS_FAR) &&
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_FOCUS_NEAR)&&
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_IRIS_SMALL)&&
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_IRIS_LARGE)&&
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->Mode != VF_CONTROL_LEN_STOP))
                {
                    VI_DEBUG("LEN CONTROL MODE is error\n");
                    ret = IP_CMD_DATA_ERROR;
                }
                if((((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->speed > 10) || 
                        (((pVF_LEN_CONTROL_S)cmd_server_snd_msg.msg_data)->speed == 0))
                {
                    VI_DEBUG("LEN CONTROL Speed is error\n");
                    ret = IP_CMD_DATA_ERROR;
                }
                if(ret == 0) 
                {
                    VI_DEBUG("LEN CONTROL CMD is right\n");
                    msgsnd(len_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                }
            }
            break;
        case IP_CMD_LEN_SET_SAVE_SPEED:
            if(data_len != 0) 
            {
                VI_DEBUG("cmd error!\n");
                ret = IP_CMD_DATA_LENGTH_ERROR;
            }
            else
            {
                msgsnd(len_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        default:
            ret = IP_CMD_MSG_ERROR;
            break;
    }

    return ret;
}

int main(int argc, char **argv)
{
    int ret = 0, i;

    int cmd_socketfd = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    unsigned char recv_buf[BUF_SIZE] = {0};
    unsigned char send_buf[BUF_SIZE] = {0};
    char *client_ip = NULL;
    unsigned int cmd_data_len = 0;
    unsigned int cmd_len = 0;
    unsigned char check_code = 0;
    int recv_count = 0;
    int ack_len = 0;
    int send_len = 0;
    int addr_len = sizeof(struct sockaddr_in);
    fd_set recv_fd;
    struct timeval timeout;
    struct timeval tv1, tv2, tv3;

    sprintf(version, "%s", VERSION_NUMBER);

    ret = cmd_server_init();
    if(ret < 0)
    {
        printf("Init cmd server error\n");
        return ret;
    }
    
    memset(recv_buf, 0, sizeof(recv_buf));
    memset(send_buf, 0, sizeof(send_buf));
    memcpy(send_buf, server_id, sizeof(server_id));
//create udp socket
    cmd_socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(cmd_socketfd < 0)
    {
        printf("Create cmd socket error\n");
        return cmd_socketfd;
    }
    VI_DEBUG("cmd_socketfd = %d\n", cmd_socketfd);
    
//initial address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(CMD_PORT);

    ret = bind(cmd_socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        printf("bind socket error\n");
        return ret;
    }

    while(g_cmd_server_run)
    {
        //VI_DEBUG("wait from client\n");

        FD_ZERO(&recv_fd);
        FD_SET(cmd_socketfd, &recv_fd);
        timeout.tv_sec = RECV_TIMEOUT;
        timeout.tv_usec = 0;

        ret = select(cmd_socketfd+1, &recv_fd, 0, 0, &timeout);
        if(ret < 0)
        {
            printf("cmd server recv select error\n");
        }
        else if(ret ==0)
        {
            //VI_DEBUG("time out\n");
        }
        else
        {
            if(FD_ISSET(cmd_socketfd, &recv_fd))
            {
                recv_count = recvfrom(cmd_socketfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&client_addr,&addr_len); 
                if(recv_count < 0)
                {
                    perror("recvfrom\n");
                }
                else
                {//接收到数据
                    gettimeofday(&tv1, NULL);
                    client_ip= inet_ntoa(client_addr.sin_addr.s_addr);
                    //VI_DEBUG("recv %d data %s\n",recv_count, client_ip);
                    if(memcmp(recv_buf, client_id, 4) != 0)
                    {
                        printf("not my data\n");
                    }
                    else
                    {//包头正确
                        cmd_data_len = (recv_buf[CMD_PACK_DATA_LENGTH_OFFSET] << 8) | recv_buf[CMD_PACK_DATA_LENGTH_OFFSET+1]; 
                        //VI_DEBUG("cmd data length is %d\n", cmd_data_len);
                        cmd_len = cmd_data_len + CMD_PACK_HEADER_SIZE + CMD_PACK_MSG_SIZE + CMD_PACK_DATA_LENGTH_SIZE + 1; 
                        //VI_DEBUG("cmd length is %d\n", cmd_len);
                        //for(i = 0; i < cmd_len; i++)
                        //{
                        //    VI_DEBUG("recv_buf[%02d] = %02x \n", i, recv_buf[i]);
                        //}
                        check_code = calc_check_code(recv_buf, (cmd_len-1));
                        //VI_DEBUG("check_code = %02x\n", check_code);
                        if(check_code == recv_buf[cmd_len - 1])
                        {//校验正确
                            //VI_DEBUG("recv data check ok\n");
                            if(memcmp(recv_buf, client_heart_beat, HEART_BEAT_SIZE) == 0)
                            {//心跳数据
                                VI_DEBUG("it is heart beat\n");
                                sendto(cmd_socketfd, server_heart_beat, sizeof(server_heart_beat), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            }//心跳数据
                            else
                            {//非心跳数据
                                //parse_cmd begain with cmd no
                                ret = parse_cmd(recv_buf, send_buf);
                                if(1 == ret)
                                {//需要回复数据的
                                    //VI_DEBUG("many bytes need send\n");
                                    ack_len = CMD_PACK_DATA_OFFSET + (send_buf[CMD_PACK_DATA_LENGTH_OFFSET] << 8) + 
                                              (send_buf[CMD_PACK_DATA_LENGTH_OFFSET + 1]) + 1;
                                    //VI_DEBUG("ack_len = %d\n", ack_len);
                                    send_buf[ack_len - 1] = calc_check_code(send_buf, ack_len - 1 );
                                    //for(i = 0; i < ack_len; i++)
                                    //   VI_DEBUG("send_buf[%02d] = %02X\n", i, send_buf[i]);
                                    gettimeofday(&tv2, NULL);
                                    send_len = sendto(cmd_socketfd, send_buf, ack_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)); 
                                    if(send_len != ack_len)
                                    {
                                       VI_DEBUG("send_len = %d\n", send_len); 
                                    }
                                    gettimeofday(&tv3, NULL);
                                    printf("tv1 = %ld\n", tv1.tv_sec*1000 + tv1.tv_usec/1000);
                                    printf("tv2 = %ld\n", tv2.tv_sec*1000 + tv2.tv_usec/1000);
                                    printf("tv3 = %ld\n", tv3.tv_sec*1000 + tv3.tv_usec/1000);
                                    printf("tv2 - tv1 = %d\n", ((tv2.tv_sec*1000 + tv2.tv_usec/1000) - (tv1.tv_sec*1000 + tv1.tv_usec/1000)));
                                    printf("tv3 - tv2 = %d\n", ((tv3.tv_sec*1000 + tv3.tv_usec/1000) - (tv2.tv_sec*1000 + tv2.tv_usec/1000)));
                               }
                                else if(0 == ret)
                                {//不需要回复数据的，回复执行ok的
                                    server_ok[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                                    server_ok[sizeof(server_ok) - 1] = calc_check_code(server_ok, sizeof(server_ok) - 1);
                                    sendto(cmd_socketfd, server_ok, sizeof(server_ok), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                    //for(i = 0; i < sizeof(server_ok); i++)
                                    //    VI_DEBUG("server_ok[%02d] = %02X\n", i, server_ok[i]);
                                }
                                else 
                                {//各种错误的
                                    server_error[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                                    memcpy(&(server_error[SERVER_ERROR_CODE_OFFSET]), &ret, sizeof(ret));
                                    server_error[SERVER_ERROR_SIZE - 1] = calc_check_code(server_error, sizeof(server_error) - 1);
                                    sendto(cmd_socketfd, server_error, sizeof(server_error), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                    //for(i = 0; i < sizeof(server_error); i++)
                                    //    VI_DEBUG("server_error[%02d] = %02X\n", i, server_error[i]);
                                }
                            }//非心跳数据
                        }//校验正确
                        else 
                        {//校验错
                            server_error[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                            VI_DEBUG("check_code is error\n");
                            ret = IP_CMD_CRC_ERROR;
                            memcpy(&(server_error[SERVER_ERROR_CODE_OFFSET]), &ret, sizeof(ret));
                            server_error[SERVER_ERROR_SIZE - 1] = calc_check_code(server_error, sizeof(server_error) - 1);
                            sendto(cmd_socketfd, server_error, sizeof(server_error), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        }//校验错
                        memcpy(send_buf, server_id, sizeof(server_id));//接收发送完成之后，重新初始化一下发送buffer
                    }//包头正确
                }//接收到数据
            }//FD_ISSET(cmd_socketfd, &recv_fd)
        }//select返回非0
    }//while

    close(cmd_socketfd); 
    cmd_server_exit();
    return ret;
}

