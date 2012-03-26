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
  u16 id;         //< アイテムID
  u16 pocket;     //< ポケット番号
  BOOL visible;   //< 表示状態
}REPOSITORY_ITEM_ST;

//-------------------------------------
///	バッグ用アイテム管理ワーク
// （ITEM_DATA_MAXは最後のアイテムIDをさしているので＋１）
//=====================================
typedef struct{
  ITEM_ST     dummy;      //< 保管用ダミーデータ実体
  MYITEM_PTR  p_myitem;   //< マイアイテムポインタ
  HEAPID      heapID;     //< ヒープID
  REPOSITORY_ITEM_ST  repository_item[ITEM_DATA_MAX+1]; //< 保管アイテムデータ
  u8  item_existence_table[ITEM_DATA_MAX+1];            //< アイテム存在フラグ
}BAGITEM_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void BAGITEM_InitData(BAGITEM_WORK* p_wk,MYITEM_PTR p_myitem ,REPOSITORY_SORTTYPE rep_sort_mode , HEAPID heapID);
extern void BAGITEM_DeleteData(BAGITEM_WORK* p_wk);
extern ITEM_ST * BAGITEM_PosItemGet( BAGITEM_WORK *p_wk, u16 pocket, u16 pos );
extern void BAGITEM_DeleteRepositoryItem( BAGITEM_WORK *p_wk ,u16 pos ,BOOL is_rm_rep_flag );
extern void BAGITEM_AddRepositoryItem( BAGITEM_WORK *p_wk ,u32 item_id ,s16 pocket_no );
extern void BAGITEM_SortRepositoryItem( BAGITEM_WORK *p_wk , u16 SortType );
extern u32 BAGITEM_GetItemPocketNumber( BAGITEM_WORK *p_wk , u32 pocket );
extern s32 CountRepositoryItem( BAGITEM_WORK *p_wk ,u16 pocket_no );


