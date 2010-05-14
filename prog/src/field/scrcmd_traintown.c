//======================================================================
/**
 * @file	scrcmd_traintown.c
 *
 * @brief	スクリプトコマンド：トレインタウン関連
 * @date  2010.03.28
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

#include "scrcmd_traintown.h"

#include "ev_time.h"    //EVTIME_Get

#include "arc/fieldmap/buildmodel_outdoor.naix" //NARC_output_buildmodel_


//======================================================================
//======================================================================
static BOOL isSpecialDay( int month, int day );

/**
 * @def SP_DAY_VALUE
 * @brief 日付をu16に変換
 * @param m 月
 * @param d 日
 */
#define SP_DAY_VALUE(m,d) (m << 8 | d )

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief トレインタウンの電車用モデルIDを取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetTrainModelID( VMHANDLE * core, void *wk )
{
  u16 train_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  static const BMODEL_ID trains[] = {
    NARC_output_buildmodel_outdoor_tt_train01_nsbmd,  //エラーよけ用
    NARC_output_buildmodel_outdoor_tt_train01_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train02_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train03_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train04_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train05_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train06_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train07_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train08_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train09_nsbmd,
    NARC_output_buildmodel_outdoor_tt_train10_nsbmd,
  };
  if ( train_id >= NELEMS(trains) )
  {
    GF_ASSERT( 0 );
    train_id = 0;
  }
  *ret_wk = trains[train_id];
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 特別な日か？のチェック
 */
//--------------------------------------------------------------
static BOOL isSpecialDay( int month, int day )
{
  static const u16 spDaysTable[] = {
    SP_DAY_VALUE(10, 1),  //新幹線の創設
    SP_DAY_VALUE(10,14),  //鉄道の日
    SP_DAY_VALUE(12,30),  //地下鉄開業
    SP_DAY_VALUE( 2, 1),  //電気鉄道開業
    SP_DAY_VALUE( 6,12),  //日本初の鉄道仮開業日
  };
  int i;
  u16 today = SP_DAY_VALUE( month, day );
  for (i = 0; i < NELEMS(spDaysTable); i++ )
  {
    if ( spDaysTable[i] == today ) return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 * @brief トレインタウン：電車の種類抽選コマンド
 *
 * @li  01	シングルトレイン	日付の下一桁が1
 * @li  02	スーパーシングルトレイン	日付の下一桁が5
 * @li  03	ダブルトレイン	日付の下一桁が2
 * @li  04	スーパーダブルトレイン	日付の下一桁が4
 * @li  05	マルチトレイン	日付の下一桁が3
 * @li  06	スーパーマルチトレイン	日付の下一桁が6
 * @li  07	Wi-Fiトレイン	日付の下一桁が9
 * @li  08	トレインタウン	日付の下一桁が7
 * @li  09	古いレアトレイン　	日付の下一桁が0
 * @li  10	新しいレアトレイン	特別な日
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetTrainTownCondition( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK * work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  int month = EVTIME_GetMonth( gamedata );
  int   day = EVTIME_GetDay( gamedata );

  if ( isSpecialDay( month, day ) == TRUE )
  {
    *ret_wk = 10;   //特別な日＝最新型レアトレイン
  }
  else
  {
    static const u8 ids[10] = {
      9,  //?0:古いレアトレイン
      1,  //?1:シングルトレイン
      3,  //?2:ダブルトレイン
      5,  //?3:マルチトレイン
      4,  //?4:スーパーダブルトレイン
      2,  //?5:スーパーシングルトレイン
      6,  //?6:スーパーマルチトレイン
      8,  //?7:Wi-Fiトレイン
      0,  //?8:おやすみ
      7,  //?9:古いレアトレイン

    };
    *ret_wk = ids[day % 10];
  }

  return VMCMD_RESULT_CONTINUE;
}


