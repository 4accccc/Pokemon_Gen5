///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセンPC関連イベント
 * @file   event_pokecen_pc.c
 * @author obata
 * @date   2009.12.07
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "event_pokecen_pc.h"
#include "gamesystem/game_event.h"
#include "sound/pm_sndsys.h"
#include "field_player.h"
#include "field_buildmodel.h"
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "fieldmap.h"


//=========================================================================================
// ■イベントワーク
//=========================================================================================
typedef struct
{
  GAMESYS_WORK*  gsys;
  FIELDMAP_WORK* fieldmap;
  G3DMAPOBJST*   pcStatus;
  int            count;

} EVENT_WORK;


//=========================================================================================
// ■PC起動イベント
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief PC起動イベント処理
 */
//-----------------------------------------------------------------------------------------
static GMEVENT_RESULT PcOnEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // アニメ, SE再生
  case 0:
    PMSND_PlaySE( SEQ_SE_PC_ON );
    {
      FIELD_PLAYER* player;
      VecFx32 pos;
      G3DMAPOBJST** objst;
      FLDHIT_RECT rect;
      u32 objnum;
      FLDMAPPER* mapper;
      FIELD_BMODEL_MAN* man;
      // 検索範囲を決定
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      FIELD_PLAYER_GetPos( player, &pos );
      rect.top    = pos.z - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.bottom = pos.z + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.left   = pos.x - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.right  = pos.x + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      // PCを検索
      mapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
      man    = FLDMAPPER_GetBuildModelManager( mapper );
      objst  = FIELD_BMODEL_MAN_CreateObjStatusList( man, &rect, BM_SEARCH_ID_PC, &objnum );
      // アニメ再生
      if( objst )
      { 
        work->pcStatus = objst[0];
        G3DMAPOBJST_setAnime( man, objst[0], 0, BMANM_REQ_START );
        GFL_HEAP_FreeMemory( objst );
      }
    }
    ++(*seq);
    break;
  // アニメ, SE待ち
  case 1:
    {
      SEPLAYER_ID player_id;
      player_id = PMSND_GetSE_DefaultPlayerID( SEQ_SE_PC_ON );
      if( PMSND_CheckPlaySE_byPlayerID(player_id) == FALSE ){ ++(*seq); }
    }
    break;
  // 待ち
  case 2:
    work->count = 0;
    ++(*seq);
    break;
  case 3:
    {
      const int wait = 10;
      if( wait < work->count++ ){ ++(*seq); }
    }
    break;
  // PC起動中アニメ再生
  case 4:
    PMSND_PlaySE( SEQ_SE_PC_LOGIN );
    if( work->pcStatus )
    {
      FLDMAPPER* mapper;
      FIELD_BMODEL_MAN* man;
      mapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
      man    = FLDMAPPER_GetBuildModelManager( mapper );
      G3DMAPOBJST_setAnime( man, work->pcStatus, 1, BMANM_REQ_LOOP );
    }
    ++(*seq);
    break;
  case 5:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC起動イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
GMEVENT* EVENT_PokecenPcOn( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, parent, PcOnEvent, sizeof(EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  work->pcStatus = NULL;
  return event;
} 


//=========================================================================================
// ■PC起動中イベント
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief PC起動中イベント処理
 */
//-----------------------------------------------------------------------------------------
static GMEVENT_RESULT PcRunEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  case 0:
    // PC起動中アニメ再生
    {
      FIELD_PLAYER* player;
      VecFx32 pos;
      G3DMAPOBJST** objst;
      FLDHIT_RECT rect;
      u32 objnum;
      FLDMAPPER* mapper;
      FIELD_BMODEL_MAN* man;
      // 検索範囲を決定
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      FIELD_PLAYER_GetPos( player, &pos );
      rect.top    = pos.z - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.bottom = pos.z + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.left   = pos.x - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.right  = pos.x + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      // PCを検索
      mapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
      man    = FLDMAPPER_GetBuildModelManager( mapper );
      objst  = FIELD_BMODEL_MAN_CreateObjStatusList( man, &rect, BM_SEARCH_ID_PC, &objnum );
      // アニメ再生
      if( objst )
      { 
        work->pcStatus = objst[0];
        G3DMAPOBJST_setAnime( man, objst[0], 1, BMANM_REQ_LOOP );
        GFL_HEAP_FreeMemory( objst );
      }
    }
    ++(*seq);
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC起動中イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
GMEVENT* EVENT_PokecenPcRun( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, parent, PcRunEvent, sizeof(EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  work->pcStatus = NULL;
  return event;
} 


//=========================================================================================
// ■PC停止イベント
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief PC停止イベント処理
 */
//-----------------------------------------------------------------------------------------
static GMEVENT_RESULT PcOffEvent( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // アニメ, SE再生
  case 0:
    PMSND_PlaySE( SEQ_SE_PC_LOGOFF );
    {
      FIELD_PLAYER* player;
      VecFx32 pos;
      G3DMAPOBJST** objst;
      FLDHIT_RECT rect;
      u32 objnum;
      FLDMAPPER* mapper;
      FIELD_BMODEL_MAN* man;
      // 検索範囲を決定
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      FIELD_PLAYER_GetPos( player, &pos );
      rect.top    = pos.z - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.bottom = pos.z + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.left   = pos.x - (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      rect.right  = pos.x + (FIELD_CONST_GRID_SIZE << FX32_SHIFT);
      // PCを検索
      mapper = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
      man    = FLDMAPPER_GetBuildModelManager( mapper );
      objst  = FIELD_BMODEL_MAN_CreateObjStatusList( man, &rect, BM_SEARCH_ID_PC, &objnum );
      // アニメ再生
      if( objst )
      { 
        work->pcStatus = objst[0];
        G3DMAPOBJST_setAnime( man, objst[0], 2, BMANM_REQ_START );
        GFL_HEAP_FreeMemory( objst );
      }
    }
    ++(*seq);
    break;
  // アニメ, SE待ち
  case 1:
    {
      SEPLAYER_ID player_id;
      player_id = PMSND_GetSE_DefaultPlayerID( SEQ_SE_PC_LOGOFF );
      if( PMSND_CheckPlaySE_byPlayerID(player_id) == FALSE ){ ++(*seq); }
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC停止イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
GMEVENT* EVENT_PokecenPcOff( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, parent, PcOffEvent, sizeof(EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  work->pcStatus = NULL;
  return event;
} 
