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
#define SCRIPT_PRINT_LABEL(str) ARI_TPrintf("SCRIPT No[%d]%s\n", SCRIPT_ENUM_ ## str ,#str);

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
static BOOL SCRIPT_TCB_UpdateMoveVec( MOVE_WORK_VEC *moveWork , VecFx32 *newPos );

static void SCRIPT_TCB_MoveCurtainTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MovePokeTCB(  GFL_TCB *tcb, void *work );

//------------------------------------------------------------------
//	汎用関数
//------------------------------------------------------------------
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
//スクリプトの終了
SCRIPT_FUNC_DEF( ScriptFinish )
{
	SCRIPT_PRINT_LABEL(ScriptFinish);
	return SFT_END;
}

//指定フレーム待機
SCRIPT_FUNC_DEF( FrameWait )
{
	s32 count;
	SCRIPT_PRINT_LABEL(FrameWait);
	count = ScriptFunc_GetValueS32(scriptWork->loadPos);
	scriptWork->waitCnt = count;
	return SFT_SUSPEND;
}

//カーテン上げる(固定速度
SCRIPT_FUNC_DEF( UpCurtain )
{
	MOVE_WORK_S32 *moveWork;
	SCRIPT_PRINT_LABEL(UpCurtain);

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
SCRIPT_FUNC_DEF( DownCurtain )
{
	MOVE_WORK_S32 *moveWork;
	SCRIPT_PRINT_LABEL(DownCurtain);
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
SCRIPT_FUNC_DEF( MoveCurtain )
{
	const s32 frame = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 pos = ScriptFunc_GetValuefx32(scriptWork->loadPos);

	SCRIPT_PRINT_LABEL(MoveCurtain);
	
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
	
	return SFT_CONTINUE;
}
//カーテン制御TCB
static void SCRIPT_TCB_MoveCurtainTCB(  GFL_TCB *tcb, void *work )
{
	MOVE_WORK_S32 *moveWork = (MOVE_WORK_S32*)work;
	
	moveWork->step++;
	
	if( moveWork->step >= moveWork->frame )
	{
		STA_ACT_SetCurtainHeight( moveWork->actWork , moveWork->end);
		
		GFL_TCB_DeleteTask( tcb );
	}
	else
	{
		STA_ACT_SetCurtainHeight( moveWork->actWork , moveWork->start+(moveWork->stepVal*moveWork->step));
	}
}

//ポケモン表示制御
SCRIPT_FUNC_DEF( ShowPoke )
{
	const s32 pokeNo = ScriptFunc_GetValueS32(scriptWork->loadPos);
	const s32 flg = ScriptFunc_GetValueS32(scriptWork->loadPos);

	STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
	STA_POKE_DRAW_SetShowFlg( pokeWork , flg );

	SCRIPT_PRINT_LABEL(ShowPoke);
	return SFT_CONTINUE;
}

//ポケモン移動
SCRIPT_FUNC_DEF( MovePoke )
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
	
	SCRIPT_PRINT_LABEL(MovePoke);
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


#undef SCRIPT_FUNC_DEF
