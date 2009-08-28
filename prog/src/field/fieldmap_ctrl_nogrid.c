//======================================================================
/**
 * @file	fieldmap_ctrl_nogrid.c
 * @brief	フィールドマップ　コントロール　ノングリッド処理
 * @author	tamada
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "arc/fieldmap/field_rail_data.naix"

#include "fieldmap.h"
#include "field_player_nogrid.h"
#include "fldeff_shadow.h"
#include "field/zonedata.h"

#include "gamesystem/game_data.h"  //PLAYER_WORK

#include "fieldmap_ctrl_nogrid.h"

#include "field_rail.h"
#include "field_rail_func.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//--------------------------------------------------------------
///	NOGRID_MOVE_WORK
//--------------------------------------------------------------
typedef struct {
  FIELD_PLAYER_NOGRID_WORK player_work;
}NOGRID_MOVE_WORK;

static const RAIL_LOCATION locationStart = {
  FIELD_RAIL_TYPE_POINT,
  0,
};

//======================================================================
//	proto
//======================================================================
static void mapCtrlNoGrid_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlNoGrid_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlNoGrid_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );

//======================================================================
//	フィールドマップ　ノングリッド処理
//======================================================================
//--------------------------------------------------------------
///	マップコントロール　グリッド移動
//--------------------------------------------------------------
const DEPEND_FUNCTIONS FieldMapCtrl_NoGridFunctions =
{
	FLDMAP_CTRLTYPE_NOGRID,
	mapCtrlNoGrid_Create,
	mapCtrlNoGrid_Main,
	mapCtrlNoGrid_Delete,
};

//--------------------------------------------------------------
/**
 * フィールドマップ　ノングリッド処理　初期化
 * @param	fieldWork	FIELD_MAIN_WORK
 * @param	pos	自機初期位置
 * @param	dir 自機初期方向
 */
//--------------------------------------------------------------
static void mapCtrlNoGrid_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir )
{
	FIELD_PLAYER *fld_player;
  FIELD_CAMERA * camera = FIELDMAP_GetFieldCamera(fieldWork);
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
	NOGRID_MOVE_WORK *work;

	work = GFL_HEAP_AllocClearMemory( FIELDMAP_GetHeapID( fieldWork ), sizeof(NOGRID_MOVE_WORK) );
	FIELDMAP_SetMapCtrlWork( fieldWork, work );

  {
    FLDNOGRID_RESISTDATA resist;

    resist.railDataID = ZONEDATA_GetRailDataID( FIELDMAP_GetZoneID(fieldWork) );
    resist.areaDataID = FLDNOGRID_RESISTDATA_NONE;
    resist.attrDataID = NARC_field_rail_data_h01_atdat; // 橋意外だと壊れる

    FLDNOGRID_MAPPER_ResistData( p_mapper, &resist, FIELDMAP_GetHeapID(fieldWork) );  
  }

  work->player_work.railwork = FLDNOGRID_MAPPER_CreateRailWork(p_mapper);  // 移動管理１オブジェ生成
  
  // ロケーションの設定
  FIELD_RAIL_WORK_SetLocation( work->player_work.railwork, &locationStart );
  FIELD_RAIL_WORK_GetPos(work->player_work.railwork, pos );
  FLDNOGRID_MAPPER_BindCameraWork( p_mapper, work->player_work.railwork );
//
	
  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

	FIELD_PLAYER_SetPos( fld_player, pos );
	FIELD_PLAYER_SetDir( fld_player, dir );

  FIELD_CAMERA_SetTargetPos( camera, pos );

  FIELD_PLAYER_NOGRID_Rail_SetUp( fld_player, &work->player_work );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　ノングリッド処理　削除
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void mapCtrlNoGrid_Delete( FIELDMAP_WORK *fieldWork )
{
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
	NOGRID_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );

	FLDNOGRID_MAPPER_Release( p_mapper );
  FLDNOGRID_MAPPER_DeleteRailWork( p_mapper, work->player_work.railwork );
  FLDNOGRID_MAPPER_UnBindCameraWork( p_mapper );
	//DeletePlayerAct( fieldWork->field_player );
	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　ノングリッド処理　メイン
 * @param fieldWork FIELDMAP_WORK
 * @param pos
 * @retval nothing
 */
//--------------------------------------------------------------
static void mapCtrlNoGrid_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos )
{
	int key_cont = GFL_UI_KEY_GetCont();
  int key_trg = GFL_UI_KEY_GetTrg();
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  BOOL rail_flag = FLDNOGRID_MAPPER_DEBUG_IsActive(p_mapper);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	NOGRID_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );

  //デバッグのため、レール処理をON/OFF
  if (key_trg & PAD_BUTTON_L)
  {
    FIELD_RAIL_WORK_SetActiveFlag(work->player_work.railwork, !rail_flag);
    if (!rail_flag)
    {
			FIELD_CAMERA_SetMode( FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_MODE_DIRECT_POS );
//      FIELD_CAMERA_BindNoCamera(FIELDMAP_GetFieldCamera(fieldWork), TRUE);
  //    FIELD_CAMERA_SetCameraType(FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_TYPE_H01);
    }
    else
    {
			FIELD_CAMERA_SetMode( FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_MODE_CALC_CAMERA_POS );
//      FIELD_CAMERA_BindNoCamera(FIELDMAP_GetFieldCamera(fieldWork), FALSE);
   //   FIELD_CAMERA_SetCameraType(FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_TYPE_NOUSE);
    }
  }


  if (rail_flag)
  {
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork(
        GAMESYSTEM_GetGameData(FIELDMAP_GetGameSysWork(fieldWork)) );

		FIELD_PLAYER_NOGRID_Rail_Move( fld_player, FIELDMAP_GetFldEffCtrl(fieldWork), FIELDMAP_GetFieldCamera(fieldWork), GFL_UI_KEY_GetCont(), &work->player_work, FIELDMAP_GetFldNoGridMapper( fieldWork ) );

    FIELD_PLAYER_GetPos( fld_player, pos );
    PLAYERWORK_setPosition( player, pos );

  }
	else
  {
    GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );

		FIELD_PLAYER_NOGRID_Free_Move( fld_player, key_cont, FX32_ONE );
		
		FIELD_PLAYER_GetPos( fld_player, pos );
    PLAYERWORK_setPosition( player, pos );
	}
}


//======================================================================
//======================================================================
//======================================================================
//
//    POINT定義
//
//======================================================================
