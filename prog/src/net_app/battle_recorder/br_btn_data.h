//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn_data.h
 *	@brief	ボタン情報
 *	@author	Toru=Nagihashi
 *	@date		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "system/main.h"  //HEAPID
#include "br_menu_proc.h"	//BR_MENUID
#include "br_inner.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	データが入っていない値
//=====================================
#define BR_BTN_DATA_NONE	(0xFFFF)

//-------------------------------------
///	
//=====================================
#define BR_BTN_DATA_WIDTH	      	(20*8)
#define BR_BTN_DATA_SHORT_WIDTH		(12*8)
#define BR_BTN_DATA_HEIGHT	      (4*8)
#define BR_BTN_POS_X		    			(42)

//-------------------------------------
///	ボタンの種類
//=====================================
typedef enum
{
	BR_BTN_TYPE_SELECT,				//選択用ボタン
	BR_BTN_TYPE_RETURN,				//1つ前のメニューへ戻る用ボタン
	BR_BTN_TYPE_EXIT,					//バトルレコーダー終了用ボタン
	BR_BTN_TYPE_MYRECORD,			//自分の記録ボタン
	BR_BTN_TYPE_OTHERRECORD,	//誰かの記録ボタン
	BR_BTN_TYPE_DELETE_MY,		//自分の記録を消すボタン
	BR_BTN_TYPE_DELETE_OTHER,	//誰かの記録を消すボタン
	BR_BTN_TYPE_CHANGESEQ,		//シーケンス変更ボタン

} BR_BTN_TYPE;

//-------------------------------------
///	無いときのボタンの動作
//=====================================
typedef enum
{
	BR_BTN_NONEPROC_VANISH,	//消える
	BR_BTN_NONEPROC_NOPUSH,	//押せない（上画面にメッセージ表示）
} BR_BTN_NONEPROC;

//-------------------------------------
///	ボタンデータ公開情報インデックス
//=====================================
typedef enum
{
	BR_BTN_DATA_PARAM_X,			//位置X
	BR_BTN_DATA_PARAM_Y,			//位置Y
	BR_BTN_DATA_PARAM_MENUID,	//自分の属しているメニューID
	BR_BTN_DATA_PARAM_MSGID,	//自分のアイコンに表示するメッセージID
	BR_BTN_DATA_PARAM_ANMSEQ,	//自分のアイコンのアニメシーケンス
	BR_BTN_DATA_PARAM_TYPE,		//ボタンの種類		BR_BTN_TYPE列挙
	BR_BTN_DATA_PARAM_DATA1,	//種類別のデータ１
	BR_BTN_DATA_PARAM_DATA2,	//種類別のデータ２
	BR_BTN_DATA_PARAM_VALID,	//有効なボタンか	TRUE or FALSE
	BR_BTN_DATA_PARAM_NONE_PROC,	//有効でないときの処理		BR_BTN_NONEPROC列挙
	BR_BTN_DATA_PARAM_NONE_DATA,	//有効でないときの処理で使うデータ

	BR_BTN_DATA_PARAM_MAX
} BR_BTN_DATA_PARAM;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン情報バッファ
//=====================================
typedef struct _BR_BTN_DATA_SYS  BR_BTN_DATA_SYS;

//-------------------------------------
///	ボタン情報
//=====================================
typedef struct _BR_BTN_DATA  BR_BTN_DATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern BR_BTN_DATA_SYS * BR_BTN_DATA_SYS_Init( const BR_BTLREC_SET *cp_rec, HEAPID heapID );
extern void BR_BTN_DATA_SYS_Exit( BR_BTN_DATA_SYS *p_wk );
//-------------------------------------
///	データ取得
//=====================================
extern const BR_BTN_DATA * BR_BTN_DATA_SYS_GetData( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID, u16 btnID );
extern u32 BR_BTN_DATA_SYS_GetDataNum( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID );
extern u32 BR_BTN_DATA_SYS_GetDataMax( const BR_BTN_DATA_SYS *cp_wk );

extern void BR_BTN_DATA_SYS_GetLink( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID, u32 idx, BR_MENUID *p_preID, u32 *p_btnID );
extern u32 BR_BTN_DATA_SYS_GetLinkBtnID( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID, BR_MENUID preID );
extern u32 BR_BTN_DATA_SYS_GetLinkMax( const BR_BTN_DATA_SYS *cp_wk, BR_MENUID menuID );

//-------------------------------------
///	内部情報取得
//=====================================
extern u16 BR_BTN_DATA_GetParam( const BR_BTN_DATA *cp_data, BR_BTN_DATA_PARAM paramID );
