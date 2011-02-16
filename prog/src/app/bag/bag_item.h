//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		bag_item.h
 *	@brief  バッグ用アイテム
 *	@author	Masayuki Onoue
 *	@date		2011.02.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "item/itemsym.h"
#include "savedata/myitem_savedata.h"

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バッグ用アイテム管理ワーク
//=====================================
typedef struct{
  u16 id;
  s16 pocket;
}REPOSITORY_ITEM_ST;

//-------------------------------------
///	バッグ用アイテム管理ワーク
//=====================================
typedef struct{
  ITEM_ST     dummy;    // 保管用ダミーデータ実体
  MYITEM_PTR  p_myitem;
  HEAPID      heapID;
  REPOSITORY_ITEM_ST  repository_item[ITEM_DATA_MAX]; // 保管に登録されているアイテムID
}BAGITEM_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void BAGITEM_InitData(BAGITEM_WORK* p_wk,MYITEM_PTR p_myitem , HEAPID heapID);
extern void BAGITEM_DeleteData(BAGITEM_WORK* p_wk);
extern ITEM_ST * BAGITEM_PosItemGet( BAGITEM_WORK *p_wk, u16 pocket, u16 pos );
extern void BAGITEM_PosItemDelete( BAGITEM_WORK *p_wk ,u16 pos );
extern void BAGITEM_AddRepositoryItem( BAGITEM_WORK *p_wk ,u32 item_id ,s16 pocket_no );

