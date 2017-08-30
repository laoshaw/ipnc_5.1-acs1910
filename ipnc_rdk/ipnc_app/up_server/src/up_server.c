/***********************************************************
@file     up_server.c
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

#include "sem_util.h"    

static g_up_server_run = 1;



void up_server_stop(void)
{
    //VI_DEBUG("up_server_stop now!\n");
    g_up_server_run = 0;
}

static int up_server_init(void)
{
    int ret = 0;
    char *ip_a;
         
    signal(SIGINT, up_server_stop);

    //VI_DEBUG("Intialize msg in up_server!\n");
     
    ret = 0;
    return ret;
}


/***********************************************************
@brief 清除与系统其它进程通信的工作环境
***********************************************************/
static void up_server_exit(void)
{
    
//    CleanupFileMsgDrv();
//    SysDrvExit();
//    
//    sem_destroy(&emif_sem);
//    msgctl(ch_id, IPC_RMID, 0);
    //msgctl(len_cmd_msqid, IPC_RMID, 0);
    //

    //VI_DEBUG("up_server_exit success\n");
}

#define UP_PORT 4444
#define UP_TIMEOUT 10
#define BUF_SIZE 1024

int main(int argc, char **argv)
{
    int ret = 0, i;

    int up_socketfd = 0;
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

    ret = up_server_init();
    if(ret < 0)
    {
        printf("Init cmd server error\n");
        return ret;
    }
    
    memset(recv_buf, 0, sizeof(recv_buf));
    memset(send_buf, 0, sizeof(send_buf));
//    memcpy(send_buf, server_id, sizeof(server_id));

//create udp socket
    up_socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(up_socketfd < 0)
    {
        printf("Create cmd socket error\n");
        return up_socketfd;
    }
    //VI_DEBUG("up_socketfd = %d\n", up_socketfd);
    
//initial address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UP_PORT);

    ret = bind(up_socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        printf("bind socket error\n");
        return ret;
    }

    while(g_up_server_run)
    {
        //VI_DEBUG("wait from client\n");

        FD_ZERO(&recv_fd);
        FD_SET(up_socketfd, &recv_fd);
        timeout.tv_sec = UP_TIMEOUT;
        timeout.tv_usec = 0;

        ret = select(up_socketfd+1, &recv_fd, 0, 0, &timeout);
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
            if(FD_ISSET(up_socketfd, &recv_fd))
            {
                recv_count = recvfrom(up_socketfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&client_addr,&addr_len); 
                if(recv_count < 0)
                {
                    perror("recvfrom\n");
                }
                else
                {//接收到数据
                    gettimeofday(&tv1, NULL);
                    client_ip= inet_ntoa(client_addr.sin_addr.s_addr);
                    //VI_DEBUG("recv %d data %s\n",recv_count, client_ip);
                    //if(memcmp(recv_buf, client_id, 4) != 0)
                    {
                        printf("not my data\n");
                    }
                    //else
                    {//包头正确
                        //cmd_data_len = (recv_buf[CMD_PACK_DATA_LENGTH_OFFSET] << 8) | recv_buf[CMD_PACK_DATA_LENGTH_OFFSET+1]; 
                        //VI_DEBUG("cmd data length is %d\n", cmd_data_len);
                        //cmd_len = cmd_data_len + CMD_PACK_HEADER_SIZE + CMD_PACK_MSG_SIZE + CMD_PACK_DATA_LENGTH_SIZE + 1; 
                        //VI_DEBUG("cmd length is %d\n", cmd_len);
                        //for(i = 0; i < cmd_len; i++)
                        //{
                        //    VI_DEBUG("recv_buf[%02d] = %02x \n", i, recv_buf[i]);
                        //}
                        //check_code = calc_check_code(recv_buf, (cmd_len-1));
                        //VI_DEBUG("check_code = %02x\n", check_code);
                        if(check_code == recv_buf[cmd_len - 1])
                        {//校验正确
                            //VI_DEBUG("recv data check ok\n");
                         //   if(memcmp(recv_buf, client_heart_beat, HEART_BEAT_SIZE) == 0)
                            {//心跳数据
                           //     VI_DEBUG("it is heart beat\n");
                                //sendto(up_socketfd, server_heart_beat, sizeof(server_heart_beat), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                            }//心跳数据
                            //else
                            {//非心跳数据
                                //parse_cmd begain with cmd no
                              //  ret = parse_cmd(recv_buf, send_buf);
                                if(1 == ret)
                                {//需要回复数据的
                                    //VI_DEBUG("many bytes need send\n");
                          //          ack_len = CMD_PACK_DATA_OFFSET + (send_buf[CMD_PACK_DATA_LENGTH_OFFSET] << 8) + 
                          //                    (send_buf[CMD_PACK_DATA_LENGTH_OFFSET + 1]) + 1;
                                    //VI_DEBUG("ack_len = %d\n", ack_len);
                               //     send_buf[ack_len - 1] = calc_check_code(send_buf, ack_len - 1 );
                                    //for(i = 0; i < ack_len; i++)
                                    //   VI_DEBUG("send_buf[%02d] = %02X\n", i, send_buf[i]);
                                    gettimeofday(&tv2, NULL);
                                    send_len = sendto(up_socketfd, send_buf, ack_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)); 
                                    if(send_len != ack_len)
                                    {
                          //             VI_DEBUG("send_len = %d\n", send_len); 
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
                           //         server_ok[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                                    //server_ok[sizeof(server_ok) - 1] = calc_check_code(server_ok, sizeof(server_ok) - 1);
                                //    sendto(up_socketfd, server_ok, sizeof(server_ok), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                    //for(i = 0; i < sizeof(server_ok); i++)
                                    //    VI_DEBUG("server_ok[%02d] = %02X\n", i, server_ok[i]);
                                }
                                else 
                                {//各种错误的
                            //        server_error[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                              //      memcpy(&(server_error[SERVER_ERROR_CODE_OFFSET]), &ret, sizeof(ret));
                             //       server_error[SERVER_ERROR_SIZE - 1] = calc_check_code(server_error, sizeof(server_error) - 1);
                                 //   sendto(up_socketfd, server_error, sizeof(server_error), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                                    //for(i = 0; i < sizeof(server_error); i++)
                                    //    VI_DEBUG("server_error[%02d] = %02X\n", i, server_error[i]);
                                }
                            }//非心跳数据
                        }//校验正确
                        else 
                        {//校验错
                        //    server_error[CMD_PACK_MSG_OFFSET] = recv_buf[CMD_PACK_MSG_OFFSET];
                         //   VI_DEBUG("check_code is error\n");
                         //   ret = IP_CMD_CRC_ERROR;
                         //   memcpy(&(server_error[SERVER_ERROR_CODE_OFFSET]), &ret, sizeof(ret));
                         //   server_error[SERVER_ERROR_SIZE - 1] = calc_check_code(server_error, sizeof(server_error) - 1);
                        //    sendto(up_socketfd, server_error, sizeof(server_error), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                        }//校验错
                        //memcpy(send_buf, server_id, sizeof(server_id));//接收发送完成之后，重新初始化一下发送buffer
                    }//包头正确
                }//接收到数据
            }//FD_ISSET(up_socketfd, &recv_fd)
        }//select返回非0
    }//while

    close(up_socketfd); 
    up_server_exit();
    return ret;
}

