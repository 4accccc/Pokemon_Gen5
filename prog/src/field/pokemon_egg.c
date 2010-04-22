//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケモンタマゴ
 * @file   pokemon_egg.c
 * @author obata
 * @date   2010.04.21
 */
////////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h> 
#include "gamesystem/game_data.h"
#include "savedata/mystatus.h"
#include "system/gfl_use.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_personal.h"
#include "poke_tool/poke_memo.h"
#include "item/itemsym.h"         // for ITEM_xxxx
#include "item/item.h"            // for ITEM_xxxx
#include "waza_tool/wazano_def.h" // for WAZANO_xxxx
#include "print/global_msg.h"     // for GlobalMsg_PokeName 
#include "arc/arc_def.h"          // for ARCID_PMSTBL

#include "pokemon_egg.h"


//========================================================================================
// ■ 定数
//========================================================================================
#define RARE_EGG_CHANCE (5) // レアタマゴ抽選回数


//========================================================================================
// ■prototype
//========================================================================================
// タマゴパラメータの決定
static void EggCordinate_monsno( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // モンスターNo.
static void EggCordinate_seikaku( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 性格
static void EggCordinate_tokusei( HEAPID heap_id, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 特性
static void EggCordinate_ability_rand( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 個体乱数
static void EggCordinate_rare( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // レア抽選
static void EggCordinate_waza_default( POKEMON_PARAM* egg ); // デフォルト技セット
static void EggCordinate_waza_egg( HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg ); // タマゴ技セット
static void EggCordinate_waza_parent( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 継承技 ( 両親 ) セット
static void EggCordinate_waza_machine( HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg ); // 継承技 ( 技マシン ) セット
static void EggCordinate_pityuu( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 特殊タマゴ処理 ( ピチュー )
static void EggCordinate_karanakusi( const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg ); // 特殊タマゴ処理 ( カラナクシ )
static void EggCordinate_finish( POKEMON_PARAM* egg, const MYSTATUS* mystatus, int memo, HEAPID heap_id );
// タマゴ孵化
static void EggBirth( POKEMON_PARAM* egg, const MYSTATUS* owner, HEAPID heap_id ); // タマゴを孵化させる
// ユーティリティ
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno ); // 技マシンが使えるかどうかをチェックする
static u16 GetSeedMonsNo( u16 monsno ); // 種ポケモンを取得する
// デバッグ
#ifdef PM_DEBUG
static POKEMON_PARAM* CreateDebugEgg( GAMEDATA* gameData, HEAPID heapID, u32 monsno ); // デバッグ用タマゴを作成する
#endif


//========================================================================================
// ■public functions
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを作成する
 *
 * @param heap_id  使用するヒープID
 * @param mystatus タマゴの所有者
 * @param memo     トレーナーメモの記載
 * @param father   父ポケモン
 * @param mother   母ポケモン
 * @param egg      作成したタマゴの格納先
 */
//---------------------------------------------------------------------------------------- 
void POKEMON_EGG_Create( 
    HEAPID heap_id, const MYSTATUS* mystatus, int memo, 
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  // タマゴのパラメータを設定
  EggCordinate_monsno( father, mother, egg );          // モンスターナンバー
  EggCordinate_seikaku( father, mother, egg );         // 性格
  EggCordinate_tokusei( heap_id, mother, egg );        // 特性
  EggCordinate_ability_rand( father, mother, egg );    // 個体乱数
  EggCordinate_rare( father, mother, egg );            // レアポケ抽選
  EggCordinate_waza_default( egg );                    // 技 ( デフォルト )
  EggCordinate_waza_egg( heap_id, father, egg );       // 技 ( タマゴ技 )
  EggCordinate_waza_parent( father, mother, egg );     // 技 ( 両親共通の技 )
  EggCordinate_waza_machine( heap_id, father, egg );   // 技 ( 父のマシン技 )
  EggCordinate_pityuu( father, mother, egg );          // 特殊タマゴ ( ピチュー )
  EggCordinate_karanakusi( father, mother, egg );      // 特殊タマゴ ( カラナクシ )
  EggCordinate_finish( egg, mystatus, memo, heap_id ); // 仕上げ
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを孵化させる
 *
 * @param egg     孵化させるタマゴ
 * @param owner   タマゴの所持者
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------------------- 
void POKEMON_EGG_Birth( POKEMON_PARAM* egg, const MYSTATUS* owner, HEAPID heap_id )
{
  EggBirth( egg, owner, heap_id );
}


//========================================================================================
// ■private functions
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_monsno( 
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  int i;
  u32 monsno_egg;
  u32 monsno_father = PP_Get( father, ID_PARA_monsno, NULL );
  u32 monsno_mother = PP_Get( mother, ID_PARA_monsno, NULL );
  u32 itemno_mother = PP_Get( mother, ID_PARA_item, NULL );
  const int table_size = 9;
  u32 exception_table[table_size][3] = // 例外テーブル
  {
    // 母ポケモン      // アイテム          // 子ポケモン
    {MONSNO_MARIRU,    ITEM_USIONOOKOU,     MONSNO_MARIRU},
    {MONSNO_SOONANSU,  ITEM_NONKINOOKOU,    MONSNO_SOONANO},
    {MONSNO_BARIYAADO, ITEM_AYASIIOKOU,     MONSNO_MANENE},
    {MONSNO_USOKKII,   ITEM_GANSEKIOKOU,    MONSNO_USOHATI},
    {MONSNO_KABIGON,   ITEM_MANPUKUOKOU,    MONSNO_GONBE},
    {MONSNO_MANTAIN,   ITEM_SAZANAMINOOKOU, MONSNO_TAMANTA},
    {MONSNO_ROZERIA,   ITEM_OHANANOOKOU,    MONSNO_SUBOMII},
    {MONSNO_RAKKII,    ITEM_KOUUNNOOKOU,    MONSNO_PINPUKU},
    {MONSNO_TIRIIN,    ITEM_KIYOMENOOKOU,   MONSNO_RIISYAN},
  };
  
  // 通常は母親の種ポケモン
  monsno_egg = GetSeedMonsNo( monsno_mother );

  // 母親がメタモンなら, 父親の種ポケモンが産まれる
  if( monsno_mother == MONSNO_METAMON ) 
  {
    monsno_egg = GetSeedMonsNo( monsno_father );
  }

  // 母親がマナフィなら, フィオネが産まれる
  if( monsno_mother == MONSNO_MANAFI )
  {
    monsno_egg = MONSNO_FIONE;
  }

  // 例外テーブル適用
  for( i=0; i<table_size; i++ )
  {
    // 特定の母ポケが特定アイテムを持っていたら, 子ポケモンが変化
    if( ( monsno_mother == exception_table[i][0] ) &&
        ( itemno_mother == exception_table[i][1] ) )
    {
      monsno_egg = exception_table[i][2];
      break;
    }
  }

  // ポケモンパラメータ構築
  PP_SetupEx( egg, monsno_egg, 1, 
      PTL_SETUP_ID_AUTO, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_AUTO );

  // DEBUG:
  OBATA_Printf( 
      "EggCordinate_monsno: father=%d, mother=%d, egg=%d\n", 
      monsno_father, monsno_mother, monsno_egg );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの性格を決定する
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_seikaku(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  BOOL kawarazu_f; // 父親が『かわらずのいし』を持っているかどうか
  BOOL kawarazu_m; // 母親が『かわらずのいし』を持っているかどうか
  u32 seikaku_f = PP_Get( father, ID_PARA_seikaku, NULL ); // 父親の性格
  u32 seikaku_m = PP_Get( mother, ID_PARA_seikaku, NULL ); // 母親の性格

  //『かわらずのいし』を持っているかどうかを取得
  kawarazu_f = ( PP_Get( father, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );
  kawarazu_m = ( PP_Get( mother, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );

  // 両親とも持っている
  if( kawarazu_f && kawarazu_m ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      // 父親から継承
      if( GFUser_GetPublicRand0(2) == 0 )  {
        PP_Put( egg, ID_PARA_seikaku, seikaku_f );
      }
      // 母親から継承
      else {
        PP_Put( egg, ID_PARA_seikaku, seikaku_m );
      }
    }
  }
  // 父親のみ持っている
  else if( kawarazu_f ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      PP_Put( egg, ID_PARA_seikaku, seikaku_f );
    }
  }
  // 母親のみ持っている
  else if( kawarazu_m ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      PP_Put( egg, ID_PARA_seikaku, seikaku_m );
    }
  }
  // 両親とも持っていない
  else {
    // 乱数で決定
    PP_Put( egg, ID_PARA_seikaku, GFUser_GetPublicRand0( PTL_SEIKAKU_MAX ) );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの特性を決定する ( 母親の特性を遺伝する )
 *
 * @param heap_id 使用するヒープID
 * @param mother  母親ポケモン
 * @param egg     設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_tokusei(
    HEAPID heap_id, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  u32 rnd;
  u32 monsno, formno; // 母のパラメータ
  u32 speabi_egg;     // 子の特性
  u32 speabi_mother;  // 母の特性
  u32 speabi_index;   // 母の特性は特性何番か？
  u32 speabi_list[3]; // 特性リスト
  POKEMON_PERSONAL_DATA* ppd;

  // 母親の特性を取得
  speabi_mother = PP_Get( mother, ID_PARA_speabino, NULL );

  // 特性リスト(種の特性)を取得
  monsno         = PP_Get( mother, ID_PARA_monsno, NULL );
  formno         = PP_Get( mother, ID_PARA_form_no, NULL );
  ppd            = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
  speabi_list[0] = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi1 );
  speabi_list[1] = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi2 );
  speabi_list[2] = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_speabi3 );
  POKE_PERSONAL_CloseHandle( ppd );
  
  // 母親の特性番号を調べる
  for( speabi_index=0; speabi_index<3; speabi_index++ )
  {
    if( speabi_mother == speabi_list[speabi_index] ) break;
  }
  if( 3 <= speabi_index )
  { // 母の特性が, 種の特性に一致しない場合
    OBATA_Printf( "母ポケモンの特性に誤りがあります。\n" );
    speabi_index = 0; // とりあえず, 特性1を継承することにする
  }

  // 継承する特性を決定
  rnd = GFUser_GetPublicRand0(100);  // 乱数取得[0, 99]
  switch( speabi_index )
  {
  // 母が特性1
  case 0:
    if( rnd < 80 ) speabi_egg = speabi_list[0]; // 80% ==> 子の特性1
    else           speabi_egg = speabi_list[1]; // 20% ==> 子の特性2
    break;
  // 母が特性2
  case 1: 
    if( rnd < 20 ) speabi_egg = speabi_list[0]; // 20% ==> 子の特性1
    else           speabi_egg = speabi_list[1]; // 80% ==> 子の特性2
    break;
  // 母が特性3
  case 2: 
    if( rnd < 20 )      speabi_egg = speabi_list[0]; // 20% ==> 子の特性1
    else if( rnd < 40 ) speabi_egg = speabi_list[1]; // 20% ==> 子の特性2
    else                speabi_egg = speabi_list[2]; // 60% ==> 子の特性3
    break;
  }

  // 特性を継承
  PP_Put( egg, ID_PARA_speabino, speabi_egg );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個体乱数を決定する
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_ability_rand(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  int i=0, j=0;
  u32 itemno;
  u32 ability[3];   // 継承する個体乱数

  // 父親が『パワーXXXX』を持っている場合
  itemno = PP_Get( father, ID_PARA_item, NULL );
  switch( itemno )
  {
  case ITEM_PAWAAUEITO:  ability[i++] = PTL_ABILITY_HP;    break;
  case ITEM_PAWAARISUTO: ability[i++] = PTL_ABILITY_ATK;   break;
  case ITEM_PAWAABERUTO: ability[i++] = PTL_ABILITY_DEF;   break;
  case ITEM_PAWAARENZU:  ability[i++] = PTL_ABILITY_SPATK; break;
  case ITEM_PAWAABANDO:  ability[i++] = PTL_ABILITY_SPDEF; break;
  case ITEM_PAWAAANKURU: ability[i++] = PTL_ABILITY_AGI;   break;
  }

  // 母親が『パワーXXXX』を持っている場合
  if( ( i == 0 ) ||
      ( GFUser_GetPublicRand0(2) == 0 ) )    // 父親も持っていた場合, 1/2で上書きする
  {
    i = 0;
    itemno = PP_Get( father, ID_PARA_item, NULL );
    switch( itemno )
    {
    case ITEM_PAWAAUEITO:  ability[i++] = PTL_ABILITY_HP;    break;
    case ITEM_PAWAARISUTO: ability[i++] = PTL_ABILITY_ATK;   break;
    case ITEM_PAWAABERUTO: ability[i++] = PTL_ABILITY_DEF;   break;
    case ITEM_PAWAARENZU:  ability[i++] = PTL_ABILITY_SPATK; break;
    case ITEM_PAWAABANDO:  ability[i++] = PTL_ABILITY_SPDEF; break;
    case ITEM_PAWAAANKURU: ability[i++] = PTL_ABILITY_AGI;   break;
    }
  }

  // 継承する個体乱数をランダムに選ぶ
  for( ; i<3; i++ )
  {
    // ランダムに選ぶ
    ability[i++] = GFUser_GetPublicRand0(PTL_ABILITY_MAX);

    // すでに選ばれていたら, 選び直す
    for( j=0; j<i; j++ )
    {
      if( ability[i] == ability[j] ) i--;
    }
  }

  // 選んだ個体乱数を継承する 
  for( i=0; i<3; i++ )
  {
    const POKEMON_PARAM* pp;
    int id;
    u32 arg;

    // 父・母のどちらから受け継ぐのかを決定
    if( GFUser_GetPublicRand0(2) == 0 ) pp = father;
    else                             pp = mother;

    // 継承
    switch( ability[i] )
    {
    case PTL_ABILITY_HP:    id = ID_PARA_hp_rnd;     break;
    case PTL_ABILITY_ATK:   id = ID_PARA_pow_rnd;    break;
    case PTL_ABILITY_DEF:   id = ID_PARA_def_rnd;    break;
    case PTL_ABILITY_SPATK: id = ID_PARA_spepow_rnd; break;
    case PTL_ABILITY_SPDEF: id = ID_PARA_spedef_rnd; break;
    case PTL_ABILITY_AGI:   id = ID_PARA_agi_rnd;    break;
    }
    arg = PP_Get( pp, id, NULL );
    PP_Put( egg, id, arg );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief レアポケモンの抽選を行う
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_rare(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  int i;
  u32 id, rnd;
  u32 country_father = PP_Get( father, ID_PARA_country_code, NULL );
  u32 country_mother = PP_Get( mother, ID_PARA_country_code, NULL );

  // 親同士の国コードが同じなら, 何もしない
  if( country_father == country_mother ) return;

  // すでにレアなら, 何もしない
  id  = PP_Get( egg, ID_PARA_id_no, NULL );
  rnd = PP_Get( egg, ID_PARA_personal_rnd, NULL );
  if( POKETOOL_CheckRare( id, rnd ) ) return;

  // 最大RARE_EGG_CHANCE回の抽選を行う
  for( i=0; i<RARE_EGG_CHANCE; i++ )
  {
    rnd = GFUser_GetPublicRand0( 0xffffffff );
    if( POKETOOL_CheckRare( id, rnd ) ) {
      // レアが出たら抽選終了とし、個性乱数を書き換える
      PP_Put( egg, ID_PARA_personal_rnd, rnd );
      return;
    }
  }

  // レアが出なかったのであれば個性乱数は書き換えない（かわらずのいし対応）
  
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴにデフォルト技を覚えさせる
 *
 * @param egg     設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_default( POKEMON_PARAM* egg )
{
  int i;
  u32 monsno, formno, level;
  POKEPER_WAZAOBOE_CODE waza_table[POKEPER_WAZAOBOE_TABLE_ELEMS];

  // 基本情報を取得
  monsno = PP_Get( egg, ID_PARA_monsno, NULL );
  formno = PP_Get( egg, ID_PARA_form_no, NULL );
  level  = PP_Get( egg, ID_PARA_level, NULL );

  // Lv.1で覚える技をセット
  POKE_PERSONAL_LoadWazaOboeTable( monsno, formno, waza_table );
  i = 0;
  while( POKEPER_WAZAOBOE_IsEndCode( waza_table[i] ) )
  {
    int waza_lv = POKEPER_WAZAOBOE_GetLevel( waza_table[i] );
    int waza_id = POKEPER_WAZAOBOE_GetWazaID( waza_table[i] );
    if( level < waza_lv ) break;
    else PP_SetWazaPush( egg, waza_id );
    i++;
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴ技を覚えさせる
 *        タマゴから孵るポケモンのみが覚えられる技のうち, 父ポケモンが覚えている技を習得
 *
 * @param heap_id 使用するヒープID 
 * @param father  父親ポケモン
 * @param egg     設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_egg( 
    HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg )
{
  int i, j;
  u32 monsno = PP_Get( egg, ID_PARA_monsno, NULL );
  u32 id_para[] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };
  u16 kowaza_num;
  u16* kowaza;

  // 子技テーブルを取得
  kowaza = GFL_ARC_LoadDataAlloc( ARCID_KOWAZA, monsno, heap_id );
  kowaza_num = kowaza[0];

  // 父親の全技をチェックする
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    u32 wazano = PP_Get( father, id_para[i], NULL );

    // 父親の技がタマゴ技なら, 習得する
    for( j=1; j<=kowaza_num; j++ )
    {
      if( wazano == kowaza[j] )
      {
        PP_SetWazaPush( egg, wazano );
        break;
      }
    }
  }

  // 子技テーブル破棄
  GFL_HEAP_FreeMemory( kowaza );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 両親の技を受け継ぐ
 *        自分もレベルを上げることで覚える技のうち, 両親ともに覚えている技を習得
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_parent(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  int ifa, imo;
  u32 id_para[] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  // 父の全技をチェック
  for( ifa=0; ifa<PTL_WAZA_MAX; ifa++ )
  {
    // 技コード取得
    u32 wazano = PP_Get( father, id_para[ifa], NULL );

    // 母も覚えているかどうかをチェック
    for( imo=0; imo<PTL_WAZA_MAX; imo++ )
    {
      // 母も覚えていたら, 継承する
      if( wazano == PP_Get( mother, id_para[imo], NULL ) )
      {
        PP_SetWazaPush( egg, wazano );
        break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父ポケモンのマシン技を受け継ぐ
 *        自分も覚えることの出来る技マシンのうち, 父ポケモンが覚えている技を習得
 *
 * @param heap_id 使用するヒープID 
 * @param father  父親ポケモン
 * @param egg     設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_machine(
    HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg )
{
  int i, itemno;
  u32 wazano;
  u32 monsno_egg = PP_Get( egg, ID_PARA_monsno, NULL );
  u32 formno_egg = PP_Get( egg, ID_PARA_form_no, NULL );
  u32 id_para[] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  // 父の全技をチェック
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    // 技コード取得
    wazano = PP_Get( father, id_para[i], NULL );

    // 同技の技マシンが存在するかどうかチェック
    for( itemno=ITEM_WAZAMASIN01; itemno<=ITEM_DATA_MAX; itemno++ )
    {
      // 技マシンがあり, タマゴポケモンに使用できるなら, 習得する
      if( (wazano == ITEM_GetWazaNo( itemno )) &&
          (IsWazaMachineAbleToUse( heap_id, monsno_egg, formno_egg, itemno)) )
      {
        PP_SetWazaPush( egg, wazano );
        break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 特殊タマゴ作成処理 (ピチュー)
 *        親ポケが『でんきだま』を持っていたら,『ボルテッカー』を習得
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_pityuu(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  // ピチューのタマゴでなければ何もしない
  if( PP_Get( egg, ID_PARA_monsno, NULL ) != MONSNO_PITYUU ) return;

  // 両親のどちらかが『でんきだま』を持っていたら,『ボルテッカー』を習得
  if( ( PP_Get( father, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) ||
      ( PP_Get( mother, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) )
  {
    PP_SetWazaPush( egg, WAZANO_BORUTEKKAA );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 特殊タマゴ作成処理 (カラナクシ)
 *        母親が『カラナクシ』or『トリトドン』なら, フォルムを継承
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_karanakusi(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  u32 monsno_mother = PP_Get( mother, ID_PARA_monsno, NULL );

  // 母親が『カラナクシ』or『トリトドン』なら, フォルムを継承
  if( (monsno_mother == MONSNO_KARANAKUSI) ||
      (monsno_mother == MONSNO_TORITODON) )
  {
    u32 formno = PP_Get( mother, ID_PARA_form_no, NULL );
    PP_Put( egg, ID_PARA_form_no, formno );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴ作成仕上げ処理
 *
 * @param egg      設定するタマゴ
 * @param mystatus タマゴの所有者
 * @param memo     トレーナーメモ指定
 * @param heap_id  ヒープID
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_finish( 
    POKEMON_PARAM* egg, const MYSTATUS* mystatus, int memo, HEAPID heap_id )
{
  // 再計算
  PP_Renew( egg );

  // ID
  {
    u32 id = MyStatus_GetID( mystatus );
    PP_Put( egg, ID_PARA_id_no, id );
    OBATA_Printf( "EggCordinate_finish: id = %d\n", id );
  }

  // 親の名前
  {
    STRBUF* name = MyStatus_CreateNameString( mystatus, heap_id );
    PP_Put( egg, ID_PARA_oyaname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // ニックネーム ( タマゴ )
  {
    STRBUF* name;

    name = GFL_MSG_CreateString( GlobalMsg_PokeName, MONSNO_TAMAGO );
    PP_Put( egg, ID_PARA_nickname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // 孵化歩数
  {
    u32 monsno, formno, birth;
    POKEMON_PERSONAL_DATA* personal;

    // ポケモンごとの孵化歩数を取得
    monsno   = PP_Get( egg, ID_PARA_monsno, NULL );
    formno   = PP_Get( egg, ID_PARA_form_no, NULL );
    personal = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
    birth    = POKE_PERSONAL_GetParam( personal, POKEPER_ID_egg_birth );
    POKE_PERSONAL_CloseHandle( personal );

    // タマゴの間は, なつき度を孵化カウンタとして利用する
    PP_Put( egg, ID_PARA_friend, birth );
  }

  // トレーナーメモ
  {
    POKE_MEMO_SetTrainerMemoPP( egg, POKE_MEMO_EGG_FIRST, mystatus, memo, heap_id );
  }

  // タマゴフラグ
  PP_Put( egg, ID_PARA_tamago_flag, TRUE );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを孵化させる
 *
 * @param egg     孵化させるタマゴ
 * @param owner   タマゴの所持者
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------------------- 
static void EggBirth( POKEMON_PARAM* egg, const MYSTATUS* owner, HEAPID heap_id )
{ 
  // タマゴフラグ
  PP_Put( egg, ID_PARA_tamago_flag, FALSE );

  // なつき度 ==> パーソナルの初期値を設定
  {
    u32 monsno, formno, friend;
    POKEMON_PERSONAL_DATA* personal;
    monsno   = PP_Get( egg, ID_PARA_monsno, NULL );
    formno   = PP_Get( egg, ID_PARA_form_no, NULL );
    personal = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
    friend   = POKE_PERSONAL_GetParam( personal, POKEPER_ID_friend );
    PP_Put( egg, ID_PARA_friend, friend );
    POKE_PERSONAL_CloseHandle( personal );
  }

  // ID ==> タマゴ所持者のIDを設定
  {
    u32 id = MyStatus_GetID( owner );
    PP_Put( egg, ID_PARA_id_no, id );
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 技マシンが使用できるかどうかを判定する
 *
 * @param heap_id 使用するヒープID
 * @param monsno  技マシン使用対象ポケモンのモンスターナンバー
 * @param formno  モンスターのフォームナンバー
 * @param itemno  使用する技マシンのアイテムナンバー
 * @return 技マシンが使用できる場合 TRUE
 */
//---------------------------------------------------------------------------------------- 
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno )
{
  u16 seed_monsno;
  u32 bit_pos;
  u32 flag;
  u8 machine_no;
  POKEMON_PERSONAL_DATA* ppd;

  // モンスターナンバーを検査
  if( MONSNO_END <= monsno )
  {
    OBATA_Printf( "IsWazaMachineAbleToUse: 無効なモンスターナンバーが指定されました。\n" );
    return FALSE;
  } 
  // 技マシン以外のアイテムが指定された場合
//  if( (itemno < ITEM_WAZAMASIN01 ) || (ITEM_HIDENMASIN08 < itemno ) )
	if( ITEM_CheckWazaMachine( itemno ) == FALSE )
  {
    OBATA_Printf( "IsWazaMachineAbleToUse: 技マシン以外のアイテムが指定されました。\n" );
    return FALSE;
  } 

  // パーソナルデータをオープン
  ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );

  // 技マシン番号(ゼロオリジン)を取得
  machine_no = ITEM_GetWazaMashineNo( itemno );

  // 該当する技マシンフラグ・検査ビット位置を取得
  if( machine_no < 32*1 )
  {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine1 );
    bit_pos = machine_no;
  }
  else if( machine_no < 32*2 ) 
  {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine2 );
    bit_pos = machine_no - 32;
  }
  else if( machine_no < 32*3 ) 
  {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine3 );
    bit_pos = machine_no - 32*2;
  }
  else
  {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine4 );
    bit_pos = machine_no - 32*3;
  }

  // パーソナルデータをクローズ
  POKE_PERSONAL_CloseHandle( ppd );

  // フラグの特定ビットを検査
  return ( flag & (1 << bit_pos) );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定したモンスターの種ポケモンを取得する
 *
 * @param monsno 種ポケモンを調べるモンスター
 * @return 指定モンスターの種ポケモンのモンスターナンバー
 */
//---------------------------------------------------------------------------------------- 
static u16 GetSeedMonsNo( u16 monsno )
{
  u16 seed_monsno;

  // モンスターナンバーを検査
  if( MONSNO_END <= monsno )
  {
    OBATA_Printf( "GetSeedMonsNo: 無効なモンスターナンバーが指定されました。\n" );
    return monsno; // とりあえず, そのまま返す
  } 

  // 種ポケモンNoを取得
  GFL_ARC_LoadDataOfs( &seed_monsno, ARCID_PMSTBL, monsno, 0, sizeof(u16) );
  return seed_monsno;
} 


#ifdef PM_DEBUG
//========================================================================================
// ■debug functions
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( 産まれたて )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreatePlainEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( もうすぐ孵化 )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateQuickEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );
  PP_Put( egg, ID_PARA_friend, 0 );  // 孵化歩数

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( だめタマゴ )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateIllegalEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );
  PP_Put( egg, ID_PARA_fusei_tamago_flag, TRUE );  // ダメたまごフラグ

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する
 *
 * @param gameData
 * @param heapID
 * @param monsno   モンスターNo.
 */
//---------------------------------------------------------------------------------------- 
static POKEMON_PARAM* CreateDebugEgg( GAMEDATA* gameData, HEAPID heapID, u32 monsno )
{
  POKEMON_PARAM* egg;

  egg = PP_Create( monsno, 1, 1, heapID );

  // 親の名前
  {
    MYSTATUS* myStatus;
    STRBUF* name;

    myStatus = GAMEDATA_GetMyStatus( gameData );
    name     = MyStatus_CreateNameString( myStatus, heapID );
    PP_Put( egg, ID_PARA_oyaname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // 孵化歩数
  {
    u32 monsno, formno, birth;
    POKEMON_PERSONAL_DATA* personal;

    monsno   = PP_Get( egg, ID_PARA_monsno, NULL );
    formno   = PP_Get( egg, ID_PARA_form_no, NULL );
    personal = POKE_PERSONAL_OpenHandle( monsno, formno, heapID );
    birth    = POKE_PERSONAL_GetParam( personal, POKEPER_ID_egg_birth );
    POKE_PERSONAL_CloseHandle( personal );

    // タマゴの間は, なつき度を孵化カウンタとして利用する
    PP_Put( egg, ID_PARA_friend, birth );
  }

  // タマゴフラグ
  PP_Put( egg, ID_PARA_tamago_flag, TRUE );

  // ニックネーム ( タマゴ )
  {
    STRBUF* name;

    name = GFL_MSG_CreateString( GlobalMsg_PokeName, MONSNO_TAMAGO );
    PP_Put( egg, ID_PARA_nickname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  PP_Renew( egg );
  return egg;
}


#endif // PM_DEBUG
