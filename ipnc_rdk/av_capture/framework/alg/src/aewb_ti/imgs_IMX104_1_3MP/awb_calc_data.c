#include <alg_ti_aewb_priv.h>

awb_calc_data_t awb_calc_data = {

// Sensor specific calibration data

   12,			// num_of_ref_1
   5,			// num_of_ref_2
   4,			// num_of_gray

   // color_temp_1[num_of_ref_1]
   {6500,5750,5000,4750,4500,4250,4000,3700,3400,3100,2800,4200},

   // wbreferencesCb[num_of_ref_1][num_of_gray]
   {
       {-28,-27,-28,-24},
       {-31,-30,-32,-28},
       {-35,-34,-35,-31},
       {-40,-40,-42,-37},
       {-46,-46,-46,-43},
       {-51,-51,-51,-49},
       {-58,-57,-57,-55},
       {-63,-64,-62,-62},
       {-70,-70,-69,-68},
       {-77,-77,-76,-74},
       {-83,-83,-83,-80},
       {-62,-61,-62,-58}
   },

   // wbreferencesCr[num_of_ref_1][num_of_gray]
   {
       {-109,-109,-109,-109},
       {-104,-104,-104,-104},
       {-98,-98,-98,-98},
       {-96,-96,-94,-96},
       {-90,-90,-89,-90},
       {-85,-85,-84,-84},
       {-80,-81,-80,-81},
       {-73,-72,-73,-72},
       {-66,-66,-66,-65},
       {-59,-59,-59,-59},
       {-52,-52,-52,-52},
       {-88,-88,-87,-88}
   },

   // ref_gray_R_1[num_of_ref_1][num_of_gray]
   {
       {529,332,177,80},
       {543,341,183,82},
       {556,349,189,84},
       {598,374,202,90},
       {639,400,215,96},
       {681,425,227,101},
       {722,450,240,107},
       {758,473,253,113},
       {794,496,266,118},
       {829,519,279,124},
       {865,542,292,129},
       {590,368,200,88}
   },

   // ref_gray_G_1[num_of_ref_1][num_of_gray]
   {
       {1745,1095,584,265},
       {1661,1044,559,252},
       {1576,992,534,238},
       {1599,1005,539,241},
       {1623,1018,544,245},
       {1646,1030,549,248},
       {1669,1043,554,251},
       {1632,1020,543,245},
       {1594,997,533,239},
       {1557,973,522,232},
       {1519,950,511,226},
       {1502,941,506,224}
   },

   // ref_gray_B_1[num_of_ref_1][num_of_gray]
   {
       {1060,668,355,165},
       {985,622,333,154},
       {909,576,310,142},
       {876,554,297,137},
       {844,532,284,131},
       {811,510,271,126},
       {778,488,258,120},
       {713,447,237,110},
       {648,407,216,100},
       {583,366,195,90},
       {518,325,174,80},
       {651,410,220,101}
   },

   // ref_index_2[num_of_ref_2]
   {0,2,6,10,11},

   // color_temp_2[num_of_ref_2]
   {6500,5000,4000,2800,4200},

   // img_ref[NUM_OF_REF_2*1120]
   {
       //6500
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,2,
       	1,1,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,3,3,
       	2,2,2,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	9,0,3,2,
       	2,2,2,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,2,2,3,
       	2,2,2,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	18,0,3,3,
       	2,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	5,0,3,4,
       	2,1,0,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,4,4,3,
       	2,1,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,5,4,2,
       	1,1,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	67,18,5,2,
       	1,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	32,5,4,2,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	7,7,3,2,
       	3,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	7,0,2,2,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	19,2,2,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	14,0,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,4,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	9,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	5,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

       //5000
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	1,1,1,0,
       	0,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	2,2,1,1,
       	1,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,3,
       	3,2,2,1,
       	2,3,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	6,4,3,3,
       	3,2,2,1,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,8,2,2,
       	2,3,2,1,
       	1,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	10,8,2,1,
       	2,3,2,1,
       	0,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,2,
       	3,4,2,1,
       	0,1,2,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,5,2,
       	2,2,1,1,
       	1,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	12,50,9,4,
       	1,1,1,1,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	6,10,9,4,
       	1,1,1,3,
       	3,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	10,0,6,4,
       	2,1,2,3,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	10,4,5,3,
       	2,1,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,8,4,3,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	10,0,3,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,1,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	8,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

       //4000
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,1,
       	2,2,2,1,
       	1,1,1,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,2,
       	3,3,3,3,
       	2,1,2,2,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,3,2,
       	2,3,3,3,
       	2,1,1,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	11,4,3,2,
       	1,2,3,3,
       	2,1,1,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,3,3,
       	1,2,3,4,
       	2,1,1,1,
       	1,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,6,8,
       	5,2,2,2,
       	2,1,1,1,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	9,0,10,15,
       	10,4,1,1,
       	1,2,2,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	11,0,9,13,
       	10,4,1,1,
       	1,2,2,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,7,5,7,
       	5,3,1,0,
       	0,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,4,4,
       	3,2,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,2,2,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	9,2,1,1,
       	1,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,0,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	5,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	7,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

       //2800
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,1,
       	1,1,2,3,
       	4,3,3,2,
       	2,2,3,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	3,3,3,3,
       	3,4,5,3,
       	2,2,3,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,8,2,3,
       	5,5,5,3,
       	2,4,5,4,
       	3,3,2,2,
       	1,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,2,2,
       	5,8,9,6,
       	2,2,3,3,
       	3,3,3,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,12,3,3,
       	5,9,12,8,
       	3,1,1,1,
       	2,2,2,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,3,4,
       	4,7,9,8,
       	4,1,0,0,
       	1,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,3,
       	3,4,5,5,
       	3,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,1,1,
       	1,2,2,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	8,0,0,1,
       	1,1,0,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

       //4200
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,2,
       	3,3,2,1,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,4,2,3,
       	4,3,2,2,
       	2,2,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,4,3,3,
       	3,3,2,2,
       	2,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	11,13,3,2,
       	3,4,3,2,
       	1,1,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,4,3,
       	3,4,3,1,
       	1,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,9,7,
       	4,3,2,1,
       	1,1,1,1,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	11,0,13,10,
       	5,2,2,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	11,0,11,9,
       	4,1,2,3,
       	3,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,7,7,5,
       	2,1,1,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,4,3,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,3,3,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	9,0,2,2,
       	2,1,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,1,
       	1,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,6,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,1,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

   },

   // referencesCb_2[num_of_ref_2][num_of_gray]
   {
       {-28,-27,-28,-24},
       {-35,-34,-35,-31},
       {-58,-57,-57,-55},
       {-83,-83,-83,-80},
       {-62,-61,-62,-58}
   },

   // referencesCr_2[num_of_ref_2][num_of_gray]
   {
       {-109,-109,-109,-109},
       {-98,-98,-98,-98},
       {-80,-81,-80,-81},
       {-52,-52,-52,-52},
       {-88,-88,-87,-88}
   },

   // ref_gray_R_2[num_of_ref_2][num_of_gray]
   {
       {529,332,177,80},
       {556,349,189,84},
       {722,450,240,107},
       {865,542,292,129},
       {590,368,200,88}
   },

   // ref_gray_G_2[num_of_ref_2][num_of_gray]
   {
       {1745,1095,584,265},
       {1576,992,534,238},
       {1669,1043,554,251},
       {1519,950,511,226},
       {1502,941,506,224}
   },

   // ref_gray_B_2[num_of_ref_2][num_of_gray]
   {
       {1060,668,355,165},
       {909,576,310,142},
       {778,488,258,120},
       {518,325,174,80},
       {651,410,220,101}
   },

   
// AWB tuning parameters

   50,     // radius

   5,     // luma_awb_min
   250,     // luma_awb_max

   3500,     // low_color_temp_thresh
   0,     // apply_rgb_adjust
   110,     // R_adjust(%)
   90,     // B_adjust(%)

   65530,     // SB_1
   65531,     // SB_2
   0,     // SB_low_bound

   6500,     // default_T_Hh
   5000,     // default_T_MH
   4000,     // default_T_ML
   2800,     // default_T_L

   0,		// default_T_Hh_index
   2,		// default_T_MH_index
   6,		// default_T_ML_index
   10,		// default_T_L_index

   1,     // best_gray_index_default

};
