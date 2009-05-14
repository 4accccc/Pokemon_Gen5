//=============================================================================================
/**
 * @file  hand_tok_ikaku.c
 * @brief ポケモンWB バトルシステム イベントファクター[とくせい]：『いかく』
 * @author  taya
 *
 * @date  2008.11.11  作成
 */
//=============================================================================================

#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_MemberComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWork, u8 pokeID, int* work );



//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
  { BTL_EVENT_MEMBER_IN, handler_MemberComp },
  { BTL_EVENT_NULL, NULL },
};
//BTL_EVENT_FACTOR_Remove

BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u16 tokID, u8 pokeID )
{
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


// ポケモンが出場した時のハンドラ
static void handler_MemberComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

    param->header.tokwin_flag = TRUE;
    param->rankType = BPP_ATTACK;
    param->rankVolume = -1;
    param->fAlmost = TRUE;

    {
      BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );
      BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_ENEMY_ALL, myPos );

      param->poke_cnt = BTL_SERVERFLOW_RECEPT_GetTargetPokeID( flowWk, expos, param->pokeID );
    }
/*
    BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
    BTL_SERVER_RECEPT_RankDownEffect( flowWk, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, myPos), BPP_ATTACK, 1, TRUE );
    BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
*/
//    BTL_EVENT_FACTOR_Remove( myHandle );
  }
}

