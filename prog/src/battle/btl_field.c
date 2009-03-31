//=============================================================================================
/**
 * @file	btl_field.c
 * @brief	ポケモンWB バトルシステム	天候など「場」に対する効果に関する処理
 * @author	taya
 *
 * @date	2009.03.04	作成
 */
//=============================================================================================
#include <gflib.h>

#include "btl_common.h"
#include "btl_calc.h"

#include "btl_field.h"

//--------------------------------------------------------------
/**
 *	定数
 */
//--------------------------------------------------------------
enum {
	TURN_MAX = 16,
};
//--------------------------------------------------------------
/**
 *	グローバルワーク
 */
//--------------------------------------------------------------
static struct {

	BtlWeather		weather;
	u16						weatherTurn;

}Work;






void BTL_FIELD_Init( BtlWeather weather )
{
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
	BTL_Printf("天候フラットに\n");
}

//=============================================================================================
/**
 * 天候によってダメージが増加・減少するワザの増減率を返す
 *
 * @param   waza		ワザID
 *
 * @retval  fx32		増減率（パーセンテージ）
 */
//=============================================================================================
fx32 BTL_FIELD_GetWeatherDmgRatio( WazaID waza )
{
	switch( Work.weather ){
	case BTL_WEATHER_SHINE:
		{
			PokeType type = WAZADATA_GetType( waza );
			if( type == POKETYPE_FIRE ){
				return BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE;
			}
			if( type == POKETYPE_WATER ){
				return BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE;
			}
		}
		break;
	case BTL_WEATHER_RAIN:
		{
			PokeType type = WAZADATA_GetType( waza );
			if( type == POKETYPE_FIRE ){
				return BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE;
			}
			if( type == POKETYPE_WATER ){
				return BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE;
			}
		}
		break;
	default:
		break;
	}
	return BTL_CALC_DMG_WEATHER_RATIO_NONE;
}

//=============================================================================================
/**
 * ターンチェック
 *
 * @retval  BtlWeather		ターンチェックにより終わった天候
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
 * 場がとくていの状態にあるかどうか判定
 *
 * @param   state		
 *
 * @retval  BOOL		
 */
//=============================================================================================
BOOL BTL_FIELD_CheckState( BtlFieldState state )
{
	// @@@ とりあえず
	return FALSE;
}

