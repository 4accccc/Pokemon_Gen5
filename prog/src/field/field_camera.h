//============================================================================================
/**
 *	@file	field_camera.h
 *	@brief
 */
//============================================================================================
#pragma once

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FIELD_CAMERA	FIELD_CAMERA;


//------------------------------------------------------------------
//	生成・メイン・消去
//------------------------------------------------------------------
extern FIELD_CAMERA*		FIELD_CAMERA_Create( FIELD_MAIN_WORK * fieldWork, HEAPID heapID );
extern void				FIELD_CAMERA_Delete( FIELD_CAMERA* camera );
extern void				FIELD_CAMERA_Main( FIELD_CAMERA* camera);

extern void FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA * camera, int key);

//------------------------------------------------------------------
//	以下はリネームされる予定の関数群
//------------------------------------------------------------------
extern void				FLD_SetCameraTrans( FIELD_CAMERA* camera, const VecFx32* trans );
extern void				FLD_GetCameraTrans( const FIELD_CAMERA* camera, VecFx32* trans );
extern void				FLD_SetCameraDirection( FIELD_CAMERA* camera, u16* direction );
extern void				FLD_GetCameraDirection( FIELD_CAMERA* camera, u16* direction );

extern void	FLD_SetCameraLength( FIELD_CAMERA *camera, u16 leng );
extern void	FLD_GetCameraLength( FIELD_CAMERA *camera, u16 *leng );
extern void	FLD_SetCameraHeight( FIELD_CAMERA *camera, fx32 height );
extern void	FLD_GetCameraHeight( FIELD_CAMERA *camera, fx32 *height );
extern void	FLD_SetCameraTransOffset(
		FIELD_CAMERA *camera, const VecFx32 *offs );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern const GFL_G3D_CAMERA * FIELD_CAMERA_GetCameraPtr(const FIELD_CAMERA * camera);
extern const void FIELD_CAMERA_InitPositionWatcher(FIELD_CAMERA * camera, const VecFx32 * watch_pos);
extern const void FIELD_CAMERA_InitOffsetWatcher(FIELD_CAMERA * camera, const VecFx32 * watch_ofs);

