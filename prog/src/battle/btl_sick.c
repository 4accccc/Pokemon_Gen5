
#include "btl_common.h"
#include "btl_pokeparam.h"
#include "btl_server_flow.h"
#include "btl_event.h"
#include "handler\hand_item.h"

#include "btl_sick.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void contProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick );
static void contDamageCommon( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick, u16 damage );
static int getWazaSickDamageStrID( WazaSick sick );
static void cont_HorobiNoUta( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID );
static void cont_Yadorigi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID );
static void cont_NeWoHaru( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID );
static void cureProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick );
static void cure_Akubi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp );
static void cure_HorobiNoUta( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp );
static void putHorobiCounter( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp, u8 count );
static void cure_Sasiosae( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp );
static int getCureStrID( WazaSick sick, BOOL fUseItem );



//=============================================================================================
/**
 * 状態異常化させられた時の処理
 *
 * @param   bpp
 * @param   sick
 */
//=============================================================================================
void BTL_SICK_AddProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, WazaSick sick )
{
  switch( sick ){
  case WAZASICK_SASIOSAE:
    BTL_HANDLER_ITEM_Remove( bpp );
    break;
  }
}



void BTL_SICK_TurnCheckCallback( BTL_POKEPARAM* bpp, WazaSick sick, BOOL fCure, void* work )
{
  u8 pokeID = BPP_GetID( bpp );

  if( fCure ){
    cureProc( work, bpp, pokeID, sick );
  }else{
    contProc( work, bpp, pokeID, sick );
  }
}

//----------------------------------------------------------------------------------------------
// 状態異常継続
//----------------------------------------------------------------------------------------------
static void contProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick )
{
  {
    u32 damage =  BPP_CalcSickDamage( bpp, sick );
    if( damage ){
      contDamageCommon( flowWk, bpp, pokeID, sick, damage );
    }
  }
  switch( sick ){
  case WAZASICK_HOROBINOUTA:    cont_HorobiNoUta( flowWk, bpp, pokeID ); break;
  case WAZASICK_YADORIGI:       cont_Yadorigi( flowWk, bpp, pokeID ); break;
  case WAZASICK_NEWOHARU:       cont_NeWoHaru( flowWk, bpp, pokeID ); break;
  }
}

/**
 *  ダメージを受ける状態異常の共通処理
 */
static void contDamageCommon( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick, u16 damage )
{
  BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
  int strID;

  param->poke_cnt = 1;
  param->pokeID[0] = pokeID;
  param->damage[0] = damage;

  strID = getWazaSickDamageStrID( sick );
  if( strID >= 0 ){
    param->fSucceedStrEx = TRUE;
    param->succeedStrID = strID;
  }
}
/**
 *  状態異常によるダメージ時の標準メッセージID取得
 */
static int getWazaSickDamageStrID( WazaSick sick )
{
  switch( sick ){
  case WAZASICK_DOKU:   return BTL_STRID_SET_DokuDamage;
  case WAZASICK_YAKEDO: return BTL_STRID_SET_YakedoDamage;
  case WAZASICK_AKUMU:  return BTL_STRID_SET_AkumuDamage;
  case WAZASICK_NOROI:  return BTL_STRID_SET_NoroiDamage;
  }
  return -1;
}

/**
 *  ほろびのうた：継続
 */
static void cont_HorobiNoUta( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID )
{
  BPP_SICK_CONT cont = BPP_GetSickCont( bpp, WAZASICK_HOROBINOUTA );
  u8 turnMax = BPP_SICCONT_GetTurnMax( cont );
  u8 turnNow = BPP_GetSickTurnCount( bpp, WAZASICK_HOROBINOUTA );
  int turnDiff = turnMax - turnNow;
  if( turnDiff > 0 ){
    putHorobiCounter( flowWk, bpp, turnDiff );
  }
}
/**
 *  やどりぎのたね：継続
 */
static void cont_Yadorigi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID )
{
  BTL_HANDEX_PARAM_DAMAGE* dmg_param;
  u16 damage = BTL_CALC_QuotMaxHP( bpp, 16 );

  dmg_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
  dmg_param->poke_cnt = 1;
  dmg_param->pokeID[0] = pokeID;
  dmg_param->damage[0] = damage;
  dmg_param->fSucceedStrEx = TRUE;
  dmg_param->succeedStrID = BTL_STRID_SET_YadorigiTurn;

  {
    BPP_SICK_CONT  cont = BPP_GetSickCont( bpp, WAZASICK_YADORIGI );
    BtlPokePos  pos = BPP_SICKCONT_GetParam( cont );
    u8 userPokeID[BTL_POS_MAX];
    if( BTL_SERVERFLOW_RECEPT_GetTargetPokeID(flowWk, pos, userPokeID) )
    {
      BTL_HANDEX_PARAM_DRAIN* drain_param;
      drain_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DRAIN, pokeID );
      drain_param->header.failSkipFlag = TRUE;
      drain_param->recoverPokeID = userPokeID[0];
      drain_param->damagedPokeID = pokeID;
      drain_param->recoverHP = damage;
    }
  }

}
/**
 *  ねをはる：継続
 */
static void cont_NeWoHaru( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID )
{
  if( !BPP_IsHPFull(bpp) )
  {
    BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
    param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 16 );
    param->pokeID = pokeID;
    HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_NeWoHaruRecover );
    HANDEX_STR_AddArg( &param->exStr, pokeID );
  }
}

//----------------------------------------------------------------------------------------------
// 状態異常回復
//----------------------------------------------------------------------------------------------
static void cureProc( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp, u8 pokeID, WazaSick sick )
{
  int strID = getCureStrID( sick, FALSE );
  if( strID >= 0 )
  {
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
    HANDEX_STR_AddArg( &param->str, pokeID );
    return;
  }

  switch( sick ){
  case WAZASICK_AKUBI:       cure_Akubi( flowWk, bpp ); break;
  case WAZASICK_HOROBINOUTA: cure_HorobiNoUta( flowWk, bpp ); break;
  case WAZASICK_SASIOSAE:    cure_Sasiosae( flowWk, bpp ); break;
  }
}

static void cure_Akubi( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp )
{
  int turns = BTL_CALC_RandRange( BTL_NEMURI_TURN_MIN, BTL_NEMURI_TURN_MAX );
  turns--;  // ターンチェック処理中のため、１ターン分あらかじめ短くしておく
  if( turns > 0 )
  {
    u8 pokeID = BPP_GetID( bpp );
    BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
    param->fAlmost = TRUE;
    param->sickID = WAZASICK_NEMURI;
    param->sickCont = BPP_SICKCONT_MakeTurn( turns );
    param->pokeID[0] = pokeID;
    param->poke_cnt = 1;
  }
}

static void cure_HorobiNoUta( BTL_SVFLOW_WORK* flowWk, BTL_POKEPARAM* bpp )
{
  putHorobiCounter( flowWk, bpp, 0 );
  {
    u8 pokeID = BPP_GetID( bpp );
    BTL_HANDEX_PARAM_KILL* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_KILL, pokeID );
    param->pokeID = pokeID;
  }
}

// ほろびのうたカウント表示共通処理
static void putHorobiCounter( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp, u8 count )
{
  u8 pokeID = BPP_GetID( bpp );
  BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
  HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_HorobiCountDown );
  HANDEX_STR_AddArg( &param->str, pokeID );
  HANDEX_STR_AddArg( &param->str, count );

}

static void cure_Sasiosae( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* bpp )
{
  BTL_HANDLER_ITEM_Add( bpp );
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
    { WAZASICK_SASIOSAE,  BTL_STRID_SET_Sasiosae,         BTL_STRID_SET_SasiosaeCure        },

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
  case WAZASICK_YADORIGI:   strID = BTL_STRID_SET_Yadorigi; break;
  case WAZASICK_MIYABURU:   strID = BTL_STRID_SET_Miyaburu; break;
  case WAZASICK_NEWOHARU:   strID = BTL_STRID_SET_NeWoHaru; break;
  case WAZASICK_DOKU:
    strID = BPP_SICKCONT_IsMoudokuCont(cont)? BTL_STRID_SET_MoudokuGet : BTL_STRID_SET_DokuGet;
    break;

  default:
    break;
  }

  return strID;
}







//=============================================================================================
/**
 *
 *
 * @param   flowWk
 * @param   defender
 */
//=============================================================================================
void BTL_SICK_CheckNotEffectByType( BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender )
{
  if( BPP_CheckSick(defender, WAZASICK_MIYABURU) )
  {
    BPP_SICK_CONT cont = BPP_GetSickCont( defender, WAZASICK_MIYABURU );
    PokeType type = BPP_SICKCONT_GetParam( cont );
    if( BPP_IsMatchType(defender, type) ){
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, FALSE );
    }
  }
}

void BTL_SICK_CheckDamageAffinity(  BTL_SVFLOW_WORK* flowWk, const BTL_POKEPARAM* defender )
{
  if( BPP_CheckSick(defender, WAZASICK_MIYABURU) )
  {
    BPP_SICK_CONT cont = BPP_GetSickCont( defender, WAZASICK_MIYABURU );
    PokeType type = BPP_SICKCONT_GetParam( cont );
    if( BPP_IsMatchType(defender, type) ){
      BtlTypeAff aff = BTL_EVENTVAR_GetValue( BTL_EVAR_TYPEAFF );
      if( aff == BTL_TYPEAFF_0 ){
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_TYPEAFF, BTL_TYPEAFF_100 );
      }
    }
  }
}

