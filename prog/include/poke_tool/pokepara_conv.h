//============================================================================================
/**
 * @file  pokepara_conv.h
 * @bfief ポケモンパラメータコンバータツール群
 * @author  HisashiSogabe
 * @date  10.02.18
 */
//============================================================================================
#pragma once

extern  void  PPCONV_ConvertPPPFromPokeShifter( POKEMON_PASO_PARAM* ppp );
extern  void  PPCONV_ConvertStr( const u16 *src , STRCODE *dst , const u8 dstLen );
