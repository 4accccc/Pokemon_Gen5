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
#include "gamesystem/game_beacon.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_local.h" //SCRIPT_GetEvent

#include "scrcmd_medal.h"

#include "savedata/medalbox.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "system/medaldata.h"

#include "../../../resource/fldmapdata/script/medal_scr_def.h"  //SCRID_MEDAL_OYAJI_PC

#include "field/medal_check.h"

//スクリプト参照の値とプログラム中のenum定義の値に差がでた場合Assertとなる
SDK_COMPILER_ASSERT( SCR_MEDALRANK_IRON == MEDALBOX_RANK_IRON );
SDK_COMPILER_ASSERT( SCR_MEDALRANK_COPPER == MEDALBOX_RANK_COPPER );
SDK_COMPILER_ASSERT( SCR_MEDALRANK_SILVER == MEDALBOX_RANK_SILVER );
SDK_COMPILER_ASSERT( SCR_MEDALRANK_GOLD == MEDALBOX_RANK_GOLD );
SDK_COMPILER_ASSERT( SCR_MEDALRANK_BW == MEDALBOX_RANK_BW );

//======================================================================
//======================================================================

static int getLastAchieveMedal( SCRCMD_WORK * work );
static u32 countMedals( SCRCMD_WORK * work, u32 match_state );
static void updateHintMedals( SCRCMD_WORK * work );
static void updateTitleMedal( SCRCMD_WORK * work );
static u32 countRestHintMedal( SCRCMD_WORK * work );
static MMDL * searchMedalOyajiOBJ( FIELDMAP_WORK * fieldWork );
static int searchEventDataMedalOyajiObj( GAMEDATA * gamedata );

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
  const MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

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
    *ret_wk = MEDALBOX_GetMedalBoxMax( medalbox ) + 1;
    break;
  case SCR_MEDALCOUNT_HINT_REST: // (5) 残りのヒントメダル数
    *ret_wk = countRestHintMedal( work );
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
  TAMADA_Printf("EvCmdGetMedalCount:%d:%d\n", idx, *ret_wk );
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
    GAMEBEACON_SendDataUpdate_MedalCount( MEDALBOX_GetMedalCount(medalbox,FALSE));
  } else {
    MEDALBOX_SetHintMedalGetFlg( medalbox, medal_id, date.year, date.month, date.day );
  }
  TAMADA_Printf("EvCmdGetMedal:%d:%d\n", medal_id, mode );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：メダルを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetMedalOyajiObjID( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( SCRCMD_WORK_GetGameSysWork( work ) );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 * obj_id = SCRCMD_GetVMWork( core, wk );

  if ( searchMedalOyajiOBJ( fieldmap ) != NULL )
  {
    *ret_wk = TRUE;
    *obj_id = searchEventDataMedalOyajiObj( gamedata );
  } else {
    *ret_wk = FALSE;
    *obj_id = searchEventDataMedalOyajiObj( gamedata );
  }
  TAMADA_Printf("EvCmdGetMedalOyajiObjID:%d:%d\n", *ret_wk, *obj_id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：メダルを達成状態にする
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetMedalAchive( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( SCRCMD_WORK_GetGameSysWork( work ) );
  GAMEDATA * gamedata         = SCRCMD_WORK_GetGameData( wk );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gamedata );
  MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  u16 medal_id = SCRCMD_GetVMWorkValue( core, wk );
  u8  medal_status = MEDALBOX_GetMedalState( medalbox, medal_id );

  if ( medal_status < MEDAL_STATE_TITLE_ACHIVE )
  {
    ONOUE_Printf("EvCmdSetMedalAchive:%u\n",medal_id);
    MEDALBOX_SetTitleMedalAchive( medalbox, medal_id );
  }
  return VMCMD_RESULT_CONTINUE;
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
 * @retval  -1  達成メダルが存在しない
 * @retval  それ以外  達成メダルのID
 */
//--------------------------------------------------------------
static u32 countMedals( SCRCMD_WORK * work, u32 match_state )
{
  u32 id, count;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  const MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

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
//--------------------------------------------------------------
/**
 * @brief 獲得可能な残りのヒントメダルを数える
 */
//--------------------------------------------------------------
static u32 countRestHintMedal( SCRCMD_WORK * work )
{
  u32 id, count;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX * medalbox   = SaveData_GetMedalBox( save );
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , SCRCMD_WORK_GetHeapID(work) );
  MEDALDATA medal_data;
  for ( id = 0, count = 0; id < MDLID_MAX; id ++ )
  {
    GFL_ARC_LoadDataByHandle( p_handle , id, &medal_data );
    if ( medal_data.hint_type != MEDALDATA_HINTTYPE_NONE
        && MEDALBOX_GetMedalState( medalbox, id ) == MEDAL_STATE_NONE )
    {
      count ++;
    }
  }
  GFL_ARC_CloseDataHandle(p_handle);
  return count;
}

//--------------------------------------------------------------
/**
 * @brief 特定のメダルを持っているかチェック
 * @retval  TRUE  取得済み
 * @retval  FALSE 未取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckIsMedalGet( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  const MEDALBOX * medalbox   = SaveData_GetMedalBox( save );

  u16* ret_wk  = SCRCMD_GetVMWork( core, wk );
  u16 medal_id = SCRCMD_GetVMWorkValue( core, wk );
  u8 medal_status = MEDALBOX_GetMedalState( medalbox, medal_id );

  *ret_wk = (medal_status == MEDAL_STATE_TITLE_GET);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 取得率のもっとも高い比率のカテゴリを取得
 * @retval  TRUE  取得済み
 * @retval  FALSE 未取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetMostGetCategory( VMHANDLE * core, void * wk )
{
  int i;
  u32 id;
  fx32 max_div = 0;
  fx32 count[SCR_MEDALCATEGORY_MAX] = {0};
  fx32 max_count[SCR_MEDALCATEGORY_MAX] = {0};
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gameData = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK* save     = GAMEDATA_GetSaveControlWork( gameData );
  MEDALBOX * medalbox         = SaveData_GetMedalBox( save );
  ARCHANDLE *p_handle         = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , SCRCMD_WORK_GetHeapID(work) );
  MEDALDATA medal_data;
  
  u16* ret_wk  = SCRCMD_GetVMWork( core, wk );

  // カテゴリ分けテーブル
  int category_index[] = {
    SCR_MEDALCATEGORY_ADVENTURE,SCR_MEDALCATEGORY_ADVENTURE,
    SCR_MEDALCATEGORY_ADVENTURE,SCR_MEDALCATEGORY_ADVENTURE,
    SCR_MEDALCATEGORY_ADVENTURE,  // 5
    SCR_MEDALCATEGORY_BATTLE,SCR_MEDALCATEGORY_BATTLE,
    SCR_MEDALCATEGORY_BATTLE,SCR_MEDALCATEGORY_BATTLE,
    SCR_MEDALCATEGORY_BATTLE,     // 5
    SCR_MEDALCATEGORY_AMUSEMENT,SCR_MEDALCATEGORY_AMUSEMENT,
    SCR_MEDALCATEGORY_AMUSEMENT,SCR_MEDALCATEGORY_AMUSEMENT,
    SCR_MEDALCATEGORY_AMUSEMENT,  // 5
    SCR_MEDALCATEGORY_CHALLENGE,SCR_MEDALCATEGORY_CHALLENGE,
    SCR_MEDALCATEGORY_CHALLENGE,SCR_MEDALCATEGORY_CHALLENGE,
    SCR_MEDALCATEGORY_CHALLENGE,  // 5
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,SCR_MEDALCATEGORY_SPECIAL,
    SCR_MEDALCATEGORY_SPECIAL,    // 7
  };

  // カテゴリごとの数をカウント
  for ( id = 0; id < MDLID_MAX; id ++ )
  {
    GFL_ARC_LoadDataByHandle( p_handle , id, &medal_data );
    if ( MEDALBOX_GetMedalState( medalbox, id ) == MEDAL_STATE_TITLE_GET )
    {
      count[category_index[medal_data.graphic_type]] += FX32_ONE;
    }
    max_count[category_index[medal_data.graphic_type]] += FX32_ONE;
  }
  GFL_ARC_CloseDataHandle(p_handle);

  // 最大値を調べる
  {
    // 比較優先度
    int cmp_table[ SCR_MEDALCATEGORY_MAX ] =
    {
      SCR_MEDALCATEGORY_SPECIAL,    // とくべつ
      SCR_MEDALCATEGORY_CHALLENGE,  // ちょうせん
      SCR_MEDALCATEGORY_BATTLE,     // せんとう
      SCR_MEDALCATEGORY_AMUSEMENT,  // おたのしみ
      SCR_MEDALCATEGORY_ADVENTURE,  // ぼうけん
    };

    *ret_wk = 0;
    for ( i = 0; i < SCR_MEDALCATEGORY_MAX; i ++ )
    {
      int index = cmp_table[i];
      fx32 div = FX_Div(count[index],max_count[index]);
      if( max_div < div )
      {
        max_div = div;
        *ret_wk = index;
      }
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief メダルおやじOBJ検索
 *
 * 現在存在している動作モデルデータからメダルおやじを検索しポインタを返す
 */
//--------------------------------------------------------------
static MMDL * searchMedalOyajiOBJ( FIELDMAP_WORK * fieldWork )
{
  u32 no = 0;
	MMDL *mmdl;
  MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	while( MMDLSYS_SearchUseMMdl(mmdlsys,&mmdl,&no) == TRUE ){
		if( MMDL_GetEventID(mmdl) == SCRID_MEDAL_OYAJI_PC ){
			return( mmdl );
		}
	}
  return NULL;
}

//--------------------------------------------------------------
/**
 * @brief メダルおやじOBJデータ検索
 *
 * イベントデータから、メダルおやじOBJを検索しobj_idを返す
 */
//--------------------------------------------------------------
static int searchEventDataMedalOyajiObj( GAMEDATA * gamedata )
{
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gamedata );
  const MMDL_HEADER * tbl = EVENTDATA_GetNpcData( evdata );
  int max = EVENTDATA_GetNpcCount( evdata );
  int i;

  if ( tbl == NULL || max == 0 ) return -1;
  
  for ( i = 0; i < max; i ++ )
  {
    if ( tbl[i].event_id == SCRID_MEDAL_OYAJI_PC ) return tbl[i].id;
  }
  return -1;
}

