//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wifi_bsubway.h
 *	@brief  Wi-Fiバトルサブウェイ
 *	@author	tomoya takahashi
 *	@date		2010.02.17
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>
#include "gamesystem/gamesystem.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	WiFiバトルサブウェイ処理　起動モード
//=====================================
typedef enum {
  WIFI_BSUBWAY_MODE_SCORE_UPLOAD,         // 前回の記録をアップロード
  WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD,    // ゲーム情報をダウンロード
  WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD, // 歴代情報をダウンロード

  WIFI_BSUBWAY_MODE_MAX, // 処理内で使用
} WIFI_BSUBWAY_MODE;


//-------------------------------------
///	結果
//=====================================
typedef enum {
  WIFI_BSUBWAY_RESULT_OK,                   //アップデート・ダウンロード成功
  WIFI_BSUBWAY_RESULT_NG,                   //失敗


  WIFI_BSUBWAY_RESULT_MAX, //処理内で使用
} WIFI_BSUBWAY_RESULT;


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	パラメータ
//=====================================
typedef struct {

  // 設定データ
  WIFI_BSUBWAY_MODE mode;
  GAMESYS_WORK*     p_gamesystem;

  // 結果データ
  WIFI_BSUBWAY_RESULT result;

} WIFI_BSUBWAY_PARAM;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
extern const GFL_PROC_DATA WIFI_BSUBWAY_Proc;

FS_EXTERN_OVERLAY(wifi_bsubway);

#ifdef _cplusplus
}	// extern "C"{
#endif



