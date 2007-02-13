//============================================================================================
/**
 * @file	tamada_internal.h
 * @brief	デバッグ＆テスト用アプリ（玉田専用）
 * @author	tamada
 * @date	2006.11.28
 */
//============================================================================================

#ifndef	__TAMADA_INTERNAL_H__
#define	__TAMADA_INTERNAL_H__

#include "gflib.h"
#include "procsys.h"
#include "ui.h"

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
typedef void (*GFL_INTR_FUNC)(void *);

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	u32 debug_heap_id;
	UISYS * uisys;					///<UIへのポインタ
	GFL_INTR_FUNC vblank_func;
	void * vblank_work;
}DEBUG_TAMADA_CONTROL;




//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GFL_INTR_SetVblankFunc(GFL_INTR_FUNC func, void * work);


extern const GFL_PROC_DATA DebugTamadaMainProcData;
extern const GFL_PROC_DATA OamKeyDemoProcData;
extern const GFL_PROC_DATA DebugTamadaSubProcData1;
extern const GFL_PROC_DATA DebugTamadaSubProcData2;
extern const GFL_PROC_DATA DebugTamadaSubProcData3;

#endif	/* __TAMADA_INTERNAL_H__ */
