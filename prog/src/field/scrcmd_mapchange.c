//======================================================================
/**
 * @file  scrcmd_mapchange.c
 * @bfief  スクリプトコマンド：アイテム関連
 * @date  2009.09.11
 * @author	tamada GAMEFREAK inc.
 */
//======================================================================
#include <gflib.h>
//#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
//#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_mapchange.h"

#include "event_mapchange.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//  アイテム関連
//======================================================================
//--------------------------------------------------------------
/**
 * マップ遷移（流砂）
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMapChangeBySandStream( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  u16 zone_id = VMGetU16(core);
  u16 next_x = VMGetU16(core);
  u16 next_z = VMGetU16(core);
  u16 dir;
  
  {
    GAMESYS_WORK * gsys = SCRIPT_GetGameSysWork( sc );
    FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    GMEVENT * parent_event = GAMESYSTEM_GetEvent( gsys );
    GMEVENT * mapchange_event;
    VecFx32 disappear_pos;
    VecFx32 appear_pos;

    
    mapchange_event = DEBUG_EVENT_ChangeMapBySandStream( gsys, fieldmap,
        &disappear_pos, zone_id, &appear_pos, dir );
    GMEVENT_CallEvent( parent_event, mapchange_event );
  }

  return VMCMD_RESULT_CONTINUE;
}

