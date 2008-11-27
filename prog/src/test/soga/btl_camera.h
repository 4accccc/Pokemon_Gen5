
//============================================================================================
/**
 * @file	btl_camera.h
 * @brief	êÌì¨âÊñ ÉJÉÅÉâêßå‰
 * @author	soga
 * @date	2008.11.20
 */
//============================================================================================

#ifndef	__BTL_CAMERA_H_
#define	__BTL_CAMERA_H_

typedef struct _BTL_CAMERA_WORK BTL_CAMERA_WORK;

extern	BTL_CAMERA_WORK	*BTL_CAMERA_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
extern	void			BTL_CAMERA_Exit( BTL_CAMERA_WORK *bcw );
extern	void			BTL_CAMERA_Main( BTL_CAMERA_WORK *bcw );
extern	void			BTL_CAMERA_MoveCameraPosition( BTL_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target );
extern	void			BTL_CAMERA_MoveCameraAngle( BTL_CAMERA_WORK *bcw, int phi, int theta );
extern	void			BTL_CAMERA_MoveCameraInterpolation( BTL_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target, int flame, int brake );
extern	void			BTL_CAMERA_GetCameraPosition( BTL_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target );
extern	void			BTL_CAMERA_GetDefaultCameraPosition( VecFx32 *pos, VecFx32 *target );
extern	BOOL			BTL_CAMERA_CheckExecute( BTL_CAMERA_WORK *bcw );

#endif	//__BTL_CAMERA_H_
