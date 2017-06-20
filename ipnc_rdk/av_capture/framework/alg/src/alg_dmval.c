#include <alg_priv.h>

#include <osa_cmem.h>
#include <osa_file.h>

#include <alg_dmval.h>
//#include <DMVAL.h>

DMVALstatus ALG_dmvalCreate(ALG_DmvalCreateParams * params)
{

	DMVALstatus libStatus;
	int  size;
	void *tmpA;
	void *tmpB;
	void *tmpA_PhysAddr, *tmpB_PhysAddr;
	int bytesMemBufAllocated = 0;

	/////////////////////////////////////////////////////////////////////////////
	//
	// CREATE INSTANCE of DMVAL. The call populates handle variables
	// with the memory required for permanent and output buffers
	//
	/////////////////////////////////////////////////////////////////////////////
	libStatus = DMVAL_create(params->dmvalHdl,
		params->detectMode,
		params->imgType,
		params->sensitiveness,
		//params->camAngle,
		//params->camLens,
		params->inCols,
		params->inRows,
		params->inStride);

	if (libStatus == DMVAL_ERR_INSTANCE_CREATE_FAILED)
	{
		OSA_ERROR("ALG_dmvalCreate()\n");
	}


	/////////////////////////////////////////////////////////////////////////////
	//
	// ALLOCATE REQUESITE MEMORY blocks (permanent & output) for DMVAL 
	//
	/////////////////////////////////////////////////////////////////////////////
	size = params->dmvalHdl->bytesMemBufOutput;  
	tmpB = OSA_cmemAllocCached(size, 32);

	printf("\n> DMVA APP: Allocated %d bytes for DMVAL output memory @ 0x%x", size, (unsigned int)tmpB);
	fflush(stdout);
	if ( tmpB == NULL ) 
	{
		OSA_ERROR("\n> DMVA APP: Error allocating bytes for DMVAL output!!\n");
	}
	bytesMemBufAllocated  += size;

	size = params->dmvalHdl->bytesMemBufPermanent;
	tmpA = OSA_cmemAllocCached(size, 32);

	printf("\n> DMVA APP: Allocated %d bytes for DMVAL internal memory @ 0x%x", size, (unsigned int)tmpA);
	fflush(stdout);
	if ( tmpA == NULL )
	{
		OSA_ERROR("\n> DMVA APP: Error allocating bytes for DMVAL instance\n");
	}
	bytesMemBufAllocated  += size;


	/////////////////////////////////////////////////////////////////////////////
	//
	// ASSIGN allocated memory to DMVAL instance, which it partitions as required 
	// for internal use
	//
	/////////////////////////////////////////////////////////////////////////////
	printf("\n> DMVA APP: Calling DMVAL_configure ");fflush(stdout);
	fflush(stdout);

	libStatus = DMVAL_configure(params->dmvalHdl, tmpA, tmpB);
	if (libStatus != DMVAL_NO_ERROR)
	{
		OSA_ERROR("DMVAL_configure()\n");
	}
	/////////////////////////////////////////////////////////////////////////////
	//
	// ALL DMVAL API FUNCTIONS CALLED ABOVE SHOULD ONLY BE CALLED ONCE; THOSE 
	// BELOW CAN BE CALLED AS REQUIRED TO INITIALIZE OR RE-INITIALIZE A MODULE
	//
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//
	// INITIALIZE allocated memory assigned to selected DMVAL modules
	//
	/////////////////////////////////////////////////////////////////////////////
	libStatus = DMVAL_initModule(params->dmvalHdl);

	if (libStatus != DMVAL_NO_ERROR)
	{
		OSA_ERROR("DMVAL_initModule()\n");
	}

	printf("\n> DMVA APP: DMVAL modules initialized.");
	return(libStatus);

}


int ALG_dmvalDelete( DMVALhandle* dmvalHdl)
{
	printf("\n> DMVA APP: Deallocating via dmvalHdl->bytesMemBufPermanent = %d bytes, address = 0x%x\n", dmvalHdl->bytesMemBufPermanent, dmvalHdl->memBufPermanent);
	printf("\n> DMVA APP: Deallocating via dmvalHdl->bytesMemBufOutput = %d bytes, address = 0x%x\n", dmvalHdl->bytesMemBufOutput, dmvalHdl->output);

	OSA_cmemFree(dmvalHdl->memBufPermanent);
	OSA_cmemFree((void *)dmvalHdl->output);

	DMVAL_delete(dmvalHdl);

	return OSA_SOK;

}

