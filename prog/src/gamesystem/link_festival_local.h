//==============================================================================
/**
 * @file    link_festival_local.h
 * @brief   リンクフェスティバル
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#pragma once

//==============================================================================
//  構造体定義
//==============================================================================
///リンクフェスティバル　管理領域
struct _LINK_FESTIVAL{
  GAMESYS_WORK* gsys;

  FEST_MISSION  mission;      //現ミッションデータ
  u8            live_f;       //ミッションアップデートの有効無効フラグ
  u16           ready_timer;  //ミッション開始前タイマー

  u32 beacon_wait;  ///frame
  u32 beacon_timer; //frame

  u16 log_num;
  u16 log_idx;
  LINKFEST_LOG  log[LINKFEST_LOG_MAX];
  LINKFEST_LOG  last;  //最後のミッションログ

  u16 finfo_score;  //フィールドで表示しているスコア
  u16 finfo_bonus;  //フィールドで表示しているタイムボーナス
};

