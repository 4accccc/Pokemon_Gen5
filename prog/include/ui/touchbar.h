//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		touchbar.h
 *	@brief	アプリ用下画面タッチバー
 *	@author	Toru=Nagihashi
 *	@date		2009.09.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//lib
#include <gflib.h>

//resource
#include "app/app_menu_common.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	アイコンの種類
//=====================================
typedef enum
{	
	TOUCHBAR_ICON_CLOSE,
	TOUCHBAR_ICON_RETURN,
	TOUCHBAR_ICON_CUR_D,
	TOUCHBAR_ICON_CUR_U,
	TOUCHBAR_ICON_CUR_L,
	TOUCHBAR_ICON_CUR_R,
	TOUCHBAR_ICON_CHECK,

	TOUCHBAR_ICON_MAX,
} TOUCHBAR_ICON;
//-------------------------------------
///	下画面バー選択していない場合
//=====================================
#define TOUCHBAR_SELECT_NONE		((TOUCHBAR_ICON)GFL_UI_TP_HIT_NONE)

//-------------------------------------
///	デフォルト位置
//=====================================
//バー
#define TOUCHBAR_MENUBAR_X					(0)
#define TOUCHBAR_MENUBAR_Y					(21)
#define TOUCHBAR_MENUBAR_W					(32)
#define TOUCHBAR_MENUBAR_H					(3)
//アイコン
#define TOUCHBAR_ICON_Y						(168)
#define TOUCHBAR_ICON_Y_CHECK			(172)
#define TOUCHBAR_ICON_X_00				(16)
#define TOUCHBAR_ICON_X_01				(48)
#define TOUCHBAR_ICON_X_02				(80)
#define TOUCHBAR_ICON_X_03				(112)
#define TOUCHBAR_ICON_X_04				(144)
#define TOUCHBAR_ICON_X_05				(168)
#define TOUCHBAR_ICON_X_06				(200)
#define TOUCHBAR_ICON_X_07				(232)
//１つのアイコンの幅
#define TOUCHBAR_ICON_WIDTH					(24)
#define TOUCHBAR_ICON_HEIGHT				(24)

//-------------------------------------
///	アイコンのデフォルトプライオリティ
//=====================================
#define TOUCHBAR_ICON_OBJ_PRIORITY	(0)
#define TOUCHBAR_ICON_BG_PRIORITY		(0)

//-------------------------------------
///	使用するリソース
//=====================================
//バーのBGキャラサイズ
#define TOUCHBAR_BG_CHARAAREA_SIZE	(8*8*GFL_BG_1CHRDATASIZ)

//バーのパレット本数
#define TOUCHBAR_BG_PLT_NUM					(APP_COMMON_BAR_PLT_NUM)

//バーのボタンのパレット本数
#define TOUCHBAR_OBJ_PLT_NUM				(APP_COMMON_BARICON_PLT_NUM)

//アイコンの最大登録個数
#define APBAR_ICON_REGISTER_MAX			(8)


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	アプリケーションバーアイコン
//=====================================
typedef struct 
{
	TOUCHBAR_ICON			icon;
	GFL_CLACTPOS			pos;
} TOUCHBAR_ITEM_ICON;

//-------------------------------------
///	アプリケーションバー設定構造体
//=====================================
typedef struct 
{
	//アイコン情報
	TOUCHBAR_ITEM_ICON	*p_item;	//作成したいアイコン分ワークを作成しテーブルを渡してください
	u32									item_num;	//↑のアイコンワークの数です。

	//タッチバーで使用するモジュール
	GFL_CLUNIT* p_unit;

	
	//リソース読み込み情報
	u32 bar_frm;								//BGキャラ読み込みフレーム	AreaManでTOUCHBAR_BG_CHARAAREA_SIZE読みます
	u32 bg_plt;									//BG使用パレット0～0xFTOUCHBAR_BG_PLT_NUM本読みます
	u32 obj_plt;								//OBJパレット0～0xF		TOUCHBAR_OBJ_PLT_NUM本読みます
	APP_COMMON_MAPPING mapping;	//OBJマッピングモード

	//BG読みこまない
	BOOL is_notload_bg;					//TRUEを指定するとBGとBG用パレットを読み込みません
} TOUCHBAR_SETUP;

//-------------------------------------
///	下画面バー
//=====================================
typedef struct _TOUCHBAR_WORK TOUCHBAR_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	初期化
//=====================================
extern TOUCHBAR_WORK * TOUCHBAR_Init( TOUCHBAR_SETUP *p_setup, HEAPID heapID );
//-------------------------------------
///	破棄
//=====================================
extern void TOUCHBAR_Exit( TOUCHBAR_WORK *p_wk );
//-------------------------------------
///	メイン処理
//=====================================
extern void TOUCHBAR_Main( TOUCHBAR_WORK *p_wk );
//-------------------------------------
///	アイコントリガー情報	何もをしてない場合TOUCHBAR_SELECT_NONE
//=====================================
extern TOUCHBAR_ICON TOUCHBAR_GetTrg( const TOUCHBAR_WORK *cp_wk );
//-------------------------------------
///	アイコンアクティブ設定
//		アクティブ、ノンアクティブでアニメが変わります
//		また、ノンアクティブ状態ではアイコンが押せません
//=====================================
extern void TOUCHBAR_SetActive( TOUCHBAR_WORK *p_wk,  TOUCHBAR_ICON icon, BOOL is_active );
extern BOOL TOUCHBAR_GetActive( const TOUCHBAR_WORK *cp_wk, TOUCHBAR_ICON icon );
//-------------------------------------
///	アイコン非表示設定
//=====================================
extern void TOUCHBAR_SetVisible( TOUCHBAR_WORK *p_wk,  TOUCHBAR_ICON icon, BOOL is_visible );
extern BOOL TOUCHBAR_GetVisible( const TOUCHBAR_WORK *cp_wk, TOUCHBAR_ICON icon );
//-------------------------------------
///	チェックボックス用に現在のチェック取得
//=====================================
extern BOOL TOUCHBAR_GetCheck( const TOUCHBAR_WORK *cp_wk );


