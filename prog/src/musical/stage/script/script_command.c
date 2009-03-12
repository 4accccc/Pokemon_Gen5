//======================================================================
/**
 * @file	sta_act_script.h
 * @brief	ステージ スクリプト処理 コマンド群
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "script_table.h"
#include "../sta_local_def.h"
#include "test/ariizumi/ari_debug.h"


//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define SCRIPT_PRINT_LABEL(str) ARI_TPrintf("SCRIPT Frame[%4d] No[%d]%s\n",scriptWork->frame, SCRIPT_ENUM_ ## str ,#str);

#define ScriptFunc_GetValueS32(ptr) (*((s32*)ptr)++)
#define ScriptFunc_GetValuefx32(ptr) (*((fx32*)ptr)++)

//通常のタスクプライオリティ
//追従系などは低く設定することで、本体の移動を優先させる
#define SCRIPT_TCB_PRI_NORMAL (10)

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
	//TODO 精度確保のためにfx32とかでやったほうが良いかも
	ACTING_WORK *actWork;
	s32 start;
	s32 end;
	s32 stepVal;	//毎フレームの増加量
	u16 frame;
	u16 step;
}MOVE_WORK_S32;

typedef struct
{
	ACTING_WORK *actWork;
	VecFx32 start;
	VecFx32 end;
	VecFx32 stepVal;	//毎フレームの増加量
	u16 frame;
	u16 step;
}MOVE_WORK_VEC;

typedef struct
{
	STA_POKE_WORK *pokeWork;
	MOVE_WORK_VEC moveWork;
}MOVE_POKE_VEC;

typedef struct
{
	STA_OBJ_WORK *objWork;
	MOVE_WORK_VEC moveWork;
}MOVE_OBJ_VEC;

typedef struct
{
	STA_LIGHT_WORK *lightWork;
	MOVE_WORK_VEC moveWork;
}MOVE_LIGHT_VEC;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//関数を全部staticで宣言
//static SCRIPT_FINISH_TYPE STA_SCRIPT_FuncName_Func(STA_SCRIPT_SYS *work,STA_SCRIPT_WORK *scriptWork);
#define SCRIPT_FUNC_DEF(str) SCRIPT_FINISH_TYPE STA_SCRIPT_ ## str ## _Func(STA_SCRIPT_SYS *work,STA_SCRIPT_WORK *scriptWork);
#include "script_table.dat"
#undef SCRIPT_FUNC_DEF

//TCB用関数定義
//座標補完移動
static BOOL SCRIPT_TCB_UpdateMoveS32( MOVE_WORK_S32 *moveWork , s32 *newPos );
static BOOL SCRIPT_TCB_UpdateMoveVec( MOVE_WORK_VEC *moveWork , VecFx32 *newPos );

static void SCRIPT_TCB_MoveCurtainTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveStageTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MovePokeTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveObjTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveLightTCB(  GFL_TCB *tcb, void *work );

//------------------------------------------------------------------
//	汎用関数
//------------------------------------------------------------------
static BOOL SCRIPT_TCB_UpdateMoveS32( MOVE_WORK_S32 *moveWork , s32 *newPos )
{
	moveWork->step++;
	
	if( moveWork->step >= moveWork->frame )
	{
		*newPos = moveWork->end;
		return TRUE;
	}
	else
	{
		const s32 ofs = moveWork->end - moveWork->start;
		*newPos = moveWork->start + (ofs * moveWork->step / moveWork->frame);
		return FALSE;
	}
}

static BOOL SCRIPT_TCB_UpdateMoveVec( MOVE_WORK_VEC *moveWork , VecFx32 *newPos )
{
	moveWork->step++;
	
	if( moveWork->step >= moveWork->frame )
	{
		newPos->x = moveWork->end.x;
		newPos->y = moveWork->end.y;
		newPos->z = moveWork->end.z;
		return TRUE;
	}
	else
	{
		VEC_MultAdd( FX32_CONST(moveWork->step) , &moveWork->stepVal ,
					 &moveWork->start , newPos );
		return FALSE;
	}
}



#define SCRIPT_FUNC_DEF(str) SCRIPT_FINISH_TYPE STA_SCRIPT_ ## str ## _Func(STA_SCRIPT_SYS *work,STA_SCRIPT_WORK *scriptWork)

//------------------------------------------------------------------
//	システム
//------------------------------------------------------------------
#pragma mark [>System
//スクリプトの終了
SCRIPT_FUNC_DEF( ScriptFinish )
{
	SCRIPT_PRINT_LABEL(ScriptFinish);
	return SFT_END;
}

//指定フレーム待機
SCRIPT_FUNC_DEF( FrameWait )
{
	const s32 count = ScriptFunc_GetValueS32(scriptWork->loadPos);
	scriptWork->waitCnt = count;

	SCRIPT_PRINT_LABEL(FrameWait);
	return SFT_SUSPEND;
}

//指定フレームまで待つ
SCRIPT_FUNC_DEF( FrameWaitTime )
{
	const s32 count = ScriptFunc_GetValueS32(scriptWork->loadPos);
;
	SCRIPT_PRINT_LABEL(FrameWaitTime);
	
	if( scriptWork->frame >= count )
	{
		return SFT_CONTINUE;
	} 
	else
	{
		scriptWork->waitCnt = count - scriptWork->frame;
		return SFT_SUSPEND;
	}
	
}

//------------------------------------------------------------------
//	カーテン
//------------------------------------------------------------------
#pragma mark [>Curtain

//カーテン上げる(固定速度
SCRIPT_FUNC_DEF( CurtainUp )
{
	MOVE_WORK_S32 *moveWork;
	SCRIPT_PRINT_LABEL(CurtainUp);

	moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
	moveWork->actWork = work->actWork;
	moveWork->step = 0;
	moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
	moveWork->end = ACT_CURTAIN_SCROLL_MAX;
	moveWork->frame = MATH_ABS(moveWork->end-moveWork->start)/ACT_CURTAIN_SCROLL_SPEED;
	moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
	
	GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , 10 );
	
	return SFT_CONTINUE;
	
}
//カーテン下げる(固定速度
SCRIPT_FUNC_DEF( CurtainDown )
{
	MOVE_WORK_S32 *moveWork;
	SCRIPT_PRINT_LABEL(CurtainDown);
	moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
	moveWork->actWork = work->actWork;
	moveWork->step = 0;
	moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
	moveWork->end = ACT_CURTAIN_SCROLL_MIN;
	moveWork->frame = MATH_ABS(moveWork->end-moveWork->start)/ACT_CURTAIN_SCROLL_SPEED;
	moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
	
	GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , 10 );
	
	return SFT_CONTINUE;
}
//カーテン動かす
SCRIPT_FUNC_DEF( CurtainMove )
{
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 pos = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	
	if( frame == 0 )
	{
		STA_ACT_SetCurtainHeight( work->actWork , pos);
	}
	else
	{
		MOVE_WORK_S32 *moveWork;
		moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
		moveWork->actWork = work->actWork;
		moveWork->step = 0;
		moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
		moveWork->end = pos;
		moveWork->frame = frame;
		moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
	
		GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , 10 );
	}
	
	SCRIPT_PRINT_LABEL(CurtainMove);
	return SFT_CONTINUE;
}
//カーテン制御TCB
static void SCRIPT_TCB_MoveCurtainTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_WORK_S32 *moveWork = (MOVE_WORK_S32*)work;
	
	s32 newPos;
	
	const BOOL isFinish = SCRIPT_TCB_UpdateMoveS32( moveWork , &newPos );
	
	STA_ACT_SetCurtainHeight( moveWork->actWork , newPos);
	if( isFinish == TRUE )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

//------------------------------------------------------------------
//	舞台系
//------------------------------------------------------------------
#pragma mark [>Stage

//ステージを移動させる
SCRIPT_FUNC_DEF( StageMove )
{
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 pos = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	
	if( frame == 0 )
	{
		STA_ACT_SetStageScroll( work->actWork , pos);
	}
	else
	{
		MOVE_WORK_S32 *moveWork;
		moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
		moveWork->actWork = work->actWork;
		moveWork->step = 0;
		moveWork->start = STA_ACT_GetStageScroll( work->actWork );
		moveWork->end = pos;
		moveWork->frame = frame;
		moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
	
		GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveStageTCB , (void*)moveWork , 10 );
	}
		SCRIPT_PRINT_LABEL(StageMove);
	return SFT_CONTINUE;
}

//ステージ制御TCB
static void SCRIPT_TCB_MoveStageTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_WORK_S32 *moveWork = (MOVE_WORK_S32*)work;
	
	s32 newPos;
	
	const BOOL isFinish = SCRIPT_TCB_UpdateMoveS32( moveWork , &newPos );
	
	STA_ACT_SetStageScroll( moveWork->actWork , newPos);
	if( isFinish == TRUE )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

//------------------------------------------------------------------
//	ポケモン
//------------------------------------------------------------------
#pragma mark [>Pokemon

//ポケモン表示制御
SCRIPT_FUNC_DEF( PokeShow )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 flg = ScriptFunc_GetValueS32(scriptWork->loadPos);

	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );

	STA_POKE_SetShowFlg( pokeSys , pokeWork , flg );

	SCRIPT_PRINT_LABEL(PokeShow);
	return SFT_CONTINUE;
}

//ポケモン向き
SCRIPT_FUNC_DEF( PokeDir )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 dir = ScriptFunc_GetValueS32(scriptWork->loadPos);

	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );

	STA_POKE_SetPokeDir( pokeSys , pokeWork , (dir==0 ? SPD_LEFT : SPD_RIGHT) );

	SCRIPT_PRINT_LABEL(PokeDir);
	return SFT_CONTINUE;
}

//ポケモン移動
SCRIPT_FUNC_DEF( PokeMove )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 posX = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const fx32 posY = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const fx32 posZ = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	
	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );

	if( frame == 0 )
	{
		VecFx32 pos;
		VEC_Set( &pos , posX,posY,posZ );
		STA_POKE_SetPosition( pokeSys , pokeWork , &pos );		
	}
	else
	{
		VecFx32 subVec;
		MOVE_POKE_VEC *movePoke;
		movePoke = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_POKE_VEC ));
		movePoke->pokeWork = pokeWork;
		movePoke->moveWork.actWork = work->actWork;
		movePoke->moveWork.step = 0;
		STA_POKE_GetPosition( pokeSys , pokeWork , &movePoke->moveWork.start );
		VEC_Set( &movePoke->moveWork.end , posX,posY,posZ );
		movePoke->moveWork.frame = frame;

		VEC_Subtract( &movePoke->moveWork.end , &movePoke->moveWork.start , &subVec );
		movePoke->moveWork.stepVal.x = subVec.x / frame;
		movePoke->moveWork.stepVal.y = subVec.y / frame;
		movePoke->moveWork.stepVal.z = subVec.z / frame;

		GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MovePokeTCB , (void*)movePoke , 10 );
	}
	
	SCRIPT_PRINT_LABEL(PokeMove);
	return SFT_CONTINUE;
}
//ポケモン移動制御TCB
static void SCRIPT_TCB_MovePokeTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_POKE_VEC *movePoke = (MOVE_POKE_VEC*)work;
	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( movePoke->moveWork.actWork );
	VecFx32 newPos;
	
	const BOOL isFinish = SCRIPT_TCB_UpdateMoveVec( &movePoke->moveWork , &newPos );
	STA_POKE_SetPosition( pokeSys , movePoke->pokeWork , &newPos );
	
	if( isFinish == TRUE )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

//ポケモンアニメ停止
SCRIPT_FUNC_DEF( PokeStopAnime )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
	
	STA_POKE_StopAnime( pokeSys , pokeWork );
	
	SCRIPT_PRINT_LABEL(PokeStopAnime);
	return SFT_CONTINUE;
}

//ポケモンアニメ開始
SCRIPT_FUNC_DEF( PokeStartAnime )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );

	STA_POKE_StartAnime( pokeSys , pokeWork );

	SCRIPT_PRINT_LABEL(PokeStartAnime);
	return SFT_CONTINUE;
}

//ポケモンアニメ変更
SCRIPT_FUNC_DEF( PokeChangeAnime )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 anmIdx = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );

	STA_POKE_ChangeAnime( pokeSys , pokeWork , anmIdx );

	SCRIPT_PRINT_LABEL(PokeChangeAnime);
	return SFT_CONTINUE;
}

//------------------------------------------------------------------
//オブジェクト系
//------------------------------------------------------------------
#pragma mark [>Object

//オブジェクト作成
SCRIPT_FUNC_DEF( ObjectCreate )
{
	const s32 objNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 objType = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork;

	objWork = STA_OBJ_CreateObject( objSys , objType );

	STA_ACT_SetObjectWork( work->actWork , objWork , objNo );

	SCRIPT_PRINT_LABEL(ObjectCreate);
	return SFT_CONTINUE;
}

//オブジェクト破棄
SCRIPT_FUNC_DEF( ObjectDelete )
{
	const s32 objNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
	
	STA_OBJ_DeleteObject( objSys , objWork );

	SCRIPT_PRINT_LABEL(ObjectDelete);
	return SFT_CONTINUE;
}

//オブジェクト表示
SCRIPT_FUNC_DEF( ObjectShow )
{
	const s32 objNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
	
	STA_OBJ_SetShowFlg( objSys , objWork , TRUE );
	
	SCRIPT_PRINT_LABEL(ObjectShow);
	return SFT_CONTINUE;
}

//オブジェクト非表示
SCRIPT_FUNC_DEF( ObjectHide )
{
	const s32 objNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
	
	STA_OBJ_SetShowFlg( objSys , objWork , FALSE );
	
	SCRIPT_PRINT_LABEL(ObjectHide);
	return SFT_CONTINUE;
}

//オブジェクト移動
SCRIPT_FUNC_DEF( ObjectMove )
{
	const s32 objNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 posX = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const fx32 posY = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const fx32 posZ = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );

	if( frame == 0 )
	{
		VecFx32 pos;
		VEC_Set( &pos , posX,posY,posZ );
		STA_OBJ_SetPosition( objSys , objWork , &pos );		
	}
	else
	{
		VecFx32 subVec;
		MOVE_OBJ_VEC *moveObj;
		moveObj = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_OBJ_VEC ));
		moveObj->objWork = objWork;
		moveObj->moveWork.actWork = work->actWork;
		moveObj->moveWork.step = 0;
		STA_OBJ_GetPosition( objSys , objWork , &moveObj->moveWork.start );
		VEC_Set( &moveObj->moveWork.end , posX,posY,posZ );
		moveObj->moveWork.frame = frame;

		VEC_Subtract( &moveObj->moveWork.end , &moveObj->moveWork.start , &subVec );
		moveObj->moveWork.stepVal.x = subVec.x / frame;
		moveObj->moveWork.stepVal.y = subVec.y / frame;
		moveObj->moveWork.stepVal.z = subVec.z / frame;

		GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveObjTCB , (void*)moveObj , 10 );
	}

	SCRIPT_PRINT_LABEL(ObjectMove);
	return SFT_CONTINUE;
}

//オブジェクト移動制御TCB
static void SCRIPT_TCB_MoveObjTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_OBJ_VEC *moveObj = (MOVE_OBJ_VEC*)work;
	STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( moveObj->moveWork.actWork );
	VecFx32 newPos;
	
	const BOOL isFinish = SCRIPT_TCB_UpdateMoveVec( &moveObj->moveWork , &newPos );
	STA_OBJ_SetPosition( objSys , moveObj->objWork , &newPos );
	
	if( isFinish == TRUE )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

//------------------------------------------------------------------
//	ライト
//------------------------------------------------------------------
#pragma mark [>SpotLight

//ライトON(円形
SCRIPT_FUNC_DEF( LightShowCircle )
{
	const s32 lightNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 rad = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork;
	
	lightWork = STA_LIGHT_CreateObject( lightSys , ALT_CIRCLE );
	STA_LIGHT_SetColor( lightSys , lightWork , GX_RGB(31,31,0) , 16 );
	STA_LIGHT_SetOptionValue( lightSys , lightWork , rad , 0 );
	
	STA_ACT_SetLightWork( work->actWork , lightWork , lightNo );
	
	SCRIPT_PRINT_LABEL(LightShowCircle);
	return SFT_CONTINUE;
}

//ライトON(台形
SCRIPT_FUNC_DEF( LightShowShine )
{
	const s32 lightNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 upLen = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 downLen = ScriptFunc_GetValueS32(scriptWork->loadPos);
	
	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork;
	
	lightWork = STA_LIGHT_CreateObject( lightSys , ALT_SHINES );
	STA_LIGHT_SetColor( lightSys , lightWork , GX_RGB(31,31,0) , 16 );
	STA_LIGHT_SetOptionValue( lightSys , lightWork , upLen , downLen );
	
	STA_ACT_SetLightWork( work->actWork , lightWork , lightNo );
	

	SCRIPT_PRINT_LABEL(LightShowShine);
	return SFT_CONTINUE;
}

//ライトOFF
SCRIPT_FUNC_DEF( LightHide )
{
	const s32 lightNo = ScriptFunc_GetValueS32(scriptWork->loadPos);

	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );
	
	STA_LIGHT_DeleteObject( lightSys , lightWork );

	SCRIPT_PRINT_LABEL(LightHide);
	return SFT_CONTINUE;
}

//ライト移動
SCRIPT_FUNC_DEF( LightMove )
{
	const s32 lightNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const fx32 posX = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const fx32 posY = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	//現在Zは計算してない。有効にすることは可能だが必要？
	const fx32 posZ = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	
	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );

	if( frame == 0 )
	{
		VecFx32 pos;
		VEC_Set( &pos , posX,posY,posZ );
		STA_LIGHT_SetPosition( lightSys , lightWork , &pos );
	}
	else
	{
		VecFx32 subVec;
		MOVE_LIGHT_VEC *moveLight;
		moveLight = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_LIGHT_VEC ));
		moveLight->lightWork = lightWork;
		moveLight->moveWork.actWork = work->actWork;
		moveLight->moveWork.step = 0;
		STA_LIGHT_GetPosition( lightSys , lightWork , &moveLight->moveWork.start );
		VEC_Set( &moveLight->moveWork.end , posX,posY,posZ );
		moveLight->moveWork.frame = frame;

		VEC_Subtract( &moveLight->moveWork.end , &moveLight->moveWork.start , &subVec );
		moveLight->moveWork.stepVal.x = subVec.x / frame;
		moveLight->moveWork.stepVal.y = subVec.y / frame;
		moveLight->moveWork.stepVal.z = subVec.z / frame;

		GFL_TCB_AddTask( work->tcbSys , SCRIPT_TCB_MoveLightTCB , (void*)moveLight , 10 );
	}
	
	SCRIPT_PRINT_LABEL(LightMove);
	return SFT_CONTINUE;
}

//ライト移動制御TCB
static void SCRIPT_TCB_MoveLightTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_LIGHT_VEC *moveLight = (MOVE_LIGHT_VEC*)work;
	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( moveLight->moveWork.actWork );
	VecFx32 newPos;
	
	const BOOL isFinish = SCRIPT_TCB_UpdateMoveVec( &moveLight->moveWork , &newPos );
	STA_LIGHT_SetPosition( lightSys , moveLight->lightWork , &newPos );
	
	if( isFinish == TRUE )
	{
		GFL_TCB_DeleteTask( tcb );
	}
}

//ライト色設定
SCRIPT_FUNC_DEF( LightColor )
{
	const s32 lightNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 colR = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 colG = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 colB = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 alpha = ScriptFunc_GetValuefx32(scriptWork->loadPos);
	const GXRgb col = GX_RGB( (u8)colR , (u8)colG , (u8)colB );
	
	STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );

	STA_LIGHT_SetColor( lightSys , lightWork , col , alpha );
	
	SCRIPT_PRINT_LABEL(LightColor);
	return SFT_CONTINUE;
}

//------------------------------------------------------------------
//	メッセージ
//------------------------------------------------------------------
#pragma mark [>Message

//メッセージ表示
SCRIPT_FUNC_DEF( MessageShow )
{
	const s32 msgNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 dispSpd = ScriptFunc_GetValueS32(scriptWork->loadPos);

	STA_ACT_ShowMessage( work->actWork , msgNo , dispSpd );
	SCRIPT_PRINT_LABEL(MessageShow);
	return SFT_CONTINUE;
}

//メッセージ消去
SCRIPT_FUNC_DEF( MessageHide )
{
	
	STA_ACT_HideMessage( work->actWork );
	SCRIPT_PRINT_LABEL(MessageHide);
	return SFT_CONTINUE;
}

#undef SCRIPT_FUNC_DEF
