//============================================================================
/**
 *
 *	@file		fmission_app.h
 *	@brief  フェスミッションアプリ メインヘッダ
 *	@author Miyuki Iwasawa	
 *	@data		2011.04.22
 */
//============================================================================
#pragma once
//	lib
#include <gflib.h>

//system
#include "gamesystem/gamesystem.h"
#include "gamesystem/link_festival.h"
#include "savedata/fmission_save.h"

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern const GFL_PROC_DATA FMissionAppProcData;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

typedef enum{
  FMISSION_APP_MODE_INFO,		// 詳細画面
  FMISSION_APP_MODE_RESULT,		// 評価画面
  //
  FMISSION_APP_MODE_MAX,
}FMISSION_APP_MODE;

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
	GAMESYS_WORK*		gsys;	///<[in]ゲームシステム
	GAMEDATA*			gdata;	///<ゲームデータ
	//
	FMISSION_APP_MODE	mode;	///< 画面切り替え

  LINK_FESTIVAL* fest_sys;  ///<ハイリンクフェス　管理ワーク
  const FEST_MISSION* fmwk; ///<現在のミッションデータ

  FMISSION_SAVE*  fmission_sv;  ///<フェスセーブデータポインタ

}FMISSION_APP_PARAM;

