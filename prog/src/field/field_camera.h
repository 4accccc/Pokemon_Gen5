//============================================================================================
/**
 *	@file	field_camera.h
 *	@brief
 */
//============================================================================================
#pragma once

#include "../../../../resource/fldmapdata/camera_data/fieldcameraformat.h"


//------------------------------------------------------------------
/**
 * @brief	フィールドカメラ制御構造体への不完全型定義
 */
//------------------------------------------------------------------
typedef struct _FIELD_CAMERA	FIELD_CAMERA;


//カメラトレースマスク定義
#define CAM_TRACE_MASK_X	(1)
#define CAM_TRACE_MASK_Y	(2)
#define CAM_TRACE_MASK_Z	(4)
#define CAM_TRACE_MASK_ALL	(CAM_TRACE_MASK_X|CAM_TRACE_MASK_Y|CAM_TRACE_MASK_Z)


//-----------------------------------------------------------------------------
/**
 *			フィールドカメラモード
 */
//-----------------------------------------------------------------------------
typedef enum
{
	FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定

	FIELD_CAMERA_MODE_MAX,								// モード最大数（システム内で使用）
} FIELD_CAMERA_MODE;


//============================================================================================
//	生成・メイン・消去
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern FIELD_CAMERA* FIELD_CAMERA_Create(
		u8 type,
		FIELD_CAMERA_MODE mode,
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
//	カメラモードの変更
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetMode( FIELD_CAMERA * camera, FIELD_CAMERA_MODE mode );
extern FIELD_CAMERA_MODE FIELD_CAMERA_GetMode( const FIELD_CAMERA * camera );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void FIELD_CAMERA_SetNear(FIELD_CAMERA * camera, fx32 near);
extern fx32 FIELD_CAMERA_GetNear(const FIELD_CAMERA * camera);
extern void FIELD_CAMERA_SetFar(FIELD_CAMERA * camera, fx32 far);
extern fx32 FIELD_CAMERA_GetFar(const FIELD_CAMERA * camera);


//-----------------------------------------------------------------------------
/**
 *			angle操作
 *	有効カメラモード
 *					FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
 *					FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
 *
 *	『FIELD_CAMERA_MODE_CALC_CAMERA_POS』
 *		ターゲット座標から見た、カメラ座標のアングル設定
 *
 *	『FIELD_CAMERA_MODE_CALC_TARGET_POS』
 *		カメラ座標から見た、ターゲット座標のアングル設定
 */
//-----------------------------------------------------------------------------
// ターゲットを中心とした、アングル操作
// Pitch  垂直方向回転
// Yaw    水平方向回転
extern u16 FIELD_CAMERA_GetAnglePitch(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAnglePitch(FIELD_CAMERA * camera, u16 angle );
extern u16 FIELD_CAMERA_GetAngleYaw(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAngleYaw(FIELD_CAMERA * camera, u16 angle );
extern const u16 * FIELD_CAMERA_GetAngleYawAddress( const FIELD_CAMERA *camera );
extern fx32 FIELD_CAMERA_GetAngleLen(const FIELD_CAMERA * camera );
extern void FIELD_CAMERA_SetAngleLen(FIELD_CAMERA * camera, fx32 length );

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
 * @brief	カメラ注視点の設定
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ注視点を渡すVecFx32へのポインタ
 *
 *	有効カメラモード
 *					FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
 *					FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_SetTargetPos( FIELD_CAMERA* camera, const VecFx32* target );


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		    FIELDカメラ制御ポインタ
 * @param	target_offset	カメラ注視点を補正する座標受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_GetTargetOffset( const FIELD_CAMERA* camera, VecFx32* target_offset );


//------------------------------------------------------------------
/**
 * @brief	カメラ注視点　補正座標の取得
 * @param	camera		        FIELDカメラ制御ポインタ
 * @param	target_offset			カメラ注視点を補正する座標渡すVecFx32へのポインタ
 */
//------------------------------------------------------------------
extern void	FIELD_CAMERA_SetTargetOffset( FIELD_CAMERA* camera, const VecFx32* target_offset );

//------------------------------------------------------------------
//  カメラ位置の取得・セット
//  カメラ動作状態によっては正しく反映されていないので注意
//------------------------------------------------------------------
extern void FIELD_CAMERA_GetCameraPos( const FIELD_CAMERA * camera, VecFx32 * camPos);
//
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
extern void FIELD_CAMERA_SetCameraPos( FIELD_CAMERA * camera, const VecFx32 * camPos);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera);


//------------------------------------------------------------------
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_CAMERA_POS,		// カメラ座標をターゲット座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
//------------------------------------------------------------------
extern void FIELD_CAMERA_BindTarget(FIELD_CAMERA * camera, const VecFx32 * watch_target);
extern void FIELD_CAMERA_BindDefaultTarget(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_FreeTarget(FIELD_CAMERA * camera);
extern BOOL FIELD_CAMERA_IsBindDefaultTarget( const FIELD_CAMERA * camera );

//------------------------------------------------------------------
//	カメラ座標更新の停止（デバック用？）
//	DIRECT_POSモードに変更することで対応可能
//------------------------------------------------------------------
//extern void FIELD_CAMERA_BindNoCamera(FIELD_CAMERA * camera, BOOL flag);

//------------------------------------------------------------------
//	カメラ座標のバインド
//有効カメラモード
//	FIELD_CAMERA_MODE_CALC_TARGET_POS,		// ターゲット座標をカメラ座標とアングルから計算する
//	FIELD_CAMERA_MODE_DIRECT_POS,					// カメラ座標、ターゲット座標　直接指定
//------------------------------------------------------------------
extern void FIELD_CAMERA_BindCamera(FIELD_CAMERA * camera, const VecFx32 * watch_camera);
extern void FIELD_CAMERA_FreeCamera(FIELD_CAMERA * camera);

#ifdef  PM_DEBUG
typedef enum{
  FIELD_CAMERA_DEBUG_BIND_NONE,
  FIELD_CAMERA_DEBUG_BIND_CAMERA_POS,
  FIELD_CAMERA_DEBUG_BIND_TARGET_POS,
} FIELD_CAMERA_DEBUG_BIND_TYPE;
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type);
extern void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target );


//------------------------------------------------------------------
//  デフォルトターゲット操作
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_SetDefaultTarget( FIELD_CAMERA* camera, const VecFx32* target );
extern const VecFx32* FIELD_CAMERA_DEBUG_GetDefaultTarget( const FIELD_CAMERA* camera );

extern void FIELD_CAMERA_GetInitialParameter( const FIELD_CAMERA* camera, FLD_CAMERA_PARAM * result);
#endif

