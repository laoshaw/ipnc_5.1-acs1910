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
#include <netinet/in.h>
#include <string.h>
#include <drv_csl.h>
#include <drv_gpio.h>
#include "drv.h"
#include "drv_ACS1910.h"
#include "cmd_server.h"
#include "VIM_API_Release.h"
#include "drv_roi.h"

unsigned char VIM_roi_calc_histogram_avg()
{
    
}

void VIM_roi_autoexp_thread()
{

    VI_DEBUG("Hello roi autoexp thread!\n");

    while(VIM_roi_autoexp_thread_run)
    {
        if(gACS1910_current_cfg.ISPAllCfg.ISPNormalCfg.AEMode.AE_Shutter_Mode == VF_AE_ROI)
        {
            VI_DEBUG("ROI autoexp\n");
            if(gACS1910_current_cfg.ISPAllCfg.AERoi[0].onoff == 1)
            {
                VIM_roi_calc_histogram_avg();
            }
            usleep(1000000);
        }
        else 
        {
            sleep(1);
        }
    }
}


int VIM_roi_autoexp_thread_init()
{
    int retVal = 0;
    VI_DEBUG("Initialize roi autoexp_thread\n");

    if((retVal = pthread_create(&VIM_roi_autoexp_thread_id, NULL, VIM_roi_autoexp_thread, NULL)) != 0)
    {
        perror("Create ROI autoexp thread\n");
    }
    else 
    {
        VIM_roi_autoexp_thread_run = 1;
        VI_DEBUG("Create roi autoexp thread done!\n\n");
    }
}




