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

typedef struct TH_POINT_WORK_tag
{
  u16 TurnNum;       //かかったターン数
  u16 PokeChgNum;    //交代回数
  u16 VoidAtcNum;    //効果がない技を出した回数
  u16 WeakAtcNum;    //ばつぐんの技を出した回数
  u16 ResistAtcNum;  //いまひとつの技を出した回数
  u16 VoidNum;       //効果がない技を受けた回数
  u16 ResistNum;     //いまひとつの技を受けた回数
  u16 WinTrainerNum; //倒したトレーナー数
  u16 WinPokeNum;    //倒したポケモン数
  u16 LosePokeNum;   //倒されたポケモン数
  u16 RestHpPer;     //残りＨＰ割合
  u16 UseWazaNum;    //使用した技の数
}TH_POINT_WORK;

typedef struct TRIAL_HOUSE_WORK_tag
{
  BSUBWAY_PARTNER_DATA  TrData;
  HEAPID HeapID;
  int MemberNum;        //シングル3　ダブル4
  POKEPARTY *Party;
  BSWAY_PLAYMODE PlayMode;
  BOOL DownLoad;
  TH_POINT_WORK PointWork;
}TRIAL_HOUSE_WORK;
