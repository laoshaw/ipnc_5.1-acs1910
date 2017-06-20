#ifndef _ALG_DEI_H_
#define _ALG_DEI_H_

#define ALG_VID_ADV_FEATURE_DEI_NAME  "dei"
#define DEI_Y_UV__GAP                 0

typedef struct
{
    Uint32 width;
    Uint32 height;
	Uint32 inFormat;
	Uint32 outFormat;
	Uint32 threshold[2];
    Uint32 sysBaseAddr;

}ALG_DeiCreate;

typedef struct
{
    Uint32 inAddr;
    Uint32 inAddrPast;
    Uint32 outAddr;

}ALG_DeiRun;

typedef struct
{
   Uint32 outWidth;
   Uint32 outHeight;
   
}ALG_DeiStatus;

Int32 ALG_deiCreate(Uint32 deiId,ALG_DeiCreate *pCreate);
Int32 ALG_deiRun(Uint32 deiId,ALG_DeiRun *pRun,ALG_DeiStatus *pStatus);
Int32 ALG_deiDelete(Uint32 deiId);

#endif
