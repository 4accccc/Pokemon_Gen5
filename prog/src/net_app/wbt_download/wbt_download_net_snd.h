//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_download_net_snd.h
 *	@brief  サウンド定義ファイル
 *	@author	Toru=Nagihashi
 *	@date		2011.07.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "sound/pm_sndsys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//システム音
#define WBTDLNET_SE_SELCT       SEQ_SE_SELECT1  //カーソル移動音
#define WBTDLNET_SE_DECIDE      SEQ_SE_DECIDE1  //決定音
#define WBTDLNET_SE_CANCEL      SEQ_SE_CANCEL1  //キャンセル音
#define WBTDLNET_SE_BEEP        SEQ_SE_BEEP     //ビープ音

#define WBTDLNET_BGM_MAIN       SEQ_BGM_DATA_CONV   //メイン曲
