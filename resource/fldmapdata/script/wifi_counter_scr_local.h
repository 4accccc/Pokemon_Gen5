//======================================================================
//                wifi_counter_scr_local.h
//
//    スクリプトファイル：wifi_counter_scr用ローカル定数定義
//
//======================================================================

///メインシーケンス
#define SEQ_MAIN_MENU         (0)
#define SEQ_COUNTER_EXP       (1)
#define SEQ_RANDOM_MATCH      (2)
#define SEQ_GLOBAL_TERMINAL   (3)
#define SEQ_GOODBYE           (10)
#define SEQ_RETRY             (11)
#define SEQ_EXIT              (12)
#define SEQ_THROW_EXIT        (13)
#define SEQ_END               (0xFF)
#define SEQ_CONNECT           (0)

///ランダムマッチシーケンス
#define SEQ_SUB_RNDMATCH_MENU        (0)
#define SEQ_SUB_RNDMATCH_TRY_SELECT  (1)
#define SEQ_SUB_RNDMATCH_TRY_CHECK   (2)
#define SEQ_SUB_RNDMATCH_TRY         (3)
#define SEQ_SUB_RNDMATCH_EXP         (4)
#define SEQ_SUB_RNDMATCH_END         (0xFD)
#define SEQ_SUB_RNDMATCH_RETURN      (0xFE)
#define SEQ_SUB_RNDMATCH_EXIT        (0xFF)

///グローバルターミナルシーケンス
#define SEQ_SUB_GTERMINAL_MENU        (0)
#define SEQ_SUB_GTERMINAL_GTS         (1)
#define SEQ_SUB_GTERMINAL_GTS_NEGO    (2)
#define SEQ_SUB_GTERMINAL_BTL_VIDEO   (3)
#define SEQ_SUB_GTERMINAL_MUSICAL     (4)
#define SEQ_SUB_GTERMINAL_EXP         (5)
#define SEQ_SUB_GTERMINAL_RETURN      (0xFE)
#define SEQ_SUB_GTERMINAL_EXIT        (0xFF)

///グローバルターミナル:GTSシーケンス
#define SEQ_SUB_GTERMINAL_GTS_MENU        (0)
#define SEQ_SUB_GTERMINAL_GTS_EXP         (1)
#define SEQ_SUB_GTERMINAL_GTS_EXCHANGE    (2)
#define SEQ_SUB_GTERMINAL_GTS_RETURN      (0xFE)
#define SEQ_SUB_GTERMINAL_GTS_EXIT        (0xFF)

///グローバルターミナル:GTSシーケンス
#define SEQ_SUB_GTERMINAL_GTS_NEGO_MENU        (0)
#define SEQ_SUB_GTERMINAL_GTS_NEGO_EXP         (1)
#define SEQ_SUB_GTERMINAL_GTS_NEGO_EXCHANGE    (2)
#define SEQ_SUB_GTERMINAL_GTS_NEGO_RETURN      (0xFE)
#define SEQ_SUB_GTERMINAL_GTS_NEGO_EXIT        (0xFF)

