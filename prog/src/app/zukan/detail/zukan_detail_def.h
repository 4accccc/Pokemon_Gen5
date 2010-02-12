//============================================================================
/**
 *  @file   zukan_detail_def.h
 *  @brief  図鑑詳細画面共通
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *
 *  モジュール名：ZKNDTL
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "zknd_tbar.h"


//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
// メインBGフレーム
#define ZKNDTL_BG_FRAME_M_TOUCHBAR              (GFL_BG_FRAME1_M)
// メインBGフレームのプライオリティ
#define ZKNDTL_BG_FRAME_PRI_M_TOUCHBAR          (0)

// メインBGパレット
// 本数
enum
{
  ZKNDTL_BG_PAL_NUM_M_TOUCHBAR     =  ZKND_TBAR_BG_PLT_NUM,  // 1本
};
// 位置
enum
{
  ZKNDTL_BG_PAL_POS_M_TOUCHBAR     =  13,
};

// メインOBJパレット
// 本数
enum
{
  ZKNDTL_OBJ_PAL_NUM_M_TOUCHBAR    =  ZKND_TBAR_OBJ_PLT_NUM + 3,  // 3本+3本
};
// 位置
enum
{
  ZKNDTL_OBJ_PAL_POS_M_TOUCHBAR    =   8,
};


// サブBGフレーム
#define ZKNDTL_BG_FRAME_S_HEADBAR               (GFL_BG_FRAME1_S)
// サブBGフレームのプライオリティ
#define ZKNDTL_BG_FRAME_PRI_S_HEADBAR           (0)

// サブBGパレット
// 本数
enum
{
  ZKNDTL_BG_PAL_NUM_S_HEADBAR     =  1,
};
// 位置
enum
{
  ZKNDTL_BG_PAL_POS_S_HEADBAR     =  13,
};

// サブOBJパレット
// 本数
enum
{
  ZKNDTL_OBJ_PAL_NUM_S_HEADBAR    =  0,
};
// 位置
enum
{
  ZKNDTL_OBJ_PAL_POS_S_HEADBAR    =  0,
};


// セルの読みわけに使うマッピングモード
#define ZKNDTL_OBJ_MAPPING_M                (APP_COMMON_MAPPING_128K)
#define ZKNDTL_OBJ_MAPPING_S                (APP_COMMON_MAPPING_128K)


// コマンド
typedef enum
{
  ZKNDTL_CMD_NONE,
  ZKNDTL_CMD_CLOSE,
  ZKNDTL_CMD_RETURN,
  ZKNDTL_CMD_CUR_D,
  ZKNDTL_CMD_CUR_U,
  ZKNDTL_CMD_CHECK,
  ZKNDTL_CMD_INFO,
  ZKNDTL_CMD_MAP,
  ZKNDTL_CMD_VOICE,
  ZKNDTL_CMD_FORM,
  ZKNDTL_CMD_MAP_RETURN,
  ZKNDTL_CMD_FORM_RETURN,
  ZKNDTL_CMD_FORM_CUR_R,
  ZKNDTL_CMD_FORM_CUR_L,
  ZKNDTL_CMD_FORM_CUR_D,
  ZKNDTL_CMD_FORM_CUR_U,
  ZKNDTL_CMD_FORM_EXCHANGE,
}
ZKNDTL_COMMAND;

