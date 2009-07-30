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
	VecFx32 player_way;
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
  FIELD_RAIL_MAN * railMan = FIELDMAP_GetFieldRailMan(fieldWork);
	FIELD_RAIL_LOADER* railLoader = FIELDMAP_GetFieldRailLoader(fieldWork);
	NOGRID_MOVE_WORK *work;

	work = GFL_HEAP_AllocClearMemory( FIELDMAP_GetHeapID( fieldWork ), sizeof(NOGRID_MOVE_WORK) );
	FIELDMAP_SetMapCtrlWork( fieldWork, work );

	FIELD_RAIL_LOADER_Load( railLoader, NARC_field_rail_data_h01_dat, FIELDMAP_GetHeapID(fieldWork) );
  FIELD_RAIL_MAN_Load(railMan, FIELD_RAIL_LOADER_GetData(railLoader));
  FIELD_RAIL_MAN_SetLocation( railMan, &locationStart );
  //FIELD_RAIL_MAN_Load(railMan, &point_c03_start);
  FIELD_RAIL_MAN_GetPos(railMan, pos );
//  FIELD_CAMERA_BindNoCamera(camera, TRUE);
//
	
	{
		MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
		MMDL_BLACTCONT_SetGlobalScaleOne( mmdlsys );

	}

	{
		FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl(fieldWork);
		VecFx32 shadow_scale = { FX32_CONST(0.570f), FX32_CONST(0.570f), FX32_CONST(0.570f) };
		FLDEFF_SHADOW_SetGlobalScale( fectrl, &shadow_scale );
	}
	
  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

	FIELD_PLAYER_SetPos( fld_player, pos );
	FIELD_PLAYER_SetDir( fld_player, dir );

  FIELD_CAMERA_SetTargetPos( camera, pos );

  FIELD_PLAYER_NOGRID_Rail_SetUp( fld_player, FIELD_RAIL_MAN_GetActionWay( railMan ), &work->player_way );
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
	FIELD_RAIL_LOADER* railLoader = FIELDMAP_GetFieldRailLoader(fieldWork);
	NOGRID_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );

	FIELD_RAIL_LOADER_Clear( railLoader );
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
  FIELD_RAIL_MAN * railMan = FIELDMAP_GetFieldRailMan(fieldWork);
  BOOL rail_flag = FIELD_RAIL_MAN_GetActiveFlag(railMan);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	NOGRID_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );

  //デバッグのため、レール処理をON/OFF
  if (key_trg & PAD_BUTTON_L)
  {
    FIELD_RAIL_MAN_SetActiveFlag(railMan, !rail_flag);
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
		int rail_action_key;

    FIELD_RAIL_MAN_GetPos(railMan, pos );
    FIELD_PLAYER_SetPos( fld_player, pos );
    PLAYERWORK_setPosition( player, pos );

		rail_action_key = FIELD_RAIL_MAN_GetActionKey( railMan );
		FIELD_PLAYER_NOGRID_Rail_Move( fld_player, FIELDMAP_GetFldEffCtrl(fieldWork), &work->player_way, rail_action_key, FIELDMAP_GetFieldCamera(fieldWork) );
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
