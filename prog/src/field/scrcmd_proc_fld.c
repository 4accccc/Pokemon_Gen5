//======================================================================
/**
 * @file    scrcmd_proc_fld.c
 * @brief   スクリプトコマンド用関数　各種アプリ呼び出し系(フィールド常駐)
 * @author  Miyuki Iwasawa 
 * @date    09.10.23
 *
 * フェードやフィールドプロセスコントロールを自前で行うプロセス呼び出し法を取り
 * プロセス呼び出しコマンド自体が常駐している必要があるときは
 * scrcmd_proc.cにコマンドを置いてください
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "msgdata.h"
#include "print/wordset.h"

#include "sound/pm_sndsys.h"
//#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field_sound.h"
#include "field_player.h"
#include "event_fieldmap_control.h"

#include "scrcmd_proc.h"
#include "scrcmd_proc_fld.h"
//#include "../../../resource/fldmapdata/script/usescript.h"

#include "event_gameclear.h"  //EVENT_GameClear
#include "app/box2.h"
#include "savedata/battle_box_save.h"

// ボックスプロセスデータとコールバック関数
static void callback_BoxProc( void* work );

//--------------------------------------------------------------
/**
 * @brief ボックスプロセス終了後に呼ばれるコールバック関数
 */
//--------------------------------------------------------------
typedef struct
{
  GAMEDATA* gdata;
  BOX2_GFL_PROC_PARAM* box_param;

} BOX_CALLBACK_WORK;
//--------------------------------------------------------------
static void callback_BoxProc( void* work )
{
  BOX_CALLBACK_WORK* cw = (BOX_CALLBACK_WORK*)work;

  // バトルボックスのセーブデータに反映
  if( cw->box_param->callMode == BOX_MODE_BATTLE )
  {
    SAVE_CONTROL_WORK*      sv = GAMEDATA_GetSaveControlWork( cw->gdata );
    BATTLE_BOX_SAVE* bbox_save = BATTLE_BOX_SAVE_GetBattleBoxSave( sv );
    BATTLE_BOX_SAVE_SetPokeParty( bbox_save, cw->box_param->pokeparty );
  }

  // 後始末
  GFL_HEAP_FreeMemory( cw->box_param );
}

//--------------------------------------------------------------
/**
 * @brief   ボックスプロセスを呼び出します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallBoxProc( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*              work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*               scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*             gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*                gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*          sv = GAMEDATA_GetSaveControlWork( gdata );
  FIELDMAP_WORK*        fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u16                   box_mode = SCRCMD_GetVMWorkValue( core, work );  // コマンド第一引数
  BOX2_GFL_PROC_PARAM* box_param = NULL;
  BOX_CALLBACK_WORK*          cw = NULL;
  GMEVENT*                 event = NULL;
  
  // ボックスのプロセスパラメータを作成
  box_param            = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(BOX2_GFL_PROC_PARAM) );
  box_param->gamedata  = gdata;
  box_param->sv_box    = GAMEDATA_GetBoxManager( gdata );
  box_param->pokeparty = GAMEDATA_GetMyPokemon( gdata );
  box_param->myitem    = GAMEDATA_GetMyItem( gdata );
  box_param->mystatus  = GAMEDATA_GetMyStatus( gdata );
  box_param->cfg       = SaveData_GetConfig( sv );
  box_param->zknMode   = 0;
  box_param->callMode  = box_mode;
  
  // バトルボックスのパーティーをセット
  if( box_mode == BOX_MODE_BATTLE )
  {
    BATTLE_BOX_SAVE* bbox_save;
    bbox_save = BATTLE_BOX_SAVE_GetBattleBoxSave( sv );
    box_param->pokeparty = BATTLE_BOX_SAVE_MakePokeParty( bbox_save, HEAPID_PROC );
  }

  // コールバックのパラメータを作成
  cw = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(BOX_CALLBACK_WORK) );
  cw->gdata     = gdata;
  cw->box_param = box_param;

  // イベントを呼び出す
  event = EVENT_FieldSubProc_Callback(
      gsys, fieldmap, 
      FS_OVERLAY_ID(box), &BOX2_ProcData, box_param, // 呼び出すプロセスを指定
      callback_BoxProc, cw );  // コールバック関数と, その引数を指定
  SCRIPT_CallEvent( scw, event );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief ゲームクリア処理呼び出し
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGameClearDemo( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u16 mode = SCRCMD_GetVMWorkValue( core, work );
  GMEVENT * call_event;

  call_event = EVENT_GameClear( gsys, GAMECLEAR_MODE_NORMAL );
  //スクリプト終了後、指定したイベントに遷移する
  SCRIPT_EntryNextEvent( sc, call_event );
  
  VM_End( core );   //スクリプト終了

  return VMCMD_RESULT_SUSPEND;
}


