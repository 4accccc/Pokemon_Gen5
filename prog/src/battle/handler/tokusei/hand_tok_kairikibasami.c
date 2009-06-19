//=============================================================================================
/**
 * @file  hand_tok_kairikibasami.c
 * @brief ポケモンWB バトルシステム  イベントファクター[とくせい]：『かいりきばさみ』
 * @author  taya
 *
 * @date  2009.03.12  作成
 */
//=============================================================================================
#include "../../btl_string.h"
#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_BeforeRankDown( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );



//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------


BTL_EVENT_FACTOR*  HAND_TOK_ADD_KairikiBasami( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_KairikiBasami_Check },
    { BTL_EVENT_NULL, NULL },
  };

  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


static void handler_KairikiBasami_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  WazaRankEffect  effType = WAZA_RANKEFF_ATTACK;
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_STATUS_TYPE) ==  effType)
  ){
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_VOLUME) < 0 )
    {
      BTL_EVWK_CHECK_RANKEFF* evwk = (BTL_EVWK_CHECK_RANKEFF*)BTL_EVENTVAR_GetValue( BTL_EVAR_WORK_ADRS );
      evwk->failFlag = TRUE;
      evwk->failTokuseiFlag = TRUE;
      evwk->failSpecificType = effType;
    }
  }
}

