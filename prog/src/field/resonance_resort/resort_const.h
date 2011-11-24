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
/*
 *  お店の品物・サービスの価格
 *  レベルキャップ
 *
 */
#define RESORT_CONST_LEVEL_CAP    (50)
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
#define RESORT_SCAM1_SHOP2_PITCH   (7205)
#define RESORT_SCAM1_SHOP2_YAW     (13568)
#define RESORT_SCAM1_SHOP2_DIST    (790528)
#define RESORT_SCAM1_SHOP2_X       (745472)
#define RESORT_SCAM1_SHOP2_Y       (139264)
#define RESORT_SCAM1_SHOP2_Z       (3743744)
//        段階２
#define RESORT_SCAM2_SHOP2_PITCH   (7205)
#define RESORT_SCAM2_SHOP2_YAW     (13568)
#define RESORT_SCAM2_SHOP2_DIST    (790528)
#define RESORT_SCAM2_SHOP2_X       (745472)
#define RESORT_SCAM2_SHOP2_Y       (139264)
#define RESORT_SCAM2_SHOP2_Z       (3743744)
//        段階３
#define RESORT_SCAM3_SHOP2_PITCH   (7205)
#define RESORT_SCAM3_SHOP2_YAW     (13568)
#define RESORT_SCAM3_SHOP2_DIST    (790528)
#define RESORT_SCAM3_SHOP2_X       (745472)
#define RESORT_SCAM3_SHOP2_Y       (139264)
#define RESORT_SCAM3_SHOP2_Z       (3743744)
//お店３  段階１
#define RESORT_SCAM1_SHOP3_PITCH   (7077)
#define RESORT_SCAM1_SHOP3_YAW     (51968)
#define RESORT_SCAM1_SHOP3_DIST    (757760)
#define RESORT_SCAM1_SHOP3_X       (1314816)
#define RESORT_SCAM1_SHOP3_Y       (176128)
#define RESORT_SCAM1_SHOP3_Z       (3276800)
//        段階２
#define RESORT_SCAM2_SHOP3_PITCH   (7077)
#define RESORT_SCAM2_SHOP3_YAW     (51968)
#define RESORT_SCAM2_SHOP3_DIST    (757760)
#define RESORT_SCAM2_SHOP3_X       (1314816)
#define RESORT_SCAM2_SHOP3_Y       (176128)
#define RESORT_SCAM2_SHOP3_Z       (3276800)
//        段階３
#define RESORT_SCAM3_SHOP3_PITCH   (7077)
#define RESORT_SCAM3_SHOP3_YAW     (51968)
#define RESORT_SCAM3_SHOP3_DIST    (757760)
#define RESORT_SCAM3_SHOP3_X       (1314816)
#define RESORT_SCAM3_SHOP3_Y       (176128)
#define RESORT_SCAM3_SHOP3_Z       (3276800)
//お店４  段階１
#define RESORT_SCAM1_SHOP4_PITCH   (6949)
#define RESORT_SCAM1_SHOP4_YAW     (13824)
#define RESORT_SCAM1_SHOP4_DIST    (692224)
#define RESORT_SCAM1_SHOP4_X       (737280)
#define RESORT_SCAM1_SHOP4_Y       (188416)
#define RESORT_SCAM1_SHOP4_Z       (2813952)
//        段階２
#define RESORT_SCAM2_SHOP4_PITCH   (6949)
#define RESORT_SCAM2_SHOP4_YAW     (13824)
#define RESORT_SCAM2_SHOP4_DIST    (692224)
#define RESORT_SCAM2_SHOP4_X       (737280)
#define RESORT_SCAM2_SHOP4_Y       (188416)
#define RESORT_SCAM2_SHOP4_Z       (2813952)
//        段階３
#define RESORT_SCAM3_SHOP4_PITCH   (6949)
#define RESORT_SCAM3_SHOP4_YAW     (13824)
#define RESORT_SCAM3_SHOP4_DIST    (692224)
#define RESORT_SCAM3_SHOP4_X       (737280)
#define RESORT_SCAM3_SHOP4_Y       (188416)
#define RESORT_SCAM3_SHOP4_Z       (2813952)
//お店５  段階１
#define RESORT_SCAM1_SHOP5_PITCH   (6693)
#define RESORT_SCAM1_SHOP5_YAW     (51840)
#define RESORT_SCAM1_SHOP5_DIST    (724992)
#define RESORT_SCAM1_SHOP5_X       (1277952)
#define RESORT_SCAM1_SHOP5_Y       (188416)
#define RESORT_SCAM1_SHOP5_Z       (2363392)
//        段階２
#define RESORT_SCAM2_SHOP5_PITCH   (6693)
#define RESORT_SCAM2_SHOP5_YAW     (51840)
#define RESORT_SCAM2_SHOP5_DIST    (724992)
#define RESORT_SCAM2_SHOP5_X       (1277952)
#define RESORT_SCAM2_SHOP5_Y       (188416)
#define RESORT_SCAM2_SHOP5_Z       (2363392)
//        段階３
#define RESORT_SCAM3_SHOP5_PITCH   (6693)
#define RESORT_SCAM3_SHOP5_YAW     (51840)
#define RESORT_SCAM3_SHOP5_DIST    (724992)
#define RESORT_SCAM3_SHOP5_X       (1277952)
#define RESORT_SCAM3_SHOP5_Y       (188416)
#define RESORT_SCAM3_SHOP5_Z       (2363392)
//お店６  段階１
#define RESORT_SCAM1_SHOP6_PITCH   (6437)
#define RESORT_SCAM1_SHOP6_YAW     (13568)
#define RESORT_SCAM1_SHOP6_DIST    (692224)
#define RESORT_SCAM1_SHOP6_X       (753664)
#define RESORT_SCAM1_SHOP6_Y       (176128)
#define RESORT_SCAM1_SHOP6_Z       (1925120)
//        段階２
#define RESORT_SCAM2_SHOP6_PITCH   (6437)
#define RESORT_SCAM2_SHOP6_YAW     (13568)
#define RESORT_SCAM2_SHOP6_DIST    (692224)
#define RESORT_SCAM2_SHOP6_X       (753664)
#define RESORT_SCAM2_SHOP6_Y       (176128)
#define RESORT_SCAM2_SHOP6_Z       (1925120)
//        段階３
#define RESORT_SCAM3_SHOP6_PITCH   (6437)
#define RESORT_SCAM3_SHOP6_YAW     (13568)
#define RESORT_SCAM3_SHOP6_DIST    (692224)
#define RESORT_SCAM3_SHOP6_X       (753664)
#define RESORT_SCAM3_SHOP6_Y       (176128)
#define RESORT_SCAM3_SHOP6_Z       (1925120)
//お店７  段階１
#define RESORT_SCAM1_SHOP7_PITCH   (6949)
#define RESORT_SCAM1_SHOP7_YAW     (51712)
#define RESORT_SCAM1_SHOP7_DIST    (692224)
#define RESORT_SCAM1_SHOP7_X       (1269760)
#define RESORT_SCAM1_SHOP7_Y       (200704)
#define RESORT_SCAM1_SHOP7_Z       (1445888)
//        段階２
#define RESORT_SCAM2_SHOP7_PITCH   (6949)
#define RESORT_SCAM2_SHOP7_YAW     (51712)
#define RESORT_SCAM2_SHOP7_DIST    (692224)
#define RESORT_SCAM2_SHOP7_X       (1269760)
#define RESORT_SCAM2_SHOP7_Y       (200704)
#define RESORT_SCAM2_SHOP7_Z       (1445888)
//        段階３
#define RESORT_SCAM3_SHOP7_PITCH   (6949)
#define RESORT_SCAM3_SHOP7_YAW     (51712)
#define RESORT_SCAM3_SHOP7_DIST    (692224)
#define RESORT_SCAM3_SHOP7_X       (1269760)
#define RESORT_SCAM3_SHOP7_Y       (200704)
#define RESORT_SCAM3_SHOP7_Z       (1445888)
//お店８  段階１
#define RESORT_SCAM1_SHOP8_PITCH   (6821)
#define RESORT_SCAM1_SHOP8_YAW     (13056)
#define RESORT_SCAM1_SHOP8_DIST    (724992)
#define RESORT_SCAM1_SHOP8_X       (753664)
#define RESORT_SCAM1_SHOP8_Y       (188416)
#define RESORT_SCAM1_SHOP8_Z       (995328)
//        段階２
#define RESORT_SCAM2_SHOP8_PITCH   (6821)
#define RESORT_SCAM2_SHOP8_YAW     (13056)
#define RESORT_SCAM2_SHOP8_DIST    (724992)
#define RESORT_SCAM2_SHOP8_X       (753664)
#define RESORT_SCAM2_SHOP8_Y       (188416)
#define RESORT_SCAM2_SHOP8_Z       (995328)
//        段階３
#define RESORT_SCAM3_SHOP8_PITCH   (6821)
#define RESORT_SCAM3_SHOP8_YAW     (13056)
#define RESORT_SCAM3_SHOP8_DIST    (724992)
#define RESORT_SCAM3_SHOP8_X       (753664)
#define RESORT_SCAM3_SHOP8_Y       (188416)
#define RESORT_SCAM3_SHOP8_Z       (995328)

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  街に経験値が入るときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define RESORT_CAM_CITY_PITCH   (62245)
#define RESORT_CAM_CITY_YAW     (0)
#define RESORT_CAM_CITY_DIST    (921600)
#define RESORT_CAM_CITY_X       (1015808)
#define RESORT_CAM_CITY_Y       (282624)
#define RESORT_CAM_CITY_Z       (4096000)
