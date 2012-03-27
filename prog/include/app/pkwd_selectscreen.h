//============================================================================
/**
 *
 *	@file		pkwd_selectscreen.h
 *	@brief  ポケウッド上映選択画面
 *	@author	mori
 *	@data		2011.07.19
 *
 */
//============================================================================
#pragma once
//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"
#include "savedata/save_control.h"
#include "savedata/pokewood_savedata.h"
#include "savedata/mystatus.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//オーバーレイ領域の指定
FS_EXTERN_OVERLAY( pkwd_selectscreen );

extern const GFL_PROC_DATA PkwdSelectScreenProcData;


//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

typedef enum {
  PKWD_SELECT_MODE_NORMAL = ( 0 ), ///< 上映する映画を選択
  PKWD_SELECT_MODE_DEL    = ( 1 ), ///< 削除する映画を選択
}PKWD_SELECT_MODE;


//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
///	PROCに渡す引数
//=====================================
typedef struct
{	
  MYSTATUS          *mystatus;    ///< 主人公情報（性別・名前）を取得するためのMYSTATUS
  SAVE_CONTROL_WORK *sv;          ///< 外部セーブデータにアクセスするために必要
  u16               *result;      ///< 選択結果を返す。(POKEWOOD_SAVEDATA_MAXだったらキャンセル)
  POKEWOOD_SAVEDATA *pokewood_sv; ///< ポケウッド関連セーブデータ（内部）
  PKWD_SELECT_MODE  mode;         ///< PKWD_SELECT_MODE_NORMAL（選択）かPKWD_SELECT_MODE_DEL(削除）を入れる
}PKWD_SELECT_SCREEN_PARAM;
