//====================================================================
// @file  shop_data_index.h
// @brief ショップデータ指定用ヘッダ
//
// ●fs_item.xlsをresource/shopdata/でコンバートして出力しています
//
//====================================================================

#ifndef __SHOP_DATA_INDEX_H__
#define __SHOP_DATA_INDEX_H__

#define T02_FS    ( 0 )  // カラクサタウン
#define C01_FS    ( 1 )  // サンヨウシティ
#define C02_FS    ( 2 )  // シッポウシティ
#define C03_FS    ( 3 )  // ヒウンシティ
#define C04_FS    ( 4 )  // ライモンシティ
#define C05_FS    ( 5 )  // ホドモエシティ
#define C06_FS    ( 6 )  // フキヨセシティ
#define C07_FS    ( 7 )  // セッカシティ
#define C08_FS    ( 8 )  // ソウリュウシティ
#define C09_FS    ( 9 )  // ポケモンリーグ
#define T03_FS    ( 10 )  // カゴメタウン
#define T04_FS    ( 11 )  // サザナミタウン
#define C10_FS    ( 12 )  // ブラックシティ／ホワイトフォレスト
#define SHOP_UP    ( 13 )  // シッポウシティ個人商店
#define SHOP_KANPO    ( 14 )  // ホドモエシティ市場１
#define SHOP_OKOU    ( 15 )  // ホドモエシティ市場２
#define SHOP_MALL1    ( 16 )  // ショッピングモール１Ｆ
#define SHOP_MALL2    ( 17 )  // ショッピングモール２Ｆ右
#define SHOP_MALL3    ( 18 )  // ショッピングモール２Ｆ左
#define SHOP_MALL4    ( 19 )  // ショッピングモール３Ｆ左
#define C12_FS    ( 20 )  // ヒウギシティ
#define C13_FS    ( 21 )  // タチワキシティ
#define C14_FS    ( 22 )  // セイガイハシティ
#define T06_FS    ( 23 )  // サンギタウン
#define T07_FS    ( 24 )  // ヤマジタウン
#define BC_A_DEFAULT    ( 25 )  // ブラックシティお店Aデフォルト
#define BC_A_RANK_01    ( 26 )  // ブラックシティお店A１段階目
#define BC_A_RANK_02    ( 27 )  // ブラックシティお店A２段階目
#define BC_A_RANK_03    ( 28 )  // ブラックシティお店A３段階目
#define BC_A_RANK_04    ( 29 )  // ブラックシティお店A４段階目
#define BC_A_RANK_05    ( 30 )  // ブラックシティお店A５段階目
#define BC_B_DEFAULT    ( 31 )  // ブラックシティお店Bデフォルト
#define BC_B_RANK_01    ( 32 )  // ブラックシティお店B１段階目
#define BC_B_RANK_02    ( 33 )  // ブラックシティお店B２段階目
#define BC_B_RANK_03    ( 34 )  // ブラックシティお店B３段階目
#define BC_B_RANK_04    ( 35 )  // ブラックシティお店B４段階目
#define BC_B_RANK_05    ( 36 )  // ブラックシティお店B５段階目
#define BC_C_DEFAULT    ( 37 )  // ブラックシティお店Cデフォルト
#define BC_C_RANK_01    ( 38 )  // ブラックシティお店C１段階目
#define BC_C_RANK_02    ( 39 )  // ブラックシティお店C２段階目
#define BC_C_RANK_03    ( 40 )  // ブラックシティお店C３段階目
#define BC_C_RANK_04    ( 41 )  // ブラックシティお店C４段階目
#define BC_C_RANK_05    ( 42 )  // ブラックシティお店C５段階目
#define BC_D_DEFAULT    ( 43 )  // ブラックシティお店Dデフォルト
#define BC_D_RANK_01    ( 44 )  // ブラックシティお店D１段階目
#define BC_D_RANK_02    ( 45 )  // ブラックシティお店D２段階目
#define BC_D_RANK_03    ( 46 )  // ブラックシティお店D３段階目
#define BC_D_RANK_04    ( 47 )  // ブラックシティお店D４段階目
#define BC_D_RANK_05    ( 48 )  // ブラックシティお店D５段階目
#define WF_A_DEFAULT    ( 49 )  // ホワイトフォレストお店Aデフォルト
#define WF_A_RANK_01    ( 50 )  // ホワイトフォレストお店A１段階目
#define WF_A_RANK_02    ( 51 )  // ホワイトフォレストお店A２段階目
#define WF_A_RANK_03    ( 52 )  // ホワイトフォレストお店A３段階目
#define WF_A_RANK_04    ( 53 )  // ホワイトフォレストお店A４段階目
#define WF_A_RANK_05    ( 54 )  // ホワイトフォレストお店A５段階目
#define WF_B_DEFAULT    ( 55 )  // ホワイトフォレストお店Bデフォルト
#define WF_B_RANK_01    ( 56 )  // ホワイトフォレストお店B１段階目
#define WF_B_RANK_02    ( 57 )  // ホワイトフォレストお店B２段階目
#define WF_B_RANK_03    ( 58 )  // ホワイトフォレストお店B３段階目
#define WF_B_RANK_04    ( 59 )  // ホワイトフォレストお店B４段階目
#define WF_B_RANK_05    ( 60 )  // ホワイトフォレストお店B５段階目
#define WF_C_DEFAULT    ( 61 )  // ホワイトフォレストお店Cデフォルト
#define WF_C_RANK_01    ( 62 )  // ホワイトフォレストお店C１段階目
#define WF_C_RANK_02    ( 63 )  // ホワイトフォレストお店C２段階目
#define WF_C_RANK_03    ( 64 )  // ホワイトフォレストお店C３段階目
#define WF_C_RANK_04    ( 65 )  // ホワイトフォレストお店C４段階目
#define WF_C_RANK_05    ( 66 )  // ホワイトフォレストお店C５段階目
#define WF_D_DEFAULT    ( 67 )  // ホワイトフォレストお店Dデフォルト
#define WF_D_RANK_01    ( 68 )  // ホワイトフォレストお店D１段階目
#define WF_D_RANK_02    ( 69 )  // ホワイトフォレストお店D２段階目
#define WF_D_RANK_03    ( 70 )  // ホワイトフォレストお店D３段階目
#define WF_D_RANK_04    ( 71 )  // ホワイトフォレストお店D４段階目
#define WF_D_RANK_05    ( 72 )  // ホワイトフォレストお店D５段階目


#endif
