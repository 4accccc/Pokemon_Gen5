//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut.h
 *	@brief	Yボタン登録セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	Yボタン登録箇所
//=====================================
typedef enum
{
	SHORTCUT_ID_JITENSYA,
	SHORTCUT_ID_TSURIZAO,
	SHORTCUT_ID_TOWNMAP,
	SHORTCUT_ID_BAG_WAZAMACHINE,
	SHORTCUT_ID_PSTATUS_WAZA,

	SHORTCUT_ID_MAX,
	SHORTCUT_ID_NULL	= SHORTCUT_ID_MAX,	//データなし
} SHORTCUT_ID;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	未分類セーブデータ不完全型
//=====================================
typedef struct _SHORTCUT SHORTCUT;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int SHORTCUT_GetWorkSize( void );
extern void SHORTCUT_Init( SHORTCUT *p_msc );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const SHORTCUT * SaveData_GetShortCutConst( const SAVE_CONTROL_WORK * cp_sv );
extern SHORTCUT * SaveData_GetShortCut( SAVE_CONTROL_WORK * p_sv );

//----------------------------------------------------------
//各アプリ内での、Yボタン登録関数
//----------------------------------------------------------
extern void SHORTCUT_SetRegister( SHORTCUT *p_wk, SHORTCUT_ID shortcutID, BOOL is_on );
extern BOOL SHORTCUT_GetRegister( const SHORTCUT *cp_wk, SHORTCUT_ID shortcutID );

//----------------------------------------------------------
//Yボタンメニューで取得、操作する関数
//----------------------------------------------------------
extern SHORTCUT_ID SHORTCUT_GetType( const SHORTCUT *cp_wk, u8 idx );
extern void SHORTCUT_Insert( SHORTCUT *p_wk, SHORTCUT_ID shortcutID, u8 insert_idx );
