//=============================================================================================
/**
 * @file  hand_tok_clearbody.c
 * @brief ポケモンWB バトルシステム  イベントファクター[とくせい]：『クリアボディ＆しろいけむり』
 * @author  taya
 *
 * @date  2008.11.11  作成
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


static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ClearBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_BeforeRankDown },
    { BTL_EVENT_NULL, NULL },
  };

  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


static void handler_BeforeRankDown( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_VOLUME) < 0 )
    {
      BTL_EVWK_CHECK_RANKEFF* evwk = (BTL_EVWK_CHECK_RANKEFF*)BTL_EVENTVAR_GetValue( BTL_EVAR_WORK_ADRS );
      evwk->failFlag = TRUE;
      evwk->failTokuseiFlag = TRUE;
    }
  }
}

