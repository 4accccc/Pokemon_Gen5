//============================================================================================
/**
 * @file  status_rcv.c
 * @brief アイテム使用時のポケモン回復処理
 * @author  Hiroyuki Nakamura
 * @date  05.12.01
 *        09.08.06 GSより移植
 */
//============================================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/status_rcv.h"

//============================================================================================
//  定数定義
//============================================================================================

//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//============================================================================================
//============================================================================================


//--------------------------------------------------------------------------------------------
/**
 * PP回復チェック
 *
 * @param pp    回復するポケモンのデータ
 * @param pos   回復位置（技位置など）
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 PP_RcvCheck( POKEMON_PARAM * pp, u32 pos )
{
  u16 waza;
  u8  npp;
  u8  ppc;

  waza = (u16)PP_Get( pp, ID_PARA_waza1+pos, NULL );
  if( waza == 0 ){
    return FALSE;
  }
  npp = (u8)PP_Get( pp, ID_PARA_pp1+pos, NULL );
  ppc = (u8)PP_Get( pp, ID_PARA_pp_count1+pos, NULL );

  if( npp < WAZADATA_GetMaxPP( waza, ppc ) ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * PP回復
 *
 * @param pp    回復するポケモンのデータ
 * @param pos   回復位置（技位置など）
 * @param rcv   回復値
 *
 * @retval  "TRUE = 使用可能"
 * @retval  "FALSE = 使用不可"
 */
//--------------------------------------------------------------------------------------------
u8 PP_Recover( POKEMON_PARAM * pp, u32 pos, u32 rcv )
{
  u16 waza;
  u8  npp;
  u8  mpp;

  waza = (u16)PP_Get( pp, ID_PARA_waza1+pos, NULL );
  if( waza == 0 ){
    return FALSE;
  }
  npp  = (u8)PP_Get( pp, ID_PARA_pp1+pos, NULL );
  mpp  = (u8)WAZADATA_GetMaxPP( waza, PP_Get(pp,ID_PARA_pp_count1+pos,NULL) );

  if( npp < mpp ){
    if( rcv == PP_RCV_ALL ){
      npp = mpp;
    }else{
      npp += rcv;
      if( npp > mpp ){
        npp = mpp;
      }
    }
    PP_Put( pp, ID_PARA_pp1+pos, npp );
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**ポケモン回復処理(単体・全体
 */
//--------------------------------------------------------------------------------------------
void STATUS_RCV_PokeParam_RecoverAll(POKEMON_PARAM * pp)
{
  int j;
  u32 buf;
  //HP全回復
  buf = PP_Get( pp, ID_PARA_hpmax, NULL );
  PP_Put(pp, ID_PARA_hp, buf);
  //状態異常回復
  buf = 0;
  PP_Put(pp, ID_PARA_condition, buf);
  //PP全回復
  for (j = 0; j < 4; j++) {
    if (PP_RcvCheck(pp,j) == TRUE) {
      PP_Recover(pp, j, PP_RCV_ALL);
    }
  }
}

void STATUS_RCV_PokeParty_RecoverAll(POKEPARTY * party)
{
  int i,total;
  POKEMON_PARAM * pp;

  total = PokeParty_GetPokeCount(party);
  for (i = 0; i < total; i++) {
    pp = PokeParty_GetMemberPointer(party, i);
    if (PP_Get(pp, ID_PARA_poke_exist, NULL) == FALSE) {
      continue;
    }
    STATUS_RCV_PokeParam_RecoverAll( pp );
  }
}




