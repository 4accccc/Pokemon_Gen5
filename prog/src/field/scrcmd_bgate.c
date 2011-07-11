//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_bgate.c
 *	@brief  バッジチェックゲートスクリプト
 *	@author	Masayuki Onoue
 *	@date		2011.07.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "system/main.h"          //HEAPID_FIELDMAP
#include "fieldmap/zone_id.h"
#include "script_def.h"
#include "scrcmd_bgate.h"
#include "field_gimmick_bgate.h"

//--------------------------------------------------------------
/**
 * バッジチェックゲートギミック　モニュメントの起動イベント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBGateMonumentStart( VMHANDLE *p_core, void *p_wk_adrs )
{
  GMEVENT *p_call_event;
  SCRCMD_WORK   *p_wk       = p_wk_adrs;
  SCRIPT_WORK   *p_sc         = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK  *p_gsys     = SCRCMD_WORK_GetGameSysWork( p_wk );
  u8            monument_id = SCRCMD_GetVMWorkValue( p_core, p_wk );

  p_call_event = BADGEGATE_CreateMonumentStartEvent(p_gsys,monument_id);

  if (p_call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( p_sc, p_call_event );
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * バッジチェックゲートギミック　ゲートの起動イベント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBGateGateOpen( VMHANDLE *p_core, void *p_wk_adrs )
{
  GMEVENT *p_call_event;
  SCRCMD_WORK   *p_wk       = p_wk_adrs;
  SCRIPT_WORK   *p_sc         = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK  *p_gsys     = SCRCMD_WORK_GetGameSysWork( p_wk );

  p_call_event = BADGEGATE_CreateGateOpenEvent(p_gsys);

  if (p_call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( p_sc, p_call_event );
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

