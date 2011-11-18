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
#define RESORT_CONST_WANTSPOT_FAVORITE_LOW    (35)
/*
 *  行きたいお店抽選でグループ中のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_MIDDLE (30)
/*
 *  行きたいお店抽選時でグループ強のとき
 *  お気に入りに行きたい！という場合の確率
 *    ???/100
 */
#define RESORT_CONST_WANTSPOT_FAVORITE_HIGH   (25)
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
#define RESORT_BCAM_SHOP1_PITCH   (2981)
#define RESORT_BCAM_SHOP1_YAW     (49536)
#define RESORT_BCAM_SHOP1_DIST    (757760)
#define RESORT_BCAM_SHOP1_X       (1409024)
#define RESORT_BCAM_SHOP1_Y       (184320)
#define RESORT_BCAM_SHOP1_Z       (4161536)
//お店２
#define RESORT_BCAM_SHOP2_PITCH   (4389)
#define RESORT_BCAM_SHOP2_YAW     (16512)
#define RESORT_BCAM_SHOP2_DIST    (659456)
#define RESORT_BCAM_SHOP2_X       (622592)
#define RESORT_BCAM_SHOP2_Y       (135168)
#define RESORT_BCAM_SHOP2_Z       (3706880)
//お店３
#define RESORT_BCAM_SHOP3_PITCH   (4645)
#define RESORT_BCAM_SHOP3_YAW     (49024)
#define RESORT_BCAM_SHOP3_DIST    (528384)
#define RESORT_BCAM_SHOP3_X       (1409024)
#define RESORT_BCAM_SHOP3_Y       (65536)
#define RESORT_BCAM_SHOP3_Z       (3244032)
//お店４
#define RESORT_BCAM_SHOP4_PITCH   (2469)
#define RESORT_BCAM_SHOP4_YAW     (16640)
#define RESORT_BCAM_SHOP4_DIST    (626688)
#define RESORT_BCAM_SHOP4_X       (303104)
#define RESORT_BCAM_SHOP4_Y       (114688)
#define RESORT_BCAM_SHOP4_Z       (2793472)
//お店５
#define RESORT_BCAM_SHOP5_PITCH   (2597)
#define RESORT_BCAM_SHOP5_YAW     (49152)
#define RESORT_BCAM_SHOP5_DIST    (692224)
#define RESORT_BCAM_SHOP5_X       (1351680)
#define RESORT_BCAM_SHOP5_Y       (135168)
#define RESORT_BCAM_SHOP5_Z       (2326528)
//お店６
#define RESORT_BCAM_SHOP6_PITCH   (3493)
#define RESORT_BCAM_SHOP6_YAW     (16512)
#define RESORT_BCAM_SHOP6_DIST    (593920)
#define RESORT_BCAM_SHOP6_X       (622592)
#define RESORT_BCAM_SHOP6_Y       (110592)
#define RESORT_BCAM_SHOP6_Z       (1871872)
//お店７
#define RESORT_BCAM_SHOP7_PITCH   (933)
#define RESORT_BCAM_SHOP7_YAW     (49024)
#define RESORT_BCAM_SHOP7_DIST    (724992)
#define RESORT_BCAM_SHOP7_X       (1409024)
#define RESORT_BCAM_SHOP7_Y       (196608)
#define RESORT_BCAM_SHOP7_Z       (1409024)
//お店８
#define RESORT_BCAM_SHOP8_PITCH   (5925)
#define RESORT_BCAM_SHOP8_YAW     (16384)
#define RESORT_BCAM_SHOP8_DIST    (299008)
#define RESORT_BCAM_SHOP8_X       (610304)
#define RESORT_BCAM_SHOP8_Y       (61440)
#define RESORT_BCAM_SHOP8_Z       (942080)

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  お店に案内するときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//お店１  段階１
#define RESORT_SCAM1_SHOP1_PITCH   (1957)
#define RESORT_SCAM1_SHOP1_YAW     (55808)
#define RESORT_SCAM1_SHOP1_DIST    (495616)
#define RESORT_SCAM1_SHOP1_X       (1355776)
#define RESORT_SCAM1_SHOP1_Y       (294912)
#define RESORT_SCAM1_SHOP1_Z       (4280320)
//        段階２
#define RESORT_SCAM2_SHOP1_PITCH   (1957)
#define RESORT_SCAM2_SHOP1_YAW     (55808)
#define RESORT_SCAM2_SHOP1_DIST    (495616)
#define RESORT_SCAM2_SHOP1_X       (1355776)
#define RESORT_SCAM2_SHOP1_Y       (294912)
#define RESORT_SCAM2_SHOP1_Z       (4280320)
//        段階３
#define RESORT_SCAM3_SHOP1_PITCH   (1957)
#define RESORT_SCAM3_SHOP1_YAW     (55808)
#define RESORT_SCAM3_SHOP1_DIST    (495616)
#define RESORT_SCAM3_SHOP1_X       (1355776)
#define RESORT_SCAM3_SHOP1_Y       (294912)
#define RESORT_SCAM3_SHOP1_Z       (4280320)
//お店２  段階１
#define RESORT_SCAM1_SHOP2_PITCH   (1829)
#define RESORT_SCAM1_SHOP2_YAW     (9088)
#define RESORT_SCAM1_SHOP2_DIST    (397312)
#define RESORT_SCAM1_SHOP2_X       (618496)
#define RESORT_SCAM1_SHOP2_Y       (241664)
#define RESORT_SCAM1_SHOP2_Z       (3923968)
//        段階２
#define RESORT_SCAM2_SHOP2_PITCH   (1829)
#define RESORT_SCAM2_SHOP2_YAW     (9088)
#define RESORT_SCAM2_SHOP2_DIST    (397312)
#define RESORT_SCAM2_SHOP2_X       (618496)
#define RESORT_SCAM2_SHOP2_Y       (241664)
#define RESORT_SCAM2_SHOP2_Z       (3923968)
//        段階３
#define RESORT_SCAM3_SHOP2_PITCH   (1829)
#define RESORT_SCAM3_SHOP2_YAW     (9088)
#define RESORT_SCAM3_SHOP2_DIST    (397312)
#define RESORT_SCAM3_SHOP2_X       (618496)
#define RESORT_SCAM3_SHOP2_Y       (241664)
#define RESORT_SCAM3_SHOP2_Z       (3923968)
//お店３  段階１
#define RESORT_SCAM1_SHOP3_PITCH   (1701)
#define RESORT_SCAM1_SHOP3_YAW     (54272)
#define RESORT_SCAM1_SHOP3_DIST    (495616)
#define RESORT_SCAM1_SHOP3_X       (1454080)
#define RESORT_SCAM1_SHOP3_Y       (249856)
#define RESORT_SCAM1_SHOP3_Z       (3411968)
//        段階２
#define RESORT_SCAM2_SHOP3_PITCH   (1701)
#define RESORT_SCAM2_SHOP3_YAW     (54272)
#define RESORT_SCAM2_SHOP3_DIST    (495616)
#define RESORT_SCAM2_SHOP3_X       (1454080)
#define RESORT_SCAM2_SHOP3_Y       (249856)
#define RESORT_SCAM2_SHOP3_Z       (3411968)
//        段階３
#define RESORT_SCAM3_SHOP3_PITCH   (1701)
#define RESORT_SCAM3_SHOP3_YAW     (54272)
#define RESORT_SCAM3_SHOP3_DIST    (495616)
#define RESORT_SCAM3_SHOP3_X       (1454080)
#define RESORT_SCAM3_SHOP3_Y       (249856)
#define RESORT_SCAM3_SHOP3_Z       (3411968)
//お店４  段階１
#define RESORT_SCAM1_SHOP4_PITCH   (805)
#define RESORT_SCAM1_SHOP4_YAW     (10624)
#define RESORT_SCAM1_SHOP4_DIST    (561152)
#define RESORT_SCAM1_SHOP4_X       (622592)
#define RESORT_SCAM1_SHOP4_Y       (253952)
#define RESORT_SCAM1_SHOP4_Z       (2785280)
//        段階２
#define RESORT_SCAM2_SHOP4_PITCH   (805)
#define RESORT_SCAM2_SHOP4_YAW     (10624)
#define RESORT_SCAM2_SHOP4_DIST    (561152)
#define RESORT_SCAM2_SHOP4_X       (622592)
#define RESORT_SCAM2_SHOP4_Y       (253952)
#define RESORT_SCAM2_SHOP4_Z       (2785280)
//        段階３
#define RESORT_SCAM3_SHOP4_PITCH   (805)
#define RESORT_SCAM3_SHOP4_YAW     (10624)
#define RESORT_SCAM3_SHOP4_DIST    (561152)
#define RESORT_SCAM3_SHOP4_X       (622592)
#define RESORT_SCAM3_SHOP4_Y       (253952)
#define RESORT_SCAM3_SHOP4_Z       (2785280)
//お店５  段階１
#define RESORT_SCAM1_SHOP5_PITCH   (1829)
#define RESORT_SCAM1_SHOP5_YAW     (54912)
#define RESORT_SCAM1_SHOP5_DIST    (430080)
#define RESORT_SCAM1_SHOP5_X       (1409024)
#define RESORT_SCAM1_SHOP5_Y       (208896)
#define RESORT_SCAM1_SHOP5_Z       (2461696)
//        段階２
#define RESORT_SCAM2_SHOP5_PITCH   (1829)
#define RESORT_SCAM2_SHOP5_YAW     (54912)
#define RESORT_SCAM2_SHOP5_DIST    (430080)
#define RESORT_SCAM2_SHOP5_X       (1409024)
#define RESORT_SCAM2_SHOP5_Y       (208896)
#define RESORT_SCAM2_SHOP5_Z       (2461696)
//        段階３
#define RESORT_SCAM3_SHOP5_PITCH   (1829)
#define RESORT_SCAM3_SHOP5_YAW     (54912)
#define RESORT_SCAM3_SHOP5_DIST    (430080)
#define RESORT_SCAM3_SHOP5_X       (1409024)
#define RESORT_SCAM3_SHOP5_Y       (208896)
#define RESORT_SCAM3_SHOP5_Z       (2461696)
//お店６  段階１
#define RESORT_SCAM1_SHOP6_PITCH   (2085)
#define RESORT_SCAM1_SHOP6_YAW     (10880)
#define RESORT_SCAM1_SHOP6_DIST    (593920)
#define RESORT_SCAM1_SHOP6_X       (614400)
#define RESORT_SCAM1_SHOP6_Y       (282624)
#define RESORT_SCAM1_SHOP6_Z       (1961984)
//        段階２
#define RESORT_SCAM2_SHOP6_PITCH   (2085)
#define RESORT_SCAM2_SHOP6_YAW     (10880)
#define RESORT_SCAM2_SHOP6_DIST    (593920)
#define RESORT_SCAM2_SHOP6_X       (614400)
#define RESORT_SCAM2_SHOP6_Y       (282624)
#define RESORT_SCAM2_SHOP6_Z       (1961984)
//        段階３
#define RESORT_SCAM3_SHOP6_PITCH   (2085)
#define RESORT_SCAM3_SHOP6_YAW     (10880)
#define RESORT_SCAM3_SHOP6_DIST    (593920)
#define RESORT_SCAM3_SHOP6_X       (614400)
#define RESORT_SCAM3_SHOP6_Y       (282624)
#define RESORT_SCAM3_SHOP6_Z       (1961984)
//お店７  段階１
#define RESORT_SCAM1_SHOP7_PITCH   (1317)
#define RESORT_SCAM1_SHOP7_YAW     (54016)
#define RESORT_SCAM1_SHOP7_DIST    (364544)
#define RESORT_SCAM1_SHOP7_X       (1015808)
#define RESORT_SCAM1_SHOP7_Y       (253952)
#define RESORT_SCAM1_SHOP7_Z       (1605632)
//        段階２
#define RESORT_SCAM2_SHOP7_PITCH   (1317)
#define RESORT_SCAM2_SHOP7_YAW     (54016)
#define RESORT_SCAM2_SHOP7_DIST    (364544)
#define RESORT_SCAM2_SHOP7_X       (1015808)
#define RESORT_SCAM2_SHOP7_Y       (253952)
#define RESORT_SCAM2_SHOP7_Z       (1605632)
//        段階３
#define RESORT_SCAM3_SHOP7_PITCH   (1317)
#define RESORT_SCAM3_SHOP7_YAW     (54016)
#define RESORT_SCAM3_SHOP7_DIST    (364544)
#define RESORT_SCAM3_SHOP7_X       (1015808)
#define RESORT_SCAM3_SHOP7_Y       (253952)
#define RESORT_SCAM3_SHOP7_Z       (1605632)
//お店８  段階１
#define RESORT_SCAM1_SHOP8_PITCH   (2085)
#define RESORT_SCAM1_SHOP8_YAW     (9600)
#define RESORT_SCAM1_SHOP8_DIST    (626688)
#define RESORT_SCAM1_SHOP8_X       (745472)
#define RESORT_SCAM1_SHOP8_Y       (323584)
#define RESORT_SCAM1_SHOP8_Z       (1085440)
//        段階２
#define RESORT_SCAM2_SHOP8_PITCH   (2085)
#define RESORT_SCAM2_SHOP8_YAW     (9600)
#define RESORT_SCAM2_SHOP8_DIST    (626688)
#define RESORT_SCAM2_SHOP8_X       (745472)
#define RESORT_SCAM2_SHOP8_Y       (323584)
#define RESORT_SCAM2_SHOP8_Z       (1085440)
//        段階３
#define RESORT_SCAM3_SHOP8_PITCH   (2085)
#define RESORT_SCAM3_SHOP8_YAW     (9600)
#define RESORT_SCAM3_SHOP8_DIST    (626688)
#define RESORT_SCAM3_SHOP8_X       (745472)
#define RESORT_SCAM3_SHOP8_Y       (323584)
#define RESORT_SCAM3_SHOP8_Z       (1085440)

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/*
 *  街に経験値が入るときのカメラ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define RESORT_CAM_CITY_PITCH   (1957)
#define RESORT_CAM_CITY_YAW     (0)
#define RESORT_CAM_CITY_DIST    (888832)
#define RESORT_CAM_CITY_X       (1015808)
#define RESORT_CAM_CITY_Y       (208896)
#define RESORT_CAM_CITY_Z       (2392064)
