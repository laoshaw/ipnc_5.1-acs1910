
#ifndef _ALG_NMS_H_
#define _ALG_NMS_H_

#include <alg.h>

typedef struct {

  /* NMS Parameters */
  Uint16  filterWidth;    /* Pitch */
  Uint16  threshold;      /* threshold  */

} ALG_nmsParams;

typedef struct {

  Uint16  width;
  Uint16  height;
  Uint16  offsetH;      // offsetH >= width, input and output offsetH are same
  Uint16  offsetV;      // offsetV >= height+16, , input and output offsetV are same

  ALG_nmsParams *pNmsParams;  // if NULL, default values will be used
  Uint32  sysBaseAddr;

} ALG_NmsCreate;

typedef struct {

  Uint16  outWidth;

} ALG_NmsStatus;

typedef struct {

  Uint8  *inAddr;               //Virt Addr
  Uint8  *outAddr;              //Virt Addr
  Uint16  width;
  Uint16  height;
  Uint16  inStartX;
  Uint16  inStartY;

  ALG_nmsParams *pNmsParams; // if NULL, previously set values are applied

} ALG_NmsRunPrm;


void   *ALG_nmsCreate(ALG_NmsCreate * create);
int     ALG_nmsRun(void *hndl, ALG_NmsRunPrm *prm, ALG_NmsStatus * status);
int     ALG_nmsDelete(void *hndl);
void *NMS_Algcreate(
							int sysRegBase,
						    short imW,						// image width in pixels
						    short imH						// image height in pixels
					);
void NMS_AlgRun( void *algNmsHndl,
							Uint8 *im1,		// input image buffer
						    short imW,						// image width in pixels
						    short imH,						// image height in pixels
							short filterWidth,
						    short threshold,			// threshold for number of motion pixels
						    short *outim);					// output image with detection scores



#endif
