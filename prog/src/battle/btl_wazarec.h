//=============================================================================================
/**
 * @file  btl_wazarec.h
 * @brief 出たワザ記録機構
 * @author  taya
 *
 * @date  2009.11.21  作成
 */
//=============================================================================================
#pragma once

#include "waza_tool\wazadata.h"
#include "btl_common.h"

enum {
  BTL_WAZAREC_TURN_MAX = 20,
  BTL_WAZAREC_RECORD_MAX = BTL_WAZAREC_TURN_MAX*BTL_FRONT_POKE_MAX,
};

typedef struct {

    u32   ptr;

    struct {
      u32     turn;
      WazaID  wazaID;
      u8      pokeID;
      u8      fEffective;
    }record[ BTL_WAZAREC_RECORD_MAX ];

}BTL_WAZAREC;


extern void BTL_WAZAREC_Init( BTL_WAZAREC* rec );
extern void BTL_WAZAREC_Add( BTL_WAZAREC* rec, WazaID waza, u32 turn, u8 pokeID );
extern void BTL_WAZAREC_SetEffectiveLast( BTL_WAZAREC* rec );
extern void BTL_WAZAREC_EndTurn( BTL_WAZAREC* rec );

extern BOOL BTL_WAZAREC_IsUsedWaza( const BTL_WAZAREC* rec, WazaID waza, u32 turn );
extern u32 BTL_WAZAREC_GetUsedWazaCount( const BTL_WAZAREC* rec, WazaID waza, u32 turn );



