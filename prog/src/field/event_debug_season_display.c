////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示選択イベント
 * @file   event_debug_season_disyplay.c
 * @author obata
 * @date   2009.12.01
 */
//////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "gamesystem/pm_season.h"
#include "gamesystem/gamesystem.h"
#include "fieldmap.h"
#include "event_debug_season_display.h"
#include "message.naix"  // NARC_xxxx
#include "msg/msg_d_field.h"  // for DEBUG_FIELD_STR_xxxx
#include "event_debug_local.h"  // for DEBUG_MENU_INITIALIZER
#include "event_season_display.h" // for EVENT_SeasonDisplay


/// どこでもジャンプ　メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_ZoneSel =
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

static const FLDMENUFUNC_LIST DATA_SeasonMenuList[PMSEASON_TOTAL] =
{
  { DEBUG_FIELD_STR_SPRING, (void*)PMSEASON_SPRING },
  { DEBUG_FIELD_STR_SUMMER, (void*)PMSEASON_SUMMER },
  { DEBUG_FIELD_STR_AUTUMN, (void*)PMSEASON_AUTUMN },
  { DEBUG_FIELD_STR_WINTER, (void*)PMSEASON_WINTER },
};

static const DEBUG_MENU_INITIALIZER DebugSeasonJumpMenuData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SeasonMenuList),
  DATA_SeasonMenuList,
  &DATA_DebugMenuList_ZoneSel, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};


//======================================================================================
// ■四季表示イベントワーク
//======================================================================================
typedef struct
{
  HEAPID           heapID;
  GAMESYS_WORK*      gsys;
  GAMEDATA*         gdata;
  FIELDMAP_WORK* fieldmap;
  FLDMENUFUNC*   menuFunc;

} EVENT_WORK;


//======================================================================================
// ■非公開関数のプロトタイプ宣言
//======================================================================================
static GMEVENT_RESULT debugMenuSeasonSelectEvent( GMEVENT *event, int *seq, void *wk );


//======================================================================================
// ■外部公開関数
//======================================================================================

//--------------------------------------------------------------------------------------
/**
 * @brief 四季表示選択イベントを作成する
 */
//--------------------------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_SeasonDispSelect( GAMESYS_WORK * gsys, HEAPID heapID )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, NULL, debugMenuSeasonSelectEvent, sizeof(EVENT_WORK) );
   // 初期化
  work = GMEVENT_GetEventWork( event );
  work->heapID   = heapID;
  work->gsys     = gsys;
  work->gdata    = GAMESYSTEM_GetGameData( gsys );
  work->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  work->menuFunc = NULL;
  return event;
}


//======================================================================================
// ■非公開関数
//======================================================================================

//--------------------------------------------------------------------------------------
/**
 * @brief イベント：四季表示選択
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuSeasonSelectEvent( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK *work = wk;

  switch( (*seq) )
  {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldmap, work->heapID,  
                                        &DebugSeasonJumpMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL )
      { //キャンセル
        return( GMEVENT_RES_FINISH );
      }
      else
      {
        GMEVENT_ChangeEvent( event, 
                             EVENT_SeasonDisplay(work->gsys, work->fieldmap, ret, ret) );
      }
    }
    break;
  }
  
  return( GMEVENT_RES_CONTINUE );
}
