//=============================================================================================
/**
 * @file  hand_common.c
 * @brief ポケモンWB バトルシステム イベントハンドラ共有関数群
 * @author  taya
 *
 * @date  2010.05.19  作成
 */
//=============================================================================================


#include "..\btl_common.h"
#include "..\btl_event_factor.h"


#include "hand_common.h"


/**
 *  複数体ターゲットポケモンIDが設定されているハンドラで、指定IDのポケモンがターゲットに含まれるか判定
 */
BOOL HandCommon_CheckTargetPokeID( u8 pokeID )
{
  u32 i, target_cnt = BTL_EVENTVAR_GetValue( BTL_EVAR_TARGET_POKECNT );
  for(i=0; i<target_cnt; ++i)
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_TARGET1+i) == pokeID ){
      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  指定ポケモンの行動順が最後かどうか判定
 */
BOOL HandCommon_IsPokeOrderLast( BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  u8 myOrder, maxOrder;
  if( BTL_SVFTOOL_GetMyActionOrder(flowWk, pokeID, &myOrder, &maxOrder) )
  {
    if( (myOrder+1) == maxOrder ){
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  ルール上、相手のどうぐを失わせる行為が禁止されているポケのチェック
 * （野生戦で相手の場合をチェック）
 */
BOOL HandCommon_CheckCantStealPoke( BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  if( BTL_SVFTOOL_GetCompetitor(flowWk) == BTL_COMPETITOR_WILD )
  {
    u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
    if( (clientID == BTL_CLIENT_ENEMY1) || (clientID == BTL_CLIENT_ENEMY1) ){
      return TRUE;
    }
  }
  return FALSE;
}

