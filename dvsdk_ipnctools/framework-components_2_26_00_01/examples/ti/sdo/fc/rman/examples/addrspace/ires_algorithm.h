/* 
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/**
 *  @file       ti/sdo/fc/rman/examples/addrspace/ires_algorithm.h
 *
 *  @brief      Definitions for dummy implementation of the IRES interfaces
 *              by an algorithm
 *
 */

#include <ti/xdais/ires.h>
#include <ti/sdo/fc/ires/addrspace/ires_addrspace.h>

/*
 * @brief  Number of resources the algorithm will request
 */
#define NUM_RESOURCES   2

/*
 * @brief  Number of memory resources required by IALG implementation
 */
#define NUM_ALLOC       1

/*
 *  ======== DUMRES_TI_Obj ========
 *  Implementation of the IALG interface for BUFALG
 */
typedef struct DUMRES_TI_Obj {
    IALG_Obj                    alg;
    IRES_ADDRSPACE_Handle       resource[NUM_RESOURCES];
    short                       resourceState[NUM_RESOURCES];
    IRES_ADDRSPACE_ProtocolArgs protocolArgs[NUM_RESOURCES];
} DUMRES_TI_Obj;

typedef DUMRES_TI_Obj * DUMRES_TI_Handle;

/*
 *  ======== IDUMRES_Params ========
 *  Creation paramters for the DUMRES object
 */
typedef struct IDUMRES_Params {
    Int                         size;
    Int                         yieldFlag;
    Int                         taskId;
} IDUMRES_Params;

/*
 *  ======== IDUMRES_Obj ========
 *  This structure must be first field of all DUMRES instance objects
 */
typedef struct IDUMRES_Obj {
    struct IDUMRES_Fxns *fxns;
} IDUMRES_Obj;

typedef IDUMRES_Obj * IDUMRES_Handle;

/*
 *  ======== IDUMRES_Fxns ========
 */
typedef struct IDUMRES_Fxns {
    IALG_Fxns alg;
} IDUMRES_Fxns;

/*
 *  TI's implementation of the IDUMRES interface
 */
extern IDUMRES_Fxns DUMRES_TI_IDUMRES;

/*
 * @brief   IRES interface of the algorithm object
 */
extern IRES_Fxns DUMRES_TI_IRES;

/*
 * @brief   IALG interface of the algorithm object
 */
extern IALG_Fxns DUMRES_TI_IALG;

/*
 * Paramter values for DUMRES instance objects
 */
extern IDUMRES_Params IDUMRES_PARAMS;
/*
 *  @(#) ti.sdo.fc.rman.examples.addrspace; 1, 0, 0,22; 12-1-2010 17:25:34; /db/atree/library/trees/fc/fc-p01x/src/ xlibrary

 */

