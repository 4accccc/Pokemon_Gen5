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
#include "btl_field.h"


//--------------------------------------------------------------
/**
 *	定数
 */
//--------------------------------------------------------------
enum {
	TURN_PERMANENT = -1,
	TURN_MAX = 16,
};
//--------------------------------------------------------------
/**
 *	グローバルワーク
 */
//--------------------------------------------------------------
static struct {

	BtlWeather		weather;
	s16						weatherTurn;

}Work;



void BTL_FIELD_Init( BtlWeather weather )
{
	Work.weather = weather;
	Work.weatherTurn = TURN_PERMANENT;
}

BtlWeather BTL_FIELD_GetWeather( void )
{
	return Work.weather;
}

//=============================================================================================
/**
 * ターンチェック
 *
 * @retval  BOOL		ターンチェックにより終わった効果がある場合TRUE
 */
//=============================================================================================
BOOL BTL_FIELD_TurnCheck( void )
{
	if( Work.weather != BTL_WEATHER_NONE )
	{
		if( Work.weatherTurn != TURN_PERMANENT )
		{
			Work.weatherTurn--;
			if( Work.weatherTurn == 0 )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

