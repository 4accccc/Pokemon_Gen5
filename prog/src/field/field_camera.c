//============================================================================================
/**
 * @file	field_camera.c
 * @brief	フィールドカメラ制御
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
	HEAPID				heapID;
	FIELD_MAIN_WORK * fieldWork;
	GFL_G3D_CAMERA * g3Dcamera;

	VecFx32				pos;
	fx32				cameraHeight;
	u16					cameraLength;
	u16					direction;
	
	VecFx32				transOffset;

	const VecFx32 *		watch_pos;
	const VecFx32 *		watch_ofs;
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

#define	CAMERA_LENGTH	(16)
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
FIELD_CAMERA* FIELD_CAMERA_Create(FIELD_MAIN_WORK * fieldWork, GFL_G3D_CAMERA * cam, HEAPID heapID)
{
	FIELD_CAMERA* camera = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_CAMERA) );

	GF_ASSERT(cam != NULL);
	camera->heapID = heapID;
	camera->fieldWork = fieldWork;
	camera->g3Dcamera = cam;

	VEC_Set( &camera->pos, 0, 0, 0 );
	camera->cameraHeight = 0;
	camera->cameraLength = CAMERA_LENGTH;
	camera->direction = 0;

	camera->watch_pos = NULL;
	camera->watch_ofs = NULL;

	{
		fx32 far = 1024 << FX32_SHIFT;

		GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &far );
	}
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
//------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA * camera, int key)
{
	VecFx32	vecMove = { 0, 0, 0 };
	VecFx32	vecUD = { 0, 0, 0 };
	BOOL	mvFlag = FALSE;
	if( key & PAD_BUTTON_R ){
		camera->direction -= RT_SPEED/2;
	}
	if( key & PAD_BUTTON_L ){
		camera->direction += RT_SPEED/2;
	}
#if 0
	if( key & PAD_BUTTON_B ){
		if( camera->cameraLength > 8 ){
			camera->cameraLength -= 8;
		}
		//vecMove.y = -MV_SPEED;
	}
	if( key & PAD_BUTTON_A ){
		if( camera->cameraLength < 4096 ){
			camera->cameraLength += 8;
		}
		//vecMove.y = MV_SPEED;
	}
	if( key & PAD_BUTTON_Y ){
		camera->cameraHeight -= MV_SPEED;
	}
	if( key & PAD_BUTTON_X ){
		camera->cameraHeight += MV_SPEED;
	}
#endif
}

//------------------------------------------------------------------
/**
 * @brief	メイン
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_Main( FIELD_CAMERA* camera )
{
	VecFx32	pos, target;

	if (camera->watch_pos) {
		(camera->pos) = *(camera->watch_pos);
	}
	if (camera->watch_ofs) {
		(camera->transOffset) = *(camera->watch_ofs);
	}
	
	pos = camera->pos;
	pos.x += camera->transOffset.x;
	pos.y += camera->transOffset.y;
	pos.z += camera->transOffset.z;

	VEC_Set( &target,
			pos.x,
			pos.y + CAMERA_TARGET_HEIGHT*FX32_ONE,
			pos.z);

	pos.x = pos.x + camera->cameraLength * FX_SinIdx(camera->direction);
	pos.y = pos.y + camera->cameraHeight;
	pos.z = pos.z + camera->cameraLength * FX_CosIdx(camera->direction);

	GFL_G3D_CAMERA_SetTarget( camera->g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( camera->g3Dcamera, &pos );
}



//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera)
{
	return camera->g3Dcamera;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const void FIELD_CAMERA_InitPositionWatcher(FIELD_CAMERA * camera, const VecFx32 * watch_pos)
{
	camera->watch_pos = watch_pos;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const void FIELD_CAMERA_InitOffsetWatcher(FIELD_CAMERA * camera, const VecFx32 * watch_ofs)
{
	camera->watch_ofs = watch_ofs;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_CAMERA_SetNormalCameraMode(FIELD_CAMERA * camera, const VecFx32 * watch_pos)
{
	FIELD_CAMERA_InitPositionWatcher(camera, watch_pos);
	FIELD_CAMERA_InitOffsetWatcher(camera, NULL);
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
void	FIELD_CAMERA_SetPos( FIELD_CAMERA* camera, const VecFx32* pos )
{
	camera->pos = *pos;
}
//------------------------------------------------------------------
/**
 * @brief	カメラ位置の取得
 * @param	camera		FIELDカメラ制御ポインタ
 * @param	pos			カメラ位置を受け取るVecFx32へのポインタ
 */
//------------------------------------------------------------------
void	FIELD_CAMERA_GetPos( const FIELD_CAMERA* camera, VecFx32* pos )
{
	*pos = camera->pos;
}

void	FLD_SetCameraDirection( FIELD_CAMERA* camera, u16* direction )
{
	camera->direction = *direction;
}
void	FLD_GetCameraDirection( FIELD_CAMERA* camera, u16* direction )
{
	*direction = camera->direction;
}

void	FLD_SetCameraLength( FIELD_CAMERA *camera, u16 leng )
{
	camera->cameraLength = leng;
}
void	FLD_GetCameraLength( FIELD_CAMERA *camera, u16 *leng )
{
	*leng = camera->cameraLength;
}
void	FLD_SetCameraHeight( FIELD_CAMERA *camera, fx32 height )
{
	camera->cameraHeight = height;
}
void	FLD_GetCameraHeight( FIELD_CAMERA *camera, fx32 *height )
{
	*height = camera->cameraHeight;
}


