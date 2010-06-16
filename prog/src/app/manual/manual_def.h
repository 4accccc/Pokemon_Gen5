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

#include "message.naix"


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


//=============================================================================
/**
*  バージョン違いを吸収する
*/
//=============================================================================
#if PM_VERSION == VERSION_BLACK
  #include "msg/msg_manual_text_b.h"
#else
  #include "msg/msg_manual_text_w.h"
#endif

#if PM_VERSION == VERSION_BLACK
  #include "manual_b.naix"
#else
  #include "manual_w.naix"
#endif

#if PM_VERSION == VERSION_BLACK
  #include "manual_image_b.naix"
#else
  #include "manual_image_w.naix"
#endif

// リソースで生成したヘッダ
#if PM_VERSION == VERSION_BLACK
  #include "../../../../resource/manual/manual_data_def_b.h"
#else
  #include "../../../../resource/manual/manual_data_def_w.h"
#endif


#if PM_VERSION == VERSION_BLACK

// manual_b.naix
enum
{
  NARC_manual_bg_NCLR                   = NARC_manual_b_bg_NCLR,
  NARC_manual_obj_NCLR                  = NARC_manual_b_obj_NCLR,
  NARC_manual_bg_NCGR                   = NARC_manual_b_bg_NCGR,
  NARC_manual_obj_NCGR                  = NARC_manual_b_obj_NCGR,
  NARC_manual_bg1_NSCR                  = NARC_manual_b_bg1_NSCR,
  NARC_manual_bg2_NSCR                  = NARC_manual_b_bg2_NSCR,
  NARC_manual_bg3_NSCR                  = NARC_manual_b_bg3_NSCR,
  NARC_manual_obj_NCER                  = NARC_manual_b_obj_NCER,
  NARC_manual_obj_NANR                  = NARC_manual_b_obj_NANR,
  NARC_manual_manual_data_cate_dat      = NARC_manual_b_manual_data_cate_b_dat,
  NARC_manual_manual_data_cate_ref_dat  = NARC_manual_b_manual_data_cate_ref_b_dat,
  NARC_manual_manual_data_title_dat     = NARC_manual_b_manual_data_title_b_dat,
  NARC_manual_manual_data_title_ref_dat = NARC_manual_b_manual_data_title_ref_b_dat,
};

// msg_manual_text_b.hとmanual_image_b.naixは
// 定義名を使わずに直接番号を使っている。

// message.naix
enum
{
  NARC_message_manual_text_dat1          = NARC_message_manual_text_b_dat,
};

#else

// manual_w.naix
enum
{
  NARC_manual_bg_NCLR                   = NARC_manual_w_bg_NCLR,
  NARC_manual_obj_NCLR                  = NARC_manual_w_obj_NCLR,
  NARC_manual_bg_NCGR                   = NARC_manual_w_bg_NCGR,
  NARC_manual_obj_NCGR                  = NARC_manual_w_obj_NCGR,
  NARC_manual_bg1_NSCR                  = NARC_manual_w_bg1_NSCR,
  NARC_manual_bg2_NSCR                  = NARC_manual_w_bg2_NSCR,
  NARC_manual_bg3_NSCR                  = NARC_manual_w_bg3_NSCR,
  NARC_manual_obj_NCER                  = NARC_manual_w_obj_NCER,
  NARC_manual_obj_NANR                  = NARC_manual_w_obj_NANR,
  NARC_manual_manual_data_cate_dat      = NARC_manual_w_manual_data_cate_w_dat,
  NARC_manual_manual_data_cate_ref_dat  = NARC_manual_w_manual_data_cate_ref_w_dat,
  NARC_manual_manual_data_title_dat     = NARC_manual_w_manual_data_title_w_dat,
  NARC_manual_manual_data_title_ref_dat = NARC_manual_w_manual_data_title_ref_w_dat,
};

// msg_manual_text_w.hとmanual_image_w.naixは
// 定義名を使わずに直接番号を使っている。

// message.naix
enum
{
  NARC_message_manual_text_dat1          = NARC_message_manual_text_w_dat,
};

#endif

