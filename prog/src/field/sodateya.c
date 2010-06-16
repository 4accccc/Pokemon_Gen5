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
#include "gamesystem/game_data.h"
#include "savedata/mystatus.h"
#include "savedata/perapvoice.h"
#include "savedata/sodateya_work.h" 
#include "savedata/zukan_savedata.h" 
#include "print/global_msg.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/status_rcv.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_personal.h"
#include "poke_tool/poke_memo.h"
#include "waza_tool/wazano_def.h" // for WAZANO_xxxx 

#include "fieldmap.h"
#include "pokemon_egg.h" 

#include "arc/arc_def.h" // for ARCID_PMSTBL
#include "../../../resource/fldmapdata/flagwork/work_define.h" // for WK_OTHER_DISCOVER_EGG

#include "sodateya.h" 

//========================================================================================
// ■ 定数
//========================================================================================
#define CHARGE_MIN         (100) // 最小料金
#define CHARGE_PER_LEVEL   (100) // 1レベル毎の引き取り料金
#define EXP_PER_WALK         (1) // 歩くたびに加算する経験値
#define EGG_CHECK_INTERVAL (256) // 産卵判定の頻度

// 相性(タマゴができる確率[%])
#define LOVE_LV_GOOD   (70)  // 「とっても よい ようじゃ」
#define LOVE_LV_NORMAL (50)  // 「まずまずの ようじゃ」
#define LOVE_LV_BAD    (20)  // 「それほど よくないがなぁ」
#define LOVE_LV_WORST   (0)  // 「ちがう ポケモンたちと あそんでおるがなぁ」


//========================================================================================
// ■育て屋さんワーク
//========================================================================================
struct _SODATEYA {

  HEAPID         heapID;      // ヒープID
  FIELDMAP_WORK* fieldmap;    // フィールドマップ
  SODATEYA_WORK* work;        // 管理ワーク
  u16            layEggCount; // 産卵カウンタ

};


//========================================================================================
// ■プロトタイプ宣言
//========================================================================================
static u32 CalcExpAdd( u32 exp1, u32 exp2 );
static void GrowUpPokemon( POKEMON_PARAM* poke, u32 exp );
static void LearnNewWaza( POKEMON_PARAM* poke, u32 wazano );
static BOOL CheckWazaHave( const POKEMON_PARAM* poke, u32 wazano );
static void SortSodateyaPokemon( SODATEYA_WORK* work );
static u32 LoveCheck( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 );
static u32 CalcLoveLv_normal( const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2 );
static BOOL LayEggCheck( SODATEYA* sodateya );
static void CreateEgg( SODATEYA* sodateya, POKEMON_PARAM* egg );
static void DeletePerapVoice( FIELDMAP_WORK* fieldmap, POKEPARTY* party );
static void SetR03OldmanEventWork( FIELDMAP_WORK* fieldmap );
static void ResetR03OldmanEventWork( FIELDMAP_WORK* fieldmap );
static void RecoverPokemon( POKEMON_PARAM* poke );
static void ResetSheimiForm( POKEMON_PARAM* poke );
static void RegisterPokemonInZukan( SODATEYA* sodateya, POKEMON_PARAM* poke );


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
  poke  = PokeParty_GetMemberPointer( party, pos );

  // すでに最大数預かっている場合
  if( SODATEYA_POKE_MAX <= index ) {
    OBATA_Printf( "SODATEYA_TakePokemon: すでに最大数のポケモンを預かっています。\n" );
    return;
  }

  // ポケモンを移動 (手持ち→育て屋)
  RecoverPokemon( poke );
  ResetSheimiForm( poke );
  SODATEYA_WORK_SetPokemon( sodateya->work, index, poke );
  PokeParty_Delete( party, pos );

  // ぺラップ録音データを削除
  DeletePerapVoice( sodateya->fieldmap, party );
}

/**
 * @brief ポケモンを全回復させる
 */
void RecoverPokemon( POKEMON_PARAM* poke )
{
  STATUS_RCV_PokeParam_RecoverAll( poke );
} 

/**
 * @brief シェイミをランドフォルムに戻す
 */
static void ResetSheimiForm( POKEMON_PARAM* poke )
{
  u32 monsno = PP_Get( poke, ID_PARA_monsno, NULL );

  if( monsno == MONSNO_SHEIMI ) {
    PP_Put( poke, ID_PARA_form_no, FORMNO_SHEIMI_LAND );
  }
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
  if( SODATEYA_WORK_IsValidPokemon( sodateya->work, index ) != TRUE ) {
    OBATA_Printf( "SODATEYA_TakeBackPokemon: 指定インデックスには飼育ポケがいません。\n" );
    return;
  }

  // ポケモンを成長させる
  poke = SODATEYA_WORK_GetPokemon( sodateya->work, index );
  exp  = SODATEYA_WORK_GetGrowUpExp( sodateya->work, index );
  GrowUpPokemon( (POKEMON_PARAM*)poke, exp );

  // 図鑑登録
  RegisterPokemonInZukan( sodateya, (POKEMON_PARAM*)poke );

  // ポケモンを移動 (育て屋→手持ち)
  PokeParty_Add( party, poke );
  SODATEYA_WORK_ClrPokemon( sodateya->work, index );
  SortSodateyaPokemon( sodateya->work );
}

/**
 * @brief ポケモンを図鑑に登録する
 */
static void RegisterPokemonInZukan( SODATEYA* sodateya, POKEMON_PARAM* poke )
{
  GAMESYS_WORK*   gameSystem = FIELDMAP_GetGameSysWork( sodateya->fieldmap );
  GAMEDATA*       gameData   = GAMESYSTEM_GetGameData( gameSystem );
  ZUKAN_SAVEDATA* zukanSave  = GAMEDATA_GetZukanSave( gameData );
  u32             monsno     = PP_Get( poke, ID_PARA_monsno, NULL );

  if( ZUKANSAVE_GetPokeGetFlag( zukanSave, monsno ) == FALSE ) {
    ZUKANSAVE_SetPokeGet( zukanSave, poke );
  }
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
  if( SODATEYA_WORK_IsValidEgg( sodateya->work ) != TRUE ) {
    OBATA_Printf( "SODATEYA_TakeBackEgg: タマゴがありません。\n" );
    return;
  }
  GAMEBEACON_Set_SodateyaEgg();
  ResetR03OldmanEventWork( sodateya->fieldmap );

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
  if( SODATEYA_WORK_IsValidEgg( sodateya->work ) != TRUE ) {
    OBATA_Printf( "SODATEYA_TakeBackEgg: タマゴがありません。\n" );
    return;
  }

  SODATEYA_WORK_ClrEgg( sodateya->work );
  ResetR03OldmanEventWork( sodateya->fieldmap );
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
  if( LayEggCheck(sodateya) == TRUE ) {
    SODATEYA_WORK_SetEgg( sodateya->work ); // タマゴが産まれたことを記憶
    SetR03OldmanEventWork( sodateya->fieldmap ); // じいさんイベントをセット
    return TRUE;
  }
  return FALSE;
}

/**
 * @brief 育て屋じいさんの呼び掛けイベントをセットする
 */
static void SetR03OldmanEventWork( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK* gameSystem = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*     gameData   = GAMESYSTEM_GetGameData( gameSystem );
  EVENTWORK*    eventWork  = GAMEDATA_GetEventWork( gameData );
  u16* work;

  work = EVENTWORK_GetEventWorkAdrs( eventWork, WK_OTHER_DISCOVER_EGG );
  *work = 1;
}

/**
 * @brief 育て屋じいさんの呼び掛けイベントをリセットする
 */
static void ResetR03OldmanEventWork( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK* gameSystem = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*     gameData   = GAMESYSTEM_GetGameData( gameSystem );
  EVENTWORK*    eventWork  = GAMEDATA_GetEventWork( gameData );
  u16* work;

  work = EVENTWORK_GetEventWorkAdrs( eventWork, WK_OTHER_DISCOVER_EGG );
  *work = 0;
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
    OBATA_Printf( "SODATEYA_GetPokeLv_Current: 指定インデックスには飼育ポケがいません。\n" );
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
    OBATA_Printf( "SODATEYA_GetLoveLevel: 預けポケモンが2体いません。\n" );
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
    OBATA_Printf( "SODATEYA_GetPokemonParam: 指定インデックスには飼育ポケがいません。\n" );
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

  // 技を更新 ( 成長過程で覚える技を習得 )
  POKE_PERSONAL_LoadWazaOboeTable( monsno, formno, waza_table );
  i = 0;
  while( !POKEPER_WAZAOBOE_IsEndCode( waza_table[i] ) )
  {
    int waza_lv = POKEPER_WAZAOBOE_GetLevel( waza_table[i] );
    int waza_id = POKEPER_WAZAOBOE_GetWazaID( waza_table[i] );
    if( after_lv < waza_lv ) { break; }
    if( before_lv < waza_lv ) { LearnNewWaza( poke, waza_id ); }
    i++;
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 技を覚えさせる
 *
 * @param poke
 * @param wazano
 */
//---------------------------------------------------------------------------------------- 
static void LearnNewWaza( POKEMON_PARAM* poke, u32 wazano )
{
  int pos;
  u32 param_id[ PTL_WAZA_MAX ] = 
  {
    ID_PARA_waza1,
    ID_PARA_waza2,
    ID_PARA_waza3,
    ID_PARA_waza4,
  };

  if( wazano == WAZANO_NULL ) { return; } // 技番号が不正
  if( CheckWazaHave(poke, wazano) ) { return; } // すでに覚えている

  // 空き要素に追加
  for( pos=0; pos < PTL_WAZA_MAX; pos++ )
  {
    u32 waza = PP_Get( poke, param_id[ pos ], NULL );

    if( waza == WAZANO_NULL ) {
      PP_SetWazaPos( poke, wazano, pos );
      return;
    }
  }

  // 空きがなければプッシュする
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
  u32 id_para[ PTL_WAZA_MAX ] = 
  { 
    ID_PARA_waza1, 
    ID_PARA_waza2, 
    ID_PARA_waza3, 
    ID_PARA_waza4 
  };

  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    if( PP_Get( poke, id_para[i], NULL ) == wazano ) {
      return TRUE;
    }
  }

  return FALSE;
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

    // タマゴグループの不正値を検出
    GF_ASSERT( egg_group1[i] != POKEPER_EGG_GROUP_NONE );
    GF_ASSERT( egg_group2[i] != POKEPER_EGG_GROUP_NONE );
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
  OBATA_Printf( "LoveCheck: 意図しない処理フロー発生!!\n" );
  return LOVE_LV_WORST;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定したポケモンの相性を求める (同じタマゴグループの場合)
 *
 * [ルール]
 *  同種 and 異ID then ◎
 *  同種 and 同ID then ○
 *  異種 and 異ID then ○
 *  異種 and 同ID then △
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
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( sodateya->fieldmap );
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
  MYSTATUS* mystatus = GAMEDATA_GetMyStatus( gdata ); 
  const POKEMON_PARAM* father = SODATEYA_WORK_GetPokemon( sodateya->work, 0 );
  const POKEMON_PARAM* mother = SODATEYA_WORK_GetPokemon( sodateya->work, 1 );

  // タマゴを作成
  POKEMON_EGG_Create( 
      sodateya->heapID, mystatus, POKE_MEMO_PERSON_SODATEYA, father, mother, egg );
}


//========================================================================================
// ■ポケモン個別処理
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief ぺラップの「おしゃべり」データを削除する
 *
 * @param sodateya
 */
//---------------------------------------------------------------------------------------- 
static void DeletePerapVoice( FIELDMAP_WORK* fieldmap, POKEPARTY* party )
{
  GAMESYS_WORK* gameSystem;
  GAMEDATA* gameData;
  PERAPVOICE* perapVoice;

  gameSystem = FIELDMAP_GetGameSysWork( fieldmap );
  gameData = GAMESYSTEM_GetGameData( gameSystem );
  perapVoice = GAMEDATA_GetPerapVoice( gameData );

  PERAPVOICE_CheckPerapInPokeParty( perapVoice, party );
} 
