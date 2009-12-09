//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：Wi-Fi関連(常駐)
 * @file   scrcmd_wifi_st.c
 * @author iwasawa
 * @date   2009.12.08
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_sodateya.h"
#include "app/pokelist.h"
#include "system/main.h"
#include "script_local.h"
#include "app/p_status.h"
#include "event_fieldmap_control.h"
#include "savedata/sodateya_work.h"
#include "fieldmap.h"

#include "scrcmd_wifi_st.h"
#include "savedata/wifilist.h"

//--------------------------------------------------------------------
/**
 * @brief   GeoNetを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdWifiGeoNetCall( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

#if 0
  SCRIPT_CallEvent( sc, EVENT_WiFiClub( gsys, GAMESYSTEM_GetFieldMapWork(gsys)));
#endif
	return VMCMD_RESULT_SUSPEND;		///<コマンド実行を中断して制御を返す
}

//--------------------------------------------------------------------
/**
 * @brief   Wi-Fiランダムマッチを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdWifiRandomMatchEventCall( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  /*
  SCRIPT_CallEvent( sc, EVENT_WiFiClub( gsys, GAMESYSTEM_GetFieldMapWork(gsys)));
  */ 
	return VMCMD_RESULT_SUSPEND;		///<コマンド実行を中断して制御を返す
}

//--------------------------------------------------------------------
/**
 * @brief   バトルレコーダーイベントを呼び出す
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 *
 * 呼び出しモード指定 field/script_def.h
 *  SCRCMD_BTL_RECORDER_MODE_VIDEO   (0)
 *  SCRCMD_BTL_RECORDER_MODE_MUSICAL (1)
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdWifiBattleRecorderEventCall( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 mode = SCRCMD_GetVMWorkValue( core, work );

  /*
  SCRIPT_CallEvent( sc, EVENT_WiFiClub( gsys, GAMESYSTEM_GetFieldMapWork(gsys)));
  */
	return VMCMD_RESULT_SUSPEND;		///<コマンド実行を中断して制御を返す
}

