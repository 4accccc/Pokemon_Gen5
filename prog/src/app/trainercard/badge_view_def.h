//============================================================================================
/**
 * @file    badge_view_def.h
 * @brief   バッジ閲覧画面
 * @author  Akito Mori
 * @date    10.03.02
 */
//============================================================================================
#ifndef __BADGE_VIEW_DEF_H__
#define __BADGE_VIEW_DEF_H__


//--------------------------------------------------------------------------------------------
// 定数定義
//--------------------------------------------------------------------------------------------

// １画面に表示する人数
#define BV_1PAGE_NUM        ( 10 )

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
#define BV_PLATE_ID   (  0 )
#define BV_PLATE0_X   (  1 )
#define BV_PLATE1_X   ( 17 )
#define BV_PLATE0_Y   (  0 )
#define BV_PLATE2_Y   (  4 )
#define BV_PLATE4_Y   (  8 )
#define BV_PLATE6_Y   ( 12 )
#define BV_PLATE8_Y   ( 16 )
#define BV_PLATE_W    ( 14 )
#define BV_PLATE_H    (  4 )




// セルアクター
enum{
  BV_OBJ_CARD=0,   ///< カードに戻るボタン
  BV_OBJ_RETURN,   ///< 戻る「U」
  BV_OBJ_END,      ///< やめる「×」
  BV_OBJ_BOOKMARK, ///< ブックマーク〆
  BV_OBJ_BADGE0,   ///< バッジ１
  BV_OBJ_BADGE1,
  BV_OBJ_BADGE2,
  BV_OBJ_BADGE3,
  BV_OBJ_BADGE4,
  BV_OBJ_BADGE5,
  BV_OBJ_BADGE6,
  BV_OBJ_BADGE7,  ///< バッジ８
  BV_OBJ_KIRAKIRA0_0, // キラキラOBJ（バッジひとつに3個ｘ8つ）
  BV_OBJ_KIRAKIRA0_1,
  BV_OBJ_KIRAKIRA0_2,
  BV_OBJ_KIRAKIRA1_0,
  BV_OBJ_KIRAKIRA1_1,
  BV_OBJ_KIRAKIRA1_2,
  BV_OBJ_KIRAKIRA2_0,
  BV_OBJ_KIRAKIRA2_1,
  BV_OBJ_KIRAKIRA2_2,
  BV_OBJ_KIRAKIRA3_0,
  BV_OBJ_KIRAKIRA3_1,
  BV_OBJ_KIRAKIRA3_2,
  BV_OBJ_KIRAKIRA4_0,
  BV_OBJ_KIRAKIRA4_1,
  BV_OBJ_KIRAKIRA4_2,
  BV_OBJ_KIRAKIRA5_0,
  BV_OBJ_KIRAKIRA5_1,
  BV_OBJ_KIRAKIRA5_2,
  BV_OBJ_KIRAKIRA6_0,
  BV_OBJ_KIRAKIRA6_1,
  BV_OBJ_KIRAKIRA6_2,
  BV_OBJ_KIRAKIRA7_0,
  BV_OBJ_KIRAKIRA7_1,
  BV_OBJ_KIRAKIRA7_2,
  BV_OBJ_MAX,
};

// セルアクターリソース
enum{
  BV_RES_PLTT=0,
  BV_RES_CHR,
  BV_RES_CELL,
  BV_RES_COMMON_PLTT,
  BV_RES_COMMON_CHR,
  BV_RES_COMMON_CELL,
  BV_RES_MAX,
};


// BMPWIN
enum{
  BV_BMPWIN_M_NAME0=0,    // リーダー名0
  BV_BMPWIN_M_NAME1,
  BV_BMPWIN_M_NAME2,
  BV_BMPWIN_M_NAME3,
  BV_BMPWIN_M_NAME4,
  BV_BMPWIN_M_NAME5,
  BV_BMPWIN_M_NAME6,
  BV_BMPWIN_M_NAME7,
  BV_BMPWIN_M_NAME8,
  BV_BMPWIN_M_NAME9,

  BV_BMPWIN_S_REKIDAI_NO_TOREINAA,  // 「れきだいのトレーナー」
  BV_BMPWIN_S_RANK,                 // ランク○○
  BV_BMPWIN_S_TRAIN_NO,             // トレインナンバー○○
  BV_BMPWIN_S_NAME,         // 上画面リーダー名
  BV_BMPWIN_S_SUNDERUBASHO, // 上画面「住んでる場所」
  BV_BMPWIN_S_COUTRY,       // 国
  BV_BMPWIN_S_JIKOSHOUKAI,  // 上画面「自己紹介」
  BV_BMPWIN_S_PMS,          // 簡易会話領域

  // 簡易会話はPMS_DRAWで描画

  BV_BMPWIN_ID_MAX,  
};

// STRBUF指定用
enum{
  BV_STR_REKIDAI=0,
  BV_STR_RANk,
  BV_STR_TRAINNO,
  BV_STR_SUNDERU,
  BV_STR_COUNTRY,
  BV_STR_LOCAL,
  BV_STR_JIKOSHOUKAI,
  BV_STR_MAX,
};

// タッチバー検出時の機能
enum{
  FUNC_LEFT_PAGE=0,
  FUNC_RIGHT_PAGE,
  FUNC_END,
};

#define BV_CLACT_NUM        ( BV_OBJ_MAX ) // OBJ登録最大数
#define BV_PMSDRAW_OBJ_NUM  ( 2 )          // 簡易会話表示につかう数


// サブ画面のBMPWINの数
#define SUB_BMPWIN_NUM  ( BV_BMPWIN_ID_MAX - BV_BMPWIN_S_REKIDAI_NO_TOREINAA)
// メイン画面のBMPWINの数
#define MAIN_BMPWIN_NUM ( BV_BMPWIN_M_NAME9+1 )
// プレートクリアパレットデータ
#define NAME_PLATE_CLEAR_COL  ( 0x0404 )


  
#endif //__BADGE_VIEW_DEF_H__