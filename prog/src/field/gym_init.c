//======================================================================
/**
 * @file  gym_init.c
 * @bfief  ジム初期化
 * @author  Saito
 */
//======================================================================

#include "gym_init.h"
#include "gym_elec_sv.h"
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "fieldmap.h"

//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_INIT_Elec(FIELDMAP_WORK *fieldWork)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }
  gmk_sv_work->NowRaleIdx[0] = 0;
  gmk_sv_work->NowRaleIdx[1] = 2;
  gmk_sv_work->NowRaleIdx[2] = 4;
  gmk_sv_work->NowRaleIdx[3] = 6;

  gmk_sv_work->StopPlatformIdx[0] = PLATFORM_NO_STOP;
  gmk_sv_work->StopPlatformIdx[1] = PLATFORM_NO_STOP;
  gmk_sv_work->StopPlatformIdx[2] = PLATFORM_NO_STOP;
  gmk_sv_work->StopPlatformIdx[3] = PLATFORM_NO_STOP;
}
