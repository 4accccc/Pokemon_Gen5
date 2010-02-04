//=============================================================================================
/**
 * @file  btl_field.h
 * @brief ポケモンWB バトルシステム 天候など「場」に対する効果に関する処理
 * @author  taya
 *
 * @date  2009.03.04  作成
 */
//=============================================================================================
#ifndef __BTL_FIELD_H__
#define __BTL_FIELD_H__

#include "battle\battle.h"
#include "btl_util.h"


/**
 *  バトル独自のフィールド状態（重複発動あり）
 */
typedef enum {

  BTL_FLDEFF_WEATHER,     ///< 各種天候
  BTL_FLDEFF_TRICKROOM,   ///< トリックルーム
  BTL_FLDEFF_JURYOKU,     ///< じゅうりょく
  BTL_FLDEFF_FUIN,        ///< ふういん
  BTL_FLDEFF_MIZUASOBI,   ///< みずあそび
  BTL_FLDEFF_DOROASOBI,   ///< どろあそび
  BTL_FLDEFF_WONDERROOM,  ///< ワンダールーム
  BTL_FLDEFF_MAGICROOM,   ///< マジックルーム

  BTL_FLDEFF_MAX,
  BTL_FLDEFF_NULL = BTL_FLDEFF_MAX,

}BtlFieldEffect;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/

extern void BTL_FIELD_Init( BtlWeather weather );
extern BtlWeather BTL_FIELD_GetWeather( void );
extern void BTL_FIELD_SetWeather( BtlWeather weather, u16 turn );
extern void BTL_FIELD_ClearWeather( void );

extern BOOL BTL_FIELD_AddEffect( BtlFieldEffect effect, BPP_SICK_CONT cont );
extern BOOL BTL_FIELD_RemoveEffect( BtlFieldEffect state );
extern void BTL_FIELD_RemoveDependPokeEffect( u8 pokeID );
extern BOOL BTL_FIELD_CheckEffect( BtlFieldEffect state );
extern u8 BTL_FIELD_GetDependPokeID( BtlFieldEffect effect );



extern BtlWeather BTL_FIELD_TurnCheckWeather( void );


typedef void (*pFieldTurnCheckCallback)( BtlFieldEffect, void* );
extern void BTL_FIELD_TurnCheck( pFieldTurnCheckCallback callbackFunc, void* callbackArg);


#endif
