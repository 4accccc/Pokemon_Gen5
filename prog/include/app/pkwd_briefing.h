//==============================================================================
/**
 * @file	  pkwd_briefing.h
 * @brief   ポケウッドあらすじ画面
 * @author	mori
 * @date	  2011.08.01(水)
 */
//==============================================================================
#pragma once

#include "savedata/mystatus.h"
#include "gamesystem/game_data.h"
//==============================================================================
//	定数定義
//==============================================================================
FS_EXTERN_OVERLAY(pkwd_briefing);

//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA PkwdBriefingProcData;


typedef struct{
  int               scenario_no;  // シナリオＮ
  int               audienceNum;  // [in]初期観客人数
  u32               audienceBit;  // [out]観客配置bitデータ
  MYSTATUS          *mystatus;    // 
  SAVE_CONTROL_WORK *sv;          // 外部セーブデータにアクセスするために必要
  GAMEDATA          *gameData;    // InfoWin表示に必要
}PKWD_BRIEFING_PARAM;

