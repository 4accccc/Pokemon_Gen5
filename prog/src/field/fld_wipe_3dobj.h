//============================================================================================
/**
 * @file	fld_wipe_3dobj.h
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#pragma once

typedef struct _FLD_WIPEOBJ	FLD_WIPEOBJ;

extern FLD_WIPEOBJ* FLD_WIPEOBJ_Create( HEAPID heapID ); 
extern void					FLD_WIPEOBJ_Delete( FLD_WIPEOBJ* fw );
extern void					FLD_WIPEOBJ_Main( FLD_WIPEOBJ* fw, fx32 scale, u8 alpha );

