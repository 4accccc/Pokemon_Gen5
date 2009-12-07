/**
 *  @file   field_bg_def.h
 *  @brief  フィールドBG設定　コモン
 */

#pragma once

///メインフレーム名定義
typedef enum{
  FLDBG_MFRM_3D = GFL_BG_FRAME0_M,      //マップ(フィールド常駐)
  FLDBG_MFRM_MSG = GFL_BG_FRAME1_M,     //メッセージ(フィールド常駐)
  FLDBG_MFRM_EFF1 = GFL_BG_FRAME2_M,    //各種メニュー(フィールド常駐)
  FLDBG_MFRM_EFF2 = GFL_BG_FRAME3_M,    //各種エフェクト、効果(非常駐)
}FLDBG_MAIN_FRAME;

///メインフレームデフォルトプライオリティ
typedef enum{
  FLDBG_MFRM_3D_PRI = 3,
  FLDBG_MFRM_MSG_PRI = 0,
  FLDBG_MFRM_EFF1_PRI = 2,
  FLDBG_MFRM_EFF2_PRI = 1,
}FLDBG_MAIN_PRIORITY;

/////////////////////////////////////////////////////////////
///フィールド常駐フレーム　設定
/////////////////////////////////////////////////////////////

///メッセージフレーム
#define FLDBG_MFRM_MSG_COLORMODE  (GX_BG_COLORMODE_16)
#define FLDBG_MFRM_MSG_SCRBASE    (GX_BG_SCRBASE_0x0000)
#define FLDBG_MFRM_MSG_SCRSIZE    (0x800) //最大0x1000(ショートカットメニュー表示時 <shortcut_menu.c)
#define FLDBG_MFRM_MSG_CHARBASE   (GX_BG_CHARBASE_0x10000)
#define FLDBG_MFRM_MSG_CHARSIZE   (0x8000)

//各種メニューフレーム
#define FLDBG_MFRM_EFF1_COLORMODE  (GX_BG_COLORMODE_16)
#define FLDBG_MFRM_EFF1_SCRBASE    (GX_BG_SCRBASE_0xf800)
#define FLDBG_MFRM_EFF1_SCRSIZE    (0x800)
#define FLDBG_MFRM_EFF1_CHARBASE   (GX_BG_CHARBASE_0x18000)
#define FLDBG_MFRM_EFF1_CHARSIZE   (0x8000)

