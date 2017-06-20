
#ifndef _ALG_MORP_H_
#define _ALG_MORP_H_

#include <alg.h>

typedef struct {

  /* MORP Parameters */
  Uint16  MorphFunc;      /* Morphology function  */

} ALG_morpParams;

typedef struct {

  Uint16  width;
  Uint16  height;
  Uint16  offsetH;      // offsetH >= width, input and output offsetH are same
  Uint16  offsetV;      // offsetV >= height+16, , input and output offsetV are same

  ALG_morpParams *pMorpParams;  // if NULL, default values will be used
  Uint32  sysBaseAddr;

} ALG_MorpCreate;

typedef struct {

  Uint16  outWidth;

} ALG_MorpStatus;

typedef struct {

  Uint8  *inAddr;               //Virt Addr
  Uint8  *outAddr;              //Virt Addr
  Uint16  width;
  Uint16  height;
  Uint16  inStartX;
  Uint16  inStartY;

  ALG_morpParams *pMorpParams; // if NULL, previously set values are applied

} ALG_MorpRunPrm;


void   *ALG_morpCreate(ALG_MorpCreate * create);
int     ALG_morpRun(void *hndl, ALG_MorpRunPrm *prm, ALG_MorpStatus * status);
int     ALG_morpDelete(void *hndl);
void *MORP_Algcreate(
							int sysRegBase,
						    short imW,						// image width in pixels
						    short imH						// image height in pixels
					);
void MORP_AlgRun( void *algMorpHndl,
							unsigned char im1[],		// input image buffer
							unsigned char resIm1[],		// output image buffer
						    short imW,						// image width in pixels
						    short imH,						// image height in pixels
						    short MorphFunc);			// MorphFunc 					


#endif
