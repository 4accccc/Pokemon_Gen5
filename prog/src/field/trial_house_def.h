//======================================================================
/**
 * @file  trial_house_def.h
 * @brief  トライアルハウス関連構造体定義
 * @author  Saito
 */
//======================================================================
#pragma once

#include "savedata/bsubway_savedata.h"  //for _BSUBWAY_PARTNER_DATA
#include "battle/bsubway_battle_data.h"  //for _BSUBWAY_PARTNER_DATA

#include "poke_tool/pokeparty.h"

typedef struct TRIAL_HOUSE_WORK_tag
{
  BSUBWAY_PARTNER_DATA  TrData;
  HEAPID HeapID;
  int MemberNum;        //シングル3　ダブル4
  POKEPARTY *Party;
  BSWAY_PLAYMODE PlayMode;
  BOOL DownLoad;
}TRIAL_HOUSE_WORK;
