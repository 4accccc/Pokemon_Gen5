//======================================================================
/**
 * @file  scrcmd_bsybway.h
 * @brief  バトルサブウェイ　スクリプトコマンド専用ソース
 * @author	Miyuki Iwasawa
 * @date  2006.05.23
 *
 * 2007.05.24	Satoshi Nohara*
 *
 * @note plから移植 kagaya
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern VMCMD_RESULT EvCmdBSubwayWorkCreate( VMHANDLE* core, void *wk );
extern VMCMD_RESULT EvCmdBSubwayWorkClear(VMHANDLE* core, void *wk );
extern VMCMD_RESULT EvCmdBSubwayWorkRelease(VMHANDLE* core, void *wk );
extern VMCMD_RESULT EvCmdBSubwayTool( VMHANDLE *core, void *wk );

#ifdef PM_DEBUG
extern void BSUBWAY_SCRWORK_DebugCreateWork( GAMESYS_WORK *gsys, u16 mode );
extern void BSUBWAY_SCRWORK_DebugCreateWorkTrNo(
    GAMESYS_WORK *gsys, u16 mode, const u16 *tr_no );
extern void BSUBWAY_SCRWORK_DebugFightAnyRound(
    GAMESYS_WORK *gsys, u16 game_round_now );
extern u8 BSUBWAY_SCRWORK_DebugGetFlag( GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_DebugSetFlag( GAMESYS_WORK *gsys, u8 flag );
extern int BSUBWAY_SCRWORK_DebugGetWifiRank( GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_DebugSetWifiRank( GAMESYS_WORK *gsys, u8 rank );
extern void BSUBWAY_SCRWORK_DebugClearWifiRoomData( GAMESYS_WORK *gsys );
extern void BSUBWAY_SCRWORK_DebugSetCommMultiStage(
    GAMESYS_WORK *gsys, u16 play_mode, u16 stage );
#endif
