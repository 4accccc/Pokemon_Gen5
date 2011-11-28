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
#define RESORT_GET_PARAM_IS_SCOUT           (1)
#define RESORT_GET_PARAM_MERGE_FLAG         (2)
#define RESORT_GET_PARAM_PRESENT_ID         (3)
#define RESORT_GET_PARAM_IS_INSTITUTION_EMPTY   (6)
#define RESORT_GET_PARAM_IS_EVENT           (7)
#define RESORT_GET_PARAM_TYPE               (8)
#define RESORT_GET_PARAM_LOT_DAY_FLAG       (9)
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
#define RESORT_GET_PARAM_CHECK_RESIDENT_00  (25)
#define RESORT_GET_PARAM_CHECK_RESIDENT_01  (26)
#define RESORT_GET_PARAM_CHECK_RESIDENT_02  (27)
#define RESORT_GET_PARAM_CHECK_RESIDENT_03  (28)
#define RESORT_GET_PARAM_CHECK_RESIDENT_04  (29)
#define RESORT_GET_PARAM_CHECK_RESIDENT_05  (30)
#define RESORT_GET_PARAM_CHECK_RESIDENT_06  (31)
#define RESORT_GET_PARAM_CHECK_RESIDENT_07  (32)
#define RESORT_GET_PARAM_IS_NEON            (33)
#define RESORT_GET_PARAM_IS_CITYNAME        (34)
#define RESORT_GET_PARAM_CITYNAME_SEC_SEX   (35)
#define RESORT_GET_PARAM_NEON_SEC_SEX       (36)
#define RESORT_GET_PARAM_CITYNAME_SEC_INDEX   (37)
#define RESORT_GET_PARAM_NEON_SEC_INDEX       (38)
#define RESORT_GET_PARAM_CELEB_TRADE_NO       (39)
#define RESORT_GET_PARAM_INDEX                (40)
#define RESORT_GET_PARAM_PEOPLE_TYPE          (41)
#define RESORT_GET_PARAM_OBJID_RESIDENT_00    (44)
#define RESORT_GET_PARAM_OBJID_RESIDENT_01    (45)
#define RESORT_GET_PARAM_OBJID_RESIDENT_02    (46)
#define RESORT_GET_PARAM_OBJID_RESIDENT_03    (47)
#define RESORT_GET_PARAM_OBJID_RESIDENT_04    (48)
#define RESORT_GET_PARAM_OBJID_RESIDENT_05    (49)
#define RESORT_GET_PARAM_OBJID_RESIDENT_06    (50)
#define RESORT_GET_PARAM_OBJID_RESIDENT_07    (51)
#define RESORT_GET_PARAM_GRADE_RESIDENT_00    (52)
#define RESORT_GET_PARAM_GRADE_RESIDENT_01    (53)
#define RESORT_GET_PARAM_GRADE_RESIDENT_02    (54)
#define RESORT_GET_PARAM_GRADE_RESIDENT_03    (55)
#define RESORT_GET_PARAM_GRADE_RESIDENT_04    (56)
#define RESORT_GET_PARAM_GRADE_RESIDENT_05    (57)
#define RESORT_GET_PARAM_GRADE_RESIDENT_06    (58)
#define RESORT_GET_PARAM_GRADE_RESIDENT_07    (59)
#define RESORT_GET_PARAM_IS_CELEB             (60)
#define RESORT_GET_PARAM_CITY_ITEM            (61)
#define RESORT_GET_PARAM_IS_PARADE            (62)
#define RESORT_GET_PARAM_CITY_RANK_NUM        (63)

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
#define RESORT_MY_PARAM_IS_FEVER           (19)
#define RESORT_MY_PARAM_EV_PRIZE           (20)
#define RESORT_MY_PARAM_EV_PRIZE_END       (21)
#define RESORT_MY_PARAM_CELEB_TRADE_FLAG   (22)
#define RESORT_MY_PARAM_PARADE_MODE        (23)
#define RESORT_MY_PARAM_NOISE_REQ          (24)
#define RESORT_MY_PARAM_IS_PRIZE_DENDOU    (25)
#define RESORT_MY_PARAM_MAPCHANGE_MMDL     (26)

//_RESORT_JOB_CHANGE用
#define RESORT_JOBCHANGE_RESIDENT           (0)
#define RESORT_JOBCHANGE_SECETARY           (1)
///_RESORT_ACTION用
#define RESORT_ACTION_LEAVE           (0)
//_RESORT_SET_WORD用
#define RESORT_WORD_PEOPLE            (0) //人の名前
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
#define RESORT_WORD_FEVER_DAY         (17)
#define RESORT_WORD_FREE_THANKS       (18)  
#define RESORT_WORD_OWNER_PHRASE      (19)
#define RESORT_WORD_OWNER_THANKS      (20)
#define RESORT_WORD_MEET_YEAR         (21)
#define RESORT_WORD_MEET_MONTH        (22)
#define RESORT_WORD_MEET_DAY          (23)

#define RESORT_WORD_PARAM_FRONT       (0xFE)  //目の前の人
#define RESORT_WORD_PARAM_NONE        (0xFF)

//_RESORT_CALL_LIST用
#define RESORT_LIST_TYPE_BUY            (0)
#define RESORT_LIST_TYPE_SELECT         (1)
#define RESORT_LIST_TYPE_SORT           (2)

#define RESORT_LIST_CANCEL              (0xFF)

#define RESORT_LIST_CAMERA_GRID         (0)
#define RESORT_LIST_CAMERA_SHOP         (1)
#define RESORT_LIST_CAMERA_RECEPTION    (2)

  //RESORT_LIST_TYPE_SELECT
#define RESORT_LIST_SELECT_SECRETARY_ADD      (0)
#define RESORT_LIST_SELECT_SECRETARY_DEL      (1)
#define RESORT_LIST_SELECT_INSTATITUTION      (2)
#define RESORT_LIST_SELECT_INSTATITUTION_DEL  (3)

  //RESORT_LIST_TYPE_SORT
#define RESORT_LIST_SORT_SECRETARY      (0)
#define RESORT_LIST_SORT_INSTATITUTION  (1)

//_RESORT_GET_MSGID用
#define RESORT_MSGID_NONE      (0)  //返り値チェク０ならなし
  //特殊MSG
#define RESORT_MSGID_RECENT_00      (0)  //できごと文
#define RESORT_MSGID_RECENT_01      (1)  //できごと文
#define RESORT_MSGID_RECENT_02      (2)  //できごと文
#define RESORT_MSGID_RECENT_03      (3)  //できごと文
#define RESORT_MSGID_WHERETOGO      (4)  //どこへ行きたいか
#define RESORT_MSGID_APPEAL         (5)  //アピール文
#define RESORT_MSGID_SHOP_INFO      (6)  //お店情報
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
#define RESORT_TOURIST_SHOP_GREET_MSGID   (60) //案内でお店に来たときに挨拶文
#define RESORT_TOURIST_SPOT_GRADE_NG      (61)  //評価NG文
#define RESORT_TOURIST_COMBO_EFF_MSGID      (62)  //コンボ用案内演出文
  //住人
#define RESORT_RESIDENT_GREET_MSGID           (80)  //挨拶
#define RESORT_RESIDENT_BYE_MSGID             (81)  //またね
#define RESORT_RESIDENT_CHANGE_MSGID          (82)  //転職したい
#define RESORT_RESIDENT_CHANGE_CONFIM_MSGID   (83)  //転職確認
#define RESORT_RESIDENT_CHANGE_OK_MSGID       (84)  //転職OKがんばります
#define RESORT_RESIDENT_CHANGE_NG_MSGID       (85)  //転職NGこのままやります
#define RESORT_RESIDENT_LV_MSGID              (86)  //お店LV
#define RESORT_RESIDENT_FEVER_MSGID           (87)  //フィーバー
#define RESORT_RESIDENT_RANK_MSGID            (88)  //ランキング１番！
#define RESORT_RESIDENT_COMBO_RECOMMEND_MSGID (89)  //コンボおすすめ文
#define RESORT_RESIDENT_COMBO_TEACH_MSGID     (90)  //コンボ紹介した文
#define RESORT_RESIDENT_COMBO_OK_MSGID        (91)  //コンボ行ってみたい文
#define RESORT_RESIDENT_COMBO_NG_MSGID        (92)  //コンボまたこんど文
#define RESORT_RESIDENT_COMBO_EFF_MSGID_RARE  (93)  //コンボ期待文
#define RESORT_RESIDENT_COMBO_EFF_MSGID_HIGH  (94)  //コンボ期待文
#define RESORT_RESIDENT_COMBO_EFF_MSGID_LOW   (95)  //コンボ期待文
#define RESORT_RESIDENT_COMBO_RND_MSGID       (96)  //コンボ確率文

  //秘書
#define RESORT_SECRETARY_GREET_MSGID          (100) //挨拶
#define RESORT_SECRETARY_BYE_MSGID            (101) //またね
#define RESORT_SECRETARY_CITYLV_MSGID         (102) //まちレベル
#define RESORT_SECRETARY_SECSORT_MSGID        (103) //秘書交換
#define RESORT_SECRETARY_SHOPSORT_MSGID       (104) //お店交換
#define RESORT_SECRETARY_NEON_OK_MSGID        (105) //ネオン変更
#define RESORT_SECRETARY_NEON_SEL_MSGID       (106) //ネオン選択
#define RESORT_SECRETARY_RANK_SEL_MSGID       (107) //ランキング画面選択
#define RESORT_SECRETARY_FEVER_MSGID          (108) //フィーバー情報
#define RESORT_SECRETARY_SEL_SELECT           (112) //なにがしたいですか？
#define RESORT_SECRETARY_SEL_ONE              (113) //１つだけ
#define RESORT_SECRETARY_SEL_MANY             (114) //複数
#define RESORT_SECRETARY_SEL_PARADE           (115) //パレードを行いますか？

  //ファン
#define RESORT_FUN_GREET_MSGID                (120) //挨拶

  //チュートリアル１
#define RESORT_TUTORIAL01_GREET_MSGID         (125) //挨拶

  //チュートリアル２
#define RESORT_TUTORIAL02_GREET_MSGID         (130) //挨拶

  //お店
#define RESORT_SHOP_LOT_SUCCESS_MSGID         (140) //くじ当たり
#define RESORT_SHOP_LOT_FAILER_MSGID          (141) //くじハズレ
#define RESORT_SHOP_LOT_DAYEND_MSGID          (142) //くじ終了
#define RESORT_SHOP_LOT_READY_MSGID           (143) //でははっぴょうします
#define RESORT_SHOP_LOT_BIG_MSGID             (144) //くじ大当たり
#define RESORT_SHOP_LOT_FEVER_MSGID           (145) //フィーバー中

#define RESORT_SHOP_PARLOR_OK_MSGID           (150)
#define RESORT_SHOP_PARLOR_POKESEL_MSGID      (151)
#define RESORT_SHOP_BREEDER_OK_MSGID          (152)
#define RESORT_SHOP_BREEDER_POKESEL_MSGID     (153)
#define RESORT_SHOP_EXERCIDE_OK_MSGID         (154)
#define RESORT_SHOP_EXERCIDE_POKESEL_MSGID    (155)
#define RESORT_SHOP_NURSERY_OK_MSGID          (156)
#define RESORT_SHOP_NURSERY_POKESEL_MSGID     (157)
#define RESORT_SHOP_NURSERY_HATCH_00_MSGID       (158)
#define RESORT_SHOP_NURSERY_HATCH_01_MSGID       (159)
#define RESORT_SHOP_NURSERY_HATCH_02_MSGID       (160)
#define RESORT_SHOP_NURSERY_HATCH_03_MSGID       (161)
#define RESORT_SHOP_NURSERY_HATCH_04_MSGID       (162)

#define RESORT_SHOP_MONEY_MSGID               (163) //お代をいただきますね
#define RESORT_SHOP_EGG_NG_MSGID              (164) //タマゴはだめ
#define RESORT_SHOP_POKE_NG_MSGID             (165) //ポケモンはだめ
#define RESORT_SHOP_DORYOKU_U_NG_MSGID        (166) //努力値あがらない
#define RESORT_SHOP_DORYOKU_D_NG_MSGID        (167) //努力値さがらない
#define RESORT_SHOP_NATUKI_NG_MSGID           (168) //なつきあがらない
#define RESORT_SHOP_LV_NG_MSGID               (169) //LVあがらない

  //セレブ
#define RESORT_CELEB_GREET_MSGID              (170)
#define RESORT_CELEB_TRADE_GREET_MSGID        (171)
#define RESORT_CELEB_TRADE_BYE_MSGID          (172)
#define RESORT_CELEB_TRADE_SELECT_MSGID       (173)
#define RESORT_CELEB_TRADE_NG_MSGID           (174)
#define RESORT_CELEB_TRADE_OK_MSGID           (175)

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
#define RESORT_DEC_EFFORT_HP                  (6)
#define RESORT_DEC_EFFORT_POW                 (7)
#define RESORT_DEC_EFFORT_DEF                 (8)
#define RESORT_DEC_EFFORT_AGI                 (9)
#define RESORT_DEC_EFFORT_SPEPOW              (10)
#define RESORT_DEC_EFFORT_SPEDEF              (11)

//_RESORT_DELETE_PEOPLE用
#define RESORT_DELTE_PEOPLE_MINE              (0xFF)
#define RESORT_DELTE_PEOPLE_REMOVE_SEC        (0xFE)

#define RESORT_DELTE_PEOPLE_MODE_ERASE        (0)
#define RESORT_DELTE_PEOPLE_MODE_REMOVE       (1)

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
#define RESORT_NPC_DATA_MOUTHCOMM_A_06        (10)
#define RESORT_NPC_DATA_MOUTHCOMM_A_07        (11)
#define RESORT_NPC_DATA_MOUTHCOMM_A_08        (12)
#define RESORT_NPC_DATA_MOUTHCOMM_A_09        (13)
#define RESORT_NPC_DATA_MOUTHCOMM_A_10        (14)
#define RESORT_NPC_DATA_MOUTHCOMM_A_11        (15)
#define RESORT_NPC_DATA_MOUTHCOMM_I_00        (16)
#define RESORT_NPC_DATA_MOUTHCOMM_I_01        (17)
#define RESORT_NPC_DATA_MOUTHCOMM_I_02        (18)
#define RESORT_NPC_DATA_MOUTHCOMM_I_03        (19)
#define RESORT_NPC_DATA_MOUTHCOMM_I_04        (20)
#define RESORT_NPC_DATA_MOUTHCOMM_I_05        (21)
#define RESORT_NPC_DATA_MOUTHCOMM_I_06        (22)
#define RESORT_NPC_DATA_MOUTHCOMM_I_07        (23)
#define RESORT_NPC_DATA_MOUTHCOMM_I_08        (24)
#define RESORT_NPC_DATA_MOUTHCOMM_I_09        (25)
#define RESORT_NPC_DATA_MOUTHCOMM_I_10        (26)
#define RESORT_NPC_DATA_MOUTHCOMM_I_11        (27)
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_00      (28)  //初期挨拶文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_01      (29)  //通常挨拶文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_02      (30)  //回答確認文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_03      (31)  //繰り返し文
#define RESORT_NPC_DATA_MOUTHCOMM_MSG_04      (32)  //噂がないとき文

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
#define RESORT_SET_SPOT_CITY_LVUP_PRIZE       (3)
#define RESORT_SET_SPOT_CITY_LVUP_RANK        (4)
#define RESORT_SET_SPOT_CITY_LVUP_PRIZE_RANK  (5)

//_RESORT_GET_INSTITUTION_DATA用
#define RESORT_INSTITUTION_SHOPTYPE            (0)

//_RESORT_xxxx_EFFECT用
#define RESORT_EFFECT_CONFETTI                (0)
#define RESORT_EFFECT_PARADE                  (1)

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
#define RESORT_RANK_WND_NET_TRADE             (0)
#define RESORT_RANK_WND_NICKNAME              (1)
#define RESORT_RANK_WND_JA_TOURIST            (2)
#define RESORT_RANK_WND_BUY_MONEY             (3)
#define RESORT_RANK_WND_CROSSING              (4)
#define RESORT_RANK_WND_NET_BATTLE            (5)
#define RESORT_RANK_WND_GET_POKE              (6)
#define RESORT_RANK_WND_HATCH_EGG             (7)
#define RESORT_RANK_WND_MEDALNUM              (8)
#define RESORT_RANK_WND_MAX                   (9)

//_RESORT_GET_OFFRECO用
#define RESORT_OFFRECO_GAMESTART_MSGID        (0)
#define RESORT_OFFRECO_DENDOU_MSGID           (1)
#define RESORT_OFFRECO_PLAYTIME_MSGID         (2)
#define RESORT_OFFRECO_ZUKAN_MSGID            (3)
#define RESORT_OFFRECO_POKE_MSGID             (5)
#define RESORT_OFFRECO_COMEFROM_MSGID         (6)
#define RESORT_OFFRECO_JOB_MSGID              (7)
#define RESORT_OFFRECO_HOBBY_MSGID            (8)
#define RESORT_OFFRECO_CITYLV_MSGID           (9)
#define RESORT_OFFRECO_CITYMSG_EXSIT          (10)
#define RESORT_OFFRECO_CITYSHOP_CNT           (11)
#define RESORT_OFFRECO_ENDSHOP_MSGID          (12)
#define RESORT_OFFRECO_SHOP_MSGID_00          (13)
#define RESORT_OFFRECO_SHOP_MSGID_01          (14)
#define RESORT_OFFRECO_SHOP_MSGID_02          (15)
#define RESORT_OFFRECO_SHOP_MSGID_03          (16)
#define RESORT_OFFRECO_SHOP_MSGID_04          (17)
#define RESORT_OFFRECO_SHOP_MSGID_05          (18)
#define RESORT_OFFRECO_SHOP_MSGID_06          (19)
#define RESORT_OFFRECO_SHOP_MSGID_07          (20)

//_RESORT_ADD_PEOPLE
#define RESORT_ADD_PEOPLE_SECRETARY_01        (0)
#define RESORT_ADD_PEOPLE_SECRETARY_02        (1)
#define RESORT_ADD_PEOPLE_SECRETARY_03        (2)
#define RESORT_ADD_PEOPLE_SECRETARY_04        (3)

//_RESORT_RESET_MMDL
#define RESORT_RESET_MMDL_RESIDENT_00         (0)
#define RESORT_RESET_MMDL_RESIDENT_01         (1)
#define RESORT_RESET_MMDL_RESIDENT_02         (2)
#define RESORT_RESET_MMDL_RESIDENT_03         (3)
#define RESORT_RESET_MMDL_RESIDENT_04         (4)
#define RESORT_RESET_MMDL_RESIDENT_05         (5)
#define RESORT_RESET_MMDL_RESIDENT_06         (6)
#define RESORT_RESET_MMDL_RESIDENT_07         (7)
#define RESORT_RESET_MMDL_SECRETARY_ALL       (8)

//_RESORT_SET_SCENE_VISIBLE
#define RESORT_SCENE_VISIBLE_OFF_CROWDPEOPLE      (0) //話しかけてる以外の歩行者
#define RESORT_SCENE_VISIBLE_OFF_ALL_CROWDPEOPLE  (1) //全歩行者
#define RESORT_SCENE_VISIBLE_DEFAULT              (2) //通常に戻す
#define RESORT_SCENE_VISIBLE_OFF_ALL_PEOPLE       (3) //全ての人

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
#define  RESORT_RECORD_ID_INFINITYD_B (22)//くろのまてんろうへいった
#define  RESORT_RECORD_ID_WBT         (23)//WBTへ参加した
#define  RESORT_RECORD_ID_JA          (24)//JAへ行った
#define  RESORT_RECORD_ID_TVT_GAME    (25)//ライブキャスターミニゲームをした
#define  RESORT_RECORD_ID_WIFICLUB    (26)//WIFIクラブへ行った
#define  RESORT_RECORD_ID_ANIMALTRAIL (27)//けもの道へ行った
#define  RESORT_RECORD_ID_GAMESYNC    (28)//ゲームシンクをした
#define  RESORT_RECORD_ID_MYSTERYGIFT (29)//ふしぎなおくりものを貰った
#define  RESORT_RECORD_ID_IR          (30)//IRをした
#define  RESORT_RECORD_ID_GETPOKE     (31)//ポケモンを捕まえた
#define  RESORT_RECORD_ID_INFINITYD_W (32)//しろのじゅどうへいった
#define  RESORT_RECORD_ID_MAX         (33)//最大数


//_RESORT_GET_COMBO
#define  RESORT_COMBO_PARAM_RECOMMEND  (0) //コンボおすすめ取得
#define  RESORT_COMBO_PARAM_EXP        (1) //コンボ経験値
#define  RESORT_COMBO_PARAM_SHOPRESULT (2) //コンボお店にきてどうだったか
#define  RESORT_COMBO_PARAM_GRADE_MSGID (3) //コンボお店評価文
#define  RESORT_COMBO_PARAM_INIT        (4) //コンボ初期化
#define  RESORT_COMBO_PARAM_DECIDE      (5) //コンボ確定

//RESORT_COMBO_PARAM_RECOMMENDの結果
#define  RESORT_COMBO_RESULT_SUCSESS  (0) //いってきませんか？いってみます！
#define  RESORT_COMBO_RESULT_FAILER   (1) //いってきませんかすらでない
#define  RESORT_COMBO_RESULT_PITY     (2) //いってきませんか？まだ今度！
