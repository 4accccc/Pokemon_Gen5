////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー「タマゴ作成」
 * @file   event_debug_menu_make_egg.c
 * @author obata
 * @date   2010.04.16
 */
////////////////////////////////////////////////////////////////
#include <gflib.h> 
#include "print/global_msg.h" 
#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "event_debug_local.h"
#include "pokemon_egg.h"

#include "arc/debug_message.naix"
#include "msg/debug/msg_debug_make_egg.h"

#include "event_debug_menu_make_egg.h"

#ifdef PM_DEBUG

//--------------------------------------------------------------
// タマゴ作成　メニューヘッダー
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
  {STR_NEWBORN,     (void*)STR_NEWBORN},      // うまれたて
  {STR_OLDBORN,     (void*)STR_OLDBORN},      // もうすぐ孵化
  {STR_ILLEGAL_EGG, (void*)STR_ILLEGAL_EGG},  // だめタマゴ
};

//--------------------------------------------------------------
// メニュー初期化構造体
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER menuInitializer = 
{
  NARC_debug_message_debug_make_egg_dat,
  NELEMS(menuList),
  menuList,
  &menuHeader,
  1, 1, 20, 16,
  NULL,
  NULL,
};

//--------------------------------------------------------------
/// タマゴ作成イベントワーク
//--------------------------------------------------------------
typedef struct {

  HEAPID         heapID;
  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;
  FLDMENUFUNC*   menuFunc;
  u32            select;

} EVENT_WORK;


//--------------------------------------------------------------
/**
 * @brief タマゴ作成イベント
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakeEggEvent( GMEVENT *event, int *seq, void *wk )
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
    // タマゴ作成
    {
      POKEMON_PARAM* egg = NULL;

      // 作成
      if( work->select == STR_NEWBORN ) { 
        //「うまれたて」
        egg = DEBUG_POKEMON_EGG_CreatePlainEgg( work->gameData, work->heapID );
      }
      else if( work->select == STR_OLDBORN ) { 
        //「もうすぐ孵化」
        egg = DEBUG_POKEMON_EGG_CreateQuickEgg( work->gameData, work->heapID );
      }
      else if( work->select == STR_ILLEGAL_EGG ) { 
        //「だめタマゴ」
        egg = DEBUG_POKEMON_EGG_CreateIllegalEgg( work->gameData, work->heapID );
      }

      // 手持ちに追加
      if( egg ) {
        POKEPARTY* party = GAMEDATA_GetMyPokemon( work->gameData );
        PokeParty_Add( party, egg ); 
        GFL_HEAP_FreeMemory( egg );
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
 * @brief タマゴ作成イベント生成
 *
 * @parma gameSystem
 * @param wk FIELDMAP_WORKへのポインタ
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_MakeEgg( GAMESYS_WORK* gameSystem, void* wk )
{
  GMEVENT* event;
  EVENT_WORK* work;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)wk;
  
  event = GMEVENT_Create(
      gameSystem, NULL, debugMenuMakeEggEvent, sizeof(EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
	work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem ); 
	work->heapID     = FIELDMAP_GetHeapID( fieldmap );
	work->fieldmap   = fieldmap;

  return event;
}


#endif //PM_DEBUG
