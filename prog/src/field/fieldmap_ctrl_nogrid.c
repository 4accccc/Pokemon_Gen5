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
#include "arc/fieldmap/zone_id.h"

#include "fieldmap.h"
#include "field_player_nogrid.h"
#include "fldeff_shadow.h"
#include "field/zonedata.h"

#include "gamesystem/game_data.h"  //PLAYER_WORK

#include "fieldmap_ctrl_nogrid.h"

#include "fieldmap_ctrl_nogrid_work.h"

#include "field_rail.h"
#include "field_rail_func.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

static const RAIL_LOCATION locationStart = {
  FIELD_RAIL_TYPE_POINT,
  0,
};


//======================================================================
//	ダミーNPC
//======================================================================
#define DEBUG_TEST_NPC

#ifdef DEBUG_TEST_NPC
static const MMDL_HEADER dummyNpc = 
{
  128,
  BOY1,
  MV_RAIL_DMY,
  EV_TYPE_TRAINER, 0, 0,
  DIR_DOWN,
  0,0,0,
  0,0,
  0,0,
  0
};
static const RAIL_LOCATION sc_initLocation[] = 
{
  {
    13,
    FIELD_RAIL_TYPE_LINE,
    RAIL_KEY_DOWN,
    4,
    3,
  },
  {
    6,
    FIELD_RAIL_TYPE_LINE,
    RAIL_KEY_DOWN,
    2,
    15,
  },
  {
    6,
    FIELD_RAIL_TYPE_LINE,
    RAIL_KEY_DOWN,
    -2,
    15,
  },
};
static MMDL* s_DUMMY_MDL[NELEMS(sc_initLocation)] = {NULL};
#endif // DEBUG_TEST_NPC

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
	FIELDMAP_CTRL_NOGRID_WORK *work;
  FIELD_PLAYER_NOGRID* p_ngrid_player;

  {
    FLDNOGRID_RESISTDATA resist;

    resist.railDataID = ZONEDATA_GetRailDataID( FIELDMAP_GetZoneID(fieldWork) );
    resist.areaDataID = FLDNOGRID_RESISTDATA_NONE;

    FLDNOGRID_MAPPER_ResistData( p_mapper, &resist, FIELDMAP_GetHeapID(fieldWork) );  
  }

  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

	work = FIELDMAP_CTRL_NOGRID_WORK_Create( fieldWork, FIELDMAP_GetHeapID( fieldWork ) );
	FIELDMAP_SetMapCtrlWork( fieldWork, work );

  p_ngrid_player = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( work );


  // ロケーションの設定
  FIELD_PLAYER_NOGRID_SetLocation( p_ngrid_player, &locationStart );
  FIELD_PLAYER_NOGRID_GetPos( p_ngrid_player, pos );
//
	

	FIELD_PLAYER_SetPos( fld_player, pos );
	FIELD_PLAYER_SetDir( fld_player, dir );


#ifdef DEBUG_TEST_NPC
  {
    MMDLSYS* fos = FIELDMAP_GetMMdlSys( fieldWork );
    int i;
    
    for( i=0; i<NELEMS(sc_initLocation); i++ )
    {
      s_DUMMY_MDL[i] = MMDLSYS_AddMMdl(
      	fos, &dummyNpc, ZONE_ID_H01 );
      MMDL_OnStatusBit( s_DUMMY_MDL[i], MMDL_STABIT_RAIL_MOVE );
      MMDL_SetRailLocation( s_DUMMY_MDL[i], &sc_initLocation[i] );
    }
  }
#endif
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
	FIELDMAP_CTRL_NOGRID_WORK* work = FIELDMAP_GetMapCtrlWork( fieldWork );

#ifdef DEBUG_TEST_NPC
  {
    int i;

    for( i=0; i<NELEMS(sc_initLocation); i++ )
    {
//      MMDL_Delete( s_DUMMY_MDL[i] );
//      s_DUMMY_MDL[i] = NULL;
    }
  }
#endif

  FIELDMAP_CTRL_NOGRID_WORK_Delete( work );
  
	FLDNOGRID_MAPPER_Release( p_mapper );
  FLDNOGRID_MAPPER_UnBindCameraWork( p_mapper );
	//DeletePlayerAct( fieldWork->field_player );

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
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	FIELDMAP_CTRL_NOGRID_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );
  FIELD_PLAYER_NOGRID* p_ngrid_player = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( work );

  PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork(
      GAMESYSTEM_GetGameData(FIELDMAP_GetGameSysWork(fieldWork)) );
  MMDL* mmdl = FIELD_PLAYER_GetMMdl( fld_player );

  FIELDMAP_CTRL_NOGRID_WORK_Main( work );

  MMDL_GetVectorPos( mmdl, pos );
  FIELD_PLAYER_SetPos( fld_player, pos );
  PLAYERWORK_setPosition( player, pos );
}


//======================================================================
//======================================================================
//======================================================================
//
//    POINT定義
//
//======================================================================
