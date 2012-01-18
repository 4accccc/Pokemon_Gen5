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
#define KEYSYS_SE_DECIDE      SEQ_SE_DECIDE1  //決定音
#define KEYSYS_SE_CANCEL      SEQ_SE_CANCEL1  //キャンセル音
#define KEYSYS_SE_BEEP        SEQ_SE_BEEP     //ビープ音

#define KEYSYS_SE_NET         SEQ_SE_SW_KEYSYS_01	//通信中のループ音
#define KEYSYS_SE_CONNECT_OK  SEQ_SE_SYS_12    	  //接続完了
#define KEYSYS_SE_FALLKEY     SEQ_SE_SW_KEYSYS_02	//鍵が落ちてくる
#define KEYSYS_SE_OPENKEY     SEQ_SE_SW_KEYSYS_03	//鍵が鍵穴に入る
#define KEYSYS_SE_KIRAKIRA    SEQ_SE_SW_KEYSYS_04 //きらきら
#define KEYSYS_SE_SAVE        SEQ_SE_SAVE   //セーブ音

#define KEYSYS_BGM_MAIN       SEQ_BGM_DATA_CONV   //メイン曲

#define CYGNUS_SE_MOVIE_FALL   SEQ_SE_SYS_88  //光の玉が降りてくる
#define CYGNUS_SE_MOVIE_EMIT   SEQ_SE_SYS_89  //光のたまが破裂する
#define CYGNUS_SE_RECV_PRESENT SEQ_SE_SYS_50	//贈り物を受け取った完了音

#define CYGNUS_SE_ICON        SEQ_SE_MSCL_04  //アイコン着地
#define CYGNUS_SE_FINISH      SEQ_SE_SW_CYGNUS_03 //終了
