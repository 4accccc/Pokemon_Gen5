//============================================================================================
/**
 * @file    event_fmission.c
 * @brief	  イベント：フェスタミッション関連イベント
 * @author  iwasawa	
 * @date	  2010.12.15
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "system/time_icon.h"
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_data.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "field/field_fade.h"
#include "field/fes_gimmick.h"
#include "field/fld3d_ci.h"      //FIELD_ENCOUNT_CheckEncount
#include "field_encount.h"      //FIELD_ENCOUNT_CheckEncount
#include "effect_encount.h"

#include "./event_fieldmap_control.h"
#include "./event_fmission.h"
#include "./event_fmission_app.h"
#include "./event_subscreen.h"
#include "savedata/fmission_save.h"
#include "savedata/resonance_resort_sv.h"
#include "savedata/medalbox.h"
#include "app/mdlid_def.h"  // for MDLID_

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_fmission_event.h"

#include "../../../resource/fldmapdata/script/item_get_scr_def.h" //SCRID_～
#include "../../../resource/fldmapdata/script/usescript.h"  //DISP_FADE_SPEED

///////////////////////////////////////////////////////
//リテラル
///////////////////////////////////////////////////////
#define BUFLEN_TMP_MSG  (BUFLEN_FMISSION_EXP)

#define SYSMSG_WIN_SX   (19)
#define SYSWIN_PX       (1)
#define SYSWIN_PY       (1)
#define SYSWIN_SX       (30)
#define SYSWIN_SY       (18)

#define SMSG_WIN_PX     (1)
#define SMSG_WIN_PY     (20)
#define SMSG_WIN_SX     (28)
#define SMSG_WIN_SY     (4)

#define CWIN_STR_OX01  (0)
#define CWIN_STR_OX02  (8)
#define CWIN_STR_OY    (16)

#define EVFMISSION_SE_FADE_EFF  (SEQ_SE_W295_01)
#define EVFMISSION_SE_END_EFF   (SEQ_SE_SW_FES_05)

typedef struct _POWER_FLD_EFF{
  u8  ct;
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
}POWER_FLD_EFF;

typedef struct _MSGPRINT_EVWORK{
  FLDSYSWIN_STREAM* winStream;
}MSGPRINT_EVWORK;

typedef struct _FMISSION_EVWORK{
  u8  ct;
  u8  succeed_f;
  HEAPID  heapID;
  HEAPID  tmpHeapID;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  FLDMSGBG* fmb;
  FLDSYSWIN* win;
  FLDSYSWIN* msg_win;
  FLDMENUFUNC* fmenu;
  MMDLSYS * mmdl_sys;
  FES_GIMMICK * fes_gimmick;
  FLDSYSWIN_STREAM* winStream;
  GFL_BMPWIN* bmpwin;
  GFL_MSGDATA* msgData;
  WORDSET* wset;
  POWER_CONV_DATA* p_data;
  TIMEICON_WORK* timeIcon;

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION mission;
  FMISSION_DATA_MAN* fm_data_man;

  LINKFEST_MISSION_RESULT result;
  int   param;

  STRBUF* s_buf;
  STRBUF* s_tmp;
}FMISSION_EVWORK;

static GMEVENT_RESULT event_FMissionStartMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FMissionEndMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FMissionCheckMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FMissionRetireMain(GMEVENT * event, int *  seq, void * work);
static void sub_InitFMissionWork( FMISSION_EVWORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap );
static void sub_ReleaseFMissionWork( FMISSION_EVWORK* wk );
static void sub_FMissionParamDraw( FMISSION_EVWORK* wk, STRBUF* str, u8 px, u8 line );
static void sub_FMissionStartParamDraw( FMISSION_EVWORK* wk );
static void sub_FMissionCheckParamDraw( FMISSION_EVWORK* wk );

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FMissionStart( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  FMISSION_START_PARAM* param = (FMISSION_START_PARAM*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FMissionStartMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );

  //ミッションパラメータ取得
  wk->mission = param->mission; 
  wk->succeed_f = param->succeed_f;

  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション終了イベント
 *
 *  @param  work  FIELDMAP_WORK*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FMissionEnd( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)work;

  event = GMEVENT_Create(gsys, NULL, event_FMissionEndMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション内容確認イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FMissionCheck( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FMissionCheckMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  
  //ミッションパラメータ取得
  wk->mission = *fmwk; 

  return event;
}


//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッションリタイアイベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FMissionRetire( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FMissionRetireMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  wk->param = 0;
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション強制終了イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FMissionRetireForce( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FMissionRetireMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  wk->param = 1;
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  イベントワーク　共通初期化
 */
//----------------------------------------------------------------------
static void sub_InitFMissionWork( FMISSION_EVWORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap )
{
  wk->heapID = HEAPID_PROC;
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );

  wk->gsys = gsys;
  wk->fieldmap = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fmb = FIELDMAP_GetFldMsgBG( wk->fieldmap );
  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldmap );
  wk->fes_gimmick = FIELDMAP_GetFesGimmickSys( wk->fieldmap );

  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  
  wk->fm_data_man = FMISSION_DATA_Init( wk->heapID );
  wk->p_data = GPOWER_PowerData_LoadAlloc( wk->heapID );

  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          NARC_script_message_fmission_event_dat, wk->heapID );
   
  wk->wset = WORDSET_Create( wk->heapID );
 
  wk->s_buf = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->s_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  
  MMDLSYS_PauseMoveProc( wk->mmdl_sys );
}

//----------------------------------------------------------------------
/*
 *  @brief  イベントワーク　共通開放
 */
//----------------------------------------------------------------------
static void sub_ReleaseFMissionWork( FMISSION_EVWORK* wk )
{
  GFL_STR_DeleteBuffer( wk->s_tmp );
  GFL_STR_DeleteBuffer( wk->s_buf );
  WORDSET_Delete( wk->wset );
  GFL_MSG_Delete( wk->msgData );
  GPOWER_PowerData_Unload( wk->p_data );
  FMISSION_DATA_Release( wk->fm_data_man );

  MMDLSYS_ClearPauseMoveProc( wk->mmdl_sys );
}



//============================================================================================
//
//    サブイベント
//
//============================================================================================

//=======================================================
/*
 *  メッセージプリントイベント
 */
//=======================================================
static GMEVENT_RESULT event_MsgPrint(GMEVENT * event, int *  seq, void * work);

/*
 *  @brief  システムメッセージプリントイベント
 */
static void evcall_MsgPrint( GAMESYS_WORK * gsys, GMEVENT* parent, FLDMSGBG* fmb, GFL_MSGDATA* msgData, u16 msg_id )
{
  GMEVENT* event;
  MSGPRINT_EVWORK* evwk;

  event = GMEVENT_Create(gsys, NULL, event_MsgPrint, sizeof(MSGPRINT_EVWORK));
  evwk = GMEVENT_GetEventWork(event);

  evwk->winStream = FLDSYSWIN_STREAM_Add( fmb, msgData, SYSMSG_WIN_SX );
  FLDSYSWIN_STREAM_PrintStart( evwk->winStream, 0, 0, msg_id );
  
  //イベントコール取得
  GMEVENT_CallEvent( parent, event );
}

static GMEVENT_RESULT event_MsgPrint(GMEVENT * event, int *  seq, void * work)
{
  MSGPRINT_EVWORK* evwk = work;

  if( !FLDSYSWIN_STREAM_Print( evwk->winStream )){
    return GMEVENT_RES_CONTINUE;
  }
  FLDSYSWIN_STREAM_Delete( evwk->winStream );
  return GMEVENT_RES_FINISH;
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動メイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FMissionStartMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    {
      SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( wk->gdata ); 
      FMISSION_SAVE* f_save = SAVEDATA_GetFMissionSave( save );
      MEDALBOX * medalbox = SaveData_GetMedalBox( save );
      RTCDate date;

      if( wk->succeed_f ){  //他者ミッションに参加
        LINK_FESTIVAL_MissionStart( wk->fest_sys, (const FEST_MISSION*)&wk->mission, wk->succeed_f );
        FMISSION_SAVE_IncMissionEntryNum( f_save );
      }else{  //自分で開催
        u32 open_num = FMISSION_SAVE_IncMissionOpenNum( f_save );
       
        // 現在時刻を取得
        GFL_RTC_GetDate( &date );

        //開催回数でメダルセット
        if( open_num >= 30 ){
          MEDALBOX_SetTitleMedalGetFlg( medalbox, MDLID_FES_HOLD_LV1, date.year, date.month, date.day );
          if( open_num >= 50 ){
            MEDALBOX_SetTitleMedalGetFlg( medalbox, MDLID_FES_HOLD_LV2, date.year, date.month, date.day );
          }
        }
      }
      //ジョインアベニュー履歴セット
      RESORT_RECORD_AddID( RESORT_RECORD_ID_HILINKFES );
    }
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionStartParamDraw( wk );
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM( wk->gsys, SEQ_ME_MISSION_START ));
    (*seq)++;
    break;
  case 1:
    if( PMSND_CheckPlayBGM() == FALSE ){
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM( wk->gsys, FSND_FADE_NONE, FSND_FADE_FAST));
      (*seq)++;
    }
    break;
  case 2:
    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
    FES_GIMMICK_MissionInitSetup( FIELDMAP_GetFesGimmickSys( wk->fieldmap ) ); //最初の1回だけの初期化
    FES_GIMMICK_ZoneChangeSetup( wk->gsys, FIELDMAP_GetZoneID( wk->fieldmap) );
    if( !wk->succeed_f ){
      (*seq) = 6;
      break;
    }
    
    PMSND_PlaySE( EVFMISSION_SE_FADE_EFF );
    GMEVENT_CallEvent( event, EVENT_FieldFadeOut_White( wk->gsys, wk->fieldmap, FIELD_FADE_WAIT ) );
    (*seq)++;
    break;
  case 3:
    //エフェクトエンカウントを一度リセット
    EFFECT_ENC_EffectDelete( FIELDMAP_GetEncount( wk->fieldmap ) );

    FIELD_SUBSCREEN_ChangeForce( FIELDMAP_GetFieldSubscreenWork( wk->fieldmap ), FIELD_SUBSCREEN_FMISSION_VIEW );
    GMEVENT_CallEvent( event, EVENT_FieldClose( wk->gsys, wk->fieldmap ) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldOpen( wk->gsys ) );
    (*seq)++;
    break;
  case 5:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_White( wk->gsys, wk->fieldmap, FIELD_FADE_WAIT ) );
    (*seq)++;
    break;
  case 6:
    LINK_FESTIVAL_MissionLiveStatusSet( wk->fest_sys, TRUE );
    sub_ReleaseFMissionWork( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション終了メイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FMissionEndMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;
  FEST_BEACON* beacon = &wk->mission.beacon;

  switch( *seq ){
  case 0:
    {
      FLD3D_CI_PTR ci_ptr = FIELDMAP_GetFld3dCiPtr( wk->fieldmap );
      GMEVENT* call_event = FLD3D_CI_CreateCutInEvt( wk->gsys, ci_ptr, FLDCIID_FES_FINISH );
      GMEVENT_CallEvent( event, call_event );
    }
    (*seq)++;
    break;
  case 1:
    evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_end );
    (*seq)++;
    break;
  case 2:
    {
      FMISSION_APP_MODE mode = FMISSION_APP_MODE_RESULT;
      GMEVENT* call_event = GMEVENT_CreateOverlayEventCall( wk->gsys,
          FS_OVERLAY_ID( notwifi_event_fmission_app ), EVENT_FMissionApp, (void*)&mode );
      GMEVENT_CallEvent( event, call_event );
    }
    (*seq)++;
    break;
  default:
    sub_ReleaseFMissionWork( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション　パラメータチェックメイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FMissionCheckMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_check );
    (*seq)++;
    break;
  case 1:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionCheckParamDraw( wk );
    (*seq)++;
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)){
      (*seq)++;
    }
    break;
  default:
    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
    sub_ReleaseFMissionWork( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッションリタイアメイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FMissionRetireMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_retire+wk->param );
    (*seq)++;
    break;
  case 1:
    GMEVENT_CallEvent( event, EVENT_FieldFadeOut_White( wk->gsys, wk->fieldmap, FIELD_FADE_WAIT ) );
    (*seq)++;
    break;
  case 2:
    //ミッションギミック削除処理
    FES_GIMMICK_Delete( FIELDMAP_GetFesGimmickSys( wk->fieldmap ) );
    LINK_FESTIVAL_MissionEnd( wk->fest_sys ); //ミッション終了
    GMEVENT_CallEvent( event, EVENT_FieldClose( wk->gsys, wk->fieldmap ) );
    (*seq)++;
    break;
  case 3:
    GMEVENT_CallEvent( event, EVENT_FieldOpen( wk->gsys ) );
    (*seq)++;
    break;
  case 4:
    FIELD_SUBSCREEN_ChangeForce( FIELDMAP_GetFieldSubscreenWork( wk->fieldmap ), FIELD_SUBSCREEN_NORMAL );
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_White( wk->gsys, wk->fieldmap, FIELD_FADE_WAIT ) );
    (*seq)++;
    break;
  default:
    sub_ReleaseFMissionWork( wk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  ミッションパラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionParamDraw( FMISSION_EVWORK* wk, STRBUF* str, u8 px, u8 line )
{
  WORDSET_ExpandStr( wk->wset, wk->s_tmp, str );
  FLDSYSWIN_PrintStrBuf( wk->win, px, CWIN_STR_OY*line, wk->s_tmp );
}

//-----------------------------------------------------------------
/*
 *  @brief  ミッションパラメータ　書き込みコモン
 */
//-----------------------------------------------------------------
static void sub_FMissionParamDrawCommon( FMISSION_EVWORK* wk )
{
  int sec = LINKFEST_FRAME2SEC(wk->mission.beacon.life);
  
  //説明文
  FMISSION_DATA_GetExpStr( wk->fm_data_man, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
  
  //パラメータまとめてWordset
  FMISSION_DATA_WordsetParamAll( wk->wset, &wk->mission );

  //残り時間
  if( (sec / 60) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 7 );
  
  //報酬
  if( wk->mission.beacon.pay_bonus == FMISSION_PAY_BONUS_NONE ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_reward01, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_reward02, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 8 );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionStartParamDraw( FMISSION_EVWORK* wk )
{
//  int sec = LINKFEST_FRAME2SEC(wk->mission.beacon.life);
  
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_start_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );

  sub_FMissionParamDrawCommon( wk );

#if 0
  //説明文
  FMISSION_DATA_GetExpStr( wk->fm_data_man, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
  
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );

  //残り時間
  if( (sec / 60) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 7 );
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 8 );
#endif
}


//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッションパラメータ確認時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionCheckParamDraw( FMISSION_EVWORK* wk )
{
  //タイトル
  FMISSION_DATA_GetTitleStr( wk->fm_data_man, wk->s_buf, &wk->mission, wk->heapID );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );

  sub_FMissionParamDrawCommon( wk );
#if 0
  //説明文
  FMISSION_DATA_GetExpStr( wk->fm_data_man, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
 
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );
  
  //残り時間
  if( (wk->mission.beacon.life / (60*60)) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 7 );
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 8 );
#endif
}


