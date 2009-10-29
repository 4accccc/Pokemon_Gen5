//============================================================================================
/**
 * @file	field_gimmick_def.h
 * @brief	マップ固有の仕掛けのIDを定義するファイル
 * @author	saito
 * @date	2006.02.09
 *
 */
//============================================================================================
#pragma once

typedef enum{
	FLD_GIMMICK_NONE,			//0:なし
  FLD_GIMMICK_TEST,
  FLD_GIMMICK_H01,
  FLD_GIMMICK_GYM_ELEC,
  FLD_GIMMICK_GYM_NORM,
  FLD_GIMMICK_GYM_ANTI,
  FLD_GIMMICK_GYM_FLY,
  FLD_GIMMICK_H03,
  FLD_GIMMICK_GATE,

  FLD_GIMMICK_MAX,			//ギミック最大数(ギミック無しを含めた数)

}FLD_GYMMICK_ID;


