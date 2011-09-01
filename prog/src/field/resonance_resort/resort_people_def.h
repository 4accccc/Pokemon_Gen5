//============================================================================================
/**
 * @file resort_people_def.h
 * @brief #レゾナンスリゾートバイナリの定義情報です
 * @author Toru=Nagihashi
 * @note conv_tool.rbから自動生成されています。
*/
//============================================================================================
#pragma once
//データ定義
//人物
#define RESORT_PEOPLE_TYPE_PROXY (1) //代理人
#define RESORT_PEOPLE_TYPE_SECRETARY (3) //秘書
#define RESORT_PEOPLE_TYPE_TOURIST (2) //旅行者
#define RESORT_PEOPLE_TYPE_RESIDENT (0) //住民
#define RESORT_PEOPLE_TYPE_MAX (4) //最大
//住民
#define RESORT_RESIDENT_TYPE_NURSERY (5) //保育所
#define RESORT_RESIDENT_TYPE_EXERCICE (4) //道場
#define RESORT_RESIDENT_TYPE_LOT (0) //くじ
#define RESORT_RESIDENT_TYPE_ANTIQUE (6) //古道具屋
#define RESORT_RESIDENT_TYPE_PARLOR (1) //美容室
#define RESORT_RESIDENT_TYPE_MARKET (2) //マーケット
#define RESORT_RESIDENT_TYPE_CAFE (7) //カフェ
#define RESORT_RESIDENT_TYPE_FLORIST (3) //花屋
#define RESORT_RESIDENT_TYPE_MAX (8) //最大
//代理人
#define RESORT_PROXY_TYPE_PDW (1) //PDW
#define RESORT_PROXY_TYPE_GTS (0) //GTS
#define RESORT_PROXY_TYPE_MAX (2) //最大
//観光客
#define RESORT_TURIST_TYPE_NONE (0) //種類なし
#define RESORT_TURIST_TYPE_MAX (1) //最大
//秘書
#define RESORT_SECRETARY_TYPE_MOVE (1) //移動
#define RESORT_SECRETARY_TYPE_WORK (0) //執務
#define RESORT_SECRETARY_TYPE_PC (3) //パソコン
#define RESORT_SECRETARY_TYPE_CENTER (2) //ポケセン
#define RESORT_SECRETARY_TYPE_MAX (4) //最大
//NPC
#define RESORT_NPC_TYPE_OFFRECO (1) //オフレコ
#define RESORT_NPC_TYPE_MOUTHCOMM (0) //クチコミ
#define RESORT_NPC_TYPE_MAX (2) //最大
//サービス
#define RESORT_SERVICE_TYPE_EFFORT_SPEDEF (7) //努力値特防
#define RESORT_SERVICE_TYPE_EFFORT_SPEPOW (6) //努力値特攻
#define RESORT_SERVICE_TYPE_EFFORT_AGI (8) //努力値素早
#define RESORT_SERVICE_TYPE_FRIEND (1) //なつき
#define RESORT_SERVICE_TYPE_EFFORT_DEF (5) //努力値防
#define RESORT_SERVICE_TYPE_EFFORT_POW (4) //努力値攻
#define RESORT_SERVICE_TYPE_EFFORT_HP (3) //努力値HP
#define RESORT_SERVICE_TYPE_HATCH (9) //孵化
#define RESORT_SERVICE_TYPE_LV (2) //LV
#define RESORT_SERVICE_TYPE_ITEM (0) //道具
#define RESORT_SERVICE_TYPE_MAX (10) //最大
//スポットタイプ
#define RESORT_SPOT_TYPE_MOUTHCOMM_07 (9) //クチコミ８
#define RESORT_SPOT_TYPE_MOUTHCOMM_08 (10) //クチコミ９
#define RESORT_SPOT_TYPE_MOUTHCOMM_09 (11) //クチコミ１０
#define RESORT_SPOT_TYPE_MOUTHCOMM_00 (2) //クチコミ１
#define RESORT_SPOT_TYPE_MOUTHCOMM_10 (12) //クチコミ１１
#define RESORT_SPOT_TYPE_MOUTHCOMM_01 (3) //クチコミ２
#define RESORT_SPOT_TYPE_MOUTHCOMM_11 (13) //クチコミ１２
#define RESORT_SPOT_TYPE_MOUTHCOMM_02 (4) //クチコミ３
#define RESORT_SPOT_TYPE_SHOPTYPE (0) //住民
#define RESORT_SPOT_TYPE_NONE (18) //なし
#define RESORT_SPOT_TYPE_MOUTHCOMM_12 (14) //クチコミ１３
#define RESORT_SPOT_TYPE_MOUTHCOMM_03 (5) //クチコミ４
#define RESORT_SPOT_TYPE_SEX (1) //性別
#define RESORT_SPOT_TYPE_MOUTHCOMM_13 (15) //クチコミ１４
#define RESORT_SPOT_TYPE_MOUTHCOMM_04 (6) //クチコミ５
#define RESORT_SPOT_TYPE_MOUTHCOMM_14 (16) //クチコミ１５
#define RESORT_SPOT_TYPE_MOUTHCOMM_05 (7) //クチコミ６
#define RESORT_SPOT_TYPE_MOUTHCOMM_15 (17) //クチコミ１６
#define RESORT_SPOT_TYPE_MOUTHCOMM_06 (8) //クチコミ７
#define RESORT_SPOT_TYPE_MAX (19) //最大
//評価
#define RESORT_GRADE_TYPE_GOOD (0) //△
#define RESORT_GRADE_TYPE_GREAT (1) //○
#define RESORT_GRADE_TYPE_EXCELLENT (2) //◎
#define RESORT_GRADE_TYPE_MAX (3) //最大
//オフレコタイプ
#define RESORT_OFFRECO_TYPE_TEST1 (0) //テスト１
#define RESORT_OFFRECO_TYPE_TEST2 (1) //テスト２
#define RESORT_OFFRECO_TYPE_TEST3 (2) //テスト３
#define RESORT_OFFRECO_TYPE_ALL_BIT_OFF (3) //全ビットOFF
#define RESORT_OFFRECO_TYPE_MAX (4) //最大
//行きたいお店グループ
#define RESORT_APPEAL_LV_MIDDLE (1) //中
#define RESORT_APPEAL_LV_LOW (0) //弱
#define RESORT_APPEAL_LV_HIGH (2) //強
#define RESORT_APPEAL_LV_MAX (3) //最大
