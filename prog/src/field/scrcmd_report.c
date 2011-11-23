//============================================================================================
/**
 * @file  scrcmd_report.c
 * @brief  スクリプトコマンド:レポート
 * @author  tamada
 * @date  2011.11.23
 *
 * レポートを書くコマンドが常駐領域のファイルscrcmd.cにあったので分離
 */
//============================================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_report.h"

#include "field_status_local.h"
#include "field_sound.h"
#include "field_sound_system.h"

#include "field_saveanime.h"


#include "debug/debug_flg.h" //DEBUG_FLG_～

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/// セーブイベント管理用のワーク
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELD_SAVEANIME * saveanime;
  u16 * ret_wk;
}SAVE_COMMON_EVENT_WORK;

//--------------------------------------------------------------
///  セーブイベントのシーケンス定義
//--------------------------------------------------------------
enum {
  SAVE_SEQ_INIT,
  SAVE_SEQ_WAIT,
  SAVE_SEQ_WAIT_SUBSCREEN,
  SAVE_SEQ_WAIT_SAVEANIME,
  SAVE_SEQ_FINISH,
};

//--------------------------------------------------------------
/// スクリプトからのセーブイベント本体
//--------------------------------------------------------------
static GMEVENT_RESULT saveCommonEvent( GMEVENT * event, int *seq, void *work )
{
  SAVE_COMMON_EVENT_WORK * scew = (SAVE_COMMON_EVENT_WORK*)work;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( scew->gsys );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( scew->gsys );
  FIELD_SUBSCREEN_WORK * subscreen = FIELDMAP_GetFieldSubscreenWork( fieldmap );
  HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
  SAVE_RESULT result;

  switch (*seq)
  {
  case SAVE_SEQ_INIT:
    scew->saveanime = FIELD_SAVEANIME_Create( heapID, fieldmap );
    FIELD_SAVEANIME_Start( scew->saveanime );

    // もし下画面がレポートであれば、セーブ演出開始
    if( FIELD_SUBSCREEN_GetMode( subscreen ) == FIELD_SUBSCREEN_REPORT ){
      FIELD_SUBSCREEN_SetReportSize( subscreen );
      FIELD_SUBSCREEN_SetReportStart( subscreen );
    }
    
    { //レポートを書いた数を記録
      RECORD * rec = GAMEDATA_GetRecordPtr( gamedata );
      RECORD_Inc( rec, RECID_REPORT_COUNT );
    }
    // 分割セーブ開始
    GAMEDATA_SaveAsyncStart( gamedata );
    (*seq) = SAVE_SEQ_WAIT;
    break;

  case SAVE_SEQ_WAIT:
    // 分割セーブ実行
    result = GAMEDATA_SaveAsyncMain( gamedata );

    switch( result )
    {
    // 続行
    case SAVE_RESULT_CONTINUE:
    case SAVE_RESULT_LAST:
      break;

    // 完了
    case SAVE_RESULT_OK:
      if (scew->ret_wk != NULL)
      {
        *scew->ret_wk = TRUE;
      }
      (*seq) = SAVE_SEQ_WAIT_SUBSCREEN;
      break;
    case SAVE_RESULT_NG:
      if (scew->ret_wk != NULL)
      {
        *scew->ret_wk = FALSE;
      }
      (*seq) = SAVE_SEQ_WAIT_SUBSCREEN;
      if( FIELD_SUBSCREEN_GetMode( subscreen ) == FIELD_SUBSCREEN_REPORT ){
        FIELD_SUBSCREEN_SetReportBreak( subscreen );  // 下画面終了
      }
      break;
    }
    break;

  case SAVE_SEQ_WAIT_SUBSCREEN:
    GF_ASSERT( GAMEDATA_GetIsSave( gamedata ) == FALSE );
    // 下画面の表示が終わるまで待つ。
    if( FIELD_SUBSCREEN_GetMode( subscreen ) == FIELD_SUBSCREEN_REPORT ){
      if( FIELD_SUBSCREEN_SetReportEnd( subscreen ) == FALSE ){
        break;
      }
    }
    *seq = SAVE_SEQ_WAIT_SAVEANIME;
    break;

  case SAVE_SEQ_WAIT_SAVEANIME:
    FIELD_SAVEANIME_End( scew->saveanime );
    FIELD_SAVEANIME_Delete( scew->saveanime );
    *seq = SAVE_SEQ_FINISH;
    break;

  case SAVE_SEQ_FINISH:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * createCommonSaveEvent( GAMESYS_WORK * gsys, u16 * ret_wk )
{
  GMEVENT * event = GMEVENT_Create( gsys, NULL, saveCommonEvent, sizeof(SAVE_COMMON_EVENT_WORK) );
  SAVE_COMMON_EVENT_WORK * scew = GMEVENT_GetEventWork( event );
  scew->gsys = gsys;
  scew->saveanime = NULL;
  scew->ret_wk = ret_wk;
  return event;
}

//--------------------------------------------------------------
/**
 * @brief   スクリプトコマンド：セーブ機能呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdReportCall( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  u16* ret_wk = SCRCMD_GetVMWork( core, work );
  GMEVENT * new_event;

#ifdef  PM_DEBUG
  if (DEBUG_FLG_GetFlg(DEBUG_FLG_DisableReport) ) {
    *ret_wk = TRUE;
    return VMCMD_RESULT_SUSPEND;
  }
#endif
  new_event = createCommonSaveEvent( gsys, ret_wk );
  SCRIPT_CallEvent( sc, new_event );

  return VMCMD_RESULT_SUSPEND;
}

