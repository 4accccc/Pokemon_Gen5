///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『育て屋』
 * @file   event_debug_sodateya.c
 * @author obata
 * @date   2010.04.22
 */
///////////////////////////////////////////////////////////////////////////////
#ifdef PM_DEBUG
#include <gflib.h> 
#include "print/global_msg.h" 
#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "event_debug_local.h"
#include "pokemon_egg.h"
#include "sodateya.h"
#include "field/zonedata.h"

#include "arc/message.naix"
#include "msg/msg_debug_sodateya.h"

#include "event_debug_sodateya.h"


//=============================================================================
// ■育て屋 メニューヘッダ
//=============================================================================
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

//=============================================================================
// ■メニューリスト
//=============================================================================
static const FLDMENUFUNC_LIST menuList[] = 
{
  {sodateya_azukeru,       (void*)sodateya_azukeru},       // あずける
  {sodateya_hikitoru,      (void*)sodateya_hikitoru},      // ひきとる
  {sodateya_shiiku_1,      (void*)sodateya_shiiku_1},      // 飼育する ( 100歩分 )
  {sodateya_shiiku_2,      (void*)sodateya_shiiku_2},      // 飼育する ( 1万歩分 )
  {sodateya_shiiku_3,      (void*)sodateya_shiiku_3},      // 飼育する ( 100万歩分 )
  {sodateya_tamago_get,    (void*)sodateya_tamago_get},    // タマゴを貰う
  {sodateya_tamago_del,    (void*)sodateya_tamago_del},    // タマゴを捨てる
  {sodateya_tamago_breed,  (void*)sodateya_tamago_breed},  // タマゴを温める
  {sodateya_tamago_hatch,  (void*)sodateya_tamago_hatch},  // タマゴを孵す
  {sodateya_tamago_delete, (void*)sodateya_tamago_delete}, // タマゴを孵す
};

//=============================================================================
// ■メニュー初期化構造体
//=============================================================================
static const DEBUG_MENU_INITIALIZER menuInitializer = 
{
  NARC_message_debug_sodateya_dat,
  NELEMS(menuList),
  menuList,
  &menuHeader,
  1, 1, 20, 16,
  NULL,
  NULL,
};

//=============================================================================
// ■タマゴ作成イベントワーク
//=============================================================================
typedef struct {

  GAMESYS_WORK*  gameSystem;
  GAMEDATA*      gameData;
  FIELDMAP_WORK* fieldmap;
  FLDMENUFUNC*   menuFunc;
  HEAPID         heap_id;
  u32            select;

} EVENT_WORK;


//=============================================================================
// ■prototype
//=============================================================================
static GMEVENT_RESULT DebugMenuSodateyaEvent( GMEVENT *event, int *seq, void *wk );
static void DebugSodateya_Azukeru( EVENT_WORK* work ); // 先頭ポケモンを育て屋に預ける
static void DebugSodateya_Hikitoru( EVENT_WORK* work ); // 育て屋の先頭ポケモンを引き取る
static void DebugSodateya_Shiiku( EVENT_WORK* work, u32 time ); // 預けたポケモンを飼育する
static void DebugSodateya_TamagoGet( EVENT_WORK* work ); // タマゴを貰う
static void DebugSodateya_TamagoDel( EVENT_WORK* work ); // タマゴを捨てる
static void DebugSodateya_TamagoBreed( EVENT_WORK* work ); // タマゴを温める
static void DebugSodateya_TamagoHatch( EVENT_WORK* work ); // タマゴを孵す
static void DebugSodateya_DeleteTailPoke( EVENT_WORK* work ); // 末尾ポケモンを削除する


//=============================================================================
// ■public functions
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief 育て屋デバッグメニューイベントを生成する
 *
 * @parma gameSystem
 * @param wk         FIELDMAP_WORKへのポインタ
 */
//-----------------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_DebugMenuSodateya( GAMESYS_WORK* gameSystem, void* wk )
{
  GMEVENT* event;
  EVENT_WORK* work;
  FIELDMAP_WORK* fieldmap = (FIELDMAP_WORK*)wk;
  
  event = GMEVENT_Create(
      gameSystem, NULL, DebugMenuSodateyaEvent, sizeof(EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
	work->gameSystem = gameSystem;
  work->gameData   = GAMESYSTEM_GetGameData( gameSystem ); 
	work->heap_id    = FIELDMAP_GetHeapID( fieldmap );
	work->fieldmap   = fieldmap;

  return event;
}


//=============================================================================
// ■private functions
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief 育て屋デバッグメニューイベント
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT DebugMenuSodateyaEvent( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK* work = wk;
  
  switch( *seq ) {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( 
        work->fieldmap, work->heap_id,  &menuInitializer );
    (*seq)++;
    break;

  case 1:
    work->select = FLDMENUFUNC_ProcMenu( work->menuFunc );

    if( work->select != FLDMENUFUNC_NULL ) {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      (*seq)++; 
    }
    break; 

  case 2: 
    switch( work->select ) {
    case sodateya_azukeru:       DebugSodateya_Azukeru( work );          break;
    case sodateya_hikitoru:      DebugSodateya_Hikitoru( work );         break;
    case sodateya_shiiku_1:      DebugSodateya_Shiiku( work, 100 );      break;
    case sodateya_shiiku_2:      DebugSodateya_Shiiku( work, 10000 );    break;
    case sodateya_shiiku_3:      DebugSodateya_Shiiku( work, 1000000 );  break;
    case sodateya_tamago_get:    DebugSodateya_TamagoGet( work );        break;
    case sodateya_tamago_del:    DebugSodateya_TamagoDel( work );        break;
    case sodateya_tamago_breed:  DebugSodateya_TamagoBreed( work );      break;
    case sodateya_tamago_hatch:  DebugSodateya_TamagoHatch( work );      break;
    case sodateya_tamago_delete: DebugSodateya_DeleteTailPoke( work );   break;
    } 

    // キャンセルで終了
    if( work->select == FLDMENUFUNC_CANCEL ) { 
      return GMEVENT_RES_FINISH;
    }

    // ループする
    *seq = 0;
    break;
  } 
  return( GMEVENT_RES_CONTINUE );
}


//-----------------------------------------------------------------------------
/**
 * @brief 先頭ポケモンを育て屋に預ける
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_Azukeru( EVENT_WORK* work )
{
  SODATEYA*  sodateya   = FIELDMAP_GetSodateya( work->fieldmap );
  POKEPARTY* party      = GAMEDATA_GetMyPokemon( work->gameData );
  int        poke_num   = PokeParty_GetPokeCount( party );
  int        shiiku_num = SODATEYA_GetPokemonNum( sodateya );

  // これ以上預けられない
  if( 2 <= shiiku_num ) {
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_BEEP );
  }
  // 預けられる
  else {
    SODATEYA_TakePokemon( sodateya, party, 0 );
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
}
//-----------------------------------------------------------------------------
/**
 * @brief 育て屋の先頭ポケモンを引き取る
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_Hikitoru( EVENT_WORK* work )
{
  POKEPARTY* party      = GAMEDATA_GetMyPokemon( work->gameData );
  SODATEYA*  sodateya   = FIELDMAP_GetSodateya( work->fieldmap );
  int        shiiku_num = SODATEYA_GetPokemonNum( sodateya );

  // 預けていない
  if( shiiku_num == 0 ) {
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_BEEP );
  }
  // 預けている
  else {
    SODATEYA_TakeBackPokemon( sodateya, 0, party );
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
}
//-----------------------------------------------------------------------------
/**
 * @brief 預けたポケモンを飼育する
 *
 * @param work
 * @param time 飼育する回数 ( 何歩分育てるか )
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_Shiiku( EVENT_WORK* work, u32 time )
{
  u32 count;
  BOOL egg = FALSE;
  SODATEYA* sodateya = FIELDMAP_GetSodateya( work->fieldmap );
  
  for( count=0; count<time; count++ )
  {
    if( SODATEYA_BreedPokemon(sodateya) ) {
      egg = TRUE; // タマゴが産まれた!!
    }
  }

  // 完了!!
  PMSND_StopSE();
  if( egg ) {
    PMSND_PlaySE( SEQ_SE_DECIDE4 );
  }
  else {
    PMSND_PlaySE( SEQ_SE_EXPMAX );
  }
}
//-----------------------------------------------------------------------------
/**
 * @brief タマゴを貰う
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_TamagoGet( EVENT_WORK* work )
{
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
  SODATEYA*  sodateya = FIELDMAP_GetSodateya( work->fieldmap );

  // タマゴがある場合
  if( SODATEYA_HaveEgg(sodateya) ) {
    SODATEYA_TakeBackEgg( sodateya, party );
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
  // タマゴがない場合
  else {
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_BEEP );
  }
}
//-----------------------------------------------------------------------------
/**
 * @brief タマゴを捨てる
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_TamagoDel( EVENT_WORK* work )
{
  SODATEYA* sodateya = FIELDMAP_GetSodateya( work->fieldmap );

  // タマゴがある場合
  if( SODATEYA_HaveEgg(sodateya) ) {
    SODATEYA_DeleteEgg( sodateya );
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
  // タマゴがない場合
  else {
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_BEEP );
  }
} 
//-----------------------------------------------------------------------------
/**
 * @brief タマゴを温める
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_TamagoBreed( EVENT_WORK* work )
{
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
  int        poke_num = PokeParty_GetPokeCount( party );

  int i;

  // 手持ちにある全てのタマゴを孵化直前の状態にする
  for( i=0; i<poke_num; i++ )
  {
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, i );
    u32 tamago_flag = PP_Get( pp, ID_PARA_tamago_flag, NULL );
    if( tamago_flag ) {
      PP_Put( pp, ID_PARA_friend, 0 );  // 孵化歩数
    }
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief タマゴを孵す
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_TamagoHatch( EVENT_WORK* work )
{
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
  int        poke_num = PokeParty_GetPokeCount( party );
  MYSTATUS*  owner    = GAMEDATA_GetMyStatus( work->gameData );
  u32        zone_id  = FIELDMAP_GetZoneID( work->fieldmap );
  u32        place_id = ZONEDATA_GetPlaceNameID( zone_id );

  int i;

  // 手持ちにある全てのタマゴを孵化させる
  for( i=0; i<poke_num; i++ )
  {
    POKEMON_PARAM* poke = PokeParty_GetMemberPointer( party, i );
    u32 tamago_flag = PP_Get( poke, ID_PARA_tamago_flag, NULL );
    if( tamago_flag ) {
      PP_Birth( poke, owner, place_id, work->heap_id );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 末尾ポケモンを削除する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_DeleteTailPoke( EVENT_WORK* work )
{
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
  int        poke_num = PokeParty_GetPokeCount( party );

  if( 2 <= poke_num ) {
    PokeParty_Delete( party, poke_num - 1 );
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
  else {
    PMSND_StopSE();
    PMSND_PlaySE( SEQ_SE_BEEP );
  }
}

#endif // PM_DEBUG
