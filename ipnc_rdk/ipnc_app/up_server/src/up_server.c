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

//#define UP_DEBUG_MODE

#ifdef UP_DEBUG_MODE
#define UP_DEBUG(...) \
    do \
    { \
        fprintf(stderr, " DEBUG (%s|%s|%d): ", \
                __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    while(0)

#else
#define UP_DEBUG(...)
#endif


#define UP_PORT 2501
#define UP_TIMEOUT 10
#define RCV_BUF_SIZE 1296 
#define SND_BUF_SIZE 12 

//更新数据命令头
#define UP_PACK_PRO_OFFSET 3
#define UP_PACK_PRO_SIZE 1
#define UP_PACK_MSG_OFFSET (UP_PACK_PRO_OFFSET + UP_PACK_PRO_SIZE) 
#define UP_PACK_MSG_SIZE 1
#define UP_PACK_DATA_LENGTH_OFFSET (UP_PACK_MSG_OFFSET + UP_PACK_MSG_SIZE) 
#define UP_PACK_DATA_LENGTH_SIZE 2
#define UP_PACK_DATA_OFFSET (UP_PACK_DATA_LENGTH_OFFSET + UP_PACK_DATA_LENGTH_SIZE) 
#define UP_PACK_SND_DATA_SIZE 4
//升级数据偏移 格式1 格式2的头 格式3、4、5、6
#define UP_PACK_FILE_BLK_NO_OFFSET (UP_PACK_DATA_LENGTH_OFFSET + UP_PACK_DATA_LENGTH_SIZE)
#define UP_PACK_FILE_BLK_NO_SIZE 3
#define UP_PACK_FILE_NO_OFFSET (UP_PACK_FILE_BLK_NO_OFFSET + UP_PACK_FILE_BLK_NO_SIZE)
#define UP_PACK_FILE_NO_SIZE 1
#define UP_PACK_BLK_LENGTH_OFFSET (UP_PACK_FILE_BLK_NO_OFFSET + UP_PACK_FILE_NO_SIZE)
#define UP_PACK_BLK_LENGTH_SIZE 2
#define UP_PACK_BLK_DATA_OFFSET (UP_PACK_BLK_LENGTH_OFFSET + UP_PACK_BLK_LENGTH_SIZE)
//升级数据格式1
#define UP_PACK_ALL_FILE_CNT_OFFSET (UP_PACK_BLK_LENGTH_OFFSET + UP_PACK_BLK_LENGTH_SIZE)
#define UP_PACK_ALL_FILE_CNT_SIZE 4
#define UP_PACK_ALL_FILE_LENGTH_OFFSET (UP_PACK_ALL_FILE_CNT_OFFSET + UP_PACK_ALL_FILE_CNT_SIZE)
//升级数据格式2
#define UP_PACK_FILE_LENGTH_OFFSET (UP_PACK_BLK_LENGTH_OFFSET + UP_PACK_BLK_LENGTH_SIZE)
#define UP_PACK_FILE_LENGTH_SIZE 4
#define UP_PACK_FILE_BLK_CNT_OFFSET (UP_PACK_FILE_LENGTH_OFFSET + UP_PACK_FILE_LENGTH_SIZE)
#define UP_PACK_FILE_BLK_CNT_SIZE 4
#define UP_PACK_FILE_TYPE_OFFSET (UP_PACK_FILE_BLK_CNT_OFFSET + UP_PACK_FILE_BLK_CNT_SIZE)
#define UP_PACK_FILE_TYPE_SIZE 1
#define UP_PACK_FILE_CHECK_OFFSET (UP_PACK_FILE_TYPE_OFFSET + UP_PACK_FILE_TYPE_SIZE)
#define UP_PACK_FILE_CHECK_SIZE 64
#define UP_PACK_FILE_NAME_OFFSET (UP_PACK_FILE_CHECK_OFFSET + UP_PACK_FILE_CHECK_SIZE)
#define UP_PACK_FILE_NAME_SIZE 20
#define UP_PACK_FILE_PATH_OFFSET (UP_PACK_FILE_NAME_OFFSET + UP_PACK_FILE_NAME_SIZE)
#define UP_PACK_FILE_PATH_SIZE 40

#define SND_CHECK_CODE_OFFSET 11

#define UP_DATA_RCV_CHECK_CODE_ERROR 20000

#define UP_DATA_RCV_SIZE_ERROR 31000
#define UP_DATA_CREATE_BAK_ERROR 31001
#define UP_DATA_WRITE_BAK_ERROR 31002
#define UP_DATA_UPDATE_FILE_ERROR 31003
#define UP_DATA_UPDATE_END_ERROR 31004


#define UP_DATA_IS_STEP_IDLE 30000
#define UP_DATA_IS_STEP1_UP_START 30001
#define UP_DATA_IS_STEP2_FILE_START 30002
#define UP_DATA_IS_STEP3_FILE_RCV 30003
#define UP_DATA_IS_STEP4_FILE_END 30004
#define UP_DATA_IS_STEP5_UP_END 30005
#define UP_DATA_IS_STEP6_UP_CANCEL 30006

#define FILE_DATA_SIZE 1282
//#define FILE_DATA_SIZE 74
#pragma pack(1)
typedef struct format1
{
    unsigned int all_file_cnt;
    unsigned int all_file_len;
}format1_t;

typedef struct format2 
{
    unsigned int file_len;
    unsigned int blk_cnt;
    unsigned char file_type;
    unsigned char check_code[UP_PACK_FILE_CHECK_SIZE];
    char file_name[UP_PACK_FILE_NAME_SIZE];
    char path_name[UP_PACK_FILE_PATH_SIZE];
}format2_t;

typedef struct format3456
{
    unsigned char file_data[FILE_DATA_SIZE];
}format3456_t;

typedef struct up_pack
{
   unsigned char header[3];
   unsigned char pro;
   unsigned char msg;
   unsigned char data_len[2];
   unsigned int file_blk_no;
   unsigned short cur_blk_len;
   union 
   {
       format1_t f1_data;
       format2_t f2_data;
       format3456_t f3456_data;
   }format_data;
   unsigned char check_code;
}up_pack_t, *p_up_pack_t;

#pragma pack()

static int g_up_server_run = 1;



void up_server_stop(void)
{
    //VI_DEBUG("up_server_stop now!\n");
    g_up_server_run = 0;
}

static int up_server_init(void)
{
    int ret = 0;
    //char *ip_a;
         
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

static void pack_snd_data(unsigned char *buf, unsigned char pro, unsigned int *data)
{
    buf[UP_PACK_PRO_OFFSET] = pro;
    memcpy(&(buf[UP_PACK_DATA_OFFSET]), data, UP_PACK_SND_DATA_SIZE);
}

static FILE *create_bak_file(char *file_name)
{
    FILE *fp;
    char *bak_file_name[64];
    
    sprintf(bak_file_name, "/tmp/%s.bak", file_name);
    UP_DEBUG("bak_file_name = %s\n", bak_file_name);
    fp = fopen(bak_file_name, "wb+");
    return fp;
}

static int update_file(FILE *bak_fp, char *path_name, char *file_name)
{
    FILE *up_fp;
    char f[64];
    unsigned char buf[4096];
    unsigned int cnt = 0;
    int ret = 0;
    int fd;

    sprintf(f, "%s%s", path_name, file_name);
    UP_DEBUG("update %s\n", f);
    up_fp = fopen(f, "w");
    if(up_fp == NULL)
    {
        UP_DEBUG("%s ", f);
        perror("fopen");
        return -1;
    }
    fseek(bak_fp, 0, SEEK_SET);
    
    while((cnt = fread(buf, 1, sizeof(buf), bak_fp)) > 0)
    {
        ret = fwrite(buf, 1, cnt, up_fp);
        if(ret < cnt)
        {
            UP_DEBUG("%s ", f);
            perror("fwrite");
            ret = -1;
            break;
        }
    }
    fd = fileno(up_fp);
    fsync(fd);
    fclose(up_fp);

    return ret;    
}

int main(int argc, char **argv)
{
    int ret = 0;
    //int i;

    int up_socketfd = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    //unsigned char rcv_buf[RCV_BUF_SIZE] = {0};
    unsigned char snd_buf[SND_BUF_SIZE] = {0};
    //char *client_ip = NULL;
    //unsigned int cmd_data_len = 0;
    //unsigned int cmd_len = 0;
    unsigned char check_code = 0;
    int rcv_count = 0;
    //int ack_len = 0;
    int snd_len = 0;
    int addr_len = sizeof(struct sockaddr_in);
    fd_set rcv_fd;
    struct timeval timeout;
    //struct timeval tv1, tv2, tv3;
    
    unsigned char client_id[] = {'V', 'F', 'U'};
    unsigned char server_id[] = {'V', 'F', 'D', 0x00/*PRO*/, 0xFD/*MSG*/, 0x00/*LEN_M*/, 0x04/*LEN_L*/, 0x00, 0x00, 0x00, 0x00/*DATA 4Bytes*/, 0x00/*CHECK Code*/};
    unsigned int snd_error_code;
    unsigned int blk_no;
    unsigned int file_no;
    char file_name[UP_PACK_FILE_NAME_SIZE];
    char path_name[UP_PACK_FILE_PATH_SIZE];
    char bak_file[(UP_PACK_FILE_NAME_SIZE + 8)];
    unsigned int cur_step = UP_DATA_IS_STEP_IDLE;
    unsigned int all_file_cnt;          //总的文件数量
    unsigned int rcv_file_cnt = 0;          //已经接收的文件数量
    unsigned int all_file_len;          //总的文件长度
    unsigned int rcv_all_file_len = 0;
    unsigned char cur_file_no = 0x01;   //当前文件序号
    unsigned int cur_blk_no = 0x0000001;//当前文件块序号
    unsigned int cur_file_blk_cnt = 0;  //当前文件块数量
    unsigned int rcv_file_blk_cnt = 0;  //已经接收的块数量
    unsigned int cur_file_len = 0;      //当前文件长度
    unsigned int rcv_file_len = 0;      //已经接收到的文件长度
    FILE *bak_fp = NULL;
    //FILE *up_fp = NULL;
    //int fd;
    

    up_pack_t up_pack_data;

    ret = up_server_init();
    if(ret < 0)
    {
        UP_DEBUG("Init cmd server error\n");
        return ret;
    }
    
    memset(&up_pack_data, 0, sizeof(up_pack_data));
    memset(snd_buf, 0, sizeof(snd_buf));

//create udp socket
    up_socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(up_socketfd < 0)
    {
        UP_DEBUG("Create cmd socket error\n");
        return up_socketfd;
    }
    UP_DEBUG("up_socketfd = %d\n", up_socketfd);
    UP_DEBUG("sizeof(up_pack_data) = %d\n", sizeof(up_pack_data));
    
//initial address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UP_PORT);

    ret = bind(up_socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if(ret < 0)
    {
        UP_DEBUG("bind socket error\n");
        return ret;
    }

    while(g_up_server_run)
    {
        //UP_DEBUG("wait from client\n");
        memcpy(snd_buf, server_id, SND_BUF_SIZE); 
        memset(&up_pack_data, 0, sizeof(up_pack_data));

        FD_ZERO(&rcv_fd);
        FD_SET(up_socketfd, &rcv_fd);
        timeout.tv_sec = UP_TIMEOUT;
        timeout.tv_usec = 0;

        ret = select(up_socketfd+1, &rcv_fd, 0, 0, &timeout);
        if(ret < 0)
        {
            //UP_DEBUG("cmd server rcv select error\n");
        }
        else if(ret ==0)
        {
            //UP_DEBUG("time out\n");
        }
        else
        {
            if(FD_ISSET(up_socketfd, &rcv_fd))
            {
                rcv_count = recvfrom(up_socketfd, &up_pack_data, sizeof(up_pack_data), 0, (struct sockaddr *)&client_addr,&addr_len); 
                if(rcv_count < 0)
                {
                    perror("rcvfrom\n");
                }
                else
                {//接收到数据
                    if(memcmp(up_pack_data.header, client_id, 3) != 0)
                    {
                        UP_DEBUG("it is not my data\n");
                    }
                    else if(up_pack_data.pro != 0x01)
                    {
                        UP_DEBUG("pro is error\n");
                    }
                    else if(up_pack_data.msg != 0xFD)
                    {
                        UP_DEBUG("msg is error\n");
                    }
                    else
                    {//是我的数据
                        if(rcv_count != sizeof(up_pack_data)) 
                        {//接收到的长度不对
                            snd_error_code = UP_DATA_RCV_SIZE_ERROR;
                            pack_snd_data(snd_buf, 0xff, &snd_error_code);
                        }
                        else 
                        {//长度正确
                            check_code = calc_check_code((unsigned char *)&up_pack_data, sizeof(up_pack_data) - 1);
                            //UP_DEBUG("check_code = %02X\n", check_code);
                            //UP_DEBUG("up_pack_data.check_code = %02X\n", up_pack_data.check_code);
                            if(check_code != up_pack_data.check_code)
                            {//校验错
                                snd_error_code = UP_DATA_RCV_CHECK_CODE_ERROR; 
                                pack_snd_data(snd_buf, 0xff, &snd_error_code);
                            }
                            else 
                            {//校验正确
                             //   memcpy(&up_pack_data, rcv_buf, RCV_BUF_SIZE);
                                //UP_DEBUG("up_pack_data.header: ");
                                //for(i = 0; i < 3; i++)
                                //    printf("%02X ", up_pack_data.header[i]);
                                //UP_DEBUG("\n");
                                //UP_DEBUG("up_pack_data.pro: %02X\n", up_pack_data.pro); 
                                //UP_DEBUG("up_pack_data.msg: %02X\n", up_pack_data.msg); 
                                //UP_DEBUG("up_pack_data.file_blk_no: %08X\n", up_pack_data.file_blk_no); 
                                blk_no = (up_pack_data.file_blk_no) & 0x00ffffff;
                                file_no = ((up_pack_data.file_blk_no) & 0xff000000) >> 24;
                                UP_DEBUG("blk_no = %x\n", blk_no);
                                UP_DEBUG("file_no = %x\n", file_no);
                                //UP_DEBUG("up_pack_data.format_data.f2_data.file_name = %s\n", up_pack_data.format_data.f2_data.file_name);
                                //UP_DEBUG("up_pack_data.format_data.f2_data.path_name = %s\n", up_pack_data.format_data.f2_data.path_name);
                                if((blk_no == 0x000000) && (file_no == 0x00))
                                {//格式1
                                    UP_DEBUG("it is update start format 1\n");
                                    if(cur_step == UP_DATA_IS_STEP_IDLE)
                                    {//在IDLE阶段接收到更新开始指令
                                        UP_DEBUG("update start\n");
                                        all_file_cnt = up_pack_data.format_data.f1_data.all_file_cnt;
                                        all_file_len = up_pack_data.format_data.f1_data.all_file_len;
                                        UP_DEBUG("all_file_cnt = %d\n", all_file_cnt);
                                        UP_DEBUG("all_file_len = %d\n", all_file_len);
                                        UP_DEBUG("cur_blk_len = %d\n", up_pack_data.cur_blk_len);
                                        cur_step = UP_DATA_IS_STEP1_UP_START;
                                        system("/opt/ipnc/killall.sh");
                                        sleep(2);
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                    } 
                                    else 
                                    {
                                        UP_DEBUG("cur_step is %d\n", cur_step);
                                        //pack_snd_data(snd_buf, 0xff, &cur_step); 
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                    }
                                }//格式1
                                if((blk_no == 0x000000) && (file_no > 0x00) && (file_no < 0x80))
                                {//格式2
                                    UP_DEBUG("it is file start format 2\n");
                                    if(cur_step == UP_DATA_IS_STEP1_UP_START)
                                    {//已经开始进行升级，进入获取文件开始阶段
                                       UP_DEBUG("file start\n"); 
                                       if(file_no == 0x01)
                                       {//如果文件序号为1
                                          cur_file_no = 1;
                                          cur_file_len = up_pack_data.format_data.f2_data.file_len;
                                          cur_file_blk_cnt = up_pack_data.format_data.f2_data.blk_cnt;
                                          memcpy(file_name, up_pack_data.format_data.f2_data.file_name, sizeof(file_name));
                                          memcpy(path_name, up_pack_data.format_data.f2_data.path_name, sizeof(path_name));
                                          UP_DEBUG("file_name = %s\n", up_pack_data.format_data.f2_data.file_name);
                                          UP_DEBUG("path_name = %s\n", up_pack_data.format_data.f2_data.path_name);
                                          bak_fp = create_bak_file(file_name);
                                          if(bak_fp == NULL)
                                          {//创建文件失败
                                              snd_error_code = UP_DATA_CREATE_BAK_ERROR;
                                              pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                          }    
                                          else
                                          {//创建文件成功
                                              cur_step = UP_DATA_IS_STEP2_FILE_START;
                                              pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                          }
                                       }
                                       else 
                                       {//如果文件序号不为1,由于是从STEP1进入的STEP2,因此序号必须为1
                                           UP_DEBUG("cur_step is %d\n", cur_step); 
                                           pack_snd_data(snd_buf, 0xff, &cur_step);
                                       }
                                    }//已经开始进行升级，进入文件开始阶段
                                    else if(cur_step == UP_DATA_IS_STEP4_FILE_END)
                                    {//当前是文件更新完成阶段，进入下一个文件更新
                                        UP_DEBUG("maybe it is next file start\n");
                                        UP_DEBUG("cur_file_no = %d\n", cur_file_no);
                                        UP_DEBUG("file_no = %d\n", file_no);
                                        if(file_no != (cur_file_no + 1))
                                        {//如果文件序号并不是加1, 返回当前的文件序号
                                            UP_DEBUG("not right next file\n");
                                            snd_error_code = cur_file_no << 24;
                                            snd_error_code = snd_error_code & 0xff000000;
                                            pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                        }//如果文件序号并不是加1,返回当前文件序号
                                        else 
                                        {//是正确的文件序号
                                            cur_file_no = file_no; 
                                            cur_file_len = up_pack_data.format_data.f2_data.file_len;
                                            cur_file_blk_cnt = up_pack_data.format_data.f2_data.blk_cnt;
                                            memcpy(file_name, up_pack_data.format_data.f2_data.file_name, sizeof(file_name));
                                            memcpy(path_name, up_pack_data.format_data.f2_data.path_name, sizeof(path_name));
                                            bak_fp = create_bak_file(file_name);
                                            if(bak_fp == NULL)
                                            {//创建文件失败
                                                snd_error_code = UP_DATA_CREATE_BAK_ERROR;
                                                pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                            }    
                                            else
                                            {//创建文件成功
                                                cur_step = UP_DATA_IS_STEP2_FILE_START;
                                                pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                            }
                                        }//是正确的文件序号
                                    }//当前是文件更新完成阶段，进入下一个文件更新
                                    else 
                                    {//别的状态
                                        UP_DEBUG("format 2 is not for cur_step = %d\n", cur_step);
                                        //pack_snd_data(snd_buf, 0xff, &cur_step); 
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                    }//别的状态
                                }//格式2
                                if((blk_no != 0x000000) && (file_no != 0x00) && (up_pack_data.cur_blk_len != 0x0000))
                                {//格式3
                                    UP_DEBUG("it is file data format 3\n");
                                    if(cur_step == UP_DATA_IS_STEP2_FILE_START)
                                    {//从文件开始阶段进入到获取文件内容阶段
                                        UP_DEBUG("it is first rcv file %s\n", file_name);
                                        if((cur_file_no == file_no) && (blk_no == 0x000001))
                                        {//是当前的文件序号且当前块号为1
                                            UP_DEBUG("file_no  %d is right, and blk_no %d is right\n", file_no, blk_no);
                                            if(bak_fp != NULL)
                                            {
                                                ret = fwrite(up_pack_data.format_data.f3456_data.file_data, 1, up_pack_data.cur_blk_len, bak_fp);
                                                if(ret != up_pack_data.cur_blk_len)
                                                {//写入文件错误
                                                    UP_DEBUG("write file: %s ", file_name);
                                                    perror("error");
                                                    snd_error_code = UP_DATA_WRITE_BAK_ERROR; 
                                                    pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                                }
                                                else 
                                                {
                                                    rcv_file_len = rcv_file_len + up_pack_data.cur_blk_len;
                                                    rcv_file_blk_cnt++; 
                                                    cur_blk_no = blk_no;
                                                    cur_step = UP_DATA_IS_STEP3_FILE_RCV;
                                                    UP_DEBUG("write file %d blk %d ok\n", cur_file_no, cur_blk_no);
                                                    pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                                }
                                            }
                                            else 
                                            {
                                               UP_DEBUG("bak_fp is NULL"); 
                                               snd_error_code = UP_DATA_WRITE_BAK_ERROR;
                                               pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                            }
                                        }//是当前文件序号且当前块号为1
                                        else 
                                        {//从文件开始阶段进入到获取文件内容阶段，如果是格式3，则只能是当前文件序号且当前为1,否则都是错误情况
                                            UP_DEBUG("it is first rcv file, but blk_no is not 0x000001\n");
                                            snd_error_code = (cur_file_no << 24) & 0xff00000;
                                            pack_snd_data(snd_buf, 0x0ff, &snd_error_code);
                                        }
                                    }//从文件开始阶段进入到获取文件内容阶段
                                    else if(cur_step == UP_DATA_IS_STEP3_FILE_RCV)
                                    {//已经是文件接收阶段了
                                        UP_DEBUG("it is already in file rcv\n");
                                        if((file_no == cur_file_no) && (blk_no = (cur_blk_no + 1)))
                                        {//文件序号及块号都是对的
                                            UP_DEBUG("file_no %d is right, and blk_no %d is right\n", file_no, blk_no);
                                            if(bak_fp != NULL)
                                            {
                                                ret = fwrite(up_pack_data.format_data.f3456_data.file_data, 1, up_pack_data.cur_blk_len, bak_fp);
                                                if(ret != up_pack_data.cur_blk_len)
                                                {//写入文件错误
                                                    UP_DEBUG("write file: %s ", file_name);
                                                    perror("error");
                                                    snd_error_code = UP_DATA_WRITE_BAK_ERROR; 
                                                    pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                                }
                                                else 
                                                {
                                                    rcv_file_len = rcv_file_len + up_pack_data.cur_blk_len;
                                                    rcv_file_blk_cnt++; 
                                                    cur_blk_no = blk_no;
                                                    cur_step = UP_DATA_IS_STEP3_FILE_RCV;
                                                    UP_DEBUG("write file %d blk %d ok\n", cur_file_no, cur_blk_no);
                                                    pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                                }
                                            }
                                            else 
                                            {
                                               UP_DEBUG("bak_fp is NULL"); 
                                               snd_error_code = UP_DATA_WRITE_BAK_ERROR;
                                               pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                            }
                                        }//文件序号及块号都是对的
                                        else 
                                        {//文件序号及块号有一个不对
                                            UP_DEBUG("file_no %d is not cur_file_no %d, or blk_no %d is not cur_blk_no %d\n", file_no, cur_file_no, blk_no, cur_blk_no);
                                            snd_error_code = ((cur_file_no << 24) & 0xff000000) | (cur_blk_no & 0x00ffffff);
                                            pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                        }
                                    }//已经是文件接收阶段了
                                    else 
                                    {//其他状态
                                        UP_DEBUG("format 3 is not right for other steps %d\n", cur_step);
                                        //pack_snd_data(snd_buf, 0xff, &cur_step);
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                    }
                                }//格式3
                                if((blk_no == 0x00ffffff) && (file_no != 0xff) && (file_no != 0x00) && (up_pack_data.cur_blk_len == 0x0000))
                                {//格式4
                                   UP_DEBUG("it is file end format 4\n"); 
                                   if(cur_step == UP_DATA_IS_STEP3_FILE_RCV)
                                   {//如果处于接收文件内容的阶段，那就有可能对了
                                       UP_DEBUG("it is file rcv status\n");
                                       if((file_no == cur_file_no) && (rcv_file_blk_cnt == cur_file_blk_cnt) && (rcv_file_len == cur_file_len))
                                       {//文件序号对上、接收到的块数对上，接收到的文件大小对上，该文件序号的文件完成接收
                                            UP_DEBUG("it is right file ending\n");
                                            if(bak_fp != NULL)
                                            {
                                                UP_DEBUG("begain to update file\n");
                                                ret = update_file(bak_fp, path_name, file_name);
                                                if(ret != -1)
                                                {
                                                    UP_DEBUG("update file success\n");
                                                    fclose(bak_fp);
                                                    sprintf(bak_file, "/tmp/%s.bak", file_name);    
                                                    ret = remove(bak_file);
                                                    if(ret == 0)
                                                    {
                                                        UP_DEBUG("remove bak file success\n");
                                                        rcv_file_cnt = rcv_file_cnt + 1;
                                                        rcv_all_file_len = rcv_all_file_len + rcv_file_len;
                                                        rcv_file_len = 0;
                                                        rcv_file_blk_cnt = 0;
                                                        cur_step = UP_DATA_IS_STEP4_FILE_END;
                                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                                    }
                                                    else 
                                                    {
                                                        UP_DEBUG("remove bak file: ");
                                                        perror("bak file");
                                                        snd_error_code = UP_DATA_UPDATE_FILE_ERROR;
                                                        pack_snd_data(snd_buf, 0x0ff, &snd_error_code);
                                                    }
                                                }
                                                else 
                                                {
                                                    UP_DEBUG("update file failed\n");
                                                    snd_error_code = UP_DATA_UPDATE_FILE_ERROR;
                                                    pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                                }
                                            }
                                            else 
                                            {
                                                UP_DEBUG("write file error\n");
                                                snd_error_code = UP_DATA_WRITE_BAK_ERROR;
                                                pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                            }
                                       }//文件序号对上、接收到的块数对上，接收到的文件大小对上，该文件序号的文件完成接收
                                       else
                                       {//只要有一个没对上
                                           UP_DEBUG("file_no/rcv_file_blk_cnt/rcv_file_len is not error\n");
                                           snd_error_code = ((cur_file_no << 24) & 0xff000000) | (cur_blk_no & 0x00ffffff);
                                           pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                       }

                                   }//如果处于接收文件内容的阶段，那就有可能对了
                                   else 
                                   {//其他状态
                                        UP_DEBUG("format4 is not right for other steps %d\n", cur_step);
                                        //pack_snd_data(snd_buf, 0xff, &cur_step);
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                   }
                                }//格式4
                                if((blk_no == 0x00ffffff) && (file_no == 0xff) && (up_pack_data.cur_blk_len == 0x0000))
                                {//格式5
                                    UP_DEBUG("it is update end format 5\n");
                                    if(cur_step == UP_DATA_IS_STEP4_FILE_END)
                                    {//只有在状态4的时候接收到格式5的指令才是正确的操作
                                     //需要进行更新文件总数，文件总长度的校验之后，就可以进行重启操作，完成更新工作
                                        UP_DEBUG("it is file end status\n");
                                        UP_DEBUG("rcv_file_cnt = %d\n", rcv_file_cnt);
                                        UP_DEBUG("all_file_cnt = %d\n", all_file_cnt);
                                        UP_DEBUG("rcv_all_file_len = %d\n", rcv_all_file_len);
                                        UP_DEBUG("all_file_cnt = %d\n", all_file_cnt);
                                        if((rcv_file_cnt == all_file_cnt) && (rcv_all_file_len == all_file_len))
                                        {//接收到的文件数量和文件总长度对头
                                            UP_DEBUG("file rcv right\n");
                                            UP_DEBUG("update end\n");
                                            pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                            ret = 12345;
                                        }
                                        else 
                                        {
                                            UP_DEBUG("file rcv wrong\n");
                                            snd_error_code = UP_DATA_UPDATE_END_ERROR;
                                            pack_snd_data(snd_buf, 0xff, &snd_error_code);
                                        }

                                    }//只有在状态4的时候接收到格式5的指令才是正确的操作
                                    else 
                                    {//其他状态
                                        UP_DEBUG("format5 is not right for other steps %d\n", cur_step);
                                        //pack_snd_data(snd_buf, 0xff, &cur_step);
                                        pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                    }
                                }//格式5
                                if((blk_no == 0x00000000) && (file_no == 0xff) && (up_pack_data.cur_blk_len == 0x0000))
                                {//格式6
                                    UP_DEBUG("it is update cancle format 6\n");
                                    ret = 12345;
                                    pack_snd_data(snd_buf, 0x01, &up_pack_data.file_blk_no);
                                }
                            }//校验正确
                        }

                        snd_buf[SND_BUF_SIZE - 1] = calc_check_code(snd_buf, SND_BUF_SIZE - 1);
                        //UP_DEBUG("snd_buf: ");
                        //for(i = 0; i < SND_BUF_SIZE; i++)
                        //{
                        //    printf("%02x ", snd_buf[i]);
                        //}
                        //printf("\n");
                        snd_len = sendto(up_socketfd, snd_buf, SND_BUF_SIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)); 
                        if(snd_len != SND_BUF_SIZE)
                        {
                            UP_DEBUG("snd_len = %d\n", snd_len);
                            perror("sendto");
                        }
                        if(ret == 12345)
                        {
                            system("reboot");
                        }
                    }//是我的数据
                }//接收到数据
            }//FD_ISSET(up_socketfd, &rcv_fd)
        }//select返回非0
    }//while

    close(up_socketfd); 
    up_server_exit();
    return ret;
}

