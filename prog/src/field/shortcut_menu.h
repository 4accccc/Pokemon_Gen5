//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut_menu.h
 *	@brief	ショートカットメニュー動作
 *	@author	Toru=Nagihashi
 *	@date		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "gamesystem/game_data.h"
#include "system/shortcut_data.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ショートカットメニューで入力した種類
//=====================================
typedef enum
{
	SHORTCUTMENU_INPUT_NONE,		//何も入力していない
	SHORTCUTMENU_INPUT_SELECT,	//選択した
	SHORTCUTMENU_INPUT_CANCEL,	//キャンセルした
} SHORTCUTMENU_INPUT;

//-------------------------------------
///	ショートカットメニュー起動モード
//=====================================
typedef enum
{
	SHORTCUTMENU_MODE_POPUP,	//下から出てくる
	SHORTCUTMENU_MODE_STAY,		//すでに表示されている
} SHORTCUTMENU_MODE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ショートカットメニュー
//=====================================
typedef struct _SHORTCUTMENU_WORK SHORTCUTMENU_WORK;

//=============================================================================
/**
 *					外部参照宣言
*/
//=============================================================================
//-------------------------------------
///	システム
//=====================================
extern SHORTCUTMENU_WORK *SHORTCUTMENU_Init( GAMEDATA *p_gamedata, SHORTCUTMENU_MODE mode, SHORTCUT_CURSOR *p_cursor, HEAPID sys_heapID, HEAPID res_heapID );
extern void SHORTCUTMENU_Exit( SHORTCUTMENU_WORK *p_wk );
extern void SHORTCUTMENU_Main( SHORTCUTMENU_WORK *p_wk );

//-------------------------------------
///	メニュー開始
//=====================================
extern void SHORTCUTMENU_Open( SHORTCUTMENU_WORK *p_wk );
extern void SHORTCUTMENU_Close( SHORTCUTMENU_WORK *p_wk );
extern BOOL SHORTCUTMENU_IsMove( const SHORTCUTMENU_WORK *cp_wk );

//-------------------------------------
///	取得
//=====================================
extern SHORTCUTMENU_INPUT SHORTCUTMENU_GetInput( const SHORTCUTMENU_WORK *cp_wk, SHORTCUT_ID *p_shortcutID );
