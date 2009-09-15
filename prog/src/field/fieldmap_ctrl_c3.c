/**
 * @file	fieldmap_ctrl_c3.c
 * @brief	フィールドマップ　コントロール　C3マップ処理
 * @author	tamada
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "arc/fieldmap/field_rail_data.naix"
#include "arc/fieldmap/field_scenearea_data.naix"

#include "fieldmap.h"
#include "field_player_nogrid.h"
#include "fieldmap_ctrl_nogrid_work.h"

#include "field_easytp.h"

#include "fieldmap_ctrl_c3.h"

#include "gamesystem/game_data.h"  //PLAYER_WORK

#include "field_rail.h"
#include "field/field_rail_loader.h"
#include "fld_scenearea.h"
#include "fld_scenearea_loader.h"
#include "fld_scenearea_loader_func.h"


//======================================================================
//	シーンエリア情報
//======================================================================


//======================================================================
//	define
//======================================================================
#define HEIGHT	(0x3000)

#define CAMERA_DEF_PITCH  ( 0x100 )
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
}C3_MOVE_WORK;




//======================================================================
//	proto
//======================================================================
static void mapCtrlC3_Create(
		FIELDMAP_WORK *fieldWork, VecFx32 *pos, u16 dir );
static void mapCtrlC3_Delete( FIELDMAP_WORK *fieldWork );
static void mapCtrlC3_Main( FIELDMAP_WORK *fieldWork, VecFx32 *pos );
static const VecFx32 * mapCtrlC3_GetCameraTarget( FIELDMAP_WORK *fieldWork );

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
  mapCtrlC3_GetCameraTarget,
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
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  FIELDMAP_CTRL_NOGRID_WORK* p_ngridwork;
  FIELD_PLAYER_NOGRID* p_ngridplayer;
	static const C3_MOVE_WORK init = {
		 0x1f0,
		 0,
		 1,
		 16,
     CAMERA_DEF_LEN,
     CAMERA_DEF_PITCH,
	};

  // レール起動
  {
    static const FLDNOGRID_RESISTDATA sc_RESIST = 
    {
      NARC_field_rail_data_c3_dat,
      NARC_field_scenearea_data_c3_dat,
    };

    FLDNOGRID_MAPPER_ResistData( p_mapper, &sc_RESIST, FIELDMAP_GetHeapID(fieldWork) );
  }
	
	
	heapID = FIELDMAP_GetHeapID( fieldWork );

  p_ngridwork = FIELDMAP_CTRL_NOGRID_WORK_Create( fieldWork, heapID );
	FIELDMAP_SetMapCtrlWork( fieldWork, p_ngridwork );

  work = FIELDMAP_CTRL_NOGRID_WORK_AllocLocalWork( p_ngridwork,  heapID, sizeof(C3_MOVE_WORK) );
	*work = init;

	fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
  FIELDMAP_SetNowPosTarget( fieldWork, NULL );
  
  // 位置情報を設定
  p_ngridplayer = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( p_ngridwork );
  FIELD_PLAYER_NOGRID_SetLocation( p_ngridplayer, &sc_RAIL_START_LOCATION );

  FIELD_PLAYER_NOGRID_GetPos( p_ngridplayer, pos );

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
  FIELDMAP_CTRL_NOGRID_WORK* p_ngridwork = FIELDMAP_GetMapCtrlWork( fieldWork );
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );

  FIELDMAP_CTRL_NOGRID_WORK_FreeLocalWork( p_ngridwork );
  FIELDMAP_CTRL_NOGRID_WORK_Delete( p_ngridwork );

  FLDNOGRID_MAPPER_Release( p_mapper );
  FLDNOGRID_MAPPER_UnBindCameraWork( p_mapper );
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
  FIELDMAP_CTRL_NOGRID_WORK* p_ngridwork = FIELDMAP_GetMapCtrlWork( fieldWork );
	C3_MOVE_WORK *mwk = FIELDMAP_CTRL_NOGRID_WORK_GetLocalWork( p_ngridwork );
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
  BOOL rail_flag = FLDNOGRID_MAPPER_DEBUG_IsActive(p_mapper);
	FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
  PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork(
      GAMESYSTEM_GetGameData(FIELDMAP_GetGameSysWork(fieldWork)) );
  const FLD_SCENEAREA* cp_areaMan = FLDNOGRID_MAPPER_GetSceneAreaMan( p_mapper );
  FIELD_PLAYER_NOGRID* p_ngrid_player = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( p_ngridwork );
  

  //デバッグのため、レール処理をON/OFF
#ifdef PM_DEBUG
  if (key_trg & PAD_BUTTON_L)
  {
    FLDNOGRID_MAPPER_DEBUG_SetActive( p_mapper, !rail_flag );
  }
#endif

  if (rail_flag)
  {
    // カメラ動作限界管理
    cameraRailAreaControl( FIELDMAP_GetFieldCamera( fieldWork ) );

		// 移動方向の設定
		FIELDMAP_CTRL_NOGRID_WORK_Main( p_ngridwork );


    FIELD_PLAYER_GetPos( fld_player, pos );
    PLAYERWORK_setPosition( player, pos );
//		TOMOYA_Printf( "pos->y 0x%x\n", pos->y );
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
			FIELD_CAMERA_SetMode( camera_control, FIELD_CAMERA_MODE_DIRECT_POS );
      CalcPos( &player_pos, &cam, mwk->player_len, mwk->pos_angle );
      //SetPlayerActTrans( fieldWork->field_player, &player_pos );
      FIELD_PLAYER_SetPos( fld_player, &player_pos);
      *pos = player_pos;
      // プレイヤー位置から、カメラ座標を決定
      mapCtrlC3_CameraMain( fieldWork, pos, mwk->camera_pitch, mwk->camera_len );
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

//--------------------------------------------------------------
/**
 * フィールドマップ　C3処理　カメラターゲット取得
 * @param fieldWork FIELDMAP_WORK
 * @retval VecFx32* カメラターゲット
 */
//--------------------------------------------------------------
static const VecFx32 * mapCtrlC3_GetCameraTarget( FIELDMAP_WORK *fieldWork )
{
  return NULL;
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






