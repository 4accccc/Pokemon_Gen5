//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_const.h
 *	@brief  プランナーの人が触ってよい数値外部化
 *	@author	Toru=Nagihashi
 *	@date		2011.09.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================


/*
 *  行きたいお店抽選でグループ弱のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_LOW    (50)
/*
 *  行きたいお店抽選でグループ中のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_MIDDLE (50)
/*
 *  行きたいお店抽選時でグループ強のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_HIGH   (50)
/*
 *  お店に行き、◎のときベース経験値を
 *  どれぐらいアップさせるのかの割合
 *
 */
#define RESORT_CONST_SPOT_EXP_EXCELLENT FX32_CONST(2.0f)
/*
 *  お店に行き、○のときベース経験値を
 *  どれぐらいアップさせるのかの割合
 *
 */
#define RESORT_CONST_SPOT_EXP_GREAT FX32_CONST(1.0f)
/*
 *  お店に行き、△のときベース経験値を
 *  どれぐらいアップさせるのかの割合
 *
 */
#define RESORT_CONST_SPOT_EXP_GOOD FX32_CONST(0.5f)
/*
 *  お店にファンが行ったときの経験値レート
 *
 */
#define RESORT_CONST_FUN_EXP_RATE   FX32_CONST(0.5f)
/*
 *  お店にセレブが行ったときの経験値レート
 *
 */
#define RESORT_CONST_CELEB_EXP_RATE   FX32_CONST(2.0f)
/*
 *  お店の品物・サービスの価格
 *  基礎数
 *
 */
#define RESORT_CONST_PRICE_COEFFEICENT  FX32_CONST(0.002f)
/*
 *  お店の品物・サービスの価格
 *  フィーバー状態ではない場合のMAX係数
 *
 */
#define RESORT_CONST_FEVER_NONE_COEFFEICENT     (1)
/*
 *  お店の品物・サービスの価格
 *  フィーバー状態のときのMAX係数
 *
 */
#define RESORT_CONST_FEVER_EXIST_COEFFEICENT    (2)
