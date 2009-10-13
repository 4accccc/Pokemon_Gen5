//=============================================================================
/**
 * @file	  ircpokemontrade_anim.h
 * @bfief	  ポケモン交換 アニメーション間隔定義
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/08/28
 */
//=============================================================================

#pragma once

#include "ircpokemontrade_local.h"


//時間に関する事
#define _POKEMON_CENTER_TIME   (60)  ///ポケモンが真ん中による時間

#define _POKEMON_DELETE_TIME   (100)  ///ポケモンを消す


#define _POKEUP_WHITEOUT_START (480)  //モンスターボールが打ちあがった際のホワイトアウト
#define _POKEUP_WHITEOUT_TIMER (29)  //モンスターボールが打ちあがった際のホワイトアウト時間 1/
#define _POKEUP_WHITEIN_START  (510)  //ポケモン交換シーン前のフェードイン
#define _POKEUP_WHITEIN_TIMER (30)  //ポケモン交換シーン前のフェードイン時間

#define _POKE_APPEAR_START   (560)  //ポケモン登場 自分のは裏
#define _POKE_APPEAR_TIME    (30)  //ポケモン登場移動時間
#define _POKE_SIDEOUT_START   (600)  //ポケモン左右に移動していく
#define _POKE_SIDEOUT_TIME    (80)  //ポケモン登場移動時間
#define _POKE_SIDEIN_START   (720)  //ポケモン左右に移動して戻ってくる
#define _POKE_SIDEIN_TIME    (240)   //ポケモン登場移動時間
#define _POKE_LEAVE_START   (980)  //ポケモン退場
#define _POKE_LEAVE_TIME    (30)  //ポケモン退場移動時間


#define _BALL_PARTICLE_START   (1300)  ///モンスターボール激突 パーティクル開始


#define _PARTICLE_DEMO2_START   (240)  ///パーティクル開始
#define _PARTICLE_DEMO3_START   (300)  ///パーティクル開始
#define _PARTICLE_DEMO3_START2  (1400)  ///パーティクル開始
#define _PARTICLE_DEMO4_START   (-1)  ///パーティクル開始
#define _PARTICLE_DEMO5_START   (240)  ///パーティクル開始
#define _PARTICLE_DEMO6_START   (480)  ///パーティクル開始
#define _PARTICLE_DEMO7_START   (1050)  ///パーティクル開始



#define _POKECHANGE_WHITEOUT_START (1020)  //ポケモン交換シーンのホワイトアウト
#define _POKECHANGE_WHITEOUT_TIMER (29)  //ポケモン交換シーンのホワイトアウト時間
#define _POKECHANGE_WHITEIN_START  (1050)  //ボールおち前のフェードイン
#define _POKECHANGE_WHITEIN_TIMER (30)  //ボールおち前のフェードイン時間


#define _POKESPLASH_WHITEOUT_START (1440)  //ボールおちシーンのホワイトアウト
#define _POKESPLASH_WHITEOUT_TIMER (29)  //ボールおちシーンのホワイトアウト時間
#define _POKESPLASH_WHITEIN_START  (1470)  //もう一回ボール落ちのフェードイン
#define _POKESPLASH_WHITEIN_TIMER (90)  //もう一回ボール落ちのフェードイン時間


#define _DEMO_END (1920)   //デモが終わる最後のフレーム

// 座標に関する事
#define _POKEMON_PLAYER_APPEAR_POSX _MCSS_POS_X( 60)  // 自分のポケモンが登場する場合のX
#define _POKEMON_PLAYER_APPEAR_POSY _MCSS_POS_Y(140)  // 自分のポケモンが登場する場合のY
#define _POKEMON_FRIEND_APPEAR_POSX _MCSS_POS_X(190)  // 相手のポケモンが登場する場合のX
#define _POKEMON_FRIEND_APPEAR_POSY _MCSS_POS_Y(120)  // 相手のポケモンが登場する場合のY


#define _POKEMON_PLAYER_SIDEOUT_POSX _MCSS_POS_X(-40)  // 自分のポケモンが左に退場する場合のX
#define _POKEMON_PLAYER_SIDEOUT_POSY _MCSS_POS_Y(140)  // 自分のポケモンが左に退場する場合のY
#define _POKEMON_FRIEND_SIDEOUT_POSX _MCSS_POS_X(290)  // 相手のポケモンが右に退場する場合のX
#define _POKEMON_FRIEND_SIDEOUT_POSY _MCSS_POS_Y(120)  // 相手のポケモンが右に退場する場合のY


#define _POKEMON_PLAYER_SIDEIN_POSX _MCSS_POS_X(190)  // 自分のポケモンが左から登場する場合のX
#define _POKEMON_PLAYER_SIDEIN_POSY _MCSS_POS_Y(120)  // 自分のポケモンが左から登場する場合のY
#define _POKEMON_FRIEND_SIDEIN_POSX _MCSS_POS_X( 60)  // 相手のポケモンが右から登場する場合のX
#define _POKEMON_FRIEND_SIDEIN_POSY _MCSS_POS_Y(140)  // 相手のポケモンが右から登場する場合のY


#define _POKEMON_PLAYER_LEAVE_POSX _MCSS_POS_X(190)  // 自分のポケモンが上に退場する場合のX
#define _POKEMON_PLAYER_LEAVE_POSY _MCSS_POS_Y(-50)  // 自分のポケモンが上に退場する場合のY
#define _POKEMON_FRIEND_LEAVE_POSX _MCSS_POS_X( 60)  // 相手のポケモンが下に退場する場合のX
#define _POKEMON_FRIEND_LEAVE_POSY _MCSS_POS_Y(240)  // 相手のポケモンが下に退場する場合のY

