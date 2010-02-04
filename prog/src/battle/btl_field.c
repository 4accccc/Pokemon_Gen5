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
};
//--------------------------------------------------------------
/**
 *  グローバルワーク
 */
//--------------------------------------------------------------
static struct {

  BtlWeather    weather;
  u16           weatherTurn;

  BTL_EVENT_FACTOR*  factor[ BTL_FLDEFF_MAX ];
  BPP_SICK_CONT      cont[ BTL_FLDEFF_MAX ];
  u8                 turnCount[ BTL_FLDEFF_MAX ];

}Work;



static void clearFactorWork( BtlFieldEffect effect )
{
  Work.factor[ effect ] = NULL;
  Work.cont[ effect ] = BPP_SICKCONT_MakeNull();
  Work.turnCount[ effect ] = 0;
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
  BTL_Printf("天候変化=%d\n", weather);
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
    BTL_Printf("FldEff %d 追加\n");
    Work.factor[ effect ] = BTL_HANDLER_FLD_Add( effect, 0 );
    if( Work.factor[ effect ] ){
      Work.cont[ effect ] = cont;
      Work.turnCount[ effect ] = 0;
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
    BTL_Printf("FldEff %d 除去\n");

    BTL_HANDLER_FLD_Remove( Work.factor[effect] );
    clearFactorWork( effect );
    return TRUE;
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
      if( BPP_SICKCONT_GetPokeID(Work.cont[i]) == pokeID )
      {
        BTL_HANDLER_FLD_Remove( Work.factor[i] );
        clearFactorWork( i );
      }
    }
  }
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

