//======================================================================
/**
 * @file  scrcmd_fldci.c
 * @brief  フィールドカットインスクリプトコマンド用関数
 * @author  Nozomu Satio
 *
 */
//======================================================================

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

#include "event_fieldmap_control.h"

#include "scrcmd_fldci.h"
#include "fld3d_ci.h"

#include "item/itemsym.h" //for ITEM_
#include "../../../resource/fld3d_ci/fldci_id_def.h"  //for FLDCIID_

#define ITEM_CUTIN_MAX  (17)

typedef struct ITEM_CI_NO_tag
{
  u16 ItemNo;
  u16 CutinNo;
}ITEM_CI_NO;

const ITEM_CI_NO ItemCutinNo[ITEM_CUTIN_MAX] = {
  {ITEM_RAITOSUTOON, FLDCIID_GET1},
  {ITEM_DAAKUSUTOON, FLDCIID_GET1_02},
  {ITEM_GODDOSUTOON, FLDCIID_GET1_03},
  {ITEM_GURASIDEANOHANA, FLDCIID_GET1_04},

  {ITEM_TOMODATITETYOU, FLDCIID_GET2},
  {ITEM_TAUNMAPPU, FLDCIID_GET2_02},
  {ITEM_SUGOITURIZAO, FLDCIID_GET2_03},
  {ITEM_ZITENSYA, FLDCIID_GET2_04},
  {ITEM_BATORUREKOODAA, FLDCIID_GET2_05},
  {ITEM_DAUZINGUMASIN, FLDCIID_GET2_06},
//  {ITEM_ROKKUKAPUSERU, FLDCIID_GET2_07},   不要になりました 20100420
  {ITEM_RAIBUKYASUTAA, FLDCIID_GET2_08M},
  {ITEM_RAIBUKYASUTAA_1, FLDCIID_GET2_08},
  {ITEM_GUZZUKEESU, FLDCIID_GET2_09},
  {ITEM_HAITATUBUTU1, FLDCIID_GET2_10},
  {ITEM_HAITATUBUTU2, FLDCIID_GET2_11},
  {ITEM_HAITATUBUTU3, FLDCIID_GET2_12},
  {ITEM_DORAGONNOHONE, FLDCIID_GET2_13},
//※リバティチケットは配達員からなのでここにはなし
};

//--------------------------------------------------------------
/**
 * フィールドカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallCutin( VMHANDLE *core, void *wk )
{
  u16 cutin_no;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  cutin_no = VMGetU16( core );

  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreateCutInEvt(gsys, ptr, cutin_no);
    if (call_event != NULL )
    {
      SCRIPT_CallEvent( sc, call_event );
    }
  }
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ポケモンカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallPokeCutin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*    work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  u16 pos = SCRCMD_GetVMWorkValue( core, work );
  
  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreatePokeCutInEvtTemoti( gsys, ptr, pos );
    if (call_event != NULL)
    {
      SCRIPT_CallEvent( sc, call_event );
    }
  }
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * NPC空を飛ぶカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallNpcFlyCutin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*    work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  
  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreateNpcFlyCutInEvt( gsys, ptr, obj_id );
    if (call_event != NULL)
    {
      SCRIPT_CallEvent( sc, call_event );
    }
  }
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * アイテムカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallItemCutin( VMHANDLE *core, void *wk )
{
  u16 cutin_no;
  u16 item_no;
  BOOL find;
  int i;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  item_no = SCRCMD_GetVMWorkValue( core, work );

  find = FALSE;

  //カットインナンバー検索
  for (i=0;i<ITEM_CUTIN_MAX;i++)
  {
    if (ItemCutinNo[i].ItemNo == item_no)
    {
      cutin_no = ItemCutinNo[i].CutinNo;
      find = TRUE;
      break;
    }
  }

  GF_ASSERT_MSG(find,"item cutin not finded %d",item_no);

  //カットインイベントをコール
  if (find){
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreateCutInEvt(gsys, ptr, cutin_no);
    if (call_event != NULL)
    {
      SCRIPT_CallEvent( sc, call_event );
    }
  }
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}


