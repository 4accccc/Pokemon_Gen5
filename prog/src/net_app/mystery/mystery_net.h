//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_net.h
 *	@brief  ふしぎなおくりもの通信処理
 *	@author	Toru=Nagihashi
 *	@data		2009.12.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "gamesystem/game_comm.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	通信システムへの状態リクエスト
//=====================================
typedef enum
{
  MYSTERY_NET_STATE_WAIT, //待機中

  //ビーコン
  MYSTERY_NET_STATE_START_BEACON,   //START終了時、MAINへ勝手に状態遷移
  MYSTERY_NET_STATE_MAIN_BEACON,
  MYSTERY_NET_STATE_END_BEACON,

  //WiFiダウンロード
  MYSTERY_NET_STATE_WIFI_DOWNLOAD,          //
  MYSTERY_NET_STATE_CANCEL_WIFI_DOWNLOAD,  //ダウンロードのキャンセル

  //WIFIログアウト  ( ログインは外部で行うため　)
  MYSTERY_NET_STATE_LOGOUT_WIFI,

} MYSTERY_NET_STATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ふしぎなおくりもの通信ワーク
//=====================================
typedef struct _MYSTERY_NET_WORK  MYSTERY_NET_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//初期化。破棄、メイン処理
extern MYSTERY_NET_WORK * MYSTERY_NET_Init( const SAVE_CONTROL_WORK *cp_sv, HEAPID heapID );
extern void MYSTERY_NET_Exit( MYSTERY_NET_WORK *p_wk );
extern void MYSTERY_NET_Main( MYSTERY_NET_WORK *p_wk );

//状態変移.状態取得
extern void MYSTERY_NET_ChangeStateReq( MYSTERY_NET_WORK *p_wk, MYSTERY_NET_STATE state );
extern MYSTERY_NET_STATE MYSTERY_NET_GetState( const MYSTERY_NET_WORK *cp_wk );

//現在の通信を検知
extern GAME_COMM_STATUS_BIT MYSTERY_NET_GetCommStatus( const MYSTERY_NET_WORK *cp_wk );

//ダウンロードしたデータうけとり
extern BOOL MYSTERY_NET_GetDownloadData( const MYSTERY_NET_WORK *cp_wk, void *p_data, u32 size );
