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
#include "br_inner.h"			//BR_MENUID
#include "net_app/battle_recorder.h"	//BR_MODE
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	データが入っていない値
//=====================================
#define BR_BTN_DATA_NONE	(0xFFFF)

#define BR_BTN_DATA_WIDTH		(0)
#define BR_BTN_DATA_HEIGHT	(0)

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
///	ボタンデータ公開情報インデックス
//=====================================
typedef enum
{
	BR_BTN_DATA_PARAM_X,			//位置X
	BR_BTN_DATA_PARAM_Y,			//位置Y
	BR_BTN_DATA_PARAM_MENUID,	//自分の属しているメニューID
	BR_BTN_DATA_PARAM_MSGID,	//自分のアイコンに表示するメッセージID
	BR_BTN_DATA_PARAM_ANMSEQ,	//自分のアイコンのアニメシーケンス
	BR_BTN_DATA_PARAM_TYPE,		//ボタンの種類
	BR_BTN_DATA_PARAM_DATA,		//種類別のデータ

	BR_BTN_DATA_PARAM_MAX
} BR_BTN_DATA_PARAM;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン情報
//=====================================
typedef struct _BR_BTN_DATA  BR_BTN_DATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	データ取得
//=====================================
extern const BR_BTN_DATA * BR_BTN_DATA_SYS_GetData( BR_MENUID menuID, u16 btnID );
extern u32 BR_BTN_DATA_SYS_GetDataNum( BR_MENUID menuID );
extern u32 BR_BTN_DATA_SYS_GetDataMax( void );
extern BR_MENUID BR_BTN_DATA_SYS_GetStartMenuID( BR_MODE mode );
//-------------------------------------
///	内部情報取得
//=====================================
extern u16 BR_BTN_DATA_GetParam( const BR_BTN_DATA *cp_data, BR_BTN_DATA_PARAM paramID );
