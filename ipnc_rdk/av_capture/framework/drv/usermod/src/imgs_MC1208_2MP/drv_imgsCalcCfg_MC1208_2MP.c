
#include "drv_imgs_MC1208_2MP.h"
#include <math.h>
#define DRV_SHOW_INFO
int DRV_imgsCalcFrameRate(int fps)
{
  if(fps<5)
    fps=30;

  if(fps>30)
    fps=30;

  gDRV_imgsObj.curFrameTime.fps    = fps;

  gDRV_imgsObj.curFrameTime.t_frame = 1000.0*IMGS_MHZ/(gDRV_imgsObj.curFrameTime.fps);
  gDRV_imgsObj.curFrameTime.VB      = gDRV_imgsObj.curFrameTime.t_frame/gDRV_imgsObj.curFrameTime.t_row - gDRV_imgsObj.curFrameTime.H;

  if(gDRV_imgsObj.curFrameTime.VB < gDRV_imgsObj.curFrameTime.VBmin)
    gDRV_imgsObj.curFrameTime.VB = gDRV_imgsObj.curFrameTime.VBmin;

  return 0;
}

int DRV_imgsCalcFrameTime(Uint32 fps, Uint32 dataW, Uint32 dataH, Bool binEnable, Bool flipH, Bool flipV)
{
  DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;
  DRV_ImgsModeConfig *pModeCfg = &gDRV_imgsObj.curModeConfig;

  memset(pFrame, 0, sizeof(*pFrame));

  if(dataW>IMGS_W_MAX)
    dataW = IMGS_W_MAX;

  if(dataH>IMGS_H_MAX)
    dataH = IMGS_H_MAX;

  if(fps<5)
    fps=30;

  if(fps>30)
    fps=30;

  pFrame->col_skip = IMGS_W_MAX/dataW - 1;
  pFrame->row_skip = IMGS_H_MAX/dataH - 1;

  if(pFrame->col_skip < pFrame->row_skip)
    pFrame->row_skip = pFrame->col_skip;

  if(pFrame->col_skip > pFrame->row_skip)
    pFrame->col_skip = pFrame->row_skip;

  pFrame->col_size = dataW*(pFrame->col_skip+1) - 1;
  pFrame->row_size = dataH*(pFrame->row_skip+1) - 1;

  pFrame->col_bin  = 0;
  pFrame->row_bin  = 0;

  if(binEnable) {
    switch(pFrame->col_skip) {
      case 3:
        pFrame->col_bin = 2;
        break;
      case 1:
      case 5:
        pFrame->col_bin = 1;
        break;
    }
    switch(pFrame->row_skip) {
    	case 2:
      case 3:
        pFrame->row_bin = 2;
        break;
      case 1:
      case 5:
        pFrame->row_bin = 1;
        break;
    }
  }

  pFrame->H      = 2*OSA_ceil(dataH/2, 2);
  pFrame->W      = 2*OSA_ceil(dataW/2, 2);

  pFrame->col_start = IMGS_COL_START_MIN + (IMGS_W_MAX - (pFrame->col_size+1))/2;
  pFrame->row_start = IMGS_ROW_START_MIN + (IMGS_H_MAX - (pFrame->row_size+1))/2;

  if(pFrame->col_start < 0 )
    pFrame->col_start = 0;

  if(pFrame->row_start < 0 )
    pFrame->row_start = 0;

  pFrame->col_start = OSA_align(pFrame->col_start, 4*(pFrame->col_bin+1));
  pFrame->row_start = OSA_align(pFrame->row_start, 4*(pFrame->row_bin+1));

  pFrame->VBmin  = 9;
  pFrame->HBmin  = 346*(pFrame->row_bin+1)+64+(80/(pFrame->col_bin+1))/2;
  pFrame->HB     = pFrame->HBmin;
  pFrame->SW     = pFrame->H-1;
  pFrame->SD     = 0;

#if 0
  if(dataW*dataH >= 1920*1080) {
    if(fps>24)
      fps = 24;
  }
  if(dataW*dataH >= 2048*1536) {
    if(fps>15)
      fps = 15;
  }
#endif

  pFrame->f_pclk = 48*IMGS_MHZ;
  if(dataW*dataH >= 720*480)
    pFrame->f_pclk = 64*IMGS_MHZ;
  if(dataW*dataH >= 1280*960) {
    if(pFrame->row_bin==0)
      pFrame->f_pclk = 64*IMGS_MHZ;
    else
      pFrame->f_pclk = 96*IMGS_MHZ;
  }
  if(dataW*dataH >= 1600*1200)
    pFrame->f_pclk = 96*IMGS_MHZ;

  pFrame->pll_N = (2-1);

  pFrame->pll_M = (16);

  pFrame->pll_DIV = (IMGS_PLL_REF_CLK*pFrame->pll_M)/((pFrame->pll_N+1)*pFrame->f_pclk) - 1;

  pFrame->t_pclk = 1000.0*IMGS_MHZ/pFrame->f_pclk;

  pFrame->t_row  = 2*pFrame->t_pclk*OSA_max(
                                    pFrame->W/2 + OSA_max(pFrame->HB, pFrame->HBmin),
                                    41+396*(pFrame->row_bin+1)+99
                                    );


  pFrame->shutterOverhead = 208 * (pFrame->row_bin+1) + 98 + (pFrame->SD+1) - 94;
  pFrame->shutterOverhead = 2 * pFrame->t_pclk * pFrame->shutterOverhead;

  DRV_imgsCalcFrameRate(fps);
  OSA_printf(" SENSOR FPS VALUE = %d\n", pFrame->fps);

  DRV_imgsCalcSW(pFrame->t_frame/1000);

  pModeCfg->sensorDataWidth   = pFrame->W;
  pModeCfg->sensorDataHeight  = pFrame->H;
  #ifdef IMGS_OV10630_1MP
  pModeCfg->validStartX       = 0;
  pModeCfg->validStartY       = 0;
  pModeCfg->validWidth        = pModeCfg->sensorDataWidth;
  pModeCfg->validHeight       = pModeCfg->sensorDataHeight;
  #else
  pModeCfg->validStartX       = IMGS_H_PAD/2;
  pModeCfg->validStartY       = IMGS_V_PAD/2;
  pModeCfg->validWidth        = pModeCfg->sensorDataWidth-IMGS_H_PAD;
  pModeCfg->validHeight       = pModeCfg->sensorDataHeight-IMGS_V_PAD;
  #endif
  pModeCfg->binEnable         = binEnable;

  if( flipV == 0 )
  {
		if(pModeCfg->sensorDataWidth > 1360 && pModeCfg->sensorDataWidth < 1440 )
		{
			pModeCfg->vdint0						= 990;//930;
		  pModeCfg->vdint1						= 1060;
	  }
	  else if(pModeCfg->sensorDataWidth > 1440 && pModeCfg->sensorDataWidth < 1620 )
	  {
			pModeCfg->vdint0						= 1000;//950;
			pModeCfg->vdint1						= 1060;
	  }
	  else if(pModeCfg->sensorDataWidth > 1620 && pModeCfg->sensorDataWidth < 1910 )
	  {
		  pModeCfg->vdint0						= 1000;//960;
		  pModeCfg->vdint1						= 1060;

	  }else{
		  pModeCfg->vdint0						= 955;//950;
		  pModeCfg->vdint1						= 1060;//1060;
		}
	}else{

		if(pModeCfg->sensorDataWidth > 1360 && pModeCfg->sensorDataWidth < 1440 )
		{
			pModeCfg->vdint0						= 1045;
			pModeCfg->vdint1						= 1067;
		}
		else if(pModeCfg->sensorDataWidth > 1440 && pModeCfg->sensorDataWidth < 1620 )
		{
			pModeCfg->vdint0						= 1000;
			pModeCfg->vdint1						= 1060;
		}
		else if(pModeCfg->sensorDataWidth > 1620 && pModeCfg->sensorDataWidth < 1910 )
		{
			pModeCfg->vdint0						= 1000;
			pModeCfg->vdint1						= 1060;

		}else{
			pModeCfg->vdint0						= 1040;//950;
			pModeCfg->vdint1						= 1067;//1060;
		}

	}

  pModeCfg->vdint2						= pModeCfg->vdint1;

  #ifdef DRV_SHOW_INFO
  OSA_printf(" Sensor Mode Info,\n");
  OSA_printf(" Width      = %4d \n", pModeCfg->sensorDataWidth);
  OSA_printf(" Height     = %4d \n", pModeCfg->sensorDataHeight);
  OSA_printf(" fps        = %4d \n", fps);
  OSA_printf(" Bin Enable = %4d \n", pModeCfg->binEnable );
  OSA_printf(" flipH      = %4d \n", flipH );
  OSA_printf(" flipV      = %4d \n", flipV );
  OSA_printf(" \n");
  OSA_printf(" Sensor Frame Timing Info,\n");
  OSA_printf(" fps                  = %d\n", pFrame->fps              );
  OSA_printf(" t_frame (ns)         = %f\n", pFrame->t_frame          );
  OSA_printf(" t_row   (ns)         = %f\n", pFrame->t_row            );
  OSA_printf(" t_pclk  (ns)         = %f\n", pFrame->t_pclk           );
  OSA_printf(" W                    = %d\n", pFrame->W                );
  OSA_printf(" H                    = %d\n", pFrame->H                );
  OSA_printf(" SW                   = %d\n", pFrame->SW               );
  OSA_printf(" SD                   = %d\n", pFrame->SD               );
  OSA_printf(" HB                   = %d\n", pFrame->HB               );
  OSA_printf(" VB                   = %d\n", pFrame->VB               );
  OSA_printf(" HBmin                = %d\n", pFrame->HBmin            );
  OSA_printf(" VBmin                = %d\n", pFrame->VBmin            );
  OSA_printf(" f_pclk               = %d\n", pFrame->f_pclk           );
  OSA_printf(" shutterOverhead (ns) = %f\n", pFrame->shutterOverhead  );
  OSA_printf(" col_size             = %d\n", pFrame->col_size         );
  OSA_printf(" row_size             = %d\n", pFrame->row_size         );
  OSA_printf(" col_skip             = %d\n", pFrame->col_skip         );
  OSA_printf(" row_skip             = %d\n", pFrame->row_skip         );
  OSA_printf(" col_bin              = %d\n", pFrame->col_bin          );
  OSA_printf(" row_bin              = %d\n", pFrame->row_bin          );
  OSA_printf(" col_start            = %d\n", pFrame->col_start        );
  OSA_printf(" row_start            = %d\n", pFrame->row_start        );
  OSA_printf(" pll_M                = %d\n", pFrame->pll_M            );
  OSA_printf(" pll_N                = %d\n", pFrame->pll_N            );
  OSA_printf(" pll_DIV              = %d\n", pFrame->pll_DIV          );
  OSA_printf(" vdint0               = %d\n", pModeCfg->vdint0         );
  OSA_printf(" vdint1               = %d\n", pModeCfg->vdint1         );
  OSA_printf(" \n");
  #endif

  return OSA_SOK;
}

int DRV_imgsCalcSW(int exposureTimeInUsecs)
{
	int hts = 2420;
	int	div_base = 1;
  DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;

  pFrame->SW = (int)((double)exposureTimeInUsecs/(double)33333*(double)(0x44a0));

  if(pFrame->SW<1)
    pFrame->SW = 1;

  if(pFrame->SW>0x11280)
    pFrame->SW = 0x11280 ;

	if(exposureTimeInUsecs <= 33333)
	{
		div_base = 1;
	}
	else if(exposureTimeInUsecs <= 66666)
	{
		div_base = 2;
	}
	else
	{
		div_base = 4;
	}

	hts *= div_base;
	//pFrame->SW /= div_base;
	pFrame->div_base = (div_base-1)<<4;

  //OSA_printf(" exposureTimeInUsecs = %d sw = %d\n", exposureTimeInUsecs,pFrame->SW);
  //OSA_printf(" div_base = 0x%X \n", div_base);

  return OSA_SOK;
}

int DRV_imgsCalcAgain(int aGain)
{
  int gainRegVal=0;
  int temp1=0;
  int temp2=0;

  if(aGain<10000)
    aGain = 10000;

  if(aGain>310000)
    aGain = 310000;

	/*bit[7:4]*/
  temp1 = (Uint16)(log(aGain/10000)/log(2));
  gainRegVal = ((1<<temp1)-1)<<4;


	/*
		((aGain/1000)/(2^temp1)-1)*16
		= ((aGain/1000)*16/(2^temp1)-16)
		= ((aGain/1000)<<4>>temp1-16)
		= ((aGain/1000)<<(4-temp1)-16)
		= ((aGain<<(4-temp1)/1000)-16)
	*/
	/*bit[3:0]*/
	temp2 = (((aGain<<(4-temp1))/10000)-16);
  gainRegVal |= temp2;


	//OSA_printf(" aGain = %d gainRegVal = 0x%X\n", aGain, gainRegVal);
	//OSA_printf(" temp1 = %d temp2 = %d caltemp1 = %d \n", temp1, temp2, ((1<<temp1)-1)<<4);
  return gainRegVal;
}

int DRV_imgsInvertSW(int regVal)
{
  DRV_ImgsFrameTime *pFrame = &gDRV_imgsObj.curFrameTime;
  int exposureTimeInUsecs;

  exposureTimeInUsecs = (regVal*pFrame->t_row - pFrame->shutterOverhead)/1000;

  return exposureTimeInUsecs;
}

