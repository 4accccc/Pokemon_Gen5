/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file	 scrcmd_gate.c
 * @brief	 スクリプトコマンド：ゲート関連
 * @date   2009.11.05
 * @author obata
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd_gate.h"
#include "field_gimmick_gate.h"
#include "gimmick_obj_elboard.h"
#include "scrcmd_work.h"
#include "script_local.h"
#include "scrcmd.h"
#include "event_look_elboard.h"


#if 0  // ニュースの管理方法を変更し, 不要になったために削除 2009.12.17
//---------------------------------------------------------------------------------------
/**
 * @brief 平常時のニュースをセットする
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdElboard_SetNormalNews( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  GATE_GIMMICK_Elboard_SetupNormalNews( fieldmap );
  return VMCMD_RESULT_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースを追加する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdElboard_AddSpecialNews( VMHANDLE *core, void *wk )
{ 
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u16              str_id = SCRCMD_GetVMWorkValue( core, work );   // コマンド第一引数
  SCRIPT_WORK*   scr_work = SCRCMD_WORK_GetScriptWork( work );
  WORDSET**       wordset = SCRIPT_GetMemberWork( scr_work, ID_EVSCR_WORDSET );

  GATE_GIMMICK_Elboard_AddSpecialNews( fieldmap, str_id, *wordset );
  return VMCMD_RESULT_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief 掲示板を復帰させる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdElboard_Recovery( VMHANDLE *core, void *wk )
{ 
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  GATE_GIMMICK_Elboard_Recovery( fieldmap );
  return VMCMD_RESULT_CONTINUE;
}
#endif


//---------------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を見るイベント呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdLookElboard( VMHANDLE *core, void *wk )
{ 
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*     script = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u16               frame = SCRCMD_GetVMWorkValue( core, work );  // コマンド第一引数
  GMEVENT* event;

  // イベント呼び出し
  event = EVENT_LookElboard( gsys, fieldmap, frame );
  SCRIPT_CallEvent( script, event );
  return VMCMD_RESULT_SUSPEND;
}
