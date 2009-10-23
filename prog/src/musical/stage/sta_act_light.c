//======================================================================
/**
 * @file	sta_act_light.c
 * @brief	ステージ用　スポットライト
 * @author	ariizumi
 * @data	09/03/11
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_gra.naix"

#include "sta_acting.h"
#include "sta_local_def.h"
#include "sta_act_light.h"

#include "test/ariizumi/ari_debug.h"

//注目ポケ用
#include "sta_act_poke.h"
#include "script/sta_act_script_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//描画のときfx16しか使えないのでG3_Scale使って座標計算の倍率を変える
#define STA_LIGHT_POS_X(val)	((val)/16/4)
#define STA_LIGHT_POS_Y(val)	(FX32_CONST(192.0f)-(val))/16/4


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_LIGHT_WORK
{
	STA_LIGHT_TYPE	type;	//有効フラグ兼
	GXRgb	color;
	u8		alpha;
	VecFx32	pos;
	fx32	val1;	//半径 or 上の幅(半分
	fx32	val2;	//無効 or 下の幅(半分
	
	GFL_CLWK	*lightCell;

};

struct _STA_LIGHT_SYS
{
	HEAPID heapId;
	ACTING_WORK* actWork;

	STA_LIGHT_WORK	lightWork[ACT_LIGHT_MAX];
	STA_LIGHT_WORK	attentinLightWork;
	
	GFL_CLUNIT	*cellUnit;
	u32	pltIdx;
	u32	ncgIdx;
	u32	anmIdx;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void STA_LIGHT_UpdateObjFunc( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork );
static void STA_LIGHT_UpdateAttentionLight( STA_LIGHT_SYS *work );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STA_LIGHT_SYS* STA_LIGHT_InitSystem( HEAPID heapId , ACTING_WORK* actWork )
{
	u8 i;
	STA_LIGHT_SYS *work = GFL_HEAP_AllocMemory( heapId , sizeof(STA_LIGHT_SYS) );
	
	work->heapId = heapId;
	work->actWork = actWork;
	
	for( i=0 ; i<ACT_LIGHT_MAX ; i++ )
	{
		work->lightWork[i].type = ALT_NONE;
	}
	work->attentinLightWork.type = ALT_NONE;
	
	//OBJ用
	work->cellUnit  = GFL_CLACT_UNIT_Create( ACT_LIGHT_MAX+1 , 0, work->heapId );
	GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );

	//各種素材の読み込み
	{
		ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_STAGE_GRA , work->heapId );
	
		work->pltIdx = GFL_CLGRP_PLTT_Register( arcHandle , NARC_stage_gra_spotlight_NCLR , CLSYS_DRAW_MAIN , 0 , work->heapId  );
		work->ncgIdx = GFL_CLGRP_CGR_Register( arcHandle , NARC_stage_gra_spotlight_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
		work->anmIdx = GFL_CLGRP_CELLANIM_Register( arcHandle , NARC_stage_gra_spotlight_NCER , NARC_stage_gra_spotlight_NANR, work->heapId  );
	
		GFL_ARC_CloseDataHandle(arcHandle);
	}
	
	return work;
}

void	STA_LIGHT_ExitSystem( STA_LIGHT_SYS *work )
{
		
	GFL_CLGRP_PLTT_Release( work->pltIdx );
	GFL_CLGRP_CGR_Release( work->ncgIdx );
	GFL_CLGRP_CELLANIM_Release( work->anmIdx );
	
	GFL_CLACT_UNIT_Delete( work->cellUnit );
	GFL_HEAP_FreeMemory( work );
}

void	STA_LIGHT_UpdateSystem( STA_LIGHT_SYS *work )
{
  //work->actWork が NULL の時はミュージカルショット
	int idx;
	for( idx=0 ; idx<ACT_LIGHT_MAX ; idx++ )
	{
		if( work->lightWork[idx].type != ALT_NONE )
		{
			STA_LIGHT_UpdateObjFunc( work,&work->lightWork[idx] );
		}
	}
  if( work->actWork != NULL )
  {
  	STA_LIGHT_UpdateAttentionLight( work );
  }
}

static void STA_LIGHT_UpdateObjFunc( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork )
{
  //work->actWork が NULL の時はミュージカルショット
  const u16 scrOfs = ( work->actWork == NULL ? 128 : STA_ACT_GetStageScroll( work->actWork ));
  if( lightWork->type == ALT_CIRCLE )
  {
    GFL_CLACTPOS cellPos;
    cellPos.x = F32_CONST( lightWork->pos.x ) - scrOfs;
    cellPos.y = F32_CONST( lightWork->pos.y );
    GFL_CLACT_WK_SetPos( lightWork->lightCell , &cellPos , CLSYS_DEFREND_MAIN );
    if( work->actWork != NULL && STA_ACT_GetUseItemAttentionPoke( work->actWork ) != MUSICAL_POKE_MAX )
    {
    	GFL_CLACT_WK_SetDrawEnable( lightWork->lightCell, FALSE );
    }
    else
    {
    	GFL_CLACT_WK_SetDrawEnable( lightWork->lightCell, TRUE );
    }
  }
}

static void STA_LIGHT_UpdateAttentionLight( STA_LIGHT_SYS *work )
{
  if( work->attentinLightWork.type == ALT_NONE )
  {
    if( STA_ACT_GetUseItemAttentionPoke( work->actWork ) != MUSICAL_POKE_MAX )
    {
      //目立ってるやつがいる！
      GFL_CLWK_DATA	cellInitData;
    	work->attentinLightWork.type = ALT_CIRCLE;
    	cellInitData.pos_x = 128;
    	cellInitData.pos_y =  96;
    	cellInitData.anmseq = 0;
    	cellInitData.softpri = 0;
    	cellInitData.bgpri = 2;
    	work->attentinLightWork.lightCell = GFL_CLACT_WK_Create( work->cellUnit ,work->ncgIdx,work->pltIdx,work->anmIdx,
    						 		&cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
    	GFL_CLACT_WK_SetDrawEnable( work->attentinLightWork.lightCell, TRUE );
    }
  }
  //elseが無いのは生成後に即動かすため
  if( work->attentinLightWork.type == ALT_CIRCLE )
  {
    if( STA_ACT_GetUseItemAttentionPoke( work->actWork ) == MUSICAL_POKE_MAX )
    {
      //目立ちタイム終了
    	GFL_CLACT_WK_Remove( work->attentinLightWork.lightCell );
    	work->attentinLightWork.type = ALT_NONE;
    }
    else
    {
      VecFx32 pokePos;
      GFL_CLACTPOS cellPos;
      STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , STA_ACT_GetUseItemAttentionPoke( work->actWork ) );
      const u16 scrOfs = STA_ACT_GetStageScroll( work->actWork );
      
      STA_POKE_GetPosition( pokeSys , pokeWork , &pokePos );

      cellPos.x = F32_CONST( pokePos.x ) - scrOfs;
      cellPos.y = F32_CONST( pokePos.y ) - 32;
      
      GFL_CLACT_WK_SetPos( work->attentinLightWork.lightCell , &cellPos , CLSYS_DEFREND_MAIN );
      
    }
    
  }
}

void	STA_LIGHT_DrawSystem( STA_LIGHT_SYS *work )
{
}

STA_LIGHT_WORK* STA_LIGHT_CreateObject( STA_LIGHT_SYS *work , const STA_LIGHT_TYPE type )
{
	u8 idx;
	STA_LIGHT_WORK *lightWork;
	const BOOL flg = TRUE;
	GFL_CLWK_DATA	cellInitData;
	
	for( idx=0 ; idx<ACT_LIGHT_MAX ; idx++ )
	{
		if( work->lightWork[idx].type == ALT_NONE )
		{
			break;
		}
	}
	GF_ASSERT_MSG( idx < ACT_LIGHT_MAX , "Stage acting obj work is full!\n" );
	
	lightWork = &work->lightWork[idx];

	lightWork->type = type;

	//セルの生成

	cellInitData.pos_x = 128;
	cellInitData.pos_y =  96;
	cellInitData.anmseq = 0;
	cellInitData.softpri = 0;
	cellInitData.bgpri = 2;
	lightWork->lightCell = GFL_CLACT_WK_Create( work->cellUnit ,work->ncgIdx,work->pltIdx,work->anmIdx,
						 		&cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
	GFL_CLACT_WK_SetDrawEnable( lightWork->lightCell, TRUE );

	return lightWork;
}
void STA_LIGHT_DeleteObject( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork )
{
	GFL_CLACT_WK_Remove( lightWork->lightCell );
	lightWork->type = ALT_NONE;
}

void STA_LIGHT_SetPosition( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , const VecFx32 *pos )
{
	lightWork->pos.x = pos->x;
	lightWork->pos.y = pos->y;
	lightWork->pos.z = pos->z;
}

void STA_LIGHT_GetPosition( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , VecFx32 *pos )
{
	pos->x = lightWork->pos.x;
	pos->y = lightWork->pos.y;
	pos->z = lightWork->pos.z;
}
void STA_LIGHT_SetColor( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , const GXRgb col , const u8 alpha )
{
	lightWork->color = col;
	lightWork->alpha = alpha;
}

void STA_LIGHT_GetColor( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , GXRgb *col , u8 *alpha )
{
	*col = lightWork->color;
	*alpha = lightWork->alpha;
}

void STA_LIGHT_SetOptionValue( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , const fx32 val1 , const fx32 val2 )
{
	lightWork->val1 = val1;
	lightWork->val2 = val2;
}

void STA_LIGHT_GetOptionValue( STA_LIGHT_SYS *work , STA_LIGHT_WORK *lightWork , fx32 *val1 , fx32 *val2 )
{
	*val1 = lightWork->val1;
	*val2 = lightWork->val2;
}
