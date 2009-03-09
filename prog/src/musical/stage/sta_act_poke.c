//======================================================================
/**
 * @file	sta_act_poke.h
 * @brief	ステージ用　ポケモン
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_gra.naix"

#include "musical/mus_poke_draw.h"
#include "musical/mus_item_draw.h"

#include "sta_local_def.h"
#include "sta_act_poke.h"

#include "test/ariizumi/ari_debug.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_POKE_WORK
{
	BOOL		isEnable;
	BOOL		isUpdate;	//座標計算が発生したか？
	
	VecFx32					pokePos;
	MUS_POKE_DRAW_WORK		*drawWork;
	MUS_POKE_DATA_WORK		*pokeData;

	GFL_G3D_RES			*itemRes[MUS_POKE_EQUIP_MAX];
	MUS_ITEM_DRAW_WORK	*itemWork[MUS_POKE_EQUIP_MAX];
};

struct _STA_POKE_SYS
{
	HEAPID heapId;

	MUS_POKE_DRAW_SYSTEM	*drawSys;
	MUS_ITEM_DRAW_SYSTEM	*itemDrawSys;
	STA_POKE_WORK			pokeWork[MUSICAL_POKE_MAX];
	
	u16		scrollOffset;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void STA_POKE_UpdatePokeFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
static void STA_POKE_UpdateItemFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STA_POKE_SYS* STA_POKE_InitSystem( HEAPID heapId , MUS_POKE_DRAW_SYSTEM* drawSys , MUS_ITEM_DRAW_SYSTEM *itemDrawSys )
{
	u8 i;
	STA_POKE_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_POKE_SYS) );
	
	work->heapId = heapId;
	work->drawSys = drawSys;
	work->itemDrawSys = itemDrawSys;
	work->scrollOffset = 0;
	
	for( i=0 ; i<MUSICAL_POKE_MAX ; i++ )
	{
		work->pokeWork[i].isEnable = FALSE;
		work->pokeWork[i].isUpdate = FALSE;
	}
	
	return work;
}

void	STA_POKE_ExitSystem( STA_POKE_SYS *work )
{
	int idx;
	for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
	{
		if( work->pokeWork[idx].isEnable == TRUE )
		{
			STA_POKE_DeletePoke( work , &work->pokeWork[idx] );
		}
	}
	GFL_HEAP_FreeMemory( work );
}

void	STA_POKE_UpdateSystem( STA_POKE_SYS *work )
{
	int idx;
	for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
	{
		if( work->pokeWork[idx].isEnable == TRUE )
		{
			STA_POKE_UpdatePokeFunc( work , &work->pokeWork[idx] );
		}
	}
}

void	STA_POKE_UpdateSystem_Item( STA_POKE_SYS *work )
{
	int idx;
	for( idx=0 ; idx<MUSICAL_POKE_MAX ; idx++ )
	{
		if( work->pokeWork[idx].isEnable == TRUE )
		{
			STA_POKE_UpdateItemFunc( work , &work->pokeWork[idx] );
		}
	}
}

static void STA_POKE_UpdatePokeFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
	if( pokeWork->isUpdate == TRUE )
	{
		VecFx32 musPos;
		musPos.x = ACT_POS_X_FX(pokeWork->pokePos.x - FX32_CONST(work->scrollOffset));
		musPos.y = ACT_POS_Y_FX(pokeWork->pokePos.y);
		musPos.z = pokeWork->pokePos.z;
		
		MUS_POKE_DRAW_SetPosition( pokeWork->drawWork , &musPos);
		
		//アイテムの更新要る？
		pokeWork->isUpdate = FALSE;
	}
}

static void STA_POKE_UpdateItemFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
	int ePos;
	VecFx32 pos;
	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		if( pokeWork->itemWork[ePos] != NULL )
		{
			MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( pokeWork->drawWork , ePos );
			if( equipData->isEnable == TRUE )
			{
				pos.x = ACT_POS_X_FX(equipData->pos.x+FX32_CONST(128.0f));
				pos.y = ACT_POS_Y_FX(equipData->pos.y+FX32_CONST(96.0f));
				pos.z = pokeWork->pokePos.z+FX32_HALF;	//とりあえずポケの前に出す
				//OS_Printf("[%.2f][%.2f]\n",F32_CONST(equipData->pos.z),F32_CONST(pokePos.z));
				MUS_ITEM_DRAW_SetPosition(	work->itemDrawSys , 
											pokeWork->itemWork[ePos] ,
											&pos );
				MUS_ITEM_DRAW_SetRotation(	work->itemDrawSys , 
											pokeWork->itemWork[ePos] ,
											equipData->rot );
				
			}
		}
	}
}


void	STA_POKE_DrawSystem( STA_POKE_SYS *work )
{
}

void	STA_POKE_System_SetScrollOffset( STA_POKE_SYS *work , const u16 scrOfs )
{
	u8 idx;
	work->scrollOffset = scrOfs;

	for( idx=0 ; idx<MUS_POKE_EQUIP_MAX ; idx++ )
	{
		if( work->pokeWork[idx].isEnable == TRUE )
		{
			work->pokeWork[idx].isUpdate = TRUE;
		}
	}
}


STA_POKE_WORK* STA_POKE_CreatePoke( STA_POKE_SYS *work , MUSICAL_POKE_PARAM *musPoke )
{
	u8 idx;
	u8 ePos;	//equipPos
	STA_POKE_WORK *pokeWork;
	
	for( idx=0 ; idx<MUS_POKE_EQUIP_MAX ; idx++ )
	{
		if( work->pokeWork[idx].isEnable == FALSE )
		{
			break;
		}
	}
	GF_ASSERT_MSG( idx < MUS_POKE_EQUIP_MAX , "Stage acting poke work is full!\n" );
	
	pokeWork = &work->pokeWork[idx];

	pokeWork->isEnable = TRUE;
	pokeWork->isUpdate = TRUE;
	pokeWork->drawWork = MUS_POKE_DRAW_Add( work->drawSys , musPoke );
	pokeWork->pokeData = MUS_POKE_DRAW_GetPokeData( pokeWork->drawWork );
	pokeWork->pokePos.x= 0;
	pokeWork->pokePos.y= 0;
	pokeWork->pokePos.z= 0;
	MUS_POKE_DRAW_StartAnime( pokeWork->drawWork );

	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		const u16 itemNo = musPoke->equip[ePos].type;
		if( itemNo != MUSICAL_ITEM_INVALID )
		{
			pokeWork->itemRes[ePos] = MUS_ITEM_DRAW_LoadResource( itemNo );
			pokeWork->itemWork[ePos] = MUS_ITEM_DRAW_AddResource( work->itemDrawSys , itemNo , pokeWork->itemRes[ePos] , &pokeWork->pokePos );
			MUS_ITEM_DRAW_SetSize( work->itemDrawSys , pokeWork->itemWork[ePos] , FX16_ONE , FX16_ONE );
		}
		else
		{
			pokeWork->itemRes[ePos] = NULL;
			pokeWork->itemWork[ePos] = NULL;
		}
	}
	
	return pokeWork;
}

void STA_POKE_DeletePoke( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork )
{
	u8 ePos;	//equipPos
	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		if( pokeWork->itemWork[ePos] != NULL )
		{
			MUS_ITEM_DRAW_DelItem( work->itemDrawSys , pokeWork->itemWork[ePos] );
			MUS_ITEM_DRAW_DeleteResource( pokeWork->itemRes[ePos] );

			pokeWork->itemRes[ePos] = NULL;
			pokeWork->itemWork[ePos] = NULL;
		}
	}
	MUS_POKE_DRAW_Del( work->drawSys , pokeWork->drawWork );
	pokeWork->isEnable = FALSE;
}

void STA_POKE_SetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos )
{
	pokeWork->pokePos.x = pos->x;
	pokeWork->pokePos.y = pos->y;
	pokeWork->pokePos.z = pos->z;
	
	pokeWork->isUpdate = TRUE;
}


