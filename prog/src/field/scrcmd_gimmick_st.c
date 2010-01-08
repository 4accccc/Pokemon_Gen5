/*
 *  @file   scrcmd_gimmick_st.c
 *  @brief  スクリプトコマンド　フィールドギミック用常駐
 *  @author Miyuki Iwasawa
 *  @date   10.01.07
 */

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
#include "field_sound.h"

#include "field_gimmick_bgate_jet.h"

FS_EXTERN_OVERLAY(field_gimmick_bgate);

//--------------------------------------------------------------
/**
 * ジェットバッジゲートマップギミック初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdJetBadgeGateGimmickInit( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_gimmick_bgate) );		//オーバーレイロード
  FLD_GIMMICK_JetBadgeGate_Init( gsys );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_gimmick_bgate));		//オーバーレイアンロード

  return VMCMD_RESULT_CONTINUE;
}

