#pragma once

#include "net.h"
#include "gamesystem/game_comm.h"

/*---------------------------------------------------------------------------*
  Name:         WH_AllBeaconStart
  Description:  ビーコンなら何でも収集 GEAR用
  Arguments:    num ビーコン収集数
 *---------------------------------------------------------------------------*/

extern void WH_AllBeaconStart(int num);
extern void WH_AllBeaconEnd(void);

/*---------------------------------------------------------------------------*
  Name:         WH_MainLoopScanBeaconData
  Description:  CGEAR用なんでもビーコン収集MAIN
 *---------------------------------------------------------------------------*/

extern void WH_MainLoopScanBeaconData(void);
extern GAME_COMM_STATUS WH_GetAllBeaconType(void);


