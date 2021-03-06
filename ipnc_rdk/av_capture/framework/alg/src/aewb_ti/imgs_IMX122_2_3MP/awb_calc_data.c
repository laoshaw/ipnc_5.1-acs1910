#include <alg_ti_aewb_priv.h>

awb_calc_data_t awb_calc_data = {
   
// Sensor specific calibration data

   13,			// num_of_ref_1
   4,			// num_of_ref_2
   4,			// num_of_gray

   // color_temp_1[num_of_ref_1]
   {6500,6000,5500,5000,4750,4500,4250,4000,3800,3600,3400,3200,3000},

   // wbReferenceCb[num_of_ref_1][num_of_gray]
   {
       {-36,-37,-45,-35},
       {-44,-44,-50,-43},
       {-50,-50,-53,-46},
       {-58,-58,-59,-56},
       {-62,-62,-63,-57},
       {-68,-68,-68,-65},
       {-74,-74,-73,-71},
       {-77,-77,-77,-75},
       {-83,-83,-83,-83},
       {-86,-87,-87,-87},
       {-89,-90,-91,-90},
       {-91,-93,-95,-92},
       {-94,-95,-98,-92}
   },

   // wbReferenceCr[num_of_ref_1][num_of_gray]
   {
       {-62,-60,-54,-50},
       {-57,-56,-51,-48},
       {-53,-52,-48,-47},
       {-48,-48,-45,-45},
       {-42,-42,-39,-39},
       {-36,-37,-34,-34},
       {-30,-31,-28,-28},
       {-24,-25,-22,-22},
       {-19,-19,-16,-15},
       {-14,-13,-10,-8},
       {-9,-6,-3,0},
       {-4,0,3,7},
       {1,6,9,14}
   },

   // ref_gray_R_1[num_of_ref_1][num_of_gray]
   {
       {589,411,275,150},
       {596,414,269,148},
       {604,417,262,145},
       {611,420,256,143},
       {638,440,269,151},
       {666,460,282,159},
       {693,479,294,166},
       {720,499,307,174},
       {749,524,331,188},
       {778,549,355,202},
       {807,573,378,215},
       {836,598,402,229},
       {865,623,426,243}
   },

   // ref_gray_G_1[num_of_ref_1][num_of_gray]
   {
       {1088,739,471,246},
       {1059,722,451,240},
       {1030,706,430,233},
       {1001,689,410,227},
       {993,685,409,228},
       {985,682,408,228},
       {977,678,407,229},
       {969,674,406,229},
       {967,672,415,232},
       {966,671,423,235},
       {964,669,432,238},
       {963,668,440,241},
       {961,666,449,244}
   },

   // ref_gray_B_1[num_of_ref_1][num_of_gray]
   {
       {673,456,269,157},
       {615,418,247,143},
       {557,381,225,130},
       {499,343,203,116},
       {473,325,193,111},
       {446,308,184,106},
       {420,290,174,101},
       {393,272,164,96},
       {375,259,158,93},
       {358,247,152,90},
       {340,234,145,88},
       {323,222,139,85},
       {305,209,133,82}
   },

   // ref_index_2[num_of_ref_2]
   {0,3,7,12},

   // color_temp_2[num_of_ref_2]
   {6500,5000,4000,3000},

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
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,7,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,5,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,9,0,
       	0,0,0,0,
       	2,7,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,2,0,
       	0,2,2,0,
       	0,12,0,0,
       	2,0,2,7,
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,7,2,0,
       	0,9,0,0,
       	0,0,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,5,0,5,
       	19,5,0,0,
       	0,7,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,2,2,7,
       	2,0,2,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	21,88,9,2,
       	2,5,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	12,7,2,2,
       	0,5,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	16,5,0,0,
       	0,9,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,21,
       	2,0,0,0,
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,5,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,2,9,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,7,5,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
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
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,12,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,5,5,
       	0,0,2,0,
       	0,5,5,0,
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,5,0,
       	0,0,2,2,
       	0,0,5,7,
       	2,0,0,2,
       	7,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,7,0,
       	0,0,9,0,
       	0,9,9,0,
       	2,0,2,7,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,5,0,
       	2,14,0,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	2,7,89,21,
       	5,2,5,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	2,12,14,2,
       	0,0,5,7,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	2,9,9,0,
       	0,0,5,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	19,0,0,0,
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
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,7,
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,12,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
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
       	7,2,0,2,
       	7,0,0,2,
       	9,4,4,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,2,
       	0,0,0,0,
       	2,4,0,0,
       	4,0,4,0,
       	4,4,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,2,
       	7,0,0,0,
       	9,0,0,17,
       	11,2,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,7,
       	57,70,7,0,
       	7,4,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,4,4,
       	22,11,2,0,
       	2,9,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,2,17,
       	4,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,7,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,7,4,2,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,7,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	

       //3000
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,7,
       	0,0,9,2,
       	0,0,9,0,
       	9,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,2,0,0,
       	0,0,5,5,
       	0,0,9,16,
       	5,0,5,9,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,5,5,0,
       	0,0,7,37,
       	9,5,11,11,
       	2,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,9,14,85,
       	14,2,2,2,
       	9,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,18,9,7,
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
       	0,2,2,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,2,
       	7,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	11,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
       	0,0,0,0,
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
       {-36,-37,-45,-35},
       {-58,-58,-59,-56},
       {-77,-77,-77,-75},
       {-94,-95,-98,-92}
   },

   // referencesCr_2[num_of_ref_2][num_of_gray]
   {
       {-62,-60,-54,-50},
       {-48,-48,-45,-45},
       {-24,-25,-22,-22},
       {1,6,9,14}
   },

   // ref_gray_R_2[num_of_ref_2][num_of_gray]
   {
       {589,411,275,150},
       {611,420,256,143},
       {720,499,307,174},
       {865,623,426,243}
   },

   // ref_gray_G_2[num_of_ref_2][num_of_gray]
   {
       {1088,739,471,246},
       {1001,689,410,227},
       {969,674,406,229},
       {961,666,449,244}
   },

   // ref_gray_B_2[num_of_ref_2][num_of_gray]
   {
       {673,456,269,157},
       {499,343,203,116},
       {393,272,164,96},
       {305,209,133,82}
   },

   
// AWB tuning parameters

   50,     // radius

   5,     // luma_awb_min
   250,     // luma_awb_max

   3500,     // low_color_temp_thresh
   0,     // apply_rgb_adjust
   110,     // R_adjust(%)
   90,     // B_adjust(%)

   92,     // SB_1
   185,     // SB_2
   0,     // SB_low_bound

   6500,     // default_T_Hh
   5000,     // default_T_MH
   4000,     // default_T_ML
   3000,     // default_T_L

   0,		// default_T_Hh_index
   3,		// default_T_MH_index
   7,		// default_T_ML_index
   12,		// default_T_L_index

   1,     // best_gray_index_default

};