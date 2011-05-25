//******************************************************************************
/**
 * 
 * @file	  scrcmd_pair.c
 * @brief   連れ歩き関連コマンド(常駐)
 * @author  iwasawa	
 * @data	  10.11.18
 *
 */
//******************************************************************************

#include <gflib.h>

#include "system/main.h"

#include "system/vm_cmd.h"
#include "script_local.h"
#include "scrcmd.h"
#include "script.h"
#include "fieldmap.h"

#include "app/pokelist.h"
#include "event_fieldmap_control.h"
#include "savedata/sodateya_work.h"

#include "pair_sys.h"
#include "scrcmd_pair.h"

//--------------------------------------------------------------
/**
 * @brief 連れ歩き開始パラメータセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPairTrainerParamSet( VMHANDLE * core, void *wk )
{
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );  //ID
  u16 pair_id = SCRCMD_GetVMWorkValue( core, wk );  //ペアID
  u16 pair_type = SCRCMD_GetVMWorkValue( core, wk );  //タイプ
  u16 tr_id = SCRCMD_GetVMWorkValue( core, wk );  //トレーナーID
  u16 ev_id = SCRCMD_GetVMWorkValue( core, wk );  //イベントID
  
  GAMEDATA* gdata = SCRCMD_WORK_GetGameData( wk );
  MMDL *mmdl = MMDLSYS_SearchOBJID( GAMEDATA_GetMMdlSys( gdata ), obj_id );
  if ( mmdl == NULL ) {
    GF_ASSERT_MSG( 0, " 対象のOBJ(%d)が居ません\n", obj_id );
    return VMCMD_RESULT_SUSPEND;
  }

  if( ev_id == 0 ){
    ev_id = MMDL_GetEventID(mmdl);
  }
  PAIRSYS_PairSet( gdata, pair_id, pair_type, obj_id, MMDL_GetOBJCode(mmdl), ev_id, tr_id );
  MMDL_ChangeMoveCode( mmdl, MV_PAIR );
  MMDL_ChangeOBJID( mmdl, MMDL_ID_PAIR );

  if( pair_type == PAIR_TYPE_OPEN ){
    MMDL_SetStatusBitNotZoneDelete( mmdl, TRUE );
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief 連れ歩き開始パラメータ解放
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPairTrainerParamFree( VMHANDLE * core, void *wk )
{
  u16 ev_id = SCRCMD_GetVMWorkValue( core, wk );  //EventID
  u16 move_code = SCRCMD_GetVMWorkValue( core, wk );  //MoveCode
  GAMEDATA * gdata = SCRCMD_WORK_GetGameData( wk );
  MMDL *mmdl = MMDLSYS_SearchOBJID( GAMEDATA_GetMMdlSys( gdata ), MMDL_ID_PAIR );

  if( mmdl != NULL ){
    MMDL_SetEventID( mmdl, ev_id );
    MMDL_ChangeMoveCode( mmdl, move_code );
  
    if( PAIRSYS_GetPairType( gdata ) == PAIR_TYPE_CLOSE ){ //クローズ(マップチェンジ無)タイプはOBJIDを元に戻す
      MMDL_ChangeOBJID( mmdl, PAIRSYS_GetPairTrainerOriginalObjID(gdata) );
    }
    MMDL_SetStatusBitNotZoneDelete( mmdl, FALSE );
    MMDL_SetStatusBitPlayerHit( mmdl, TRUE );
  }
  PAIRSYS_PairFree( gdata );
  return VMCMD_RESULT_SUSPEND;
}


