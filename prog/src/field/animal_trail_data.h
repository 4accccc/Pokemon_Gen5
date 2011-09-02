//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		animal_trail_data.h
 *	@brief  獣道用データアクセサ
 *	@author	Toru=Nagihashi
 *	@date		2011.09.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
typedef enum
{
  ANIMAL_TRAIL_DATA_PARAM_ZONEID,
  ANIMAL_TRAIL_DATA_PARAM_X,
  ANIMAL_TRAIL_DATA_PARAM_Y,
  ANIMAL_TRAIL_DATA_PARAM_Z,
  ANIMAL_TRAIL_DATA_PARAM_MAX
}ANIMAL_TRAIL_DATA_PARAM;
extern s32 ANIMAL_TRAIL_DATA_GetExitData( u32 idx, ANIMAL_TRAIL_DATA_PARAM param );
