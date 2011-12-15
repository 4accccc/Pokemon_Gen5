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
#define RESORT_PROXY_TYPE_CELEB (5) //セレブ
#define RESORT_PROXY_TYPE_TUTORIAL_02 (3) //チュートリアル案内
#define RESORT_PROXY_TYPE_TUTORIAL_01 (2) //チュートリアル招待
#define RESORT_PROXY_TYPE_FUN (4) //ファン
#define RESORT_PROXY_TYPE_MAX (6) //最大
//観光客
#define RESORT_TURIST_TYPE_NONE (0) //種類なし
#define RESORT_TURIST_TYPE_MAX (1) //最大
//秘書
#define RESORT_SECRETARY_TYPE_ARRANGE (2) //整理
#define RESORT_SECRETARY_TYPE_MOVE (1) //移動
#define RESORT_SECRETARY_TYPE_WORK (0) //執務
#define RESORT_SECRETARY_TYPE_CENTER (3) //ポケセン
#define RESORT_SECRETARY_TYPE_MAX (4) //最大
//NPC
#define RESORT_NPC_TYPE_OFFRECO (1) //オフレコ
#define RESORT_NPC_TYPE_MOUTHCOMM (0) //クチコミ
#define RESORT_NPC_TYPE_MAX (2) //最大
//サービス
#define RESORT_SERVICE_TYPE_DEF_DOWN_FRIEND_UP (14) //防↓なつき↑
#define RESORT_SERVICE_TYPE_EFFORT_SPEDEF (7) //努力値特防
#define RESORT_SERVICE_TYPE_POW_DOWN_FRIEND_UP (13) //攻↓なつき↑
#define RESORT_SERVICE_TYPE_EFFORT_SPEPOW (6) //努力値特攻
#define RESORT_SERVICE_TYPE_TREASURE (11) //掘り出し物
#define RESORT_SERVICE_TYPE_EFFORT_AGI (8) //努力値素早
#define RESORT_SERVICE_TYPE_SD_DOWN_FRIEND_UP (16) //特防↓なつき↑
#define RESORT_SERVICE_TYPE_SP_DOWN_FRIEND_UP (15) //特攻↓なつき↑
#define RESORT_SERVICE_TYPE_FRIEND (1) //なつき
#define RESORT_SERVICE_TYPE_DRAWLOTS (10) //くじ抽選
#define RESORT_SERVICE_TYPE_EFFORT_DEF (5) //努力値防
#define RESORT_SERVICE_TYPE_EFFORT_POW (4) //努力値攻
#define RESORT_SERVICE_TYPE_EFFORT_HP (3) //努力値HP
#define RESORT_SERVICE_TYPE_AGI_DOWN_FFIEND_UP (17) //素早↓なつき↑
#define RESORT_SERVICE_TYPE_HP_DOWN_FRIEND_UP (12) //HP↓なつき↑
#define RESORT_SERVICE_TYPE_HATCH (26) //孵化
#define RESORT_SERVICE_TYPE_LV (2) //LV
#define RESORT_SERVICE_TYPE_ITEM (0) //道具
#define RESORT_SERVICE_TYPE_MAX (18) //最大
//スポットタイプ
#define RESORT_SPOT_TYPE_RANKDATA_06 (25) //捕まえたポケ
#define RESORT_SPOT_TYPE_MOUTHCOMM_07 (9) //クチコミ８
#define RESORT_SPOT_TYPE_MOUTHCOMM_08 (10) //クチコミ９
#define RESORT_SPOT_TYPE_RANKDATA_02 (21) //JA旅行者
#define RESORT_SPOT_TYPE_RANKDATA_05 (24) //通信対戦
#define RESORT_SPOT_TYPE_RANKDATA_03 (22) //購入金額
#define RESORT_SPOT_TYPE_MOUTHCOMM_09 (11) //クチコミ１０
#define RESORT_SPOT_TYPE_MOUTHCOMM_00 (2) //クチコミ１
#define RESORT_SPOT_TYPE_RANKDATA_04 (23) //すれ違った人
#define RESORT_SPOT_TYPE_MOUTHCOMM_10 (12) //クチコミ１１
#define RESORT_SPOT_TYPE_MOUTHCOMM_01 (3) //クチコミ２
#define RESORT_SPOT_TYPE_MOUTHCOMM_11 (13) //クチコミ１２
#define RESORT_SPOT_TYPE_MOUTHCOMM_02 (4) //クチコミ３
#define RESORT_SPOT_TYPE_SHOPTYPE (0) //住民
#define RESORT_SPOT_TYPE_NONE (18) //なし
#define RESORT_SPOT_TYPE_MOUTHCOMM_12 (14) //クチコミ１３
#define RESORT_SPOT_TYPE_MOUTHCOMM_03 (5) //クチコミ４
#define RESORT_SPOT_TYPE_SEX (1) //性別
#define RESORT_SPOT_TYPE_RANKDATA_08 (27) //メダル数
#define RESORT_SPOT_TYPE_RANKDATA_00 (19) //通信交換
#define RESORT_SPOT_TYPE_MOUTHCOMM_13 (15) //クチコミ１４
#define RESORT_SPOT_TYPE_MOUTHCOMM_04 (6) //クチコミ５
#define RESORT_SPOT_TYPE_MOUTHCOMM_14 (16) //クチコミ１５
#define RESORT_SPOT_TYPE_MOUTHCOMM_05 (7) //クチコミ６
#define RESORT_SPOT_TYPE_RANKDATA_07 (26) //孵化
#define RESORT_SPOT_TYPE_RANKDATA_01 (20) //ニックネーム
#define RESORT_SPOT_TYPE_MOUTHCOMM_15 (17) //クチコミ１６
#define RESORT_SPOT_TYPE_MOUTHCOMM_06 (8) //クチコミ７
#define RESORT_SPOT_TYPE_MAX (28) //最大
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
#define RESORT_APPEAL_LV_GROUP_NONE (3) //グループ無し
#define RESORT_APPEAL_LV_HIGH (2) //強
#define RESORT_APPEAL_LV_MAX (4) //最大
//プリセットタイプ
#define RESORT_PRESET_TYPE_SECRETARY_03 (6) //秘書３
#define RESORT_PRESET_TYPE_SECRETARY_04 (7) //秘書４
#define RESORT_PRESET_TYPE_TUTORIAL1 (0) //チュートリアル１
#define RESORT_PRESET_TYPE_TUTORIAL2 (1) //チュートリアル２
#define RESORT_PRESET_TYPE_APPOINT (3) //セレブ特定出現
#define RESORT_PRESET_TYPE_RANDOM (2) //ファンランダム出現
#define RESORT_PRESET_TYPE_SECRETARY_01 (4) //秘書１
#define RESORT_PRESET_TYPE_SECRETARY_02 (5) //秘書２
#define RESORT_PRESET_TYPE_MAX (8) //最大
//街ご褒美
#define RESORT_CITYPRIZE_TYPE_PARADE (7) //パレード
#define RESORT_CITYPRIZE_TYPE_CHANGE_CITYNAME (3) //街変更権利
#define RESORT_CITYPRIZE_TYPE_ADD_SECRETARY4 (2) //秘書４追加
#define RESORT_CITYPRIZE_TYPE_DENDOU (6) //殿堂解放
#define RESORT_CITYPRIZE_TYPE_ITEM (8) //アイテムをもらい続ける
#define RESORT_CITYPRIZE_TYPE_SCOUT_SECRETARY (5) //秘書スカウト権利
#define RESORT_CITYPRIZE_TYPE_CHANGE_CITYNEON (4) //ネオン変更権利
#define RESORT_CITYPRIZE_TYPE_NONE (0) //ご褒美なし
#define RESORT_CITYPRIZE_TYPE_ADD_SECRETARY3 (1) //秘書３追加
#define RESORT_CITYPRIZE_TYPE_MAX (9) //最大
//スポットサービス
#define RESORT_SPOTRANK_TYPE_RANKDATA_FIRST (0) //１位の人
#define RESORT_SPOTRANK_TYPE_MAX (1) //最大
//バージョン
#define RESORT_VERSION_W (1) //W
#define RESORT_VERSION_B (0) //B
#define RESORT_VERSION_W2 (3) //W2
#define RESORT_VERSION_B2 (2) //B2
#define RESORT_VERSION_MAX (4) //最大
//条件
#define RESORT_SPOT_CONDITION_MOUTHCOMM_12 (13) //条件クチコミ１３
#define RESORT_SPOT_CONDITION_MOUTHCOMM_05 (6) //条件クチコミ６
#define RESORT_SPOT_CONDITION_MOUTHCOMM_13 (14) //条件クチコミ１４
#define RESORT_SPOT_CONDITION_MOUTHCOMM_06 (7) //条件クチコミ７
#define RESORT_SPOT_CONDITION_MOUTHCOMM_14 (15) //条件クチコミ１５
#define RESORT_SPOT_CONDITION_MOUTHCOMM_07 (8) //条件クチコミ８
#define RESORT_SPOT_CONDITION_MOUTHCOMM_15 (16) //条件クチコミ１６
#define RESORT_SPOT_CONDITION_MOUTHCOMM_08 (9) //条件クチコミ９
#define RESORT_SPOT_CONDITION_MOUTHCOMM_00 (1) //条件クチコミ１
#define RESORT_SPOT_CONDITION_MOUTHCOMM_01 (2) //条件クチコミ２
#define RESORT_SPOT_CONDITION_MOUTHCOMM_09 (10) //条件クチコミ１０
#define RESORT_SPOT_CONDITION_MOUTHCOMM_02 (3) //条件クチコミ３
#define RESORT_SPOT_CONDITION_MOUTHCOMM_10 (11) //条件クチコミ１１
#define RESORT_SPOT_CONDITION_MOUTHCOMM_03 (4) //条件クチコミ４
#define RESORT_SPOT_CONDITION_NONE (0) //条件なし
#define RESORT_SPOT_CONDITION_MOUTHCOMM_11 (12) //条件クチコミ１２
#define RESORT_SPOT_CONDITION_MOUTHCOMM_04 (5) //条件クチコミ５
#define RESORT_SPOT_CONDITION_MAX (17) //最大
