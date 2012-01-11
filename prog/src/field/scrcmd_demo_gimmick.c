//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_demo_gimmick.c
 *	@brief  ギミック関係スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 *	@note   SWANで追加されたデモで使用するギミックの
 *	        スクリプトコマンドを集約しています
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "gflib.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/brightness.h" // for

// script
#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

// field
#include "fieldmap.h"
#include "event_fieldmap_control.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

// arc
#include "arc/message.naix" 

// mine
#include "scrcmd_demo_gimmick.h"
#include "field_gimmick_gianthole.h"
#include "field_gimmick_c12.h"
#include "field_gimmick_ending.h"
#include "legend_gmk.h"

// debug
#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	花火SEフェードアウトイベントワーク
//=====================================
typedef struct {
  int   se_no;
  int   volume;
} SE_FADE_OUT_TASK_WORK;

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// 花火SEフェードアウトタスク関数
static FIELD_TASK_RETVAL SE_FadeOutTask( void *work );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　モデル表示
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_Disp( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  id  = VMGetU16( core );

  GIANTHOLE_GIMMICK_Disp( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　アニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_AnmStart( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  id  = VMGetU16( core );

  GIANTHOLE_GIMMICK_AnmStart( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　ストーンを浮遊させる
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_StoneFloat( VMHANDLE *core, void *wk )
{
  int obj_id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ストーンのOBJID
  obj_id  = VMGetU16( core );

  // ふわふわさせるタスクを作る
  GIANTHOLE_GIMMICK_StoneFloat( gsys, obj_id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　ゼクロム(レシラム)に乗ったNアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_NPokeAnmStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  int param1 = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  BOOL is_down;

  is_down = (param1==1) ? TRUE : FALSE;
  // 
  GIANTHOLE_GIMMICK_N_POKE_AnmStart( gsys, is_down );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　ゼクロム(レシラム)に乗ったNアニメ終了
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_NPokeAnmEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // レシゼクNを非表示
  GIANTHOLE_GIMMICK_N_POKE_AnmEnd( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　OBJのオフセット設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_ObjOffsetSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  int obj_id;
  s16 offset_x;
  s16 offset_z;
  MMDL *mmdl;

  obj_id    = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  offset_x  = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数
  offset_z  = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数

  // 動作モデル取得
  {
    MMDLSYS*  mmdlsys  = SCRCMD_WORK_GetMMdlSys( work );
    mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );  // 
    // 指定されたOBJIDは存在しない
    if( mmdl == NULL ) {
      GF_ASSERT(0);
      return VMCMD_RESULT_CONTINUE;
    }
  }

  // 動作モデルの描画オフセット設定
  {
    VecFx32 offset = {0};

    MMDL_GetVectorPos( mmdl, &offset );

    offset.x += offset_x << FX32_SHIFT;
    offset.z += offset_z << FX32_SHIFT;

    MMDL_SetVectorPos( mmdl, &offset );
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　キュラム・キュロム戦闘後専用処理
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_WildWin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // 通常のWILD_WINでやっていること
  // ・フィールドBGを初期化
  // ・マスター輝度でフェードイン
  //
  // この演出では暗転中にメッセージをだしたいので
  // ・フィールドBGを初期化
  // ・上画面を通常輝度で黒にする
  // ・下画面はマスター輝度で黒にする
  // を処理する
  //

  // フィールドBG初期化
  {
    int mv = GFL_DISP_GetMainVisible();
    int	sv = GFL_DISP_GetSubVisible();
    FIELDMAP_InitBGMode();
    GFL_DISP_GX_SetVisibleControlDirect( mv );
    GFL_DISP_GXS_SetVisibleControlDirect( sv );

    FIELDMAP_InitBG( fieldmap );
  }

  // 輝度設定
  {
    // 上画面
    GX_SetMasterBrightness( 0 );
		G2_SetBlendBrightness( (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD), -16 );
    // 下画面
    GXS_SetMasterBrightness( 0 );
		G2S_SetBlendBrightness( (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD), -16 );
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  物語開始デモ  花びらアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdC12_AnmStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  C12_GIMMICK_AnmStart( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エンディング  花火アニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdEnding_FireWorks( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  ENDING_GIMMICK_FireWorks( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エンディング  花火アニメSE終了
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdEnding_FireWorksSeEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = FIELDMAP_GetHeapID( p_fieldmap );
  FIELD_TASK      *task;
  FIELD_TASK_MAN  *taskMan;
	SE_FADE_OUT_TASK_WORK *p_wk;

  int se_no = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  taskMan = FIELDMAP_GetTaskManager( p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heapID, sizeof(SE_FADE_OUT_TASK_WORK), SE_FadeOutTask );

  // ワーク初期化
	p_wk = FIELD_TASK_GetWork( task );
  p_wk->se_no = se_no;
  p_wk->volume = 127;

  // タスク追加
  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );

  return VMCMD_RESULT_CONTINUE;
}

// 花火SEフェードアウトタスク関数
static FIELD_TASK_RETVAL SE_FadeOutTask( void *work )
{
  SE_FADE_OUT_TASK_WORK *p_wk = work;

  p_wk->volume -= 2; // 音量減算

  if( p_wk->volume < 0 )
  {
    PMSND_StopSE_byPlayerID( PMSND_GetSE_DefaultPlayerID( p_wk->se_no ) );// 停止
    return FIELD_TASK_RETVAL_FINISH;
  }
  else
  {
    PMSND_PlayerSetVolume( PMSND_GetSE_DefaultPlayerID( p_wk->se_no ), p_wk->volume );
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エンディング  C12初期化
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdEnding_C12Init( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  C12_GIMMICK_Init( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ギミック開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdStartLegGmk( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = LEGEND_GMK_Start( gsys );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdLegMoveBall( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  u16 target_obj = SCRCMD_GetVMWorkValue( core, work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );
  u16 height = SCRCMD_GetVMWorkValue( core, work );
  u16 sync = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 start, end;
  VecFx32 *dst;
  VecFx32 *src;
  fx32 fx_height;

  //開始地点算出
  {
    MMDLSYS *fmmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
    MMDL *target = MMDLSYS_SearchOBJID( fmmdlsys, target_obj );
    GF_ASSERT( target != NULL );
    if (target != NULL)
    {
      MMDL_GetVectorPos( target, &start );
    }
  }

  //目的座標セット
  end.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  end.y = y * GRID_HALF_SIZE;
  end.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  src = &start;
  dst = &end;

  fx_height = height * GRID_HALF_SIZE;

  event = LEGEND_GMK_MoveBall( gsys, src, dst, fx_height, sync);

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdLegStartBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 pos;
  pos.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  pos.y = y * GRID_HALF_SIZE;
  pos.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  LEGEND_GMK_StartBallAnm( gsys, &pos );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdLegWaitBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = LEGEND_GMK_WaitBallAnmEnd( gsys );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * ポケモン表示、非表示していいフレームまで待つ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdLegWaitBallPokeApp( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = LEGEND_GMK_WaitPokeAppear( gsys );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

