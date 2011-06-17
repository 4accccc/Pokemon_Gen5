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
#include "gym_insect_sv.h"
#include "gym_dragon_sv.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "../../../resource/fldmapdata/gimmick/gym_normal/gym_normal_exit_def.h"
#include "eventwork_def.h"  // LOCALWORK*

//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param   gsys            ゲームシステムポインタ
 * @param   inEvtFlgBtl1    イベントフラグ状況　トレーナー戦1を終えたか？　1で終了
 *
 * @return  none
 */
//--------------------------------------------------------------
void GYM_INIT_Elec(GAMESYS_WORK *gsys, u16 seq)
{
  GYM_ELEC_SV_WORK *gmk_sv_work;

  GF_ASSERT_MSG( seq < GYM_ELEC_EVENT_SEQ_MAX, "初期化シーケンスの値が不正ですseq=%d\n",seq );

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }
  //初期シーケンスを設定
  gmk_sv_work->event_seq  = seq;
}

//--------------------------------------------------------------
/**
 * ノーマルジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @param     ギミック発動後かどうか　TRUE：発動後
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Normal(GAMESYS_WORK *gsys, const BOOL inOpened)
{
  u8 i;
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  PLAYER_WORK *player_work;

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );
    player_work = GAMEDATA_GetMyPlayerWork(gamedata);
  }
  //gmk_sv_workへの初期代入等を行う
}

//--------------------------------------------------------------
/**
 * アンチジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @param     inCurtain   カーテン状況
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Anti(GAMESYS_WORK *gsys, const int inCurtain)
{
  u8 i;
  GYM_ANTI_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }

  //gmk_sv_workへの初期代入等を行う
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
  //gmk_sv_workへの初期代入等を行う
}

//--------------------------------------------------------------
/**
 * 虫ジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @param     sw_work 格納ワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Insect(GAMESYS_WORK *gsys, u16 **sw_work)
{
  int i,j;
  GYM_INSECT_SV_WORK *gmk_sv_work;
  u16 *ex_wk;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  //gmk_sv_workへの初期代入等を行う
}

//--------------------------------------------------------------
/**
 * ドラゴンジム初期化
 * @param	    gsys    ゲームシステムポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_INIT_Dragon(GAMESYS_WORK *gsys)
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );

  // レイヤーとローケーションを設定
  gmk_sv_work->now_floor_layer  = 0;
  gmk_sv_work->now_location     = GYM_LOCATION_CENTER;
}


