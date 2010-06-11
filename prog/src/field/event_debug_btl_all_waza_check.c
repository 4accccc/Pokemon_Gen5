//=============================================================================
/**
 *
 *	@file		event_debug_btl_all_waza_check.c
 *	@brief
 *	@author		hosaka genya
 *	@data		2010.06.10
 *
 */
//=============================================================================
#ifdef PM_DEBUG

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "poke_tool/poke_memo.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "field/zonedata.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "fieldmap.h"

#include "message.naix"
#include "font/font.naix"
#include "msg/debug/msg_d_field.h"

#include "event_fieldmap_control.h"
#include "event_debug_btl_all_waza_check.h"

#include "field/event_battle.h" // for EVENT_WildPokeBattle
#include "field/field_encount.h" // for BTL_FIELD_SITUATION_SetFromFieldStatus
#include "gamesystem/btl_setup.h" // for BATTLE_PARAM_Create
#include "waza_tool/wazano_def.h" // for WAZANO_

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	
//==============================================================
typedef struct {
  GAMESYS_WORK*   gsys;
  int count;
} DEBUG_BTL_ALL_WAZA_CHECK_WORK;

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static GMEVENT_RESULT BtlAllWazaCheck( GMEVENT* event, int* seq, void* wk_adrs );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  全技チェックイベントを生成する
 *
 *	@param	GAMESYS_WORK* gsys
 *	@param	wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_DEBUG_BtlAllWazaCheck( GAMESYS_WORK* gsys, void* wk_adrs )
{
  GMEVENT* event;
  DEBUG_BTL_ALL_WAZA_CHECK_WORK* wk;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gsys );

  event = GMEVENT_Create( gsys, NULL, BtlAllWazaCheck, sizeof(DEBUG_BTL_ALL_WAZA_CHECK_WORK) );

  wk = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( wk, sizeof(DEBUG_BTL_ALL_WAZA_CHECK_WORK) );
  wk->gsys = gsys;

  return event;
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  全技チェックイベント
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT BtlAllWazaCheck( GMEVENT* event, int* seq, void* wk_adrs )
{
  enum
  {
    SEQ_INIT = 0,
    SEQ_SETUP,
    SEQ_RET,
    SEQ_EXIT,
  };
  
  DEBUG_BTL_ALL_WAZA_CHECK_WORK* wk = wk_adrs;
  GAMESYS_WORK* gsys = wk->gsys;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData(gsys);

  switch( *seq )
  {
    case SEQ_INIT:
      wk->count = 1;
      *seq = SEQ_SETUP;
      break;
    case SEQ_SETUP:

    // 自分のポケモンを強制上書き
    {
      POKEPARTY* mp;
      POKEMON_PARAM* pp;
      mp = GAMEDATA_GetMyPokemon(gdata);
      pp = PokeParty_GetMemberPointer( mp, 0 );

      // HP回復／状態異常回復
      PP_Put( pp, ID_PARA_hp, PP_Get( pp, ID_PARA_hpmax, NULL ) );
      PP_SetSick( pp, POKESICK_NULL ); 
      PP_ChangeMonsNo( pp, 1 );
      PP_SetWazaPos( pp, wk->count, 0 );
      OS_TPrintf("セットした技ID=%d\n",wk->count);
    }
    
    // バトル呼び出し
    {
      FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
      POKEPARTY* pe = PokeParty_AllocPartyWork( heapID );
      POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( heapID, POKETOOL_GetWorkSize() );
      BTL_FIELD_SITUATION sit;
      BATTLE_SETUP_PARAM* bp;
      GMEVENT * new_event;

      PokeParty_Init( pe, 1 );
      PP_Clear( pp );
      PP_Setup( pp, 1, 5, 0 );
      PokeParty_Add( pe, pp );
      
      bp = BATTLE_PARAM_Create( HEAPID_PROC );

      BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, gdata, fieldmap );
      BTL_SETUP_Wild( bp, gdata, pe, &sit, BTL_RULE_SINGLE, HEAPID_PROC );
//      BTL_SETUP_Single_Trainer( bp, gdata, &sit, TRID_RIVAL_01, HEAPID_PROC );

      // HP/PP無限 絶対当たる
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_HP_CONST );
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_PP_CONST );
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_HIT100PER );
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_SKIP_BTLIN ); // 登場アニメ飛ばす
      
      GFL_HEAP_FreeMemory( pp );
      GFL_HEAP_FreeMemory( pe );

      new_event = EVENT_WildPokeBattle( gsys, fieldmap, bp, FALSE, ENC_TYPE_NORMAL );
    
      GMEVENT_CallEvent( event, new_event );
    }
      *seq = SEQ_RET;
      break;

    case SEQ_RET:
      // 次の技へ
      wk->count++;
      if( wk->count < WAZANO_MAX )
      {
        *seq = SEQ_SETUP;
      }
      else
      {
        *seq = SEQ_EXIT;
      }
      break;

    case SEQ_EXIT:
      return GMEVENT_RES_FINISH;
    default : GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

#endif // PM_DEBUG
