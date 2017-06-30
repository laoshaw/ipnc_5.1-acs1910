/** ===========================================================================
* @file av_server_ctrl.c
*
* @path $(IPNCPATH)\sys_server\src\
*
* @desc
* .
* Copyright (c) Appro Photoelectron Inc.  2009
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
*
* =========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <asm/types.h>
#include <file_msg_drv.h>
#include <av_server_ctrl.h>
#include <ApproDrvMsg.h>
#include <Appro_interface.h>
#include <system_control.h>
#include <rtsp_ctrl.h>
#include <system_default.h>
#include <ipnc_ver.h>

int SYSTEM(char *arg);
extern SemHandl_t gFORKSem;

#define FLAG_STREAM1	(0x01)
#define FLAG_STREAM2	(0x02)
#define FLAG_STREAM3	(0x04)

#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)

const int audio_bitrate_aac[2][3] = {{24000,36000,48000},{32000,48000,64000}};
const int audio_bitrate_g711[2][1] = {{64000},{128000}};

int CheckValueBound(int value, int min, int max)
{
	value = (value<min) ? min:value;
	value = (value>max) ? max:value;

	return value;
}
static void attach_opt(char *dst, char *src)
{
	if (strlen(dst) > 0)
		strcat(dst, " ");

	strcat(dst, src);
}

static int no_advfeature(__u8 chip_config, __u8 mode, __u8 res)
{
	if (mode == 7 && res == 1)
		return NO_ADV_FTR;
	else if (mode == 6 && (res == 4|| res == 3))
		return NO_ADV_FTR;
	else if (mode == 5 && (res == 4|| res == 3))
		return NO_ADV_FTR;
	else if (mode == 4 && (res == 4|| res == 3))
		return NO_ADV_FTR;
	else if (mode == 3 && (res == 4|| res == 3))
		return NO_ADV_FTR;
	else if (mode == 2)
		return NO_ADV_FTR;
	else if (mode == 1 && (res == 4 || res == 3 || res == 2))
		return (((chip_config == CHIP_DM365) || (chip_config == CHIP_DMVA1))?NO_ADV_FTR:NO_VS_LDC_FTR);
	else if (mode == 0 && (res == 4 || res == 3 || res == 2))
		return (((chip_config == CHIP_DM365) || (chip_config == CHIP_DMVA1))?NO_ADV_FTR:NO_VS_LDC_FTR);
	else
		return ALL_ADV_FTR;
}
static int no_binskip(__u8 mode, __u8 res)
{
	if (mode == 7 && res == 1)
		return 1;
	else if (mode == 6 && (res == 4 || res == 3))
		return 1;
	else if (mode == 5 && (res == 4 || res == 3))
		return 1;
	else if (mode == 4 && (res == 4 || res == 3))
		return 1;
	else if (mode == 3 && (res == 4 || res == 3))
		return 1;
	else if (mode == 2)
		return 1;
	else if (mode == 1 && (res == 4 || res == 3 || res == 2))
		return 1;
	else if (mode == 0 && (res == 4 || res == 3 || res == 2))
		return 1;
	else
		return 0;
}
static int no_saldre(__u8 mode, __u8 res)
{
	if (mode == 7 && res == 1)
		return 1;
	else if (mode == 6 && (res == 4 || res == 3))
		return 1;
	else if (mode == 5 && (res == 4 || res == 3))
		return 1;
	else if (mode == 4 && (res == 4 || res == 3))
		return 1;
	else if (mode == 3 && (res == 4 || res == 3))
		return 1;
	else if (mode == 2)
		return 1;
	else if (mode == 1 && (res == 4 || res == 3 || res == 2))
		return 1;
	else if (mode == 0 && (res == 4 || res == 3 || res == 2))
		return 1;
	else
		return 0;
}

/**
* @brief Start stream
*
* @param *pConfig Pointer to stream settings
* @retval 0 Success
* @retval -1 Fail.
*/
int StartStream(StreamEnv_t* pConfig)
{
	char cmd[1024] = {0};
	char cmdopt[1024] = "";
	char aud[16] = {0};
	char *audio_SR[2] = {"AUDIO 8000","AUDIO 16000"};
	char *mirror_con[4] = {"", "FLIPH", "FLIPV", "FLIPH FLIPV"};
	char *codec_tpye[3] = {"H264", "MPEG4","JPEG"};
	char *brate_con[3] = {"RCOFF", "VBR", "CBR"};
	char *mecfg_con[4] = {"AUTO", "CUSTOM", "AUTO SVC", "CUSTOM SVC"};
	char *aewb_con[4] = {"", "AE", "AWB","AEWB"};
	char *aewb_ven[3] = {"NONE2A", "APPRO2A", "TI2A"};
	char demoFlg[] 	= "DEMO";
	char adv_VS[] 	= "VS";
	char adv_LDC[] 	= "LDC";
	char adv_SNF[] 	= "SNF";
	char adv_TNF[] 	= "TNF";
	char dynrange[] = "SALDRE";
	char win_mode[] = "WIN";
	char vnf_demo[] = "VNFDEMO";
	char *img_src[] = {"NTSC","PAL", "NOFLICKER"};
	char *chip_cfg[] = {"DM365","DM368","DM368E","DM369","DMVA1","DMVA2"};
	char *sw_brc1, *sw_brc2, *sw_mir, *sw_aewb;
	char *sw_mecfg1, *sw_mecfg2, *sw_mecfg3;
	int chipConfig = CheckCpuSpeed();

#if 0
	printf("DEMOCFG Value	: %d\n", pConfig -> nDemoCfg);
	printf("Videocodecmode	: %d\n", pConfig -> nVideocodecmode);
	printf("Videocodecres 	: %d\n", pConfig -> nVideocodecres);
#endif

	printf("\n*****************************************************************\n");

	switch(chipConfig)
	{
		case CHIP_DM365:
			printf("\n  IPNC BUILD VERSION: %s	\n", DM365_APP_VERSION);
		break;
		default:
		case CHIP_DM368:
		case CHIP_DM368E:
			printf("\n  IPNC BUILD VERSION: %s	\n", DM368_APP_VERSION);
		break;
		case CHIP_DM369:
			printf("\n  IPNC BUILD VERSION: %s	\n", DM369_APP_VERSION);
		break;
		case CHIP_DMVA1:
			printf("\n  IPNC BUILD VERSION: %s	\n", DMVA1_APP_VERSION);
		break;
		case CHIP_DMVA2:
			printf("\n  IPNC BUILD VERSION: %s	\n", DMVA2_APP_VERSION);
		break;
	}

	fSetChipConfig(chipConfig);
	attach_opt(cmdopt, chip_cfg[chipConfig]);

	printf("\n*****************************************************************\n\n");

	sw_brc1 = brate_con[pConfig -> nRateControl1];
	sw_brc2 = brate_con[pConfig -> nRateControl2];

	if(pConfig -> nVideocodecmode == 2) { //set to Auto for Megapixel mode
		pConfig -> nMEConfig1 = 0; fSetMEConfig1(pConfig -> nMEConfig1);
		pConfig -> nMEConfig2 = 0; fSetMEConfig1(pConfig -> nMEConfig2);
		pConfig -> nMEConfig3 = 0; fSetMEConfig1(pConfig -> nMEConfig3);
	}

	sw_mecfg1 = mecfg_con[pConfig -> nMEConfig1];
	sw_mecfg2 = mecfg_con[pConfig -> nMEConfig2];
	sw_mecfg3 = mecfg_con[pConfig -> nMEConfig3];

	if(pConfig->nDemoCfg) {
		attach_opt(cmdopt, demoFlg);
	}

	attach_opt(cmdopt, img_src[pConfig -> imagesource]);

	if(pConfig -> nBinning == WINDOW) {
		pConfig -> vsEnable = 0; fSetVstabValue(0);
		pConfig -> ldcEnable = 0; fSetLdcValue(0);
		attach_opt(cmdopt, win_mode);
    }

#ifdef _DMVAx_
	if(pConfig->nDemoCfg) {
		// Disable DMVA
		SetDmvaEnable(0);
		attach_opt(cmdopt, demoFlg);
	}
	else{
		// Enable DMVA
		SetDmvaEnable(1);
	}
#else
	SetDmvaEnable(0);
#endif

	switch(pConfig->nDemoCfg)
	{
		case VS_DEMO:
			pConfig -> vsEnable 	= 1;
			pConfig -> ldcEnable 	= 0;
			pConfig -> vnfEnable 	= 0;
			pConfig -> nFaceDetect 	= 0;
			pConfig -> nBinning 	= BINNING;
			break;

		case LDC_DEMO:
			pConfig -> vsEnable 	= 0;
			pConfig -> ldcEnable 	= 1;
			pConfig -> vnfEnable 	= 0;
			pConfig -> nFaceDetect 	= 0;
			pConfig -> nBinning 	= BINNING;
			break;

		case VNF_DEMO:
			/* Force to Appro 2A or TI2A based on platform */
#ifdef _DM369_
			pConfig -> nAEWswitch  	= 2; fSetImageAEW(pConfig -> nAEWswitch);
#else
			pConfig -> nAEWswitch  	= 1; fSetImageAEW(pConfig -> nAEWswitch);
#endif
			/* Set skipping to increase Noise Level */
			pConfig -> nBinning 	= SKIPPING; fSetBinning(pConfig -> nBinning);
			attach_opt(cmdopt, vnf_demo);
			pConfig -> vsEnable 	= 0;
			pConfig -> ldcEnable 	= 0;
			pConfig -> vnfEnable 	= 1;
			pConfig -> nFaceDetect 	= 0;
			pConfig -> vnfMode 		= FFLAG_SNF_TNF;
			break;

		case FD_DEMO:
			pConfig -> vsEnable 	= 0;
			pConfig -> ldcEnable 	= 0;
			pConfig -> vnfEnable 	= 0;
			pConfig -> nFaceDetect 	= 1;
			pConfig -> nBitrate1   	= 4000000;
			pConfig -> nBinning 	= BINNING;
			break;

		case ROI_FD_DEMO:
			pConfig -> vsEnable 	= 0;
			pConfig -> ldcEnable 	= 0;
			pConfig -> vnfEnable 	= 0;
			pConfig -> nFaceDetect 	= 1;
			pConfig -> nBitrate1   	= 512000;
			pConfig -> nBinning 	= BINNING;
			break;

		case ROI_CENTER_DEMO:
			pConfig -> vsEnable 	= 0;
			pConfig -> ldcEnable 	= 0;
			pConfig -> vnfEnable 	= 0;
			pConfig -> nFaceDetect 	= 0;
			pConfig -> nBitrate1   	= 512000;
			pConfig -> nBinning 	= BINNING;
			break;
    }

	if((pConfig->nDemoCfg == VNF_DEMO)||(pConfig->nDemoCfg == VS_DEMO)||(pConfig->nDemoCfg == LDC_DEMO))
	{
		fSetVstabValue(pConfig -> vsEnable);	fSetLdcValue(pConfig -> ldcEnable);
		fSetVnfValue(pConfig -> vnfEnable);		fSetFaceDetect(pConfig -> nFaceDetect);
		fSetBinning(pConfig -> nBinning);
		pConfig -> nBitrate1 = 2000000; fSetMP41bitrate(pConfig -> nBitrate1);
		pConfig -> nBitrate2 = 2000000; fSetMP42bitrate(pConfig -> nBitrate2);
		pConfig->nVideocodecmode = 5; fSetVideoCodecMode(5);
		pConfig->nVideocodecres  = 1; fSetVideoCodecRes(1);
		fSetVideoCodecCombo(2);fSetVideoMode(1);
	}
	else if((pConfig->nDemoCfg == FD_DEMO)||(pConfig->nDemoCfg == ROI_FD_DEMO)||(pConfig->nDemoCfg == ROI_CENTER_DEMO))
	{
		fSetVstabValue(pConfig -> vsEnable);	fSetLdcValue(pConfig -> ldcEnable);
		fSetVnfValue(pConfig -> vnfEnable);		fSetFaceDetect(pConfig -> nFaceDetect);
		fSetBinning(pConfig -> nBinning);
		fSetMP41bitrate(pConfig -> nBitrate1);
		pConfig->nVideocodecmode = 0; fSetVideoCodecMode(0);
		pConfig->nVideocodecres  = 0; fSetVideoCodecRes(0);
		fSetVideoCodecCombo(0);fSetVideoMode(0);
	}

	{
		int advFtr = no_advfeature(chipConfig, pConfig -> nVideocodecmode, pConfig -> nVideocodecres);
		if(advFtr == NO_ADV_FTR)
		{
			pConfig -> vsEnable=0;fSetVstabValue(0);
			pConfig -> ldcEnable=0;fSetLdcValue(0);
			pConfig -> vnfEnable=0;fSetVnfValue(0);
		}
		else if(advFtr == NO_VS_LDC_FTR)
		{
			pConfig -> vsEnable=0;fSetVstabValue(0);
			pConfig -> ldcEnable=0;fSetLdcValue(0);
		}
	}

	if(pConfig -> dynRange) {
		if(no_saldre(pConfig -> nVideocodecmode, pConfig -> nVideocodecres)!=0) {
			pConfig -> dynRange = 0;
			fSetDynRange(pConfig -> dynRange);
		}
		else {
			attach_opt(cmdopt, dynrange);
			pConfig -> nAEWswitch  = 2; fSetImageAEW(pConfig -> nAEWswitch); // Force to TI2A as it works only with this
		}
	}

	if(pConfig -> audioenable) {
		attach_opt(cmdopt, audio_SR[pConfig -> audioSampleRate]);
		if(pConfig -> audiocodectype == 0)
			sprintf(aud, "G711 %d",audio_bitrate_g711[pConfig -> audioSampleRate][pConfig -> audiobitrate]);
		else if(pConfig -> audiocodectype == 1)
			sprintf(aud, "AAC %d",audio_bitrate_aac[pConfig -> audioSampleRate][pConfig -> audiobitrate]);
		attach_opt(cmdopt, aud);
	}

#ifdef _ONLY_TI2A
	if(pConfig -> nAEWswitch==1) {
		pConfig -> nAEWswitch = 2;
		fSetImageAEW(pConfig -> nAEWswitch);
	}
#endif

	attach_opt(cmdopt, aewb_ven[pConfig -> nAEWswitch]);
	if (pConfig -> nAEWtype > 0) {
		sw_aewb = aewb_con[3]; /* Always force to AEWB mode and change runtime */
		attach_opt(cmdopt, sw_aewb);
	}

	if (pConfig -> nMirror > 0) {
		sw_mir = mirror_con[(pConfig -> nMirror)];
		attach_opt(cmdopt, sw_mir);
	}

	{
		/* Enabling Advanced Features */
		if (pConfig -> vsEnable) {
			attach_opt(cmdopt, adv_VS);
		}
		if (pConfig -> ldcEnable) {
			attach_opt(cmdopt, adv_LDC);
		}
		if(pConfig -> vnfEnable) {
			attach_opt(cmdopt, adv_SNF);
			attach_opt(cmdopt, adv_TNF);
		}
	}

	if (pConfig -> encrypt > 0) {
		attach_opt(cmdopt, "ENCRYPT");
	}

	switch (pConfig -> nVideocodecmode) {
		case CODEC_COMB_TRIPLE_MPEG4:
		{	// Dual MPEG4 + MJPEG
			fSetStreamActive(1, 1, 1, 0, 0, 0);
			if (pConfig -> nVideocodecres != 0)
			{
				pConfig -> nVideocodecres = 0;
				fSetVideoCodecRes(0);
				__E("\nCODEC Resolution Error mode9\n");
			}
			sprintf(cmd, "./av_server.out %s 720P_VGA_30 MPEG4 %d %s %s MJPEG %d MPEG4 %d %s %s MENUOFF &\n",
				cmdopt,
				pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality, pConfig -> nBitrate2, sw_brc2, sw_mecfg3);
			fSetVideoSize(1, 640, 352);
			fSetVideoSize(2, 1280, 720);
			fSetVideoSize(3, 320, 192);
			fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
			fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
			fSetStreamConfig(2, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
			break;
		}
		case CODEC_COMB_TRIPLE_H264:
		{	// Dual H.264 + MJPEG
			fSetStreamActive(1, 0, 0, 0, 1, 1);
			if (pConfig -> nVideocodecres != 0)
			{
				pConfig -> nVideocodecres = 0;
				fSetVideoCodecRes(0);
				__E("\nCODEC Resolution Error mode8\n");
			}
			sprintf(cmd, "./av_server.out %s 720P_VGA_30 H264 %d %s %s MJPEG %d H264 %d %s %s MENUOFF &\n",
				cmdopt,
				pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality, pConfig -> nBitrate2, sw_brc2, sw_mecfg3);
			fSetVideoSize(1, 640, 352);
			fSetVideoSize(5, 1280, 720);
			fSetVideoSize(6, 320, 192);
			fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
			fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
			fSetStreamConfig(2, 320, 192, codec_tpye[0], H264_2_PORTNUM);
			break;
		}
		case CODEC_COMB_H264_MPEG4:
		{	// H.264 + MPEG4: Only D1+D1
			fSetStreamActive(0, 0, 1, 0, 1, 0);
			if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s 1080P_D1 H264 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 1920, 1080);
				fSetVideoSize(3, 720, 480);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);

			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode7\n");
				}
				sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 720, 480);
				fSetVideoSize(3, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_DUAL_MPEG4:
		{	// DUAL MPEG4
			fSetStreamActive(0, 1, 1, 0, 0, 0);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 1080P_D1 MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(2, 1920, 1080);
				fSetVideoSize(3, 720, 480);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(2, 1920, 1080);
				fSetVideoSize(3, 320, 192);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s D1 MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(2, 720, 480);
				fSetVideoSize(3, 288, 192);
				fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 288, 192, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1_D1 MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(2, 720, 480);
				fSetVideoSize(3, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode6\n");
				}
				sprintf(cmd, "./av_server.out %s 720P MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(2, 1280, 720);
				fSetVideoSize(3, 320, 192);
				fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_DUAL_H264:
		{	// DUAL H.264
			fSetStreamActive(0, 0, 0, 0, 1, 1);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 1080P_D1 H264 %d %s %s H264 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 1920, 1080);
				fSetVideoSize(6, 720, 480);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[0], H264_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P H264 %d %s %s H264 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 1920, 1080);
				fSetVideoSize(6, 320, 192);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[0], H264_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s D1 H264 %d %s %s H264 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 720, 480);
				fSetVideoSize(6, 288, 192);
				fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 288, 192, codec_tpye[0], H264_2_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s H264 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 720, 480);
				fSetVideoSize(6, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[0], H264_2_PORTNUM);
			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode5\n");
				}
				sprintf(cmd, "./av_server.out %s 720P H264 %d %s %s H264 %d %s %s MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nBitrate2, sw_brc2, sw_mecfg2);
				fSetVideoSize(5, 1280, 720);
				fSetVideoSize(6, 320, 192);
				fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[0], H264_2_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_MPEG4_JPEG:
		{	// MPEG4 + JPEG
			fSetStreamActive(1, 1, 0, 0, 0, 0);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 1080P_D1 MPEG4 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1920, 1080);
				fSetVideoSize(2, 720, 480);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P MPEG4 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1920, 1080);
				fSetVideoSize(2, 320, 192);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s 720P_720P_30 MPEG4 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1280, 720);
				fSetVideoSize(2, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 1280, 720, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1_D1 MPEG4 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 720, 480);
				fSetVideoSize(2, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode4\n");
				}
				sprintf(cmd, "./av_server.out %s 720P_VGA_30 MPEG4 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 640, 352);
				fSetVideoSize(2, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
				fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_H264_JPEG:
		{	// H.264 + JPEG
			fSetStreamActive(1, 0, 0, 0, 1, 0);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 1080P_D1 H264 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1920, 1080);
				fSetVideoSize(5, 720, 480);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P H264 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1920, 1080);
				fSetVideoSize(5, 320, 192);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 320, 192, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s 720P_720P_30 H264 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1280, 720);
				fSetVideoSize(5, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 1280, 720, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 720, 480);
				fSetVideoSize(5, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode3\n");
				}
				sprintf(cmd, "./av_server.out %s 720P_VGA_30 H264 %d %s %s MJPEG %d MENUOFF &\n",
					cmdopt,
					pConfig -> nBitrate1, sw_brc1, sw_mecfg1, pConfig -> nJpegQuality);
				fSetVideoSize(1, 640, 352);
				fSetVideoSize(5, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
				fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_MEGAPIXEL:
		{	//MegaPixel
			if(pConfig -> nVideocodecres&1) //if odd then JPEG
				fSetStreamActive(1, 0, 0, 0, 0, 0);
			else
				fSetStreamActive(0, 0, 0, 0, 1, 0);

			if (pConfig -> nVideocodecres == 5)
			{
				sprintf(cmd, "./av_server.out %s 5MP MJPEG %d MENUOFF &\n",
					cmdopt, pConfig -> nJpegQuality);
				fSetVideoSize(1, 2592, 1920);
				fSetStreamConfig(0, 2592, 1920, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 5MP H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 2592, 1920);
				fSetStreamConfig(0, 2592, 1920, codec_tpye[0], H264_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 3MP MJPEG %d MENUOFF &\n",
					cmdopt, pConfig -> nJpegQuality);
				fSetVideoSize(1, 2048, 1536);
				fSetStreamConfig(0, 2048, 1536, codec_tpye[2], MJPEG_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s 3MP H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 2048, 1536);
				fSetStreamConfig(0, 2048, 1536, codec_tpye[0], H264_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s 2MP MJPEG %d MENUOFF &\n",
					cmdopt, pConfig -> nJpegQuality);
				fSetVideoSize(1, 1600, 1200);
				fSetStreamConfig(0, 1600, 1200, codec_tpye[2], MJPEG_PORTNUM);
			}
			else {
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode2\n");
				}
				sprintf(cmd, "./av_server.out %s 2MP H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 1600, 1200);
				fSetStreamConfig(0, 1600, 1200, codec_tpye[0], H264_1_PORTNUM);
			}
			break;
		}
		case CODEC_COMB_SINGLE_MPEG4:
		{	// SINGLE MPEG4
			fSetStreamActive(0, 1, 0, 0, 0, 0);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 720P_MAX MPEG4 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(2, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P MPEG4 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(2, 1920, 1080);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s SXVGA MPEG4 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(2, 1280, 960);
				fSetStreamConfig(0, 1280, 960, codec_tpye[1], MPEG4_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1 MPEG4 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(2, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
			}
			else
			{
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode1\n");
				}
				sprintf(cmd, "./av_server.out %s 720P MPEG4 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(2, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
			}
			break;
		}
/////////////////////////add by pamsimochen
        case CODEC_COMB_ACS1910_H264:
            sprintf(cmd, "av_server.out %s 720P H264 %d %s %s MENUOFF &\n",
                cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1); 
            fSetVideoSize(5, 1280, 720);
            fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
            break;
////////////////////////end add
		case CODEC_COMB_SINGLE_H264:
		default :
		{	// SINGLE H.264
			if (pConfig -> nVideocodecmode != 0)
			{
				__E("\nCODEC Mode Error\n");
				pConfig -> nVideocodecmode = 0;
				fSetVideoCodecMode(0);
			}
			fSetStreamActive(0, 0, 0, 0, 1, 0);
			if (pConfig -> nVideocodecres == 4)
			{
				sprintf(cmd, "./av_server.out %s 720P_MAX H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 3)
			{
				sprintf(cmd, "./av_server.out %s 1080P H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 1920, 1080);
				fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 2)
			{
				sprintf(cmd, "./av_server.out %s SXVGA H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 1280, 960);
				fSetStreamConfig(0, 1280, 960, codec_tpye[0], H264_1_PORTNUM);
			}
			else if (pConfig -> nVideocodecres == 1)
			{
				sprintf(cmd, "./av_server.out %s D1 H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 720, 480);
				fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
			}
			else {
				if (pConfig -> nVideocodecres != 0)
				{
					pConfig -> nVideocodecres = 0;
					fSetVideoCodecRes(0);
					__E("\nCODEC Resolution Error mode0\n");
				}
				sprintf(cmd, "./av_server.out %s 720P H264 %d %s %s MENUOFF &\n",
					cmdopt, pConfig -> nBitrate1, sw_brc1, sw_mecfg1);
				fSetVideoSize(5, 1280, 720);
				fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
			}
			break;
		}
	}

	// 20090611: Remove PTZ Support
	fSetPtzSupport(0);

	printf(cmd);

	SemWait(gFORKSem);
	system(cmd);
	SemRelease(gFORKSem);

	return 0;
}

/**
* @brief Wait AV server ready
*
* @param stream_flag Stream flag to tell which stream is available.
* @retval 0 Success
* @retval -1 Fail.
*/
int WaitStreamReady(__u8 stream_flag)
{
	AV_DATA vol_data;
	int count=0, ret;
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	vol_data.serial = -1;

	if (stream_flag & FLAG_STREAM1) {
		while(count++ < BOOT_PROC_TIMEOUT_CNT){
			if(pSysInfo->lan_config.codectype1 == MJPEG_CODEC)
				ret = GetAVData(AV_OP_GET_MJPEG_SERIAL, -1, &vol_data);
			else
				ret = GetAVData(AV_OP_GET_MPEG4_SERIAL, -1, &vol_data);
			if(ret == RET_SUCCESS && vol_data.serial > 10){
				return 0;
			}
			usleep(33333);
		}
	}
	if (stream_flag & FLAG_STREAM2){
		while(count++ < BOOT_PROC_TIMEOUT_CNT){
			if(pSysInfo->lan_config.codectype2 == MJPEG_CODEC)
				ret = GetAVData(AV_OP_GET_MJPEG_SERIAL, -1, &vol_data);
			else
				ret = GetAVData(AV_OP_GET_MPEG4_CIF_SERIAL, -1, &vol_data);
			if(ret == RET_SUCCESS && vol_data.serial > 10){
				return 0;
			}
			usleep(33333);
		}
	}
	if (stream_flag & FLAG_STREAM3){
		while(count++ < BOOT_PROC_TIMEOUT_CNT){
			ret = GetAVData(AV_OP_GET_MPEG4_CIF_SERIAL, -1, &vol_data);
			if(ret == RET_SUCCESS && vol_data.serial > 10){
				return 0;
			}
			usleep(33333);
		}
	}
	return -1;
}

/**
* @brief Initial AV server
*
* @param *pConfig AV server comunicate setting
* @retval 0 Success
* @retval -1 Fail.
*/
int InitAV_Server(StreamEnv_t* pConfig)
{
	__u8 stream_flag = 0, nROICfg = ROI_NONE;
	if(ApproDrvInit(SYS_MSG_TYPE)){
		__E("ApproDrvInit fail\n");
		return -1;
	}

	while(ApproPollingMsg() < 0){
		usleep(100000);
	}
	if(func_get_mem(NULL)){
		ApproDrvExit();
		__E("CMEM map fail\n");
		return -1;
	}

	SetVideoCodecType(pConfig -> nVideocodecmode, pConfig->nVideocodecres);

	if(pConfig -> nVideocodecmode >= CODEC_COMB_TRIPLE_H264) {
		stream_flag = (FLAG_STREAM1|FLAG_STREAM2|FLAG_STREAM3);
	}
	else if(pConfig -> nVideocodecmode >= CODEC_COMB_H264_JPEG) {
		stream_flag = (FLAG_STREAM1|FLAG_STREAM2);
	}
	else {
		stream_flag = FLAG_STREAM1;
	}

	if(WaitStreamReady(stream_flag) != 0) {
		ApproDrvExit();
		__E("WaitStreamReady Fail.\n");
		return -1;
	}

	switch(pConfig->nDemoCfg)
	{
		case ROI_FD_DEMO:
			nROICfg = ROI_FD_CFG;
			break;

		case ROI_CENTER_DEMO:
			nROICfg  = ROI_CENTER_CFG;
			break;
		default:
			nROICfg = ROI_NONE;
			break;
    }

	SetDisplayValue(pConfig -> nDisplay);
	SetImage2AType(pConfig -> nAEWtype);
	Set2APriority(pConfig -> expPriority);
	SetTvSystem(pConfig -> imagesource);
	SetCamDayNight(pConfig -> nDayNight);
	SetWhiteBalance(pConfig -> nWhiteBalance);
	SetBacklight(pConfig -> nBackLight);
	SetBrightness(pConfig -> nBrightness);
	SetContrast(pConfig -> nContrast);
	SetSaturation(pConfig -> nSaturation);
	SetSharpness(pConfig -> nSharpness);
	SetHistEnable(pConfig -> histogram);

	if(no_binskip(pConfig->nVideocodecmode, pConfig->nVideocodecres) == 0) {
		SetBinningSkip(pConfig -> nBinning);
	}
	else {
		//fSetBinning(SKIPPING);
		SetBinningSkip(SKIPPING);
	}

	SetROIConfig(nROICfg);
	SetFaceDetectPrm();
	/* Set Motion Parameters */
	SetMotionDetectParam();
	SetTimeDateDetail();
	SetOSDEnable(stream_flag);

	if((stream_flag & FLAG_STREAM1) > 0) {
		SetFramerate1(pConfig -> nFrameRate1);
		SetOSDDetail(0);
		SetCodecAdvParam(0);
		SetCodecROIParam(0);
	}
	if((stream_flag & FLAG_STREAM2) > 0) {
		SetFramerate2(pConfig -> nFrameRate2);
		SetOSDDetail(1);
		SetCodecAdvParam(1);
		SetCodecROIParam(1);
	}
	if((stream_flag & FLAG_STREAM3) > 0) {
		SetFramerate3(pConfig -> nFrameRate3);
		SetOSDDetail(2);
		SetCodecAdvParam(2);
		SetCodecROIParam(2);
	}

	SetDynRangeParam();
	SetVnfParam();

	SetAudioAlarmValue(pConfig->audioalarmlevel);
    SetAudioAlarmFlag(pConfig->audioalarm);
	SetAudioInVolume(pConfig->audioinvolume);
	SetAudioOutVolume(pConfig->audiooutvolume);
	CheckAudioParam();
	/* Don't start wis-streamer here, we'll do it later. */

	return 0;
}

/**
* @brief Set Binning
*
* @param value 0:binning ; 1:skipping
* @return function to set Binning
*/
int SetBinning(unsigned char value) // 0:binning / 1:skipping
{
	int ret = 0;
	unsigned char prev;
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.nBinning!=value) {
		prev = pSysInfo->lan_config.nBinning;
		if((ret = fSetBinning(value)) == 0)
		{
			if(no_binskip(pSysInfo->lan_config.nVideocodecmode, pSysInfo->lan_config.nVideocodecres) == 0) {
				if (prev == 2 || value == 2) {
					SetRestart();
					return ret;
				}
				else {
					SetBinningSkip(value);
				}
			}
			else {
				//fSetBinning(SKIPPING);
				SetBinningSkip(SKIPPING);
			}

		}
	}

	return ret;
}
/**
* @brief Set day or night
*
* @param value value of day or night
* @return function to set day or night
*/
int SetCamDayNight(unsigned char value)
{
	int i, j;
	unsigned int frameRateVal1, frameRateVal2, frameRateVal3;
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	if (pSysInfo->lan_config.nDayNight == value)
		return 0;

	i = pSysInfo->lan_config.nVideocodecmode;
	j = pSysInfo->lan_config.nVideocodecres;

	if(fSetCamDayNight(value)==0) {
		SetDayNight(value);
		if (value == 0) {
			frameRateVal1 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_night_1_dm365[i][j][0]:enc_framerate_night_1_dm368[i][j][0];
			frameRateVal2 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_night_2_dm365[i][j][0]:enc_framerate_night_2_dm368[i][j][0];
			frameRateVal3 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_night_3_dm365[i][j][0]:enc_framerate_night_3_dm368[i][j][0];
		}
		else {
			frameRateVal1 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_day_1_dm365[i][j][0]:enc_framerate_day_1_dm368[i][j][0];
			frameRateVal2 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_day_2_dm365[i][j][0]:enc_framerate_day_2_dm368[i][j][0];
			frameRateVal3 = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			                enc_framerate_day_3_dm365[i][j][0]:enc_framerate_day_3_dm368[i][j][0];
		}
		fSetFramerate1(0);fSetFramerate2(0);fSetFramerate3(0);
		fSetFramerateVal1(frameRateVal1);fSetFramerateVal2(frameRateVal2);fSetFramerateVal3(frameRateVal3);
		SetStreamFramerate(0, frameRateVal1);SetStreamFramerate(1, frameRateVal2);SetStreamFramerate(2, frameRateVal3);
	}
	else
		return -1;

	return 0;
}

/**
* @brief Set MPEG4-1 frame rate
*
* @param "unsigned char value": selected frame rate
* @return function to set MPEG4-1 frame rate
* @retval 0: success
* @retval -1: failed to set MPEG4-1 frame rate
*/
int SetFramerate1(unsigned char value)
{
	int i = 0, j = 0, ret = 0;
	unsigned int frameRateVal;
	SysInfo* pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	i = pSysInfo->lan_config.nVideocodecmode;
	j = pSysInfo->lan_config.nVideocodecres;

	ret = fSetFramerate1(value);

	if(ret == 0) {
		if (pSysInfo->lan_config.nDayNight == 0)
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_night_1_dm365[i][j][value]:enc_framerate_night_1_dm368[i][j][value];
		else
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_day_1_dm365[i][j][value]:enc_framerate_day_1_dm368[i][j][value];

		ret = fSetFramerateVal1(frameRateVal);
		if(!IsRestart())
			SetStreamFramerate(0, frameRateVal);
	}

	return ret;
}
int SetFramerate2(unsigned char value)
{
	int i = 0, j = 0, ret = 0;
	unsigned int frameRateVal;
	SysInfo* pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	i = pSysInfo->lan_config.nVideocodecmode;
	j = pSysInfo->lan_config.nVideocodecres;

	ret = fSetFramerate2(value);

	if(ret == 0) {
		if (pSysInfo->lan_config.nDayNight == 0)
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_night_2_dm365[i][j][value]:enc_framerate_night_2_dm368[i][j][value];
		else
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_day_2_dm365[i][j][value]:enc_framerate_day_2_dm368[i][j][value];

		ret = fSetFramerateVal2(frameRateVal);
		if(!IsRestart())
			SetStreamFramerate(1, frameRateVal);
	}

	return ret;
}
int SetFramerate3(unsigned char value)
{
	int i = 0, j = 0, ret = 0;
	unsigned int frameRateVal;
	SysInfo* pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	i = pSysInfo->lan_config.nVideocodecmode;
	j = pSysInfo->lan_config.nVideocodecres;

	ret = fSetFramerate3(value);

	if(ret == 0) {
		if (pSysInfo->lan_config.nDayNight == 0)
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_night_3_dm365[i][j][value]:enc_framerate_night_3_dm368[i][j][value];
		else
			frameRateVal = ((pSysInfo->lan_config.chipConfig == CHIP_DM365) || (pSysInfo->lan_config.chipConfig == CHIP_DMVA1)) ?
			               enc_framerate_day_3_dm365[i][j][value]:enc_framerate_day_3_dm368[i][j][value];

		ret = fSetFramerateVal3(frameRateVal);
		if(!IsRestart())
			SetStreamFramerate(2, frameRateVal);
	}

	return ret;
}

/**
* @brief Set white balance
*
* @param value value of white balance
* @return function to set white balance
*/
int SetCamWhiteBalance(unsigned char value)
{
	SetWhiteBalance(value);
	return fSetCamWhiteBalance(value);
}

/**
* @brief Set backlight value
*
* @param value backlight value
* @return function to set backlight value
*/
int SetCamBacklight(unsigned char value)
{
	SetBacklight(value);
	return fSetCamBacklight(value);
}

/**
* @brief Set brightness
*
* @param value brightness value
* @return function to set brightness value
*/
int SetCamBrightness(unsigned char value)
{
	SetBrightness(value);
	return fSetCamBrightness(value);
}

/**
* @brief Set contrast
*
* @param value contrast value
* @return function to set contrast value
*/
int SetCamContrast(unsigned char value)
{
	SetContrast(value);
	return fSetCamContrast(value);
}

/**
* @brief Set saturation
*
* @param value saturation value
* @return function to set saturation value
*/
int SetCamSaturation(unsigned char value)
{
	SetSaturation(value);
	return fSetCamSaturation(value);
}

/**
* @brief Set ptz function
*
* @param value means ptz function; ex:zoomin, zoomout and so on
* @return function to set ptz value
*/
int SetCamPtz(int value)
{
	SetPtz(value);
	return 0;
}

/**
* @brief Set sharpness
*
* @param value sharpness value
* @return function to set sharpness value
*/
int SetCamSharpness(unsigned char value)
{
	SetSharpness(value);
	return fSetCamSharpness(value);
}

/**
* @brief Clear System Log
*
* @param value sharpness value
* @return function to set sharpness value
*/
int SetClearSysLog(unsigned char value)
{
	return fSetClearSysLog(value);
}

/**
* @brief Clear Access Log
*
* @param value sharpness value
* @return function to set sharpness value
*/
int SetClearAccessLog(unsigned char value)
{
	return fSetClearAccessLog(value);
}

/**
* @brief Set audio status
*
* @param value audio status.
*
* @return function to set audio status
*/

int audiocount=0, audiochange=0;
#define AUDIO_MAX_VARIABLE (2)

void CheckAudioParam(void)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	if(pSysInfo->audio_config.audioON) {
		if (pSysInfo->audio_config.audiomode==1) {
			fSetAudioEnable(0);
			fSetAudioAlarm(0);
		}
		else {
			fSetAudioEnable(1);
		}
		if(pSysInfo->audio_config.audiomode==0) {
			fSetAlarmAudioPlay(0);
		}
	}
	else {
		fSetAudioEnable(0);
		fSetAlarmAudioPlay(0);
		fSetAudioAlarm(0);
	}
}

void SetAudioParams(void)
{
	int audioMode, prevAudioMode;
	int nExtFunc = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	audiocount = 0;
	if(audiochange==0)
		return;
	else
		audiochange = 0;

	audioMode = pSysInfo->audio_config.audiomode;
	prevAudioMode = pSysInfo->audio_config.audioenable;

	CheckAudioParam();

	if(prevAudioMode != pSysInfo->audio_config.audioenable) {

		// If audio is disabled and DMVA audio recording is ON
		// then disable the DMVA audio recording
		if((pSysInfo->audio_config.audioenable == 0) &&
		   (pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys & 0x4))
		{
		    pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys = (pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys & 0xB);
		}

		SetRestart();
	}
	else {
		SetRTSPStreamingOFF();
		nExtFunc = GetRTSPStreamOption();
		SetRTSPStreamingON(nExtFunc);
	}

	return;
}

int SetAudioON(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	int ret = 0;

	audiocount++;
	if(value != pSysInfo->audio_config.audioON) {
		ret = fSetAudioON(value);
		audiochange = 1;
	}

	if(audiocount == AUDIO_MAX_VARIABLE)
		SetAudioParams();

	return ret;
}

int SetAudioMode(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	int ret = 0;

	audiocount++;
	if(value != pSysInfo->audio_config.audiomode) {
		ret = fSetAudioMode(value);
		audiochange = 1;
	}

	if(audiocount == AUDIO_MAX_VARIABLE)
		SetAudioParams();

	return ret;
}

int SetAudioInVolume(unsigned char value)
{

	if (value < 10)
		SYSTEM("amixer set 'PGA' 0");
	else if ( value < 40)
		SYSTEM("amixer set 'PGA' 1");
	else if ( value < 80)
		SYSTEM("amixer set 'PGA' 2");
	else
		SYSTEM("amixer set 'PGA' 3");

	return fSetAudioInVolume(value);
}

int SetAudioEncode(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->audio_config.codectype) {
		ret = fSetAudioEncode(value);
		if(ret == 0)
			SetRestart();
	}

	return ret;
}

int SetAudioSampleRate(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->audio_config.samplerate) {
		ret = fSetAudioSampleRate(value);
		if(ret == 0)
			SetRestart();
	}

	return ret;
}

int SetAudioBitrate(unsigned char value)
{
	int ret = 0;
	int bitrate;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->audio_config.codectype == 1) { //if AAC LC
		bitrate = audio_bitrate_aac[pSysInfo->audio_config.samplerate][value];
		SetAACBitrate(bitrate);
	}
	else {
		bitrate = audio_bitrate_g711[pSysInfo->audio_config.samplerate][value];
	}

	fSetAudioBitrateValue(bitrate);

	ret = fSetAudioBitrate(value);

	return ret;
}

int SetAudioAlarmLevel(unsigned char value)
{
	value = CheckValueBound(value, 1, 100);
 	SetAudioAlarmValue(value);
	return fSetAudioAlarmLevel(value);
}

int SetAudioOutVolume(unsigned char value)
{
	char cmd[64];
	int setval = (value*63)/100;
	sprintf(cmd, "amixer set 'Mono DAC' %d", setval);

	SemWait(gFORKSem);
	system(cmd);
	SemRelease(gFORKSem);

	return fSetAudioOutVolume(value);
}

int fdetectcount=0;
#define FD_MAX_VARIABLE (12)

void SetFaceDetectPrm(void)
{
	FaceDetectParam faceParam;
    SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return;

	fdetectcount = 0;

	faceParam.fdetect 		= pSysInfo->face_config.fdetect;
	faceParam.startX 		= pSysInfo->face_config.startX;
	faceParam.startY 		= pSysInfo->face_config.startY;
	faceParam.width 		= pSysInfo->face_config.width;
	faceParam.height 		= pSysInfo->face_config.height;
	faceParam.fdconflevel 	= pSysInfo->face_config.fdconflevel;
	faceParam.fddirection 	= pSysInfo->face_config.fddirection;
	faceParam.frecog 		= pSysInfo->face_config.frecog;
	faceParam.frconflevel 	= pSysInfo->face_config.frconflevel;
	faceParam.frdatabase 	= pSysInfo->face_config.frdatabase;
	faceParam.pmask 		= pSysInfo->face_config.pmask;
	faceParam.maskoption 	= pSysInfo->face_config.maskoption;

	//printf("FD DEBUG VALUE: %d %d %d %d %d\n" , faceParam.fdetect,
	//	faceParam.startX, faceParam.startY, faceParam.width, faceParam.height);

	SendFaceDetectMsg(&faceParam);
}

int SetFaceDetect(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFaceDetect(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();

	return ret;
}

int SetFDX(unsigned int value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDX(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFDY(unsigned int value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDY(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFDW(unsigned int value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDW(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFDH(unsigned int value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDH(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFDConfLevel(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDConfLevel(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFDDirection(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFDDirection(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;

}

int SetFRecognition(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFRecognition(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFRConfLevel(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFRConfLevel(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetFRDatabase(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetFRDatabase(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}
int SetPrivacyMask(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetPrivacyMask(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetMaskOptions(unsigned char value)
{
	int ret = 0;
	fdetectcount++;
	ret = fSetMaskOptions(value);
	if(fdetectcount == FD_MAX_VARIABLE)
		SetFaceDetectPrm();
	return ret;
}

int SetVstabValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    if(value != pSysInfo->advanceMode.vsEnable){
        if ((ret = fSetVstabValue(value)) == 0) {
            SetRestart();
        }
    }

    return ret;
}

int SetLdcValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    if(value != pSysInfo->advanceMode.ldcEnable){
        if ((ret = fSetLdcValue(value)) == 0) {
            SetRestart();
        }
    }

    return ret;
}

#define VNF_ALL_VALUES (3)
unsigned int vnfCount=0;

int SetVnfParam(void)
{
	VnfParam vnfParam;

    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    vnfCount=0;

	vnfParam.enable     = pSysInfo->advanceMode.vnfEnable;
	vnfParam.mode       = pSysInfo->advanceMode.vnfMode;
	vnfParam.strength   = pSysInfo->advanceMode.vnfStrength;

	SetVNFParamMsg(0, &vnfParam);
	if(pSysInfo->lan_config.nVideocodecmode > 7)
		SetVNFParamMsg(2, &vnfParam);

	vnfParam.enable     = (pSysInfo->lan_config.democfg==VNF_DEMO) ? 0:pSysInfo->advanceMode.vnfEnable;
	if(pSysInfo->lan_config.nVideocodecmode > 2)
		SetVNFParamMsg(1, &vnfParam);

    return 0;
}

int SetVnfValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    vnfCount ++;

    if(value != pSysInfo->advanceMode.vnfEnable){
        if ((ret = fSetVnfValue(value)) == 0) {
            SetRestart();
        }
    }

    if(vnfCount==VNF_ALL_VALUES)
        SetVnfParam();

    return ret;
}

int SetVnfMode(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    vnfCount ++;

    if(value != pSysInfo->advanceMode.vnfMode){
        ret = fSetVnfMode(value);
    }

    if(vnfCount==VNF_ALL_VALUES)
        SetVnfParam();

    return ret;
}

int SetVnfStrength(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

    vnfCount ++;

    if(value != pSysInfo->advanceMode.vnfStrength){
    	ret = fSetVnfStrength(value);
	}

    if(vnfCount==VNF_ALL_VALUES)
        SetVnfParam();

    return ret;
}

void ResetBasicDefault(void)
{
	fSetVstabValue(0); fSetLdcValue(0); fSetVnfValue(0); fSetFaceDetect(0);
	fSetImageAEW(AEW_SWITCH_DEFAULT);
	fSetBinning(BINNING);
	fSetMP41bitrate(MPEG41BITRATE_DEFAULT);
	fSetMP42bitrate(MPEG42BITRATE_DEFAULT);
	fSetVideoCodecMode(0); fSetVideoCodecRes(0);
	fSetVideoCodecCombo(0); fSetVideoMode(0);
}

int SetDemoCfg(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.democfg) {
		if ((ret = fSetDemoCfg(value)) == 0) {
			if(value==DEMO_NONE) { /* reset to basic default modes */
				ResetBasicDefault();
			}
			SetRestart();
		}
	}

	return ret;
}

int SetClickSnapFilename(void * buf, int length)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

    if((ret = fSetClickSnapFilename(buf, length))==0) {
		if(pSysInfo->sdcard_config.sdinsert==3) {
    		SetClipSnapName((char*)buf, length);
		}
	}

	return ret;
}

int SetClickSnapStorage(unsigned char value)
{
	int ret =  fSetClickSnapStorage(value);

    if(ret==0)
    	SetClipSnapLocation(value);

	return ret;
}

int OSDChangeVal = 0;

void SendOSDChangeMsg(void)
{
	OSDChangeVal++;
	if(OSDChangeVal==OSD_PARAMS_UPDATED) {
		SetOSDWindow(OSD_MSG_TRUE);
		OSDChangeVal = 0;
	}

	return;
}

int SetOSDTextInfo(void * buf, int length)
{
	int ret;
	ret = fSetOSDText(buf, length);
	if(ret == 0)
		SetAvOsdText((char*)buf, length);

	return ret;
}

int SetOSDWin(unsigned char value)
{
	int ret = 0;
	int ivalue = value;
	fprintf(stderr,"\nSetOSDWin: %d\n", value);
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.osdwin) {
		ret = fSetOSDWin(value);
		SetAvOsdTextEnable(ivalue);
	}

	return 0;
}

int SetHistogram(unsigned char value)
{
	int ret = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.histogram) {
		ret = fSetHistogram(value);
		SetHistEnable(value);
	}

	return 0;
}
#define DYNRANGE_ALL_VALUES (2)
unsigned int dynRangeCount=0;

int SetDynRangeParam(void)
{
    SysInfo *pSysInfo = GetSysInfo();
    if(pSysInfo == NULL)
        return -1;

	DynRangePrm dynRangePrm;

    dynRangeCount=0;

	dynRangePrm.enable 	= (pSysInfo->advanceMode.dynRange>0) ? 1:0;
	dynRangePrm.mode 	= pSysInfo->advanceMode.dynRange;
	dynRangePrm.level 	= pSysInfo->advanceMode.dynRangeStrength;

	SetDynRangePrmMsg(&dynRangePrm);

	return 0;
}

int SetDynRangeStr(unsigned char value)
{
    int ret=0;

    dynRangeCount++;

    ret = fSetDynRangeStr(value);

    if(dynRangeCount==DYNRANGE_ALL_VALUES)
        SetDynRangeParam();

    return ret;
}

int SetDynRange(unsigned char value)
{
	int ret = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

    dynRangeCount++;

	int prev_val = pSysInfo->advanceMode.dynRange;

	if(no_saldre(pSysInfo->lan_config.nVideocodecmode, pSysInfo->lan_config.nVideocodecres) != 0)
		value = 0;

	if(value != prev_val) {
		if ((ret = fSetDynRange(value)) == 0) {
			if((value==0)||(prev_val==0))
				SetRestart();
		}
	}

    if(dynRangeCount==DYNRANGE_ALL_VALUES)
        SetDynRangeParam();

	return 0;
}

int SetExpPriority(unsigned char value)
{
	int ret = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.expPriority) {
		ret = fSetExpPriority(value);
		Set2APriority(value);
	}

	return 0;
}

int SetOSDWinNum(unsigned char value)
{
#if 0
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.osdwinnum) {
		ret = fSetOSDWinNum(value);
	}

	if (ret == 0)
		SendOSDChangeMsg();

	return ret;
#else
	int ret = 0;
	int ivalue = value;
	fprintf(stderr,"\nSetOSDWinNum: %d\n", value);
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.osdwinnum) {
		ret = fSetOSDWinNum(value);
		SetAvOsdLogoEnable(ivalue);
	}
	return ret;
#endif
}

int SetOSDStream(unsigned char value)
{
#if 0
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.osdstream) {
		ret = fSetOSDStream(value);
	}

	if (ret == 0)
		SendOSDChangeMsg();

	return ret;
#else
	fprintf(stderr,"SetOSDWin: %d", value);
	return 0;
#endif
}

/**
* @brief Set MPEG4-1 bitrate
*
* @param value MPEG4-1 bitrate value
* @return function to set MPEG4-1 bitrate
* @retval -1 failed to set MPEG4-1 bitrate
*/

int SetMP41bitrate(unsigned int value)
{
	SysInfo *pSysInfo = GetSysInfo();
	unsigned char nVideocodecmode, nVideocodecres;
	if(pSysInfo == NULL)
		return -1;

	nVideocodecmode= pSysInfo->lan_config.nVideocodecmode;
	nVideocodecres = pSysInfo->lan_config.nVideocodecres;

#if 0
	if((nVideocodecres == 3 && (nVideocodecmode == 6 || nVideocodecmode == 5))||(nVideocodecmode==2)){
		value = CheckValueBound(value, 64000, 8000000);
	}
	else {
		value = CheckValueBound(value, 64000, 12000000);
	}
#else
	value = CheckValueBound(value, 64000, 12000000);
#endif

	if(value != pSysInfo->lan_config.nMpeg41bitrate) {
		SetEncBitrate(0, value);
		return fSetMP41bitrate(value);
	}
	else
		return 0;
}

/**
* @brief Set MPEG4-2 bitrate
*
* @param value MPEG4-2 bitrate value
* @return function to set MPEG4-2 bitrate
* @retval -1 failed to set MPEG4-2 bitrate
*/
int SetMP42bitrate(unsigned int value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	value = CheckValueBound(value, 64000, 8000000);

	if(value != pSysInfo->lan_config.nMpeg42bitrate) {
		SetEncBitrate(1, value);
		return fSetMP42bitrate(value);
	}
	else
		return 0;
}

/**
* @brief Set motion JPEG quality
*
* @param value : QP value
* @return function to set motion JPEG quality
*/
int SetMJPEGQuality(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	value = (unsigned char)CheckValueBound((int)value, 2, 97);

	if(value != pSysInfo->lan_config.njpegquality) {
		SetJpgQuality(value);
		return fSetJpegQuality(value);
	}
	else
		return 0;
}

int SetMirror(unsigned char value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.mirror) {
#if 0
    	int ret = 0;
		if ((ret = fSetMirror(value)) == 0)
			SetRestart();
		return ret;
#else
		SetMirr(value);
		return fSetMirror(value);
#endif

	}
	return 0;

}

int SetEncryption(unsigned char value)
{
    int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

#ifndef ENCRYPT_ON
	value = 0;
#endif

	if(value != pSysInfo->lan_config.encryptEnable) {
		if ((ret = fSetEncryption(value)) == 0)
			SetRestart();
	}

	return ret;
}


/**
* @brief Set image AEW
*
* @param "unsigned char value": 0: OFF, 1: APPRO
* @return SUCCESS: 0, FAIL: -1
*/
int SetImageAEW(unsigned char value)/*img2a*/
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	if (pSysInfo->lan_config.nAEWswitch != value) {
		if ((ret = fSetImageAEW(value)) == 0) {
				SetRestart();
		}
	}

	return ret;
}

/**
* @brief Set image AEW
*
* @param "unsigned char value": 0: OFF, 1: APPRO
* @return SUCCESS: 0, FAIL: -1
*/
int SetImageAEWType(unsigned char value)/*img2a*/
{
	int ret = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	if (pSysInfo->lan_config.nAEWtype != value) {
		if ((ret = fSetImageAEWType(value)) == 0) {
			SetImage2AType(value);
		}
	}

	return ret;
}

/**
* @brief Set image source
*
* @param value value of image source( NTSC/PAL Select/)
* @return function to set image source
*/
/* NTSC/PAL Select */
int SetImagesource(unsigned char value)
{
	SetTvSystem(value);
	return fSetImageSource(value);
}
/**
* @brief Set time stamp status
* @param value Time stamp status
*/
int SetTStampEnable(unsigned char value)
{
	 if(value ==0){
	 SetTStamp(value);
	 return fSetTStampEnable(value);
	 }
	 return fSetTStampEnable(value);
}

int SetRateControl1(unsigned char value)
{
    int ret =0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if (value != pSysInfo->lan_config.nRateControl1) {
		if ((ret = fSetRateControl1(value)) == 0) {
			SetRestart();
		}
	}

	return ret;
}
int SetRateControl2(unsigned char value)
{
    int ret =0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if (value != pSysInfo->lan_config.nRateControl2) {
		if ((ret = fSetRateControl2(value)) == 0) {
			SetRestart();
		}
	}

	return ret;
}

/**
* @brief Set time stamp format
* @param value format
*/
int timedatecount=0;
#define TIMEDATE_MAX_VARIABLE (4)

int SetTimeDateDetail(void)
{
	DateTimePrm datetimeParam;
    SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	timedatecount = 0;

	datetimeParam.dateFormat = pSysInfo->lan_config.dateformat;
	datetimeParam.datePos = pSysInfo->lan_config.dateposition;
	datetimeParam.timeFormat = pSysInfo->lan_config.tstampformat;
	datetimeParam.timePos = pSysInfo->lan_config.timeposition;

	return SetDateTimeFormat(&datetimeParam);
}

int SetDateFormat(unsigned char value)
{
	int ret = 0;
	timedatecount++;
	ret = fSetDateFormat(value);
	if(timedatecount == TIMEDATE_MAX_VARIABLE)
		SetTimeDateDetail();
	return ret;
}
int SetTStampFormat(unsigned char value)
{
	int ret = 0;
	timedatecount++;
	ret = fSetTStampFormat(value);
	if(timedatecount == TIMEDATE_MAX_VARIABLE)
		SetTimeDateDetail();
	return ret;
}
int SetDatePosition(unsigned char value)
{
	int ret = 0;
	timedatecount++;
	ret = fSetDatePosition(value);
	if(timedatecount == TIMEDATE_MAX_VARIABLE)
		SetTimeDateDetail();
	return ret;
}
int SetTimePosition(unsigned char value)
{
	int ret = 0;
	timedatecount++;
	ret = fSetTimePosition(value);
	if(timedatecount == TIMEDATE_MAX_VARIABLE)
		SetTimeDateDetail();
	return ret;
}

int osdcount[3]={0,0,0};

#define OSD_MAX_VARIABLE (8)

int SetOSDDetail(int id)
{
	OSDPrm osdPrm;
    SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	osdcount[id] = 0;

	osdPrm.dateEnable = pSysInfo->osd_config[id].dstampenable;
	osdPrm.timeEnable = pSysInfo->osd_config[id].tstampenable;
	osdPrm.logoEnable = pSysInfo->osd_config[id].nLogoEnable;
	osdPrm.logoPos = pSysInfo->osd_config[id].nLogoPosition;
	osdPrm.textEnable = pSysInfo->osd_config[id].nTextEnable;
	osdPrm.textPos = pSysInfo->osd_config[id].nTextPosition;
	strcpy(osdPrm.text, (char *)pSysInfo->osd_config[id].overlaytext);
	osdPrm.detailedInfo = pSysInfo->osd_config[id].nDetailInfo;

	return SetOSDPrmMsg(id, &osdPrm);
}

int SetDateStampEnable1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetDateStampEnable1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetDateStampEnable2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetDateStampEnable2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetDateStampEnable3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetDateStampEnable3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetTimeStampEnable1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetTimeStampEnable1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetTimeStampEnable2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetTimeStampEnable2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetTimeStampEnable3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetTimeStampEnable3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetLogoEnable1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetLogoEnable1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetLogoEnable2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetLogoEnable2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetLogoEnable3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetLogoEnable3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}
int SetLogoPosition1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetLogoPosition1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetLogoPosition2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetLogoPosition2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetLogoPosition3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetLogoPosition3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetTextPosition1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetTextPosition1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetTextPosition2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetTextPosition2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetTextPosition3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetTextPosition3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetTextEnable1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetTextEnable1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetTextEnable2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetTextEnable2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetTextEnable3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetTextEnable3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetOverlayText1(void * buf, int length)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetOverlayText1(buf, length);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetOverlayText2(void * buf, int length)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetOverlayText2(buf, length);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetOverlayText3(void * buf, int length)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetOverlayText3(buf, length);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetDetailInfo1(unsigned char value)
{
	int ret = 0;
	osdcount[0]++;
	ret = fSetDetailInfo1(value);
	if(osdcount[0] == OSD_MAX_VARIABLE)
		SetOSDDetail(0);
	return ret;
}

int SetDetailInfo2(unsigned char value)
{
	int ret = 0;
	osdcount[1]++;
	ret = fSetDetailInfo2(value);
	if(osdcount[1] == OSD_MAX_VARIABLE)
		SetOSDDetail(1);
	return ret;
}

int SetDetailInfo3(unsigned char value)
{
	int ret = 0;
	osdcount[2]++;
	ret = fSetDetailInfo3(value);
	if(osdcount[2] == OSD_MAX_VARIABLE)
		SetOSDDetail(2);
	return ret;
}

int SetLocalDisplay(unsigned char value)
{
      int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	SetDisplayValue(value);
    ret = fSetLocalDisplay(value);

	return ret;
}

int codeccount[3]={0,0,0};

#define CODEC_MAX_VARIABLE (7)

void SetCodecAdvParam(int id)
{
	CodecAdvPrm codecPrm;
    SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return ;

	codeccount[id] = 0;

	codecPrm.ipRatio 		= pSysInfo->codec_advconfig[id].ipRatio;
	codecPrm.fIframe 		= pSysInfo->codec_advconfig[id].fIframe;
	codecPrm.qpInit 		= pSysInfo->codec_advconfig[id].qpInit;
	codecPrm.qpMin 			= pSysInfo->codec_advconfig[id].qpMin;
	codecPrm.qpMax 			= pSysInfo->codec_advconfig[id].qpMax;
	codecPrm.meConfig 		= pSysInfo->codec_advconfig[id].meConfig;
	codecPrm.packetSize 	= pSysInfo->codec_advconfig[id].packetSize;

	SetCodecAdvPrmMsg(id, &codecPrm);
}

int SetIpratio1(unsigned int value)
{
	int ret = 0, i = 0;

	value = CheckValueBound(value, 1, 30);

	codeccount[i]++;
    ret = fSetIpratio1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetIpratio2(unsigned int value)
{
	int ret = 0, i = 1;

	value = CheckValueBound(value, 1, 30);

	codeccount[i]++;
    ret = fSetIpratio2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetIpratio3(unsigned int value)
{
	int ret = 0, i = 2;

	value = CheckValueBound(value, 1, 30);

	codeccount[i]++;
	ret = fSetIpratio3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetForceIframe1(unsigned char value)
{
	int ret = 0, i = 0;

	codeccount[i]++;
	ret = fSetForceIframe1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetForceIframe2(unsigned char value)
{
	int ret = 0, i = 1;

	codeccount[i]++;
	ret = fSetForceIframe2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetForceIframe3(unsigned char value)
{
	int ret = 0, i = 2;

	codeccount[i]++;
	ret = fSetForceIframe3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPInit1(unsigned char value)
{
	int ret = 0, i = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype1==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype1==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	codeccount[i]++;
	ret = fSetQPInit1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPInit2(unsigned char value)
{
	int ret = 0, i = 1;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype2==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype2==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	codeccount[i]++;
	ret = fSetQPInit2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPInit3(unsigned char value)
{
	int ret = 0, i = 2;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype3==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype3==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	codeccount[i]++;
	ret = fSetQPInit3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMin1(unsigned char value)
{
	int ret = 0, i = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype1==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype1==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
	ret = fSetQPMin1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMin2(unsigned char value)
{
	int ret = 0, i = 1;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype2==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype2==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
    ret = fSetQPMin2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMin3(unsigned char value)
{
	int ret = 0, i = 2;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype3==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype3==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
	ret = fSetQPMin3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMax1(unsigned char value)
{
	int ret = 0, i = 0;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype1==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype1==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
    ret = fSetQPMax1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMax2(unsigned char value)
{
	int ret = 0, i = 1;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype2==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype2==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
    ret = fSetQPMax2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetQPMax3(unsigned char value)
{
	int ret = 0, i = 2;

	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(pSysInfo->lan_config.codectype3==H264_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 51);
	else if(pSysInfo->lan_config.codectype3==MPEG4_CODEC)
		value = (unsigned char)CheckValueBound((int)value, 4, 31);

	value = (value==0) ? 1: value;

	codeccount[i]++;
    ret = fSetQPMax3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetMEConfig1(unsigned char value)
{
	int ret = 0, i = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	codeccount[i]++;

	if (value != pSysInfo->codec_advconfig[i].meConfig) {
		if ((ret = fSetMEConfig1(value)) == 0) {
			SetRestart();
		}
	}

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetMEConfig2(unsigned char value)
{
	int ret = 0, i = 1;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	codeccount[i]++;

	if(value != SVC) {
		if (value != pSysInfo->codec_advconfig[i].meConfig) {
			if ((ret = fSetMEConfig2(value)) == 0) {
				SetRestart();
			}
		}
	}

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetMEConfig3(unsigned char value)
{
	int ret = 0, i = 2;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	codeccount[i]++;

	if(value != SVC) {
		if (value != pSysInfo->codec_advconfig[i].meConfig) {
			if ((ret = fSetMEConfig3(value)) == 0) {
				SetRestart();
			}
		}
	}

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetPacketSize1(unsigned char value)
{
	int ret = 0, i = 0;

	codeccount[i]++;
    ret = fSetPacketSize1(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetPacketSize2(unsigned char value)
{
	int ret = 0, i = 1;

	codeccount[i]++;
    ret = fSetPacketSize2(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int SetPacketSize3(unsigned char value)
{
	int ret = 0, i = 2;

	codeccount[i]++;
    ret = fSetPacketSize3(value);

	if(codeccount[i] == CODEC_MAX_VARIABLE)
		SetCodecAdvParam(i);

	return ret;
}

int roicount[MAX_GUI_ROI]={0,0,0};

#define ROI_MAX_VARIABLE (13)

void SetCodecROIParam(int id)
{
	int i=0;
	CodecROIPrm codecROIPrm;
    SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return ;

	roicount[id] = 0;

	codecROIPrm.numROI 			= pSysInfo->codec_roiconfig[id].numROI;

	for(i=0;i<MAX_GUI_ROI;i++) {
		codecROIPrm.roi[i].startx 	= pSysInfo->codec_roiconfig[id].roi[i].startx;
		codecROIPrm.roi[i].starty 	= pSysInfo->codec_roiconfig[id].roi[i].starty;
		codecROIPrm.roi[i].width	= pSysInfo->codec_roiconfig[id].roi[i].width;
		codecROIPrm.roi[i].height 	= pSysInfo->codec_roiconfig[id].roi[i].height;
	}

	SetCodecROIMsg(id, &codecROIPrm);
}

int SetROIEnable1(unsigned char value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetROIEnable1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetROIEnable2(unsigned char value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetROIEnable2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetROIEnable3(unsigned char value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetROIEnable3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1X1(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1X1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1Y1(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1Y1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1W1(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1W1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1H1(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1H1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1X2(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1X2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1Y2(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1Y2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1W2(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1W2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1H2(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1H2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1X3(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1X3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1Y3(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1Y3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1W3(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1W3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr1H3(unsigned int value)
{
	int ret = 0, i = 0;

	roicount[i]++;
    ret = fSetStr1H3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2X1(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2X1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2Y1(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2Y1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2W1(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2W1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2H1(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2H1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2X2(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2X2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2Y2(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2Y2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2W2(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2W2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2H2(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2H2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2X3(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2X3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2Y3(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2Y3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2W3(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2W3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr2H3(unsigned int value)
{
	int ret = 0, i = 1;

	roicount[i]++;
    ret = fSetStr2H3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3X1(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3X1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3Y1(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3Y1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3W1(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3W1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3H1(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3H1(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3X2(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3X2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3Y2(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3Y2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3W2(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3W2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3H2(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3H2(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3X3(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3X3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3Y3(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3Y3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3W3(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3W3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetStr3H3(unsigned int value)
{
	int ret = 0, i = 2;

	roicount[i]++;
    ret = fSetStr3H3(value);

	if(roicount[i] == ROI_MAX_VARIABLE)
		SetCodecROIParam(i);

	return ret;
}

int SetAudioReceiverEnable(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->audio_config.audiorecvenable)
	{
    	ret = fSetAudioReceiverEnable(value);
 		SYSTEM("killall -9 audio-receiver\n");
 		sleep(2);
		RTSP_AudioRecvON();
	}

	return ret;
}

int SetAudioSeverIp(void * buf, int length)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(strncmp((char *)buf, (char *)pSysInfo->audio_config.audioServerIp, length) != 0)
	{
	    ret = fSetAudioSeverIp(buf, length);

	    if(pSysInfo->audio_config.audiorecvenable)
	    {
 	  	    SYSTEM("killall -9 audio-receiver\n");
 		    sleep(2);
		    RTSP_AudioRecvON();
	    }
	}

	return ret;
}

int SetLinearWdr(unsigned char value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	if(value != pSysInfo->lan_config.linearwdr)
	{
		if ((ret = fSetLinearWdr(value)) == 0) {
			SetLinWdr(value);
		}
	}

	return ret;
}

int SetFlickerDetect(int value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if(pSysInfo == NULL)
		return -1;

	fSetFlickerDetect(SetFlickDec(value));

	return ret;
}

/* DMVA Variables */
/* DMVA TZ Variables */
int dmvaTZParamCount = 0;
int dmvaTZDiffParamCount = 0;
#define DMVA_TZ_MAX_VARIABLE (77)/* (11+2+64) Not considering TZENABLE since it is not being passed by GUI */
/* Set DMVA Main Page specific Parameters */
int dmvaMainParamCount = 0;
int dmvaMainDiffParamCount = 0;                        // AYK - 0226
#define DMVA_MAIN_MAX_VARIABLE (1) //(16)/*(10+6)*/

int dmvaMainSearchParamCount = 0;
int dmvaMainSearchDiffParamCount = 0;                        // AYK - 0226
#define DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE  (6)
/* Set DMVA Advanced Settings Page specific Parameters */
#define DMVA_ADVANCED_MAX_VARIABLE (14)
int dmvaAdvancedParamCount = 0;
int dmvaAdvancedDiffParamCount = 0;                         // AYK - 0226
/* Set DMVA CTD Page specific Parameters */
int dmvaCTDParamCount = 0;
int dmvaCTDDiffParamCount = 0;                         // AYK - 0226
#define DMVA_CTD_MAX_VARIABLE (4)/*(4)*/
/* Set DMVA OC Page specific Parameters */
int dmvaOCParamCount = 0;
int dmvaOCDiffParamCount = 0;                          // AYK - 0226
#define DMVA_OC_MAX_VARIABLE (6) /*(7)*/
/* Set DMVA SMETA Page specific Parameters */
int dmvaSMETAParamCount = 0;
int dmvaSMETADiffParamCount = 0;                        // AYK - 0226
#define DMVA_SMETA_MAX_VARIABLE (44)/*(19)*/
/* Set DMVA IMD Page specific Parameters */
int dmvaIMDParamCount = 0;
int dmvaIMDDiffParamCount = 0;                         // AYK - 0226
#define DMVA_IMD_MAX_VARIABLE (144) /*(12+4+128)*/

void SetDmvaPrm(int ret)
{
	DmvaParam dmvaParam;
    SysInfo *pSysInfo = GetSysInfo();

    if(ret != -2)
    {
		dmvaParam.dmvaenable = pSysInfo->dmva_config.dmvaenable_sys;
		//fprintf(stderr, "\n dmvaenable=%d, dmvaenable_sys = %d\n",dmvaenable, dmvaenable_sys);
		SendDmvaEnableMsg(&dmvaParam);
    }
}

void SetExptAlgoFrameRatePrm(int ret)
{
	DmvaParam dmvaParam;
    SysInfo *pSysInfo = GetSysInfo();

    if(ret != -2)
    {
		dmvaParam.dmvaexptalgoframerate = pSysInfo->dmva_config.dmvaexptalgoframerate_sys;
		SendDmvaExptAlgoFrameRateMsg(&dmvaParam);
    }
}

void SetExptAlgoDetectFreqPrm(int ret)
{
	DmvaParam dmvaParam;
    SysInfo *pSysInfo = GetSysInfo();

    if(ret != -2)
    {
		dmvaParam.dmvaexptalgodetectfreq = pSysInfo->dmva_config.dmvaexptalgodetectfreq_sys;
		SendDmvaExptAlgoDetectFreqMsg(&dmvaParam);
    }
}

void SetExptEvtRecordEnablePrm(int ret)
{
	DmvaParam dmvaParam;
    SysInfo *pSysInfo = GetSysInfo();

    if(ret != -2)
    {
		dmvaParam.dmvaexptevtrecordenable = pSysInfo->dmva_config.dmvaexptevtrecordenable_sys;
		SendDmvaExptEvtRecordEnableMsg(&dmvaParam);
    }
}

void SetExptSmetaTrackerEnablePrm(int ret)
{
	DmvaParam dmvaParam;
    SysInfo *pSysInfo = GetSysInfo();

    if(ret != -2)
    {
		dmvaParam.dmvaexptsmetatrackerenable = pSysInfo->dmva_config.dmvaexptSmetaTrackerEnable_sys;
		SendDmvaExptSmetaTrackerEnableMsg(&dmvaParam);
    }
}

int SetDmvaEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEnable(value);
	if(ret == -2) ret = 0;
	SetDmvaPrm(ret);

	return ret;
}
int SetDmvaExptAlgoFrameRate(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaExptAlgoFrameRate(value);
	if(ret == -2) ret = 0;
	SetExptAlgoFrameRatePrm(ret);

	return ret;
}
int SetDmvaExptAlgoDetectFreq(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaExptAlgoDetectFreq(value);
	if(ret == -2) ret = 0;
	SetExptAlgoDetectFreqPrm(ret);

	return ret;
}
int SetDmvaExptEvtRecordEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaExptEvtRecordEnable(value);
	if(ret == -2) ret = 0;
	SetExptEvtRecordEnablePrm(ret);

	return ret;
}

int SetDmvaExptSmetaTrackerEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaExptSmetaTrackerEnable(value);
	if(ret == -2) ret = 0;
	SetExptSmetaTrackerEnablePrm(ret);

	return ret;
}


void SetDmvaTZPrm(void)
{
	int i;

	DmvaTZParam dmvaTZParam;
    SysInfo *pSysInfo;

    if(dmvaTZDiffParamCount == 0)
    {
        dmvaTZParamCount = 0;
	}
	else
	{
		dmvaTZParamCount     = 0;
		dmvaTZDiffParamCount = 0;

        pSysInfo = GetSysInfo();

		/* DMVA TZ parameters */
		dmvaTZParam.dmvaTZSensitivity      = pSysInfo->dmva_config.dmvaAppTZSensitivity_sys;
		dmvaTZParam.dmvaTZPersonMinWidth   = pSysInfo->dmva_config.dmvaAppTZPersonMinWidth_sys;
		dmvaTZParam.dmvaTZPersonMinHeight  = pSysInfo->dmva_config.dmvaAppTZPersonMinHeight_sys;
		dmvaTZParam.dmvaTZVehicleMinWidth  = pSysInfo->dmva_config.dmvaAppTZVehicleMinWidth_sys;
		dmvaTZParam.dmvaTZVehicleMinHeight = pSysInfo->dmva_config.dmvaAppTZVehicleMinHeight_sys;
		dmvaTZParam.dmvaTZPersonMaxWidth   = pSysInfo->dmva_config.dmvaAppTZPersonMaxWidth_sys;
		dmvaTZParam.dmvaTZPersonMaxHeight  = pSysInfo->dmva_config.dmvaAppTZPersonMaxHeight_sys;
		dmvaTZParam.dmvaTZVehicleMaxWidth  = pSysInfo->dmva_config.dmvaAppTZVehicleMaxWidth_sys;
		dmvaTZParam.dmvaTZVehicleMaxHeight = pSysInfo->dmva_config.dmvaAppTZVehicleMaxHeight_sys;
		dmvaTZParam.dmvaTZDirection        = pSysInfo->dmva_config.dmvaAppTZDirection_sys;
		dmvaTZParam.dmvaTZPresentAdjust    = pSysInfo->dmva_config.dmvaAppTZPresentAdjust_sys;
		dmvaTZParam.dmvaTZZone1_ROI_numSides           = pSysInfo->dmva_config.dmvaAppTZZone1_ROI_numSides_sys;
		dmvaTZParam.dmvaTZZone2_ROI_numSides           = pSysInfo->dmva_config.dmvaAppTZZone2_ROI_numSides_sys;

		/* dmvaTZEnable is not transmitted as GUI keyword. So this is just a dummy copying */
		dmvaTZParam.dmvaTZEnable           = pSysInfo->dmva_config.dmvaAppTZEnable_sys;

		for(i=0;i<16;i++) {
			dmvaTZParam.dmvaTZZone1_x[i]           = pSysInfo->dmva_config.dmvaAppTZZone1_x[i+1];
			dmvaTZParam.dmvaTZZone1_y[i]           = pSysInfo->dmva_config.dmvaAppTZZone1_y[i+1];
			dmvaTZParam.dmvaTZZone2_x[i]           = pSysInfo->dmva_config.dmvaAppTZZone2_x[i+1];
			dmvaTZParam.dmvaTZZone2_y[i]           = pSysInfo->dmva_config.dmvaAppTZZone2_y[i+1];
		}
	//fprintf(stderr,"\n HARMEET:CHECK SetDmvaTZPrm\n");
  //fprintf(stderr,"\n ----------------------------\n");
  //fprintf(stderr,"\n dmvaTZParam.dmvaTZSensitivity    = %d\n",dmvaTZParam.dmvaTZSensitivity);
  //fprintf(stderr,"\n pSysInfo->dmva_config.dmvaAppTZSensitivity_sys      = %d\n",pSysInfo->dmva_config.dmvaAppTZSensitivity_sys);

		SendDmvaTZSetupMsg(&dmvaTZParam);
    }
}

int SetDmvaTZSensitivity(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZSensitivity(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

int SetDmvaTZPersonMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZPersonMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZPersonMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZPersonMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZVehicleMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZVehicleMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZVehicleMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZVehicleMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZPersonMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZPersonMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZPersonMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZPersonMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZVehicleMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZVehicleMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZVehicleMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZVehicleMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZDirection(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZDirection(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}
int SetDmvaTZPresentAdjust(unsigned int value)
{
	int ret = 0;
	/* Note: This function need to be updated to connect the parameters appropriately */
	ret = fSetDmvaTZPresentAdjust(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

int SetDmvaTZEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZEnable(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

int SetDmvaTZZone1Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

int SetDmvaTZZone2Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

int SetDmvaTZZone1_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x01(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x02(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x03(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x04(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x05(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x06(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x07(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x08(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x09(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x10(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x11(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x12(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x13(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x14(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x15(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_x16(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}



int SetDmvaTZZone1_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y01(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y02(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y03(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y04(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y05(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y06(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y07(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y08(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y09(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y10(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y11(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y12(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y13(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y14(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y15(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone1_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone1_y16(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x01(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x02(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x03(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x04(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x05(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x06(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x07(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x08(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x09(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x10(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x11(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x12(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x13(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x14(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x15(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_x16(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}



int SetDmvaTZZone2_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y01(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y02(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y03(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y04(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y05(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y06(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y07(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y08(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y09(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y10(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y11(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y12(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y13(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y14(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y15(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}


int SetDmvaTZZone2_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaTZZone2_y16(value);
	if(ret == -2) ret = 0;
	else dmvaTZDiffParamCount ++;
	dmvaTZParamCount++;
	if(dmvaTZParamCount == DMVA_TZ_MAX_VARIABLE)
		SetDmvaTZPrm();

	return ret;
}

/* DMVA TZ save/load params */
int SetDmvaTZSave(void * buf, int length)
{
    return fSetDmvaTZSave(buf,length);
}

int SetDmvaTZLoad(unsigned int value)
{
    int ret = 0;
    ret = fSetDmvaTZLoad(value);

    if(ret == 0)
    {
        // Send the loaded TZ params to AV server
        dmvaTZDiffParamCount = 1;
        SetDmvaTZPrm();
	}

    return ret;
}

/* Set DMVA Main Page specific Parameters */

void SetDmvaMainListSearchPrm(void)
{
	DmvaMainParam dmvaMainParam;
    SysInfo *pSysInfo;

    if(dmvaMainSearchDiffParamCount == 0)
    {
        dmvaMainSearchParamCount = 0;
	}
    else
    {
		dmvaMainSearchParamCount     = 0;
        dmvaMainSearchDiffParamCount = 0;

        pSysInfo = GetSysInfo();

		/* DMVA MAIN PAGE parameters */
		dmvaMainParam.dmvaEventListStartMonth             = pSysInfo->dmva_config.dmvaAppEventListStartMonth_sys;
		dmvaMainParam.dmvaEventListStartDay               = pSysInfo->dmva_config.dmvaAppEventListStartDay_sys;
		dmvaMainParam.dmvaEventListStartTime              = pSysInfo->dmva_config.dmvaAppEventListStartTime_sys;
		dmvaMainParam.dmvaEventListEndMonth               = pSysInfo->dmva_config.dmvaAppEventListEndMonth_sys;
		dmvaMainParam.dmvaEventListEndDay                 = pSysInfo->dmva_config.dmvaAppEventListEndDay_sys;
		dmvaMainParam.dmvaEventListEndTime                = pSysInfo->dmva_config.dmvaAppEventListEndTime_sys;

		SendDmvaMainSetupMsg(&dmvaMainParam);
    }
}
void SetDmvaMainPrm(void)
{
	DmvaMainParam dmvaMainParam;
    SysInfo *pSysInfo;

    pSysInfo = GetSysInfo();

    dmvaMainParam.dmvaCfgCTDImdSmetaTzOc = pSysInfo->dmva_config.dmvaAppCfgCTDImdSmetaTzOc_sys;
    dmvaMainParam.dmvaDisplayOptions     = pSysInfo->dmva_config.dmvaAppDisplayOptions_sys;
    dmvaMainParam.dmvaEventRecordingVAME = pSysInfo->dmva_config.dmvaAppEventRecordingVAME_sys;

    SendDmvaMainSetupMsg(&dmvaMainParam);
}
int SetDmvaCamID(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCamID(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}
int SetDmvaGetSchedule(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaGetSchedule(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}
int SetDmvaCfgFD(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgFD(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}
int SetDmvaCfgCTDImdSmetaTzOc(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgCTDImdSmetaTzOc(value);
	if(ret == -2) ret = 0;
	else SetDmvaMainPrm();
	return ret;
}
int SetDmvaDisplayOptions(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaDisplayOptions(value);
	if(ret == -2) ret = 0;
	else SetDmvaMainPrm();
	return ret;
}
int SetDmvaEventRecordingVAME(unsigned int value)
{
	int ret = 0;
	SysInfo *pSysInfo = GetSysInfo();
	if((pSysInfo->audio_config.audioenable == 0) && (value & 0x4))
	{
		value = (value & 0xB);
	}
	ret = fSetDmvaEventRecordingVAME(value);
	if(ret == -2) ret = 0;
	else SetDmvaMainPrm();
	return ret;
}
int SetDmvaEventListActionPlaySendSearchTrash(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListActionPlaySendSearchTrash(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();
#endif

	return ret;
}
int SetDmvaEventListSelectEvent(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListSelectEvent(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}
int SetDmvaEventListArchiveFlag(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListArchiveFlag(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}
int SetDmvaEventListEventType(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListEventType(value);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();

	return ret;
}


int SetDmvaEventListStartMonth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListStartMonth(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListStartDay(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListStartDay(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListStartTime(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListStartTime(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListEndMonth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListEndMonth(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListEndDay(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListEndDay(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListEndTime(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventListEndTime(value);
	if(ret == -2) ret = 0;

#if 0
	else dmvaMainSearchDiffParamCount ++;
	dmvaMainSearchParamCount++;
	if(dmvaMainSearchParamCount == DMVA_MAIN_LIST_SEARCH_MAX_VARIABLE)
		SetDmvaMainListSearchPrm();
#endif

	return ret;
}

int SetDmvaEventListTimeStamp(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaEventListTimeStamp(buf, length);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();
	return ret;
}

int SetDmvaEventListEventDetails(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaEventListEventDetails(buf, length);
	if(ret == -2) ret = 0;
	else dmvaMainDiffParamCount ++;
	dmvaMainParamCount++;
	if(dmvaMainParamCount == DMVA_MAIN_MAX_VARIABLE)
		SetDmvaMainPrm();
	return ret;
}

/* Set DMVA Advanced Settings Page specific Parameters */

void SetDmvaAdvancedPrm(void)
{
	//DmvaAdvancedParam dmvaAdvancedParam;
    SysInfo *pSysInfo;

	/* TO BE IMPLEMENTED TO SEND PARAMETERS TO AVSERVER */
    if(dmvaAdvancedDiffParamCount == 0)
    {
	    dmvaAdvancedParamCount = 0;
	}
	else
	{
	    dmvaAdvancedParamCount     = 0;
	    dmvaAdvancedDiffParamCount = 0;

	    pSysInfo = GetSysInfo();

	    /* DMVA Advanced Settings PAGE parameters */
	    //dmvaAdvancedParam.dmvaCfgTDEnv1        = pSysInfo->dmva_config.dmvaAppCfgTDEnv1_sys;
	    //dmvaAdvancedParam.dmvaCfgTDEnv2        = pSysInfo->dmva_config.dmvaAppCfgTDEnv2_sys;
	    //dmvaAdvancedParam.dmvaCfgTDSensitivity = pSysInfo->dmva_config.dmvaAppCfgTDSensitivity_sys;
	    //dmvaAdvancedParam.dmvaCfgTDResetTime   = pSysInfo->dmva_config.dmvaAppCfgTDResetTime_sys;

	    //SendDmvaCTDSetupMsg(&dmvaCTDParam);
    }
}
int SetDmvaObjectMetaData(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaObjectMetaData(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesFontType(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesFontType(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesFontSize(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesFontSize(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesFontColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesFontColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesBoxColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesBoxColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesCentroidColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesCentroidColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesActiveZoneColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesActiveZoneColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesInactiveZoneColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesInactiveZoneColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaOverlayPropertiesVelocityColor(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOverlayPropertiesVelocityColor(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}

int SetDmvaEnvironmentProfileLoad(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEnvironmentProfileLoad(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}
int SetDmvaEnvironmentProfileStore(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEnvironmentProfileStore(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}
int SetDmvaEnvironmentProfileClear(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEnvironmentProfileClear(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}
int SetDmvaEnvironmentProfileState(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEnvironmentProfileState(value);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}

	return ret;
}
int SetDmvaEnvironmentProfileDesc(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaEnvironmentProfileDesc(buf, length);
	if(ret == -2) ret = 0;
	else dmvaAdvancedDiffParamCount ++;
	dmvaAdvancedParamCount++;
	if(dmvaAdvancedParamCount == DMVA_ADVANCED_MAX_VARIABLE){
		SetDmvaAdvancedPrm();
	}
	return ret;
}

/* Set DMVA event delete specific functions */
int SetDmvaEventDeleteIndex(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventDeleteIndex(value);
	return ret;
}

int SetDmvaEventDeleteStartIndex(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventDeleteStartIndex(value);
	return ret;
}

int SetDmvaEventDeleteEndIndex(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventDeleteEndIndex(value);
	return ret;
}

int SetDmvaEventDeleteAll(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaEventDeleteAll(value);
	return ret;
}

/* Set DMVA CTD Page specific Parameters */
void SetDmvaCTDPrm(void)
{
	DmvaCTDParam dmvaCTDParam;
    SysInfo *pSysInfo;

    if(dmvaCTDDiffParamCount == 0)
    {
	    dmvaCTDParamCount = 0;
	}
	else
	{
	    dmvaCTDParamCount     = 0;
	    dmvaCTDDiffParamCount = 0;

	    pSysInfo = GetSysInfo();

	    /* DMVA CTD PAGE parameters */
	    dmvaCTDParam.dmvaCfgTDEnv1        = pSysInfo->dmva_config.dmvaAppCfgTDEnv1_sys;
	    dmvaCTDParam.dmvaCfgTDEnv2        = pSysInfo->dmva_config.dmvaAppCfgTDEnv2_sys;
	    dmvaCTDParam.dmvaCfgTDSensitivity = pSysInfo->dmva_config.dmvaAppCfgTDSensitivity_sys;
	    dmvaCTDParam.dmvaCfgTDResetTime   = pSysInfo->dmva_config.dmvaAppCfgTDResetTime_sys;

	    SendDmvaCTDSetupMsg(&dmvaCTDParam);
    }
}
int SetDmvaCfgTDEnv1(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgTDEnv1(value);
	if(ret == -2) ret = 0;
	else dmvaCTDDiffParamCount ++;
	dmvaCTDParamCount++;
	if(dmvaCTDParamCount == DMVA_CTD_MAX_VARIABLE)
		SetDmvaCTDPrm();

	return ret;
}
int SetDmvaCfgTDEnv2(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgTDEnv2(value);
	if(ret == -2) ret = 0;
	else dmvaCTDDiffParamCount ++;
	dmvaCTDParamCount++;
	if(dmvaCTDParamCount == DMVA_CTD_MAX_VARIABLE)
		SetDmvaCTDPrm();

	return ret;
}
int SetDmvaCfgTDSensitivity(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgTDSensitivity(value);
	if(ret == -2) ret = 0;
	else dmvaCTDDiffParamCount ++;
	dmvaCTDParamCount++;
	if(dmvaCTDParamCount == DMVA_CTD_MAX_VARIABLE)
		SetDmvaCTDPrm();

	return ret;
}
int SetDmvaCfgTDResetTime(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaCfgTDResetTime(value);
	if(ret == -2) ret = 0;
	else dmvaCTDDiffParamCount ++;
	dmvaCTDParamCount++;
	if(dmvaCTDParamCount == DMVA_CTD_MAX_VARIABLE)
		SetDmvaCTDPrm();

	return ret;
}

/* Set DMVA OC Page specific Parameters */
void SetDmvaOCPrm(void)
{
	DmvaOCParam dmvaOCParam;
    SysInfo *pSysInfo;

    if(dmvaOCDiffParamCount == 0)
    {
	    dmvaOCParamCount = 0;
	}
	else
	{
		dmvaOCParamCount     = 0;
		dmvaOCDiffParamCount = 0;

		pSysInfo = GetSysInfo();

		/* DMVA OC PAGE parameters */
		dmvaOCParam.dmvaOCSensitivity    = pSysInfo->dmva_config.dmvaAppOCSensitivity_sys;
		dmvaOCParam.dmvaOCObjectWidth    = pSysInfo->dmva_config.dmvaAppOCObjectWidth_sys;
		dmvaOCParam.dmvaOCObjectHeight   = pSysInfo->dmva_config.dmvaAppOCObjectHeight_sys;
		dmvaOCParam.dmvaOCDirection      = pSysInfo->dmva_config.dmvaAppOCDirection_sys;
		dmvaOCParam.dmvaOCEnable         = pSysInfo->dmva_config.dmvaAppOCEnable_sys;
		dmvaOCParam.dmvaOCLeftRight      = pSysInfo->dmva_config.dmvaAppOCLeftRight_sys;
		dmvaOCParam.dmvaOCTopBottom      = pSysInfo->dmva_config.dmvaAppOCTopBottom_sys;

		SendDmvaOCSetupMsg(&dmvaOCParam);
    }
}
int SetDmvaOCSensitivity(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCSensitivity(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCObjectWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCObjectWidth(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCObjectHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCObjectHeight(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCDirection(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCDirection(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCEnable(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCLeftRight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCLeftRight(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}
int SetDmvaOCTopBottom(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaOCTopBottom(value);
	if(ret == -2) ret = 0;
	else dmvaOCDiffParamCount ++;
	dmvaOCParamCount++;
	if(dmvaOCParamCount == DMVA_OC_MAX_VARIABLE)
		SetDmvaOCPrm();

	return ret;
}

/* DMVA OC save/load params */
int SetDmvaOCSave(void * buf, int length)
{
    return fSetDmvaOCSave(buf,length);
}

int SetDmvaOCLoad(unsigned int value)
{
    int ret = 0;
    ret = fSetDmvaOCLoad(value);

    if(ret == 0)
    {
        // Send the loaded OC params to AV server
        dmvaOCDiffParamCount = 1;
        SetDmvaOCPrm();
	}

    return ret;
}


/* Set DMVA SMETA Page specific Parameters */

void SetDmvaSMETAPrm(void)
{
	int i;

	DmvaSMETAParam dmvaSMETAParam;
    SysInfo *pSysInfo;

	//printf("\n Current Count is %d\n",dmvaSMETAParamCount);
    if(dmvaSMETADiffParamCount == 0)
    {
	    dmvaSMETAParamCount = 0;
		//printf("\n Making Count to %d\n",dmvaSMETAParamCount);
	}
	else
    {
		dmvaSMETAParamCount     = 0;
		dmvaSMETADiffParamCount = 0;

		pSysInfo = GetSysInfo();

		/* DMVA SMETA PAGE parameters */
		dmvaSMETAParam.dmvaSMETASensitivity          = pSysInfo->dmva_config.dmvaAppSMETASensitivity_sys;
		dmvaSMETAParam.dmvaSMETAEnableStreamingData  = pSysInfo->dmva_config.dmvaAppSMETAEnableStreamingData_sys;
		dmvaSMETAParam.dmvaSMETAStreamBB             = pSysInfo->dmva_config.dmvaAppSMETAStreamBB_sys;
		dmvaSMETAParam.dmvaSMETAStreamVelocity       = pSysInfo->dmva_config.dmvaAppSMETAStreamVelocity_sys;
		dmvaSMETAParam.dmvaSMETAStreamHistogram      = pSysInfo->dmva_config.dmvaAppSMETAStreamHistogram_sys;
		dmvaSMETAParam.dmvaSMETAStreamMoments        = pSysInfo->dmva_config.dmvaAppSMETAStreamMoments_sys;

		dmvaSMETAParam.dmvaSMETAPresentAdjust    = pSysInfo->dmva_config.dmvaAppSMETAPresentAdjust_sys;
		dmvaSMETAParam.dmvaSMETAPersonMinWidth   = pSysInfo->dmva_config.dmvaAppSMETAPersonMinWidth_sys;
		dmvaSMETAParam.dmvaSMETAPersonMinHeight  = pSysInfo->dmva_config.dmvaAppSMETAPersonMinHeight_sys;
		dmvaSMETAParam.dmvaSMETAVehicleMinWidth  = pSysInfo->dmva_config.dmvaAppSMETAVehicleMinWidth_sys;
		dmvaSMETAParam.dmvaSMETAVehicleMinHeight = pSysInfo->dmva_config.dmvaAppSMETAVehicleMinHeight_sys;
		dmvaSMETAParam.dmvaSMETAPersonMaxWidth   = pSysInfo->dmva_config.dmvaAppSMETAPersonMaxWidth_sys;
		dmvaSMETAParam.dmvaSMETAPersonMaxHeight  = pSysInfo->dmva_config.dmvaAppSMETAPersonMaxHeight_sys;
		dmvaSMETAParam.dmvaSMETAVehicleMaxWidth  = pSysInfo->dmva_config.dmvaAppSMETAVehicleMaxWidth_sys;
		dmvaSMETAParam.dmvaSMETAVehicleMaxHeight = pSysInfo->dmva_config.dmvaAppSMETAVehicleMaxHeight_sys;
		dmvaSMETAParam.dmvaSMETANumZones         = pSysInfo->dmva_config.dmvaAppSMETANumZones_sys;

		dmvaSMETAParam.dmvaSMETAZone1_ROI_numSides           = pSysInfo->dmva_config.dmvaAppSMETAZone1_ROI_numSides_sys;

		for(i=0;i<16;i++) {
			dmvaSMETAParam.dmvaSMETAZone1_x[i]           = pSysInfo->dmva_config.dmvaAppSMETAZone1_x[i+1];
			dmvaSMETAParam.dmvaSMETAZone1_y[i]           = pSysInfo->dmva_config.dmvaAppSMETAZone1_y[i+1];
		}

		SendDmvaSMETASetupMsg(&dmvaSMETAParam);
    }
}
/* This variable not used */
int SetDmvaSMETAStreamBB(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAStreamBB(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
/* This variable not used */
int SetDmvaSMETAStreamVelocity(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAStreamVelocity(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

/* This variable not used */
int SetDmvaSMETAStreamHistogram(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAStreamHistogram(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

/* This variable not used */
int SetDmvaSMETAStreamMoments(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAStreamMoments(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETANumZones(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETANumZones(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETASensitivity(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETASensitivity(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAEnableStreamingData(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAEnableStreamingData(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAPresentAdjust(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAPresentAdjust(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAPersonMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAPersonMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAPersonMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAPersonMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAVehicleMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAVehicleMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAVehicleMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAVehicleMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAPersonMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAPersonMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAPersonMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAPersonMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAVehicleMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAVehicleMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}
int SetDmvaSMETAVehicleMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAVehicleMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x01(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x02(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x03(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x04(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x05(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x06(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x07(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x08(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x09(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x10(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x11(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x12(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x13(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x14(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x15(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_x16(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}


int SetDmvaSMETAZone1_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y01(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y02(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y03(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y04(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y05(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y06(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y07(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y08(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y09(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y10(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y11(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y12(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y13(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y14(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y15(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_y16(value);
	if(ret == -2) ret = 0;
	else dmvaSMETADiffParamCount ++;
	dmvaSMETAParamCount++;
	if(dmvaSMETAParamCount == DMVA_SMETA_MAX_VARIABLE)
		SetDmvaSMETAPrm();

	return ret;
}

int SetDmvaSMETAZone1_Label(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaSMETAZone1_Label(buf, length);
	if(ret == -2) ret = 0;
	//else dmvaIMDDiffParamCount ++;
	return ret;
}

/* DMVA SMETA save/load params */
int SetDmvaSMETASave(void * buf, int length)
{
    return fSetDmvaSMETASave(buf,length);
}

int SetDmvaSMETALoad(unsigned int value)
{
    int ret = 0;
    ret = fSetDmvaSMETALoad(value);

    if(ret == 0)
    {
        // Send the loaded SM params to AV server
        dmvaSMETADiffParamCount = 1;
        SetDmvaSMETAPrm();
	}

    return ret;
}


/* Set DMVA IMD Page specific Parameters */

void SetDmvaIMDPrm(void)
{
	DmvaIMDParam dmvaIMDParam;
	int i;
    SysInfo *pSysInfo;

    if(dmvaIMDDiffParamCount == 0)
    {
        dmvaIMDParamCount = 0;
	}
	else
	{
		dmvaIMDParamCount     = 0;
		dmvaIMDDiffParamCount = 0;

		pSysInfo = GetSysInfo();

		/* DMVA IMD PAGE parameters */
		dmvaIMDParam.dmvaIMDSensitivity      = pSysInfo->dmva_config.dmvaAppIMDSensitivity_sys;
		dmvaIMDParam.dmvaIMDPresentAdjust    = pSysInfo->dmva_config.dmvaAppIMDPresentAdjust_sys;
		dmvaIMDParam.dmvaIMDPersonMinWidth   = pSysInfo->dmva_config.dmvaAppIMDPersonMinWidth_sys;
		dmvaIMDParam.dmvaIMDPersonMinHeight  = pSysInfo->dmva_config.dmvaAppIMDPersonMinHeight_sys;
		dmvaIMDParam.dmvaIMDVehicleMinWidth  = pSysInfo->dmva_config.dmvaAppIMDVehicleMinWidth_sys;
		dmvaIMDParam.dmvaIMDVehicleMinHeight = pSysInfo->dmva_config.dmvaAppIMDVehicleMinHeight_sys;
		dmvaIMDParam.dmvaIMDPersonMaxWidth   = pSysInfo->dmva_config.dmvaAppIMDPersonMaxWidth_sys;
		dmvaIMDParam.dmvaIMDPersonMaxHeight  = pSysInfo->dmva_config.dmvaAppIMDPersonMaxHeight_sys;
		dmvaIMDParam.dmvaIMDVehicleMaxWidth  = pSysInfo->dmva_config.dmvaAppIMDVehicleMaxWidth_sys;
		dmvaIMDParam.dmvaIMDVehicleMaxHeight = pSysInfo->dmva_config.dmvaAppIMDVehicleMaxHeight_sys;
		dmvaIMDParam.dmvaIMDNumZones         = pSysInfo->dmva_config.dmvaAppIMDNumZones_sys;
		dmvaIMDParam.dmvaIMDZoneEnable       = pSysInfo->dmva_config.dmvaAppIMDZoneEnable_sys;

		dmvaIMDParam.dmvaIMDZone1_ROI_numSides           = pSysInfo->dmva_config.dmvaAppIMDZone1_ROI_numSides_sys;
		dmvaIMDParam.dmvaIMDZone2_ROI_numSides           = pSysInfo->dmva_config.dmvaAppIMDZone2_ROI_numSides_sys;
		dmvaIMDParam.dmvaIMDZone3_ROI_numSides           = pSysInfo->dmva_config.dmvaAppIMDZone3_ROI_numSides_sys;
		dmvaIMDParam.dmvaIMDZone4_ROI_numSides           = pSysInfo->dmva_config.dmvaAppIMDZone4_ROI_numSides_sys;

		for(i=0;i<16;i++) {
			dmvaIMDParam.dmvaIMDZone1_x[i]           = pSysInfo->dmva_config.dmvaAppIMDZone1_x[i+1];
			dmvaIMDParam.dmvaIMDZone1_y[i]           = pSysInfo->dmva_config.dmvaAppIMDZone1_y[i+1];
			dmvaIMDParam.dmvaIMDZone2_x[i]           = pSysInfo->dmva_config.dmvaAppIMDZone2_x[i+1];
			dmvaIMDParam.dmvaIMDZone2_y[i]           = pSysInfo->dmva_config.dmvaAppIMDZone2_y[i+1];
			dmvaIMDParam.dmvaIMDZone3_x[i]           = pSysInfo->dmva_config.dmvaAppIMDZone3_x[i+1];
			dmvaIMDParam.dmvaIMDZone3_y[i]           = pSysInfo->dmva_config.dmvaAppIMDZone3_y[i+1];
			dmvaIMDParam.dmvaIMDZone4_x[i]           = pSysInfo->dmva_config.dmvaAppIMDZone4_x[i+1];
			dmvaIMDParam.dmvaIMDZone4_y[i]           = pSysInfo->dmva_config.dmvaAppIMDZone4_y[i+1];
		}

		SendDmvaIMDSetupMsg(&dmvaIMDParam);
    }
}
int SetDmvaIMDSensitivity(unsigned int value)
{
	int ret = 0;

	ret = fSetDmvaIMDSensitivity(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDPresentAdjust(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDPresentAdjust(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDPersonMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDPersonMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDPersonMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDPersonMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDVehicleMinWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDVehicleMinWidth(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDVehicleMinHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDVehicleMinHeight(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDPersonMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDPersonMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDPersonMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDPersonMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDVehicleMaxWidth(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDVehicleMaxWidth(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDVehicleMaxHeight(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDVehicleMaxHeight(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDNumZones(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDNumZones(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}
int SetDmvaIMDZoneEnable(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZoneEnable(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone1Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4Numsides(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4Numsides(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone1_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_x16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone1_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_y16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}







int SetDmvaIMDZone2_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_x16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone2_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone2_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_y16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}





int SetDmvaIMDZone3_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_x16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone3_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone3_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_y16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}





int SetDmvaIMDZone4_x01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_x16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_x16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}


int SetDmvaIMDZone4_y01(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y01(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y02(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y02(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y03(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y03(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y04(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y04(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y05(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y05(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y06(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y06(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y07(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y07(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y08(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y08(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y09(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y09(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y10(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y10(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y11(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y11(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y12(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y12(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y13(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y13(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y14(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y14(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y15(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y15(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone4_y16(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_y16(value);
	if(ret == -2) ret = 0;
	else dmvaIMDDiffParamCount ++;
	dmvaIMDParamCount++;
	if(dmvaIMDParamCount == DMVA_IMD_MAX_VARIABLE)
		SetDmvaIMDPrm();

	return ret;
}

int SetDmvaIMDZone1_Label(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaIMDZone1_Label(buf, length);
	if(ret == -2) ret = 0;
	//else dmvaIMDDiffParamCount ++;
	return ret;
}

int SetDmvaIMDZone2_Label(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaIMDZone2_Label(buf, length);
	if(ret == -2) ret = 0;
	//else dmvaIMDDiffParamCount ++;
	return ret;
}
int SetDmvaIMDZone3_Label(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaIMDZone3_Label(buf, length);
	if(ret == -2) ret = 0;
	//else dmvaIMDDiffParamCount ++;
	return ret;
}
int SetDmvaIMDZone4_Label(void * buf, int length)
{
	int ret = 0;
	ret = fSetDmvaIMDZone4_Label(buf, length);
	if(ret == -2) ret = 0;
	//else dmvaIMDDiffParamCount ++;
	return ret;
}

/* DMVA IMD save/load params */
int SetDmvaIMDSave(void * buf, int length)
{
	return fSetDmvaIMDSave(buf,length);
}

int SetDmvaIMDLoad(unsigned int value)
{
	int ret = 0;
	ret = fSetDmvaIMDLoad(value);

	if(ret == 0)
	{
        // Send the loaded IMD params to AV server
        dmvaIMDDiffParamCount = 1;
        SetDmvaIMDPrm();
	}

	return ret;
}


/* DMVA Scheduler */
#define DMVA_SCH_PARAM_COUNT 	7

int dmvaSchParamCount = 0;
DMVA_schedule_t tempDmvaSch;

int SetDmvaSchEnable(unsigned int value)
{
	SysInfo *pSysInfo = GetSysInfo();
	if (pSysInfo == NULL)
		return -1;

	// Whenever DMVA Scheduler is turned ON/OFF change the detect mode to NONE
	SetDmvaCfgCTDImdSmetaTzOc(0);

	#ifdef DMVA_SCH_DBG
	fprintf(stderr," DMVA SCH > setting detect mode to NONE\n");
	#endif

    if(value == 0)
    {
	    pSysInfo->dmva_config.dmvaScheduleMgr.curSchedule = -1;
	}

    return fSetDmvaSchEnable(value);
}

int SetDmvaSchRuleType(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.ruleType = value;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchLoadSetting(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.loadSetting = value;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchRuleLabel(void * buf, int length)
{
    int ret = 0;

    memcpy(tempDmvaSch.ruleLabel,buf,length);
    tempDmvaSch.ruleLabel[length] = '\0';

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchStartDay(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.startDay = value + 1;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchStartTime(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.startTimeInMin = value;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchEndDay(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.endDay = value + 1;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchEndTime(unsigned int value)
{
    int ret = 0;

    tempDmvaSch.endTimeInMin = value;

    dmvaSchParamCount ++;
    if(dmvaSchParamCount == DMVA_SCH_PARAM_COUNT)
    {
		SemWait(gSemDmvaSchedule);
        ret = fSetDmvaSchCheckAddRule(&tempDmvaSch);
        SemRelease(gSemDmvaSchedule);
        dmvaSchParamCount = 0;
    }

    return ret;
}

int SetDmvaSchStart(unsigned int value)
{
    return fSetDmvaSchStart(value);
}

int SetDmvaSchDelRule(void * buf, int length)
{
	int ret = 0;

	SemWait(gSemDmvaSchedule);

	ret = fSetDmvaSchDelRule(buf,length);

	if(ret == -2)
	{
	    // the rule to be deleted is currently running
		// set detect mode to NONE
		SetDmvaCfgCTDImdSmetaTzOc(0);

		#ifdef DMVA_SCH_DBG
		fprintf(stderr," DMVA SCH > stopping the schedule rule\n");
		#endif

		ret = 0;
	}

    SemRelease(gSemDmvaSchedule);

    return ret;
}

/* DMVA Enc Roi Enable */
int SetDmvaEncRoiEnable(unsigned int value)
{
	int ret = 0;
    ret = fSetDmvaEncRoiEnable(value);
    if(ret == -2) ret = 0;
    else SendDmvaEncRoiEnableMsg(&value);
    return ret;
}
