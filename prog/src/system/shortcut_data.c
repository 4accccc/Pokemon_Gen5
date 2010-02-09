//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut_data.c
 *	@brief	Yボタン登録
 *	@author	Toru=Nagihashi
 *	@data		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/shortcut_data.h"
#include "savedata/shortcut.h"
#include "item/itemsym.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル初期化
 *
 *	@param	SHORTCUT_CURSOR *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUT_CURSOR_Init( SHORTCUT_CURSOR *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(SHORTCUT_CURSOR) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	アイテム番号からショートカットIDへの変換
 *
 *	@param	u8 item		アイテム
 *
 *	@return	SHORTCUT_ID	ショートカットID
 */
//-----------------------------------------------------------------------------
SHORTCUT_ID SHORTCUT_DATA_GetItemToShortcutID( u16 item )
{	
	
#ifdef DEBUG_ONLY_FOR_toru_nagihashi
#warning ITEM_ID add or changed//アイテムIDが変更したり追加されたりするはず
#endif

	NAGI_Printf( "アイテム変換 %d\n", item );

	switch( item )
	{	
	case ITEM_ZITENSYA:
		return SHORTCUT_ID_ZITENSYA;

	case ITEM_BORONOTURIZAO:
		/* fallthrough */
	case ITEM_IITURIZAO:
		/* fallthrough */
	case ITEM_SUGOITURIZAO:
		return SHORTCUT_ID_TURIZAO;

	case ITEM_TAUNMAPPU:
		return SHORTCUT_ID_TOWNMAP;
	
	case ITEM_BATORUREKOODAA:
		return SHORTCUT_ID_BTLRECORDER;

	case ITEM_TOMODATITETYOU:
		return SHORTCUT_ID_FRIENDNOTE;

	case ITEM_PARESUHEGOO:
		return SHORTCUT_ID_PALACEGO;

	default:
		return SHORTCUT_ID_NULL;
	}
}

