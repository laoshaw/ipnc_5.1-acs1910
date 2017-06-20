
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"
#include "TI_aewb_ISP.h"

struct rgb2rgb_index rgb_matrixes_0[]= {
  {
//U30 
    .color_temp = 2800,
    .rgb2rgbparam = {
      347,  -64, -26,
      -111,  321, 46,
      -49, -242, 547,
        0,    0,   0
    },
    .rgb2rgb2param = {
      251,   -9,  14,
       -1,  256,  0,
       -5,  -4, 265,
        0,    0,   0
    }
  },
  {
//TL84
    .color_temp = 4000,
    .rgb2rgbparam = {
         345,  -106,  17,
         -88,   332,  12,
         -29,  -153, 437,
           0,     0,    0
    },
    .rgb2rgb2param = {
        252,    -8,   12,
          1,   255,    0,
         -2,    -3,  261,
          0,     0,    0
    }
  },
//D50
  {
    .color_temp = 5000,
    .rgb2rgbparam = {
      361, -125,  20,
      -77,  303,  30,
       -9, -142, 408,
        0,    0,   0
    },
    .rgb2rgb2param = {
      255,  -13,   14,
       -2,  256,   1,
       -6,  -1,  262,
        0,    0,    0
    }
  },
//D65
  {
    .color_temp = 6500,
    .rgb2rgbparam = {
      366, -127,  17,
      -69,  284,  41,
       -3, -131, 389,
        0,    0,   0
    },
    .rgb2rgb2param = {
      256,  -11,   11,
        0,  254,    2,
       -4,  -3,   262,
        0,    0,    0
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
            0, 0, 0}
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
    .edgeSharpHpValLowThres = 300,
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
    .edgeSharpHpValLowThres = 900,
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
    .lutIntensityTable = {20,20,18,18, 16,16,16,16},
};
static CSL_IpipeNfConfig nf2_1 = {
    .enable = 1,
    .spreadVal = 3,
    .lutAddrShift = 2,
    .greenSampleMethod = 0,
    .lscGainEnable = 0,
    .edgeDetectThresMin = 0,
    .edgeDetectThresMax = 2047,
    .lutThresTable = {20,20,20,20, 20,20,20,20},
    .lutIntensityTable = {16,16,16,16, 16,16,16,16},
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
    if (aGain >= 8000)
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




