/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file	 scrcmd_league_front.c
 * @brief	 スクリプトコマンド：ポケモンリーグ フロント関連
 * @date   2009.12.14
 * @author obata
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "system/vm_cmd.h"
#include "scrcmd_league_front.h"
#include "field_gimmick_league_front01.h"
#include "scrcmd_work.h"
#include "script_local.h"
#include "scrcmd.h"


//---------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを開始する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdLeagueFrontLiftDown( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*     script = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys ); 
  GMEVENT* event;

  // イベント呼び出し
  event = LEAGUE_FRONT_01_GIMMICK_GetLiftDownEvent( gsys, fieldmap );
  SCRIPT_CallEvent( script, event );
  return VMCMD_RESULT_SUSPEND;
} 
