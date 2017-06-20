
#include "alg_aewb_priv.h"
#include "TI_aewb.h"

//#define AEWB_PRINTS

#define TI_AEW_WIN_HZ_CNT	        32	//12
#define TI_AEW_WIN_VT_CNT	        12	//16

Uint32 TI_YEE_TABLE[1024] = {
    0   ,0   ,0   ,1   ,1   ,1   ,1   ,1   ,2   ,2   ,2   ,2   ,2   ,3   ,3   ,3   ,
    3   ,3   ,4   ,4   ,4   ,4   ,4   ,5   ,5   ,5   ,5   ,5   ,6   ,6   ,6   ,6   ,
    6   ,7   ,7   ,7   ,7   ,7   ,8   ,8   ,8   ,8   ,8   ,9   ,9   ,9   ,9   ,9   ,
    10  ,10  ,10  ,10  ,10  ,11  ,11  ,11  ,11  ,11  ,12  ,12  ,12  ,12  ,12  ,13  ,
    13  ,13  ,13  ,13  ,14  ,14  ,14  ,14  ,14  ,15  ,15  ,15  ,15  ,15  ,16  ,16  ,
    16  ,16  ,16  ,17  ,17  ,17  ,17  ,17  ,18  ,18  ,18  ,18  ,18  ,19  ,19  ,19  ,
    19  ,19  ,20  ,20  ,20  ,20  ,20  ,21  ,21  ,21  ,21  ,21  ,22  ,22  ,22  ,22  ,
    22  ,23  ,23  ,23  ,23  ,23  ,24  ,24  ,24  ,24  ,24  ,25  ,25  ,25  ,25  ,25  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,26  ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,-26 ,
    -26 ,-25 ,-25 ,-25 ,-25 ,-25 ,-24 ,-24 ,-24 ,-24 ,-24 ,-23 ,-23 ,-23 ,-23 ,-23 ,
    -22 ,-22 ,-22 ,-22 ,-22 ,-21 ,-21 ,-21 ,-21 ,-21 ,-20 ,-20 ,-20 ,-20 ,-20 ,-19 ,
    -19 ,-19 ,-19 ,-19 ,-18 ,-18 ,-18 ,-18 ,-18 ,-17 ,-17 ,-17 ,-17 ,-17 ,-16 ,-16 ,
    -16 ,-16 ,-16 ,-15 ,-15 ,-15 ,-15 ,-15 ,-14 ,-14 ,-14 ,-14 ,-14 ,-13 ,-13 ,-13 ,
    -13 ,-13 ,-12 ,-12 ,-12 ,-12 ,-12 ,-11 ,-11 ,-11 ,-11 ,-11 ,-10 ,-10 ,-10 ,-10 ,
    -10 ,-9  ,-9  ,-9  ,-9  ,-9  ,-8  ,-8  ,-8  ,-8  ,-8  ,-7  ,-7  ,-7  ,-7  ,-7  ,
    -6  ,-6  ,-6  ,-6  ,-6  ,-5  ,-5  ,-5  ,-5  ,-5  ,-4  ,-4  ,-4  ,-4  ,-4  ,-3  ,
    -3  ,-3  ,-3  ,-3  ,-2  ,-2  ,-2  ,-2  ,-2  ,-1  ,-1  ,-1  ,-1  ,-1  ,0   ,0   ,
};

unsigned char TI_WEIGHTING_MATRIX[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];
unsigned char TI_WEIGHTING_SPOT[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];
unsigned char TI_WEIGHTING_CENTER[TI_AEW_WIN_HZ_CNT * TI_AEW_WIN_VT_CNT];

static void generate_windows(int width1, int height1, int h_start2, int v_start2, int width2, int height2, int weight,
          unsigned char *win_coeffs)
{
    int total_pixels_1, total_pixels_2;
    int pixel_weight1, pixel_weight2;
    int max_weight;
    int shift;
    int i, j;
    float total_weight;

    /* calculate weight for each window */
    total_pixels_2 = width2 * height2;
    total_pixels_1 = width1 * height1 - total_pixels_2;

    pixel_weight1 = (100 - weight) * 65536 / total_pixels_1;
    pixel_weight2 = weight * 65536 / total_pixels_2;

#ifdef AEWB_PRINTS
    OSA_printf("weights for each window: %d, %d\n", pixel_weight1, pixel_weight2);
#endif

    /* normalize weights */
    max_weight = pixel_weight1 > pixel_weight2 ? pixel_weight1 : pixel_weight2;
    shift = 0;
    while(max_weight >= 255){
        shift ++;
        max_weight = (max_weight + 1) >> 1;
    }
    pixel_weight1 = (pixel_weight1 + (1<<(shift-1))) >> shift;
    pixel_weight2 = (pixel_weight2 + (1<<(shift-1))) >> shift;

    if(pixel_weight2 > pixel_weight1 ){
        pixel_weight2 = (pixel_weight1 * total_pixels_1  * weight) /
                (total_pixels_2 * (100 - weight));
    } else {
        pixel_weight1 = (pixel_weight2 * total_pixels_2  * (100 - weight)) /
                (total_pixels_1 * weight);
    }
    total_weight = pixel_weight1 * total_pixels_1 + pixel_weight2 * total_pixels_2;

#ifdef AEWB_PRINTS
    OSA_printf("weight = %f, %f\n", pixel_weight1 * total_pixels_1/total_weight,
               pixel_weight2 * total_pixels_2 / total_weight);
    OSA_printf("weights for each window after shifting: %d, %d, %d\n", pixel_weight1, pixel_weight2, shift);
#endif

    for(i = 0; i < height1; i ++){
        for(j = 0; j < width1; j ++) {
            if(i >= v_start2 && i < (v_start2 + height2)
              && j >= h_start2 && j < (h_start2 + width2)){
                win_coeffs[i * width1 + j] = pixel_weight2;
            }else {
                win_coeffs[i * width1 + j] = pixel_weight1;
            }
        }
    }
}


short TI_2A_SetEEValues(int shift_val) //shift=3 for 1080P/720P, shift=4 for D1
{
  CSL_IpipeEdgeEnhanceConfig config;

  config.enable                   = TRUE;
  config.haloReduceEnable         = FALSE;
  config.mergeMethod              = CSL_IPIPE_YEE_EE_ES_MERGING_METHOD_SUMMATION;
  config.hpfShift                 = shift_val;
  config.hpfCoeff[0][0]           = 48;
  config.hpfCoeff[0][1]           = 12;
  config.hpfCoeff[0][2]           = 1024-10;
  config.hpfCoeff[1][0]           = 12;
  config.hpfCoeff[1][1]           = 0;
  config.hpfCoeff[1][2]           = 1024-6;
  config.hpfCoeff[2][0]           = 1024-10;
  config.hpfCoeff[2][1]           = 1024-6;
  config.hpfCoeff[2][2]           = 1024-2;
  config.lowerThres               = 8;//32; //changed based on Gang Comment
  config.edgeSharpGain            = 0;
  config.edgeSharpHpValLowThres   = 128;
  config.edgeSharpHpValUpLimit    = 768;
  config.edgeSharpGainGradient    = 32;
  config.edgeSharpOffsetGradient  = 0;
  config.table                    = TI_YEE_TABLE;

  DRV_ipipeSetEdgeEnhance(&config);

  return 0;
}

void TI_2A_init_tables(int width, int height)
{
#ifdef AEWB_PRINTS
    OSA_printf("TI2A weight matrix: width = %d, height = %d\n", width, height);
#endif
    generate_windows(width, height, 0, 0, width, height/2, 50, TI_WEIGHTING_MATRIX);
    generate_windows(width, height, width/4, height/4, width/2, height/2, 50, TI_WEIGHTING_CENTER);
    generate_windows(width, height, width*3/8, height/3, width/4, height/4, 80, TI_WEIGHTING_SPOT);

}

