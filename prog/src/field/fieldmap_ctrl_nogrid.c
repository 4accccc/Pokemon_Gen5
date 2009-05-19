//======================================================================
/**
 * @file	fieldmap_ctrl_nogrid.c
 * @brief	フィールドマップ　コントロール　ノングリッド処理
 * @author	tamada
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_player_nogrid.h"

#include "fieldmap_ctrl_nogrid.h"

#include "field_rail.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//ポインタ
static FIELD_RAIL_MAN * railMan;

static const RAIL_POINT point_bridge1;
static const RAIL_POINT point_start;
static const RAIL_CAMERA_SET camera_point_start;
static const RAIL_CAMERA_SET camera_point_slope;
static const RAIL_CAMERA_SET camera_changeAngle;
static const RAIL_CAMERA_SET camera_point_loop_up;

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

  railMan = FIELD_RAIL_MAN_Create( FIELDMAP_GetHeapID(fieldWork), camera );
  FIELD_RAIL_MAN_Load(railMan, &point_start);
  FIELD_RAIL_MAN_GetPos(railMan, pos );

  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	FIELD_PLAYER_SetPos( fld_player, pos );
	FIELD_PLAYER_SetDir( fld_player, dir );

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
  FIELD_RAIL_MAN_Delete(railMan);
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
	int key_cont = FIELDMAP_GetKeyCont( fieldWork );
  int key_trg = FIELDMAP_GetKeyTrg(fieldWork);
  BOOL rail_flag = FIELD_RAIL_MAN_GetActiveFlag(railMan);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );

  //デバッグのため、レール処理をON/OFF
  if (key_trg & PAD_BUTTON_L)
  {
    FIELD_RAIL_MAN_SetActiveFlag(railMan, !rail_flag);
  }

	FIELD_PLAYER_NOGRID_Move( fld_player, key_cont );
	FIELD_PLAYER_GetPos( fld_player, pos );

  if (rail_flag)
  {
    FIELD_RAIL_MAN_Update(railMan, FIELDMAP_GetKeyCont(fieldWork) );
    FIELD_RAIL_MAN_GetPos(railMan, pos );
    FIELD_PLAYER_SetPos( fld_player, pos );
    FIELD_RAIL_MAN_UpdateCamera(railMan);
  }
  {
    GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    PLAYERWORK_setPosition( player, pos );
	}
}


//======================================================================
//======================================================================
static const RAIL_LINE line_slope_up;
static const RAIL_LINE line_slope_start;
static const RAIL_LINE line_bridge_start;
static const RAIL_LINE line_bridge_long;

//======================================================================
//
//    POINT定義
//
//======================================================================
//--------------------------------------------------------------
/**
 * 最初のPOINT
 */
//--------------------------------------------------------------
static const RAIL_POINT point_start =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_slope_start, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002bfdac, 0x00055578, 0x00ea45c
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_start,
  //const char * name;
  "point_start",
};

//--------------------------------------------------------------
/**
 * 次のPOINT（上り途中）
 */
//--------------------------------------------------------------
static const RAIL_POINT point_slope =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_slope_start, &line_slope_up, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002beb9c, 0x000c691c, 0x0016e5b6
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_slope,
  //const char * name;
  "point_slope",
};

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static const RAIL_POINT point_bridge1 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_slope_up, &line_bridge_start, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002bfcfa, 0x000f8000, 0x0021f666
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_slope,
  //const char * name;
  "point_bridge1",
};

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static const RAIL_POINT point_bridge2 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_bridge_start, &line_bridge_long, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002c2ae2, 0x000f8000, 0x0325c46
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_slope,
  //const char * name;
  "point_start",
};

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static const RAIL_POINT point_bridge_loop_top =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_bridge_long, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002c03f4, 0x000f4951, 0x008122c4,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_loop_up,
  //const char * name;
  "point_start",
};


//======================================================================
//
//      LINE定義
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_slope_up =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_slope, &point_bridge1,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  NULL,
  //const char * name;
  "line_slope_up",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_slope_start =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_start, &point_slope,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_slope_start",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_bridge_start =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_bridge1, &point_bridge2,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  NULL,
  //const char * name;
  "line_bridge_start",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_bridge_long =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_bridge2, &point_bridge_loop_top,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  100,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_bridge_long",
};

//======================================================================
//
//    RAIL_CAMERA_SET定義
//
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_start =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x0fa8, //angle:pitch = angle h
  0x8000, //angle:yaw = angle v
  0x009b * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_slope =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x0fa8, //angle:pitch = angle h
  0x8000, //angle:yaw = angle v
  0x0120 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_loop_up =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x31a8, //angle:pitch = angle h
  0x8000, //angle:yaw = angle v
  0x0280 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_changeAngle =
{
  FIELD_RAIL_CAMERAFUNC_OfsAngleCamera,
  0,    //no parameter
  0,
  0,
  0,
};




