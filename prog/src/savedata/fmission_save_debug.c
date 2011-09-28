//============================================================================================
/**
 * @file    fmission_save_debug.c
 * @brief   フェスミッションセーブデータ　デバッグ用関数群(D_LINKFESTオーバーレイ)
 * @author　Miyuki Iwasawa
 * @date    11.09.28
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_local.h"
#include "savedata/save_tbl.h"
#include "savedata/fmission_save.h"
#include "fmission_save_local.h"

#ifdef PM_DEBUG

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 全ミッションのクリアレベルをセット
 * @param	work		ワークへのポインタ
 *
 * @param   FMISSION_CLEAR_LEVEL_0-FMISSION_LEVEL_4
 */
//---------------------------------------------------------------------------
void DEBUG_FMISSION_SV_SetMissionStatusClearLevelAll( FMISSION_SAVE* sv,  FMISSION_CLEAR_LEVEL level )
{
  int i;

  for( i = 0;i < FMISSION_ID_MAX; i++ ){
    sv->status[i].level = level;
  }
  IWASAWA_Printf("FMissionClearLvSet %d\n", level );
}


#endif //PM_DEBUG
