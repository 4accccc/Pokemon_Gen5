//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_snd.h
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
#define KEYSYS_SE_SELCT       SEQ_SE_SELECT1  //カーソル移動音
#define KEYSYS_SE_DECIDE      SEQ_SE_DECIDE1    //決定音
#define KEYSYS_SE_CANCEL      SEQ_SE_CANCEL1  //キャンセル音

#define KEYSYS_SE_NET         SEQ_SE_SW_KEYSYS_01	//通信中のループ音
#define KEYSYS_SE_CONNECT_OK  SEQ_SE_DECIDE1    	//接続完了
#define KEYSYS_SE_FALLKEY     SEQ_SE_SW_KEYSYS_02	//鍵が落ちてくる
#define KEYSYS_SE_OPENKEY     SEQ_SE_SW_KEYSYS_03	//鍵が鍵穴に入る
