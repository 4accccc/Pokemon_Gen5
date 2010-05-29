//======================================================================
/**
 * @brief  デバッグメニュー「ハイリンク」
 * @file   event_debug_intrude.c
 * @author tamada
 * @date   2010.05.29
 */
//======================================================================
#ifdef PM_DEBUG

#include <gflib.h> 
#include "print/global_msg.h" 
#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "event_debug_local.h"
#include "pokemon_egg.h"

#include "arc/message.naix"
#include "msg/msg_debug_intrude.h"

#include "event_debug_menu_intrude.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"

//======================================================================
//======================================================================
static void clearTutorial( FIELDMAP_WORK * fieldmap );

//======================================================================
//======================================================================
//--------------------------------------------------------------
// ハイリンクデバッグ　メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER menuHeader =
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

//--------------------------------------------------------------
// メニューリスト
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST menuList[] = 
{
  {STR_INTRUDE_TUTORIAL_CLEAR,     (void*)0},      // チュートリアルクリア
};

//--------------------------------------------------------------
// メニュー初期化構造体
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER menuInitializer = 
{
  NARC_message_debug_intrude_dat,
  NELEMS(menuList),
  menuList,
  &menuHeader,
  1, 1, 20, 16,
  NULL,
  NULL,
};

//--------------------------------------------------------------
/// ハイリンクデバッグイベントワーク
//--------------------------------------------------------------
typedef struct {

  HEAPID         heapID;
  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;
  FLDMENUFUNC*   menuFunc;
  u32            select;

} EVENT_WORK;


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief ハイリンクデバッグイベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuIntrudeEvent( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK* work = wk;
  
  switch( *seq ) {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldmap, work->heapID,  &menuInitializer );
    (*seq)++;
    break;

  case 1:
    work->select = FLDMENUFUNC_ProcMenu( work->menuFunc );

    if( work->select != FLDMENUFUNC_NULL )
    {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      (*seq)++; 
    }
    break; 

  case 2: 
    // ハイリンクデバッグ
    {

      // 作成
      if( work->select == 0 ) { 
        //「チュートリアルクリア」
        clearTutorial( work->fieldmap );
       // egg = DEBUG_POKEMON_EGG_CreatePlainEgg( work->gameData, work->heapID );
      }
      else if( work->select == 1 ) { 
        //「もうすぐ孵化」
        //egg = DEBUG_POKEMON_EGG_CreateQuickEgg( work->gameData, work->heapID );
      }
      else if( work->select == 2 ) { 
        //「だめタマゴ」
        //egg = DEBUG_POKEMON_EGG_CreateIllegalEgg( work->gameData, work->heapID );
      }
    }
    (*seq)++;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * @brief ハイリンクデバッグイベント生成
 *
 * @parma gameSystem
 * @param wk FIELDMAP_WORKへのポインタ
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_Intrude( GAMESYS_WORK* gameSystem, void* wk )
{
  GMEVENT* event;
  EVENT_WORK* work;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)wk;
  
  event = GMEVENT_Create(
      gameSystem, NULL, debugMenuIntrudeEvent, sizeof(EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
	work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem ); 
	work->heapID     = FIELDMAP_GetHeapID( fieldmap );
	work->fieldmap   = fieldmap;

  return event;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static void clearTutorial( FIELDMAP_WORK * fieldmap )
{
  GAMESYS_WORK * gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  EVENTWORK * ev = GAMEDATA_GetEventWork( gamedata );

  EVENTWORK_SetEventFlag( ev, SYS_FLAG_PALACE_MISSION_START );
  EVENTWORK_SetEventFlag( ev, SYS_FLAG_PALACE_MISSION_CLEAR );
  EVENTWORK_SetEventFlag( ev, SYS_FLAG_PALACE_DPOWER );

  *( EVENTWORK_GetEventWorkAdrs( ev, WK_SCENE_PALACE01 ) ) = 1;
}

#endif //PM_DEBUG
