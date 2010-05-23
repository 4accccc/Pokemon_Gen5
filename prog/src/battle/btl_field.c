//=============================================================================================
/**
 * @file  btl_field.c
 * @brief ポケモンWB バトルシステム 天候など「場」に対する効果に関する処理
 * @author  taya
 *
 * @date  2009.03.04  作成
 */
//=============================================================================================
#include <gflib.h>

#include "btl_common.h"
#include "btl_calc.h"
#include "btl_event_factor.h"

#include "handler\hand_field.h"
#include "btl_field.h"

//--------------------------------------------------------------
/**
 *  定数
 */
//--------------------------------------------------------------
enum {
  TURN_MAX = 16,
  DEPEND_POKE_NUM_MAX = BTL_POS_MAX,
};
//--------------------------------------------------------------
/**
 *  グローバルワーク
 */
//--------------------------------------------------------------
static struct {

  /*
   * VRAM_H に置くのでu32型に揃える必要あり
   */
  u32       weather;
  u32       weatherTurn;

  BTL_EVENT_FACTOR*  factor[ BTL_FLDEFF_MAX ];
  BPP_SICK_CONT      cont[ BTL_FLDEFF_MAX ];
  u32                turnCount[ BTL_FLDEFF_MAX ];
  u32                dependPokeID[ BTL_FLDEFF_MAX ][ DEPEND_POKE_NUM_MAX ];
  u32                dependPokeCount[ BTL_FLDEFF_MAX ];

}Work;



static void clearFactorWork( BtlFieldEffect effect )
{
  u32 i;

  Work.factor[ effect ] = NULL;
  Work.cont[ effect ] = BPP_SICKCONT_MakeNull();
  Work.turnCount[ effect ] = 0;
  Work.dependPokeCount[ effect ] = 0;

  for(i=0; i<DEPEND_POKE_NUM_MAX; ++i)
  {
    Work.dependPokeID[effect][i] = BTL_POKEID_NULL;
  }
}


void BTL_FIELD_Init( BtlWeather weather )
{
  u32 i;

  for(i=0; i<BTL_FLDEFF_MAX; ++i){
    clearFactorWork( i );
  }

  Work.weather = weather;
  Work.weatherTurn = BTL_WEATHER_TURN_PERMANENT;
}

BtlWeather BTL_FIELD_GetWeather( void )
{
  return Work.weather;
}
u32 BTL_FIELD_GetWeatherTurn( void )
{
  if( Work.weather != BTL_WEATHER_NONE ){
    return Work.weatherTurn;
  }
  return 0;
}

//=============================================================================================
/**
 *
 *
 * @param   weather
 * @param   turn
 *
 */
//=============================================================================================
void BTL_FIELD_SetWeather( BtlWeather weather, u16 turn )
{
  Work.weather = weather;
  Work.weatherTurn = turn;
}

void BTL_FIELD_ClearWeather( void )
{
  Work.weather = BTL_WEATHER_NONE;
  Work.weatherTurn = 0;
}

//=============================================================================================
/**
 * ターンチェック
 *
 * @retval  BtlWeather    ターンチェックにより終わった天候
 */
//=============================================================================================
BtlWeather BTL_FIELD_TurnCheckWeather( void )
{
  if( Work.weather != BTL_WEATHER_NONE )
  {
    if( Work.weatherTurn != BTL_WEATHER_TURN_PERMANENT )
    {
      Work.weatherTurn--;
      if( Work.weatherTurn == 0 )
      {
        BtlWeather endWeather = Work.weather;
        Work.weather = BTL_WEATHER_NONE;
        return endWeather;
      }
    }
  }
  return BTL_WEATHER_NONE;
}


//=============================================================================================
/**
 * フィールドエフェクト追加
 *
 * @param   state
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_FIELD_AddEffect( BtlFieldEffect effect, BPP_SICK_CONT cont )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  if( Work.factor[effect] == NULL )
  {
    Work.factor[ effect ] = BTL_HANDLER_FLD_Add( effect, 0 );
    if( Work.factor[ effect ] )
    {
      u32 i;
      Work.cont[ effect ] = cont;
      Work.turnCount[ effect ] = 0;
      Work.dependPokeCount[ effect ] = 0;
      for(i=0; i<DEPEND_POKE_NUM_MAX; ++i){
        Work.dependPokeID[ effect ][i] = BTL_POKEID_NULL;
      }

      {
        u8 dependPokeID = BPP_SICKCONT_GetPokeID( cont );
        if( dependPokeID != BTL_POKEID_NULL )
        {
          BTL_FIELD_AddDependPoke( effect, dependPokeID );
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * フィールドエフェクト除去
 *
 * @param   effect
 */
//=============================================================================================
BOOL BTL_FIELD_RemoveEffect( BtlFieldEffect effect )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  if( Work.factor[effect] != NULL )
  {
    BTL_HANDLER_FLD_Remove( Work.factor[effect] );
    clearFactorWork( effect );
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * 依存対象ポケモンを追加
 *
 * @param   BtlFieldEffect
 * @param   pokeID
 *
 * @retval  BOOL  新規追加できたらTRUE / 既に登録されてる場合＆登録失敗したらFALSE
 */
//=============================================================================================
BOOL BTL_FIELD_AddDependPoke( BtlFieldEffect effect, u8 pokeID )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  if( Work.factor[effect] != NULL )
  {
    u32 count = Work.dependPokeCount[ effect ];

    if( count < DEPEND_POKE_NUM_MAX )
    {
      u32 i;
      for(i=0; i<count; ++i)
      {
        if( Work.dependPokeID[ effect ][i] == pokeID ){
          return FALSE;
        }
      }

      Work.dependPokeID[ effect ][ count++ ] = pokeID;
      Work.dependPokeCount[ effect ] = count;
      return TRUE;
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * 指定エフェクトの依存対象ポケモンかどうか判定
 *
 * @param   effect
 * @param   pokeID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_FIELD_IsDependPoke( BtlFieldEffect effect, u8 pokeID )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  if( Work.factor[effect] != NULL )
  {
    u32 count = Work.dependPokeCount[ effect ];
    u32 i;
    for(i=0; i<count; ++i)
    {
      if( Work.dependPokeID[ effect ][i] == pokeID ){
        return TRUE;
      }
    }
  }
  return FALSE;
}

//=============================================================================================
/**
 * 特定ポケモン依存のエフェクトを全て除去
 *
 * @param   pokeID
 */
//=============================================================================================
void BTL_FIELD_RemoveDependPokeEffect( u8 pokeID )
{
  u32 i;
  for(i=0; i<BTL_FLDEFF_MAX; ++i)
  {
    if( Work.factor[i] )
    {
      if( Work.dependPokeCount[i] )
      {
        u32 p, fMatch = FALSE;
        for(p=0; p<Work.dependPokeCount[i]; ++p)
        {
          if( Work.dependPokeID[i][p] == pokeID ){
            fMatch = TRUE;
            break;
          }
        }
        if( fMatch )
        {
          for( ; p<(DEPEND_POKE_NUM_MAX-1); ++p){
            Work.dependPokeID[i][p] = Work.dependPokeID[i][p+1];
          }
          Work.dependPokeID[i][p] = BTL_POKEID_NULL;
          Work.dependPokeCount[i]--;

          if( Work.dependPokeCount[i] == 0 )
          {
            BTL_HANDLER_FLD_Remove( Work.factor[i] );
            clearFactorWork( i );
          }
          else if( BPP_SICKCONT_GetPokeID(Work.cont[i]) == pokeID )
          {
            u8 nextPokeID = Work.dependPokeID[i][0];
            BPP_SICKCONT_SetPokeID( &(Work.cont[i]), nextPokeID );
          }
        }
      }
    }
  }
}

//=============================================================================================
/**
 * ふういん禁止対象のワザかどうか判定
 *
 * @param   pokeCon
 * @param   attacker
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_FIELD_CheckFuin( const BTL_POKE_CONTAINER* pokeCon, const BTL_POKEPARAM* attacker, WazaID waza )
{
  u8  atkPokeID = BPP_GetID( attacker );
  u8  fuinPokeID;
  u32 i, fuinPokeCnt = Work.dependPokeCount[ BTL_FLDEFF_FUIN ];


  for(i=0; i<fuinPokeCnt; ++i)
  {
    fuinPokeID = Work.dependPokeID[ BTL_FLDEFF_FUIN ][ i ];

    BTL_N_Printf( DBGSTR_FIDLD_FuinCheck, i, fuinPokeID, waza );
    if( !BTL_MAINUTIL_IsFriendPokeID(atkPokeID, fuinPokeID) )
    {
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParamConst( pokeCon, fuinPokeID );
      if( BPP_WAZA_IsUsable(bpp, waza) )
      {
        BTL_N_Printf( DBGSTR_FIELD_FuinHit, fuinPokeID, waza);
        return TRUE;
      }
    }
  }
  return FALSE;
}

//=============================================================================================
/**
 * ターンチェック
 */
//=============================================================================================
void BTL_FIELD_TurnCheck( pFieldTurnCheckCallback callbackFunc, void* callbackArg )
{
  u32 i;
  for(i=1; i<BTL_FLDEFF_MAX; ++i)
  {
    if( Work.factor[i] )
    {
      u8 turnMax = BPP_SICCONT_GetTurnMax( Work.cont[i] );
      if( turnMax )
      {
        if( ++(Work.turnCount[i]) >= turnMax )
        {
          BTL_HANDLER_FLD_Remove( Work.factor[i] );
          clearFactorWork( i );
          callbackFunc( i, callbackArg );
        }
      }
    }
  }
}
//=============================================================================================
/**
 * 特定のフィールドエフェクトが働いているかチェック
 *
 * @param   effect
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_FIELD_CheckEffect( BtlFieldEffect effect )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  return Work.factor[ effect ] != NULL;
}

//=============================================================================================
/**
 * 継続依存ポケモンIDを返す
 *
 * @param   effect
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_FIELD_GetDependPokeID( BtlFieldEffect effect )
{
  GF_ASSERT(effect < BTL_FLDEFF_MAX);

  if( Work.factor[ effect ] )
  {
    return BPP_SICKCONT_GetPokeID( Work.cont[effect] );
  }

  return BTL_POKEID_NULL;
}

