//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：ショップ関連
 * @file   scrcmd_shop.c
 * @author iwasawa
 * @date   2009.10.20
 *
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "app/pokelist.h"
#include "system/main.h"

#include "script_local.h"
//#include "script.h"     // SCRIPT_SetSubProcWorkPointerAdrs
#include "app/p_status.h"
#include "event_fieldmap_control.h"
#include "scrcmd_shop.h"
#include "fieldmap.h"
#include "script_def.h"
#include "field_camera.h"

FS_EXTERN_OVERLAY(pokelist);

typedef struct _SHOP_EVENT_WORK{
  void* bag_param;
  u16*  ret_work;
}SHOP_EVENT_WORK;

//====================================================================
// ■プロトタイプ宣言
//====================================================================

static BOOL callproc_WaitShopBuyEnd( VMHANDLE *core, void *wk );


typedef struct {
  int seq;
  int wait;
} SHOP_BUY_CALL_WORK;

//--------------------------------------------------------------------
/**
 * @brief ショップ購入イベント呼び出し 
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCallShopProcBuy( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  u16               shop_id = SCRCMD_GetVMWorkValue( core, work );
  u16*              ret_work = SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  void** scr_subproc_work = SCRIPT_SetSubProcWorkPointerAdrs( sc );

  SHOP_BUY_CALL_WORK* sbw = GFL_HEAP_AllocClearMemory(HEAPID_PROC,sizeof(SHOP_BUY_CALL_WORK));
  *scr_subproc_work = sbw;


  if(shop_id == SCR_SHOPID_NULL){
    //変動ショップ呼び出し 
     VMCMD_SetWait( core, callproc_WaitShopBuyEnd );
  }else{
    //固定ショップ呼び出し
     VMCMD_SetWait( core, callproc_WaitShopBuyEnd );
  }

  /*
    イベント終了時に ret_work に終了モードを返してください
    SCR_PROC_RETMODE_EXIT 一発終了
    SCR_PROC_RETMODE_NORMAL 継続
  */
  return VMCMD_RESULT_SUSPEND;
}


//----------------------------------------------------------------------------------
/**
 * @brief	ショップ買うサブシーケンス
 *
 * @param   core  SCRCMD_WORK
 * @param   wk    SHOP_BUY_CALL_WORK
 *
 * @retval  BOOL  終了:TRUE   待ち:FALSE
 */
//----------------------------------------------------------------------------------
static BOOL callproc_WaitShopBuyEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  void **scr_subproc_work = SCRIPT_SetSubProcWorkPointerAdrs( sc );
  SHOP_BUY_CALL_WORK *sbw = *scr_subproc_work;
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_CAMERA *fieldcamera = FIELDMAP_GetFieldCamera( fieldmap );


  // 
  switch(sbw->seq)
  {
    case 0:
      OS_Printf("ショップ起動\n");
      FIELD_CAMERA_SetRecvCamParam( fieldcamera );
      {
        FLD_CAM_MV_PARAM param;
        param.Core.Shift.x = FX32_ONE*16*8;
        param.Core.Shift.y = 0;
        param.Core.Shift.z = 0;
  
        param.Chk.Shift    = TRUE;
        param.Chk.Pitch    = FALSE;
        param.Chk.Yaw   = FALSE;
        param.Chk.Dist   = FALSE;
        param.Chk.Fovy = FALSE;
        param.Chk.Pos   = FALSE;
        FIELD_CAMERA_SetLinerParam( fieldcamera, &param, 40 );
      }
      sbw->seq++;
      break;
    case 1:
      if(!FIELD_CAMERA_CheckMvFunc(fieldcamera)){
        sbw->seq++;
      }
      break;
    case 2:
      if(++sbw->wait>30){
        sbw->seq++;
      }
      break;
    case 3:
      {
        FLD_CAM_MV_PARAM_CHK chk = {TRUE, FALSE,FALSE,FALSE,FALSE,};
        FIELD_CAMERA_RecvLinerParam(fieldcamera, &chk, 40);
      }
      sbw->seq++;
      sbw->wait=0;
      break;
    case 4:
      if(!FIELD_CAMERA_CheckMvFunc(fieldcamera)){
        FIELD_CAMERA_ClearRecvCamParam(fieldcamera);
        sbw->seq++;
      }
      break;
    case 5:
      return TRUE;
      break;
  }
  
  return FALSE;
}