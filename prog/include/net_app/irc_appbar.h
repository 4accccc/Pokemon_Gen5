//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_appbar.h
 *	@brief	赤外線チェック用下画面バー
 *	@author	Toru=Nagihashi
 *	@date		2009.07.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//lib
#include <gflib.h>

#include "print/gf_font.h"
#include "print/printsys.h"

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
	APPBAR_ICON_CLOSE,
	APPBAR_ICON_RETURN,
	APPBAR_ICON_CUR_D,
	APPBAR_ICON_CUR_U,
	APPBAR_ICON_CUR_L,
	APPBAR_ICON_CUR_R,
	APPBAR_ICON_CHECK,
	APPBAR_ICON_MAX,
} APPBAR_ICON;
//-------------------------------------
///	下画面バー
//=====================================
typedef enum
{	
	APPBAR_OPTION_MASK_CLOSE		= 1<<APPBAR_ICON_CLOSE,	//閉じる
	APPBAR_OPTION_MASK_RETURN		= 1<<APPBAR_ICON_RETURN,	//戻る
	APPBAR_OPTION_MASK_CUR_D		= 1<<APPBAR_ICON_CUR_D,	//カーソル上
	APPBAR_OPTION_MASK_CUR_U		= 1<<APPBAR_ICON_CUR_U,	//カーソル下
	APPBAR_OPTION_MASK_CUR_L		= 1<<APPBAR_ICON_CUR_L,	//カーソル左
	APPBAR_OPTION_MASK_CUR_R		= 1<<APPBAR_ICON_CUR_R,	//カーソル右
	APPBAR_OPTION_MASK_CHECK		= 1<<APPBAR_ICON_CHECK,	//チェックボックス
} APPBAR_OPTION_MASK;
//-------------------------------------
///	下画面バー
//=====================================
#define APPBAR_SELECT_NONE		    ((APPBAR_ICON)GFL_UI_TP_HIT_NONE)

//-------------------------------------
///	使用するリソース
//=====================================
//バーのパレット本数
#define APPBAR_BG_PLT_NUM					(APP_COMMON_BAR_PLT_NUM)

//バーのボタンのパレット本数
#define APPBAR_OBJ_PLT_NUM				(APP_COMMON_BARICON_PLT_NUM)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
typedef struct _APPBAR_WORK APPBAR_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern APPBAR_WORK * APPBAR_Init( APPBAR_OPTION_MASK mask, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, APP_COMMON_MAPPING mapping, GFL_FONT* p_font, PRINT_QUE* p_que, HEAPID heapID );
extern void APPBAR_Exit( APPBAR_WORK *p_wk );
extern void APPBAR_Main( APPBAR_WORK *p_wk );
extern APPBAR_ICON APPBAR_GetTrg( const APPBAR_WORK *cp_wk );
extern void APPBAR_SetNormal( APPBAR_WORK *p_wk );

