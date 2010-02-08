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
static const BtlEventHandlerTable* ADD_Fld_Weather( u32* numElems );
static void handler_fld_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_TrickRoom( u32* numElems );
static void handler_fld_TrickRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_Juryoku( u32* numElems );
static void handler_fld_Jyuryoku_AdjustDmg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static void handler_fld_Jyuryoku_CheckAff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_Sawagu( u32* numElems );
static const BtlEventHandlerTable* ADD_Fld_MizuAsobi( u32* numElems );
static void handler_fld_MizuAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_DoroAsobi( u32* numElems );
static void handler_fld_DoroAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_WonderRoom( u32* numElems );
static void handler_fld_WonderRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );
static const BtlEventHandlerTable* ADD_Fld_Fuin( u32* numElems );
static const BtlEventHandlerTable* ADD_Fld_MagicRoom( u32* numElems );
static void handler_fld_dummy( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work );




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
  typedef const BtlEventHandlerTable* (*pEventAddFunc)( u32* numHandlers );

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
    { BTL_FLDEFF_MAGICROOM,  ADD_Fld_MagicRoom   }, ///< マジックルーム
  };

  {
    u32 i;
    for(i=0; i<NELEMS(funcTbl); ++i)
    {
      if( funcTbl[i].eff == effect )
      {
        const BtlEventHandlerTable* handlerTable;
        u32 numHandlers;

        handlerTable = funcTbl[i].func( &numHandlers );
        return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_FIELD, effect, 0, sub_param, handlerTable, numHandlers );
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
static const BtlEventHandlerTable* ADD_Fld_Weather( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,  handler_fld_Weather   },  // ダメージ補正
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{

}
//--------------------------------------------------------------------------------------
/**
 *  トリックルーム
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_TrickRoom( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY,  handler_fld_TrickRoom   },  // ダメージ補正
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_TrickRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_TRICK_FLAG, TRUE );
}
//--------------------------------------------------------------------------------------
/**
 *  じゅうりょく
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_Juryoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO,  handler_fld_Jyuryoku_AdjustDmg   },  // ダメージ補正ハンドラ
    { BTL_EVENT_CHECK_AFFINITY,  handler_fld_Jyuryoku_CheckAff    },  // 相性チェックハンドラ
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
// ダメージ補正ハンドラ
static void handler_fld_Jyuryoku_AdjustDmg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.67) );
}
// 相性チェックハンドラ
static void handler_fld_Jyuryoku_CheckAff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_JIMEN )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FLAT_FLAG, TRUE );
  }
}


//--------------------------------------------------------------------------------------
/**
 *  さわぐ
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_Sawagu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,  handler_fld_Weather   },  // ダメージ補正
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
//--------------------------------------------------------------------------------------
/**
 *  みずあそび
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_MizuAsobi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,  handler_fld_MizuAsobi   },  // ダメージ補正
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_MizuAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_HONOO ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.33) );
  }
}
//--------------------------------------------------------------------------------------
/**
 *  どろあそび
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_DoroAsobi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,  handler_fld_DoroAsobi   },  // ダメージ補正
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_DoroAsobi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_DENKI ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.33) );
  }
}
//--------------------------------------------------------------------------------------
/**
 *  ワンダールーム
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_WonderRoom( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DEFENDER_GUARD_PREV,  handler_fld_WonderRoom   },  // 防御側能力値計算の前
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_WonderRoom( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{
  int swap_cnt = BTL_EVENTVAR_GetValue( BTL_EVAR_VID_SWAP_CNT);
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_VID_SWAP_CNT, (swap_cnt+1) );
}

//--------------------------------------------------------------------------------------
/**
 *  ふういん
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_Fuin( u32* numElems )
{
  // 今のところ、何もハンドラを用意する必要が無くなってしまった…
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NULL, handler_fld_dummy },
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
//--------------------------------------------------------------------------------------
/**
 *  マジックルーム
 */
//--------------------------------------------------------------------------------------
static const BtlEventHandlerTable* ADD_Fld_MagicRoom( u32* numElems )
{
  // 現状、マジックルームは登録しておくことで効果は各所で判断するため、
  // こいつのハンドラ自体は何もしない。
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NULL, handler_fld_dummy },
  };
  *numElems = NELEMS( HandlerTable );
  return HandlerTable;
}
static void handler_fld_dummy( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 subParam, int* work )
{

}

