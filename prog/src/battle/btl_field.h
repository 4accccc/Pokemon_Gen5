//=============================================================================================
/**
 * @file	btl_field.h
 * @brief	ポケモンWB バトルシステム	天候など「場」に対する効果に関する処理
 * @author	taya
 *
 * @date	2009.03.04	作成
 */
//=============================================================================================
#ifndef __BTL_FIELD_H__
#define __BTL_FIELD_H__

#include "battle\battle.h"

/**
 *	バトル独自のフィールド状態（重複発動あり）
 */
typedef enum {
	BTL_FLDSTATE_NONE = 0,

	BTL_FLDSTATE_TRICKROOM,		///< トリックルーム
	BTL_FLDSTATE_GRAVITY,			///< じゅうりょく

	BTL_FLDSTATE_MAX,

}BtlFieldState;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/

extern void BTL_FIELD_Init( BtlWeather weather );
extern BtlWeather BTL_FIELD_GetWeather( void );
extern void BTL_FIELD_SetWeather( BtlWeather weather, u16 turn );
extern void BTL_FIELD_ClearWeather( void );

extern BOOL BTL_FIELD_CheckState( BtlFieldState state );

//=============================================================================================
/**
 * 天候によってダメージが増加・減少するワザの増減率を返す
 *
 * @param   waza		ワザID
 *
 * @retval  fx32		増減率（パーセンテージ）
 */
//=============================================================================================
extern fx32 BTL_FIELD_GetWeatherDmgRatio( WazaID waza );


extern BtlWeather BTL_FIELD_TurnCheckWeather( void );

#endif
