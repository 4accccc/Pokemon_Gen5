//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_dragon.c
 *	@brief  ドラゴンジム
 *	@author	Masayuki Onoue
 *	@date		2011.06.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fieldmap.h"
#include "gym_dragon_sv.h"
#include "gym_dragon.h"
#include "gym_dragon_local.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_dragon.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript

#include "sound/pm_sndsys.h"
#include "gym_dragon_se_def.h"

#include "rail_line/c08gym0101.h" //for RAIL
#include "system/ica_anime.h"

#include "gym_dragon.cdat"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // SYS_FLAG_C08GYM_BUTTON_～

#include "eventwork_def.h"  // LOCALWORK*

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_DRAGON_UNIT_IDX (0)
#define GYM_DRAGON_TMP_ASSIGN_ID  (1)


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================


//-------------------------------------
///	モデルワーク
//=====================================
typedef struct
{
  s32                 unitID;
  MODELID             modelID;
  EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
} MODEL;

//-------------------------------------
///	ジム内ワーク
//=====================================
typedef struct GYM_DRAGON_TMP_tag
{
  // sys
  HEAPID              heapID;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fld_wk;  

  // data
  GYM_DRAGON_SV_WORK  *p_sv_work;
  MODEL       model[MODELID_MAX]; // モデル
  
}GYM_DRAGON_TMP;

//-------------------------------------
///	プレイヤー移動ワーク
//=====================================
typedef struct PLAYER_MOVE_WORK_tag
{
  // sys
  HEAPID            heapLowID;
  GYM_DRAGON_TMP    *p_gmkwk;
  FIELDMAP_WORK     *p_fld_wk;
  EVENTWORK         *p_event_wk;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
 
  // data
  s32           ica_frame;
  s32           ica_frame_max;
  FIELD_PLAYER  *p_player;
  MMDL          *p_plmmdl;
  u16           event_seq;
  const BUTTON_EVENT_DATA *p_button_evdata;
  GYM_DRAGON_SV_WORK  *p_sv_work;
  
  // res
  ICA_ANIME   *p_ica_anm;
}PLAYER_MOVE_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
typedef BOOL (*UPLAYERCECHK)(EVENTWORK*);

static BOOL CheckCanUpLayer( EVENTWORK* p_event_wk,FLOOR_LAYER layer_id );
static BOOL CheckUpLayerEntrance(EVENTWORK* p_wk);
static BOOL CheckUpLayerTail(EVENTWORK* p_wk);
static BOOL CheckUpLayerFoot(EVENTWORK* p_wk);
static BOOL CheckUpLayerArm(EVENTWORK* p_wk);
static BOOL CheckUpLayerHead(EVENTWORK* p_wk);

static GYM_BUTTON_SEQ GetNowButtonType( EVENTWORK* p_event_wk ,u16 layer ,u16 loc );
static GMEVENT * CreateButtonEvent_UDLR( GAMESYS_WORK *p_gamesys, u16 event_seq );
static GMEVENT_RESULT PlayerMove_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static GMEVENT_RESULT ButtonActive_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static void MOVE_EVENT_Init( PLAYER_MOVE_WORK *p_wk );
static BOOL MOVE_Event_UpdatePlayerPos( PLAYER_MOVE_WORK *p_wk );
static void MOVE_EVENT_UpdateDragonGymData( PLAYER_MOVE_WORK *p_wk );
static void UpdateButtonStatus( GYM_BUTTON_SEQ button_seq ,EVENTWORK* p_event_wk );
static void SetWorkValue( EVENTWORK* p_event_wk ,u16 work_no ,u16 value );

//-------------------------------------
///	上レイヤーへ行けるか判定関数テーブル
//=====================================
UPLAYERCECHK sc_uplayer_check[] = {
  CheckUpLayerEntrance,
  CheckUpLayerTail,
  CheckUpLayerFoot,
  CheckUpLayerArm,
  CheckUpLayerHead,
};

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_DRAGON_TMP *p_wk;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *p_gmkwork = GAMEDATA_GetGimmickWork(gamedata);

  //汎用ワーク確保
  p_wk = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_DRAGON_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_DRAGON_TMP));
  
  p_wk->heapID    = HEAPID_FIELDMAP;
  p_wk->p_fld_wk  = fieldWork;
  p_wk->p_sv_work = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_DRAGON );
  p_wk->p_ex_obj = FIELDMAP_GetExpObjCntPtr( fieldWork );
 
  // 3D拡張オブジェクトユニットの追加
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup, GYM_DRAGON_UNIT_IDX );
  
  //モデル初期化
  {
    int i;
    for( i = 0; i < MODELID_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];
      p_model->unitID   = GYM_DRAGON_UNIT_IDX;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, FALSE );
    }
  }

  
  {
    int i;
    EXP_OBJ_ANM_CNT_PTR exobj_anm;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < 2 ; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          GYM_DRAGON_UNIT_IDX,
          p_model->modelID );

      // 位置を設定
      p_status->trans.x = FX32_CONST(456+100*i);
      p_status->trans.y = FX32_CONST(0);
      p_status->trans.z = FX32_CONST(648);


      // 見える状態に
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
          GYM_DRAGON_UNIT_IDX, p_model->modelID, TRUE );

      // フレームの初期化
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, 0, TRUE);

      // アニメーション取得
      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, 0 );

      // アニメーション再生
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, 0 , 0);
      FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, TRUE );
    }
  }
}
//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_End(FIELDMAP_WORK *fieldWork)
{
  GYM_DRAGON_TMP *p_wk = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  // 破棄
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, GYM_DRAGON_UNIT_IDX );
  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_DRAGON_Move(FIELDMAP_WORK *fieldWork)
{
  GYM_DRAGON_SV_WORK *gmk_sv_work;
  GYM_DRAGON_TMP *p_wk = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_DRAGON );
  }

 
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );

//    {
//      MtxFx43 trans;
//      MODEL *p_model    = &p_wk->model[0];
//      GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, 0);
//      NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
//      GFL_G3D_OBJSTATUS *p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
//          GYM_DRAGON_UNIT_IDX,
//          p_model->modelID );
//
//      // ジョイントIDを取得する。検索で見つからない場合は負の値が返る。
//      int jntID = NNS_G3dGetNodeIdxByName( NNS_G3dGetNodeInfo( renderobj->resMdl ),(NNSG3dResName*)"bind_null_hero" );
//
//      BOOL res = NNS_G3dGetResultMtx( renderobj, &trans, NULL, (u32)jntID );
//      if( res == TRUE )
//      {
//        OS_Printf("結果：○\n");
//        OS_Printf("jntID=%d\n",jntID);
//        OS_Printf("%d,%d,%d\n",FX_Whole(trans._00),FX_Whole(trans._01),FX_Whole(trans._02));
//        OS_Printf("%d,%d,%d\n",FX_Whole(trans._10),FX_Whole(trans._11),FX_Whole(trans._12));
//        OS_Printf("%d,%d,%d\n",FX_Whole(trans._20),FX_Whole(trans._21),FX_Whole(trans._22));
//        OS_Printf("%d,%d,%d\n",FX_Whole(trans._30),FX_Whole(trans._31),FX_Whole(trans._32));
//      }
//      else
//      {
//        OS_Printf("結果：×\n");
//      }
//      {
//        VecFx32 vec;
//        VecFx32 cameraPos;
//        FIELD_CAMERA* fieldCamera;
//        MtxFx44 projection =* NNS_G3dGlbGetProjectionMtx(); 
//        fieldCamera = FIELDMAP_GetFieldCamera(p_wk->p_fld_wk); 
//        FIELD_CAMERA_GetCameraPos( fieldCamera, &cameraPos );
//        vec.x = p_status->trans.x + trans._30 + cameraPos.x;
//        vec.y = p_status->trans.y + trans._31 + cameraPos.y;
//        vec.z = p_status->trans.z + trans._32 + cameraPos.z;
//        //FIELD_PLAYER_SetPos( FIELDMAP_GetFieldPlayer( p_wk->p_fld_wk ), &vec );
//        OS_Printf("PL座標-%d,%d,%d\n",FX_Whole(vec.x),FX_Whole(vec.y),FX_Whole(vec.z));
//        OS_Printf("CAM座標-%d,%d,%d\n",FX_Whole(cameraPos.x),FX_Whole(cameraPos.y),FX_Whole(cameraPos.z));
//      }
//  }

  
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の状態に対応するボタン状態を調べる
 *
 *	@param	p_event_wk 
 *	@param	layer 
 *	@param	loc 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GYM_BUTTON_SEQ GetNowButtonType( EVENTWORK* p_event_wk ,u16 layer ,u16 loc )
{
  BOOL  can_up        = CheckCanUpLayer( p_event_wk, layer);
  GYM_BUTTON_SEQ ret  = GYM_BUTTON_NONE;

  ret = sc_button_seq_table[layer][loc][can_up];
  GF_ASSERT_MSG( GYM_BUTTON_NONE != ret ,"押すことの出来るボタンがありません" );
  return ret;
}

//============================================================================
// 上レイヤーチェック関数郡
//============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  レイヤーごとの上に行ける判定
 *
 *	@param	p_wk      イベントワーク
 *	@param	layer_id  レイヤーID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckCanUpLayer( EVENTWORK* p_event_wk,FLOOR_LAYER layer_id )
{
  return sc_uplayer_check[layer_id]( p_event_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief 上のレイヤーに上がれるかチェックする(ENTRANCE)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerEntrance(EVENTWORK* p_wk)
{
  // 常に上がれる
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief 上のレイヤーに上がれるかチェックする(TAIL)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerTail(EVENTWORK* p_wk)
{
  // 常に上がれる
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief 上のレイヤーに上がれるかチェックする(FOOT)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerFoot(EVENTWORK* p_wk)
{
  // 常に上がれる
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief 上のレイヤーに上がれるかチェックする(ARM)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerArm(EVENTWORK* p_wk)
{
  // 常に上がれる
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief 上のレイヤーに上がれるかチェックする(HEAD)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerHead(EVENTWORK* p_wk)
{
  // 常に上がれる
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークアドレスの内容を設定する
 *
 *	@param	p_event_wk 
 *	@param	work_no 
 *	@param	value 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetWorkValue( EVENTWORK* p_event_wk ,u16 work_no ,u16 value )
{
  u16* wk_adrs = EVENTWORK_GetEventWorkAdrs( p_event_wk ,work_no );
  (*wk_adrs) = value;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン状況をボタンシーケンスにあわせて設定する
 *
 *	@param	button_seq 
 *	@param	p_event_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UpdateButtonStatus( GYM_BUTTON_SEQ button_seq ,EVENTWORK* p_event_wk )
{
  // ボタン状況のリセット
  SetWorkValue(p_event_wk ,SC_BUTTON_WK_U,FALSE);
  SetWorkValue(p_event_wk ,SC_BUTTON_WK_D,FALSE);
  SetWorkValue(p_event_wk ,SC_BUTTON_WK_L,FALSE);
  SetWorkValue(p_event_wk ,SC_BUTTON_WK_R,FALSE);

  // ボタン状況の更新
  if( button_seq == GYM_BUTTON_UDLR  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_U,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_D,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_L,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_R,TRUE);
  }
  else if( button_seq == GYM_BUTTON_DLR  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_D,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_L,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_R,TRUE);
  }
  else if( button_seq == GYM_BUTTON_UD  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_U,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_D,TRUE);
  }
  else if( button_seq == GYM_BUTTON_LR  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_L,TRUE);
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_R,TRUE);
  }
  else if( button_seq == GYM_BUTTON_L  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_L,TRUE);
  }
  else if( button_seq == GYM_BUTTON_R  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_R,TRUE);
  }
  else if( button_seq == GYM_BUTTON_D  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_D,TRUE);
  }
  else if( button_seq == GYM_BUTTON_U  )
  {
    SetWorkValue(p_event_wk ,SC_BUTTON_WK_U,TRUE);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジム内ボタンイベント
 *
 *	@param	*p_gamesys
 *	@param	event_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_DRAGON_CreateButtonEvent( GAMESYS_WORK *p_gamesys, u16 event_seq )
{
  //ボタンイベント開始
  switch( event_seq )
  {
  case GYM_DRAGON_EVENT_SEQ_UP_BUTTON:
  case GYM_DRAGON_EVENT_SEQ_DOWN_BUTTON:
  case GYM_DRAGON_EVENT_SEQ_LEFT_BUTTON:
  case GYM_DRAGON_EVENT_SEQ_RIGHT_BUTTON:
    return CreateButtonEvent_UDLR( p_gamesys, event_seq );
  }

  GF_ASSERT( 0 );
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  上下左右共通ボタンイベント
 *
 *	@param	*p_gamesys
 *	@param	event_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT * CreateButtonEvent_UDLR( GAMESYS_WORK *p_gamesys, u16 event_seq )
{
  FIELDMAP_WORK   *p_fld_wk = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_DRAGON_TMP  *p_gmkwk = GMK_TMP_WK_GetWork( p_fld_wk, GYM_DRAGON_TMP_ASSIGN_ID);

  // イベント作成
  GMEVENT *p_event = GMEVENT_Create( p_gamesys, NULL, PlayerMove_Event, sizeof(PLAYER_MOVE_WORK) );
  PLAYER_MOVE_WORK *p_wk = GMEVENT_GetEventWork(p_event);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  GIMMICKWORK *p_gmkwork = GAMEDATA_GetGimmickWork(p_gamedata);

  // パラメータ設定
  {
    // ボタンデータの取得
    FLOOR_LAYER   now_layer = p_gmkwk->p_sv_work->now_floor_layer;
    GYM_LOCATION  now_loc   = p_gmkwk->p_sv_work->now_location;
    p_wk->p_button_evdata   = &sc_button_event_data[now_layer][now_loc][event_seq];
    p_wk->event_seq         = event_seq;
    // ギミックワークの取得
    p_wk->p_gmkwk           = p_gmkwk;
    // フィールドワークを取得
    p_wk->p_fld_wk          = p_fld_wk;
    // ドラゴンジムのセーブデータを取得
    p_wk->p_sv_work         = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_DRAGON );
  
    // イベントワークの初期化
    p_wk->p_event_wk    = GAMEDATA_GetEventWork( p_gamedata );

  }
  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤー移動イベント
 *
 *	@param	p_event
 *	@param	p_seq
 *	@param	p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT PlayerMove_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  PLAYER_MOVE_WORK *p_wk = p_wk_adrs;
  switch( (*p_seq) )
  {
  case MOVE_EVENT_SEQ_INIT:
    {
      MOVE_EVENT_Init( p_wk );
      (*p_seq) = MOVE_EVENT_SEQ_MOVE;
    }
    break;
  case MOVE_EVENT_SEQ_MOVE:
    {
//      BOOL is_end = MOVE_Event_UpdatePlayerPos( p_wk );
//      if( is_end == TRUE )
      {
        (*p_seq) = MOVE_EVENT_SEQ_EXIT;
      }
    }
    break;
  case MOVE_EVENT_SEQ_EXIT:
    {
      MOVE_EVENT_UpdateDragonGymData( p_wk );
      ICA_ANIME_Delete( p_wk->p_ica_anm );
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤー移動イベント初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_EVENT_Init( PLAYER_MOVE_WORK *p_wk )
{
  p_wk->ica_frame = 0;
  p_wk->heapLowID     = GFL_HEAP_LOWID(HEAPID_FIELDMAP);
  p_wk->p_ica_anm     = ICA_ANIME_CreateAlloc( p_wk->heapLowID, ARCID_GYM_DRAGON,NARC_gym_dragon_hero_dra_move02_bin );
  p_wk->ica_frame_max = ICA_ANIME_GetMaxFrame( p_wk->p_ica_anm );
  p_wk->p_player      = FIELDMAP_GetFieldPlayer( p_wk->p_fld_wk );
  p_wk->p_plmmdl      = FIELD_PLAYER_GetMMdl( p_wk->p_player );
  p_wk->p_ex_obj      = p_wk->p_gmkwk->p_ex_obj;
  
  // アニメーション再生
  {
    EXP_OBJ_ANM_CNT_PTR exobj_anm;
    MODEL *p_model    = &p_wk->p_gmkwk->model[0];
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, 0 );
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, 0 , 0);
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーの位置を更新
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL MOVE_Event_UpdatePlayerPos( PLAYER_MOVE_WORK *p_wk )
{
//  if(  p_wk->ica_frame_max <= p_wk->ica_frame ) return TRUE; // 終了後は実行しない
//
//  {
//    MtxFx43 trans;
//    MODEL *p_model    = &p_wk->p_gmkwk->model[0];
//    GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, 0);
//    NNSG3dRenderObj* renderobj = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd(g3Dobj) );
//
//    // ジョイントIDを取得する。検索で見つからない場合は負の値が返る。
//    int jntID = NNS_G3dGetNodeIdxByName( NNS_G3dGetNodeInfo( renderobj->resMdl ),(NNSG3dResName*)"bind_null_hero" );
//
//    // レンダリングオブジェクトのjntIDノードに対応する行列をカレント行列に持ってくる
//    // ２番目の引数にMtxFx43へのポインタを指定すると位置座標行列が、
//    // ３番目の引数にMtxFx33へのポインタを指定すると方向ベクトル行列が得られる。
//    NNS_G3dGetResultMtx( renderobj, &trans, NULL, (u32)jntID );
//
//    OS_Printf("jntID=%d\n",jntID);
//    OS_Printf("座標%d,%d,%d\n",FX_Whole(trans._30),FX_Whole(trans._31),FX_Whole(trans._32));
//    {
//      VecFx32 vec;
//      vec.x = trans._30;
//      vec.y = trans._31;
//      vec.z = trans._32;
//      FIELD_PLAYER_SetPos( p_wk->p_player, &vec );
//    }
//  }
//  return TRUE;

  // 移動処理
  {
    VecFx32 vec = {0,0,0};
    if ( ICA_ANIME_GetTranslateAt( p_wk->p_ica_anm, &vec, p_wk->ica_frame ) )
    {
      MtxFx43 trans;
      MODEL *p_model    = &p_wk->p_gmkwk->model[0];
      GFL_G3D_OBJSTATUS *p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          GYM_DRAGON_UNIT_IDX,
          p_model->modelID );

      vec.x += p_status->trans.x;
      vec.y += p_status->trans.y;
      vec.z += p_status->trans.z;
      FIELD_PLAYER_SetPos( p_wk->p_player, &vec );
      p_wk->ica_frame++;
      if(  p_wk->ica_frame_max <= p_wk->ica_frame )
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動後のジムデータ更新
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_EVENT_UpdateDragonGymData( PLAYER_MOVE_WORK *p_wk )
{
  GYM_BUTTON_SEQ button_seq;

  // レイヤーとロケーションの更新
  if( p_wk->event_seq == GYM_DRAGON_EVENT_SEQ_UP_BUTTON )
  {
    p_wk->p_sv_work->now_floor_layer++;
  }
  else if( p_wk->event_seq == GYM_DRAGON_EVENT_SEQ_DOWN_BUTTON )
  {
    p_wk->p_sv_work->now_floor_layer--;
  }
  else if( p_wk->event_seq == GYM_DRAGON_EVENT_SEQ_LEFT_BUTTON )
  {
    if( p_wk->p_sv_work->now_location == GYM_LOCATION_CENTER )
    {
      p_wk->p_sv_work->now_location = GYM_LOCATION_LEFT;
    }
    else
    {
      p_wk->p_sv_work->now_location = GYM_LOCATION_CENTER;
    }
  }
  else if( p_wk->event_seq == GYM_DRAGON_EVENT_SEQ_RIGHT_BUTTON )
  {
    if( p_wk->p_sv_work->now_location == GYM_LOCATION_CENTER )
    {
      p_wk->p_sv_work->now_location = GYM_LOCATION_RIGHT;
    }
    else
    {
      p_wk->p_sv_work->now_location = GYM_LOCATION_CENTER;
    }
  }

  OS_Printf("現在のレイヤー：%u\n",p_wk->p_sv_work->now_floor_layer);
  OS_Printf("現在のロケーション：%u\n",p_wk->p_sv_work->now_location);

  // 表示するべきボタン状況を取得し、その状況に合わせてフラグを操作
  {
    FLOOR_LAYER   now_layer = p_wk->p_sv_work->now_floor_layer;
    GYM_LOCATION  now_loc   = p_wk->p_sv_work->now_location;
    button_seq = GetNowButtonType( p_wk->p_event_wk, now_layer, now_loc );
    UpdateButtonStatus( button_seq ,p_wk->p_event_wk);
  }
}

