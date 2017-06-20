
#ifndef _ALG_AES_H_
#define _ALG_AES_H_

#include <alg.h>

typedef struct {

  /* AES Parameters */
  Int32 AES_bigEndian;    /*  AES_bigEndian... 0: Little Endian, 1: Big Endian, key is always little endian */
  Int32 AES_keySize;      /*  AES_keySize... 0: 128 bit, 1: 192 bit, 2:256 bit */
  Int32 AES_encrypt;      /*  AES_encrypt... 1: Encrypt, 0: Decrypt     */
  Int32 q_num;            /*  EDMA Queue Number     */
  Uint8 roundKey[32];     /*  ROUND KEY. 16 bytes for 128 bit, 24 bytes for 192 bit and 32 bytes for 256 bit     */

} ALG_aesParams;

typedef struct {
  Uint32  dataSize;     //dataSize in bytes as an input for processing 

  ALG_aesParams *pAesParams;  // if NULL, default values will be used
  Uint32  askIMCOPRes;
  Uint32  sysBaseAddr;

} ALG_AesCreate;

typedef struct {

  Uint32  outDataSize;

} ALG_AesStatus;

typedef struct {

  Uint8  *inAddr;               //Virt Addr
  Uint8  *outAddr;              //Virt Addr

  ALG_aesParams *pAesParams; // if NULL, previously set values are applied

} ALG_AesRunPrm;


void *ALG_aesCreate(ALG_AesCreate * create);
Int32 ALG_aesRun(void *hndl, ALG_AesRunPrm *prm, ALG_AesStatus * status);
Int32 ALG_aesDelete(void *hndl);

#endif
