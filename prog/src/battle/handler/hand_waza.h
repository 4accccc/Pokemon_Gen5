//=============================================================================================
/**
 * @file  hand_waza.h
 * @brief ポケモンWB バトルシステム イベントファクター（ワザ）追加処理
 * @author  taya
 *
 * @date  2009.05.15  作成
 */
//=============================================================================================
#pragma once


#include "..\btl_pokeparam.h"
#include "..\btl_event_factor.h"
#include "..\btl_server_local.h"


extern BOOL BTL_HANDLER_Waza_Add( const BTL_POKEPARAM* pp, WazaID waza );
extern void BTL_HANDLER_Waza_Remove( const BTL_POKEPARAM* pp, WazaID waza );
extern void BTL_HANDLER_Waza_RemoveForce( const BTL_POKEPARAM* pp, WazaID waza );
extern void BTL_HANDLER_Waza_RemoveForceAll( const BTL_POKEPARAM* pp );


