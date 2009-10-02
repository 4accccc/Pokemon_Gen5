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
#define ALL_SLOW_SPEED_NUM (1)    ///< ?倍おそい


#define ANIM_SPEED  (FX32_ONE / ALL_SLOW_SPEED_NUM)

#define _BALL_PARTICLE_START   (120)  ///モンスターボールが打ちあがる パーティクル開始
#define _POKEUP_WHITEOUT_START (240)  //モンスターボールが打ちあがった際のホワイトアウト
#define _POKEUP_WHITEOUT_TIMER (14)  //モンスターボールが打ちあがった際のホワイトアウト時間
#define _POKEUP_WHITEIN_START  (255)  //ポケモン交換シーン前のフェードイン
#define _POKEUP_WHITEIN_TIMER (15)  //ポケモン交換シーン前のフェードイン時間


#define _POKECHANGE_WHITEOUT_START (510)  //ポケモン交換シーンのホワイトアウト
#define _POKECHANGE_WHITEOUT_TIMER (14)  //ポケモン交換シーンのホワイトアウト時間
#define _POKECHANGE_WHITEIN_START  (525)  //ボールおち前のフェードイン
#define _POKECHANGE_WHITEIN_TIMER (15)  //ボールおち前のフェードイン時間


#define _POKESPLASH_WHITEOUT_START (720)  //ボールおちシーンのホワイトアウト
#define _POKESPLASH_WHITEOUT_TIMER (14)  //ボールおちシーンのホワイトアウト時間
#define _POKESPLASH_WHITEIN_START  (735)  //もう一回ボール落ちのフェードイン
#define _POKESPLASH_WHITEIN_TIMER (45)  //もう一回ボール落ちのフェードイン時間


#define _DEMO_END (960)
