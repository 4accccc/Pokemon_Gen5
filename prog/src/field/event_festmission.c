//============================================================================================
/**
 * @file    event_festmission.c
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
#include "gamesystem/fest_mission_nw.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "field/field_fade.h"
#include "field/fmission_gimmick.h"
#include "./event_fieldmap_control.h"
#include "./event_festmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_festmission_event.h"

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
#define SYSWIN_SY       (16)

#define SMSG_WIN_PX     (1)
#define SMSG_WIN_PY     (20)
#define SMSG_WIN_SX     (28)
#define SMSG_WIN_SY     (4)

#define CWIN_STR_OX01  (0)
#define CWIN_STR_OX02  (8)
#define CWIN_STR_OY    (16)

#define EVFMISSION_SE_FADE_EFF  (SEQ_SE_W295_01)
#define EVFMISSION_SE_END_EFF   (SEQ_SE_PC_LOGOFF)

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
  FLDSYSWIN_STREAM* winStream;
  GFL_BMPWIN* bmpwin;
  GFL_MSGDATA* msgData;
  GFL_MSGDATA* msgFmis;
  WORDSET* wset;
  POWER_CONV_DATA* p_data;
  TIMEICON_WORK* timeIcon;

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION mission;

  BOOL  result;
  u32   res_score;
  u32   res_item_num;
  int   param;

  STRBUF* s_buf;
  STRBUF* s_tmp;
}FMISSION_EVWORK;

static GMEVENT_RESULT event_FestMissionStartMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FestMissionEndMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FestMissionCheckMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FestMissionRetireMain(GMEVENT * event, int *  seq, void * work);
static void sub_InitFMissionWork( FMISSION_EVWORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap );
static void sub_ReleaseFMissionWork( FMISSION_EVWORK* wk );
static void sub_FMissionParamDraw( FMISSION_EVWORK* wk, STRBUF* str, u8 px, u8 line );
static void sub_FMissionStartParamDraw( FMISSION_EVWORK* wk );
static void sub_FMissionEndParamDraw( FMISSION_EVWORK* wk );
static void sub_FMissionCheckParamDraw( FMISSION_EVWORK* wk );

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FestMissionStart( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  FMISSION_START_PARAM* param = (FMISSION_START_PARAM*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionStartMain, sizeof(FMISSION_EVWORK));
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
GMEVENT* EVENT_FestMissionEnd( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)work;

  event = GMEVENT_Create(gsys, NULL, event_FestMissionEndMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );

  //現ミッションパラメータを取得
  {
    FEST_MISSION* fmwk = LINK_FESTIVAL_GetMissionWork( wk->fest_sys );
    wk->mission = *fmwk;

    wk->result = LINK_FESTIVAL_GetMissionResult( wk->fest_sys, &wk->res_score, &wk->res_item_num );
  }
  return event;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション内容確認イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
GMEVENT* EVENT_FestMissionCheck( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionCheckMain, sizeof(FMISSION_EVWORK));
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
GMEVENT* EVENT_FestMissionRetire( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionRetireMain, sizeof(FMISSION_EVWORK));
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
GMEVENT* EVENT_FestMissionRetireForce( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionRetireMain, sizeof(FMISSION_EVWORK));
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
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );

  wk->gsys = gsys;
  wk->fieldmap = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fmb = FIELDMAP_GetFldMsgBG( wk->fieldmap );
  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldmap );
  
  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );

  wk->p_data = GPOWER_PowerData_LoadAlloc( wk->heapID );
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          NARC_script_message_festmission_event_dat, wk->heapID );
  wk->msgFmis = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
          NARC_message_fest_mission_dat, wk->heapID );
 
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
  GFL_MSG_Delete( wk->msgFmis );
  GFL_MSG_Delete( wk->msgData );
  GPOWER_PowerData_Unload( wk->p_data );

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
static GMEVENT_RESULT event_FestMissionStartMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    if( wk->succeed_f ){
      LINK_FESTIVAL_MissionStart( wk->fest_sys, (const FEST_MISSION*)&wk->mission, wk->succeed_f );
    }
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionStartParamDraw( wk );
#if 0 
    wk->winStream = FLDSYSWIN_STREAM_Add( wk->fmb, wk->msgData, SYSMSG_WIN_SX );
    GFL_MSG_GetString( wk->msgData, msg_fmission_start, wk->s_buf );
    FLDSYSWIN_STREAM_AllPrintStrBuf( wk->winStream, 0, 0, wk->s_buf );
    wk->timeIcon = TIMEICON_Create( GFUser_VIntr_GetTCBSYS(),
      FLDSYSWIN_STREAM_GetBmpWin( wk->winStream ), 15, TIMEICON_DEFAULT_WAIT, GFL_HEAP_LOWID(wk->heapID) );
#endif
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
#if 0
    TIMEICON_Exit( wk->timeIcon );
    FLDSYSWIN_STREAM_Delete( wk->winStream );
#endif    
    if( !wk->succeed_f ){
      GMEVENT* call_ev = EVENT_ChangeSubScreen( wk->gsys, wk->fieldmap, FIELD_SUBSCREEN_FMISSION_VIEW );
      GMEVENT_CallEvent( event, call_ev );
      (*seq) = 5;
      break;
    }
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, DISP_FADE_SPEED );
    PMSND_PlaySE( EVFMISSION_SE_FADE_EFF );

    (*seq)++;
    break;
  case 3:
    if( GFL_FADE_CheckFade() == FALSE ){
      FMISSION_GIMMICK_MapChangeSetup( wk->fest_sys, wk->gsys, FIELDMAP_GetZoneID( wk->fieldmap) );
      FIELD_SUBSCREEN_ChangeForce( FIELDMAP_GetFieldSubscreenWork( wk->fieldmap ), FIELD_SUBSCREEN_FMISSION_VIEW );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, DISP_FADE_SPEED );
      (*seq)++;
    }
    break;
  case 4:
    if( GFL_FADE_CheckFade() == FALSE ){
      (*seq)++;
    }
    break;
  case 5:
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
static GMEVENT_RESULT event_FestMissionEndMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    if( wk->result && wk->mission.life > 0 ){ //クリアでミッション終了した場合
      evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_end_clear );
    }else{  //タイムアップの場合
      evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_end_timeup );
    }
    PMSND_PlaySE( EVFMISSION_SE_END_EFF );
    (*seq)++;
    break;
  case 1:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionEndParamDraw( wk );

    if( wk->result ){
      GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM( wk->gsys, SEQ_ME_MISSION_CLEAR ));
    }else{
      GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM( wk->gsys, SEQ_ME_MISSION_FAILED ));
    }
    (*seq)++;
    break;
  case 2:
    if( PMSND_CheckPlayBGM() == FALSE ){
      LINK_FESTIVAL_MissionEnd( wk->fest_sys );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, DISP_FADE_SPEED );
      PMSND_PlaySE( EVFMISSION_SE_FADE_EFF );
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM( wk->gsys, FSND_FADE_NONE, FSND_FADE_FAST));
      (*seq)++;
    }
    break;
  case 3:
    if( GFL_FADE_CheckFade() == TRUE ){
      break;
    }
//  FMISSION_GIMMICK_MapChangeSetup( wk->fest_sys, wk->gsys, FIELDMAP_GetZoneID( wk->fieldmap) );
    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
    FMISSION_GIMMICK_Delete( wk->fest_sys, wk->gsys );
    FIELD_SUBSCREEN_ChangeForce( FIELDMAP_GetFieldSubscreenWork( wk->fieldmap ), FIELD_SUBSCREEN_NORMAL );
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, DISP_FADE_SPEED );
    (*seq)++;
    break;
  case 4:
#if 0
    {
      GMEVENT* call_ev = EVENT_ChangeSubScreen( wk->gsys, wk->fieldmap, FIELD_SUBSCREEN_NORMAL );
      GMEVENT_CallEvent( event, call_ev );
    }
#endif
    if( GFL_FADE_CheckFade() == TRUE ){
      break;
    }
    if( wk->result ){ //クリアしていればアイテム入手イベントコール
      (*seq)++;
    }else{
      (*seq) = 0xFF;  //終了
    }
    break;
  case 5:
    { //クリアしていればアイテム入手イベントコール
      s16 num;
      MYITEM_PTR item_sv = GAMEDATA_GetMyItem( wk->gdata );
      u32 max = MYITEM_GetItemMax( wk->mission.item );
      u16 now = MYITEM_GetItemNum( item_sv, wk->mission.item, GFL_HEAP_LOWID(wk->heapID) );
      GMEVENT* sc_event = SCRIPT_SetEventScript( wk->gsys, SCRID_ITEM_EVENT_FMISSION_CLEAR, NULL, HEAPID_FIELDMAP );

      SCRIPT_WORK* sc = SCRIPT_GetScriptWorkFromEvent( sc_event );

      num = max-now;
      if( num >= wk->res_item_num ){
        num = wk->res_item_num;  
      }
      SCRIPT_SetScriptWorkParam( sc, wk->mission.item, num, 0, 0 );
      GMEVENT_CallEvent( event, sc_event );
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
static GMEVENT_RESULT event_FestMissionCheckMain(GMEVENT * event, int *  seq, void * work)
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
static GMEVENT_RESULT event_FestMissionRetireMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* evwk = work;

  switch (*seq) {
  case 0:
    evcall_MsgPrint( evwk->gsys, event, evwk->fmb, evwk->msgData, msg_fmission_retire+evwk->param );
    (*seq)++;
    break;
  case 1:
    {
      FIELD_SUBSCREEN_MODE screen_mode = FIELD_SUBSCREEN_GetMode( FIELDMAP_GetFieldSubscreenWork( evwk->fieldmap ) );

      LINK_FESTIVAL_MissionEnd( evwk->fest_sys );

      if( screen_mode == FIELD_SUBSCREEN_FMISSION_VIEW ){
        GMEVENT* call_ev = EVENT_ChangeSubScreen( evwk->gsys, evwk->fieldmap, FIELD_SUBSCREEN_NORMAL );
        GMEVENT_CallEvent( event, call_ev );
      }
    }
    (*seq)++;
    break;
  default:
    sub_ReleaseFMissionWork( evwk );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------
/*
 *  @brief  ミッションパラメータ　ワードセット展開
 */
//-----------------------------------------------------------------
static void sub_FMissionParamWordExpand( FMISSION_EVWORK* wk )
{
  IWASAWA_Printf("Mission %d Time = %d\n",wk->mission.time);
  FEST_MISSION_WordsetParamAll( wk->wset, &wk->mission );
  WORDSET_RegisterNumber( wk->wset, 7, wk->res_score, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
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
 *  @brief  フェスタミッション発動時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionStartParamDraw( FMISSION_EVWORK* wk )
{
  int sec = LINKFEST_FRAME2SEC(wk->mission.life);

  //パラメータ
  FEST_MISSION_GetExpStr( wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
  
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );
  
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_start_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );

  //残り時間
  if( (sec / 60) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 6 );
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 7 );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション終了時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionEndParamDraw( FMISSION_EVWORK* wk )
{
  //パラメータ
  FEST_MISSION_GetExpStr( wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 5 );
  
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );
  
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_end_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  GFL_MSG_GetString( wk->msgData, msg_fmission_failed+wk->result, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
  GFL_MSG_GetString( wk->msgData, msg_fmission_score, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 3 );
  
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 8 );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッションパラメータ確認時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionCheckParamDraw( FMISSION_EVWORK* wk )
{
  //タイトル
  FEST_MISSION_GetTitleStr( wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  //説明文
  FEST_MISSION_GetExpStr( wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
 
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );
  
  //残り時間
  if( (wk->mission.life / (60*60)) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 6 );
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 7 );
}


