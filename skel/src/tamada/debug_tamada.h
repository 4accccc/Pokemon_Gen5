//============================================================================================
/**
 * @file	debug_tamada.h
 * @brief	デバッグ＆テスト用アプリ（玉田専用）
 * @author	tamada
 * @date	2006.11.28
 */
//============================================================================================

#ifndef	__DEBUG_TAMADA_H__
#define	__DEBUG_TAMADA_H__

#include "gflib.h"
#include "procsys.h"

extern void DebugTamadaInit(u32 heap_id);
extern void DebugTamadaMain(void);
extern void DebugTamadaExit(void);

#endif	/* __DEBUG_TAMADA_H__ */
