//======================================================================
/**
 * @file  scrcmd_un.c
 * @brief  スクリプトコマンド：国連関連
 * @author  Saito
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

#include "savedata/wifihistory.h"
#include "scrcmd_un.h"
#include "united_nations.h" 

//--------------------------------------------------------------
/**
 * 交換した国の数
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_GetCountryNum( VMHANDLE *core, void *wk )
{
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  ret = SCRCMD_GetVMWork( core, work );

  //セーブデータにアクセス
  {
    GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
    *ret = WIFIHISTORY_GetMyCountryCount(wh);
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 有効データ数取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_GetValidDataNum( VMHANDLE *core, void *wk )
{
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  ret = SCRCMD_GetVMWork( core, work );

  //セーブデータにアクセス
  {
    GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
    *ret = WIFIHISTORY_GetValidUnDataNum(wh);
  }

  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * 会話したフラグをON
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_SetTalkFlg( VMHANDLE *core, void *wk )
{
  u8 obj_idx;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);
  
  obj_idx = VMGetU16( core ); //0～4

  //セーブデータにアクセス
  {
    u8 idx = UN_GetUnIdx(unsv_work, obj_idx);
    if (idx != UN_IDX_OVER)
    {
      SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
      WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
      //インデックス番目のデータの話しかけフラグをONにする
      WIFIHISTORY_SetUnInfo(wh, idx, UN_INFO_TALK, 1);
    }
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 会話したかをチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_CheckTalkFlg( VMHANDLE *core, void *wk )
{
  u8 obj_idx;
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);
  
  obj_idx = VMGetU16( core ); //0～4
  ret = SCRCMD_GetVMWork( core, work );

  *ret = FALSE;

  //セーブデータにアクセス
  {
    u8 idx = UN_GetUnIdx(unsv_work, obj_idx);
    if (idx != UN_IDX_OVER)
    {
      u8 flg;
      SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
      WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
      //インデックス番目のデータの話しかけフラグを取得
      flg = WIFIHISTORY_GetUnInfo(wh, idx, UN_INFO_TALK);
      if (flg) *ret = TRUE;
    }
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自分の性格をセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_SetPlayerNature( VMHANDLE *core, void *wk )
{
  u8 nature_idx;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  nature_idx = VMGetU16( core ); //0～4

  //セーブデータにアクセス
  {
    GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
    SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gdata );
    WIFI_HISTORY *wh = SaveData_GetWifiHistory(sv);
    //性格をセット
    WIFIHISTORY_SetMyNature(wh, nature_idx);
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 指定国コードの情報を国連ワークに情報をセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_SetCountryInfo( VMHANDLE *core, void *wk )
{
  u16 country_code;
  u16 floor;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  country_code = SCRCMD_GetVMWorkValue( core, work );
  floor = SCRCMD_GetVMWorkValue( core, work );

  UN_SetData(gsys, country_code, floor);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * メッセージ取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_GetRoomObjMsg( VMHANDLE *core, void *wk )
{
  u16 obj_idx;
  u16 first;
  u16 *msg;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );

  obj_idx = SCRCMD_GetVMWorkValue( core, work );
  first = VMGetU16( core );
  msg = SCRCMD_GetVMWork( core, work );
  *msg = UN_GetRoomObjMsg(wordset, gsys, unsv_work, obj_idx, first);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 国連フロア選択アプリコール
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_CallFloorSelApp( VMHANDLE *core, void *wk )
{
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);

  ret = SCRCMD_GetVMWork( core, work );
  //@todo
  return VMCMD_RESULT_CONTINUE;
}


