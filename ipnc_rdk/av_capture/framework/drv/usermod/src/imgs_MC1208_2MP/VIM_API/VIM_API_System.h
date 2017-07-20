/*******************************************************************************
 *  All rights reserved, Copyright (C) vimicro LIMITED 2011                                                     
 * -----------------------------------------------------------------------------
 * [File Name]: 			API/VIM_API_System.h
 * [Description]: 			system
 *				
 * [Author]: 				
 * [Date Of Creation]: 		2012-06-07 10:49
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
#ifndef _VIM_API_SYSTEM_H_
#define _VIM_API_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _SYSTEM_CLK_MODULE_
{
	CLK_MCU			= 0x0,
	CLK_I2CSLV		,
	CLK_SPISLV		,
	CLK_SPIBOOT		,
	CLK_SPIBOOTAXI	,
	CLK_SAXI		,
	CLK_DMAC		,
	CLK_APB			,
	CLK_I2CSNR		,
	CLK_I2CEEP		,
	CLK_UART0		,
	CLK_UART1		,
	CLK_SPISNR		,
	CLK_SPIMST		,
	CLK_DDRC		,
	CLK_CS			,
	CLK_PWM			,
	CLK_VAXI		,
	CLK_VOUT		,
	CLK_VIDEO		,
	CLK_JTAG0		,
	CLK_JTAG1		,

	CLK_END			,
}VIM_CLK_MODULE_E;

typedef enum _SYSTEM_FW_RUNPOS_
{
	FW_RUNPOS_DDR = 1,
	FW_RUNPOS_SRAM,	
	FW_RUNPOS_NONE,
}VIM_FW_RUNPOS_E;

//rising edge/high level or falling edge/low level trigger interrupt
typedef enum _SYSTEM_GPIOTRIG_MODE
{
    FALLINGEDGE_LOWLEVEL = 0,
	RISINGEDGE_HIGHLEVEL
}VIM_GPIOTRIG_MODE_E;

#define VC0710_CHIP_REG_ADDR_CHECK(x) (0x20000000<=(x) && (x)<=0x6001FFFF)

void VIM_SystemInit(void);
VIM_RESULT VIM_OpenChannel(VIM_SYS_COM_E channel, UINT32 rate, char *devNode);
VIM_RESULT VIM_AutoOpenChannel(UINT32 rate);
VIM_RESULT VIM_CloseCurChannel(void);

VIM_RESULT VIM_SetModuleClk(VIM_CLK_MODULE_E mod,UINT32 rate);
UINT32 VIM_GetModuleClk(VIM_CLK_MODULE_E mod);

VIM_RESULT VIM_MemToMem(UINT32 srcAddr, UINT32 dstAddr,UINT32 len,UINT8 burstLen);
VIM_RESULT VIM_ReadMemToFile(char *filePath,UINT32 memAddr,UINT32 len);
VIM_RESULT VIM_WriteMemFromFile(char *filePath,UINT32 memAddr,UINT32 len);
VIM_RESULT VIM_DownRunFW(char *filePath,VIM_FW_RUNPOS_E pos);

VIM_RESULT VIM_WriteReg(int reg_addr, int value);
VIM_RESULT VIM_SetGPIOB13Trig(VIM_GPIOTRIG_MODE_E value);

VIM_RESULT VIM_I2CEEInit(UINT8 Mode, UINT8 WorkMode, UINT16 Speed);
VIM_RESULT VIM_I2CEERead(UINT8 SlaveAddr, UINT8 SlaveSubAddrMode, UINT32 SlaveSubAddr, UINT8 DataNumToRead, UINT8 *RevBuf, UINT8 *ReadNum);

VIM_RESULT VIM_Debug(UINT32 Set_Value, UINT32 *value);
#ifdef __cplusplus
}
#endif

#endif
