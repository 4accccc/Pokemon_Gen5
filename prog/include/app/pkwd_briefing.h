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
#include "battle/battle.h"

//==============================================================================
//	定数定義
//==============================================================================
FS_EXTERN_OVERLAY(pkwd_briefing);

//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA PkwdBriefingProcData;
extern const GFL_PROC_DATA PkwdEndrollProcData;


typedef struct{
  int               scenario_no;  ///< [in]シナリオＮ
  int               audienceNum;  ///< [in]初期観客人数
  u32               audienceBit;  ///< [out]観客配置bitデータ
  MYSTATUS          *mystatus;    ///< 
  SAVE_CONTROL_WORK *sv;          ///< [in]外部セーブデータにアクセスするために必要
  GAMEDATA          *gameData;    ///< [in]InfoWin表示に必要
  BATTLE_SETUP_PARAM *bsp;        ///< [in]バトルセットアップパラメータ
  BOOL              no_cancel   ; ///< [in]キャンセル不可フラグ(TRUE:不可 FALSE:可能)
  BOOL              done_cancel;  ///< [out]キャンセルしたか？(TRUE:した FALSE:してない）
}PKWD_BRIEFING_PARAM;

