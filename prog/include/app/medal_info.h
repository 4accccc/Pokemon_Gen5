//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_info.h
 *	@brief  メダル詳細画面
 *	@author	Masayuki Onoue
 *	@date		2011.05.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "../../../resource/medal/data/mdlid_def.h" // for MedalID
FS_EXTERN_OVERLAY(medalinfo);

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//-------------------------------------
///	呼び出しモード
//=====================================
typedef enum 
{
  MEDALINFO_CALL_TOP,  // 直接呼出し(戻るボタンなし)
  MEDALINFO_CALL_SUB,  // 戻る先がある場合(トレーナーカードなどからの呼び出し)
} MEDALINFO_CALL_MODE;

//-------------------------------------
///	終了モード
//=====================================
typedef enum 
{
  MEDALINFO_RESULT_RETURN,       ///<戻るボタンでの終了
  MEDALINFO_RESULT_EXIT,         ///<キャンセルボタンでの終了
} MEDALINFO_RESULT;

//-------------------------------------
/// PROC 初期化パラメータ
//=====================================
typedef struct _MEDALINFO_INIT_PARAM
{
  MEDALINFO_CALL_MODE call_mode;  // 呼び出しモード
	HEAPID    parent_heap_id;       // メモリ領域確保用ヒープID
  STRBUF    *p_trainer_name;      // トレーナー名
  MedalID   medal_id;             // 表示するメダルID
  u16       medal_cnt;            // 所持しているメダル数
  u8        box_rank;             // メダルボックスのランク
  u8        get_year;             // 取得年
  u8        get_month;            // 取得月
  u8        get_day;              // 取得日

  MEDALINFO_RESULT result;        // 終了方法が格納されます
}
MEDALINFO_INIT_PARAM;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
//-------------------------------------
/// PROC　データ
//=====================================
extern GFL_PROC_DATA medalInfo_ProcData;

