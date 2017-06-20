
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"
#include "TI_aewb_ISP.h"

struct rgb2rgb_index rgb_matrixes_0[]= {
  {
//U30 
    .color_temp = 2800,
    .rgb2rgbparam = {
      343,  -50, -37,
      -111,  323, 44,
      -24, -266, 546,
        0,    0,   0
    },
    .rgb2rgb2param = {
      252,   -8,  13,
       -2,  258,  0,
      -25,   26, 255,
        0,    0,   0
    }
  },
  {
//TL84
    .color_temp = 4000,
    .rgb2rgbparam = {
         362,  -111,   5,
         -90,   338,   8,
         -22,  -138, 416,
           0,     0,    0
    },
    .rgb2rgb2param = {
        250,    -6,   12,
         -2,   258,    0,
         -9,     3,  262,
          0,     0,    0
    }
  },
//D50
  {
    .color_temp = 5000,
    .rgb2rgbparam = {
      360, -112,   8,
      -76,  308,  25,
      -10, -121, 387,
        0,    0,   0
    },
    .rgb2rgb2param = {
      251,   -8,   12,
       -3,  256,    3,
       -4,    2,  258,
        0,    0,    0
    }
  },
//D65
  {
    .color_temp = 6500,
    .rgb2rgbparam = {
      357, -109,   7,
      -66,  282,  40,
       -5, -113, 374,
        0,    0,   0
    },
    .rgb2rgb2param = {
      256,  -10,   10,
       -3,  256,    3,
       -3,  -3,   262,
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
static struct rgb2rgb_index rgb_matrixes_2[] = {
    {
        .color_temp = 3000,
        .rgb2rgbparam = {
            77, 150, 29,
            77, 150, 29,
            77, 150, 29,
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

static CSL_IpipeEdgeEnhanceConfig ee_0 = {
    .enable = 1,
    .hpfShift = 3,
    .table = TI_YEE_TABLE,
    .lowerThres = 32,
    .hpfCoeff = {{80, -10, 0}, {-10,-10,0}, {0,0,0}},
    .edgeSharpGain = 0,
};
static CSL_IpipeEdgeEnhanceConfig ee_1 = {
    .enable = 0,
    .hpfShift = 4,
    .table = TI_YEE_TABLE,
    .lowerThres = 32,
    .hpfCoeff = {{80, -10, 0}, {-10,-10,0}, {0,0,0}},
    .edgeSharpGain = 0,
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

static void ISP_config(int i)
{
    CSL_IpipeEdgeEnhanceConfig* ee[] = { &ee_0, &ee_1, &ee_1, };
    DRV_ipipeSetEdgeEnhance(ee[i]);

    CSL_IpipeNfConfig* nf2[] = { &nf2_0, &nf2_1, &nf2_1, };
    DRV_ipipeSetNf2(nf2[i]);
}

static int getSetIndex(int eTime, int aGain, int dGain, int cTemp)
{
    if (aGain >= 128000)
    {
        return 2;
    }
    else if (aGain >= 8000)
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

    struct rgb2rgb_index* rgb_matrixes[]= {
        rgb_matrixes_0, 
        rgb_matrixes_1,
        rgb_matrixes_2,
    };
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




