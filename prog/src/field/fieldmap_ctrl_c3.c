//======================================================================
/**
 * @file	fieldmap_ctrl_c3.c
 * @brief	フィールドマップ　コントロール　C3マップ処理
 * @author	tamada
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_player_nogrid.h"

#include "field_easytp.h"

#include "fieldmap_ctrl_c3.h"

#include "field_rail.h"


//======================================================================
//	rail情報
//======================================================================
//ポインタ
static FIELD_RAIL_MAN * sp_railMan;

// 出発と終端
static const RAIL_POINT sc_POINT_C3_START;
static const RAIL_POINT sc_POINT_C3_END;

// 分岐地点
static const RAIL_POINT sc_POINT_C3_JOINT00;
static const RAIL_POINT sc_POINT_C3_JOINT01;
static const RAIL_POINT sc_POINT_C3_JOINT02;
static const RAIL_POINT sc_POINT_C3_JOINT03;
static const RAIL_POINT sc_POINT_C3_JOINT04;

// 路地
static const RAIL_POINT sc_POINT_C3_ALLAY00;
static const RAIL_POINT sc_POINT_C3_ALLAY01;
static const RAIL_POINT sc_POINT_C3_ALLAY02;
static const RAIL_POINT sc_POINT_C3_ALLAY03;
static const RAIL_POINT sc_POINT_C3_ALLAY04;

// ベイエリア
static const RAIL_POINT sc_POINT_C3_BEACH00;
static const RAIL_POINT sc_POINT_C3_BEACH01;
static const RAIL_POINT sc_POINT_C3_BEACH02;
static const RAIL_POINT sc_POINT_C3_BEACH03;
static const RAIL_POINT sc_POINT_C3_BEACH04;

// 路地カメラズーム用
static const RAIL_POINT sc_POINT_C3_CA_ALLAY00;
static const RAIL_POINT sc_POINT_C3_CA_ALLAY01;
static const RAIL_POINT sc_POINT_C3_CA_ALLAY02;
static const RAIL_POINT sc_POINT_C3_CA_ALLAY03;
static const RAIL_POINT sc_POINT_C3_CA_ALLAY04;

// ライン
static const RAIL_LINE sc_LINE_C3_STARTtoJOINT00;
static const RAIL_LINE sc_LINE_C3_JOINT00toEND;
static const RAIL_LINE sc_LINE_C3_JOINT00toCA_ALLAY00;
static const RAIL_LINE sc_LINE_C3_JOINT00toBEACH00;
static const RAIL_LINE sc_LINE_C3_JOINT00toJOINT01;
static const RAIL_LINE sc_LINE_C3_JOINT01toCA_ALLAY01;
static const RAIL_LINE sc_LINE_C3_JOINT01toBEACH01;
static const RAIL_LINE sc_LINE_C3_JOINT01toJOINT02;
static const RAIL_LINE sc_LINE_C3_JOINT02toCA_ALLAY02;
static const RAIL_LINE sc_LINE_C3_JOINT02toBEACH02;
static const RAIL_LINE sc_LINE_C3_JOINT02toJOINT03;
static const RAIL_LINE sc_LINE_C3_JOINT03toCA_ALLAY03;
static const RAIL_LINE sc_LINE_C3_JOINT03toBEACH03;
static const RAIL_LINE sc_LINE_C3_JOINT03toJOINT04;
static const RAIL_LINE sc_LINE_C3_JOINT04toCA_ALLAY04;
static const RAIL_LINE sc_LINE_C3_JOINT04toBEACH04;
static const RAIL_LINE sc_LINE_C3_CA_ALLAY00toALLAY00;
static const RAIL_LINE sc_LINE_C3_CA_ALLAY01toALLAY01;
static const RAIL_LINE sc_LINE_C3_CA_ALLAY02toALLAY02;
static const RAIL_LINE sc_LINE_C3_CA_ALLAY03toALLAY03;
static const RAIL_LINE sc_LINE_C3_CA_ALLAY04toALLAY04;


// カメラ
static const RAIL_CAMERA_SET sc_CAMERA_C3_NORMAL;
static const RAIL_CAMERA_SET sc_CAMERA_C3_PICKUP;

// 位置
static const RAIL_LINEPOS_SET sc_LINEPOS_C3_NORMAL;


// 基本分割数
#define RAIL_LINE_DIV_NORMAL    (256)
#define RAIL_LINE_DIV_FIRST     (26)
#define RAIL_LINE_DIV_WALKIN    (16)
#define RAIL_LINE_DIV_PICKUP    (2)



//======================================================================
//	define
//======================================================================
#define HEIGHT	0x2000

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	C3_MOVE_WORK
//--------------------------------------------------------------
typedef struct {
	u16 player_len;
	u16 pos_angle;
	s16 df_len;
	s16 df_angle;
}C3_MOVE_WORK;




//======================================================================
//	proto
//======================================================================
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );

static void CalcPos( VecFx32 *pos, const VecFx32 *center, u16 len, u16 dir );

//======================================================================
//	フィールドマップ　コントロール　C3
//======================================================================
//--------------------------------------------------------------
///	マップコントロール　グリッド移動
//--------------------------------------------------------------
const DEPEND_FUNCTIONS FieldMapCtrl_C3Functions =
{
	FLDMAP_CTRLTYPE_NOGRID,
	mapCtrlC3_Create,
	mapCtrlC3_Main,
	mapCtrlC3_Delete,
};

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　初期化
 * @param	fieldWork	FIELDMAP_WORK
 * @param	pos	自機初期位置
 * @param	dir 自機初期方向
 */
//--------------------------------------------------------------
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir )
{
	HEAPID heapID;
	C3_MOVE_WORK *work;
	FIELD_PLAYER *fld_player;
  FIELD_CAMERA * camera = FIELDMAP_GetFieldCamera(fieldWork);
	static const C3_MOVE_WORK init = {
		 0x1f0,
		 0,
		 1,
		 16
	};
	
	heapID = FIELDMAP_GetHeapID( fieldWork );
	work = GFL_HEAP_AllocClearMemory( heapID, sizeof(C3_MOVE_WORK) );
	*work = init;
	FIELDMAP_SetMapCtrlWork( fieldWork, work );

  // レール起動
  sp_railMan = FIELD_RAIL_MAN_Create( FIELDMAP_GetHeapID(fieldWork), camera );
  FIELD_RAIL_MAN_Load(sp_railMan, &sc_POINT_C3_START);
  FIELD_RAIL_MAN_GetPos(sp_railMan, pos );
	
	{	//ビルボード設定
		VecFx32 scale = {
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
			FX32_ONE+(FX32_ONE/2)+(FX32_ONE/4),
		};
		GFL_BBDACT_SYS *bbdActSys = FIELDMAP_GetBbdActSys( fieldWork );
		GFL_BBD_SetScale( GFL_BBDACT_GetBBDSystem(bbdActSys), &scale );
	}
	fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
	FIELD_PLAYER_SetPos( fld_player, pos);
	FIELD_PLAYER_SetDir( fld_player, dir );
}

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　削除
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork )
{
	C3_MOVE_WORK *work = FIELDMAP_GetMapCtrlWork( fieldWork );
	GFL_HEAP_FreeMemory( work );

  FIELD_RAIL_MAN_Delete(sp_railMan);
}

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　メイン
 * @param	fieldWork	FIELDMAP_WORK
 * @param	pos
 * @retval nothing
 */
//--------------------------------------------------------------
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos )
{
	int key_cont = FIELDMAP_GetKeyCont( fieldWork );
  int key_trg = FIELDMAP_GetKeyTrg(fieldWork);
	C3_MOVE_WORK *mwk = FIELDMAP_GetMapCtrlWork( fieldWork );
  BOOL rail_flag = FIELD_RAIL_MAN_GetActiveFlag(sp_railMan);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );;

  //デバッグのため、レール処理をON/OFF
  if (key_trg & PAD_BUTTON_L)
  {
    FIELD_RAIL_MAN_SetActiveFlag(sp_railMan, !rail_flag);
  }

  if (rail_flag)
  {
    FIELD_RAIL_MAN_Update(sp_railMan, FIELDMAP_GetKeyCont(fieldWork) );
    FIELD_RAIL_MAN_GetPos(sp_railMan, pos );
    pos->y = HEIGHT;
    FIELD_PLAYER_SetPos( fld_player, pos );
    FIELD_RAIL_MAN_UpdateCamera(sp_railMan);

		FIELD_PLAYER_C3_Move( fld_player, key_cont, 0 );
    return;
  }
	
	{
		s16 df_angle;
		s16 df_len;

		if( key_cont & PAD_BUTTON_B ){
			df_angle	= mwk->df_angle*4;
			df_len		= mwk->df_len*4;
		}else{
			df_angle	= mwk->df_angle;
			df_len		= mwk->df_len;
		}
		
		if (key_cont & PAD_KEY_LEFT) {
			mwk->pos_angle -= df_angle;
		}
		if (key_cont & PAD_KEY_RIGHT) {
			mwk->pos_angle += df_angle;
		}
		if (key_cont & PAD_KEY_UP) {
			mwk->player_len -= df_len;
		}
		if (key_cont & PAD_KEY_DOWN) {
			mwk->player_len += df_len;
		}
	}

	{
		VecFx32 cam, player_pos;
		FIELD_CAMERA *camera_control;
		camera_control = FIELDMAP_GetFieldCamera( fieldWork );
		FIELD_CAMERA_GetTargetPos( camera_control, &cam);
		CalcPos( &player_pos, &cam, mwk->player_len, mwk->pos_angle );
		//SetPlayerActTrans( fieldWork->field_player, &player_pos );
		FIELD_CAMERA_SetAngleYaw( camera_control, mwk->pos_angle );
		FIELD_PLAYER_SetPos( fld_player, &player_pos);
		FIELD_PLAYER_C3_Move( fld_player, key_cont, mwk->pos_angle );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      OS_TPrintf( "player pos x=0x%x y=0x%x z=0x%x\n", player_pos.x, player_pos.y, player_pos.z );
      OS_TPrintf( "0x%x, 0x%x, 0x%x\n", player_pos.x, player_pos.y, player_pos.z );
    }
	}
}

//======================================================================
//	C3　サブ
//======================================================================
//--------------------------------------------------------------
/**
 * 座標計算
 * @param pos 座標
 * @param center 中心位置
 * @param len 距離
 * @param dir 方向
 * @retval nothing
 */
//--------------------------------------------------------------
static void CalcPos(VecFx32 * pos, const VecFx32 * center, u16 len, u16 dir)
{
	pos->x = center->x + len * FX_SinIdx(dir);
	pos->y = center->y + HEIGHT;
	pos->z = center->z + len * FX_CosIdx(dir);
}










//======================================================================
//	RAIL情報
//======================================================================
// 出発と終端
static const RAIL_POINT sc_POINT_C3_START = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_STARTtoJOINT00, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_LEFT, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x461fa2, 0x6000, 0x20189e
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_START",
};

static const RAIL_POINT sc_POINT_C3_END = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT00toEND, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_LEFT, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x461fa2, 0x6000, 0x20189e
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_END",
};

// 分岐地点
static const RAIL_POINT sc_POINT_C3_JOINT00 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT00toEND, &sc_LINE_C3_JOINT00toJOINT01, &sc_LINE_C3_JOINT00toCA_ALLAY00, &sc_LINE_C3_JOINT00toBEACH00,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_RIGHT, RAIL_KEY_LEFT, RAIL_KEY_UP, RAIL_KEY_DOWN,
  },
  //VecFx32 pos;
  {
    0x476e5e, 0x6000, 0x222466
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_JOINT00",
};
static const RAIL_POINT sc_POINT_C3_JOINT01 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT00toJOINT01, &sc_LINE_C3_JOINT01toJOINT02, &sc_LINE_C3_JOINT01toCA_ALLAY01, &sc_LINE_C3_JOINT01toBEACH01,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_RIGHT, RAIL_KEY_LEFT, RAIL_KEY_UP, RAIL_KEY_DOWN,
  },
  //VecFx32 pos;
  {
    0x47e292, 0x6000, 0x3d31c2
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_JOINT01",
};
static const RAIL_POINT sc_POINT_C3_JOINT02 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT01toJOINT02, &sc_LINE_C3_JOINT02toJOINT03, &sc_LINE_C3_JOINT02toCA_ALLAY02, &sc_LINE_C3_JOINT02toBEACH02,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_RIGHT, RAIL_KEY_LEFT, RAIL_KEY_UP, RAIL_KEY_DOWN,
  },
  //VecFx32 pos;
  {
    0x303d9a, 0x6000, 0x4bd08a
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_JOINT02",
};
static const RAIL_POINT sc_POINT_C3_JOINT03 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT02toJOINT03, &sc_LINE_C3_JOINT03toJOINT04, &sc_LINE_C3_JOINT03toCA_ALLAY03, &sc_LINE_C3_JOINT03toBEACH03,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_RIGHT, RAIL_KEY_LEFT, RAIL_KEY_UP, RAIL_KEY_DOWN,
  },
  //VecFx32 pos;
  {
    0x18cc86, 0x6000, 0x3d5d06
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_JOINT03",
};
static const RAIL_POINT sc_POINT_C3_JOINT04 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT03toJOINT04, &sc_LINE_C3_JOINT04toCA_ALLAY04, &sc_LINE_C3_JOINT04toBEACH04, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_RIGHT, RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, 
  },
  //VecFx32 pos;
  {
    0x196733, 0x6000, 0x22dee4
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_JOINT04",
};

// 路地
static const RAIL_POINT sc_POINT_C3_ALLAY00 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY00toALLAY00, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x3ecd6e, 0x6000, 0x27b672
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_ALLAY00",
};
static const RAIL_POINT sc_POINT_C3_ALLAY01 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY01toALLAY01, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x3ed0b6, 0x6000, 0x386c1a
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_ALLAY01",
};
static const RAIL_POINT sc_POINT_C3_ALLAY02 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY02toALLAY02, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x2ff15e, 0x6000, 0x4191d2
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_ALLAY02",
};
static const RAIL_POINT sc_POINT_C3_ALLAY03 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY03toALLAY03, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x205b5e, 0x6000, 0x38f5ea
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_ALLAY03",
};
static const RAIL_POINT sc_POINT_C3_ALLAY04 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY04toALLAY04, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x208fde, 0x6000, 0x26da9a
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_ALLAY04",
};

// ベイエリア
static const RAIL_POINT sc_POINT_C3_BEACH00 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT00toBEACH00, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x51ae62, 0x6000, 0x22beda
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_BEACH00",
};
static const RAIL_POINT sc_POINT_C3_BEACH01 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT01toBEACH01, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x4db4de, 0x6000, 0x44e95a
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_BEACH01",
};
static const RAIL_POINT sc_POINT_C3_BEACH02 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT02toBEACH02, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x300db2, 0x6000, 0x54d1b6
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_BEACH02",
};
static const RAIL_POINT sc_POINT_C3_BEACH03 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT03toBEACH03, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x159152, 0x6000, 0x4a2fea
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_BEACH03",
};
static const RAIL_POINT sc_POINT_C3_BEACH04 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_JOINT04toBEACH04, NULL, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_NULL, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0xc9f34, 0x6000, 0x206329
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_POINT_C3_BEACH04",
};

// 路地カメラズーム用
static const RAIL_POINT sc_POINT_C3_CA_ALLAY00 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY00toALLAY00, &sc_LINE_C3_JOINT00toCA_ALLAY00, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x45b43b, 0x6000, 0x23473f
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_CA_ALLAY00",
};
static const RAIL_POINT sc_POINT_C3_CA_ALLAY01 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY01toALLAY01, &sc_LINE_C3_JOINT01toCA_ALLAY01, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x454772, 0x6000, 0x3c1a3b
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_CA_ALLAY01",
};
static const RAIL_POINT sc_POINT_C3_CA_ALLAY02 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY02toALLAY02, &sc_LINE_C3_JOINT02toCA_ALLAY02, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x3015fb, 0x6000, 0x490273
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_CA_ALLAY02",
};
static const RAIL_POINT sc_POINT_C3_CA_ALLAY03 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY03toALLAY03, &sc_LINE_C3_JOINT03toCA_ALLAY03, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x3001b8, 0x6000, 0x490273
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_CA_ALLAY03",
};
static const RAIL_POINT sc_POINT_C3_CA_ALLAY04 = 
{
  //const RAIL_LINE * lines[RAIL_CONNECT_LINE_MAX];
  {
    &sc_LINE_C3_CA_ALLAY04toALLAY04, &sc_LINE_C3_JOINT04toCA_ALLAY04, NULL, NULL,
  },
  //RAIL_KEY keys[RAIL_CONNECT_LINE_MAX];
  {
    RAIL_KEY_UP, RAIL_KEY_DOWN, RAIL_KEY_NULL, RAIL_KEY_NULL,
  },
  //VecFx32 pos;
  {
    0x19fc49, 0x6000, 0x235a8c
  },
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_POINT_C3_CA_ALLAY04",
};

// ライン
static const RAIL_LINE sc_LINE_C3_STARTtoJOINT00 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_START, &sc_POINT_C3_JOINT00,
  //RAIL_KEY key;
  RAIL_KEY_LEFT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_FIRST,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_STARTtoJOINT00",
};
static const RAIL_LINE sc_LINE_C3_JOINT00toEND =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT00, &sc_POINT_C3_END,
  //RAIL_KEY key;
  RAIL_KEY_RIGHT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_FIRST,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT00toEND",
};
static const RAIL_LINE sc_LINE_C3_JOINT00toCA_ALLAY00 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT00, &sc_POINT_C3_CA_ALLAY00,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_PICKUP,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT00toCA_ALLAY00",
};
static const RAIL_LINE sc_LINE_C3_JOINT00toBEACH00 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT00, &sc_POINT_C3_BEACH00,
  //RAIL_KEY key;
  RAIL_KEY_DOWN,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT00toBEACH00",
};
static const RAIL_LINE sc_LINE_C3_JOINT00toJOINT01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT00, &sc_POINT_C3_JOINT01,
  //RAIL_KEY key;
  RAIL_KEY_LEFT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT00toJOINT01",
};
static const RAIL_LINE sc_LINE_C3_JOINT01toCA_ALLAY01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT01, &sc_POINT_C3_CA_ALLAY01,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_PICKUP,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT01toCA_ALLAY01",
};
static const RAIL_LINE sc_LINE_C3_JOINT01toBEACH01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT01, &sc_POINT_C3_BEACH01,
  //RAIL_KEY key;
  RAIL_KEY_DOWN,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT01toBEACH01",
};
static const RAIL_LINE sc_LINE_C3_JOINT01toJOINT02 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT01, &sc_POINT_C3_JOINT02,
  //RAIL_KEY key;
  RAIL_KEY_LEFT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT01toJOINT02",
};
static const RAIL_LINE sc_LINE_C3_JOINT02toCA_ALLAY02 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT02, &sc_POINT_C3_CA_ALLAY02,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_PICKUP,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT02toCA_ALLAY02",
};
static const RAIL_LINE sc_LINE_C3_JOINT02toBEACH02 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT02, &sc_POINT_C3_BEACH02,
  //RAIL_KEY key;
  RAIL_KEY_DOWN,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT02toBEACH02",
};
static const RAIL_LINE sc_LINE_C3_JOINT02toJOINT03 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT02, &sc_POINT_C3_JOINT03,
  //RAIL_KEY key;
  RAIL_KEY_LEFT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT02toJOINT03",
};
static const RAIL_LINE sc_LINE_C3_JOINT03toCA_ALLAY03 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT03, &sc_POINT_C3_CA_ALLAY03,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_PICKUP,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT03toCA_ALLAY03",
};
static const RAIL_LINE sc_LINE_C3_JOINT03toBEACH03 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT03, &sc_POINT_C3_BEACH03,
  //RAIL_KEY key;
  RAIL_KEY_DOWN,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT03toBEACH03",
};
static const RAIL_LINE sc_LINE_C3_JOINT03toJOINT04 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT03, &sc_POINT_C3_JOINT04,
  //RAIL_KEY key;
  RAIL_KEY_LEFT,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT03toJOINT04",
};
static const RAIL_LINE sc_LINE_C3_JOINT04toCA_ALLAY04 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT04, &sc_POINT_C3_CA_ALLAY04,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_PICKUP,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT04toCA_ALLAY04",
};
static const RAIL_LINE sc_LINE_C3_JOINT04toBEACH04 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_JOINT04, &sc_POINT_C3_BEACH04,
  //RAIL_KEY key;
  RAIL_KEY_DOWN,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_NORMAL,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_NORMAL,
  //const char * name;
  "sc_LINE_C3_JOINT04toBEACH04",
};
static const RAIL_LINE sc_LINE_C3_CA_ALLAY00toALLAY00 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_CA_ALLAY00, &sc_POINT_C3_ALLAY00,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_WALKIN,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_LINE_C3_CA_ALLAY00toALLAY00",
};
static const RAIL_LINE sc_LINE_C3_CA_ALLAY01toALLAY01 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_CA_ALLAY01, &sc_POINT_C3_ALLAY01,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_WALKIN,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_LINE_C3_CA_ALLAY01toALLAY01",
};
static const RAIL_LINE sc_LINE_C3_CA_ALLAY02toALLAY02 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_CA_ALLAY02, &sc_POINT_C3_ALLAY02,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_WALKIN,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_LINE_C3_CA_ALLAY02toALLAY02",
};
static const RAIL_LINE sc_LINE_C3_CA_ALLAY03toALLAY03 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_CA_ALLAY03, &sc_POINT_C3_ALLAY03,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_WALKIN,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_LINE_C3_CA_ALLAY03toALLAY03",
};
static const RAIL_LINE sc_LINE_C3_CA_ALLAY04toALLAY04 =
{
  //const RAIL_POINT * point_s
  //const RAIL_POINT * point_e
  &sc_POINT_C3_CA_ALLAY04, &sc_POINT_C3_ALLAY04,
  //RAIL_KEY key;
  RAIL_KEY_UP,
  //const RAIL_LINEPOS_SET * line_pos_set;
  &sc_LINEPOS_C3_NORMAL,
  //u32 line_divider;
  RAIL_LINE_DIV_WALKIN,
  //const RAIL_CAMERA_SET * camera;
  &sc_CAMERA_C3_PICKUP,
  //const char * name;
  "sc_LINE_C3_CA_ALLAY04toALLAY04",
};


// カメラ
static const RAIL_CAMERA_SET sc_CAMERA_C3_NORMAL = 
{
  FIELD_RAIL_POSFUNC_CircleCamera,
  0x38D000,
  0x600,
};
static const RAIL_CAMERA_SET sc_CAMERA_C3_PICKUP = 
{
  FIELD_RAIL_POSFUNC_CircleCamera,
  0x26D000,
  0x800,
};
// 位置
static const RAIL_LINEPOS_SET sc_LINEPOS_C3_NORMAL = 
{
  FIELD_RAIL_POSFUNC_CircleLine,
};


