//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカル用 ポケモン描画
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "arc_def.h"
#include "musical_item.naix"

#include "musical/mus_item_draw.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//描画１個分
struct _MUS_ITEM_DRAW_WORK
{
	BOOL	enable;
	int		resIdx;
	int		bbdIdx;
};

//描画システム
struct _MUS_ITEM_DRAW_SYSTEM
{
	HEAPID heapId;
	GFL_BBD_SYS			*bbdSys;
	MUS_ITEM_DRAW_WORK	*musItem;

	u16	itemMax;
};

//======================================================================
//	proto
//======================================================================
static u16 MUS_ITEM_DRAW_GetArcIdx( const u16 itemIdx );
static u16 MUS_ITEM_DRAW_SearchEmptyWork( MUS_ITEM_DRAW_SYSTEM* work );
static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos );


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
//システムの初期化と開放
MUS_ITEM_DRAW_SYSTEM*	MUS_ITEM_DRAW_InitSystem( GFL_BBD_SYS *bbdSys , u16 itemMax , HEAPID heapId )
{
	int i;
	MUS_ITEM_DRAW_SYSTEM *work = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DRAW_SYSTEM) );
	
	work->musItem = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DRAW_WORK)*itemMax );
	work->itemMax = itemMax;
	work->bbdSys = bbdSys;
	for( i=0;i<work->itemMax;i++ )
	{
		work->musItem[i].enable = FALSE;
	}
	
	return work;
}

void MUS_ITEM_DRAW_TermSystem( MUS_ITEM_DRAW_SYSTEM* work )
{
	int i;
	for( i=0;i<work->itemMax;i++ )
	{
		if( work->musItem[i].enable == TRUE )
		{
			MUS_ITEM_DRAW_Del( work , &work->musItem[i] );
		}
	}
	GFL_HEAP_FreeMemory( work->musItem );
	GFL_HEAP_FreeMemory( work );
}

//更新
void MUS_ITEM_DRAW_UpdateSystem( MUS_ITEM_DRAW_SYSTEM* work )
{
}

//アイテム番号からARCの番号を調べる
static u16 MUS_ITEM_DRAW_GetArcIdx( const u16 itemIdx )
{
	//FIXME 今は４パターンだからループ
	return (NARC_musical_item_item01_nsbtx + itemIdx)%4;
}

//アイテム番号からリソースの読み込み
GFL_G3D_RES* MUS_ITEM_DRAW_LoadResource( u16 itemIdx )
{
	u16	arcIdx = MUS_ITEM_DRAW_GetArcIdx( itemIdx );
	return GFL_G3D_CreateResourceArc( ARCID_MUSICAL_ITEM , arcIdx );
}
void MUS_ITEM_DRAW_DeleteResource( GFL_G3D_RES *res )
{
	GFL_G3D_DeleteResource( res );
}

//アイテムワークの空きを調べる
static u16 MUS_ITEM_DRAW_SearchEmptyWork( MUS_ITEM_DRAW_SYSTEM* work )
{
	int i;
	//空きスペースの検索
	for( i=0;i<work->itemMax;i++ )
	{
		if( work->musItem[i].enable == FALSE )
		{
			break;
		}
	}
	if( i == work->itemMax )
	{
		GF_ASSERT_MSG( FALSE , "MusicalItem draw max over!![max = %d]\n",i);
		return 0;
	}
	return i;
}

MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddItemId( MUS_ITEM_DRAW_SYSTEM* work , u16 itemIdx , VecFx32 *pos )
{
	VecFx32 scale;
	MCSS_ADD_WORK maw;
	int i = MUS_ITEM_DRAW_SearchEmptyWork(work);
	u16	arcIdx = MUS_ITEM_DRAW_GetArcIdx( itemIdx );
	

	work->musItem[i].resIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_MUSICAL_ITEM , arcIdx,
							GFL_BBD_TEXFMT_PAL16 , GFL_BBD_TEXSIZ_32x32 , 32 , 32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos );
	
	return &work->musItem[i];
}

MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddResource( MUS_ITEM_DRAW_SYSTEM* work , GFL_G3D_RES* g3DresTex, VecFx32 *pos )
{
	VecFx32 scale;
	MCSS_ADD_WORK maw;
	int i = MUS_ITEM_DRAW_SearchEmptyWork(work);
	
	work->musItem[i].resIdx = GFL_BBD_AddResource( work->bbdSys , g3DresTex , 
							GFL_BBD_TEXFMT_PAL16 , GFL_BBD_TEXSIZ_32x32 ,
							32 , 32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos );
	
	return &work->musItem[i];
}

static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos )
{
	const BOOL flg = TRUE;
	work->musItem[idx].bbdIdx = GFL_BBD_AddObject( work->bbdSys , work->musItem[idx].resIdx ,
											FX32_ONE,FX32_ONE , pos , 31 ,GFL_BBD_LIGHT_NONE);
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->musItem[idx].bbdIdx , &flg );
	
	return &work->musItem[idx];
}

void MUS_ITEM_DRAW_Del( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork )
{
	GFL_BBD_RemoveObject( work->bbdSys , itemWork->bbdIdx );
	GFL_BBD_RemoveResource( work->bbdSys , itemWork->resIdx );
	itemWork->enable = FALSE;
}

void MUS_ITEM_DRAW_SetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	GFL_BBD_SetObjectTrans( work->bbdSys , itemWork->bbdIdx , pos );
}

void MUS_ITEM_DRAW_GetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	GFL_BBD_GetObjectTrans( work->bbdSys , itemWork->bbdIdx , pos );
}

void MUS_ITEM_DRAW_SetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 *sizeX , fx16 *sizeY )
{
	GFL_BBD_SetObjectSiz( work->bbdSys , itemWork->bbdIdx , sizeX , sizeY );
}

void MUS_ITEM_DRAW_GetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 *sizeX , fx16 *sizeY )
{
	GFL_BBD_GetObjectSiz( work->bbdSys , itemWork->bbdIdx , sizeX , sizeY );
}

