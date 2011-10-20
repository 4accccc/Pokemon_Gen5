//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_pokecen.c
 *	@brief  ポケモンセンター専用スクリプトコマンド
 *	@author	Masayuki Onoue
 *	@date		2011.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "savedata/medalbox.h"

#include "field/medal_check_pokecen.h"

#include "scrcmd_pokecen.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void updateTitleMedal( SCRCMD_WORK * work );
static void updateHintMedals( SCRCMD_WORK * work );
static int getLastAchieveMedal( SCRCMD_WORK * work );

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
  HEAPID           heapID     = SCRCMD_WORK_GetHeapID( work );

 	u16 idx = VMGetU16( core );
  switch ( idx )
  {
  case SCR_MEDALUPDATE_INFO://メダル状態の更新（レコードやセーブデータなどを参照）
    MEDAL_AchiveCheck( gameData ,save , heapID );
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
  case SCR_MEDALUPDATE_INIT://メダルの初期化（ヒントメダルの初期設定など）
    MEDAL_InitHintMedal( save, heapID );
    break;
  default:
    GF_ASSERT( 0 );
  }
  TAMADA_Printf("EvCmdUpdateMedalInfo:%d\n", idx );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//--------------------------------------------------------------
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

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//--------------------------------------------------------------
static int getLastAchieveMedal( SCRCMD_WORK * work )
{
  u32 id;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  const MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

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
  TAMADA_Printf("EvCmdGetLastAchieveMedalID:%d\n", id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 最新の達成メダルのIDを取得する
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//--------------------------------------------------------------
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
    GAMEBEACON_SendDataUpdate_MedalCount( MEDALBOX_GetMedalCount(medalbox,FALSE));
  }
}

