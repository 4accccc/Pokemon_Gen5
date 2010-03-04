//============================================================================================
/**
 * @file	wifi_note.h
 * @bfief	WIFI友達手帳
 * @author	k.ohno
 * @date	06.04.05
 */
//============================================================================================
#pragma once

//============================================================================================
//	定義
//============================================================================================

#include "gamesystem/game_data.h"

//------------------------------------------------------
/**
 * WIFIともだちリストPROCパラメータ
 */
//------------------------------------------------------
typedef struct {
  GAMEDATA* pGameData;
	u32	retMode;
}WIFINOTE_PROC_PARAM;

// 終了モード
enum {
	WIFINOTE_RET_MODE_CANCEL = 0,		// キャンセル（↓ボタン）
	WIFINOTE_RET_MODE_EXIT,					// Ｘボタン
};

// ポインタ参照だけできるレコードコーナーワーク構造体
typedef struct _WIFINOTE_WORK WIFINOTE_WORK;	

// プロセス定義データ
extern const GFL_PROC_DATA WifiNoteProcData;


FS_EXTERN_OVERLAY(wifinote);

