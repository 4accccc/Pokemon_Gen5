//=============================================================================================
/**
 * @file  hand_tokusei.h
 * @brief ポケモンWB バトルシステム イベントファクター[とくせい]追加処理
 * @author  taya
 *
 * @date  2008.11.11  作成
 */
//=============================================================================================
#ifndef __HAND_TOKUSEI_H__
#define __HAND_TOKUSEI_H__

#include "..\btl_pokeparam.h"
#include "..\btl_event_factor.h"
#include "..\btl_server_local.h"

extern BTL_EVENT_FACTOR*  BTL_HANDLER_TOKUSEI_Add( const BTL_POKEPARAM* pp );
extern void BTL_HANDLER_TOKUSEI_Remove( const BTL_POKEPARAM* pp );
extern void BTL_HANDLER_TOKUSEI_Swap( const BTL_POKEPARAM* pp1, const BTL_POKEPARAM* pp2 );

extern void BTL_HANDLER_TOKUSEI_RotationSleep( const BTL_POKEPARAM* bpp );
extern void BTL_HANDLER_TOKUSEI_RotationWake( const BTL_POKEPARAM* bpp );

#endif
