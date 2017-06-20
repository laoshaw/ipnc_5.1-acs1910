
#ifndef _ALG_TZIMAGESYM_H_
#define _ALG_TZIMAGESYM_H_

#include <alg.h>

typedef struct {

  /* TZIMAGESYM Parameters */
  Uint16  imageDiffThreshold;    /* General value: 0 */
  Uint16  basicThreshold_8;      /* threshold for number of motion pixels */

} ALG_tzImageSymParams;

typedef struct {

  Uint16  width;
  Uint16  height;
  Uint16  offsetH;      // offsetH >= width, input and output offsetH are same
  Uint16  offsetV;      // offsetV >= height+16, , input and output offsetV are same

  ALG_tzImageSymParams *pTzImgSymParams;  // if NULL, default values will be used
  Uint32  sysBaseAddr;

} ALG_TzImgSymCreate;

typedef struct {

  Uint16  outWidth;

} ALG_TzImgSymStatus;

typedef struct {

  Uint8  *inAddr;               //Virt Addr
  Uint8  *outAddr;              //Virt Addr
  Uint16  width;
  Uint16  height;
  Uint16  stride;  
  Uint16  inStartX;
  Uint16  inStartY;

  ALG_tzImageSymParams *pTzImgSymParams; // if NULL, previously set values are applied

} ALG_TzImgSymRunPrm;


void   *ALG_tzImgSymCreate(ALG_TzImgSymCreate * create);
int     ALG_tzImgSymRun(void *hndl, ALG_TzImgSymRunPrm *prm, ALG_TzImgSymStatus * status);
int     ALG_tzImgSymDelete(void *hndl);
void *ISF_Algcreate(
					int sysRegBase,
                    short imW,						// image width in pixels
                    short imH						// image height in pixels
                    );
void ISF_AlgRun( void *algTzImgSymHndl,
                 unsigned char im1[],		// input image buffer
                 short imW,						// image width in pixels
                 short imH,						// image height in pixels
                 short imS,
                 short imageDiffThreshold,
                 short basicThreshold_8,			// threshold for number of motion pixels
                 short buffer[],					// buffer for internal use
                 short resIm1[],					// output image with detection scores
                 short* LUT_S16);					// look-up table containing 1024 elements


#endif
