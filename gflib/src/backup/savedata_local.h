//============================================================================================
/**
 * @file	savedata_local.h
 * @brief
 * @date	2006.04.17
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================

#ifndef	__SAVEDATA_LOCAL_H__
#define	__SAVEDATA_LOCAL_H__

#include "heap.h"

//=============================================================================
//=============================================================================


typedef struct _SVDT SVDT;

//=============================================================================
//=============================================================================


extern SVDT * SVDT_Create(HEAPID heap_id, const GFL_SAVEDATA_TABLE * table, u32 table_max,
		u32 savearea_size, u32 footer_size);
extern void SVDT_Delete(SVDT * svdt);

extern void SVDT_ClearWork(u8 * svwk, const SVDT * svdt);
extern u32 SVDT_GetPageOffset(SVDT * svdt, GFL_SVDT_ID gmdataid);
extern u32 SVDT_GetWorkSize(const SVDT * svdt);


#endif	/* __SAVEDATA_LOCAL_H__ */
