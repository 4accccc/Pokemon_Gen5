//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_debug.h
 *	@brief  ギミック内デバッグ処理
 *	@author	Toru=Nagihashi
 *	@date		2011.09.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#ifdef PM_DEBUG
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef enum
{
  RESORT_DEBUG_FLAG_FORCE_EXCELLENT,    //強制◎
  RESORT_DEBUG_FLAG_EXP_9999,          //経験値9999

  RESORT_DEBUG_FLAG_MAX
}RESORT_DEBUG_FLAG;


//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern void RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG flag, BOOL is_on );
extern BOOL RESORT_DEBUG_FLAG_Get( RESORT_DEBUG_FLAG flag );


#endif  //PM_DEBUG
