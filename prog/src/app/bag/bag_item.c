//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		bag_item.c
 *	@brief  バッグ用アイテム
 *	@author	Masayuki Onoue
 *	@date		2011.02.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "bag_item.h"
#include "item/item.h"

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void SortSpace( BAGITEM_WORK *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  バッグアイテム初期化
 *
 *	@param	p_wk
 *	@param	hedaID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BAGITEM_InitData(BAGITEM_WORK* p_wk,MYITEM_PTR p_myitem , HEAPID heapID)
{
  int item_id;
  int count;
  u32 data_size;
  
  count           = 0;
  p_wk->heapID    = heapID;
  p_wk->p_myitem  = p_myitem;

#ifdef PM_DEBUG
//  for( item_id = 0; item_id < BAG_TOTAL_NUM ; item_id++ )
//  {
//    MYITEM_SetRepositoryFlag( p_wk->p_myitem , item_id , TRUE );
//  }
//    MYITEM_SetRepositoryFlag( p_wk->p_myitem , 1 , TRUE );
#endif


  // 保管ポケットデータから保管済みのアイテムID一覧を作成する
  for( item_id = 0; item_id <= ITEM_DATA_MAX ; item_id++ )
  {
    BOOL is_in = MYITEM_GetRepositoryFlag( p_wk->p_myitem ,item_id);

    if( is_in )
    {
      s32 pocket = MYITEM_GetPocketID( p_wk->p_myitem , item_id );
      if( pocket != BAG_POKE_MAX )
      {
        p_wk->repository_item[count].id = item_id;
        p_wk->repository_item[count].pocket = pocket;
        count++;
      }
      else
      {
        p_wk->repository_item[count].id = ITEM_DUMMY_DATA;
        p_wk->repository_item[count].pocket = 0;
      }
    }
    else
    {
      p_wk->repository_item[count].id = ITEM_DUMMY_DATA;
      p_wk->repository_item[count].pocket = 0;
    }
  }

  // ダミーデータを初期化する
  p_wk->dummy.id = 0;
  p_wk->dummy.no = 0;

}

//----------------------------------------------------------------------------
/**
 *	@brief  バッグアイテム開放
 *
 *	@param	p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BAGITEM_DeleteData(BAGITEM_WORK* p_wk)
{
}

//----------------------------------------------------------------------------
/**
 *  @brief	指定ポケットの指定位置のアイテムを取得(保管対応版)
 *
 *	@param	*p_wk   バッグアイテムワーク
 *	@param	pocket  ポケット番号
 *	@param	pos     ポケット内の場所
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
ITEM_ST * BAGITEM_PosItemGet( BAGITEM_WORK *p_wk, u16 pocket, u16 pos )
{
  ITEM_ST *item = NULL;

  if( pocket != BAG_POKE_REPOSITORY )
  {
    BOOL  is_in;
    s32   index = 0;
    s32   i;

    for( i = 0; i <= ITEM_DATA_MAX ;i++ )
    {
      item = MYITEM_PosItemGet( p_wk->p_myitem, pocket ,i );
      is_in = MYITEM_GetRepositoryFlag( p_wk->p_myitem ,item->id);
      if( is_in == FALSE )
      {
        if( index == pos )
        {
          break;
        }
        else
        {
          index++;
        }
      }
    }

    if( item == NULL )
    {
      // アイテムが見つからなかった
      item = &p_wk->dummy;
      GF_ASSERT( 0 );
    }
  }
  else
  {
    // 保管の場合は、アイテム位置を検索してから返す
    REPOSITORY_ITEM_ST *rep_st = &p_wk->repository_item[pos];
    item = MYITEM_IdItemGet( p_wk->p_myitem, rep_st->pocket , rep_st->id );
    if( item  == NULL )
    {
      // 見つからなかった場合は、ダミーデータを返す
      item = &p_wk->dummy;
    }
  }
  return item;
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管ポケット内の特定の位置のアイテムを取り除く
 *
 *	@param	*p_wk バッグアイテムワーク
 *	@param	pos   保管ポケット内の位置
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BAGITEM_PosItemDelete( BAGITEM_WORK *p_wk ,u16 pos )
{
  p_wk->repository_item[pos].id = ITEM_DUMMY_DATA;
  p_wk->repository_item[pos].pocket = 0;

  SortSpace( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管ポケットの空いている位置にアイテムを追加する
 *
 *	@param	*p_wk     バッグアイテムワーク
 *	@param	item_id   アイテムID
 *	@param	pocket_no ポケット番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BAGITEM_AddRepositoryItem( BAGITEM_WORK *p_wk ,u32 item_id ,s16 pocket_no )
{
  int i;
  for( i=0; i<=BAG_TOTAL_NUM; i++ )
  {
    if( p_wk->repository_item[i].id == ITEM_DUMMY_DATA)
    {
      p_wk->repository_item[i].id = item_id;
      p_wk->repository_item[i].pocket = pocket_no;
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテムが入っていない部分をつめる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SortSpace( BAGITEM_WORK *p_wk )
{
  int i;
  int j;
  REPOSITORY_ITEM_ST copy;

  for( i=0; i<=BAG_TOTAL_NUM-1; i++ ){
    for( j=i+1; j<=BAG_TOTAL_NUM; j++ ){
      if( p_wk->repository_item[i].id == ITEM_DUMMY_DATA ){
        copy = p_wk->repository_item[i];
        p_wk->repository_item[i] = p_wk->repository_item[j];
        p_wk->repository_item[j] = copy;
      }
    }
  }
}

