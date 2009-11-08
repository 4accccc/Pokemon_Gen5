//=============================================================================
/**
 * @file	  pokemontrade.h
 * @bfief	  ポケモン交換
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/07/09
 */
//=============================================================================


#pragma once

/// ポケモン交換のタイプ
typedef enum{
  POKEMONTRADE_IRC,   ///< 赤外線
  POKEMONTRADE_UNION, ///< ユニオン
  POKEMONTRADE_WIFICLUB, ///< WIFIクラブ
  POKEMONTRADE_GTSNEGO, ///< GTSネゴシエーション
  POKEMONTRADE_GTS, ///< GTS
  POKEMONTRADE_TYPE_MAX,
} POKEMONTRADE_TYPE;



typedef struct _POKEMON_TRADE_WORK POKEMON_TRADE_WORK;
typedef struct _POKEMONTRADE_DEMO_WORK POKEMONTRADE_DEMO_WORK;

extern const GFL_PROC_DATA PokemonTradeProcData;
extern const GFL_PROC_DATA PokemonTradeIrcProcData;


