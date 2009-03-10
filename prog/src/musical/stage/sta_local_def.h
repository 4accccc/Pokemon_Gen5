//======================================================================
/**
 * @file	sta_local_def.h
 * @brief	ステージ用　定義
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#ifndef STA_LOCAL_DEF_H__
#define STA_LOCAL_DEF_H__

//BBD用座標変換(カメラの幅から計算
#define ACT_POS_X(val)	FX32_CONST((val)/16.0f)
#define ACT_POS_Y(val)	FX32_CONST((192.0f-(val))/16.0f)
#define ACT_POS_X_FX(val)	((val)/16)
#define ACT_POS_Y_FX(val)	(FX32_CONST(192.0f)-(val))/16

//オブジェクト関係
#define ACT_OBJECT_MAX (5)

//エフェクト関係
#define ACT_EFFECT_MAX (3)

//幕関係
#define ACT_CURTAIN_SCROLL_SPEED (2)
#define ACT_CURTAIN_SCROLL_MIN (0)
#define ACT_CURTAIN_SCROLL_MAX (224)

#endif STA_LOCAL_DEF_H__
