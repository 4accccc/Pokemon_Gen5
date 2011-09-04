//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_h06.h
 *	@brief  マリンチューブ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//-------------------------------------
///	時間帯の指定
//=====================================
typedef enum{
  TIME_ZONE_AM,     // 朝から昼
  TIME_ZONE_PM,     // 夕方から夜
  TIME_ZONE_RARE,   // レアタイム

  TIME_ZONE_MAX,
}TIME_ZONE;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void H06_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void H06_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void H06_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

