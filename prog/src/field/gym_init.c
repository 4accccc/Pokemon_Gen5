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

#define INSECT_CHK_MAX  (2)

//--虫ジムギミックデータ--
//スイッチに対応するポールインデックス
static const s8 InsSw[INSECT_SW_MAX][INSECT_CHK_MAX] = {
  {0,-1},     //スイッチ0：0番ポール
  {1,-1},     //スイッチ1：1番ポール
  {2,-1},     //スイッチ2：2番ポール
  {3,4},      //スイッチ3：3,4番ポール
  {5,-1},     //スイッチ4：5番ポール
  {6,-1},     //スイッチ5：6番ポール
  {7,8},      //スイッチ6：7,8番ポール
  {9,-1},     //スイッチ7：9番ポール
};

//壁に対応するポールインデックス
static const u8 InsWall[INSECT_WALL_MAX-1][INSECT_CHK_MAX] = {
  {0,1},     //壁0：0,1番ポール
  {2,3},     //壁1：2,3番ポール
  {4,5},     //壁2：4,5番ポール
  {6,6},      //壁3：6番ポール
  {7,7},     //壁4：7番ポール
  {8,9},     //壁5：8,9番ポール
  //※壁6はポールチェック無し
};

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
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ELEC );
  }

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

  if (inOpened) gmk_sv_work->GmkUnrock = TRUE;
  else gmk_sv_work->GmkUnrock = FALSE;

  //PLAYER_WORKから、初回位置座標を取得し、ギミック稼動後かどうかを判定する @todo
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

  for (i=0;i<ANTI_DOOR_NUM_MAX;i++){
    gmk_sv_work->Door[i] = 0;
  }

  if ( inCurtain & 0x01 ) gmk_sv_work->Door[0] = 1;
  if ( (inCurtain>>1) & 0x01 ) gmk_sv_work->Door[1] = 1;
  if ( (inCurtain>>2) & 0x01 ) gmk_sv_work->Door[2] = 1;
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
    ex_wk = GIMMICKWORK_GetExWork( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  //ギミックワーク拡張部をみて、ワークを書き換える
  for (i=0;i<INSECT_SW_MAX;i++)
  {
    if ( ex_wk[i] )
    {
      //スイッチを押下応対にする
      gmk_sv_work->Sw[i] = TRUE;
      *sw_work[i] = 1;
      //スイッチに対応するポールの状態を変化させる
      for(j=0;j<INSECT_CHK_MAX;j++)
      {
        s8 idx = InsSw[i][j];
        if ( idx != -1) gmk_sv_work->Pl[idx] = TRUE;
      }
    }
  }
  //壁の状態をポール常態からセット
  for (i=0;i<INSECT_WALL_MAX-1;i++)
  {
    for (j=0;j<INSECT_CHK_MAX;j++)
    {
      u8 idx = InsWall[i][j];
      if ( !gmk_sv_work->Pl[idx] ) break;
    }
    if (j==INSECT_CHK_MAX) gmk_sv_work->WallSt[i] = WALL_ST_WEAKNESS;
  }
  //インデックス6番の壁は始めから壊せる
  gmk_sv_work->WallSt[6] = WALL_ST_WEAKNESS;

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
  DRAGON_WORK *wk;
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

  wk = &gmk_sv_work->DraWk[0];
  wk->HeadDir = HEAD_DIR_RIGHT;
  wk->ArmDir[0] = ARM_DIR_UP;   //左
  wk->ArmDir[1] = ARM_DIR_DOWN;   //右
  wk = &gmk_sv_work->DraWk[1];
  wk->HeadDir = HEAD_DIR_DOWN;
  wk->ArmDir[0] = ARM_DIR_DOWN;   //左
  wk->ArmDir[1] = ARM_DIR_DOWN;   //右
  wk = &gmk_sv_work->DraWk[2];
  wk->HeadDir = HEAD_DIR_LEFT;
  wk->ArmDir[0] = ARM_DIR_DOWN;     //左
  wk->ArmDir[1] = ARM_DIR_UP;     //右
}


