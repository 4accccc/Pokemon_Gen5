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
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "../../../resource/fldmapdata/gimmick/gym_normal/gym_normal_exit_def.cdat"

//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param   gsys            ゲームシステムポインタ
 * @param   inEvtFlgBtl1    イベントフラグ状況　トレーナー戦1を終えたか？　1で終了
 * @param   inEvtFlgBtl2    イベントフラグ状況　トレーナー戦2を終えたか？　1で終了
 *
 * @return  none
 */
//--------------------------------------------------------------
void GYM_INIT_Elec(GAMESYS_WORK *gsys, const u16 inEvtFlgBtl1, const u16 inEvtFlgBtl2)
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

  //トレーナー戦進捗状況をセット
  if (inEvtFlgBtl1){
    gmk_sv_work->EvtFlg[0] = 1;
  }
  if (inEvtFlgBtl2){
    gmk_sv_work->EvtFlg[1] = 1;
  }
}

//--------------------------------------------------------------
/**
 * ノーマルジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Normal(GAMESYS_WORK *gsys)
{
  u8 i;
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  PLAYER_WORK *player_work;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );
    player_work = GAMEDATA_GetMyPlayerWork(gamedata);
  }

  //PLAYER_WORKから、初回位置座標を取得し、ギミック稼動後かどうかを判定する
  {
    int gx,gz;
    gx = player_work->position.x / FIELD_CONST_GRID_FX32_SIZE;
    gz = player_work->position.z / FIELD_CONST_GRID_FX32_SIZE;

    OS_Printf("NormalGymInit:: pos gx gz  = %d,%d\n",gx,gz);
  
    if ((gx == HIDEN_EXIT_GX)&&(gz == HIDEN_EXIT_GZ)){
      gmk_sv_work->GmkUnrock = TRUE;
    }else{
      gmk_sv_work->GmkUnrock = FALSE;
    }
  }

  for(i=0;i<NRM_WALL_NUM_MAX;i++){
    if (gmk_sv_work->GmkUnrock){
      gmk_sv_work->Wall[i] = 1;
    }else{
      gmk_sv_work->Wall[i] = 0;
    }
  }
}

//--------------------------------------------------------------
/**
 * アンチジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @return    none
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

  for (i=0;i<ANTI_DOOR_NUM_MAX;i++){
    gmk_sv_work->Door[i] = 0;
  }
  gmk_sv_work->PushSwIdx = ANTI_SW_NUM_MAX;
}

//--------------------------------------------------------------
/**
 * 飛行ジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Fly(GAMESYS_WORK *gsys)
{
  ;
}


