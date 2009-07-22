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

#include "btl_pokeparam.h"


extern void BTL_SICK_AddProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, WazaSick sick );

extern void BTL_SICK_TurnCheckCallback( BTL_POKEPARAM* bpp, WazaSick sick, BOOL fCure, void* work );
extern int BTL_SICK_GetDefaultSickStrID( WazaSick sickID, BPP_SICK_CONT cont );

extern void BTL_SICK_CheckNotEffectByType( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender );
extern void BTL_SICK_CheckDamageAffinity(  BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender );

