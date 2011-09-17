//============================================================================================
/**
 * @file  scrcmd_pokewood_st.c
 * @brief スクリプトコマンド：ポケウッド常駐コマンド
 * @date  2011.09.17
 * @author  tamada GAMEFREAK inc.
 */
//============================================================================================

#include "gflib.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "gamesystem/game_data.h"
#include "savedata/pokewood_savedata.h"

#include "scrcmd_pokewood_st.h"

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　スターランク取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetStarRank( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  POKEWOOD_SAVEDATA * pkwdsave = GAMEDATA_GetPokewoodSavedata( gamedata );
  u32 point = POKEWOOD_SAVEDATA_GetStarPoint( pkwdsave );
  u32 rank = POKEWOOD_SAVEDATA_CalcStarPoint2StarRank( point );

  if ( rank >= POKEWOOD_STAR_RANK_MAX )
  {
    GF_ASSERT( 0 );
    rank = POKEWOOD_STAR_RANK_0;
  }
  *ret_wk = rank;

  //OS_Printf("EvCmdPokewoodGetStarRank() --> %d(%d)\n", rank, point );
  return VMCMD_RESULT_CONTINUE;
}

