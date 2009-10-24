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


//フェードに関する時間
#define _POKEUP_WHITEOUT_START (380)       //絵コンテ６番モンスターボールが打ちあがった際のホワイトアウト
#define _POKEUP_WHITEOUT_TIMER (29)       //絵コンテ６番モンスターボールが打ちあがった際のホワイトアウト時間 1/
#define _POKEUP_WHITEIN_START  (430)      //絵コンテ７番ポケモン交換シーン前のフェードイン
#define _POKEUP_WHITEIN_TIMER (30)        //絵コンテ７番ポケモン交換シーン前のフェードイン時間

#define _POKECHANGE_WHITEOUT_START (930)  //絵コンテ１４番ポケモン交換シーンのホワイトアウト
#define _POKECHANGE_WHITEOUT_TIMER (29)      //絵コンテ１４番ポケモン交換シーンのホワイトアウト時間
#define _POKECHANGE_WHITEIN_START  (990)  //絵コンテ１５番ボールおち前のフェードイン
#define _POKECHANGE_WHITEIN_TIMER (60)        //絵コンテ１５番ボールおち前のフェードイン時間

#define _POKESPLASH_WHITEOUT_START (1280)  //絵コンテ２０番ボールおちシーンのホワイトアウト
#define _POKESPLASH_WHITEOUT_TIMER (29)       //絵コンテ２０番ボールおちシーンのホワイトアウト時間

#define _POKESPLASH_WHITEIN_START  (1390)  //絵コンテ２１番もう一回ボール落ちのフェードイン
#define _POKESPLASH_WHITEIN_TIMER (30)        //絵コンテ２１番もう一回ボール落ちのフェードイン時間

#define _DEMO_END (1740)   //デモが終わる最後のフレーム


//ポケモンの動きに関する時間
#define _POKEMON_DELETE_TIME   (5)     // 絵コンテ３番  ポケモンを消す

#define _POKE_APPEAR_START   (470)   //絵コンテ８番  ポケモン登場 自分のは裏
#define _POKE_APPEAR_TIME    (20)       //絵コンテ８番  ポケモン登場移動時間
#define _POKE_APPEAR_START2   (485)   //絵コンテ９番     さらにゆっくり移動
#define _POKE_APPEAR_TIME2   (40)       //絵コンテ９番  さらにゆっくり移動
#define _POKE_SIDEOUT_START   (525)  //絵コンテ１０番  ポケモン左右に移動していく
#define _POKE_SIDEOUT_TIME    (65)      //絵コンテ１０番  ポケモン登場移動時間
#define _POKE_SIDEIN_START   (620)   //絵コンテ１１番  ポケモン左右に移動して戻ってくる
#define _POKE_SIDEIN_TIME    (200)     //絵コンテ１１番 ポケモン登場移動時間
#define _POKE_LEAVE_START   (910)  //絵コンテ１３番  ポケモン退場
#define _POKE_LEAVE_TIME    (10)      //絵コンテ１３番  ポケモン退場移動時間

#define _POKEMON_CREATE_TIME   (1665)  ///絵コンテ２４番 交換終了してポケモン登場 


///パーティクルのタイミング
#define _BALL_PARTICLE_START   (1220)  ///モンスターボール激突 パーティクル開始
#define _PARTICLE_DEMO2_START   (145)  ///demo_tex002パーティクル開始
#define _PARTICLE_DEMO3_START   (220)  ///demo_tex003パーティクル開始
#define _PARTICLE_DEMO3_START2  (1220)  ///パーティクル開始
#define _PARTICLE_DEMO4_START   (20)     ///ポケモン＞ボールエフェクト
#define _PARTICLE_DEMO4_START2   (1650)  ///ボール＞ポケモンエフェクト
#define _PARTICLE_DEMO5_START   (220)  ///モンスターボールが打ちあがるパーティクル開始
#define _PARTICLE_DEMO6_START   (490)  ///パーティクル開始
#define _PARTICLE_DEMO7_START   (1000)  ///パーティクル開始

///OAM登場のタイミング
#define _OAM_POKECREATE_START   (20)  ///ポケモン＞ボールエフェクト
#define _OAM_POKECREATE_START2  (1645)  ///ボール＞ポケモンエフェクト




// 座標に関する事
//絵コンテ２番の移動の中心
#define _POKEMON_PLAYER_CENTER_POSX _MCSS_POS_X(  0)   // 自分のポケモンが真ん中に移動する際のX
#define _POKEMON_PLAYER_CENTER_POSY _MCSS_POS_Y(-20)   // 自分のポケモンが真ん中に移動する際のY
#define _POKEMON_PLAYER_CENTER_POSZ _MCSS_POS_Z( 10)   // 自分のポケモンが真ん中に移動する際のZ

//絵コンテ８番の最初の位置
#define _POKEMON_PLAYER_APPEAR_POSX _MCSS_POS_X( -50)  // 自分のポケモンのX
#define _POKEMON_PLAYER_APPEAR_POSY _MCSS_POS_Y(-150)  // 自分のポケモンのY
#define _POKEMON_PLAYER_APPEAR_POSZ _MCSS_POS_Z(  10)  // 自分のポケモンのZ
#define _POKEMON_FRIEND_APPEAR_POSX _MCSS_POS_X( 60)  // 相手のポケモンのX
#define _POKEMON_FRIEND_APPEAR_POSY _MCSS_POS_Y(80)  // 相手のポケモンのY
#define _POKEMON_FRIEND_APPEAR_POSZ _MCSS_POS_Z(-10)  // 相手のポケモンのZ

//絵コンテ９番の移動後の位置
#define _POKEMON_PLAYER_UP_POSX   _POKEMON_PLAYER_APPEAR_POSX  // 自分のポケモンのX
#define _POKEMON_PLAYER_UP_POSY   _MCSS_POS_Y(-55)             // 自分のポケモンのY
#define _POKEMON_PLAYER_UP_POSZ   _POKEMON_PLAYER_APPEAR_POSZ  // 自分のポケモンのZ
#define _POKEMON_FRIEND_DOWN_POSX _POKEMON_FRIEND_APPEAR_POSX  // 相手のポケモンのX
#define _POKEMON_FRIEND_DOWN_POSY _MCSS_POS_Y( 5)              // 相手のポケモンのY
#define _POKEMON_FRIEND_DOWN_POSZ _POKEMON_FRIEND_APPEAR_POSZ  // 相手のポケモンのZ


//絵コンテ１０番
#define _POKEMON_PLAYER_SIDEOUT_POSX _MCSS_POS_X(-140)           // 自分のポケモンのX
#define _POKEMON_PLAYER_SIDEOUT_POSY _POKEMON_PLAYER_UP_POSY    // 自分のポケモンのY
#define _POKEMON_PLAYER_SIDEOUT_POSZ _POKEMON_PLAYER_UP_POSZ    // 自分のポケモンのZ
#define _POKEMON_FRIEND_SIDEOUT_POSX _MCSS_POS_X(140)           // 相手のポケモンのX
#define _POKEMON_FRIEND_SIDEOUT_POSY _POKEMON_FRIEND_DOWN_POSY  // 相手のポケモンのY
#define _POKEMON_FRIEND_SIDEOUT_POSZ _POKEMON_FRIEND_DOWN_POSZ  // 相手のポケモンのZ

//絵コンテ１１番
#define _POKEMON_PLAYER_SIDEST_POSX _POKEMON_PLAYER_SIDEOUT_POSX  // 自分のポケモンが左から登場する場合のX
#define _POKEMON_PLAYER_SIDEST_POSY _MCSS_POS_Y(5)  // 自分のポケモンが左から登場する場合のY
#define _POKEMON_PLAYER_SIDEST_POSZ _MCSS_POS_Z(-10)  // 自分のポケモンが左から登場する場合のY
#define _POKEMON_FRIEND_SIDEST_POSX _POKEMON_FRIEND_SIDEOUT_POSX  // 相手のポケモンが右から登場する場合のX
#define _POKEMON_FRIEND_SIDEST_POSY _MCSS_POS_Y(-55)  // 相手のポケモンが右から登場する場合のY
#define _POKEMON_FRIEND_SIDEST_POSZ _MCSS_POS_Z(10)  // 相手のポケモンが右から登場する場合のY

//絵コンテ１２番
#define _POKEMON_PLAYER_SIDEIN_POSX _MCSS_POS_X( 60)  // 自分のポケモンが左から登場する場合のX
#define _POKEMON_PLAYER_SIDEIN_POSY _POKEMON_PLAYER_SIDEST_POSY  // 自分のポケモンが左から登場する場合のY
#define _POKEMON_PLAYER_SIDEIN_POSZ _POKEMON_PLAYER_SIDEST_POSZ  // 自分のポケモンが左から登場する場合のY
#define _POKEMON_FRIEND_SIDEIN_POSX _MCSS_POS_X(-45)  // 相手のポケモンが右から登場する場合のX
#define _POKEMON_FRIEND_SIDEIN_POSY _POKEMON_FRIEND_SIDEST_POSY  // 相手のポケモンが右から登場する場合のY
#define _POKEMON_FRIEND_SIDEIN_POSZ _POKEMON_FRIEND_SIDEST_POSZ  // 相手のポケモンが右から登場する場合のY

//絵コンテ１３番
#define _POKEMON_PLAYER_LEAVE_POSX _POKEMON_PLAYER_SIDEIN_POSX  // 自分のポケモンが上に退場する場合のX
#define _POKEMON_PLAYER_LEAVE_POSY _MCSS_POS_Y(100)  // 自分のポケモンが上に退場する場合のY
#define _POKEMON_PLAYER_LEAVE_POSZ _POKEMON_PLAYER_SIDEIN_POSZ  // 自分のポケモンが上に退場する場合のY
#define _POKEMON_FRIEND_LEAVE_POSX _POKEMON_FRIEND_SIDEIN_POSX  // 相手のポケモンが下に退場する場合のX
#define _POKEMON_FRIEND_LEAVE_POSY _MCSS_POS_Y(-160)  // 相手のポケモンが下に退場する場合のY
#define _POKEMON_FRIEND_LEAVE_POSZ _POKEMON_FRIEND_SIDEIN_POSZ  // 相手のポケモンが下に退場する場合のY


//手前のポケモンのサイズ
#define _POKMEON_SCALE_SIZE   (1.2)
