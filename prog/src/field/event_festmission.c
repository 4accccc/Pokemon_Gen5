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
#include "./event_fieldmap_control.h"
#include "./event_festmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_festmission_event.h"

#include "../../../resource/fldmapdata/script/gpower_scr_def.h" //SCRID_～

///////////////////////////////////////////////////////
//リテラル
///////////////////////////////////////////////////////
#define BUFLEN_TMP_MSG  (BUFLEN_FMISSION_EXP)

#define SYSMSG_WIN_SX   (19)
#define SYSWIN_PX       (4)
#define SYSWIN_PY       (1)
#define SYSWIN_SX       (24)
#define SYSWIN_SY       (22)

#define CWIN_STR_OX01  (0)
#define CWIN_STR_OX02  (8)
#define CWIN_STR_OY    (16)

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
  HEAPID  heapID;
  HEAPID  tmpHeapID;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  FLDMSGBG* fmb;
  FLDSYSWIN* win;
  FLDMENUFUNC* fmenu;
  MMDLSYS * mmdl_sys;
  FLDSYSWIN_STREAM* winStream;
  GFL_BMPWIN* bmpwin;
  GFL_MSGDATA* msgData;
  GFL_MSGDATA* msgFmis;
  WORDSET* wset;
  POWER_CONV_DATA* p_data;

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION mission;

  BOOL  result;
  u32   res_score;

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

/*
 *  @brief  フェスタミッション発動イベント
 *
 *  @param  work  FEST_MISSION*
 */
GMEVENT* EVENT_FestMissionStart( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionStartMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );

  //ミッションパラメータ取得
  wk->mission = *fmwk; 

  return event;
}

/*
 *  @brief  フェスタミッション終了イベント
 *
 *  @param  work  FIELDMAP_WORK*
 */
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

    wk->result = LINK_FESTIVAL_GetMissionResult( wk->fest_sys, &wk->res_score );
  }
  return event;
}

/*
 *  @brief  フェスタミッション内容確認イベント
 *
 *  @param  work  FEST_MISSION*
 */
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

/*
 *  @brief  フェスタミッションリタイアイベント
 *
 *  @param  work  FEST_MISSION*
 */
GMEVENT* EVENT_FestMissionRetire( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSION_EVWORK* wk;
  const FEST_MISSION* fmwk = (FEST_MISSION*)work;
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create(gsys, NULL, event_FestMissionRetireMain, sizeof(FMISSION_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  return event;
}

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
    evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_start );
    (*seq)++;
    break;
  case 1:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionStartParamDraw( wk );
    (*seq)++;
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)){
      (*seq)++;
    }
    break;
  case 3:
    LINK_FESTIVAL_MissionStart( wk->fest_sys, (const FEST_MISSION*)&wk->mission );

    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );

    {
      GMEVENT* call_ev = EVENT_ChangeSubScreen( wk->gsys, wk->fieldmap, FIELD_SUBSCREEN_FMISSION_VIEW );
      GMEVENT_CallEvent( event, call_ev );
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
 *  @brief  フェスタミッション終了メイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FestMissionEndMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSION_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    evcall_MsgPrint( wk->gsys, event, wk->fmb, wk->msgData, msg_fmission_end );
    (*seq)++;
    break;
  case 1:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionEndParamDraw( wk );
    (*seq)++;
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)){
      (*seq)++;
    }
    break;
  case 3:
    LINK_FESTIVAL_MissionEnd( wk->fest_sys );

    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
    {
      GMEVENT* call_ev = EVENT_ChangeSubScreen( wk->gsys, wk->fieldmap, FIELD_SUBSCREEN_NORMAL );
      GMEVENT_CallEvent( event, call_ev );
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
    evcall_MsgPrint( evwk->gsys, event, evwk->fmb, evwk->msgData, msg_fmission_retire );
    (*seq)++;
    break;
  case 1:
    LINK_FESTIVAL_MissionEnd( evwk->fest_sys );
    {
      GMEVENT* call_ev = EVENT_ChangeSubScreen( evwk->gsys, evwk->fieldmap, FIELD_SUBSCREEN_NORMAL );
      GMEVENT_CallEvent( event, call_ev );
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
//  IWASAWA_Printf("GPower %d Life = %d\n",g_power,time);
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
  //パラメータ
  FEST_MISSION_GetExpStr( wk->wset, wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX01, 2 );
  
  //パラメータまとめてWordset
  sub_FMissionParamWordExpand( wk );
  
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_start_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );

  //残り時間
  if( (wk->mission.life / (60*60)) == 0 ){
    GFL_MSG_GetString( wk->msgData, msg_fmission_life02, wk->s_buf );
  }else{
    GFL_MSG_GetString( wk->msgData, msg_fmission_life01, wk->s_buf );
  }
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 5 );
  //報酬
  GFL_MSG_GetString( wk->msgData, msg_fmission_reward, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, CWIN_STR_OX02, 6 );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション終了時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionEndParamDraw( FMISSION_EVWORK* wk )
{
  //パラメータ
  FEST_MISSION_GetExpStr( wk->wset, wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
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
  FEST_MISSION_GetTitleStr( wk->wset, wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  //説明文
  FEST_MISSION_GetExpStr( wk->wset, wk->msgFmis, wk->s_buf, &wk->mission, wk->heapID );
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


