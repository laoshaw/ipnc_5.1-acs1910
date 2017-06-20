
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"
#include "TI_aewb_ISP.h"

static struct rgb2rgb_index rgb_matrixes_0[] = {
    {
        //U30
        .color_temp = 3000,
        .rgb2rgbparam = {
            365, -62, -47,
           -115, 323,  48,
            -44,-306, 605,
            0, 0, 0},
        .rgb2rgb2param = {
            249, -4,  11,
            -3, 259,   0,
           -25,  27, 254,
            0, 0, 0}
    },
    {
        //TL84
        .color_temp = 4000,
        .rgb2rgbparam = {
            367, -119,   8,
            -94, 336,   15,
            -26, -157, 439,
            0, 0, 0},
        .rgb2rgb2param = {
            251, -6, 11,
            -1, 256, 1,
            -5, 1, 260,
            0, 0, 0}
    },
    {
        //D50
        .color_temp = 5000,
        .rgb2rgbparam = {
            378, -124,   2,
            -82,  301,  37,
            -12, -136, 404,
            0, 0, 0},
        .rgb2rgb2param = {
            250, -8,   14,
              0, 254,   2,
             -2,  -1, 259,
            0, 0, 0}
    },
    //D65
    {
        .color_temp = 6500,
        .rgb2rgbparam = {
            384, -127, -1,
            -74, 288,  42,
            -5, -118, 380,
            0, 0, 0},
        .rgb2rgb2param = {
            255, -9, 10,
            0, 251, 5,
            -4, -3, 263,
            0, 0, 0}
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
            256, 0, 0,
            0, 256, 0,
            0, 0, 256,
            0, 0, 0},
        .rgb2rgb2param = {
            77, 150, 29,
            77, 150, 29,
            77, 150, 29,
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
    .lutThresTable = {14,26,36,44, 50,56,64,70},
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
    CSL_IpipeEdgeEnhanceConfig* ee[] = {&ee_0, &ee_1, &ee_1};
    CSL_IpipeNfConfig* nf2[] = {&nf2_0, &nf2_1, &nf2_1};
    DRV_ipipeSetEdgeEnhance(ee[i]);
    DRV_ipipeSetNf2(nf2[i]);
}

static int getSetIndex(int eTime, int aGain, int dGain, int cTemp)
{
    if (aGain >= 8000 && dGain >= 8000)
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

    int i = getSetIndex(eTime, aGain, dGain, cTemp);
    int j = 0;

    struct rgb2rgb_index* rgb_matrixes[]= {rgb_matrixes_0, rgb_matrixes_1, rgb_matrixes_2};
    if (steaySet == i) 
    {
        j = switch_rgb2rgb_matrixes(cTemp, rgb_matrixes[i], 0);
        tempSet = -1;
        tempCnt = 0;
        //auto_contrast(0);
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
        //auto_contrast(1);
    }
    return 100 * steaySet + j;
}

