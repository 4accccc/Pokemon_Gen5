//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ドレスアップ 試着メイン
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef DUP_FITTING_ITEM_H__
#define DUP_FITTING_ITEM_H__

#include "musical/musical_system.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct
{
  u16   itemId;
  int   itemResIdx;
  int   itemShadowResIdx;
  BOOL  isOutList;  //リスト外にいる
  BOOL  isNew;
}ITEM_STATE;

typedef struct _FIT_ITEM_WORK FIT_ITEM_WORK;
typedef struct _FIT_ITEM_GROUP FIT_ITEM_GROUP;

//======================================================================
//	proto
//======================================================================

//グループの作成と削除
FIT_ITEM_GROUP* DUP_FIT_ITEMGROUP_CreateGroup( HEAPID heapId , const u16 maxNum );
void DUP_FIT_ITEMGROUP_DeleteGroup( FIT_ITEM_GROUP* group );

//アイテムの追加・削除(リンクにつなぐだけなのでメモリの取得開放はしない
void DUP_FIT_ITEMGROUP_AddItem( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item );
void DUP_FIT_ITEMGROUP_AddItemTop( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item );
void DUP_FIT_ITEMGROUP_RemoveItem( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item );

//リンクのスタートを取得
FIT_ITEM_WORK* DUP_FIT_ITEMGROUP_GetStartItem( FIT_ITEM_GROUP *group );

//各数値の取得
const u16 DUP_FIT_ITEMGROUP_GetItemNum( FIT_ITEM_GROUP *group );
const BOOL DUP_FIT_ITEMGROUP_IsItemMax( FIT_ITEM_GROUP *group );


//--------------------------------------------------------------

//アイテムの作成・削除
FIT_ITEM_WORK* DUP_FIT_ITEM_CreateItem( HEAPID heapId , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , ITEM_STATE *itemState, VecFx32 *pos );
void DUP_FIT_ITEM_DeleteItem( FIT_ITEM_WORK *item , MUS_ITEM_DRAW_SYSTEM *itemDrawSys );

//絵の変更
void DUP_FIT_ITEM_ChengeGraphic( FIT_ITEM_WORK *item , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , ITEM_STATE *itemState );

//次のアイテムを取得
FIT_ITEM_WORK* DUP_FIT_ITEM_GetNextItem( FIT_ITEM_WORK *item );

//各数値の取得
void DUP_FIT_ITEM_SetItemState( FIT_ITEM_WORK *item , ITEM_STATE *itemState );
ITEM_STATE* DUP_FIT_ITEM_GetItemState( FIT_ITEM_WORK *item );
void DUP_FIT_ITEM_SetNewItemState( FIT_ITEM_WORK *item , ITEM_STATE *itemState);
ITEM_STATE* DUP_FIT_ITEM_GetNewItemState( FIT_ITEM_WORK *item );
MUS_ITEM_DRAW_WORK* DUP_FIT_ITEM_GetItemDrawWork( FIT_ITEM_WORK *item );

void DUP_FIT_ITEM_SetPosition( FIT_ITEM_WORK *item ,const GFL_POINT *pos );
GFL_POINT* DUP_FIT_ITEM_GetPosition( FIT_ITEM_WORK *item );
void DUP_FIT_ITEM_SetBefPosition( FIT_ITEM_WORK *item ,const GFL_POINT *pos );
GFL_POINT* DUP_FIT_ITEM_GetBefPosition( FIT_ITEM_WORK *item );
void DUP_FIT_ITEM_SetBefScale( FIT_ITEM_WORK *item , const fx32 befScale );
fx32 DUP_FIT_ITEM_GetBefScale( FIT_ITEM_WORK *item );
void DUP_FIT_ITEM_SetScale( FIT_ITEM_WORK *item , const fx32 scale );
fx32 DUP_FIT_ITEM_GetScale( FIT_ITEM_WORK *item );
void DUP_FIT_ITEM_SetCount( FIT_ITEM_WORK *item , const u16 count );
u16	 DUP_FIT_ITEM_GetCount( FIT_ITEM_WORK *item );

void	DUP_FIT_ITEM_SetEquipPos( FIT_ITEM_WORK *item , const MUS_POKE_EQUIP_POS ePos );
const MUS_POKE_EQUIP_POS	DUP_FIT_ITEM_GetEquipPos( FIT_ITEM_WORK *item );


//座標とのチェック
const BOOL DUP_FIT_ITEM_CheckHit( FIT_ITEM_WORK *item , u32 posX , u32 posY );
const u32 DUP_FIT_ITEM_CheckLength( FIT_ITEM_WORK *item , u32 posX , u32 posY );
const u16 DUP_FIT_ITEM_CheckLengthSqrt( FIT_ITEM_WORK *item , u32 posX , u32 posY );

#if DEB_ARI
void	DUP_FIT_ITEM_DumpList( FIT_ITEM_GROUP *group , u8 dispIdx );
#endif

#endif DUP_FITTING_ITEM_H__