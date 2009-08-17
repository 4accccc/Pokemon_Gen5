//============================================================================================
/**
 * @file	field_camera.c
 * @brief	フィールドカメラ制御
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================
#include <gflib.h>
#include "field_g3d_mapper.h"
#include "field_common.h"
#include "field_camera.h"
	
#include "arc_def.h"
#include "arc/fieldmap/field_camera_data.naix"
//============================================================================================
//============================================================================================

#define FIELD_CAMERA_DELAY	(6)
#define FIELD_CAMERA_TRACE_BUFF	(FIELD_CAMERA_DELAY+1)



/*---------------------------------------------------------------------------*
	カメラトレース構造体
 *---------------------------------------------------------------------------*/
typedef struct 
{
	int bufsize;
	int CamPoint;
	int TargetPoint;
	int Delay;			//遅延時間
	BOOL UpdateFlg;		//更新開始フラグ
	BOOL ValidX;		//X遅延を有効にするかのフラグ
	BOOL ValidY;		//Y遅延を有効にするかのフラグ
	BOOL ValidZ;		//Z遅延を有効にするかのフラグ

  VecFx32 * targetBuffer;
  VecFx32 * camPosBuffer;

}CAMERA_TRACE;

//------------------------------------------------------------------
/**
 * @brief	フィールドカメラタイプ
 */
//------------------------------------------------------------------
typedef enum {
	FIELD_CAMERA_TYPE_GRID,
	FIELD_CAMERA_TYPE_H01,
	FIELD_CAMERA_TYPE_C03,

  FIELD_CAMERA_TYPE_POKECEN,
  FIELD_CAMERA_TYPE_H01P01,

  FIELD_CAMERA_TYPE_PLP01,

	FIELD_CAMERA_TYPE_MAX,
}FIELD_CAMERA_TYPE;

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _FIELD_CAMERA {
	HEAPID				heapID;			///<使用するヒープ指定ID
	FIELD_MAIN_WORK * fieldWork;		///<フィールドマップへのポインタ
	GFL_G3D_CAMERA * g3Dcamera;			///<カメラ構造体へのポインタ

	FIELD_CAMERA_TYPE	type;			///<カメラのタイプ指定
	FIELD_CAMERA_MODE mode;			///<カメラモード

  const VecFx32 * default_target;
	const VecFx32 *		watch_target;	///<追随する注視点へのポインタ
  const VecFx32 *   watch_camera; ///<追随するカメラ位置へのポインタ

  VecFx32       camPos;             ///<カメラ位置用ワーク

	VecFx32				target;			        ///<注視点用ワーク
	VecFx32				target_offset;			///<注視点用補正座標
  VecFx32       target_before;

  u16         angle_pitch;
  u16         angle_yaw;
  fx32        angle_len;

  u16         fovy;
  u16         pad;
#ifdef PM_DEBUG
  u32 debug_subscreen_type;

  // ターゲット操作用
  u16         debug_target_pitch;
  u16         debug_target_yaw;
  fx32        debug_target_len;
  VecFx32     debug_target;

  fx32        debug_far;
#endif

  CAMERA_TRACE * Trace;
};


//============================================================================================
//============================================================================================
#include "../resource/fldmapdata/camera_data/fieldcameraformat.h"



static void createTraceData(const int inHistNum, const int inDelay,
						const int inTraceMask, const int inHeapID,
						FIELD_CAMERA * ioCamera);
static void deleteTraceData(FIELD_CAMERA * camera_ptr);
static void updateTraceData(CAMERA_TRACE * trace,
    const VecFx32 * inTarget, const VecFx32 * inCamPos, VecFx32 * outTarget, VecFx32 * outCamPos);
static void traceUpdate(FIELD_CAMERA * camera);
//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadCameraParameters(FIELD_CAMERA * camera);

static void ControlParameter( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_CalcCamera( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_CalcTarget( FIELD_CAMERA * camera, u16 key_cont );
static void ControlParameter_Direct( FIELD_CAMERA * camera, u16 key_cont );


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
FIELD_CAMERA* FIELD_CAMERA_Create(
		FIELD_MAIN_WORK * fieldWork,
		u8 type,
		FIELD_CAMERA_MODE mode,
		GFL_G3D_CAMERA * cam,
		const VecFx32 * target,
		HEAPID heapID)
{
	FIELD_CAMERA* camera = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_CAMERA) );

	enum { CAMERA_LENGTH = 16 };

	GF_ASSERT(cam != NULL);
	TAMADA_Printf("FIELD CAMERA TYPE = %d\n", type);
	camera->fieldWork = fieldWork;
	camera->type = type;
	camera->mode = mode;
	camera->g3Dcamera = cam;
  camera->default_target = target;
	camera->watch_target = target;
  camera->watch_camera = NULL;
	camera->heapID = heapID;

  VEC_Set( &camera->camPos, 0, 0, 0 );
	VEC_Set( &camera->target, 0, 0, 0 );
  VEC_Set( &camera->target_before, 0, 0, 0 );

  camera->angle_yaw = 0;
  camera->angle_pitch = 0;
  camera->angle_len = 0x0078;

  // パースをカメラの情報から取得
  if( GFL_G3D_CAMERA_GetProjectionType( cam ) == GFL_G3D_PRJPERS ){
    fx32 fovyCos;
    GFL_G3D_CAMERA_GetfovyCos( cam, &fovyCos );
    camera->fovy = FX_AcosIdx( fovyCos );
  }

  loadCameraParameters(camera);

	createTraceData(	FIELD_CAMERA_TRACE_BUFF, FIELD_CAMERA_DELAY,
							CAM_TRACE_MASK_Y, heapID, camera);

	return camera;
}
//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_Delete( FIELD_CAMERA* camera )
{
  deleteTraceData( camera );
	GFL_HEAP_FreeMemory( camera );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
void FIELD_CAMERA_Main( FIELD_CAMERA* camera, u16 key_cont)
{
  ControlParameter(camera, key_cont);
}


//----------------------------------------------------------------------------
/**
 *	@brief	カメラモード　設定
 *	
 *	@param	camera		カメラ
 *	@param	mode			モード
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_SetMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode )
{
	GF_ASSERT( camera );
	GF_ASSERT( mode < FIELD_CAMERA_MODE_MAX );
	camera->mode = mode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラモード　取得
 *
 *	@param	camera	カメラ
 *
 *	@return	カメラモード
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA_MODE FIELD_CAMERA_GetMode( const FIELD_CAMERA * camera )
{
	GF_ASSERT( camera );
	return camera->mode;
}


//============================================================================================
//
//
//		カメラ初期化用関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadCameraParameters(FIELD_CAMERA * camera)
{
  enum {
    FILE_ID = NARC_field_camera_data_field_camera_bin,
    ARC_ID  = ARCID_FIELD_CAMERA,
  };
  FLD_CAMERA_PARAM param;
  ARCHANDLE * handle = GFL_ARC_OpenDataHandle(ARC_ID, camera->heapID);
  u16 size = GFL_ARC_GetDataSizeByHandle(handle, FILE_ID);
  if ( camera->type * sizeof(FLD_CAMERA_PARAM) >= size )
  {
    OS_TPrintf("カメラタイプ（%d）が指定できません\n", camera->type);
    GF_ASSERT(0);
    camera->type = 0;
  }
  GFL_ARC_LoadDataOfsByHandle(handle, FILE_ID,
      camera->type * sizeof(FLD_CAMERA_PARAM), sizeof(FLD_CAMERA_PARAM), &param);
#if 0
  TAMADA_Printf("FIELD CAMERA INIT INFO\n");
  TAMADA_Printf("FIELD CAMERA TYPE =%d\n",camera->type);
  TAMADA_Printf("dist = %08x\n", param.Distance);
  TAMADA_Printf("angle = %08x,%08x,%08x\n", param.Angle.x, param.Angle.y, param.Angle.z );
  TAMADA_Printf("viewtype = %d, persp = %08x\n", param.View, param.PerspWay);
  TAMADA_Printf("near=%8x, far =%08x\n", param.Near, param.Far);
  TAMADA_Printf("shift = %08x,%08x,%08x\n", param.Shift.x, param.Shift.y, param.Shift.z );
  TAMADA_Printf("now near = %d\n", FX_Whole( FIELD_CAMERA_GetNear(camera) ) );
  TAMADA_Printf("now far = %d\n", FX_Whole( FIELD_CAMERA_GetFar(camera) ) );
#endif

  camera->angle_len = param.Distance * FX32_ONE;
  camera->angle_pitch = param.Angle.x;
  camera->angle_yaw = param.Angle.y;
  //camera->viewtype = param.ViewType;
  camera->fovy = param.PerspWay;
  camera->target_offset = param.Shift;
  FIELD_CAMERA_SetFar( camera, param.Far );
  FIELD_CAMERA_SetNear(camera, param.Near );
  switch (param.depthType)
  {
  case DEPTH_TYPE_ZBUF:
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    break;
  case DEPTH_TYPE_WBUF:
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
    break;
  }
  if( GFL_G3D_CAMERA_GetProjectionType(camera->g3Dcamera) == GFL_G3D_PRJPERS ){
    GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
    GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
  }

	GFL_ARC_CloseDataHandle(handle);
}

//============================================================================================
//
//
//		カメラ状態アップデート用関数
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	注視点を保持する座標ワークを監視、追随する
 */
//------------------------------------------------------------------
static void updateTargetBinding(FIELD_CAMERA * camera)
{
	if (camera->watch_target) {
		(camera->target) = *(camera->watch_target);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL updateCamPosBinding(FIELD_CAMERA * camera)
{
  if (camera->watch_camera)
  {
    (camera->camPos) = *(camera->watch_camera);
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void calcAnglePos( const VecFx32* cp_target, VecFx32* p_pos, u16 yaw, u16 pitch, fx32 len )
{
  enum {  PITCH_LIMIT = 0x200 };
  VecFx32 cameraPos;
	fx16 sinYaw = FX_SinIdx(yaw);
	fx16 cosYaw = FX_CosIdx(yaw);
	fx16 sinPitch = FX_SinIdx(pitch);
	fx16 cosPitch = FX_CosIdx(pitch);

	if(cosPitch < 0){ cosPitch = -cosPitch; }	// 裏周りしないようにマイナス値はプラス値に補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }	// 0値近辺は不正表示になるため補正

	// カメラの座標計算
	VEC_Set( &cameraPos, FX_Mul(sinYaw, cosPitch), sinPitch, FX_Mul(cosYaw, cosPitch) );
	VEC_Normalize(&cameraPos, &cameraPos);
	VEC_MultAdd( len, &cameraPos, cp_target, &cameraPos );
  //cameraPos = cameraPos * length + camera->target
  *p_pos = cameraPos;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateAngleCameraPos(FIELD_CAMERA * camera)
{ 
  // カメラポジション計算
	calcAnglePos( &camera->target, &camera->camPos,
			camera->angle_yaw, camera->angle_pitch, camera->angle_len );
}
static void updateAngleTargetPos(FIELD_CAMERA * camera)
{ 
  // ターゲットポジション計算
	calcAnglePos( &camera->camPos, &camera->target,
			camera->angle_yaw, camera->angle_pitch, camera->angle_len );
}
	
//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateG3Dcamera(FIELD_CAMERA * camera)
{
	VecFx32 cameraTarget;

#ifndef PM_DEBUG
	// カメラターゲット補正
  VEC_Add( &camera->target, &camera->target_offset, &cameraTarget );

	GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &cameraTarget );
	GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->camPos );

  traceUpdate( camera );
#else

  if( camera->debug_subscreen_type != FIELD_CAMERA_DEBUG_BIND_TARGET_POS )
  {
    // 通常のカメラターゲット
    // カメラターゲット補正
    VEC_Add( &camera->target, &camera->target_offset, &cameraTarget );

    GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &cameraTarget );
    GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->camPos );

		if( camera->debug_subscreen_type == FIELD_CAMERA_DEBUG_BIND_NONE )
		{
//			traceUpdate( camera );
		}
  }
  else
  {
    // ターゲットを変更
    calcAnglePos( &camera->camPos, &camera->debug_target, camera->debug_target_yaw,
        camera->debug_target_pitch, camera->debug_target_len );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
    {
      VecFx32 target_offset;
      
      OS_TPrintf( "DEBUG Camera Target x[0x%x] y[0x%x] z[0x%x]\n", camera->debug_target.x, camera->debug_target.y, camera->debug_target.z  );
      OS_TPrintf( "DEBUG Camera Pos x[0x%x] y[0x%x] z[0x%x]\n", camera->camPos.x, camera->camPos.y, camera->camPos.z  );

      VEC_Subtract( &camera->debug_target, &camera->target, &target_offset );
      OS_TPrintf( "DEBUG Scroll Camera Target Offset x[0x%x] y[0x%x] z[0x%x]\n", target_offset.x, target_offset.y, target_offset.z  );
    }

    GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &camera->debug_target );
    GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &camera->camPos );
  }

  // Far,Fovy座標の設定
  if( camera->debug_subscreen_type != FIELD_CAMERA_DEBUG_BIND_NONE ){
    GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &camera->debug_far );
    if( GFL_G3D_CAMERA_GetProjectionType(camera->g3Dcamera) == GFL_G3D_PRJPERS ){
      GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
      GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
    }
  }

#endif


}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ControlParameter(FIELD_CAMERA * camera, u16 key_cont)
{
	static void (*const cp_Control[FIELD_CAMERA_MODE_MAX])( FIELD_CAMERA * camera, u16 key_cont ) = 
	{
		ControlParameter_CalcCamera,
		ControlParameter_CalcTarget,
		ControlParameter_Direct,
	};
	GF_ASSERT( camera->mode < FIELD_CAMERA_MODE_MAX );
	cp_Control[ camera->mode ]( camera, key_cont );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	カメラ座標計算モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_CalcCamera( FIELD_CAMERA * camera, u16 key_cont )
{
	updateTargetBinding(camera);
  updateAngleCameraPos(camera);
  updateG3Dcamera(camera);
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	ターゲット座標計算モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_CalcTarget( FIELD_CAMERA * camera, u16 key_cont )
{
	updateCamPosBinding( camera );
  updateAngleTargetPos(camera);
  updateG3Dcamera(camera);
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ管理	ダイレクト座標モード
 */
//-----------------------------------------------------------------------------
static void ControlParameter_Direct( FIELD_CAMERA * camera, u16 key_cont )
{
	updateTargetBinding(camera);
	updateCamPosBinding( camera );
  updateG3Dcamera(camera);
}



//============================================================================================
//
//
//			カメラ操作用外部公開関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera)
{
	return camera->g3Dcamera;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_BindTarget(FIELD_CAMERA * camera, const VecFx32 * watch_target)
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->watch_target = watch_target;
  camera->target_before = *watch_target;
  camera->target = *watch_target;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera)
{
	camera->watch_target = NULL;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_BindDefaultTarget(FIELD_CAMERA * camera)
{
  camera->watch_target = camera->default_target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットがあるかチェック
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_IsBindDefaultTarget( const FIELD_CAMERA * camera )
{
	if( camera->default_target )
	{
		return TRUE;
	}
	return FALSE;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
/*
void FIELD_CAMERA_BindNoCamera(FIELD_CAMERA * camera, BOOL flag)
{
	if(flag)
	{
		camera->watch_camera = &camera->camPos;
	}
	else
	{
		camera->watch_camera = NULL;
	}
}
//*/
//----------------------------------------------------------------------------
/**
 *	@brief	カメラ座標のバインド
 *
 *	@param	camera				カメラ
 *	@param	watch_camera	カメラついづい座標
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_BindCamera(FIELD_CAMERA * camera, const VecFx32 * watch_camera)
{
	GF_ASSERT( camera );
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) || (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) );
	camera->watch_camera = watch_camera;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ座標のバインド　終了
 *
 *	@param	camera	カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_FreeCamera(FIELD_CAMERA * camera)
{
	GF_ASSERT( camera );
	camera->watch_camera = NULL;
}


//------------------------------------------------------------------
/**
 * @brief	nearクリップ面までの距離を設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	near		設定する距離（fx32)
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetNear(FIELD_CAMERA * camera, fx32 near)
{
	GFL_G3D_CAMERA_SetNear(camera->g3Dcamera, &near);
}

//------------------------------------------------------------------
/**
 * @brief	nearクリップ面までの距離を取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @return	fx32		取得した距離
 */
//------------------------------------------------------------------
fx32 FIELD_CAMERA_GetNear(const FIELD_CAMERA * camera)
{
	fx32 near;
	GFL_G3D_CAMERA_GetNear( camera->g3Dcamera, &near);
	return near;
}

//------------------------------------------------------------------
/**
 * @brief	farクリップ面までの距離を設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	far		設定する距離（fx32)
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetFar(FIELD_CAMERA * camera, fx32 far)
{
	GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &far );
}

//------------------------------------------------------------------
/**
 * @brief	farクリップ面までの距離を取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @return	fx32		取得した距離
 */
//------------------------------------------------------------------
fx32 FIELD_CAMERA_GetFar(const FIELD_CAMERA * camera)
{
	fx32 far;
	GFL_G3D_CAMERA_GetFar( camera->g3Dcamera, &far);
	return far;
}

//------------------------------------------------------------------
/**
 * @brief	カメラ位置の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ位置を渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_SetTargetPos( FIELD_CAMERA* camera, const VecFx32* target )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) || (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS) );
	camera->target = *target;
  camera->target_before = * target;
}
//------------------------------------------------------------------
/**
 * @brief	カメラ位置の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ位置を受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_GetTargetPos( const FIELD_CAMERA* camera, VecFx32* pos )
{
	*pos = camera->target;
}

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		    FIELDカメラ制御ポインタ
 * @param	target_offset	カメラ注視点を補正する座標受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_GetTargetOffset( const FIELD_CAMERA* camera, VecFx32* target_offset )
{
  *target_offset = camera->target_offset;
}


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param	target_offset			カメラ注視点を補正する座標渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_SetTargetOffset( FIELD_CAMERA* camera, const VecFx32* target_offset )
{
  camera->target_offset = *target_offset;
}

//------------------------------------------------------------------
/**
 * @brief カメラ位置の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param camPos            取得位置を格納するVecFx32へのポインタ
 */
//------------------------------------------------------------------
void FIELD_CAMERA_GetCameraPos( const FIELD_CAMERA * camera, VecFx32 * camPos)
{
  *camPos = camera->camPos;
}
//------------------------------------------------------------------
/**
 * @brief カメラ位置のセット
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param camPos            設定する位置情報
 *
 * FIELD_CAMERA内部でカメラ位置が制御されているときには
 * 反映されない場合もあるので注意
 */
//------------------------------------------------------------------
void FIELD_CAMERA_SetCameraPos( FIELD_CAMERA * camera, const VecFx32 * camPos)
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_DIRECT_POS) || (camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
  camera->camPos = *camPos;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　垂直方向回転
 */
//-----------------------------------------------------------------------------
u16 FIELD_CAMERA_GetAnglePitch(const FIELD_CAMERA * camera )
{
	return camera->angle_pitch;
}
void FIELD_CAMERA_SetAnglePitch(FIELD_CAMERA * camera, u16 angle )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_pitch = angle;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　水平方向回転
 */
//-----------------------------------------------------------------------------
u16 FIELD_CAMERA_GetAngleYaw(const FIELD_CAMERA * camera )
{
	return camera->angle_yaw;
}

void FIELD_CAMERA_SetAngleYaw(FIELD_CAMERA * camera, u16 angle )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_yaw = angle;
}

const u16 * FIELD_CAMERA_GetAngleYawAddress( const FIELD_CAMERA *camera )
{
  return( &camera->angle_yaw );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラangle　ターゲットからの距離操作
 */
//-----------------------------------------------------------------------------
fx32 FIELD_CAMERA_GetAngleLen(const FIELD_CAMERA * camera )
{
	return camera->angle_len;	
}
void FIELD_CAMERA_SetAngleLen(FIELD_CAMERA * camera, fx32 length )
{
	GF_ASSERT( (camera->mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)||(camera->mode == FIELD_CAMERA_MODE_CALC_TARGET_POS) );
	camera->angle_len = length;
}



#ifdef  PM_DEBUG
#include "test/camera_adjust_view.h"
extern fx32	fldWipeScale;
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type)
{ 
  GFL_CAMADJUST * gflCamAdjust = param;

  // ファー座標を取得
  GFL_G3D_CAMERA_GetFar( camera->g3Dcamera, &camera->debug_far );

  camera->debug_subscreen_type = type;
  if( type == FIELD_CAMERA_DEBUG_BIND_CAMERA_POS )
  {
	  FIELD_CAMERA_SetMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->angle_yaw, 
																	&camera->angle_pitch, 
																	&camera->angle_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
  else if( type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS )
  {
	  FIELD_CAMERA_SetMode( camera, FIELD_CAMERA_MODE_CALC_TARGET_POS );
    camera->debug_target_yaw    = camera->angle_yaw - 0x8000;
    camera->debug_target_pitch  = 0x10000 - camera->angle_pitch;
    camera->debug_target_len    = camera->angle_len;
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->debug_target_yaw, 
																	&camera->debug_target_pitch, 
																	&camera->debug_target_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
}

void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera)
{ 

  if( camera->debug_subscreen_type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS ){

    OS_TPrintf( "DEBUG Camera Target x[0x%x] y[0x%x] z[0x%x]\n", camera->debug_target.x, camera->debug_target.y, camera->debug_target.z  );
    OS_TPrintf( "DEBUG Camera Pos x[0x%x] y[0x%x] z[0x%x]\n", camera->camPos.x, camera->camPos.y, camera->camPos.z  );

    VEC_Subtract( &camera->debug_target, &camera->target, &camera->target_offset );
    OS_TPrintf( "DEBUG Scroll Camera Target Offset x[0x%x] y[0x%x] z[0x%x]\n", camera->target_offset.x, camera->target_offset.y, camera->target_offset.z  );
  }

  camera->debug_subscreen_type = FIELD_CAMERA_DEBUG_BIND_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	操作しているターゲットを取得
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target )
{
	*p_target = camera->debug_target;
}


//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットの設定
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_SetDefaultTarget( FIELD_CAMERA* camera, const VecFx32* target )
{
	camera->default_target = target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デフォルトターゲットの取得
 */
//-----------------------------------------------------------------------------
const VecFx32* FIELD_CAMERA_DEBUG_GetDefaultTarget( const FIELD_CAMERA* camera )
{
	return camera->default_target;
}

#endif  //PM_DEBUG

//---------------------------------------------------------------------------
/**
 * @brief
 */
//---------------------------------------------------------------------------
static void updateTraceData(CAMERA_TRACE * trace,
    const VecFx32 * inTarget, const VecFx32 * inCamPos, VecFx32 * outTarget, VecFx32 * outCamPos)
{
	int cam_ofs;
	int tgt_ofs;

	if (trace==NULL)
  {
		*outTarget = *inTarget;
		*outCamPos = *inCamPos;
    return;
  }

  cam_ofs = trace->CamPoint;
  tgt_ofs = trace->TargetPoint;

  if (trace->UpdateFlg){
    //履歴データから座標取得
    *outTarget = trace->targetBuffer[cam_ofs];
    *outCamPos = trace->camPosBuffer[cam_ofs];
  }
  else
  {
		*outTarget = *inTarget;
		*outCamPos = *inCamPos;
    if (cam_ofs==trace->Delay){
      trace->UpdateFlg = TRUE;
    }
  }

  //参照位置更新
  cam_ofs = (cam_ofs+1) % trace->bufsize;
  //履歴に積む
  trace->targetBuffer[tgt_ofs] = *inTarget;
  trace->camPosBuffer[tgt_ofs] = *inCamPos;
  //書き換え位置更新
  tgt_ofs = (tgt_ofs+1) % trace->bufsize;
  
  trace->CamPoint = cam_ofs;
  trace->TargetPoint = tgt_ofs;

  //トレースデータの無効座標軸は、そのまま現在座標を採用
  if (!trace->ValidX){	//ｘ無効判定
    outTarget->x = inTarget->x;
    outCamPos->x = inCamPos->x;
  }
  if (!trace->ValidY){	//ｙ無効判定
    outTarget->y = inTarget->y;
    outCamPos->y = inCamPos->y;
  }
  if (!trace->ValidZ){	//ｚ無効判定
    outTarget->z = inTarget->z;
    outCamPos->z = inCamPos->z;
  }
}


//---------------------------------------------------------------------------
/**
 * @brief	トレース構造体のアロケーション　カメラが対象物にバインド済みであること
 * 
 * @param	inHistNum		バッファサイズ
 * @param	inDelay			遅延度合い
 * @param	inTraceMask		トレースマスク	camera.hに定義
 * @param	inHeapID		ヒープID
 * @param	ioCamera		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void createTraceData(const int inHistNum, const int inDelay,
						const int inTraceMask, const int inHeapID,
						FIELD_CAMERA * ioCamera)
{
	int i;
	CAMERA_TRACE *trace;
	//バインドされていなければ、処理しない
	if (ioCamera->watch_target == NULL){
		return;
	}
	//配列サイズの整合性チェック
	GF_ASSERT_MSG(inDelay+1<=inHistNum, "配列数が足りません");
	
	trace = GFL_HEAP_AllocMemory(inHeapID,sizeof(CAMERA_TRACE));
  trace->targetBuffer = GFL_HEAP_AllocMemory(inHeapID, sizeof(VecFx32)*inHistNum);
  trace->camPosBuffer = GFL_HEAP_AllocMemory(inHeapID, sizeof(VecFx32)*inHistNum);
	
	//トレース配列クリア
	for(i=0;i<inHistNum;i++){
    VEC_Set(&trace->targetBuffer[i], 0, 0, 0);
    VEC_Set(&trace->camPosBuffer[i], 0, 0, 0);
	}
	
	trace->bufsize = inHistNum;
	//０番目にカメラ参照位置をセット
	trace->CamPoint = 0;
	//対象物参照位置セット
	trace->TargetPoint = 0+inDelay;
	
	trace->Delay = inDelay;
	trace->UpdateFlg = FALSE;
	
	trace->ValidX = FALSE;
	trace->ValidY = FALSE;
	trace->ValidZ = FALSE;
	if (inTraceMask & CAM_TRACE_MASK_X){
		trace->ValidX = TRUE;
	}
	if (inTraceMask & CAM_TRACE_MASK_Y){
		trace->ValidY = TRUE;
	}
	if (inTraceMask & CAM_TRACE_MASK_Z){
		trace->ValidZ = TRUE;
	}
	
	ioCamera->Trace = trace;
}

//---------------------------------------------------------------------------
/**
 * @brief	トレースデータの解放
 * 
 * @param	camera_ptr		カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
void deleteTraceData(FIELD_CAMERA * camera_ptr)
{
	if (camera_ptr->Trace!=NULL){
    GFL_HEAP_FreeMemory(camera_ptr->Trace->targetBuffer);
    GFL_HEAP_FreeMemory(camera_ptr->Trace->camPosBuffer);
		GFL_HEAP_FreeMemory(camera_ptr->Trace);
		camera_ptr->Trace = NULL;
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void traceUpdate(FIELD_CAMERA * camera)
{
  VecFx32 cam, tgt;
  VecFx32 outCam, outTgt;

  if (camera->watch_target == NULL) return;


  GFL_G3D_CAMERA_GetTarget(camera->g3Dcamera, &tgt);
  GFL_G3D_CAMERA_GetPos(camera->g3Dcamera, &cam);

  updateTraceData( camera->Trace, &tgt, &cam, &outTgt, &outCam );

  GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &outTgt );
  GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &outCam );

  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) {
    TAMADA_Printf("inTGT:%3d inCAM:%3d ", FX_Whole(tgt.y), FX_Whole(cam.y) );
    TAMADA_Printf("outTGT:%3d outCAM:%3d \n", FX_Whole(outTgt.y), FX_Whole(outCam.y) );
  }
}


