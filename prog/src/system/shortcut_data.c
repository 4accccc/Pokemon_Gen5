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
  int i;
  static const ITEM2SHORTCUT_TBL Item2ShortcutTbl[] = {
 	  { ITEM_ZITENSYA, SHORTCUT_ID_ZITENSYA },
    { ITEM_SUGOITURIZAO, SHORTCUT_ID_TURIZAO },
    { ITEM_TAUNMAPPU, SHORTCUT_ID_TOWNMAP },
	  { ITEM_BATORUREKOODAA, SHORTCUT_ID_BTLRECORDER },
    { ITEM_TOMODATITETYOU, SHORTCUT_ID_FRIENDNOTE },
    { ITEM_DAUZINGUMASIN, SHORTCUT_ID_DOWSINGMACHINE },
    { ITEM_GURASIDEANOHANA, SHORTCUT_ID_GURASHIDEA },
    { ITEM_RAIBUKYASUTAA, SHORTCUT_ID_LIVECASTER },
    { ITEM_RAIBUKYASUTAA_1, SHORTCUT_ID_LIVECASTER },
    { ITEM_MEDARUBOKKUSU, SHORTCUT_ID_MEDALBOX },
    { ITEM_IDENSINOKUSABI, SHORTCUT_ID_IDENSINOKUSABI_U },
    { ITEM_IDENSINOKUSABI_1, SHORTCUT_ID_IDENSINOKUSABI_P },
  };

  for( i = 0;i < NELEMS( Item2ShortcutTbl);i++){
    if( Item2ShortcutTbl[i].itemno == item ){
      return Item2ShortcutTbl[i].shortcut_id;
    }
  }
	return SHORTCUT_ID_NULL;
}

