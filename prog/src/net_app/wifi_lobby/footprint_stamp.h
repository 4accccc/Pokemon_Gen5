//==============================================================================
/**
 * @file	footprint_stamp.h
 * @brief	スタンプ動作のヘッダ
 * @author	matsuda
 * @date	2008.01.20(日)
 */
//==============================================================================
#ifndef __FOOTPRINT_STAMP_H__
#define __FOOTPRINT_STAMP_H__


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void StampSys_Init(STAMP_SYSTEM_WORK *ssw, BOOL arceus_flg, fx32 *camera_distance, FOOT_CAMERA_ANGLE *camera_angle, VecFx32 *camera_target, VecFx32 *camera_up);
extern void StampSys_Exit(STAMP_SYSTEM_WORK *ssw);
extern BOOL StampSys_Add(FOOTPRINT_SYS_PTR fps, STAMP_SYSTEM_WORK *ssw, const STAMP_PARAM *param, ARCHANDLE *hdl_main, ARCHANDLE *hdl_mark, BOOL arceus_flg);
extern void StampSys_Update(STAMP_SYSTEM_WORK *ssw, GFL_G3D_CAMERA * camera_ptr, 
	int game_status, int board_type);
extern void StampSys_ObjDraw(STAMP_SYSTEM_WORK *ssw);
extern void StampSys_HitCheck(STAMP_SYSTEM_WORK *ssw);
extern void StampSys_VWaitUpdate(STAMP_SYSTEM_WORK *ssw, int game_status);

extern void Foot_SetCamPosByTarget_Dist_Ang(VecFx32 *pos, FOOT_CAMERA_ANGLE *angle, fx32 distance, VecFx32 *target);



#endif	//__FOOTPRINT_STAMP_H__
