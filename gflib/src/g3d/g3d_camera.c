//=============================================================================================
/**
 * @file	g3d_camera.c                                                  
 * @brief	カメラ設定
 * @date	
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
struct _GFL_G3D_CAMERA {
	GFL_G3D_PROJECTION	projection; 
	GFL_G3D_LOOKAT		lookAt; 
};

//=============================================================================================
/**
 *
 *
 * 管理システム
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * カメラ作成
 *
 * @param	type		射影タイプ
 * @param	param1		パラメータ１（射影タイプによって異なる。g3d_system.h 参照）
 * @param	param2		パラメータ２（射影タイプによって異なる。g3d_system.h 参照）
 * @param	param3		パラメータ３（射影タイプによって異なる。g3d_system.h 参照）
 * @param	param4		パラメータ４（射影タイプによって異なる。g3d_system.h 参照）
 * @param	near		視点からnearクリップ面までの距離	
 * @param	far			視点からfarクリップ面までの距離	
 * @param	scaleW		ビューボリュームの精度調整パラメータ（使用しないときは0）
 * @param	camPos		カメラ位置ベクトルポインタ
 * @param	camUp		カメラの上方向のベクトルへのポインタ
 * @param	target		カメラ焦点へのポインタ
 *
 * @return	g3Dcamera	カメラハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_CAMERA*
	GFL_G3D_CameraCreate
		( const GFL_G3D_PROJECTION_TYPE type, 
			const fx32 param1, const fx32 param2, const fx32 param3, const fx32 param4, 
				const fx32 near, const fx32 far, const fx32 scaleW,
					const VecFx32* camPos, const VecFx32* camUp, const VecFx32* target,
						HEAPID heapID )
{
	GFL_G3D_CAMERA*	g3Dcamera;
	//管理領域確保
	g3Dcamera = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_CAMERA) );

	g3Dcamera->projection.type		= type;
	g3Dcamera->projection.param1	= param1;
	g3Dcamera->projection.param2	= param2;
	g3Dcamera->projection.param3	= param3;
	g3Dcamera->projection.param4	= param4;
	g3Dcamera->projection.near		= near;
	g3Dcamera->projection.far		= far;
	g3Dcamera->projection.scaleW	= scaleW;
	g3Dcamera->lookAt.camPos		= *camPos;
	g3Dcamera->lookAt.camUp			= *camUp;
	g3Dcamera->lookAt.target		= *target;

	return g3Dcamera;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ破棄
 *
 * @param	g3Dcamera	カメラハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraDelete
		( GFL_G3D_CAMERA* g3Dcamera )
{
	GF_ASSERT( g3Dcamera );

	GFL_HEAP_FreeMemory( g3Dcamera );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ反映
 *
 * @param	g3Dcamera	カメラハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraSwitching
		( GFL_G3D_CAMERA* g3Dcamera )
{
	GF_ASSERT( g3Dcamera );

	//射影設定
	GFL_G3D_sysProjectionSet(	g3Dcamera->projection.type,
								g3Dcamera->projection.param1, g3Dcamera->projection.param2,
								g3Dcamera->projection.param3, g3Dcamera->projection.param4,
								g3Dcamera->projection.near, g3Dcamera->projection.far,
								g3Dcamera->projection.scaleW );
	//カメラ行列設定
	GFL_G3D_sysLookAtSet(	&g3Dcamera->lookAt.camPos, 
							&g3Dcamera->lookAt.camUp,
							&g3Dcamera->lookAt.target );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ座標の取得と変更
 *
 * @param	g3Dcamera	カメラハンドル
 * @param	pos			座標ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraPosGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camPos )
{
	GF_ASSERT( g3Dcamera );
	*camPos = g3Dcamera->lookAt.camPos;
}

void
	GFL_G3D_CameraPosSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camPos )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->lookAt.camPos = *camPos;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ上方向の取得と変更
 *
 * @param	g3Dcamera	カメラハンドル
 * @param	pos			上方向ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraUpGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camUp )
{
	GF_ASSERT( g3Dcamera );
	*camUp = g3Dcamera->lookAt.camUp;
}

void
	GFL_G3D_CameraUpSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camUp )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->lookAt.camUp = *camUp;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ注視点の取得と変更
 *
 * @param	g3Dcamera	カメラハンドル
 * @param	target		注視点ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraTargetGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	*target = g3Dcamera->lookAt.target;
}

void
	GFL_G3D_CameraTargetSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->lookAt.target = *target;
}

