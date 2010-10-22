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

#include "arc/debug_message.naix"
#include "msg/debug/msg_debug_sodateya.h"

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
static const DEBUG_MENU_LIST menuList[] = 
{
  {sodateya_azukeru,       (void*)sodateya_azukeru, GFL_OVERLAY_BLANK_ID },       // あずける
  {sodateya_hikitoru,      (void*)sodateya_hikitoru, GFL_OVERLAY_BLANK_ID },      // ひきとる
  {sodateya_shiiku_1,      (void*)sodateya_shiiku_1, GFL_OVERLAY_BLANK_ID },      // 飼育する ( 100歩分 )
  {sodateya_shiiku_2,      (void*)sodateya_shiiku_2, GFL_OVERLAY_BLANK_ID },      // 飼育する ( 1万歩分 )
  {sodateya_shiiku_3,      (void*)sodateya_shiiku_3, GFL_OVERLAY_BLANK_ID },      // 飼育する ( 100万歩分 )
  {sodateya_tamago_get,    (void*)sodateya_tamago_get, GFL_OVERLAY_BLANK_ID },    // タマゴを貰う
  {sodateya_tamago_del,    (void*)sodateya_tamago_del, GFL_OVERLAY_BLANK_ID },    // タマゴを捨てる
  {sodateya_tamago_breed,  (void*)sodateya_tamago_breed, GFL_OVERLAY_BLANK_ID },  // タマゴを温める
  {sodateya_tamago_hatch,  (void*)sodateya_tamago_hatch, GFL_OVERLAY_BLANK_ID },  // タマゴを孵す
  {sodateya_tamago_delete, (void*)sodateya_tamago_delete, GFL_OVERLAY_BLANK_ID }, // タマゴを孵す

  {sodateya_tamago_auto_test, (void*)sodateya_tamago_auto_test, GFL_OVERLAY_BLANK_ID }, // オートデバッグ（１００匹孵化してデバッグ出力する）
};

//=============================================================================
// ■メニュー初期化構造体
//=============================================================================
static const DEBUG_MENU_INITIALIZER menuInitializer = 
{
  NARC_debug_message_debug_sodateya_dat,
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
static void DebugSodateya_AutoTest( EVENT_WORK* work ); // 指定引数孵化しデバグプリント


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
    case sodateya_tamago_auto_test: DebugSodateya_AutoTest( work );   break;
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

  // 預けられない
  if( (poke_num <= 1) || (2 <= shiiku_num) ) {
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
//----------------------------------------------------------------------------
/**
 *	@brief  指定数孵化してデバッグ出力する
 *
 *	@param	EVENT_WORK* work ワーク
 */
//-----------------------------------------------------------------------------
static void DebugSodateya_AutoTest( EVENT_WORK* work )
{
  enum
  {
    DEBUG_SODATEYA_AUTO_TEST_DETAIL_FLAG  = 1,  //ここの真偽で詳細情報を載せるかどうか
    DEBUG_SODATEYA_AUTO_TEST_TEST_NUM = 100,    //何回テストするか
  };

  //手持ちに空きがないならばやらない
  {
    POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
    int        poke_num = PokeParty_GetPokeCount( party );
    if( poke_num ==  PokeParty_GetPokeCountMax( party ))
    {
      OS_Printf( "手持ちに空きを作ってください\n" );
      PMSND_StopSE();
      PMSND_PlaySE( SEQ_SE_BEEP );
      return;
    }
  }

  //生まれない組み合わせならばやらない
  {
    SODATEYA*  sodateya = FIELDMAP_GetSodateya( work->fieldmap );
    if( SODATEYA_GetLoveLevel( sodateya ) == 0 )
    {
      OS_Printf( "育て屋に預けてある組み合わせでは生まれません\n" );
      PMSND_StopSE();
      PMSND_PlaySE( SEQ_SE_BEEP );
      return;
    }
  }

  //テスト実行
  OS_Printf( "▼孵化オートテスト開始\n" );
  {
    int i;

    //統計情報
    u32 total_sex[3] = {0};
    u32 total_tokusei[3] = {0};
    u32 total_tokusei_num[3] = {0};
    u32 total_seikaku[PTL_SEIKAKU_MAX] = {0};

    //指定回数テストを行う
    for( i = 0; i < DEBUG_SODATEYA_AUTO_TEST_TEST_NUM; i++ )
    {
      //卵ができるまで歩く
      {
        SODATEYA*  sodateya = FIELDMAP_GetSodateya( work->fieldmap );
        while( !SODATEYA_BreedPokemon(sodateya) )
        {
          /* nothing */
        }
      }

      //卵を末尾に追加
      {
        POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
        SODATEYA*  sodateya = FIELDMAP_GetSodateya( work->fieldmap );
        SODATEYA_TakeBackEgg( sodateya, party );
      }

      //卵情報チェック
      {
        POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
        int        poke_num = PokeParty_GetPokeCount( party );
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party, poke_num-1 );

        //詳細情報表示
        if( DEBUG_SODATEYA_AUTO_TEST_DETAIL_FLAG )
        {
          OS_Printf( "・詳細表示[%d]\n", i );
          OS_Printf( "ポケモン:%d\n", PP_Get( pp, ID_PARA_monsno, NULL ) );
          OS_Printf( "フォルム:%d\n", PP_Get( pp, ID_PARA_form_no, NULL ) );
          OS_Printf( "せいべつ:%d\n", PP_Get( pp, ID_PARA_sex, NULL ) );
          OS_Printf( "特性番号:%d\n", PP_GetTokuseiIndex( pp ) );
          OS_Printf( "とくせい:%d\n", PP_Get( pp, ID_PARA_speabino, NULL ) );
          OS_Printf( "せいかく:%d\n", PP_Get( pp, ID_PARA_seikaku, NULL ) );
          OS_Printf( "HP:%d\n", PP_Get( pp, ID_PARA_hp_rnd, NULL ) );
          OS_Printf( "こうげき:%d\n", PP_Get( pp, ID_PARA_pow_rnd, NULL ) );
          OS_Printf( "ぼうぎょ:%d\n", PP_Get( pp, ID_PARA_def_rnd, NULL ) );
          OS_Printf( "すばやさ:%d\n", PP_Get( pp, ID_PARA_agi_rnd, NULL ) );
          OS_Printf( "とくこう:%d\n", PP_Get( pp, ID_PARA_spepow_rnd, NULL ) );
          OS_Printf( "とくぼう:%d\n", PP_Get( pp, ID_PARA_spedef_rnd, NULL ) );
        }

        //統計情報チェック
        {
          total_sex[ PP_Get( pp, ID_PARA_sex, NULL ) ]++;
          total_tokusei[ PP_GetTokuseiIndex( pp ) ]++;
          total_seikaku[ PP_Get( pp, ID_PARA_seikaku, NULL ) ]++;

          if( total_tokusei_num[ PP_GetTokuseiIndex( pp ) ] == 0 )
          {
            total_tokusei_num[ PP_GetTokuseiIndex( pp ) ] = PP_Get( pp, ID_PARA_speabino, NULL );
          }
          else
          {

            //とくせいチェック
            {
              POKEMON_PERSONAL_DATA *p_ppd  = POKE_PERSONAL_OpenHandle( 
                  PP_Get( pp, ID_PARA_monsno, NULL ),
                  PP_Get( pp, ID_PARA_form_no, NULL ),
                  GFL_HEAP_LOWID(work->heap_id) );
              u32 tokusei = POKE_PERSONAL_GetParam( p_ppd, POKEPER_ID_speabi1 + PP_GetTokuseiIndex( pp ) );
              GF_ASSERT_MSG( PP_Get( pp, ID_PARA_speabino, NULL ) == tokusei, "異なった特性が入っています now[%d],real[%d]\n", PP_Get( pp, ID_PARA_speabino, NULL ), tokusei );
              POKE_PERSONAL_CloseHandle( p_ppd );
            }

            //とくせいチェック
            //第N特性に2種類の特性があった場合はアサートで止める
            {
              GF_ASSERT_MSG( total_tokusei_num[ PP_GetTokuseiIndex( pp ) ] == PP_Get( pp, ID_PARA_speabino, NULL ), "違う特性！特性index[%d]前の特性[%d]今の特性[%d]\n", PP_GetTokuseiIndex( pp ), total_tokusei_num[ PP_GetTokuseiIndex( pp ) ], PP_Get( pp, ID_PARA_speabino, NULL ) );
            }
          }
        }
      }

      //末尾の卵削除
      {
        POKEPARTY* party    = GAMEDATA_GetMyPokemon( work->gameData );
        int        poke_num = PokeParty_GetPokeCount( party );
        PokeParty_Delete( party, poke_num - 1 );
      }
    }

    //特性情報表示
    {
      static const char* scp_sex_label[]  =
      {
        "オス",
        "メス",
        "不明",
      };
      static const char* scp_tokusei_label[]  =
      {
        "第一特性",
        "第二特性",
        "第三特性",
      };
      static const char* scp_seikaku_label[]  =
      {
        "がんばりや",
        "さみしがり",
        "ゆうかん",
        "いじっぱり",
        "やんちゃ",
        "ずぶとい",
        "すなお",
        "のんき",
        "わんぱく",
        "のうてんき",
        "おくびょう",
        "せっかち",
        "まじめ",
        "ようき",
        "むじゃき",
        "ひかえめ",
        "おっとり",
        "れいせい",
        "てれや",
        "うっかりや",
        "おだやか",
        "おとなしい",
        "なまいき",
        "しんちょう",
        "きまぐれ",
      };
      int i;
      OS_Printf( "【統計情報】\n" );
      OS_Printf( "・性別\n"  );
      for( i = 0; i < 3; i++ )
      {
        OS_Printf( "%s:%d％\n", scp_sex_label[i], total_sex[i] * 100 / DEBUG_SODATEYA_AUTO_TEST_TEST_NUM );
        
      }
      OS_Printf( "・特性\n" );
      for( i = 0; i < 3; i++ )
      {
        OS_Printf( "%s[%d]:%d％\n", scp_tokusei_label[i], total_tokusei_num[i], total_tokusei[i] * 100 / DEBUG_SODATEYA_AUTO_TEST_TEST_NUM );
      }
      OS_Printf( "・性格\n" );
      for( i = 0; i < PTL_SEIKAKU_MAX; i++ )
      {
        OS_Printf( "%s:%d％\n", scp_seikaku_label[i], total_seikaku[i] * 100 / DEBUG_SODATEYA_AUTO_TEST_TEST_NUM );
        
      }
    }
  }

  OS_Printf( "▲孵化オートテスト終了！！\n" );
}

#endif // PM_DEBUG
