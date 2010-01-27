//=============================================================================================
/**
 * @file  btl_sick.h
 * @brief ポケモンWB バトルシステム 状態異常処理関連
 * @author  taya
 *
 * @date  2009.07.09  作成
 */
//=============================================================================================
#pragma once

#include "btl_server_flow.h"
#include "btl_pokeparam.h"


extern void BTL_SICK_TurnCheckCallback( BTL_POKEPARAM* bpp, WazaSick sick, BPP_SICK_CONT oldCont, BOOL fCure, void* work );
extern BOOL BTL_SICK_MakeSickDamageMsg( BTL_HANDEX_STR_PARAMS* strParam, const BTL_POKEPARAM* bpp, WazaSick sickID );

extern void BTL_SICK_MakeDefaultMsg( WazaSick sickID, BPP_SICK_CONT cont, const BTL_POKEPARAM* bpp, BTL_HANDEX_STR_PARAMS* str );
BOOL BTL_SICK_MakeDefaultCureMsg( WazaSick sickID, BPP_SICK_CONT cont, const BTL_POKEPARAM* bpp,
  u16 itemID, BTL_HANDEX_STR_PARAMS* str );

extern void BTL_SICKEVENT_CheckNotEffectByType( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender );
extern void BTL_SICKEVENT_CheckDamageAffinity(  BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender );
extern void BTL_SICKEVENT_CheckFlying(  BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender );
extern void BTL_SICKEVENT_CheckPushOutFail( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp );
extern void BTL_SICKEVENT_CheckHitRatio( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );

extern BOOL BTL_SICK_CheckBatonTouch( WazaSick sick );

