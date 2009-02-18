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
	u32		arcIdx;
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
static u16 MUS_ITEM_DRAW_SearchEmptyWork( MUS_ITEM_DRAW_SYSTEM* work );
static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos );

//--------------------------------------------------------------
//システムの初期化と開放
//--------------------------------------------------------------
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
			MUS_ITEM_DRAW_DelItem( work , &work->musItem[i] );
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
u16 MUS_ITEM_DRAW_GetArcIdx( const u16 itemIdx )
{
	//FIXME 今は32パターンだからループ
	return (NARC_musical_item_item01_nsbtx + itemIdx)%32;
}
//ファイルIdxからサイズを調べる
void MUS_ITEM_DRAW_GetPicSize( const u16 fileId , GFL_BBD_TEXSIZ *texSize , u8 *sizeXRate , u8 *sizeYRate )
{
	if( fileId >= NARC_musical_item_item29_nsbtx )
	{
		*texSize = GFL_BBD_TEXSIZ_32x64;
		*sizeXRate = 1;
		*sizeYRate = 2;
	}
	else
	if( fileId >= NARC_musical_item_item25_nsbtx )
	{
		*texSize = GFL_BBD_TEXSIZ_64x32;
		*sizeXRate = 2;
		*sizeYRate = 1;
	}
	else
	{
		*texSize = GFL_BBD_TEXSIZ_32x32;
		*sizeXRate = 1;
		*sizeYRate = 1;
	}
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
	GFL_BBD_TEXSIZ texSize;
	u8 sizeX,sizeY;
	
	work->musItem[i].arcIdx = MUS_ITEM_DRAW_GetArcIdx( itemIdx );
	MUS_ITEM_DRAW_GetPicSize( work->musItem[i].arcIdx , &texSize , &sizeX , &sizeY );

	work->musItem[i].resIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_MUSICAL_ITEM , work->musItem[i].arcIdx,
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos );
	
	return &work->musItem[i];
}

MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddResource( MUS_ITEM_DRAW_SYSTEM* work , u16 itemIdx , GFL_G3D_RES* g3DresTex, VecFx32 *pos )
{
	VecFx32 scale;
	MCSS_ADD_WORK maw;
	int i = MUS_ITEM_DRAW_SearchEmptyWork(work);
	GFL_BBD_TEXSIZ texSize;
	u8 sizeX,sizeY;
	
	work->musItem[i].arcIdx = MUS_ITEM_DRAW_GetArcIdx( itemIdx );
	MUS_ITEM_DRAW_GetPicSize( work->musItem[i].arcIdx , &texSize , &sizeX , &sizeY );
	
	work->musItem[i].resIdx = GFL_BBD_AddResource( work->bbdSys , g3DresTex , 
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos );
	
	return &work->musItem[i];
}

static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos )
{
	const BOOL flg = TRUE;
	work->musItem[idx].bbdIdx = GFL_BBD_AddObject( work->bbdSys , work->musItem[idx].resIdx ,
											FX16_ONE,FX16_ONE , pos , 31 ,GFL_BBD_LIGHT_NONE);
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->musItem[idx].bbdIdx , &flg );
	work->musItem[idx].enable = TRUE;
	
	return &work->musItem[idx];
}

void MUS_ITEM_DRAW_DelItem( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork )
{
	GFL_BBD_RemoveObject( work->bbdSys , itemWork->bbdIdx );
	GFL_BBD_RemoveResourceVram( work->bbdSys , itemWork->resIdx );
	itemWork->enable = FALSE;
}

//--------------------------------------------------------------
//絵の変更
//--------------------------------------------------------------
void MUS_ITEM_DRAW_ChengeGraphic( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 newId , GFL_G3D_RES *newRes)
{
	VecFx32 pos;
	fx16 scaleX,scaleY;
	const BOOL flg = TRUE;
	GFL_BBD_TEXSIZ texSize;
	u8 sizeX,sizeY;
	
	itemWork->arcIdx = MUS_ITEM_DRAW_GetArcIdx( newId );
	MUS_ITEM_DRAW_GetPicSize( itemWork->arcIdx , &texSize , &sizeX , &sizeY );
	GFL_BBD_GetObjectTrans( work->bbdSys , itemWork->bbdIdx , &pos );
	GFL_BBD_GetObjectSiz( work->bbdSys , itemWork->bbdIdx , &scaleX , &scaleY );
	MUS_ITEM_DRAW_DelItem( work , itemWork );
	itemWork->resIdx = GFL_BBD_AddResource( work->bbdSys , newRes , 
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
	itemWork->bbdIdx = GFL_BBD_AddObject( work->bbdSys , itemWork->resIdx ,
											FX16_ONE,FX16_ONE , &pos , 31 ,GFL_BBD_LIGHT_NONE);
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , itemWork->bbdIdx , &flg );
	GFL_BBD_SetObjectSiz( work->bbdSys , itemWork->bbdIdx , &scaleX , &scaleY );
	itemWork->enable = TRUE;
}


void MUS_ITEM_DRAW_SetDrawEnable( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , BOOL flg )
{
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , itemWork->bbdIdx , &flg );
}
void MUS_ITEM_DRAW_SetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	GFL_BBD_SetObjectTrans( work->bbdSys , itemWork->bbdIdx , pos );
}

void MUS_ITEM_DRAW_GetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	GFL_BBD_GetObjectTrans( work->bbdSys , itemWork->bbdIdx , pos );
}

void MUS_ITEM_DRAW_SetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 sizeX , fx16 sizeY )
{
	GFL_BBD_TEXSIZ texSize;
	fx16 workX,workY;
	u8	rateX,rateY;
	MUS_ITEM_DRAW_GetPicSize( itemWork->arcIdx , &texSize , &rateX,&rateY );
	workX = sizeX*rateX;
	workY = sizeY*rateY;
	GFL_BBD_SetObjectSiz( work->bbdSys , itemWork->bbdIdx , &workX , &workY );
}

void MUS_ITEM_DRAW_GetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 *sizeX , fx16 *sizeY )
{
	GFL_BBD_TEXSIZ texSize;
	u8	rateX,rateY;
	MUS_ITEM_DRAW_GetPicSize( itemWork->arcIdx , &texSize , &rateX,&rateY );
	GFL_BBD_GetObjectSiz( work->bbdSys , itemWork->bbdIdx , sizeX , sizeY );
	*sizeX /= rateX;
	*sizeY /= rateY;
}

