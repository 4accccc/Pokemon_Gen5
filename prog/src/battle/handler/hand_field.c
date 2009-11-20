//=============================================================================================
/**
 * @file  hand_field.c
 * @brief ポケモンWB バトルシステム イベントファクター [フィールドエフェクト]
 * @author  taya
 *
 * @date  2009.06.18  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "battle\battle.h"

#include "..\btl_common.h"
#include "..\btl_calc.h"
#include "..\btl_field.h"
#include "..\btl_client.h"
#include "..\btl_event_factor.h"

#include "hand_field.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  WORKIDX_CONT = EVENT_HANDLER_WORK_ELEMS - 1,
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR* ADD_Fld_Weather( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static BTL_EVENT_FACTOR* ADD_Fld_TrickRoom( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_TrickRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static BTL_EVENT_FACTOR* ADD_Fld_Juryoku( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_Jyuryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static BTL_EVENT_FACTOR* ADD_Fld_Fuin( u16 pri, BtlFieldEffect effect, u8 subParam );
static BTL_EVENT_FACTOR* ADD_Fld_Sawagu( u16 pri, BtlFieldEffect effect, u8 subParam );
static BTL_EVENT_FACTOR* ADD_Fld_MizuAsobi( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_MizuAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static BTL_EVENT_FACTOR* ADD_Fld_DoroAsobi( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_DoroAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static BTL_EVENT_FACTOR* ADD_Fld_WonderRoom( u16 pri, BtlFieldEffect effect, u8 subParam );
static void handler_fld_WonderRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );




//=============================================================================================
/**
 * フィールドエフェクトハンドラをシステムに追加
 *
 * @param   effect      エフェクトタイプ
 * @param   sub_param   エフェクトサブパラメータ（現状、天候の種類を指定するのみ）
 *
 * @retval  BTL_EVENT_FACTOR*
 */
//=============================================================================================
BTL_EVENT_FACTOR*  BTL_HANDLER_FLD_Add( BtlFieldEffect effect, u8 sub_param )
{
  typedef BTL_EVENT_FACTOR* (*pEventAddFunc)( u16 pri, BtlFieldEffect effect, u8 subParam );

  static const struct {
    BtlFieldEffect eff;
    pEventAddFunc  func;
  }funcTbl[] = {
    { BTL_FLDEFF_WEATHER,    ADD_Fld_Weather     }, ///< 各種天候
    { BTL_FLDEFF_TRICKROOM,  ADD_Fld_TrickRoom   }, ///< トリックルーム
    { BTL_FLDEFF_JURYOKU,    ADD_Fld_Juryoku     }, ///< じゅうりょく
    { BTL_FLDEFF_FUIN ,      ADD_Fld_Fuin        }, ///< ふういん
    { BTL_FLDEFF_MIZUASOBI,  ADD_Fld_MizuAsobi   }, ///< みずあそび
    { BTL_FLDEFF_DOROASOBI,  ADD_Fld_DoroAsobi   }, ///< どろあそび
    { BTL_FLDEFF_WONDERROOM, ADD_Fld_WonderRoom  }, ///< ワンダールーム
  };

  {
    u32 i;
    for(i=0; i<NELEMS(funcTbl); ++i)
    {
      if( funcTbl[i].eff == effect )
      {
        return funcTbl[i].func( 0, effect, sub_param );
      }
    }
  }
  return NULL;
}

//=============================================================================================
/**
 * ハンドラを削除
 *
 * @param   factor
 */
//=============================================================================================
void BTL_HANDLER_FLD_Remove( BTL_EVENT_FACTOR* factor )
{
  BTL_EVENT_FACTOR_Remove( factor );
}


//--------------------------------------------------------------------------------------
/**
 *  天候
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_Weather( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,  handler_fld_Weather   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{

}
//--------------------------------------------------------------------------------------
/**
 *  トリックルーム
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_TrickRoom( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY,  handler_fld_TrickRoom   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_TrickRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  u16 agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY );
  agi = BTL_CALC_AGILITY_MAX - agi;
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_AGILITY, agi );
}
//--------------------------------------------------------------------------------------
/**
 *  じゅうりょく
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_Juryoku( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO,  handler_fld_Jyuryoku   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_Jyuryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.67) );
}
//--------------------------------------------------------------------------------------
/**
 *  ふういん
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_Fuin( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  // 今のところ、何もハンドラを用意する必要が無くなってしまった…
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
//--------------------------------------------------------------------------------------
/**
 *  さわぐ
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_Sawagu( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,  handler_fld_Weather   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
//--------------------------------------------------------------------------------------
/**
 *  みずあそび
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_MizuAsobi( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,  handler_fld_MizuAsobi   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_MizuAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_MIZU ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.5) );
  }
}
//--------------------------------------------------------------------------------------
/**
 *  どろあそび
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_DoroAsobi( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,  handler_fld_DoroAsobi   },  // ダメージ補正
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_DoroAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_DENKI ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.5) );
  }
}
//--------------------------------------------------------------------------------------
/**
 *  ワンダールーム
 */
//--------------------------------------------------------------------------------------
static BTL_EVENT_FACTOR* ADD_Fld_WonderRoom( u16 pri, BtlFieldEffect effect, u8 subParam )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DEFENDER_GUARD_PREV,  handler_fld_WonderRoom   },  // 防御側能力値計算の前
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, pri, subParam, HandlerTable );
}
static void handler_fld_WonderRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  int swap_cnt = BTL_EVENTVAR_GetValue( BTL_EVAR_VID_SWAP_CNT);
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_VID_SWAP_CNT, (swap_cnt+1) );
}


