/*
 *  @file   event_debug_numinput.c
 *  @brief  デバッグ数値入力
 *  @author Miyuki Iwasawa
 *  @date   09.11.25
 */
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "sound/pm_sndsys.h"

#include "field_sound.h"
#include "eventwork.h"

#include "event_debug_local.h"
#include "event_debug_numinput.h"

#include "message.naix"
#include "msg/msg_d_numinput.h"


///項目選択用データの定義
typedef enum {
  D_NINPUT_DATA_CONST,    ///<固定データ
  D_NINPUT_DATA_ALLOC,    ///<自動生成データ
}D_NINPUT_DATA_TYPE;

//数値入力メインイベント制御ワーク
typedef struct _DNINPUT_EV_WORK{
  HEAPID  heapID;

  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK* fieldWork;

  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DNINPUT_EV_WORK;


///数値入力リスト初期化パラメータ定義
typedef struct{ 
  D_NINPUT_DATA_TYPE data_type;       ///<データの種別
  u32   file_id;                      ///<D_NINPUT_DATA_ALLOCの場合の参照ファイルID
#if 0
  /** D_NINPUT_DATA_CONSTの場合：各項目の数値入力時処理指定データ
   *  D_NINPUT_DATA_ALLOCの場合：生成時に使用するデフォルトデータ
   */
  const DEBUG_NUMINPUT_LIST * pList; 
  const GMM_MENU_PARAM * pMenu;       ///<項目リスト表示用データ
#else
  void* list;
  void* menu;
#endif
  const u32 list_count;               ///<リストの大きさ

}DEBUG_NUMINPUT_INITIALIZER;


/////////////////////////////////////////////////////////
//プロトタイプ
/////////////////////////////////////////////////////////
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work);


/////////////////////////////////////////////////////////
//データ定義
/////////////////////////////////////////////////////////
static const DEBUG_NUMINPUT_INITIALIZER DATA_Initializer[D_NUMINPUT_MODE_MAX] = { 
  { D_NINPUT_DATA_CONST,  0,
    NULL /*Debug_NumInputL_Tower*/,
    NULL /*Debug_NumInputG_Tower*/, 0/*NELEMS(Debug_NumInputG_Tower)*/ },
  { D_NINPUT_DATA_CONST,  0,
    NULL /*Debug_NumInputL_Tower*/,
    NULL /*Debug_NumInputG_Tower*/, 0/*NELEMS(Debug_NumInputG_Tower)*/ },
};

/// 数値入力　メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DNumInput_MenuFuncHeader =
{
  1,    //リスト項目数
  9,    //表示最大項目数
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

static const FLDMENUFUNC_LIST DATA_DNumInputMenu[D_NUMINPUT_MODE_MAX] =
{
  { dni_top_effect_encount, (void*)D_NUMINPUT_ENCEFF },
  { dni_top_scenario, (void*)D_NUMINPUT_DUMMY },
};

static const DEBUG_MENU_INITIALIZER DATA_DNumInput_MenuInitializer = {
  NARC_message_d_numinput_dat,  //メッセージアーカイブ
  NELEMS(DATA_DNumInputMenu),  //項目数max
  DATA_DNumInputMenu, //メニュー項目リスト
  &DATA_DNumInput_MenuFuncHeader, //メニュヘッダ
  1, 4, 30, 16,
  NULL/*DEBUG_SetMenuWorkZoneID_SelectZone*/,
  NULL,
};

/////////////////////////////////////////////////////////
//関数定義
/////////////////////////////////////////////////////////
//--------------------------------------------------------------
/**
 * 数値入力デバッグメニューイベント生成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
GMEVENT* EVENT_DMenuNumInput( GAMESYS_WORK* gsys, void* work )
{
  DEBUG_MENU_EVENT_WORK* dme_wk = (DEBUG_MENU_EVENT_WORK*)work;
  DNINPUT_EV_WORK* wk;
  GMEVENT* event;

  event = GMEVENT_Create( gsys, NULL, dninput_MainEvent, sizeof(DNINPUT_EV_WORK) );

  wk = GMEVENT_GetEventWork( event );
  wk->gsys = gsys; 
  wk->gdata = dme_wk->gdata; 
  wk->fieldWork = dme_wk->fieldWork;
  wk->heapID = dme_wk->heapID;

  return event;
}

/*
 *  @brief  数値入力メインメニューイベント
 */
static GMEVENT_RESULT dninput_MainEvent( GMEVENT * event, int *seq, void * work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  DNINPUT_EV_WORK* wk = (DNINPUT_EV_WORK*)work;

  switch(*seq)
  {
  case 0:
    wk->menuFunc = DebugMenuInit( wk->fieldWork, wk->heapID, &DATA_DNumInput_MenuInitializer );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){
        break;
      }
      FLDMENUFUNC_DeleteMenu( wk->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){
        return GMEVENT_RES_FINISH;
      }
      (*seq)++;
    }
    break;
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

