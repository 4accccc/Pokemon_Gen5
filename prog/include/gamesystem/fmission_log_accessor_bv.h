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
 *  @param  fest_sys  LINK_FESTIVAL型構造体へのポインタ
 *  @param  fmwk      ミッションパラメータを受けるSTRBUFへのポインタ
 *  @param  name_buf  ミッション配布者の名前を受けるSTRBUFへのポインタ
 *
 *  @retval TRUE      データ取得成功
 *  @retval FALSE     データ取得失敗
 *
 *  情報取得に成功しても、テーブル内のデータは消えません。
 *  適切な処理後は FMISSION_ENTRY_TBL_DeleteData()を呼び出して不要になったデータを消してください
 */
//------------------------------------------------------------------
extern BOOL FMISSION_ENTRY_TBL_GetData( LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, STRBUF* name_buf );

