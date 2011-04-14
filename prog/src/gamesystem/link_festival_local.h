//==============================================================================
/**
 * @file    link_festival_local.h
 * @brief   リンクフェスティバル
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#pragma once

#include "fmission_log_local.h"

//==============================================================================
//  構造体定義
//==============================================================================


///リンクフェスティバル　管理領域
struct _LINK_FESTIVAL{
  GAMESYS_WORK* gsys;

  u8  live_f;         ///<ミッションアップデートの有効無効フラグ
  u16 ready_timer;    ///<ミッション開始前タイマー
  
  u32 beacon_wait;  ///フェスビーコン送信制御frame
  u32 beacon_timer; //frame
  
  u16 finfo_score;  //フィールドで表示しているスコア
  u16 finfo_bonus;  //フィールドで表示しているタイムボーナス

  u16 tmap_root;    //現在のルートゾーンID

  u16 log_num;  //スコアログ登録数
  u16 log_idx;  //スコアログリング参照トップindex
  
  FEST_MISSION  mission;            //現ミッションデータ
//  GFL_STD_RandContext rnd_context;  //現ミッションで用いる乱数コンテキスト

  //スコアログ
  LINKFEST_LOG  log[LINKFEST_LOG_MAX];
  LINKFEST_LOG  last;  //最後のミッションログ

  struct  _FMISSION_LOG_TBL mlog_view_tbl;   //ミッションログ表示バッファ
  struct  _FMISSION_LOG_TBL mlog_stack_tbl;  //ミッションログスタックバッファ

  struct  _FMISSION_GPOWER_TBL  mlog_gpower_tbl;  //ミッションログ　デルパワーストックテーブル
};

