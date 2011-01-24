//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_util.h
 *	@brief  ローカルライブキャスター汎用モジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/game_data.h"
#include "local_tvt_message.h"
#include "local_tvt_type_def.h"
//=============================================================================
/**
 *					タイムゾーン
*/
//=============================================================================
extern LOCAL_TVT_TIME_ZONE LOCAL_TVT_UTIL_GetTimeZone( const GAMEDATA *cp_gamedata );

//=============================================================================
/**
 *					背景素材
*/
//=============================================================================
typedef struct _LOCAL_TVT_BG LOCAL_TVT_BG;

extern LOCAL_TVT_BG * LOCAL_TVT_BG_Load( GAMEDATA *p_gamedata, HEAPID heapID );
extern void LOCAL_TVT_BG_UnLoad( LOCAL_TVT_BG *p_wk );

//=============================================================================
/**
 *					下画面バー(リターンマークがあるだけ)
*/
//=============================================================================
typedef struct _LOCAL_TVT_TOUCHBAR LOCAL_TVT_TOUCHBAR;

extern LOCAL_TVT_TOUCHBAR * LOCAL_TVT_TOUCHBAR_Init( GFL_CLUNIT *p_clunit, HEAPID heapID );
extern void LOCAL_TVT_TOUCHBAR_Exit( LOCAL_TVT_TOUCHBAR *p_wk );
extern void LOCAL_TVT_TOUCHBAR_Main( LOCAL_TVT_TOUCHBAR *p_wk );
extern BOOL LOCAL_TVT_TOUCHBAR_IsTrg( const LOCAL_TVT_TOUCHBAR *cp_wk );
extern BOOL LOCAL_TVT_TOUCHBAR_IsEnd( const LOCAL_TVT_TOUCHBAR *cp_wk );
extern void LOCAL_TVT_TOUCHBAR_SetActive( LOCAL_TVT_TOUCHBAR *p_wk, BOOL is_active);
extern void LOCAL_TVT_TOUCHBAR_SetVisible( LOCAL_TVT_TOUCHBAR *p_wk, BOOL is_visible );

//=============================================================================
/**
 *					シーケンス管理
*/
//=============================================================================
//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _LOCAL_TVT_SEQ LOCAL_TVT_SEQ;

//-------------------------------------
///	シーケンス関数
//=====================================
typedef void (*LOCAL_TVT_SEQ_FUNCTION)( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	パブリック
//=====================================
extern LOCAL_TVT_SEQ *LOCAL_TVT_SEQ_Init( void *p_wk_adrs, LOCAL_TVT_SEQ_FUNCTION seq_function, HEAPID heapID );
extern void LOCAL_TVT_SEQ_Exit( LOCAL_TVT_SEQ *p_wk );
extern void LOCAL_TVT_SEQ_Main( LOCAL_TVT_SEQ *p_wk );
extern BOOL LOCAL_TVT_SEQ_IsEnd( const LOCAL_TVT_SEQ *cp_wk );

extern void LOCAL_TVT_SEQ_SetNextCore( LOCAL_TVT_SEQ *p_wk, LOCAL_TVT_SEQ_FUNCTION seq_function );

#define LOCAL_TVT_SEQ_SetNext( x, y ) \
        LOCAL_TVT_SEQ_SetNextCore( x, y ); \
        OS_TPrintf( "%s%d\n", __FILE__, __LINE__ )

extern void LOCAL_TVT_SEQ_End( LOCAL_TVT_SEQ *p_wk );

//=============================================================================
/**
 *					リスト
*/
//=============================================================================
#define LOCAL_TVT_LIST_WINDOW_MAX (4)

typedef struct _LOCAL_TVT_LIST LOCAL_TVT_LIST;

typedef struct
{
  LOCAL_TVT_MESSAGE *p_message;
  GFL_CLUNIT        *p_clunit;
  const GFL_MSGDATA       *cp_msgdata;
  u32 msgID[LOCAL_TVT_LIST_WINDOW_MAX];
  u32 max;
  u32 cursor_pos;
}LOCAL_TVT_LIST_PARAM;

extern LOCAL_TVT_LIST * LOCAL_TVT_LIST_Init( const LOCAL_TVT_LIST_PARAM *cp_param, HEAPID heapID );
extern void LOCAL_TVT_LIST_Exit( LOCAL_TVT_LIST *p_wk );
extern void LOCAL_TVT_LIST_Main( LOCAL_TVT_LIST *p_wk );
extern BOOL LOCAL_TVT_LIST_IsEnd( const LOCAL_TVT_LIST *cp_wk );
extern s32 LOCAL_TVT_LIST_GetSelect( const LOCAL_TVT_LIST *cp_wk );

//=============================================================================
/**
 *					ポケモンリスト
*/
//=============================================================================
typedef enum
{
  LOCAL_TVT_POKELIST_RESULT_NONE,
  LOCAL_TVT_POKELIST_RESULT_DECIDE,
  LOCAL_TVT_POKELIST_RESULT_CANCEL,
}LOCAL_TVT_POKELIST_RESULT;

typedef struct _LOCAL_TVT_POKELIST LOCAL_TVT_POKELIST;

typedef struct
{
  LOCAL_TVT_MESSAGE *p_message;
  GFL_CLUNIT        *p_clunit;
  POKEPARTY         *p_party;
}LOCAL_TVT_POKELIST_PARAM;

extern LOCAL_TVT_POKELIST * LOCAL_TVT_POKELIST_Init( const LOCAL_TVT_POKELIST_PARAM *cp_param, HEAPID heapID );
extern void LOCAL_TVT_POKELIST_Exit( LOCAL_TVT_POKELIST *p_wk );
extern void LOCAL_TVT_POKELIST_Main( LOCAL_TVT_POKELIST *p_wk );
extern void LOCAL_TVT_POKELIST_SetActive( LOCAL_TVT_POKELIST *p_wk, BOOL is_active );
extern void LOCAL_TVT_POKELIST_ReStart( LOCAL_TVT_POKELIST *p_wk );
extern LOCAL_TVT_POKELIST_RESULT LOCAL_TVT_POKELIST_GetResult( const LOCAL_TVT_POKELIST *cp_wk );
extern const POKEMON_PARAM * LOCAL_TVT_POKELIST_GetSelect( const LOCAL_TVT_POKELIST *cp_wk );

//=============================================================================
/**
 *					リストをカーソルとタッチで操作するシステム
*/
//=============================================================================
typedef enum
{
  LIST_UI_RESULT_NONE,    //何もおこらない
  LIST_UI_RESULT_MOVE,    //キーやタッチをした
  LIST_UI_RESULT_DECIDE,  //決定をした
}LIST_UI_RESULT;

//-------------------------------------
///	リストをカーソルとタッチで操作するシステム
//=====================================
typedef struct
{
  void *p_wk_adrs;
  LIST_UI_RESULT (*UpdateTouch)( void *p_wk_adrs );
  LIST_UI_RESULT (*UpdateKey)( void *p_wk_adrs );
  void (*Change)( void *p_wk_adrs, int touch_or_key );
}LIST_UI_PARAM;

typedef struct _LIST_UI LIST_UI;

extern LIST_UI * LIST_UI_Init( const LIST_UI_PARAM *cp_param, HEAPID heapID );
extern void LIST_UI_Exit( LIST_UI *p_wk );
extern void LIST_UI_Main( LIST_UI *p_wk );

extern BOOL LIST_UI_IsDecide( const LIST_UI *cp_wk );
extern void LIST_UI_ReStart( LIST_UI *p_wk );

//=============================================================================
/**
 *					ETC
*/
//=============================================================================
extern void LOCAL_TVT_UTIL_MainPltAnmLine( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, GXRgb start[], GXRgb end[] );
