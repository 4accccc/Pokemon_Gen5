//=============================================================================================
/**
 * @file  hand_waza.c
 * @brief ポケモンWB バトルシステム イベントファクター [ワザ] 処理
 * @author  taya
 *
 * @date  2009.05.15  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazano_def.h"

#include "..\btl_common.h"
#include "..\btl_calc.h"
#include "..\btl_field.h"
#include "..\btl_client.h"
#include "..\btl_event_factor.h"

#include "hand_waza.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR*  ADD_Texture( u16 pri, WazaID waza, u8 pokeID );
static void handler_Texture( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  ADD_KuroiKiri( u16 pri, WazaID waza, u8 pokeID );
static void handler_KuroiKiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  ADD_Yumekui( u16 pri, WazaID waza, u8 pokeID );
static void handler_Yumekui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  ADD_TriAttack( u16 pri, WazaID waza, u8 pokeID );
static void handler_TriAttack( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  ADD_IkariNoMaeba( u16 pri, WazaID waza, u8 pokeID );
static void handler_IkariNoMaeba( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  ADD_RyuuNoIkari( u16 pri, WazaID waza, u8 pokeID );
static void handler_RyuuNoIkari( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );




//=============================================================================================
/**
 * ポケモンの「ワザ」ハンドラをシステムに追加
 *
 * @param   pp
 *
 * @retval  BTL_EVENT_FACTOR*
 */
//=============================================================================================
BTL_EVENT_FACTOR*  BTL_HANDLER_Waza_Add( const BTL_POKEPARAM* pp, WazaID waza )
{
  typedef BTL_EVENT_FACTOR* (*pEventAddFunc)( u16 pri, WazaID wazaID, u8 pokeID );

  static const struct {
    WazaID         waza;
    pEventAddFunc  func;
  }funcTbl[] = {
    { WAZANO_TEKUSUTYAA,      ADD_Texture       },
    { WAZANO_KUROIKIRI,       ADD_KuroiKiri     },
    { WAZANO_YUMEKUI,         ADD_Yumekui       },
    { WAZANO_TORAIATAKKU,     ADD_TriAttack     },
    { WAZANO_IKARINOMAEBA,    ADD_IkariNoMaeba  },
    { WAZANO_RYUUNOIKARI,     ADD_RyuuNoIkari   },
  };

  int i;

  for(i=0; i<NELEMS(funcTbl); i++)
  {
    if( funcTbl[i].waza == waza )
    {
      u16 agi = BTL_POKEPARAM_GetValue( pp, BPP_AGILITY );
      u8 pokeID = BTL_POKEPARAM_GetID( pp );

      return funcTbl[i].func( agi, waza, pokeID );
    }
  }

  return NULL;
}
//=============================================================================================
/**
 * ポケモンの「ワザ」ハンドラをシステムから削除
 *
 * @param   pp
 * @param   waza
 */
//=============================================================================================
void BTL_HANDLER_Waza_Remove( const BTL_POKEPARAM* pp, WazaID waza )
{
  BTL_EVENT_FACTOR* factor;
  u8 pokeID = BTL_POKEPARAM_GetID( pp );

  while( (factor = BTL_EVENT_SeekFactor(BTL_EVENT_FACTOR_WAZA, pokeID)) != NULL )
  {
    BTL_EVENT_FACTOR_Remove( factor );
  }

}

//----------------------------------------------------------------------------------
/**
 * テクスチャー
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_Texture( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_UNCATEGORY_WAZA, handler_Texture },    // 未分類ワザハンドラ
//    { BTL_EVENT_USE_ITEM,      handler_KuraboNomi_Exe },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_Texture( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    u16 waza_cnt = BTL_POKEPARAM_GetWazaCount( bpp );
    PokeType next_type;
    {
      PokeType type[ PTL_WAZA_MAX ];
      u16 i;
      for(i=0; i<waza_cnt; ++i){
        type[i] = WAZADATA_GetType( BTL_POKEPARAM_GetWazaNumber(bpp, i) );
      }
      i = GFL_STD_MtRand( waza_cnt );
      next_type = PokeTypePair_MakePure( type[i] );
    }

    {
      BTL_HANDEX_PARAM_CHANGE_TYPE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CHANGE_TYPE, pokeID );
      param->next_type = next_type;
      param->pokeID = pokeID;
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * くろいきり
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_KuroiKiri( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_UNCATEGORY_WAZA, handler_KuroiKiri },    // 未分類ワザハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_KuroiKiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_HANDEX_PARAM_RESET_RANK* reset_param;
    BTL_HANDEX_PARAM_MESSAGE* msg_param;

    reset_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RESET_RANK, pokeID );
    msg_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE_SET, pokeID );

    {
      BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );
      BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_ALL, myPos );

      reset_param->poke_cnt = BTL_SERVERFLOW_RECEPT_GetTargetPokeID( flowWk, expos, reset_param->pokeID );
    }

    msg_param->strID = BTL_STRID_STD_RankReset;
  }
}

//----------------------------------------------------------------------------------
/**
 * ゆめくい
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_Yumekui( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L2, handler_Yumekui },    // 無効化チェックレベル２ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_Yumekui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
    const BTL_POKEPARAM* target = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, targetPokeID );

    if( !BTL_POKEPARAM_CheckSick(target, POKESICK_NEMURI) )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * トライアタック
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_TriAttack( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L2, handler_TriAttack },    // 無効化チェックレベル２ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_TriAttack( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    static const PokeSick sick_tbl[] = {
      POKESICK_MAHI, POKESICK_YAKEDO, POKESICK_KOORI
    };
    PokeSick sick;
    BPP_SICK_CONT cont;
    u8 rand = GFL_STD_MtRand( NELEMS(sick_tbl) );

    sick = sick_tbl[ rand ];
    BTL_CALC_MakeDefaultPokeSickCont( sick, &cont );

    BTL_EVENTVAR_RewriteValue( BTL_EVAR_SICKID, sick );
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_SICK_CONT, cont.raw );
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_ADD_PER, 20 );
  }
}
//----------------------------------------------------------------------------------
/**
 * いかりのまえば
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_IkariNoMaeba( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_IkariNoMaeba },    // ダメージ計算最終ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_IkariNoMaeba( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
    const BTL_POKEPARAM* target = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, targetPokeID );

    u16 damage = BTL_POKEPARAM_GetValue(target, BPP_HP) / 2;

    BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, damage );
  }
}
//----------------------------------------------------------------------------------
/**
 * りゅうのいかり
 */
//----------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  ADD_RyuuNoIkari( u16 pri, WazaID waza, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_RyuuNoIkari },    // ダメージ計算最終ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_WAZA, waza, pri, pokeID, HandlerTable );
}

static void handler_RyuuNoIkari( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, 40 );
  }
}



