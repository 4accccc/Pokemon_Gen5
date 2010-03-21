//==============================================================================
/**
 * @file    scrcmd_symbol.c
 * @brief   侵入：シンボルポケモン
 * @author  matsuda
 * @date    2010.03.16(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_symbol.h"
#include "savedata/symbol_save.h"
#include "event_symbol.h"

#include "symbol_map.h"

#include "field_comm/intrude_minimono.h"

//==================================================================
/**
 * シンボルポケモンバトル
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * 捕獲した場合、結果にTRUEを。捕獲しなかった場合、FALSEになります
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolPokeBattle( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work);
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //結果を返すワーク

  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  MMDL *mmdl = SCRIPT_GetTargetObj( scw );  //話しかけ時のみ有効
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  ZONEID zone_id = SCRCMD_WORK_GetZoneID( work );
  static SYMBOL_POKEMON test_symbol_poke = {
    10, 20, 0, 0, 
  };
  
  SCRIPT_CallEvent( scw, 
    EVENT_SymbolPokeBattle(gsys, fieldWork, &test_symbol_poke, ret_wk, HEAPID_PROC) );
  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * シンボルポケモンセッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolPokeSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  DEBUG_INTRUDE_BingoPokeSet(fieldWork);
  
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapMovePokemon( VMHANDLE * core, void * wk )
{
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  return VMCMD_RESULT_CONTINUE;
}
//==================================================================
/**
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapSetMonsName( VMHANDLE * core, void * wk )
{
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapGetInfo( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  u16 mode = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  GAMESYS_WORK *  gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	 
  switch ( mode )
  {
  case SCR_SYMMAP_INFO_IS_MINE:
#if 0
    {
      //@todo 松田くん作成のチェック関数で判定
      if ( IntrudeSymbol_CheckIntrudeNetID( game_comm, gamedata ) == INTRUDE_NETID_NULL ) {
        *ret_wk = TRUE;
      } else {
        *ret_wk = FALSE;
      }
    }
#endif
    break;
  case SCR_SYMMAP_INFO_IS_KEEPZONE:
    {
      SYMBOL_MAP_ID sid = GAMEDATA_GetSymbolMapID( gamedata );
      *ret_wk = SYMBOLMAP_IsKeepzoneID( sid );
    }
    break;
  case SCR_SYMMAP_INFO_IS_ENTRANCE:
    {
      SYMBOL_MAP_ID sid = GAMEDATA_GetSymbolMapID( gamedata );
      *ret_wk = SYMBOLMAP_IsEntranceID( sid );
    }
    break;
  }
  return VMCMD_RESULT_CONTINUE;
}
//==================================================================
/**
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapWarp( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *   work = wk;
  SCRIPT_WORK *     sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *  gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  u16         warp_dir = SCRCMD_GetVMWorkValue( core, work );

  SYMBOL_MAP_ID sid;
  GMEVENT * new_event;

  if ( warp_dir == DIR_NOT ) {
    sid = GAMEDATA_GetSymbolMapID( gamedata );
  } else {
    sid = SYMBOLMAP_GetNextSymbolMapID( GAMEDATA_GetSymbolMapID( gamedata ), warp_dir );
  }
  OS_TPrintf( "SCRCMD:SYMMAP_WARP:id=%2d, dir=%d\n", sid, warp_id );
  new_event = EVENT_SymbolMapWarpEasy( gsys, warp_dir, sid );
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}



