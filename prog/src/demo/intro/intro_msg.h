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
#include "system/bmp_menulist.h" // for BMPMENU_LIST_WORK

typedef struct _INTRO_MSG_WORK INTRO_MSG_WORK;

// WORDSET用コールバック関数宣言
typedef void (*WORDSET_CALLBACK)(WORDSET*, void*);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//-------------------------------------
///	リストの選択結果
//=====================================
typedef enum
{
  INTRO_LIST_SELECT_NONE,
  INTRO_LIST_SELECT_DECIDE,
  INTRO_LIST_SELECT_CANCEL,
} INTRO_LIST_SELECT;

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	リストの要素
//==============================================================
typedef struct {
  int str_id;
  int param;
} INTRO_LIST_DATA;

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================

// main
extern INTRO_MSG_WORK* INTRO_MSG_Create( HEAPID heap_id );
extern void INTRO_MSG_Exit( INTRO_MSG_WORK* wk );
extern void INTRO_MSG_Main( INTRO_MSG_WORK* wk );
extern void INTRO_MSG_LoadGmm( INTRO_MSG_WORK* wk, GflMsgLoadType type, u16 msg_dat_id );

// stream
extern void INTRO_MSG_SetPrint( INTRO_MSG_WORK* wk, int str_id );
extern BOOL INTRO_MSG_PrintProc( INTRO_MSG_WORK* wk );

// list
extern void INTRO_MSG_LIST_Start( INTRO_MSG_WORK* wk, const INTRO_LIST_DATA *cp_tbl, u32 tbl_max, BOOL is_cansel );
extern void INTRO_MSG_LIST_Finish( INTRO_MSG_WORK *wk );
extern void INTRO_MSG_LIST_Main( INTRO_MSG_WORK *wk );
extern INTRO_LIST_SELECT INTRO_MSG_LIST_IsDecide( INTRO_MSG_WORK *wk, u32 *p_select );

// データ公開
extern WORDSET* INTRO_MSG_GetWordSet( INTRO_MSG_WORK* wk );

