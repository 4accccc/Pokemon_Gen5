//====================================================================================
/**
 *  @file   fmission_log.h
 *  @brief  フェスミッションログ　管理(常駐)
 *  @author Miyuki Iwasawa
 *  @date   11.03.09
 */
//====================================================================================

#pragma once

#include "fmission_log_id.h"
#include "field/gpower_id.h"

typedef u8 FMISSION_LOG_ACTION; //FMISSION_LOG_ACT_NULL etc

#define FMISSION_LOG_MAX  (30)  ///<ログ格納数max
#define FMISSION_LOG_END  (FMISSION_LOG_MAX-1)

#define FMISSION_GPOWER_TBL_MAX (2)
#define FMISSION_GPOWER_TBL_END (FMISSION_GPOWER_TBL_MAX-1)

#define FMISSION_ENTRY_TBL_MAX  (3)
#define FMISSION_ENTRY_TBL_END  (FMISSION_ENTRY_TBL_MAX-1)

#define FMISSION_LOG_BUFLEN_NAME  (PERSON_NAME_SIZE+EOM_SIZE)
#define FMISSION_LOG_BUFLEN_MSG   ((15*2)+EOM_SIZE)

typedef struct _FMISSION_LOG  FMISSION_LOG;
typedef struct _FMISSION_LOG_TBL FMISSION_LOG_TBL;
typedef struct _FMISSION_GPOWER  FMISSION_GPOWER;
typedef struct _FMISSION_GPOWER_TBL FMISSION_GPOWER_TBL;
typedef struct _FMISSION_ENTRY  FMISSION_ENTRY;
typedef struct _FMISSION_ENTRY_TBL FMISSION_ENTRY_TBL;

//最新ログ取得用構造体
typedef struct _FMISSION_LOG_LATEST_PARAM{
  STRBUF* name; ///<人物名格納バッファ
  u16 score;    ///<スコア
  u16 tr_id;    ///<トレーナーID
}FMISSION_LOG_LATEST_PARAM;

#define FMISSION_LOG_LATEST_NUM_MAX (3)

//最新3件のログ取得用構造体
typedef struct _FMISSION_LOG_LATEST{
  int num;
  FMISSION_LOG_LATEST_PARAM  log[FMISSION_LOG_LATEST_NUM_MAX];
}FMISSION_LOG_LATEST;

//------------------------------------------------------------------
/**
 * ログパラメータ取得　名前
 *
 * @param   log	    	ログ情報
 * @param   s_buf       PERSON_NAME_SIZE+EOM_SIZEを格納可能なSTRBUFのポインタ
 */
//------------------------------------------------------------------
extern void FMISSION_LOG_GetName( FMISSION_LOG* log, STRBUF* s_buf );

//------------------------------------------------------------------
/*
 *  デルパワースタック データセット ミッションログから
 *
 *  @param  tbl   FMISSION_GPOWER_TBL型構造体へのポインタ
 *  @param  log   デルパワー情報を抜き出すフェスミッションログ構造体情報
 *  @return データをセットしたデルパワーID
 */
//------------------------------------------------------------------
extern GPOWER_ID FMISSION_GPOWER_TBL_SetFromLog( FMISSION_GPOWER_TBL* tbl, const FMISSION_LOG* log );

//他関数のextern宣言は src/gamesystem/fmission_log_local.h

