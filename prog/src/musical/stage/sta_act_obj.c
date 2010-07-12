//======================================================================
/**
 * @file	sta_act_obj.h
 * @brief	ステージ用　オブジェクト
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
#include "musical/musical_local.h"

#include "sta_local_def.h"
#include "sta_act_obj.h"

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
struct _STA_OBJ_WORK
{
	BOOL		isEnable;
	BOOL		isUpdate;	//座標計算が発生したか？
	
	VecFx32		pos;

	int			resIdx;
	int			bbdIdx;
#if USE_MUSICAL_EDIT
	u16			objId;
#endif USE_MUSICAL_EDIT
};

struct _STA_OBJ_SYS
{
	HEAPID heapId;
	u16		scrollOffset;

	GFL_BBD_SYS		*bbdSys;
	STA_OBJ_WORK	objWork[ACT_OBJECT_MAX];
	
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void STA_OBJ_UpdateObjFunc( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STA_OBJ_SYS* STA_OBJ_InitSystem( HEAPID heapId , GFL_BBD_SYS* bbdSys )
{
	u8 i;
	STA_OBJ_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_OBJ_SYS) );
	
	work->heapId = heapId;
	work->bbdSys = bbdSys;
	work->scrollOffset = 0;
	
	for( i=0 ; i<ACT_OBJECT_MAX ; i++ )
	{
		work->objWork[i].isEnable = FALSE;
		work->objWork[i].isUpdate = FALSE;
	}
	
	return work;
}

void	STA_OBJ_ExitSystem( STA_OBJ_SYS *work )
{
	u8 idx;
	
	for( idx=0 ; idx<ACT_OBJECT_MAX ; idx++ )
	{
		if( work->objWork[idx].isEnable == TRUE )
		{
			STA_OBJ_DeleteObject( work , &work->objWork[idx] );
		}
	}

	GFL_HEAP_FreeMemory( work );
}

void	STA_OBJ_UpdateSystem( STA_OBJ_SYS *work )
{
	int idx;
	for( idx=0 ; idx<ACT_OBJECT_MAX ; idx++ )
	{
		if( work->objWork[idx].isEnable == TRUE )
		{
			STA_OBJ_UpdateObjFunc( work,&work->objWork[idx] );
		}
	}
}

void	STA_OBJ_DrawSystem( STA_OBJ_SYS *work )
{
}

static void STA_OBJ_UpdateObjFunc( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork )
{
	if( objWork->isUpdate == TRUE )
	{
		VecFx32 musPos;
	
		musPos.x = ACT_POS_X_FX(objWork->pos.x - FX32_CONST(work->scrollOffset));
		musPos.y = ACT_POS_Y_FX(objWork->pos.y);
		musPos.z = objWork->pos.z;
		
		GFL_BBD_SetObjectTrans( work->bbdSys , objWork->bbdIdx , &musPos );

		objWork->isUpdate = FALSE;
	}
	
}

void	STA_OBJ_System_SetScrollOffset( STA_OBJ_SYS *work , const u16 scrOfs )
{
	u8 idx;
	work->scrollOffset = scrOfs;

	for( idx=0 ; idx<ACT_OBJECT_MAX ; idx++ )
	{
		if( work->objWork[idx].isEnable == TRUE )
		{
			work->objWork[idx].isUpdate = TRUE;
		}
	}
}


STA_OBJ_WORK* STA_OBJ_CreateObject( STA_OBJ_SYS *work , const u16 objId )
{
	u8 idx;
	STA_OBJ_WORK *objWork;
	const BOOL flg = TRUE;
	
	for( idx=0 ; idx<ACT_OBJECT_MAX ; idx++ )
	{
		if( work->objWork[idx].isEnable == FALSE )
		{
			break;
		}
	}
	GF_ASSERT_MSG( idx < ACT_OBJECT_MAX , "Stage acting obj work is full!\n" );
	
	objWork = &work->objWork[idx];

	objWork->isEnable = TRUE;
	objWork->isUpdate = TRUE;
	objWork->pos.x= 0;
	objWork->pos.y= 0;
	objWork->pos.z= 0;
#if USE_MUSICAL_EDIT
	objWork->objId = objId;
#endif //USE_MUSICAL_EDIT

//仮素材削除でmakeを通すための対応。
#ifndef BUGFIX_GFBTS1965_20100712
	objWork->resIdx = GFL_BBD_AddResourceArc( work->bbdSys , ARCID_STAGE_GRA , 
											NARC_stage_gra_stage_ojb01_nsbtx + objId , GFL_BBD_TEXFMT_PAL256 ,
											GFL_BBD_TEXSIZ_128x128 , 128 , 128 );
#endif BUGFIX_GFBTS_1964_20100712
	objWork->bbdIdx = GFL_BBD_AddObject( work->bbdSys , objWork->resIdx , ACT_BBD_SIZE(128.0f) , ACT_BBD_SIZE(128.0f) , 
										&objWork->pos , 31 , GFL_BBD_LIGHT_NONE );
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , objWork->bbdIdx , &flg );

	return objWork;
}
void STA_OBJ_DeleteObject( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork )
{
	GFL_BBD_RemoveObject( work->bbdSys , objWork->bbdIdx );
	GFL_BBD_RemoveResource( work->bbdSys , objWork->resIdx );
	objWork->isEnable = FALSE;
}

void STA_OBJ_SetPosition( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork , const VecFx32 *pos )
{
	objWork->pos.x = pos->x;
	objWork->pos.y = pos->y;
	objWork->pos.z = pos->z;
	
	objWork->isUpdate = TRUE;
}

void STA_OBJ_GetPosition( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork , VecFx32 *pos )
{
	pos->x = objWork->pos.x;
	pos->y = objWork->pos.y;
	pos->z = objWork->pos.z;
}

void STA_OBJ_SetShowFlg( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork , const BOOL flg )
{
	GFL_BBD_SetObjectDrawEnable( work->bbdSys , objWork->bbdIdx , &flg );
}

BOOL STA_OBJ_GetShowFlg( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork )
{
	return GFL_BBD_GetObjectDrawEnable( work->bbdSys , objWork->bbdIdx );
}

#if USE_MUSICAL_EDIT
//デバッグ用
BOOL STA_OBJ_IsEnable( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork )
{
	return objWork->isEnable;
}
BOOL STA_OBJ_GetObjNo( STA_OBJ_SYS *work , STA_OBJ_WORK *objWork )
{
	return objWork->objId;
}
#endif USE_MUSICAL_EDIT

