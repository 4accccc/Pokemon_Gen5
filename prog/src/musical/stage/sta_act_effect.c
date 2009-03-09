//======================================================================
/**
 * @file	sta_act_effect.h
 * @brief	ステージ用　エフェクト
 * @author	ariizumi
 * @data	09/03/03
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "stage_gra.naix"

#include "test/ariizumi/ari_debug.h"

#include "sta_act_effect.h"
#include "sta_local_def.h"
#include "eff_def/mus_eff.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define STA_EFF_TCB_TASK (4)
#define STA_EFF_WORK_SIZE (0x2000)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _STA_EFF_WORK
{
	BOOL			isEnable;
	u8				effWork[ PARTICLE_LIB_HEAP_SIZE ];
//	void			*effWork;
	GFL_PTC_PTR		ptcWork;
	GFL_EMIT_PTR	emitter;
};

struct _STA_EFF_SYS
{
	HEAPID heapId;
	
	STA_EFF_WORK effWork[ACT_EFFECT_MAX];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STA_EFF_SYS* STA_EFF_InitSystem( HEAPID heapId )
{
	u8 i;
	STA_EFF_SYS *work;
	
	work = GFL_HEAP_AllocMemory( heapId , sizeof( STA_EFF_SYS ));
	work->heapId = heapId;
	GFL_PTC_Init( heapId );
	
	for( i=0;i<ACT_EFFECT_MAX;i++ )
	{
		work->effWork[i].isEnable = FALSE;
	}
	
	return work;
}

void	STA_EFF_ExitSystem( STA_EFF_SYS *work )
{
	u8 i;
	for( i=0;i<ACT_EFFECT_MAX;i++ )
	{
		if( work->effWork[i].isEnable == TRUE )
		{
			STA_EFF_DeleteEffect( work , &work->effWork[i] );
		}
	}

	GFL_PTC_Exit();
	GFL_HEAP_FreeMemory( work );
}

void	STA_EFF_UpdateSystem( STA_EFF_SYS *work )
{
	GFL_PTC_CalcAll();
}

void	STA_EFF_DrawSystem( STA_EFF_SYS *work )
{
	GFL_PTC_DrawAll();
}

STA_EFF_WORK*	STA_EFF_CreateEffect( STA_EFF_SYS *work , int fileIdx )
{
	u8 i;
	STA_EFF_WORK *effWork = NULL;
	void *effRes;

	for( i=0;i<ACT_EFFECT_MAX;i++ )
	{
		if( work->effWork[i].isEnable == FALSE )
		{
			effWork = &work->effWork[i];
		}
	}
	
	GF_ASSERT_MSG( effWork != NULL ,"Stage acting effect work is full!!\n" );

	//まずマネージャの作成
	effWork->ptcWork = GFL_PTC_Create( effWork->effWork,PARTICLE_LIB_HEAP_SIZE,FALSE,work->heapId);
	//リソース読み込み
	effRes = GFL_PTC_LoadArcResource( ARCID_STAGE_GRA , fileIdx , work->heapId );
	//リソースとマネージャの関連付け
	GFL_PTC_SetResource( effWork->ptcWork , effRes , FALSE , GFUser_VIntr_GetTCBSYS() );
	
	effWork->isEnable = TRUE;
	
	return effWork;
}

void	STA_EFF_DeleteEffect( STA_EFF_SYS *work , STA_EFF_WORK *effWork )
{
	effWork->isEnable = FALSE;
	GFL_PTC_DeleteEmitterAll( effWork->ptcWork );
	GFL_PTC_Delete( effWork->ptcWork );
}

void	STA_EFF_CreateEmmitter( STA_EFF_WORK *effWork , u16 emmitNo , VecFx32 *pos )
{
	GFL_PTC_CreateEmitter( effWork->ptcWork , emmitNo , pos );
	
}
