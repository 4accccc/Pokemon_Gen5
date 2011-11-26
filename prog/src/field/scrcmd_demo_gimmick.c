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

// arc
#include "arc/message.naix" 

// mine
#include "scrcmd_demo_gimmick.h"
#include "field_gimmick_gianthole.h"
#include "field_gimmick_c12.h"

// debug
#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

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

  // 
  GIANTHOLE_GIMMICK_N_POKE_AnmStart( gsys );

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

  int obj_id, offset_x, offset_z;
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

    offset.x = offset_x << FX32_SHIFT;
    offset.z = offset_z << FX32_SHIFT;

    MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
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

