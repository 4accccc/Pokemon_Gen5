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
#include "gamesystem/link_festival_log.h"

///LINK_FESTIVAL->tmap_rootメンバの初期無効値
#define LINK_FESTIVAL_TMAP_ROOT_NONE  (0xFFFF)

//==============================================================================
//  構造体定義
//==============================================================================


///リンクフェスティバル　管理領域
struct _LINK_FESTIVAL{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  
  u8  live_f:1;             ///<ミッションアップデートの有効無効フラグ
  u8  top_score_update_f:1; ///<トップスコアアップデートフラグ
  
  u8  log_num;  //スコアログ登録数
  u8  log_idx;  //スコアログリング参照トップindex
  
  u16 beacon_wait;  ///フェスビーコン送信制御frame
  u16 beacon_timer; //frame
  
  u16 finfo_total_score;  //フィールドで表示しているトータルスコア
  u16 finfo_my_score;     //フィールドで表示しているマイスコア
  u16 finfo_bonus;  //フィールドで表示しているタイムボーナス

  u16 tmap_root;    //現在のルートゾーンID
  u16 now_score;    //現在のスコア

  //ミッションデータアーカイブハンドル
  ARCHANDLE *fm_data_h;

  //ミッションアーカイブ常駐データ
  FMISSION_ARC_COMMON*  p_arc_data;

  FEST_MISSION  mission;            //現ミッションデータ
  FEST_BEACON*  m_beacon;           //現ミッションデータ ビーコン構造体へのアクセスポインタ
  FMISSION_ARC_COMMON*  m_arc;      //現ミッションデータ コモンアーカイブデータ構造体へのアクセスポインタ

  //スコアログ
  LINKFEST_LOG  log[LINKFEST_LOG_MAX];
  LINKFEST_LOG  last;  //最後のミッションログ
  LINKFEST_LOG_TOP_SCORE*  top_score;  //トップスコア保存用データ構造体

  struct  _FMISSION_LOG_TBL mlog_view_tbl;   //ミッションログ表示バッファ
  struct  _FMISSION_LOG_TBL mlog_stack_tbl;  //ミッションログスタックバッファ

  struct  _FMISSION_GPOWER_TBL  mlog_gpower_tbl;  //ミッションログ　デルパワーストックテーブル
  struct  _FMISSION_ENTRY_TBL   mlog_entry_tbl;   //ミッションログ　エントリーデータストックテーブル

  u8  m_gmk_work[FMISSION_WORK_AREA_LEN];

#ifdef PM_DEBUG
  u8  deb_time_stop_f:1;
  u8  deb_spot_mode_f:1;
  u8  deb_log_stack_check_throw_f:1;
#endif
};

