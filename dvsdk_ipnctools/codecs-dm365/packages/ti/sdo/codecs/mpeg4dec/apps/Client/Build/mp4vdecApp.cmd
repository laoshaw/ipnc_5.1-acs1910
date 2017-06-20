/****************************************************************************/
/*  lnk.cmd   v1.01                                                         */
/*  Copyright (c) 1996-1997  Texas Instruments Incorporated                 */
/****************************************************************************/
/* -cr */
-c
-heap  0x1000000 /*0x48FB00*/
-stack 0x2700
__F_STACK_SIZE = 0x100;                 /* IRQ Stack */
__S_STACK_SIZE = 0x100;                 /* SUPERVISOR Stack */

MEMORY
{
	RESET	   :            o = 0x00000000     l = 0x00000004
	VECS       :            o = 0x00000004     l = 0x000000FB
	ITCM	   :            o = 0x00000100     l = 0x00003F00  /* IRAM 0 */
	DTCM_SHARE :            o = 0x82900000     l = 0x100
	DTCM_DMA   :            o = 0x82900100     l = 0x3000
	DTCM	   : 			o = 0x00014000     l = 0x00004000  /* IRAM 1 */
	EXTMEM	   :            o = 0x80000000     l = 0x2000000
	EXTMEMDATA :            o = 0x82000000     l = 0x200000
}

SECTIONS
{
    .vectors        >       VECS
    .text           >       EXTMEM
    .stack          >       EXTMEM
    .far            >       EXTMEM
    .switch         >       EXTMEM 
    .tables         >       EXTMEM
    .data           >       EXTMEM
    .bss            >       EXTMEM
    .sysmem         >       EXTMEM
    .cinit          >       EXTMEM
    .const          >       EXTMEM
    .cio            >       EXTMEM 
    .input_buffer   >       EXTMEMDATA
    .display_buffer >       EXTMEM
    .ref_buffer     >       EXTMEM
    .outBuffer      >       EXTMEM
    .DTCM_share		>       DTCM_SHARE
    .DTCM_dma		>       DTCM_DMA
    .ext_mem		>		EXTMEM
	.handler        >       ITCM
	.intvecs        >       VECS
	.sstack         >       EXTMEM
	.fstack         >       EXTMEM
	.mmutable	    >	    DTCM
}
