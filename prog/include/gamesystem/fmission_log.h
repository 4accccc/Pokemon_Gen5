//====================================================================================
/**
 *  @file   fmission_log.h
 *  @brief  フェスミッションログ　管理(常駐)
 *  @author Miyuki Iwasawa
 *  @date   11.03.09
 */
//====================================================================================

#pragma once

#define FMISSION_LOG_MAX  (30)  ///<ログ格納数max
#define FMISSION_LOG_END  (FMISSION_LOG_MAX-1)

#define FMISSION_GPOWER_TBL_MAX (2)
#define FMISSION_GPOWER_TBL_END (FMISSION_GPOWER_TBL_MAX-1)

#define FMISSION_LOG_BUFLEN_NAME  (PERSON_NAME_SIZE+EOM_SIZE)
#define FMISSION_LOG_BUFLEN_MSG   ((15*2)+EOM_SIZE)

typedef enum{
  //NotBeacon
  FMISSION_LOG_ACT_NULL,          ///<NULL
  FMISSION_LOG_ACT_NEW_ENTRY,     ///<新規参加
  FMISSION_LOG_ACT_MAP_CHANGE,    ///<マップチェンジ
  //With Param
  FMISSION_LOG_ACT_POKE_ENCOUNT,  ///<ポケモンとエンカウントした
  FMISSION_LOG_ACT_POKE_WIN,      ///<ポケモンを倒した
  FMISSION_LOG_ACT_POKE_CAPTURE,  ///<ポケモンを捕まえた
  FMISSION_LOG_ACT_POKE_ESCAPE,   ///<ポケモンに逃げられたor逃げた
  FMISSION_LOG_ACT_TR_ENCOUNT,    ///<トレーナー戦開始
  FMISSION_LOG_ACT_GAMEOVER,      ///<全滅した
  FMISSION_LOG_ACT_ITEM_USE,      ///<アイテムを使用した
  FMISSION_LOG_ACT_GPOWER,        ///<デルパワーを使った
  FMISSION_LOG_ACT_EVENT_CLEAR,   ///<イベントクリア

  //Haven't MissionParam
  FMISSION_LOG_ACT_FREEWORD,      ///<一言メッセージ
  FMISSION_LOG_ACT_POKE_DOWN,     ///<ポケモンが瀕死
  
  FMISSION_LOG_ACT_MAX,
}FMISSION_LOG_ACTION;

#define FMISSION_LOG_ACT_WP_START (FMISSION_LOG_ACT_POKE_ENCOUNT)

typedef struct _FMISSION_LOG  FMISSION_LOG;
typedef struct _FMISSION_LOG_TBL FMISSION_LOG_TBL;
typedef struct _FMISSION_GPOWER  FMISSION_GPOWER;
typedef struct _FMISSION_GPOWER_TBL FMISSION_GPOWER_TBL;


//関数のextern宣言は src/gamesystem/fmission_log_local.h

