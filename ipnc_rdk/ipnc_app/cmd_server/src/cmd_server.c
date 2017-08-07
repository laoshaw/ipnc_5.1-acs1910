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

#include "cmd_server.h"
#include "file_msg_drv.h"
#include "sys_msg_drv.h"
#include "sem_util.h"    

#define CMD_PORT 2500
#define BUF_SIZE 1024
#define RECV_TIMEOUT 10
#define NEED_ACK 1
#define DO_NOT_ACK 0
#define CMD_PACK_HEADER_SIZE (4)
#define CMD_PACK_MSG_SIZE (1)
#define CMD_PACK_MSG_OFFSET (CMD_PACK_HEADER_SIZE)
#define CMD_PACK_DATA_LENGTH_SIZE (2)
#define CMD_PACK_DATA_LENGTH_OFFSET (CMD_PACK_MSG_OFFSET + CMD_PACK_MSG_SIZE)
#define CMD_PACK_DATA_OFFSET (CMD_PACK_DATA_LENGTH_OFFSET + CMD_PACK_DATA_LENGTH_SIZE)

#define HEART_BEAT_SIZE (8)
#define SERVER_OK_SIZE (8)
#define CLIENT_ID_DATA 0x56, 0x46, 0x55, 0x01
#define SERVER_ID_DATA 0x56, 0x46, 0x44, 0x01

unsigned char client_id[CMD_PACK_HEADER_SIZE] = {CLIENT_ID_DATA}; //'VFU'0x01
unsigned char client_heart_beat[HEART_BEAT_SIZE] = {CLIENT_ID_DATA, 0x00, 0x00, 0x00, 0x44};

unsigned char server_id[CMD_PACK_HEADER_SIZE] = {SERVER_ID_DATA}; //'VFD'0x01
unsigned char server_ok[SERVER_OK_SIZE] = {SERVER_ID_DATA, 0x00, 0x00, 0x00,0x00};
unsigned char server_heart_beat[HEART_BEAT_SIZE] = {SERVER_ID_DATA, 0x00, 0x00, 0x00, 0x55};

static int g_cmd_server_run = 1;
static int len_cmd_msqid;
static int vim_cmd_msqid;
static int sys_cmd_msqid;

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
         
    signal(SIGINT, cmd_server_stop);

    VI_DEBUG("Intialize msg in cmd_server!\n");
     
    if((len_cmd_msqid = cmd_server_msg_init((key_t)LEN_CMD_MSG_KEY)) < 0)
        return -1;

    VI_DEBUG("Get len_cmd_msqid %d done!\n", len_cmd_msqid);

    if((vim_cmd_msqid = cmd_server_msg_init((key_t)VIM_CMD_MSG_KEY)) < 0)
        return -1;

    VI_DEBUG("Get vim_cmd_msqid %d done!\n", vim_cmd_msqid);

    if((sys_cmd_msqid = cmd_server_msg_init((key_t)SYS_CMD_MSG_KEY)) < 0)
        return -1;

    VI_DEBUG("Get sys_cmd_msqid %d done!\n", sys_cmd_msqid);

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
    VI_DEBUG("cmd_server_exit success\n");
}

static unsigned char calc_check_code(unsigned char *buf, int size)
{
    unsigned char check_code = 0;
    int i;

    for(i = 0; i < size; i++)
    {
        check_code ^= *(buf++);
    }

    return check_code;
}

static int parse_cmd(unsigned char *recv, unsigned char *send)
{
    int ret = 0;
    tCmdServerMsg cmd_server_snd_msg;
    tCmdServerMsg cmd_server_rcv_msg;
    int data_len = 0;
    int send_len = 0;
    int i;

    cmd_server_snd_msg.msg_type = recv[CMD_PACK_MSG_OFFSET];
    VI_DEBUG("cmd msg type = %x\n", cmd_server_snd_msg.msg_type);

    data_len = (recv[CMD_PACK_DATA_LENGTH_OFFSET] << 8) | recv[CMD_PACK_DATA_LENGTH_OFFSET+1];
    VI_DEBUG("data_len = %d\n", data_len);
    
    memcpy(cmd_server_snd_msg.msg_data, &recv[CMD_PACK_DATA_OFFSET], data_len);

    switch(recv[CMD_PACK_MSG_OFFSET])
    {
        case IP_CMD_ISP_SET_EXPOSURE: 
            if(data_len != sizeof(VF_AE_MODE_S))
            {
                VI_DEBUG("SET_EXPOSURE cmd msg error!\n");
                ret = -1;
            }
            else 
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            break;
        case IP_CMD_ISP_SET_AE_DELAY:
            if(data_len != sizeof(int))
            {
                VI_DEBUG("SET_AE_DELAY cmd error!\n");
                ret = -1;
            }
            else 
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            break;
        case IP_CMD_ISP_GET_ETGAIN:
            if(data_len != sizeof(VF_AE_ETGain_S))
            {
                VI_DEBUG("GET_ETGAIN cmd error!\n");
                ret = -1;
            }
            else 
            {
                send[CMD_PACK_DATA_LENGTH_OFFSET] = (sizeof(VF_AE_ETGain_S) << 8);
                send[CMD_PACK_DATA_LENGTH_OFFSET + 1] = sizeof(VF_AE_ETGain_S);
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
                msgrcv(vim_cmd_msqid, &cmd_server_rcv_msg, MSG_BUF_SIZE, 0, 0);
                send[CMD_PACK_MSG_OFFSET] = cmd_server_rcv_msg.msg_type;
                memcpy(&send[CMD_PACK_DATA_OFFSET], &cmd_server_rcv_msg.msg_data, sizeof(VF_AE_ETGain_S));
                send[(CMD_PACK_DATA_OFFSET + sizeof(VF_AE_ETGain_S))]
                    = calc_check_code(send, (CMD_PACK_DATA_OFFSET + sizeof(VF_AE_ETGain_S)));
                ret = CMD_PACK_DATA_OFFSET + sizeof(VF_AE_ETGain_S) + 1;
                VI_DEBUG("ret = %d\n", ret);
                for(i = 0; i < ret; i++)
                {
                    VI_DEBUG("send[%d] = %02x\n", i, send[i]);
                }
            }
            break;
        case IP_CMD_ISP_SET_AE_ROI:
            if(data_len != sizeof(VF_AE_ROI_S))
            {
                VI_DEBUG("cmd error!\n");
                ret = -1;
            }            
            break;
        case IP_CMD_ISP_SET_AWB:
            //if(data_len != sizeof())
            {
                VI_DEBUG("cmd error!\n");
                ret = -1;
            }
            break;
        case IP_CMD_ISP_SET_BASE_ATTR:
            if(data_len != sizeof(VF_BASE_ATTRIBUTE_S))
            {
                VI_DEBUG("cmd error!\n");
                ret= -1;
            }
            else 
            {
                msgsnd(vim_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            }
            break;
        case IP_CMD_ISP_SET_FLIP_MIRROR:
            break;
        case IP_CMD_ISP_SET_IRIS:
            break;
        case IP_CMD_ISP_SET_IRCUT:
            break;
        case IP_CMD_ISP_SET_COLOR_BLACK:
            break;
        case IP_CMD_ISP_SET_DR_MODE:
            break;
        case IP_CMD_ISP_SET_DENOISE:
            break;
        case IP_CMD_ISP_SET_EIS:
            break;
        case IP_CMD_ISP_SET_DEFOG:
            break;
        case IP_CMD_ISP_SET_SAVE_ATTR:
            break;
        case IP_CMD_ISP_SET_DEFAULT_ATTR:
            break;
        case IP_CMD_ISP_GET_CURRENT_ATTR:
            break;
        case IP_CMD_SYS_SET_CAMERA_IP:
            break;
        case IP_CMD_SYS_SET_TIME:
            break;
        case IP_CMD_LEN_CONTROL:
            VI_DEBUG("cmd is IP_CMD_LEN_CONTROL");
            msgsnd(len_cmd_msqid, &cmd_server_snd_msg, MSG_BUF_SIZE, 0);
            break;
        default:
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
    int addr_len = sizeof(struct sockaddr_in);
    fd_set recv_fd;
    struct timeval timeout;

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
                {
                    client_ip= inet_ntoa(client_addr.sin_addr.s_addr);
                    VI_DEBUG("recv %d data %s\n",recv_count, client_ip);
                    if(memcmp(recv_buf, client_id, 4) != 0)
                    {
                        printf("not my data\n");
                    }
                    else
                    {
                        cmd_data_len = (recv_buf[CMD_PACK_DATA_LENGTH_OFFSET] << 8) | recv_buf[CMD_PACK_DATA_LENGTH_OFFSET+1]; 
                        VI_DEBUG("cmd data length is %d\n", cmd_data_len);
                        cmd_len = cmd_data_len + CMD_PACK_HEADER_SIZE + CMD_PACK_MSG_SIZE + CMD_PACK_DATA_LENGTH_SIZE + 1; 
                        VI_DEBUG("cmd length is %d\n", cmd_len);
                        for(i = 0; i < cmd_len; i++)
                        {
                            VI_DEBUG("recv_buf[%d] = %x \n", i, recv_buf[i]);
                        }
                        check_code = calc_check_code(recv_buf, (cmd_len-1));
                        VI_DEBUG("check_code = %x\n", check_code);
                        if(check_code == recv_buf[cmd_len - 1])
                        {
                            VI_DEBUG("recv data check ok\n");
                            if(memcmp(recv_buf, client_heart_beat, HEART_BEAT_SIZE) == 0)
                            {
                                VI_DEBUG("it is heart beat\n");
                                sendto(cmd_socketfd, server_heart_beat, sizeof(server_heart_beat), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            }
                            else
                            {
                                server_ok[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET]; 
                                check_code = calc_check_code(server_ok, sizeof(server_ok));
                                VI_DEBUG("server_ok check_code = %x\n", check_code);
                                server_ok[(sizeof(server_ok) - 1)] = check_code; 
                                sendto(cmd_socketfd, server_ok, sizeof(server_ok), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                //parse_cmd begain with cmd no
                                ack_len = parse_cmd(recv_buf, send_buf);
                                if(ack_len > 0)
                                {
                                    sendto(cmd_socketfd, send_buf, ack_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(cmd_socketfd); 
    cmd_server_exit();
    return ret;
}

