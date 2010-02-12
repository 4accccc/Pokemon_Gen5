//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		zknd_tbar.h
 *	@brief	アプリ用下画面タッチバー
 *	@author	Koji Kawada
 *	@date		2010.02.04
 *
 *  @note   touchbar.hをコピペして、
 *          (1) TOUCHBARをZKND_TBARに置換し、
 *          必要な機能を追加しただけです。
 *          Revision 14819 Date 2009.12.16が元。
 *          変更箇所にはZKND_Modifiedという印を付けておきました。
 *
 *  モジュール名：ZKND_TBAR
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
	ZKND_TBAR_ICON_CLOSE,	//×ボタン
	ZKND_TBAR_ICON_RETURN,	//←┘ボタン
	ZKND_TBAR_ICON_CUR_D,	//↓ボタン
	ZKND_TBAR_ICON_CUR_U,	//↑ボタン
	ZKND_TBAR_ICON_CUR_L,	//←ボタン
	ZKND_TBAR_ICON_CUR_R,	//→ボタン
	ZKND_TBAR_ICON_CHECK,	//ちぇっくボタン

	ZKND_TBAR_ICON_MAX,		//.c内で使用
	//ZKND_TBAR_ICON_CUTSOM1	= ZKND_TBAR_ICON_MAX,	//カスタムボタン１  // ZKND_Modified カスタムの個数制限なしにした。
	//ZKND_TBAR_ICON_CUTSOM2,	//カスタムボタン２
	//ZKND_TBAR_ICON_CUTSOM3,	//カスタムボタン３
	//ZKND_TBAR_ICON_CUTSOM4,	//カスタムボタン４
	
  ZKND_TBAR_ICON_CUSTOM	= ZKND_TBAR_ICON_MAX,	//カスタムボタン(これ以上の値を与えること。ボタンごとに値を変えること。)  // ZKND_Modified カスタムの個数制限なしにした。

} ZKND_TBAR_ICON;
//-------------------------------------
///	下画面バー選択していない場合
//=====================================
#define ZKND_TBAR_SELECT_NONE		((ZKND_TBAR_ICON)GFL_UI_TP_HIT_NONE)

//-------------------------------------
///	デフォルト位置
//=====================================
//バー
#define ZKND_TBAR_MENUBAR_X				(0)
#define ZKND_TBAR_MENUBAR_Y				(21)
#define ZKND_TBAR_MENUBAR_W				(32)
#define ZKND_TBAR_MENUBAR_H				(3)
//アイコン
#define ZKND_TBAR_ICON_Y						(168)
#define ZKND_TBAR_ICON_Y_CHECK			(172)
#define ZKND_TBAR_ICON_X_00				(16)
#define ZKND_TBAR_ICON_X_01				(48)
#define ZKND_TBAR_ICON_X_02				(80)
#define ZKND_TBAR_ICON_X_03				(112)
#define ZKND_TBAR_ICON_X_04				(136)
#define ZKND_TBAR_ICON_X_05				(168)
#define ZKND_TBAR_ICON_X_06				(200)
#define ZKND_TBAR_ICON_X_07				(232)
//１つのアイコンの幅
#define ZKND_TBAR_ICON_WIDTH					(24)
#define ZKND_TBAR_ICON_HEIGHT				(24)

//-------------------------------------
///	アイコンのデフォルトプライオリティ
//=====================================
#define ZKND_TBAR_ICON_OBJ_PRIORITY	(0)
#define ZKND_TBAR_ICON_BG_PRIORITY		(0)

//-------------------------------------
///	使用するリソース
//=====================================
//バーのBGキャラサイズ
#define ZKND_TBAR_BG_CHARAAREA_SIZE	(8*8*GFL_BG_1CHRDATASIZ)

//バーのパレット本数
#define ZKND_TBAR_BG_PLT_NUM					(APP_COMMON_BAR_PLT_NUM)

//バーのボタンのパレット本数
#define ZKND_TBAR_OBJ_PLT_NUM				(APP_COMMON_BARICON_PLT_NUM)

//アイコンの最大登録個数
//#define APBAR_ICON_REGISTER_MAX			(8)  // ZKND_Modified 個数制限なしにした。

//-------------------------------------
///	デフォルトで使用するサウンド定義
//=====================================
#define ZKND_TBAR_SE_DECIDE	(SEQ_SE_DECIDE1)
#define ZKND_TBAR_SE_CANCEL	(SEQ_SE_CANCEL1)
#define ZKND_TBAR_SE_CLOSE	  (SEQ_SE_CLOSE1)
#define ZKND_TBAR_SE_Y_REG		(SEQ_SE_SYS_07)

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
	ZKND_TBAR_ICON			icon;
	GFL_CLACTPOS			pos;
  u16               width;  // ZKND_Modified 幅を指定できるようにした。

	//以下、カスタムボタンのみ必要な情報です
	u16	cg_idx;			// obj_graphic_manのキャラクタインデックス
	u16	plt_idx;		// obj_graphic_manのパレットインデックス
	u16	cell_idx;		// obj_graphic_manのセル　アニメインデックス
	u16	active_anmseq;			// ボタンのアクティブ時アニメシーケンス
	u16	noactive_anmseq;		// ボタンのノンアクティブ時アニメシーケンス
	u16	push_anmseq;				// ボタンを押したときのアニメシーケンス
	u32	key;								// 指定されたキーのボタンを押したときにアイコンも押されたことにする(PAD_KEY_UP等をいれてください ０だと行わない) 
	u32	se;									// SE　0だと行わない

} ZKND_TBAR_ITEM_ICON;

//-------------------------------------
///	アプリケーションバー設定構造体
//=====================================
typedef struct 
{
	//アイコン情報
	ZKND_TBAR_ITEM_ICON	*p_item;	//作成したいアイコン分ワークを作成しテーブルを渡してください
	u32									item_num;	//↑のアイコンワークの数です。

	//タッチバーで使用するモジュール
	GFL_CLUNIT* p_unit;
	
	//リソース読み込み情報
	u32 bar_frm;								//BGキャラ読み込みフレーム	AreaManでZKND_TBAR_BG_CHARAAREA_SIZE読みます
	u32 bg_plt;									//BG使用パレット0～0xFZKND_TBAR_BG_PLT_NUM本読みます
	u32 obj_plt;								//OBJパレット0～0xF		ZKND_TBAR_OBJ_PLT_NUM本読みます
	APP_COMMON_MAPPING mapping;	//OBJマッピングモード

	//BG読みこまない
	BOOL is_notload_bg;					//TRUEを指定するとBGとBG用パレットを読み込みません
} ZKND_TBAR_SETUP;

//-------------------------------------
///	下画面バー
//=====================================
typedef struct _ZKND_TBAR_WORK ZKND_TBAR_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	初期化
//=====================================
extern ZKND_TBAR_WORK * ZKND_TBAR_Init( ZKND_TBAR_SETUP *p_setup, HEAPID heapID );
//-------------------------------------
///	破棄
//=====================================
extern void ZKND_TBAR_Exit( ZKND_TBAR_WORK *p_wk );
//-------------------------------------
///	メイン処理
//=====================================
extern void ZKND_TBAR_Main( ZKND_TBAR_WORK *p_wk );
//-------------------------------------
///	アイコントリガー情報	何もしてない場合ZKND_TBAR_SELECT_NONE
//		ボタンのアニメが終わってから１Fのみ返します
//=====================================
extern ZKND_TBAR_ICON ZKND_TBAR_GetTrg( const ZKND_TBAR_WORK *cp_wk );

//-------------------------------------
///	アイコントリガー情報	何もしてない場合ZKND_TBAR_SELECT_NONE
//		ボタンを触った瞬間に１Fのみ返します
//=====================================
extern ZKND_TBAR_ICON ZKND_TBAR_GetTouch( const ZKND_TBAR_WORK *cp_wk );

//-------------------------------------
///	全体の設定
//(内部ではすべてのICONを設定しているだけなので、全体のフラグと個別のフラグは重複しています)
//=====================================
extern void ZKND_TBAR_SetActiveAll( ZKND_TBAR_WORK *p_wk, BOOL is_active );
extern void ZKND_TBAR_SetVisibleAll( ZKND_TBAR_WORK *p_wk, BOOL is_visible );

// ZKND_Modified 全体専用の設定を追加。↓
//-------------------------------------
///	全体のアクティブを切り替える
// 見た目をアクティブ状態にしたまま、全体専用のアクティブフラグの切り替えを行う
//=====================================
extern void ZKND_TBAR_SetActiveWhole( ZKND_TBAR_WORK *p_wk, BOOL is_active );
//-------------------------------------
///	全体のロックを解除する
// 見た目をアクティブ状態にしたままかかっている全体専用のロックを解除する
// ２度押しがないように、１度押したら押せないようにロックをかけることにする
//=====================================
extern void ZKND_TBAR_UnlockWhole( ZKND_TBAR_WORK *p_wk );
// ZKND_Modified 全体専用の設定を追加。↑

//=============================================================================
/**
 *	アイコン個別設定、取得
 */
//=============================================================================
//-------------------------------------
///	アイコンアクティブ設定
//		アクティブ、ノンアクティブでアニメが変わります
//		また、ノンアクティブ状態ではアイコンが押せません
//=====================================
extern void ZKND_TBAR_SetActive( ZKND_TBAR_WORK *p_wk,  ZKND_TBAR_ICON icon, BOOL is_active );
extern BOOL ZKND_TBAR_GetActive( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon );
//-------------------------------------
///	アイコン非表示設定
//=====================================
extern void ZKND_TBAR_SetVisible( ZKND_TBAR_WORK *p_wk,  ZKND_TBAR_ICON icon, BOOL is_visible );
extern BOOL ZKND_TBAR_GetVisible( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon );
//-------------------------------------
///	SE設定
//=====================================
extern void ZKND_TBAR_SetSE( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, u32 se );
extern u32 ZKND_TBAR_GetSE( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon );
//-------------------------------------
///	Key設定
//=====================================
extern void ZKND_TBAR_SetUseKey( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, u32 key );
extern u32 ZKND_TBAR_GetUseKey( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon );
//-------------------------------------
///	Flip設定
//	フリップ式ボタン（ON,OFF切り替え式ボタン）への設定
//=====================================
extern void ZKND_TBAR_SetFlip( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon, BOOL is_flip );
extern BOOL ZKND_TBAR_GetFlip( const ZKND_TBAR_WORK *cp_wk, ZKND_TBAR_ICON icon );

// ZKND_Modified アイコン個別操作を追加。↓
//-------------------------------------
///	アイコンのGFL_CLWKを取得する
//=====================================
extern GFL_CLWK* ZKND_TBAR_GetClwk( ZKND_TBAR_WORK *p_wk, ZKND_TBAR_ICON icon );
// ZKND_Modified アイコン個別操作を追加。↑

