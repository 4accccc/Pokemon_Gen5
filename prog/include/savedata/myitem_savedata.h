//=============================================================================
/**
 * @file	myitem_savedata.h
 * @brief	手持ちアイテム操作用ヘッダ
 * @author	tamada
 * @author	hiroyuki nakamura
 * @date	2005.10.13
 */
//=============================================================================
#ifndef	__MYITEM_SAVEDATA_H__
#define	__MYITEM_SAVEDATA_H__

#include "myitem_savedata_def.h"
//============================================================================================
//============================================================================================
// バッグのカーソル位置
typedef struct _BAG_CURSOR	BAG_CURSOR;

//------------------------------------------------------------------------
//	アイテムデータ
//------------------------------------------------------------------------
typedef	struct {
	u16	id;		// アイテム番号
	u16	no;		// 個数
}ITEM_ST;

#define	BAG_EVENT_ITEM_MAX		( 50 )		// 大切な物ポケット最大数
#define	BAG_WAZA_ITEM_MAX		( 101 )		// 技マシンポケット最大数
#define	BAG_DRUG_ITEM_MAX		( 40 )		// 薬ポケット最大数
#define	BAG_NUTS_ITEM_MAX		( 64 )		// 木の実ポケット最大数
#define	BAG_NORMAL_ITEM_MAX		( 165+30+15+12 )		// 道具ポケット最大数

#define	BAG_MYITEM_MAX (BAG_NORMAL_ITEM_MAX) //一番要素が大きい物


#define DUMMY_SHORTCUT_MAX (10) //便利ボタンの最大数 @TODO 削除予定

//==============================================================================
//	型定義
//==============================================================================
///手持ちアイテムの不定形アクセス型
typedef struct _MYITEM MYITEM;
///手持ちアイテムの不定形ポインタ
typedef struct _MYITEM * MYITEM_PTR;


//============================================================================================
//============================================================================================
//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int MYITEM_GetWorkSize(void);
extern MYITEM_PTR MYITEM_AllocWork(HEAPID heapID);
extern void MYITEM_Copy(const MYITEM_PTR from, MYITEM_PTR to);
extern void MYITEM_ITEM_STCopy(MYITEM_PTR myitem, ITEM_ST* itemst, int pocket, int bMyGet);

//----------------------------------------------------------
//	MYITEM操作のための関数
//----------------------------------------------------------
extern void MYITEM_Init(MYITEM_PTR item);
extern BOOL MYITEM_AddCheck( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap );
extern BOOL MYITEM_AddItem(MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap);
extern BOOL MYITEM_SubItem( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap );
extern BOOL MYITEM_SubItemDirect( ITEM_ST * myitem, u32 max, u16 item_no, u16 num, HEAPID heap );
extern BOOL MYITEM_CheckItem( MYITEM_PTR myitem, u16 item_no, u16 num, HEAPID heap );
extern u16 MYITEM_GetItemNum( MYITEM_PTR myitem, u16 item_no, HEAPID heap );
extern u16 MYITEM_GetItemNumDirect( ITEM_ST * myitem, u32 max, u16 item_no, HEAPID heap );
extern void MYITEM_SortSpace( ITEM_ST * item, const u32 max );
extern void MYITEM_SortNumber( ITEM_ST * item, const u32 max );
extern void MYITEM_CheckSafety( MYITEM_PTR myitem );
extern BOOL MYITEM_CheckItemPocket( MYITEM_PTR myitem, u32 pocket );
extern u32 MYITEM_GetItemPocketNumber( MYITEM_PTR myitem, u32 pocket );
extern ITEM_ST * MYITEM_PosItemGet( MYITEM_PTR myitem, u16 pocket, u16 pos );
extern void MYITEM_BattlePocketItemMake( MYITEM_PTR myitem, ITEM_ST * make[], HEAPID heap );
extern u32 MYITEM_CnvButtonItemGet( const MYITEM_PTR myitem, int index );
extern void MYITEM_CnvButtonItemSet( MYITEM_PTR myitem, int index, u32 item );

//----------------------------------------------------------
//	ITEMLIST操作のための関数
//----------------------------------------------------------
extern u32 MYITEM_GetItemThisPocketNumber( ITEM_ST * item,int max );


//----------------------------------------------------------
//	バッグ作成関数
//----------------------------------------------------------
#if 0
extern void * MYITEM_MakeBagData( MYITEM_PTR myitem, const u8 * list, HEAPID heap );
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
extern u32 MYITEM_GetItemMax( u16 item );

//------------------------------------------------------------------
//	バッグのカーソル位置データ
//------------------------------------------------------------------
extern BAG_CURSOR * MYITEM_BagCursorAlloc( HEAPID heapID );

extern void MYITEM_FieldBagCursorGet( BAG_CURSOR * wk, u16 pocket, s16 * pos, s16 * scr );
extern u16 MYITEM_FieldBagPocketGet( BAG_CURSOR * wk );
extern void MYITEM_FieldBagCursorSet( BAG_CURSOR * wk, u16 pocket, s16 pos, s16 scr );
extern void MYITEM_FieldBagPocketSet( BAG_CURSOR * wk, u16 pocket );

extern void MYITEM_BattleBagCursorGet( BAG_CURSOR * wk, u16 pocket, s16 * pos, s16 * scr );
extern u16 MYITEM_BattleBagLastItemGet( BAG_CURSOR * wk );
extern u16 MYITEM_BattleBagLastPageGet( BAG_CURSOR * wk );
//extern u16 MYITEM_BattleBagPocketPagePosGet( BAG_CURSOR * wk );
extern void MYITEM_BattleBagCursorSet( BAG_CURSOR * wk, s16 * pos, s16 * scr );
extern void MYITEM_BattleBagLastItemSet( BAG_CURSOR * wk, u16 item, u16 page );
//extern void MYITEM_BattleBagPocketPagePosSet( BAG_CURSOR * wk, u16 pocket );
extern void MYITEM_BattleBagCursorPosInit( BAG_CURSOR * wk );


#ifdef CREATE_INDEX
extern void *Index_Get_Myitem_Offset(MYITEM_PTRitem, int type);
#endif
#endif	/* __MYITEM_SAVEDATA_H__ */
