//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *  GAME FREAK inc.
 *
 *  @file   wifi_earth_snd.h
 *  @brief    世界時計  音
 *  @author   tomoya takahashi
 *  @data   2007.09.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WIFI_EARTH_SND_H__
#define __WIFI_EARTH_SND_H__

#include "sound/pm_sndsys.h"

#define WIFIEARTH_SND_ZOMEIN      ( SEQ_SE_DECIDE1 )    // ズームイン
#define WIFIEARTH_SND_ZOMEOUT     ( SEQ_SE_DECIDE1 )    // ズームアウト

#define WIFIEARTH_SND_SELECT      ( SEQ_SE_SELECT1 )    // セレクト
#define WIFIEARTH_SND_XSELECT     ( SEQ_SE_DECIDE1 )    // xセレクト

#define WIFIEARTH_SND_YAMERU      ( SEQ_SE_CANCEL1 )    // やめるをタッチ、Bキャンセル
#define WIFIEARTH_MOVE_CURSOR     ( SEQ_SE_MESSAGE )    // リスト時にカーソルを移動させた時

#endif    // __WIFI_EARTH_SND_H__

