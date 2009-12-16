//============================================================================
/**
 *
 *	@file		intro_msg.h
 *	@brief  イントロ メッセージ処理
 *	@author	hosaka genya
 *	@data		2009.12.15
 *
 */
//============================================================================
#pragma once

#include "print/wordset.h"

typedef struct _INTRO_MSG_WORK INTRO_MSG_WORK;

// WORDSET用コールバック関数宣言
typedef void (*WORDSET_CALLBACK)(WORDSET*, void*);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================


extern INTRO_MSG_WORK* INTRO_MSG_Create( u16 msg_dat_id, HEAPID heap_id );

extern void INTRO_MSG_Exit( INTRO_MSG_WORK* wk );

extern void INTRO_MSG_Main( INTRO_MSG_WORK* wk );

extern void INTRO_MSG_SetPrint( INTRO_MSG_WORK* wk, int str_id, WORDSET_CALLBACK callback_func, void* callback_arg );

extern BOOL INTRO_MSG_PrintProc( INTRO_MSG_WORK* wk );

