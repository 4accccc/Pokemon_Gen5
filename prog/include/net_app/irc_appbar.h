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
#define APPBAR_SELECT_NONE		((APPBAR_ICON)GFL_UI_TP_HIT_NONE)

//-------------------------------------
///	デフォルト位置
//=====================================
//バー
#define APPBAR_MENUBAR_X	(0)
#define APPBAR_MENUBAR_Y	(21)
#define APPBAR_MENUBAR_W	(32)
#define APPBAR_MENUBAR_H	(3)
//アイコン
#define APPBAR_ICON_Y							(168)
#define APPBAR_ICON_CUR_L_X				(16)
#define APPBAR_ICON_CUR_R_X				(38)
#define APPBAR_ICON_CUR_U_X				(16)
#define APPBAR_ICON_CUR_D_X				(38)
#define APPBAR_ICON_CHECK_X				(38)
#define APPBAR_ICON_CLOSE_X				(200)
#define APPBAR_ICON_RETURN_X			(232)
//１つのアイコンの幅
#define APPBAR_ICON_WIDTH					(24)
#define APPBAR_ICON_HEIGHT				(24)

//-------------------------------------
///	アイコンのデフォルトプライオリティ
//=====================================
#define APPBAR_ICON_OBJ_PRIORITY	(0)
#define APPBAR_ICON_BG_PRIORITY		(0)

//-------------------------------------
///	使用するリソース
//=====================================
//バーのBGキャラサイズ
#define APPBAR_BG_CHARAAREA_SIZE	(8*8*GFL_BG_1CHRDATASIZ)

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
///	アプリケーションバー設定構造体
//=====================================
typedef struct 
{
	APPBAR_ICON			icon;
	GFL_CLACTPOS		pos;
} APPBAR_SETUP;

//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
typedef struct _APPBAR_WORK APPBAR_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern APPBAR_WORK * APPBAR_Init( APPBAR_OPTION_MASK mask, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, APP_COMMON_MAPPING mapping, HEAPID heapID );
extern APPBAR_WORK * APPBAR_InitEx( const APPBAR_SETUP *cp_setup_tbl, u16 tbl_len, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, APP_COMMON_MAPPING mapping, HEAPID heapID );
extern void APPBAR_Exit( APPBAR_WORK *p_wk );
extern void APPBAR_Main( APPBAR_WORK *p_wk );
extern APPBAR_ICON APPBAR_GetTrg( const APPBAR_WORK *cp_wk );
extern APPBAR_ICON APPBAR_GetCont( const APPBAR_WORK *cp_wk );

