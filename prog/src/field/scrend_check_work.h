//======================================================================
/**
 * @file	scrend_check_work.h
 * @brief	スクリプト終了時チェック構造体定義
 * @author	NozomuSaito
 *
 */
//======================================================================

#pragma once

typedef struct SCREND_CHECK_WK_tag
{
  int CameraEndCheck:1;
  int rest:31;
}SCREND_CHECK_WK;
