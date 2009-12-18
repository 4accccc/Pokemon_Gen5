//======================================================================
/**
 * @file scr_musical.c
 * @brief スクリプトコマンド　ミュージカル関連
 * @author	Satoshi Nohara
 * @date	06.06.26
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_palpark.h"


#include "../../../resource/fldmapdata/script/palpark_scr_local.h"
#include "savedata/save_control.h"
#include "event_fieldmap_control.h"

#include "multiboot/mb_parent_sys.h"

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
//  スクリプトコマンド　ミュージカル
//======================================================================
//--------------------------------------------------------------
/**
 * パルパーク：パルパーク呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPalparkCall( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  MB_PARENT_INIT_WORK *param;
  
  param = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(MB_PARENT_INIT_WORK) ); 
  param->gameData    = gdata;

  event = EVENT_FieldSubProc( gsys, fieldmap, 
                              FS_OVERLAY_ID(multiboot), &MultiBoot_ProcData, param );

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * パルパーク：パルパーク系数値取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPalparkValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;

  u8   type   = VMGetU8( core );
  u16* ret_wk = SCRCMD_GetVMWork( core, work );

  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  //SAVE_CONTROL_WORK *svWork = GAMEDATA_GetSaveControlWork( gdata );
  
  switch(type)
  {
  case 0:
    OS_TPrintf("パルパーク数値取得 終了ステート\n");
    *ret_wk = 0;
    //PALPARK_FINISH_NORMAL    (0)  //捕獲した
    //PALPARK_FINISH_HIGHSOCRE (1)  //捕獲した＋ハイスコア
    //PALPARK_FINISH_NO_GET    (2)  //捕獲できなかった
    //PALPARK_FINISH_ERROR     (3)  //エラー終了

    break;
  case 1:
    OS_TPrintf("パルパーク数値取得 ハイスコア\n");
    *ret_wk = 100;
    break;
  }
  //仮処理

  return VMCMD_RESULT_CONTINUE;
}

