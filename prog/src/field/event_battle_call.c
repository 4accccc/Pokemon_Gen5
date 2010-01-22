//=============================================================================
/**
 *
 *	@file		event_battle_call.c
 *	@brief  イベント：バトル呼び出し
 *	@author	hosaka genya
 *	@data		2010.01.22
 *
 */
//=============================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"

#include "sound/pm_sndsys.h"
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "net_app/union/union_types.h"
#include "net_app/union/colosseum_types.h"
#include "field/event_fldmmdl_control.h"
#include "field/event_colosseum_battle.h"

#include "gamesystem/btl_setup.h"

#include "field/event_battle_call.h"

//============================================================================================
//============================================================================================

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

FS_EXTERN_OVERLAY(battle);

//============================================================================================
//
//    サブイベント
//
//============================================================================================
GMEVENT_RESULT EVENT_BattleMain(GMEVENT * event, int *  seq, void * work)
{
  enum
  {
    SEQ_CALL_START_DEMO = 0,  ///< バトル前デモ呼び出し
    SEQ_BATTLE_TIMING_INIT,
    SEQ_BATTLE_TIMING_WAIT,
    SEQ_BATTLE_INIT,
    SEQ_BATTLE_END,
    SEQ_CALL_END_DEMO,        ///< バトル後デモ呼び出し
    SEQ_BGM_POP,
    SEQ_END
  };
  
  EVENT_BATTLE_CALL_WORK * bcw = work;
  GAMESYS_WORK * gsys = bcw->gsys;

  switch (*seq) {
  case SEQ_CALL_START_DEMO:
    // 通信バトル前デモ呼び出し
    {
      GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID( comm_btl_demo ), &CommBtlDemoProcData, &bcw->demo_prm);
    }
    (*seq)++;
    break;

  case SEQ_BATTLE_TIMING_INIT:
    {
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING);
      OS_TPrintf("戦闘用通信コマンドテーブルをAddしたので同期取り\n");
      (*seq) ++;
    }
    break;
  case SEQ_BATTLE_TIMING_WAIT:
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), EVENT_BATTLE_ADD_CMD_TBL_TIMING)){
      OS_TPrintf("戦闘用通信コマンドテーブルをAdd後の同期取り完了\n");
      (*seq) ++;
    }
    break;
  case SEQ_BATTLE_INIT:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayNextBGM(gsys, bcw->btl_setup_prm.musicDefault, FSND_FADE_SHORT, FSND_FADE_NONE)); 
    GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &BtlProcData, &bcw->btl_setup_prm);
    (*seq)++;
    break;
  case SEQ_BATTLE_END:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    OS_TPrintf("バトル完了\n");
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    BATTLE_PARAM_Release( &bcw->btl_setup_prm );
    (*seq)++;
    break;
  case SEQ_CALL_END_DEMO:
    // 通信バトル後デモ呼び出し
    {
      //@TODO
      (*seq)++;
    }
    break;
  case SEQ_BGM_POP:
    GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_SHORT, FSND_FADE_NORMAL));
    (*seq) ++;
    break;
  case SEQ_END:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

