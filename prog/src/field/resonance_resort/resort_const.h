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
#define RESORT_CONST_WANTSPOT_FAVORITE_LOW    (10)
/*
 *  行きたいお店抽選でグループ中のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_MIDDLE (15)
/*
 *  行きたいお店抽選時でグループ強のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_HIGH   (20)
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
#define RESORT_CONST_PRICE_COEFFEICENT  FX32_CONST(0.01f)
/*
 *  お店の品物・サービスの価格
 *  フィーバー状態ではない場合のMAX係数
 *
 */
#define RESORT_CONST_FEVER_NONE_COEFFEICENT     FX32_CONST(1)
/*
 *  お店の品物・サービスの価格
 *  フィーバー状態のときのMAX係数
 *
 */
#define RESORT_CONST_FEVER_EXIST_COEFFEICENT    FX32_CONST(1.75f)
/*
 *  お店の品物・サービスの価格
 *  レベルキャップ
 *
 */
#define RESORT_CONST_LEVEL_CAP    (40)

/*
 *  コンボ  調子どう？で確率がわかる文
 *  ショップ確率＋トレーナー確率が下記値より↑だったら
 *  確率最大時のメッセージresort_resident_lv_04_mがでます。
 *    確率最大
 *
 */
#define RESORT_CONST_COMBO_RND_MSG_00    (20)
/*
 *  コンボ  調子どう？で確率がわかる文
 *  ショップ確率＋トレーナー確率が下記値より↑だったら
 *  確率最大時のメッセージresort_resident_lv_03_mがでます。
 *    確率中確率
 *
 */
#define RESORT_CONST_COMBO_RND_MSG_01    (10)
/*
 *  コンボ  調子どう？で確率がわかる文
 *  ショップ確率＋トレーナー確率が下記値より↑だったら
 *  確率最大時のメッセージresort_resident_lv_02_mがでます。
 *  この値より下回ったら何もでません。
 *    確率普通
 *
 */
#define RESORT_CONST_COMBO_RND_MSG_02    (5)

/*
 *  コンボ  コンボ経験値補正
 *  計算式は以下です
 *
 *  ( 1 + RESORT_CONST_COMBO_ADD_RATE * コンボ回数 ) * 経験値
 *
 */
#define RESORT_CONST_COMBO_ADD_RATE    (0.25)


/*
 *  コンボ  オススメされた先のお店で、気に入らなかった確率です
 *  分母は１００
 *
 */
#define RESORT_CONST_COMBO_SHOP_RESULT_RATE    (75)

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  お店を建てたときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//お店１
#define RESORT_BCAM_SHOP1_PITCH   (3877)
#define RESORT_BCAM_SHOP1_YAW     (49152)
#define RESORT_BCAM_SHOP1_DIST    (724992)
#define RESORT_BCAM_SHOP1_X       (1277952)
#define RESORT_BCAM_SHOP1_Y       (180224)
#define RESORT_BCAM_SHOP1_Z       (4161536)
//お店２
#define RESORT_BCAM_SHOP2_PITCH   (3877)
#define RESORT_BCAM_SHOP2_YAW     (16384)
#define RESORT_BCAM_SHOP2_DIST    (724992)
#define RESORT_BCAM_SHOP2_X       (753664)
#define RESORT_BCAM_SHOP2_Y       (184320)
#define RESORT_BCAM_SHOP2_Z       (3702784)
//お店３
#define RESORT_BCAM_SHOP3_PITCH   (3877)
#define RESORT_BCAM_SHOP3_YAW     (49152)
#define RESORT_BCAM_SHOP3_DIST    (724992)
#define RESORT_BCAM_SHOP3_X       (1277952)
#define RESORT_BCAM_SHOP3_Y       (180224)
#define RESORT_BCAM_SHOP3_Z       (3244032)
//お店４
#define RESORT_BCAM_SHOP4_PITCH   (3877)
#define RESORT_BCAM_SHOP4_YAW     (16384)
#define RESORT_BCAM_SHOP4_DIST    (724992)
#define RESORT_BCAM_SHOP4_X       (753664)
#define RESORT_BCAM_SHOP4_Y       (184320)
#define RESORT_BCAM_SHOP4_Z       (2785280)
//お店５
#define RESORT_BCAM_SHOP5_PITCH   (3877)
#define RESORT_BCAM_SHOP5_YAW     (49152)
#define RESORT_BCAM_SHOP5_DIST    (724992)
#define RESORT_BCAM_SHOP5_X       (1277952)
#define RESORT_BCAM_SHOP5_Y       (180224)
#define RESORT_BCAM_SHOP5_Z       (2326528)
//お店６
#define RESORT_BCAM_SHOP6_PITCH   (3877)
#define RESORT_BCAM_SHOP6_YAW     (16384)
#define RESORT_BCAM_SHOP6_DIST    (724992)
#define RESORT_BCAM_SHOP6_X       (753664)
#define RESORT_BCAM_SHOP6_Y       (184320)
#define RESORT_BCAM_SHOP6_Z       (1867776)
//お店７
#define RESORT_BCAM_SHOP7_PITCH   (3877)
#define RESORT_BCAM_SHOP7_YAW     (49152)
#define RESORT_BCAM_SHOP7_DIST    (724992)
#define RESORT_BCAM_SHOP7_X       (1277952)
#define RESORT_BCAM_SHOP7_Y       (180224)
#define RESORT_BCAM_SHOP7_Z       (1409024)
//お店８
#define RESORT_BCAM_SHOP8_PITCH   (3877)
#define RESORT_BCAM_SHOP8_YAW     (16384)
#define RESORT_BCAM_SHOP8_DIST    (724992)
#define RESORT_BCAM_SHOP8_X       (753664)
#define RESORT_BCAM_SHOP8_Y       (184320)
#define RESORT_BCAM_SHOP8_Z       (950272)

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  お店に案内するときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//お店１  段階１
#define RESORT_SCAM1_SHOP1_PITCH   (6821)
#define RESORT_SCAM1_SHOP1_YAW     (53376)
#define RESORT_SCAM1_SHOP1_DIST    (331776)
#define RESORT_SCAM1_SHOP1_X       (1277952)
#define RESORT_SCAM1_SHOP1_Y       (106496)
#define RESORT_SCAM1_SHOP1_Z       (4227072)
//        段階２
#define RESORT_SCAM2_SHOP1_PITCH   (5797)
#define RESORT_SCAM2_SHOP1_YAW     (54016)
#define RESORT_SCAM2_SHOP1_DIST    (430080)
#define RESORT_SCAM2_SHOP1_X       (1277952)
#define RESORT_SCAM2_SHOP1_Y       (135168)
#define RESORT_SCAM2_SHOP1_Z       (4235264)
//        段階３
#define RESORT_SCAM3_SHOP1_PITCH   (4517)
#define RESORT_SCAM3_SHOP1_YAW     (52480)
#define RESORT_SCAM3_SHOP1_DIST    (561152)
#define RESORT_SCAM3_SHOP1_X       (1277952)
#define RESORT_SCAM3_SHOP1_Y       (172032)
#define RESORT_SCAM3_SHOP1_Z       (4202496)
//お店２  段階１
#define RESORT_SCAM1_SHOP2_PITCH   (6693)
#define RESORT_SCAM1_SHOP2_YAW     (12288)
#define RESORT_SCAM1_SHOP2_DIST    (331776)
#define RESORT_SCAM1_SHOP2_X       (753664)
#define RESORT_SCAM1_SHOP2_Y       (94208)
#define RESORT_SCAM1_SHOP2_Z       (3764224)
//        段階２
#define RESORT_SCAM2_SHOP2_PITCH   (6693)
#define RESORT_SCAM2_SHOP2_YAW     (11776)
#define RESORT_SCAM2_SHOP2_DIST    (430080)
#define RESORT_SCAM2_SHOP2_X       (753664)
#define RESORT_SCAM2_SHOP2_Y       (155648)
#define RESORT_SCAM2_SHOP2_Z       (3792896)
//        段階３
#define RESORT_SCAM3_SHOP2_PITCH   (4773)
#define RESORT_SCAM3_SHOP2_YAW     (13696)
#define RESORT_SCAM3_SHOP2_DIST    (561152)
#define RESORT_SCAM3_SHOP2_X       (753664)
#define RESORT_SCAM3_SHOP2_Y       (180224)
#define RESORT_SCAM3_SHOP2_Z       (3727360)
//お店３  段階１
#define RESORT_SCAM1_SHOP3_PITCH   (6821)
#define RESORT_SCAM1_SHOP3_YAW     (53376)
#define RESORT_SCAM1_SHOP3_DIST    (331776)
#define RESORT_SCAM1_SHOP3_X       (1277952)
#define RESORT_SCAM1_SHOP3_Y       (106496)
#define RESORT_SCAM1_SHOP3_Z       (3309568)
//        段階２
#define RESORT_SCAM2_SHOP3_PITCH   (5797)
#define RESORT_SCAM2_SHOP3_YAW     (54016)
#define RESORT_SCAM2_SHOP3_DIST    (430080)
#define RESORT_SCAM2_SHOP3_X       (1277952)
#define RESORT_SCAM2_SHOP3_Y       (135168)
#define RESORT_SCAM2_SHOP3_Z       (3317760)
//        段階３
#define RESORT_SCAM3_SHOP3_PITCH   (4517)
#define RESORT_SCAM3_SHOP3_YAW     (52480)
#define RESORT_SCAM3_SHOP3_DIST    (561152)
#define RESORT_SCAM3_SHOP3_X       (1277952)
#define RESORT_SCAM3_SHOP3_Y       (172032)
#define RESORT_SCAM3_SHOP3_Z       (3284992)
//お店４  段階１
#define RESORT_SCAM1_SHOP4_PITCH   (6693)
#define RESORT_SCAM1_SHOP4_YAW     (12288)
#define RESORT_SCAM1_SHOP4_DIST    (331776)
#define RESORT_SCAM1_SHOP4_X       (753664)
#define RESORT_SCAM1_SHOP4_Y       (94208)
#define RESORT_SCAM1_SHOP4_Z       (2846720)
//        段階２
#define RESORT_SCAM2_SHOP4_PITCH   (6693)
#define RESORT_SCAM2_SHOP4_YAW     (11776)
#define RESORT_SCAM2_SHOP4_DIST    (430080)
#define RESORT_SCAM2_SHOP4_X       (753664)
#define RESORT_SCAM2_SHOP4_Y       (155648)
#define RESORT_SCAM2_SHOP4_Z       (2875392)
//        段階３
#define RESORT_SCAM3_SHOP4_PITCH   (4773)
#define RESORT_SCAM3_SHOP4_YAW     (13696)
#define RESORT_SCAM3_SHOP4_DIST    (561152)
#define RESORT_SCAM3_SHOP4_X       (753664)
#define RESORT_SCAM3_SHOP4_Y       (180224)
#define RESORT_SCAM3_SHOP4_Z       (2809856)
//お店５  段階１
#define RESORT_SCAM1_SHOP5_PITCH   (6821)
#define RESORT_SCAM1_SHOP5_YAW     (53376)
#define RESORT_SCAM1_SHOP5_DIST    (331776)
#define RESORT_SCAM1_SHOP5_X       (1277952)
#define RESORT_SCAM1_SHOP5_Y       (106496)
#define RESORT_SCAM1_SHOP5_Z       (2392064)
//        段階２
#define RESORT_SCAM2_SHOP5_PITCH   (5797)
#define RESORT_SCAM2_SHOP5_YAW     (54016)
#define RESORT_SCAM2_SHOP5_DIST    (430080)
#define RESORT_SCAM2_SHOP5_X       (1277952)
#define RESORT_SCAM2_SHOP5_Y       (135168)
#define RESORT_SCAM2_SHOP5_Z       (2400256)
//        段階３
#define RESORT_SCAM3_SHOP5_PITCH   (4517)
#define RESORT_SCAM3_SHOP5_YAW     (52480)
#define RESORT_SCAM3_SHOP5_DIST    (561152)
#define RESORT_SCAM3_SHOP5_X       (1277952)
#define RESORT_SCAM3_SHOP5_Y       (172032)
#define RESORT_SCAM3_SHOP5_Z       (2367488)
//お店６  段階１
#define RESORT_SCAM1_SHOP6_PITCH   (6693)
#define RESORT_SCAM1_SHOP6_YAW     (12288)
#define RESORT_SCAM1_SHOP6_DIST    (331776)
#define RESORT_SCAM1_SHOP6_X       (753664)
#define RESORT_SCAM1_SHOP6_Y       (94208)
#define RESORT_SCAM1_SHOP6_Z       (1929216)
//        段階２
#define RESORT_SCAM2_SHOP6_PITCH   (6693)
#define RESORT_SCAM2_SHOP6_YAW     (11776)
#define RESORT_SCAM2_SHOP6_DIST    (430080)
#define RESORT_SCAM2_SHOP6_X       (753664)
#define RESORT_SCAM2_SHOP6_Y       (155648)
#define RESORT_SCAM2_SHOP6_Z       (1957888)
//        段階３
#define RESORT_SCAM3_SHOP6_PITCH   (4773)
#define RESORT_SCAM3_SHOP6_YAW     (13696)
#define RESORT_SCAM3_SHOP6_DIST    (561152)
#define RESORT_SCAM3_SHOP6_X       (753664)
#define RESORT_SCAM3_SHOP6_Y       (180224)
#define RESORT_SCAM3_SHOP6_Z       (1892352)
//お店７  段階１
#define RESORT_SCAM1_SHOP7_PITCH   (6821)
#define RESORT_SCAM1_SHOP7_YAW     (53376)
#define RESORT_SCAM1_SHOP7_DIST    (331776)
#define RESORT_SCAM1_SHOP7_X       (1277952)
#define RESORT_SCAM1_SHOP7_Y       (106496)
#define RESORT_SCAM1_SHOP7_Z       (1474560)
//        段階２
#define RESORT_SCAM2_SHOP7_PITCH   (5797)
#define RESORT_SCAM2_SHOP7_YAW     (54016)
#define RESORT_SCAM2_SHOP7_DIST    (430080)
#define RESORT_SCAM2_SHOP7_X       (1277952)
#define RESORT_SCAM2_SHOP7_Y       (135168)
#define RESORT_SCAM2_SHOP7_Z       (1482752)
//        段階３
#define RESORT_SCAM3_SHOP7_PITCH   (4517)
#define RESORT_SCAM3_SHOP7_YAW     (52480)
#define RESORT_SCAM3_SHOP7_DIST    (561152)
#define RESORT_SCAM3_SHOP7_X       (1277952)
#define RESORT_SCAM3_SHOP7_Y       (172032)
#define RESORT_SCAM3_SHOP7_Z       (1449984)
//お店８  段階１
#define RESORT_SCAM1_SHOP8_PITCH   (6693)
#define RESORT_SCAM1_SHOP8_YAW     (12288)
#define RESORT_SCAM1_SHOP8_DIST    (331776)
#define RESORT_SCAM1_SHOP8_X       (753664)
#define RESORT_SCAM1_SHOP8_Y       (94208)
#define RESORT_SCAM1_SHOP8_Z       (1011712)
//        段階２
#define RESORT_SCAM2_SHOP8_PITCH   (6693)
#define RESORT_SCAM2_SHOP8_YAW     (11776)
#define RESORT_SCAM2_SHOP8_DIST    (430080)
#define RESORT_SCAM2_SHOP8_X       (753664)
#define RESORT_SCAM2_SHOP8_Y       (155648)
#define RESORT_SCAM2_SHOP8_Z       (1040384)
//        段階３
#define RESORT_SCAM3_SHOP8_PITCH   (4773)
#define RESORT_SCAM3_SHOP8_YAW     (13696)
#define RESORT_SCAM3_SHOP8_DIST    (561152)
#define RESORT_SCAM3_SHOP8_X       (753664)
#define RESORT_SCAM3_SHOP8_Y       (180224)
#define RESORT_SCAM3_SHOP8_Z       (974848)


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  街に経験値が入るときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define RESORT_CAM_CITY_PITCH   (64037)
#define RESORT_CAM_CITY_YAW     (0)
#define RESORT_CAM_CITY_DIST    (921600)
#define RESORT_CAM_CITY_X       (1015808)
#define RESORT_CAM_CITY_Y       (147456)
#define RESORT_CAM_CITY_Z       (2387968)
