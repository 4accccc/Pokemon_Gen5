//======================================================================
/**
 * @file	musical_camera_def.h
 * @brief	ミュージカル3Dカメラ系設定
 * @author	ariizumi
 * @data	09/03/05
 */
//======================================================================

#ifndef MUSICAL_CAMERA_DEF_H__
#define MUSICAL_CAMERA_DEF_H__

// カメラの設定を各パートとmusical_mcssと共有する必要があるので
// ヘッダとして定義。

#define MUSICAL_CAMERA_POS_X (FX32_CONST(0.0f))
#define MUSICAL_CAMERA_POS_Y (FX32_CONST(0.0f))
#define MUSICAL_CAMERA_POS_Z (FX32_CONST(301.0f))
#define MUSICAL_CAMERA_TRG_X (FX32_CONST(0.0f))
#define MUSICAL_CAMERA_TRG_Y (FX32_CONST(0.0f))
#define MUSICAL_CAMERA_TRG_Z (FX32_CONST(0.0f))

#define MUSICAL_CAMERA_POS {MUSICAL_CAMERA_POS_X,MUSICAL_CAMERA_POS_Y,MUSICAL_CAMERA_POS_Z}
#define MUSICAL_CAMERA_TRG {MUSICAL_CAMERA_TRG_X,MUSICAL_CAMERA_TRG_Y,MUSICAL_CAMERA_TRG_Z}
#define MUSICAL_CAMERA_UP  {0,FX32_ONE,0}

#define MUSICAL_CAMERA_NEAR (FX32_ONE)
#define MUSICAL_CAMERA_FAR  (FX32_ONE*300)

//BBD用座標変換(カメラの幅から計算
#define MUSICAL_POS_X(val)		FX32_CONST((val)/16.0f)
#define MUSICAL_POS_Y(val)		FX32_CONST((192.0f-(val))/16.0f)
#define MUSICAL_POS_X_FX(val)	((val)/16)
#define MUSICAL_POS_Y_FX(val)	(FX32_CONST(192.0f)-(val))/16

#endif MUSICAL_CAMERA_DEF_H__

