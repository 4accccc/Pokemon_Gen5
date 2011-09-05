//====================================================================================
/**
 *  @file   fmission_log_accessor_fapp.h
 *  @brief  フェスミッション詳細画面　データアクセス(フィールド FMISSION_APPオーバーレイ)
 *  @author Miyuki Iwasawa
 *  @date   11.09.05
 *
 *  fmission_logのルーチンのうち、フェスミッション詳細画面アプリ上からだけ
 *  アクセスできれば良い関数を集めてある
 */
//====================================================================================

#pragma once

//---------------------------------------------------------
/*
 * @brief フェス最新ログ格納ワーク生成
 *
 * FMISSION_LOG_LATEST_Delete()で開放してください
 */
//---------------------------------------------------------
extern FMISSION_LOG_LATEST* FMISSION_LOG_LATEST_Create( HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief フェス最新ログ格納ワーク開放
 */
//---------------------------------------------------------
extern void FMISSION_LOG_LATEST_Delete( FMISSION_LOG_LATEST* lp );

//------------------------------------------------------------------
/*
 *  ログスタック 最新3件取得
 *
 *  @param  tbl     FMISSION_LOG_TBL型構造体へのポインタ
 *  @param  lp   取得するログ情報を格納するFMISSION_LOG_LATEST型ワークへのポインタ
 *
 *  @retval TRUE    データ取得成功
 *  @retval FALSE   スタックテーブルにデータがない
 */
//------------------------------------------------------------------
extern BOOL FMISSION_LOG_StackTbl_GetLogLatest( FMISSION_LOG_TBL* tbl, FMISSION_LOG_LATEST* lp );


