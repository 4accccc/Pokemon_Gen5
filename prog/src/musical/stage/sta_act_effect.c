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
	u8				effWork[ PARTICLE_LIB_HEAP_SIZE ];
//	void			*effWork;
	GFL_PTC_PTR		ptcWork;
	GFL_EMIT_PTR	emitter;
};

struct _STA_EFF_SYS
{
	HEAPID heapId;
	
	STA_EFF_WORK *effWork;
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
	STA_EFF_SYS *work;
	
	work = GFL_HEAP_AllocMemory( heapId , sizeof( STA_EFF_SYS ));
	work->heapId = heapId;
	GFL_PTC_Init( heapId );
	
	return work;
}

void	STA_EFF_ExitSystem( STA_EFF_SYS *work )
{
	GFL_PTC_Exit();
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
	STA_EFF_WORK *effWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(STA_EFF_WORK) );
	void *effRes;

	//まずマネージャの作成
//	effWork->effWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(PARTICLE_LIB_HEAP_SIZE) );
	effWork->ptcWork = GFL_PTC_Create( effWork->effWork,PARTICLE_LIB_HEAP_SIZE,FALSE,work->heapId);
	//リソース読み込み
	effRes = GFL_PTC_LoadArcResource( ARCID_STAGE_GRA , fileIdx , work->heapId );
	//リソースとマネージャの関連付け
	GFL_PTC_SetResource( effWork->ptcWork , effRes , FALSE , GFUser_VIntr_GetTCBSYS() );
	
	return effWork;
}

void	STA_EFF_CreateEmmitter( STA_EFF_WORK *effWork , u16 emmitNo , VecFx32 *pos )
{
	GFL_PTC_CreateEmitter( effWork->ptcWork , emmitNo , pos );
	
}
