#include <gflib.h>
#include "field_g3d_mapper.h"
#include "field_common.h"
#include "field_camera.h"
	
#include "arc_def.h"
//============================================================================================
/**
 * @brief	カーソル
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
struct _FIELD_CAMERA {
	HEAPID				heapID;
	FIELD_SETUP*		gs;

	VecFx32				trans;
	fx32				cameraHeight;
	u16					cameraLength;
	u16					direction;
	
	VecFx32				transOffset;
};


#define	CAMERA_LENGTH	(16)
//------------------------------------------------------------------
/**
 * @brief	初期化
 */
//------------------------------------------------------------------
FIELD_CAMERA*	FIELD_CAMERA_Create( FIELD_SETUP*	gs, HEAPID heapID )
{
	FIELD_CAMERA* camera = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_CAMERA) );

	camera->heapID = heapID;
	camera->gs = gs;

	VEC_Set( &camera->trans, 0, 0, 0 );
	camera->cameraHeight = 0;
	camera->cameraLength = CAMERA_LENGTH;
	camera->direction = 0;

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
//#define MV_SPEED		(2*FX32_ONE)
//#define RT_SPEED		(FX32_ONE/8)
//#define	CAMERA_TARGET_HEIGHT	(4)//(8)

void	FIELD_CAMERA_Main( FIELD_CAMERA* camera, int key )
{
	GFL_G3D_CAMERA * g3Dcamera = GetG3Dcamera(camera->gs);
	VecFx32	pos, target, trans;
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
	
	trans = camera->trans;
	trans.x += camera->transOffset.x;
	trans.y += camera->transOffset.y;
	trans.z += camera->transOffset.z;

	VEC_Set( &target,
			trans.x,
			trans.y + CAMERA_TARGET_HEIGHT*FX32_ONE,
			trans.z);

	pos.x = trans.x + camera->cameraLength * FX_SinIdx(camera->direction);
	pos.y = trans.y + camera->cameraHeight;
	pos.z = trans.z + camera->cameraLength * FX_CosIdx(camera->direction);

	GFL_G3D_CAMERA_SetTarget( g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( g3Dcamera, &pos );
}



#if 0
static void	FriendCursor( FIELD_CAMERA* camera )
{
    
    GFL_STD_MemCopy((const void*)&fieldWork->recvWork ,&camera->trans, sizeof(VecFx32));
	GFL_G3D_SCENEOBJ_SetPos(	GFL_G3D_SCENEOBJ_Get(camera->gs->g3Dscene, camera->cursorIdx), 
								&camera->trans );
}
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera)
{
	return GetG3Dcamera(camera->gs);
}

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------
void	FLD_SetCameraTrans( FIELD_CAMERA* camera, const VecFx32* trans )
{
	camera->trans = *trans;
}
void	FLD_GetCameraTrans( FIELD_CAMERA* camera, VecFx32* trans )
{
	*trans = camera->trans;
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

void	FLD_SetCameraTransOffset(
		FIELD_CAMERA *camera, const VecFx32 *offs )
{
	camera->transOffset = *offs;
}

