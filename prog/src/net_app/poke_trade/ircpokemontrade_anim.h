//=============================================================================
/**
 * @file	  ircpokemontrade_anim.h
 * @bfief	  ポケモン交換 アニメーション間隔定義
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/08/28
 */
//=============================================================================

#pragma once


// ゆっくりで調整してからはやめる
#define ALL_SLOW_SPEED_NUM (8)    ///< 八倍おそい


#define ANIM_SPEED  (FX32_ONE / ALL_SLOW_SPEED_NUM)


#define _TRADE01_ANMCOUNT (30)   //交換選択
#define _TRADE02_ANMCOUNT (30)   //交換選択から真ん中に移動するまで
#define _TRADE03_ANMCOUNT (30)   //真ん中からぼーるになるまで
#define _TRADE04_ANMCOUNT (30)   //ボール発光
#define _TRADE05_ANMCOUNT (30)   //光の筋 飛び開始
#define _TRADE06_ANMCOUNT (30)   //飛んでいく
#define _TRADE07_ANMCOUNT (30)   //光飛び交う空間
#define _TRADE08_ANMCOUNT (30)   //上下ポケモン登場
#define _TRADE09_ANMCOUNT (30)   //とまる
#define _TRADE10_ANMCOUNT (30)   //ポケモン左右外に
#define _TRADE11_ANMCOUNT (30)   //正面背面入れ替わって中に
#define _TRADE12_ANMCOUNT (30)   //とまる
#define _TRADE13_ANMCOUNT (30)   //上下に移動
#define _TRADE14_ANMCOUNT (30)   //ホワイトアウト
#define _TRADE15_ANMCOUNT (30)   //ボール振ってくる
#define _TRADE16_ANMCOUNT (90)   //落ちてる最中
#define _TRADE17_ANMCOUNT (30)   //ぶつかり波紋
#define _TRADE18_ANMCOUNT (30)   //おちたエフェクト
#define _TRADE19_ANMCOUNT (30)   //はもんひろがる
#define _TRADE20_ANMCOUNT (30)   //ブラックアウト
#define _TRADE21_ANMCOUNT (30)   //ボール落ちる
#define _TRADE22_ANMCOUNT (30)   //？
#define _TRADE23_ANMCOUNT (30)   //発光
#define _TRADE24_ANMCOUNT (30)   //ポケモン出てくる

