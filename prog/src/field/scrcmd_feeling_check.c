//======================================================================
/**
 * @file  scrcmd_feeling_check.c
 * @brief  スクリプトコマンド用関数：相性チェック
 * @date  2010.02.01
 *
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "ev_time.h"
//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 相性チェック：本日の仲良しの人の名前をバッファにセットする
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetFeelingCheckName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
  
  const RTCDate * EVTIME_GetRTCDate( gdata );
  u16 set_pos = SCRCMD_GetVMWorkValue( core, work );
  u32 sex = PM_MALE;
  //STRBUF * word = GFL_STR_CreateBuffer(...)
  //WORDSET_RegisterWord( wordset, set_pos, word, sex, TRUE, PM_LANG );
  //GFL_STR_DeleteBuffer( word );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   相性チェック：相性チェックした人数を取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetFeelingCheckCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );

  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  *ret_wk = 0;  //ここに相性チェックの人数を入れる

  return VMCMD_RESULT_CONTINUE;
}

