//=============================================================================
/**
 *
 *	@file		event_debug_btl_all_poke_check.c
 *	@brief  バトルで全ポケモン表示
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

#include "poke_tool/status_rcv.h" // for STATUS_RCV_PokeParty_RecoverAll

#include "event_fieldmap_control.h"
#include "event_debug_btl_all_poke_check.h"

#include "field/event_battle.h" // for EVENT_WildPokeBattle
#include "field/field_encount.h" // for BTL_FIELD_SITUATION_SetFromFieldStatus
#include "gamesystem/btl_setup.h" // for BATTLE_PARAM_Create
#include "waza_tool/wazano_def.h" // for WAZANO_
#include "waza_tool/wazadata.h" // for WAZADATA_GetCategory

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
} DEBUG_BTL_ALL_POKE_CHECK_WORK;

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static GMEVENT_RESULT BtlAllPokeCheck( GMEVENT* event, int* seq, void* wk_adrs );

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
GMEVENT* EVENT_DEBUG_BtlAllPokeCheck( GAMESYS_WORK* gsys, void* wk_adrs )
{
  GMEVENT* event;
  DEBUG_BTL_ALL_POKE_CHECK_WORK* wk;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gsys );

  event = GMEVENT_Create( gsys, NULL, BtlAllPokeCheck, sizeof(DEBUG_BTL_ALL_POKE_CHECK_WORK) );

  wk = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( wk, sizeof(DEBUG_BTL_ALL_POKE_CHECK_WORK) );
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
 *	@brief  全ポケモンチェックイベント
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT BtlAllPokeCheck( GMEVENT* event, int* seq, void* wk_adrs )
{
  enum
  {
    SEQ_INIT = 0,
    SEQ_SETUP,
    SEQ_RET,
    SEQ_EXIT,
  };
  
  int i;
  DEBUG_BTL_ALL_POKE_CHECK_WORK* wk = wk_adrs;
  GAMESYS_WORK* gsys = wk->gsys;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData(gsys);

  switch( *seq )
  {
    case SEQ_INIT:
      wk->count = 1;
      OS_TPrintf("トリプル全ポケモンチェック開始！\n");
      *seq = SEQ_SETUP;
      break;

    case SEQ_SETUP:
  
    // バトル呼び出し
    {
      FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
      BTL_FIELD_SITUATION sit;
      BATTLE_SETUP_PARAM* bp;
      GMEVENT * new_event;

      bp = BATTLE_PARAM_Create( HEAPID_PROC );

      BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, gdata, fieldmap );
      BTL_SETUP_Triple_Trainer( bp, gdata, &sit, TRID_BIGFOUR3_01, HEAPID_PROC );

      // 味方／敵を上書き
      {
        POKEPARTY* mp = bp->party[ BTL_CLIENT_PLAYER ];
        POKEPARTY* ep = bp->party[ BTL_CLIENT_ENEMY1 ];
      
        for( i=0; i<3; i++ )
        {
          POKEMON_PARAM* pp;

          pp = PokeParty_GetMemberPointer( mp, i );
          PP_Setup( pp, wk->count, 1, PTL_SETUP_ID_AUTO );
          pp = PokeParty_GetMemberPointer( ep, i );
          PP_ChangeMonsNo( pp, wk->count );
          
          OS_TPrintf("セットしたpokeID=%d\n",wk->count);
          wk->count++;
          if( wk->count >= MONSNO_TAMAGO )
          {
            break;
          }
        }
      }

      // HP/PP無限 
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_HP_CONST );
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_PP_CONST );
      BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_SKIP_BTLIN ); // 登場アニメ飛ばす
      
//      new_event = EVENT_WildPokeBattle( gsys, fieldmap, bp, FALSE, ENC_TYPE_NORMAL );
      new_event = EVENT_BSubwayTrainerBattle( gsys, fieldmap, bp );
    
      GMEVENT_CallEvent( event, new_event );
    }
      *seq = SEQ_RET;
      break;

    case SEQ_RET:

      if( wk->count < MONSNO_TAMAGO-1 )
      {
        *seq = SEQ_SETUP;
      }
      else
      {
        *seq = SEQ_EXIT;
      }
      break;

    case SEQ_EXIT:
      OS_TPrintf("トリプル全ポケモンチェック終了！\n");
      return GMEVENT_RES_FINISH;
    default : GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

#endif // PM_DEBUG
