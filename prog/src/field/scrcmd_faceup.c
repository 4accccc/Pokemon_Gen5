//======================================================================
/**
 * @file  scrcmd_faceup.c
 * @brief  スクリプトコマンド：顔アップ関連
 * @author  Saito
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

#include "event_fieldmap_control.h"

#include "fld_faceup.h"
#include "scrcmd_faceup.h"

FS_EXTERN_OVERLAY(fld_faceup);

static GMEVENT_RESULT EndEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * 顔アップ開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFaceup_Start( VMHANDLE *core, void *wk )
{
  u8 back_idx;
  u8 char_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  back_idx = VMGetU16( core );
  char_idx = VMGetU16( core );

  //終了チェックフラグをオン
  SCREND_CHK_SetBitOn(SCREND_CHK_FACEUP);

  GFL_OVERLAY_Load( FS_OVERLAY_ID(fld_faceup) );		//オーバーレイロード

  call_event = FLD_FACEUP_Start(back_idx, char_idx, gsys);
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 顔アップ終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFaceup_End( VMHANDLE *core, void *wk )
{
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  call_event = GMEVENT_Create( gsys, NULL, EndEvt, 0 );
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 顔アップ顔変更
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFaceup_Change( VMHANDLE *core, void *wk )
{
  u8 char_idx;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  char_idx = VMGetU16( core );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 *スクリプト終了時 顔アップ終了チェック
 * @param   end_chk     チェック構造体
 * @param   seq     サブシーケンサ
 * @retval  BOOL    TRUEでチェック終了
 */
//--------------------------------------------------------------
BOOL SCREND_CheckEndFaceup(SCREND_CHECK *end_check , int *seq)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(end_check->gsys);
  FLD_FACEUP_Release( fieldWork );
  //終了チェックフラグをオフ
  SCREND_CHK_SetBitOff(SCREND_CHK_FACEUP);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(fld_faceup));		//オーバーレイアンロード

  return  TRUE;
}

//--------------------------------------------------------------
/**
 * 終了監視イベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EndEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  switch(*seq){
  case 0:
    {
      GMEVENT *call_event;
      //イベントコール
      call_event = FLD_FACEUP_End(gsys);
      GMEVENT_CallEvent(event, call_event);
    }
    (*seq)++;
    break;
  case 1:
    //終了チェックフラグをオフ
    SCREND_CHK_SetBitOff(SCREND_CHK_FACEUP);
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(fld_faceup));		//オーバーレイアンロード
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


