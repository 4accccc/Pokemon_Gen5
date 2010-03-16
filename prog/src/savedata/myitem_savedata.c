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
	s16	scr[BAG_POKE_MAX];  //10
	s16	pos[BAG_POKE_MAX];  //20
	u16	pocket;        //22
	u16	dummy;         //24byte
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

#define	ITEM_MAX_NORMAL			( 999 )		// 通常のアイテム所持数最大
#define	ITEM_MAX_WAZAMACHINE	( 99 )		// 技マシンの所持数最大

static u32 MyItemDataGet( MYITEM_PTR myitem, u16 id, ITEM_ST ** item, u32 * max, HEAPID heap );
static u32 MyPocketDataGet( MYITEM_PTR myitem, s32 pocket, ITEM_ST ** item, u32 * max);
static void bottomInsert( ITEM_ST * item, const u32 max );



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
 * @brief	MYITEMのITEM_STのコピー
 * @param	myitem	  MYITEMのポインタ
 * @param	itemst		アイテムリストの先頭ポインタ
 * @param	pocket		ポケット番号
 * @param	bMyGet		myitemからデータがほしい場合TRUE  myitemに格納する場合FALSE
 */
//------------------------------------------------------------------
void MYITEM_ITEM_STCopy(MYITEM_PTR myitem, ITEM_ST* itemst, int pocket, int bMyGet)
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


//------------------------------------------------------------------
/**
 * @brief	便利ボタンに割り当てられているアイテム取得
 * @param	myitem	MYITEMへのポインタ
 * @return	アイテム番号
 */
//------------------------------------------------------------------
u32 MYITEM_CnvButtonItemGet( const MYITEM_PTR myitem, int index )
{
	return myitem->cnv_button[index];
}

//------------------------------------------------------------------
/**
 * @brief	便利ボタンにアイテムセット
 * @param	myitem	MYITEMへのポインタ
 * @param	setitem	セットするアイテム
 * @return	アイテム番号
 */
//------------------------------------------------------------------
void MYITEM_CnvButtonItemSet( MYITEM_PTR myitem, int index , u32 setitem )
{
  GF_ASSERT(index < DUMMY_SHORTCUT_MAX);
	myitem->cnv_button[index] = setitem;
}

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
  int before = 0;
	ITEM_ST * add = AddItemPosGet( myitem, item_no, num, heap );

	if( add == NULL ){ return FALSE; }
  before = add->no;
	add->id = item_no;
	add->no += num;

	{
		u32	pocket;
		u32	max;
		pocket = MyItemDataGet( myitem, item_no, &add, &max, heap );
		if( pocket == BAG_POKE_NUTS || pocket == BAG_POKE_WAZA ){
			MYITEM_SortNumber( add, max );
		}
    else if(before == 0){  //加えたアイテムが一個目だった場合、リストの最初にもっていく
      ITEM_ST * item;
      u32	num;
      MyPocketDataGet(myitem, pocket, &item, &num);
      num = MYITEM_GetItemThisPocketNumber(item, num);
      bottomInsert( item,  num );
    }
	}
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
	u32	i;

	MyPocketDataGet(myitem, pocket, &item, &max);	
	return MYITEM_GetItemThisPocketNumber(item, max);
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
 * @brief	バッグのデータ作成
 * @param	myitem		手持ちアイテム構造体へのポインタ
 * @param	list		ポケットリスト
 * @param	heap		ヒープID
 */
//------------------------------------------------------------------
#if 0	//まだバッグ画面はないのでとりあえず消す 2008.12.05(金) matsuda
void * MYITEM_MakeBagData( MYITEM_PTR myitem, const u8 * list, HEAPID heap )
{
	BAG_DATA * bag;
	int i;

	bag = BagSystemDataAlloc( heap );
	for( i=0; list[i] != 0xff; i++ ){
		switch( list[i] ){
		case BAG_POKE_EVENT:
			BagPocketDataMake( bag, myitem->MyEventItem, BAG_POKE_EVENT, i );
			break;
		case BAG_POKE_NORMAL:
			BagPocketDataMake( bag, myitem->MyNormalItem, BAG_POKE_NORMAL, i );
			break;
		case BAG_POKE_NUTS:
			BagPocketDataMake( bag, myitem->MyNutsItem, BAG_POKE_NUTS, i );
			break;
		case BAG_POKE_DRUG:
			BagPocketDataMake( bag, myitem->MyDrugItem, BAG_POKE_DRUG, i );
			break;
		case BAG_POKE_WAZA:
			BagPocketDataMake( bag, myitem->MySkillItem, BAG_POKE_WAZA, i );
			break;
		}
	}
	return bag;
}
#endif


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
	}

	if( pos >= max ){ return NULL; }

	return &item[pos];
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

