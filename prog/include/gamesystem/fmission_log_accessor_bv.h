//====================================================================================
/**
 *  @file   fmission_log_accessor_bv.h
 *  @brief  フェスミッションログ　データアクセス(フィールド BEACON_VIEWオーバーレイ)
 *  @author Miyuki Iwasawa
 *  @date   11.03.23
 *
 *  fmission_logのルーチンのうち、フィールド上からだけアクセスできれば良い関数を集めてある
 *  BeaconViewバージョン
 */
//====================================================================================

#pragma once

//------------------------------------------------------------------
/*
 *  ミッションエントリーデータテーブル データ取得
 *
 *  @param  tbl       FMISSION_ENTRY_TBL型構造体へのポインタ
 *  @param  fmwk      ミッションパラメータを受けるSTRBUFへのポインタ
 *  @param  name_buf  ミッション配布者の名前を受けるSTRBUFへのポインタ
 *
 *  @retval TRUE      データ取得成功
 *  @retval FALSE     データ取得失敗
 *
 *  情報取得に成功した場合、テーブル内のデータが前詰めされます
 */
//------------------------------------------------------------------
extern BOOL FMISSION_ENTRY_TBL_GetData( FMISSION_ENTRY_TBL* tbl, FEST_MISSION* fmwk, STRBUF* name_buf );

