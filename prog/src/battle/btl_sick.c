
#include "btl_common.h"
#include "btl_pokeparam.h"
#include "btl_server_flow.h"

#include "btl_sick.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void cureProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, WazaSick sick );
static void cure_Akubi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp );
static int getCureStrID( WazaSick sick, BOOL fUseItem );




void BTL_SICK_TurnCheckCallback( BTL_POKEPARAM* bpp, WazaSick sick, BOOL fCure, void* work )
{
  if( fCure ){
    cureProc( work, bpp, sick );
  }
}

static void cureProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, WazaSick sick )
{
  int strID = getCureStrID( sick, FALSE );
  if( strID >= 0 )
  {
    u8 pokeID = BTL_POKEPARAM_GetID( bpp );
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
    HANDEX_STR_AddArg( &param->str, pokeID );
    return;
  }

  switch( sick ){
  case WAZASICK_AKUBI:    cure_Akubi( flowWk, bpp ); break;
  }
}

static void cure_Akubi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp )
{
  int turns = BTL_CALC_RandRange( BTL_NEMURI_TURN_MIN, BTL_NEMURI_TURN_MAX );
  turns--;  // ターンチェック処理中のため、１ターン分あらかじめ短くしておく
  if( turns > 0 )
  {
    u8 pokeID = BTL_POKEPARAM_GetID( bpp );
    BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
    param->fAlmost = TRUE;
    param->sickID = WAZASICK_NEMURI;
    param->sickCont = BPP_SICKCONT_MakeTurn( turns );
    param->pokeID[0] = pokeID;
    param->poke_cnt = 1;
  }
}


static int getCureStrID( WazaSick sick, BOOL fUseItem )
{
  static const struct {
    WazaSick  sick;
    s16       strID_notItem;
    s16       strID_useItem;
  }dispatchTbl[] = {
    { WAZASICK_DOKU,      BTL_STRID_SET_PokeSick_Cure,    BTL_STRID_SET_UseItem_CureDoku    },
    { POKESICK_YAKEDO,    BTL_STRID_SET_PokeSick_Cure,    BTL_STRID_SET_UseItem_CureYakedo  },
    { WAZASICK_NEMURI,    BTL_STRID_SET_NemuriWake,       BTL_STRID_SET_UseItem_CureNemuri  },
    { WAZASICK_KOORI,     BTL_STRID_SET_KoriMelt,         BTL_STRID_SET_UseItem_CureKoori   },
    { WAZASICK_MAHI,      BTL_STRID_SET_MahiCure,         BTL_STRID_SET_UseItem_CureMahi    },
    { WAZASICK_ENCORE,    BTL_STRID_SET_EncoreCure,       -1                                },
    { WAZASICK_KANASIBARI,BTL_STRID_SET_KanasibariCure,   -1                                },

    { WAZASICK_KONRAN,    -1,                             BTL_STRID_SET_UseItem_CureKonran  },
    { WAZASICK_MEROMERO,  -1,                             BTL_STRID_SET_UseItem_CureMero    },
  };

  u32 i;

  for(i=0; i<NELEMS(dispatchTbl); ++i){
    if( dispatchTbl[i].sick == sick ){
      return (fUseItem)? dispatchTbl[i].strID_useItem : dispatchTbl[i].strID_notItem;
    }
  }

  return -1;

}


  #if 0
    for( sick=WAZASICK_ORIGIN; sick<WAZASICK_MAX; ++sick )
    {
      damage = BTL_POKEPARAM_CalcSickDamage( bpp, sick );
      if( damage )
      {
        BTL_EVENTVAR_Push();
          BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );
          BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
          BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, damage );
          BTL_EVENT_CallHandlers( wk, BTL_EVENT_SICK_DAMAGE );
          damage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
        BTL_EVENTVAR_Pop();
        if( damage > 0 )
        {
          BTL_POKEPARAM_HpMinus( bpp, damage );
          SCQUE_PUT_OP_HpMinus( wk->que, pokeID, damage );
          SCQUE_PUT_SickDamage( wk->que, pokeID, sick, damage );
          if( BTL_POKEPARAM_IsDead(bpp) ){
            break;
          }
        }
        else if( damage < 0 )
        {
          int recover = -damage;
          BTL_POKEPARAM_HpPlus( bpp, recover );
          SCQUE_PUT_OP_HpPlus( wk->que, pokeID, recover );
          SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
        }
      }
    }
    if( !BTL_POKEPARAM_IsDead(bpp) ){
      BTL_POKEPARAM_WazaSick_TurnCheck( bpp );
      SCQUE_PUT_OP_WazaSickTurnCheck( wk->que, pokeID );
    }
    scproc_CheckDeadCmd( wk, bpp );
  #endif



//=============================================================================================
/**
 * 状態異常にかかった時のデフォルトメッセージＩＤを返す
 *
 * @param   sickID
 * @param   cont
 *
 * @retval  int   メッセージID（存在しない場合-1）
 */
//=============================================================================================
int BTL_SICK_GetDefaultSickStrID( WazaSick sickID, BPP_SICK_CONT cont )
{
  int strID = -1;


  switch( sickID ){
  case WAZASICK_YAKEDO:     strID = BTL_STRID_SET_YakedoGet; break;
  case WAZASICK_MAHI:       strID = BTL_STRID_SET_MahiGet; break;
  case WAZASICK_KOORI:      strID = BTL_STRID_SET_KoriGet; break;
  case WAZASICK_NEMURI:     strID = BTL_STRID_SET_NemuriGet; break;
  case WAZASICK_KONRAN:     strID = BTL_STRID_SET_KonranGet; break;
  case WAZASICK_MEROMERO:   strID = BTL_STRID_SET_MeromeroGet; break;
  case WAZASICK_AKUMU:      strID = BTL_STRID_SET_AkumuGet;  break;
  case WAZASICK_NOROI:      strID = BTL_STRID_SET_Noroi; break;
  case WAZASICK_TYOUHATSU:  strID = BTL_STRID_SET_Chouhatu; break;
  case WAZASICK_ICHAMON:    strID = BTL_STRID_SET_Ichamon; break;
  case WAZASICK_AKUBI:      strID = BTL_STRID_SET_Akubi; break;

  case WAZASICK_DOKU:
    strID = BPP_SICKCONT_IsMoudokuCont(cont)? BTL_STRID_SET_MoudokuGet : BTL_STRID_SET_DokuGet;
    break;

  default:
    break;
  }

  return strID;
}

