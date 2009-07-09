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
#include "field_rail_loader.h"
#include "fld_scenearea.h"


//======================================================================
//	シーンエリア情報
//======================================================================
enum
{
	C3_SCENEAREA_FUNC_AREACHECK = 0,
	C3_SCENEAREA_FUNC_AREACHECK_MAX,

	C3_SCENEAREA_FUNC_UPDATE = 0,
	C3_SCENEAREA_FUNC_INSIDE,
	C3_SCENEAREA_FUNC_OUTSIDE,
	C3_SCENEAREA_FUNC_INSIDEFXCAMERA,
	C3_SCENEAREA_FUNC_OUTSIDEFXCAMERA,
	C3_SCENEAREA_FUNC_UPDATE_MAX,
};

static BOOL C3_SCENEAREA_CheckArea( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void C3_SCENEAREA_Update( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void C3_SCENEAREA_Inside( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void C3_SCENEAREA_Outside( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void C3_SCENEAREA_InsideFxCamera( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );
static void C3_SCENEAREA_OutsideFxCamera( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos );

//-------------------------------------
///	エリアパラメータ構造体
//=====================================
typedef struct {
  u32   rot_start;
  u32   rot_end;
  fx32  dist_min;
  fx32  dist_max;
  u32   pitch;
  fx32  length;

  // 固定カメラに必要なパラメータ
  fx32  target_x;
  fx32  target_y;
  fx32  target_z;
  fx32  camera_x;
  fx32  camera_y;
  fx32  camera_z;
} C3_SCENEAREA_PARAM;

#define SCENE_AREA_DIST_MIN ( 0xfc024 )
#define SCENE_AREA_DIST_MAX ( 0x1a8fea )
//#define SCENE_AREA_DIST_MAX ( 0x2000000 )

static const FLD_SCENEAREA_DATA sc_SCENE[] = 
{
  // ALLAY00
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0x2598, 0x3048, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // pitch len
      0x600, 0x26D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },

  // ALLAY01
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0x4f28, 0x6868, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // pitch len
      0x600, 0x26D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },

  // ALLAY02
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0x7a28, 0x9298, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // pitch len
      0x600, 0x26D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },

  // ALLAY03
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0xa638, 0xaff8, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // pitch len
      0x600, 0x26D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },

  // ALLAY04
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0xcfb8, 0xe148, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // pitch len
      0x600, 0x26D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },

  // ピカチューのビルの固定カメラ
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
//      0xc268, 0xcfb8, SCENE_AREA_DIST_MIN, 0x1a5001,
      0xc268, 0xcfb8, SCENE_AREA_DIST_MIN, SCENE_AREA_DIST_MAX,
      // カメラ
      // dummy, dummy,
      0, 0,
      // target x,y,z
      0x3e2504,0xfff1ee7f,0x10d83a,
      // camera x,y,z
      0xb573b,0x76e53,0x30e81c,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_INSIDEFXCAMERA,
    C3_SCENEAREA_FUNC_INSIDEFXCAMERA,
    C3_SCENEAREA_FUNC_OUTSIDEFXCAMERA,
  },

  // 外周エリア
  {
    { 
      // エリア
      // rot_start, rot_end, dist_min, dist_max
      // (基準を{0,0,-FX32_ONE}としたときの角度)
      0x1ca8, 0xe398, 0x1c8f6b, 0x300000,
      // カメラ
      // pitch len
      0x800, 0x38D000,
    },
    C3_SCENEAREA_FUNC_AREACHECK,
    C3_SCENEAREA_FUNC_UPDATE,
    C3_SCENEAREA_FUNC_INSIDE,
    C3_SCENEAREA_FUNC_OUTSIDE,
  },
};

static FLD_SCENEAREA_CHECK_AREA_FUNC* sp_FLD_SCENEAREA_CHECK_AREA_FUNC[C3_SCENEAREA_FUNC_AREACHECK_MAX] = 
{
	C3_SCENEAREA_CheckArea,
};

static FLD_SCENEAREA_UPDATA_FUNC* sp_FLD_SCENEAREA_UPDATA_FUNC[C3_SCENEAREA_FUNC_UPDATE_MAX] = 
{
	C3_SCENEAREA_Update,
	C3_SCENEAREA_Inside,
	C3_SCENEAREA_Outside,
	C3_SCENEAREA_InsideFxCamera,
	C3_SCENEAREA_OutsideFxCamera,
};

static const FLD_SCENEAREA_FUNC sc_FLD_SCENEAREA_FUNC = 
{
	sp_FLD_SCENEAREA_CHECK_AREA_FUNC,
	sp_FLD_SCENEAREA_UPDATA_FUNC,
	C3_SCENEAREA_FUNC_AREACHECK_MAX,
	C3_SCENEAREA_FUNC_UPDATE_MAX,
};


//======================================================================
//	define
//======================================================================
#define HEIGHT	(0x3000)

#define CAMERA_DEF_PITCH  ( 0x800 )
#define CAMERA_DEF_LEN    ( 0x38D000 )
#define CAMERA_DEF_TARGET_X ( 0x2f6f36 )
#define CAMERA_DEF_TARGET_Y ( 0x4000 )
#define CAMERA_DEF_TARGET_Z ( 0x301402 )

// C3カメラ可動範囲
static const VecFx32 sc_RAIL_CAMERA_MOVE_AREA_START = { 0x464cf6, 0, 0x202210 };
static const VecFx32 sc_RAIL_CAMERA_MOVE_AREA_END = { 0x177042, 0, 0x21e54a };



// レール位置情報
static const RAIL_LOCATION sc_RAIL_START_LOCATION = {
  FIELD_RAIL_TYPE_POINT,
  0,
};


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
  fx32  camera_len;
  u16   camera_pitch;

  FLD_SCENEAREA* p_sceneArea;

}C3_MOVE_WORK;




//======================================================================
//	proto
//======================================================================
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );

static void CalcPos( VecFx32 *pos, const VecFx32 *center, u16 len, u16 dir );

static void mapCtrlC3_CameraMain( FIELDMAP_WORK *fieldWork, const VecFx32* cp_pos, u16 pitch, fx32 len );
static void cameraMain( FIELD_CAMERA* p_camera, const VecFx32* cp_pos, u16 pitch, fx32 len );

static void cameraRailAreaControl( FIELD_CAMERA * p_camera );


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
  FIELD_RAIL_MAN* railMan = FIELDMAP_GetFieldRailMan(fieldWork);
	FIELD_RAIL_LOADER* p_rail_loader = FIELDMAP_GetFieldRailLoader(fieldWork);
	static const C3_MOVE_WORK init = {
		 0x1f0,
		 0,
		 1,
		 16,
     CAMERA_DEF_LEN,
     CAMERA_DEF_PITCH,
	};
	
	heapID = FIELDMAP_GetHeapID( fieldWork );
	work = GFL_HEAP_AllocClearMemory( heapID, sizeof(C3_MOVE_WORK) );
	*work = init;
	FIELDMAP_SetMapCtrlWork( fieldWork, work );

  // レール起動
	FIELD_RAIL_LOADER_Load( p_rail_loader, 0, FIELDMAP_GetHeapID(fieldWork) );
  FIELD_RAIL_MAN_Load(railMan, FIELD_RAIL_LOADER_GetData(p_rail_loader));
  FIELD_RAIL_MAN_SetLocation( railMan, &sc_RAIL_START_LOCATION );
  FIELD_RAIL_MAN_GetPos(railMan, pos );
  //2009.07.05 ROMのための一時的な補正処理
  pos->y += FX32_ONE * 8;

  // シーンエリア
  work->p_sceneArea = FLD_SCENEAREA_Create( FIELDMAP_GetHeapID(fieldWork), camera );
  FLD_SCENEAREA_Load( work->p_sceneArea, sc_SCENE, NELEMS(sc_SCENE), &sc_FLD_SCENEAREA_FUNC );
	
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
	FIELD_RAIL_LOADER* p_rail_loader = FIELDMAP_GetFieldRailLoader(fieldWork);
  FLD_SCENEAREA_Delete( work->p_sceneArea );

	FIELD_RAIL_LOADER_Clear( p_rail_loader );

	GFL_HEAP_FreeMemory( work );

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
	int key_cont = GFL_UI_KEY_GetCont(  );
  int key_trg = GFL_UI_KEY_GetTrg();
	C3_MOVE_WORK *mwk = FIELDMAP_GetMapCtrlWork( fieldWork );
  FIELD_RAIL_MAN* railMan = FIELDMAP_GetFieldRailMan(fieldWork);
  BOOL rail_flag = FIELD_RAIL_MAN_GetActiveFlag(railMan);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );;

  //デバッグのため、レール処理をON/OFF
  if (key_trg & PAD_BUTTON_L)
  {
    FIELD_RAIL_MAN_SetActiveFlag(railMan, !rail_flag);
  }

  if (rail_flag)
  {
    // レール動作
    FIELD_RAIL_MAN_GetPos(railMan, pos );
    pos->y = HEIGHT;
    //2009.07.05 ROMのための一時的な補正処理
    pos->y += FX32_ONE * 8;
    FIELD_PLAYER_SetPos( fld_player, pos );
    
    // シーンエリア処理でカメラ上書き
    if( FLD_SCENEAREA_Update( mwk->p_sceneArea, pos ) == FLD_SCENEAREA_UPDATE_NONE ){
      // カメラ動作限界管理
      cameraRailAreaControl( FIELDMAP_GetFieldCamera( fieldWork ) );
    }

		FIELD_PLAYER_C3_Move( fld_player, key_cont, 0 );
  }	
  else
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
		
    if( (key_cont & PAD_BUTTON_R) ){
      if (key_cont & PAD_KEY_LEFT) {
        mwk->camera_len -= FX32_ONE*2;
      }
      if (key_cont & PAD_KEY_RIGHT) {
        mwk->camera_len += FX32_ONE*2;
      }

      if (key_cont & PAD_KEY_UP) {
        mwk->camera_pitch += 0x200;
      }
      if (key_cont & PAD_KEY_DOWN) {
        mwk->camera_pitch -= 0x200;
      }
    }else{
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
      // 基準プレイヤー距離
      if (key_cont & PAD_BUTTON_SELECT) {
        mwk->player_len = 0x01B8;
      }
    }


    {
      VecFx32 cam, player_pos;
      FIELD_CAMERA *camera_control;
      camera_control = FIELDMAP_GetFieldCamera( fieldWork );
      FIELD_CAMERA_GetTargetPos( camera_control, &cam);
      CalcPos( &player_pos, &cam, mwk->player_len, mwk->pos_angle );
      //SetPlayerActTrans( fieldWork->field_player, &player_pos );
      FIELD_PLAYER_SetPos( fld_player, &player_pos);
      *pos = player_pos;
      // プレイヤー位置から、カメラ座標を決定
      mapCtrlC3_CameraMain( fieldWork, pos, mwk->camera_pitch, mwk->camera_len );
      FIELD_CAMERA_SetAngleYaw( camera_control, mwk->pos_angle);
      FIELD_PLAYER_C3_Move( fld_player, key_cont, mwk->pos_angle );

      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
        VecFx32 normal_vec = {0,0,-FX32_ONE};
        VecFx32 npos;
        u16 rotate;
        fx32 dist;
        VecFx32 normal;
        
        OS_TPrintf( "player pos x=0x%x y=0x%x z=0x%x\n", player_pos.x, player_pos.y, player_pos.z );
        OS_TPrintf( "0x%x, 0x%x, 0x%x\n", player_pos.x, player_pos.y, player_pos.z );

        VEC_Subtract( pos, &cam, &npos );
        npos.y = 0;
        dist = VEC_Mag( &npos );
        VEC_Normalize( &npos, &npos );
        rotate  = FX_AcosIdx( VEC_DotProduct( &npos, &normal_vec ) );
        VEC_CrossProduct( &npos, &normal_vec, &normal );

        if( normal.y < 0 ){
          rotate = 0x10000 - rotate;
        }
        
        OS_TPrintf( "rotate\n0x%x dist=0x%x normal=0x%x\n", rotate, dist, normal.y );
      }
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


//----------------------------------------------------------------------------
/**
 *	@brief  カメラの計算
 *
 *	@param	fieldWork
 *	@param	cp_pos 
 */
//-----------------------------------------------------------------------------
static void mapCtrlC3_CameraMain( FIELDMAP_WORK *fieldWork, const VecFx32* cp_pos, u16 pitch, fx32 len )
{
  FIELD_CAMERA* p_camera;
  
  p_camera = FIELDMAP_GetFieldCamera( fieldWork );

  cameraMain( p_camera, cp_pos, pitch, len );
}
static void cameraMain( FIELD_CAMERA* p_camera, const VecFx32* cp_pos, u16 pitch, fx32 len )
{
  VecFx32 pos, target, n0, camera_pos;
  fx32 xz_dist;
  fx32 target_y;

	FIELD_CAMERA_GetTargetPos( p_camera, &target);
  target_y  = target.y;
  target.y  = 0;
  
  pos     = *cp_pos;
  pos.y   = 0;

  VEC_Subtract( &pos, &target, &pos );
  VEC_Normalize( &pos, &n0 );

  // 方向ベクトルから、カメラangleを求める
  camera_pos.y = FX_Mul( FX_SinIdx( pitch ), len );
  xz_dist      = FX_Mul( FX_CosIdx( pitch ), len );
  camera_pos.x = FX_Mul( n0.x, xz_dist );
  camera_pos.z = FX_Mul( n0.z, xz_dist );
  camera_pos.x += target.x;
  camera_pos.y += target_y;
  camera_pos.z += target.z;
  
	FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レール動作時のカメラ可動範囲の管理
 *  
 *	@param	fieldWork   フィールドワーク
 */
//-----------------------------------------------------------------------------
static void cameraRailAreaControl( FIELD_CAMERA * p_camera )
{
  VecFx32 target;
  VecFx32 camera_pos, camera_way;
  VecFx32 area_start, area_sn;
  VecFx32 area_end, area_en;
  u16 rot_start, rot_end;

  FIELD_CAMERA_GetTargetPos( p_camera, &target );
  FIELD_CAMERA_GetCameraPos( p_camera, &camera_pos );


  // ターゲットを中心とした距離に変更
  VEC_Subtract( &camera_pos, &target, &camera_pos ); 
  VEC_Subtract( &sc_RAIL_CAMERA_MOVE_AREA_START, &target, &area_start ); 
  VEC_Subtract( &sc_RAIL_CAMERA_MOVE_AREA_END, &target, &area_end );
  camera_way = camera_pos;  // この後の座標調整に使用するため保存
  camera_pos.y  = 0;
  area_start.y  = 0;
  area_end.y    = 0;
  
  VEC_Normalize( &camera_pos, &camera_pos );
  VEC_Normalize( &area_start, &area_start );
  VEC_Normalize( &area_end, &area_end );

  // start endとの外積を求める
  VEC_CrossProduct( &area_start, &camera_pos, &area_sn );
  VEC_CrossProduct( &area_end, &camera_pos, &area_en );
  rot_start   = FX_AcosIdx( VEC_DotProduct( &area_start, &camera_pos ) );
  rot_end     = FX_AcosIdx( VEC_DotProduct( &area_end, &camera_pos ) );

/*
  OS_TPrintf( "start = 0x%x end = 0x%x\n", area_sn.y, area_en.y );
  OS_TPrintf( "rot_start = %d rot_end = %d\n", rot_start/182, rot_end/182 );
//*/
  
  // 範囲外なのでカメラ座標を変更
  if( (area_sn.y > 0) && (area_en.y < 0) ){
    fx32 dist;

    // 高さはそのまま
    camera_pos.y = camera_way.y;
    camera_way.y = 0;

    // 平面の方向を調整する
    dist = VEC_Mag( &camera_way );
    if( rot_start < rot_end ){
      camera_pos.x = FX_Mul( area_start.x, dist );
      camera_pos.z = FX_Mul( area_start.z, dist );
    }else{
      camera_pos.x = FX_Mul( area_end.x, dist );
      camera_pos.z = FX_Mul( area_end.z, dist );
    }

    camera_pos.x += target.x;
    camera_pos.z += target.z;

	  FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );
  }
}






//======================================================================
//	SCENEAREA情報
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリア情報  エリア判定
 *
 *	@param	cp_sys      システム
 *	@param	cp_data     データ
 *	@param	cp_pos      自機位置
 *
 *	@retval TRUE    エリア内
 *	@retval FALSE   エリア外
 */
//-----------------------------------------------------------------------------
static BOOL C3_SCENEAREA_CheckArea( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
  VecFx32 normal_vec = {0,0,-FX32_ONE};
  VecFx32 npos;
  fx32 dist;
  u32 rotate;
  VecFx32 target, normal;
  const C3_SCENEAREA_PARAM* cp_param = (C3_SCENEAREA_PARAM*)cp_data->area;

  VEC_Set( &target, CAMERA_DEF_TARGET_X, 0, CAMERA_DEF_TARGET_Z );
  

  VEC_Subtract( cp_pos, &target, &npos );
  npos.y  = 0;
  dist    = VEC_Mag( &npos );
  VEC_Normalize( &npos, &npos );
  rotate  = FX_AcosIdx( VEC_DotProduct( &npos, &normal_vec ) );
  VEC_CrossProduct( &npos, &normal_vec, &normal );
  if( normal.y < 0 ){
    rotate = 0x10000 - rotate;
  }

  // エリア内判定
  if( (cp_param->rot_start <= rotate) && (cp_param->rot_end > rotate) ){
    if( (cp_param->dist_min <= dist) && (cp_param->dist_max > dist) ){
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  更新処理
 *
 *	@param	cp_sys      システム
 *	@param	cp_data     データ
 *	@param	cp_pos      位置情報
 */
//-----------------------------------------------------------------------------
static void C3_SCENEAREA_Update( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
  VecFx32 pos, target, n0, camera_pos;
  FIELD_CAMERA* p_camera;
  fx32 xz_dist;
  fx32 target_y;
  const C3_SCENEAREA_PARAM* cp_param = (const C3_SCENEAREA_PARAM*)cp_data->area;

  p_camera = FLD_SCENEAREA_GetFieldCamera( cp_sys );
	FIELD_CAMERA_GetTargetPos( p_camera, &target);
  target_y  = target.y;
  target.y  = 0;
  
  pos     = *cp_pos;
  pos.y   = 0;

  VEC_Subtract( &pos, &target, &pos );
  VEC_Normalize( &pos, &n0 );

  // 方向ベクトルから、カメラangleを求める
  camera_pos.y = FX_Mul( FX_SinIdx( cp_param->pitch ), cp_param->length );
  xz_dist      = FX_Mul( FX_CosIdx( cp_param->pitch ), cp_param->length );
  camera_pos.x = FX_Mul( n0.x, xz_dist );
  camera_pos.z = FX_Mul( n0.z, xz_dist );
  camera_pos.x += target.x;
  camera_pos.y += target_y;
  camera_pos.z += target.z;
  
	FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );

  // カメラ動作限界管理
  cameraRailAreaControl( p_camera );
}

//----------------------------------------------------------------------------
/**
 *	@brief  範囲に入った瞬間の処理
 *
 *	@param	cp_sys      システム
 *	@param	cp_data     データ
 *	@param	cp_pos      位置情報
 */
//-----------------------------------------------------------------------------
static void C3_SCENEAREA_Inside( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
  C3_SCENEAREA_Update( cp_sys, cp_data, cp_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  範囲から出た瞬間の処理
 *
 *	@param	cp_sys      システム
 *	@param	cp_data     データ
 *	@param	cp_pos      位置情報
 */
//-----------------------------------------------------------------------------
static void C3_SCENEAREA_Outside( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  固定カメラの　設定処理
 *  
 *	@param	cp_sys    システムワーク
 *	@param	cp_data   データ
 *	@param	cp_pos    位置情報
 */
//-----------------------------------------------------------------------------
static void C3_SCENEAREA_InsideFxCamera( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
  FIELD_CAMERA* p_camera;
  const C3_SCENEAREA_PARAM* cp_param = (const C3_SCENEAREA_PARAM*)cp_data->area;
  VecFx32 target, camera_pos;

  p_camera = FLD_SCENEAREA_GetFieldCamera( cp_sys );

  // ターゲット位置設定
  VEC_Set( &target, cp_param->target_x, cp_param->target_y, cp_param->target_z );
  VEC_Set( &camera_pos, cp_param->camera_x, cp_param->camera_y, cp_param->camera_z );
  FIELD_CAMERA_SetTargetPos( p_camera, &target );
  FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  固定カメラの　回避処理
 *  
 *	@param	cp_sys    システムワーク
 *	@param	cp_data   データ
 *	@param	cp_pos    位置情報
 */
//-----------------------------------------------------------------------------
static void C3_SCENEAREA_OutsideFxCamera( const FLD_SCENEAREA* cp_sys, const FLD_SCENEAREA_DATA* cp_data, const VecFx32* cp_pos )
{
  FIELD_CAMERA* p_camera;
  VecFx32 target, camera_pos;

  p_camera = FLD_SCENEAREA_GetFieldCamera( cp_sys );

  // ターゲット位置設定
  VEC_Set( &target, CAMERA_DEF_TARGET_X, CAMERA_DEF_TARGET_Y, CAMERA_DEF_TARGET_Z );
  FIELD_CAMERA_SetTargetPos( p_camera, &target );

  // カメラーパラメータ設定
  cameraMain( p_camera, cp_pos, 0x600, 0x26D000 );
}



