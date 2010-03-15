//////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  育て屋さん
 * @file   sodateya.c
 * @author obata
 * @date   2009.10.01
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_personal.h"
#include "item/itemsym.h"
#include "item/item.h"
#include "waza_tool/wazano_def.h"
#include "savedata/sodateya_work.h" 
#include "fieldmap.h"
#include "sodateya.h" 
#include "print/global_msg.h"
#include "poke_tool/poke_memo.h"

#include "arc/arc_def.h"  // for ARCID_PMSTBL
#include "arc/kowaza.naix"
#include "gamesystem/game_data.h"
#include "savedata/mystatus.h"


//========================================================================================
// ■ 定数
//========================================================================================
#define CHARGE_MIN         (100) // 最小料金
#define CHARGE_PER_LEVEL   (100) // 1レベル毎の引き取り料金
#define EXP_PER_WALK         (1) // 歩くたびに加算する経験値
#define EGG_CHECK_INTERVAL (256) // 産卵判定の頻度
#define RARE_EGG_CHANCE      (5) // レアタマゴ抽選回数

// 相性(タマゴができる確率[%])
#define LOVE_LV_GOOD   (70)  // 「とっても よい ようじゃ」
#define LOVE_LV_NORMAL (50)  // 「まずまずの ようじゃ」
#define LOVE_LV_BAD    (20)  // 「それほど よくないがなぁ」
#define LOVE_LV_WORST   (0)  // 「ちがう ポケモンたちと あそんでおるがなぁ」


//========================================================================================
// ■育て屋さんワーク
//========================================================================================
struct _SODATEYA
{
  HEAPID           heapID; // ヒープID
  FIELDMAP_WORK* fieldmap; // フィールドマップ
  SODATEYA_WORK*     work; // 管理ワーク
  u16         layEggCount; // 産卵カウンタ
};


//========================================================================================
// ■プロトタイプ宣言
//========================================================================================
static u32 CalcExpAdd( u32 exp1, u32 exp2 );
static void GrowUpPokemon( POKEMON_PARAM* poke, u32 exp );
static void SortSodateyaPokemon( SODATEYA_WORK* work );
static u32 LoveCheck( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 );
static u32 CalcLoveLv_normal( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 );
static BOOL LayEggCheck( SODATEYA* sodateya );
static void CreateEgg( SODATEYA* sodateya, POKEMON_PARAM* egg );
static void EggCordinate_monsno( 
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_personality(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_special_ability(
    HEAPID heap_id, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_ability_rand(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_rare(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_waza_default( POKEMON_PARAM* egg );
static void EggCordinate_waza_egg( 
    HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg );
static void EggCordinate_waza_parent(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_waza_machine(
    HEAPID heap_id, const POKEMON_PARAM* father, POKEMON_PARAM* egg );
static void EggCordinate_pityuu(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_karanakusi(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg );
static void EggCordinate_finish( POKEMON_PARAM* egg, SODATEYA* sodateya );
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno );
static u16 GetSeedMonsNo( u16 monsno );


//========================================================================================
// ■作成・破棄
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 育て屋さんを作成する
 *
 * @param heap_id  使用するヒープID
 * @param fieldmap 関連付けるフィールドマップ
 * @param work     管理する育て屋ワーク
 * @return 作成した育て屋さん
 */
//---------------------------------------------------------------------------------------- 
SODATEYA* SODATEYA_Create( HEAPID heap_id, FIELDMAP_WORK* fieldmap, SODATEYA_WORK* work )
{
  SODATEYA* sodateya;

  // 育て屋さんワーク確保
  sodateya = (SODATEYA*)GFL_HEAP_AllocMemory( heap_id, sizeof(SODATEYA) );

  // ワーク初期化
  sodateya->heapID      = heap_id;
  sodateya->fieldmap    = fieldmap;
  sodateya->work        = work;
  sodateya->layEggCount = 0;
  return sodateya;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 育て屋さんを破棄する
 *
 * @param sodateya 破棄する育て屋さん
 */
//----------------------------------------------------------------------------------------
void SODATEYA_Delete( SODATEYA* sodateya )
{
  GFL_HEAP_FreeMemory( sodateya );
}


//========================================================================================
// ■ポケモン, タマゴの受け渡し
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief ポケモンを預かる
 *
 * @param sodateya 育て屋さん
 * @param party    預かるポケモンが属するパーティー
 * @param pos      預かるポケモンを指定
 */
//---------------------------------------------------------------------------------------- 
void SODATEYA_TakePokemon( SODATEYA* sodateya, POKEPARTY* party, int pos )
{
  int index;
  POKEMON_PARAM* poke;

  // 格納先インデックスを取得
  index = SODATEYA_WORK_GetPokemonNum( sodateya->work );

  // すでに最大数預かっている場合
  if( SODATEYA_POKE_MAX <= index )
  {
    OBATA_Printf( "--------------------------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_TakePokemon: すでに最大数のポケモンを預かっています。\n" );
    OBATA_Printf( "--------------------------------------------------------------\n" );
    return;
  }

  // ポケモンを移動 (手持ち→育て屋)
  poke = PokeParty_GetMemberPointer( party, pos );
  SODATEYA_WORK_SetPokemon( sodateya->work, index, poke );
  PokeParty_Delete( party, pos );
}

//----------------------------------------------------------------------------------------
/**
 * @brief ポケモンを返す
 *
 * @param sodateya 育て屋さん
 * @param index    返すポケモンを指定
 * @param party    返し先のパーティー
 */
//---------------------------------------------------------------------------------------- 
void SODATEYA_TakeBackPokemon( SODATEYA* sodateya, int index, POKEPARTY* party )
{
  const POKEMON_PARAM* poke;
  u32 exp;

  // 指定インデックスにポケモンがいない場合
  if( SODATEYA_WORK_IsValidPokemon( sodateya->work, index ) != TRUE )
  {
    OBATA_Printf( "------------------------------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_TakeBackPokemon: 指定インデックスには飼育ポケがいません。\n" );
    OBATA_Printf( "------------------------------------------------------------------\n" );
    return;
  }

  // ポケモンを成長させる
  poke = SODATEYA_WORK_GetPokemon( sodateya->work, index );
  exp  = SODATEYA_WORK_GetGrowUpExp( sodateya->work, index );
  GrowUpPokemon( (POKEMON_PARAM*)poke, exp );

  // ポケモンを移動 (育て屋→手持ち)
  PokeParty_Add( party, poke );
  SODATEYA_WORK_ClrPokemon( sodateya->work, index );
  SortSodateyaPokemon( sodateya->work );
}

//----------------------------------------------------------------------------------------
/**
 * @brief タマゴを渡す
 *
 * @param sodateya 育て屋さん
 * @param party    渡す先のパーティー
 */
//---------------------------------------------------------------------------------------- 
void SODATEYA_TakeBackEgg( SODATEYA* sodateya, POKEPARTY* party )
{
  POKEMON_PARAM* egg = PP_Create( 1, 1, 1, sodateya->heapID );

  // タマゴが存在しない場合
  if( SODATEYA_WORK_IsValidEgg( sodateya->work ) != TRUE )
  {
    OBATA_Printf( "------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_TakeBackEgg: タマゴがありません。\n" );
    OBATA_Printf( "------------------------------------------\n" );
    return;
  }

  // タマゴを作成し, 手持ちに追加
  CreateEgg( sodateya, egg );
  PokeParty_Add( party, egg );

  // タマゴを削除
  SODATEYA_WORK_ClrEgg( sodateya->work ); 
  GFL_HEAP_FreeMemory( egg );
}

//----------------------------------------------------------------------------------------
/**
 * @brief タマゴを破棄する
 *
 * @param sodateya 育て屋さん
 */
//---------------------------------------------------------------------------------------- 
void SODATEYA_DeleteEgg( SODATEYA* sodateya )
{
  // タマゴが存在しない場合
  if( SODATEYA_WORK_IsValidEgg( sodateya->work ) != TRUE )
  {
    OBATA_Printf( "------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_TakeBackEgg: タマゴがありません。\n" );
    OBATA_Printf( "------------------------------------------\n" );
  }

  SODATEYA_WORK_ClrEgg( sodateya->work );
}


//========================================================================================
// ■育て屋さん仕事関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief ポケモンを育てる (1歩分)
 *
 * @param sodateya ポケモンを育てる育て屋さん
 */
//---------------------------------------------------------------------------------------- 
BOOL SODATEYA_BreedPokemon( SODATEYA* sodateya )
{
  // 経験値を加算
  SODATEYA_WORK_AddGrowUpExp( sodateya->work, EXP_PER_WALK );

  // 産卵チェック
  if( LayEggCheck(sodateya) == TRUE )
  {
    // タマゴが産まれたことを記憶
    SODATEYA_WORK_SetEgg( sodateya->work );
    return TRUE;
  }
  return FALSE;
}


//========================================================================================
// ■取得関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @breif タマゴの有無を取得する
 *
 * @param sodateya 育て屋さん
 * @return タマゴがある場合 TRUE
 */
//----------------------------------------------------------------------------------------
BOOL SODATEYA_HaveEgg( const SODATEYA* sodateya )
{
  return SODATEYA_WORK_IsValidEgg( sodateya->work );
}

//----------------------------------------------------------------------------------------
/**
 * @breif 飼育中のポケモンの数を取得する
 *
 * @param sodateya 育て屋さん
 * @return 飼育中のポケモン数
 */
//----------------------------------------------------------------------------------------
u8 SODATEYA_GetPokemonNum( const SODATEYA* sodateya )
{
  return SODATEYA_WORK_GetPokemonNum( sodateya->work );
}

//----------------------------------------------------------------------------------------
/**
 * @brief 飼育ポケモンについて, 現在のレベルを取得する
 *
 * @param sodateya 育て屋さん
 * @param index    対象となるポケモンを指定
 * @return 指定ポケモンの現時点のレベル
 */
//---------------------------------------------------------------------------------------- 
u32 SODATEYA_GetPokeLv_Current( const SODATEYA* sodateya, int index )
{
  u32 monsno, formno;
  u64 before_exp, after_exp;
  u32 before_level, after_level;
  const POKEMON_PARAM* poke;

  // 指定インデックスにポケモンがいない場合
  if( SODATEYA_WORK_IsValidPokemon( sodateya->work, index ) != TRUE )
  {
    OBATA_Printf( "--------------------------------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_GetPokeLv_Current: 指定インデックスには飼育ポケがいません。\n" );
    OBATA_Printf( "--------------------------------------------------------------------\n" );
    return 1;
  }

  // 指定ポケモンのパラメータを取得
  poke   = SODATEYA_WORK_GetPokemon( sodateya->work, index );
  monsno = PP_Get( poke, ID_PARA_monsno, NULL );
  formno = PP_Get( poke, ID_PARA_form_no, NULL );

  // 預けた時のレベルを取得
  before_exp   = PP_Get( poke, ID_PARA_exp, NULL );
  before_level = PP_Get( poke, ID_PARA_level, NULL );

  // 現在のレベルを算出
  after_exp   = CalcExpAdd( before_exp, SODATEYA_WORK_GetGrowUpExp( sodateya->work, index ) );
  after_level = POKETOOL_CalcLevel( monsno, formno, after_exp );

  // 現在のレベルを返す
  return after_level;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 飼育ポケモンについて, 育ったレベルを取得する
 *
 * @param sodateya 育て屋さん
 * @param index    対象となるポケモンを指定
 * @return 指定ポケモンの成長レベル数
 */
//---------------------------------------------------------------------------------------- 
u32 SODATEYA_GetPokeLv_GrowUp( const SODATEYA* sodateya, int index )
{
  u32 before_level, after_level;
  const POKEMON_PARAM* poke;

  // 預けた時 & 現在のレベルを取得
  poke         = SODATEYA_WORK_GetPokemon( sodateya->work, index );
  before_level = PP_Get( poke, ID_PARA_level, NULL );
  after_level  = SODATEYA_GetPokeLv_Current( sodateya, index );

  // 育ったレベルを返す
  return ( after_level - before_level );
}

//----------------------------------------------------------------------------------------
/**
 * @brief 飼育ポケモンの引き取り料金を取得する
 *
 * @param sodateya 育て屋さん
 * @param index    対象となるポケモンを指定
 * @return 指定ポケモンの引き取り金額
 */
//---------------------------------------------------------------------------------------- 
u32 SODATEYA_GetCharge( const SODATEYA* sodateya, int index )
{
  u32 grow_level;
  u32 charge;
  
  // 料金計算 (基本料 + レベル代)
  grow_level = SODATEYA_GetPokeLv_GrowUp( sodateya, index );
  charge     = CHARGE_MIN + (grow_level * CHARGE_PER_LEVEL); 
  return charge;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 飼育ポケモンの相性を取得する
 *
 * @param sodateya 育て屋さん
 * @return 飼育ポケモン同士の相性[%]
 */
//---------------------------------------------------------------------------------------- 
u8 SODATEYA_GetLoveLevel( const SODATEYA* sodateya )
{
  const POKEMON_PARAM* poke1;
  const POKEMON_PARAM* poke2;

  // 飼育ポケモンが2体いない場合
  if( SODATEYA_WORK_GetPokemonNum( sodateya->work ) < 2 )
  {
    OBATA_Printf( "--------------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_GetLoveLevel: 預けポケモンが2体いません。\n" );
    OBATA_Printf( "--------------------------------------------------\n" );
    return LOVE_LV_WORST;
  }

  // ポケモンパラメータ取得
  poke1 = SODATEYA_WORK_GetPokemon( sodateya->work, 0 );
  poke2 = SODATEYA_WORK_GetPokemon( sodateya->work, 1 );

  // 相性を返す
  return LoveCheck( poke1, poke2 );
}

//----------------------------------------------------------------------------------------
/**
 * @brief 飼育ポケモンの預かった当時のパラメータを取得する
 *
 * @param sodateya 育て屋さん
 * @param index    対象となるポケモンを指定
 * @return 指定ポケモンの飼育開始当初のパラメータ
 */
//---------------------------------------------------------------------------------------- 
const POKEMON_PARAM* SODATEYA_GetPokemonParam( const SODATEYA* sodateya, int index )
{
  // 指定インデックスにポケモンがいない場合
  if( SODATEYA_WORK_IsValidPokemon( sodateya->work, index ) != TRUE )
  {
    OBATA_Printf( "------------------------------------------------------------------\n" );
    OBATA_Printf( "SODATEYA_GetPokemonParam: 指定インデックスには飼育ポケがいません。\n" );
    OBATA_Printf( "------------------------------------------------------------------\n" );
  }

  return SODATEYA_WORK_GetPokemon( sodateya->work, index );
}


#if 0
//----------------------------------------------------------------------------------------
/**
 * @brief ワーク取得
 *
 * @param sodateya 育て屋さん
 * @return 育て屋ワーク
 */
//---------------------------------------------------------------------------------------- 
SODATEYA_WORK* SODATEYA_GetSodateyaWork( const SODATEYA* sodateya )
{
  return sodateya->work;
}
#endif


//========================================================================================
// ■非公開関数
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief 経験値の加算を行う
 *
 * @param exp1 経験値1
 * @param exp2 経験値2
 *
 * @return 経験値1 + 経験値2
 *        ( u32 でオーバーフローを起こした場合 最大値である 0xffffffff を返す )
 */
//---------------------------------------------------------------------------------------- 
static u32 CalcExpAdd( u32 exp1, u32 exp2 )
{
  u64 sum;

  sum = (u64)exp1 + (u64)exp2;

  // 加算値を補正
  if( 0xffffffff < sum )
  {
    sum = 0xffffffff;
  } 
  return sum;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief ポケモンを成長させる
 *
 * @param poke 成長させるポケモン
 * @param addExp  加算する経験値
 */
//---------------------------------------------------------------------------------------- 
static void GrowUpPokemon( POKEMON_PARAM* poke, u32 addExp )
{
  int i;
  u32 monsno, formno, exp;
  int before_lv, after_lv;
  POKEPER_WAZAOBOE_CODE waza_table[POKEPER_WAZAOBOE_TABLE_ELEMS];

  // 基本情報を取得
  monsno = PP_Get( poke, ID_PARA_monsno, NULL );
  formno = PP_Get( poke, ID_PARA_form_no, NULL );

  // 預かった当時のレベルを取得
  before_lv = PP_Get( poke, ID_PARA_level, NULL ); 

  // 経験値を加算し, パラメータを再計算
  exp = PP_Get( poke, ID_PARA_exp, NULL );
  exp = CalcExpAdd( exp, addExp );
  PP_Put( poke, ID_PARA_exp, exp );
  PP_Renew( poke );

  // 成長後のレベルを取得
  after_lv = PP_Get( poke, ID_PARA_level, NULL );

  // 技を更新 (成長過程で覚える技をプッシュ)
  POKE_PERSONAL_LoadWazaOboeTable( monsno, formno, waza_table );
  i = 0;
  while( POKEPER_WAZAOBOE_IsEndCode( waza_table[i] ) )
  {
    int waza_lv = POKEPER_WAZAOBOE_GetLevel( waza_table[i] );
    int waza_id = POKEPER_WAZAOBOE_GetWazaID( waza_table[i] );
    if( after_lv < waza_lv ) break;
    if( before_lv < waza_lv ) PP_SetWazaPush( poke, waza_id );
    i++;
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 飼育ポケモンを並べ替える (前詰めになるようにする)
 *
 * @param work 育て屋ワーク
 */
//---------------------------------------------------------------------------------------- 
static void SortSodateyaPokemon( SODATEYA_WORK* work )
{
  int i;
  int j;

  // 空き要素を検索
  for( i=0; i<SODATEYA_POKE_MAX; i++ )
  {
    // 空き要素を発見したら, 以降のデータを詰める
    if( SODATEYA_WORK_IsValidPokemon( work, i ) != TRUE )
    {
      for( j=i+1; j<SODATEYA_POKE_MAX; j++ )
      {
        if( SODATEYA_WORK_IsValidPokemon( work, j ) == TRUE )
        {
          SODATEYA_WORK_ExchangePokemon( work, j, j-1 );
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定したポケモンの相性をチェックする
 *
 * @param poke1 ポケモン1
 * @param poke2 ポケモン2
 *
 * @return 相性[%]
 */
//---------------------------------------------------------------------------------------- 
static u32 LoveCheck( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 )
{
  int i;
  u32 monsno[2];
  u32 id_no[2];
  u32 form[2];
  u32 sex[2];
  u32 egg_group1[2];
  u32 egg_group2[2];
  const POKEMON_PARAM* param[2];

  // 2体それぞれのパラメータを取得
  param[0] = poke1;
  param[1] = poke2;
  for( i=0; i<2; i++ )
  {
    monsno[i]     = PP_Get( param[i], ID_PARA_monsno, NULL );
    id_no[i]      = PP_Get( param[i], ID_PARA_id_no, NULL );
    form[i]       = PP_Get( param[i], ID_PARA_form_no, NULL );
    sex[i]        = PP_Get( param[i], ID_PARA_sex, NULL );
    egg_group1[i] = POKETOOL_GetPersonalParam( monsno[i], form[i], POKEPER_ID_egg_group1 );
    egg_group2[i] = POKETOOL_GetPersonalParam( monsno[i], form[i], POKEPER_ID_egg_group2 );
  }

  //---------------------------
  // こづくりグループ『無生殖』
  //---------------------------
  // どちらかのタマゴグループが『無生殖』==> ×
  if( ( egg_group1[0] == POKEPER_EGG_GROUP_MUSEISYOKU ) ||
      ( egg_group1[1] == POKEPER_EGG_GROUP_MUSEISYOKU ) ||
      ( egg_group2[0] == POKEPER_EGG_GROUP_MUSEISYOKU ) ||
      ( egg_group2[1] == POKEPER_EGG_GROUP_MUSEISYOKU )  )
  {
    return LOVE_LV_WORST;
  }

  //----------
  // メタモン
  //----------
  // 両方ともメタモン ==> ×
  if( ( monsno[0] == MONSNO_METAMON ) &&
      ( monsno[1] == MONSNO_METAMON ) )
  {
    return LOVE_LV_WORST;
  }
  // どちらか一方がメタモン ==> 通常判定
  else if( ( monsno[0] == MONSNO_METAMON ) ||
           ( monsno[1] == MONSNO_METAMON ) )
  {
    return CalcLoveLv_normal( poke1, poke2 );
  }

  //-------
  // 性別
  //-------
  // 同性 ==> ×
  if( sex[0] == sex[1] ) return LOVE_LV_WORST;

  // どちらかが性別なし ==> ×
  if( ( sex[0] == PTL_SEX_UNKNOWN ) ||
      ( sex[1] == PTL_SEX_UNKNOWN ) )
  {
    return LOVE_LV_WORST;
  }

  //---------------------------------
  // こづくりグループ『無生殖』以外
  //---------------------------------
  // グループが違う ==> ×
  if( ( egg_group1[0] != egg_group1[1] ) && 
      ( egg_group1[0] != egg_group2[1] ) &&
      ( egg_group2[0] != egg_group1[1] ) &&
      ( egg_group2[0] != egg_group2[1] )  )
  {
    return LOVE_LV_WORST;
  }
  // グループが同じ ==> 通常判定
  else 
  {
    return CalcLoveLv_normal( poke1, poke2 );
  }

  // ココに処理は来ないハズ!!
  OBATA_Printf( "-------------------------------------\n" );
  OBATA_Printf( "LoveCheck: 意図しない処理フロー発生!!\n" );
  OBATA_Printf( "-------------------------------------\n" );
  return LOVE_LV_WORST;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定したポケモンの相性を求める (同じタマゴグループの場合)
 *
 * [ルール]
 *  同種 and 同ID then ◎
 *  同種 and 異ID then ○
 *  異種 and 同ID then △
 *  異種 and 異ID then ○
 *
 * @param poke1 ポケモン1
 * @param poke2 ポケモン2
 *
 * @return 相性[%]
 */
//---------------------------------------------------------------------------------------- 
static u32 CalcLoveLv_normal( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 )
{
  int i;
  u32 monsno[2];
  u32 id_no[2];
  const POKEMON_PARAM* param[2];

  // 2体それぞれのパラメータを取得
  param[0] = poke1;
  param[1] = poke2;
  for( i=0; i<2; i++ )
  {
    monsno[i]     = PP_Get( param[i], ID_PARA_monsno, NULL );
    id_no[i]      = PP_Get( param[i], ID_PARA_id_no, NULL );
  }

  // ポケモンの種類が違う場合
  if( monsno[0] != monsno[1] ) 
  {
    if( id_no[0] == id_no[1] ) // IDが同じ ==> △
    {
      return LOVE_LV_BAD;
    }
    else                       // IDが違う ==> ○
    {
      return LOVE_LV_NORMAL;
    }
  }
  // ポケモンの種類が同じ場合
  else 
  {
    if( id_no[0] == id_no[1] ) // IDが同じ ==> ○
    {
      return LOVE_LV_NORMAL;
    }
    else                       // IDが違う ==> ◎
    {
      return LOVE_LV_GOOD;
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 産卵チェック
 *
 * @param sodateya チェックする育て屋さん
 * @return タマゴが産まれる場合 TRUE
 */
//---------------------------------------------------------------------------------------- 
static BOOL LayEggCheck( SODATEYA* sodateya )
{
  // 一定間隔で判定
  if( EGG_CHECK_INTERVAL < sodateya->layEggCount++ )
  {
    // 産卵カウンタをリセット
    sodateya->layEggCount = 0;

    // 2体預けていたら産卵判定
    if( 2 <= SODATEYA_WORK_GetPokemonNum( sodateya->work ) )
    { 
      // 乱数 < 相性 ==> 産卵
      if( GFUser_GetPublicRand0(100) < SODATEYA_GetLoveLevel(sodateya) ) 
      {
        return TRUE; 
      }
    }
  }

  // 産まない
  return FALSE;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを作成する
 *
 * @param sodateya 育て屋さん
 * @param egg      作成したタマゴパラメータの格納先
 */
//---------------------------------------------------------------------------------------- 
static void CreateEgg( SODATEYA* sodateya, POKEMON_PARAM* egg )
{
  int i;
  HEAPID heap_id = sodateya->heapID;
  const POKEMON_PARAM* father = NULL;
  const POKEMON_PARAM* mother = NULL;

  // 父・母ポケモンを取得
  for( i=0; i<2; i++ )
  {
    const POKEMON_PARAM* pp = SODATEYA_WORK_GetPokemon( sodateya->work, i );
    u32 sex = PP_GetSex( pp );
    switch( sex )
    {
    case PTL_SEX_MALE:   father = pp; break; 
    case PTL_SEX_FEMALE: mother = pp; break;
    default:
      if( !father ) father = pp;
      if( !mother ) mother = pp;
      break;
    }
  }

  // タマゴを作成
  EggCordinate_monsno( father, mother, egg );              // モンスターナンバー
  EggCordinate_personality( father, mother, egg );         // 性格
  EggCordinate_special_ability( heap_id, mother, egg );    // 特性
  EggCordinate_ability_rand( father, mother, egg );        // 個体乱数
  EggCordinate_rare( father, mother, egg );                // レアポケ抽選
  EggCordinate_waza_default( egg );                        // 技 (デフォルト)
  EggCordinate_waza_egg( heap_id, father, egg );           // 技 (タマゴ技)
  EggCordinate_waza_parent( father, mother, egg );         // 技 (両親共通の技)
  EggCordinate_waza_machine( heap_id, father, egg );       // 技 (父のマシン技)
  EggCordinate_pityuu( father, mother, egg );              // 特殊タマゴ (ピチュー)
  EggCordinate_karanakusi( father, mother, egg );          // 特殊タマゴ (カラナクシ)
  EggCordinate_finish( egg, sodateya );                    // 仕上げ
}

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
static void EggCordinate_personality(
    const POKEMON_PARAM* father, const POKEMON_PARAM* mother, POKEMON_PARAM* egg )
{
  BOOL kawarazu_f; // 父親が『かわらずのいし』を持っているかどうか
  BOOL kawarazu_m; // 母親が『かわらずのいし』を持っているかどうか
  u32 personal_f = PP_Get( father, ID_PARA_personal_rnd, NULL );    // 父親の個性乱数
  u32 personal_m = PP_Get( mother, ID_PARA_personal_rnd, NULL );    // 母親の個性乱数

  //『かわらずのいし』を持っているかどうかを取得
  kawarazu_f = ( PP_Get( father, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );
  kawarazu_m = ( PP_Get( mother, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );

  //『かわらずのいし』を持っていたら, 個性乱数を継承
  if( kawarazu_f && kawarazu_m ) // 両親とも持っている
  {
    // 1/2で継承する
    if( GFUser_GetPublicRand0(100)%2 == 0 )
    {
      if( GFUser_GetPublicRand0(100)%2 == 0 )  // 1/2で父親から継承
      {
        PP_Put( egg, ID_PARA_personal_rnd, personal_f );
      }
      else                             // 1/2で母親から継承
      {
        PP_Put( egg, ID_PARA_personal_rnd, personal_m );
      }
    }
  }
  else if( kawarazu_f ) // 父親のみ持っている ==> 1/2で継承
  {
    if( GFUser_GetPublicRand0(100)%2 == 0 )
    {
      PP_Put( egg, ID_PARA_personal_rnd, personal_f );
    }
  }
  else if( kawarazu_m ) // 母親のみ持っている ==> 1/2で継承
  {
    if( GFUser_GetPublicRand0(100)%2 == 0 ) 
    {
      PP_Put( egg, ID_PARA_personal_rnd, personal_m );
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの特性を決定する
 *        母親の特性を遺伝する
 *
 * @param heap_id 使用するヒープID
 * @param mother  母親ポケモン
 * @param egg     設定するタマゴ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_special_ability(
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
    OBATA_Printf( "----------------------------------\n" );
    OBATA_Printf( "母ポケモンの特性に誤りがあります。\n" );
    OBATA_Printf( "----------------------------------\n" );
    speabi_index = 0; // とりあえず, 特性1を継承することにする
  }

  // 継承する特性を決定
  rnd = GFUser_GetPublicRand0(100);  // 乱数取得[0, 99]
  switch( speabi_index )
  {
  case 0: //---------------------------------------------- 母が特性1
    if( rnd < 80 ) speabi_egg = speabi_list[0]; // 80% ==> 子の特性1
    else           speabi_egg = speabi_list[1]; // 20% ==> 子の特性2
    break;
  case 1: //---------------------------------------------- 母が特性2
    if( rnd < 20 ) speabi_egg = speabi_list[0]; // 20% ==> 子の特性1
    else           speabi_egg = speabi_list[1]; // 80% ==> 子の特性2
    break;
  case 2: //---------------------------------------------- 母が特性3
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
      ( GFUser_GetPublicRand0(100)%2 == 0 ) )    // 父親も持っていた場合, 1/2で上書きする
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
    if( GFUser_GetPublicRand0(100)%2 == 0 ) pp = father;
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
    for( itemno=ITEM_WAZAMASIN01; itemno<=ITEM_WAZAMASIN92; itemno++ )
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
 * @param sodateya 育て屋さん
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_finish( POKEMON_PARAM* egg, SODATEYA* sodateya )
{
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( sodateya->fieldmap );
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
  MYSTATUS* mystatus = GAMEDATA_GetMyStatus( gdata );

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
    STRBUF* name = MyStatus_CreateNameString( mystatus, sodateya->heapID );
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

    monsno   = PP_Get( egg, ID_PARA_monsno, NULL );
    formno   = PP_Get( egg, ID_PARA_form_no, NULL );
    personal = POKE_PERSONAL_OpenHandle( monsno, formno, sodateya->heapID );
    birth    = POKE_PERSONAL_GetParam( personal, POKEPER_ID_egg_birth );
    POKE_PERSONAL_CloseHandle( personal );

    // タマゴの間は, なつき度を孵化カウンタとして利用する
    PP_Put( egg, ID_PARA_friend, birth );
  }

  // トレーナーメモ
  {
    GAMESYS_WORK* gameSystem;
    GAMEDATA* gameData;
    MYSTATUS* mystatus;

    gameSystem = FIELDMAP_GetGameSysWork( sodateya->fieldmap );
    gameData = GAMESYSTEM_GetGameData( gameSystem );
    mystatus = GAMEDATA_GetMyStatus( gameData );

    POKE_MEMO_SetTrainerMemoPP( 
        egg, POKE_MEMO_EGG_FIRST, mystatus, POKE_MEMO_PERSON_SODATEYA, sodateya->heapID );
  }

  // タマゴフラグ
  PP_Put( egg, ID_PARA_tamago_flag, TRUE );
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
    OBATA_Printf( "------------------------------------------------------------------\n" );
    OBATA_Printf( "IsWazaMachineAbleToUse: 無効なモンスターナンバーが指定されました。\n" );
    OBATA_Printf( "------------------------------------------------------------------\n" );
    return FALSE;
  } 
  // 技マシン以外のアイテムが指定された場合
  if( (itemno < ITEM_WAZAMASIN01 ) || (ITEM_HIDENMASIN08 < itemno ) )
  {
    OBATA_Printf( "----------------------------------------------------------------\n" );
    OBATA_Printf( "IsWazaMachineAbleToUse: 技マシン以外のアイテムが指定されました。\n" );
    OBATA_Printf( "----------------------------------------------------------------\n" );
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
    OBATA_Printf( "---------------------------------------------------------\n" );
    OBATA_Printf( "GetSeedMonsNo: 無効なモンスターナンバーが指定されました。\n" );
    OBATA_Printf( "---------------------------------------------------------\n" );
    return monsno; // とりあえず, そのまま返す
  } 

  // 種ポケモンNoを取得
  GFL_ARC_LoadDataOfs( &seed_monsno, ARCID_PMSTBL, monsno, 0, sizeof(u16) );
  return seed_monsno;
} 



//========================================================================================
// ■タマゴの孵化
//========================================================================================

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
    OBATA_Printf( "id = %d\n", id );
    OBATA_Printf( "id = %d\n", PP_Get(egg,ID_PARA_id_no,NULL) );
  } 
}
