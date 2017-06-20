
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"
#include "TI_aewb_ISP.h"

struct rgb2rgb_index rgb_matrixes_0[]= {
 {
    //U30-1080 (T=3000K)
    .color_temp = 2800,
    .rgb2rgbparam = {
      357,  -82,-19,
      -76,  325,  7,
      -38, -265,558,
        0, 0, 0
    },
    .rgb2rgb2param = {
      250,  -8,  14,
       -2, 257,   1,
       -5,  -2, 263,
      0, 0, 0
    }
  },
  {
//TL84  (T=4000K)
    .color_temp = 4000,
    .rgb2rgbparam = {
        367, -138,  27,
        -61,  353, -36,
        -24, -168, 448,
         0,     0,   0
    },
    .rgb2rgb2param = {
         252,-9,  13,
           1, 255, 0,
          -2,  -2, 260,
           0, 0, 0

    }
  },
//DAY-1080 (T=5000K)
  {
    .color_temp = 5000,
    .rgb2rgbparam = {
      382, -167,  41,
      -65,  355, -34,
      -11, -159, 426,
        0,    0,   0
    },
    .rgb2rgb2param = {
      254, -12, 13,
       -1, 256, 0,
       -4,   1, 259,
      0,  0,   0
    }
  },
// Day - 1080 (T=6500K)
  {
    .color_temp = 6500,
	.rgb2rgbparam = {
      395, -177,  38,
      -59,  350, -35,
      -10, -141, 407,
        0,    0,   0
    },
    .rgb2rgb2param = {
      256, -11,  11,
       -1, 255,   2,
       -4,  -3, 263,
        0,  0,   0
      	}
  },
  //end
  {
        .color_temp = 0,
  }
};
static struct rgb2rgb_index rgb_matrixes_1[] = {
    {
        .color_temp = 3000,
        .rgb2rgbparam = {
            256, 0, 0,
            0, 256, 0,
            0, 0, 256,
            0, 0, 0},
        .rgb2rgb2param = {
            256, 0, 0,
            0, 256, 0,
            0, 0, 256,
            0, 0, 0
			}
    },
    //end
    {
        .color_temp = 0,
    }
};
static struct rgb2rgb_index* rgb_matrixes[]= {
    rgb_matrixes_0, 
    rgb_matrixes_1,
};

static CSL_IpipeEdgeEnhanceConfig ee_0 = {
    .enable = 1,
    .haloReduceEnable = 1,
    .mergeMethod = 0,    
    .hpfShift = 15,
    .table = NULL,
    .edgeSharpGain = 64,
    .edgeSharpGainGradient = 96,
    .edgeSharpHpValLowThres = 500,
    .edgeSharpOffsetGradient = 24, 
    .edgeSharpHpValUpLimit = 32,
};
static CSL_IpipeEdgeEnhanceConfig ee_1 = {
    .enable = 1,
    .haloReduceEnable = 1,
    .mergeMethod = 0,    
    .hpfShift = 15,      
    .table = NULL, 
    .edgeSharpGain = 32,
    .edgeSharpGainGradient = 48,
    .edgeSharpHpValLowThres = 1000,
    .edgeSharpOffsetGradient = 24, 
    .edgeSharpHpValUpLimit = 32,
}; 
static CSL_IpipeEdgeEnhanceConfig* ee[] = {
    &ee_0,
    &ee_1,
};

static CSL_IpipeNfConfig nf2_0 = {
    .enable = 1,
    .spreadVal = 3,
    .lutAddrShift = 2,
    .greenSampleMethod = 0,
    .lscGainEnable = 0,
    .edgeDetectThresMin = 0,
    .edgeDetectThresMax = 2047,
    .lutThresTable = {15,25,35,45, 55,60,65,70},
    .lutIntensityTable = {30,28,26,22, 16,16,16,16},
};
static CSL_IpipeNfConfig nf2_1 = {
    .enable = 1,
    .spreadVal = 3,
    .lutAddrShift = 2,
    .greenSampleMethod = 0,
    .lscGainEnable = 0,
    .edgeDetectThresMin = 0,
    .edgeDetectThresMax = 2047,
    .lutThresTable = {22,22,22,22, 22,22,22,22},
    .lutIntensityTable = {20,20,20,20, 20,20,20,20},
}; 
static CSL_IpipeNfConfig* nf2[] = {
    &nf2_0,
    &nf2_1,
};


static void ISP_config(int i)
{
    DRV_ipipeSetEdgeEnhance(ee[i]);
    DRV_ipipeSetNf2(nf2[i]);
}

static int getSetIndex(int eTime, int aGain, int dGain, int cTemp)
{
    if (aGain >= 16000 && dGain >= 2048)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int TI_2A_ISP_control(int eTime, int aGain, int dGain, int cTemp)
{
    static int steaySet = -1;
    static int tempSet = -1, tempCnt = 0;
    int tempCountLimit = 10;

    int i = getSetIndex(eTime, aGain, dGain, cTemp);
    int j = 0;

    if (steaySet == i) 
    {
        j = switch_rgb2rgb_matrixes(cTemp, rgb_matrixes[i], 0);
        tempSet = -1;
        tempCnt = 0;
    }
    else if (tempSet == i && tempCnt >= tempCountLimit) 
    {
        steaySet = i; 
        tempSet = -1;
        tempCnt = 0;
        j = switch_rgb2rgb_matrixes(cTemp, rgb_matrixes[i], 1);
        ISP_config(i);
    }
    else if (tempSet == i)
    {
        tempCnt++;
    }
    else
    {
        tempSet = i;
        tempCnt = 0;
    }
    return 100 * steaySet + j;
}







