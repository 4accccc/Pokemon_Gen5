//============================================================================================
/**
 * @file    leader_board_def.h
 * @brief   リーダーボード画面（Wifiバトルサブウェイダウンロードデータ）定義画面
 * @author  Akito Mori
 * @date    10.02.22
 */
//============================================================================================
#ifndef __LEADER_BOARD_DEF_H__
#define __LEADER_BOARD_DEF_H__


//--------------------------------------------------------------------------------------------
// 定数定義
//--------------------------------------------------------------------------------------------

// １画面に表示する人数
#define LB_1PAGE_NUM        ( 10 )

// タッチバーの各情報定義
#define TOUCHBAR_X    (  0 )
#define TOUCHBAR_Y    ( 21 )
#define TOUCHBAR_W    ( 32 )
#define TOUCHBAR_H    (  3 )
#define TOUCHBAR_PAL  ( 14 )

// メイン画面のパレット定義
#define TALKFONT_PAL          ( 15 )
#define TALKFONT_PAL_OFFSET   ( TALKFONT_PAL*0x20 )

// プレートの各情報
#define LB_PLATE_ID   (  0 )
#define LB_PLATE0_X   (  1 )
#define LB_PLATE1_X   ( 17 )
#define LB_PLATE0_Y   (  0 )
#define LB_PLATE2_Y   (  4 )
#define LB_PLATE4_Y   (  8 )
#define LB_PLATE6_Y   ( 12 )
#define LB_PLATE8_Y   ( 16 )
#define LB_PLATE_W    ( 14 )
#define LB_PLATE_H    (  4 )




// セルアクター
enum{
  LB_OBJ_CURSOR=0,
  LB_OBJ_END,
  LB_OBJ_LEFT,
  LB_OBJ_RIGHT,
  LB_OBJ_MAX,
};

// セルアクターリソース
enum{
  LB_RES_PLTT=0,
  LB_RES_CHR,
  LB_RES_CELL,
  LB_RES_COMMON_PLTT,
  LB_RES_COMMON_CHR,
  LB_RES_COMMON_CELL,
  LB_RES_MAX,
};


// BMPWIN
enum{
  LB_BMPWIN_M_NAME0=0,    // リーダー名0
  LB_BMPWIN_M_NAME1,
  LB_BMPWIN_M_NAME2,
  LB_BMPWIN_M_NAME3,
  LB_BMPWIN_M_NAME4,
  LB_BMPWIN_M_NAME5,
  LB_BMPWIN_M_NAME6,
  LB_BMPWIN_M_NAME7,
  LB_BMPWIN_M_NAME8,
  LB_BMPWIN_M_NAME9,

  LB_BMPWIN_S_REKIDAI_NO_TOREINAA,  // 「れきだいのトレーナー」
  LB_BMPWIN_S_RANK,                 // ランク○○
  LB_BMPWIN_S_TRAIN_NO,             // トレインナンバー○○
  LB_BMPWIN_S_NAME,         // 上画面リーダー名
  LB_BMPWIN_S_SUNDERUBASHO, // 上画面「住んでる場所」
  LB_BMPWIN_S_COUTRY,       // 国
  LB_BMPWIN_S_JIKOSHOUKAI,  // 上画面「自己紹介」
  LB_BMPWIN_S_PMS,          // 簡易会話領域

  // 簡易会話はPMS_DRAWで描画

  LB_BMPWIN_ID_MAX,  
};

// STRBUF指定用
enum{
  LB_STR_REKIDAI=0,
  LB_STR_RANk,
  LB_STR_TRAINNO,
  LB_STR_SUNDERU,
  LB_STR_COUNTRY,
  LB_STR_LOCAL,
  LB_STR_JIKOSHOUKAI,
  LB_STR_MAX,
};

// タッチバー検出時の機能
enum{
  FUNC_LEFT_PAGE=0,
  FUNC_RIGHT_PAGE,
  FUNC_END,
};

#define LB_CLACT_NUM        ( LB_OBJ_MAX ) // OBJ登録最大数
#define LB_PMSDRAW_OBJ_NUM  ( 2 )          // 簡易会話表示につかう数


// サブ画面のBMPWINの数
#define SUB_BMPWIN_NUM  ( LB_BMPWIN_ID_MAX - LB_BMPWIN_S_REKIDAI_NO_TOREINAA)
// メイン画面のBMPWINの数
#define MAIN_BMPWIN_NUM ( LB_BMPWIN_M_NAME9+1 )
// プレートクリアパレットデータ
#define NAME_PLATE_CLEAR_COL  ( 0x0404 )


  
#endif