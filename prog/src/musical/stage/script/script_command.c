//======================================================================
/**
 * @file  sta_act_script.h
 * @brief ステージ スクリプト処理 コマンド群
 * @author  ariizumi
 * @data  09/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "print/gf_font.h"

#include "stage_gra.naix"
#include "script_table.h"
#include "../sta_local_def.h"
#include "../sta_snd_def.h"
#include "test/ariizumi/ari_debug.h"


//======================================================================
//  define
//======================================================================
#pragma mark [> define

#if defined(DEBUG_ONLY_FOR_iwao_kazumasa)|1
#define SCRIPT_TPrintf ARI_TPrintf
#define SCRIPT_Printf OS_Printf
#else
#define SCRIPT_TPrintf ARI_TPrintf
#define SCRIPT_Printf ARI_Printf
#endif


#define ScriptFunc_GetValueS32() (VMGetU32(scriptWork->vmHandle))
#define ScriptFunc_GetValuefx32() (VMGetU32(scriptWork->vmHandle))

//通常のタスクプライオリティ
//追従系などは低く設定することで、本体の移動を優先させる
#define SCRIPT_TCB_PRI_NORMAL (10)
#define SCRIPT_TCB_PRI_LOW (7)

#if DEB_ARI
#define SCRIPT_PRINT_LABEL(str) SCRIPT_TPrintf("SCRIPT[%d] Frame[%4d] No[%2d]%s\n",scriptWork->lineNo,scriptWork->frame, SCRIPT_ENUM_ ## str ,CommandStr[SCRIPT_ENUM_ ## str]);
static const char* const CommandStr[] = {
                  "スクリプトの終了",
                  "指定フレーム待つ",
                  "指定フレームまで待つ",
                  "スクリプトの同期をとる",
                  "幕を上げる",
                  "幕を下げる",
                  "幕を動かす",
                  "ステージスクロール",
                  "BG読み変え",
                  "ポケ表示切替",
                  "ポケ方向",
                  "ポケ移動",
                  "ポケ相対移動",
                  "ポケアニメ停止",
                  "ポケアニメ開始",
                  "ポケアニメ変更",
                  "ポケアクション跳ねる",
                  "ポケモン動作番号予約",
                  "オブジェ作成",
                  "オブジェ破棄",
                  "オブジェ表示",
                  "オブジェ非表示",
                  "オブジェ移動",
                  "エフェクト作成",
                  "エフェクト破棄",
                  "エフェクト再生",
                  "エフェクト停止",
                  "エフェクト連続再生",
                  "ライトON(円形)",
                  "ライトON(半径)",
                  "ライトOFF",
                  "ライト移動",
                  "ライト追従移動",
                  "ライト色設定",
                  "メッセージ表示",
                  "メッセージ消去",
                  "メッセージ色変え",
                  "BGM再生",
                  "BGM停止",
                  "ポケアクション回転",
                  "ポケ正面・背面切り替え",
                  "ポケアイテム表示切替",
                  "注目ポケモン設定",
                  "注目ポケモン解除",
                  "メインパートの開始",
                  "メインパートの終了",
                  "トップのポケモンに寄る",
                  "ラベル",
                  };
#else
#define SCRIPT_PRINT_LABEL(str) 
#endif

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  RMR_WAIT, //待機
  RMR_ACTION, //アクション実行
  RMR_END,  //終了
}REPEAT_MNG_RETURN;


//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
//直線移動管理構造体(幕・舞台用)
typedef struct
{
  //TODO 精度確保のためにfx32とかでやったほうが良いかも
  //    現状精度確保のために毎フレーム除算してる
  ACTING_WORK *actWork;
  s32 start;
  s32 end;
//  s32 stepVal;  //毎フレームの増加量
  u16 frame;
  u16 step;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}MOVE_WORK_S32;

//ベクトル移動管理構造体(ポケ・ライト・OBJ・エフェクト等)
typedef struct
{
  ACTING_WORK *actWork;
  VecFx32 start;
  VecFx32 end;
  VecFx32 stepVal;  //毎フレームの増加量
  u16 frame;
  u16 step;
}MOVE_WORK_VEC;

typedef struct
{
  STA_POKE_WORK *pokeWork;
  MOVE_WORK_VEC moveWork;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}MOVE_POKE_VEC;

typedef struct
{
  STA_OBJ_WORK *objWork;
  MOVE_WORK_VEC moveWork;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}MOVE_OBJ_VEC;

typedef struct
{
  STA_LIGHT_WORK *lightWork;
  MOVE_WORK_VEC moveWork;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}MOVE_LIGHT_VEC;

//ポケ追従移動管理構造体(ライト・OBJ・エフェクト等)
typedef struct
{
  ACTING_WORK *actWork;
  STA_POKE_WORK *pokeWork;
  VecFx32 ofs;      //オフセット
  u16 frame;
  u16 step;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}POKE_TRACE_WORK_VEC;


typedef struct
{
  u8 trgPokeNo;
  STA_LIGHT_WORK *lightWork;
  POKE_TRACE_WORK_VEC moveWork;
  
  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}MOVE_TRACE_LIGHT_VEC;


//指定回数繰り返す時の管理
typedef struct
{
  ACTING_WORK *actWork;
  u32 step;
  u32 interval;
  u32 num;
}REPEAT_MNG_WORK;

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

//関数を全部を宣言
//VMCMD_RESULT STA_SCRIPT_FuncName_Func(VMHANDLE *vmh, void *context_work);
#define SCRIPT_FUNC_DEF(str) VMCMD_RESULT STA_SCRIPT_ ## str ## _Func(VMHANDLE *vmh, void *context_work);
#include "script_table.dat"
#undef SCRIPT_FUNC_DEF

//TCB用関数定義
//座標補完移動
static BOOL SCRIPT_TCB_UpdateMoveS32( MOVE_WORK_S32 *moveWork , s32 *newPos );
static BOOL SCRIPT_TCB_UpdateMoveVec( MOVE_WORK_VEC *moveWork , VecFx32 *newPos );
static REPEAT_MNG_RETURN SCRIPT_TCB_UpdateRepeat( REPEAT_MNG_WORK *repeatWork );
static BOOL SCRIPT_TCB_UpdatePokeTrace( POKE_TRACE_WORK_VEC *moveWork , VecFx32 *newPos );

//移動系
static void SCRIPT_TCB_MoveCurtainTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveStageTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MovePokeTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveObjTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveLightTCB(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_MoveTraceLightTCB(  GFL_TCB *tcb, void *work );

//アクション系
static void SCRIPT_TCB_PokeAct_Jump(  GFL_TCB *tcb, void *work );
static void SCRIPT_TCB_PokeAct_Rotate(  GFL_TCB *tcb, void *work );

static void SCRIPT_TCB_EffectRepeat(  GFL_TCB *tcb, void *work );

//便利関数
static const s32 ScriptFunc_GetPokeBit( const STA_SCRIPT_WORK *scriptWork , const s32 pokeNo );


//======================================================================
//  data
//======================================================================

//------------------------------------------------------------------
//  汎用関数
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

static BOOL SCRIPT_TCB_UpdatePokeTrace( POKE_TRACE_WORK_VEC *moveWork , VecFx32 *newPos )
{
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( moveWork->actWork );
  VecFx32 pokePos;
  
  STA_POKE_GetPosition( pokeSys , moveWork->pokeWork , &pokePos );
  VEC_Add( &pokePos , &moveWork->ofs , newPos );

  moveWork->step++;
  if( moveWork->step >= moveWork->frame )
  {
    return TRUE;
  }
  return FALSE;
  
}

static REPEAT_MNG_RETURN SCRIPT_TCB_UpdateRepeat( REPEAT_MNG_WORK *repeatWork )
{
  repeatWork->step++;
  
  if( repeatWork->step >= repeatWork->interval )
  {
    repeatWork->num--;
    repeatWork->step = 0;

    if( repeatWork->num <= 0 )
    {
      return RMR_END;
    }
    else
    {
      return RMR_ACTION;
    }
  }
  return RMR_WAIT;
}



#define SCRIPT_FUNC_DEF(str) VMCMD_RESULT STA_SCRIPT_ ## str ## _Func(VMHANDLE *vmh, void *context_work)

//------------------------------------------------------------------
//  システム
//------------------------------------------------------------------
#pragma mark [>System
//スクリプトの終了
SCRIPT_FUNC_DEF( ScriptFinish )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  SCRIPT_PRINT_LABEL(ScriptFinish);
  scriptWork->isFlag |= SFB_IS_FINISH;
  return SFT_SUSPEND;
}

//スクリプトの終了
SCRIPT_FUNC_DEF( Label )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  const s32 value = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(Label);
  ARI_TPrintf("     LabelCommand[%d]\n",value);
  return SFT_CONTINUE;
}

//指定フレーム待機
SCRIPT_FUNC_DEF( FrameWait )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  const s32 count = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(FrameWait);
  scriptWork->waitCnt = count;

  return SFT_SUSPEND;
}

//指定フレームまで待つ
SCRIPT_FUNC_DEF( FrameWaitTime )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 count = ScriptFunc_GetValueS32();

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

//同期待ちを行う
SCRIPT_FUNC_DEF( SyncScript )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 count = ScriptFunc_GetValueS32();

  SCRIPT_PRINT_LABEL(SyncScript);

  GF_ASSERT_MSG( scriptWork->isFlag & SFB_IS_TARGET_SYNC , "Script is not sync target!!\n")
  if( scriptWork->isFlag & SFB_IS_TARGET_SYNC )
  {
    scriptWork->isFlag |= SFB_WAIT_SYNC;
  }
  
  return SFT_SUSPEND;
}

//------------------------------------------------------------------
//  カーテン
//------------------------------------------------------------------
#pragma mark [>Curtain

//カーテン上げる(固定速度
SCRIPT_FUNC_DEF( CurtainUp )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  MOVE_WORK_S32 *moveWork;
  const BOOL autoWait = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(CurtainUp);

  moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
  moveWork->scriptSys = work;
  moveWork->actWork = work->actWork;
  moveWork->step = 0;
  moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
  moveWork->end = ACT_CURTAIN_SCROLL_MAX;
  moveWork->frame = MATH_ABS(moveWork->end-moveWork->start)/ACT_CURTAIN_SCROLL_SPEED;
//  moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
  
  moveWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , SCRIPT_TCB_PRI_NORMAL );
  
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = moveWork->frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  return SFT_CONTINUE;
  
}
//カーテン下げる(固定速度
SCRIPT_FUNC_DEF( CurtainDown )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const BOOL autoWait = ScriptFunc_GetValueS32();
  MOVE_WORK_S32 *moveWork;
  SCRIPT_PRINT_LABEL(CurtainDown);
  moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
  moveWork->scriptSys = work;
  moveWork->actWork = work->actWork;
  moveWork->step = 0;
  moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
  moveWork->end = ACT_CURTAIN_SCROLL_MIN;
  moveWork->frame = MATH_ABS(moveWork->end-moveWork->start)/ACT_CURTAIN_SCROLL_SPEED;
//  moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
  
  moveWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , SCRIPT_TCB_PRI_NORMAL );
  
  PMSND_PlaySE( STA_SE_CLOSE_CURTAIN );
//  PMSND_PlaySE( SEQ_SE_MSCL_09 );

  if( autoWait == 1 )
  {
    scriptWork->waitCnt = moveWork->frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  
  return SFT_CONTINUE;
}
//カーテン動かす
SCRIPT_FUNC_DEF( CurtainMove )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 frame = ScriptFunc_GetValueS32();
  const s32 pos = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(CurtainMove);
  
  if( frame == 0 )
  {
    STA_ACT_SetCurtainHeight( work->actWork , pos);
  }
  else
  {
    MOVE_WORK_S32 *moveWork;
    moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
    moveWork->scriptSys = work;
    moveWork->actWork = work->actWork;
    moveWork->step = 0;
    moveWork->start = STA_ACT_GetCurtainHeight( work->actWork );
    moveWork->end = pos;
    moveWork->frame = frame;
//    moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
  
    moveWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveCurtainTCB , (void*)moveWork , SCRIPT_TCB_PRI_NORMAL );
  
    if( autoWait == 1 )
    {
      scriptWork->waitCnt = moveWork->frame;
      SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
      return SFT_SUSPEND;
    }
    
  }
  
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
    STA_SCRIPT_DeleteTcbTask( moveWork->scriptSys , moveWork->tcbObj );
  }
}

//------------------------------------------------------------------
//  舞台系
//------------------------------------------------------------------
#pragma mark [>Stage

//ステージを移動させる
SCRIPT_FUNC_DEF( StageMove )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 frame = ScriptFunc_GetValueS32();
  const s32 pos = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(StageMove);
  
  if( frame == 0 )
  {
    STA_ACT_SetStageScroll( work->actWork , pos);
  }
  else
  {
    MOVE_WORK_S32 *moveWork;
    moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
    moveWork->scriptSys = work;
    moveWork->actWork = work->actWork;
    moveWork->step = 0;
    moveWork->start = STA_ACT_GetStageScroll( work->actWork );
    moveWork->end = pos;
    moveWork->frame = frame;
//    moveWork->stepVal = (moveWork->end-moveWork->start)/moveWork->frame;
  
    moveWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveStageTCB , (void*)moveWork , SCRIPT_TCB_PRI_NORMAL );
  
    if( autoWait == 1 )
    {
      scriptWork->waitCnt = moveWork->frame;
      SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
      return SFT_SUSPEND;
    }
    
  }

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
    STA_SCRIPT_DeleteTcbTask( moveWork->scriptSys , moveWork->tcbObj );
  }
}

//BG読み替え
SCRIPT_FUNC_DEF( StageChangeBg )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 bgNo = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(StageChangeBg);
  
  //ダミー
  //STA_ACT_LoadBg( work->actWork , bgNo );

  return SFT_CONTINUE;
}

//メインパート開始
SCRIPT_FUNC_DEF( StageStartMainPart )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  SCRIPT_PRINT_LABEL(StageStartMainPart);
  
  STA_ACT_StartMainPart( work->actWork );
  return SFT_CONTINUE;
}

//メインパート終了
SCRIPT_FUNC_DEF( StageFinishMainPart )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  SCRIPT_PRINT_LABEL(StageFinishMainPart);
  
  STA_ACT_FinishMainPart( work->actWork );
  return SFT_CONTINUE;
}

//------------------------------------------------------------------
//  ポケモン
//------------------------------------------------------------------
#pragma mark [>Pokemon

static const s32 ScriptFunc_GetPokeBit( const STA_SCRIPT_WORK *scriptWork , const s32 pokeNo )
{
  s32 bit;
  if( pokeNo == -1 )
  {
    bit = scriptWork->trgPokeFlg;
  }
  else
  {
    bit = 1<<pokeNo;
  }
  return bit;
}

//ポケモン表示制御
SCRIPT_FUNC_DEF( PokeShow )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 flg = ScriptFunc_GetValueS32();
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );

  u8  pokeNo;
  
  SCRIPT_PRINT_LABEL(PokeShow);
  
  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_SetShowFlg( pokeSys , pokeWork , flg );
    }
    
  }

  return SFT_CONTINUE;
}

//ポケモン向き
SCRIPT_FUNC_DEF( PokeDir )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 dir = ScriptFunc_GetValueS32();
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeDir);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_SetPokeDir( pokeSys , pokeWork , (dir==0 ? SPD_LEFT : SPD_RIGHT) );
    }
  }

  return SFT_CONTINUE;
}

//ポケモン移動
SCRIPT_FUNC_DEF( PokeMove )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 frame = ScriptFunc_GetValueS32();
  const fx32 posX = ScriptFunc_GetValuefx32();
  const fx32 posY = ScriptFunc_GetValuefx32();
  const fx32 posZ = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  u8  pokeNo;
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeMove);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
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
        movePoke->scriptSys = work;
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

        movePoke->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MovePokeTCB , (void*)movePoke , SCRIPT_TCB_PRI_NORMAL );
      }
    }
  }
  
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  
  return SFT_CONTINUE;
}

//ポケモン相対移動
SCRIPT_FUNC_DEF( PokeMoveOffset )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 frame = ScriptFunc_GetValueS32();
  const fx32 ofsX = ScriptFunc_GetValuefx32();
  const fx32 ofsY = ScriptFunc_GetValuefx32();
  const fx32 ofsZ = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  u8  pokeNo;
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeMoveOffset);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      if( frame == 0 )
      {
        VecFx32 pos;
        STA_POKE_GetPosition( pokeSys , pokeWork , &pos );
        pos.x += ofsX;
        pos.y += ofsY;
        pos.z += ofsZ;
        STA_POKE_SetPosition( pokeSys , pokeWork , &pos );    
      }
      else
      {
        VecFx32 subVec;
        MOVE_POKE_VEC *movePoke;
        movePoke = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_POKE_VEC ));
        movePoke->scriptSys = work;
        movePoke->pokeWork = pokeWork;
        movePoke->moveWork.actWork = work->actWork;
        movePoke->moveWork.step = 0;
        STA_POKE_GetPosition( pokeSys , pokeWork , &movePoke->moveWork.start );
        VEC_Set( &movePoke->moveWork.end , 
            movePoke->moveWork.start.x + ofsX,
            movePoke->moveWork.start.y + ofsY,
            movePoke->moveWork.start.z + ofsZ );
        movePoke->moveWork.frame = frame;

        movePoke->moveWork.stepVal.x = ofsX / frame;
        movePoke->moveWork.stepVal.y = ofsY / frame;
        movePoke->moveWork.stepVal.z = ofsZ / frame;

        movePoke->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MovePokeTCB , (void*)movePoke , SCRIPT_TCB_PRI_NORMAL );
      }
    }
  }
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  
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
    STA_SCRIPT_DeleteTcbTask( movePoke->scriptSys , movePoke->tcbObj );
  }
}

//ポケモンアニメ停止
SCRIPT_FUNC_DEF( PokeStopAnime )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  u8  pokeNo;
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeStopAnime);
  
  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_StopAnime( pokeSys , pokeWork );
    }
  }
  
  return SFT_CONTINUE;
}

//ポケモンアニメ開始
SCRIPT_FUNC_DEF( PokeStartAnime )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  u8  pokeNo;
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeStartAnime);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_StartAnime( pokeSys , pokeWork );
      
    }
  }

  return SFT_CONTINUE;
}

//ポケモンアニメ変更
SCRIPT_FUNC_DEF( PokeChangeAnime )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 anmIdx = ScriptFunc_GetValueS32();
  u8  pokeNo;
  
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeChangeAnime);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_ChangeAnime( pokeSys , pokeWork , anmIdx );
    }
  }

  return SFT_CONTINUE;
}

//ポケモンポケモン 正面：背面切り替え
SCRIPT_FUNC_DEF( PokeSetFrontBack )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 flg = ScriptFunc_GetValueS32();
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeSetFrontBack);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_SetFrontBack( pokeSys , pokeWork , (flg==0 ? FALSE : TRUE) );
    }
  }

  return SFT_CONTINUE;
}

//ポケモンポケモン アイテム表示切替
SCRIPT_FUNC_DEF( PokeDispItem )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 flg = ScriptFunc_GetValueS32();
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeDispItem);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_POKE_SetDrawItem( pokeSys , pokeWork , (flg==0 ? FALSE : TRUE) );
    }
  }

  return SFT_CONTINUE;
}

//ポケモン 変身エフェクト
SCRIPT_FUNC_DEF( PokeTransEffect )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeTransEffect);

  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      STA_ACT_PlayTransEffect( work->actWork , pokeNo );
    }
  }

  return SFT_CONTINUE;
}

//注目ポケモン設定	ポケモン番号
SCRIPT_FUNC_DEF( PokeAttentionOn )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNo = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(PokeAttentionOn);

  STA_ACT_SetLightUpFlg( work->actWork , pokeNo , TRUE );

  return SFT_CONTINUE;
}

//注目ポケモン解除
SCRIPT_FUNC_DEF( PokeAttentionOff )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  SCRIPT_PRINT_LABEL(PokeAttentionOff);

  STA_ACT_SetLightUpFlg( work->actWork , 0 , FALSE );
  STA_ACT_SetLightUpFlg( work->actWork , 1 , FALSE );
  STA_ACT_SetLightUpFlg( work->actWork , 2 , FALSE );
  STA_ACT_SetLightUpFlg( work->actWork , 3 , FALSE );

  return SFT_CONTINUE;
}

#pragma mark [>PokemonAction
//ポケモンアクション・跳ねる
typedef struct
{
  REPEAT_MNG_WORK repeatWork;
  STA_POKE_WORK *pokeWork;
  
  fx32  height;

  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}POKE_ACT_JUMP_WORK;

SCRIPT_FUNC_DEF( PokeActionJump )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 interval = ScriptFunc_GetValueS32();
  const s32 num = ScriptFunc_GetValueS32();
  const fx32 height = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeActionJump);
  
  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      POKE_ACT_JUMP_WORK  *jumpWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(POKE_ACT_JUMP_WORK));
      jumpWork->scriptSys = work;
      jumpWork->pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      jumpWork->height = height;
      
      jumpWork->repeatWork.actWork = work->actWork;
      jumpWork->repeatWork.step = 0;
      jumpWork->repeatWork.interval = interval;
      jumpWork->repeatWork.num = num;
      
      jumpWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_PokeAct_Jump , (void*)jumpWork , SCRIPT_TCB_PRI_NORMAL );
    }
  }
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = num*interval;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  return SFT_CONTINUE;

}

//ポケモンアクション　ジャンプ制御
static void SCRIPT_TCB_PokeAct_Jump(  GFL_TCB *tcb, void *work )
{
  POKE_ACT_JUMP_WORK *jumpWork = (POKE_ACT_JUMP_WORK*)work;
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( jumpWork->repeatWork.actWork );
  
  const REPEAT_MNG_RETURN ret = SCRIPT_TCB_UpdateRepeat( &jumpWork->repeatWork );
  if( ret == RMR_END )
  {
    STA_SCRIPT_DeleteTcbTask( jumpWork->scriptSys , jumpWork->tcbObj );
    return;
  }
  else
  {
    VecFx32 ofs;
    
    ofs.x = 0;
    ofs.y = -FX_Mul(FX_SinIdx( 0x8000 * jumpWork->repeatWork.step / jumpWork->repeatWork.interval ) , jumpWork->height );
    ofs.z = 0;
    
    STA_POKE_SetPositionOffset( pokeSys , jumpWork->pokeWork , &ofs );
    
  }
}

//ポケモンアクション・回転
typedef struct
{
  MOVE_WORK_S32 moveWork;
  STA_POKE_WORK *pokeWork;

  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}POKE_ACT_ROTATE_WORK;

SCRIPT_FUNC_DEF( PokeActionRotate )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 pokeNoTemp = ScriptFunc_GetValueS32();
  const s32 pokeNoBit = ScriptFunc_GetPokeBit( scriptWork , pokeNoTemp );
  const s32 frame = ScriptFunc_GetValueS32();
  const s32 startAngle = ScriptFunc_GetValueS32();
  const s32 rotateAngle = ScriptFunc_GetValuefx32();
  const BOOL autoWait = ScriptFunc_GetValueS32();
  u8  pokeNo;

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  SCRIPT_PRINT_LABEL(PokeActionJump);
  
  for( pokeNo=0;pokeNo<MUSICAL_POKE_MAX;pokeNo++ )
  {
    if( pokeNoBit & (1<<pokeNo) )
    {
      POKE_ACT_ROTATE_WORK  *rotWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(POKE_ACT_ROTATE_WORK));
      rotWork->scriptSys = work;
      rotWork->pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
      
      rotWork->moveWork.actWork = work->actWork;
      rotWork->moveWork.start = startAngle;
      rotWork->moveWork.end   = startAngle+rotateAngle;
      rotWork->moveWork.frame = frame;
      rotWork->moveWork.step  = 0;
      
      rotWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_PokeAct_Rotate , (void*)rotWork , SCRIPT_TCB_PRI_NORMAL );
    }
  }
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }

  return SFT_CONTINUE;

}

//ポケモンアクション　回転制御
static void SCRIPT_TCB_PokeAct_Rotate(  GFL_TCB *tcb, void *work )
{
  POKE_ACT_ROTATE_WORK *rotWork = (POKE_ACT_ROTATE_WORK*)work;
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( rotWork->moveWork.actWork );

  s32 newAngle;
  u16 angle;
  
  const BOOL isFinish = SCRIPT_TCB_UpdateMoveS32( &rotWork->moveWork , &newAngle );
  
  if( isFinish == TRUE )
  {
    angle = rotWork->moveWork.end*0x10000/360;
    
    STA_POKE_SetRotate( pokeSys , rotWork->pokeWork , angle );
    STA_SCRIPT_DeleteTcbTask( rotWork->scriptSys , rotWork->tcbObj );
  }
  else
  {
    angle = newAngle*0x10000/360;
    
    STA_POKE_SetRotate( pokeSys , rotWork->pokeWork , angle );
  }
}

//ポケモンアクション・トップへ寄る

SCRIPT_FUNC_DEF( PokeActionComeNearToTop )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 frame = ScriptFunc_GetValueS32();
  const BOOL autoWait = ScriptFunc_GetValueS32();

  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  u8 topPoke;
  VecFx32 topPos;
  s8 ofsPosArr[4]={0,0,0,0}; //Topからの相対位置
  u8 maxOfs = 0;
  
  SCRIPT_PRINT_LABEL(PokeActionComeNearToTop);
  
  //対象選択
  {
    u8 i;
    u8 maxPoint = 0;
    //最高点をチェック
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      if( maxPoint < STA_ACT_GetPokeEquipPoint( work->actWork , i ) )
      {
        maxPoint = STA_ACT_GetPokeEquipPoint( work->actWork , i );
      }
    }
    //トップかのチェック
    // todoトップが複数人いたらどうする？
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      if( maxPoint == STA_ACT_GetPokeEquipPoint( work->actWork , i ) )
      {
        topPoke = STA_ACT_GetPokeEquipPoint( work->actWork , i );
      }
    }
  }
  
  topPoke = GFUser_GetPublicRand0(4);
  ARI_TPrintf("Top[%d]\n",topPoke);
  //相対位置のチェック
  {
    u8 i;
    fx32 posArr[4];
    //座標収集
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      VecFx32 pos;
      STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , i );
      STA_POKE_GetPosition( pokeSys , pokeWork , &pos );
      posArr[i] = pos.x;
      if( i == topPoke )
      {
        topPos.x = pos.x;
        topPos.y = pos.y;
        topPos.z = pos.z;
      }
    }
    //差分に変換
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      posArr[i] -= topPos.x;
    }
    //位置チェック
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      u8 j;
      if( i == topPoke )
      {
        continue;
      }
      for( j=0;j<MUSICAL_POKE_MAX;j++ )
      {
        if( i == j )
        {
          continue;
        }
        if( posArr[i] > 0 && posArr[j] >= 0 &&
            posArr[i] > posArr[j] )
        {
          ofsPosArr[i]++;
        }
        else
        if( posArr[i] < 0 && posArr[j] <= 0 &&
            posArr[i] < posArr[j] )
        {
          ofsPosArr[i]--;
        }
      }
      //一番離れている距離
      if( maxOfs < MATH_ABS(ofsPosArr[i]) )
      {
        maxOfs = MATH_ABS(ofsPosArr[i]);
      }
    }
  }
  
  //動作設定
  {
    const fx32 COME_NEAR_VALUE = FX32_CONST(64.0f);
    fx32 posXSum = 0; //カメラ中心計算用
    u8 i;
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      VecFx32 subVec;
      MOVE_POKE_VEC *movePoke = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_POKE_VEC ));
      //水平移動
      movePoke->scriptSys = work;
      movePoke->pokeWork = STA_ACT_GetPokeWork( work->actWork , i );;
      movePoke->moveWork.actWork = work->actWork;
      movePoke->moveWork.step = 0;
      STA_POKE_GetPosition( pokeSys , movePoke->pokeWork , &movePoke->moveWork.start );
      movePoke->moveWork.end.x = topPos.x + COME_NEAR_VALUE * ofsPosArr[i];
      movePoke->moveWork.end.y = movePoke->moveWork.start.y;
      movePoke->moveWork.end.z = movePoke->moveWork.start.z;
      movePoke->moveWork.frame = frame*MATH_ABS(ofsPosArr[i])/maxOfs;

      movePoke->moveWork.stepVal.x = (movePoke->moveWork.end.x-movePoke->moveWork.start.x) / movePoke->moveWork.frame;
      movePoke->moveWork.stepVal.y = (movePoke->moveWork.end.y-movePoke->moveWork.start.y) / movePoke->moveWork.frame;
      movePoke->moveWork.stepVal.z = (movePoke->moveWork.end.z-movePoke->moveWork.start.z) / movePoke->moveWork.frame;

//      OS_Printf("---------------------------------\n");
//      OS_Printf("[%d]\n",movePoke->moveWork.frame);
//      OS_Printf("[%.1f][%.1f][%.1f]\n",FX_FX32_TO_F32(movePoke->moveWork.start.x),FX_FX32_TO_F32(movePoke->moveWork.start.y),FX_FX32_TO_F32(movePoke->moveWork.start.z));
//      OS_Printf("[%.1f][%.1f][%.1f]\n",FX_FX32_TO_F32(movePoke->moveWork.end.x),FX_FX32_TO_F32(movePoke->moveWork.end.y),FX_FX32_TO_F32(movePoke->moveWork.end.z));
//      OS_Printf("[%.1f][%.1f][%.1f]\n",FX_FX32_TO_F32(movePoke->moveWork.stepVal.x),FX_FX32_TO_F32(movePoke->moveWork.stepVal.y),FX_FX32_TO_F32(movePoke->moveWork.stepVal.z));

      movePoke->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MovePokeTCB , (void*)movePoke , SCRIPT_TCB_PRI_NORMAL );
      
      //ジャンプ
      if( movePoke->moveWork.frame != 0 )
      {
        POKE_ACT_JUMP_WORK  *jumpWork = GFL_HEAP_AllocMemory( work->heapId , sizeof(POKE_ACT_JUMP_WORK));
        const fx32 height = FX32_CONST(8);
        const u8 interval = 15;
        const u8 mod = movePoke->moveWork.frame%interval;
        const u8 num = movePoke->moveWork.frame/interval + (mod!=0?1:0);
        jumpWork->scriptSys = work;
        jumpWork->pokeWork = STA_ACT_GetPokeWork( work->actWork , i );
        jumpWork->height = height;
        
        jumpWork->repeatWork.actWork = work->actWork;
        jumpWork->repeatWork.step = 0;
        jumpWork->repeatWork.interval = interval;
        jumpWork->repeatWork.num = num;
        
        jumpWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_PokeAct_Jump , (void*)jumpWork , SCRIPT_TCB_PRI_NORMAL );
      }
      
      //カメラ中心計算用処理
      posXSum += movePoke->moveWork.end.x;

    }
    //カメラスクロール
    {
      MOVE_WORK_S32 *moveWork;
      moveWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_WORK_S32 ));
      moveWork->scriptSys = work;
      moveWork->actWork = work->actWork;
      moveWork->step = 0;
      moveWork->start = STA_ACT_GetStageScroll( work->actWork );
      moveWork->end = FX_FX32_TO_F32( posXSum/4 )-128;
      moveWork->frame = frame;
    
      moveWork->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveStageTCB , (void*)moveWork , SCRIPT_TCB_PRI_NORMAL );
      STA_ACT_SetForceScroll( work->actWork , TRUE );
    }
  }
  if( autoWait == 1 )
  {
    scriptWork->waitCnt = frame;
    SCRIPT_TPrintf("   AutoWait[%d]\n",scriptWork->waitCnt );
    return SFT_SUSPEND;
  }
  
  return SFT_CONTINUE;
}

#pragma mark [>PokemonManage
//ポケモン動作番号予約
SCRIPT_FUNC_DEF( PokeMngSetFlag )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  
  const s32 flg1 = ScriptFunc_GetValueS32();
  const s32 flg2 = ScriptFunc_GetValueS32();
  const s32 flg3 = ScriptFunc_GetValueS32();
  const s32 flg4 = ScriptFunc_GetValueS32();
  
  SCRIPT_PRINT_LABEL( PokeMngSetFlag );
  
  scriptWork->trgPokeFlg = 0;
  
  if( flg1 == TRUE )
  {
    scriptWork->trgPokeFlg += 1;
  }
  if( flg2 == TRUE )
  {
    scriptWork->trgPokeFlg += 2;
  }
  if( flg3 == TRUE )
  {
    scriptWork->trgPokeFlg += 4;
  }
  if( flg4 == TRUE )
  {
    scriptWork->trgPokeFlg += 8;
  }
  
  return SFT_CONTINUE;

}

//------------------------------------------------------------------
//オブジェクト系
//------------------------------------------------------------------
#pragma mark [>Object

//オブジェクト作成
SCRIPT_FUNC_DEF( ObjectCreate )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 objNo = ScriptFunc_GetValueS32();
  const s32 objType = ScriptFunc_GetValueS32();
  
  STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork;
  SCRIPT_PRINT_LABEL(ObjectCreate);

  objWork = STA_OBJ_CreateObject( objSys , objType );

  STA_ACT_SetObjectWork( work->actWork , objWork , objNo );

  return SFT_CONTINUE;
}

//オブジェクト破棄
SCRIPT_FUNC_DEF( ObjectDelete )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 objNo = ScriptFunc_GetValueS32();
  
  STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
  SCRIPT_PRINT_LABEL(ObjectDelete);
  
  STA_OBJ_DeleteObject( objSys , objWork );
  STA_ACT_SetObjectWork( work->actWork , NULL , objNo );

  return SFT_CONTINUE;
}

//オブジェクト表示
SCRIPT_FUNC_DEF( ObjectShow )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 objNo = ScriptFunc_GetValueS32();
  
  STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
  SCRIPT_PRINT_LABEL(ObjectShow);
  
  STA_OBJ_SetShowFlg( objSys , objWork , TRUE );
  
  return SFT_CONTINUE;
}

//オブジェクト非表示
SCRIPT_FUNC_DEF( ObjectHide )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 objNo = ScriptFunc_GetValueS32();
  
  STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
  SCRIPT_PRINT_LABEL(ObjectHide);
  
  STA_OBJ_SetShowFlg( objSys , objWork , FALSE );
  
  return SFT_CONTINUE;
}

//オブジェクト移動
SCRIPT_FUNC_DEF( ObjectMove )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 objNo = ScriptFunc_GetValueS32();
  const s32 frame = ScriptFunc_GetValueS32();
  const fx32 posX = ScriptFunc_GetValuefx32();
  const fx32 posY = ScriptFunc_GetValuefx32();
  const fx32 posZ = ScriptFunc_GetValuefx32();
  
  STA_OBJ_SYS  *objSys = STA_ACT_GetObjectSys( work->actWork );
  STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , objNo );
  SCRIPT_PRINT_LABEL(ObjectMove);

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
    moveObj->scriptSys = work;
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

    moveObj->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveObjTCB , (void*)moveObj , SCRIPT_TCB_PRI_NORMAL );
  }

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
    STA_SCRIPT_DeleteTcbTask( moveObj->scriptSys , moveObj->tcbObj );
  }
}

//------------------------------------------------------------------
//  エフェクト
//------------------------------------------------------------------
#pragma mark [>Effect
//エフェクト作成
SCRIPT_FUNC_DEF( EffectCreate )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 effectNo = ScriptFunc_GetValueS32();
  const s32 fileNo = ScriptFunc_GetValueS32();
  
  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  STA_EFF_WORK *effWork = STA_EFF_CreateEffect( effSys , NARC_stage_gra_mus_eff_00_spa + fileNo );
  SCRIPT_PRINT_LABEL(EffectCreate);

  STA_ACT_SetEffectWork( work->actWork , effWork , effectNo );

  return SFT_CONTINUE;
}

//エフェクト破棄
SCRIPT_FUNC_DEF( EffectDelete )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 effectNo = ScriptFunc_GetValueS32();
  
  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  STA_EFF_WORK *effWork = STA_ACT_GetEffectWork( work->actWork , effectNo );
  SCRIPT_PRINT_LABEL(EffectDelete);
  
  STA_EFF_DeleteEffect( effSys , effWork );
  STA_ACT_SetEffectWork( work->actWork , NULL , effectNo );

  return SFT_CONTINUE;
}

//エフェクト再生
SCRIPT_FUNC_DEF( EffectStart )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 effectNo = ScriptFunc_GetValueS32();
  const s32 emiterNo = ScriptFunc_GetValueS32();
  const fx32 posX = ScriptFunc_GetValuefx32();
  const fx32 posY = ScriptFunc_GetValuefx32();
  const fx32 posZ = ScriptFunc_GetValuefx32();
  VecFx32 pos;

//  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  STA_EFF_WORK *effWork = STA_ACT_GetEffectWork( work->actWork , effectNo );
  SCRIPT_PRINT_LABEL(EffectStart);

  VEC_Set( &pos , ACT_POS_X_FX(posX),ACT_POS_Y_FX(posY),posZ );
  STA_EFF_CreateEmitter( effWork , emiterNo , &pos );

  return SFT_CONTINUE;
}

//エフェクト停止
SCRIPT_FUNC_DEF( EffectStop )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 effectNo = ScriptFunc_GetValueS32();
  const s32 emiterNo = ScriptFunc_GetValueS32();

//  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  STA_EFF_WORK *effWork = STA_ACT_GetEffectWork( work->actWork , effectNo );
  SCRIPT_PRINT_LABEL(EffectStop);

  STA_EFF_DeleteEmitter( effWork , emiterNo );

  return SFT_CONTINUE;
}

//エフェクト連続再生
typedef struct
{
  REPEAT_MNG_WORK repeatWork;
  STA_EFF_WORK  *effWork;
  VecFx32 pos;
  VecFx32 range;
  u16 emitNo;

  STA_SCRIPT_SYS *scriptSys;
  STA_SCRIPT_TCB_OBJECT *tcbObj;
}EFFECT_REPEAT_WORK;

SCRIPT_FUNC_DEF( EffectRepeatStart )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 effectNo = ScriptFunc_GetValueS32();
  const s32 emiterNo = ScriptFunc_GetValueS32();
  const s32 interval = ScriptFunc_GetValueS32();
  const s32 num = ScriptFunc_GetValuefx32();
  const fx32 posXStart = ScriptFunc_GetValuefx32();
  const fx32 posYStart = ScriptFunc_GetValuefx32();
  const fx32 posZStart = ScriptFunc_GetValuefx32();
  const fx32 posXEnd = ScriptFunc_GetValuefx32();
  const fx32 posYEnd = ScriptFunc_GetValuefx32();
  const fx32 posZEnd = ScriptFunc_GetValuefx32();
  
//  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  STA_EFF_WORK *effWork = STA_ACT_GetEffectWork( work->actWork , effectNo );
  
  EFFECT_REPEAT_WORK *effRepeat = GFL_HEAP_AllocMemory( work->heapId , sizeof(EFFECT_REPEAT_WORK));
  SCRIPT_PRINT_LABEL(EffectRepeatStart);
  
  GF_ASSERT( posXStart <= posXEnd );
  GF_ASSERT( posYStart <= posYEnd );
  GF_ASSERT( posZStart <= posZEnd );

  effRepeat->scriptSys = work;
  effRepeat->effWork = effWork;
  effRepeat->emitNo = emiterNo;
  VEC_Set( &effRepeat->pos , posXStart,posYStart,posZStart );
  VEC_Set( &effRepeat->range , posXEnd-posXStart,posYEnd-posYStart,posZEnd-posZStart );
  
  effRepeat->repeatWork.actWork = work->actWork;
  effRepeat->repeatWork.step = 0;
  effRepeat->repeatWork.interval = interval;
  effRepeat->repeatWork.num = num;

  effRepeat->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_EffectRepeat , (void*)effRepeat , SCRIPT_TCB_PRI_NORMAL );

  return SFT_CONTINUE;
}
//エフェクト連続再生制御
static void SCRIPT_TCB_EffectRepeat(  GFL_TCB *tcb, void *work )
{
  EFFECT_REPEAT_WORK *effRepeat = (EFFECT_REPEAT_WORK*)work;
//  STA_EFF_SYS  *effSys = STA_ACT_GetEffectSys( work->actWork );
  
  const REPEAT_MNG_RETURN ret = SCRIPT_TCB_UpdateRepeat( &effRepeat->repeatWork );
  if( ret == RMR_END )
  {
    STA_SCRIPT_DeleteTcbTask( effRepeat->scriptSys , effRepeat->tcbObj );
    return;
  }
  else if( ret == RMR_ACTION )
  {
    VecFx32 pos;
    pos.x = ACT_POS_X_FX(effRepeat->pos.x + GFUser_GetPublicRand0( effRepeat->range.x ));
    pos.y = ACT_POS_Y_FX(effRepeat->pos.y + GFUser_GetPublicRand0( effRepeat->range.y ));
    pos.z = effRepeat->pos.z + GFUser_GetPublicRand0( effRepeat->range.z );

    STA_EFF_CreateEmitter( effRepeat->effWork , effRepeat->emitNo , &pos );
  }
}

//------------------------------------------------------------------
//  ライト
//------------------------------------------------------------------
#pragma mark [>SpotLight

//ライトON(円形
SCRIPT_FUNC_DEF( LightShowCircle )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 lightNo = ScriptFunc_GetValueS32();
  const fx32 rad = ScriptFunc_GetValueS32();
  
  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork;
  SCRIPT_PRINT_LABEL(LightShowCircle);
  
  lightWork = STA_LIGHT_CreateObject( lightSys , ALT_CIRCLE );
  STA_LIGHT_SetColor( lightSys , lightWork , GX_RGB(31,31,0) , 16 );
  STA_LIGHT_SetOptionValue( lightSys , lightWork , rad , 0 );
  
  STA_ACT_SetLightWork( work->actWork , lightWork , lightNo );
  
  return SFT_CONTINUE;
}

//ライトOFF
SCRIPT_FUNC_DEF( LightHide )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 lightNo = ScriptFunc_GetValueS32();

  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );
  SCRIPT_PRINT_LABEL(LightHide);
  
  STA_LIGHT_DeleteObject( lightSys , lightWork );
  STA_ACT_SetLightWork( work->actWork , NULL , lightNo );

  return SFT_CONTINUE;
}

//ライト移動
SCRIPT_FUNC_DEF( LightMove )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 lightNo = ScriptFunc_GetValueS32();
  const s32 frame = ScriptFunc_GetValueS32();
  const fx32 posX = ScriptFunc_GetValuefx32();
  const fx32 posY = ScriptFunc_GetValuefx32();
  //現在Zは使えない
  const fx32 posZ = ScriptFunc_GetValuefx32();
  
  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );
  SCRIPT_PRINT_LABEL(LightMove);

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
    moveLight->scriptSys = work;
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

    moveLight->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveLightTCB , (void*)moveLight , SCRIPT_TCB_PRI_NORMAL );
  }
  
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
    STA_SCRIPT_DeleteTcbTask( moveLight->scriptSys , moveLight->tcbObj );
  }
}

//ライト追従移動
SCRIPT_FUNC_DEF( LightMoveTrace )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 lightNo = ScriptFunc_GetValueS32();
  const s32 pokeNo = ScriptFunc_GetValueS32();
  const s32 frame = ScriptFunc_GetValueS32();
  const fx32 ofsX = ScriptFunc_GetValuefx32();
  const fx32 ofsY = ScriptFunc_GetValuefx32();
  //現在Zは使えない
  const fx32 ofsZ = ScriptFunc_GetValuefx32();
  
  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );
  STA_POKE_SYS  *pokeSys = STA_ACT_GetPokeSys( work->actWork );
  STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , (u8)pokeNo );
  SCRIPT_PRINT_LABEL(LightMoveTrace);
  
  if( frame == 0 )
  {
    VecFx32 pos,ofs;
    STA_POKE_GetPosition( pokeSys , pokeWork , &pos );
    VEC_Set( &ofs , ofsX,ofsY,ofsZ );
    VEC_Add( &pos,&ofs,&pos );
    STA_LIGHT_SetPosition( lightSys , lightWork , &pos );
  }
  else
  {
    STA_AUDI_SYS *audiSys = STA_ACT_GetAudienceSys( work->actWork );
    MOVE_TRACE_LIGHT_VEC *moveTraceLight;
    moveTraceLight = GFL_HEAP_AllocMemory( work->heapId , sizeof( MOVE_TRACE_LIGHT_VEC ));
    moveTraceLight->scriptSys = work;
    moveTraceLight->trgPokeNo = pokeNo;
    moveTraceLight->lightWork = lightWork;
    moveTraceLight->moveWork.actWork = work->actWork;
    moveTraceLight->moveWork.pokeWork = pokeWork;
    moveTraceLight->moveWork.step = 0;
    VEC_Set( &moveTraceLight->moveWork.ofs , ofsX,ofsY,ofsZ );
    moveTraceLight->moveWork.frame = frame;
    
    moveTraceLight->tcbObj = STA_SCRIPT_CreateTcbTask( work , SCRIPT_TCB_MoveTraceLightTCB , (void*)moveTraceLight , SCRIPT_TCB_PRI_LOW );
  }
  
  return SFT_CONTINUE;
}

//ライト追従移動制御TCB
static void SCRIPT_TCB_MoveTraceLightTCB(  GFL_TCB *tcb, void *work )
{
  MOVE_TRACE_LIGHT_VEC *moveLight = (MOVE_TRACE_LIGHT_VEC*)work;
  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( moveLight->moveWork.actWork );
  STA_AUDI_SYS *audiSys = STA_ACT_GetAudienceSys( moveLight->moveWork.actWork );
  VecFx32 newPos;
  
  const BOOL isFinish = SCRIPT_TCB_UpdatePokeTrace( &moveLight->moveWork , &newPos );
  STA_LIGHT_SetPosition( lightSys , moveLight->lightWork , &newPos );
  
  if( isFinish == TRUE )
  {
    STA_SCRIPT_DeleteTcbTask( moveLight->scriptSys , moveLight->tcbObj );
  }
}

//ライト色設定
SCRIPT_FUNC_DEF( LightColor )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 lightNo = ScriptFunc_GetValueS32();
  const s32 colR = ScriptFunc_GetValueS32();
  const s32 colG = ScriptFunc_GetValueS32();
  const s32 colB = ScriptFunc_GetValueS32();
  const s32 alpha = ScriptFunc_GetValuefx32();
  const GXRgb col = GX_RGB( (u8)colR , (u8)colG , (u8)colB );
  
  STA_LIGHT_SYS  *lightSys = STA_ACT_GetLightSys( work->actWork );
  STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , lightNo );
  SCRIPT_PRINT_LABEL(LightColor);

  STA_LIGHT_SetColor( lightSys , lightWork , col , alpha );
  
  return SFT_CONTINUE;
}

//------------------------------------------------------------------
//  メッセージ
//------------------------------------------------------------------
#pragma mark [>Message

//メッセージ表示
SCRIPT_FUNC_DEF( MessageShow )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 msgNo = ScriptFunc_GetValueS32();
  const s32 dispSpd = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(MessageShow);

  STA_ACT_ShowMessage( work->actWork , msgNo , dispSpd );
  return SFT_CONTINUE;
}

//メッセージ消去
SCRIPT_FUNC_DEF( MessageHide )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  SCRIPT_PRINT_LABEL(MessageHide);
  
  STA_ACT_HideMessage( work->actWork );
  return SFT_CONTINUE;
}

//メッセージ消去
SCRIPT_FUNC_DEF( MessageColor )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 msgCol = ScriptFunc_GetValueS32();
  const s32 shadowCol = ScriptFunc_GetValueS32();
  const s32 backNo = ScriptFunc_GetValueS32();
  SCRIPT_PRINT_LABEL(MessageColor);
  GFL_FONTSYS_SetColor( msgCol,shadowCol,backNo );

  return SFT_CONTINUE;
}

//------------------------------------------------------------------
//  音楽
//------------------------------------------------------------------
#pragma mark [>Bgm
SCRIPT_FUNC_DEF( BgmStart )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 bgmNo = ScriptFunc_GetValueS32();
  
  SCRIPT_PRINT_LABEL(BgmStart);

  STA_ACT_StartBgm( work->actWork );

  return SFT_CONTINUE;
}

SCRIPT_FUNC_DEF( BgmStop )
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  
  SCRIPT_PRINT_LABEL(BgmStop);
  STA_ACT_StopBgm( work->actWork );
  

  return SFT_CONTINUE;
}

SCRIPT_FUNC_DEF( SeqBgmStart )  //SEQ_BGM開始
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  const s32 bgmNo = ScriptFunc_GetValueS32();

  PMSND_PlayBGM( bgmNo );
  

  return SFT_CONTINUE;
}

SCRIPT_FUNC_DEF( SeqBgmStop )   //SEQ_BGM開始
{
  STA_SCRIPT_WORK *scriptWork = (STA_SCRIPT_WORK*)context_work;
  STA_SCRIPT_SYS *work = scriptWork->sysWork;
  
  PMSND_StopBGM( );
  return SFT_CONTINUE;
}

#undef SCRIPT_FUNC_DEF
