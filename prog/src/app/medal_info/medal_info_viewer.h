//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		medal_info_viewer.h
//	 @brief  メダル詳細画面モジュール
//   @author	Masayuki Onoue
//   @date		2011.07.21
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
//
//					定数宣言
//
//=============================================================================

//-------------------------------------
///	終了モード
//=====================================
typedef enum 
{
  MEDALVIEWER_RESULT_RETURN,       ///<戻るボタンでの終了
  MEDALVIEWER_RESULT_EXIT,         ///<キャンセルボタンでの終了
} MEDALVIEWER_RESULT;

//-------------------------------------
///	呼び出しモード
//=====================================
typedef enum 
{
  MEDALVIEWER_CALL_TOP,  // 直接呼出し(戻るボタンなし)
  MEDALVIEWER_CALL_SUB,  // 戻る先がある場合(トレーナーカードなどからの呼び出し)
} MEDALVIEWER_CALL_MODE;

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

// 公開
typedef struct MEDALVIEWER_WK_tag MEDALVIEWER_WK;

//-------------------------------------
///	表示メダル情報
//=====================================
typedef struct {
  MedalID medal_id; // 表示するメダルID
  STRBUF *p_trainer_name;
  u16     medal_cnt;    // 所持しているメダル数
  u8      box_rank;     // メダルボックスのランク
  u8      get_year;     // 取得年
  u8      get_month;    // 取得月
  u8      get_day;      // 取得日
} MEDAL_INFO;

//-------------------------------------
///	メダル詳細画面モジュール初期化構造体
//=====================================
typedef struct MEDALVIEWER_INITPARAM_tag{
  HEAPID      heapID;
  MEDAL_INFO  *p_info_list;
  u32         list_size;
  u32         target;
  MEDALVIEWER_CALL_MODE call_mode;  // 呼び出しモード
}MEDALVIEWER_INITPARAM;

extern MEDALVIEWER_WK* MEDALVIEWER_Setup( MEDALVIEWER_INITPARAM *p_init_param );
extern GFL_PROC_RESULT MEDALVIEWER_Main( MEDALVIEWER_WK *p_wk );
extern u32 MEDALVIEWER_Delete( MEDALVIEWER_WK *p_wk );


