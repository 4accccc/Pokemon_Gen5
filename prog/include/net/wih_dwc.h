//=============================================================================
/**
 * @file	  wih_dwc.h
 * @brief	  ビーコン解析を行い、APの判別を行う
 * @author	k.ohno
 * @date    2009.11.15
 */
//=============================================================================
#pragma once

#include "net.h"
#include <dwc.h>
#include "gamesystem/game_comm.h"
#include "wih.h"


typedef struct _WIH_DWC_WORK  WIH_DWC_WORK;


/*---------------------------------------------------------------------------*
  Name:         WH_AllBeaconStart
  Description:  ビーコンなら何でも収集 GEAR用
  Arguments:    num ビーコン収集数
 *---------------------------------------------------------------------------*/

extern WIH_DWC_WORK* WIH_DWC_AllBeaconStart(int num, HEAPID id);
extern void WIH_DWC_AllBeaconEnd(WIH_DWC_WORK* pWork);

/*---------------------------------------------------------------------------*
  Name:         WH_MainLoopScanBeaconData
  Description:  CGEAR用なんでもビーコン収集MAIN
 *---------------------------------------------------------------------------*/

extern void WIH_DWC_MainLoopScanBeaconData(void);
extern GAME_COMM_STATUS WIH_DWC_GetAllBeaconType(void);
extern GAME_COMM_STATUS_BIT WIH_DWC_GetAllBeaconTypeBit(void);


extern void WIH_DWC_Stop(void);
extern void WIH_DWC_Restart(void);

extern const BOOL WIH_DWC_IsEnableBeaconData( const u8 idx );
extern const WMBssDesc* WIH_DWC_GetBeaconData( const u8 idx );

//   AP情報
extern void WIH_DWC_CreateCFG(HEAPID id);
extern void WIH_DWC_DeleteCFG(void);
extern void WIH_DWC_ReloadCFG(void);
