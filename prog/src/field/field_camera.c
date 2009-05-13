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
	
//#include "arc_def.h"
//============================================================================================
//============================================================================================

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

	const VecFx32 *		watch_target;	///<追随する注視点へのポインタ

	VecFx32				target;			        ///<注視点用ワーク
	VecFx32				target_offset;			///<注視点用補正座標

  u16         angle_pitch;
  u16         angle_yaw;
  fx32        angle_len;
};


#if 0
//------------------------------------------------------------------
/**
 * @brief	カメラパラメータ
 */
//------------------------------------------------------------------
typedef struct {
	fx32			Distance;
	VecFx32			Angle;
	u16				View;
	u16				PerspWay;
	fx32			Near;
	fx32			Far;
	VecFx32			Shift;
}FIELD_CAMERA_PARAM;

static const FIELD_CAMERA_PARAM FieldCameraParam[] = {
	{
	},
};
#endif


//------------------------------------------------------------------
/**
 * @brief	場面ごとのカメラ制御関数テーブル定義
 */
//------------------------------------------------------------------
typedef struct {
	void (*init_func)(FIELD_CAMERA * camera);
	void (*control)(FIELD_CAMERA * camera, u16 key_cont);
}CAMERA_FUNC_TABLE;

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initGridParameter(FIELD_CAMERA * camera);
static void initBridgeParameter(FIELD_CAMERA * camera);
static void initC3Parameter(FIELD_CAMERA * camera);
static void ControlGridParameter(FIELD_CAMERA * camera, u16 key_cont);
static void ControlBridgeParameter(FIELD_CAMERA * camera, u16 key_cont);
static void ControlC3Parameter(FIELD_CAMERA * camera, u16 key_cont);


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	場面ごとのカメラ制御関数テーブル定義
 */
//------------------------------------------------------------------
static const CAMERA_FUNC_TABLE CameraFuncTable[] = {
	{
		initGridParameter,
		ControlGridParameter,
	},
	{
		initBridgeParameter,
		ControlBridgeParameter,
	},
	{
		initC3Parameter,
		ControlC3Parameter,
	}
};


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
FIELD_CAMERA* FIELD_CAMERA_Create(
		FIELD_MAIN_WORK * fieldWork,
		FIELD_CAMERA_TYPE type,
		GFL_G3D_CAMERA * cam,
		const VecFx32 * target,
		HEAPID heapID)
{
	FIELD_CAMERA* camera = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_CAMERA) );

	enum { CAMERA_LENGTH = 16 };

	GF_ASSERT(cam != NULL);
	GF_ASSERT(type < FIELD_CAMERA_TYPE_MAX);
	TAMADA_Printf("FIELD CAMERA TYPE = %d\n", type);
	camera->fieldWork = fieldWork;
	camera->type = type;
	camera->g3Dcamera = cam;
	camera->watch_target = target;
	camera->heapID = heapID;

	VEC_Set( &camera->target, 0, 0, 0 );

  camera->angle_yaw = 0;
  camera->angle_pitch = 0;
  camera->angle_len = 0x0078;

	CameraFuncTable[camera->type].init_func(camera);

	return camera;
}
//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_Delete( FIELD_CAMERA* camera )
{
	GFL_HEAP_FreeMemory( camera );
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
void FIELD_CAMERA_Main( FIELD_CAMERA* camera, u16 key_cont)
{
	CameraFuncTable[camera->type].control(camera, key_cont);
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
static void initGridParameter(FIELD_CAMERA * camera)
{
	enum { 
		CAMERA_LENGTH = 0x78000,
		CAMERA_HEIGHT = 0xd8000,
	};
	//{ 0x78, 0xd8000 },	//DPぽい
	// 距離
	camera->angle_len = FX_Mul( CAMERA_LENGTH, CAMERA_LENGTH ) + FX_Mul( CAMERA_HEIGHT, CAMERA_HEIGHT );
	camera->angle_len = FX_Sqrt( camera->angle_len );

	// X軸の角度
	{
		fx32 cos;
		VecFx32 vec0 = { 0,0,FX32_ONE };
		VecFx32 vec1 = { 0 };

		vec1.z = CAMERA_LENGTH;
		vec1.y = CAMERA_HEIGHT;

		VEC_Normalize( &vec0, &vec0 );
		VEC_Normalize( &vec1, &vec1 );

		cos = VEC_DotProduct( &vec0, &vec1 );
		camera->angle_pitch = FX_AcosIdx( cos );
	}
	
	FIELD_CAMERA_SetFar( camera, (1024 << FX32_SHIFT) );
	
  // ターゲット補正値
  VEC_Set( &camera->target_offset, 0, 0x4000, 0 );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initC3Parameter(FIELD_CAMERA * camera)
{
	enum { 
		CAMERA_LENGTH = 0x308,
		CAMERA_HEIGHT = 0x07c,

		CAMERA_ANGLE_LENGTH = 0x315000,
	};
	VecFx32 trans = {0x2f6f36, 0x4000, 0x301402};

	
	// 距離
	camera->angle_len = CAMERA_ANGLE_LENGTH;

	// X軸の角度
	{
		fx32 cos;
		VecFx32 vec0 = { 0,0,FX32_ONE };
		VecFx32 vec1 = { 0 };

		vec1.z = CAMERA_LENGTH;
		vec1.y = CAMERA_HEIGHT;

		VEC_Normalize( &vec0, &vec0 );
		VEC_Normalize( &vec1, &vec1 );

		cos = VEC_DotProduct( &vec0, &vec1 );
		camera->angle_pitch = FX_AcosIdx( cos );
	}
	
	FIELD_CAMERA_SetTargetPos(camera, &trans);

	FIELD_CAMERA_SetFar(camera, 0x480000);
	FIELD_CAMERA_SetNear(camera, 0x64000);
	camera->watch_target = NULL;

  // ターゲット補正値
  VEC_Set( &camera->target_offset, 0, 0, 0 );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initBridgeParameter(FIELD_CAMERA * camera)
{
	enum { 
		CAMERA_LENGTH = 0x90000,
		CAMERA_HEIGHT = 0x3a000,
	};


	// 距離
	camera->angle_len = FX_Mul( CAMERA_LENGTH, CAMERA_LENGTH ) + FX_Mul( CAMERA_HEIGHT, CAMERA_HEIGHT );
	camera->angle_len = FX_Sqrt( camera->angle_len );

	// X軸の角度
	{
		fx32 cos;
		VecFx32 vec0 = { 0,0,FX32_ONE };
		VecFx32 vec1 = { 0 };

		vec1.z = CAMERA_LENGTH;
		vec1.y = CAMERA_HEIGHT;

		VEC_Normalize( &vec0, &vec0 );
		VEC_Normalize( &vec1, &vec1 );

		cos = VEC_DotProduct( &vec0, &vec1 );
		camera->angle_pitch = FX_AcosIdx( cos );
	}

	FIELD_CAMERA_SetFar(camera, 4096 << FX32_SHIFT );

  // ターゲット補正値
  VEC_Set( &camera->target_offset, 0, 0x4000, 0 );
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
static void updateAngleCamera(FIELD_CAMERA * camera)
{ 
  enum {  PITCH_LIMIT = 0x200 };
  VecFx32 cameraPos;
	VecFx32 cameraTarget = {0};
	fx16 sinYaw = FX_SinIdx(camera->angle_yaw);
	fx16 cosYaw = FX_CosIdx(camera->angle_yaw);
	fx16 sinPitch = FX_SinIdx(camera->angle_pitch);
	fx16 cosPitch = FX_CosIdx(camera->angle_pitch);

	if(cosPitch < 0){ cosPitch = -cosPitch; }	// 裏周りしないようにマイナス値はプラス値に補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }	// 0値近辺は不正表示になるため補正

	// カメラの座標計算
	VEC_Set( &cameraPos, FX_Mul(sinYaw, cosPitch), sinPitch, FX_Mul(cosYaw, cosPitch) );
	VEC_Normalize(&cameraPos, &cameraPos);
	VEC_MultAdd( camera->angle_len, &cameraPos, &camera->target, &cameraPos );
  //cameraPos = cameraPos * length + camera->target
	
	// カメラターゲット補正
  VEC_Add( &camera->target, &camera->target_offset, &cameraTarget );

	GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &cameraTarget );
	GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &cameraPos );
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void updateCameraCalc(FIELD_CAMERA * camera)
{ 
   updateAngleCamera(camera);
}
//------------------------------------------------------------------
/**
 * @brief	デバッグのためのキー制御
 */
//------------------------------------------------------------------
static void debugControl( FIELD_CAMERA * camera, int key)
{
	VecFx32	vecMove = { 0, 0, 0 };
	VecFx32	vecUD = { 0, 0, 0 };
	BOOL	mvFlag = FALSE;
	if( key & PAD_BUTTON_R ){
		camera->angle_yaw -= RT_SPEED/2;
	}
	if( key & PAD_BUTTON_L ){
		camera->angle_yaw += RT_SPEED/2;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ControlGridParameter(FIELD_CAMERA * camera, u16 key_cont)
{
	updateTargetBinding(camera);
  updateCameraCalc(camera);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ControlBridgeParameter(FIELD_CAMERA * camera, u16 key_cont)
{
	debugControl(camera, key_cont);
	updateTargetBinding(camera);
  updateCameraCalc(camera);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ControlC3Parameter(FIELD_CAMERA * camera, u16 key_cont)
{
	updateTargetBinding(camera);
  updateCameraCalc(camera);
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
	camera->watch_target = watch_target;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera)
{
	camera->watch_target = NULL;
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
	camera->target = *target;
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
	camera->angle_yaw = angle;
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
	camera->angle_len = length;
}



#ifdef  PM_DEBUG
#include "test/camera_adjust_view.h"
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param)
{ 
  GFL_CAMADJUST * gflCamAdjust = param;
  GFL_CAMADJUST_SetCameraParam(gflCamAdjust,
      &camera->angle_yaw, &camera->angle_pitch, &camera->angle_len);

}

void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera)
{ 
}
#endif  //PM_DEBUG

