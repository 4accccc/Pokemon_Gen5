//======================================================================
/**
 * @file	scrcmd_screeneffects.c
 * @brief	スクリプトコマンド：システム管理データ関連
 * @date  2009.09.22
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

#include "scrcmd_enviroments.h"

#include "system/rtc_tool.h"  //GFL_RTC_GetTimeZone
#include "field/zonedata.h"   //ZONEDATA_GetMessageArcID
#include "savedata/mystatus.h"  //MyStatus_～

#include "savedata/save_control.h"



//======================================================================
//======================================================================

//--------------------------------------------------------------
/**
 * 言語IDを切り替え
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChangeLangID( VMHANDLE *core, void *wk )
{
  u8 id = GFL_MSGSYS_GetLangID();
  if( id == 0 ){ //ひらがな
    id = 1;
  }else if( id == 1 ){ //漢字
    id = 0;
  }
  GFL_MSGSYS_SetLangID( id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * メッセージ表示モードを取得する
 * @retval  VMCMD_RESULT_CONTINUE
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetLangID( VMHANDLE *core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  *ret_wk = GFL_MSGSYS_GetLangID();
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 乱数の取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetRand( VMHANDLE *core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 limit = SCRCMD_GetVMWorkValue( core, wk );
  *ret_wk = GFUser_GetPublicRand( limit );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetNowMsgArcID( VMHANDLE * core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  { //とりあえず。
    //本来はスクリプト起動時に使用するメッセージアーカイブIDを保存しておき、
    //そこからわたすような仕組みとするべき
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
    PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
    u16 zone_id = PLAYERWORK_getZoneID( player );
    *ret_wk = ZONEDATA_GetMessageArcID( zone_id );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナーカードランクの取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  とりあえずバッジの数を返している。
 * トレーナーカードの企画ができたらそれにあわせて戻り値をかえす
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetTrainerCardRank( VMHANDLE *core, void *wk )
{
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = MyStatus_GetBadgeCount( GAMEDATA_GetMyStatus(gdata) );
  return VMCMD_RESULT_CONTINUE;
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * 時間帯の取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  時間帯が季節で変わることをどうするか企画と協議
 * @todo  直接RTCでなくイベントで保持している時間帯を参照するのか？
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetTimeZone( VMHANDLE *core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  *ret_wk = GFL_RTC_GetTimeZone();
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 曜日の取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  RTCから直接取得しているが、本当にそれでよいのか？検討する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetWeek( VMHANDLE * core, void *wk )
{
  RTCDate now_date;
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );

  GFL_RTC_GetDate( &now_date );
  *ret_wk = now_date.week;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * バッジフラグの取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetBadgeFlag( VMHANDLE * core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 badge_id = SCRCMD_GetVMWorkValue( core, wk );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );

  *ret_wk = MyStatus_GetBadgeFlag( GAMEDATA_GetMyStatus(gdata), badge_id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * バッジフラグのセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetBadgeFlag( VMHANDLE * core, void *wk )
{
  u16 badge_id = SCRCMD_GetVMWorkValue( core, wk );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );

  MyStatus_SetBadgeFlag( GAMEDATA_GetMyStatus(gdata), badge_id );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * バッジフラグ取得数のカウント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetBadgeCount( VMHANDLE * core, void *wk )
{
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );

  *ret_wk = MyStatus_GetBadgeCount( GAMEDATA_GetMyStatus(gdata) );
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * セーブデータの状態を取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo 書き込むデータサイズに応じて, その段階を返す
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetSaveDataStatus( VMHANDLE * core, void *wk )
{
  GAMEDATA*        gdata = SCRCMD_WORK_GetGameData( wk );
  SAVE_CONTROL_WORK* scw = GAMEDATA_GetSaveControlWork( gdata );
  u16*         ret_exist = SCRCMD_GetVMWork( core, wk );  // コマンド第1引数
  u16*           ret_new = SCRCMD_GetVMWork( core, wk );  // コマンド第2引数
  u16*        ret_volume = SCRCMD_GetVMWork( core, wk );  // コマンド第3引数
  u32         total_size = 0; // セーブ全体のサイズ
  u32        actual_size = 0; // セーブされる実サイズ

  // セーブデータが存在するかどうか
  *ret_exist = SaveData_GetExistFlag( scw );

  // プレイ中のデータが新規データかどうか
  *ret_new   = SaveControl_NewDataFlagGet( scw );

  // @todo 書き込むデータサイズに応じて, その段階を返す
  SaveControl_GetActualSize( scw, &actual_size, &total_size );
  *ret_volume = 0;

  return VMCMD_RESULT_CONTINUE;
} 

//--------------------------------------------------------------
/**
 * @brief ワープIDのセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  どのレイヤーで不正な値のチェックを入れるか検討する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetWarpID( VMHANDLE * core, void *wk )
{
  GAMEDATA*        gamedata = SCRCMD_WORK_GetGameData( wk );
  u16 warp_id = SCRCMD_GetVMWorkValue( core, wk );

  GAMEDATA_SetWarpID( gamedata, warp_id );

  return VMCMD_RESULT_CONTINUE;
}

