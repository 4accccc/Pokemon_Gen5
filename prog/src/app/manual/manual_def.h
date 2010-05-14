//============================================================================
/**
 *  @file   manual_def.h
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *
 *  モジュール名：MANUAL_DEF
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "ui/touchbar.h"
#include "sound/pm_sndsys.h"


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインBGフレーム
#define BG_FRAME_M_PIC       (GFL_BG_FRAME2_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_PIC   (0)
// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_PIC       = 0,
};
// 位置
enum
{
  BG_PAL_POS_M_PIC       = 0,
};
// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_         = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_M_         = 0,
};

// サブBGフレーム
#define BG_FRAME_S_REAR       (GFL_BG_FRAME0_S)
#define BG_FRAME_S_MAIN       (GFL_BG_FRAME1_S)
#define BG_FRAME_S_TB_BAR     (GFL_BG_FRAME2_S)
#define BG_FRAME_S_TB_TEXT    (GFL_BG_FRAME3_S)
// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_REAR     (3)
#define BG_FRAME_PRI_S_MAIN     (2)
#define BG_FRAME_PRI_S_TB_BAR   (1)
#define BG_FRAME_PRI_S_TB_TEXT  (0)
// サブBGパレット
// 本数
enum
{
  BG_PAL_NUM_S_REAR        = 2,
  BG_PAL_NUM_S_TEXT        = 1,
  BG_PAL_NUM_S_TB_BAR      = 1,  // TOUCHBAR_BG_PLT_NUM
  BG_PAL_NUM_S_ATM         = 2,  // APP_TASKMENU
};
// 位置
enum
{
  BG_PAL_POS_S_REAR        = 0,
  BG_PAL_POS_S_TEXT        = 2,
  BG_PAL_POS_S_TB_BAR      = 3,
  BG_PAL_POS_S_ATM         = 4,  // APP_TASKMENU
};
// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_LIST      =  1,
  OBJ_PAL_NUM_S_TB_ICON   =  3,  // TOUCHBAR_OBJ_PLT_NUM
  OBJ_PAL_NUM_S_BTN       =  3,  // TOUCHBAR_OBJ_PLT_NUM  // タッチバーのアイコンと同じパレット(同じのを読み込むのはイヤだが仕方がない)
};
// 位置
enum
{
  OBJ_PAL_POS_S_LIST      = 0,
  OBJ_PAL_POS_S_TB_ICON   = 1,
  OBJ_PAL_POS_S_BTN       = 4,
};

// サウンド
#define MANUAL_SND_ATM_MOVE    (SEQ_SE_SELECT1)
#define MANUAL_SND_ATM_DECIDE  (SEQ_SE_DECIDE1)
#define MANUAL_SND_BTN_PUSH    (SEQ_SE_SELECT1)
#define MANUAL_SND_SB_MOVE     (SEQ_SE_SELECT1)


// 上画面のダイレクトカラー画像なしのとき
#define MANUAL_BG_M_DCBMP_NO_IMAGE  (0xFFFF)    // 画像ファイルなしのとき

