//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_c03center.c
 *	@brief  スクリプトコマンド　C03中央　の出入り口イベント
 *	@author	tomoya takahshi
 *	@date		2010.01.13
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "scrcmd_c03center.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	管理処理のシーケンス
//=====================================
enum {
  SEQ_MAIN,
} ;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	自機方向管理処理のワーク
//=====================================
typedef struct {
 
  s32 start_rot;
  s32 div_rot;

  s32 frame;
  s32 frame_max;
  
} PLAYERWAY_CONTORL_WK;


// MMDLSYS_SetUpに渡す角度
static u16 s_ControlPlayerWay = 0;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void ControlPlayerWayTCB( GFL_TCB* p_tcb, void* p_work );

//----------------------------------------------------------------------------
/**
 *	@brief  C03センター自機方向の調整システム
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdC03Center_ControlPlayerWay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  GFL_TCBSYS* tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldWork );
  PLAYERWAY_CONTORL_WK* local_wk;
  HEAPID heapID = FIELDMAP_GetHeapID( fieldWork );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( fieldWork );
  MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
  const u16* now_angle;
  FLD_CAM_MV_PARAM_CORE* camera_move;
  u16          frame = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  u32 amari;

  // ワーク生成
  local_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(PLAYERWAY_CONTORL_WK) );

  now_angle = FIELD_CAMERA_GetAngleYawAddress( camera );
  s_ControlPlayerWay = *now_angle;

  // 調整TCBをコールし、FLDMMDLに渡しているYawの値を調整
  MMDLSYS_SetupDrawProc( mmdlsys, &s_ControlPlayerWay );

  OS_TPrintf( "MMDL_SYSに設定する DrawAngleをEvCmdC03Center_ControlPlayerWayで変更\n" );

  // 移動させるアングルを求める
  camera_move = FIELD_CAMERA_GetMoveDstPrmPtr( camera );

  // 
  local_wk->start_rot = s_ControlPlayerWay;
  local_wk->div_rot   = camera_move->AngleYaw;

  // div_rotを90度単位にする
  amari               = local_wk->div_rot % 0x4000;
  if( amari >= 0x2000 ){ 
    local_wk->div_rot += 0x4000 - amari;
  }else{
    local_wk->div_rot -= amari;
  }

  // 回転動作値を求める
  local_wk->div_rot   = local_wk->div_rot - local_wk->start_rot;
  if( MATH_ABS( local_wk->div_rot ) >= 0x8000 ) // 180度以上なら逆回転
  {
    if( local_wk->div_rot > 0 ){
      local_wk->div_rot = local_wk->div_rot - 0x10000;
    }else{
      local_wk->div_rot = local_wk->div_rot + 0x10000;
    }
  }
  local_wk->frame     = 0;
  local_wk->frame_max = frame;


  // TCB起動
  GFL_TCB_AddTask( tcbsys, ControlPlayerWayTCB, local_wk, 0 );

  return VMCMD_RESULT_CONTINUE;
}






//-----------------------------------------------------------------------------
/**
 *  private関数郡
 */
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief    自機の方向を管理　（最終的に、前を向くように管理）
 */
//-----------------------------------------------------------------------------
static void ControlPlayerWayTCB( GFL_TCB* p_tcb, void* p_work )
{
  PLAYERWAY_CONTORL_WK* p_wk = p_work;

  p_wk->frame ++;

  s_ControlPlayerWay = (p_wk->div_rot * p_wk->frame) / p_wk->frame_max;
  s_ControlPlayerWay += p_wk->start_rot;
  
  if( p_wk->frame >= p_wk->frame_max )
  {
    GFL_HEAP_FreeMemory( p_work );
    // 自滅
    GFL_TCB_DeleteTask( p_tcb );
  }
}




