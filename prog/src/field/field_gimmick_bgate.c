//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_bgate.c
 *	@brief  バッジチェックゲート
 *	@author	Masayuki Onoue
 *	@date		2011.07.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fieldmap.h"
#include "fldmmdl.h"
#include "savedata/gimmickwork.h"
#include "field_gimmick_bgate_sv.h"
#include "gmk_tmp_wk.h"
#include "field_g3d_mapper.h"
#include "field_gimmick_bgate.h"
#include "field_task_fade.h"

#include "../../../resource/fldmapdata/gimmick/badge_gate/bage_gate_def.h"    // 外部指定パラメータ
#include "../../../resource/fldmapdata/script/usescript.h"  //DISP_FADE_SPEED

#include "eventwork_def.h"                    // for WK_SCENE_D26R0101_GATE_*
#include "system/main.h"                      // for HEAPID_FIELDMAP
#include "field_gimmick_def.h"                // for FLD_GIMMICK_D26R0101
#include "arc/fieldmap/badge_gate_gmk.naix"   // モデルデータアーカイブ
#include "field/field_const.h"                // for GRID_TO_FX32
#include "field_gimmick_bgate.cdat"           // モデルセットアップデータ
#include "sound/pm_sndsys.h"                  // for SEQ_SE_*


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// モデルの登録インデックス
#define MMANAGER_FEXPOBJ_EV_UID (0)
#define MMANAGER_FEXPOBJ_OFF_UID (1)
#define MMANAGER_FEXPOBJ_ON_UID (2)
#define MMANAGER_FEXPOBJ_GATE_UID (3)

// 像の数
#define MONUMENT_CNT  (8)

// イベントが起動済みの時にワークに設定されている値
#define IS_ACTION_WK_VALUE  (2)

//-------------------------------------
///	SEシーケンス定義
//=====================================
typedef enum{
  BADGEGATE_SE_SEQ_MONUMENT_START   = SEQ_SE_SW_BCG_01,
  BADGEGATE_SE_SEQ_MONUMENT_LASER   = SEQ_SE_SW_BCG_06,
  BADGEGATE_SE_SEQ_GATE_START       = SEQ_SE_SW_BCG_03,
  BADGEGATE_SE_SEQ_GATE_LIGHT       = SEQ_SE_SW_BCG_02,
  BADGEGATE_SE_SEQ_GATE_COLUMN_DOWN1 = SEQ_SE_SW_BCG_04,
  BADGEGATE_SE_SEQ_GATE_COLUMN_DOWN2 = SEQ_SE_SW_BCG_07,
  BADGEGATE_SE_SEQ_GATE_OPEN        = SEQ_SE_SW_BCG_05,
}GYM_DRAGON_SE_SEQ;

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
  u32                 modelID;
  EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
} MODEL;

//-------------------------------------
///	ギミックワーク
//=====================================
typedef struct {
  // sys
  HEAPID              heapID;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fld_wk;  
  EVENTWORK           *p_event_wk;

  // data
  MODEL model[BADGEGATE_MDLID_MAX];       // モデル
  MODEL model_off[BADGEGATE_MDLID_OFF_MAX];   // モデル
  MODEL model_on[BADGEGATE_MDLID_ON_MAX];    // モデル
  u32                 seq;

}BADGEGATE_WK;

//-------------------------------------
///	モニュメント起動ワーク
//=====================================
typedef struct MONUMENT_START_WK_tag
{
  // sys
  BADGEGATE_WK *p_gmk_wk; 

  // data
  u16 monument_id;
  u32 event_frame;
  MODEL ev_model;
}MONUMENT_START_WK;

//-------------------------------------
///	振動パラメータ
//=====================================
typedef struct
{
  u32 start_frame;
  u32 end_frame;
  u32 loop_sync;
  u32 w;
  u32 h;
} SHAKE_PARAM;

// 門の演出時に再生するカメラ揺れテーブル
static const SHAKE_PARAM sc_shake_param_table[] =
{
  { SHAKE_START_FRAME_0, SHAKE_END_FRAME_0 , SHAKE_LOOP_SYNC_0, SHAKE_W_0, SHAKE_H_0,},
  { SHAKE_START_FRAME_1, SHAKE_END_FRAME_1 , SHAKE_LOOP_SYNC_1, SHAKE_W_1, SHAKE_H_1,},
};
#define SHAKE_PARAM_CNT  (NELEMS(sc_shake_param_table))

//-------------------------------------
///	ゲート起動ワーク
//=====================================
typedef struct GATE_START_WK_tag
{
  // sys
  BADGEGATE_WK  *p_gmk_wk; 
  FIELD_CAMERA  *p_cam;
  MODEL         ev_model;
  u32           shake_event_frame;
  u16           is_shake_end[SHAKE_PARAM_CNT];
  VecFx32       default_cam_ofs;
  VecFx32       default_target_ofs;
  FIELDMAP_WORK *p_fieldmap;
}GATE_START_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void setup_model_data( BADGEGATE_WK *p_wk );
static GMEVENT_RESULT MonumentStart_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static void SetAnmStopFlag( MODEL *p_model  ,u16 anmid ,BOOL is_stop );
static MODEL* GetModelData( BADGEGATE_WK *p_gmk_wk ,u32 model_id ,u32 unit_id );
static GMEVENT_RESULT GateOpen_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static void GateOpen_CameraShakeMain( GATE_START_WK *p_wk );
static fx32 GetSynchroAnmFrame( BADGEGATE_WK *p_wk );
static BOOL get_is_action_end( u32 id, EVENTWORK *p_event_wk );
static void GateOpen_SePlayMain( GATE_START_WK *p_wk );
static void MonumentStart_SePlayMain( MONUMENT_START_WK *p_wk );
static void DeleteEventModelData( FLD_EXP_OBJ_CNT_PTR p_ex_obj );
static void MakeEventGateModelData( MODEL *p_model, FLD_EXP_OBJ_CNT_PTR p_ex_obj );

//----------------------------------------------------------------------------
/**
 *	@brief  バッジチェックゲートギミック初期化
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BADGEGATE_Setup( FIELDMAP_WORK *p_fieldmap )
{
  MMDLSYS               *p_mmdlsys;
  BADGEGATE_WK          *p_wk = NULL;
  SAVE_CONTROL_WORK     *p_save_data;
  const HEAPID  heapID = FIELDMAP_GetHeapID( p_fieldmap );
  GAMESYS_WORK  *p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gsys );
  GIMMICKWORK   *p_gmkwork = GAMEDATA_GetGimmickWork(p_gamedata);

  //汎用ワーク確保
  p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, BADGEGATE_WORK_ASSIGN_ID, 
      heapID, sizeof(BADGEGATE_WK));
  
  // パラメータの初期化
  p_wk->heapID          = HEAPID_FIELDMAP;
  p_wk->p_fld_wk        = p_fieldmap;
  p_wk->p_ex_obj        = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_event_wk      = GAMEDATA_GetEventWork( p_gamedata );
  p_wk->seq             = 0;
    
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup, MMANAGER_FEXPOBJ_EV_UID );
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup_off, MMANAGER_FEXPOBJ_OFF_UID );
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup_on, MMANAGER_FEXPOBJ_ON_UID );
  setup_model_data( p_wk );
 
}
 
//----------------------------------------------------------------------------
/**
 *	@brief  モデルデータのセットアップ
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void setup_model_data( BADGEGATE_WK *p_wk )
{
  //モデル初期化
  {
    int i;
    for( i = 0; i < BADGEGATE_MDLID_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];
      p_model->unitID   = MMANAGER_FEXPOBJ_EV_UID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
    }
    for( i = 0; i < BADGEGATE_MDLID_OFF_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model_off[i];
      p_model->unitID   = MMANAGER_FEXPOBJ_OFF_UID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
    }
    for( i = 0; i < BADGEGATE_MDLID_ON_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model_on[i];
      p_model->unitID   = MMANAGER_FEXPOBJ_ON_UID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
    }
  }

  // モデルステータス設定(イベント)
  {
    int i;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < BADGEGATE_MDLID_MAX ; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exobj_anm;
      MODEL *p_model    = &p_wk->model[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          MMANAGER_FEXPOBJ_EV_UID,
          p_model->modelID );

      // 位置を設定
      p_status->trans = d3obj_pos_table[i];

      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
          MMANAGER_FEXPOBJ_EV_UID, p_model->modelID, TRUE );

      //カリングする
      FLD_EXP_OBJ_SetCulling( p_model->p_ex_obj, MMANAGER_FEXPOBJ_EV_UID, p_model->modelID, TRUE);
      
      // アニメーションの初期化
      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA );
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA, TRUE);
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA ,0);
      FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,FALSE );
      SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, TRUE );

      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA );
      SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ITA, TRUE );
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA, TRUE);
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA ,0);
      FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,FALSE );
    }
  }

  // モデルステータス設定(OFF)
  {
    int i;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < BADGEGATE_MDLID_OFF_MAX ; i++ )
    {
      MODEL *p_model    = &p_wk->model_off[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          MMANAGER_FEXPOBJ_OFF_UID,
          p_model->modelID );

      // 位置を設定
      p_status->trans = d3obj_pos_table[i];

      if( get_is_action_end( i, p_wk->p_event_wk ) )
      {
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            p_model->unitID, p_model->modelID, TRUE );
      }
      else
      {
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            p_model->unitID, p_model->modelID, FALSE );
      }
      
      //カリングする
      FLD_EXP_OBJ_SetCulling( p_model->p_ex_obj, MMANAGER_FEXPOBJ_OFF_UID, p_model->modelID, TRUE);
    }
  }

  // モデルステータス設定(ON)
  {
    int i;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < BADGEGATE_MDLID_ON_MAX ; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exobj_anm;
      MODEL *p_model    = &p_wk->model_on[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          p_model->unitID,
          p_model->modelID );

      // 位置を設定
      p_status->trans = d3obj_pos_table[i];
      
      // アニメーションを有効に
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_LOOP_ITA, TRUE);

      if( get_is_action_end( i, p_wk->p_event_wk ) )
      {
        SetAnmStopFlag( p_model ,MDL_ANM_LOOP_ITA, FALSE );
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            p_model->unitID, p_model->modelID, FALSE );
      }
      else
      {
        SetAnmStopFlag( p_model ,MDL_ANM_LOOP_ITA, TRUE );
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            p_model->unitID, p_model->modelID, TRUE );
      }
      
      //カリングする
      FLD_EXP_OBJ_SetCulling( p_model->p_ex_obj, p_model->unitID, p_model->modelID, TRUE);
      // フレームの初期化
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_LOOP_ITA ,0);
      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_LOOP_ITA );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,TRUE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  モニュメントが起動済みかを調べる
 *
 *	@param	id
 *	@param	*p_event_wk 
 *
 *	@retval TRUEの時にモニュメントは起動済み
 */
//-----------------------------------------------------------------------------
static BOOL get_is_action_end( u32 id, EVENTWORK *p_event_wk )
{
  const u16 scr_wk_id = sc_script_work_id_table[ id ];
  const u16 value = *EVENTWORK_GetEventWorkAdrs( p_event_wk ,scr_wk_id);
  
  // ワーク内容を調べて起動状態を返す
  if( value == IS_ACTION_WK_VALUE )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バッジチェックゲートギミック終了処理
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BADGEGATE_End( FIELDMAP_WORK *p_fieldmap )
{
  int i;
  BADGEGATE_WK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, BADGEGATE_WORK_ASSIGN_ID);

  // 破棄
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MMANAGER_FEXPOBJ_OFF_UID );
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MMANAGER_FEXPOBJ_ON_UID );
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MMANAGER_FEXPOBJ_EV_UID );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, BADGEGATE_WORK_ASSIGN_ID);
}

//----------------------------------------------------------------------------
/**
 *	@brief  バッジチェックゲートメイン処理
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void BADGEGATE_Move( FIELDMAP_WORK *p_fieldmap )
{
  BADGEGATE_WK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, BADGEGATE_WORK_ASSIGN_ID);
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );

  
#ifdef PM_DEBUG
  // 空きヒープサイズ
  {
    u32 heap_size = GFI_HEAP_GetHeapFreeSize( FIELDMAP_GetHeapID(p_fieldmap) );
    ONOUE_Printf("HeapFreeSize:%x\n",heap_size);
  }
#endif

}


//----------------------------------------------------------------------------
/**
 *	@brief  モニュメント起動イベント作成
 *
 *	@param	*p_gamesys
 *	@param	monument_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT* BADGEGATE_CreateMonumentStartEvent( GAMESYS_WORK *p_gamesys, u16 monument_id )
{
  FIELDMAP_WORK   *p_fld_wk = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  // イベント作成
  GMEVENT *p_event = GMEVENT_Create( p_gamesys, NULL, MonumentStart_Event, sizeof(MONUMENT_START_WK) );
  MONUMENT_START_WK *p_wk = GMEVENT_GetEventWork(p_event);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );

  // パラメータ設定
  {
    GFL_STD_MemClear( p_wk, sizeof(MONUMENT_START_WK) );
    p_wk->monument_id = monument_id;
    p_wk->event_frame = 0;
    p_wk->p_gmk_wk    = GMK_TMP_WK_GetWork( p_fld_wk, BADGEGATE_WORK_ASSIGN_ID);
  }

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モニュメント起動ワーク
 *
 *	@param	p_event
 *	@param	p_seq
 *	@param	p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MonumentStart_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  MODEL *p_ev_model;
  MODEL *p_off_model;
  MODEL *p_on_model;
  MONUMENT_START_WK *p_wk = p_wk_adrs;
  
  // 像のモデルデータを取得
  p_ev_model = GetModelData( p_wk->p_gmk_wk , p_wk->monument_id ,MMANAGER_FEXPOBJ_EV_UID);
  p_off_model = GetModelData( p_wk->p_gmk_wk , p_wk->monument_id ,MMANAGER_FEXPOBJ_OFF_UID);
  p_on_model = GetModelData( p_wk->p_gmk_wk , p_wk->monument_id ,MMANAGER_FEXPOBJ_ON_UID);

  // SEを再生する
  MonumentStart_SePlayMain( p_wk );

  p_wk->event_frame++;

  switch( *p_seq )
  {
  case 0:
    {
      // OFFモデルを非表示に
      FLD_EXP_OBJ_SetVanish( p_off_model->p_ex_obj,
          p_off_model->unitID , p_off_model->modelID, TRUE );

      // EVモデルを表示に
      FLD_EXP_OBJ_SetVanish( p_ev_model->p_ex_obj,
          p_ev_model->unitID , p_ev_model->modelID, FALSE );

      // 起動アニメーションを開始
      SetAnmStopFlag( p_ev_model ,MDL_ANM_ACTION_ICA, FALSE );
      SetAnmStopFlag( p_ev_model ,MDL_ANM_ACTION_ITA, FALSE );
      (*p_seq)++;
    }
    break;
  case 1:
    {
      // モニュメントが開くアニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_ev_model->p_ex_obj, p_ev_model->unitID, p_ev_model->modelID, MDL_ANM_ACTION_ICA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
        SetAnmStopFlag( p_ev_model ,MDL_ANM_ACTION_ICA, TRUE );
        (*p_seq)++;
        return GMEVENT_RES_CONTINUE_DIRECT;
      }
    }
    break;
  case 2:
    {
      // EVモデルを表示に
      FLD_EXP_OBJ_SetVanish( p_ev_model->p_ex_obj,
          p_ev_model->unitID , p_ev_model->modelID, TRUE );

      // ONモデルを表示
      FLD_EXP_OBJ_SetVanish( p_on_model->p_ex_obj,
          p_on_model->unitID , p_on_model->modelID, FALSE );

      // フレームを設定
      FLD_EXP_OBJ_SetObjAnmFrm(p_on_model->p_ex_obj, p_on_model->unitID, p_on_model->modelID, MDL_ANM_LOOP_ITA ,GetSynchroAnmFrame( p_wk->p_gmk_wk ) );
      SetAnmStopFlag( p_on_model ,MDL_ANM_LOOP_ITA, FALSE );
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モニュメントのSEを鳴らすメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MonumentStart_SePlayMain( MONUMENT_START_WK *p_wk )
{
  // レーザー音
  if( p_wk->event_frame == MONUMENT_SE_LASER_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_MONUMENT_LASER );
  }
  // せりあがる音
  if( p_wk->event_frame == MONUMENT_SE_OPNE_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_MONUMENT_START );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデルデータを取得する
 *
 *	@param	*p_gmk_wk 
 *	@param	model_id 
 *	@param	unit_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static MODEL* GetModelData( BADGEGATE_WK *p_gmk_wk ,u32 model_id ,u32 unit_id )
{
  // ユニットIDごとに返す
  switch( unit_id )
  {
  case MMANAGER_FEXPOBJ_EV_UID:
    return &p_gmk_wk->model[model_id];
  case MMANAGER_FEXPOBJ_OFF_UID:
    return &p_gmk_wk->model_off[model_id];
  case MMANAGER_FEXPOBJ_ON_UID:
    return &p_gmk_wk->model_on[model_id];
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル内のアニメーション状態を操作する
 *
 *	@param	*p_model  
 *	@param	anmid 
 *	@param	is_stop 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetAnmStopFlag( MODEL *p_model  ,u16 anmid ,BOOL is_stop )
{
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  
  exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, anmid );
  FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, is_stop );
}

//----------------------------------------------------------------------------
/**
 *	@brief  同期アニメーションフレームを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static fx32 GetSynchroAnmFrame( BADGEGATE_WK *p_wk )
{
  int i;

  for( i = 0; i < MONUMENT_CNT ; i++ )
  {
    BOOL is_stop = FALSE;
    MODEL *p_model = GetModelData( p_wk , i ,MMANAGER_FEXPOBJ_ON_UID );
    EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_LOOP_ITA );
    is_stop = FLD_EXP_OBJ_GetAnmStopFlg( exobj_anm );

    ONOUE_Printf("起動状況%d:%d\n",i,is_stop);

    // 停止していないものをチェックする
    if( is_stop == FALSE )
    {
      fx32 anm_frame = FLD_EXP_OBJ_GetObjAnmFrm( p_model->p_ex_obj ,p_model->unitID, p_model->modelID, MDL_ANM_LOOP_ITA );
      ONOUE_Printf("同期番号:%d\n",i);
      return anm_frame;
    }
  }

  // 全てのアニメーションが停止している場合はいつでも同期タイミング
  ONOUE_Printf("ゼロフレーム同期\n");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲート起動イベント作成
 *
 *	@param	*p_gamesys
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT* BADGEGATE_CreateGateOpenEvent( GAMESYS_WORK *p_gamesys )
{
  int i;
  FIELDMAP_WORK   *p_fld_wk = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  // イベント作成
  GMEVENT *p_event = GMEVENT_Create( p_gamesys, NULL, GateOpen_Event, sizeof(GATE_START_WK) );
  GATE_START_WK *p_wk = GMEVENT_GetEventWork(p_event);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );

  // パラメータ設定
  {
    GFL_STD_MemClear( p_wk, sizeof(GATE_START_WK) );
    p_wk->p_gmk_wk    = GMK_TMP_WK_GetWork( p_fld_wk, BADGEGATE_WORK_ASSIGN_ID);
    p_wk->p_cam       = FIELDMAP_GetFieldCamera( p_fld_wk );
    p_wk->p_fieldmap  = p_fld_wk;
    p_wk->shake_event_frame = 0;
    FIELD_CAMERA_GetCamPosOffset( p_wk->p_cam, &p_wk->default_cam_ofs );
    FIELD_CAMERA_GetTargetOffset( p_wk->p_cam, &p_wk->default_target_ofs );
  }

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲート起動イベント
 *
 *	@param	p_event
 *	@param	p_seq
 *	@param	p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GateOpen_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  int i;
  GATE_START_WK *p_wk = p_wk_adrs;
  MODEL *p_off_model;
  MODEL *p_on_model;
 
  // 門のモデルデータを取得
  p_off_model = GetModelData( p_wk->p_gmk_wk ,BADGEGATE_MDLID_GATE_OFF ,MMANAGER_FEXPOBJ_OFF_UID);
  p_on_model = GetModelData( p_wk->p_gmk_wk ,BADGEGATE_MDLID_GATE_ON ,MMANAGER_FEXPOBJ_ON_UID);

  // カメラ揺れの更新
  GateOpen_CameraShakeMain( p_wk );


  // フレーム加算シーケンス,SE再生
  switch( *p_seq )
  {
  case 4:
  case 5:
  case 6:
    // カメラ揺れフレームの加算
    GateOpen_SePlayMain( p_wk );
    p_wk->shake_event_frame++;
    break;
  }

  
  // アニメーション更新メイン
  switch( *p_seq )
  {
  case 0:
    {
      // フェードアウトタスク
      FIELD_TASK    *p_fade_task;
      FIELD_TASK_MAN* p_task_man = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      p_fade_task = FIELD_TASK_Fade( p_wk->p_fieldmap, GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, DISP_FADE_SPEED );
      FIELD_TASK_MAN_AddTask( p_task_man, p_fade_task, NULL );
      (*p_seq)++;
    }
    break;
  case 1:
    {
      if( GFL_FADE_CheckFade() == FALSE )
      {
        // カメラ設定
        FLD_CAM_MV_PARAM param;
        u16 frame;
        param.Core.AnglePitch = 7640;
        param.Core.AngleYaw   = 0;
        param.Core.Distance   = 1626112;
        param.Core.TrgtPos.x  = 2064384;
        param.Core.TrgtPos.y  = 393311;
        param.Core.TrgtPos.z  = 2547712;
        param.Chk.Shift = FALSE;
        param.Chk.Pitch = TRUE;
        param.Chk.Yaw = TRUE;
        param.Chk.Dist = TRUE;
        param.Chk.Fovy = FALSE;
        param.Chk.Pos = TRUE;
        frame = 1;
        FIELD_CAMERA_SetLinerParam( p_wk->p_cam, &param, frame);

        // イベントモデルを生成
        MakeEventGateModelData( &p_wk->ev_model ,p_wk->p_gmk_wk->p_ex_obj );

        (*p_seq)++;
      }
    }
    break;
  case 2:
    {
      // フェードインタスク
      FIELD_TASK    *p_fade_task;
      FIELD_TASK_MAN* p_task_man = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      p_fade_task = FIELD_TASK_Fade( p_wk->p_fieldmap, GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, DISP_FADE_SPEED );
      FIELD_TASK_MAN_AddTask( p_task_man, p_fade_task, NULL );
      (*p_seq)++;
    }
    break;
  case 3:
    {
      if( GFL_FADE_CheckFade() == FALSE )
      {
        // OFFモデルを非表示に
        FLD_EXP_OBJ_SetVanish( p_off_model->p_ex_obj,
            p_off_model->unitID , p_off_model->modelID, TRUE );

        // EVモデルを表示に
        FLD_EXP_OBJ_SetVanish( p_wk->ev_model.p_ex_obj,
            p_wk->ev_model.unitID , p_wk->ev_model.modelID, FALSE );

        // 起動アニメーションを開始
        SetAnmStopFlag( &p_wk->ev_model ,MDL_ANM_ACTION_ITA, FALSE );
        (*p_seq)++;
      }
    }
    break;
  case 4:
    {
      // 発光アニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->ev_model.p_ex_obj, p_wk->ev_model.unitID, p_wk->ev_model.modelID, MDL_ANM_ACTION_ITA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
       // ループアニメーションにきりかえる,ICA起動
        SetAnmStopFlag( &p_wk->ev_model ,MDL_ANM_ACTION_ITA, TRUE );

        // アニメーション切り替え
        FLD_EXP_OBJ_ValidCntAnm(p_wk->ev_model.p_ex_obj, p_wk->ev_model.unitID, p_wk->ev_model.modelID, MDL_ANM_ACTION_ITA, FALSE);
        FLD_EXP_OBJ_ValidCntAnm(p_wk->ev_model.p_ex_obj, p_wk->ev_model.unitID, p_wk->ev_model.modelID, MDL_ANM_GATE_LOOP_ITA, TRUE);
        SetAnmStopFlag( &p_wk->ev_model ,MDL_ANM_GATE_LOOP_ITA, FALSE );
        SetAnmStopFlag( &p_wk->ev_model ,MDL_ANM_ACTION_ICA, FALSE );

        FLD_EXP_OBJ_SetObjAnmFrm(p_wk->ev_model.p_ex_obj, p_wk->ev_model.unitID, p_wk->ev_model.modelID, MDL_ANM_GATE_LOOP_ITA ,GetSynchroAnmFrame( p_wk->p_gmk_wk ) );
        (*p_seq)++;
      }
    }
    break;
  case 5:
    {
      // 門が開くアニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->ev_model.p_ex_obj, p_wk->ev_model.unitID, p_wk->ev_model.modelID, MDL_ANM_ACTION_ICA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
        SetAnmStopFlag( &p_wk->ev_model ,MDL_ANM_ACTION_ICA, TRUE );
        (*p_seq)++;
      }
    }
    break;
  case 6:
    {
      for( i = 0; i < SHAKE_PARAM_CNT ; i++ )
      {
        // 振動アニメーション終了待ち
        if( p_wk->is_shake_end[i] == FALSE )
        {
          return GMEVENT_RES_CONTINUE;
        }
      }
      // ONモデルを表示に
      FLD_EXP_OBJ_SetVanish( p_on_model->p_ex_obj,
          p_on_model->unitID , p_on_model->modelID, FALSE );
      FLD_EXP_OBJ_SetObjAnmFrm(p_on_model->p_ex_obj, p_on_model->unitID, p_on_model->modelID, MDL_ANM_LOOP_ITA ,GetSynchroAnmFrame( p_wk->p_gmk_wk ) );
      SetAnmStopFlag( p_on_model ,MDL_ANM_LOOP_ITA, FALSE );

      // EVモデルを非表示に
      FLD_EXP_OBJ_SetVanish( p_wk->ev_model.p_ex_obj,
          p_wk->ev_model.unitID , p_wk->ev_model.modelID, TRUE );

      // ユニット削除
      DeleteEventModelData( p_wk->p_gmk_wk->p_ex_obj );
      
      // 全てのアニメーションと、振動が終了
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドカメラ揺れを行う
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GateOpen_CameraShakeMain( GATE_START_WK *p_wk )
{
  int i;

  for( i = 0; i < SHAKE_PARAM_CNT ; i++ )
  {
    fx32 w, h;
    VecFx32 set_cam_ofs = p_wk->default_cam_ofs;
    VecFx32 set_target_ofs = p_wk->default_cam_ofs;
    const SHAKE_PARAM *p_sp = &sc_shake_param_table[i];

    // 指定フレーム内の時はゆらす
    if( (p_wk->shake_event_frame >= p_sp->start_frame) &&
        (p_wk->shake_event_frame <= p_sp->end_frame) )
    {
      u16 rad;
      s32 tmp;
      u32 shake_frame = p_wk->shake_event_frame - p_sp->start_frame;
      tmp = shake_frame * 0x10000 / p_sp->loop_sync;
      rad = tmp;

      set_cam_ofs.x += p_sp->w * FX_SinIdx(rad);
      set_cam_ofs.y += p_sp->h * FX_CosIdx(rad);

      set_target_ofs.x += p_sp->w * FX_SinIdx(rad);
      set_target_ofs.y += p_sp->h * FX_CosIdx(rad);

      FIELD_CAMERA_SetCamPosOffset( p_wk->p_cam, &set_cam_ofs );
      FIELD_CAMERA_SetTargetOffset( p_wk->p_cam, &set_target_ofs );
      p_wk->is_shake_end[i] = FALSE;

      // 終了フレームを調べ、振動をリセット
      if( p_wk->shake_event_frame >= p_sp->end_frame )
      {
        FIELD_CAMERA_SetCamPosOffset( p_wk->p_cam, &p_wk->default_cam_ofs );
        p_wk->is_shake_end[i] = TRUE;
        ONOUE_Printf("振動終了\n");
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートオープンイベントのSE再生
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GateOpen_SePlayMain( GATE_START_WK *p_wk )
{
  // ゲート起動、シンボルマークが光る
  if( p_wk->shake_event_frame == GATE_SE_SYMBOL_LIGHT_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_GATE_START );
  }
  // ゲートの光が立ち上がる
  if( p_wk->shake_event_frame == GATE_SE_LIGHT_MOVE_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_GATE_LIGHT );
  }
  // 石柱と、石版が沈む
  if( p_wk->shake_event_frame == GATE_SE_COLUMNLE_DOWN_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_GATE_COLUMN_DOWN1 );
    PMSND_PlaySE( BADGEGATE_SE_SEQ_GATE_COLUMN_DOWN2 );
  }
  // 門が沈む
  if( p_wk->shake_event_frame == GATE_SE_OPEN_FRAME )
  {
    PMSND_PlaySE( BADGEGATE_SE_SEQ_GATE_OPEN );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデルデータ作成（門）
 *
 *	@param	*p_model
 *	@param	p_ex_obj 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeEventGateModelData( MODEL *p_model, FLD_EXP_OBJ_CNT_PTR p_ex_obj )
{
  s32 i;

  // ユニット生成
  FLD_EXP_OBJ_AddUnit( p_ex_obj, &sc_g3d_util_setup_gate, MMANAGER_FEXPOBJ_GATE_UID );

  // モデルデータ設定
  p_model->unitID   = MMANAGER_FEXPOBJ_GATE_UID;
  p_model->modelID  = 0;
  p_model->p_ex_obj = p_ex_obj;

  {
    EXP_OBJ_ANM_CNT_PTR exobj_anm;
    GFL_G3D_OBJSTATUS *p_status;

    // ステータス取得
    p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
        MMANAGER_FEXPOBJ_GATE_UID,
        p_model->modelID );

    // 位置を設定
    p_status->trans = d3obj_pos_table[BADGEGATE_MDLID_GATE];

    FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
        MMANAGER_FEXPOBJ_GATE_UID, p_model->modelID, TRUE );

    //カリングする
    FLD_EXP_OBJ_SetCulling( p_model->p_ex_obj, MMANAGER_FEXPOBJ_GATE_UID, p_model->modelID, TRUE);

    // アニメーションの初期化
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA );
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA, TRUE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ICA ,0);
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,FALSE );
    SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, TRUE );

    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA );
    SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ITA, TRUE );
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA, TRUE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_ACTION_ITA ,0);
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,FALSE );

    
    // ゲート専用アニメーション初期化
    SetAnmStopFlag( p_model ,MDL_ANM_GATE_LOOP_ITA, TRUE );
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_GATE_LOOP_ITA, FALSE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_GATE_LOOP_ITA ,0);    
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, p_model->unitID, p_model->modelID, MDL_ANM_GATE_LOOP_ITA );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm ,TRUE );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントモデルデータを削除
 *
 *	@param	p_ex_obj 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DeleteEventModelData( FLD_EXP_OBJ_CNT_PTR p_ex_obj )
{
  FLD_EXP_OBJ_DelUnit( p_ex_obj, MMANAGER_FEXPOBJ_GATE_UID );
}

