//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		battle_championship_flow.h
 *	@brief  大会メインメニュー  フロー
 *	@author	Toru=Nagihashi
 *	@date		2010.03.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>
//WIFIBATTLEMATCHのモジュール
#include "net_app/wifi_match/wifibattlematch_util.h"
#include "net_app/wifi_match/wifibattlematch_graphic.h"
#include "net_app/wifi_match/wifibattlematch_view.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	終了モード
//=====================================
typedef enum
{
  BATTLE_CHAMPIONSHIP_FLOW_RET_CONTINUE,  //処理続行中
  BATTLE_CHAMPIONSHIP_FLOW_RET_TITLE,     //終了してタイトルへ
  BATTLE_CHAMPIONSHIP_FLOW_RET_WIFICUP,   //終了してWIFI大会へ
  BATTLE_CHAMPIONSHIP_FLOW_RET_LIVECUP,   //ライブ大会へ
} BATTLE_CHAMPIONSHIP_FLOW_RET;

//-------------------------------------
///	開始モード
//=====================================
typedef enum
{
  BATTLE_CHAMPIONSHIP_FLOW_MODE_MAINMENU,   //メインメニューから開始
  BATTLE_CHAMPIONSHIP_FLOW_MODE_WIFIMENU,   //WIFIメニューから開始
  BATTLE_CHAMPIONSHIP_FLOW_MODE_LIVEMENU,   //LIVEメニューから開始
} BATTLE_CHAMPIONSHIP_FLOW_MODE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ハンドル
//=====================================
typedef struct _BATTLE_CHAMPIONSHIP_FLOW_WORK BATTLE_CHAMPIONSHIP_FLOW_WORK;

//-------------------------------------
///	引数
//=====================================
typedef struct 
{
  BATTLE_CHAMPIONSHIP_FLOW_MODE mode;         //[in ]開始のフロー
  WBM_TEXT_WORK                 *p_text;      //[in ]テキスト表示モジュール
  GFL_FONT                      *p_font;      //[in ]フォント
  GFL_MSGDATA                   *p_msg;       //[in ]メッセージ
  PRINT_QUE                     *p_que;       //[in ]プリントキュー
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_view;      //[in ]リソース共通モジュール
  WIFIBATTLEMATCH_GRAPHIC_WORK  *p_graphic;   //[in ]描画共通モジュール
  GAMEDATA                      *p_gamedata;  //[in ]ゲームデータ
} BATTLE_CHAMPIONSHIP_FLOW_PARAM;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern BATTLE_CHAMPIONSHIP_FLOW_WORK *BATTLE_CHAMPIONSHIP_FLOW_Init( const BATTLE_CHAMPIONSHIP_FLOW_PARAM *cp_param, HEAPID heapID );
extern void BATTLE_CHAMPIONSHIP_FLOW_Main( BATTLE_CHAMPIONSHIP_FLOW_WORK *p_wk );
extern void BATTLE_CHAMPIONSHIP_FLOW_Exit( BATTLE_CHAMPIONSHIP_FLOW_WORK *p_wk );

extern BATTLE_CHAMPIONSHIP_FLOW_RET BATTLE_CHAMPIONSHIP_FLOW_IsEnd( const BATTLE_CHAMPIONSHIP_FLOW_WORK *cp_wk );
