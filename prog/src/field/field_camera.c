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

	FIELD_CAMERA_TYPE	type;
	VecFx32				pos;
	fx32				cameraHeight;
	u16					cameraLength;
	u16					direction;
	
	const VecFx32 *		watch_target;
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


typedef struct {
	void (*setup)(FIELD_CAMERA * camera);
	void (*control)(FIELD_CAMERA * camera);
}CAMERA_FUNC_TABLE;

//------------------------------------------------------------------
//------------------------------------------------------------------
static void SetupGridParameter(FIELD_CAMERA * camera);
static void SetupBridgeParameter(FIELD_CAMERA * camera);
static void SetupC3Parameter(FIELD_CAMERA * camera);


#define	CAMERA_LENGTH	(16)
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

	GF_ASSERT(cam != NULL);
	camera->heapID = heapID;
	camera->fieldWork = fieldWork;
	camera->g3Dcamera = cam;
	camera->type = type;
	TAMADA_Printf("FIELD CAMERA TYPE = %d\n", type);

	VEC_Set( &camera->pos, 0, 0, 0 );
	camera->cameraHeight = 0;
	camera->cameraLength = CAMERA_LENGTH;
	camera->direction = 0;

	camera->watch_target = target;

	switch ( (FIELD_CAMERA_TYPE)type ) {
	case FIELD_CAMERA_TYPE_GRID:
		SetupGridParameter(camera);
		break;
	case FIELD_CAMERA_TYPE_H01:
		SetupBridgeParameter(camera);
		break;
	case FIELD_CAMERA_TYPE_C03:
		SetupC3Parameter(camera);
		break;
	default:
		GF_ASSERT_MSG(0, "CAMERA TYPE ERROR! check maptable.xls\n");
	}

	return camera;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void SetupGridParameter(FIELD_CAMERA * camera)
{
	//{ 0x78, 0xd8000 },	//DPぽい
	FLD_SetCameraLength( camera, 0x0078);
	FLD_SetCameraHeight( camera, 0xd8000);
	{
		fx32 far = 1024 << FX32_SHIFT;

		GFL_G3D_CAMERA_SetFar( camera->g3Dcamera, &far );
	}
	FIELD_CAMERA_SetPos(camera, camera->watch_target);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void SetupC3Parameter(FIELD_CAMERA * camera)
{
	u16 dir = 0;
	fx32 height = 0x7c000;
	VecFx32 trans = {0x2f6f36, 0, 0x301402};

	FLD_SetCameraLength(camera, 0x0308);
	FLD_SetCameraDirection(camera, &dir);
	FLD_SetCameraHeight(camera, height);
	FIELD_CAMERA_SetPos(camera, &trans);

	//player_len = 0x1f0;
	//v_len = 1;
	//v_angle = 16;
	//pos_angle = 0;
	FIELD_CAMERA_SetFar(camera, (512 + 256 + 128) << FX32_SHIFT);
	camera->watch_target = NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void SetupBridgeParameter(FIELD_CAMERA * camera)
{
	u16 len;
	fx32 height;
	FLD_GetCameraLength(camera, &len);
	len += 0x0080;
	FLD_SetCameraLength(camera, len);
	FLD_GetCameraHeight(camera, &height);
	height += 0x0003a000;
	FLD_SetCameraHeight(camera, height);
	FIELD_CAMERA_SetFar(camera, 4096 << FX32_SHIFT );
	FIELD_CAMERA_SetPos(camera, camera->watch_target);
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

	if (camera->watch_target) {
		(camera->pos) = *(camera->watch_target);
	}
	
	pos = camera->pos;

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


