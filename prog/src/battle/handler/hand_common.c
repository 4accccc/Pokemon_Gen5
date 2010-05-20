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


