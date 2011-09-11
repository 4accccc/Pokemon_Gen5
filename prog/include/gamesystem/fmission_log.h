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

#define FMISSION_ENTRY_TBL_MAX  (3)
#define FMISSION_ENTRY_TBL_END  (FMISSION_ENTRY_TBL_MAX-1)

#define FMISSION_LOG_BUFLEN_NAME  (PERSON_NAME_SIZE+EOM_SIZE)
#define FMISSION_LOG_BUFLEN_MSG   ((15*2)+EOM_SIZE)

typedef enum{
  //NotBeacon
  FMISSION_LOG_ACT_NULL = 0,          ///<NULL
  FMISSION_LOG_ACT_OPEN = 1,          ///<開催
  FMISSION_LOG_ACT_NEW_ENTRY = 2,     ///<新規参加
  FMISSION_LOG_ACT_MAP_CHANGE = 3,    ///<マップチェンジ
  //With Param
  FMISSION_LOG_ACT_POKE_ENCOUNT = 4,  ///<[ポケモン名]とエンカウントした
  FMISSION_LOG_ACT_POKE_WIN = 5,      ///<[ポケモン名]を倒した
  FMISSION_LOG_ACT_POKE_CAPTURE = 6,  ///<[ポケモン名]を捕まえた
  FMISSION_LOG_ACT_POKE_ESCAPE = 7,   ///<[ポケモン名]に逃げられたor逃げた
  FMISSION_LOG_ACT_TR_ENCOUNT = 8,    ///<トレーナー戦開始
  FMISSION_LOG_ACT_TR_WIN = 9,        ///<トレーナー戦勝利
  FMISSION_LOG_ACT_GAMEOVER = 10,      ///<全滅した
  FMISSION_LOG_ACT_ITEM_USE = 11,     ///<[アイテム名]を使用した
  FMISSION_LOG_ACT_ITEM_GET = 12,     ///<[アイテム名]を手に入れた
  FMISSION_LOG_ACT_FIELD_SKILL = 13,  ///<[フィールド技]を使った
  FMISSION_LOG_ACT_SHOPING = 14,      ///<買い物中
  FMISSION_LOG_ACT_GPOWER = 15,       ///<デルパワーを使った
  FMISSION_LOG_ACT_FERRIS_WHEEL = 16, ///<観覧車に乗った
  FMISSION_LOG_ACT_WATCH_SIGNBOARD = 17,    ///<看板を調べている
  FMISSION_LOG_ACT_SODATEYA_EGG = 18,       ///<タマゴを受け取った

  FMISSION_LOG_ACT_EFF_ENC_WIN = 19,         ///<[ポケモン名]を倒した(揺れ草)
  FMISSION_LOG_ACT_EFF_ENC_CAPTURE = 20,     ///<[ポケモン名]を捕まえた(揺れ草)
  FMISSION_LOG_ACT_KIRA_ITEM_GET = 21,       ///<[アイテム名]を見つけた(エフェクトアイテム)

  FMISSION_LOG_ACT_TOWER_OF_HEAVEN = 22,       ///<鐘を鳴らした
  FMISSION_LOG_ACT_FEATHER_GET = 23,           ///<[アイテム名]を見つけた(羽)
  FMISSION_LOG_ACT_JEWEL_GET = 24,             ///<[アイテム名]を見つけた(ジュエル)
  FMISSION_LOG_ACT_EGG_HATCH = 25,             ///<[ポケモン名]が生まれた
  FMISSION_LOG_ACT_FISHING_ENCOUNT = 26,       ///<[ポケモン名]を釣り上げた
  FMISSION_LOG_ACT_ANIMAL_TRAIL_ENCOUNT = 27,  ///<[ポケモン名]を見つけた(獣道)
  FMISSION_LOG_ACT_MUSHROOM_WIN = 28,          ///<[ポケモン名]を倒した(キノコエンカウント)
  FMISSION_LOG_ACT_MUSHROOM_CAPTURE = 29,      ///<[ポケモン名]を捕まえた(キノコエンカウント)

  FMISSION_LOG_ACT_HELP_CHILD = 30,         ///<まいごを助けた
  FMISSION_LOG_ACT_PRESENT_ITEM = 31,       ///<[アイテム名]を渡した
  FMISSION_LOG_ACT_ELITE_TR_WIN = 32,       ///<エリートトレーナーを倒した
  FMISSION_LOG_ACT_ITEM_TRADE = 33,         ///<道具交換成功
  FMISSION_LOG_ACT_ITEM_BUY = 34,           ///<[アイテム名]を買った
  FMISSION_LOG_ACT_QUIZ_CLEAR = 35,         ///<クイズに正解した
  FMISSION_LOG_ACT_RPS_WIN = 36,            ///<じゃんけんに買った
  FMISSION_LOG_ACT_GET_A_PRESENT = 37,      ///<[アイテム名]をもらった
  FMISSION_LOG_ACT_RUN_OUT_OF_MONEY = 38,   ///<お金が足りない
  FMISSION_LOG_ACT_RPS_LOSE = 39,           ///<じゃんけんに負けた

  //Haven't MissionParam
  FMISSION_LOG_ACT_POKE_DOWN = 40,        ///<[ポケモンニックネーム]が瀕死
  FMISSION_LOG_ACT_POKE_HP_LITTLE = 41,   ///<[ポケモンニックネーム]のHPが少ない
  FMISSION_LOG_ACT_POKE_PP_LITTLE = 42,   ///<[ポケモンニックネーム]のPPが少ない
  FMISSION_LOG_ACT_FREEWORD = 43,         ///<一言メッセージ
  
  FMISSION_LOG_ACT_MAX = 44,
  
  FMISSION_LOG_ACT_EVENT_CLEAR=FMISSION_LOG_ACT_TOWER_OF_HEAVEN,   ///<イベントクリア
  
}FMISSION_LOG_ACTION;

#define FMISSION_LOG_ACT_WP_START (FMISSION_LOG_ACT_POKE_ENCOUNT)

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


//他関数のextern宣言は src/gamesystem/fmission_log_local.h

