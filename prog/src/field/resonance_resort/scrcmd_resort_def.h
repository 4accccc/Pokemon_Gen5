//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.h
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/*
 *					スクリプトコマンドとソースの共通定義
*/
//=============================================================================
//_RESORT_GET_DATA用
#define RESORT_GET_PARAM_FRONT_OBJID        (0)
#define RESORT_GET_PARAM_IS_SECREARY_FULL   (1)
#define RESORT_GET_PARAM_MERGE_FLAG         (2)
#define RESORT_GET_PARAM_PRESENT_ID         (3)
#define RESORT_GET_PARAM_IS_INSTITUTION_EMPTY   (6)
#define RESORT_GET_PARAM_IS_EVENT           (7)
#define RESORT_GET_PARAM_TYPE               (8)
#define RESORT_GET_PARAM_DAY_FLAG           (9)
#define RESORT_GET_PARAM_IS_INSTITUTION_FULL  (10)
#define RESORT_GET_PARAM_SHOPKIND             (11)
#define RESORT_GET_PARAM_HAS_RECORD         (12)
#define RESORT_GET_PARAM_HAS_MEDAL          (13)
#define RESORT_GET_PARAM_NUM_RECORD         (15)
#define RESORT_GET_PARAM_CHECK_SCOUT        (16)
#define RESORT_GET_PARAM_SELECT_LOT         (17)
#define RESORT_GET_PARAM_SELECT_PARLOR      (18)
#define RESORT_GET_PARAM_SELECT_FLORIST     (19)
#define RESORT_GET_PARAM_SELECT_MARKET      (20)
#define RESORT_GET_PARAM_SELECT_EXERCICE    (21)
#define RESORT_GET_PARAM_SELECT_NURSERY     (22)
#define RESORT_GET_PARAM_SELECT_ANTIQUE     (23)
#define RESORT_GET_PARAM_SELECT_CAFE        (24)

//RESORT_GET_PARAM_CHECK_SCOUTの戻り値
#define RESORT_CHECK_SCOUT_YES          (0) //いいよ
#define RESORT_CHECK_SCOUT_NO           (1) //だめ
#define RESORT_CHECK_SCOUT_REFUSE       (2) //興味なし

//_RESORT_SET_MY_DATA, _RESORT_GET_MY_DATA用
#define RESORT_MY_PARAM_GOSSIP_00          (0)
#define RESORT_MY_PARAM_GOSSIP_01          (1)
#define RESORT_MY_PARAM_GOSSIP_02          (2)
#define RESORT_MY_PARAM_GOSSIP_03          (3)
#define RESORT_MY_PARAM_GOSSIP_04          (4)
#define RESORT_MY_PARAM_GOSSIP_05          (5)
#define RESORT_MY_PARAM_GOSSIP_06          (6)
#define RESORT_MY_PARAM_GOSSIP_07          (7)
#define RESORT_MY_PARAM_GOSSIP_08          (8)
#define RESORT_MY_PARAM_GOSSIP_09          (9)
#define RESORT_MY_PARAM_GOSSIP_10          (10)
#define RESORT_MY_PARAM_GOSSIP_11          (11)
#define RESORT_MY_PARAM_GOSSIP_12          (12)
#define RESORT_MY_PARAM_GOSSIP_13          (13)
#define RESORT_MY_PARAM_GOSSIP_14          (14)
#define RESORT_MY_PARAM_GOSSIP_15          (15)
#define RESORT_MY_PARAM_WANT_JOB           (16)
#define RESORT_MY_PARAM_TEX                (17)
#define RESORT_MY_PARAM_IS_TUTORIAL        (18)

//_RESORT_JOB_CHANGE用
#define RESORT_JOBCHANGE_RESIDENT           (0)
#define RESORT_JOBCHANGE_SECETARY           (1)
///_RESORT_ACTION用
#define RESORT_ACTION_LEAVE           (0)
//_RESORT_SET_WORD用
#define RESORT_WORD_PEOPLE            (0)
#define RESORT_WORD_SHOP              (1) //「フラワー○○」とか（現在のお店）
#define RESORT_WORD_SHOP_LABEL        (2) //「売る」とか
#define RESORT_WORD_FREE_GREET        (3)
#define RESORT_WORD_FREE_NAME         (4)
#define RESORT_WORD_COMM_TYPE         (5) //通信種別
#define RESORT_WORD_FREE_PHRASE       (6)
#define RESORT_WORD_SHOP_GENRE        (7) //「花屋」とか(現在のお店)
#define RESORT_WORD_FREE_CITY         (8)
#define RESORT_WORD_SHOP_RESTEXP      (9) //次のレベルまでの経験値
#define RESORT_WORD_SHOP_LV           (10)
#define RESORT_WORD_CITY_LV           (11)
#define RESORT_WORD_CITY_RANK         (12)
#define RESORT_WORD_CITY_RESTEXP      (13)
#define RESORT_WORD_WANT_SHOP         (14) //「フラワー○○」とか（なりたいお店）
#define RESORT_WORD_WANT_SHOP_GENRE   (15) //「花屋」とか（なりたいお店）
#define RESORT_WORD_OWNER_SHOP_GENRE  (16) //「花屋」とか（プレイヤーの得意なお店）

#define RESORT_WORD_PARAM_FRONT       (0xFE)  //目の前の人
#define RESORT_WORD_PARAM_NONE        (0xFF)

//_RESORT_CALL_LIST用
#define RESORT_LIST_TYPE_BUY            (0)
#define RESORT_LIST_TYPE_SELECT         (1)
#define RESORT_LIST_TYPE_SORT           (2)

#define RESORT_LIST_CANCEL              (0xFF)

  //RESORT_LIST_TYPE_SELECT
#define RESORT_LIST_SELECT_SECRETARY_ADD      (0)
#define RESORT_LIST_SELECT_SECRETARY_DEL      (1)
#define RESORT_LIST_SELECT_INSTATITUTION      (2)
#define RESORT_LIST_SELECT_INSTATITUTION_DEL  (3)

  //RESORT_LIST_TYPE_SORT
#define RESORT_LIST_SORT_SECRETARY      (0)
#define RESORT_LIST_SORT_INSTATITUTION  (1)

//_RESORT_GET_MSGID用
  //特殊MSG
#define RESORT_MSGID_RECENT_00      (0)  //できごと文
#define RESORT_MSGID_RECENT_01      (1)  //できごと文
#define RESORT_MSGID_RECENT_02      (2)  //できごと文
#define RESORT_MSGID_RECENT_03      (3)  //できごと文
#define RESORT_MSGID_WHERETOGO      (4)  //どこへ行きたいか
#define RESORT_MSGID_APPEAL         (5)  //アピール文
#define RESORT_MSGID_SHOP_INFO      (6)  //お店情報
#define RESORT_MSGID_GOSSIP         (7)  //噂（特殊NPCの話)
#define RESORT_MSGID_MEDAL_HAVE     (8)  //メダル自慢
#define RESORT_MSGID_MEDAL_NONE     (9)  //メダルないよ
#define RESORT_MSGID_RECORD_NEXT    (10)  //出来事まだあるよ
#define RESORT_MSGID_RECORD_NONE    (11)  //出来事なし
//看板
#define RESORT_MSGID_BOARD_00       (12)  //看板０番目の施設
#define RESORT_MSGID_BOARD_01       (13)  //看板０番目の施設
#define RESORT_MSGID_BOARD_02       (14)  //看板０番目の施設
#define RESORT_MSGID_BOARD_03       (15)  //看板０番目の施設
#define RESORT_MSGID_BOARD_04       (16)  //看板０番目の施設
#define RESORT_MSGID_BOARD_05       (17)  //看板０番目の施設
#define RESORT_MSGID_BOARD_06       (18)  //看板０番目の施設
#define RESORT_MSGID_BOARD_07       (19)  //看板０番目の施設
  //旅行者
#define RESORT_TOURIST_GREET_MSGID        (50)  //挨拶
#define RESORT_TOURIST_CITY_MSGID         (51)  //街興味
#define RESORT_TOURIST_SCOUT_YES_MSGID    (52)  //スカウトはい
#define RESORT_TOURIST_SCOUT_THANKS_MSGID (53)  //スカウトありがとう
#define RESORT_TOURIST_SCOUT_NO_MSGID     (54)  //スカウトいいえ
#define RESORT_TOURIST_SCOUT_REFUSE_MSGID (55)  //スカウト興味なし
#define RESORT_TOURIST_SPOT_GOING         (56)  //いってきます
#define RESORT_TOURIST_ITEM_MSGID         (57) //アイテムわたし
#define RESORT_TOURIST_BYE_MSGID          (58) //またね
#define RESORT_TOURIST_JOY_MSGID          (59) //招待喜び
  //住人
#define RESORT_RESIDENT_GREET_MSGID           (80)  //挨拶
#define RESORT_RESIDENT_BYE_MSGID             (81)  //またね
#define RESORT_RESIDENT_CHANGE_MSGID          (82)  //転職したい
#define RESORT_RESIDENT_CHANGE_CONFIM_MSGID   (83)  //転職確認
#define RESORT_RESIDENT_CHANGE_OK_MSGID       (84)  //転職OKがんばります
#define RESORT_RESIDENT_CHANGE_NG_MSGID       (85)  //転職NGこのままやります
#define RESORT_RESIDENT_LV_MSGID              (86)  //お店LV
  //秘書
#define RESORT_SECRETARY_GREET_MSGID          (100) //挨拶
#define RESORT_SECRETARY_BYE_MSGID            (101) //またね
#define RESORT_SECRETARY_CITYLV_MSGID         (102) //まちレベル
#define RESORT_SECRETARY_SECSORT_MSGID        (103) //秘書交換
#define RESORT_SECRETARY_SHOPSORT_MSGID       (104) //お店交換
#define RESORT_SECRETARY_NEON_OK_MSGID        (105) //ネオン変更
#define RESORT_SECRETARY_NEON_SEL_MSGID       (106) //ネオン選択

#define RESORT_SECRETARY_SEL_GREET_MSGID      (110) //挨拶
#define RESORT_SECRETARY_SEL_BYE_MSGID        (111) //またね
#define RESORT_SECRETARY_SEL_SELECT           (112) //なにがしたいですか？
#define RESORT_SECRETARY_SEL_ONE              (113) //１つだけ
#define RESORT_SECRETARY_SEL_MANY             (114) //複数

  //ファン
#define RESORT_FUN_GREET_MSGID                (120) //挨拶

//_RESORT_SEARCH_RECEPTION用
#define RESORT_SEARCH_PROXY_GTS               (0)
#define RESORT_SEARCH_PROXY_MYSTERY           (0)

//_RESORT_LOT用
#define RESORT_LOT_FAILURE                    (0)

//_RESORT_ADD_EFFORT
#define RESORT_ADD_EFFORT_HP                  (0)
#define RESORT_ADD_EFFORT_POW                 (1)
#define RESORT_ADD_EFFORT_DEF                 (2)
#define RESORT_ADD_EFFORT_AGI                 (3)
#define RESORT_ADD_EFFORT_SPEPOW              (4)
#define RESORT_ADD_EFFORT_SPEDEF              (5)

//_RESORT_DELETE_PEOPLE用
#define RESORT_DELTE_PEOPLE_MINE              (0xFF)

//_RESORT_GET_NPC_DATA用
#define RESORT_NPC_DATA_OBJID                 (0)
#define RESORT_NPC_DATA_INDEX                 (1)
#define RESORT_NPC_DATA_MOUTHCOMM_Q           (2)
#define RESORT_NPC_DATA_MOUTHCOMM_A_NUM       (3)
#define RESORT_NPC_DATA_MOUTHCOMM_A_00        (4)
#define RESORT_NPC_DATA_MOUTHCOMM_A_01        (5)
#define RESORT_NPC_DATA_MOUTHCOMM_A_02        (6)
#define RESORT_NPC_DATA_MOUTHCOMM_A_03        (7)
#define RESORT_NPC_DATA_MOUTHCOMM_A_04        (8)
#define RESORT_NPC_DATA_MOUTHCOMM_A_05        (9)
#define RESORT_NPC_DATA_MOUTHCOMM_I_00        (10)
#define RESORT_NPC_DATA_MOUTHCOMM_I_01        (11)
#define RESORT_NPC_DATA_MOUTHCOMM_I_02        (12)
#define RESORT_NPC_DATA_MOUTHCOMM_I_03        (13)
#define RESORT_NPC_DATA_MOUTHCOMM_I_04        (14)
#define RESORT_NPC_DATA_MOUTHCOMM_I_05        (15)

#define RESORT_NPC_DATA_MOUTHCOMM_MSG_00      (16)  //初期挨拶文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_01      (17)  //通常挨拶文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_02      (18)  //回答確認文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_03      (19)  //繰り返し文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_04      (20)  //噂がないとき文

//_RESORT_GET_SERVICE_DATA用
#define RESORT_SERVICE_DATA_EFFECT            (0)

//_RESORT_SET_SPOT用
#define RESORT_SET_SPOT_SHOP_NONE             (0)
#define RESORT_SET_SPOT_SHOP_LVUP             (1)
#define RESORT_SET_SPOT_SHOP_CHANGE           (2)
#define RESORT_SET_SPOT_SHOP_LVMAX            (3)

#define RESORT_SET_SPOT_CITY_NONE             (0)
#define RESORT_SET_SPOT_CITY_LVUP             (1)
#define RESORT_SET_SPOT_CITY_LVMAX            (2)

//_RESORT_GET_INSTITUTION_DATA用
#define RESORT_INSTITUTION_SHOPTYPE            (0)

//_RESORT_xxxx_EFFECT用
#define RESORT_EFFECT_CONFETTI                (0)

//_RESORT_SET_PRESET_PEOPLE用
#define RESORT_PRESET_TUTORIAL_01             (0)
#define RESORT_PRESET_TUTORIAL_02             (1)
#define RESORT_PRESET_FUN_RANDOM              (2)
#define RESORT_PRESET_CELEV_RANDOM            (3)
#define RESORT_PRESET_SECRETARY_01            (4)
#define RESORT_PRESET_SECRETARY_02            (5)
#define RESORT_PRESET_SECRETARY_03            (6)
#define RESORT_PRESET_SECRETARY_04            (7)

//_RESORT_PRINT_RECORD_WND用
#if 0
#define RESORT_RANK_WND_CAPTURE_NET_TRADE
#define RESORT_RANK_WND_CAPTURE_NICKNAME
#define RESORT_RANK_WND_CAPTURE_TOURIST_COME
#define RESORT_RANK_WND_CAPTURE_BUY_MONEY
#define RESORT_RANK_WND_CAPTURE_SIRETOGAI
#define RESORT_RANK_WND_CAPTURE_NET_BATTLE
#define RESORT_RANK_WND_CAPTURE_
#endif
//リゾート用レコードID
#define  RESORT_RECORD_ID_NONE        ( 0)//なにもない
#define  RESORT_RECORD_ID_DENDOU      ( 1)//殿堂入りした
#define  RESORT_RECORD_ID_SUBWAY      ( 2)//サブウェイに参加した
#define  RESORT_RECORD_ID_POKEWOOD    ( 3)//ポケウッドで映画を撮った
#define  RESORT_RECORD_ID_TRIALHOUSE  ( 4)//トライアルハウスに参加した
#define  RESORT_RECORD_ID_MUSICAL     ( 5)//ミュージカルをした
#define  RESORT_RECORD_ID_HILINK      ( 6)//ハイリンクをした
#define  RESORT_RECORD_ID_HILINKFES   ( 7)//ハイリンクフェスをした
#define  RESORT_RECORD_ID_EGG         ( 8)//たまごを孵化した
#define  RESORT_RECORD_ID_POKE_LVUP   ( 9)//ポケモンのレベルを上げた
#define  RESORT_RECORD_ID_TV          (10)//テレビをみた
#define  RESORT_RECORD_ID_DUSTBOX     (11)//ゴミ箱を漁った
#define  RESORT_RECORD_ID_POKE_NAME   (12)//ポケモンにニックネームをつけた
#define  RESORT_RECORD_ID_FISHING     (13)//ポケモンを釣り上げた
#define  RESORT_RECORD_ID_HIDE_ITEM   (14)//隠しアイテムを見つけた
#define  RESORT_RECORD_ID_FOSSIL      (15)//化石を復元した
#define  RESORT_RECORD_ID_UNION       (16)//ユニオンルームへ行った
#define  RESORT_RECORD_ID_GTS         (17)//GTSをした
#define  RESORT_RECORD_ID_GTSNEGO     (18)//GTSネゴをした
#define  RESORT_RECORD_ID_VIDEO       (19)//バトルビデオをみた
#define  RESORT_RECORD_ID_RNDMATCH    (20)//ランダムマッチをした
#define  RESORT_RECORD_ID_WHEEL       (21)//観覧車に乗った
#define  RESORT_RECORD_ID_INFINITYD   (22)//無限ダンジョンをした
#define  RESORT_RECORD_ID_WBT         (23)//WBTへ参加した
#define  RESORT_RECORD_ID_JA          (24)//JAへ行った
#define  RESORT_RECORD_ID_TVT_GAME    (25)//ライブキャスターミニゲームをした
#define  RESORT_RECORD_ID_WIFICLUB    (26)//WIFIクラブへ行った
#define  RESORT_RECORD_ID_ANIMALTRAIL (27)//けもの道へ行った
#define  RESORT_RECORD_ID_GAMESYNC    (28)//ゲームシンクをした
#define  RESORT_RECORD_ID_MYSTERYGIFT (29)//ふしぎなおくりものを貰った
#define  RESORT_RECORD_ID_IR          (30)//IRをした
#define  RESORT_RECORD_ID_MAX         (31)//最大数
