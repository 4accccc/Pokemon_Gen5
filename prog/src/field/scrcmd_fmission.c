//==============================================================================
/**
 * @file    scrcmd_fmission.c
 * @brief   スクリプトコマンド：フェスミッション関連(フィールド常駐)
 * @author  iwasawa 
 * @date    2011.02.27
 */
//==============================================================================
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

#include "scrcmd_intrude.h"

#include "field_comm\intrude_types.h"
#ifdef  DISABLE_INTRUDE // swanで侵入を削除
#include "field/field_comm/intrude_field.h"
#else
#include "field_comm\intrude_main.h"
#include "field_comm\intrude_comm_command.h"
#include "field/field_comm/intrude_monolith.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_field.h"
#include "field\field_comm\intrude_work.h"
#include "field/field_comm/intrude_mission_field.h"
#endif  //undef(DISABLE_INTRUDE)

#include "gamesystem/link_festival.h"

#include "message.naix"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"

#ifndef  DISABLE_INTRUDE // swanで侵入を削除
#include "event_intrude.h"
#include "event_intrude_subscreen.h"
#endif  //undef(DISABLE_INTRUDE)
#include "fieldmap/zone_id.h"
#include "field/zonedata.h"
#include "item/itemsym.h"

#include "scrcmd_fmission.h"
#include "fes_gimmick.h"
#include "fes_gimmick_local.h"
#include "fes_gimmick_def.h"

#include "event_mapchange.h"

//==================================================================
/**
 * フェスミッション　各種施設受け付け用強制終了コマンド
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT
 *
 * フェス中でないときに呼び出されたら、なにもせずに副作用無しで終了
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionEndForce( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  LINK_FESTIVAL* fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  u32 bgm;

  if( LINK_FESTIVAL_IsMissionEntry( fest_sys ) == FALSE ){
    return VMCMD_RESULT_SUSPEND;  //ミッション中でなければ何もせずに帰る
  }
  //フェス終了処理
  if( FES_GIMMICK_Delete( fes_gmk ) ){
    GF_ASSERT(0); //無理やりギミック削除はホントはやっちゃダメなので、チェック用アサート仕掛けておく
  }
  bgm = LINK_FESTIVAL_MissionEnd( fest_sys );
  
  if( bgm != FES_BGM_RECOVER_NONE ){
    SCRIPT_CallEvent( scw, EVENT_FSND_PlayEventBGM( gsys, bgm ) );
  }
  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスミッション　指定ログアクションビーコン送信リクエスト
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionLogAction( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  FMISSION_LOG_ACTION log_act = (u8)SCRCMD_GetVMWorkValue( core, wk );
  u16 param = SCRCMD_GetVMWorkValue( core, wk );

  GAMEBEACON_Set_FMissionLogAction( log_act, param );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション 人物mmdl削除とクリアフラグOn処理 
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcEventClear( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  u16 zone_id = SCRCMD_WORK_GetZoneID( work );
  MMDL *mmdl = SCRIPT_GetTargetObj( scw );
 
  u8 obj_idx = ( MMDL_GetOBJID(mmdl)-MMDL_ID_FMISSION_OBJ_01 );

  if( !FES_GIMMICK_MissionTypeCheck( FIELDMAP_GetFesGimmickSys( fieldmap ), FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  //取得済みフラグセットと動作モデルの削除
  FES_GIMMICK_FlagSet( FIELDMAP_GetFesGimmickSys( fieldmap ), zone_id, obj_idx );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション 話しかけターゲットのNPCのMMDLが持つパラメータを取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcParamGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  u16 param_no = SCRCMD_GetVMWorkValue( core, wk );
  u16* ret_wk = SCRCMD_GetVMWork( core, wk );

  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_wk = 0;
    return VMCMD_RESULT_SUSPEND;
  }
  *ret_wk = MMDL_GetParam( SCRIPT_GetTargetObj( scw ), param_no );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション アイテム交換ミッションのアイテムを取得 
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcTradeItemGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  u16 my_rank = SCRCMD_GetVMWorkValue( core, wk );
  u16 npc_rank = SCRCMD_GetVMWorkValue( core, wk );
  u16* ret_mine = SCRCMD_GetVMWork( core, wk );
  u16* ret_npc = SCRCMD_GetVMWork( core, wk );

  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_mine = *ret_npc = ITEM_KIZUGUSURI;
    return VMCMD_RESULT_SUSPEND;
  }
  FES_GIMMICK_NPC_TradeItemGet( fes_gmk, SCRIPT_GetTargetObj( scw ), my_rank, npc_rank, ret_mine, ret_npc );
  
  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション アイテム交換ミッション 
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcShoppingItemGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  
  u16* ret_item = SCRCMD_GetVMWork( core, wk );
  u16* ret_price = SCRCMD_GetVMWork( core, wk );

  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_item = ITEM_KIZUGUSURI;
    *ret_price = 9999;
    return VMCMD_RESULT_SUSPEND;
  }
  FES_GIMMICK_NPC_ShoppingItemGet( fes_gmk, SCRIPT_GetTargetObj(scw), ret_item, ret_price );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション 記憶クイズミッション  クイズパラメータ生成
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  
  u16* ret_q_num = SCRCMD_GetVMWork( core, wk );
  u16* ret_q_idx = SCRCMD_GetVMWork( core, wk );
  u16* ret_a_idx = SCRCMD_GetVMWork( core, wk );
  
  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_q_num = 1;
    *ret_q_idx = *ret_a_idx = 0;
    return VMCMD_RESULT_SUSPEND;
  }
  FES_GIMMICK_NPC_MemoryQuizParamGet( fes_gmk, ret_q_num, ret_q_idx, ret_a_idx );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション 記憶クイズミッション  出題パラメータ取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGetQuestionPoke( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  
  u16  q_idx = SCRCMD_GetVMWorkValue( core, wk );
  u16* ret_q_poke = SCRCMD_GetVMWork( core, wk );

  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_q_poke = MONSNO_MINEZUMI+q_idx;
    return VMCMD_RESULT_SUSPEND;
  }
  *ret_q_poke = FES_GIMMICK_NPC_MemoryQuizParamGetQuestionPoke( fes_gmk, q_idx );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * フェスNPCミッション 記憶クイズミッション  回答パラメータ取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGetAnswerPoke( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*  scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FES_GIMMICK* fes_gmk = FIELDMAP_GetFesGimmickSys( fieldmap );
  
  u16  a_idx = SCRCMD_GetVMWorkValue( core, wk );
  u16* ret_a_poke = SCRCMD_GetVMWork( core, wk );

  if( !FES_GIMMICK_MissionTypeCheck( fes_gmk, FMISSION_TYPE_NPC ) ){
    GF_ASSERT(0);
    *ret_a_poke = MONSNO_MINEZUMI+a_idx;
    return VMCMD_RESULT_SUSPEND;
  }
  *ret_a_poke = FES_GIMMICK_NPC_MemoryQuizParamGetAnswerPoke( fes_gmk, a_idx );

  return VMCMD_RESULT_SUSPEND;
}

//==============================================================================
//  
//  以下はscrcmd_intrude_field.cから引っ越したコマンド
//
//==============================================================================
//==================================================================
/**
 * ミッションエントリーしているかを取得
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * TRUE:エントリーしている　FALSE:エントリーしていない
 */
//==================================================================
VMCMD_RESULT EvCmdGetMissionEntry( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  LINK_FESTIVAL* fest_sys = GAMESYSTEM_GetLinkFestivalWork(gsys);

  if ( FEST_MISSION_IsEntry( LINK_FESTIVAL_GetMissionWork(fest_sys) ) )
  {
    *ret_wk = SCR_PALACE_MISSION_ENTRY_FEST;
  }
  else
  {
    *ret_wk = SCR_PALACE_MISSION_ENTRY_NONE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * Gパワーを何か装備しているか取得
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * TRUE:何か装備している
 * FALSE:何も装備していない
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeCheckEqpGPower( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_SAVE_WORK * intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( gamedata ) );
  GPOWER_ID gpower_id;
  
  gpower_id = ISC_SAVE_GetGPowerID(intsave,0);
  if(gpower_id != GPOWER_ID_NULL){
    *ret_wk = TRUE;
  }
  else{
    *ret_wk = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * フェスミッション開始時に表フィールドにマップチェンジさせる
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeMapChangeFMissionStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  SCRIPT_CallEvent( scw, EVENT_IntrudeChangeMapFMissionStart(gsys) );

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * @brief スクリプトコマンド：パレスから戻る
 * @date  2011.10.25
 *
 * swan追加。swanでは侵入はなく、パレスから戻るのはワープポイントのみ。
 */
//==================================================================
VMCMD_RESULT EvCmdReturnFromHilink( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_CallEvent( scw, EVENT_ChangeMapFromPalace( gsys ) );
  return VMCMD_RESULT_SUSPEND;
}

