//=============================================================================================
/**
 * @file  hand_side.c
 * @brief ポケモンWB バトルシステム イベントファクター [サイドエフェクト]
 * @author  taya
 *
 * @date  2009.06.18  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "poke_tool\monsno_def.h"

#include "..\btl_common.h"
#include "..\btl_calc.h"
#include "..\btl_field.h"
#include "..\btl_client.h"
#include "..\btl_event_factor.h"

#include "hand_side.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  WORKIDX_CONT = EVENT_HANDLER_WORK_ELEMS - 1,
};


/**
 *  重複管理用ワーク
 */
typedef struct {

  BPP_SICK_CONT  contParam;
  u16            counter;
  u8             enableFlag;

}EXIST_EFFECT;

static EXIST_EFFECT ExistEffect[ BTL_SIDE_MAX ][ BTL_SIDEEFF_MAX ];


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR* ADD_Refrector( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Hikarinokabe( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Sinpinomamori( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Siroikiri( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Makibisi( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Dokubisi( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Mizuasobi( u16 pri, BtlSideEffect eff, u8 pokeID );
static BTL_EVENT_FACTOR* ADD_Doroasobi( u16 pri, BtlSideEffect eff, u8 pokeID );




//=============================================================================================
/**
 * システム初期化
 *
 * @param   none
 */
//=============================================================================================
void BTL_HANDLER_SIDE_InitSystem( void )
{
  GFL_STD_MemClear( ExistEffect, sizeof(ExistEffect) );
}

//=============================================================================================
/**
 * サイドエフェクトハンドラをシステムに追加
 *
 * @param   pp
 * @param   sideEffect
 * @param   contParam
 *
 * @retval  BTL_EVENT_FACTOR*   追加されたイベントハンドラ（重複して追加できない場合NULL）
 */
//=============================================================================================
BTL_EVENT_FACTOR*  BTL_HANDLER_SIDE_Add( const BTL_POKEPARAM* pp, BtlSideEffect sideEffect, BPP_SICK_CONT contParam )
{
  typedef BTL_EVENT_FACTOR* (*pEventAddFunc)( u16 pri, BtlSideEffect eff, u8 pokeID );

  static const struct {
    BtlSideEffect  eff;
    pEventAddFunc  func;
  }funcTbl[] = {
    { BTL_SIDEEFF_REFRECTOR,      ADD_Refrector     },
    { BTL_SIDEEFF_HIKARINOKABE,   ADD_Hikarinokabe  },
    { BTL_SIDEEFF_SINPINOMAMORI,  ADD_Sinpinomamori },
    { BTL_SIDEEFF_SIROIKIRI,      ADD_Siroikiri     },
    { BTL_SIDEEFF_MAKIBISI,       ADD_Makibisi      },
    { BTL_SIDEEFF_DOKUBISI,       ADD_Dokubisi      },
    { BTL_SIDEEFF_MIZUASOBI,      ADD_Mizuasobi     },
    { BTL_SIDEEFF_DOROASOBI,      ADD_Doroasobi     },
  };
  BtlSide  side = BTL_MAINUTIL_PokeIDtoSide( BTL_POKEPARAM_GetID(pp) );
  if( ExistEffect[ side ][ sideEffect ].enableFlag == FALSE )
  {
    u32 i;
    for(i=0; i<NELEMS(funcTbl); ++i)
    {
      if( funcTbl[i].eff == sideEffect )
      {
        BTL_EVENT_FACTOR* factor = funcTbl[i].func( 0, sideEffect, BTL_POKEPARAM_GetID(pp) );
        BTL_EVENT_FACTOR_SetWorkValue( factor, WORKIDX_CONT, contParam.raw );
        return factor;
      }
    }
  }
  return NULL;
}

static BTL_EVENT_FACTOR* ADD_Refrector( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Hikarinokabe( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Sinpinomamori( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Siroikiri( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Makibisi( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Dokubisi( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Mizuasobi( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}
static BTL_EVENT_FACTOR* ADD_Doroasobi( u16 pri, BtlSideEffect eff, u8 pokeID )
{
  return NULL;
}

