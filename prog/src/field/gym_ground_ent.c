//======================================================================
/**
 * @file  gym_ground_ent.c
 * @brief  地面ジムエントランス
 * @author  Saito
 * @date  09.11.11
 */
//======================================================================

#include "fieldmap.h"
//#include "gym_ground_sv.h"
#include "gym_ground_ent.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_ground.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP

//#include "sound/pm_sndsys.h"

//#include "gym_ground_se_def.h"

#define GYM_GROUND_ENT_UNIT_IDX (0)
#define GYM_GROUND_ENT_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define LIFT_ANM_NUM    (2)

#define LIFT_0_GX (7*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define LIFT_0_GY (0*FIELD_CONST_GRID_FX32_SIZE)
#define LIFT_0_GZ (4*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define LIFT_MOVE_SPD (2*FX32_ONE)

#define BASE_HEIGHT ( 0*FIELD_CONST_GRID_FX32_SIZE )
#define DOWN_HEIGHT ( -5*FIELD_CONST_GRID_FX32_SIZE )



//ジム内部中の一時ワーク
typedef struct GYM_GROUND_ENT_TMP_tag
{
  fx32 NowHeight;
  fx32 DstHeight;
  fx32 AddVal;
  const VecFx32 *Watch;
  BOOL Exit;
}GYM_GROUND_ENT_TMP;

//リソースの並び順番
enum {
  RES_ID_LIFT_MDL = 0,
  RES_ID_LIFT_ANM_A,
  RES_ID_LIFT_ANM_B,
};

//ＯＢＪインデックス
enum {
  OBJ_LIFT_0 = 0,
};

//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_GYM_GROUND, NARC_gym_ground_rif_anim01_nsbmd, GFL_G3D_UTIL_RESARC }, //IMD

  { ARCID_GYM_GROUND, NARC_gym_ground_rif_anim01_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
  { ARCID_GYM_GROUND, NARC_gym_ground_rif_anim02_nsbta, GFL_G3D_UTIL_RESARC }, //ITA
};

//3Dアニメ　リフト
static const GFL_G3D_UTIL_ANM g3Dutil_anmTbl_lift[] = {
	{ RES_ID_LIFT_ANM_A,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ID_LIFT_ANM_B,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};

static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //リフト
	{
		RES_ID_LIFT_MDL, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		RES_ID_LIFT_MDL, 	//テクスチャリソースID
		g3Dutil_anmTbl_lift,			//アニメテーブル(複数指定のため)
		NELEMS(g3Dutil_anmTbl_lift),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

static GMEVENT_RESULT ExitLiftEvt( GMEVENT* event, int* seq, void* work );
static BOOL CheckArrive(GYM_GROUND_ENT_TMP *tmp);

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_GROUND_ENT_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_GROUND_ENT_TMP));

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_GROUND_ENT_UNIT_IDX );

  //座標セット
  {
    int j;
    EXP_OBJ_ANM_CNT_PTR anm;
    VecFx32 pos = {LIFT_0_GX, LIFT_0_GY, LIFT_0_GZ};
    int obj_idx = OBJ_LIFT_0;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx);
    status->trans = pos;
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx, TRUE);
    for (j=0;j<LIFT_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
    
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx, 1, TRUE);
    FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx, 0, FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_ENT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_GROUND_ENT_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
//  GYM_GROUND_ENT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);

  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GMEVENT *event;
      {
        event = GYM_GROUND_ENT_CreateExitLiftMoveEvt(gsys, TRUE);
        GAMESYSTEM_SetEvent(gsys, event);
      }
    }else if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GMEVENT *event;
      {
        event = GYM_GROUND_ENT_CreateExitLiftMoveEvt(gsys, FALSE);
        GAMESYSTEM_SetEvent(gsys, event);
      }
    }
  }

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * 出口昇降イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inExit      エントランスを出る（ジムに行く）とき TRUE
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_GROUND_ENT_CreateExitLiftMoveEvt(GAMESYS_WORK *gsys, const BOOL inExit)
{
  GMEVENT * event;
  GYM_GROUND_ENT_TMP *tmp;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    
  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);

  tmp->Exit = inExit;

  //対象リフトの状態を取得
  if ( inExit )
  {
    //基準位置→移動後位置
    tmp->NowHeight = BASE_HEIGHT;
    tmp->DstHeight = DOWN_HEIGHT;
  }
  else
  {
    //移動後位置→基準位置
    tmp->NowHeight = DOWN_HEIGHT;
    tmp->DstHeight = BASE_HEIGHT;
    
  }
  if ( tmp->DstHeight - tmp->NowHeight < 0 ) tmp->AddVal = -LIFT_MOVE_SPD;
  else tmp->AddVal = LIFT_MOVE_SPD;
  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ExitLiftEvt, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * 出口昇降イベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ExitLiftEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_ENT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( fieldWork );

  switch(*seq){
  case 0:
    {
      //現在ウォッチターゲットを保存
      tmp->Watch = FIELD_CAMERA_GetWatchTarget(camera);
      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(camera);
    }
    (*seq)++;
    break;
  case 1:
    {
      //変動後の高さに書き換え
      tmp->NowHeight += tmp->AddVal;
      //目的の高さに到達したか？
      if(  CheckArrive(tmp) )
      {
        //目的高さで上書き
        tmp->NowHeight = tmp->DstHeight;
        //次のシーケンスへ
        (*seq)++;
      }
      //対象リフト高さ更新
      {
        int obj_idx = OBJ_LIFT_0;
        GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_ENT_UNIT_IDX, obj_idx);
        status->trans.y = tmp->NowHeight;
      }
      //自機の高さを更新
      {
        VecFx32 pos;
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
        FIELD_PLAYER_GetPos( fld_player, &pos );
        pos.y = tmp->NowHeight;
        FIELD_PLAYER_SetPos( fld_player, &pos );
      }
    }
    break;
  case 2:
    //カメラを再バインド
    if ( (!tmp->Exit)&&(tmp->Watch != NULL) ){
      FIELD_CAMERA_BindTarget(camera, tmp->Watch);
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * リフト到達チェック
 * @param	  GYM_GROUND_TMP *tmp
 * @return  BOOL      TRUEで到達
 */
//--------------------------------------------------------------
static BOOL CheckArrive(GYM_GROUND_ENT_TMP *tmp)
{
  if (tmp->AddVal < 0){
    if ( tmp->DstHeight >= tmp->NowHeight ) return TRUE;
  }else{
    if ( tmp->DstHeight <= tmp->NowHeight ) return TRUE;
  }

  return FALSE;
}


