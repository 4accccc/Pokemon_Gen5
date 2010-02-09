//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *  GAME FREAK inc.
 *
 *  @file   field_light_status.h
 *  @brief  バトルに引き継ぐフィールドライトカラー
 *  @author tomoya takahashi
 *  @date   2010.01.29
 *
 *  モジュール名：FIELD_LIGHT_STATUS
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

//-----------------------------------------------------------------------------
/**
 *          定数宣言
*/
//-----------------------------------------------------------------------------
#define FIELD_LIGHT_STATUS_WEATHER_LIGHT_DAT_NONE ( 0xffff )

//-----------------------------------------------------------------------------
/**
 *          構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// FIELD_LIGHT_STATUS
//=====================================
typedef struct {
  GXRgb   light;
  GXRgb   diffuse;
  GXRgb   ambient;
  GXRgb   specular;
  GXRgb   emission;
  GXRgb   pad;      // padding
} FIELD_LIGHT_STATUS;

//-----------------------------------------------------------------------------
/**
 *          プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// ライトカラーの取得
extern void FIELD_LIGHT_STATUS_Get( u32 zone_id, int hour, int minute, int weather_id, int season, FIELD_LIGHT_STATUS* p_status, HEAPID heapID );
// 天気NOからライトデータのアーカイブインデックス取得
extern u16 FIELD_LIGHT_STATUS_GetWeatherLightDatIdx( int weather_id );

#ifdef _cplusplus
} // extern "C"{
#endif



