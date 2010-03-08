//=============================================================================
/**
 * @file	  app_printsys_common.h
 * @brief	  PRINTSYS_PrintStreamGetStateの処理を共通化するプログラム
 * @author  k.ohno
 * @date	  10/02/22
 */
//=============================================================================

#pragma once

#include "print/printsys.h"
#include "sound/wb_sound_data.sadl"

//-------------------------------------
///	定義
//=====================================
//入力キー
#define APP_PRINTSYS_COMMON_TRG_KEY (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)

//ページ送り音
#define APP_PRINTSYS_COMMON_SE_TRG  (SEQ_SE_MESSAGE)

//-------------------------------------
///	タッチ方式
//=====================================
typedef enum
{
  APP_PRINTSYS_COMMON_TYPE_THROUGH,
  APP_PRINTSYS_COMMON_TYPE_KEY    = 1<<1,
  APP_PRINTSYS_COMMON_TYPE_TOUCH  = 1<<2,
  APP_PRINTSYS_COMMON_TYPE_BOTH   = APP_PRINTSYS_COMMON_TYPE_KEY|APP_PRINTSYS_COMMON_TYPE_TOUCH,
}APP_PRINTSYS_COMMON_TYPE;

//-------------------------------------
///	制御ワーク
//=====================================
typedef struct 
{
  u8                         trg;  //離した検知
  u8                         dummy[3];
  APP_PRINTSYS_COMMON_TYPE   type; //トリガで使用するもの
} APP_PRINTSYS_COMMON_WORK;


//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム制御初期化
 *	@param	トリガーを制御すりワーク u8
 */
//-----------------------------------------------------------------------------
extern void APP_PRINTSYS_COMMON_PrintStreamInit(APP_PRINTSYS_COMMON_WORK *wk, APP_PRINTSYS_COMMON_TYPE type);
//----------------------------------------------------------------------------
/**
 *	@brief	プリントシステム実行関数
 *  @param  PRINT_STREAM* ストリームハンドル
 *	@param	初期化で渡したトリガーを制御すりワーク u8
 */
//-----------------------------------------------------------------------------
extern BOOL APP_PRINTSYS_COMMON_PrintStreamFunc( APP_PRINTSYS_COMMON_WORK *wk, PRINT_STREAM* handle );

