//============================================================================
/**
 *
 *	@file		pkwd_select_screen.h
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

#define POKEWOOD_SAVEDATA_MAX   ( 5 ) //ポケウッド録画データ最大数（＝キャンセル値）

#define PKWD_SELECT_MODE_NORMAL ( 0 ) // 上映する映画を選択
#define PKWD_SELECT_MODE_DEL    ( 1 ) // 削除する映画を選択


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
  MYSTATUS          *mystatus;
  SAVE_CONTROL_WORK *sv;          // 外部セーブデータにアクセスするために必要
  s16               *result;      // 選択結果を返す。(POKEWOOD_SAVEDATA_MAXだったらキャンセル)
  POKEWOOD_SAVEDATA *pokewood_sv; // ポケウッド関連セーブデータ（内部）
  int               mode;         // PKWD_SELECT_MODE_NORMAL（選択）かPKWD_SELECT_MODE_DEL(削除）を入れる
}PKWD_SELECT_SCREEN_PARAM;
