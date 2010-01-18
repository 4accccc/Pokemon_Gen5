//======================================================================
/**
 * @file  pl_boat_def.h
 * @brief  遊覧船関連定義 スクリプト参照可
 * @author  Saito
 */
//======================================================================

#pragma once

#define PL_TR_SEARCH_TYPE_LEFT  (0)     //船内左側
#define PL_TR_SEARCH_TYPE_RIGHT  (1)    //船内右側
#define PL_TR_SEARCH_TYPE_SINGLE  (2)   //シングルルーム
#define PL_TR_SEARCH_TYPE_DOUBLE  (3)   //ダブルルーム
#define PL_TR_SEARCH_TYPE_TOTAL  (4)   //総数
#define PL_TR_SEARCH_TYPE_WIN  (5)   //勝ったトレーナー数

#define PL_BOAT_ROOM_INFO_OBJ_CODE  (4)
#define PL_BOAT_ROOM_INFO_NPC_TYPE  (5)
#define PL_BOAT_ROOM_INFO_TR_ID     (6)
#define PL_BOAT_ROOM_INFO_MSG       (7)
#define PL_BOAT_ROOM_INFO_BTL_FLG   (8)
//#define PL_BOAT_ROOM_INFO_BTL_TYPE  (9)

//ＮＰＣ種類
#define NPC_TYPE_TRAINER (0)
#define NPC_TYPE_SHORT_TALKER (1)
#define NPC_TYPE_LONG_TALKER  (2)

//戦闘したことあるかチェック用
#define PL_BOAT_TR_BTL_YET (0)      //まだ戦っていない
#define PL_BOAT_TR_BTL_ALREADY (1)  //既に戦った

//ダブルバトル・シングルバトル部屋
//#define PL_BOAT_BTL_TYPE_DOUBLE (0)   //ダブルバトル
//#define PL_BOAT_BTL_TYPE_SINGLE (1)   //シングルバトル

//#define PL_BOAT_TALK_NOT_PAIR   (0)   //通常トレーナーＯＲダブルバトル可変型の方
//#define PL_BOAT_TALK_PAIR   (1)       //ダブルバトル可変型のじゃない方

//メッセージの種類
#define PL_BOAT_BTL_BEFORE_NORMAL  (0)    //通常トレーナー
#define PL_BOAT_BTL_AFTER_NORMAL  (1)     //通常トレーナー
//#define PL_BOAT_BTL_BEFORE_PAIR  (2)      //ダブルバトル可変型のじゃない方
//#define PL_BOAT_BTL_AFTER_PAIR  (3)       //ダブルバトル可変型のじゃない方

//デモの終わり方
#define PL_BOAT_DEMO_USER_END (0)         //プレーヤーがデモを終了させた
#define PL_BOAT_DEMO_FINISHED (1)         //デモが最後まで再生された



