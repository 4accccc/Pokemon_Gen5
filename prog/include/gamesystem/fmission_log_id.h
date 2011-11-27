//====================================================================================
/**
 *  @file   fmission_log.h
 *  @brief  フェスミッションログアクションID定義
 *  @author Miyuki Iwasawa
 *  @date   11.09.11
 */
//====================================================================================

#pragma once
  
//NotBeacon
#define  FMISSION_LOG_ACT_NULL         ( 0 )    ///<NULL
#define  FMISSION_LOG_ACT_OPEN         ( 1 )    ///<開催
#define  FMISSION_LOG_ACT_NEW_ENTRY    ( 2 )    ///<新規参加
#define  FMISSION_LOG_ACT_MAP_CHANGE   ( 3 )    ///<マップチェンジ
//With Param
#define  FMISSION_LOG_ACT_POKE_ENCOUNT     ( 4 )   ///<[ポケモン名]とエンカウントした
#define  FMISSION_LOG_ACT_POKE_WIN         ( 5 )   ///<[ポケモン名]を倒した
#define  FMISSION_LOG_ACT_POKE_CAPTURE     ( 6 )   ///<[ポケモン名]を捕まえた
#define  FMISSION_LOG_ACT_POKE_ESCAPE      ( 7 )   ///<[ポケモン名]に逃げられたor逃げた
#define  FMISSION_LOG_ACT_TR_ENCOUNT       ( 8 )   ///<トレーナー戦開始
#define  FMISSION_LOG_ACT_TR_WIN           ( 9 )   ///<トレーナー戦勝利
#define  FMISSION_LOG_ACT_GAMEOVER         ( 10 )  ///<全滅した
#define  FMISSION_LOG_ACT_ITEM_USE         ( 11 )  ///<[アイテム名]を使用した
#define  FMISSION_LOG_ACT_ITEM_GET         ( 12 )  ///<[アイテム名]を手に入れた
#define  FMISSION_LOG_ACT_FIELD_SKILL      ( 13 )  ///<[フィールド技]を使った
#define  FMISSION_LOG_ACT_SHOPPING         ( 14 )  ///<買い物中
#define  FMISSION_LOG_ACT_GPOWER           ( 15 )  ///<デルパワーを使った
#define  FMISSION_LOG_ACT_FERRIS_WHEEL     ( 16 )  ///<観覧車に乗った
#define  FMISSION_LOG_ACT_WATCH_SIGNBOARD  ( 17 )  ///<看板を調べている×
#define  FMISSION_LOG_ACT_SODATEYA_EGG     ( 18 )  ///<タマゴを受け取った

#define  FMISSION_LOG_ACT_EFF_ENC_WIN      ( 19 )     ///<[ポケモン名]を倒した(揺れ草)○
#define  FMISSION_LOG_ACT_EFF_ENC_CAPTURE  ( 20 )     ///<[ポケモン名]を捕まえた(揺れ草)○
#define  FMISSION_LOG_ACT_MUSHROOM_WIN     ( 21 )     ///<[ポケモン名]を倒した(キノコエンカウント)○
#define  FMISSION_LOG_ACT_MUSHROOM_CAPTURE ( 22 )     ///<[ポケモン名]を捕まえた(キノコエンカウント)○
#define  FMISSION_LOG_ACT_KIRA_ITEM_GET    ( 23 )     ///<[アイテム名]を見つけた(エフェクトアイテム)

#define  FMISSION_LOG_ACT_TOWER_OF_HEAVEN       ( 24 )  ///<鐘を鳴らした
#define  FMISSION_LOG_ACT_FEATHER_GET           ( 25 )  ///<[アイテム名]を見つけた(羽)○
#define  FMISSION_LOG_ACT_JEWEL_GET             ( 26 )  ///<[アイテム名]を見つけた(ジュエルと石)○
#define  FMISSION_LOG_ACT_EGG_HATCH             ( 27 )  ///<[ポケモン名]が生まれた○
#define  FMISSION_LOG_ACT_FISHING_ENCOUNT       ( 28 )  ///<[ポケモン名]を釣り上げた○
#define  FMISSION_LOG_ACT_ANIMAL_TRAIL_ENCOUNT  ( 29 )  ///<[ポケモン名]を見つけた(獣道)

#define  FMISSION_LOG_ACT_HELP_CHILD         ( 30 )  ///<まいごを助けた
#define  FMISSION_LOG_ACT_PRESENT_ITEM       ( 31 )  ///<[アイテム名]を渡した
#define  FMISSION_LOG_ACT_ITEM_TRADE         ( 32 )  ///<道具交換成功
#define  FMISSION_LOG_ACT_ITEM_BUY           ( 33 )  ///<[アイテム名]を買った
#define  FMISSION_LOG_ACT_RPS_WIN            ( 34 )  ///<じゃんけんに買った
#define  FMISSION_LOG_ACT_MEMORY_QUIZ_CLEAR  ( 35 )  ///<記憶クイズに正解した
#define  FMISSION_LOG_ACT_TR_BATTLE_WIN      ( 36 )  ///<トレーナーを倒した
#define  FMISSION_LOG_ACT_GET_A_PRESENT      ( 37 )  ///<[アイテム名]をもらった
#define  FMISSION_LOG_ACT_POKE_QUIZ_CLEAR    ( 38 )  ///<図鑑クイズに正解
#define  FMISSION_LOG_ACT_RUN_OUT_OF_MONEY   ( 39 )  ///<お金が足りない
#define  FMISSION_LOG_ACT_RPS_LOSE           ( 40 )  ///<じゃんけんに負けた
#define  FMISSION_LOG_ACT_QUIZ_WRONG_ANSWER  ( 41 )  ///<クイズに不正(記憶・図鑑クイズ共用)
#define  FMISSION_LOG_ACT_POKE_DOWN          ( 42 )   ///<[ポケモン名]が瀕死
#define  FMISSION_LOG_ACT_POKE_HP_LITTLE     ( 43 )   ///<[ポケモン名]のHPが少ない
#define  FMISSION_LOG_ACT_POKE_PP_LITTLE     ( 44 )   ///<[ポケモン名]のPPが少ない

//Haven't MissionParam
#define  FMISSION_LOG_ACT_FREEWORD           ( 45 )   ///<一言メッセージ
  
#define  FMISSION_LOG_ACT_MAX                ( 46 )
  
#define  FMISSION_LOG_ACT_GRP_BEACON (FMISSION_LOG_ACT_POKE_ENCOUNT)
#define  FMISSION_LOG_ACT_GRP_EVENT  (FMISSION_LOG_ACT_TOWER_OF_HEAVEN)
#define  FMISSION_LOG_ACT_GRP_NPC    (FMISSION_LOG_ACT_HELP_CHILD)


