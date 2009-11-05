/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file	 scrcmd_gate.c
 * @brief	 スクリプトコマンド：ゲート関連
 * @date   2009.11.05
 * @author obata
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd_gate.h"
#include "field_gimmick_gate.h"
#include "gimmick_obj_elboard.h"


//---------------------------------------------------------------------------------------
/**
 * @brief 平常時のニュースをセットする
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdElboard_SetNormalNews( VMHANDLE *core, void *wk )
{
  /*
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*     gsys = SCRCMD_WORK_getGameData( work );
  FIELMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  GATE_GIMMICK_Elboard_SetupNormalNews( fieldmap );
  */
  return VMCMD_RESULT_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースを追加する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param  wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//---------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdElboard_AddSpecialNews( VMHANDLE *core, void *wk )
{ 
  /*
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*     gsys = SCRCMD_WORK_getGameData( work );
  FIELMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  NEWS_PARAM news;

  GATE_GIMMICK_Elboard_AddSpecialNews( fieldmap, &news );
  */
  return VMCMD_RESULT_CONTINUE;
}
