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
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "./event_fieldmap_control.h"
#include "./event_festmission.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "msg/script/msg_festmission_event.h"

#include "../../../resource/fldmapdata/script/gpower_scr_def.h" //SCRID_～

///////////////////////////////////////////////////////
//リテラル
///////////////////////////////////////////////////////
#define BUFLEN_TMP_MSG  (64*2+EOM_SIZE)

#define SYSMSG_WIN_SX   (19)
#define SYSWIN_PX       (4)
#define SYSWIN_PY       (1)
#define SYSWIN_SX       (24)
#define SYSWIN_SY       (22)

#define CWIN_STR_OX01  (0)
#define CWIN_STR_OX02  (8)
#define CWIN_STR_OY    (16)

typedef struct POWER_FLD_EFF{
  u8  ct;
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
}POWER_FLD_EFF;

typedef struct FMISSIN_EVWORK{
  u8  ct;
  HEAPID  heapID;
  HEAPID  tmpHeapID;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldmap;

  FLDMSGBG* fmb;
  FLDSYSWIN* win;
  MMDLSYS * mmdl_sys;
  FLDSYSWIN_STREAM* winStream;
  GFL_BMPWIN* bmpwin;
  GFL_MSGDATA* msgData;
  WORDSET* wset;
  POWER_CONV_DATA* p_data;

  LINK_FESTIVAL* fest_sys;
  FEST_MISSION* mission;

  STRBUF* s_buf;
  STRBUF* s_tmp;
}FMISSIN_EVWORK;

static GMEVENT_RESULT event_FestMissionStartMain(GMEVENT * event, int *  seq, void * work);
static GMEVENT_RESULT event_FestMissionEndMain(GMEVENT * event, int *  seq, void * work);
static void sub_InitFMissionWork( FMISSIN_EVWORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap );
static void sub_ReleaseFMissionWork( FMISSIN_EVWORK* wk );
static void sub_FMissionParamDraw( FMISSIN_EVWORK* wk, STRBUF* str, u8 line );
static void sub_FMissionStartParamDraw( FMISSIN_EVWORK* wk );
static void sub_FMissionEndParamDraw( FMISSIN_EVWORK* wk );

/*
 *  @brief  フェスタミッション発動イベント
 *
 *  @param  work  FIELDMAP_WORK*
 */
GMEVENT* EVENT_FestMissionStart( GAMESYS_WORK * gsys, void* work )
{
  GMEVENT* event;
  FMISSIN_EVWORK* wk;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)work;

  event = GMEVENT_Create(gsys, NULL, event_FestMissionStartMain, sizeof(FMISSIN_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
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
  FMISSIN_EVWORK* wk;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)work;

  event = GMEVENT_Create(gsys, NULL, event_FestMissionEndMain, sizeof(FMISSIN_EVWORK));
  wk = GMEVENT_GetEventWork(event);

  sub_InitFMissionWork( wk, gsys, fieldmap );
  return event;
}

static void sub_InitFMissionWork( FMISSIN_EVWORK* wk, GAMESYS_WORK * gsys, FIELDMAP_WORK* fieldmap )
{
  wk->heapID = HEAPID_FIELDMAP;
  wk->tmpHeapID = GFL_HEAP_LOWID( wk->heapID );

  wk->gsys = gsys;
  wk->fieldmap = fieldmap;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fmb = FIELDMAP_GetFldMsgBG( wk->fieldmap );
  wk->mmdl_sys = FIELDMAP_GetMMdlSys( wk->fieldmap );

  wk->fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->mission = LINK_FESTIVAL_GetMissionWork( wk->fest_sys );

  wk->p_data = GPOWER_PowerData_LoadAlloc( wk->heapID );
  wk->msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
          NARC_script_message_festmission_event_dat, wk->heapID );
 
  wk->wset = WORDSET_Create( wk->heapID );
 
  wk->s_buf = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->s_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  
  MMDLSYS_PauseMoveProc( wk->mmdl_sys );
}

static void sub_ReleaseFMissionWork( FMISSIN_EVWORK* wk )
{
  GFL_STR_DeleteBuffer( wk->s_tmp );
  GFL_STR_DeleteBuffer( wk->s_buf );
  WORDSET_Delete( wk->wset );
  GFL_MSG_Delete( wk->msgData );
  GPOWER_PowerData_Unload( wk->p_data );

  MMDLSYS_ClearPauseMoveProc( wk->mmdl_sys );
}



//============================================================================================
//
//    サブイベント
//
//============================================================================================

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動メイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FestMissionStartMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSIN_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    wk->winStream = FLDSYSWIN_STREAM_Add( wk->fmb, wk->msgData, SYSMSG_WIN_SX );
    FLDSYSWIN_STREAM_PrintStart( wk->winStream, 0, 0, msg_fmission_start );
    (*seq)++;
    break;
  case 1:
    if( !FLDSYSWIN_STREAM_Print( wk->winStream )){
      break;
    }
    FLDSYSWIN_STREAM_Delete( wk->winStream );
    (*seq)++;
    break;
  case 2:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionStartParamDraw( wk );
    (*seq)++;
    break;
  case 3:
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
 *  @brief  フェスタミッション発動メイン
 */
//-----------------------------------------------------------------
static GMEVENT_RESULT event_FestMissionEndMain(GMEVENT * event, int *  seq, void * work)
{
  FMISSIN_EVWORK* wk = work;

  switch (*seq) {
  case 0:
    wk->winStream = FLDSYSWIN_STREAM_Add( wk->fmb, wk->msgData, SYSMSG_WIN_SX );
    FLDSYSWIN_STREAM_PrintStart( wk->winStream, 0, 0, msg_fmission_end );
    (*seq)++;
    break;
  case 1:
    if( !FLDSYSWIN_STREAM_Print( wk->winStream )){
      break;
    }
    FLDSYSWIN_STREAM_Delete( wk->winStream );
    (*seq)++;
    break;
  case 2:
    wk->win = FLDSYSWIN_AddEx( wk->fmb, wk->msgData, SYSWIN_PX, SYSWIN_PY, SYSWIN_SX, SYSWIN_SY );
    FLDSYSWIN_ClearWindow( wk->win );
    sub_FMissionEndParamDraw( wk );
    (*seq)++;
    break;
  case 3:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)){
      (*seq)++;
    }
    break;
  default:
    FEST_MISSION_End( wk->mission );

    FLDSYSWIN_ClearWindow( wk->win );
    FLDSYSWIN_Delete( wk->win );
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
static void sub_FMissionParamDraw( FMISSIN_EVWORK* wk, STRBUF* str, u8 line )
{
//  IWASAWA_Printf("GPower %d Life = %d\n",g_power,time);
  WORDSET_RegisterNumber( wk->wset, 0, wk->mission->time, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wk->wset, 1, wk->mission->prm[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wk->wset, 2, wk->mission->prm[1], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  WORDSET_ExpandStr( wk->wset, wk->s_tmp, str );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX02, CWIN_STR_OY*line, wk->s_tmp );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション発動時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionStartParamDraw( FMISSIN_EVWORK* wk )
{
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_start_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  //パラメータ
  GFL_MSG_GetString( wk->msgData, msg_fmission_exp00+wk->mission->id, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, 2 );
}

//-----------------------------------------------------------------
/*
 *  @brief  フェスタミッション終了時　パラメータ描画
 */
//-----------------------------------------------------------------
static void sub_FMissionEndParamDraw( FMISSIN_EVWORK* wk )
{
  //タイトル
  GFL_MSG_GetString( wk->msgData, msg_fmission_end_title, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*0, wk->s_buf );
  
  GFL_MSG_GetString( wk->msgData, msg_fmission_success, wk->s_buf );
  FLDSYSWIN_PrintStrBuf( wk->win, CWIN_STR_OX01, CWIN_STR_OY*2, wk->s_buf );
  
  //パラメータ
  GFL_MSG_GetString( wk->msgData, msg_fmission_exp00+wk->mission->id, wk->s_buf );
  sub_FMissionParamDraw( wk, wk->s_buf, 4 );
}

