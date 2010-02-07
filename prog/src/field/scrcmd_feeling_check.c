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
#include "system/main.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_feeling_check.h"

#include "ev_time.h"

#include "savedata/irc_compatible_savedata.h"

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
  SAVE_CONTROL_WORK* p_sv_ctrl  = GAMEDATA_GetSaveControlWork( gdata );
  IRC_COMPATIBLE_SAVEDATA * p_sv  = IRC_COMPATIBLE_SV_GetSavedata( p_sv_ctrl );
  
  u32 sex;
  const STRCODE *cp_name;
  u32 rank;


  GF_ASSERT_MSG( IRC_COMPATIBLE_SV_GetRankNum( p_sv ), "誰とも相性チェックをしていません\n" );

  //プレイヤーのデータを作成し、仲良しの人のインデックスを取り出す
  { 
    const RTCDate *cp_date  = EVTIME_GetRTCDate( gdata );
    OSOwnerInfo info;
    OS_GetOwnerInfo( &info );
    rank  = IRC_COMPATIBLE_SV_GetBestBioRhythm( p_sv, 
        info.birthday.month, info.birthday.day, cp_date );
  }

  //インデックスから仲良しの人のデータを取り出す
  sex     = IRC_COMPATIBLE_SV_GetSex( p_sv, rank );
  cp_name = IRC_COMPATIBLE_SV_GetPlayerName( p_sv, rank );


  //WORDSETに設定
  { 
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    WORDSET *wordset    = SCRIPT_GetWordSet( sc );
    u16 set_pos = SCRCMD_GetVMWorkValue( core, work );
    STRBUF * word = GFL_STR_CreateBuffer( IRC_COMPATIBLE_SV_DATA_NAME_LEN, GFL_HEAP_LOWID( GFL_HEAPID_APP ) );
    WORDSET_RegisterWord( wordset, set_pos, word, sex, TRUE, PM_LANG );
    GFL_STR_DeleteBuffer( word );
  }

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
  SAVE_CONTROL_WORK* p_sv_ctrl  = GAMEDATA_GetSaveControlWork( gdata );
  IRC_COMPATIBLE_SAVEDATA * p_sv  = IRC_COMPATIBLE_SV_GetSavedata( p_sv_ctrl );

  //ランキング入りしている人を返す
  *ret_wk = IRC_COMPATIBLE_SV_GetRankNum( p_sv );

  return VMCMD_RESULT_CONTINUE;
}

