//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn.h
 *	@brief	バトルレコーダー	ボタン管理
 *	@author	Toru=Nagihashi
 *	@data		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "system/main.h"  //HEAPID
#include "net_app/battle_recorder.h"	//BR_MODE
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン状態
//=====================================
typedef enum
{
	BR_BTN_SYS_STATE_WAIT,	//タッチ待ち
	BR_BTN_SYS_STATE_HANGER_MOVE,	//移動中
	BR_BTN_SYS_STATE_BTN_CHANGE,	//ボタン切り替え中
	BR_BTN_SYS_STATE_APPEAR_MOVE,
	BR_BTN_SYS_STATE_INPUT,
} BR_BTN_SYS_STATE;

//-------------------------------------
///	ボタン入力
//=====================================
typedef enum
{
	BR_BTN_SYS_INPUT_NONE,			//押していない（or押した後の動作中）
	BR_BTN_SYS_INPUT_CHANGESEQ,	//別シーケンスに飛ぶ
	BR_BTN_SYS_INPUT_EXIT,			//終了
} BR_BTN_SYS_INPUT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
typedef struct _BR_BTN_SYS_WORK BR_BTN_SYS_WORK;
//=============================================================================
/**
 *					PUBILIC関数
*/
//=============================================================================
extern BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MODE mode, GFL_CLUNIT *p_unit, HEAPID heapID );
extern void BR_BTN_SYS_Exit( BR_BTN_SYS_WORK *p_wk );
extern void BR_BTN_SYS_Main( BR_BTN_SYS_WORK *p_wk );
extern BR_BTN_SYS_INPUT BR_BTN_SYS_GetInput( const BR_BTN_SYS_WORK *cp_wk, u32 *p_seq );
extern BR_BTN_SYS_STATE BR_BTN_SYS_GetState( const BR_BTN_SYS_WORK *cp_wk );
