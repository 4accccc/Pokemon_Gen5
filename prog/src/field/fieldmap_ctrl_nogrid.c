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


enum
{
  NOGRID_RAIL_point_d02_start_00,
  NOGRID_RAIL_point_c03_start,
  NOGRID_RAIL_point_c03_start_01,
  NOGRID_RAIL_point_bridge1,
  NOGRID_RAIL_point_bridge2,
  NOGRID_RAIL_point_bridge_loop_top,
  NOGRID_RAIL_point_d02_loop_03,
  NOGRID_RAIL_point_d02_loop_02,
  NOGRID_RAIL_point_d02_loop_01,
  NOGRID_RAIL_point_d02_start_02,
  NOGRID_RAIL_point_d02_start_01,

  NOGRID_RAIL_point_MAX,
};

enum
{
  NOGRID_RAIL_line_d02_loop_03,
  NOGRID_RAIL_line_d02_loop_02,
  NOGRID_RAIL_line_d02_loop_01,
  NOGRID_RAIL_line_d02_start_01,
  NOGRID_RAIL_line_d02_start_00,
  NOGRID_RAIL_line_slope_up,
  NOGRID_RAIL_line_slope_start,
  NOGRID_RAIL_line_bridge_start,
  NOGRID_RAIL_line_bridge_long,
  NOGRID_RAIL_line_d02_bridge_start,

	NOGRID_RAIL_line_MAX,
};

enum
{
	NOGRID_RAIL_camera_point_c03_start_00,
	NOGRID_RAIL_camera_point_c03_start_01,
	NOGRID_RAIL_camera_point_slope,
	NOGRID_RAIL_camera_changeAngle,
	NOGRID_RAIL_camera_point_loop_up,
	NOGRID_RAIL_camera_point_d02_start_00,
	NOGRID_RAIL_camera_point_d02_start_01,
	NOGRID_RAIL_camera_point_d02_start_02,
	NOGRID_RAIL_camera_point_d02_loop_01,
	NOGRID_RAIL_camera_point_d02_loop_02,
	NOGRID_RAIL_camera_point_d02_loop_03,

	NOGRID_RAIL_camera_MAX,
};

enum
{
	NOGRID_RAIL_LoopLinePosSet,
	NOGRID_RAIL_linepos_MAX,
};

enum
{
	NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
	NOGRID_RAIL_CAMERAFUNC_OFSANGLE,

	NOGRID_RAIL_CAMERAFUNC_MAX,
};

enum
{
	NOGRID_RAIL_LINEPOSFUNC_CURVE,

	NOGRID_RAIL_LINEPOSFUNC_MAX,
};

enum
{
	NOGRID_RAIL_LINEDISTFUNC_CURVE,

	NOGRID_RAIL_LINEDISTFUNC_MAX,
};


// レール情報
static const RAIL_POINT pointTable[NOGRID_RAIL_point_MAX] = {
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_start_00, RAIL_TBL_NULL, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x0022f76c, 0x00008000, 0x00867618,
			////0x0022f76c, 0x00008000, 0x0085f618,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_start_00,
		//const char * name;
		"point_d02_start_00",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_slope_start, RAIL_TBL_NULL, RAIL_TBL_NULL, RAIL_TBL_NULL,
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
		NOGRID_RAIL_camera_point_c03_start_00,
		//const char * name;
		"point_c03_start",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_slope_up, NOGRID_RAIL_line_slope_start, RAIL_TBL_NULL, RAIL_TBL_NULL,
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
		NOGRID_RAIL_camera_point_c03_start_01,
		//const char * name;
		"point_c03_start_01",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_bridge_start, NOGRID_RAIL_line_slope_up, RAIL_TBL_NULL, RAIL_TBL_NULL,
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
		NOGRID_RAIL_camera_point_slope,
		//const char * name;
		"point_bridge1",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_bridge_long, NOGRID_RAIL_line_bridge_start, RAIL_TBL_NULL, RAIL_TBL_NULL,
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
		NOGRID_RAIL_camera_point_slope,
		//const char * name;
		"point_bridge2",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_bridge_start, NOGRID_RAIL_line_bridge_long, RAIL_TBL_NULL, RAIL_TBL_NULL, 
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
		NOGRID_RAIL_camera_point_d02_loop_03,
		//NOGRID_RAIL_camera_point_loop_up,
		//const char * name;
		"point_bridge_loop_top",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_loop_03, NOGRID_RAIL_line_d02_bridge_start, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x002bf7fa, 0x000cf974, 0x008d2807,
			////0x002bf7fa, 0x000cf96f, 0x008d2807,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_loop_03,
		//const char * name;
		"point_d02_loop_03",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_loop_02, NOGRID_RAIL_line_d02_loop_03, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x00324057, 0x000b106b, 0x009454c3,
			////0x0032b422, 0x000b106b, 0x0093ec33,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_loop_02,
		//const char * name;
		"point_d02_loop_02",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_loop_01, NOGRID_RAIL_line_d02_loop_02, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x003940e7, 0x00085f20, 0x008a2b83,
			////0x00392cb0, 0x00085f20, 0x0089a7e7,
			//0x003a4c22, 0x0008f5cb, 0x008cc224,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_loop_01,
		//const char * name;
		"point_d02_loop_01",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_start_01, NOGRID_RAIL_line_d02_loop_01, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x003285e2, 0x0006f167, 0x0086a9af,
			////0x0031ffcc, 0x0006f167, 0x0085f618,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_start_02,
		//const char * name;
		"point_d02_start_02",
	},
	{
		//const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
		{
			NOGRID_RAIL_line_d02_start_00, NOGRID_RAIL_line_d02_start_01, RAIL_TBL_NULL, RAIL_TBL_NULL,
		},
		//RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
		{
			RAIL_KEY_DOWN, RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL,
		},
		//VecFx32 pos;
		{
			0x002a2c90, 0x00063f70, 0x00867618,
			////0x0029bf6c, 0x00063f53, 0x0085f618,
		},
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_point_d02_start_01,
		//const char * name;
		"point_d02_start_01",
	},
};
static const RAIL_LINE lineTable[NOGRID_RAIL_line_MAX] = {
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		 NOGRID_RAIL_point_d02_loop_02, NOGRID_RAIL_point_d02_loop_03,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		NOGRID_RAIL_LoopLinePosSet, //NOGRID_RAIL_RAIL_LINEPOS_SET_Default,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_loop_03",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		 NOGRID_RAIL_point_d02_loop_01, NOGRID_RAIL_point_d02_loop_02,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		NOGRID_RAIL_LoopLinePosSet, //NOGRID_RAIL_RAIL_LINEPOS_SET_Default,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_loop_02",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		 NOGRID_RAIL_point_d02_start_02, NOGRID_RAIL_point_d02_loop_01,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		NOGRID_RAIL_LoopLinePosSet, //NOGRID_RAIL_RAIL_LINEPOS_SET_Default,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_loop_01",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		 NOGRID_RAIL_point_d02_start_01, NOGRID_RAIL_point_d02_start_02,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_start_01",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		 NOGRID_RAIL_point_d02_start_00, NOGRID_RAIL_point_d02_start_01,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_start_00",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		NOGRID_RAIL_point_bridge1, NOGRID_RAIL_point_c03_start_01,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_slope_up",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		NOGRID_RAIL_point_c03_start_01, NOGRID_RAIL_point_c03_start,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_slope_start",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		NOGRID_RAIL_point_bridge2, NOGRID_RAIL_point_bridge1,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_bridge_start",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		NOGRID_RAIL_point_bridge_loop_top, NOGRID_RAIL_point_bridge2,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_bridge_long",
	},
	{
		//const RAIL_POINT * point_s
		//const RAIL_POINT * point_e
		NOGRID_RAIL_point_d02_loop_03, NOGRID_RAIL_point_bridge_loop_top,
		//RAIL_KEY key;
		RAIL_KEY_UP,
		//const RAIL_LINEPOS_SET * line_pos_set;
		RAIL_TBL_NULL,
		//const RAIL_CAMERA_SET * camera;
		NOGRID_RAIL_camera_changeAngle,
		//const char * name;
		"line_d02_bridge_start",
	},
};
static const RAIL_CAMERA_SET cameraTbl[NOGRID_RAIL_camera_MAX] = {
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x31a8, //angle:pitch = angle h
		0x0000, //angle:yaw = angle v
		0x0090 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x31a8, //angle:pitch = angle h
		0x0000, //angle:yaw = angle v
		0x00f8 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x31a8, //angle:pitch = angle h
		0x0000, //angle:yaw = angle v
		0x0280 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_OFSANGLE,
		0,    //no parameter
		0,
		0,
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x31a8, //angle:pitch = angle h
		0x0000, //angle:yaw = angle v
		0x0280 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x1e00, //angle:pitch = angle h
		0xc000, //angle:yaw = angle v
		0x0058 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x1200, //angle:pitch = angle h
		0xc000, //angle:yaw = angle v
		0x0090 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x1200, //angle:pitch = angle h
		0xc000, //angle:yaw = angle v
		0x0090 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x0800, //angle:pitch = angle h
		0x8600, //angle:yaw = angle v
		0x00f8 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x0800, //angle:pitch = angle h
		0x3c00, //angle:yaw = angle v
		0x00f8 * FX32_ONE,  //angle:len
		0,
	},
	{
		NOGRID_RAIL_CAMERAFUNC_FIXANGLE,
		0x0800, //angle:pitch = angle h
		0x0000, //angle:yaw = angle v
		0x00f8 * FX32_ONE,  //angle:len
		0,
	},
};
static RAIL_CAMERA_FUNC* camera_func[NOGRID_RAIL_CAMERAFUNC_MAX] = 
{
	FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
	FIELD_RAIL_CAMERAFUNC_OfsAngleCamera,
};
static const RAIL_LINEPOS_SET lineposTbl[NOGRID_RAIL_linepos_MAX] = 
{
	{
		NOGRID_RAIL_LINEPOSFUNC_CURVE,
		NOGRID_RAIL_LINEDISTFUNC_CURVE,
		0x00329c22,
		0x0008f5cb,
		0x008cc224,
		0,
	},
};
static RAIL_POS_FUNC* linepos_func[NOGRID_RAIL_LINEPOSFUNC_MAX] = 
{
	FIELD_RAIL_POSFUNC_CurveLine,
};
static RAIL_LINE_DIST_FUNC* linedist_func[NOGRID_RAIL_LINEDISTFUNC_MAX] = 
{
	FIELD_RAIL_LINE_DIST_FUNC_CircleLine,
};

// 使用されてないが、linepos_setの定義の下に書かれていたので、
// とっておきます。
static const VecFx32 loopCenter =
{
  0x00329c22,
  0x0008f5cb,
  0x008cc224,
};

static const RAIL_SETTING lineWidth = {
  NOGRID_RAIL_point_MAX,
  NOGRID_RAIL_line_MAX,
  NOGRID_RAIL_camera_MAX,
  NOGRID_RAIL_linepos_MAX,
	NOGRID_RAIL_CAMERAFUNC_MAX,
	NOGRID_RAIL_LINEPOSFUNC_MAX,
	NOGRID_RAIL_LINEDISTFUNC_MAX,
	0,
  pointTable,
  lineTable,
  cameraTbl,
  lineposTbl,
	camera_func,
	linepos_func,
	linedist_func,
  4,
  4*FX32_ONE
};
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

  FIELD_RAIL_MAN_Load(railMan, &lineWidth);
  FIELD_RAIL_MAN_SetLocation( railMan, &locationStart );
  //FIELD_RAIL_MAN_Load(railMan, &point_c03_start);
  FIELD_RAIL_MAN_GetPos(railMan, pos );
  FIELD_CAMERA_BindNoCamera(camera, TRUE);

  fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
  //2009.07.05 ROMのための一時的な補正処理
  pos->y += FX32_ONE * 4;
	FIELD_PLAYER_SetPos( fld_player, pos );
	FIELD_PLAYER_SetDir( fld_player, dir );

  FIELD_CAMERA_SetTargetPos( camera, pos );
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
	int key_cont = GFL_UI_KEY_GetCont();
  int key_trg = GFL_UI_KEY_GetTrg();
  FIELD_RAIL_MAN * railMan = FIELDMAP_GetFieldRailMan(fieldWork);
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
    FIELD_RAIL_MAN_GetPos(railMan, pos );
    //2009.07.05 ROMのための一時的な補正処理
    pos->y += FX32_ONE * 4;
    FIELD_PLAYER_SetPos( fld_player, pos );
    FIELD_CAMERA_SetTargetPos( FIELDMAP_GetFieldCamera(fieldWork), pos );
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
//======================================================================
//
//    POINT定義
//
//======================================================================
