//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：通信関連
 * @file   scrcmd_network.c
 * @author obata
 * @date   2009.10.19
 *
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_network.h"
#include "fieldmap.h" //FIELDCOMM_xxxx
#include "event_comm_error.h" // EVENT_FieldCommErroProc
#include "system/net_err.h" //NetErr_ErrorSet();


//====================================================================
// ■プロトタイプ
//====================================================================
static BOOL VM_WAIT_FUNC_FieldCommExitWait( VMHANDLE* core, void* wk );


//--------------------------------------------------------------------
/**
 * @brief フィールド通信の終了待ち関数
 */
//--------------------------------------------------------------------
static BOOL VM_WAIT_FUNC_FieldCommExitWait( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*           work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*          gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr( gsys );
  FIELDMAP_WORK*     fieldmap = GAMESYSTEM_GetFieldMapWork( gsys ); 
  FIELDCOMM_EXIT exit;

  // 通信終了処理
  exit = FIELDCOMM_ExitWait( game_comm ); 
  switch( exit )
  {
  case FIELDCOMM_EXIT_CONTINUE: // 通信終了未完了
    OBATA_Printf( "VM_WAIT_FUNC_FieldCommExitWait: 通信終了未完了\n" );
    return FALSE;
  case FIELDCOMM_EXIT_END:      // 通信終了完了
    OBATA_Printf( "VM_WAIT_FUNC_FieldCommExitWait: 通信終了完了\n" );
    return TRUE;
  case FIELDCOMM_EXIT_ERROR:    // エラー発生
    NetErr_ErrorSet();
    { // イベント呼び出し
      GMEVENT* parent_event = GAMESYSTEM_GetEvent( gsys ); //現在のイベント
      GMEVENT*        event = EVENT_FieldCommErrorProc( gsys, fieldmap );
      GMEVENT_CallEvent( parent_event, event );
    }
    OBATA_Printf( "VM_WAIT_FUNC_FieldCommExitWait: エラー発生\n" );
    return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------------
/**
 * @brief フィールド通信を終了する
 * @param core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdFieldCommExitWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, VM_WAIT_FUNC_FieldCommExitWait );
  OBATA_Printf( "EvCmdFieldCommExitWait\n" );
  return VMCMD_RESULT_SUSPEND;
}
