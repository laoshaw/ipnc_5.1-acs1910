
#ifndef _ALG_DMVAL_H_
#define _ALG_DMVAL_H_

#include <alg.h>

#include <DMVALINC.h>
#define MAX_CHARS_IN_FILENAME 100
typedef struct {

	DMVALhandle		*dmvalHdl;
	//DMVALcamAngle		camAngle;
	//DMVALcamLens		camLens;
	DMVALdetectMode	detectMode;
	DMVALimgType		imgType; 
	DMVALsensitivity	sensitivity;  
	DMVALsensitivity  sensitiveness;

	U08 ccTamperRatio;
	U16 targetFPS;
	U16 targetCols;
	U16 targetRows;
	U16 inRows;			// The dimensions of the input image
	U16 inCols;			// The dimensions of the input image
	U16 inStride;

	U16 outRows;		// The dimensions of the output image
	U16 outCols;		// The dimensions of the output image
	U16 outColOffset;	// horizontal offset
	U16 outRowOffset;	// vertical offset
	S32 outputDimensionsSet;
	S32 outputOffsetSet;

	S16 enableUseSeqInMem;
	S16 enableDisplay;
	S16 enableSVideo;
	S16 enablePAL;
	S16 enableRestart;
	S16 enableSaveDisplay2File;
	S16 enableDebugDisplay;
	S16 enableSaveDebugDisplay2File;
	S16 enableAppDisplay;
	S16 enableVerbose;
	S16 enableSaveLogFile;
	S16 enableIMDdebugGrid;

	S08 status[MAX_CHARS_IN_FILENAME*2];
	S08 inDirPath[MAX_CHARS_IN_FILENAME];
	S08 outDirPath[MAX_CHARS_IN_FILENAME];
	S08 inRootFilename[MAX_CHARS_IN_FILENAME];
	S08 outRootFilename[MAX_CHARS_IN_FILENAME];
	S08 indexZeros;

	S16 minPersonSize[2];
	S16 maxPersonSize[2];
	S16 minVehicleSize[2];
	S16 maxVehicleSize[2];


	S16 debug_opt;

	S16 defaultFullFrmZone;

	DMVALpolygon polygon[6];
	S08 numPolygons;

	S16 direction;		// for trip-zone and counting
	S16 orientation;

	S16 frameRate;		// specify the frame rate of the sequence
	S16 detectionRate;

	// File pointer for log files
	FILE * fpLog;
	FILE * fpTiming;

	int configFilePos;

	// String to hold log messages
	S08 logMessage[500];
	S08 timingInfo[1000];

	// Tamper specific data
	S16 resetCounter;

	// Value to indicate if Person/Vehicle are present in the scene
	int dmvaAppTZPresentAdjust;
	int dmvaAppIMDPresentAdjust;
	int dmvaAppSMETAPresentAdjust;

} ALG_DmvalCreateParams;

// Temporary structure.
// Created so that we can draw zones as overlays on the video stream
// for the IMD demo (09/29/2009)
typedef struct
{
  int numZones;
  DMVALpolygon poly[DMVAL_MAX_POLYGONS]; // DMVAL_MAX_POLYGONS picked as the max number of supported polygons
} DMVALzones;


DMVALstatus ALG_dmvalCreate(ALG_DmvalCreateParams * params);
int ALG_dmvalDelete( DMVALhandle* hndl);



#endif
