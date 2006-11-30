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

#define	DEBUG_TAMADA

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void DebugTamadaInit(u32 heap_id);
extern void DebugTamadaMain(void);
extern void DebugTamadaExit(void);

#ifdef	DEBUG_TAMADA
#define	DEBUG_INIT(heap_id)	DebugTamadaInit(heap_id)
#define	DEBUG_MAIN			DebugTamadaMain
#define	DEBUG_EXIT			DebugTamadaExit
#else
#define	DEBUG_INIT(heap_id)	/* */
#define	DEBUG_MAIN			/* */
#define	DEBUG_EXIT			/* */
#endif


#endif	/* __DEBUG_TAMADA_H__ */
