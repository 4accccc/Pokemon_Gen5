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
#include "msg/msg_mission_msg.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "item/itemsym.h"
#include "event_intrude.h"
#include "field_comm\intrude_field.h"
#include "sound/pm_sndsys.h"
#include "event_mapchange.h"
#include "sound/pm_sndsys.h"

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
	INTRUDE_EVENT_DISGUISE_WORK iedw;
	
	u8 mission_view;
	u8 padding[3];
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
#if 0
	if(mdata->accept_netid != GAMEDATA_GetIntrudeMyID(gamedata)){
    //受注したのが自分で無いなら、まずミッション内容を表示
    dis_wk->mission_view = TRUE;
  }
#else //誰でも表示に変更　2010.02.01(月)
  dis_wk->mission_view = TRUE;
#endif

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
  	SEQ_ME_START,
  	SEQ_ME_WAIT,
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
    
    MISSIONDATA_Wordset(intcomm, &dis_wk->mdata.cdata, &dis_wk->mdata.target_info, 
      dis_wk->iem.wordset, dis_wk->heapID);
    {
      u16 explain_msgid, title_msgid;
      
      title_msgid = msg_mistype_000 + dis_wk->mdata.cdata.type;
      explain_msgid = dis_wk->mdata.cdata.msg_id_contents_monolith;
      
      IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&dis_wk->iem, title_msgid, 8, 0);
      IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&dis_wk->iem, explain_msgid, 8, 16);
    }
    (*seq)++;
    break;
  
  case SEQ_ME_START:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM(gsys, SEQ_ME_MISSION_START ));
    (*seq)++;
    break;
  case SEQ_ME_WAIT:
    if( PMSND_CheckPlayBGM() == FALSE ){
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_NONE, FSND_FADE_SHORT));
      (*seq)++;
    }
    break;
    
  case SEQ_MISSION_VIEW_WAIT:
    IntrudeEventPrint_ExitExtraMsgWin(&dis_wk->iem);
    (*seq)++;
    break;
  
  case SEQ_DISGUISE_START:
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
      NetID my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
      IntrudeEvent_Sub_DisguiseEffectSetup(&dis_wk->iedw, gsys, dis_wk->fieldWork, 
        dis_wk->mdata.cdata.obj_id[my_net_id], dis_wk->mdata.cdata.talk_type[my_net_id],
        dis_wk->mdata.cdata.obj_sex[my_net_id]);
      (*seq)++;
    }
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&dis_wk->iedw, intcomm) == TRUE){
      (*seq)++;
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

//==================================================================
/**
 * 変装演出：セットアップ
 *
 * @param   iedw		
 * @param   fieldWork		
 * @param   disguise_code		変装するOBJCODE
 *                  (DISGUISE_NO_NULLの場合は通常の姿、DISGUISE_NO_NORMALの場合はパレス時の標準姿)
 * @param   disguise_type   変装タイプ(TALK_TYPE_xxx)
 * @param   disguise_sex    変装後の性別
 */
//==================================================================
void IntrudeEvent_Sub_DisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 disguise_code, u8 disguise_type, u8 disguise_sex)
{
  GFL_STD_MemClear(iedw, sizeof(INTRUDE_EVENT_DISGUISE_WORK));
  iedw->gsys = gsys;
  iedw->fieldWork = fieldWork;
  iedw->disguise_code =disguise_code;
  iedw->disguise_type =disguise_type;
  iedw->disguise_sex =disguise_sex;
  iedw->wait_max = DISGUISE_ANM_WAIT_MAX;
  iedw->wait = iedw->wait_max;
}

//==================================================================
/**
 * 変装演出：メインシーケンス
 *
 * @param   iedw		
 * @param   intcomm 内部でNULLチェックはする為、NULLであってもこの関数を呼び続けてください
 *
 * @retval  BOOL		TRUE:シーケンス終了　FALSE:シーケンス継続中
 */
//==================================================================
BOOL IntrudeEvent_Sub_DisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm)
{
  switch(iedw->seq){
  case 0:
    iedw->wait_max = DISGUISE_ANM_WAIT_MAX;
    iedw->wait = iedw->wait_max;
    PMSND_PlaySE( SEQ_SE_FLD_103 );
    iedw->seq++;
    break;
  case 1:
    {
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(iedw->fieldWork);
      MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
      static const u8 player_anm_tbl[] = {
        AC_DIR_U, AC_DIR_L, AC_DIR_D, AC_DIR_R,
      };
      
      if(MMDL_CheckPossibleAcmd(player_mmdl) == TRUE){
        if(iedw->wait_max >= DISGUISE_ANM_WAIT_MAX 
            && iedw->loop > 0
            && player_anm_tbl[iedw->anm_no] == AC_DIR_R){
          iedw->seq++;
          break;
        }
        iedw->anm_no = (iedw->anm_no + 1) % NELEMS(player_anm_tbl);
        iedw->wait = iedw->wait_max;
        if(iedw->loop == 0){
          iedw->wait_max--;
          if(iedw->wait_max <= DISGUISE_ANM_WAIT_MIN){
            GAMEDATA *gamedata = GAMESYSTEM_GetGameData(iedw->gsys);
            IntrudeField_PlayerDisguise(intcomm, iedw->gsys, 
              iedw->disguise_code, iedw->disguise_type, iedw->disguise_sex);
            iedw->loop++;
          }
        }
        else{
          if(iedw->wait_max < DISGUISE_ANM_WAIT_MAX){
            iedw->wait_max++;
          }
        }
        MMDL_SetAcmd(player_mmdl, player_anm_tbl[iedw->anm_no]);
      }
    }
    break;
  default:
    return TRUE;
  }
  
  return FALSE;
}


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * 会話を始める時の最初の相手のリアクション待ちなどの処理を一括して行う
 *
 * @param   iem		
 * @param   seq		
 *
 * @retval  FIRST_TALK_RET		
 */
//--------------------------------------------------------------
FIRST_TALK_RET EVENT_INTRUDE_FirstTalkSeq(INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, u8 *seq)
{
  enum{
    SEQ_FIRST_TALK,
    SEQ_TALK_SEND,
    SEQ_TALK_WAIT,
    SEQ_TALK_RECV_WAIT,
    SEQ_TALK_NG,
    SEQ_TALK_NG_MSGWAIT,
    SEQ_TALK_NG_LAST,
    SEQ_END_OK,
    SEQ_END_NG,
    SEQ_END_CANCEL,
  };
  
  switch(*seq){
	case SEQ_FIRST_TALK:
    IntrudeEventPrint_StartStream(&ccew->iem, msg_intrude_000);
		(*seq)++;
		break;
	case SEQ_TALK_SEND:
	  if(IntrudeSend_Talk(intcomm, intcomm->talk.talk_netid, &ccew->mdata, ccew->intrude_talk_type) == TRUE){
      (*seq)++;
    }
    break;
	case SEQ_TALK_WAIT:
    if(IntrudeEventPrint_WaitStream(&ccew->iem) == TRUE){
			(*seq)++;
		}
		break;
	case SEQ_TALK_RECV_WAIT:
	  {
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_OK:
        *seq = SEQ_END_OK;
        break;
      case INTRUDE_TALK_STATUS_NG:
      case INTRUDE_TALK_STATUS_CANCEL:
        *seq = SEQ_TALK_NG;
        break;
      default:  //まだ返事が来ていない
        if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
          if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid) == TRUE){
            *seq = SEQ_END_CANCEL;
          }
        }
        break;
      }
    }
		break;

  case SEQ_TALK_NG:   //断られた
    IntrudeEventPrint_StartStream(&ccew->iem, msg_intrude_002);
		(*seq)++;
    break;
  case SEQ_TALK_NG_MSGWAIT:
    if(IntrudeEventPrint_WaitStream(&ccew->iem) == TRUE){
			(*seq)++;
		}
		break;
	case SEQ_TALK_NG_LAST:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
			(*seq) = SEQ_END_NG;
		}
	  break;
	
	case SEQ_END_OK:
	  return FIRST_TALK_RET_OK;
	case SEQ_END_NG:
	  return FIRST_TALK_RET_NG;
	case SEQ_END_CANCEL:
	  return FIRST_TALK_RET_CANCEL;
  default:
    GF_ASSERT(0);
    break;
  }
  
  return FIRST_TALK_RET_NULL;
}


//==================================================================
/**
 * 自分のパレスへワープする
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 *
 * エラー時の戻りとしても使用出来るようにintcommに依存していない
 */
//==================================================================
GMEVENT * EVENT_IntrudeWarpMyPalace(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  ZONEID warp_zone_id;
  VecFx32 pos;
  int palace_area;
  
  palace_area = GAMEDATA_GetIntrudeMyID(GAMESYSTEM_GetGameData(gsys));
  warp_zone_id = Intrude_GetPalaceTownZoneID(PALACE_TOWN_DATA_PALACE);
  Intrude_GetPalaceTownRandPos(PALACE_TOWN_DATA_PALACE, &pos, palace_area);
  
  event = EVENT_ChangeMapToPalace( gsys, warp_zone_id, &pos );
  return event;
}

#if 0

  u16 target_talk_battle[TALK_TYPE_MAX];    ///<自分がターゲットで戦闘状態の相手に話しかけた
  u16 target_not_talk[TALK_TYPE_MAX];       ///<自分がターゲットで相手が話しかけられない状態
  u16 mission_talk_battle[TALK_TYPE_MAX];   ///<自分がミッション実行者で戦闘状態の相手に話しかけた
  u16 mission_talk_challenger[TALK_TYPE_MAX];  ///<自分がミッション実行者で話しかけた相手も実行者
  u16 mission_not_talk[TALK_TYPE_MAX];      ///<自分がミッション実行者で相手が話しかけられない状態
  { //自分がターゲットで戦闘状態の相手に話しかけた
    mis_btl_01_t1,
    mis_btl_01_t2,
    mis_btl_01_t3,
    mis_btl_01_t4,
    mis_btl_01_t5,
  },
  { //自分がターゲットで相手が話しかけられない状態
    mis_std_01_t1,
    mis_std_01_t2,
    mis_std_01_t3,
    mis_std_01_t4,
    mis_std_01_t5,
  },
  { //自分がミッション実行者で戦闘状態の相手に話しかけた
    mis_btl_01_m1,
    mis_btl_01_m2,
    mis_btl_01_m3,
    mis_btl_01_m4,
    mis_btl_01_m5,
  },
  { //自分がミッション実行者で話しかけた相手も実行者
    mis_m01_04_m1,
    mis_m01_04_m2,
    mis_m01_04_m3,
    mis_m01_04_m4,
    mis_m01_04_m5,
  },
  { //自分がミッション実行者で相手が話しかけられない状態
    mis_std_01_m1,
    mis_std_01_m2,
    mis_std_01_m3,
    mis_std_01_m4,
    mis_std_01_m5,
  },
#endif
