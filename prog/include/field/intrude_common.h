//==============================================================================
/**
 * @file    intrude_common.h
 * @brief   侵入全体で使用する共通ヘッダ
 * @author  matsuda
 * @date    2009.10.10(土)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================
///侵入：通信最大接続人数
#define FIELD_COMM_MEMBER_MAX (3)

///繋がっていない事を示すNETID
#define INTRUDE_NETID_NULL    (FIELD_COMM_MEMBER_MAX + 1)
///パレスエリア番号初期値(まだ通信が接続されていなくて、自分が何番目か分からない状態)
#define PALACE_AREA_NO_NULL     (128)

///侵入できる街の数
#define INTRUDE_TOWN_MAX      (8)

///街の占拠値   (WHITE ---- OCCUPY_TOWN_NEUTRALITY ---- OCCUPY_TOWN_BLACK)
enum{
  OCCUPY_TOWN_WHITE = 0,                            ///<ホワイト占拠
  OCCUPY_TOWN_NEUTRALITY = 100,                     ///<中立
  OCCUPY_TOWN_BLACK = OCCUPY_TOWN_NEUTRALITY * 2,   ///<ブラック占拠
};

//==============================================================================
//  型定義
//==============================================================================
///_INTRUDE_COMM_SYS構造体の不定形ポインタ型
typedef struct _INTRUDE_COMM_SYS * INTRUDE_COMM_SYS_PTR;
