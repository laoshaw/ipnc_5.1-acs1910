
#ifndef _ALG_RLE_H_
#define _ALG_RLE_H_

#include <alg.h>

typedef struct {

  Uint16  width;
  Uint16  height;
  Uint16  offsetH;      // offsetH >= width, input and output offsetH are same
  Uint16  offsetV;      // offsetV >= height+16, , input and output offsetV are same
  Uint32  sysBaseAddr;

} ALG_RleCreate;

typedef struct {

  Uint32  numOfTransitions;
  Uint16 *ptrEndOfRLEcontent;

} ALG_RleStatus;

typedef struct {

  Uint8  *inAddr;               //Virt Addr
  Uint8  *outAddr;              //Virt Addr
  Uint16  width;
  Uint16  height;
  Uint16  inStartX;
  Uint16  inStartY;

} ALG_RleRunPrm;


void   *ALG_rleCreate(ALG_RleCreate * create);
int     ALG_rleRun(void *hndl, ALG_RleRunPrm *prm, ALG_RleStatus * status);
int     ALG_rleDelete(void *hndl);
void *RLE_Algcreate(
							int sysRegBase,
						    short imW,						// image width in pixels
						    short imH						// image height in pixels
					);
void RLE_AlgRun( void *algRleHndl,
							Uint8 *im1,		// input image buffer
						    short imW,						// image width in pixels
						    short imH,						// image height in pixels
						    short *outim,
							Uint32 *numOfTransitions,
							Uint16 **ptrEndOfRLEcontent
							);					



#endif
