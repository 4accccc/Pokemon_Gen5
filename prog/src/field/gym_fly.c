//======================================================================
/**
 * @file  gym_fly.c
 * @brief  飛行ジム
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "gym_fly_sv.h"
#include "gym_fly.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

//#include "arc/fieldmap/gym_fly.naix"

#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
//#include "../../../resource/fldmapdata/script/c04gym0101_def.h"  //for SCRID_～



#define GYM_FLY_UNIT_IDX (0)
#define GYM_FLY_TMP_ASSIGN_ID  (1)

#define FRAME_POS_SIZE  (4*3*360)   //座標ＸＹＺ各4バイトｘ360フレーム
#define HEADER_SIZE  (8)   //フレームサイズ4バイト+格納情報3バイトのヘッダー情報があるためアライメントを加味して8バイト確保する

//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  u8 ShotIdx;
  u8 EvtDir;
  u8 dummy[2];

  u8 FramePosDat[FRAME_POS_SIZE+HEADER_SIZE];

}GYM_FLY_TMP;

static GMEVENT_RESULT ShotEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_FLY_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_FLY_TMP));
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_FLY_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_FLY_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	
 * @return
 */
//--------------------------------------------------------------
void GYM_FLY_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );

}

//--------------------------------------------------------------
/**
 * 打ち出しイベント起動
 * @param	
 * @return
 */
//--------------------------------------------------------------
GMEVENT *GYM_FLY_Shot(GAMESYS_WORK *gsys, const u8 inShotIdx)
{
  GMEVENT * event;
  GYM_FLY_SV_WORK *gmk_sv_work;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_FLY_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );

  gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_FLY );

  //イベントコール
  event = GMEVENT_Create( gsys, NULL, ShotEvt, 0 );
    
  //打ち出しインデックスをセット
  tmp->ShotIdx = inShotIdx;

}

//--------------------------------------------------------------
/**
 * スイッチイベント
 * @param	
 * @return
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ShotEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_FLY_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_FLY_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = SaveData_GetGimmickWork( GAMEDATA_GetSaveControlWork( gamedata ) );
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_FLY );
  }

  switch(*seq){
  case 0:
    //移動データロード
    ;
    //自機の向きにより分岐
    //大砲ドアオープンフレームまで再生開始
    ;
    (*seq)++;
    break;
  case 1:
    //オープンフレーム到達監視
    if (1){
      //自機、大砲に入る
      (*seq)++;
    }
    break;
  case 2:
    //自機アニメ終了待ち
    if(1){
      //大砲アニメ際開始
      (*seq)++;
    }
    break;
  case 3:
    //発射フレーム到達チェック
    if (1){
      //フレーム読み取り開始
      ;
      (*seq)++;
    }
    break;
  case 4:
    //大砲アニメ終了チェック＆フレーム読み取り終了チェック
    if (1){
      //フレーム読み取り終了
      ;
      //主人公最終位置セット
      ;
      (*seq)++;
    }
    break;
  case 5:
    //フレーム読み取り解放
    ;
    return GMEVENT_RES_FINISH;
  }

  //フレーム読み取り処理部分
  if (0){      //読み取りフラグが立っていれば、実行
    ;
  }

  return GMEVENT_RES_CONTINUE;
}


