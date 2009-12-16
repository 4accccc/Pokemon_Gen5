//==============================================================================
/**
 * @file    event_intrude.c
 * @brief   侵入：イベント
 * @author  matsuda
 * @date    2009.12.15(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_monolith.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "item/itemsym.h"
#include "event_intrude.h"
#include "field_comm\intrude_field.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  DISGUISE_ANM_WAIT_MIN = 1,
  DISGUISE_ANM_WAIT_MAX = 15,
};


//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	DISGUISE_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
  
	INTRUDE_EVENT_MSGWORK iem;
	
	MISSION_DATA mdata;
	s16 wait;
	s16 wait_max;
	
	u8 anm_no;
	u8 loop;
	u8 mission_view;
	u8 padding;
}DISGUISE_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT DisguiseEvent( GMEVENT *event, int *seq, void *wk );


//======================================================================
//	イベント：プレイヤー変身イベント
//======================================================================
//==================================================================
/**
 * プレイヤー変身イベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		      侵入システムワークへのポインタ
 * @param   fmmdl_player		自機動作モデル
 * @param   heap_id		      ヒープID
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_Disguise(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, HEAPID heap_id)
{
	DISGUISE_EVENT_WORK *dis_wk;
	GMEVENT *event = NULL;
  const MISSION_DATA *mdata = MISSION_GetRecvData(&intcomm->mission);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
	
	event = GMEVENT_Create( gsys, NULL, DisguiseEvent, sizeof(DISGUISE_EVENT_WORK) );
  
	dis_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( dis_wk, sizeof(DISGUISE_EVENT_WORK) );
	
	dis_wk->heapID = heap_id;
	dis_wk->fieldWork = fieldWork;
	dis_wk->mdata = *mdata;
	if(mdata->accept_netid != GAMEDATA_GetIntrudeMyID(gamedata)){
    //受注したのが自分で無いなら、まずミッション内容を表示
    dis_wk->mission_view = TRUE;
  }
  
  IntrudeEventPrint_SetupFieldMsg(&dis_wk->iem, gsys);

	//侵入システムのアクションステータスを更新
	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_EFFECT);

	return( event );
}

//--------------------------------------------------------------
/**
 * プレイヤー変身イベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event dis_wk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DisguiseEvent( GMEVENT *event, int *seq, void *wk )
{
	DISGUISE_EVENT_WORK *dis_wk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  enum{
  	SEQ_MISSION_VIEW,
    SEQ_MISSION_VIEW_WAIT,
    SEQ_DISGUISE_START,
    SEQ_DISGUISE_MAIN,
  	SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) == SEQ_MISSION_VIEW){ //クリティカルなタイミングだけをFINISHへ飛ばす
      *seq = SEQ_FINISH;
    }
  }

	switch( *seq ){
	case SEQ_MISSION_VIEW:
	  if(dis_wk->mission_view == FALSE){
      *seq = SEQ_DISGUISE_START;
      break;
    }
    
    IntrudeEventPrint_SetupExtraMsgWin(&dis_wk->iem, gsys, 1, 1, 32-2, 16);
    
    MISSIONDATA_Wordset(intcomm, &dis_wk->mdata, dis_wk->iem.wordset, dis_wk->heapID);
    {
      u16 explain_msgid, title_msgid;
      
      title_msgid = msg_mistype_000 + dis_wk->mdata.cdata.type;
      explain_msgid = dis_wk->mdata.cdata.msg_id_contents_monolith;
      
      IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&dis_wk->iem, title_msgid, 8, 0);
      IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&dis_wk->iem, explain_msgid, 8, 16);
    }
    
    (*seq)++;
    break;
  case SEQ_MISSION_VIEW_WAIT:
    if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL)){
      IntrudeEventPrint_ExitExtraMsgWin(&dis_wk->iem);
      (*seq)++;
    }
    break;
  
  case SEQ_DISGUISE_START:
    dis_wk->wait_max = DISGUISE_ANM_WAIT_MAX;
    dis_wk->wait = dis_wk->wait_max;
    (*seq)++;
    break;
  case SEQ_DISGUISE_MAIN:
    {
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(dis_wk->fieldWork);
      MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
      static const u8 player_anm_tbl[] = {
        AC_DIR_U, AC_DIR_L, AC_DIR_D, AC_DIR_R,
      };
      
      if(MMDL_CheckPossibleAcmd(player_mmdl) == TRUE){
        if(dis_wk->wait_max >= DISGUISE_ANM_WAIT_MAX 
            && dis_wk->loop > 0
            && player_anm_tbl[dis_wk->anm_no] == AC_DIR_R){
          (*seq)++;
          break;
        }
        MMDL_SetAcmd(player_mmdl, player_anm_tbl[dis_wk->anm_no]);
        dis_wk->anm_no = (dis_wk->anm_no + 1) % NELEMS(player_anm_tbl);
        dis_wk->wait = dis_wk->wait_max;
        if(dis_wk->loop == 0){
          dis_wk->wait_max--;
          if(dis_wk->wait_max <= DISGUISE_ANM_WAIT_MIN){
            GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
            if(intcomm != NULL){
              IntrudeField_PlayerDisguise(intcomm, gsys, 
                dis_wk->mdata.cdata.obj_id[GAMEDATA_GetIntrudeMyID(gamedata)]);
            }
            dis_wk->loop++;
          }
        }
        else{
          if(dis_wk->wait_max < DISGUISE_ANM_WAIT_MAX){
            dis_wk->wait_max++;
          }
        }
      }
    }
    break;
	  
	case SEQ_FINISH:   //終了処理
    IntrudeEventPrint_ExitFieldMsg(&dis_wk->iem);

  	//侵入システムのアクションステータスを更新
  	if(intcomm != NULL){
    	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_FIELD);
    }
		return( GMEVENT_RES_FINISH );
	}
	
	return( GMEVENT_RES_CONTINUE );
}
