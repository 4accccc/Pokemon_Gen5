//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_snd.h
 *	@brief  サウンド定義ヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2010.02.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "sound/wb_sound_data.sadl"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//ランダムマッチ
#define WBM_SND_SEQ_BATTLE_RND  (SEQ_BGM_VS_TRAINER_WIFI)
#define WBM_SND_SEQ_BATTLE_RND_WIN  (SEQ_BGM_WIN2)

//WCS予選
#define WBM_SND_BGM_NORMAL      (SEQ_BGM_VS_SUBWAY_TRAINER)
#define WBM_SND_BGM_NORMAL_WIN  (SEQ_BGM_WIN2)

//WCS本戦
#define WBM_SND_BGM_FAINAL      (SEQ_BGM_VS_WCS)
#define WBM_SND_BGM_FAINAL_WIN  (SEQ_BGM_WIN5)

//大会、ランダムマッチフロー中の曲
#define WBM_SND_SEQ_MAIN        (SEQ_BGM_WCS)

//大会、ランダムマッチフロー中のSE
#define WBM_SND_SE_MATCHING_OK  (SEQ_SE_SYS_74)
#define WBM_SND_SE_CARD_SLIDE   (SEQ_SE_SYS_75)
#define WBM_SND_SE_SAVE         (SEQ_SE_SAVE)
#define WBM_SND_SE_MATCHING     (SEQ_SE_SYS_102)
