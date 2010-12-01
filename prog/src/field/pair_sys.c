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

#include "field/fieldmap.h"
#include "field/script.h"

#include "pair_sys.h"

#include "tr_tool/trno_def.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"


//========================================
//プロトタイプ
//========================================
static void pairsys_PairParamClear( PAIR_SYSTEM_WORK* wk );

//---------------------------------------------------
/*
 *  @brief  連れ歩きシステムワーク確保
 */
//---------------------------------------------------
PAIR_SYSTEM_WORK* PAIRSYS_AllocWork( HEAPID heapID )
{
  PAIR_SYSTEM_WORK* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof( PAIR_SYSTEM_WORK ));
  wk->heapID = heapID;
  pairsys_PairParamClear( wk );

  return wk;
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きシステムワークフリー
 */
//---------------------------------------------------
void PAIRSYS_FreeWork( PAIR_SYSTEM_WORK* wk )
{
  MI_CpuClear8( wk, sizeof( PAIR_SYSTEM_WORK ));
  GFL_HEAP_FreeMemory( wk );
}


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
 *  @brief  連れ歩きペア トレーナーセット
 *
 *  @param  gdata
 *  @param  pair_id   連れ歩きペア識別ID( PAIR_ID_01～ pair_sys_def.h)
 *  @param  pair_type 連れ歩きペアタイプID( PAIR_TYPE_CLOSE～ pair_sys_def.h)
 *  @param  obj_id    連れ歩きペアのOBJID
 *  @param  obj_code  連れ歩きペアのOBJコード
 *  @param  event_id  連れ歩きペアのイベントID(関連付けないならSCRID_NULL)
 *  @retval tr_id 連れ歩きペアのトレーナーID トレーナーデータ無しならTRID_NULL(定義はtr_tool/trno_def.h)
 */
//---------------------------------------------------
BOOL PAIRSYS_PairSet( GAMEDATA* gdata, u8 pair_id, u8 pair_type, u16 obj_id, u16 obj_code, u16 event_id, u16 tr_id )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);

  if( PAIRSYS_CheckPairFlag( gdata )){
    GF_ASSERT_MSG(0,"Pair already exists\n");
    return FALSE;
  }
  if( pair_id >= PAIR_ID_MAX ){
    GF_ASSERT_MSG(0,"id=%d max=%d\n",pair_id, PAIR_ID_MAX);
    return FALSE;
  }
  pairsys_PairParamClear( wk );

  //パラメータセット
  wk->pair_id = pair_id;
  wk->pair_type = pair_type;
  wk->obj_id = obj_id;
  wk->obj_code = obj_code;
  wk->event_id = event_id;
  wk->tr_id = tr_id;

  //スクリプトフラグ&ワークセット
  EVENTWORK_SetEventFlag( ev_wk, SYS_FLAG_PAIR );
  {
    u16* id_wk = EVENTWORK_GetEventWorkAdrs( ev_wk, WK_SYS_PAIR_ID );
    *id_wk = pair_id;
  }
  return TRUE;
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット
 */
//---------------------------------------------------
void PAIRSYS_PairFree( GAMEDATA* gdata )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);

  if( !PAIRSYS_CheckPairFlag( gdata )){
    GF_ASSERT_MSG(0,"Pair none\n");
    return;
  }
  
  //フラグ&ワークリセット
  EVENTWORK_ResetEventFlag( ev_wk, SYS_FLAG_PAIR );
  {
    u16* id_wk = EVENTWORK_GetEventWorkAdrs( ev_wk, WK_SYS_PAIR_ID );
    *id_wk = PAIR_ID_NONE;
  }
  pairsys_PairParamClear( wk );
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きリセット マップチェンジ
 *
 *  @com  連れ歩きがマップチェンジOKタイプでないならフリー
 */
//---------------------------------------------------
void PAIRSYS_PairFreeForMapChange( GAMEDATA* gdata )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);

  if( wk->pair_type == PAIR_TYPE_OPEN ){
    return;
  }
  //フラグ&ワークリセット
  EVENTWORK_ResetEventFlag( ev_wk, SYS_FLAG_PAIR );
  {
    u16* id_wk = EVENTWORK_GetEventWorkAdrs( ev_wk, WK_SYS_PAIR_ID );
    *id_wk = PAIR_ID_NONE;
  }
  pairsys_PairParamClear( wk );
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア マップチェンジ後復帰
 *  
 *  @com  PAIR_SYS_WORKにセット済みのパラメータを使って初期化する
 *  @param  gdata
 */
//---------------------------------------------------
void PAIRSYS_PairMapChangeRecover( GAMEDATA* gdata )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);
	MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys( gdata );
  MMDL* mmdl;

  if( !EVENTWORK_CheckEventFlag( ev_wk, SYS_FLAG_PAIR ) ){  //連れ歩いていない
    return; 
  }
  if( wk->pair_type != PAIR_TYPE_OPEN ){
    PAIRSYS_PairFree( gdata );
    return;
  }
  {
    MMDL* player = MMDLSYS_SearchMMdlPlayer( mmdlsys );
    int gx = MMDL_GetGridPosX( player );
    int gz = MMDL_GetGridPosZ( player );
    int dir = MMDL_GetDirDisp( player );
    int zone = MMDL_GetZoneID( player );
    mmdl = MMDLSYS_AddMMdlParam( mmdlsys, gx, gz, dir, MMDL_ID_PAIR, wk->obj_code, MV_PAIR, zone );
    MMDL_SetEventID( mmdl, wk->event_id );
    MMDL_SetStatusBitNotZoneDelete( mmdl, TRUE );
    MMDL_SetStatusBitVanish( mmdl, TRUE );  //いったん消しておく
  }
}

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア マップチェンジ後表示復帰
 *  
 *  @param  gdata
 */
//---------------------------------------------------
void PAIRSYS_PairViewRecover( GAMEDATA* gdata )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );
  EVENTWORK* ev_wk = GAMEDATA_GetEventWork(gdata);
	MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys( gdata );
  MMDL* pair;

  if( !EVENTWORK_CheckEventFlag( ev_wk, SYS_FLAG_PAIR ) ){  //連れ歩いていない
    return; 
  }
  pair = MMDLSYS_SearchOBJID( mmdlsys, MMDL_ID_PAIR );
  if( pair == NULL ){
    GF_ASSERT(0);
    return;
  }
  if( !MMDL_CheckStatusBitVanish( pair )){
    return;
  }
  {
    MMDL_GRIDPOS pos;
    MMDL* player = MMDLSYS_SearchMMdlPlayer( mmdlsys );
    MMDL_GetOldGridPos( player, &pos );
    MMDL_InitGridPosition( pair, pos.gx, pos.gy, pos.gz, MMDL_GetDirDisp( player ) );

    MMDL_SetStatusBitVanish( pair, FALSE );
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

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア オリジナルOBJID取得
 *
 *  @retval 連れ歩きトレーナーの元のOBJID
 */
//---------------------------------------------------
u16 PAIRSYS_GetPairTrainerOriginalObjID( GAMEDATA* gdata )
{
  PAIR_SYSTEM_WORK* wk = GAMEDATA_GetPairSystemWork( gdata );

  return wk->obj_id;
}

//=========================================================
//サブルーチン
//=========================================================

//---------------------------------------------------
/*
 *  @brief  連れ歩きペア　パラメータクリア
 *
 *  @retval 1～ 連れ歩きペアのトレーナーID (定義はtr_tool/trno_def.h)
 *  @retval TRID_NULL  連れ歩きトレーナーデータ無し
 */
//---------------------------------------------------
static void pairsys_PairParamClear( PAIR_SYSTEM_WORK* wk )
{
  wk->pair_id = PAIR_ID_NONE;
  wk->pair_type = 0;
  wk->obj_id = 0;
  wk->obj_code = 0;
  wk->event_id = SCRID_NULL;
  wk->tr_id = TRID_NULL;
}

