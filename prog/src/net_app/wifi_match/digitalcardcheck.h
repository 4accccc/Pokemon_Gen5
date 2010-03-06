//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		digitalcardcheck.h
 *	@brief	デジタル選手証確認画面
 *	@author	Toru=Nagihashi
 *	@data		2010.1.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/game_data.h"
#include "savedata/regulation.h"

//WIFIBATTLEMATCHのモジュール
#include "net_app/wifi_match/wifibattlematch_util.h"
#include "net_app/wifi_match/wifibattlematch_graphic.h"
#include "net_app/wifi_match/wifibattlematch_view.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体
*/
//=============================================================================
//-------------------------------------
///	ハンドル
//=====================================
typedef struct _DIGITALCARD_CHECK_WORK DIGITALCARD_CHECK_WORK;

//-------------------------------------
///	引数
//=====================================
typedef struct 
{
  REGULATION_CARD_TYPE          type;         //[in ]確認する選手証のタイプ
  WBM_TEXT_WORK                 *p_text;      //[in ]テキスト表示モジュール
  GFL_FONT                      *p_font;      //[in ]フォント
  GFL_MSGDATA                   *p_msg;       //[in ]メッセージ
  PRINT_QUE                     *p_que;       //[in ]プリントキュー
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_view;      //[in ]リソース共通モジュール
  WIFIBATTLEMATCH_GRAPHIC_WORK  *p_graphic;   //[in ]描画共通モジュール
  GAMEDATA                      *p_gamedata;  //[in ]ゲームデータ
} DIGITALCARD_CHECK_PARAM;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
extern DIGITALCARD_CHECK_WORK *DIGITALCARD_CHECK_Init( const DIGITALCARD_CHECK_PARAM *cp_param, HEAPID heapID );
extern void DIGITALCARD_CHECK_Main( DIGITALCARD_CHECK_WORK *p_wk );
extern void DIGITALCARD_CHECK_Exit( DIGITALCARD_CHECK_WORK *p_wk );

extern BOOL DIGITALCARD_CHECK_IsEnd( const DIGITALCARD_CHECK_WORK *cp_wk );
