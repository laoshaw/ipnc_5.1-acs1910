; --COPYRIGHT--,EPL
;   Copyright (c) 2008 Texas Instruments and others.
;   All rights reserved. This program and the accompanying materials
;   are made available under the terms of the Eclipse Public License v1.0
;   which accompanies this distribution, and is available at
;   http://www.eclipse.org/legal/epl-v10.html
;  
;   Contributors:
;       Texas Instruments - initial implementation
;  
; --/COPYRIGHT--
;******************************************************************************
;* BOOT  v4.5.0                                                               *
;* Copyright (c) 1996-2008 Texas Instruments Incorporated                     *
;******************************************************************************

;****************************************************************************
;* BOOT.ASM
;*
;* THIS IS THE INITAL BOOT ROUTINE FOR TMS470 C++ PROGRAMS.
;* IT MUST BE LINKED AND LOADED WITH ALL C++ PROGRAMS.
;* 
;* THIS MODULE PERFORMS THE FOLLOWING ACTIONS:
;*   1) ALLOCATES THE STACK AND INITIALIZES THE STACK POINTER
;*   2) CALLS AUTO-INITIALIZATION ROUTINE
;*   3) CALLS THE FUNCTION MAIN TO START THE C++ PROGRAM
;*   4) CALLS THE STANDARD EXIT ROUTINE
;*
;* THIS MODULE DEFINES THE FOLLOWING GLOBAL SYMBOLS:
;*   1) __stack     STACK MEMORY AREA
;*   2) _c_int00    BOOT ROUTINE
;*
;****************************************************************************
   .if  __TI_TMS470_V7M3__
	.thumbfunc _c_int00
   .else
	.armfunc _c_int00
   .endif

;****************************************************************************
; Accomodate different lowerd names in different ABIs
;****************************************************************************
   .if   __TI_EABI_ASSEMBLER
        .asg	_args_main,   ARGS_MAIN_RTN
        .asg	exit,         EXIT_RTN
        .asg    main_func_sp, MAIN_FUNC_SP
	.asg	xdc_runtime_Startup_reset__I,	RESET_FUNC
   .elseif __TI_ARM9ABI_ASSEMBLER | .TMS470_32BIS
        .asg	__args_main,   ARGS_MAIN_RTN
        .asg	_exit,         EXIT_RTN
        .asg    _main_func_sp, MAIN_FUNC_SP
	.asg	_xdc_runtime_Startup_reset__I,	RESET_FUNC
   .else
        .asg	$_args_main,   ARGS_MAIN_RTN
        .asg	$exit,         EXIT_RTN
        .asg    _main_func_sp, MAIN_FUNC_SP
	.asg	_xdc_runtime_Startup_reset__I,	RESET_FUNC
   .endif

   .if .TMS470_16BIS

;****************************************************************************
;*  16 BIT STATE BOOT ROUTINE                                               *
;****************************************************************************

   .if __TI_TMS470_V7M3__
	.state16
   .else
	.state32
   .endif

	.global	__stack
;***************************************************************
;* DEFINE THE USER MODE STACK (DEFAULT SIZE IS 512)               
;***************************************************************
__stack:.usect	".stack", 0, 4

	.global	_c_int00

   .if __TI_TMS470_V7M3__
;
; Add minimal boot vector table
;
	.global __STACK_SIZE

	.sect	".bootVecs"
	.long	__stack		; This really should be __stack + __STACKSIZE
				; but since c_int00 sets the sp anyway its
				; ok
	.long	_c_int00

	.symdepend ".bootVecs", ".text"

	.text
   .endif

;***************************************************************
;* FUNCTION DEF: _c_int00                                      
;***************************************************************
_c_int00: .asmfunc

	.if !__TI_TMS470_V7M3__
	.if __TI_NEON_SUPPORT__ | __TI_VFP_SUPPORT__
        ;*------------------------------------------------------
	;* SETUP PRIVILEGED AND USER MODE ACCESS TO COPROCESSORS
	;* 10 AND 11, REQUIRED TO ENABLE NEON/VFP      
	;* COPROCESSOR ACCESS CONTROL REG 
	;* BITS [23:22] - CP11, [21:20] - CP10
	;* SET TO 0b11 TO ENABLE USER AND PRIV MODE ACCESS
        ;*------------------------------------------------------
	MRC      p15,#0x0,r0,c1,c0,#2
        MOV      r3,#0xf00000
	ORR      r0,r0,r3
        MCR      p15,#0x0,r0,c1,c0,#2

        ;*------------------------------------------------------
	; SET THE EN BIT, FPEXC[30] TO ENABLE NEON AND VFP
        ;*------------------------------------------------------
      	MOV      r0,#0x40000000
        FMXR     FPEXC,r0
	.endif

	;------------------------------------------------------
	;* SET TO SYSTEM MODE
        ;*------------------------------------------------------
        MRS     r0, cpsr
        BIC     r0, r0, #0x1F  ; CLEAR MODES
        ORR     r0, r0, #0x1F  ; SET SYSTEM MODE
        MSR     cpsr_cf, r0

        ;*------------------------------------------------------
	;* CHANGE TO 16 BIT STATE
        ;*------------------------------------------------------
	ADD	r0, pc, #1
	BX	r0

	.state16
	.endif

	;*------------------------------------------------------
        ;* INITIALIZE THE USER/SYSTEM MODE STACK                      
        ;*------------------------------------------------------
	LDR     r0, c_stack
	MOV	sp, r0
        LDR     r0, c_STACK_SIZE
	ADD	sp, r0

	;*-----------------------------------------------------
	;* ALIGN THE STACK TO 64-BITS IF EABI.
	;*-----------------------------------------------------
	.if __TI_EABI_ASSEMBLER
	MOV	r7, sp
	MOV	r0, #0x07
	BIC     r7, r0         ; Clear upper 3 bits for 64-bit alignment.
	MOV	sp, r7
	.endif

	;*-----------------------------------------------------
	;* SAVE CURRENT STACK POINTER FOR SDP ANALYSIS
	;*-----------------------------------------------------
	LDR	r0, c_mf_sp
	MOV	r7, sp
	STR	r7, [r0]

	LDR	r0, c_reset
	CMP	r0, #0
	BEQ	_no_reset_
	MOV	r1, pc
	ADD	r1, r1, #5
	MOV	lr, r1
	BX	r0

_no_reset_:

        ;*------------------------------------------------------
        ;* Perform all the required initilizations:
        ;*   - Process BINIT Table
        ;*   - Perform C auto initialization
        ;*   - Call global constructors 
        ;*------------------------------------------------------
        BL      __TI_auto_init

        ;*------------------------------------------------------
	;* CALL APPLICATION                                     
        ;*------------------------------------------------------
        BL      ARGS_MAIN_RTN

        ;*------------------------------------------------------
	;* IF APPLICATION DIDN'T CALL EXIT, CALL EXIT(1)
        ;*------------------------------------------------------
        MOV     r0, #1
        BL      EXIT_RTN

        ;*------------------------------------------------------
	;* DONE, LOOP FOREVER
        ;*------------------------------------------------------
L1:     B	L1
	.endasmfunc

   .else           ; !.TMS470_16BIS

;****************************************************************************
;*  32 BIT STATE BOOT ROUTINE                                               *
;****************************************************************************

	.global	__stack
;***************************************************************
;* DEFINE THE USER MODE STACK (DEFAULT SIZE IS 512)            
;***************************************************************
__stack:.usect	".stack", 0, 4

	.global	_c_int00
;***************************************************************
;* FUNCTION DEF: _c_int00                                      
;***************************************************************
_c_int00: .asmfunc

	.if __TI_NEON_SUPPORT__ | __TI_VFP_SUPPORT__
        ;*------------------------------------------------------
	;* SETUP PRIVILEGED AND USER MODE ACCESS TO COPROCESSORS
	;* 10 AND 11, REQUIRED TO ENABLE NEON/VFP      
	;* COPROCESSOR ACCESS CONTROL REG 
	;* BITS [23:22] - CP11, [21:20] - CP10
	;* SET TO 0b11 TO ENABLE USER AND PRIV MODE ACCESS
        ;*------------------------------------------------------
	MRC      p15,#0x0,r0,c1,c0,#2
        MOV      r3,#0xf00000
	ORR      r0,r0,r3
        MCR      p15,#0x0,r0,c1,c0,#2

        ;*------------------------------------------------------
	; SET THE EN BIT, FPEXC[30] TO ENABLE NEON AND VFP
        ;*------------------------------------------------------
      	MOV      r0,#0x40000000
        FMXR     FPEXC,r0
	.endif

        ;*------------------------------------------------------
	;* SET TO SYSTEM MODE
        ;*------------------------------------------------------
        MRS     r0, cpsr
        BIC     r0, r0, #0x1F  ; CLEAR MODES
        ORR     r0, r0, #0x1F  ; SET SYSTEM MODE
        MSR     cpsr_cf, r0

        ;*------------------------------------------------------
        ;* INITIALIZE THE USER/SYSTEM MODE STACK                      
        ;*------------------------------------------------------
	LDR     sp, c_stack
        LDR     r0, c_STACK_SIZE
	ADD	sp, sp, r0

	;*-----------------------------------------------------
	;* ALIGN THE STACK TO 64-BITS IF EABI.
	;*-----------------------------------------------------
	.if __TI_EABI_ASSEMBLER
	BIC     sp, sp, #0x07  ; Clear upper 3 bits for 64-bit alignment.
	.endif

	;*-----------------------------------------------------
	;* SAVE CURRENT STACK POINTER FOR SDP ANALYSIS
	;*-----------------------------------------------------
	LDR	r0, c_mf_sp
	STR	sp, [r0]
                
	LDR	r0, c_reset
	CMP	r0, #0
	BEQ	_no_reset_
	MOV	lr, pc
	BX	r0

_no_reset_:

        ;*------------------------------------------------------
        ;* Perform all the required initilizations:
        ;*   - Process BINIT Table
        ;*   - Perform C auto initialization
        ;*   - Call global constructors 
        ;*------------------------------------------------------
        BL      __TI_auto_init

        ;*------------------------------------------------------
	;* CALL APPLICATION                                     
        ;*------------------------------------------------------    
        BL      ARGS_MAIN_RTN

        ;*------------------------------------------------------
	;* IF APPLICATION DIDN'T CALL EXIT, CALL EXIT(1)
        ;*------------------------------------------------------
        MOV     R0, #1
        BL      EXIT_RTN

        ;*------------------------------------------------------
	;* DONE, LOOP FOREVER
        ;*------------------------------------------------------
L1:     B	L1
	.endasmfunc

   .endif    ; !.TMS470_16BIS

;***************************************************************
;* CONSTANTS USED BY THIS MODULE
;***************************************************************
c_stack		.long    __stack
c_STACK_SIZE  	.long    __STACK_SIZE
c_mf_sp	        .long    MAIN_FUNC_SP
c_reset       	.long    RESET_FUNC

	.if __TI_EABI_ASSEMBLER
        .data
        .align 4
_stkchk_called:
        .field          0,32
        .else
        .sect   ".cinit"
        .align  4
        .field          4,32
        .field          _stkchk_called+0,32
        .field          0,32

        .bss    _stkchk_called,4,4
        .symdepend ".cinit", ".bss"
        .symdepend ".cinit", ".text"
        .symdepend ".bss", ".text"
	.endif

;******************************************************
;* UNDEFINED REFERENCES                               *
;******************************************************
	.global _stkchk_called
	.global	__STACK_SIZE
	.global ARGS_MAIN_RTN
	.global MAIN_FUNC_SP
	.global	EXIT_RTN
	.global	RESET_FUNC
        .global __TI_auto_init

	.end
