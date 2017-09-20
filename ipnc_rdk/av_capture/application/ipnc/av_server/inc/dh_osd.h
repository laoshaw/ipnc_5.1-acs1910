
/**
  @file  dh_osd.h
  @brief 字体叠加函数声明

  Copyright (c) 2011, Daheng Imavision 
	
  @author zqj
  @date 2011-11-22
  @version 1.1.0
*/

#ifndef _DH_OSD_H
#define _DH_OSD_H

/**
  \brief yuv osd date type struct
*/
enum {
	DH_OSD_YUV_420 = 0,    ///yuv 420
	DH_OSD_YUV_422         ///yuv 422
};

/**
  \brief font color and out line struct
*/
typedef struct DH_OsdParm {

	unsigned char  osd_y;      ///osd y 
	unsigned char  osd_u;      ///osd u
	unsigned char  osd_v;      ///osd v
	unsigned char osd_outline ; /// out line . 0 : not outline 1:outline	
} DH_OsdParm;

/**
* @brief osd open font lib function.
* @param pCharLibName:font lib name. include chinese font and english font lib ;
* @retval  pFontLibBuf : font lib buf, Malloc by this api,but free by user after used.
*/
int  DH_Fontlib_Open(char * pCharLibName,char **pFontLibBuf);

/**
* @brief osd character function.
* @param character:osd text  ;
* @param pDataBuf:osd buf ;
* @param pLibBufCn:chinese lib ;
* @param pLibBufEn:English lib ;
* @param CharSizeW:char width ;
* @param CharSizeH:char height ;
* @param OsdStartX:osd_start_x ;
* @param OsdStartY:osd_start_y ;
* @param ImgWid:image width ;
* @param ImgHei:image height ;
* @param yuv_date_type:yuv date type ; 0:YUV420  1:YUV422 ;
* @param pOsdParm : osd corlor y u v and out line 
* @retval DH_OSD_OK :Success.
*/
int DH_Osd(unsigned char *character,
	unsigned char * pDataBuf,
	unsigned char * pLibBufCn,unsigned char * pLibBufEn,
	int CharSizeW,int CharSizeH,
	int OsdStartX,int OsdStartY,
 	int ImgWid,int ImgHei,
	int yuv_date_type,
 	DH_OsdParm *pOsdParm);


int DH_Osd_Block_Diagram(unsigned char *pDataBuf,int image_offset,
             	        int startx, int starty,
             	        int DiagramW, int DiagramH,
		        int ImgWid, int ImgHei,
		        int yuv_data_type);

/**
* @brief free osd font lib buf function.
* @brief pfontlibBuf :font lib buf, free by user after used.
* @retval DH_OSD_OK
*/
int DH_Osd_Free(char * pfontlibBuf);

#endif //_DH_OSD_H


