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

#include "fieldmap.h"
#include "field_sound.h"
#include "field_player.h"
#include "event_fieldmap_control.h"
#include "event_freeword_input.h"
#include "event_dendou_pc.h"
#include "event_game_manual.h"

#include "scrcmd_proc.h"
#include "scrcmd_proc_fld.h"

#include "event_gameclear.h"  //EVENT_GameClear
#include "app/box2.h"
#include "savedata/battle_box_save.h"

#include "c_gear/event_cgearget.h"
#include "savedata/c_gear_data.h" //

#include "net_app/wifi_earth.h" // Earth_Demo_proc_data

#include "app/name_input.h"
#include "app/dendou_pc.h"

static void callback_BoxProc( void* work );
static void CallBackFunc_byHelloMsgIn( void* wk );

//--------------------------------------------------------------
/**
 * @brief ボックスプロセス終了後に呼ばれるコールバック関数
 */
//--------------------------------------------------------------
typedef struct {

  GAMEDATA*            gdata;     // ゲームデータ
  u16*                 retWk;     // ボックス終了モードの格納先
  BOX2_GFL_PROC_PARAM* box_param; // ボックスパラメータ

} BOX_CALLBACK_WORK; 

//--------------------------------------------------------------
/**
 * @brief ボックスプロセス終了後に呼ばれるコールバック関数
 */
//--------------------------------------------------------------
static void callback_BoxProc( void* work )
{
  BOX_CALLBACK_WORK* callback_work = (BOX_CALLBACK_WORK*)work;

  // バトルボックスのセーブデータに反映
  if( callback_work->box_param->callMode == BOX_MODE_BATTLE ) {
    SAVE_CONTROL_WORK* save      = GAMEDATA_GetSaveControlWork( callback_work->gdata );
    BATTLE_BOX_SAVE*   bbox_save = BATTLE_BOX_SAVE_GetBattleBoxSave( save );
    BATTLE_BOX_SAVE_SetPokeParty( bbox_save, callback_work->box_param->pokeparty );
  }

  // ポケパーティを解放
  if( callback_work->box_param->callMode == BOX_MODE_BATTLE ) {
    GFL_HEAP_FreeMemory( callback_work->box_param->pokeparty );
  }

  // ボックス終了モードを返す
  switch( callback_work->box_param->retMode ) {
  default:
  case BOX_END_MODE_MENU:   *(callback_work->retWk) = SCR_BOX_END_MODE_MENU;   break;
  case BOX_END_MODE_C_GEAR: *(callback_work->retWk) = SCR_BOX_END_MODE_C_GEAR; break;
  }

  // 後始末
  GFL_HEAP_FreeMemory( callback_work->box_param );
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
  SCRCMD_WORK*         work          = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*         script        = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*        gsys          = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*            gdata         = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   save          = GAMEDATA_GetSaveControlWork( gdata );
  ZUKAN_SAVEDATA*      zukan_save    = GAMEDATA_GetZukanSave( gdata );
  FIELDMAP_WORK*       fieldmap      = GAMESYSTEM_GetFieldMapWork( gsys );
  u16*                 ret_wk        = SCRCMD_GetVMWork( core, work );       // コマンド第一引数
  u16                  box_mode      = SCRCMD_GetVMWorkValue( core, work );  // コマンド第二引数
  BOX2_GFL_PROC_PARAM* box_param     = NULL;
  BOX_CALLBACK_WORK*   callback_work = NULL;
  GMEVENT*             event         = NULL;
  
  // ボックスのプロセスパラメータを作成
  box_param            = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(BOX2_GFL_PROC_PARAM) );
  box_param->gamedata  = gdata;
  box_param->sv_box    = GAMEDATA_GetBoxManager( gdata );
  box_param->pokeparty = GAMEDATA_GetMyPokemon( gdata );
  box_param->myitem    = GAMEDATA_GetMyItem( gdata );
  box_param->mystatus  = GAMEDATA_GetMyStatus( gdata );
  box_param->cfg       = SaveData_GetConfig( save );
  box_param->zknMode   = ZUKANSAVE_GetZenkokuZukanFlag( zukan_save );
  box_param->callMode  = box_mode;
	box_param->bbRockFlg = FALSE;
  
  // バトルボックスのパーティーをセット
  if( box_mode == BOX_MODE_BATTLE ) {
    BATTLE_BOX_SAVE* bbox_save;
    bbox_save = BATTLE_BOX_SAVE_GetBattleBoxSave( save );
    box_param->pokeparty = BATTLE_BOX_SAVE_MakePokeParty( bbox_save, HEAPID_PROC );
		box_param->bbRockFlg = BATTLE_BOX_SAVE_GetLockType( bbox_save, BATTLE_BOX_LOCK_BIT_BOTH );
  }

  // コールバックのパラメータを作成
  callback_work = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(BOX_CALLBACK_WORK) );
  callback_work->gdata     = gdata;
  callback_work->retWk     = ret_wk;
  callback_work->box_param = box_param;

  // イベントを生成
  event = EVENT_FieldSubProcNoFade_Callback( gsys, fieldmap, 
      FS_OVERLAY_ID(box), &BOX2_ProcData, box_param, // 呼び出すプロセスを指定
      callback_BoxProc, callback_work );  // コールバック関数と, その引数を指定

  SCRIPT_CallEvent( script, event );

  return VMCMD_RESULT_SUSPEND;
}


//======================================================================
// すれ違い『挨拶メッセージ』入力画面
//======================================================================

//--------------------------------------------------------------
/**
 * @brief   すれ違い『挨拶メッセージ』入力画面の呼び出し
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCrossCommHelloMessageInputCall( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work       = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   script     = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*  gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  GMEVENT* event;
  u16* ret_wk;

  // コマンド引数を取得
  ret_wk = SCRCMD_GetVMWork( core, work ); // 第一引数
  event = EVENT_FreeWordInput( gameSystem, fieldmap, NULL, NAMEIN_GREETING_WORD, ret_wk );
  // イベントを呼ぶ
  SCRIPT_CallEvent( script, event );

  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
// すれ違い『お礼メッセージ』入力画面
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   すれ違い『お礼メッセージ』入力画面の呼び出し
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCrossCommThanksMessageInputCall( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work       = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   script     = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*  gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  GMEVENT* event;
  u16* ret_wk;

  // コマンド引数を取得
  ret_wk = SCRCMD_GetVMWork( core, work ); // 第一引数
  event = EVENT_FreeWordInput( gameSystem, fieldmap, NULL, NAMEIN_THANKS_WORD, ret_wk );
  // イベントを呼ぶ
  SCRIPT_CallEvent( script, event );
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

  call_event = EVENT_GameClear( gsys, mode );
  //スクリプト終了後、指定したイベントに遷移する
  SCRIPT_EntryNextEvent( sc, call_event );
  
  VM_End( core );   //スクリプト終了

  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief C-GEARの動作モードセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallCGearGetDemo( VMHANDLE *core, void *wk )
{
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
  CGEAR_SAVEDATA * cgear_sv = CGEAR_SV_GetCGearSaveData( sv );
  u16 power_on = SCRCMD_GetVMWorkValue( core, wk );

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  CGEAR_SV_SetCGearONOFF( cgear_sv, TRUE );

  SCRIPT_CallEvent( sc, CGEARGET_EVENT_Start( gsys, power_on ) );
  return VMCMD_RESULT_SUSPEND;
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   ジオネット呼び出し
 * @param core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallGeonet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  GMEVENT * event;
  event = EVENT_FieldSubProc( gsys, fieldmap, FS_OVERLAY_ID( geonet ), &Earth_Demo_proc_data, gamedata );
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief   殿堂入りを呼び出します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallDendouProc( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*       scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*          gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  FIELDMAP_WORK*     fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u16                id = SCRCMD_GetVMWorkValue( core, work ); // コマンド第一引数
  u16*               ret_wk = SCRCMD_GetVMWork( core, work ); // コマンド第二引数
  GMEVENT*           event = NULL;
  u16 call_mode;

  // 呼び出しタイプを決定
  switch( id ) {
  default: GF_ASSERT(0);
  case SCRCMD_DENDOU_PC_FIRST:  call_mode = DENDOUPC_CALL_CLEAR;  break;
  case SCRCMD_DENDOU_PC_DENDOU: call_mode = DENDOUPC_CALL_DENDOU; break;
  }

  // イベントを呼び出す
  SCRIPT_CallEvent( scw, EVENT_DendouCall( gsys, call_mode, ret_wk ) );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief  ゲーム内マニュアルの呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallGameManual( VMHANDLE *core, void *wk )
{
  GMEVENT* event;

  SCRCMD_WORK*  work       = wk;
  GAMESYS_WORK* gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*  script     = SCRCMD_WORK_GetScriptWork( work );
  u16*          ret_wk     = SCRCMD_GetVMWork( core, work ); // コマンド第一引数

  event = EVENT_GameManual( gameSystem, ret_wk );
  SCRIPT_CallEvent( script, event );

  return VMCMD_RESULT_SUSPEND;
}
