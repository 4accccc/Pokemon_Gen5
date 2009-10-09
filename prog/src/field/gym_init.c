//======================================================================
/**
 * @file  gym_init.c
 * @brief  ジム初期化
 * @author  Saito
 */
//======================================================================

#include "gym_init.h"
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "gym_elec_sv.h"
#include "gym_normal_sv.h"
#include "gym_anti_sv.h"

//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_INIT_Elec(GAMESYS_WORK *gsys)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
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

//--------------------------------------------------------------
/**
 * ノーマルジム初期化
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_INIT_Normal(GAMESYS_WORK *gsys)
{
  u8 i;
  GYM_NORMAL_SV_WORK *gmk_sv_work;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );
  }

  //PLAYERWORKから、初回位置座標を取得し、ギミック稼動後かどうかを判定する

  if (0){
    gmk_sv_work->GmkUnrock = TRUE;
  }else{
    gmk_sv_work->GmkUnrock = FALSE;
  }

  for(i=0;i<NRM_WALL_NUM_MAX;i++){
    gmk_sv_work->Wall[i] = 0;
  }
}

//--------------------------------------------------------------
/**
 * アンチジム初期化
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_INIT_Anti(GAMESYS_WORK *gsys)
{
  u8 i;
  GYM_ANTI_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }

  for (i=0;i<ANTI_SW_NUM_MAX;i++){
    gmk_sv_work->Sw[i] = 0;
  }
}

//--------------------------------------------------------------
/**
 * 飛行ジム初期化
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_INIT_Fly(GAMESYS_WORK *gsys)
{
  ;
}


