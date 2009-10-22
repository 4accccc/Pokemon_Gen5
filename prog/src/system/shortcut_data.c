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
 *	@brief	アイテム番号からショートカットIDへの変換
 *
 *	@param	u8 item		アイテム
 *
 *	@return	SHORTCUT_ID	ショートカットID
 */
//-----------------------------------------------------------------------------
SHORTCUT_ID SHORTCUT_DATA_GetItemToShortcutID( u8 item )
{	
	switch( item )
	{	
	case ITEM_ZITENSYA:
		return SHORTCUT_ID_ZITENSYA;
#if 0
	case ITEM_BORONOTURIZAO:
		return SHORTCUT_ID_BORONOTURIZAO;

	case ITEM_IITURIZAO:
		return SHORTCUT_ID_IITURIZAO;

	case ITEM_SUGOITURIZAO:
		return SHORTCUT_ID_SUGOITURIZAO;
#else
	case ITEM_BORONOTURIZAO:
		/* fallthrough */
	case ITEM_IITURIZAO:
		/* fallthrough */
	case ITEM_SUGOITURIZAO:
		return SHORTCUT_ID_TURIZAO;
#endif
	case ITEM_TAUNMAPPU:
		return SHORTCUT_ID_TOWNMAP;

	default:
		return SHORTCUT_ID_NULL;
	}
}
