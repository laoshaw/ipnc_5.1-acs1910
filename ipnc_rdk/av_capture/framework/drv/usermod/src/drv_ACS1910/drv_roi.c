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
    unsigned char roi0_his_avg, roi1_his_avg, roi2_his_avg;
    unsigned short roi0_height, roi0_width;
    unsigned short roi1_height, roi1_width;
    unsigned short roi2_height, roi2_width;
    unsigned int roi_his_sum, roi1_his_sum, roi2_his_sum;
    unsigned int roi_pix_sum, roi1_pix_sum, roi2_pix_sum;
    unsigned int roi_his_avg; 
    ptACS1910ISPAllCfg pISPAllCfg = &(gACS1910_current_cfg.ISPAllCfg);

    DRV_FPGASPIRead(ROI0_WIDTH_FPGA_REG_ADDR, &roi0_width);
    DRV_FPGASPIRead(ROI0_HEIGHT_FPGA_REG_ADDR, &roi0_height);
    DRV_FPGASPIRead(ROI0_HISTOGRAM_FPGA_REG_ADDR, &roi0_his_avg);
    roi_his_avg = roi0_his_avg & 0xff; 
    roi_pix_sum = roi0_width * roi0_height;
    roi_his_sum = roi_his_avg * roi_pix_sum;
    VI_DEBUG("roi0_width = %d, roi0_height = %d\n", roi0_width, roi0_height);
    VI_DEBUG("roi0_his_avg = %d, roi0_pix_sum = %d, roi0_his_sum = %d\n", roi0_his_avg, roi_pix_sum, roi_his_sum);

    if(pISPAllCfg->AERoi[1].onoff == 1)
    {
        DRV_FPGASPIRead(ROI1_WIDTH_FPGA_REG_ADDR, &roi1_width);
        DRV_FPGASPIRead(ROI1_HEIGHT_FPGA_REG_ADDR, &roi1_height);
        DRV_FPGASPIRead(ROI1_HISTOGRAM_FPGA_REG_ADDR, &roi1_his_avg);
        roi1_his_avg = roi1_his_avg & 0xff; 
        roi_pix_sum = roi_pix_sum + roi1_width * roi1_height;
        roi_his_sum = roi_his_sum + roi1_his_avg * roi1_width * roi1_height;
        roi_his_avg = roi_his_sum / roi_pix_sum; 
        VI_DEBUG("roi1_width = %d, roi1_height = %d\n", roi1_width, roi1_height);
        VI_DEBUG("roi1_his_avg = %d, roi1+roi0_pix_sum = %d, roi1+roi0_his_sum = %d\n", roi1_his_avg, roi_pix_sum, roi_his_sum);
        VI_DEBUG("roi1+0_his_avg = %d\n", roi_his_avg);
    }

    if(pISPAllCfg->AERoi[2].onoff == 1)
    {
        DRV_FPGASPIRead(ROI2_WIDTH_FPGA_REG_ADDR, &roi2_width);
        DRV_FPGASPIRead(ROI2_HEIGHT_FPGA_REG_ADDR, &roi2_height);
        DRV_FPGASPIRead(ROI2_HISTOGRAM_FPGA_REG_ADDR, &roi2_his_avg);
        roi2_his_avg = roi2_his_avg & 0xff; 
        roi_pix_sum = roi_pix_sum + roi2_width * roi2_height;
        roi_his_sum = roi_his_sum + roi2_his_avg * roi2_width * roi2_height;
        roi_his_avg = roi_his_sum / roi_pix_sum;
        VI_DEBUG("roi2_width = %d, roi2_height = %d\n", roi2_width, roi2_height);
        VI_DEBUG("roi2_his_avg = %d, roi2+roi1+roi0_pix_sum = %d, roi2+roi1+roi0_his_sum = %d\n", roi2_his_avg, roi_pix_sum, roi_his_sum);
        VI_DEBUG("roi2+1+0_his_avg = %d\n", roi_his_avg);
    }

    return roi_his_avg;
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




