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
typedef struct _ITEM2SHORTCUT_TBL{
  u16 itemno:10;
  u16 shortcut_id:6;
}ITEM2SHORTCUT_TBL;

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
#if 1 //switch caseをなくして軽量化 by iwasawa 10.05.01
  int i;
  static const ITEM2SHORTCUT_TBL Item2ShortcutTbl[] = {
 	  { ITEM_ZITENSYA, SHORTCUT_ID_ZITENSYA },
    { ITEM_SUGOITURIZAO, SHORTCUT_ID_TURIZAO },
    { ITEM_TAUNMAPPU, SHORTCUT_ID_TOWNMAP },
	  { ITEM_BATORUREKOODAA, SHORTCUT_ID_BTLRECORDER },
    { ITEM_TOMODATITETYOU, SHORTCUT_ID_FRIENDNOTE },
    { ITEM_DAUZINGUMASIN, SHORTCUT_ID_DOWSINGMACHINE },
    { ITEM_GURASIDEANOHANA, SHORTCUT_ID_GURASHIDEA },
  };

  for( i = 0;i < NELEMS( Item2ShortcutTbl);i++){
    if( Item2ShortcutTbl[i].itemno == item ){
      return Item2ShortcutTbl[i].shortcut_id;
    }
  }
	return SHORTCUT_ID_NULL;
#else
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

	case ITEM_DAUZINGUMASIN:
		return SHORTCUT_ID_DOWSINGMACHINE;

	case ITEM_GURASIDEANOHANA:
    return SHORTCUT_ID_GURASHIDEA;

	default:
		return SHORTCUT_ID_NULL;
	}
#endif
}

