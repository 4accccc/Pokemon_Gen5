//============================================================================================
/**
 * @file    corporate.h
 * @brief   会社名表示
 * @author  Hiroyuki Nakamura
 * @date    2009.12.16
 */
//============================================================================================
#pragma once


#ifdef  PM_DEBUG
// 終了モード
enum {
  CORPORATE_RET_NORMAL = 0,   // 通常終了
  CORPORATE_RET_SKIP,         // スキップ
  CORPORATE_RET_DEBUG,        // デバッグへ
};
#endif

#define ALL_COPORATE_VIEW  ( 0 )  // 任天堂・（株）ポケモンも表示
#define ONLY_DEVELOP_VIEW  ( 1 )  // 権利者・開発のみ表示

// 起動時パラメータ
typedef struct{
  int mode;   // 起動モード（ALL_CORPORATE_VIEW or ONLY_DEVELOP_VIEW)
  int ret;    // デバッグ時は戻り値が入る
} CORP_PARAM;

// PROCデータ
extern const GFL_PROC_DATA CorpProcData;
