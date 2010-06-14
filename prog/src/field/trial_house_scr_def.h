//======================================================================
/**
 * @file  trial_house_scr_def.h
 * @brief  トライアルハウス　スクリプト使用定義
 *
 */
//======================================================================
#pragma once

#define TH_PLAYMODE_SINGLE  (0)
#define TH_PLAYMODE_DOUBLE  (1)

#define TH_DL_DATA_TYPE_NONE  (0)
#define TH_DL_DATA_TYPE_SINGLE  (1)
#define TH_DL_DATA_TYPE_DOUBLE  (2)

#define TH_RDAT_ST_NONE (0)
#define TH_RDAT_ST_ROM  (1)
#define TH_RDAT_ST_DL   (2)
#define TH_RDAT_ST_BOTH (3)

#define TH_DL_RSLT_FAIL     (0)     //失敗
#define TH_DL_RSLT_SUCCESS  (1)     //成功
#define TH_DL_RSLT_SETTLED  (2)    //済み
#define TH_DL_RSLT_CANCEL   (3)    //キャンセル

#define TH_DL_DATA_CLEAR_TYPE_CLEAR (0)     //完全クリア
#define TH_DL_DATA_CLEAR_TYPE_USED (1)      //使用したことにする

