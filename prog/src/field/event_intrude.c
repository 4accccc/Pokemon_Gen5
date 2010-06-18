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
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_work.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field\field_comm\intrude_snd_def.h"
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
#include "fieldmap/zone_id.h"
#include "field/fieldmap_call.h"  //FIELDMAP_IsReady
#include "system/main.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"

#include "palace_gimmick.h"
#include "field/scrcmd.h"

#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/palace01_def.h"  //for SCRID_～
#include "../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_
#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLD_EV_


//==============================================================================
//  定数定義
//==============================================================================
enum{
  DISGUISE_ANM_WAIT_MIN = 1,
  DISGUISE_ANM_WAIT_MAX = 15,

  DISGUISE_ANM_WAIT_BARRIER_MAX = 15,
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

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
typedef struct
{
  GFL_MSGDATA *msgData;
  FLDMSGWIN_STREAM *msgStream;
  
  INTRUDE_EVENT_DISGUISE_WORK iedw;
  
  MISSION_FORCEWARP_MSGID warp_talk;   ///<強制ワープ後に表示する会話タイプ
  BOOL search_sleep_warp;
}EVENT_FORCEWARP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT DisguiseEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT EventForceWarpMyPalace( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EventIntrudeNotWarp( GMEVENT* event, int* seq, void* wk );


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
    SEQ_MISSION_START_SEND,
    SEQ_MISSION_START_SEND_ANSWER_WAIT,
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
    
    MISSIONDATA_Wordset(&dis_wk->mdata.cdata, &dis_wk->mdata.target_info, 
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
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_NONE, FSND_FADE_FAST));
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
        (dis_wk->mdata.cdata.obj_sex_bit & (1 << my_net_id)));
      (*seq)++;
    }
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&dis_wk->iedw, intcomm) == TRUE){
      (*seq)++;
    }
    break;
	
	case SEQ_MISSION_START_SEND:
	  if(intcomm != NULL){
      if(IntrudeSend_MissionClientStart(intcomm, &intcomm->mission) == TRUE){
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
	case SEQ_MISSION_START_SEND_ANSWER_WAIT:
	  if(intcomm != NULL){
  	  if(MISSION_CheckMissionStartOK(&intcomm->mission) == TRUE){
        (*seq)++;
      }
    }
    else{
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
    if(FIELDMAP_IsReady(iedw->fieldWork) == TRUE){
      FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(iedw->fieldWork);
      MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
      static const u8 player_anm_tbl[] = {
        AC_DIR_U, AC_DIR_L, AC_DIR_D, AC_DIR_R,
      };
      
      if(MMDL_CheckPossibleAcmd(player_mmdl) == TRUE){
        if(iedw->wait_max >= DISGUISE_ANM_WAIT_MAX 
            && iedw->loop > 0
            && player_anm_tbl[iedw->anm_no] == AC_DIR_D){
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



//----------------------------------------------------------------------------
/**
 *	@brief  Barrierヘンシン セットアップ
 *
 * @param   iedw		
 * @param   fieldWork		
 * @param   disguise_code		変装するOBJCODE
 *                  (DISGUISE_NO_NULLの場合は通常の姿、DISGUISE_NO_NORMALの場合はパレス時の標準姿)
 * @param   disguise_type   変装タイプ(TALK_TYPE_xxx)
 * @param   disguise_sex    変装後の性別
 * @param   move_dir        移動方向
 * @param   change          変身の有無
 */
//-----------------------------------------------------------------------------
void IntrudeEvent_Sub_BarrierDisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, u16 disguise_code, u8 disguise_type, u8 disguise_sex, u16 move_dir, BOOL change )
{
  static MMDL_HEADER TMP_MDL_HEADER = {
    0,
    0,
    MV_DMY,
    0,
    0,
    0,
    DIR_DOWN,
    0,0,0,
    0,0,
    MMDL_HEADER_POSTYPE_GRID,
  };
  
  GFL_STD_MemClear(iedw, sizeof(INTRUDE_EVENT_DISGUISE_WORK));
  iedw->gsys = gsys;
  iedw->fieldWork = fieldWork;
  iedw->disguise_code =disguise_code;
  iedw->disguise_type =disguise_type;
  iedw->disguise_sex =disguise_sex;
  iedw->wait_max = DISGUISE_ANM_WAIT_BARRIER_MAX;
  iedw->wait = iedw->wait_max;
  iedw->dir = move_dir;

  // ダミーモデル生成
  if( change ){
    GAMEDATA* gamedata = GAMESYSTEM_GetGameData(gsys);
    MMDLSYS* mmdlsys = GAMEDATA_GetMMdlSys( gamedata );
    FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldWork );
    MMDL* fldmmdl = FIELD_PLAYER_GetMMdl( player );
    u16 zone_id = FIELDMAP_GetZoneID( fieldWork );
    
    // 見た目と位置を設定
    TMP_MDL_HEADER.obj_code = MMDL_GetOBJCode( fldmmdl );
    TMP_MDL_HEADER.dir      = move_dir;
    MMDLHEADER_SetGridPos( &TMP_MDL_HEADER, 
        MMDL_GetGridPosX(fldmmdl), MMDL_GetGridPosZ(fldmmdl), MMDL_GetVectorPosY(fldmmdl) );
    iedw->tmp_mdl = MMDLSYS_AddMMdl( mmdlsys, &TMP_MDL_HEADER, zone_id );

    // 自機の絵変更
    IntrudeField_PlayerDisguise(intcomm, iedw->gsys, 
      iedw->disguise_code, iedw->disguise_type, iedw->disguise_sex);

    // 主人公とスイッチ。
    MMDL_SetStatusBitVanish( fldmmdl, TRUE );
    MMDL_SetStatusBitVanish( iedw->tmp_mdl, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Barrierヘンシン処理
 *
 * @param   iedw		
 * @param   intcomm 内部でNULLチェックはする為、NULLであってもこの関数を呼び続けてください
 *
 * @retval  BOOL		TRUE:シーケンス終了　FALSE:シーケンス継続中
 */
//-----------------------------------------------------------------------------
BOOL IntrudeEvent_Sub_BarrierDisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm)
{
  enum{
    _SEQ_INIT,
    _SEQ_CHANGE,
    _SEQ_WAIT,
    _SEQ_FINISH,
  };
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( iedw->fieldWork );
  MMDL* fldmmdl = FIELD_PLAYER_GetMMdl( player );
  
  
  switch(iedw->seq){
  case _SEQ_INIT:
    {
      u16 code = (iedw->dir == DIR_RIGHT) ? AC_WALK_R_32F : AC_WALK_L_32F;
      VecFx32 pos;
      
      // 移動開始
      MMDL_SetAcmd(fldmmdl, code);
      if(iedw->tmp_mdl){
        MMDL_SetAcmd(iedw->tmp_mdl, code);
      }

      // BARRIER演出ON
      FIELD_PLAYER_GetPos( player, &pos );
      if( iedw->dir == DIR_RIGHT ){
        pos.x += FIELD_CONST_GRID_HALF_FX32_SIZE;
      }else{
        pos.x -= FIELD_CONST_GRID_HALF_FX32_SIZE;
      }
      PALACE_GMK_StartBarrierEffect( iedw->fieldWork, &pos );
    }

    // サウンドはいる？
    PMSND_PlaySE( SEQ_SE_FLD_132 );
    
    iedw->seq++;
    break;
  case _SEQ_CHANGE:
    iedw->wait_max --;
    if( iedw->wait_max <= 0 ){

      TOMOYA_Printf( "change \n" );

      // ヘンシン
      if( iedw->tmp_mdl ){
        MMDL_SetStatusBitVanish( fldmmdl, FALSE );
        MMDL_SetStatusBitVanish( iedw->tmp_mdl, TRUE );
      }
      iedw->seq++;
    }
    break;

  case _SEQ_WAIT:

    // BARRIER演出とモデル移動の終了待ち
    if( PALACE_GMK_IsBarrierEffect( iedw->fieldWork ) ){
      break;
    }
    
    if(MMDL_CheckEndAcmd( fldmmdl ) == TRUE){

      MMDL_EndAcmd( fldmmdl );
      if( iedw->tmp_mdl ){
        MMDL_EndAcmd( iedw->tmp_mdl );
      }
      iedw->seq++;
    }
    break;

  case _SEQ_FINISH:
    if( iedw->tmp_mdl ){
      
      // オブジェ破棄
      MMDL_Delete( iedw->tmp_mdl );
      iedw->tmp_mdl = NULL;
    }
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
    SEQ_TALK_RECV_WAIT,
    SEQ_TALK_NG,
    SEQ_TALK_NG_MSGWAIT,
    SEQ_TALK_NG_LAST,
    SEQ_OK_ANSWER,
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
	case SEQ_TALK_RECV_WAIT:
	  {
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_OK:
        *seq = SEQ_OK_ANSWER;
        break;
      case INTRUDE_TALK_STATUS_NG:
      case INTRUDE_TALK_STATUS_CANCEL:
        *seq = SEQ_TALK_NG;
        break;
      default:  //まだ返事が来ていない
        //キャンセルはメッセージが終わっていないと出来ない
        if(IntrudeEventPrint_WaitStream(&ccew->iem) == TRUE){
          if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
            if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid, intcomm->talk.talk_rand) == TRUE){
              Intrude_TalkRandClose(intcomm);
              *seq = SEQ_END_CANCEL;
            }
          }
        }
        break;
      }
    }
		break;

  case SEQ_TALK_NG:   //断られた
    if(IntrudeEventPrint_WaitStream(&ccew->iem) == TRUE){
      IntrudeEventPrint_StartStream(&ccew->iem, msg_intrude_002);
  		(*seq)++;
  	}
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
	
	case SEQ_OK_ANSWER:
	  if(IntrudeSend_TalkAnswer(intcomm, intcomm->talk.talk_netid, 
	      INTRUDE_TALK_STATUS_OK, intcomm->talk.talk_rand) == TRUE){
      *seq = SEQ_END_OK;
    }
    break;
	case SEQ_END_OK:
    if(IntrudeEventPrint_WaitStream(&ccew->iem) == TRUE){
  	  return FIRST_TALK_RET_OK;
  	}
  	break;
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
 * 現在自分がいるROMのパレスへワープする
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 *
 * エラー時の戻りとしても使用出来るようにintcommに依存していない
 * エラー時は自分のパレスへワープする
 */
//==================================================================
GMEVENT * EVENT_IntrudeWarpPalace(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  VecFx32 pos;
  
  IntrudeField_GetPalaceTownZoneID(ZONE_ID_PALACE01, &pos);
  event = EVENT_ChangeMapPalace_to_Palace( gsys, ZONE_ID_PALACE01, &pos );
  return event;
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
 * エラー時は自分のパレスへワープする
 */
//==================================================================
GMEVENT * EVENT_IntrudeWarpPalace_Mine(GAMESYS_WORK *gsys)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm != NULL){
    Intrude_SetMinePalaceArea(intcomm, GAMEDATA_GetIntrudeMyID(gamedata));
  }
  else{
    GAMEDATA_SetIntrudePalaceArea(gamedata, GAMEDATA_GetIntrudeMyID(gamedata));
  }
  return EVENT_IntrudeWarpPalace(gsys);
}

//==================================================================
/**
 * 指定したpalace_areaのパレスへワープする
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 *
 * エラー時の戻りとしても使用出来るようにintcommに依存していない
 * エラー時は自分のパレスへワープする
 */
//==================================================================
GMEVENT * EVENT_IntrudeWarpPalace_NetID(GAMESYS_WORK *gsys, int net_id)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm != NULL){
    Intrude_SetMinePalaceArea(intcomm, net_id);
  }
  else{
    GAMEDATA_SetIntrudePalaceArea(GAMESYSTEM_GetGameData(gsys), net_id);
  }
  return EVENT_IntrudeWarpPalace(gsys);
}

//==================================================================
/**
 * 自分のパレスへワープし、「不思議な力で戻された」を表示
 *
 * @param   gsys		    
 * @param   warp_talk		強制ワープ後に特別なメッセージを表示したい場合に指定
 *                      MISSION_FORCEWARP_MSGID_NULLを指定するとGameCommSys_GetLastStatusで
 *                      表示されるメッセージが決定
 *
 * @retval  GMEVENT *		
 *
 * エラー時の戻りとしても使用出来るようにintcommに依存していない
 * エラー時は自分のパレスへワープする
 */
//==================================================================
GMEVENT * EVENT_IntrudeForceWarpMyPalace(GAMESYS_WORK *gsys, MISSION_FORCEWARP_MSGID warp_talk)
{
  GMEVENT * event;
  EVENT_FORCEWARP *evf;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  event = GMEVENT_Create( gsys, NULL, EventForceWarpMyPalace, sizeof(EVENT_FORCEWARP) );
  evf = GMEVENT_GetEventWork(event);
  evf->warp_talk = warp_talk;
  
  {//intcommが生きている場合は下画面の更新停止
    INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
    if(intcomm != NULL){
      intcomm->subdisp_update_stop = TRUE;
    }
  }
  
  if(GAMESYSTEM_IsBatt10Sleep(gsys) == TRUE && GameCommSys_GetLastCommNo(game_comm) == GAME_COMM_NO_FIELD_BEACON_SEARCH){
    evf->search_sleep_warp = TRUE;
  }

  PMSND_PlaySE( INTSE_WARP );

  return event;
}

//--------------------------------------------------------------
/**
 * 自分のパレスへワープし、「不思議な力で戻された」を表示
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 *
 * エラー時の戻りとしても使用出来るようにintcommに依存していない
 * エラー時は自分のパレスへワープする
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventForceWarpMyPalace( GMEVENT* event, int* seq, void* wk )
{
  EVENT_FORCEWARP *evf = wk;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  enum{
    SEQ_WARP_MY_PALACE,
    SEQ_COMM_EXIT_REQ,
    SEQ_COMM_WAIT,
    SEQ_FIELD_INIT,
    SEQ_ME_WAIT,
    SEQ_MSG_SET,
    SEQ_MSG_WAIT,
    SEQ_DISGUISE_INIT,
    SEQ_DISGUISE_MAIN,
   	SEQ_TUTORIAL,
    SEQ_FINISH_BEFORE,
    SEQ_FINISH,
  };
  
  switch( *seq )
  {
  case SEQ_WARP_MY_PALACE:  //自分のパレスへワープ
    Intrude_SetPalaceMapNotConnect(game_comm);
    GMEVENT_CallEvent(event, EVENT_IntrudeWarpPalace_Mine(gsys) );
    (*seq)++;
    break;

  case SEQ_COMM_EXIT_REQ:
    if(NetErr_App_CheckError() || GameCommSys_CheckSystemWaiting(game_comm) == FALSE){
      //通信が接続されている場合は切断もする
      if(NetErr_App_CheckError() == NET_ERR_CHECK_NONE 
          && (GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION
              || GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_FIELD_BEACON_SEARCH)){
        GameCommSys_ExitReq(game_comm);
      }
      (*seq)++;
    }
    break;
  case SEQ_COMM_WAIT:
    if(NetErr_App_CheckError() || GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL){
      if(evf->search_sleep_warp == TRUE){
        *seq = SEQ_DISGUISE_INIT;
      }
      else{
        (*seq)++;
      }
    }
    break;
  
  case SEQ_FIELD_INIT:
    if(GAMESYSTEM_CheckFieldMapWork(gsys) == TRUE){
      BOOL mission_fail = FALSE;
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      if(FIELDMAP_IsReady(fieldWork) == TRUE){
        FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
        evf->msgData = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
        evf->msgStream = FLDMSGWIN_STREAM_AddTalkWin(msgBG, evf->msgData );
        
        {
          u32 msg_id;

          if(evf->warp_talk != MISSION_FORCEWARP_MSGID_NULL){
            switch(evf->warp_talk){
            case MISSION_FORCEWARP_MSGID_BATTLE_NG:
            case MISSION_FORCEWARP_MSGID_SHOP_NG:
              msg_id = plc_connect_15;
              break;
            default:
              GF_ASSERT(0);
              msg_id = plc_connect_12;
              break;
            }
            mission_fail = TRUE;  //今の所、失敗でしかこちらにこないのでここでセット
          }
          else{
            GAME_COMM_LAST_STATUS last_status = GameCommSys_GetLastStatus(game_comm);
            switch(last_status){
            case GAME_COMM_LAST_STATUS_INTRUDE_WAYOUT:           //誰かの退出による終了
              msg_id = plc_connect_13;
              break;
            case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_SUCCESS:  //ミッション成功で終了
              msg_id = plc_connect_09;
              break;
            case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_FAIL: //ミッション失敗で終了(相手に先を越された)
              msg_id = plc_connect_10;
              mission_fail = TRUE;
              break;
            case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_TIMEOUT://ミッション失敗で終了(タイムアウト)
              msg_id = plc_connect_11;
              mission_fail = TRUE;
              break;
            case GAME_COMM_LAST_STATUS_INTRUDE_ERROR:            //通信エラー
            default:
              msg_id = plc_connect_12;
              break;
            }
          }
          FLDMSGWIN_STREAM_PrintStart(evf->msgStream, 0, 0, msg_id);
          
          if(mission_fail == TRUE){
            GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM(gsys, SEQ_ME_MISSION_FAILED ));
            *seq = SEQ_ME_WAIT;
          }
          else{
            *seq = SEQ_MSG_SET;
          }
        }
      }
    }
    break;
  case SEQ_ME_WAIT:
    if( PMSND_CheckPlayBGM() == FALSE ){
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_NONE, FSND_FADE_FAST));
      GameCommInfo_MessageEntry_MissionFail(game_comm);
      *seq = SEQ_MSG_SET;
    }
    break;
  case SEQ_MSG_SET:
    if( FLDMSGWIN_STREAM_Print(evf->msgStream) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_MSG_WAIT:
    if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
      FLDMSGWIN_STREAM_Delete(evf->msgStream);
      GFL_MSG_Delete( evf->msgData );
      (*seq)++;
    }
    break;

  case SEQ_DISGUISE_INIT:  //変装をしていれば元に戻す
    {
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
      
      if(FIELDMAP_IsReady(fieldWork) == TRUE){
        FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        
        if(FIELD_PLAYER_CheckIllegalOBJCode( fld_player ) == FALSE){
          IntrudeEvent_Sub_DisguiseEffectSetup(&evf->iedw, gsys, fieldWork, DISGUISE_NO_NULL,0, 0);
          (*seq)++;
        }
        else{
          *seq = SEQ_FINISH_BEFORE;
        }
      }
    }
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&evf->iedw, NULL) == TRUE){
      if(evf->search_sleep_warp == TRUE){
        *seq = SEQ_FINISH_BEFORE;
      }
      else{
        (*seq)++;
      }
    }
    break;

  case SEQ_TUTORIAL:
    if(EVENTWORK_CheckEventFlag(
        GAMEDATA_GetEventWork(gamedata), SYS_FLAG_PALACE_MISSION_CLEAR) == FALSE){
      GAME_COMM_LAST_STATUS last_status = GameCommSys_GetLastStatus(game_comm);
      u32 scr_id = 0;
      switch(last_status){
      case GAME_COMM_LAST_STATUS_INTRUDE_WAYOUT:           //誰かの退出による終了
      case GAME_COMM_LAST_STATUS_INTRUDE_ERROR:            //通信エラー
      default:
        break;
      case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_SUCCESS:  //ミッション成功で終了
        scr_id = SCRID_PALACE01_OLDMAN_MISSION_FIRST_CLEAR;
        break;
      case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_FAIL: //ミッション失敗で終了(相手に先を越された)
      case GAME_COMM_LAST_STATUS_INTRUDE_MISSION_TIMEOUT:  //ミッション失敗で終了(タイムアウト)
        scr_id = SCRID_PALACE01_OLDMAN_MISSION_FIRST_FAIL;
        break;
      }
      SCRIPT_CallScript( event, scr_id, NULL, NULL, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    }
    *seq = SEQ_FINISH_BEFORE;
    break;
    
  case SEQ_FINISH_BEFORE: //終了前の最終処理
    GameCommSys_ClearLastStatus(game_comm);
    GameCommStatus_InitPlayerStatus(game_comm);
    
    //協力者orスリープ切断は表フィールドへ戻す
    if(GAMEDATA_GetIntrudeMyID(gamedata) != 0 || GAMESYSTEM_IsBatt10Sleep(gsys) == TRUE){
    	GMEVENT_CallEvent( event, EVENT_ChangeMapFromPalace(gsys) );
    }
    (*seq)++;
    break;
  case SEQ_FINISH:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * 「コネクト中のため、ワープできません」を表示するだけのイベント
 *
 * @param   gsys		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeNotWarp(GAMESYS_WORK *gsys)
{
  return GMEVENT_Create( gsys, NULL, EventIntrudeNotWarp, 0 );
}

//--------------------------------------------------------------
/**
 * 「コネクト中のため、ワープできません」を表示するだけのイベント
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventIntrudeNotWarp( GMEVENT* event, int* seq, void* wk )
{
  switch(*seq){
  case 0:
    SCRIPT_CallScript(event, SCRID_FLD_EV_WARP_FAILED_CONNECTING, NULL, NULL, HEAPID_FIELDMAP);
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
