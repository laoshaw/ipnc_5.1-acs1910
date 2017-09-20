#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dh_osd.h"
//#include <dh_err_cmd_def.h>

/**
* @brief osd open font lib function.
* @param pCharLibName:font lib name. include chinese font and english font lib ;
* @retval  pFontLibBuf : font lib buf, Malloc by this api,but free by user after used.
*/
int  DH_Fontlib_Open(char * pCharLibName,char **pFontLibBuf)
{        
	FILE  *pfontlibfile ;
	int fontlib_len= 0 ;
	if((pfontlibfile=fopen(pCharLibName,"rb")) == NULL)  //open font lib file 
	{
		return  -1;//DH_OSD_OPEN_FONT_FILE_FAILED ;
	}	
	fseek(pfontlibfile,0,SEEK_END); 
	fontlib_len=ftell(pfontlibfile);      
	fseek(pfontlibfile,0,SEEK_SET);  
	*pFontLibBuf = (char *)malloc(fontlib_len);	
	if(*pFontLibBuf == NULL)
	{		
		fclose(pfontlibfile); 
		return  -1;//DH_OSD_MALLOC_FAILED ;
	}	
	memset(*pFontLibBuf,0,fontlib_len);
	fread(*pFontLibBuf,1,fontlib_len,pfontlibfile);	
	fclose(pfontlibfile);    
	
	return 0;//DH_OSD_OK ;
}


/**
* @brief osd character function for outline.
* @param pDataBuf:yuv osd buf ;
* @param image_offset:pos offset in image buf;
* @param hz_offset:character offset in image buf ;
* @param pChaLibBuf:font lib buf  ;
* @param offset:font lib offset ;
* @param CharSizeW:char width ;
* @param CharSizeH:char height ;
* @param OsdStartX:osd_start_x ;
* @param OsdStartY:osd_start_y ;
* @param ImgWid:image width ;
* @param ImgHei:image height 
* @param yuv_data_type:yuv date type ; 0:YUV420  1:YUV422 ;
* @param pOsdCorParm : osd corlor y u v and outline
* @retval DH_OSD_OK :Success.
*/
int DH_Character_Osd_OutLine(unsigned char * pDataBuf,
                 	                int image_offset,int hz_offset,
			                        unsigned char * pChaLibBuf,int offset,
                 	                int CharSizeW,int CharSizeH,
                                 	int startx,int starty,
                                 	int ImgWid,int ImgHei,
			                        int yuv_data_type,
                 	                DH_OsdParm *pOsdParm)
{
	int val_up = 0 ;
	int val_down = 0 ;
	//char fontbuf[CharSizeW/8*CharSizeH];   /* 足够大的缓冲区,也可以动态分配 */	
    char *fontbuf = NULL;
    int i_pos = 0 ;
	int     i;    /* 控制行 */
	int     j;    /* 控制一行中的8个点 */
	int     k;    /* 一行中的第几个"8个点"了 */
	int     nc = 0 ;   /* 到点阵数据的第几个字节了 */
	//int pix_x =  0 ;
	//int pix_y =  0 ;
    
	fontbuf = (char *)malloc(CharSizeW/8*CharSizeH);
	if(fontbuf == NULL)
	{
	    return -1;//DH_OSD_MALLOC_FAILED;
	}
	memcpy(fontbuf,(pChaLibBuf+offset),(CharSizeW + 7) / 8 * CharSizeH);		
	for (i=0; i<CharSizeH; i++)
	{
		i_pos = i * ImgWid ;    //line loop ++
		for (k=0; k<(CharSizeW/8); k++)
		{
			for (j=0; j<8; j++)
			{
				if(i == 0)
					val_up = 0;
				else
				{
					val_up = fontbuf[nc-CharSizeW/8];
				}
				if(i ==  (CharSizeH-1))
					val_down = 0;
				else
				{
					val_down = fontbuf[nc+CharSizeW/8];				
				}
				if (fontbuf[nc]&(0x80>>j))    //char is 1
				{
					switch(yuv_data_type)
					{
						case DH_OSD_YUV_420 :
							*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = pOsdParm->osd_y;
                           			 //不叠加颜色，暂时叠加的颜色不正确
							//pix_x = hz_offset+ startx + 8*k+j ;  //current osd point  x
							//pix_y = starty + i ;   //current osd point  y
							//osd u and v value
							//*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2 )= pOsdParm->osd_u ;
							//*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2 + 1)= pOsdParm->osd_v ;
							break ;
						case DH_OSD_YUV_422 :
							*(pDataBuf+(image_offset+hz_offset+i_pos+8*k+j)*2+1) = pOsdParm->osd_y;
							//osd u and v value
							*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4 )= pOsdParm->osd_u ;
							*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4  + 2 )= pOsdParm->osd_v ;	
							break ;

						default:
                           			free(fontbuf);
							return -1;//DH_OSD_NOT_SUPPORT_DATE_TPYE ;
					}						
				}
				else    //char is not 1 and find region to 0 for outline
				{
					if((fontbuf[nc]&(0x80>>(j-1))) 
		                                ||(fontbuf[nc]&(0x80>>(j+1)))
		                                ||(val_up & (0x80 >> j)) 
	                   				   ||(val_down & (0x80 >> j))) 
					 {
						switch(yuv_data_type)
						{
							case DH_OSD_YUV_420 :
								*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = 0;
                                //不叠加颜色，暂时叠加的颜色不正确
								//pix_x = hz_offset+ startx + 8*k+j ;  //current osd point  x
								//pix_y = starty + i ;   //current osd point  y
								//osd u and v value
								//*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2 ) = 128 ;
								//*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2 + 1 ) = 128 ;
								break ;
							case DH_OSD_YUV_422 :
								*(pDataBuf+(image_offset+hz_offset+i_pos+8*k+j)*2+1) = 0;
								//osd u and v value
								*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4  )= 128 ;
								*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4 + 2 )= 128 ;	
								break ;

							default:
                                				free(fontbuf);
								return -1;//DH_OSD_NOT_SUPPORT_DATE_TPYE ;
						}
					}	                         
				}
			}
			nc++;
		}
	}
    free(fontbuf);
    
	return 0;//DH_OSD_OK;
}

/**
* @brief osd character function for outline.
* @param pDataBuf:yuv osd buf ;
* @param image_offset:pos offset in image buf  ;
* @param hz_offset:character offset in image buf ;
* @param pChaLibBuf:font lib buf  ;
* @param offset:font lib offset ;
* @param CharSizeW:char width ;
* @param CharSizeH:char height ;
* @param OsdStartX:osd_start_x ;
* @param OsdStartY:osd_start_y ;
* @param ImgWid:image width ;
* @param ImgHei:image height 
* @param yuv_data_type:yuv date type ; 0:YUV420  1:YUV422 ;
* @param pOsdCorParm : osd corlor y u v and outline
* @retval DH_OSD_OK :Success.
*/
int DH_Character_Osd(unsigned char *pDataBuf,
             	        int image_offset, int hz_offset,
		                unsigned char *pChaLibBuf, int offset,
             	        int CharSizeW, int CharSizeH,
             	        int startx, int starty,
		                int ImgWid, int ImgHei,
		                int yuv_data_type,
		                DH_OsdParm *pOsdParm)
{

	//char fontbuf[CharSizeW/8*CharSizeH];   /* 足够大的缓冲区,也可以动态分配 */	
    	char *fontbuf = NULL;
    	int i_pos = 0;
	int i;     /* 控制行 */
	int j;     /* 控制一行中的8 个点 */
	int k;     /* 一行中的第几个"8 个点"了 */
	int nc = 0;/* 到点阵数据的第几个字节了 */

	int pix_x =  0;
	int pix_y =  0;

    fontbuf = (char *)malloc(CharSizeW/8*CharSizeH);
    if(fontbuf == NULL)
    {
        return -1;//DH_OSD_MALLOC_FAILED;
    }
    
	memcpy(fontbuf,(pChaLibBuf + offset), (CharSizeW + 7) / 8 * CharSizeH);		
	for (i = 0; i < CharSizeH; i++)
	{	
		i_pos = i * ImgWid ;    //line add and offset add
		for (k=0; k<(CharSizeW/8); k++)
		{
			for (j=0; j<8; j++)
			{
				if (fontbuf[nc]&(0x80>>j))
				{
					switch(yuv_data_type)
					{
						case DH_OSD_YUV_420 :
							if(*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) < 0x78)
								*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = 240;//pOsdParm->osd_y;
							else
								*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = 0;
							
							pix_x = hz_offset+ startx + 8*k+j ;  //current osd point  x
							pix_y = starty + i ;   //current osd point  y
							//osd u and v value                                                             pOsdParm
							*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2) = 0x80;//pOsdParm->osd_u ;
							*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2+1) = 0x80;//pOsdParm->osd_v ; 			
							break ;
						case DH_OSD_YUV_422 :

							if(*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) < 0x78)
								*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = 240;//pOsdParm->osd_y;
							else
								*(pDataBuf+image_offset+hz_offset+i_pos+8*k+j) = 0;
							//*(pDataBuf+(image_offset+hz_offset+i_pos+8*k+j)*2+1) = pOsdParm->osd_y;
							//osd u and v value
							*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4)= 0x80;//pOsdParm->osd_u ;
							*(pDataBuf+((image_offset+hz_offset+i_pos+8*k+j)*2+1)/4*4+2)= 0x80;//pOsdParm->osd_v ;	
							break ;
						default:
                            			free(fontbuf);
							return -1;//DH_OSD_NOT_SUPPORT_DATE_TPYE ;
					}
				}
			}
			nc++;
		}
	}
    free(fontbuf);
    
	return 0;//DH_OSD_OK ;
}

int DH_Osd_Block_Diagram(unsigned char *pDataBuf, int image_offset,
             	        int startx, int starty,
             	        int DiagramW, int DiagramH,
		        int ImgWid, int ImgHei,
		        int yuv_data_type)
{
    int i_pos = 0;
    int i;     /* 控制行 */
    int j;     /* 控制一行中的8 个点 */
    int k;     /* 一行中的第几个"8 个点"了 */
    int nc = 0;/* 到点阵数据的第几个字节了 */

    int pix_x =  0;
    int pix_y =  0;

    //控制列，从starty开始，到整个框图高度结束
    for(i = 0; i < DiagramH; i++)
    {
        i_pos = i * ImgWid ;    //line add and offset add
        if((i == 0) || (i == (DiagramH - 1)))
        {
            //控制行，从starx开始，到整个框图宽度结束
            for(k = 0; k < DiagramW; k++)
            {
                if(*(pDataBuf+image_offset+i_pos+k) < 0x78)
        			*(pDataBuf+image_offset+i_pos+k) = 240;
        		else
        			*(pDataBuf+image_offset+i_pos+k) = 0;
        		
        		pix_x = startx + k ;  //current osd point  x
        		pix_y = starty + i ;   //current osd point  y
        		//osd u and v value                                                             pOsdParm
        		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2) = 0x80;//pOsdParm->osd_u ;
        		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2+1) = 0x80;//pOsdParm->osd_v ; 	
            }
        }
        else
        {
                if(*(pDataBuf+image_offset+i_pos) < 0x78)
        			*(pDataBuf+image_offset+i_pos) = 240;
        		else
        			*(pDataBuf+image_offset+i_pos) = 0;

                if(*(pDataBuf+image_offset+i_pos + DiagramW - 1) < 0x78)
        			*(pDataBuf+image_offset+i_pos+DiagramW - 1) = 240;
        		else
        			*(pDataBuf+image_offset+i_pos+DiagramW - 1) = 0;
        		
    		pix_x = startx ;  //current osd point  x
    		pix_y = starty + i ;   //current osd point  y
    		//osd u and v value                                                             pOsdParm
    		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2) = 0x80;//pOsdParm->osd_u ;
    		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2+1) = 0x80;//pOsdParm->osd_v ; 	

             pix_x = startx + DiagramW - 1;  //current osd point  x
    		pix_y = starty + i ;   //current osd point  y
    		//osd u and v value                                                             pOsdParm
    		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2) = 0x80;//pOsdParm->osd_u ;
    		*(pDataBuf+ImgWid*ImgHei+pix_y/2*ImgWid+pix_x/2*2+1) = 0x80;//pOsdParm->osd_v ; 
        }
        
    }

    return 0;
}




/**
* @brief osd character function.
* @param character:osd text ;
* @param pDataBuf:osd buf ;
* @param pLibBufCn:chinese lib ;
* @param pLibBufEn:English lib ;
* @param CharSizeW:char width ;
* @param CharSizeH:char height ;
* @param OsdStartX:osd_start_x ;
* @param OsdStartY:osd_start_y ;
* @param ImgWid:image width ;
* @param ImgHei:image height ;
* @param yuv_data_type:yuv date type ; 0:YUV420  1:YUV422 ;
* @param pOsdCorParm : osd corlor y u v and outline 
* @retval DH_OSD_OK :Success.
*/
int DH_Osd(unsigned char *character,
			unsigned char *pDataBuf,
			unsigned char *pLibBufCn, unsigned char *pLibBufEn,
			int CharSizeW, int CharSizeH,
			int OsdStartX, int OsdStartY,
            int ImgWid, int ImgHei,
			int yuv_data_type,
            DH_OsdParm *pOsdParm)
{	
	int status = 0;//DH_OSD_OK;
	int sec = 0, pot = 0;
	int image_offset = 0;
	int hz_offset = 0;

	int Per_En_Num = 0;
	int Per_Cn_Num = 0;	
	long offset = 0;

    if(character == NULL)
	{		
		return -1;//DH_OSD_INPUT_CHARCTER_NULL ; 
	}	
    if(pDataBuf == NULL)
	{		
		return -1;// DH_OSD_INPUT_YUV_DATE_NULL ; 
	}	
 	
    if((pLibBufCn == NULL)||(pLibBufEn == NULL))
	{		
		return -1;// DH_OSD_INPUT_FONT_LIB_NULL ; 
	}	 	

	while(*character)
	{                 
        //在图像位置中的字体偏移量
		hz_offset = CharSizeW/2*Per_En_Num + CharSizeW*Per_Cn_Num;
		//在图像位置中的起点偏移量		
		image_offset = OsdStartY*ImgWid+OsdStartX ;
	
		if(*character < 0x80)   //english char
	        {
	            //在字体库位置中的偏移量
	            offset = (CharSizeW/2 + 7) / 8 * CharSizeH *(*character++);
	            if(pOsdParm->osd_outline == 1)
	            {
	            		status = DH_Character_Osd_OutLine(pDataBuf,
	                                        image_offset, hz_offset,
	                                        pLibBufEn, offset,
	                 				        CharSizeW/2, CharSizeH,
	                 				        OsdStartX, OsdStartY,
	                 				        ImgWid, ImgHei,
	                 				        yuv_data_type,
	                 				        pOsdParm);
	            }
	            else
	            {
	            		status = DH_Character_Osd(pDataBuf,
	                                        image_offset, hz_offset,
	                                        pLibBufEn, offset,
	                 				        CharSizeW/2, CharSizeH,
	                 				        OsdStartX,OsdStartY,
	                 				        ImgWid, ImgHei,
	                 				        yuv_data_type,
	                 				        pOsdParm);
	            }
	            Per_En_Num++;
	        }
		else   //chinese char
		{
			sec = *character++ - 0xa1; /*获得区码*/		
			pot = *character++ - 0xa1; /*获得位码*/				
			offset =  (CharSizeW + 7) / 8 * CharSizeH* (94*sec + pot);	
			if(pOsdParm->osd_outline == 1)
			{
				status = DH_Character_Osd_OutLine(pDataBuf,
                                        image_offset, hz_offset,
                                        pLibBufCn, offset,
                 				        CharSizeW, CharSizeH,
                 				        OsdStartX, OsdStartY,
                 				        ImgWid, ImgHei,
                 				        yuv_data_type,
                 				        pOsdParm);
			}
			else
			{
				status = DH_Character_Osd(pDataBuf,
                                        image_offset, hz_offset,
                                        pLibBufCn, offset,
                 				        CharSizeW, CharSizeH,
                 				        OsdStartX, OsdStartY,
                 				        ImgWid, ImgHei,
                 				        yuv_data_type,
                 				        pOsdParm);
			}			
			Per_Cn_Num++;
		}
	} 
    
	return status ;	
}

/**
* @brief free osd font lib buf function.
* @brief pfontlibBuf :font lib buf, free by user after used.
* @retval DH_OSD_OK
*/
int DH_Osd_Free(char * pfontlibBuf)
{
	if( NULL != pfontlibBuf)
	{
		free(pfontlibBuf);
		pfontlibBuf = NULL ;		
	}
    
	return  0;//DH_OSD_OK ;
}


