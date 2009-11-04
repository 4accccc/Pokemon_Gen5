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
#define STA_EFF_EMITTER_NUM (4)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct 
{
	GFL_EMIT_PTR	emitPtr;
	VecFx32			pos;
}STA_EMIT_WORK;

struct _STA_EFF_WORK
{
	BOOL			isEnable;
	u8				effWork[ PARTICLE_LIB_HEAP_SIZE ];
//	void			*effWork;
	GFL_PTC_PTR		ptcWork;
	STA_EMIT_WORK	emitWork[STA_EFF_EMITTER_NUM];
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
	
	for( i=0;i<STA_EFF_EMITTER_NUM;i++ )
	{
		effWork->emitWork[i].emitPtr = NULL;
	}
	effWork->isEnable = TRUE;
	
	return effWork;
}

void	STA_EFF_DeleteEffect( STA_EFF_SYS *work , STA_EFF_WORK *effWork )
{
	effWork->isEnable = FALSE;
	GFL_PTC_DeleteEmitterAll( effWork->ptcWork );
	GFL_PTC_Delete( effWork->ptcWork );
}

void	STA_EFF_CreateEmitter( STA_EFF_WORK *effWork , const u16 emitNo , VecFx32 *pos )
{
	const GFL_EMIT_PTR emitPtr = GFL_PTC_CreateEmitter( effWork->ptcWork , emitNo , pos );
	if( emitPtr != NULL && emitNo >= STA_EFF_EMITTER_NUM )
	{
		GF_ASSERT_MSG( NULL , "emmiter work num is over!!\n" );
		return;
	}
	effWork->emitWork[emitNo].emitPtr = emitPtr;
	VEC_Set( &effWork->emitWork[emitNo].pos , pos->x,pos->y,pos->z );
	
}

void	STA_EFF_DeleteEmitter( STA_EFF_WORK *effWork , const u16 emitNo )
{
	if( effWork->emitWork[emitNo].emitPtr != NULL )
	{
		GFL_PTC_DeleteEmitter( effWork->ptcWork , effWork->emitWork[emitNo].emitPtr );
	}
	else
	{
		ARI_TPrintf("Stage effect emitter[%d] is NULL!!\n",emitNo);
	}
}

void	STA_EFF_SetPosition( STA_EFF_WORK *effWork , const u16 emitNo , VecFx32 *pos )
{
	if( effWork->emitWork[emitNo].emitPtr != NULL )
	{
    GFL_PTC_SetEmitterPosition( effWork->emitWork[emitNo].emitPtr , pos );
	}
	else
	{
		ARI_TPrintf("Stage effect emitter[%d] is NULL!!\n",emitNo);
	}
}
