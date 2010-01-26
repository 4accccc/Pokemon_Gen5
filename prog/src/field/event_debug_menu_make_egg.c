
#include <gflib.h> 
#include "event_debug_menu_make_egg.h"

#include "field/field_msgbg.h"
#include "event_debug_local.h"

#include "arc/message.naix"
#include "msg/msg_debug_make_egg.h"


//--------------------------------------------------------------
// タマゴ作成　メニューヘッダー
//--------------------------------------------------------------
const FLDMENUFUNC_HEADER menuHeader =
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
const FLDMENUFUNC_LIST menuList[] = 
{
  {STR_TEMOTI, STR_TEMOTI},  // 手持ち
};

//--------------------------------------------------------------
// メニュー初期化構造体
//--------------------------------------------------------------
const DEBUG_MENU_INITIALIZER menuInitializer = 
{
  NARC_message_debug_make_egg_dat,
  NELEMS(menuList),
  menuList,
  &menuHeader,
  1, 1, 20, 16,
  NULL,
  NULL,
};

//--------------------------------------------------------------
/// タマゴ作成処理ワーク
//--------------------------------------------------------------
typedef struct
{
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
  
  switch( *seq )
  {
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
    // キャンセル
    if( work->select == FLDMENUFUNC_CANCEL ){ (*seq)++; }

    //「手持ちに追加」
    if( work->select == STR_TEMOTI )
    {
      POKEMON_PARAM* tamago;
      POKEPARTY* party = GAMEDATA_GetMyPokemon( work->gameData );

      tamago = PP_Create( 1, 1, 1, work->heapID );

      // 親の名前
      {
        MYSTATUS* mystatus = GAMEDATA_GetMyStatus( work->gameData );
        STRBUF* name = MyStatus_CreateNameString( mystatus, work->heapID );
        PP_Put( tamago, ID_PARA_oyaname, (u32)name );
        GFL_STR_DeleteBuffer( name );
      } 

      PP_Put( tamago, ID_PARA_tamago_flag, TRUE );
      PP_Renew( tamago );
      PokeParty_Add( party, tamago );
      GFL_HEAP_FreeMemory( tamago );
      (*seq)++;
    }
    break;
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
 * @param heapID
 */
//--------------------------------------------------------------
GMEVENT* DEBUG_EVENT_FLDMENU_MakeEgg( GAMESYS_WORK* gameSystem, HEAPID heapID )
{
  GMEVENT* newEvent;
  EVENT_WORK* work;
  
  newEvent = GMEVENT_Create(
      gameSystem, NULL, debugMenuMakeEggEvent, sizeof(EVENT_WORK) );

  work = GMEVENT_GetEventWork( newEvent );
	work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem ); 
	work->heapID     = heapID;
	work->fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );

  return newEvent;
}
