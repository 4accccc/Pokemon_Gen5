#pragma once

typedef struct _FIELD_CAMERA	FIELD_CAMERA;
extern FIELD_CAMERA*		FLD_CreateCamera( FIELD_SETUP*	gs, HEAPID heapID );
extern void				FLD_DeleteCamera( FIELD_CAMERA* camera );
extern void				FLD_MainCamera( FIELD_CAMERA* camera, int key);
//extern void             FriendCursor( FIELD_CAMERA* camera );
extern void				FLD_SetCameraTrans( FIELD_CAMERA* camera, const VecFx32* trans );
extern void				FLD_GetCameraTrans( FIELD_CAMERA* camera, VecFx32* trans );
extern void				FLD_SetCameraDirection( FIELD_CAMERA* camera, u16* direction );
extern void				FLD_GetCameraDirection( FIELD_CAMERA* camera, u16* direction );

extern void	FLD_SetCameraLength( FIELD_CAMERA *camera, u16 leng );
extern void	FLD_SetCameraHeight( FIELD_CAMERA *camera, fx32 height );
