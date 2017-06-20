
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"
#include "TI_aewb_ISP.h"

struct rgb2rgb_index rgb_matrixes_0[]= {
    {
        //U30-1080 (T=3000K)
        .color_temp = 3000,
        .rgb2rgbparam = {
            339, -46, -37,
            -108, 365,  -1,
            26,-317, 547,
            0, 0, 0
        },
        .rgb2rgb2param = {
            255,   2,  -1,
            -14, 281, -11,
            -10,   7, 259,
            0, 0, 0
        }
    },
    {
        //CWF-1080  (T=4200K)
        .color_temp = 4200,
        .rgb2rgbparam = {
            453, -186, -11,
            -99,  369, -13,
            10, -192, 438,
            0,     0,   0
        },
        .rgb2rgb2param = {
            265,   -8, -1,
            -4,  262, -2,
            -2,    4, 254,
            0,     0,   0
        }
    },
    //DAY-1080 (T=5000K)
    {
        .color_temp = 5000,
        .rgb2rgbparam = {
            423, -165,  -2,
            -87,  406, -62,
            13, -216, 459,
            0,    0,   0
        },
        .rgb2rgb2param = {
            266, -16,   6,
            -9, 273,  -8,
            -7,   1, 262,
            0,  0,   0
        }
    },
    //Day T=6500K
    {
        .color_temp = 6500,
        .rgb2rgbparam = {
            424, -171,   3,
            -87,  430, -86,
            5, -170, 421,
            0,    0,   0
        },
        .rgb2rgb2param = {
            264, -10,   3,
            -4, 263,  -3,
            -3,  -2, 261,
            0,  0,   0
        },
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
    if (aGain >= 8000 && dGain >= 2048)
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







