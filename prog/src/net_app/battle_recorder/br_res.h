//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_res.h
 *	@brief	バトルレコーダーリソース管理
 *	@author	Toru=Nagihashi
 *	@date		2009.11.11
 *
 *	各プロセスを抜けてもリソースが残ることを考えて
 *	リソース管理ワークを使い読み込む
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "print/gf_font.h"
#include "print/wordset.h"

#include "br_fade.h"
#include "br_core.h"	//BR_PROCID
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BG
//=====================================
typedef enum
{
	BR_RES_BG_START_M,
	BR_RES_BG_START_S,
  BR_RES_BG_TEXT_M,

  BR_RES_BG_BROWSE_LOGO_M,
  BR_RES_BG_GDS_LOGO_M,

	BR_RES_BG_RECORD_S,
	BR_RES_BG_RECORD_M_BTL_SINGLE,
	BR_RES_BG_RECORD_M_BTL_DOUBLE,
	BR_RES_BG_RECORD_M_PROFILE,

  BR_RES_BG_SUBWAY_S,
  BR_RES_BG_SUBWAY_M_SINGLE,
  BR_RES_BG_SUBWAY_M_DOUBLE,
  BR_RES_BG_SUBWAY_M_MULTI,
  BR_RES_BG_SUBWAY_M_WIFI,
  BR_RES_BG_SUBWAY_M_NONE,

  BR_RES_BG_RNDMATCH_S,
  BR_RES_BG_RNDMATCH_M_RND,
  BR_RES_BG_RNDMATCH_M_RATE,
  BR_RES_BG_RNDMATCH_M_FREE,
  BR_RES_BG_RNDMATCH_M_NONE,

  BR_RES_BG_POKESEARCH_S_HEAD,
  BR_RES_BG_POKESEARCH_S_LIST,

  BR_RES_BG_PHOTO_S,
  BR_RES_BG_PHOTO_SEND_S,

  BR_RES_BG_BVRANK,

  BR_RES_BG_BVSEARCH_M,
  BR_RES_BG_BVSEARCH_MENU_S,
  BR_RES_BG_BVSEARCH_PLACE_S,
  BR_RES_BG_BVSEARCH_AREA_S,

  BR_RES_BG_CODEIN_NUMBER_S,

  BR_RES_BG_BVDELETE_S,

} BR_RES_BGID;



////-------------------------------------
///	OBJ
//=====================================
typedef enum
{
	BR_RES_OBJ_BROWSE_BTN_M,		//ブラウザ用ボタン、ライン
	BR_RES_OBJ_BROWSE_BTN_S,		//ブラウザ用ボタン、ライン

	BR_RES_OBJ_MUSICAL_BTN_M,		//ミュージカル用ボタン、ライン
	BR_RES_OBJ_MUSICAL_BTN_S,		//ミュージカル用ボタン、ライン

	BR_RES_OBJ_SIDEBAR_M,				//サイドバー
	BR_RES_OBJ_SIDEBAR_S,				//サイドバー
	BR_RES_OBJ_SHORT_BTN_S,			//短いボタン	＋	スクロールバー
  BR_RES_OBJ_ALLOW_M,         //矢印下
  BR_RES_OBJ_ALLOW_S,         //矢印下

  BR_RES_OBJ_BALL_M,        //カーソルOBJ用
  BR_RES_OBJ_BALL_S,        //カーソルOBJ用

  BR_RES_OBJ_NUM_S,           //ナンバー
  BR_RES_OBJ_NUM_CURSOR_S,    //ナンバーカーソル

  BR_RES_OBJ_BPFONT_M,        //バトルポイント

	BR_RES_OBJ_MAX
} BR_RES_OBJID;

//-------------------------------------
///	色指定  ここの並びをかえると内部のテーブルがずれます
//=====================================
typedef enum
{
  BR_RES_COLOR_TYPE_GREEN = 0,
  BR_RES_COLOR_TYPE_BLUE,
  BR_RES_COLOR_TYPE_PINK,
  BR_RES_COLOR_TYPE_BLACK,
  BR_RES_COLOR_TYPE_YELLOW,
  BR_RES_COLOR_TYPE_RED,
  BR_RES_COLOR_TYPE_DBROWN, //ダークブラウン
  BR_RES_COLOR_TYPE_BROWN,

  BR_RES_COLOR_TYPE_MAX,
} BR_RES_COLOR_TYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	OBJ用リソース
//=====================================
typedef struct
{
	u32 nce;
	u32 ncg;
	u32 ncl;
}BR_RES_OBJ_DATA;

//-------------------------------------
///	リソースワーク
//=====================================
typedef struct _BR_RES_WORK BR_RES_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	リソース管理作成
//=====================================
extern BR_RES_WORK *BR_RES_Init( BR_RES_COLOR_TYPE color, BOOL is_browse, HEAPID heapID );
extern void BR_RES_Exit( BR_RES_WORK *p_wk );

//-------------------------------------
///	グラフィック素材
//=====================================
//BGはメニューにあったものを読み込む
extern void BR_RES_LoadBG( BR_RES_WORK *p_wk, BR_RES_BGID bgID, HEAPID heapID );
extern void BR_RES_UnLoadBG( BR_RES_WORK *p_wk, BR_RES_BGID bgID );

//OBJは個別に読み込む
extern void BR_RES_LoadOBJ( BR_RES_WORK *p_wk, BR_RES_OBJID objID, HEAPID heapID );
extern BOOL BR_RES_GetOBJRes( const BR_RES_WORK *cp_wk, BR_RES_OBJID objID, BR_RES_OBJ_DATA *p_data );
extern void BR_RES_UnLoadOBJ( BR_RES_WORK *p_wk, BR_RES_OBJID objID );

//共通素材
extern void BR_RES_LoadCommon( BR_RES_WORK *p_wk, CLSYS_DRAW_TYPE type, HEAPID heapID );
extern void BR_RES_UnLoadCommon( BR_RES_WORK *p_wk, CLSYS_DRAW_TYPE type );

//-------------------------------------
///	その他素材
//=====================================
extern GFL_FONT * BR_RES_GetFont( const BR_RES_WORK *cp_wk );
extern GFL_MSGDATA * BR_RES_GetMsgData( const BR_RES_WORK *cp_wk );
extern WORDSET * BR_RES_GetWordSet( const BR_RES_WORK *cp_wk );
extern u16 BR_RES_GetFadeColor( const BR_RES_WORK *cp_wk );

//-------------------------------------
///	色替え
//=====================================
extern void BR_RES_ChangeColor( BR_RES_WORK *p_wk, BR_RES_COLOR_TYPE color );
extern BR_RES_COLOR_TYPE BR_RES_GetColor( const BR_RES_WORK *cp_wk );
extern void BR_RES_TransPaletteFade( BR_RES_WORK *p_wk, BR_FADE_WORK *p_fade, HEAPID heapID );
