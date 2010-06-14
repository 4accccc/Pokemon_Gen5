//==============================================================================
/**
 * @file    event_intrude_subscreen.c
 * @brief   侵入によるサブスクリーンイベント
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/field_subscreen.h"
#include "event_intrude_subscreen.h"
#include "fieldmap.h"
#include "event_mapchange.h"
#include "print/wordset.h"
#include "field_comm/intrude_types.h"
#include "field_comm/intrude_main.h"
#include "field_comm/intrude_work.h"
#include "field_comm/intrude_mission.h"
#include "field_comm/intrude_message.h"
#include "field_comm/intrude_field.h"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"
#include "fieldmap/zone_id.h"
#include "fld_vreq.h"


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
  FIELDMAP_WORK *fieldWork;
  FIELD_SUBSCREEN_WORK *subscreen;
}EVENT_CHANGESUBSCRN;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT EventChangeSubScreen_to_CGear( GMEVENT* event, int* seq, void* wk );



//==============================================================================
//  ワープイベント
//==============================================================================
//==================================================================
/**
 * 侵入によるワープイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   partner     TRUE:協力者(表フィールドからパレスへ行く時のみ使用)
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeTownWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 zone_id, BOOL partner)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  VecFx32 pos;
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int palace_area;
  
  if(IntrudeField_GetPalaceTownZoneID(zone_id, &pos) == FALSE){
    return NULL;
  }

  //裏フィールド以外から、パレスへ飛ぶ場合、専用のイベントを起動(戻り先を記録、ワープ不可判定)
  if (GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE && zone_id == ZONE_ID_PALACE01 ){
    event = EVENT_ChangeMapFldToPalace( gsys, zone_id, &pos, partner );
  }
  else{
    event = EVENT_ChangeMapPalace_to_Palace( gsys, zone_id, &pos );
  }
  return event;
}

//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudePlayerWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, int player_netid)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  ZONEID warp_zone_id;
  VecFx32 pos;
	INTRUDE_COMM_SYS_PTR intcomm;
  const PALACE_ZONE_SETTING *zonesetting;
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL || (intcomm->recv_profile & (1 << player_netid)) == 0){
    return NULL;
  }
  
  warp_zone_id = intcomm->intrude_status[player_netid].zone_id;
  zonesetting = IntrudeField_GetZoneSettingData(warp_zone_id);
  if(zonesetting == NULL){
    return NULL;
  }
  warp_zone_id = zonesetting->warp_zone_id;
  
  pos = intcomm->intrude_status[player_netid].player_pack.pos;
  pos.z += 32 << FX32_SHIFT;
  event = EVENT_ChangeMapPos(gsys, fieldWork, warp_zone_id, &pos, DIR_UP, FALSE);
  return event;
}


//==================================================================
/**
 * 下画面を侵入用からCギアに戻す
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   partner     TRUE:協力者(表フィールドからパレスへ行く時のみ使用)
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_ChangeSubScreen_to_CGear(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_SUBSCREEN_WORK *subscreen)
{
  GMEVENT * event;
  EVENT_CHANGESUBSCRN *ecs;
  
  event = GMEVENT_Create( gsys, NULL, EventChangeSubScreen_to_CGear, sizeof(EVENT_CHANGESUBSCRN) );
  
  ecs = GMEVENT_GetEventWork( event );
  // イベントワーク初期化
  ecs->fieldWork = fieldWork;
  ecs->subscreen = subscreen;

  return event;
}

//--------------------------------------------------------------
/**
 * 下画面を侵入用からCギアに戻す
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventChangeSubScreen_to_CGear( GMEVENT* event, int* seq, void* wk )
{
  EVENT_CHANGESUBSCRN *ecs = wk;

  switch(*seq){
  case 0:
    FLD_VREQ_GXS_SetMasterBrightness( FIELDMAP_GetFldVReq(ecs->fieldWork), -16 );
    (*seq)++;
    break;
  case 1:
    FIELD_SUBSCREEN_ChangeForce( ecs->subscreen, FIELD_SUBSCREEN_NORMAL );
    (*seq)++;
    break;
  case 2:
    FLD_VREQ_GXS_SetMasterBrightness( FIELDMAP_GetFldVReq(ecs->fieldWork), 0 );
    (*seq)++;
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
