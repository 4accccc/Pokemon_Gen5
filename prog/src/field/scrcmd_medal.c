//======================================================================
/**
 * @file	scrcmd_medal.c
 *
 * @brief	スクリプトコマンド：メダルイベント関連
 * @date  2011.02.05
 * @author	tamada GAMEFREAK inc.
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
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_local.h" //SCRIPT_GetEvent

#include "scrcmd_medal.h"

#include "savedata/medalbox.h"


//======================================================================
//======================================================================

static int getLastAchieveMedal( SCRCMD_WORK * work );
static u32 countMedals( SCRCMD_WORK * work, u32 match_state );
static void updateHintMedals( SCRCMD_WORK * work );
static void updateTitleMedal( SCRCMD_WORK * work );

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：メダルのカウント取得いろいろ
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetMedalCount( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  /*const*/ MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

 	u8 idx = VMGetU8( core );
  u16 * ret_wk  = SCRCMD_GetVMWork( core, wk );

  switch (idx)
  {
  case SCR_MEDALCOUNT_HINT_ACHIEVE: //(0) // 達成したヒントメダル数
    *ret_wk = countMedals( work, MEDAL_STATE_HINT_ACHIVE );
    break;
  case SCR_MEDALCOUNT_HINT_GET: // (1) 取得したヒントメダル数
    *ret_wk = countMedals( work, MEDAL_STATE_HINT_GET );
    break;
  case SCR_MEDALCOUNT_TITLE_ACHIEVE: // (2) 達成したタイトルメダル数
    *ret_wk = countMedals( work, MEDAL_STATE_TITLE_ACHIVE );
    break;
  case SCR_MEDALCOUNT_TITLE_GET: // (3) 取得したタイトルメダル数
    *ret_wk = countMedals( work, MEDAL_STATE_TITLE_GET );
    break;
  case SCR_MEDALCOUNT_NEXT_RANK: // (4) 次のランクまでに必要なタイトルメダル数
    *ret_wk = MEDALBOX_GetMedalBoxMax( medalbox );
    break;
  case SCR_MEDALCOUNT_HINT_REST: // (5) 残りのヒントメダル数
    /* 未実装 */
    break;
  case SCR_MEDALCOUNT_TITLE_REST: // (6) 残りのタイトルメダル数
    *ret_wk = MDLID_MAX - countMedals( work, MEDAL_STATE_TITLE_GET );
    break;
  case SCR_MEDALCOUNT_RANK: // (7) 現在のランク
    *ret_wk = MEDALBOX_GetMedalBoxRank( medalbox );
    break;
  default:
    GF_ASSERT(0);

  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：メダル状態更新いろいろ
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUpdateMedalInfo( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX *       medalbox   = SaveData_GetMedalBox( save );

 	u8 idx = VMGetU8( core );
  switch ( idx )
  {
  case SCR_MEDALUPDATE_INFO://メダル状態の更新（レコードを参照）
    /* 未実装 */
    break;
  case SCR_MEDALUPDATE_TITLE://タイトルメダルの更新（達成→取得）
    updateTitleMedal( work );
    break;
  case SCR_MEDALUPDATE_HINT://ヒントメダルの更新（達成→取得）
    updateHintMedals( work );
    break;
  case SCR_MEDALUPDATE_RANK://ランクの更新
    MEDALBOX_RankUpMedalBox( medalbox );
    break;
  default:
    GF_ASSERT( 0 );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetLastAchieveMedalID( VMHANDLE * core, void * wk )
{
  u16 * ret_result  = SCRCMD_GetVMWork( core, wk );
  u16 * ret_medalid = SCRCMD_GetVMWork( core, wk );
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  int id;

  id = getLastAchieveMedal( work );
  if ( id < 0 )
  {
    *ret_result = FALSE;
    *ret_medalid = 0;
  } else {
    *ret_result = TRUE;
    *ret_medalid = id;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：メダルを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetMedal( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  u16 medal_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 mode     = SCRCMD_GetVMWorkValue( core, wk );
  RTCDate date;

  GF_ASSERT( medal_id < MDLID_MAX );

  GFL_RTC_GetDate( &date );
  if ( mode )
  {
    MEDALBOX_SetTitleMedalGetFlg( medalbox, medal_id, date.year, date.month, date.day );
  } else {
    MEDALBOX_SetHintMedalGetFlg( medalbox, medal_id, date.year, date.month, date.day );
  }
  return VMCMD_RESULT_CONTINUE;
}


//======================================================================
//======================================================================
//-----------------------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//-----------------------------------------------------------------------------
static int getLastAchieveMedal( SCRCMD_WORK * work )
{
  u32 id;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  /*const*/ MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  for ( id = 0; id < MDLID_MAX; id ++ )
  {
    u8 medal_status = MEDALBOX_GetMedalState( medalbox, id );
    if ( medal_status == MEDAL_STATE_TITLE_ACHIVE )
    {
      return id;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//-----------------------------------------------------------------------------
static u32 countMedals( SCRCMD_WORK * work, u32 match_state )
{
  u32 id, count;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  /*const*/ MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  for ( count = 0, id = 0; id < MDLID_MAX; id ++ )
  {
    u8 medal_status = MEDALBOX_GetMedalState( medalbox, id );
    if ( medal_status == match_state )
    {
      count ++;
    }
  }
  return count;
}

//-----------------------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//-----------------------------------------------------------------------------
static void updateHintMedals( SCRCMD_WORK * work )
{
  u32 id;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  RTCDate date;

  GFL_RTC_GetDate( &date );
  for ( id = 0; id < MDLID_MAX; id ++ )
  {
    u8 medal_status = MEDALBOX_GetMedalState( medalbox, id );
    if ( medal_status == MEDAL_STATE_HINT_ACHIVE )
    {
      MEDALBOX_SetHintMedalGetFlg( medalbox, id, date.year, date.month, date.day );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//-----------------------------------------------------------------------------
static void updateTitleMedal( SCRCMD_WORK * work )
{
  u32 id;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  RTCDate date;

  GFL_RTC_GetDate( &date );
  id = getLastAchieveMedal( work );
  if ( id >= 0 )
  {
    MEDALBOX_SetTitleMedalGetFlg( medalbox, id, date.year, date.month, date.day );
  }
}

