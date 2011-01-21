//======================================================================
/**
 * @file  gym_insect.c
 * @brief  虫ジム
 * @author Miyuki Iwasawa 
 * @date  11.01.18
 */
//======================================================================

#include "fieldmap.h"
#include "gym_insect_sv.h"
#include "gym_insect.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "map_attr.h"
#include "arc/fieldmap/gym_insect.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "system/ica_anime.h"
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c03gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_insect/gym_insect_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_insect_se_def.h"

#include "event_fldmmdl_control.h"  //for EVENT_ObjAnime

#define GYM_INSECT_UNIT_IDX (0)
#define GYM_INSECT_TMP_ASSIGN_ID  (1)

//フロア矩形
typedef struct FLOOR_RECT_tag
{
  u8 x;
  u8 z;
  u8 w;
  u8 h;
  u32 attr;
  fx32 height;
}FLOOR_RECT;

typedef struct POINT_tag{
  u8  x,y,z;
}POINT;

///繭ゲート座標
typedef struct GATE_PARAM_tag{
  u8  arc_id;
  u8  in_dir;
  u8  out_dir;
  POINT point;
}GATE_PARAM;

//ジム内部中の一時ワーク
typedef struct GYM_INSECT_TMP_tag
{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  GIMMICKWORK* gmkwork;
  GYM_INSECT_SV_WORK* gmk_sv;

  FLD_EXP_OBJ_CNT_PTR exp_obj;
  FIELD_PLAYER *f_player;
  MMDL* p_mmdl;
  FIELD_CAMERA *camera;

  HEAPID  heapID;
  HEAPID  heapLowID;


}GYM_INSECT_TMP;

typedef struct EVWORK_GATE_IN_tag{
  GYM_INSECT_TMP* git;
  u8  gate_id;
  const GATE_PARAM* gate_prm;

  ICA_ANIME* ica_anm;
  u16 ica_anm_frm_idx;
  u16 ica_anm_frm_max;
}EVWORK_GATE_IN;

#include "gym_insect.cdat"

///=======================================================
//プロトタイプ

///イベント
static GMEVENT* event_GateInCreate( GYM_INSECT_TMP* wk, u8 gate_id );

///サブルーチン
static void sub_SetExHeight( FIELDMAP_WORK* fieldWork );

#ifdef PM_DEBUG
static void deb_Move( GYM_INSECT_TMP* wk, FIELDMAP_WORK* fieldWork );
#endif  //PM_DEBUG

///=======================================================
//メイン関数

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_INSECT_TMP *wk;

  //汎用ワーク確保
  wk = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_INSECT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_INSECT_TMP));
 
  wk->gsys = FIELDMAP_GetGameSysWork( fieldWork );
  wk->gdata = GAMESYSTEM_GetGameData( wk->gsys );
  wk->gmkwork = GAMEDATA_GetGimmickWork( wk->gdata );
  wk->gmk_sv = GIMMICKWORK_Get( wk->gmkwork, FLD_GIMMICK_GYM_INSECT );

  wk->heapID = HEAPID_FIELDMAP;
  wk->heapLowID = GFL_HEAP_LOWID(HEAPID_FIELDMAP);
  wk->exp_obj = FIELDMAP_GetExpObjCntPtr( fieldWork );
  wk->f_player = FIELDMAP_GetFieldPlayer( fieldWork );
  wk->p_mmdl = FIELD_PLAYER_GetMMdl( wk->f_player );
  wk->camera = FIELDMAP_GetFieldCamera( fieldWork );

  //拡張高さセット
  sub_SetExHeight( fieldWork );
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_End(FIELDMAP_WORK *fieldWork)
{
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Move(FIELDMAP_WORK *fieldWork)
{
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

#ifdef PM_DEBUG
  deb_Move( wk, fieldWork );
#endif  //PM_DEBUG

}

//--------------------------------------------------------------
/**
 * 繭ゲートPushイベント作成 
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *GYM_INSECT_CreateGateInEvent(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  {
    int id;
    u8 dir;
    MMDL_GRIDPOS pos;

    MMDL_GetGridPos( wk->p_mmdl, &pos );
    dir = MMDL_GetDirDisp( wk->p_mmdl );
 
    for(id = 0;id < GATE_PARAM_NUM;id++){
      const GATE_PARAM* gp = &DATA_GateParam[id];
      
      if( gp->in_dir != dir ||
          gp->point.x != pos.gx ||
          gp->point.y != pos.gy ||
          gp->point.z != pos.gz){
        continue;
      }
      break;
    }
    if( id < GATE_PARAM_NUM ){
      //イベントコール
      return event_GateInCreate( wk, id );
    }
  }
  return NULL;  //ゲートが見つからない
}


//===============================================================
///ゲートインイベント
//===============================================================
static GMEVENT_RESULT event_GateInMain( GMEVENT * event, int *seq, void *work );

static void gatein_IcaAnmSet( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk );
static void gatein_IcaAnmFree( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk );
static BOOL gatein_IcaAnmWait( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk );

//--------------------------------------------------------------
//ゲートインイベント　生成
//--------------------------------------------------------------
GMEVENT* event_GateInCreate( GYM_INSECT_TMP* wk, u8 gate_id )
{
  GMEVENT* event = GMEVENT_Create( wk->gsys, NULL, event_GateInMain, sizeof(EVWORK_GATE_IN));
  EVWORK_GATE_IN* evwk = GMEVENT_GetEventWork( event );

  evwk->git = wk;
  evwk->gate_id = gate_id;
  evwk->gate_prm = &DATA_GateParam[gate_id];
  return event;
}

//--------------------------------------------------------------
//ゲートインイベント　メイン
//--------------------------------------------------------------
static GMEVENT_RESULT event_GateInMain( GMEVENT * event, int *seq, void *work )
{
  EVWORK_GATE_IN* evwk = (EVWORK_GATE_IN*)work;
  GYM_INSECT_TMP* wk = evwk->git;

  switch(*seq){
  case 0:
    //自機繭に入る
    SCRIPT_CallScript( event, SCRID_PROG_C03GYM0101_GATE_IN, NULL, NULL, wk->heapLowID );
    (*seq)++;
    break;
  case 1:
    //自機移動開始
    gatein_IcaAnmSet( wk, evwk );
    (*seq)++;
    break;
  case 2:
    if( gatein_IcaAnmWait( wk, evwk ) == FALSE){
      break;
    }
    gatein_IcaAnmFree( wk, evwk );

    //自機繭から出る
    {
      SCRIPT_WORK* sc = SCRIPT_CallScript( event, SCRID_PROG_C03GYM0101_GATE_OUT+(evwk->gate_id==GATE_ID_3D_4A), NULL, NULL, wk->heapLowID );
      SCRIPT_SetScriptWorkParam( sc, evwk->gate_id, evwk->gate_prm->out_dir, 0, 0 );
    }
    (*seq)++;
    break;
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

static void gatein_IcaAnmSet( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk )
{
  evwk->ica_anm = ICA_ANIME_CreateAlloc( wk->heapLowID, ARCID_GYM_INSECT, evwk->gate_prm->arc_id );
  GF_ASSERT( evwk->ica_anm != NULL);
  evwk->ica_anm_frm_max = ICA_ANIME_GetMaxFrame( evwk->ica_anm );
  evwk->ica_anm_frm_idx = 0;
}
static void gatein_IcaAnmFree( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk )
{
  ICA_ANIME_Delete( evwk->ica_anm );
}
static BOOL gatein_IcaAnmWait( GYM_INSECT_TMP* wk, EVWORK_GATE_IN* evwk )
{
  VecFx32 vec = {0,0,0};

  //座標取得
  if ( ICA_ANIME_GetTranslateAt( evwk->ica_anm, &vec, evwk->ica_anm_frm_idx ) ){
    //自機座標セット
    vec.x += FX32_CONST(16*16);
    vec.z += FX32_CONST(16*16);
    FIELD_PLAYER_SetPos( wk->f_player, &vec );
//    OS_Printf("%f,%f,%f\n", FX_FX32_TO_F32(vec.x), FX_FX32_TO_F32(vec.y), FX_FX32_TO_F32(vec.z));
  }
  if( ++evwk->ica_anm_frm_idx >= evwk->ica_anm_frm_max ){
    return TRUE;
  }
  return FALSE;
}

//===============================================================
///サブルーチン
//===============================================================

//-----------------------------------------------------
/*
 *  @brief  拡張高さデータセット
 */
//-----------------------------------------------------
static void sub_SetExHeight( FIELDMAP_WORK* fieldWork )
{
  int i;
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
  EHL_PTR exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );

  for(i = 0;i < FLOOR_RECT_NUM;i++){
    EXH_SetUpHeightData( i,
      DATA_FloorRect[i].x, DATA_FloorRect[i].z,
      DATA_FloorRect[i].w, DATA_FloorRect[i].h,
			DATA_FloorRect[i].height, DATA_FloorRect[i].attr, exHeightPtr );
  }
}



//===============================================================
///デバッグルーチン
//===============================================================
#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * 動作関数内デバッグ処理
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static void deb_Move( GYM_INSECT_TMP* wk, FIELDMAP_WORK* fieldWork )
{
  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L)
  {
    //自機の高さを変更
    {
      VecFx32 pos;
      FIELD_PLAYER_GetPos( wk->f_player, &pos );
      if (GFL_UI_KEY_GetCont() & PAD_BUTTON_START){
        if (GFL_UI_KEY_GetCont() & PAD_BUTTON_B){
          pos.y -=  10*FIELD_CONST_GRID_FX32_SIZE;
        }else{
          pos.y +=  10*FIELD_CONST_GRID_FX32_SIZE;
        }
      }else{
        if (GFL_UI_KEY_GetCont() & PAD_BUTTON_B){
          pos.y -=  FIELD_CONST_GRID_FX32_SIZE;
        }else{
          pos.y +=  FIELD_CONST_GRID_FX32_SIZE;
        }
      }
      FIELD_PLAYER_SetPos( wk->f_player, &pos );
    }
  }
}

#endif  //PM_DEBUG

