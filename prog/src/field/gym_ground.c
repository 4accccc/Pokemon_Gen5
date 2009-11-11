//======================================================================
/**
 * @file  gym_ground.c
 * @brief  地面ジム
 * @author  Saito
 * @date  09.11.11
 */
//======================================================================

#include "fieldmap.h"
#include "gym_ground_sv.h"
#include "gym_ground.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

///#include "arc/fieldmap/gym_ground.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
///#include "script.h"     //for SCRIPT_CallScript
///#include "../../../resource/fldmapdata/script/c03gym0101_def.h"  //for SCRID_～

///#include "../../../resource/fldmapdata/gimmick/gym_ground/gym_ground_local_def.cdat"

//#include "sound/pm_sndsys.h"

//#include "gym_ground_se_def.h"

#include "height_ex.h"    //for ExHeight

#define GYM_GROUND_UNIT_IDX (0)
#define GYM_GROUND_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define FLOOR_RECT_NUM  (7)

#define LIFT_MOVE_SPD (FIELD_CONST_GRID_SIZE*FX32_ONE)

//ジム内部中の一時ワーク
typedef struct GYM_GROUND_TMP_tag
{
  int TargetLiftIdx;
  fx32 NowHeight;
  fx32 DstHeight;
  fx32 AddVal;
}GYM_GROUND_TMP;

typedef struct FLOOR_RECT_tag
{
  int X;
  int Z;
  int W;
  int H;
  fx32 Height;
}FLOOR_RECT;

typedef struct LIFT_RECT_tag
{
  int X;
  int Z;
  int W;
  int H;
  fx32 Height[2];
}LIFT_RECT;


#define F1_HEIGHT ( 40*FIELD_CONST_GRID_FX32_SIZE )
#define F2_HEIGHT ( 30*FIELD_CONST_GRID_FX32_SIZE )
#define F3_HEIGHT ( 20*FIELD_CONST_GRID_FX32_SIZE )
#define F4_HEIGHT (  0*FIELD_CONST_GRID_FX32_SIZE )

#define F1_1_X  (7)
#define F1_1_Z  (19)
#define F1_1_W  (17)
#define F1_1_H  (1)
#define F1_2_X  (7)
#define F1_2_Z  (23)
#define F1_2_W  (17)
#define F1_2_H  (1)

#define F2_1_X  (7)
#define F2_1_Z  (19)
#define F2_1_W  (17)
#define F2_1_H  (1)
#define F2_2_X  (7)
#define F2_2_Z  (20)
#define F2_2_W  (1)
#define F2_2_H  (3)
#define F2_3_X  (7)
#define F2_3_Z  (23)
#define F2_3_W  (17)
#define F2_3_H  (1)

#define F3_1_X  (7)
#define F3_1_Z  (19)
#define F3_1_W  (6)
#define F3_1_H  (1)
#define F3_2_X  (7)
#define F3_2_Z  (23)
#define F3_2_W  (6)
#define F3_2_H  (1)

#define LIFT_WIDTH (5)
#define LIFT_HEIGHT (3)
#define SP_LIFT_WIDTH (3)
#define SP_LIFT_HEIGHT (3)

#define LIFT_1_X  (18)
#define LIFT_1_Z  (24)

#define LIFT_2_X  (18)
#define LIFT_2_Z  (20)

#define LIFT_3_X  (8)
#define LIFT_3_Z  (24)

#define LIFT_4_X  (13)
#define LIFT_4_Z  (24)

#define LIFT_5_X  (8)
#define LIFT_5_Z  (20)

#define LIFT_6_X  (14)
#define LIFT_6_Z  (20)

static const FLOOR_RECT FloorRect[FLOOR_RECT_NUM] = {
  {F1_1_X, F1_1_Z, F1_1_W, F1_1_H, F1_HEIGHT},
  {F1_2_X, F1_2_Z, F1_2_W, F1_2_H, F1_HEIGHT},

  {F2_1_X, F2_1_Z, F2_1_W, F2_1_H, F2_HEIGHT},
  {F2_2_X, F2_2_Z, F2_2_W, F2_2_H, F2_HEIGHT},
  {F2_3_X, F2_3_Z, F2_3_W, F2_3_H, F2_HEIGHT},

  {F3_1_X, F3_1_Z, F3_1_W, F3_1_H, F3_HEIGHT},
  {F3_2_X, F3_2_Z, F3_2_W, F3_2_H, F3_HEIGHT},
};

static const LIFT_RECT LiftRect[LIFT_NUM_MAX] = {
  {LIFT_1_X, LIFT_1_Z, LIFT_WIDTH, LIFT_HEIGHT, { F1_HEIGHT, F2_HEIGHT} },
  {LIFT_2_X, LIFT_2_Z, LIFT_WIDTH, LIFT_HEIGHT, { F2_HEIGHT, F1_HEIGHT} },
  {LIFT_3_X, LIFT_3_Z, LIFT_WIDTH, LIFT_HEIGHT, { F2_HEIGHT, F3_HEIGHT} },
  {LIFT_4_X, LIFT_4_Z, LIFT_WIDTH, LIFT_HEIGHT, { F3_HEIGHT, F2_HEIGHT} },
  {LIFT_5_X, LIFT_5_Z, LIFT_WIDTH, LIFT_HEIGHT, { F3_HEIGHT, F1_HEIGHT} },
  {LIFT_6_X, LIFT_6_Z, SP_LIFT_WIDTH, SP_LIFT_HEIGHT, { F1_HEIGHT, F4_HEIGHT} },
};


static GMEVENT_RESULT UpDownEvt( GMEVENT* event, int* seq, void* work );
static BOOL CheckArrive(GYM_GROUND_TMP *tmp);
static BOOL CheckLiftHit(u8 inLiftIdx, const int inX, const int inZ);

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  EHL_PTR exHeightPtr;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GYM_GROUND_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_GROUND_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_GROUND_TMP));

  {
    FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
    exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );
  }

  //床高さセット
  for(i=0;i<FLOOR_RECT_NUM;i++)
  {
    EXH_SetUpHeightData( i,
              FloorRect[i].X, FloorRect[i].Z, FloorRect[i].W, FloorRect[i].H,
							FloorRect[i].Height,
							exHeightPtr );
  }

  //リフト高さセット
  for(i=0;i<LIFT_NUM_MAX;i++)
  {
    u8 height_idx = FLOOR_RECT_NUM+i;
    u8 idx;
    //セーブデータで分岐
    if (0) idx = 1;
    else idx = 0;
      
    EXH_SetUpHeightData( height_idx,
              LiftRect[i].X, LiftRect[i].Z, LiftRect[i].W, LiftRect[i].H,
							LiftRect[i].Height[idx],
							exHeightPtr );
  }


  //必要なリソースの用意
///  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_GROUND_UNIT_IDX );

#if 0
  //座標セット
  for (i=0;i<GROUND_LIFT_MAX;i++)
  {
    int j;
    int idx = OBJ_SW1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_GROUND_UNIT_IDX, idx);
    status->trans = SwPos[i];
    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_GROUND_UNIT_IDX, idx, TRUE);

    for (j=0;j<LIFT_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_INSECT_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      if ( gmk_sv_work->Sw[i] ){
        fx32 last = FLD_EXP_OBJ_GetAnimeLastFrame(anm);
        //ラストフレーム
        FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_INSECT_UNIT_IDX, idx, j, last );
        //アニメ停止
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
      }
    }
  }
#endif  
  

  {
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("after_size = %x\n",size);
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  //ＯＢＪ解放
///  FLD_EXP_OBJ_DelUnit( ptr, GYM_GROUND_UNIT_IDX );
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Move(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  //フィールドの残りヒープをプリント
  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
    int size = GFI_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP);
    OS_Printf("size = %x\n",size);
  }
  
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
      GMEVENT *event;
      {
        u8 i;
        s16 gx,gz;
        const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( fieldWork ) );
        gx = MMDL_GetGridPosX( fmmdl );
        gz = MMDL_GetGridPosZ( fmmdl );
        for (i=0;i<LIFT_NUM_MAX;i++){
          if ( CheckLiftHit(i, gx, gz) ) break;
        }
        if (i != LIFT_NUM_MAX)
        {
          event = GYM_GROUND_CreateUpDownEvt(gsys, i);
          GAMESYSTEM_SetEvent(gsys, event);
        }
      }
    }
  }
  else if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L)
  {
    //自機の高さを変更
    {
      VecFx32 pos;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      pos.y -=  10*FIELD_CONST_GRID_FX32_SIZE;
      FIELD_PLAYER_SetPos( fld_player, &pos );
    }
  }

  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}

//--------------------------------------------------------------
/**
 * 昇降イベント作成
 * @param	      gsys        ゲームシステムポインタ
 * @param       inLiftIdx   リフトインデックス
 * @return      GMEVENT     イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *GYM_GROUND_CreateUpDownEvt(GAMESYS_WORK *gsys, const int inLiftIdx)
{
  GMEVENT * event;
  GYM_GROUND_TMP *tmp;
  GYM_GROUND_SV_WORK *gmk_sv_work;
  {
    FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
    tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  }

  tmp->TargetLiftIdx = inLiftIdx;
  //対象リフトの状態を取得
  if ( gmk_sv_work->LiftMoved[ inLiftIdx ] )
  {
    //移動後位置→基準位置
    tmp->NowHeight = LiftRect[inLiftIdx].Height[1];
    tmp->DstHeight = LiftRect[inLiftIdx].Height[0];
  }
  else
  {
    //基準位置→移動後位置
    tmp->NowHeight = LiftRect[inLiftIdx].Height[0];
    tmp->DstHeight = LiftRect[inLiftIdx].Height[1];
  }
  if ( tmp->DstHeight - tmp->NowHeight < 0 ) tmp->AddVal = -LIFT_MOVE_SPD;
  else tmp->AddVal = LIFT_MOVE_SPD;

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, UpDownEvt, 0 );

  return event;
}

//--------------------------------------------------------------
/**
 * 昇降イベント
 * @param	  event   イベントポインタ
 * @param   seq     シーケンサ
 * @param   work    ワークポインタ
 * @return  GMEVENT_RESULT  イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT UpDownEvt( GMEVENT* event, int* seq, void* work )
{
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_GROUND_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_GROUND );
  }

  switch(*seq){
  case 0:
    //カメラトレース終了リクエスト
    ;
    (*seq)++;
    break;
  case 1:
    if ( 1 )
    {
      //トレース終了
      (*seq)++;
    }
    //NO BREAK
  case 2:
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
    ;
    //フィールドＯＢＪ全検索
    {
      u32 no = 0;
      MMDL *mmdl;
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
      while( MMDLSYS_SearchUseMMdl(mmdlsys, &mmdl, &no) == TRUE )
      {
        //グリッド座標を取得
        s16 x = MMDL_GetGridPosX( mmdl );
        s16 z = MMDL_GetGridPosZ( mmdl );
        //内外判定
        if ( CheckLiftHit(tmp->TargetLiftIdx, x, z) )
        {
          const VecFx32 *mmdl_pos;
          VecFx32 pos;
          int gy;
          //座標更新
          mmdl_pos = MMDL_GetVectorPosAddress( mmdl );
          pos = *mmdl_pos;
          pos.y = tmp->NowHeight;
          gy = SIZE_GRID_FX32( pos.y );
          MMDL_SetGridPosY( mmdl, gy );
          MMDL_SetVectorPos( mmdl, &pos );
        }
      }
    }
    break;
  case 3:
    {
      u8 idx;
      //対象リフトのギミックデータ書き換え
      if ( gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] ){
        gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] = FALSE;
        idx = 0;
      }else{
        gmk_sv_work->LiftMoved[ tmp->TargetLiftIdx ] = TRUE;
        idx = 1;
      }
      //対象リフトの拡張高さを書き換え
      {
        u8 height_idx;
        EHL_PTR exHeightPtr;
        {
          FLDMAPPER * mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
          exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );
        }
        height_idx = FLOOR_RECT_NUM + tmp->TargetLiftIdx;
        EXH_SetHeight( height_idx,
            					 LiftRect[tmp->TargetLiftIdx].Height[idx],
                       exHeightPtr );
      }
    }
    //カメラトレース再開
    ;
    //OBJのポーズを解除
    ;
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
static BOOL CheckArrive(GYM_GROUND_TMP *tmp)
{
  if (tmp->AddVal < 0){
    if ( tmp->DstHeight >= tmp->NowHeight ) return TRUE;
  }else{
    if ( tmp->DstHeight <= tmp->NowHeight ) return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * リフト矩形内チェック
 * @param	  inLiftIdx リフトインデックス
 * @param   inX       対象Ｘグリッド座標
 * @param   inZ       対象Zグリッド座標
 * @return  BOOL      TRUEで矩形内
 */
//--------------------------------------------------------------
static BOOL CheckLiftHit(u8 inLiftIdx, const int inX, const int inZ)
{
  int x = LiftRect[inLiftIdx].X;
  int z = LiftRect[inLiftIdx].Z;
  int width = LiftRect[inLiftIdx].W;
  int height = LiftRect[inLiftIdx].H;

  if ( (x<=inX)&&(inX<=x+width-1)&&
       (z<=inZ)&&(inZ<=z+height-1) ){
    return TRUE;
  }
  return FALSE;
}
