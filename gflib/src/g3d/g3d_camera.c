//=============================================================================================
/**
 * @file	camera.c                                                  
 * @brief	カメラプログラム
 * @date	
 */
//=============================================================================================
#include "gflib.h"
#include "g3d_camera.h"

//=============================================================================================
//	型宣言
//=============================================================================================
typedef struct _GFL_G3D_CAMERA GFL_G3D_CAMERA; 

struct _GFL_G3D_CAMERA {
	GFL_G3D_PROJECTION_TYPE type; 
	fx32					param1;
	fx32					param2;
	fx32					param3;
	fx32					param4;
	fx32					near;
	fx32					far;
	fx32					scaleW;
	VecFx32					camPos;
	VecFx32					camUp;
	VecFx32					target;
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

	g3Dcamera->type		= type;
	g3Dcamera->param1	= param1;
	g3Dcamera->param2	= param2;
	g3Dcamera->param3	= param3;
	g3Dcamera->param4	= param4;
	g3Dcamera->near		= near;
	g3Dcamera->far		= far;
	g3Dcamera->scaleW	= scaleW;
	g3Dcamera->camPos	= *camPos;
	g3Dcamera->camUp	= *camUp;
	g3Dcamera->target	= *target;

	return g3Dcamera;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ破棄
 *
 * @param	g3Dcamera	カメラポインタ
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
 * カメラＯＮ
 *
 * @param	g3Dcamera	カメラポインタ
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraOn
		( GFL_G3D_CAMERA* g3Dcamera )
{
	GF_ASSERT( g3Dcamera );

	//射影設定
	GFL_G3D_sysProjectionSet(	g3Dcamera->type,
								g3Dcamera->param1, g3Dcamera->param2,
								g3Dcamera->param3, g3Dcamera->param4,
								g3Dcamera->near, g3Dcamera->far,
								g3Dcamera->scaleW );
	//カメラ行列設定
	GFL_G3D_sysLookAtSet( &g3Dcamera->camPos, &g3Dcamera->camUp, &g3Dcamera->target );
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ座標の取得と変更
 *
 * @param	g3Dcamera	カメラポインタ
 * @param	pos			座標ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraPosGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camPos )
{
	GF_ASSERT( g3Dcamera );
	*camPos = g3Dcamera->camPos;
}

void
	GFL_G3D_CameraPosSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camPos )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->camPos = *camPos;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ上方向の取得と変更
 *
 * @param	g3Dcamera	カメラポインタ
 * @param	pos			上方向ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraUpGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camUp )
{
	GF_ASSERT( g3Dcamera );
	*camUp = g3Dcamera->camUp;
}

void
	GFL_G3D_CameraUpSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camUp )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->camUp = *camUp;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ注視点の取得と変更
 *
 * @param	g3Dcamera	カメラポインタ
 * @param	target		注視点ベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_CameraTargetGet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	*target = g3Dcamera->target;
}

void
	GFL_G3D_CameraTargetSet
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->target = *target;
}

