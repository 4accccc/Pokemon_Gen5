
//============================================================================================
/**
 * @file	btl_camera_def.h
 * @brief	êÌì¨âÊñ ÉJÉÅÉâêßå‰
 * @author	soga
 * @date	2008.11.20
 */
//============================================================================================

#ifndef	__BTL_CAMERA_DEF_H_
#define	__BTL_CAMERA_DEF_H_

#define	PHI_MAX				( ( 65536 / 360 ) * 89 )

#define	CAMERA_POS_MOVE_FLAG		( 0x00000001 )
#define	CAMERA_TARGET_MOVE_FLAG		( 0x00000002 )

#define	CAMERA_POS_MOVE_FLAG_OFF	( 0x00000001 ^ 0xffffffff )
#define	CAMERA_TARGET_MOVE_FLAG_OFF	( 0x00000002 ^ 0xffffffff )

#endif	//__BTL_CAMERA_DEF_H_
