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
static const RAIL_POINT point_c03_start;
static const RAIL_POINT point_d02_loop_01;
static const RAIL_POINT point_d02_start_00;

static const RAIL_CAMERA_SET camera_point_c03_start_00;
static const RAIL_CAMERA_SET camera_point_c03_start_01;
static const RAIL_CAMERA_SET camera_point_slope;
static const RAIL_CAMERA_SET camera_changeAngle;
static const RAIL_CAMERA_SET camera_point_loop_up;
static const RAIL_CAMERA_SET camera_point_d02_start_00;
static const RAIL_CAMERA_SET camera_point_d02_start_01;
static const RAIL_CAMERA_SET camera_point_d02_start_02;
static const RAIL_CAMERA_SET camera_point_d02_loop_01;
static const RAIL_CAMERA_SET camera_point_d02_loop_02;
static const RAIL_CAMERA_SET camera_point_d02_loop_03;

static const RAIL_LINEPOS_SET LoopLinePosSet;

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
  FIELD_RAIL_MAN_Load(railMan, &point_d02_start_00);
  //FIELD_RAIL_MAN_Load(railMan, &point_c03_start);
  FIELD_RAIL_MAN_GetPos(railMan, pos );
  FIELD_CAMERA_BindNoCamera(FIELDMAP_GetFieldCamera(fieldWork), TRUE);

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
    if (!rail_flag)
    {
      FIELD_CAMERA_BindNoCamera(FIELDMAP_GetFieldCamera(fieldWork), TRUE);
  //    FIELD_CAMERA_SetCameraType(FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_TYPE_H01);
    }
    else
    {
      FIELD_CAMERA_BindNoCamera(FIELDMAP_GetFieldCamera(fieldWork), FALSE);
   //   FIELD_CAMERA_SetCameraType(FIELDMAP_GetFieldCamera(fieldWork), FIELD_CAMERA_TYPE_NOUSE);
    }
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

static const RAIL_LINE line_d02_bridge_start;

static const RAIL_LINE line_d02_start_00;
static const RAIL_LINE line_d02_start_01;
static const RAIL_LINE line_d02_loop_01;
static const RAIL_LINE line_d02_loop_02;
static const RAIL_LINE line_d02_loop_03;
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
static const RAIL_POINT point_c03_start =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_slope_start, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002bfdac, 0x00008000, 0x007b2c2
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_c03_start_00,
  //const char * name;
  "point_c03_start",
};

//--------------------------------------------------------------
/**
 * 次のPOINT（上り途中）
 */
//--------------------------------------------------------------
static const RAIL_POINT point_c03_start_01 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_slope_up, &line_slope_start, NULL, NULL,
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
  &camera_point_c03_start_01,
  //const char * name;
  "point_c03_start_01",
};

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static const RAIL_POINT point_bridge1 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_bridge_start, &line_slope_up, NULL, NULL,
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
    &line_bridge_long, &line_bridge_start, NULL, NULL,
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
  "point_bridge2",
};

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static const RAIL_POINT point_bridge_loop_top =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_bridge_start, &line_bridge_long, NULL, NULL, 
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002c03f4, 0x000f4951, 0x008122c4,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_loop_03,
  //&camera_point_loop_up,
  //const char * name;
  "point_bridge_loop_top",
};



//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_loop_03 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_loop_03, &line_d02_bridge_start, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x002bf7fa, 0x000cf96f, 0x008d2807,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_loop_03,
  //const char * name;
  "point_d02_loop_03",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_loop_02 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_loop_02, &line_d02_loop_03, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x0032b422, 0x000b106b, 0x0093ec33,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_loop_02,
  //const char * name;
  "point_d02_loop_02",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_loop_01 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_loop_01, &line_d02_loop_02, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x00392cb0, 0x00085f20, 0x0089a7e7,
    //0x003a4c22, 0x0008f5cb, 0x008cc224,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_loop_01,
  //const char * name;
  "point_d02_loop_01",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_start_02 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_start_01, &line_d02_loop_01, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x0031ffcc, 0x0006f167, 0x0085f618,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_start_02,
  //const char * name;
  "point_d02_start_02",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_start_01 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_start_00, &line_d02_start_01, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x0029bf6c, 0x00063f53, 0x0085f618,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_start_01,
  //const char * name;
  "point_d02_start_01",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_POINT point_d02_start_00 =
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &line_d02_start_00, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x0022f76c, 0x00008000, 0x0085f618,
  },
  //const RAIL_CAMERA_SET * camera;
  &camera_point_d02_start_00,
  //const char * name;
  "point_d02_start_02",
};


//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_loop_03 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_d02_loop_02, &point_d02_loop_03,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &LoopLinePosSet, //&RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_loop_03",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_loop_02 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_d02_loop_01, &point_d02_loop_02,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &LoopLinePosSet, //&RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_loop_02",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_loop_01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_d02_start_02, &point_d02_loop_01,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &LoopLinePosSet, //&RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_loop_01",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_start_01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_d02_start_01, &point_d02_start_02,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_start_01",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_start_00 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
   &point_d02_start_00, &point_d02_start_01,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_start_00",
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
  &point_bridge1, &point_c03_start_01,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_slope_up",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_slope_start =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_c03_start_01, &point_c03_start,
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
  &point_bridge2, &point_bridge1,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_bridge_start",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_bridge_long =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_bridge_loop_top, &point_bridge2,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  200,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_bridge_long",
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_LINE line_d02_bridge_start =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &point_d02_loop_03, &point_bridge_loop_top,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &RAIL_LINEPOS_SET_Default,
  //u32 line_divider;
  50,
  //const RAIL_CAMERA_SET * camera;
  &camera_changeAngle,
  //const char * name;
  "line_d02_bridge_start",
};

//======================================================================
//
//    RAIL_CAMERA_SET定義
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 立体交差ループの移動定義
 */
//--------------------------------------------------------------
static const RAIL_LINEPOS_SET LoopLinePosSet =
{
  FIELD_RAIL_POSFUNC_CurveLine,
  0x00329c22,
  0x0008f5cb,
  0x008cc224,
  0,
};
static const VecFx32 loopCenter =
{
  0x00329c22,
  0x0008f5cb,
  0x008cc224,
};

//--------------------------------------------------------------
/**
 * @brief LINE用カメラ設定：両端POINTの線形補完
 */
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_changeAngle =
{
  FIELD_RAIL_CAMERAFUNC_OfsAngleCamera,
  0,    //no parameter
  0,
  0,
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_c03_start_00 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x31a8, //angle:pitch = angle h
  0x0000, //angle:yaw = angle v
  0x0090 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_c03_start_01 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x31a8, //angle:pitch = angle h
  0x0000, //angle:yaw = angle v
  0x00f8 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_slope =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x31a8, //angle:pitch = angle h
  0x0000, //angle:yaw = angle v
  0x0280 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_loop_up =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x31a8, //angle:pitch = angle h
  0x0000, //angle:yaw = angle v
  0x0280 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_start_00 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x1e00, //angle:pitch = angle h
  0xc000, //angle:yaw = angle v
  0x0058 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_start_01 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x1200, //angle:pitch = angle h
  0xc000, //angle:yaw = angle v
  0x0090 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_start_02 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x1200, //angle:pitch = angle h
  0xc000, //angle:yaw = angle v
  0x0090 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_loop_01 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x0800, //angle:pitch = angle h
  0x8600, //angle:yaw = angle v
  0x00f8 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_loop_02 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x0800, //angle:pitch = angle h
  0x3c00, //angle:yaw = angle v
  0x00f8 * FX32_ONE,  //angle:len
  0,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static const RAIL_CAMERA_SET camera_point_d02_loop_03 =
{
  FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
  0x0800, //angle:pitch = angle h
  0x0000, //angle:yaw = angle v
  0x00f8 * FX32_ONE,  //angle:len
  0,
};





