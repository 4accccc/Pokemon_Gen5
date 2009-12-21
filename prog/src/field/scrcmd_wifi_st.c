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
#include "event_wifibattlematch.h"
#include "event_battlerecorder.h"
#include "script_def.h"
#include "poke_tool/regulation_def.h"

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
 *
 * @regulation  poke_tool/regulation_def.h
 * @party field/script_def.h SCR_BTL_PARTY_SELECT_TEMOTI,SCR_BTL_PARTY_SELECT_BTLBOX
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdWifiRandomMatchEventCall( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  u16 regulation = SCRCMD_GetVMWorkValue( core, work );
  u16 party = SCRCMD_GetVMWorkValue( core, work );

  
  WIFIBATTLEMATCH_POKE poke;
  BtlRule btl_rule;

  switch( regulation )
  { 
  case REG_RND_SINGLE:
    btl_rule  = BTL_RULE_SINGLE;
    break;
  case REG_RND_DOUBLE:
    btl_rule  = BTL_RULE_DOUBLE;
    break;
  case REG_RND_TRIPLE:
    btl_rule  = BTL_RULE_TRIPLE;
    break;
  case REG_RND_ROTATION:
    btl_rule  = BTL_RULE_ROTATION;
    break;
  case REG_RND_TRIPLE_SHOOTER:
    btl_rule  = BTL_RULE_TRIPLE;
    break;
  }

  switch( party )
  { 
  case SCR_BTL_PARTY_SELECT_TEMOTI:
    poke  = WIFIBATTLEMATCH_POKE_TEMOTI;
    break;
  case SCR_BTL_PARTY_SELECT_BTLBOX:
    poke  = WIFIBATTLEMATCH_POKE_BTLBOX;
    break;
  }

  SCRIPT_CallEvent( sc, EVENT_WifiBattleMatch( gsys, GAMESYSTEM_GetFieldMapWork(gsys), WIFIBATTLEMATCH_MODE_RANDOM, poke, btl_rule ) );
  
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

  u16 value = SCRCMD_GetVMWorkValue( core, work );

  const BR_MODE mode = (value == SCRCMD_BTL_RECORDER_MODE_VIDEO)? BR_MODE_GLOBAL_BV:  BR_MODE_GLOBAL_MUSICAL;

  SCRIPT_CallEvent( sc, EVENT_WifiBattleRecorder( gsys, GAMESYSTEM_GetFieldMapWork(gsys), mode ) );

	return VMCMD_RESULT_SUSPEND;		///<コマンド実行を中断して制御を返す
}
