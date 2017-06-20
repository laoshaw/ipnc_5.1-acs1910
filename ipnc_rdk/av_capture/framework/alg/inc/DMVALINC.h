#ifndef _DMVALINC_H_
#define _DMVALINC_H_
#include "common_types.h"

#ifdef WIN32
typedef unsigned long long		TIME;

//Data Type: struct timezone
//
//    The struct timezone structure is used to hold minimal information about the local time zone. It has the following members:
//
//    int tz_minuteswest
//        This is the number of minutes west of UTC.
//    int tz_dsttime
//        If nonzero, Daylight Saving Time applies during some part of the year. 
struct timezone
{
	S32  tz_minuteswest;	/* minutes W of Greenwich Mean Time */
	S32  tz_dsttime;		/* type of dst correction */
};

struct timevalue
{
	time_t  tv_sec;         /* seconds */
	long    tv_usec;        /* and microseconds */
};
#elif defined (IPNC)
	typedef unsigned long			TIME;
#endif

typedef struct  
{
#ifdef WIN32
	struct timevalue	clock; 
#elif defined (IPNC)
	struct timeval		clock; 
#endif
	struct timezone		timeZone;
} HPTimeStamp;	// High Precision Time Stamp


#define PACKED					1
#define DMVAL_MAX_OBJECTS		64
#define DMVAL_MAX_POLY_PTS		16
#define DMVAL_MAX_IMD_ROI		4
#define DMVAL_MAX_POLYGONS		DMVAL_MAX_IMD_ROI + 1

/////////////////////////////////////////////////////////////////////////////
//	
// DMVAL Enumerated Types
//	
/////////////////////////////////////////////////////////////////////////////

typedef enum {
	DMVAL_SENSITIVITY_LOW	= 1,
	DMVAL_SENSITIVITY_MIDLO	= 2,
	DMVAL_SENSITIVITY_MID	= 3,
	DMVAL_SENSITIVITY_MIDHI	= 4,
	DMVAL_SENSITIVITY_HIGH	= 5
} DMVALsensitivity;

typedef enum
{
	DMVAL_NO_ERROR = 0,
	DMVAL_ERR_INSTANCE_CREATE_FAILED,
	DMVAL_ERR_INPUT_INVALID,
	DMVAL_ERR_INPUT_INVALID_FRAME,
	DMVAL_ERR_INPUT_INVALID_TIMESTAMP,
	DMVAL_ERR_INPUT_NEGATIVE,
	DMVAL_ERR_INPUT_EXCEEDED_RANGE,
	DMVAL_ERR_MEMORY_EXCEEDED_BOUNDARY,
	DMVAL_ERR_MEMORY_INSUFFICIENT,
	DMVAL_ERR_MEMORY_POINTER_NULL,
	DMVAL_ERR_LIBRARY_INTERNAL_FAILURE,
	DMVAL_WARNING_LOW_MEMORY,
	DMVAL_WARNING_INITIALIZING,
	DMVAL_WARNING_PARAMETER_UNDERSPECIFIED,
	DMVAL_WARNING_DISABLED
} DMVALstatus;


typedef enum
{
	DMVAL_TAMPER_NONE		= 0,
	DMVAL_TAMPER_MOVEMENT	= 8,
	DMVAL_TAMPER_BRIGHT		= 2,
	DMVAL_TAMPER_DARK		= 4,
	DMVAL_TAMPER_SCENECHANGE= 1,
	DMVAL_TAMPER_BLUR 		= 16
} DMVALtamperType;

typedef enum
{
	DMVAL_DETECTOR_NONE		= 0,
	DMVAL_DETECTOR_IMD		= 64,
	DMVAL_DETECTOR_TRIPZONE	= 128,
	DMVAL_DETECTOR_COUNTER	= 256,
	DMVAL_DETECTOR_TAMPER	= 32,
	DMVAL_DETECTOR_SMETA	= 512,
	DMVAL_DISABLED			= 1024,
	DMVAL_INITIALIZING		= 2048
} DMVALdetectorResult;

typedef enum
{
	DMVAL_IMG_LUMA						= 0,	
	DMVAL_IMG_YUV422_INTERLEAVED		= 1,
	DMVAL_IMG_YUV420_PLANARINTERLEAVED	= 2,
	DMVAL_IMG_RGB_PLANAR				= 3,
	DMVAL_IMG_RGB_INTERLEAVED			= 4,
	DMVAL_IMG_BGR_PLANAR				= 5,
	DMVAL_IMG_BGR_INTERLEAVED			= 6
} DMVALimgType;

// Specification of number of bytes associated with a data type (used in DMVALimage)
typedef enum {
	DMVAL_PIXEL_U01 = 1,
	DMVAL_PIXEL_S01,
	DMVAL_PIXEL_U08,
	DMVAL_PIXEL_S08,
	DMVAL_PIXEL_U16,
	DMVAL_PIXEL_S16,
	DMVAL_PIXEL_U32,
	DMVAL_PIXEL_S32
} DMVALdepth;

typedef enum
{
	DMVAL_DETECTMODE_DISABLE	= 0,
	DMVAL_DETECTMODE_TAMPER		= 1,
	DMVAL_DETECTMODE_IMD		= 2,
	DMVAL_DETECTMODE_SMETA		= 4,
	DMVAL_DETECTMODE_TRIP		= 8,
	DMVAL_DETECTMODE_COUNT		= 16
} DMVALdetectMode;

typedef enum
{
	DMVAL_DIRECTION_UNSPECIFIED	= 0,
	DMVAL_DIRECTION_ZONEA2B		= 1,
	DMVAL_DIRECTION_ZONEB2A		= 2 
} DMVALdirection;

typedef enum
{
	DMVAL_PARAM_SENSITIVITY,
	DMVAL_PARAM_INVIEW_PEOPLE, 
	DMVAL_PARAM_INVIEW_VEHICLES,
	DMVAL_PARAM_MINPERSONSIZE,
	DMVAL_PARAM_MAXPERSONSIZE,
	DMVAL_PARAM_MINVEHICLESIZE,
	DMVAL_PARAM_MAXVEHICLESIZE,
	DMVAL_PARAM_TZDIRECTION,
	DMVAL_PARAM_OCDIRECTION,
	DMVAL_PARAM_OCORIENTATION,
	DMVAL_PARAM_DETECTIONINTERVAL,
	DMVAL_PARAM_TAMPERCOUNTER,
	DMVAL_PARAM_TAMPERSENSITIVITY,
	DMVAL_PARAM_ALT_ROUTINE,
    DMVAL_PARAM_DBG_VERBOSE_LEVEL,
	DMVAL_PARAM_IGNOREZONE,
	DMVAL_PARAM_SMD_TRACKER,
	DMVAL_PARAM_QUAD_FOV,
	DMVAL_PARAM_QUAD_MAP
} DMVALparameter;

typedef enum
{
	DMVAL_NO_ROI	= 0,
	DMVAL_IMD		= 1,
	DMVAL_TZ_1A		= 2,
	DMVAL_TZ_1B		= 4,
	DMVAL_OC_1A		= 8,
	DMVAL_OC_1B		= 16,
	DMVAL_META		= 32
} DMVALroiType;


/////////////////////////////////////////////////////////////////////////////
//
// PUBLIC DMVAL DEFINED TYPES 
//
/////////////////////////////////////////////////////////////////////////////

// globally unique identifier is represented by a 16-byte (128-bit) number
typedef struct
{
	U32 data1;
	U16 data2;
	U16 data3;
	U64 data4;
} DMVALguid;

typedef struct
{
	S16 x;
	S16 y;
} DMVALpoint;

typedef struct
{
	F32 x;
	F32 y;
} DMVALvector2D;

typedef struct
{
	S16	ymin;
	S16	xmin;
	S16	width;
	S16	height;
	S32 boxArea;
	S32 objArea;
} DMVALbox;


typedef struct
{
	S08					numPoints;
	S08					valid;
	S32					polygonID;
	DMVALpoint			pt[DMVAL_MAX_POLY_PTS];
	DMVALbox			bBox;
	DMVALroiType		type;
	DMVALdirection		dir;	// Not used.
} DMVALpolygon;



typedef struct
{
	HPTimeStamp		timeStamp;
	DMVALimgType	type;
	DMVALdepth		pixelDepth;		// pixel depth in bits: U8, S8, U16, S16, etc.
	S16				width;			// width in pixels
	S16				height;			// height in pixels
	S16				imageStride;	// width of image in bytes
	S16				horzOffset;		// horizontal offset in pixels
	S16				vertOffset;		// Vertical offset in pixels
	S32				imageSize;		// size of image in bytes
	PTR				imageData;		// pointer to buffer containing data
} DMVALimage;



typedef struct 
{
	S32 area;
  	S32 xsum; 
  	S32 ysum;
	S08 roiID;

	DMVALdetectorResult	result;
	DMVALguid			objID;
	DMVALbox			objBox;
	DMVALpoint			objCentroid;
	DMVALvector2D		objVelocity;
	DMVALdirection		dir;
} DMVALobjMeta;


typedef struct
{
	DMVALdetectorResult modeResult;
	S16					numObjects;
	DMVALobjMeta		objMetadata[DMVAL_MAX_OBJECTS];		
} DMVALout;


typedef struct
{
	DMVALdetectMode		detectMode;
	DMVALimgType		imgType;
	DMVALsensitivity	sensitivity;
	S16					inputFrmWidth;
	S16					inputFrmHeight;		
	S16					inputFrmStride;  
	S32					bytesMemBufPermanent;
	S32					bytesMemBufOutput;
	PTR					memBufPermanent;

	S32					sysRegBase;

	// VOID POINTERS DMVA VCOP KERNELS	
	PTR					kerVCOP_1;
	PTR					kerVCOP_2;
	PTR					kerVCOP_3;
	PTR					kerVCOP_4;

	DMVALout	        * output;
} DMVALhandle; 


/////////////////////////////////////////////////////////////////////////////
//
// PUBLIC DMVAL APPLICATION PROGRAMMING INTERFACE (API)
//
///////////////////////////////////////////////////////////////////////////// 
DMVALstatus 
DMVAL_create(DMVALhandle	  * handle,
			 DMVALdetectMode	detectMode,
			 DMVALimgType		imgType,
			 DMVALsensitivity	sensitivity,
			 S16				inputFrmWidth,
             S16				inputFrmHeight,
             S16				inputFrmStride);

DMVALstatus 
DMVAL_configure(DMVALhandle	* handle,	
				PTR			  memBufPermanent,
				PTR			  memBufOutput);

DMVALstatus
DMVAL_initModule(DMVALhandle * handle);

DMVALstatus  
DMVAL_process(DMVALhandle	* handle,
			  DMVALimage	* input);

DMVALstatus 
DMVAL_delete(DMVALhandle	* handle);

DMVALstatus 
DMVAL_setROI(DMVALhandle* handle, DMVALpolygon* roi);

DMVALstatus
DMVAL_setParameter( DMVALhandle* handle,
				    DMVALparameter param,
					S16* value,
					S32 dim);

#endif
