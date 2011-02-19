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
#include "itemsort_absolute_def.cdat"

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//--------------------------------------------------------------
/// ソート用ワーク
//==============================================================
typedef struct {
  REPOSITORY_ITEM_ST  st;
  u32 num;
} REPOSITORY_ITEM_SORTDATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void SortSpace( BAGITEM_WORK *p_wk );
static void SortRepositoryItem( BAGITEM_WORK *p_wk , u16 SortType );
static void SortType( BAGITEM_WORK *p_wk );
static void SortRepositoryNo( BAGITEM_WORK *p_wk );

//-----------------------------------------------------------------------------
/**
 *  @brief  MATH_QSort用関数 保管アイテムのソート
 *
 *  @param  elem1
 *  @param  elem2
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static s32 QSort_Num( void * elem1, void * elem2 )
{
  REPOSITORY_ITEM_SORTDATA * p1 = (REPOSITORY_ITEM_SORTDATA *)elem1;
  REPOSITORY_ITEM_SORTDATA * p2 = (REPOSITORY_ITEM_SORTDATA *)elem2;

  if( p1->num == p2->num ){
    return 0;
  }else if( p1->num > p2->num ){
    return 1;
  }
  return -1;
}

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
  int i;
  int item_id;
  int count;
  u32 data_size;
  BOOL is_in;

  count           = 0;
  p_wk->heapID    = heapID;
  p_wk->p_myitem  = p_myitem;

  // 保管ポケットデータから保管済みのアイテムID一覧を作成する
  for( i = 0; i <= ITEM_DATA_MAX ; i++ )
  {
    item_id = ItemSortByAbsolute[i];
    is_in = MYITEM_GetRepositoryFlag( p_wk->p_myitem ,item_id);

    if( is_in )
    {
      s32 pocket = MYITEM_GetPocketID( p_wk->p_myitem , item_id );
      if( pocket != BAG_POKE_MAX )
      {
        p_wk->repository_item[count].id = item_id;
        p_wk->repository_item[count].pocket = pocket;
        p_wk->repository_item[count].visible = TRUE;
        count++;
      }
      else
      {
        p_wk->repository_item[count].id = ITEM_DUMMY_DATA;
        p_wk->repository_item[count].pocket = BAG_POKE_MAX;
        p_wk->repository_item[count].visible = FALSE;
      }
    }
    else
    {
      p_wk->repository_item[count].id = ITEM_DUMMY_DATA;
      p_wk->repository_item[count].pocket = BAG_POKE_MAX;
      p_wk->repository_item[count].visible = FALSE;
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
  REPOSITORY_ITEM_ST *rep_st;
  BOOL  is_in;
  s32   index = 0;
  s32   i;


  if( pocket != BAG_POKE_REPOSITORY )
  {
    for( i = 0; i <= ITEM_DATA_MAX ;i++ )
    {
      item = MYITEM_PosItemGet( p_wk->p_myitem, pocket ,i );

      // リスト外のアイテムを探しにいった場合などで、アイテムを見つけられない
      if( item == NULL )
      {
        return &p_wk->dummy;
      }

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
  }
  else
  {
    // 保管の場合は、アイテム位置を検索してから返す
    BOOL is_get = FALSE;
    for( i = 0; i <= ITEM_DATA_MAX ;i++ )
    {
      rep_st = &p_wk->repository_item[i];
      if( (rep_st->visible == TRUE) && (rep_st->id != ITEM_DUMMY_DATA) )
      {
        if( index == pos )
        {
          is_get = TRUE;
          break;
        }
        else
        {
          index++;
        }
      }
    }
    // リスト外のアイテムを探しにいった場合などで、アイテムを見つけられない
    if( is_get == FALSE )
    {
      // 見つからなかった場合と非表示データはダミーデータを返す
      item = &p_wk->dummy;
    }
    else
    {
      item = MYITEM_IdItemGet( p_wk->p_myitem, rep_st->pocket , rep_st->id );
      if( item  == NULL )
      {
        // 見つからなかった場合と非表示データはダミーデータを返す
        item = &p_wk->dummy;
      }
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
  REPOSITORY_ITEM_ST *rep_st;
  s32   index = 0;
  s32   i;

  for( i = 0; i <= ITEM_DATA_MAX ;i++ )
  {
    rep_st = &p_wk->repository_item[i];
    if( (rep_st->visible == TRUE) && (rep_st->id != ITEM_DUMMY_DATA) )
    {
      if( index == pos )
      {
        // アイテムを一番下に移動
        MYITEM_MoveBottom( p_wk->p_myitem ,p_wk->repository_item[i].id );

        // 保管内から削除
        p_wk->repository_item[i].id = ITEM_DUMMY_DATA;
        p_wk->repository_item[i].pocket = BAG_POKE_MAX;
        p_wk->repository_item[i].visible = FALSE;
        break;
      }
      else
      {
        index++;
      }
    }
  }

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
      p_wk->repository_item[i].visible = TRUE;
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

//----------------------------------------------------------------------------
/**
 *	@brief  特定のポケットで保管に登録されている数を調べる
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
s32 CountRepositoryItem( BAGITEM_WORK *p_wk ,u16 pocket_no )
{
  int i;
  s32 count = 0;

  for( i=0; i<=ITEM_DATA_MAX; i++ ){
    if( p_wk->repository_item[i].pocket == pocket_no )
    {
      if(p_wk->repository_item[i].id != ITEM_DUMMY_DATA)
      {
        count++;
      }
    }
  }

  return count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管バッグの中をソートする
 *
 *	@param	*p_wk 
 *	@param	SortType 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BAGITEM_SortRepositoryItem( BAGITEM_WORK *p_wk , u16 SortType )
{
  int i;

  // 保管バックソートタイプと並びが一致している必要があります
  int visible_on_pocket[] =
  {
    0,                //REPOSITORY_SORTTYPE_FULL
    BAG_POKE_NORMAL,  //REPOSITORY_SORTTYPE_NORMAL
    BAG_POKE_DRUG,    //REPOSITORY_SORTTYPE_DRUG
    BAG_POKE_WAZA,    //REPOSITORY_SORTTYPE_WAZA
    BAG_POKE_NUTS,    //REPOSITORY_SORTTYPE_NUTS
    BAG_POKE_EVENT,   //REPOSITORY_SORTTYPE_EVENT
  };

  for( i=0; i<=BAG_TOTAL_NUM; i++ )
  {
    if( SortType == REPOSITORY_SORTTYPE_FULL )
    {
      p_wk->repository_item[i].visible = TRUE;
    }
    else
    {
      if( p_wk->repository_item[i].pocket == visible_on_pocket[SortType] )
      {
        p_wk->repository_item[i].visible = TRUE;
      }
      else
      {
        p_wk->repository_item[i].visible = FALSE;
      }
    }
  }

  // ソートタイプごとに並び替えを行う
  SortRepositoryItem( p_wk, SortType);
}

//----------------------------------------------------------------------------
/**
 *	@brief  非表示アイテムを除いたアイテムの個数を返す
 *
 *	@param	*p_wk 
 *	@param	pocket 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 BAGITEM_GetItemPocketNumber( BAGITEM_WORK *p_wk , u32 pocket )
{
  int i;
  u32 count;
  count = MYITEM_GetItemPocketNumber(  p_wk->p_myitem, pocket );

  // 保管ポケットの場合は非表示になっているものをカウントしない
  if( pocket == BAG_POKE_REPOSITORY )
  {
    for( i=0; i<=BAG_TOTAL_NUM; i++ )
    {
      if( p_wk->repository_item[i].visible == FALSE )
      {
        if( MYITEM_GetRepositoryFlag( p_wk->p_myitem ,p_wk->repository_item[i].id) )
        {
          count --;
        }
      }
    }
  }

  return count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管アイテムを並び替える
 *
 *	@param	*p_wk       バッグアイテムワークへのポインタ
 *	@param	SortTypeID  ソートタイプ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SortRepositoryItem( BAGITEM_WORK *p_wk , u16 SortTypeID )
{
  switch( SortTypeID )
  {
  case REPOSITORY_SORTTYPE_FULL:
    SortRepositoryNo( p_wk );
    break;
  case REPOSITORY_SORTTYPE_NORMAL:
  case REPOSITORY_SORTTYPE_DRUG:
  case REPOSITORY_SORTTYPE_EVENT:
  case REPOSITORY_SORTTYPE_WAZA:
  case REPOSITORY_SORTTYPE_NUTS:
    SortType( p_wk );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管内アイテムを保管用ソート番号順に並び替える
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SortRepositoryNo( BAGITEM_WORK *p_wk )
{
  int i;
  int length;
  REPOSITORY_ITEM_SORTDATA  *sort;

  // メモリ確保
  sort = GFL_HEAP_AllocMemory( p_wk->heapID, sizeof(REPOSITORY_ITEM_SORTDATA)*ITEM_DATA_MAX );

  // 取得
  length = MYITEM_GetItemPocketNumber(  p_wk->p_myitem, BAG_POKE_REPOSITORY );

  // ソート用データ生成
  for( i=0; i<length; i++ ){
    sort[i].num = (ItemSortByAbsolute[p_wk->repository_item[i].id] << 16) +  p_wk->repository_item[i].id;
    sort[i].st   = p_wk->repository_item[i];
  }

  MATH_QSort( (void*)sort, length, sizeof(REPOSITORY_ITEM_SORTDATA), QSort_Num, NULL );

  // セーブ用データに整形
  for( i=0; i<length; i++ ){
    p_wk->repository_item[i] = sort[i].st;
  }

  // メモリ解放
  GFL_HEAP_FreeMemory( sort );
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管アイテムを種類順に並び替える
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SortType( BAGITEM_WORK *p_wk )
{
  void * dat;
  int i;
  int length;
  REPOSITORY_ITEM_SORTDATA  *sort;
  REPOSITORY_ITEM_ST  *item;
  ARCHANDLE * ah;

  // メモリ確保
  sort = GFL_HEAP_AllocMemory( p_wk->heapID, sizeof(REPOSITORY_ITEM_SORTDATA)*ITEM_DATA_MAX );

  // 取得
  length = MYITEM_GetItemPocketNumber(  p_wk->p_myitem, BAG_POKE_REPOSITORY );

  // ソート用データ生成
  ah = ITEM_OpenItemDataArcHandle( p_wk->heapID );
  for( i=0; i<length; i++ ){
    item = &p_wk->repository_item[i];
    dat = ITEM_GetItemDataArcHandle( ah, item->id, p_wk->heapID );
    sort[i].num = (ITEM_GetBufParam(dat,ITEM_PRM_ITEM_TYPE)<<28) + (ITEM_GetBufParam(dat,ITEM_PRM_SORT_NUMBER)<<16) + item->id;
    sort[i].st   = *item;
    GFL_HEAP_FreeMemory( dat );
  }
  GFL_ARC_CloseDataHandle( ah );

  MATH_QSort( (void*)sort, length, sizeof(REPOSITORY_ITEM_SORTDATA), QSort_Num, NULL );

  // セーブ用データに整形
  for( i=0; i<length; i++ ){
    p_wk->repository_item[i] = sort[i].st;
  }

  // メモリ解放
  GFL_HEAP_FreeMemory( sort );
}

