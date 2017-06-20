
#include <avserver.h>
#include <alg_face.h>
#include <sys_env_type.h>
#include <file_msg_drv.h>

#define MAX_CHARS_IN_FILENAME1   500
#define DMVA_EVT_FILENAME_LENGTH 23 // DMVA_yyyymmddhhmmss.avi

//#define REDUCE_FRAME_RATE_TO_DMVAL
//#define INTERVAL_BETWEEN_DMVA_CALLS 4
//#define DMVA_FRAME_RATE             10    //Once in these many frames object detection happens

#define INTERVAL_BETWEEN_EVT_REC   6
#define DMVA_EVT_REC_INTERVAL      5  // in sec
#define DMVA_EVT_MIN_REC_INTERVAL  10
#define DMVA_MAX_NUM_FPS           3  // 5,10,15
#define DMVA_MAX_NUM_SENSOR_FPS    4  // 15,20,24,30
#define DMVA_MIN_SIZE_REC_KB       256 // 64

#ifdef CREATE_DMVA_DIR
  #define SD_PATH "/mnt/mmc/ipnc/DMVA"
#else
  #define SD_PATH "/mnt/mmc/ipnc"
#endif


typedef struct
{
    DMVALdetectorResult modeResult;
    int addr;

}DmvaEvt_t;

typedef struct
{
    int sensorFps;
    int dmvaFps[DMVA_MAX_NUM_FPS];
    int frameSkipMask[DMVA_MAX_NUM_FPS];
    int frameSkipMaskLen[DMVA_MAX_NUM_FPS];

}DmvaFrameSkipMask_t;

typedef struct
{
    int curDmvaFps;
    int curSkipFrameMask;
    int curSkipFrameMaskLen;
    int numFrames2Skip;
    int numSensorFps;

    DmvaFrameSkipMask_t frameSkipMask[DMVA_MAX_NUM_SENSOR_FPS];

}DmvaFpsControl_t;

typedef struct
{
    int topLeftX;
    int topLeftY;
    int bottomRightX;
    int bottomRightY;

}DmvaRoi_t;

typedef struct
{
    int numRoi;
    DmvaRoi_t dmvaRoi[4];

}DmvaRoiCtrl_t;

extern int gDmvaMinEvtRecInt;
extern char gDMVA_metaDataBuf[];
extern DmvaRoiCtrl_t gDmvaRoiCtrl;

int VIDEO_dmvaGetEvtRecStatus();
int VIDEO_dmvaEvtTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState );
int DMVA_memset(char *addr,char value,int size);
long long GetDiskfreeSpace(const char *pDisk);

