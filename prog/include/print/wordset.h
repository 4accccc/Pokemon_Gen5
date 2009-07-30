//============================================================================================
/**
 * @file	wordset.h
 * @brief	単語モジュール処理
 * @author	taya
 * @date	2005.09.28
 */
//============================================================================================
#ifndef __WORDSET_H__
#define __WORDSET_H__


#include "poke_tool/poke_tool.h"
#include "print/str_tool.h"
#include "system/pms_word.h"
#include "net_app/wifi_lobby/wflby_def.h"  //WFLBY_GAMETYPEetc
#include "savedata/mystatus.h"
#include "net_app/wifi_country.h"
#include "tr_tool/tr_tool.h"
#include "tr_tool/trno_def.h"

typedef struct _WORDSET  WORDSET;


enum {
	WORDSET_DEFAULT_SETNUM = 8,		// デフォルトバッファ数
	WORDSET_DEFAULT_BUFLEN = 32,	// デフォルトバッファ長（文字数）

	WORDSET_COUNTRY_BUFLEN = 64,	// 国地域名用バッファ長（文字数）(WORDSET_CreateExで指定)
};


extern WORDSET*  WORDSET_Create( HEAPID heapID );
extern WORDSET* WORDSET_CreateEx( u32 word_max, u32 buflen, HEAPID heapID );
extern void WORDSET_Delete( WORDSET* wordset );
extern void WORDSET_RegisterWord( WORDSET* wordset, u32 bufID, const STRBUF* word, u32 sex, BOOL singular_flag, u32 lang );
extern void WORDSET_RegisterPokeMonsName( WORDSET* wordset, u32 bufID, const POKEMON_PARAM* pp );
extern void WORDSET_RegisterPokeNickName( WORDSET* wordset, u32 bufID, const POKEMON_PARAM* pp );
extern void WORDSET_RegisterPokeOyaName( WORDSET* wordset,  u32 bufID, const POKEMON_PARAM* pp );
extern void WORDSET_RegisterWazaName( WORDSET* wordset, u32 bufID, u32 wazaID );
extern void WORDSET_RegisterItemName( WORDSET* wordset, u32 bufID, u32 itemID );
extern void WORDSET_RegisterTokuseiName( WORDSET* wordset, u32 bufID, u32 tokuseiID );
extern void WORDSET_RegisterNumber( WORDSET* wordset, u32 bufID, s32 number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType );
extern void WORDSET_RegisterPMSWord( WORDSET* wordset, u32 bufID, PMS_WORD word );
extern void WORDSET_RegisterWiFiLobbyGameName( WORDSET* wordset, u32 bufID, WFLBY_GAMETYPE type );
extern void WORDSET_RegisterWiFiLobbyEventName( WORDSET* wordset, u32 bufID, WFLBY_EVENTGMM_TYPE type );
extern void WORDSET_RegisterWiFiLobbyItemName( WORDSET* wordset, u32 bufID, WFLBY_ITEMTYPE type );
extern void WORDSET_RegisterWiFiLobbyAisatsuJapan( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterWiFiLobbyAisatsuEnglish( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterWiFiLobbyAisatsuFrance( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterWiFiLobbyAisatsuItaly( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterWiFiLobbyAisatsuGerMany( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterWiFiLobbyAisatsuSpain( WORDSET* wordset, u32 bufID, u32 time_zone );
extern void WORDSET_RegisterPokeTypeName( WORDSET* wordset, u32 bufID, u32 typeID );
extern void WORDSET_RegisterPlayerName( WORDSET* wordset, u32 bufID, const MYSTATUS* my );
extern void WORDSET_RegisterCountryName( WORDSET* wordset, u32 bufID, u32 countryID );
extern void WORDSET_RegisterLocalPlaceName(WORDSET* wordset,u32 bufID,u32 countryID,u32 placeID);
extern void WORDSET_RegisterTrTypeName( WORDSET* wordset, u32 bufID, TrainerID trID );
extern void WORDSET_RegisterTrainerName( WORDSET* wordset, u32 bufID, TrainerID trID );
extern void WORDSET_ExpandStr( const WORDSET* wordset, STRBUF* dstbuf, const STRBUF* srcbuf );
extern void WORDSET_ClearAllBuffer( WORDSET* wordset );



#endif
