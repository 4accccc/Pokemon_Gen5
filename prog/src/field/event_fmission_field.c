//============================================================================================
/**
 * @file    event_fmission_field.c
 * @brief	  イベント：フェスタミッション関連イベント
 * @author  iwasawa	
 * @date	  2011.10.25
 *
 * フェスミッションのイベントを侵入ソースintrude_field.cから切り離し
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_comm.h"
#include "gamesystem/link_festival.h"
#include "event_fmission.h"
#include "fldmmdl.h"          //MMDLSYS_PauseMoveProc
#include "field/fieldmap_call.h"   //FIELDMAP_IsReady
#include "field_comm/intrude_work.h"
#include "event_mapchange.h"  //EVENT_ChangeMapFromPalace
#include "script.h"
#include "../../../resource/fldmapdata/script/palace01_def.h"  //SCRID_PALACE01_～
#include "event_symbol.h"
#include "field/zonedata.h" //ZONEDATA_IsPalace
#include "field/palace_gimmick.h"
#include "field_comm/intrude_field.h"

static GMEVENT_RESULT EVENT_MapChangeFMissionStart(GMEVENT * event, int *seq, void*work);
//==================================================================
/**
 * フェスミッション開始時の表フィールドへのマップ移動
 *
 * @param   gsys		
 * @param   fieldmap		
 * @param   zone_id		
 * @param   pos		
 * @param   dir		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeChangeMapFMissionStart(GAMESYS_WORK * gsys)
{
	GMEVENT * event;

	event = GMEVENT_Create(gsys, NULL, EVENT_MapChangeFMissionStart, 4 );
	
	return event;
}

static GMEVENT_RESULT EVENT_MapChangeFMissionStart(GMEVENT * event, int *seq, void*work)
{
	GAMESYS_WORK  * gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	LINK_FESTIVAL* fest_sys = GAMESYSTEM_GetLinkFestivalWork(gsys);
  int* ev_work = (int*)work;

  enum{
  	SEQ_INIT,
    SEQ_PLAYER_POS_RESET,
    SEQ_DISGUISE_INIT,
    SEQ_EFFECT_WAIT,
  	SEQ_MAP_WARP,
    SEQ_FINISH,
  };
	
	switch (*seq) {
	case SEQ_INIT:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( fieldmap ) );
   
    {
      GMEVENT* mission_ev;
      FMISSION_START_PARAM param;

      MI_CpuClear8(&param,sizeof(FMISSION_START_PARAM));
      param.mission = *(LINK_FESTIVAL_GetMissionWork( fest_sys ));
      param.succeed_f = FALSE;

      mission_ev = GMEVENT_CreateOverlayEventCall( gsys,
        FS_OVERLAY_ID( event_fmission ), EVENT_FMissionStart, &param );
          
      GMEVENT_CallEvent(event, mission_ev);
    }
    (*seq)++;
    break;
  case SEQ_PLAYER_POS_RESET:
    // ミッション開始のギミックエフェクト開始
    PALACE_GMK_EffectStart( gsys );

    if(GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION){//INVASIONじゃない場合はFIELD_BEACONの場合があるのでINVASIONじゃないかどうかでチェック
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
      int i;
      PLAYER_WORK *plwork;
      VecFx32 pos = {0,0,0};
        
      for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
        plwork = GAMEDATA_GetPlayerWork( gamedata , i );
        PLAYERWORK_setPosition(plwork, &pos);
      }
    }
    (*seq)++;
    break;
  case SEQ_DISGUISE_INIT:  //変装をしていれば元に戻す
    {
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      
      if(FIELDMAP_IsReady(fieldWork) == TRUE){
        *seq = SEQ_EFFECT_WAIT;
      }
    }
    break;
	
  case SEQ_EFFECT_WAIT:
    if( (*ev_work)++ < FMISSION_START_EFFECT1_WAIT ){  //エフェクト1待ち
      break;
    }
    // ミッション開始のギミックエフェクト2開始
    PALACE_GMK_EffectStart2( gsys );
    *ev_work = 0; //ウェイトリセット
    (*seq)++;
    break;
	case SEQ_MAP_WARP:
    if( (*ev_work)++ < FMISSION_START_EFFECT2_WAIT ){  //エフェクト2待ち
      break;
    }
  	GMEVENT_CallEvent( event, EVENT_ChangeMapFromPalace(gsys) );
    (*seq)++;
    break;
	case SEQ_FINISH:
    LINK_FESTIVAL_MissionPlayStart( fest_sys, FALSE );  //時間カウントスタート
 	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(fieldmap));
 	  return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * パレス：座標イベント
 *
 * @param   fieldWork		
 * @param   gameSys		
 * @param   pcActor		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * Intrude_CheckPosEvent(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor)
{
  PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gameSys );
  ZONEID zone_id = PLAYERWORK_getZoneID( plWork );
  VecFx32 pos;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gameSys);
  INTRUDE_COMM_SYS_PTR intcomm;
  int i;
  
  intcomm = Intrude_Check_CommConnect(game_comm);

  
  //------------- ここから下はパレス時のみの処理 -----------------
  
  if(ZONEDATA_IsPalace(zone_id) == FALSE){
    return NULL;
  }

  FIELD_PLAYER_GetPos( pcActor, &pos );
  
  //表フィールドへワープ
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    if(pos.x >= FX32_CONST(504) + PALACE_MAP_LEN*i && pos.x <= FX32_CONST(504) + PALACE_MAP_LEN*i
        && pos.z >= FX32_CONST(712) && pos.z <= FX32_CONST(712)){
      return SCRIPT_SetEventScript(
          gameSys, SCRID_PALACE01_WARP_POINT, NULL, FIELDMAP_GetHeapID( fieldWork ) );
      //return EVENT_ChangeMapPosCommEnd(gameSys, fieldWork, intcomm);
    }
  }
  //ビンゴマップへワープ
  {
    fx32 entrance_x = PALACE_MAP_SYMMAP_ENTRANCE_X;
    fx32 entrance_z = PALACE_MAP_SYMMAP_ENTRANCE_Z;
    for (i = 0; i < FIELD_COMM_MEMBER_MAX; i++, entrance_x += PALACE_MAP_LEN )
    {
      if ( pos.x >= entrance_x && pos.x < entrance_x + PALACE_MAP_SYMMAP_ENTRANCE_SX
          && pos.z == entrance_z )
      {
        //入った位置によって出現先もずらす
        VecFx32 warp_pos = { SYMBOL_MAP_DOWN_ENTRANCE_X, 0, SYMBOL_MAP_DOWN_ENTRANCE_Z };
        warp_pos.x += pos.x - entrance_x;
        return EVENT_SymbolMapWarp(gameSys, TRUE, &warp_pos, DIR_UP, SYMBOL_MAP_ID_ENTRANCE);
      }
    }
  }
  
#if 0
  {//自機の座標を監視し、誰とも通信していないのにパレスの橋の一定位置まで来たら
   //注意メッセージを出して引き返させる
    BOOL left_right;
    
    if(GFL_NET_GetConnectNum() <= 1 && ((pos.x <= 136 && pos.x >= 136-16) || (pos.x >= 872 && pos.x <= 872+16)) && (pos.z >= 472 && pos.z <= 536)){
      left_right = pos.x <= (PALACE_MAP_LEN/2 >> FX32_SHIFT) ? 0 : 1;
      return EVENT_PalaceNGWin( gameSys, fieldWork, pcActor, left_right );
    }
  }
#endif
  return NULL;
}

