//======================================================================
/**
 * @file	scrcmd_hiden.c
 * @brief	スクリプトコマンド：秘伝技系
 * @date  2009.09.22
 * @author	tamada GAMEFREAK inc.
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_hiden.h"

#include "fieldmap.h"
#include "field_player_grid.h"
#include "fldeff_iaigiri.h"

//======================================================================
//  秘伝技波乗り
//======================================================================
//--------------------------------------------------------------
/**
 * 秘伝技波乗り開始　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitNaminori( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GFL_TCB *tcb = SCRCMD_WORK_GetCallProcTCB( work ); 
  
  if( FIELD_PLAYER_GRID_CheckEventNaminoriStart(tcb) == TRUE ){
    FIELD_PLAYER_GRID_DeleteEventNaminoriStart( tcb );
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 秘伝技波乗り開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdNaminori( VMHANDLE *core, void *wk )
{
  GFL_TCB *tcb;
  VecFx32 pos;
  MAPATTR attr;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  u16 dir = MMDL_GetDirDisp( mmdl );
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

  FIELD_PLAYER_GetDirPos( fld_player, dir, &pos );
  attr = MAPATTR_GetAttribute( mapper, &pos );

  tcb = FIELD_PLAYER_GRID_SetEventNaminoriStart(
      fld_player, dir, attr, heapID );
  SCRCMD_WORK_SetCallProcTCB( work, tcb );
  VMCMD_SetWait( core, EvWaitNaminori );
  
  return( VMCMD_RESULT_SUSPEND );
}

//======================================================================
//  秘伝技滝登り
//======================================================================
//--------------------------------------------------------------
/**
 * 秘伝技滝上り　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitTakinobori( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GFL_TCB *tcb = SCRCMD_WORK_GetCallProcTCB( work ); 
  
  if( FIELD_PLAYER_GRID_CheckEventTakinobori(tcb) == TRUE ){
    FIELD_PLAYER_GRID_DeleteEventTakinobori( tcb );
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 秘伝技滝登り開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTakinobori( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16 dir = FIELD_PLAYER_GetDir( fld_player );
  GFL_TCB *tcb = FIELD_PLAYER_GRID_SetEventTakinobori( fld_player,
      dir, SCRCMD_GetVMWorkValue(core,work), heapID );
  SCRCMD_WORK_SetCallProcTCB( work, tcb );
  VMCMD_SetWait( core, EvWaitTakinobori );
  return( VMCMD_RESULT_SUSPEND );
}

//--------------------------------------------------------------
/**
 * 秘伝技 居合い切りエフェクト表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdIaigiriEffect( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER*    player = FIELDMAP_GetFieldPlayer( fieldmap );
  MMDL*              mmdl = FIELD_PLAYER_GetMMdl( player );
  FLDEFF_CTRL*     fectrl = FIELDMAP_GetFldEffCtrl( fieldmap );

  FLDEFF_IAIGIRI_SetMMdl( mmdl, fectrl );
  return VMCMD_RESULT_CONTINUE;
}

