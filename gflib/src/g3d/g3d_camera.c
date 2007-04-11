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
	GFL_G3D_CAMERA_Create
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
	GFL_G3D_CAMERA_Delete
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
	GFL_G3D_CAMERA_Switching
		( GFL_G3D_CAMERA* g3Dcamera )
{
	GF_ASSERT( g3Dcamera );

	//射影設定
	GFL_G3D_SetSystemProjection(	g3Dcamera->projection.type,
									g3Dcamera->projection.param1, g3Dcamera->projection.param2,
									g3Dcamera->projection.param3, g3Dcamera->projection.param4,
									g3Dcamera->projection.near, g3Dcamera->projection.far,
									g3Dcamera->projection.scaleW );
	//カメラ行列設定
	GFL_G3D_SetSystemLookAt(		&g3Dcamera->lookAt.camPos, 
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
	GFL_G3D_CAMERA_GetPos
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camPos )
{
	GF_ASSERT( g3Dcamera );
	*camPos = g3Dcamera->lookAt.camPos;
}

void
	GFL_G3D_CAMERA_SetPos
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
	GFL_G3D_CAMERA_GetCamUp
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* camUp )
{
	GF_ASSERT( g3Dcamera );
	*camUp = g3Dcamera->lookAt.camUp;
}

void
	GFL_G3D_CAMERA_SetCamUp
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
	GFL_G3D_CAMERA_GetTarget
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	*target = g3Dcamera->lookAt.target;
}

void
	GFL_G3D_CAMERA_SetTarget
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* target )
{
	GF_ASSERT( g3Dcamera );
	g3Dcamera->lookAt.target = *target;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ方向と対象位置との内積を取得する
 *
 * @param	g3Dcamera	カメラハンドル
 * @param	objPos		オブジェクト位置ベクトル
 *
 * 主に簡易カリングなどに使用する
 * 内積の値が　	0	:対象ベクトルは水平（カメラの向きに対して垂直のベクトル）に位置する
 *				正	:対象ベクトルは前方に位置する
 *				負	:対象ベクトルは後方に位置する
 */
//--------------------------------------------------------------------------------------------
//正規版
fx32
	GFL_G3D_CAMERA_GetDotProduct
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* objPos )
{
	VecFx32	viewVec, objVec;
	fx32	scalar;
			
	GF_ASSERT( g3Dcamera );

	//視界ベクトル計算
	VEC_Subtract( &g3Dcamera->lookAt.target, &g3Dcamera->lookAt.camPos, &viewVec );
	//対象物体ベクトル計算
	VEC_Subtract( objPos, &g3Dcamera->lookAt.camPos, &objVec );

	//視界ベクトルと対象物体ベクトルの内積計算
	return VEC_DotProduct( &viewVec, &objVec );
}

//簡易版　※２Ｄ（ＸＺ座標）専用。スカラー精度は落ちるがやや高速
int
	GFL_G3D_CAMERA_GetDotProductXZfast
		( GFL_G3D_CAMERA* g3Dcamera, VecFx32* objPos )
{
	int viewVecX, viewVecZ, objVecX, objVecZ;
			
	GF_ASSERT( g3Dcamera );

	//視界ベクトル計算（整数部のみ）
	viewVecX = ( g3Dcamera->lookAt.target.x - g3Dcamera->lookAt.camPos.x )/FX32_ONE;
	viewVecZ = ( g3Dcamera->lookAt.target.z - g3Dcamera->lookAt.camPos.z )/FX32_ONE;

	//対象物体ベクトル計算（整数部のみ）
	objVecX = ( objPos->x - g3Dcamera->lookAt.camPos.x )/FX32_ONE;
	objVecZ = ( objPos->z - g3Dcamera->lookAt.camPos.z )/FX32_ONE;

	//視界ベクトルと対象物体ベクトルの内積計算（ＸＺ）
	return viewVecX * objVecX + viewVecZ * objVecZ;
}

