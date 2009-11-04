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

#include "test/ariizumi/ari_debug.h"
#include "musical/musical_local.h"
#include "musical/mus_item_draw.h"
#include "musical/mus_item_data.h"
#include "musical/musical_camera_def.h"


//======================================================================
//	define
//======================================================================
//影用のVRAM転送Task
#define VRAM_TRANS_TASK_NUM (6)

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
	BOOL	isUpdate;
	BOOL	useOffset;	//アイテムのオフセットを利用するか？
	BOOL	isShadow;
	int		resIdx;
	int		bbdIdx;
	u32		arcIdx;
	VecFx32 pos;
	fx16	sizeX;
	fx16	sizeY;
	u16		rotZ;
	MUS_ITEM_DATA_WORK *itemData;
};

//描画システム
struct _MUS_ITEM_DRAW_SYSTEM
{
	HEAPID heapId;
	GFL_BBD_SYS			*bbdSys;
	MUS_ITEM_DRAW_WORK	*musItem;
	MUS_ITEM_DATA_SYS	*itemDataSys;

	u16	itemMax;
	//影用パレット
	u16 shadowPallet[16];
  //machineuse で初期化してた
//	NNSGfdVramTransferTask vramTransTask[VRAM_TRANS_TASK_NUM];
};

//======================================================================
//	proto
//======================================================================
static u16 MUS_ITEM_DRAW_SearchEmptyWork( MUS_ITEM_DRAW_SYSTEM* work );
static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos , fx16 sizeX , fx16 sizeY);

//--------------------------------------------------------------
//システムの初期化と開放
//--------------------------------------------------------------
MUS_ITEM_DRAW_SYSTEM*	MUS_ITEM_DRAW_InitSystem( GFL_BBD_SYS *bbdSys , u16 itemMax , HEAPID heapId )
{
	int i;
	MUS_ITEM_DRAW_SYSTEM *work = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DRAW_SYSTEM) );
	
	work->musItem = GFL_HEAP_AllocMemory( heapId , sizeof(MUS_ITEM_DRAW_WORK)*itemMax );
	work->heapId = heapId;
	work->itemMax = itemMax;
	work->bbdSys = bbdSys;
	for( i=0;i<work->itemMax;i++ )
	{
		work->musItem[i].enable = FALSE;
		work->musItem[i].isUpdate = FALSE;
	}
	work->itemDataSys = MUS_ITEM_DATA_InitSystem( heapId );
	
	//影用VRAM転送Mng
  //machineuse で初期化してた
//	NNS_GfdInitVramTransferManager( work->vramTransTask , VRAM_TRANS_TASK_NUM );
	for( i=0;i<16;i++ )
	{
		work->shadowPallet[i] = 0x5a56;
	}
	
	return work;
}

void MUS_ITEM_DRAW_TermSystem( MUS_ITEM_DRAW_SYSTEM* work )
{
	int i;
	
	NNS_GfdClearVramTransferManagerTask();

	MUS_ITEM_DATA_ExitSystem( work->itemDataSys );
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
	int i;
	for( i=0;i<work->itemMax;i++ )
	{
		if( work->musItem[i].enable == TRUE &&
			work->musItem[i].isUpdate == TRUE )
		{
			MUS_ITEM_DRAW_WORK *itemWork = &work->musItem[i];
			GFL_BBD_TEXSIZ texSize;
			fx16 workX,workY;
			u8	rateX,rateY;
			GFL_POINT ofsPos;
			VecFx32 dispPos;
			fx32 rotOfsX,rotOfsY;
			
 			if( itemWork->useOffset == TRUE )
			{
	 			MUS_ITEM_DATA_GetDispOffset( itemWork->itemData , &ofsPos );
				if( itemWork->sizeX < 0 )
				{
					ofsPos.x *= -1;
				}
				rotOfsX = FX_CosIdx( itemWork->rotZ ) * ofsPos.x - FX_SinIdx( itemWork->rotZ ) * ofsPos.y;
				rotOfsY = FX_SinIdx( itemWork->rotZ ) * ofsPos.x + FX_CosIdx( itemWork->rotZ ) * ofsPos.y;
			}
			else
			{
				rotOfsX = 0;
				rotOfsY = 0;
			}
			
			dispPos.x = MUSICAL_POS_X_FX( itemWork->pos.x - rotOfsX);
			dispPos.y = MUSICAL_POS_Y_FX( itemWork->pos.y - rotOfsY);
			dispPos.z = itemWork->pos.z;
			
			GFL_BBD_SetObjectTrans( work->bbdSys , itemWork->bbdIdx , &dispPos );

			MUS_ITEM_DRAW_GetPicSize( itemWork , &rateX,&rateY );
			workX = itemWork->sizeX*rateX;
			workY = itemWork->sizeY*rateY;
			GFL_BBD_SetObjectSiz( work->bbdSys , itemWork->bbdIdx , &workX , &workY );

			GFL_BBD_SetObjectRotate( work->bbdSys , itemWork->bbdIdx , &itemWork->rotZ );
			
			work->musItem[i].isUpdate = FALSE; 
		}
	}
}

void MUS_ITEM_DRAW_UpdateSystem_VBlank( MUS_ITEM_DRAW_SYSTEM* work )
{
  //machine_useでやってた
//	NNS_GfdDoVramTransfer();
}

//アイテム番号からARCの番号を調べる
u16 MUS_ITEM_DRAW_GetArcIdx( const u16 itemIdx )
{
  //仮データ対応
  const u16 fileNum = GFL_ARC_GetDataFileCnt( ARCID_MUSICAL_ITEM , 0 );
  if( itemIdx >= fileNum - 2 )
  {
  	return (NARC_musical_item_mus_item999_nsbtx);
  }
  
	return (NARC_musical_item_mus_item000_nsbtx + itemIdx);
}
//ファイルIdxからサイズを調べる
void MUS_ITEM_DRAW_GetPicSize( MUS_ITEM_DRAW_WORK *itemWork , u8 *sizeXRate , u8 *sizeYRate )
{
	GFL_BBD_TEXSIZ texSize = MUS_ITEM_DATA_GetTexType( itemWork->itemData );
	u16 sizeX,sizeY;
	
	GFL_BBD_GetTexSize( texSize , &sizeX , &sizeY );
	
	*sizeXRate = sizeX/32;
	*sizeYRate = sizeY/32;
}
//指定箇所に装備できるか?
const BOOL MUS_ITEM_DRAW_CanEquipPos( MUS_ITEM_DRAW_WORK *itemWork , const MUS_POKE_EQUIP_POS pos )
{
	return MUS_ITEM_DATA_CanEquipPos( itemWork->itemData , pos );
}
const BOOL	MUS_ITEM_DRAW_CanEquipPosItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_POS pos )
{
	MUS_ITEM_DATA_WORK *itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , itemNo );
	return MUS_ITEM_DATA_CanEquipPos( itemData , pos );
}

const BOOL	MUS_ITEM_DRAW_CanEquipPosUserDataItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_USER pos )
{
	MUS_ITEM_DATA_WORK *itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , itemNo );
	return MUS_ITEM_DATA_CanEquipPosUserData( itemData , pos );
}

const BOOL MUS_ITEM_DRAW_CheckMainPosUserDataItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_USER pos )
{
	MUS_ITEM_DATA_WORK *itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , itemNo );
	return MUS_ITEM_DATA_CheckMainPosUserData( itemData , pos );
}

const BOOL MUS_ITEM_DRAW_IsBackItem( MUS_ITEM_DRAW_WORK *itemWork )
{
	return MUS_ITEM_DATA_IsBackItem( itemWork->itemData );
}

const BOOL MUS_ITEM_DRAW_IsFrontItem( MUS_ITEM_DRAW_WORK *itemWork )
{
	return MUS_ITEM_DATA_IsFrontItem( itemWork->itemData );
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
	work->musItem[i].itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , work->musItem[i].arcIdx );

	texSize = MUS_ITEM_DATA_GetTexType( work->musItem[i].itemData );
	MUS_ITEM_DRAW_GetPicSize( &work->musItem[i] , &sizeX , &sizeY );

	work->musItem[i].resIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_MUSICAL_ITEM , work->musItem[i].arcIdx,
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos , sizeX , sizeY );
	
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
	work->musItem[i].itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , work->musItem[i].arcIdx );

	texSize = MUS_ITEM_DATA_GetTexType( work->musItem[i].itemData );
	MUS_ITEM_DRAW_GetPicSize( &work->musItem[i] , &sizeX , &sizeY );

	
	work->musItem[i].resIdx = GFL_BBD_AddResource( work->bbdSys , g3DresTex , 
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
	MUS_ITEM_DRAW_AddFunc( work , i , pos , sizeX , sizeY );
	
	return &work->musItem[i];
}

static MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddFunc( MUS_ITEM_DRAW_SYSTEM* work , u16 idx , VecFx32 *pos , fx16 sizeX , fx16 sizeY)
{
	const BOOL flg = TRUE;
	work->musItem[idx].bbdIdx = GFL_BBD_AddObject( work->bbdSys , work->musItem[idx].resIdx ,
											FX16_ONE,FX16_ONE , pos , 31 ,GFL_BBD_LIGHT_NONE);
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , work->musItem[idx].bbdIdx , &flg );
	work->musItem[idx].enable = TRUE;
	work->musItem[idx].isUpdate = TRUE;
	work->musItem[idx].useOffset = TRUE;
	work->musItem[idx].isShadow = FALSE;

	work->musItem[idx].pos.x = pos->x;
	work->musItem[idx].pos.y = pos->y;
	work->musItem[idx].pos.z = pos->z;
	work->musItem[idx].sizeX = sizeX;
	work->musItem[idx].sizeY = sizeY;
	work->musItem[idx].rotZ = 0;
	
	return &work->musItem[idx];
}

void MUS_ITEM_DRAW_DelItem( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork )
{
	GFL_BBD_RemoveObject( work->bbdSys , itemWork->bbdIdx );
	GFL_BBD_RemoveResourceVram( work->bbdSys , itemWork->resIdx );
	itemWork->enable = FALSE;
	itemWork->isUpdate = FALSE;
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
	u16 newResId;
	
	itemWork->arcIdx = MUS_ITEM_DRAW_GetArcIdx( newId );
	itemWork->itemData = MUS_ITEM_DATA_GetMusItemData( work->itemDataSys , itemWork->arcIdx );
	itemWork->isShadow = FALSE;

	texSize = MUS_ITEM_DATA_GetTexType( itemWork->itemData );
	MUS_ITEM_DRAW_GetPicSize( itemWork , &sizeX , &sizeY );
//	GFL_BBD_GetObjectTrans( work->bbdSys , itemWork->bbdIdx , &pos );
//	GFL_BBD_GetObjectSiz( work->bbdSys , itemWork->bbdIdx , &scaleX , &scaleY );
//	MUS_ITEM_DRAW_DelItem( work , itemWork );
	newResId = GFL_BBD_AddResource( work->bbdSys , newRes , 
							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	GFL_BBD_SetObjectResIdx( work->bbdSys , itemWork->bbdIdx , &newResId );
	GFL_BBD_RemoveResourceVram( work->bbdSys , itemWork->resIdx );
	itemWork->resIdx = newResId;
//	itemWork->resIdx = GFL_BBD_AddResource( work->bbdSys , newRes , 
//							GFL_BBD_TEXFMT_PAL16 , texSize , sizeX*32 , sizeY*32 );
	
//	itemWork->bbdIdx = GFL_BBD_AddObject( work->bbdSys , itemWork->resIdx ,
//											FX16_ONE,FX16_ONE , &pos , 31 ,GFL_BBD_LIGHT_NONE);
//	GFL_BBD_SetObjectDrawEnable( work->bbdSys , itemWork->bbdIdx , &flg );
//	GFL_BBD_SetObjectSiz( work->bbdSys , itemWork->bbdIdx , &scaleX , &scaleY );
	itemWork->enable = TRUE;
	itemWork->isUpdate = TRUE; 

}

//影用灰色パレット設定
void MUS_ITEM_DRAW_SetShadowPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes )
{
	if( itemWork->isShadow == FALSE )
	{
		u32 pltAdr;
		NNSG3dPlttKey pltKey;
		u32 pltSize;
		
		//同じ16色パレットでも使用色数で配置サイズが変わるのでPltKeyからサイズを取得する
		GFL_BBD_GetResourceTexPlttAdrs( work->bbdSys , itemWork->resIdx , &pltAdr );
		pltKey = GFL_G3D_GetTexturePlttVramKey( shadowRes );
		pltSize = NNS_GfdGetPlttKeySize( pltKey );
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT , pltAdr , (void*)work->shadowPallet , pltSize );
		itemWork->isShadow = TRUE;
	}
}

//暗用灰色パレット設定
//パレット用のワークはそこからポインタをもらう。NULLなら生成。あるならそれをそのまま使う
void MUS_ITEM_DRAW_SetDarkPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes , void **pltWork )
{
	if( itemWork->isShadow == FALSE )
	{
		u32 pltAdr;
		NNSG3dPlttKey pltKey;
		u32 pltSize;
		
		//同じ16色パレットでも使用色数で配置サイズが変わるのでPltKeyからサイズを取得する
		GFL_BBD_GetResourceTexPlttAdrs( work->bbdSys , itemWork->resIdx , &pltAdr );
		pltKey = GFL_G3D_GetTexturePlttVramKey( shadowRes );
		pltSize = NNS_GfdGetPlttKeySize( pltKey );
		
		if( *pltWork == NULL )
		{
			GXRgb *pltData = (void*)GFL_G3D_GetAdrsTexturePltt( shadowRes );
			GXRgb *transData;
			u8 i;
			*pltWork = GFL_HEAP_AllocClearMemory( work->heapId , pltSize );

			transData = *pltWork;
			for( i=0;i<pltSize/2;i++ )
			{
				const u8 r = (pltData[i] & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT;
				const u8 g = (pltData[i] & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT;
				const u8 b = (pltData[i] & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT;
				//ARI_TPrintf("[%04x:%d:%d:%d]\n",pltData[i],r,g,b);
				transData[i] = GX_RGB( r/2 , g/2 , b/2 );
				
			}
		}
		
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT , pltAdr , *pltWork , pltSize );
		itemWork->isShadow = TRUE;
	}
}

//パレットをデフォルトに戻す
void MUS_ITEM_DRAW_ResetShadowPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes )
{
	if( itemWork->isShadow == TRUE )
	{
		u32 pltAdr;
		NNSG3dPlttKey pltKey;
		u32 pltSize;
		void* dataAdr;
		
		//同じ16色パレットでも使用色数で配置サイズが変わるのでPltKeyからサイズを取得する
		GFL_BBD_GetResourceTexPlttAdrs( work->bbdSys , itemWork->resIdx , &pltAdr );
		pltKey = GFL_G3D_GetTexturePlttVramKey( shadowRes );
		pltSize = NNS_GfdGetPlttKeySize( pltKey );
		
		dataAdr = (void*)GFL_G3D_GetAdrsTexturePltt( shadowRes );
		NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT , pltAdr , dataAdr , pltSize );
		itemWork->isShadow = FALSE;
	}
}

//影用に各数値をコピーする
void MUS_ITEM_DRAW_CopyItemDataToShadow( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *baseItem , MUS_ITEM_DRAW_WORK *shadowItem)
{
	shadowItem->pos.x = baseItem->pos.x+FX32_CONST(2.0f);
	shadowItem->pos.y = baseItem->pos.y+FX32_CONST(2.0f);
	shadowItem->pos.z = baseItem->pos.z-FX32_CONST(0.5f);
	shadowItem->sizeX = baseItem->sizeX;
	shadowItem->sizeY = baseItem->sizeY;
	shadowItem->rotZ  = baseItem->rotZ;
	shadowItem->useOffset = baseItem->useOffset;
	//フリップ未対応
#if 0	//点滅影(べっこ転送命令が必要
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
	{
		u8 i;
		static u8 rgb = 0;
		for( i=0;i<16;i++ )
		{
			work->shadowPallet[i] = GX_RGB(rgb,rgb,rgb);
		}
		rgb++;
		if( rgb >= 32 )
		{
			rgb = 0;
		}
	}
#endif
}


void MUS_ITEM_DRAW_SetDrawEnable( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , BOOL flg )
{
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , itemWork->bbdIdx , &flg );
	itemWork->isUpdate = TRUE;
}
void MUS_ITEM_DRAW_SetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	itemWork->pos.x = pos->x;
	itemWork->pos.y = pos->y;
	itemWork->pos.z = pos->z;
	itemWork->isUpdate = TRUE;
}

void MUS_ITEM_DRAW_GetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos )
{
	pos->x = itemWork->pos.x;
	pos->y = itemWork->pos.y;
	pos->z = itemWork->pos.z;
}

void MUS_ITEM_DRAW_SetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 sizeX , fx16 sizeY )
{
	itemWork->sizeX = sizeX;
	itemWork->sizeY = sizeY;
	itemWork->isUpdate = TRUE;
}

void MUS_ITEM_DRAW_GetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 *sizeX , fx16 *sizeY )
{
	*sizeX = itemWork->sizeX;
	*sizeY = itemWork->sizeY;
}

void MUS_ITEM_DRAW_SetRotation( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 rotZ )
{
	itemWork->rotZ = rotZ;
	itemWork->isUpdate = TRUE;
}

void MUS_ITEM_DRAW_GetRotation( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 *rotZ )
{
	*rotZ = itemWork->rotZ;
}

void MUS_ITEM_DRAW_SetAlpha( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u8 alpha )
{
	GFL_BBD_SetObjectAlpha( work->bbdSys , itemWork->bbdIdx , &alpha );
	
}

void MUS_ITEM_DRAW_SetFlipS( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , const BOOL flipS )
{
	GFL_BBD_SetObjectFlipS( work->bbdSys , itemWork->bbdIdx , &flipS );
	itemWork->isUpdate = TRUE;
}

void MUS_ITEM_DRAW_SetUseOffset( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , const BOOL flg )
{
	itemWork->useOffset = flg;
}

const BOOL MUS_ITEM_DRAW_GetUseOffset( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork )
{
	return itemWork->useOffset;
}

void MUS_ITEM_DRAW_GetOffsetPos( MUS_ITEM_DRAW_WORK *itemWork , GFL_POINT *ofsPos )
{
	MUS_ITEM_DATA_GetDispOffset( itemWork->itemData , ofsPos );
}

//MUS_ITEM_DATA_SYSの取得
MUS_ITEM_DATA_SYS* MUS_ITEM_DRAW_GetItemDataSys( MUS_ITEM_DRAW_SYSTEM* work )
{
  return work->itemDataSys;
}

#if DEB_ARI
void MUS_ITEM_DRAW_Debug_DumpResData( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork )
{
	u32 pltAdr;
	GFL_BBD_GetResourceTexPlttAdrs( work->bbdSys , itemWork->resIdx , &pltAdr );
	ARI_TPrintf("[%2d][%8x]\n",itemWork->resIdx,pltAdr);
}
#endif

#if USE_MUSICAL_EDIT
void MUS_ITEM_DRAW_GetDispOffset( MUS_ITEM_DRAW_WORK* itemWork , GFL_POINT* ofs )
{
	MUS_ITEM_DATA_GetDispOffset( itemWork->itemData , ofs );
}

#endif	//USE_MUSICAL_EDIT
