//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_ochiba.c
 *	@brief  スクリプトコマンド 落ち葉
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "scrcmd_ochiba.h"

// フィールドエフェクトと同じリソースを使用
#include "arc/fieldmap/fldeff.naix"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FALL_HEIGHT       (10 * FX32_ONE)   // 初期位置から下がる高さ
#define FALL_ADD          ( 5 * FX32_ONE )
#define WAIT_FRAME_MAX    (40)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	イベント内エフェクトワーク
//=====================================
typedef struct {
  FLD_G3DOBJ_CTRL     *obj_ctrl;
  FLD_G3DOBJ_RESIDX   res_idx;
  FLD_G3DOBJ_OBJIDX   obj_idx;
} EVENT_EFFECT_WORK;

//-------------------------------------
///	落下落ち葉イベントワーク
//=====================================
typedef struct {
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  FIELD_PLAYER        *p_player;      // 自機

  EVENT_EFFECT_WORK   effect_wk;      // エフェクトワーク

  u16                 wait_frame;     // 待ちフレーム

  fx32                now_height;     // 現在
  fx32                dst_height;     // 移動後
  fx32                add_val;        // 加算値
} OCHIBA_EVENT_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// 落下落ち葉
static GMEVENT * OCHIBA_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT OchibaEvent( GMEVENT* event, int* seq, void* wk );

// EFFECTタスク追加
static void EFFECT_TASK_Add( OCHIBA_EVENT_WORK *p_wk );
// task関数
static void tcb_EffectAnime( GFL_TCB *tcb, void* work );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  落下落ち葉コマンド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdOchiba_Fall( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = OCHIBA_CreateEvent( NULL, gsys );
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  落下落ち葉イベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステム
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * OCHIBA_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*            event;
  OCHIBA_EVENT_WORK   *work;
  FIELDMAP_WORK       *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER        *p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  VecFx32 pos;

  FIELD_PLAYER_GetPos( p_player, &pos );

  // イベント作成
  event = GMEVENT_Create( gsys, parent, OchibaEvent, sizeof(OCHIBA_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->p_fieldmap= p_fieldmap;
  work->p_player  = p_player;

  work->wait_frame = 0;
  work->now_height  = pos.y;
  work->dst_height  = pos.y - FALL_HEIGHT;
  work->add_val     = FALL_ADD;

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  落下落ち葉イベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT OchibaEvent( GMEVENT* event, int* seq, void* wk )
{
  OCHIBA_EVENT_WORK *p_wk = (OCHIBA_EVENT_WORK*)wk;

  // 処理シーケンス
  enum{
    SEQ_START,        // イベント開始
    SEQ_CYCLE_CHANGE, // 自転車変更
    SEQ_EFFECT_ADD,   // エフェクト追加 
    SEQ_FALL,         // 落ちる
    SEQ_WAIT,         // 待ち時間
    SEQ_JUMP,         // ジャンプ
    SEQ_JUMP_WAIT,    // ジャンプ待ち
    SEQ_END,          // イベント終了
  };

  switch( *seq ){
  // イベント開始
  case SEQ_START:
    {
      PLAYER_MOVE_FORM form = FIELD_PLAYER_GetDrawForm( p_wk->p_player );

      // 自機が二足歩行(自転車じゃない)かチェック
      if( form == PLAYER_MOVE_FORM_NORMAL )
      {
        *seq = SEQ_EFFECT_ADD;
      }
      else
      {
        *seq = SEQ_CYCLE_CHANGE;
      }
    }
    break;

  // 自転車から歩行に強制変更
  case SEQ_CYCLE_CHANGE:
    {
      // リクエストを出す
      FIELD_PLAYER_SetRequest( p_wk->p_player, FIELD_PLAYER_REQBIT_NORMAL );
      // リクエストを強制更新
      FIELD_PLAYER_ForceUpdateRequest( p_wk->p_player );

      *seq = SEQ_EFFECT_ADD;
    }
    break;

  // エフェクト追加 
  case SEQ_EFFECT_ADD:
    EFFECT_TASK_Add( p_wk );
    *seq = SEQ_FALL;
    /* break throw */

  // 落ちる
  case SEQ_FALL:
    {
      // 高さ減算
      p_wk->now_height -= p_wk->add_val;

      // 高さチェック
      if( p_wk->now_height <= p_wk->dst_height )
      {
        // 目標高さで上書き
        p_wk->now_height = p_wk->dst_height;

        *seq = SEQ_WAIT;
      }

      // 自機の高さ更新
      {
        VecFx32 pos;
        FIELD_PLAYER_GetPos( p_wk->p_player, &pos );
        pos.y = p_wk->now_height;
        FIELD_PLAYER_SetPos( p_wk->p_player, &pos );
      }
    }
    break;

  // 待ち時間
  case SEQ_WAIT:
    {
      // 自機の向きを変える
      { 
        MMDL *mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );
        int key = GFL_UI_KEY_GetCont();
        if( key & PAD_KEY_UP )    { MMDL_SetAcmd( mmdl, AC_STAY_WALK_U_4F ); }
        if( key & PAD_KEY_DOWN )  { MMDL_SetAcmd( mmdl, AC_STAY_WALK_D_4F ); }
        if( key & PAD_KEY_LEFT )  { MMDL_SetAcmd( mmdl, AC_STAY_WALK_L_4F ); }
        if( key & PAD_KEY_RIGHT ) { MMDL_SetAcmd( mmdl, AC_STAY_WALK_R_4F ); }
      }

      if( p_wk->wait_frame++ > WAIT_FRAME_MAX )
      {
        *seq = SEQ_JUMP;
      }
    }
    break;

  // ジャンプ
  case SEQ_JUMP:
    {
      u16   dir   = FIELD_PLAYER_GetDir( p_wk->p_player );
      MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );
      MMDL_SetAcmd( mmdl, AC_STAY_JUMP_U_8F + dir ); 

      MMDL_UpdateCurrentHeight( mmdl );   // 接地

      *seq = SEQ_JUMP_WAIT;
    }
    break;

  // ジャンプ待ち
  case SEQ_JUMP_WAIT:
    {
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->p_player) == TRUE )
      {
        *seq = SEQ_END;
      }
    }
    break;

  // イベント終了
  case SEQ_END:
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *        EFFECT
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  エフェクトアニメタスク追加
 *
 *	@param	OCHIBA_EVENT_WORK *p_wk イベントワーク
 */
//-----------------------------------------------------------------------------
static void EFFECT_TASK_Add( OCHIBA_EVENT_WORK *p_wk )
{
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( p_wk->p_fieldmap );
  EVENT_EFFECT_WORK* task_wk = GFL_HEAP_AllocClearMemoryLo( FIELDMAP_GetHeapID(p_wk->p_fieldmap), sizeof( EVENT_EFFECT_WORK ) );

  VecFx32 pos;
  ARCHANDLE *handle;
  FLD_G3DOBJ_RES_HEADER head;
  FLD_G3DOBJ_CTRL *obj_ctrl;

  FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( p_wk->p_fieldmap );

  FIELD_PLAYER_GetPos( p_wk->p_player, &pos );

  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( fectrl );
  handle = FLDEFF_CTRL_GetArcHandleEffect( fectrl );

  FLD_G3DOBJ_RES_HEADER_Init( &head );
  FLD_G3DOBJ_RES_HEADER_SetMdl( &head, handle, NARC_fldeff_ochiba02_nsbmd );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcHandle( &head, handle );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcIdx( &head, NARC_fldeff_ochiba02_nsbca );

  task_wk->obj_ctrl = obj_ctrl;
  task_wk->res_idx = FLD_G3DOBJ_CTRL_CreateResource( obj_ctrl, &head, FALSE );
  task_wk->obj_idx = FLD_G3DOBJ_CTRL_AddObject( obj_ctrl, task_wk->res_idx, 0, &pos );

  PMSND_PlaySE(SEQ_SE_OCHIBA_01);
  GFL_TCB_AddTask( tcbsys, tcb_EffectAnime, (void*)task_wk, 0 );
}

//==================================================
///	アニメ終了監視タスク
//==================================================
static void tcb_EffectAnime( GFL_TCB *tcb, void* work )
{
  EVENT_EFFECT_WORK *p_wk = (EVENT_EFFECT_WORK*)work;

  // アニメ終了していないなら戻る
  if( FLD_G3DOBJ_CTRL_AnimeObject( 
        p_wk->obj_ctrl, p_wk->obj_idx, FX32_ONE ) == TRUE )
  {
    return;
  }

  // リソース破棄
  {
    FLD_G3DOBJ_CTRL_DeleteObject( p_wk->obj_ctrl, p_wk->obj_idx );
    FLD_G3DOBJ_CTRL_DeleteResource( p_wk->obj_ctrl, p_wk->res_idx );
  }

  GFL_HEAP_FreeMemory( work );  // ワークを破棄
  GFL_TCB_DeleteTask( tcb );    // タスクを破棄(自殺)
}

