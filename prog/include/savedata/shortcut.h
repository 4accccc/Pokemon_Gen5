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
#include "system/shortcut_data.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define SHORTCUT_SAVE_FLAG_MAX  (36)

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
extern u16 SHORTCUT_GetMax( const SHORTCUT *cp_wk );

//----------------------------------------------------------
//キュレムの合体/分離時に専用アイテムのメニューをスワップする処理
//----------------------------------------------------------
extern void SHORTCUT_SwapRegisterForKyuremuItem( SHORTCUT *p_wk, SHORTCUT_ID shortcutID );

#define SHORTCUT_INSERT_LAST  0xFF
extern u8 SHORTCUT_Insert( SHORTCUT *p_wk, SHORTCUT_ID shortcutID, u8 insert_idx );
