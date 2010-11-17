//====================================================================
/*
 *  @file pair_sys.c
 *  @brief  連れ歩きシステム関連関数（常駐）
 *  @author Miyuki Iwasawa
 *  @date   10.11.16
 */
//====================================================================

#include <gflib.h>

#include "system/main.h"  // for HEAPID_xxxx
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"

#include "pair_sys.h"

#include "tr_tool/trno_def.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"

//---------------------------------------------------
/*
 *  @brief  連れ歩きフラグチェック
 */
//---------------------------------------------------
BOOL PAIRSYS_CheckPairFlag( GAMEDATA* gdata )
{
  return EVENTWORK_CheckEventFlag( GAMEDATA_GetEventWork(gdata), SYS_FLAG_PAIR );
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット
 */
//---------------------------------------------------
void PAIRSYS_PairFree( GAMEDATA* gdata )
{
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);

  //フラグ&ワークリセット
  EVENTWORK_ResetEventFlag( ev_wk, SYS_FLAG_PAIR );
  {
    u16* id_wk = EVENTWORK_GetEventWorkAdrs( ev_wk, WK_SYS_PAIR_ID );
    *id_wk = PAIR_ID_NONE;
  }
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きペアID取得
 *
 *  @retval 0～ 連れ歩きペアID (定義はfield/pair_sys_def.h)
 *  @retval PAID_ID_NONE(0xFFFF)  連れ歩き無し
 */
//---------------------------------------------------
u16 PAIRSYS_GetPairID( GAMEDATA* gdata )
{
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);

  if( EVENTWORK_CheckEventFlag( ev_wk, SYS_FLAG_PAIR ) ){
    u16* id_wk = EVENTWORK_GetEventWorkAdrs( ev_wk, WK_SYS_PAIR_ID );
    return *id_wk;
  }
  return PAIR_ID_NONE;
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア トレーナーID取得
 *
 *  @retval 1～ 連れ歩きペアのトレーナーID (定義はtr_tool/trno_def.h)
 *  @retval TRID_NULL  連れ歩きトレーナーデータ無し
 */
//---------------------------------------------------
u16 PAIRSYS_GetPairTrainerID( GAMEDATA* gdata )
{
  u16 pair_id = PAIRSYS_GetPairID( gdata );

  if( pair_id == PAIR_ID_NONE){
    return TRID_NULL;
  }
  return TRID_RIVAL_07;
}

