//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_snd.h
 *	@brief  天気サウンド　
 *	@author	tomoya takahashi
 *	@date		2009.12.12
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// 雨SE
#define WEATHER_SND_SE_RAIN   ( SEQ_SE_FLD_32 )

// 大雨SE
#define WEATHER_SND_SE_HIGHRAIN   ( SEQ_SE_FLD_31 )

// 雷SE
#define WEATHER_SND_SE_SPARK      ( SEQ_SE_FLD_33 )
#define WEATHER_SND_SE_SPARK_SML  ( SEQ_SE_FLD_34 )

// 砂嵐SE
#define WEATHER_SND_SE_STORM  ( SEQ_SE_FLD_84 )

// 吹雪SE
#define WEATHER_SND_SE_SNOWSTORM  ( SEQ_SE_FLD_84 )

// あられSE
#define WEATHER_SND_SE_ARARE  ( SEQ_SE_FLD_85 )

// カザカミ
#define WEATHER_SND_SE_KAZAKAMI_NORMAL_RAIN ( SEQ_SE_FLD_31 )         // 通常の雨
#define WEATHER_SND_SE_KAZAKAMI_NORMAL_MIDDLE_WIND ( SEQ_SE_FLD_99 )  // 中間の風
#define WEATHER_SND_SE_KAZAKAMI_MIDDLE_HIGH_WIND ( SEQ_SE_FLD_98 )    // 強い風
#define WEATHER_SND_SE_KAZAKAMI_HIGH_RAIN   ( SEQ_SE_FLD_100 )        // 強い雨


#ifdef _cplusplus
}	// extern "C"{
#endif



