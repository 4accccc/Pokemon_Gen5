//======================================================================
/**
 * @file  pleasure_boat_def.h
 * @brief  遊覧船関連構造体定義
 * @author  Saito
 */
//======================================================================
#pragma once

#define ROOM_NUM  (15)
#define DBL_BTL_ROOM_NUM  (2)

//@note 非トレーナーもいるがどこにトレーナーが配置されるか分からないので全部屋分保持しているメンバもある
typedef struct PL_BOAT_ROOM_PRM_tag
{
  int EntryID;    //抽選エントリＩＤ（0オリジンで、トレーナーと非トレーナーで振りなおし）
  int TrID;       //トレーナーＩＤ(トレーナーのときに使用)
  int ObjCode;    //見た目
  int NpcType;    //メッセージとかＮＰＣタイプとかの識別用
  int NpcMsg[2];
  BOOL BattleEnd;   //その部屋で戦闘を終えたか？（トレーナーのときに使用）
}PL_BOAT_ROOM_PRM;

typedef struct PL_BOAT_WORK_tag
{
  BOOL WhistleReq;
  s16 Time;
  u16 TotalTrNum;
  int WhistleCount;   //汽笛なった回数
  PL_BOAT_ROOM_PRM RoomParam[ROOM_NUM];
  u8 TrNumLeft;
  u8 TrNumRight;
  u8 TrNumSingle;
  u8 TrNumDouble;
  BOOL DblBtl[DBL_BTL_ROOM_NUM];
}PL_BOAT_WORK;

