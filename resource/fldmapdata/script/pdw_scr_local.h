//======================================================================
//                pdw_scr_local.h
//
//    スクリプトファイル：pdw_scr用ローカル定数定義
//
//======================================================================


//_PDW_COMMON_TOOLS用
//PDWアカウントがあるか？
#define PDW_TOOLS_CHECK_ACCOUNT (0)

//_PDW_FURNITURE_TOOLS用
//家具リストがあるか？
#define PDW_FURNITURE_TOOLS_CHECK_LIST (0)
//もう家具リストを選んだか？
#define PDW_FURNITURE_TOOLS_CHECK_SELECT (1)
//前回家具を送ったか？
#define PDW_FURNITURE_TOOLS_IS_SYNC_FURNITURE (2)
//上のフラグを落とす
#define PDW_FURNITURE_TOOLS_RESET_SYNC_FURNITURE (3)
//家具選択番号セット(val1 番号)
#define PDW_FURNITURE_TOOLS_SET_SELECT_FURNITURE (4)
