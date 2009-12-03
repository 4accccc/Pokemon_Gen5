//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_net.h
 *	@brief  WIFIバトル用ネットモジュール
 *	@author	Toru=Nagihashi
 *	@date		2009.11.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "sc/dwc_sc.h"
#include "gdb/dwc_gdb.h"
#include "battle/battle.h"
#include "wifibattlematch_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	マッチシーケンス
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_SEQ_MATCH_IDLE,
  WIFIBATTLEMATCH_NET_SEQ_MATCH_START,
  WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT,

  WIFIBATTLEMATCH_NET_SEQ_CONNECT_START,
  WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD,
  WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT,
  WIFIBATTLEMATCH_NET_SEQ_TIMING_END,

  WIFIBATTLEMATCH_NET_SEQ_MATCH_END,
} WIFIBATTLEMATCH_NET_SEQ;
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	SAKEからのデータ受け取り
//=====================================
typedef struct
{ 
  BOOL is_get;
  u16 single_win;
  u16 single_lose;
  u16 double_win;
  u16 double_lose;
  u16 rot_win;
  u16 rot_lose;
  u16 triple_win;
  u16 triple_lose;
  u16 shooter_win;
  u16 shooter_lose;
  u16 single_rate;
  u16 double_rate;
  u16 rot_rate;
  u16 triple_rate;
  u16 shooter_rate;
  u16 disconnect;
  u16 cheat;
} WIFIBATTLEMATCH_GDB_RND_SCORE_DATA;


//-------------------------------------
///	ネットモジュール
//=====================================
typedef struct _WIFIBATTLEMATCH_NET_WORK WIFIBATTLEMATCH_NET_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク作成
//=====================================
extern WIFIBATTLEMATCH_NET_WORK * WIFIBATTLEMATCH_NET_Init( HEAPID heapID );
extern void WIFIBATTLEMATCH_NET_Exit( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern void WIFIBATTLEMATCH_NET_Main( WIFIBATTLEMATCH_NET_WORK *p_wk );

//-------------------------------------
///	マッチング
//=====================================
//エラー
extern BOOL WIFIBATTLEMATCH_NET_IsError( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

//マッチング
extern void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern BOOL WIFIBATTLEMATCH_NET_WaitMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern WIFIBATTLEMATCH_NET_SEQ WIFIBATTLEMATCH_NET_GetSeqMatchMake( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

//切断
extern BOOL WIFIBATTLEMATCH_NET_SetDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_return );

//キャンセル
extern BOOL WIFIBATTLEMATCH_NET_SetCancelState( const WIFIBATTLEMATCH_NET_WORK *cp_wk );
extern BOOL WIFIBATTLEMATCH_NET_CancelDisable( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

//-------------------------------------
///	統計・競争関係（SC）
//=====================================
extern void WIFIBATTLEMATCH_SC_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, BtlRule rule, BtlResult result );
extern BOOL WIFIBATTLEMATCH_SC_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, const DWCUserData *cp_user_data, DWCScResult *p_result );

//-------------------------------------
///	データベース関係（GDB)
//=====================================
typedef enum
{ 
  WIFIBATTLEMATCH_GDB_GET_RND_SCORE,
}WIFIBATTLEMATCH_GDB_GETTYPE;
//WIFIBATTLEMATCH_GDB_GET_RND_SCORE
extern void WIFIBATTLEMATCH_GDB_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_GETTYPE type, void *p_wk_adrs );
extern BOOL WIFIBATTLEMATCH_GDB_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, const DWCUserData *cp_user_data, DWCGdbError *p_result );

//-------------------------------------
///	相手のデータ受信
//=====================================
extern BOOL WIFIBATTLEMATCH_NET_StartEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_buff );
extern BOOL WIFIBATTLEMATCH_NET_WaitEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data );
