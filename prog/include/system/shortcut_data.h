//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut_data.h
 *	@brief	Yボタン登録
 *	@author	Toru=Nagihashi
 *	@date		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

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
	SHORTCUT_ID_ZITENSYA,
	SHORTCUT_ID_BORONOTURIZAO,
	SHORTCUT_ID_IITURIZAO,
	SHORTCUT_ID_SUGOITURIZAO,
	SHORTCUT_ID_TOWNMAP,
	SHORTCUT_ID_BAG_WAZAMACHINE,
	SHORTCUT_ID_PSTATUS_WAZA,


	SHORTCUT_ID_MAX,
	SHORTCUT_ID_NULL	= SHORTCUT_ID_MAX,	//データなし
} SHORTCUT_ID;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern SHORTCUT_ID SHORTCUT_DATA_GetItemToShortcutID( u8 item );
