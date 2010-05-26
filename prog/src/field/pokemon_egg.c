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
#include "poke_tool/tokusyu_def.h"// for TOKUSYU_xxxx
#include "item/itemsym.h"         // for ITEM_xxxx
#include "item/item.h"            // for ITEM_xxxx
#include "waza_tool/wazano_def.h" // for WAZANO_xxxx
#include "print/global_msg.h"     // for GlobalMsg_PokeName 
#include "arc/arc_def.h"          // for ARCID_PMSTBL

#include "pokemon_egg.h"


#define DEBUG_PRINT_ON      // デバッグ出力スイッチ
#define PRINT_TARGET    (2) // デバッグ出力先
#define RARE_EGG_CHANCE (5) // レアタマゴ抽選回数

// 両親ポケモン
typedef struct {
  const POKEMON_PARAM* father;
  const POKEMON_PARAM* mother;
} PARENT;


// prototype /////////////////////////////////////////////////////////////////////////////

// 父母の決定
static void CorrectParent_bySex( PARENT* parent );
static void CorrectParent_byMetamon( PARENT* parent );
static void ReverseParent( PARENT* parent );
// タマゴパラメータの決定
static void EggCordinate_monsno( const PARENT* parent, POKEMON_PARAM* egg ); // モンスターNo.
static void EggCordinate_seikaku( const PARENT* parent, POKEMON_PARAM* egg ); // 性格
static void EggCordinate_tokusei( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id ); // 特性
static void EggCordinate_ability_rand( const PARENT* parent, POKEMON_PARAM* egg ); // 個体乱数
static void EggCordinate_rare( const PARENT* parent, POKEMON_PARAM* egg ); // レア抽選
static void EggCordinate_waza_default( POKEMON_PARAM* egg ); // デフォルト技セット
static void EggCordinate_waza_egg( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id ); // タマゴ技セット
static void EggCordinate_waza_parent( const PARENT* parent, POKEMON_PARAM* egg ); // 継承技 ( 両親 ) セット
static void EggCordinate_waza_machine( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id ); // 継承技 ( 技マシン ) セット
static void EggCordinate_pityuu( const PARENT* parent, POKEMON_PARAM* egg ); // 特殊タマゴ処理 ( ピチュー )
static void EggCordinate_karanakusi( const PARENT* parent, POKEMON_PARAM* egg ); // 特殊タマゴ処理 ( カラナクシ )
static void EggCordinate_waza_sort( POKEMON_PARAM* egg ); // 技をソートする
static void EggCordinate_finish( POKEMON_PARAM* egg, const MYSTATUS* mystatus, int memo, HEAPID heap_id );
static const POKEMON_PARAM* GetBasePokemon( const PARENT* parent );
static u32 ChangeEggMonsNo_manafi( const POKEMON_PARAM* base_poke, u32 monsno_egg );
static u32 ChangeEggMonsNo_item( const POKEMON_PARAM* base_poke, u32 monsno_egg );
// タマゴ孵化
static void EggBirth( POKEMON_PARAM* egg, const MYSTATUS* owner, HEAPID heap_id ); // タマゴを孵化させる
// ユーティリティ
static void PokeWazaLearnByPush( POKEMON_PARAM* poke, u32 wazano ); // 技を押し出し式に覚えさせる ( 重複チェック有 )
static BOOL CheckWazaHave( const POKEMON_PARAM* poke, u32 wazano ); // 指定した技を持っているかどうかをチェックする
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno ); // 技マシンが使えるかどうかをチェックする
static u16 GetSeedMonsNo( u16 monsno ); // 種ポケモンを取得する
// デバッグ
#ifdef PM_DEBUG
static POKEMON_PARAM* CreateDebugEgg( GAMEDATA* gameData, HEAPID heapID, u32 monsno ); // デバッグ用タマゴを作成する
#endif



// publid functions //////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを作成する
 *
 * @param heap_id  使用するヒープID
 * @param mystatus タマゴの所有者
 * @param memo     トレーナーメモの記載
 * @param poke1    親ポケモン1
 * @param poke2    親ポケモン2
 * @param egg      作成したタマゴの格納先
 */
//---------------------------------------------------------------------------------------- 
void POKEMON_EGG_Create( 
    HEAPID heap_id, const MYSTATUS* mystatus, int memo, 
    const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2, POKEMON_PARAM* egg )
{
  PARENT parent;

  // 父母を決定
  parent.father = poke1;
  parent.mother = poke2;
  CorrectParent_bySex( &parent );
  CorrectParent_byMetamon( &parent );

  // タマゴのパラメータを設定
  EggCordinate_monsno( &parent, egg );                 // モンスターナンバー
  EggCordinate_seikaku( &parent, egg );                // 性格
  EggCordinate_tokusei( &parent, egg, heap_id );       // 特性
  EggCordinate_ability_rand( &parent, egg );           // 個体乱数
  EggCordinate_rare( &parent, egg );                   // レアポケ抽選
  EggCordinate_waza_default( egg );                    // 技 ( デフォルト )
  EggCordinate_waza_egg( &parent, egg, heap_id );      // 技 ( タマゴ技 )
  EggCordinate_waza_parent( &parent, egg );            // 技 ( 両親共通の技 )
  EggCordinate_waza_machine( &parent, egg, heap_id );  // 技 ( 父のマシン技 )
  EggCordinate_pityuu( &parent, egg );                 // 特殊タマゴ ( ピチュー )
  EggCordinate_karanakusi( &parent, egg );             // 特殊タマゴ ( カラナクシ )
  EggCordinate_waza_sort( egg );                       // 技ソート
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



// private functions /////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母の組み合わせを訂正する ( 性別 )
 */
//---------------------------------------------------------------------------------------- 
static void CorrectParent_bySex( PARENT* parent )
{
  u32 sex_father, sex_mother;

  sex_father = PP_Get( parent->father, ID_PARA_sex, NULL );
  sex_mother = PP_Get( parent->mother, ID_PARA_sex, NULL );

  if( sex_father == PTL_SEX_FEMALE ) {
    ReverseParent( parent );
  }
  if( sex_mother == PTL_SEX_MALE ) {
    ReverseParent( parent );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母の組み合わせを訂正する ( メタモン )
 *
 * メタモン × ♂ ==> ( 父, 母 ) = ( ♂, メタモン )
 * メタモン × ♀ ==> ( 父, 母 ) = ( メタモン, ♀ )
 * メタモン × ？ ==> ( 父, 母 ) = ( ？, メタモン )
 */
//---------------------------------------------------------------------------------------- 
static void CorrectParent_byMetamon( PARENT* parent )
{
  u32 monsno_father;
  u32 sex_mother;

  monsno_father = PP_Get( parent->father, ID_PARA_monsno, NULL );
  sex_mother = PP_Get( parent->mother, ID_PARA_sex, NULL );

  if( (monsno_father == MONSNO_METAMON) && (sex_mother == PTL_SEX_UNKNOWN) ) {
    ReverseParent( parent );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母を入れ替える
 */
//---------------------------------------------------------------------------------------- 
static void ReverseParent( PARENT* parent )
{
  const POKEMON_PARAM* temp = parent->father;
  parent->father = parent->mother;
  parent->mother = temp;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_monsno( const PARENT* parent, POKEMON_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 monsno_base, monsno_egg;

  // 基本ルールで子を決定
  base_poke   = GetBasePokemon( parent );
  monsno_base = PP_Get( base_poke, ID_PARA_monsno, NULL );
  monsno_egg  = GetSeedMonsNo( monsno_base );

  // 例外ルールによる子ポケモンの変化
  monsno_egg = ChangeEggMonsNo_manafi( base_poke, monsno_egg );
  monsno_egg = ChangeEggMonsNo_item( base_poke, monsno_egg );
  
  // ポケモンパラメータ構築
  PP_SetupEx( egg, monsno_egg, 1, 
      PTL_SETUP_ID_AUTO, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_AUTO );

#ifdef DEBUG_PRINT_ON
  {
    u32 monsno_father = PP_Get( parent->father, ID_PARA_monsno, NULL );
    u32 monsno_mother = PP_Get( parent->mother, ID_PARA_monsno, NULL );
    OS_TFPrintf( PRINT_TARGET,
        "POKEMON-EGG: <MonsNo> father[%d], mother[%d], egg[%d]\n", 
        monsno_father, monsno_mother, monsno_egg );
  }
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 子ポケモンのベースとなる親ポケモンを取得する
 */
//---------------------------------------------------------------------------------------- 
static const POKEMON_PARAM* GetBasePokemon( const PARENT* parent )
{
  u32 monsno_father = PP_Get( parent->father, ID_PARA_monsno, NULL );
  u32 monsno_mother = PP_Get( parent->mother, ID_PARA_monsno, NULL );

  // 母親がメタモン ==> 父親
  if( monsno_mother == MONSNO_METAMON ) {
    return parent->father;
  }
  // 通常 ==> 母親
  else {
    return parent->mother;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 子ポケモンを変換する ( 例外ケース：マナフィ )
 *
 * マナフィの子はフィオネ
 */
//---------------------------------------------------------------------------------------- 
static u32 ChangeEggMonsNo_manafi( const POKEMON_PARAM* base_poke, u32 monsno_egg )
{
  u32 monsno_base;

  monsno_base = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( monsno_base == MONSNO_MANAFI ) {
    return MONSNO_FIONE;
  }
  else { 
    return monsno_egg;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 子ポケモンを変換する ( 例外ケース：アイテム )
 */
//---------------------------------------------------------------------------------------- 
static u32 ChangeEggMonsNo_item( const POKEMON_PARAM* base_poke, u32 monsno_egg )
{
  // 例外テーブル適用
  int i;
  u32 monsno_base = PP_Get( base_poke, ID_PARA_monsno, NULL );
  u32 itemno_base = PP_Get( base_poke, ID_PARA_item, NULL );

  const int table_size = 9;
  u32 exception_table[table_size][3] = // 例外テーブル
  {
    // 元ポケモン      // アイテム          // 子ポケモン
    {MONSNO_MARIRU,    ITEM_USIONOOKOU,     MONSNO_MARIRU},
    {MONSNO_SOONANSU,  ITEM_NONKINOOKOU,    MONSNO_SOONANSU},
    {MONSNO_BARIYAADO, ITEM_AYASIIOKOU,     MONSNO_BARIYAADO},
    {MONSNO_USOKKII,   ITEM_GANSEKIOKOU,    MONSNO_USOKKII},
    {MONSNO_KABIGON,   ITEM_MANPUKUOKOU,    MONSNO_KABIGON},
    {MONSNO_MANTAIN,   ITEM_SAZANAMINOOKOU, MONSNO_MANTAIN},
    {MONSNO_ROZERIA,   ITEM_OHANANOOKOU,    MONSNO_ROZERIA},
    {MONSNO_RAKKII,    ITEM_KOUUNNOOKOU,    MONSNO_RAKKII},
    {MONSNO_TIRIIN,    ITEM_KIYOMENOOKOU,   MONSNO_TIRIIN},
  };

  // 例外テーブルとの照合
  for( i=0; i<table_size; i++ )
  {
    // 元ポケが特定アイテムを持っていなかったら, 子ポケモンが変化
    if( ( monsno_base == exception_table[i][0] ) &&
        ( itemno_base != exception_table[i][1] ) ) {
      return exception_table[i][2];
    }
  }

  return monsno_egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの性格を決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_seikaku( const PARENT* parent, POKEMON_PARAM* egg )
{
  BOOL kawarazu_father; // 父親が『かわらずのいし』を持っているかどうか
  BOOL kawarazu_mother; // 母親が『かわらずのいし』を持っているかどうか
  u32 seikaku_father = PP_Get( parent->father, ID_PARA_seikaku, NULL ); // 父親の性格
  u32 seikaku_mother = PP_Get( parent->mother, ID_PARA_seikaku, NULL ); // 母親の性格
  u32 seikaku_egg; // 子の性格

  //『かわらずのいし』を持っているかどうかを取得
  kawarazu_father = ( PP_Get( parent->father, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );
  kawarazu_mother = ( PP_Get( parent->mother, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );

  // デフォルトは乱数で決定
  seikaku_egg = GFUser_GetPublicRand0( PTL_SEIKAKU_MAX );

  // 両親とも「かわらずのいし」を持っている
  if( kawarazu_father && kawarazu_mother ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      // 父親から継承
      if( GFUser_GetPublicRand0(2) == 0 )  {
        seikaku_egg = seikaku_father;
      }
      // 母親から継承
      else {
        seikaku_egg = seikaku_mother;
      }
    }
  }
  // 父親のみ持っている
  else if( kawarazu_father ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      seikaku_egg = seikaku_father;
    }
  }
  // 母親のみ持っている
  else if( kawarazu_mother ) {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(2) == 0 ) {
      seikaku_egg = seikaku_mother;
    }
  }

  // 性格をセット
  PP_Put( egg, ID_PARA_seikaku, seikaku_egg );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <Seikaku> father[%d], mother[%d] ==> egg[%d]\n", 
      seikaku_father, seikaku_mother, seikaku_egg );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの特性を決定する ( 母親の特性を遺伝する )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_tokusei( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id )
{
  u32 rnd;
  u32 speabi_egg_idx;    // 何番目の特性を継承するか
  u32 speabi_mother;     // 母の特性
  u32 speabi_mother_idx; // 母の特性は何番目のとくせいなのか
  u32 speabi_list[3];    // 特性リスト

  // 母親の特性を取得
  speabi_mother = PP_Get( parent->mother, ID_PARA_speabino, NULL );

  // 母の特性リスト ( 種の特性 ) を取得
  {
    POKEMON_PERSONAL_DATA* personalData;
    u32 monsno     = PP_Get( parent->mother, ID_PARA_monsno, NULL );
    u32 formno     = PP_Get( parent->mother, ID_PARA_form_no, NULL );
    personalData   = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
    speabi_list[0] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi1 );
    speabi_list[1] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi2 );
    speabi_list[2] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi3 );
    POKE_PERSONAL_CloseHandle( personalData );
  }
  
  // 母親の特性番号を調べる
  for( speabi_mother_idx=0; speabi_mother_idx<3; speabi_mother_idx++ )
  {
    // 発見
    if( speabi_mother == speabi_list[ speabi_mother_idx ] ) { break; }
  }
  // 母親が不正な特性を持っている場合
  if( 3 <= speabi_mother_idx ) { 
    OS_Printf( "母ポケモンの特性に誤りがあります。\n" );
    GF_ASSERT(0);
    speabi_mother_idx = 0; // 不正値なら, 特性1を継承することにする
  }

  // 継承する特性を決定
  rnd = GFUser_GetPublicRand0(100);  // 乱数取得[0, 99]
  switch( speabi_mother_idx ) {
  // 母が特性1
  case 0:
    if( rnd < 80 ) { speabi_egg_idx = 0; } // 80% ==> 子の特性1
    else           { speabi_egg_idx = 1; } // 20% ==> 子の特性2
    break;
  // 母が特性2
  case 1: 
    if( rnd < 20 ) { speabi_egg_idx = 0; } // 20% ==> 子の特性1
    else           { speabi_egg_idx = 1; } // 80% ==> 子の特性2
    break;
  // 母が特性3
  case 2: 
    if( rnd < 20 )      { speabi_egg_idx = 0; } // 20% ==> 子の特性1
    else if( rnd < 40 ) { speabi_egg_idx = 1; } // 20% ==> 子の特性2
    else                { speabi_egg_idx = 2; } // 60% ==> 子の特性3
    break;
  default:
    GF_ASSERT(0);
    break;
  }

  // 特性をセット
  if( speabi_egg_idx == 2 ) { // 第三特性
    u32 monsno = PP_Get( parent->mother, ID_PARA_monsno, NULL );
    u32 formno = PP_Get( parent->mother, ID_PARA_form_no, NULL );
    PP_SetTokusei3( egg, monsno, formno );
  }
  else {
    u32 speabi_egg = speabi_list[ speabi_egg_idx ]; 

    // 決定した番号に特性がない場合, 特性0にする
    if( speabi_egg == TOKUSYU_NULL ) {
      speabi_egg = speabi_list[0];
    } 
    PP_Put( egg, ID_PARA_speabino, speabi_egg );
  }


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <Tokusei> mother[%d](%d, %d, %d) ==> egg[%d]\n", 
      speabi_mother, speabi_list[0], speabi_list[1], speabi_list[2], 
      PP_Get( egg, ID_PARA_speabino, NULL ) );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個体乱数を決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_ability_rand( const PARENT* parent, POKEMON_PARAM* egg )
{
  int i = 0;
  u32 ability_type[3];  // 継承する個体乱数のタイプ
  int num = 0; // 継承する個体乱数のタイプの決定数

  // 父親が『パワーXXXX』を持っている場合
  {
    u32 itemno = PP_Get( parent->father, ID_PARA_item, NULL );
    switch( itemno ) {
    case ITEM_PAWAAUEITO:  ability_type[num++] = PTL_ABILITY_HP;    break;
    case ITEM_PAWAARISUTO: ability_type[num++] = PTL_ABILITY_ATK;   break;
    case ITEM_PAWAABERUTO: ability_type[num++] = PTL_ABILITY_DEF;   break;
    case ITEM_PAWAARENZU:  ability_type[num++] = PTL_ABILITY_SPATK; break;
    case ITEM_PAWAABANDO:  ability_type[num++] = PTL_ABILITY_SPDEF; break;
    case ITEM_PAWAAANKURU: ability_type[num++] = PTL_ABILITY_AGI;   break;
    }
  }

  // 母親が『パワーXXXX』を持っている場合
  if( ( num == 0 ) ||
      ( GFUser_GetPublicRand0(2) == 0 ) ) // 父親も持っていた場合, 1/2で上書きする
  {
    u32 itemno = PP_Get( parent->father, ID_PARA_item, NULL );
    num = 0;  // 父親の継承を上書き
    switch( itemno ) {
    case ITEM_PAWAAUEITO:  ability_type[num++] = PTL_ABILITY_HP;    break;
    case ITEM_PAWAARISUTO: ability_type[num++] = PTL_ABILITY_ATK;   break;
    case ITEM_PAWAABERUTO: ability_type[num++] = PTL_ABILITY_DEF;   break;
    case ITEM_PAWAARENZU:  ability_type[num++] = PTL_ABILITY_SPATK; break;
    case ITEM_PAWAABANDO:  ability_type[num++] = PTL_ABILITY_SPDEF; break;
    case ITEM_PAWAAANKURU: ability_type[num++] = PTL_ABILITY_AGI;   break;
    }
  }

  // 継承する個体乱数をランダムに選ぶ
  while( num < 3 )
  {
    // ランダムに選ぶ
    u32 type = GFUser_GetPublicRand0(PTL_ABILITY_MAX);

    // すでに選ばれていたら, 再抽選する
    for( i=0; i<num; i++ )
    {
      if( ability_type[i] == type ) { continue; }
    }

    // 継承する個体乱数のタイプを登録
    ability_type[num++] = type;
  }

  // 一応チェック
  GF_ASSERT( num == 3 );

  // 選んだ個体乱数を継承する 
  for( i=0; i<num; i++ )
  {
    const POKEMON_PARAM* param;
    int param_id;
    u32 value;

    // 父・母のどちらから受け継ぐのかを決定
    if( GFUser_GetPublicRand0(2) == 0 ) { param = parent->father; }
    else                                { param = parent->mother; }

    // 継承
    switch( ability_type[i] ) {
    case PTL_ABILITY_HP:    param_id = ID_PARA_hp_rnd;     break;
    case PTL_ABILITY_ATK:   param_id = ID_PARA_pow_rnd;    break;
    case PTL_ABILITY_DEF:   param_id = ID_PARA_def_rnd;    break;
    case PTL_ABILITY_SPATK: param_id = ID_PARA_spepow_rnd; break;
    case PTL_ABILITY_SPDEF: param_id = ID_PARA_spedef_rnd; break;
    case PTL_ABILITY_AGI:   param_id = ID_PARA_agi_rnd;    break;
    default: GF_ASSERT(0); break;
    }
    value = PP_Get( param, param_id, NULL );
    PP_Put( egg, param_id, value );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <AbilityRnd> father(%d, %d, %d, %d, %d, %d)\n", 
      PP_Get( parent->father, ID_PARA_hp_rnd, NULL ), PP_Get( parent->father, ID_PARA_pow_rnd, NULL ),
      PP_Get( parent->father, ID_PARA_def_rnd, NULL ), PP_Get( parent->father, ID_PARA_spepow_rnd, NULL ),
      PP_Get( parent->father, ID_PARA_spedef_rnd, NULL ), PP_Get( parent->father, ID_PARA_agi_rnd, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <AbilityRnd> mother(%d, %d, %d, %d, %d, %d)\n", 
      PP_Get( parent->mother, ID_PARA_hp_rnd, NULL ), PP_Get( parent->mother, ID_PARA_pow_rnd, NULL ),
      PP_Get( parent->mother, ID_PARA_def_rnd, NULL ), PP_Get( parent->mother, ID_PARA_spepow_rnd, NULL ),
      PP_Get( parent->mother, ID_PARA_spedef_rnd, NULL ), PP_Get( parent->mother, ID_PARA_agi_rnd, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <AbilityRnd> ==> egg(%d, %d, %d, %d, %d, %d)\n", 
      PP_Get( egg, ID_PARA_hp_rnd, NULL ), PP_Get( egg, ID_PARA_pow_rnd, NULL ),
      PP_Get( egg, ID_PARA_def_rnd, NULL ), PP_Get( egg, ID_PARA_spepow_rnd, NULL ),
      PP_Get( egg, ID_PARA_spedef_rnd, NULL ), PP_Get( egg, ID_PARA_agi_rnd, NULL ) );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief レアポケモンの抽選を行う
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_rare( const PARENT* parent, POKEMON_PARAM* egg )
{
  int i;
  u32 id, rnd;
  u32 country_father = PP_Get( parent->father, ID_PARA_country_code, NULL ); // 父の国コード
  u32 country_mother = PP_Get( parent->mother, ID_PARA_country_code, NULL ); // 母の国コード
  BOOL egg_is_rare = FALSE; // 子がレアポケかどうか
  BOOL chance_flag = TRUE; // レアポケの抽選を行うかどうか

  // 親同士の国コードが同じなら, 何もしない
  if( country_father == country_mother ) { chance_flag = FALSE; }

  // すでにレアなら, 何もしない
  id  = PP_Get( egg, ID_PARA_id_no, NULL );
  rnd = PP_Get( egg, ID_PARA_personal_rnd, NULL );
  if( POKETOOL_CheckRare( id, rnd ) ) { egg_is_rare = TRUE; }

  // 抽選
  if( (egg_is_rare == FALSE) && (chance_flag == TRUE) ) {

    // 最大 RARE_EGG_CHANCE 回の抽選を行う
    for( i=0; i<RARE_EGG_CHANCE; i++ )
    {
      rnd = GFUser_GetPublicRand0( 0xffffffff );

      // レアが出たら個性乱数を書き換え, 抽選を終了
      if( POKETOOL_CheckRare( id, rnd ) ) {
        PP_Put( egg, ID_PARA_personal_rnd, rnd );
        egg_is_rare = TRUE;
        break;
      }
    }
  }

  // ※レアが出なかったのであれば個性乱数は書き換えない

#ifdef DEBUG_PRINT_ON
  if( egg_is_rare ) {
    OS_TFPrintf( PRINT_TARGET,
        "POKEMON-EGG: <PersonalRnd> egg[%d] ==> Rare!!\n", 
        PP_Get( egg, ID_PARA_personal_rnd, NULL ) );
  }
  else {
    OS_TFPrintf( PRINT_TARGET,
        "POKEMON-EGG: <PersonalRnd> egg[%d] ==> NotRare\n", 
        PP_Get( egg, ID_PARA_personal_rnd, NULL ) );
  }
#endif
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
  POKEPER_WAZAOBOE_CODE wazaTable[ POKEPER_WAZAOBOE_TABLE_ELEMS ];

  // 基本情報を取得
  monsno = PP_Get( egg, ID_PARA_monsno, NULL );
  formno = PP_Get( egg, ID_PARA_form_no, NULL );
  level  = PP_Get( egg, ID_PARA_level, NULL );

  // Lv.1 で覚える技をセット
  POKE_PERSONAL_LoadWazaOboeTable( monsno, formno, wazaTable );
  i = 0;
  while( !POKEPER_WAZAOBOE_IsEndCode( wazaTable[i] ) )
  {
    int waza_lv = POKEPER_WAZAOBOE_GetLevel( wazaTable[i] );
    int waza_id = POKEPER_WAZAOBOE_GetWazaID( wazaTable[i] );
    if( level < waza_lv ) { break; }
    PokeWazaLearnByPush( egg, waza_id );
    i++;
  }


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <DefaultWaza> ==> egg(%d, %d, %d, %d)\n",
      PP_Get( egg, ID_PARA_waza1, NULL ), PP_Get( egg, ID_PARA_waza2, NULL ),
      PP_Get( egg, ID_PARA_waza3, NULL ), PP_Get( egg, ID_PARA_waza4, NULL ) );
#endif
}

//----------------------------------------------------------------------------------------
/**
 * @brief タマゴ技を覚えさせる
 *
 * タマゴから孵るポケモンのみが覚えられる技のうち, 父ポケモンが覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_egg( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id )
{
  int i, j;
  u32 monsno_egg;
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };
  u16 kowaza_num;
  u16* kowaza_table;

  // 子技テーブルを取得
  monsno_egg   = PP_Get( egg, ID_PARA_monsno, NULL );
  kowaza_table = GFL_ARC_LoadDataAlloc( ARCID_KOWAZA, monsno_egg, heap_id );
  kowaza_num   = kowaza_table[0];

  // 父親の全技をチェックする
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    u32 wazano = PP_Get( parent->father, id_para[i], NULL );

    // 父親の技がタマゴ技なら, 習得する
    for( j=1; j<=kowaza_num; j++ )
    {
      if( wazano == kowaza_table[j] ) {
        PokeWazaLearnByPush( egg, wazano );
        break;
      }
    }
  }

  // 子技テーブルを破棄
  GFL_HEAP_FreeMemory( kowaza_table );


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <TamagoWaza> father(%d, %d, %d, %d)\n",
      PP_Get( parent->father, ID_PARA_waza1, NULL ), PP_Get( parent->father, ID_PARA_waza2, NULL ),
      PP_Get( parent->father, ID_PARA_waza3, NULL ), PP_Get( parent->father, ID_PARA_waza4, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <TamagoWaza> ==> egg(%d, %d, %d, %d)\n",
      PP_Get( egg, ID_PARA_waza1, NULL ), PP_Get( egg, ID_PARA_waza2, NULL ),
      PP_Get( egg, ID_PARA_waza3, NULL ), PP_Get( egg, ID_PARA_waza4, NULL ) );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 両親の技を受け継ぐ
 *
 * 自分もレベルを上げることで覚える技のうち, 両親ともに覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_parent( const PARENT* parent, POKEMON_PARAM* egg )
{
  int i, j;
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };
  POKEPER_WAZAOBOE_CODE wazaTable[ POKEPER_WAZAOBOE_TABLE_ELEMS ]; // 子の技覚えテーブル

  // 子の技覚えテーブルを取得
  {
    u32 monsno = PP_Get( egg, ID_PARA_monsno, NULL );
    u32 formno = PP_Get( egg, ID_PARA_form_no, NULL );
    POKE_PERSONAL_LoadWazaOboeTable( monsno, formno, wazaTable );
  }

  // 子の全レベルアップ習得技をチェック
  i = 0;
  while( !POKEPER_WAZAOBOE_IsEndCode( wazaTable[i] ) )
  {
    u32  wazano      = POKEPER_WAZAOBOE_GetWazaID( wazaTable[i] );
    BOOL father_have = FALSE; // 父親が覚えている技かどうか
    BOOL mother_have = FALSE; // 母親が覚えている技かどうか

    // 父が覚えているかどうかチェック
    for( j=0; j<PTL_WAZA_MAX; j++ )
    {
      u32 wazano_father = PP_Get( parent->father, id_para[j], NULL );

      // 覚えている
      if( wazano_father == wazano ) {
        father_have = TRUE;
        break;
      }
    }

    // 母が覚えているかどうかチェック
    for( j=0; j<PTL_WAZA_MAX; j++ )
    {
      u32 wazano_mother = PP_Get( parent->mother, id_para[j], NULL );

      // 覚えている
      if( wazano_mother == wazano ) {
        mother_have = TRUE;
        break;
      }
    }

    // 両親とも覚えていたら継承する
    if( father_have && mother_have ) {
      PokeWazaLearnByPush( egg, wazano );
    }

    // 次の技へ
    i++;
  }


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <ParentWaza> father(%d, %d, %d, %d)\n",
      PP_Get( parent->father, ID_PARA_waza1, NULL ), PP_Get( parent->father, ID_PARA_waza2, NULL ),
      PP_Get( parent->father, ID_PARA_waza3, NULL ), PP_Get( parent->father, ID_PARA_waza4, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <ParentWaza> mother(%d, %d, %d, %d)\n",
      PP_Get( parent->mother, ID_PARA_waza1, NULL ), PP_Get( parent->mother, ID_PARA_waza2, NULL ),
      PP_Get( parent->mother, ID_PARA_waza3, NULL ), PP_Get( parent->mother, ID_PARA_waza4, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <ParentWaza> ==> egg(%d, %d, %d, %d)\n",
      PP_Get( egg, ID_PARA_waza1, NULL ), PP_Get( egg, ID_PARA_waza2, NULL ),
      PP_Get( egg, ID_PARA_waza3, NULL ), PP_Get( egg, ID_PARA_waza4, NULL ) );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父ポケモンのマシン技を受け継ぐ
 *
 * 自分も覚えることの出来るマシン技のうち, 父ポケモンが覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_machine( const PARENT* parent, POKEMON_PARAM* egg, HEAPID heap_id )
{
  int i, itemno;
  u32 wazano;
  u32 monsno_egg = PP_Get( egg, ID_PARA_monsno, NULL );
  u32 formno_egg = PP_Get( egg, ID_PARA_form_no, NULL );
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  // 父の全技をチェック
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    // 技コード取得
    wazano = PP_Get( parent->father, id_para[i], NULL );

    // 同技の技マシンが存在するかどうかチェック
    for( itemno=ITEM_WAZAMASIN01; itemno<=ITEM_DATA_MAX; itemno++ )
    {
      // 技マシンがあり, タマゴポケに使用できるなら, 習得する
      if( ( wazano == ITEM_GetWazaNo(itemno) ) &&
          ( IsWazaMachineAbleToUse(heap_id, monsno_egg, formno_egg, itemno) ) ) {
        PokeWazaLearnByPush( egg, wazano );
        break;
      }
    }
  }


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <MachineWaza> father(%d, %d, %d, %d)\n",
      PP_Get( parent->father, ID_PARA_waza1, NULL ), PP_Get( parent->father, ID_PARA_waza2, NULL ),
      PP_Get( parent->father, ID_PARA_waza3, NULL ), PP_Get( parent->father, ID_PARA_waza4, NULL ) );
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <MachineWaza> ==> egg(%d, %d, %d, %d)\n",
      PP_Get( egg, ID_PARA_waza1, NULL ), PP_Get( egg, ID_PARA_waza2, NULL ),
      PP_Get( egg, ID_PARA_waza3, NULL ), PP_Get( egg, ID_PARA_waza4, NULL ) );
#endif
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 特殊タマゴ作成処理 ( ピチュー )
 *        親ポケがアイテム『でんきだま』を持っていたら, 技『ボルテッカー』を習得
 *
 * @param father 父親ポケモン
 * @param mother 母親ポケモン
 * @param egg    設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_pityuu( const PARENT* parent, POKEMON_PARAM* egg )
{
  // ピチューのタマゴでなければ何もしない
  if( PP_Get( egg, ID_PARA_monsno, NULL ) != MONSNO_PITYUU ) { return; }

  // 両親のどちらかが『でんきだま』を持っていたら,『ボルテッカー』を習得
  if( ( PP_Get( parent->father, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) ||
      ( PP_Get( parent->mother, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) ) {
    PokeWazaLearnByPush( egg, WAZANO_BORUTEKKAA );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET, "POKEMON-EGG: <Pityuu> learn BORUTEKKAA\n" );
#endif 
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
static void EggCordinate_karanakusi( const PARENT* parent, POKEMON_PARAM* egg )
{
  u32 monsno_mother = PP_Get( parent->mother, ID_PARA_monsno, NULL );

  // 母親が『カラナクシ』or『トリトドン』なら, フォルムを継承
  if( (monsno_mother == MONSNO_KARANAKUSI) ||
      (monsno_mother == MONSNO_TORITODON) ) {

    u32 formno = PP_Get( parent->mother, ID_PARA_form_no, NULL );
    PP_Put( egg, ID_PARA_form_no, formno );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET, "POKEMON-EGG: <Karanakusi> derive form[%d]\n", formno );
#endif 
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 技をソートする
 *        習得している技を前詰めする
 *
 * @param egg  設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_waza_sort( POKEMON_PARAM* egg )
{
  int i, num;
  u32 wazano[ PTL_WAZA_MAX ]; 
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  // 全ての技を取得
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    wazano[i] = PP_Get( egg, id_para[i], NULL );
  }

  // 全ての技をクリア
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    PP_Put( egg, id_para[i], WAZANO_NULL );
  }

  // 覚えていた技を前詰めでセット
  for( num=0,i=0; i<PTL_WAZA_MAX; i++ )
  {
    if( wazano[i] != WAZANO_NULL ) {
      PP_Put( egg, id_para[num], wazano[i] );
      num++;
    }
  }


#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET,
      "POKEMON-EGG: <WazaSort> ==> egg(%d, %d, %d, %d)\n",
      PP_Get( egg, ID_PARA_waza1, NULL ), PP_Get( egg, ID_PARA_waza2, NULL ),
      PP_Get( egg, ID_PARA_waza3, NULL ), PP_Get( egg, ID_PARA_waza4, NULL ) );
#endif
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

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_TARGET, "POKEMON-EGG: <Finish>\n" );
#endif 
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
 * @brief 技を押し出し式に覚えさせる ( 重複チェック有 )
 *
 * @param poke
 * @param wazano
 */
//---------------------------------------------------------------------------------------- 
static void PokeWazaLearnByPush( POKEMON_PARAM* poke, u32 wazano )
{
  int i;
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  // 技番号が不正
  if( wazano == WAZANO_NULL ) { return; }

  // すでに覚えている
  if( CheckWazaHave(poke, wazano) ) { return; }

  // 空き要素にセット
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    if( PP_Get( poke, id_para[i], NULL ) == WAZANO_NULL ) {
      PP_Put( poke, id_para[i], wazano );
      return;
    }
  }

  // 空きがなければ押し出す
  PP_SetWazaPush( poke, wazano );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定した技を持っているかどうかをチェックする
 *
 * @param poke   チェックするポケモン
 * @param wazano チェックする技
 *
 * @return 指定した技を持っている場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------------- 
static BOOL CheckWazaHave( const POKEMON_PARAM* poke, u32 wazano )
{
  int i;
  u32 id_para[ PTL_WAZA_MAX ] = { ID_PARA_waza1, ID_PARA_waza2, ID_PARA_waza3, ID_PARA_waza4 };

  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    // 発見
    if( PP_Get( poke, id_para[i], NULL ) == wazano ) {
      return TRUE;
    }
  }

  return FALSE;
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

  // モンスターナンバーをチェック
  if( MONSNO_END <= monsno ) {
    OBATA_Printf( "IsWazaMachineAbleToUse: 無効なモンスターナンバーが指定されました。\n" );
    GF_ASSERT(0);
    return FALSE;
  }

  // アイテムが技マシンかどうかをチェック
	if( ITEM_CheckWazaMachine( itemno ) == FALSE ) {
    OBATA_Printf( "IsWazaMachineAbleToUse: 技マシン以外のアイテムが指定されました。\n" );
    return FALSE;
  } 

  // パーソナルデータをオープン
  ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );

  // 技マシン番号(ゼロオリジン)を取得
  machine_no = ITEM_GetWazaMashineNo( itemno );

  // 該当する技マシンフラグ・検査ビット位置を取得
  if( machine_no < 32*1 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine1 );
    bit_pos = machine_no;
  }
  else if( machine_no < 32*2 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine2 );
    bit_pos = machine_no - 32;
  }
  else if( machine_no < 32*3 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine3 );
    bit_pos = machine_no - 32*2;
  }
  else {
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
  if( MONSNO_END <= monsno ) {
    OBATA_Printf( "GetSeedMonsNo: 無効なモンスターナンバーが指定されました。\n" );
    GF_ASSERT(0);
    return monsno; // 不正値なら, そのまま返す
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
