/*******************************************************************************
 *  All rights reserved, Copyright (C) vimicro LIMITED 2011                                                     
 * -----------------------------------------------------------------------------
 * [File Name]: 			Include/VIM_TYPEDEF.h
 * [Description]: 			Type define
 *				
 * [Author]: 				Wu Pingping	
 * [Date Of Creation]: 		2012-06-07 15:49
 * [Platform]: 				Independent Of Platform
 * [Note]: 
 *
 * Modification History : 
 * ------------------------------------------------------------------------------
 * Date					Author				Modifications
 * ------------------------------------------------------------------------------
 * 2012-06-07			Wu Pingping			Created
 *******************************************************************************/
/*******************************************************************************
 * This source code has been made available to you by VIMICRO on an
 * AS-IS basis. Anyone receiving this source code is licensed under VIMICRO
 * copyrights to use it in any way he or she deems fit, including copying it,
 * modifying it, compiling it, and redistributing it either with or without
 * modifications. Any person who transfers this source code or any derivative
 * work must include the VIMICRO copyright notice and this paragraph in
 * the transferred software.
 *******************************************************************************/

#ifndef _VIM_TYPE_DEF_H_
#define _VIM_TYPE_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __FPGA__

/* type define */
#ifndef SINT8
#define SINT8			char
#endif

#ifndef SINT16
#define SINT16			short
#endif

#ifndef SINT32
#define SINT32			long
#endif

#ifndef UINT8
#define UINT8			unsigned char	
#endif

#ifndef UINT16
#define UINT16			unsigned short
#endif

#ifndef UINT32
#define UINT32			unsigned long
#endif


#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE	0
#endif


#ifndef NULL
#define  NULL	0
#endif

#ifndef ENABLE
#define ENABLE	1
#endif

#ifndef DISABLE
#define  DISABLE	0
#endif

/* source base address */
#define SRAM_BASE_ADDR				0x20000000
#define FLASH_BASE_ADDR				0x70000000
#define REG_BASE_ADDR				0x60000000
#define DDR_BASE_ADDR				0x80000000




#ifndef _BITDEF_
#define _BITDEF_
enum BITDEFINEenum
{
	BIT0	=	(1u << 0),
	BIT1	=	(1u << 1),
	BIT2	=	(1u << 2),
	BIT3	=	(1u << 3),
	BIT4	=	(1u << 4),
	BIT5	=	(1u << 5),
	BIT6	=	(1u << 6),
	BIT7	=	(1u << 7),
	BIT8	=	(1u << 8),
	BIT9	=	(1u << 9),
	BIT10	=	(1u << 10),
	BIT11	=	(1u << 11),
	BIT12	=	(1u << 12),
	BIT13	=	(1u << 13),
	BIT14	=	(1u << 14),
	BIT15	=	(1u << 15),
	BIT16	=	(1u << 16),
	BIT17	=	(1u << 17),
	BIT18	=	(1u << 18),
	BIT19	=	(1u << 19),
	BIT20	=	(1u << 20),
	BIT21	=	(1u << 21),
	BIT22	=	(1u << 22),
	BIT23	=	(1u << 23),
	BIT24	=	(1u << 24),
	BIT25	=	(1u << 25),
	BIT26	=	(1u << 26),
	BIT27	=	(1u << 27),	
	BIT28	=	(1u << 28),
	BIT29	=	(1u << 29),
	BIT30	=	(1u << 30),
	BIT31	=	(1u << 31)
};
#endif

#ifndef abs
#define	abs(a)		((a)>0 ? (a) : -(a))
#endif

#ifndef min
#define	min(a,b) 		(((a)<(b))?(a):(b))
#endif

#ifndef max
#define	max(a,b) 		(((a)>(b))?(a):(b))
#endif

#ifndef bitMap
#define	bitMap(a) 		(1u<<a)
#endif


enum _BASE_CMD_MODULE_
{
	VIMMOD_CLKRST		= 0x10,
	VIMMOD_MCU			,
	VIMMOD_DMAC			,
	VIMMOD_DDRC			,
	VIMMOD_GPIO			,
	VIMMOD_UART			,
	VIMMOD_I2CEEP		,
	VIMMOD_I2CSNR		,
	VIMMOD_JTAGSNR		,
	VIMMOD_SPISNR		,
	VIMMOD_SPIM			,
	VIMMOD_SPISLV		,
	VIMMOD_PWM			,
	VIMMOD_SIF			,
	VIMMOD_VOUT			,
	VIMMOD_ISP			,
};


typedef enum _VIM_RESULT_
{

	VIM_SUCCEED 					= 0,
		
	VIM_ERROR_RDKBASE				= 10000,
	
	VIM_ERROR_TIME_OUT				= VIM_ERROR_RDKBASE+1,
	VIM_ERROR_PARAMETER				= VIM_ERROR_RDKBASE+2,
	
	VIM_ERROR_SENDCMD_TIMEOUT		= VIM_ERROR_RDKBASE+3,
	VIM_ERROR_SENDCMD_UNDEF			= VIM_ERROR_RDKBASE+4,
	VIM_ERROR_SENDCMD_NORESPONSE	= VIM_ERROR_RDKBASE+5,
	VIM_ERROR_SENDCMD_EXECUTE		= VIM_ERROR_RDKBASE+6,

	VIM_ERROR_MALLOC_FAILED			= VIM_ERROR_RDKBASE+40,

	VIM_ERROR_PERI_OPEN				= VIM_ERROR_RDKBASE+50,
	VIM_ERROR_PERI_READ				= VIM_ERROR_RDKBASE+51,
	VIM_ERROR_PERI_WRITE			= VIM_ERROR_RDKBASE+52,
	
	//flash
	VIM_ERROR_SPIFLASH_UNDEFCMD		= VIM_ERROR_RDKBASE+60,
	VIM_ERROR_SPIFLASH_TIMEOUT		= VIM_ERROR_RDKBASE+61,
	VIM_ERROR_SPIFLASH_NOTALIGNED	= VIM_ERROR_RDKBASE+62,
	VIM_ERROR_SPIFLASH_CHECKSUMERR	= VIM_ERROR_RDKBASE+63,
	VIM_ERROR_SPIFLASH_CHECKCRCERR	= VIM_ERROR_RDKBASE+64,
	VIM_ERROR_SPIFLASH_NOFLASH		= VIM_ERROR_RDKBASE+65,

	/* file operation */
	VIM_FILE_OPEN_MODE_ERR 			= VIM_ERROR_RDKBASE+100,
	VIM_FILE_OPEN_CORE_FAIL 		= VIM_ERROR_RDKBASE+101,
	VIM_FILE_CLOSE_CORE_FAIL 		= VIM_ERROR_RDKBASE+102,
	VIM_FILE_WRITE_CORE_FAIL 		= VIM_ERROR_RDKBASE+103,
	VIM_FILE_READ_CORE_FAIL 		= VIM_ERROR_RDKBASE+104,
	VIM_FILE_PATH_ERROR				= VIM_ERROR_RDKBASE+105,
	VIM_FILE_INVALID				= VIM_ERROR_RDKBASE+106,
    VIM_FILE_CRC_ERROR              = VIM_ERROR_RDKBASE+107,
    VIM_FILE_VER_ERROR              = VIM_ERROR_RDKBASE+108,
    VIM_FILE_HEAD_ERROR             = VIM_ERROR_RDKBASE+109,
    
	VIM_ERROR_GET_MODULE_CLK		= VIM_ERROR_RDKBASE+150,
	VIM_ERROR_SET_MODULE_CLK		= VIM_ERROR_RDKBASE+151,

	VIM_ERROR_DMAC_TRANSFER			= VIM_ERROR_RDKBASE+160,

	//vout
	VIM_ERROR_CROP_FAILED			= VIM_ERROR_RDKBASE+200,
	VIM_ERROR_DOWNSIZER_FAILED		= VIM_ERROR_RDKBASE+201,
	VIM_ERROR_UNSUPPORTTED_VIDEO	= VIM_ERROR_RDKBASE+202,
	VIM_ERROR_VOUT_WRONG_PARAM 		= VIM_ERROR_RDKBASE+203,
	VIM_ERROR_VOUT_LACK_FONT 		= VIM_ERROR_RDKBASE+204,  
	VIM_ERROR_CANNOT_GET_VIDEO 		= VIM_ERROR_RDKBASE+205,

	/* isp operation error code */
	VIM_ERROR_SAVE_ATTRIBUTE		= VIM_ERROR_RDKBASE+500,
	VIM_ERROR_GET_DEFAULT_ATTRIBUTE		= VIM_ERROR_RDKBASE+501,
	VIM_ERROR_GET_CURRENT_ATTRIBUTE		= VIM_ERROR_RDKBASE+502,
    VIM_ERROR_GET_GENERAL_ATTRIBUTE     = VIM_ERROR_RDKBASE+503,
	VIM_ERROR_MAX					,	
}VIM_RESULT;


typedef enum _SYS_COM_CHANNEL_
{
	CHANNEL_I2C		= 0xa0,
	CHANNEL_SPI		= 0xa1,

	CHANNEL_NULL	= 0xff,
}VIM_SYS_COM_E;


#ifdef __cplusplus
}
#endif




#endif

