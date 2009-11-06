//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ドレスアップ 試着メイン
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "musical_item.naix"

#include "test/ariizumi/ari_debug.h"
#include "dup_local_def.h"
#include "musical/mus_item_draw.h"
#include "musical/dressup/dup_fitting_item.h"


//======================================================================
//	define
//======================================================================
static const u8 ITEM_HIT_SIZE = 16;

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

struct _FIT_ITEM_WORK
{
	u16	itemId;
	u16	newItemId;
	
	ITEM_STATE *itemState;
	ITEM_STATE *newItemState;
	
	MUS_ITEM_DRAW_WORK	*itemWork;
	GFL_POINT	pos;
	fx32	scale;
	u16		cnt;	//カウンタ
	MUS_POKE_EQUIP_POS ePos;
	
	GFL_POINT	befPos;
	fx32	    befScale;

	//単方向リスト構造
	FIT_ITEM_WORK* next;
};

struct _FIT_ITEM_GROUP
{
	HEAPID heapId;
	u16	workSize;
	u16	num;
	u16 max;
	FIT_ITEM_WORK *start;
	FIT_ITEM_WORK *end;
	
};
//======================================================================
//	proto
//======================================================================

//--------------------------------------------------------------
//グループの作成と削除
//--------------------------------------------------------------
FIT_ITEM_GROUP* DUP_FIT_ITEMGROUP_CreateGroup( HEAPID heapId , const u16 maxNum )
{
	FIT_ITEM_GROUP *group = GFL_HEAP_AllocMemory( heapId , sizeof(FIT_ITEM_GROUP) );
	
	group->heapId = heapId;
	group->num = 0;
	group->max = maxNum;
	group->start = NULL;
	group->end = NULL;
	
	return group;
}

void DUP_FIT_ITEMGROUP_DeleteGroup( FIT_ITEM_GROUP* group )
{
	if( group->num > 0 )
	{
		DUP_TPrintf("DressUp ItemGroup Delete: Item is not release yet[least %d]\n",group->num);
	}
	GFL_HEAP_FreeMemory( group );
}

//--------------------------------------------------------------
//アイテムの追加・削除(リンクにつなぐだけなのでメモリの取得開放はしない
//--------------------------------------------------------------
void DUP_FIT_ITEMGROUP_AddItem( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item )
{
	GF_ASSERT_MSG(item->next == NULL,"DressUp ItemGroup AddItem:Item link yet!!\n");
	if( group->num >= group->max )
	{
		GF_ASSERT_MSG(NULL,"DressUp ItemGroup AddItem:max over![max %d]\n",group->max);
		return;
	}
	if( group->start == NULL )
	{
		group->start = item;
		group->end	 = item;
	}
	else
	{
		FIT_ITEM_WORK *endItem = group->end;
		endItem->next = item;
		group->end = item;
	}
	item->next = NULL;
	group->num++;
	DUP_TPrintf("group add[%d]\n",group->num);
}
void DUP_FIT_ITEMGROUP_AddItemTop( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item )
{
	GF_ASSERT_MSG(item->next == NULL,"DressUp ItemGroup AddItem:Item link yet!!\n");
	if( group->num >= group->max )
	{
		GF_ASSERT_MSG(NULL,"DressUp ItemGroup AddItem:max over![max %d]\n",group->max);
		return;
	}
	if( group->start == NULL )
	{
		group->start = item;
		group->end	 = item;
		item->next = NULL;
	}
	else
	{
		FIT_ITEM_WORK *topItem = group->start;
		item->next = topItem;
		group->start = item;
	}
	group->num++;
	DUP_TPrintf("group add[%d]\n",group->num);
}

void DUP_FIT_ITEMGROUP_RemoveItem( FIT_ITEM_GROUP *group , FIT_ITEM_WORK *item )
{
	FIT_ITEM_WORK *befItem = NULL;
	FIT_ITEM_WORK *listItem = group->start;
	while( listItem != item && listItem != NULL )
	{
		befItem = listItem;
		listItem = listItem->next;
	}
	if( listItem == NULL )
	{
		GF_ASSERT_MSG(NULL,"DressUp ItemGroup RemoveItem:item not found!!\n");
		return;
	}

	if( befItem != NULL )
	{
		befItem->next = listItem->next;
	}
	else
	{
		group->start = listItem->next;
	}
	
	if( group->end == item )
	{
		group->end = befItem;
	}
	listItem->next = NULL;
	group->num--;
	DUP_TPrintf("groupe remove[%d]\n",group->num);
}

//--------------------------------------------------------------
//リンクのスタートを取得
//--------------------------------------------------------------
FIT_ITEM_WORK* DUP_FIT_ITEMGROUP_GetStartItem( FIT_ITEM_GROUP *group )
{
	return group->start;
}

//--------------------------------------------------------------
//各数値の取得
//--------------------------------------------------------------
const u16 DUP_FIT_ITEMGROUP_GetItemNum( FIT_ITEM_GROUP *group )
{
	return group->num;
}
const BOOL DUP_FIT_ITEMGROUP_IsItemMax( FIT_ITEM_GROUP *group )
{
	return ( group->num == group->max ? TRUE : FALSE );
}

//--------------------------------------------------------------
//アイテムの作成・削除
//--------------------------------------------------------------
FIT_ITEM_WORK* DUP_FIT_ITEM_CreateItem( HEAPID heapId , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , ITEM_STATE *itemState , VecFx32 *pos )
{
	FIT_ITEM_WORK *item = GFL_HEAP_AllocMemory( heapId , sizeof( FIT_ITEM_WORK ));
	
	item->next = NULL;
	item->itemState = itemState;
	//item->itemWork = MUS_ITEM_DRAW_AddResource( itemDrawSys , itemState->itemId , itemState->itemRes , pos );
	item->itemWork = MUS_ITEM_DRAW_AddResIdx( itemDrawSys , itemState->itemId , itemState->itemResIdx , pos );
	item->cnt = 0;
	item->ePos = MUS_POKE_EQU_INVALID;
	return item;
}
void DUP_FIT_ITEM_DeleteItem( FIT_ITEM_WORK *item , MUS_ITEM_DRAW_SYSTEM *itemDrawSys )
{
	MUS_ITEM_DRAW_DelItem( itemDrawSys , item->itemWork );
	GFL_HEAP_FreeMemory( item );
}

//--------------------------------------------------------------
//絵の読み替え
//--------------------------------------------------------------
void DUP_FIT_ITEM_ChengeGraphic( FIT_ITEM_WORK *item , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , ITEM_STATE *itemState )
{
	item->itemState = itemState;
	//MUS_ITEM_DRAW_ChengeGraphic( itemDrawSys , item->itemWork , itemState->itemId , itemState->itemRes );
	MUS_ITEM_DRAW_ChengeGraphicResIdx( itemDrawSys , item->itemWork , itemState->itemId , itemState->itemResIdx );
}

//--------------------------------------------------------------
//次のアイテムの取得
//--------------------------------------------------------------
FIT_ITEM_WORK* DUP_FIT_ITEM_GetNextItem( FIT_ITEM_WORK *item )
{
	return item->next;
}

//--------------------------------------------------------------
//各数値の取得
//--------------------------------------------------------------
void DUP_FIT_ITEM_SetItemState( FIT_ITEM_WORK *item , ITEM_STATE *itemState )
{
	item->itemState = itemState;
}
ITEM_STATE* DUP_FIT_ITEM_GetItemState( FIT_ITEM_WORK *item )
{
	return item->itemState;
}
void DUP_FIT_ITEM_SetNewItemState( FIT_ITEM_WORK *item , ITEM_STATE *itemState )
{
	item->newItemState = itemState;
}
ITEM_STATE* DUP_FIT_ITEM_GetNewItemState( FIT_ITEM_WORK *item )
{
	return item->newItemState;
}
MUS_ITEM_DRAW_WORK* DUP_FIT_ITEM_GetItemDrawWork( FIT_ITEM_WORK *item )
{
	return item->itemWork;
}

void DUP_FIT_ITEM_SetPosition( FIT_ITEM_WORK *item , const GFL_POINT *pos )
{
	item->pos = *pos;
}
GFL_POINT* DUP_FIT_ITEM_GetPosition( FIT_ITEM_WORK *item )
{
	return &item->pos;
}
void DUP_FIT_ITEM_SetBefPosition( FIT_ITEM_WORK *item , const GFL_POINT *pos )
{
	item->befPos = *pos;
}
GFL_POINT* DUP_FIT_ITEM_GetBefPosition( FIT_ITEM_WORK *item )
{
	return &item->befPos;
}
void DUP_FIT_ITEM_SetBefScale( FIT_ITEM_WORK *item , const fx32 befScale )
{
	item->befScale = befScale;
}
fx32 DUP_FIT_ITEM_GetBefScale( FIT_ITEM_WORK *item )
{
	return item->befScale;
}
void DUP_FIT_ITEM_SetScale( FIT_ITEM_WORK *item , const fx32 scale )
{
	item->scale = scale;
}
fx32 DUP_FIT_ITEM_GetScale( FIT_ITEM_WORK *item )
{
	return item->scale;
}
void DUP_FIT_ITEM_SetCount( FIT_ITEM_WORK *item , const u16 count )
{
	item->cnt = count;
}
u16	 DUP_FIT_ITEM_GetCount( FIT_ITEM_WORK *item )
{
	return item->cnt;
}

void	DUP_FIT_ITEM_SetEquipPos( FIT_ITEM_WORK *item , const MUS_POKE_EQUIP_POS ePos )
{
	item->ePos = ePos;
}

const MUS_POKE_EQUIP_POS	DUP_FIT_ITEM_GetEquipPos( FIT_ITEM_WORK *item )
{
	return item->ePos;
}


//--------------------------------------------------------------
//座標とのチェック
//--------------------------------------------------------------
//縦長・横長を考慮するか？
#define USE_LONG_SIZE (0)
const BOOL DUP_FIT_ITEM_CheckHit( FIT_ITEM_WORK *item , u32 posX , u32 posY )
{
	GFL_BBD_TEXSIZ texSize;
#if USE_LONG_SIZE
	u8 xRate,yRate;
#endif //USE_LONG_SIZE
	GFL_POINT ofsPos;
	s16 subX,subY;
	u16 arcIdx = MUS_ITEM_DRAW_GetArcIdx( item->itemState->itemId );
#if USE_LONG_SIZE
	MUS_ITEM_DRAW_GetPicSize( item->itemWork , &xRate,&yRate );
#endif //USE_LONG_SIZE
	MUS_ITEM_DRAW_GetOffsetPos( item->itemWork , &ofsPos );
	if( MUS_ITEM_DRAW_GetUseOffset( NULL , item->itemWork ) == TRUE )
	{
		subX = (item->pos.x-ofsPos.x) - posX;
		subY = (item->pos.y-ofsPos.y) - posY;
	}
	else
	{
		subX = item->pos.x - posX;
		subY = item->pos.y - posY;
	}

#if USE_LONG_SIZE
	if( subX < ITEM_HIT_SIZE*xRate &&
		subX >-ITEM_HIT_SIZE*xRate &&
		subY < ITEM_HIT_SIZE*yRate &&
		subY >-ITEM_HIT_SIZE*yRate )
#else
	if( subX < ITEM_HIT_SIZE &&
		subX >-ITEM_HIT_SIZE &&
		subY < ITEM_HIT_SIZE &&
		subY >-ITEM_HIT_SIZE )
#endif //USE_LONG_SIZE
	{
		return TRUE;
	}
	return FALSE;
}
const u32 DUP_FIT_ITEM_CheckLength( FIT_ITEM_WORK *item , u32 posX , u32 posY )
{
	const s16 subX = item->pos.x - posX;
	const s16 subY = item->pos.y - posY;
	return ( subX*subX + subY*subY );
}

const u16 DUP_FIT_ITEM_CheckLengthSqrt( FIT_ITEM_WORK *item , u32 posX , u32 posY )
{
	const u32 len = DUP_FIT_ITEM_CheckLength( item , posX , posY );
	CP_SetSqrt32( len );
	return CP_GetSqrtResult16();
}

#if DEB_ARI
void	DUP_FIT_ITEM_DumpList( FIT_ITEM_GROUP *group , u8 dispIdx )
{
	int	i = 0;
	FIT_ITEM_WORK *listItem = group->start;
	DUP_TPrintf("DUMP ITEM LIST[%d] [Num:%d]\n",dispIdx,group->num);
	while( listItem != NULL )
	{
		DUP_TPrintf("[id:%2d][itemId:%2d]\n",i,listItem->itemState->itemId);
		listItem = listItem->next;
		i++;
	}
	DUP_TPrintf("DUMP ITEM LIST[%d] FNISH\n");
	
}
#endif
