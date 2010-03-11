//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livebattlematch_flow.h
 *	@brief  ライブ大会フロー
 *	@author	Toru=Nagihashi
 *	@data		2010.03.06
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
  LIVEBATTLEMATCH_FLOW_RET_CONTINUE,  //処理続行中
  LIVEBATTLEMATCH_FLOW_RET_LIVEMENU,  //ライブ大会メニューへ
  LIVEBATTLEMATCH_FLOW_RET_BATTLE,    //バトルへ
  LIVEBATTLEMATCH_FLOW_RET_REC,       //録画へ
  LIVEBATTLEMATCH_FLOW_RET_BTLREC,    //録画再生へ
} LIVEBATTLEMATCH_FLOW_RET;

//-------------------------------------
///	開始モード
//=====================================
typedef enum
{
  LIVEBATTLEMATCH_FLOW_MODE_START,  //最初から開始
  LIVEBATTLEMATCH_FLOW_MODE_MENU,    //ライブ大会メインメニューから開始
  LIVEBATTLEMATCH_FLOW_MODE_BTL,    //バトル後から開始
  LIVEBATTLEMATCH_FLOW_MODE_REC,    //録画後から開始

} LIVEBATTLEMATCH_FLOW_MODE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ハンドル
//=====================================
typedef struct _LIVEBATTLEMATCH_FLOW_WORK LIVEBATTLEMATCH_FLOW_WORK;


//-------------------------------------
///	引数
//=====================================
typedef struct 
{
  LIVEBATTLEMATCH_FLOW_MODE     mode;         //[in ]開始のフロー
  WBM_TEXT_WORK                 *p_text;      //[in ]テキスト表示モジュール
  GFL_FONT                      *p_font;      //[in ]フォント
  PRINT_QUE                     *p_que;       //[in ]プリントキュー
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_view;      //[in ]リソース共通モジュール
  WIFIBATTLEMATCH_GRAPHIC_WORK  *p_graphic;   //[in ]描画共通モジュール
  GAMEDATA                      *p_gamedata;  //[in ]ゲームデータ

   //以下、[in ]常駐データ置き場
  WIFIBATTLEMATCH_ENEMYDATA     *p_player_data;
  WIFIBATTLEMATCH_ENEMYDATA     *p_enemy_data; 

} LIVEBATTLEMATCH_FLOW_PARAM;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern LIVEBATTLEMATCH_FLOW_WORK *LIVEBATTLEMATCH_FLOW_Init( const LIVEBATTLEMATCH_FLOW_PARAM *cp_param, HEAPID heapID );
extern void LIVEBATTLEMATCH_FLOW_Exit( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
extern void LIVEBATTLEMATCH_FLOW_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk );

extern LIVEBATTLEMATCH_FLOW_RET LIVEBATTLEMATCH_FLOW_IsEnd( const LIVEBATTLEMATCH_FLOW_WORK *cp_wk );
