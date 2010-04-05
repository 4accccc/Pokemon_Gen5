//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_snd.h
 *	@brief  バトルレコーダーサウンド定義
 *	@author	Toru=Nagihashi
 *	@date		2010.03.29
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
#define BR_SND_BGM_MAIN	      SEQ_BGM_GLTERMINAL	    //メインBGM

#define BR_SND_SE_TOUCHHERE	  SEQ_SE_BREC_02	//画面に滴が落ちる音（touch here）
#define BR_SND_SE_BOOT	      SEQ_SE_BREC_03	//起動音
#define BR_SND_SE_OK	        SEQ_SE_BREC_06	//タッチOK
#define BR_SND_SE_NG	        SEQ_SE_BREC_07	//タッチNG
#define BR_SND_SE_SLIDE	      SEQ_SE_BREC_01	//スクロールバーを動かす
#define BR_SND_SE_SEARCH 	    SEQ_SE_BREC_08	//データ検索中・送信中・受信中（ループ音）
#define BR_SND_SE_SEARCH_OK	  SEQ_SE_BREC_04	//データ送信／受信・完了
#define BR_SND_SE_CURSOR    	SEQ_SE_BREC_05	//滴がカーソルになるとき

