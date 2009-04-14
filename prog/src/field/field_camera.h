//============================================================================================
/**
 *	@file	field_camera.h
 *	@brief
 */
//============================================================================================
#pragma once

//------------------------------------------------------------------
/**
 * @brief	フィールドカメラ制御構造体への不完全型定義
 */
//------------------------------------------------------------------
typedef struct _FIELD_CAMERA	FIELD_CAMERA;

//------------------------------------------------------------------
/**
 * @brief	フィールドカメラタイプ
 */
//------------------------------------------------------------------
typedef enum {
	FIELD_CAMERA_TYPE_GRID,
	FIELD_CAMERA_TYPE_H01,
	FIELD_CAMERA_TYPE_C03,

	FIELD_CAMERA_TYPE_MAX,
}FIELD_CAMERA_TYPE;


//============================================================================================
//	生成・メイン・消去
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_CAMERA* FIELD_CAMERA_Create(
		FIELD_MAIN_WORK * fieldWork,
		FIELD_CAMERA_TYPE type,
		GFL_G3D_CAMERA * cam,
		const VecFx32 * target,
		HEAPID heapID);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_Delete( FIELD_CAMERA* camera );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_Main( FIELD_CAMERA* camera, u16 key_cont);


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetNear(FIELD_CAMERA * camera, fx32 near);
extern fx32 FIELD_CAMERA_GetNear(const FIELD_CAMERA * camera);
extern void FIELD_CAMERA_SetFar(FIELD_CAMERA * camera, fx32 far);
extern fx32 FIELD_CAMERA_GetFar(const FIELD_CAMERA * camera);

extern u16 FIELD_CAMERA_GetDirectionOnXZ(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetDirectionOnXZ(FIELD_CAMERA * camera, u16 dir);
extern void	FIELD_CAMERA_SetLengthOnXZ( FIELD_CAMERA *camera, u16 leng );
extern u16	FIELD_CAMERA_GetLengthOnXZ(const FIELD_CAMERA *camera);
extern void	FIELD_CAMERA_SetHeightOnXZ( FIELD_CAMERA *camera, fx32 height );
extern fx32	FIELD_CAMERA_GetHeightOnXZ( const FIELD_CAMERA *camera );

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ注視点を受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_GetTargetPos( const FIELD_CAMERA* camera, VecFx32* pos );

//------------------------------------------------------------------
/**
 * @brief	カメラ注視点の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ注視点を渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_SetTargetPos( FIELD_CAMERA* camera, const VecFx32* target );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_BindTarget(FIELD_CAMERA * camera, const VecFx32 * watch_target);



