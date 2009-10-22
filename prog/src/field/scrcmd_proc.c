//======================================================================
/**
 * @file    scrcmd_proc.c
 * @brief   スクリプトコマンド用関数　各種アプリ呼び出し系(常駐)
 * @author  Miyuki Iwasawa 
 * @date    09.10.22
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"

#include "msgdata.h"
#include "print/wordset.h"

#include "sound/pm_sndsys.h"
//#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field_sound.h"
#include "field_player.h"
#include "event_fieldmap_control.h"

#include "scrcmd_proc.h"
#include "app/bag.h"
#include "../../../resource/fldmapdata/script/usescript.h"

#include "app/box2.h"
#include "event_fieldmap_control.h"   // for EVENT_FieldSubProc_Callback


// ボックスプロセスデータとコールバック関数
extern const GFL_PROC_DATA BOX2_ProcData;
static void callback_BoxProc( void* work );


//--------------------------------------------------------------
/**
 * @brief フィールドマッププロセス生成
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFieldOpen( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  SCRIPT_CallEvent( sc, EVENT_FieldOpen( gsys ));
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief フィールドマッププロセス破棄
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFieldClose( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  SCRIPT_CallEvent( sc, EVENT_FieldClose(gsys, GAMESYSTEM_GetFieldMapWork(gsys)));
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief サブプロセスの終了を待ちます(ワーク解放処理有り)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  TRUE    終了
 * @retval  FALSE   待ち
 */
//--------------------------------------------------------------
BOOL SCMD_WaitSubProcEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

	if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
    return FALSE;
  }
  SCRIPT_FreeSubProcWorkPointer(sc);

  return TRUE;
}
//--------------------------------------------------------------
/**
 * @brief サブプロセスの終了を待ちます(ワーク解放処理無し)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  TRUE    終了
 * @retval  FALSE   待ち
 *
 * ＊こちらで待った場合、明示的に EvCmdFreeSubProcWork( VMHANDLE *core, void *wk ) を使って解放すること
 */
//--------------------------------------------------------------
BOOL SCMD_WaitSubProcEndNonFree( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

	if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief 汎用サブプロセスワーク解放コマンド
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFreeSubProcWork( VMHANDLE *core, void *wk )
{
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( (SCRCMD_WORK*)wk );
  SCRIPT_FreeSubProcWorkPointer(sc);
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief   バッグプロセスを呼び出します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 *
 * ＊サブプロセスワークの解放処理を行わないので、必ず明示的に EvCmdBagProcWorkFree()を呼び出すこと
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallBagProc( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  void** sub_wk = SCRIPT_SetSubProcWorkPointerAdrs( sc );

  u16 mode = SCRCMD_GetVMWorkValue( core, work );

  if( mode == SCR_BAG_MODE_SELL ){
    mode = BAG_MODE_SELL;
  }else{
    mode = BAG_MODE_FIELD;
  }
  *sub_wk = (void*)BAG_CreateParam( SCRCMD_WORK_GetGameData( work ), NULL, mode );
  
  GAMESYSTEM_CallProc( gsys, FS_OVERLAY_ID(bag), &ItemMenuProcData, *sub_wk ); 
 
  VMCMD_SetWait( core, SCMD_WaitSubProcEndNonFree );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief   バッグプロセスのリターンコードを取得します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 *
 * ＊EvCmdFreeSubProcWork() の前に呼び出すこと
 *
 * バッグプロセスの終了モードと選択アイテムNoを返します
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetBagProcResult( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  BAG_PARAM* bp = (BAG_PARAM*)(SCRIPT_SetSubProcWorkPointer( sc ));

  u16* ret_mode = SCRCMD_GetVMWork( core, work );
  u16* ret_item = SCRCMD_GetVMWork( core, work );

  if(bp->next_proc == BAG_NEXTPROC_EXIT ){
    *ret_mode = SCR_PROC_RETMODE_EXIT;
  }else{
    *ret_mode = SCR_PROC_RETMODE_NORMAL;
  }
  *ret_item = bp->ret_item;
  return VMCMD_RESULT_SUSPEND;
}


//--------------------------------------------------------------
/**
 * @brief ボックスプロセス終了後に呼ばれるコールバック関数
 */
//--------------------------------------------------------------
static void callback_BoxProc( void* work )
{
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * @brief   ボックスプロセスを呼び出します
 * @param   core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallBoxProc( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  BOX2_GFL_PROC_PARAM* box_param = NULL;
  
  // ボックスのプロセスパラメータを作成
  box_param = GFL_HEAP_AllocMemory( heap_id, sizeof(BOX2_GFL_PROC_PARAM) );
  box_param->sv_box = 0;
  box_param->pokeparty = 0;
  box_param->myitem = 0;
  box_param->mystatus = 0;
  box_param->cfg = 0;
  box_param->zknMode = 0;
  box_param->callMode = BOX_MODE_SEIRI;

  // イベントを呼び出す
  //SCRIPT_CallEvent( scw, EVENT_FieldSubProc_Callback(gsys, fieldmap, , index) );
  return VMCMD_RESULT_SUSPEND;
}
