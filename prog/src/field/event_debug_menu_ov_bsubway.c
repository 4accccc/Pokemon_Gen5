//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_bsubway.c
 *	@brief  デバッグメニューバトルサブウェイ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "event_mapchange.h"

//アプリ

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 
#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"

//外部公開
#include "event_debug_menu_ov_bsubway.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//======================================================================
//  デバッグメニュー　バトルサブウェイ
//======================================================================
#include "../../../resource/fldmapdata/script/bsubway_scr_def.h"

static const FLDMENUFUNC_HEADER DATA_DebugMenuList_BSubway =
{
  1,    //リスト項目数
  6,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

enum
{
  DEBUG_BSWAY_ZONE_HOME,
  DEBUG_BSWAY_ZONE_TRAIN,

  DEBUG_BSWAY_SCDATA_VIEW,
  DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD,
  DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD,
  DEBUG_BSWAY_WIFI_UPLOAD,

  DEBUG_BSWAY_ZONE_SINGLE,
  DEBUG_BSWAY_ZONE_DOUBLE,
  DEBUG_BSWAY_ZONE_MULTI,
  DEBUG_BSWAY_ZONE_WIFI,
  DEBUG_BSWAY_ZONE_S_SINGLE,
  DEBUG_BSWAY_ZONE_S_DOUBLE,
  DEBUG_BSWAY_ZONE_S_MULTI,
  DEBUG_BSWAY_AUTO_SINGLE,
  DEBUG_BSWAY_AUTO_DOUBLE,
  DEBUG_BSWAY_AUTO_MULTI,
  DEBUG_BSWAY_AUTO_S_SINGLE,
  DEBUG_BSWAY_AUTO_S_DOUBLE,
  DEBUG_BSWAY_AUTO_S_MULTI,
  DEBUG_BSWAY_BTL_SINGLE_7,
  DEBUG_BSWAY_BTL_DOUBLE_7,
  DEBUG_BSWAY_BTL_MULTI_7,
  DEBUG_BSWAY_BTL_SINGLE_21,
  DEBUG_BSWAY_BTL_DOUBLE_21,
  DEBUG_BSWAY_BTL_MULTI_21,
  DEBUG_BSWAY_BTL_S_SINGLE_49,
  DEBUG_BSWAY_BTL_S_DOUBLE_49,
  DEBUG_BSWAY_BTL_S_MULTI_49,

  DEBUG_BSWAY_SET_REGU_OFF,
  DEBUG_BSWAY_SET_BTL_SKIP,
  DEBUG_BSWAY_ANYSTAGE,
  DEBUG_BSWAY_CHANGE_WIFI_RANK,
  DEBUG_BSWAY_CLEAR_WIFI_ROOM,
  DEBUG_BSWAY_SELECT_TRAINER,

  DEBUG_BSWAY_COMM_MULTI_STAGE,
  DEBUG_BSWAY_S_COMM_MULTI_STAGE,
};

static const DEBUG_MENU_LIST DATA_BSubwayMenuList[] =
{
  { DEBUG_FIELD_BSW_01, (void*)DEBUG_BSWAY_ZONE_SINGLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_02, (void*)DEBUG_BSWAY_ZONE_DOUBLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_03, (void*)DEBUG_BSWAY_ZONE_MULTI, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_04, (void*)DEBUG_BSWAY_ZONE_WIFI, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_05, (void*)DEBUG_BSWAY_ZONE_S_SINGLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_06, (void*)DEBUG_BSWAY_ZONE_S_DOUBLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_07, (void*)DEBUG_BSWAY_ZONE_S_MULTI, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_08, (void*)DEBUG_BSWAY_ZONE_TRAIN, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_09, (void*)DEBUG_BSWAY_ZONE_HOME, GFL_OVERLAY_BLANK_ID },

  { DEBUG_FIELD_BSW_10, (void*)DEBUG_BSWAY_SCDATA_VIEW , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_WIFI_BSW_01, (void*)DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_WIFI_BSW_02, (void*)DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_WIFI_BSW_03, (void*)DEBUG_BSWAY_WIFI_UPLOAD , GFL_OVERLAY_BLANK_ID },

  { DEBUG_FIELD_BSW_26, (void*)DEBUG_BSWAY_SET_REGU_OFF, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_23, (void*)DEBUG_BSWAY_SET_BTL_SKIP, GFL_OVERLAY_BLANK_ID },

  { DEBUG_FIELD_BSW_11, (void*)DEBUG_BSWAY_AUTO_SINGLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_12, (void*)DEBUG_BSWAY_AUTO_DOUBLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_13, (void*)DEBUG_BSWAY_AUTO_MULTI, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_14, (void*)DEBUG_BSWAY_AUTO_S_SINGLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_15, (void*)DEBUG_BSWAY_AUTO_S_DOUBLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_16, (void*)DEBUG_BSWAY_AUTO_S_MULTI, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_17, (void*)DEBUG_BSWAY_BTL_SINGLE_7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_18, (void*)DEBUG_BSWAY_BTL_DOUBLE_7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_19, (void*)DEBUG_BSWAY_BTL_MULTI_7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_20, (void*)DEBUG_BSWAY_BTL_SINGLE_21, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_21, (void*)DEBUG_BSWAY_BTL_DOUBLE_21, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_22, (void*)DEBUG_BSWAY_BTL_MULTI_21, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_27, (void*)DEBUG_BSWAY_BTL_S_SINGLE_49, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_28, (void*)DEBUG_BSWAY_BTL_S_DOUBLE_49, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_29, (void*)DEBUG_BSWAY_BTL_S_MULTI_49, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_30, (void*)DEBUG_BSWAY_ANYSTAGE, GFL_OVERLAY_BLANK_ID },

  { DEBUG_FIELD_BSW_41, (void*)DEBUG_BSWAY_CHANGE_WIFI_RANK , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_42, (void*)DEBUG_BSWAY_CLEAR_WIFI_ROOM , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_45, (void*)DEBUG_BSWAY_SELECT_TRAINER , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_48, (void*)DEBUG_BSWAY_COMM_MULTI_STAGE , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_49, (void*)DEBUG_BSWAY_S_COMM_MULTI_STAGE , GFL_OVERLAY_BLANK_ID },
};

#define DEBUG_BSUBWAY_LIST_MAX ( NELEMS(DATA_BSubwayMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwayMenuData = {
  NARC_debug_message_d_field_dat,
  DEBUG_BSUBWAY_LIST_MAX,
  DATA_BSubwayMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};

static const DEBUG_MENU_LIST DATA_BSubwayAnyStageMenuList[] =
{
  { DEBUG_FIELD_BSW_31, (void*)BSWAY_MODE_SINGLE , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_32, (void*)BSWAY_MODE_DOUBLE , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_33, (void*)BSWAY_MODE_MULTI , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_34, (void*)BSWAY_MODE_S_SINGLE , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_35, (void*)BSWAY_MODE_S_DOUBLE , GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_36, (void*)BSWAY_MODE_S_MULTI , GFL_OVERLAY_BLANK_ID },
};

#define DEBUG_BSUBWAY_ANYSTAGE_LIST_MAX ( NELEMS(DATA_BSubwayAnyStageMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwayAnyStageMenuData = {
  NARC_debug_message_d_field_dat,
  DEBUG_BSUBWAY_ANYSTAGE_LIST_MAX,
  DATA_BSubwayAnyStageMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};

//DEBUG_BSUBWAY_EVENT_WORK
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;

  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  FLDSYSWIN *sysWin;
}DEBUG_BSUBWAY_EVENT_WORK;

typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;

  int play_mode;
  int game_round;
  int before_round;
  int key_repeat;
  int key_repeat_wait;

  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK;

static void debug_bsw_SetAuto( GAMESYS_WORK *gsys )
{
  u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( gsys );
  flag |= BSW_DEBUG_FLAG_AUTO;
  BSUBWAY_SCRWORK_DebugSetFlag( gsys, flag );
}

#define PAD_KEY_ALL (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

//バトルサブウェイデバッグメニュー派生　任意のステージに
static GMEVENT_RESULT debugMenuBSubwayAnyStageEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwayAnyStageMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      GMEVENT *next_event = NULL;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }

      work->play_mode = ret;
      (*seq)++;
      break;
    }
  case 2:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );

      work->strBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->strTempBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
          NARC_debug_message_d_field_dat );
      work->wordSet = WORDSET_Create( work->heapID );
      work->sysWin = FLDSYSWIN_AddTalkWin( msgbg, NULL );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->game_round = 1;
      work->before_round = 0xffff;
    }
    (*seq)++;
    break;
  case 3:
    {
      int add1 = 0, add10 = 0;
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();

      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        FLDSYSWIN_Delete( work->sysWin );
        FLDMSGWIN_Delete( work->msgWin );
        WORDSET_Delete( work->wordSet );
        GFL_MSG_Delete( work->msgData );
        GFL_STR_DeleteBuffer( work->strBuf );
        GFL_STR_DeleteBuffer( work->strTempBuf );

        if( trg & PAD_BUTTON_B ){
          return( GMEVENT_RES_FINISH );
        }

        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, work->play_mode );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, work->game_round );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      }

      if( cont && cont == work->key_repeat ){
        if( work->key_repeat_wait < 30*10 ){
          work->key_repeat_wait++;
        }
      }else{
        work->key_repeat_wait = 0;
      }

      work->key_repeat = cont;

      if( (trg & PAD_KEY_ALL) ){
        add1 = 1;
        add10 = 10;
      }else if( work->key_repeat ){
        int rep = work->key_repeat;
        int wait = work->key_repeat_wait;

        if( wait < 15 ){
          //none
        }else if( wait < 30*3 ){
          if( wait % 6 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*4 ){
          if( wait % 4 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*5 ){
          if( wait % 2 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*6 ){
          add1 = 1;
          add10 = 10;
        }else if( wait < 30*7 ){
          add1 = 2;
          add10 = 20;
        }else if( wait < 30*8 ){
          add1 = 4;
          add10 = 40;
        }else if( wait < 30*9 ){
          add1 = 8;
          add10 = 80;
        }else if( wait >= 30*9 ){
          add1 = 16;
          add10 = 160;
        }
      }

      if( add1 || add10 ){
        int check = trg & PAD_KEY_ALL;

        if( check == 0 ){
          check = cont;
        }

        if( (check & PAD_KEY_UP) ){
          work->game_round -= add1;
        }else if( (check & PAD_KEY_DOWN) ){
          work->game_round += add1;
        }else if( (check & PAD_KEY_LEFT) ){
          work->game_round -= add10;
        }else if( (check & PAD_KEY_RIGHT) ){
          work->game_round += add10;
        }
      }

      if( work->game_round <= 0 ){
        work->game_round = 1;
      }else if( work->game_round > 69986+7 ){
        work->game_round = 69986+7;
      }

      switch( work->play_mode ){
      case BSWAY_MODE_SINGLE:
      case BSWAY_MODE_DOUBLE:
      case BSWAY_MODE_MULTI:
        if( work->game_round > 21 ){
          work->game_round = 21;
        }
      }

      if( work->game_round != work->before_round ){
        work->before_round = work->game_round;
        STRTOOL_SetNumber(
            work->strBuf, work->game_round, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );

        {
          int stage = 0,round = 0;

          stage = work->game_round / 7;
          round = work->game_round % 7;

          if( work->game_round % 7 ){
            stage++;
          }

          if( round == 0 ){
            round = 7;
          }

          STRTOOL_SetNumber( work->strTempBuf, work->game_round, 5,
              STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 0, work->strTempBuf, 0, TRUE, PM_LANG );
          STRTOOL_SetNumber( work->strTempBuf, stage, 5,
              STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 1, work->strTempBuf, 0, TRUE, PM_LANG );
          STRTOOL_SetNumber( work->strTempBuf, round, 1,
              STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 2, work->strTempBuf, 0, TRUE, PM_LANG );

          GFL_MSG_GetString(
              work->msgData, DEBUG_FIELD_BSW_40, work->strTempBuf );
          WORDSET_ExpandStr(
              work->wordSet, work->strBuf, work->strTempBuf );
          FLDSYSWIN_ClearWindow( work->sysWin );
          FLDSYSWIN_PrintStrBuf( work->sysWin, 1, 0, work->strBuf );
        }
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

static void debugMenuCallProc_BSubwayAnyStage(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK *work;

  GMEVENT_Change( event, debugMenuBSubwayAnyStageEvent,
      sizeof(DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK) );

  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
}


static const DEBUG_MENU_LIST DATA_BSubwaySelectTrainerMenuList[] =
{
  { DEBUG_FIELD_BSW_31, (void*)BSWAY_MODE_SINGLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_32, (void*)BSWAY_MODE_DOUBLE, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_BSW_33, (void*)BSWAY_MODE_MULTI, GFL_OVERLAY_BLANK_ID },
};

#define DEBUG_BSUBWAY_SELECT_TRAINER_LIST_MAX ( NELEMS(DATA_BSubwaySelectTrainerMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwaySelectTrainerMenuData = {
  NARC_debug_message_d_field_dat,
  DEBUG_BSUBWAY_SELECT_TRAINER_LIST_MAX,
  DATA_BSubwaySelectTrainerMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};

typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;

  int play_mode;
  int select_trainer_num;
  int select_trainer_max;
  u16 trainer_no[2];
  int key_repeat;
  int key_repeat_wait;

  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSUBWAY_SELECT_TRAINER_EVENT_WORK;

//バトルサブウェイデバッグメニュー派生　トレーナーを選択
static GMEVENT_RESULT debugMenuBSubwaySelectTrainerEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_SELECT_TRAINER_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwaySelectTrainerMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      GMEVENT *next_event = NULL;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }

      work->play_mode = ret;
      (*seq)++;
      break;
    }
  case 2:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );

      work->strBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->strTempBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
          NARC_debug_message_d_field_dat );
      work->wordSet = WORDSET_Create( work->heapID );
      work->sysWin = FLDSYSWIN_AddTalkWin( msgbg, NULL );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->select_trainer_num = 0;
      work->trainer_no[0] = 0;
      work->trainer_no[1] = 0;
      if( work->play_mode == BSWAY_MODE_MULTI ) {
        work->select_trainer_max = 2;
      }
      else {
        work->select_trainer_max = 1;
      }
    }
    (*seq)++;
    break;

  case 3:
    STRTOOL_SetNumber(
        work->strBuf, 1, 5, // 1origineで表示
        STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
    FLDMSGWIN_ClearWindow( work->msgWin );
    FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );

    if( work->play_mode == BSWAY_MODE_MULTI ) {
      WORDSET_RegisterNumber( work->wordSet, 0, work->trainer_no[0]+1, 3,  // 1origineで表示
          STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( work->wordSet, 1, work->trainer_no[1]+1, 3,  // 1origineで表示
          STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      GFL_MSG_GetString( work->msgData, DEBUG_FIELD_BSW_47, work->strTempBuf );
      WORDSET_ExpandStr( work->wordSet, work->strBuf, work->strTempBuf );
    }
    else {
      WORDSET_RegisterNumber( work->wordSet, 0, work->trainer_no[0]+1, 3,  // 1origineで表示
          STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      GFL_MSG_GetString( work->msgData, DEBUG_FIELD_BSW_46, work->strTempBuf );
      WORDSET_ExpandStr( work->wordSet, work->strBuf, work->strTempBuf );
    }
    FLDSYSWIN_ClearWindow( work->sysWin );
    FLDSYSWIN_PrintStrBuf( work->sysWin, 1, 0, work->strBuf );
    (*seq)++;
    break;

  case 4:
    {
      int add1 = 0, add10 = 0;
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int before_trainer_no = work->trainer_no[ work->select_trainer_num ];
      int after_trainer_no = work->trainer_no[ work->select_trainer_num ];

      if( trg & PAD_BUTTON_B ){
        FLDSYSWIN_Delete( work->sysWin );
        FLDMSGWIN_Delete( work->msgWin );
        WORDSET_Delete( work->wordSet );
        GFL_MSG_Delete( work->msgData );
        GFL_STR_DeleteBuffer( work->strBuf );
        GFL_STR_DeleteBuffer( work->strTempBuf );
        return( GMEVENT_RES_FINISH );
      } 
      else if( trg & PAD_BUTTON_A ){ 
        PMSND_PlaySE( SEQ_SE_DECIDE1 );
        (work->select_trainer_num)++;
        if( work->select_trainer_max <= work->select_trainer_num ) {
          FLDSYSWIN_Delete( work->sysWin );
          FLDMSGWIN_Delete( work->msgWin );
          WORDSET_Delete( work->wordSet );
          GFL_MSG_Delete( work->msgData );
          GFL_STR_DeleteBuffer( work->strBuf );
          GFL_STR_DeleteBuffer( work->strTempBuf );

          BSUBWAY_SCRWORK_DebugCreateWorkTrNo( work->gmSys, work->play_mode, work->trainer_no );
          SCRIPT_ChangeScript( event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        } 
        return( GMEVENT_RES_CONTINUE );
      }

      if( cont && cont == work->key_repeat ){
        if( work->key_repeat_wait < 30*10 ){
          work->key_repeat_wait++;
        }
      }else{
        work->key_repeat_wait = 0;
      }

      work->key_repeat = cont;

      if( (trg & PAD_KEY_ALL) ){
        add1 = 1;
        add10 = 10;
      }else if( work->key_repeat ){
        int rep = work->key_repeat;
        int wait = work->key_repeat_wait;

        if( wait < 15 ){
          //none
        }else if( wait < 30*3 ){
          if( wait % 6 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*4 ){
          if( wait % 4 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*5 ){
          if( wait % 2 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*6 ){
          add1 = 1;
          add10 = 10;
        }else if( wait < 30*7 ){
          add1 = 2;
          add10 = 20;
        }else if( wait < 30*8 ){
          add1 = 4;
          add10 = 40;
        }else if( wait < 30*9 ){
          add1 = 8;
          add10 = 80;
        }else if( wait >= 30*9 ){
          add1 = 16;
          add10 = 160;
        }
      }

      if( add1 || add10 ){
        int check = trg & PAD_KEY_ALL;

        if( check == 0 ){
          check = cont;
        }

        if( (check & PAD_KEY_UP) ){
          after_trainer_no -= add1;
        }else if( (check & PAD_KEY_DOWN) ){
          after_trainer_no += add1;
        }else if( (check & PAD_KEY_LEFT) ){
          after_trainer_no -= add10;
        }else if( (check & PAD_KEY_RIGHT) ){
          after_trainer_no += add10;
        }
      }

      if( after_trainer_no < 0 ){
        after_trainer_no = BSW_TR_DATANO_MAX - 1;
      }
      else if( BSW_TR_DATANO_MAX <= after_trainer_no){
        after_trainer_no = 0;
      }

      if( after_trainer_no != before_trainer_no ){
        work->trainer_no[ work->select_trainer_num ] = after_trainer_no;

        STRTOOL_SetNumber(
            work->strBuf, after_trainer_no+1, 5, // 1origineで表示
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );

        if( work->play_mode == BSWAY_MODE_MULTI ) {
          WORDSET_RegisterNumber( work->wordSet, 0, work->trainer_no[0]+1, 3,  // 1origineで表示
              STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterNumber( work->wordSet, 1, work->trainer_no[1]+1, 3,  // 1origineで表示
              STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          GFL_MSG_GetString( work->msgData, DEBUG_FIELD_BSW_47, work->strTempBuf );
          WORDSET_ExpandStr( work->wordSet, work->strBuf, work->strTempBuf );
        }
        else {
          WORDSET_RegisterNumber( work->wordSet, 0, work->trainer_no[0]+1, 3,  // 1origineで表示
              STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          GFL_MSG_GetString( work->msgData, DEBUG_FIELD_BSW_46, work->strTempBuf );
          WORDSET_ExpandStr( work->wordSet, work->strBuf, work->strTempBuf );
        }
        FLDSYSWIN_ClearWindow( work->sysWin );
        FLDSYSWIN_PrintStrBuf( work->sysWin, 1, 0, work->strBuf );
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

static void debugMenuCallProc_BSubwaySelectTrainer(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSUBWAY_SELECT_TRAINER_EVENT_WORK *work;

  GMEVENT_Change( event, debugMenuBSubwaySelectTrainerEvent,
      sizeof(DEBUG_BSUBWAY_SELECT_TRAINER_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_SELECT_TRAINER_EVENT_WORK) );

  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
}


static BOOL bsubway_CheckBattleZoneID( DEBUG_BSUBWAY_EVENT_WORK *work )
{
  u16 zone_id = FIELDMAP_GetZoneID( work->fieldWork );

  if( ZONEDATA_IsBattleSubway(zone_id) == TRUE ){
    if( zone_id != ZONE_ID_C04R0110 &&
        zone_id != ZONE_ID_C04R0111 ){
      return( TRUE );
    }
  }

  FLDSYSWIN_ClearWindow( work->sysWin );
  FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_37 );
  return( FALSE );
}

//バトルサブウェイデバッグメニュー派生　wifiランク変更
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;

  int rank;
  int before_rank;

  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSW_CHANGE_WIFI_RANK;

static GMEVENT_RESULT debugMenuBSubwayChangeWifiRank(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSW_CHANGE_WIFI_RANK *work = wk;

  switch( (*seq) ){
  case 0:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->strBuf = GFL_STR_CreateBuffer( 8, work->heapID );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->rank = BSUBWAY_SCRWORK_DebugGetWifiRank( work->gmSys );
      work->before_rank = -1;
    }
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int repeat = GFL_UI_KEY_GetRepeat();

      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        FLDMSGWIN_Delete( work->msgWin );
        GFL_STR_DeleteBuffer( work->strBuf );

        if( trg & PAD_BUTTON_A ){
          BSUBWAY_SCRWORK_DebugSetWifiRank( work->gmSys, work->rank );
        }

        return( GMEVENT_RES_FINISH );
      }

      if( repeat & PAD_KEY_UP ){
        work->rank--;
        if( work->rank < 1 ){ work->rank = 1; }
      }else if( repeat & PAD_KEY_DOWN ){
        work->rank++;
        if( work->rank > 10 ){ work->rank = 10; }
      }else if( repeat & PAD_KEY_LEFT ){
        work->rank -= 2;
        if( work->rank < 1 ){ work->rank = 1; }
      }else if( repeat & PAD_KEY_RIGHT ){
        work->rank += 2;
        if( work->rank > 10 ){ work->rank = 10; }
      }

      if( work->rank != work->before_rank ){
        work->before_rank = work->rank;
        STRTOOL_SetNumber( work->strBuf, work->rank, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

static void debugMenuCallProc_BSubwayChangeWifiRank(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSW_CHANGE_WIFI_RANK *work;

  GMEVENT_Change( event, debugMenuBSubwayChangeWifiRank,
      sizeof(DEBUG_BSW_CHANGE_WIFI_RANK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSW_CHANGE_WIFI_RANK) );

  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
}

//バトルサブウェイデバッグメニュー派生　通信マルチ周回数変更
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;

  int stage;
  int before_stage;
  u16 play_mode;

  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSW_SELECT_COMM_MULTI_STAGE;

static GMEVENT_RESULT debugMenuBSubwaySelectCommMultiStage(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSW_SELECT_COMM_MULTI_STAGE *work = wk;

  switch( (*seq) ){
  case 0:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->strBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->strTempBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->stage = 0;
      work->before_stage = -1;

      work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
          NARC_debug_message_d_field_dat );
      work->wordSet = WORDSET_Create( work->heapID );
      work->sysWin = FLDSYSWIN_AddTalkWin( msgbg, NULL );
    }
    (*seq)++;
    break;
  case 1:
    {
      int add = 0,stage = 0;
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int repeat = GFL_UI_KEY_GetRepeat();

      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        FLDMSGWIN_Delete( work->msgWin );
        FLDSYSWIN_Delete( work->sysWin );
        WORDSET_Delete( work->wordSet );
        GFL_MSG_Delete( work->msgData );
        GFL_STR_DeleteBuffer( work->strBuf );
        GFL_STR_DeleteBuffer( work->strTempBuf );

        if( trg & PAD_BUTTON_A ){
          BSUBWAY_SCRWORK_DebugSetCommMultiStage(
              work->gmSys, work->play_mode, work->stage );
        }

        return( GMEVENT_RES_FINISH );
      }

      if( repeat & PAD_KEY_UP ){
        add = -1;
      }else if( repeat & PAD_KEY_DOWN ){
        add = 1;
      }else if( repeat & PAD_KEY_LEFT ){
        add = -10;
      }else if( repeat & PAD_KEY_RIGHT ){
        add = 10;
      }

      stage = work->stage;
      stage += add;

      if( work->play_mode == BSWAY_MODE_COMM_MULTI ){
        if( stage > 2 ){
          stage = 2;
        }else if( stage < 0 ){
          stage = 0;
        }
      }else{ //super
        if( stage > 9998 ){ //9999...
          stage -= 9999;
        }else if( stage < 0 ){ //-1...
          stage += 9999;
        }
      }

      work->stage = stage;

      if( work->stage != work->before_stage ){
        work->before_stage = work->stage;
        STRTOOL_SetNumber( work->strBuf, work->stage+1, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );

        STRTOOL_SetNumber( work->strTempBuf, work->stage+1, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterWord(
            work->wordSet, 0, work->strTempBuf, 0, TRUE, PM_LANG );
        STRTOOL_SetNumber( work->strTempBuf, work->stage*7, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterWord(
            work->wordSet, 1, work->strTempBuf, 0, TRUE, PM_LANG );
        GFL_MSG_GetString(
            work->msgData, DEBUG_FIELD_BSW_50, work->strTempBuf );
        WORDSET_ExpandStr(
            work->wordSet, work->strBuf, work->strTempBuf );
        FLDSYSWIN_ClearWindow( work->sysWin );
        FLDSYSWIN_PrintStrBuf( work->sysWin, 1, 0, work->strBuf );
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

static void debugMenuCallProc_BSubwaySelectCommMulti(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk, u16 play_mode )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSW_SELECT_COMM_MULTI_STAGE *work;

  GMEVENT_Change( event, debugMenuBSubwaySelectCommMultiStage,
      sizeof(DEBUG_BSW_SELECT_COMM_MULTI_STAGE) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSW_SELECT_COMM_MULTI_STAGE) );

  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
  work->play_mode = play_mode;
}

//イベント：バトルサブウェイデバッグメニュー
static GMEVENT_RESULT debugMenuBSubwayEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
        NARC_debug_message_d_field_dat );
    work->sysWin = FLDSYSWIN_AddTalkWin( work->fldMsgBG, work->msgData );
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwayMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      BOOL chg_event = FALSE;
      VecFx32 receipt_map_pos = {GRID_SIZE_FX32(9),0,GRID_SIZE_FX32(12)};
      VecFx32 home_map_pos = {GRID_SIZE_FX32(27),0,GRID_SIZE_FX32(15)};
      GFL_MSGDATA *msgData = work->msgData;
      FLDSYSWIN *sysWin = work->sysWin;
      FLDMENUFUNC *menuFunc = work->menuFunc;
      GMEVENT *next_event = NULL;
      EV_WIFIBSUBWAY_PARAM wifi_bsubway_param;

      ret = FLDMENUFUNC_ProcMenu( menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        FLDSYSWIN_Delete( sysWin );
        FLDMENUFUNC_DeleteMenu( menuFunc );
        GFL_MSG_Delete( msgData );
        return( GMEVENT_RES_FINISH );
      }

      switch( ret ){
      case DEBUG_BSWAY_ZONE_HOME: //ホームへ移動
        param = ZONE_ID_C04R0111;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &home_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_TRAIN: //車内へ移動
        param = ZONE_ID_C04R0110;
        next_event = DEBUG_EVENT_QuickChangeMapDefaultPos(
            work->gmSys, work->fieldWork, param );
        break;
      case DEBUG_BSWAY_SCDATA_VIEW: //歴代リーダー表示
        next_event = BSUBWAY_EVENT_CallLeaderBoard( work->gmSys );
        break;
      case DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD: //wifi データダウンロード
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;

        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
            FS_OVERLAY_ID(event_wifibsubway), 
            WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
        break;
      case DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD:  //wif 履歴ダウンロード
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;

        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
            FS_OVERLAY_ID(event_wifibsubway), 
            WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
        break;
      case DEBUG_BSWAY_WIFI_UPLOAD: //wifi データアップロード
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_SCORE_UPLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;

        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
            FS_OVERLAY_ID(event_wifibsubway), 
            WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
        break;
      case DEBUG_BSWAY_ZONE_SINGLE: //シングル受付へ
        param = ZONE_ID_C04R0102;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_DOUBLE: //ダブル受付へ
        param = ZONE_ID_C04R0104;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_MULTI: //マルチ受付へ
        param = ZONE_ID_C04R0106;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_WIFI: //WiFi受付へ
        param = ZONE_ID_C04R0108;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_SINGLE: //Sシングル受付へ
        param = ZONE_ID_C04R0103;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_DOUBLE: //Sダブル受付へ
        param = ZONE_ID_C04R0105;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_MULTI: //Sマルチ受付へ
        param = ZONE_ID_C04R0107;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
            work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_AUTO_SINGLE: //シングルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_DOUBLE: //ダブルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_MULTI: //マルチオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_SINGLE: //Sシングルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_DOUBLE: //Sダブルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_MULTI: //Sマルチオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_SINGLE_7: //シングル７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_DOUBLE_7: //ダブル７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_MULTI_7: //マルチ７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_SINGLE_21: //シングル２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_DOUBLE_21: //ダブル２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_MULTI_21: //マルチ２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_SET_REGU_OFF: //レギュオフ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_REGU_OFF;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );

          FLDSYSWIN_ClearWindow( work->sysWin );

          if( (flag & BSW_DEBUG_FLAG_REGU_OFF) ){
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_25 );
          }else{
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_38 );
          }
        }
        break;
      case DEBUG_BSWAY_SET_BTL_SKIP: //バトルスキップ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_BTL_SKIP;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );

          FLDSYSWIN_ClearWindow( work->sysWin );

          if( (flag & BSW_DEBUG_FLAG_BTL_SKIP) ){
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_39);
          }else{
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_24);
          }
        }
        break;
      case DEBUG_BSWAY_BTL_S_SINGLE_49: //Ｓシングル４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_S_DOUBLE_49: //Ｓダブル４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_S_MULTI_49: //Ｓマルチ４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_ANYSTAGE: //任意ステージ
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          debugMenuCallProc_BSubwayAnyStage( event, work );
        }
        break;
      case DEBUG_BSWAY_CHANGE_WIFI_RANK: //wifiランク変更
        chg_event = TRUE;
        debugMenuCallProc_BSubwayChangeWifiRank( event, work );
        break;
      case DEBUG_BSWAY_CLEAR_WIFI_ROOM: //wifiルームデータクリア
        BSUBWAY_SCRWORK_DebugClearWifiRoomData( work->gmSys );
        FLDSYSWIN_ClearWindow( work->sysWin );
        FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_43 );
        break;
      case DEBUG_BSWAY_SELECT_TRAINER: //トレーナー選択
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          debugMenuCallProc_BSubwaySelectTrainer( event, work );
        }
        break;
      case DEBUG_BSWAY_COMM_MULTI_STAGE:
        chg_event = TRUE;
        debugMenuCallProc_BSubwaySelectCommMulti(
            event, work, BSWAY_MODE_COMM_MULTI );
        break;
      case DEBUG_BSWAY_S_COMM_MULTI_STAGE:
        chg_event = TRUE;
        debugMenuCallProc_BSubwaySelectCommMulti(
            event, work, BSWAY_MODE_S_COMM_MULTI );
        break;
      default:
        break;
      }

      if( chg_event == TRUE || next_event != NULL ){
        FLDSYSWIN_Delete( sysWin );
        FLDMENUFUNC_DeleteMenu( menuFunc );
        GFL_MSG_Delete( msgData );

        if( next_event != NULL ){
          GMEVENT_CallEvent( event, next_event );
          (*seq)++;
        }
      }
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }

  return( GMEVENT_RES_CONTINUE );
}

BOOL debugMenuCallProc_BSubway( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSUBWAY_EVENT_WORK  *work;

  GMEVENT_Change( event,
      debugMenuBSubwayEvent, sizeof(DEBUG_BSUBWAY_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );

  return( TRUE );
}
