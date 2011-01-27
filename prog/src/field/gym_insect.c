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
#include "arc/fieldmap/zone_id.h"
#include "sound/pm_sndsys.h"

#include "system/main.h"    //for HEAPID_FIELDMAP
#include "system/ica_anime.h"

#include "script.h"     //for SCRIPT_CallScript
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"

#include "../../../resource/fldmapdata/script/c03gym0101_def.h"  //for SCRID_～
#include "../../../resource/fldmapdata/gimmick/gym_insect/gym_insect_local_def.h"

#include "gym_insect_se_def.h"

#include "event_fldmmdl_control.h"  //for EVENT_ObjAnime

#define GYM_INSECT_TMP_ASSIGN_ID  (1)

enum{
  OBJ_COCCON_1C,
  OBJ_COCCON_3B,
  OBJ_SPHERE,
  OBJ_COCCON_MAX = OBJ_COCCON_3B+1,
};

enum{
  MAP_NORMAL,
  MAP_LEADER,
};

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
  u8  reverse;
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

  u8  coccon_open[OBJ_COCCON_MAX];
  u8  map_id;

}GYM_INSECT_TMP;

typedef struct EVWORK_GATE_MOVE_tag{
  GYM_INSECT_TMP* git;
  const GATE_PARAM* gate_prm;

  ICA_ANIME* ica_anm;
  u16 ica_anm_frm_idx;
  u16 ica_anm_frm_max;
  s8  ica_anm_frm_dif;
  u8  gate_id;
  u8  se_player;
}EVWORK_GATE_MOVE;

#include "gym_insect.cdat"

///=======================================================
//プロトタイプ

///イベント
static GMEVENT* event_GateInCreate( GYM_INSECT_TMP* wk, u8 gate_id );
static GMEVENT* event_GateOutCreate( GYM_INSECT_TMP* wk, u8 gate_id );

///サブルーチン
static void sub_SetExHeight( FIELDMAP_WORK* fieldWork );
static void sub_ExpObjSet( GYM_INSECT_TMP* wk );
static void sub_ExpObjDell( GYM_INSECT_TMP* wk );
static void sub_ExpObjMain( GYM_INSECT_TMP* wk );

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

  wk->map_id = (FIELDMAP_GetZoneID( fieldWork ) == ZONE_ID_C03GYM0201);
  {
    int i;
    EVENTWORK* evwork = GAMEDATA_GetEventWork( wk->gdata );

    for(i = 0;i < OBJ_COCCON_MAX;i++ ){
      wk->coccon_open[i] = EVENTWORK_CheckEventFlag( evwork, FE_C03GYM0101_COCCON_01+i );
    }
  }

  //拡張高さセット
  sub_SetExHeight( fieldWork );
  sub_ExpObjSet( wk );
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

  sub_ExpObjDell( wk );
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

  sub_ExpObjMain( wk );

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
//--------------------------------------------------------------
/**
 * 繭ゲート 出口イベント作成 
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
GMEVENT *GYM_INSECT_CreateGateOutEvent(GAMESYS_WORK *gsys,u8 gate_id)
{
  GMEVENT * event;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //イベントコール
  return event_GateInCreate( wk, gate_id );
}

//===============================================================
///スクリプトコマンド
//===============================================================
//--------------------------------------------------------------
/**
 * @brief   繭オープンアニメコール 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
void GYM_INSECT_CocconOpen( FIELDMAP_WORK* fieldWork, u8 coccon_idx )
{
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  u16 idx = OBJ_COCCON_1C+coccon_idx;

  {
    int i;
    for(i = 0;i < OBJ_COCCON_ANM_MAX;i++){
      EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( wk->exp_obj, GYM_INSECT_UNIT_IDX, idx, i);
      FLD_EXP_OBJ_ChgAnmStopFlg( anm, FALSE ); //アニメ再生開始
    }
  }
}

//===============================================================
///ゲートイン・アウトイベント
//===============================================================
static GMEVENT_RESULT event_GateInMain( GMEVENT * event, int *seq, void *work );
static GMEVENT_RESULT event_GateOutMain( GMEVENT * event, int *seq, void *work );

static void gatemv_IcaAnmSet( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk );
static void gatemv_IcaAnmFree( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk );
static BOOL gatemv_IcaAnmWait( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk );
static s16 gatemv_SePitchCalc( EVWORK_GATE_MOVE* evwk );

//--------------------------------------------------------------
//ゲートインイベント　生成
//--------------------------------------------------------------
GMEVENT* event_GateInCreate( GYM_INSECT_TMP* wk, u8 gate_id )
{
  GMEVENT* event = GMEVENT_Create( wk->gsys, NULL, event_GateInMain, sizeof(EVWORK_GATE_MOVE));
  EVWORK_GATE_MOVE* evwk = GMEVENT_GetEventWork( event );

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
  EVWORK_GATE_MOVE* evwk = (EVWORK_GATE_MOVE*)work;
  GYM_INSECT_TMP* wk = evwk->git;

  switch(*seq){
  case 0:
    //自機繭に入る
    SCRIPT_CallScript( event, SCRID_PROG_C03GYM0101_GATE_IN, NULL, NULL, wk->heapLowID );
    (*seq)++;
    break;
  case 1:
    //自機移動開始
    gatemv_IcaAnmSet( wk, evwk );
    (*seq)++;
    break;
  case 2:
    if( gatemv_IcaAnmWait( wk, evwk ) == FALSE){
      break;
    }
    gatemv_IcaAnmFree( wk, evwk );

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

//--------------------------------------------------------------
//ゲートアウトイベント　生成
//--------------------------------------------------------------
GMEVENT* event_GateOutCreate( GYM_INSECT_TMP* wk, u8 gate_id )
{
  GMEVENT* event = GMEVENT_Create( wk->gsys, NULL, event_GateOutMain, sizeof(EVWORK_GATE_MOVE));
  EVWORK_GATE_MOVE* evwk = GMEVENT_GetEventWork( event );

  evwk->git = wk;
  evwk->gate_id = gate_id;
  evwk->gate_prm = &DATA_GateParam[gate_id];
  return event;
}

//--------------------------------------------------------------
//ゲートアウトイベント　メイン
//--------------------------------------------------------------
static GMEVENT_RESULT event_GateOutMain( GMEVENT * event, int *seq, void *work )
{
  EVWORK_GATE_MOVE* evwk = (EVWORK_GATE_MOVE*)work;
  GYM_INSECT_TMP* wk = evwk->git;

  switch(*seq){
  case 0:
    //自機移動開始
    gatemv_IcaAnmSet( wk, evwk );
    (*seq)++;
    break;
  case 1:
    if( gatemv_IcaAnmWait( wk, evwk ) == FALSE){
      break;
    }
    gatemv_IcaAnmFree( wk, evwk );
#if 0
    //自機繭から出る
    {
      SCRIPT_WORK* sc = SCRIPT_CallScript( event, SCRID_PROG_C03GYM0101_GATE_OUT+(evwk->gate_id==GATE_ID_3D_4A), NULL, NULL, wk->heapLowID );
      SCRIPT_SetScriptWorkParam( sc, evwk->gate_id, evwk->gate_prm->out_dir, 0, 0 );
    }
#endif
    (*seq)++;
    break;
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

static s16 gatemv_SePitchCalc( EVWORK_GATE_MOVE* evwk )
{
  s16 pitch = 0;
  u16 next_frm;
  fx32 len;
  VecFx32 v1,v2,v3;

  next_frm = evwk->ica_anm_frm_idx+evwk->ica_anm_frm_dif;
  if( next_frm < 0){
    next_frm = 0;
  }else if( next_frm > (evwk->ica_anm_frm_max-1) ){
    next_frm = evwk->ica_anm_frm_max-1;
  }
  OS_Printf("now_frm = %d, next=%d\n",evwk->ica_anm_frm_idx, next_frm );
  ICA_ANIME_GetTranslateAt( evwk->ica_anm, &v1, evwk->ica_anm_frm_idx );
  ICA_ANIME_GetTranslateAt( evwk->ica_anm, &v2, next_frm  );
  OS_Printf("now(%f,%f,%f), next(%f,%f,%f)\n",
      FX_FX32_TO_F32(v1.x),
      FX_FX32_TO_F32(v1.y),
      FX_FX32_TO_F32(v1.z),
      FX_FX32_TO_F32(v2.x),
      FX_FX32_TO_F32(v2.y),
      FX_FX32_TO_F32(v2.z)
      );

  VEC_Subtract( &v2, &v1, &v3);
  len = VEC_Mag(&v3);

  {
    fx32 step = len-FX32_CONST(5);
    OS_Printf("vec_len=%f, step=%f ",FX_FX32_TO_F32(len),FX_FX32_TO_F32(step));
    pitch = FX_Whole(FX_Mul(step,FX32_CONST(100)));
    OS_Printf("pitch=%d\n\n",pitch);
  }
  return pitch;
}

static void gatemv_IcaAnmSet( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk )
{
  evwk->ica_anm = ICA_ANIME_CreateAlloc( wk->heapLowID, ARCID_GYM_INSECT, evwk->gate_prm->arc_id );
  GF_ASSERT( evwk->ica_anm != NULL);
  evwk->ica_anm_frm_max = ICA_ANIME_GetMaxFrame( evwk->ica_anm );
  if( evwk->gate_prm->reverse ){
    evwk->ica_anm_frm_idx = evwk->ica_anm_frm_max-1;
    evwk->ica_anm_frm_dif = -1;
  }else{
    evwk->ica_anm_frm_idx = 0;
    evwk->ica_anm_frm_dif = 1;
  }
  evwk->se_player = PMSND_GetSE_DefaultPlayerID( GYM_INSECT_SE_DRIVE );
  PMSND_PlaySE(GYM_INSECT_SE_DRIVE);
  PMSND_SetStatusSE_byPlayerID( evwk->se_player, PMSND_NOEFFECT, gatemv_SePitchCalc(evwk), PMSND_NOEFFECT);
  
  FLD_EXP_OBJ_SetVanish( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE, FALSE );
}
static void gatemv_IcaAnmFree( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk )
{
  PMSND_StopSE();
  FLD_EXP_OBJ_SetVanish( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE, TRUE );
  ICA_ANIME_Delete( evwk->ica_anm );
}
static BOOL gatemv_IcaAnmWait( GYM_INSECT_TMP* wk, EVWORK_GATE_MOVE* evwk )
{
  VecFx32 vec = {0,0,0};

  //座標取得
  if ( ICA_ANIME_GetTranslateAt( evwk->ica_anm, &vec, evwk->ica_anm_frm_idx ) ){
    //自機座標セット
    vec.x += FX32_CONST(16*16);
    vec.z += FX32_CONST(16*16);
    FIELD_PLAYER_SetPos( wk->f_player, &vec );
//    OS_Printf("%f,%f,%f\n", FX_FX32_TO_F32(vec.x), FX_FX32_TO_F32(vec.y), FX_FX32_TO_F32(vec.z));
    PMSND_SetStatusSE_byPlayerID( evwk->se_player, PMSND_NOEFFECT, gatemv_SePitchCalc(evwk), PMSND_NOEFFECT);
 
    {
      VecFx32 rot = {0,0,0};
      VecFx32 scale = {FX32_ONE,FX32_ONE,FX32_ONE};
      GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE );

      status->trans = vec;
      if( ICA_ANIME_GetRotateAt( evwk->ica_anm, &rot, evwk->ica_anm_frm_idx )){
        OS_Printf("rot = %f,%f,%f\n", FX_FX32_TO_F32(rot.x), FX_FX32_TO_F32(rot.y), FX_FX32_TO_F32(rot.z));
        GFL_CALC3D_MTX_CreateRot( FX_DEG_TO_IDX(rot.x), FX_DEG_TO_IDX(rot.y), FX_DEG_TO_IDX(rot.z), &status->rotate );
      }
      if( ICA_ANIME_GetScaleAt( evwk->ica_anm, &scale, evwk->ica_anm_frm_idx )){
        status->scale = scale; 
      }
    }
  }
  evwk->ica_anm_frm_idx += evwk->ica_anm_frm_dif;
  if( evwk->ica_anm_frm_dif < 0 ){
    if( evwk->ica_anm_frm_idx <= 0 ){
      return TRUE;
    }
  }else{
    if( evwk->ica_anm_frm_idx >= evwk->ica_anm_frm_max ){
      return TRUE;
    }
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

//-----------------------------------------------------
/*
 *  @brief  拡張配置モデルデータセット
 */
//-----------------------------------------------------
static void sub_ExpObjSet( GYM_INSECT_TMP* wk )
{
  int i,j;
  GFL_G3D_OBJSTATUS* status;

  if( wk->map_id != MAP_NORMAL ){
    return;
  }
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( wk->exp_obj, &g3Dutil_Setup, GYM_INSECT_UNIT_IDX);

  {
    status = FLD_EXP_OBJ_GetUnitObjStatus( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE);
    //カリングする
    FLD_EXP_OBJ_SetCulling( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE, TRUE);
    FLD_EXP_OBJ_SetVanish( wk->exp_obj, GYM_INSECT_UNIT_IDX, OBJ_SPHERE, TRUE );
  }
  for(i = 0;i < OBJ_COCCON_MAX;i++){
    u8 idx = OBJ_COCCON_1C+i;
    status = FLD_EXP_OBJ_GetUnitObjStatus( wk->exp_obj, GYM_INSECT_UNIT_IDX, idx);

    //カリングする
    FLD_EXP_OBJ_SetCulling( wk->exp_obj, GYM_INSECT_UNIT_IDX, idx, TRUE);

    //座標セット
    status->trans.x = FX32_CONST( DATA_ObjCocconPosTbl[i].x*16 )+FX32_CONST(8);
    status->trans.y = FX32_CONST( DATA_ObjCocconPosTbl[i].y*16 );
    status->trans.z = FX32_CONST( DATA_ObjCocconPosTbl[i].z*16 )+FX32_CONST(8);
    
    for(j = 0;j < OBJ_COCCON_ANM_MAX;j++){
      EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( wk->exp_obj, GYM_INSECT_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ValidCntAnm( wk->exp_obj,GYM_INSECT_UNIT_IDX,idx,j,TRUE);

      FLD_EXP_OBJ_ChgAnmLoopFlg( anm, FALSE );  //1回再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( anm, TRUE ); //アニメ止めとく

      //フラグで再生状態を変更
      FLD_EXP_OBJ_SetObjAnmFrm( wk->exp_obj, GYM_INSECT_UNIT_IDX, idx, j,
        FLD_EXP_OBJ_GetAnimeLastFrame( anm )*wk->coccon_open[i] );
    }
  }
}
static void sub_ExpObjDell( GYM_INSECT_TMP* wk )
{
  if( wk->map_id != MAP_NORMAL ){
    return;
  }
  FLD_EXP_OBJ_DelUnit( wk->exp_obj, GYM_INSECT_UNIT_IDX );
}
static void sub_ExpObjMain( GYM_INSECT_TMP* wk )
{
  if( wk->map_id != MAP_NORMAL ){
    return;
  }
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( wk->exp_obj );
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

