//==============================================================================
/**
 * @file	myitem_savedata.c
 * @brief	手持ちアイテム
 * @author	matsuda
 * @date	2008.12.04(木)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/myitem_savedata.h"
#include "item/item.h"
#include "myitem_savedata_local.h"



//==============================================================================
//	構造体定義
//==============================================================================

// フィールドのバッグのカーソル位置
typedef struct {
	s16	scr[BAG_POKE_MAX];  //12
	s16	pos[BAG_POKE_MAX];  //24
	u16	pocket;             //25
	u8	dummy[3];           //28byte
}FLDBAG_CURSOR;

// 戦闘のバッグのカーソル位置
typedef struct {
	s16	scr[BATTLE_BAG_POKE_MAX];
	s16	pos[BATTLE_BAG_POKE_MAX];
	u16	item;			// 最後に使ったアイテム
	u16	page;			// 最後に使ったアイテムのポケット
//	u16	pocket;
}BTLBAG_CURSOR;

// バッグのカーソル位置データ
struct _BAG_CURSOR {
	FLDBAG_CURSOR	fld;
	BTLBAG_CURSOR	btl;
};

//==============================================================================
//	定数定義
//==============================================================================
#define	ITEM_CHECK_ERR	( 0xffffffff )		// チェックエラー



//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static u32 MyItemDataGet( MYITEM_PTR myitem, u16 id, ITEM_ST ** item, u32 * max, HEAPID heap );
static u32 MyPocketDataGet( MYITEM_PTR myitem, s32 pocket, ITEM_ST ** item, u32 * max);
static void bottomInsert( ITEM_ST * item, const u32 max );
static u32 GetRepositoryNum( MYITEM_PTR myitem );
static BOOL SearchInPocket( MYITEM_PTR myitem, u16 item_no, u16 pocket );
static void SortRepository(MYITEM_PTR myitem, ITEM_ST *item, u32 max );
static void ChengeItem( ITEM_ST * p1, ITEM_ST * p2 );



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   手持ちアイテムワークのサイズ取得
 * @retval  サイズ
 */
//--------------------------------------------------------------
int MYITEM_GetWorkSize(void)
{
	return sizeof(MYITEM);
}

//------------------------------------------------------------------
/**
 * @brief	手持ちアイテムワークを生成する
 * @param	myitem	手持ちアイテム構造体へのポインタ
 * @return	MYITEMへのポインタ
 */
//------------------------------------------------------------------
MYITEM_PTR MYITEM_AllocWork(HEAPID heapID)
{
	MYITEM_PTR item;
	item = GFL_HEAP_AllocMemory(heapID, sizeof(MYITEM));
	MYITEM_Init(item);
	return item;
}

//------------------------------------------------------------------
/**
 * @brief	初期化処理
 * @param	item	MYITEMへのポインタ
 */
//------------------------------------------------------------------
void MYITEM_Init(MYITEM_PTR item)
{
	GFL_STD_MemClear(item, sizeof(MYITEM));
}

//------------------------------------------------------------------
/**
 * @brief	MYITEMのコピー
 * @param	from	コピー元へのポインタ
 * @param	to		コピー先へのポインタ
 */
//------------------------------------------------------------------
void MYITEM_Copy(const MYITEM_PTR from, MYITEM_PTR to)
{
	GFL_STD_MemCopy(from, to, sizeof(MYITEM));
}


//------------------------------------------------------------------
/**
 * @brief	MYITEMのITEM_STのコピー(保管されているものはコピーされない)
 * @param	myitem	  MYITEMのポインタ
 * @param	itemst		アイテムリストの先頭ポインタ
 * @param	pocket		ポケット番号
 * @param	bMyGet		myitemからデータがほしい場合TRUE  myitemに格納する場合FALSE
 */
//------------------------------------------------------------------
void MYITEM_ITEM_STCopy(MYITEM_PTR myitem, ITEM_ST* itemst, int pocket, BOOL bMyGet)
{
  int i = 0;
  ITEM_ST* MyItemSt;
  u32 max;

  MyPocketDataGet(myitem, pocket, &MyItemSt, &max);

  // 保管に登録されているものは後ろにつめる
  SortRepository( myitem , MyItemSt , max );

  if(bMyGet)
  {
    for( i = 0; i < max ; i++ )
    {
      BOOL is_in = MYITEM_GetRepositoryFlag( myitem , MyItemSt[i].id );
      if( is_in == FALSE )
      {
        GFL_STD_MemCopy(&MyItemSt[i], &itemst[i], sizeof(ITEM_ST) );
      }
      else
      {
        itemst[i].id = ITEM_DUMMY_DATA;
        itemst[i].no = 0;
      }
    }
  }
  else
  {
    for( i = 0; i < max ; i++ )
    {
      // ダミーデータは書き込まない
      if( itemst[i].id != ITEM_DUMMY_DATA )
      {
        BOOL is_in = MYITEM_GetRepositoryFlag( myitem , itemst[i].id );
        if( is_in == FALSE )
        {
          GFL_STD_MemCopy(&itemst[i],&MyItemSt[i],sizeof(ITEM_ST) );
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  	MYITEMのITEM_STのコピー(完全コピー)
 *
 *  @param	myitem	  MYITEMのポインタ
 *  @param	itemst		アイテムリストの先頭ポインタ
 *  @param	pocket		ポケット番号
 *  @param	bMyGet		myitemからデータがほしい場合TRUE  myitemに格納する場合FALSE
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MYITEM_ITEM_STFullCopy(MYITEM_PTR myitem, ITEM_ST* itemst, int pocket, BOOL bMyGet)
{
  ITEM_ST* MyItemSt;
  u32 max;

  MyPocketDataGet(myitem, pocket, &MyItemSt, &max);

  if(bMyGet)
  {
    GFL_STD_MemCopy(MyItemSt, itemst, sizeof(ITEM_ST) * max );
  }
  else
  {
    GFL_STD_MemCopy(itemst, MyItemSt,  sizeof(ITEM_ST) * max );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管に登録されているアイテムを後ろにつめる
 *
 *	@param	myitem
 *	@param	* item
 *	@param	max 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SortRepository(MYITEM_PTR myitem, ITEM_ST *item, u32 max )
{
  u32	i, j;

  for( i=0; i<max-1; i++ ){
    for( j=i+1; j<max; j++ ){
      BOOL is_in = MYITEM_GetRepositoryFlag( myitem , item[i].id );
      if( item[j].id != ITEM_DUMMY_DATA )
      {
        if( is_in == TRUE ){
          ChengeItem( &item[i], &item[j] );
        }
      }
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief	便利ボタンに割り当てられているアイテム取得
 * @param	myitem	MYITEMへのポインタ
 * @return	アイテム番号
 */
//------------------------------------------------------------------
/*
u32 MYITEM_CnvButtonItemGet( const MYITEM_PTR myitem, int index )
{
	return myitem->cnv_button[index];
}
*/

//------------------------------------------------------------------
/**
 * @brief	便利ボタンにアイテムセット
 * @param	myitem	MYITEMへのポインタ
 * @param	setitem	セットするアイテム
 * @return	アイテム番号
 */
//------------------------------------------------------------------
/*
void MYITEM_CnvButtonItemSet( MYITEM_PTR myitem, int index , u32 setitem )
{
  GF_ASSERT(index < DUMMY_SHORTCUT_MAX);
	myitem->cnv_button[index] = setitem;
}
*/

//------------------------------------------------------------------
/**
 * @brief	各ポケットのITEM_STと最大数取得
 * @param	pocket		ポケット番号
 * @param	item	MYITEM取得場所
 * @param	max		最大数取得場所
 * @param	heap		ヒープID
 * @return none
 */
//------------------------------------------------------------------
static u32 MyPocketDataGet( MYITEM_PTR myitem, s32 pocket, ITEM_ST ** item, u32 * max)
{
	switch( pocket ){
	case BAG_POKE_EVENT:	// 大切な物
		*item = myitem->MyEventItem;
		*max = BAG_EVENT_ITEM_MAX;
		break;
	case BAG_POKE_NORMAL:	// 道具
		*item = myitem->MyNormalItem;
		*max = BAG_NORMAL_ITEM_MAX;
		break;
	case BAG_POKE_NUTS:		// 木の実
		*item = myitem->MyNutsItem;
		*max = BAG_NUTS_ITEM_MAX;
		break;
	case BAG_POKE_DRUG:		// 薬
		*item = myitem->MyDrugItem;
		*max = BAG_DRUG_ITEM_MAX;
		break;
	case BAG_POKE_WAZA:		// 技マシン
		*item = myitem->MySkillItem;
		*max = BAG_WAZA_ITEM_MAX;
		break;
  case BAG_POKE_REPOSITORY:	//  保管 
    // 保管アイテムは実態を持っていない
		*item = NULL;
		*max = BAG_TOTAL_NUM;
    break;
	default:
		GF_ASSERT(0);
		*item = NULL;
		break;
	}
	return pocket;
}

//------------------------------------------------------------------
/**
 * @brief	MYITEMと最大数取得
 * @param	id		アイテム番号
 * @param	item	MYITEM取得場所
 * @param	max		最大数取得場所
 * @param	heap		ヒープID
 * @return	ポケット番号
 */
//------------------------------------------------------------------
static u32 MyItemDataGet( MYITEM_PTR myitem, u16 id, ITEM_ST ** item, u32 * max, HEAPID heap )
{
	s32 pocket = ITEM_GetParam( id, ITEM_PRM_POCKET, heap );

	MyPocketDataGet(myitem, pocket, item, max);
	return pocket;
}


//------------------------------------------------------------------
/**
 * @brief	加えるアイテムをチェック
 * @param	item		手持ちアイテム構造体へのポインタ
 * @param	siz			手持ちアイテム構造体のサイズ（個数）
 * @param	id			指定するアイテムのナンバー
 * @param	num			加えるアイテムの個数
 * @param	max			アイテム最大数
 * @retval	成功 = アイテム位置
 * @retval	失敗 = NULL
 */
//------------------------------------------------------------------
static ITEM_ST * AddItemSearch( ITEM_ST * item, u32 siz, u16 id, u16 num, u16 max )
{
	u32	i;
	u32	pos = ITEM_CHECK_ERR;

	for( i=0; i<siz; i++ ){
		if( item[i].id == id ){
			if( ( item[i].no + num ) > max ){
				return NULL;
			}
			return &item[i];
		}
		if( pos == ITEM_CHECK_ERR ){
			if( item[i].id == 0 && item[i].no == 0 ){
				pos = i;
			}
		}
	}

	if( pos == ITEM_CHECK_ERR ){
		return NULL;
	}
	return &item[pos];
}

//------------------------------------------------------------------
/**
 * @brief	アイテムを加える場所を取得
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	成功 = アイテム位置
 * @retval	失敗 = NULL
 */
//------------------------------------------------------------------
static ITEM_ST * AddItemPosGet( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	ITEM_ST * add;
	u32	max;
	u32	pocket;

	pocket = MyItemDataGet( myitem, item_no, &add, &max, heap );
	if( pocket == BAG_POKE_WAZA ){
		return AddItemSearch( add, max, item_no, num, ITEM_MAX_WAZAMACHINE );
	}
	return AddItemSearch( add, max, item_no, num, ITEM_MAX_NORMAL );
}

//------------------------------------------------------------------
/**
 * @brief	手持ちにアイテムを加えられるかチェック
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	成功
 * @retval	FALSE	失敗
 */
//------------------------------------------------------------------
BOOL MYITEM_AddCheck( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	if( AddItemPosGet( myitem, item_no, num, heap ) == NULL ){
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	手持ちにアイテムを加える
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	成功
 * @retval	FALSE	失敗
 */
//------------------------------------------------------------------
BOOL MYITEM_AddItem( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	ITEM_ST * add = AddItemPosGet( myitem, item_no, num, heap );

	if( add == NULL ){ return FALSE; }
	add->id = item_no;
	add->no += num;

  ONOUE_Printf("ID:%u=%u個\n",add->id,add->no);


//	{
//		u32	pocket;
//		u32	max;
//		pocket = MyItemDataGet( myitem, item_no, &add, &max, heap );
//		// 木の実
//		if( pocket == BAG_POKE_NUTS ){
//			MYITEM_SortNumber( add, max );
//		// 技マシン
//		}else if( pocket == BAG_POKE_WAZA ){
//			MYITEM_SortWazaMachine( add, max );
//		}
/*
		// 木の実
		if( pocket == BAG_POKE_NUTS || pocket == BAG_POKE_WAZA ){
			MYITEM_SortNumber( add, max );
		}
*/
 
    /* スワンから、アイテムをリストの一番上に持っていく処理は廃止します */
//    else if(before == 0){  //加えたアイテムが一個目だった場合、リストの最初にもっていく
//      ITEM_ST * item;
//      u32	num;
//      MyPocketDataGet(myitem, pocket, &item, &num);
//      num = MYITEM_GetItemThisPocketNumber(item, num);
//      bottomInsert( item,  num );
//    }
//	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	取り除くアイテムをチェック
 * @param	item		手持ちアイテム構造体へのポインタ
 * @param	siz			手持ちアイテム構造体のサイズ（個数）
 * @param	id			指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @retval	成功 = アイテム位置
 * @retval	失敗 = NULL
 */
//------------------------------------------------------------------
static ITEM_ST * SubItemSearch( ITEM_ST * item, u32 siz, u16 id, u16 num )
{
	u32	i;

	for( i=0; i<siz; i++ ){
		if( item[i].id == id ){
			if( item[i].no >= num ){
				return &item[i];
			}else{
				return NULL;
			}
		}
	}
	return NULL;
}

//------------------------------------------------------------------
/**
 * @brief	アイテムを取り除く場所を取得
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	成功 = アイテム位置
 * @retval	失敗 = NULL
 */
//------------------------------------------------------------------
static ITEM_ST * SubItemPosGet( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	ITEM_ST * sub;
	u32	max;

	MyItemDataGet( myitem, item_no, &sub, &max, heap );
	return SubItemSearch( sub, max, item_no, num );
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定したアイテムIDのアイテムが保管ポケットに入っているか調べる
 *
 *	@param	myitem  手持ちアイテム構造体へのポインタ
 *	@param	item_no アイテムID
 *
 *	@retval TRUE：入っている
 */
//-----------------------------------------------------------------------------
BOOL MYITEM_GetRepositoryFlag( MYITEM_PTR myitem, u16 item_no )
{
  // ビットが入っている配列番号を計算(32で割る)
  int bit_pos = item_no >> 5;
  s32 bit_mask = 1;
  s32 bit_data;
  u32 pocket_no;

  // ビットを取り出す
  bit_mask = bit_mask << (item_no%32);
  bit_data = myitem->RepositoryFlag[bit_pos] & bit_mask;

  // 保管フラグを調査
  if( bit_data )
  {
//    // ポケット内に存在するか調べる
//    pocket_no = MYITEM_GetPocketID( myitem , item_no );
//    if( pocket_no != BAG_POKE_MAX )
//    {
//      return TRUE;
//    }

    // 入れるときに、持っていないものは入れないことで存在確認をする必要がない
    // ここでポケットや個数などを調べるとスクロール速度が大幅に落ちてしまう
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテムを保管ポケットフラグを設定する
 *
 *	@param	myitem  手持ちアイテム構造体へのポインタ
 *	@param	item_no アイテムID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MYITEM_SetRepositoryFlag( MYITEM_PTR myitem, u16 item_no , BOOL set )
{
  // ビットが入っている配列番号を計算
  int bit_pos  = item_no >> 5;

  if( item_no != ITEM_DUMMY_DATA )
  {
    if( set == FALSE )
    {
      s32 bit_mask = 0xffffffff;
      s32 del_bit  = 1;
      s32 bit_data;

      // ビットを設定
      del_bit   = del_bit << (item_no%32);
      bit_mask  = bit_mask ^ del_bit;
      myitem->RepositoryFlag[bit_pos]  = myitem->RepositoryFlag[bit_pos] & bit_mask;
    }
    else
    {
      u32 pocket_no;
      // ポケット内に存在するか調べる
      pocket_no = MYITEM_GetPocketID( myitem , item_no );
      if( pocket_no != BAG_POKE_MAX )
      {
        s32 bit_mask = 1;
        s32 bit_data;
        // ビットを設定
        bit_mask = bit_mask << (item_no%32);
        myitem->RepositoryFlag[bit_pos] = myitem->RepositoryFlag[bit_pos] | bit_mask;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータからポケット位置を検索する
 *
 *	@param	myitem  手持ちアイテム構造体へのポインタ
 *	@param	item_no 検索するアイテム番号
 *
 *	@retval ポケット番号
 */
//-----------------------------------------------------------------------------
u32 MYITEM_GetPocketID( MYITEM_PTR myitem, u16 item_no )
{
  BOOL is_in;

  is_in = SearchInPocket( myitem , item_no , BAG_POKE_NORMAL);
  if( is_in )
  {
    return BAG_POKE_NORMAL;
  }

  is_in = SearchInPocket( myitem , item_no , BAG_POKE_DRUG);
  if( is_in )
  {
    return BAG_POKE_DRUG;
  }

  is_in = SearchInPocket( myitem , item_no , BAG_POKE_WAZA);
  if( is_in )
  {
    return BAG_POKE_WAZA;
  }

  is_in = SearchInPocket( myitem , item_no , BAG_POKE_NUTS);
  if( is_in )
  {
    return BAG_POKE_NUTS;
  }

  is_in = SearchInPocket( myitem , item_no , BAG_POKE_EVENT);
  if( is_in )
  {
    return BAG_POKE_EVENT;
  }

  return BAG_POKE_MAX;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定のアイテムがポケットに入っているかを調べる
 *
 *	@param	myitem
 *	@param	item_no
 *	@param	pocket 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SearchInPocket( MYITEM_PTR myitem, u16 item_no, u16 pocket )
{
  ITEM_ST *item;
  u32	max;
  int i;

  MyPocketDataGet( myitem, pocket, &item, &max );

  for( i = 0; i < max ; i++ )
  {
    if( item[i].id == item_no )
    {
      return TRUE;
    }
    if( item[i].id == ITEM_DUMMY_DATA )
    {
      return FALSE;
    }
  }
  return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	手持ちからアイテムを取り除く
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	成功
 * @retval	FALSE	失敗
 */
//------------------------------------------------------------------
BOOL MYITEM_SubItem( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	ITEM_ST * sub = SubItemPosGet( myitem, item_no, num, heap );

	if( sub == NULL ){ return FALSE; }

	sub->no -= num;
	if( sub->no == 0 ){
    // アイテムがなくなった場合は保管ポケットからもはずす
    MYITEM_SetRepositoryFlag( myitem , item_no , FALSE );
		sub->id = 0;
  }

	{
		u32	max;

		MyItemDataGet( myitem, item_no, &sub, &max, heap );
		MYITEM_SortSpace( sub, max );
	}

	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	手持ちからアイテムを取り除く（ポケット指定）
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	max			検索数
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	成功
 * @retval	FALSE	失敗
 */
//------------------------------------------------------------------
BOOL MYITEM_SubItemDirect( ITEM_ST * myitem, u32 max, u16 item_no, u16 num, HEAPID heap )
{
	ITEM_ST * sub = SubItemSearch( myitem, max, item_no, num );

	if( sub == NULL ){ return FALSE; }

	sub->no -= num;
	if( sub->no == 0 ){
		sub->id = 0;
	}

	MYITEM_SortSpace( myitem, max );

	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	手持ちアイテム内での存在チェック
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	存在する
 * @retval	FALSE	存在しない
 */
//------------------------------------------------------------------
BOOL MYITEM_CheckItem( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap )
{
	if( SubItemPosGet( myitem, item_no, num, heap ) == NULL ){
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief	ポケットにアイテムが存在するかチェック
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	pocket		ポケット番号
 * @retval	TRUE	存在する
 * @retval	FALSE	存在しない
 */
//------------------------------------------------------------------
BOOL MYITEM_CheckItemPocket( MYITEM_PTR myitem, u32 pocket )
{
	ITEM_ST * item;
	u32	max;
	u32	i;

	MyPocketDataGet(myitem, pocket, &item, &max);	
	if(item)
	{
		for( i=0; i<max; i++ )
		{
			if( item[i].id != 0 )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ポケット構造体のアイテム数
 * @param	myitem		ポケット構造体へのポインタ
 * @return     数
 */
//------------------------------------------------------------------
u32 MYITEM_GetItemThisPocketNumber( ITEM_ST * item,int max )
{
	u32	num=0;
	u32	i;

  if(item)
	{
		for( i=0; i < max; i++ )
		{
			if( item[i].id != ITEM_DUMMY_DATA )
			{
				num++;
			}
		}
	}
	return num;
}

//----------------------------------------------------------------------------
/**
 *  @brief	ポケット構造体のアイテム数(保管に登録されているものはカウントしない)
 *
 *  @param	myitem		ポケット構造体へのポインタ
 *	@param	pocket    ポケット番号
 *	@param	is_count_repository TRUE = 保管ポケットに入っているものも数える
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 MYITEM_GetItemThisPocketNumberNoRepository( MYITEM_PTR myitem ,u32 pocket , BOOL is_count_repository )
{
	u32	num=0;
	u32	i;
	u32	max;
	ITEM_ST *item;

  MyPocketDataGet(myitem, pocket, &item, &max);	
  if(item)
	{
		for( i=0; i < max; i++ )
		{
			if( item[i].id != ITEM_DUMMY_DATA )
      {
        if( is_count_repository == FALSE )
        {
          BOOL is_in = MYITEM_GetRepositoryFlag( myitem , item[i].id );
          if( is_in == FALSE )
          {
            num++;
          }
        }
        else
        {
            num++;
        }
			}
		}
	}
	return num;
}

//------------------------------------------------------------------
/**
 * @brief	ポケットのアイテム数
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	pocket		ポケット番号
 * @return     数
 */
//------------------------------------------------------------------
u32 MYITEM_GetItemPocketNumber( MYITEM_PTR myitem, u32 pocket )
{
	ITEM_ST * item;
	u32	max,num=0;
  u32 count = 0;
	u32	i;

  if( pocket != BAG_POKE_REPOSITORY )
  {
    count = MYITEM_GetItemThisPocketNumberNoRepository(myitem,pocket , FALSE);
  }
  else
  {
    count = GetRepositoryNum( myitem );
  }

  return count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  保管ポケットに登録されているアイテムの数を取得
 *
 *	@param	myitem 手持ちアイテム構造体へのポインタ
 *
 *	@retval 数
 */
//-----------------------------------------------------------------------------
static u32 GetRepositoryNum( MYITEM_PTR myitem )
{
  int item_id;
  u32 count = 0;

  for( item_id = 0; item_id <= ITEM_DATA_MAX ; item_id++ )
  {
    if( item_id != ITEM_DUMMY_DATA )
    {
      BOOL is_in = MYITEM_GetRepositoryFlag( myitem ,item_id);

      if( is_in )
      {
        count++;
      }
    }
  }
  return count;
}

//------------------------------------------------------------------
/**
 * @brief	手持ちの個数チェック
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	item_no		指定するアイテムのナンバー
 * @param	heap		ヒープID
 * @return	手持ち数
 */
//------------------------------------------------------------------
u16 MYITEM_GetItemNum( MYITEM_PTR myitem, u16 item_no, HEAPID heap )
{
	ITEM_ST * sub = SubItemPosGet( myitem, item_no, 1, heap );

	if( sub == NULL ){
		return 0;
	}
	return sub->no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  手持ちの個数をチェック（アーカイブを開かない）
 *
 *	@param	myitem  手持ちアイテム構造体へのポインタ
 *	@param	item_no アイテム番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 MYITEM_GetItemNumFromSave( MYITEM_PTR myitem, u16 item_no )
{
  s32 pocket = MYITEM_GetPocketID( myitem, item_no );

  if( pocket != BAG_POKE_MAX )
  {
    ITEM_ST * item;
    item = MYITEM_IdItemGet( myitem ,pocket ,item_no );
    if( item == NULL )
    {
      return 0;
    }
    return item->no;
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------
/**
 * @brief	手持ちの個数チェック（ポケット指定）
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	max			検索数
 * @param	item_no		指定するアイテムのナンバー
 * @param	num			アイテムの個数
 * @param	heap		ヒープID
 * @retval	TRUE	成功
 * @retval	FALSE	失敗
 */
//------------------------------------------------------------------
u16 MYITEM_GetItemNumDirect( ITEM_ST * myitem, u32 max, u16 item_no, HEAPID heap )
{
	ITEM_ST * sub = SubItemSearch( myitem, max, item_no, 1 );

	if( sub == NULL ){ return 0; }

	return sub->no;
}

//------------------------------------------------------------------
/**
 * @brief	指定個所のアイテムを入れ替え
 * @param	p1		入れ替えるデータ1
 * @param	p2		入れ替えるデータ2
 * @return	none
 */
//------------------------------------------------------------------
static void ChengeItem( ITEM_ST * p1, ITEM_ST * p2 )
{
	ITEM_ST	copy;

	copy = *p1;
	*p1  = *p2;
	*p2  = copy;
}

//------------------------------------------------------------------
/**
 * @brief	一番下のアイテムを上に置く
 * @param	item	アイテムデータ
 * @param	max		最大値
 * @return	none
 */
//------------------------------------------------------------------
static void bottomInsert( ITEM_ST * item, const u32 num )
{
	int	i, j;
  ITEM_ST bottom;

  bottom.id = ITEM_DUMMY_DATA;

  if(num <= 1){
    return;   //０や１はやる必要がない
  }
  
	for(i = (num-1) ; i >= 0; i-- ){
    if(bottom.id == ITEM_DUMMY_DATA){
      GFL_STD_MemCopy(&item[i], &bottom, sizeof(ITEM_ST));
    }
    else{
      GFL_STD_MemCopy(&item[i], &item[i+1], sizeof(ITEM_ST));
    }
  }
  GFL_STD_MemCopy(&bottom, &item[0], sizeof(ITEM_ST));
}

//----------------------------------------------------------------------------
/**
 *	@brief  特定のアイテムIDのアイテムを一番下に移動する
 *
 *	@param	myitem
 *	@param	item_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MYITEM_MoveBottom( MYITEM_PTR myitem, u16 item_no )
{
  int i;
  s32 pocket = MYITEM_GetPocketID( myitem, item_no );

  if( pocket != BAG_POKE_MAX )
  {
    ITEM_ST * item;
    ITEM_ST * move_st;
    u32	max;
    move_st = MYITEM_IdItemGet( myitem ,pocket ,item_no );
    if( move_st == NULL )
    {
      return;
    }

    MyPocketDataGet( myitem, pocket, &item, &max );
    for( i = 0; i < max ; i++ )
    {
      if( item[i].id == ITEM_DUMMY_DATA )
      {
        item[i] = *move_st;
        move_st->id = 0;
        move_st->no = 0;
        break;
      }
    }

    MYITEM_SortSpace( item, max );
  }
}

//------------------------------------------------------------------
/**
 * @brief		アイテム番号０に個数が入ってたら消す
 * @param		item	アイテムデータ
 * @param		max		最大値
 * @return	none
 */
//------------------------------------------------------------------
static void DeleteNullItem( ITEM_ST * item, const u32 max )
{
	u32	i;

	for( i=0; i<max; i++ ){
		if( item[i].id == 0 ){
			item[i].no = 0;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	アイテムソート（スペースをつめる）
 * @param	item	アイテムデータ
 * @param	max		最大値
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_SortSpace( ITEM_ST * item, const u32 max )
{
	u32	i, j;

	DeleteNullItem( item, max );

	for( i=0; i<max-1; i++ ){
		for( j=i+1; j<max; j++ ){
			if( item[i].no == 0 ){
				ChengeItem( &item[i], &item[j] );
			}
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	アイテムソート（番号順）
 * @param	p	ポケットデータ
 * @param	max		最大値
 * @return	none
 */	
//------------------------------------------------------------------
void MYITEM_SortNumber( ITEM_ST * item, const u32 max )
{
	u32	i, j;

	DeleteNullItem( item, max );

	for( i=0; i<max-1; i++ ){
		for( j=i+1; j<max; j++ ){
			if( item[i].no == 0 || ( item[j].no != 0 && item[i].id > item[j].id ) ){
				ChengeItem( &item[i], &item[j] );
			}
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	アイテムソート（技マシン番号順）
 * @param	item	アイテムデータ
 * @param	max		最大値
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_SortWazaMachine( ITEM_ST * item, const u32 max )
{
	u32	i, j;

	DeleteNullItem( item, max );

	for( i=0; i<max-1; i++ ){
		for( j=i+1; j<max; j++ ){
			if( item[i].no == 0 || ( item[j].no != 0 && ITEM_GetWazaMashineNo(item[i].id) > ITEM_GetWazaMashineNo(item[j].id) ) ){
				ChengeItem( &item[i], &item[j] );

			}
		}
	}
}


//------------------------------------------------------------------
/**
 * @brief		手持ちアイテム全体のNULLアイテムを消して詰める
 * @param		myitem	手持ちアイテムデータ
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_CheckSafety( MYITEM_PTR myitem )
{
	ITEM_ST * item;
	u32	max;

	// 大切な物
	MyPocketDataGet( myitem, BAG_POKE_EVENT, &item, &max );
	MYITEM_SortSpace( item, max );
	// 道具
	MyPocketDataGet( myitem, BAG_POKE_NORMAL, &item, &max );
	MYITEM_SortSpace( item, max );
	// 木の実
	MyPocketDataGet( myitem, BAG_POKE_NUTS, &item, &max );
	MYITEM_SortSpace( item, max );
	// 薬
	MyPocketDataGet( myitem, BAG_POKE_DRUG, &item, &max );
	MYITEM_SortSpace( item, max );
	// 技マシン
	MyPocketDataGet( myitem, BAG_POKE_WAZA, &item, &max );
	MYITEM_SortSpace( item, max );
}


//------------------------------------------------------------------
/**
 * @brief	指定ポケットの指定位置のアイテムを取得
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	pocket		ポケットID
 * @param	pos			位置
 * @return	アイテムデータ
 */
//------------------------------------------------------------------
ITEM_ST * MYITEM_PosItemGet( MYITEM_PTR myitem, u16 pocket, u16 pos )
{
	ITEM_ST * item;
	u16	max;

	switch( pocket ){
	case BAG_POKE_EVENT:	// 大切な物
		item = myitem->MyEventItem;
		max  = BAG_EVENT_ITEM_MAX;
		break;
	case BAG_POKE_NORMAL:	// 道具
		item = myitem->MyNormalItem;
		max  = BAG_NORMAL_ITEM_MAX;
		break;
	case BAG_POKE_NUTS:		// 木の実
		item = myitem->MyNutsItem;
		max  = BAG_NUTS_ITEM_MAX;
		break;
	case BAG_POKE_DRUG:		// 薬
		item = myitem->MyDrugItem;
		max  = BAG_DRUG_ITEM_MAX;
		break;
	case BAG_POKE_WAZA:		// 技マシン
		item = myitem->MySkillItem;
		max  = BAG_WAZA_ITEM_MAX;
		break;
  case BAG_POKE_REPOSITORY:	//  保管 
    GF_ASSERT(0); // 実態がないので取得できません
    return NULL;
	}

	if( pos >= max ){ return NULL; }

	return &item[pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテムIDから、データを検索する
 *
 *	@param	myitem  手持ちアイテム構造体へのポインタ
 *	@param	pocket  ポケット番号
 *	@param	item_no アイテム番号
 *
 *	@retval 発見したアイテム構造体
 */
//-----------------------------------------------------------------------------
ITEM_ST * MYITEM_IdItemGet( MYITEM_PTR myitem ,u16 pocket ,u16 item_no )
{
  ITEM_ST *MyItemSt;
  u32     max;
  int     i;

  // アイテムSTを取得
  MyPocketDataGet(myitem, pocket, &MyItemSt, &max);

  // アイテムを検索
	for( i=0; i<max; i++ ){
		if( MyItemSt[i].id == item_no ){
			return &MyItemSt[i];
		}
	}

  return NULL;
}

//------------------------------------------------------------------
/**
 * @brief		１つのアイテムの最大所持数を取得
 *
 * @param		pocket	ポケット
 *
 * @return	１つのアイテムの最大所持数
 */
//------------------------------------------------------------------
u32 MYITEM_GetItemMax( u16 item )
{
	if( ITEM_CheckWazaMachine( item ) == TRUE ){
		return ITEM_MAX_WAZAMACHINE;
	}
	return ITEM_MAX_NORMAL;
}

//------------------------------------------------------------------
/**
 * @brief	バッグのカーソル位置データ確保
 * @param	heapID	ヒープID
 * @return	カーソル位置データ
 */
//------------------------------------------------------------------
BAG_CURSOR * MYITEM_BagCursorAlloc( HEAPID heapID )
{
	return GFL_HEAP_AllocClearMemory( heapID, sizeof(BAG_CURSOR) );
}

//------------------------------------------------------------------
/**
 * @brief	フィールドバッグのカーソル位置取得
 * @param	wk		カーソルデータ
 * @param	pocket	ポケットID ITEMPOCKET_NORMALなど (item\itempocket_def.h)
 * @param	pos		カーソル位置取得場所
 * @param	scr		スクロールカウンタ取得場所
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_FieldBagCursorGet( BAG_CURSOR * wk, u16 pocket, s16 * pos, s16 * scr )
{
	*pos = wk->fld.pos[pocket];
	*scr = wk->fld.scr[pocket];
}

//------------------------------------------------------------------
/**
 * @brief	フィールドバッグのポケット位置取得
 * @param	wk		カーソルデータ
 * @return	none
 */
//------------------------------------------------------------------
u16 MYITEM_FieldBagPocketGet( BAG_CURSOR * wk )
{
	return wk->fld.pocket;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドバッグのカーソル位置セット
 * @param	wk		カーソルデータ
 * @param	pocket	ポケットID
 * @param	pos		カーソル位置
 * @param	scr		スクロールカウンタ
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_FieldBagCursorSet( BAG_CURSOR * wk, u16 pocket, s16 pos, s16 scr )
{
  GF_ASSERT(pocket >= 0);
  GF_ASSERT(pocket < BAG_POKE_MAX);
  GF_ASSERT(scr >= 0);
  GF_ASSERT(pos >= 0);
  wk->fld.pos[pocket] = pos;
	wk->fld.scr[pocket] = scr;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドバッグのポケット位置セット
 * @param	wk		カーソルデータ
 * @param	pocket	ポケットID
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_FieldBagPocketSet( BAG_CURSOR * wk, u16 pocket )
{
  GF_ASSERT(pocket >= 0);
  GF_ASSERT(pocket < BAG_POKE_MAX);
  wk->fld.pocket = pocket;
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグのカーソル位置取得
 * @param	wk		カーソルデータ
 * @param	pocket	ポケットID
 * @param	pos		カーソル位置取得場所
 * @param	scr		スクロールカウンタ取得場所
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_BattleBagCursorGet( BAG_CURSOR * wk, u16 pocket, s16 * pos, s16 * scr )
{
  *pos = wk->btl.pos[pocket];
	*scr = wk->btl.scr[pocket];
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグの最後に使った道具取得
 * @param	wk		カーソルデータ
 * @return	最後に使った道具
 */
//------------------------------------------------------------------
u16 MYITEM_BattleBagLastItemGet( BAG_CURSOR * wk )
{
	return wk->btl.item;
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグの最後に使った道具のポケット取得
 * @param	wk		カーソルデータ
 * @return	最後に使った道具のポケット
 */
//------------------------------------------------------------------
u16 MYITEM_BattleBagLastPageGet( BAG_CURSOR * wk )
{
	return wk->btl.page;
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグのポケットページのカーソル位置取得
 * @param	wk		カーソルデータ
 * @return	最後に使った道具
 */
//------------------------------------------------------------------
/*
u16 MYITEM_BattleBagPocketPagePosGet( BAG_CURSOR * wk )
{
	return wk->btl.pocket;
}
*/

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグのカーソル位置セット
 * @param	wk		カーソルデータ
 * @param	pos		カーソル位置
 * @param	scr		スクロールカウンタ
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_BattleBagCursorSet( BAG_CURSOR * wk, s16 * pos, s16 * scr )
{
	u32	i;

	for( i=0; i<BATTLE_BAG_POKE_MAX; i++ ){
		wk->btl.pos[i] = pos[i];
		wk->btl.scr[i] = scr[i];
	}
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグのカーソル位置初期化
 * @param	wk		カーソルデータ
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_BattleBagCursorPosInit( BAG_CURSOR * wk )
{
	u32	i;
	s16 p[BATTLE_BAG_POKE_MAX];


	for( i=0; i<BATTLE_BAG_POKE_MAX; i++ ){
		p[i] = 0;
	}
	MYITEM_BattleBagCursorSet( wk, p, p );
//	MYITEM_BattleBagPocketPagePosSet( wk, 0 );
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグの最後に使った道具をセット
 * @param	wk		カーソルデータ
 * @param	item	最後に使ったアイテム
 * @param	page	最後に使ったアイテムのページ
 * @return	none
 */
//------------------------------------------------------------------
void MYITEM_BattleBagLastItemSet( BAG_CURSOR * wk, u16 item, u16 page )
{
	wk->btl.item = item;
	wk->btl.page = page;
}

//------------------------------------------------------------------
/**
 * @brief	戦闘バッグのポケットページのカーソル位置セット
 * @param	wk		カーソルデータ
 * @return	最後に使った道具
 */
//------------------------------------------------------------------
/*
void MYITEM_BattleBagPocketPagePosSet( BAG_CURSOR * wk, u16 pocket )
{
	wk->btl.pocket = pocket;
}
*/

// 外部参照インデックスを作る時のみ有効(ゲーム中は無効)
#ifdef CREATE_INDEX
void *Index_Get_Myitem_Offset(MYITEM_PTR item, int type)
{
  switch(type){
  case BAG_POKE_NORMAL: return item->MyNormalItem; break;
  case BAG_POKE_EVENT: return item->MyEventItem; break;
  case BAG_POKE_WAZA: return item->MySkillItem; break;
  case BAG_POKE_DRUG: return item->MyDrugItem; break;
  case BAG_POKE_NUTS: return item->MyNutsItem; break;
  }
}
#endif


#if PM_DEBUG
//この関数はPDWでセーブのアドレスを導きだすため用です
//かならずgamedataの方を使ってください
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
extern MYITEM_PTR* SaveData_GetMyItem(SAVE_CONTROL_WORK * sv);
//----------------------------------------------------------
/**
 * @brief	   アイテムデータ構造体の取得
 * @param	   sv		セーブデータへのポインタ
 * @return	 WIFI_HISTORYデータへのポインタ
 */
//----------------------------------------------------------
MYITEM_PTR* SaveData_GetMyItem(SAVE_CONTROL_WORK * sv)
{
  return (MYITEM_PTR*)SaveControl_DataPtrGet(sv, GMDATA_ID_MYITEM);
}

#endif

