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
 *
 */
BOOL HandCommon_CheckForbitItemPokeCombination( u16 monsno, u16 itemID )
{
  switch( monsno ){
  case MONSNO_GIRATHINA:
    if( itemID == ITEM_HAKKINDAMA ){  // ギラティナのはっきんだまはNG
      return TRUE;
    }
    break;
  case MONSNO_ARUSEUSU:
    if( BTL_TABLES_IsMatchAruseusPlate(itemID) ){  // アルセウスのプレートはNG
      return TRUE;
    }
    break;
  case MONSNO_INSEKUTA:
    if( BTL_TABLES_IsMatchInsectaCasette(itemID) ){  // インセクタのカセットはNG
      return TRUE;
    }
    break;
  }
  return FALSE;
}


/**
 *  絶対にアイテムを書き換えてはいけないポケモン判定
 */
BOOL HandCommon_CheckCantChangeItemPoke( BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  u16 monsno = BPP_GetMonsNo( bpp );
  u16 itemID = BPP_GetItem( bpp );

  if( HandCommon_CheckForbitItemPokeCombination(monsno, itemID) ){
    return TRUE;
  }

  return FALSE;
}

/**
 *  ルール上、相手のどうぐを失わせる行為が禁止されているポケのチェック
 */
BOOL HandCommon_CheckCantStealPoke( BTL_SVFLOW_WORK* flowWk, u8 attackPokeID, u8 targetPokeID )
{
  // 野生戦で相手の場合をチェック
  if( BTL_SVFTOOL_GetCompetitor(flowWk) == BTL_COMPETITOR_WILD )
  {
    u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( attackPokeID );
    if( (clientID == BTL_CLIENT_ENEMY1) || (clientID == BTL_CLIENT_ENEMY1) ){
      return TRUE;
    }
  }
  // ギラティナ・アルセウス・インセクタのチェック
  if( HandCommon_CheckCantChangeItemPoke(flowWk, targetPokeID) )
  {
    return TRUE;
  }

  {
    const BTL_POKEPARAM* attacker = BTL_SVFTOOL_GetPokeParam( flowWk, attackPokeID );
    const BTL_POKEPARAM* target = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );

    u16 atkMonsNo = BPP_GetMonsNo( attacker );
    u16 tgtItemID = BPP_GetItem( target );
    if( HandCommon_CheckForbitItemPokeCombination(atkMonsNo, tgtItemID) ){
      return TRUE;
    }
  }

  return FALSE;
}


/**
 *  マジックコート（相手が使うサイドエフェクトでマジックコート対象ワザは失敗
 */
void HandCommon_MagicCoat_CheckSideEffWaza( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 wazaUserPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
  if( !BTL_MAINUTIL_IsFriendClientID(wazaUserPokeID, pokeID) )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( (WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_SIDE_EFFECT)
    &&  (WAZADATA_GetFlag(waza, WAZAFLAG_MagicCoat))
    ){
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_NO_REACTION) ){
        BTL_SVFRET_AddMagicCoatAction( flowWk, pokeID, wazaUserPokeID );
      }
    }
  }
}
/**
 *  マジックコート（自分がワザ対象なら無効化）
 */
void HandCommon_MagicCoat_Wait( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MAGICCOAT_FLAG) == FALSE)
  ){
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );

    if( WAZADATA_GetFlag(waza, WAZAFLAG_MagicCoat) )
    {
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        // マジックコート反応呼び出し
        u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
        BTL_SVFRET_AddMagicCoatAction( flowWk, pokeID, atkPokeID );

        // この時点でリアクションなし
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
      }
    }
  }
}
/**
 *  マジックコート（跳ね返し確定反応）
 */
void HandCommon_MagicCoat_Reaction( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID) )
  {
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MagicCoatExe );
      HANDEX_STR_AddArg( &param->str, pokeID );
      HANDEX_STR_AddArg( &param->str, BTL_EVENTVAR_GetValue(BTL_EVAR_WAZAID) );
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}

/**
 *  ワザ基本威力値をn倍に
 */
void HandCommon_MulWazaBasePower( u32 ratio )
{
  u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER ) * ratio;
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
}

//------------------------------------------------
/**
 * 逃げ計算スキップ共通（にげあし・けむりだま）
 */
//------------------------------------------------
void HandCommon_NigeruCalcSkip( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 escapePokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
  if( BTL_MAINUTIL_IsFriendPokeID(escapePokeID, pokeID) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
/**
 *  逃げ特殊メッセージ表示予約
 */
BOOL HandCommon_CheckNigeruExMsg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  u8 escapePokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
  if( BTL_MAINUTIL_IsFriendPokeID(escapePokeID, pokeID) )
  {
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_GEN_FLAG, TRUE) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

