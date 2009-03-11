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



#endif
