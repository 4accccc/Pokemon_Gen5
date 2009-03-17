//=============================================================================================
/**
 * @file	hand_tokusei_common.h
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]共有
 * @author	taya
 *
 * @date	2008.11.11	作成
 */
//=============================================================================================
#ifndef __HAND_TOKUSEI_COMMON_H__
#define __HAND_TOKUSEI_COMMON_H__

#include "../../btl_common.h"
#include "../../btl_event_factor.h"


//--------------------------------------------------------------
/**
 *	とくせいイベントハンドラ登録関数の型定義
 */
//--------------------------------------------------------------
typedef BTL_EVENT_FACTOR* (*pTokuseiEventAddFunc)( u16 pri, u8 pokeID );


//--------------------------------------------------------------
/**
 *	とくせいイベントハンドラ登録関数群
 */
//--------------------------------------------------------------
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_ClearBody( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seisinryoku( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukutsuno( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_AtuiSibou( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_KairikiBasami( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikaramoti( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Iromegane( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kasoku( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Mouka( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Gekiryu( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinryoku( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_MusinoSirase( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Konjou( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_SkillLink( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surudoime( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tanjun( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_HardRock( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaUroko( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tousousin( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_ReafGuard( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amefurasi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hideri( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunaokosi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukifurasi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_AirLock( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Technician( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Donkan( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_UruoiBody( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_PoisonHeal( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_IcoBody( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_AmeukeZara( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Rinpun( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tekiouryoku( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunPower( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Suisui( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dappi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tidoriasi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hayaasi( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Harikiri( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_Filter( u16 pri, u8 pokeID );
extern BTL_EVENT_FACTOR*  HAND_TOK_ADD_TennoMegumi( u16 pri, u8 pokeID );


#endif
