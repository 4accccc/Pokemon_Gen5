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

#include "eventwork_def.h"                    // for WK_SCENE_D26R0101_GATE_*
#include "system/main.h"                      // for HEAPID_FIELDMAP
#include "field_gimmick_def.h"                // for FLD_GIMMICK_D26R0101
#include "arc/fieldmap/badge_gate_gmk.naix"   // モデルデータアーカイブ
#include "field/field_const.h"                // for GRID_TO_FX32
#include "field_gimmick_bgate.cdat"           // モデルセットアップデータ


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// モデルの登録インデックス
#define MMANAGER_FEXPOBJ_UID (0)

// 像の数
#define MONUMENT_CNT  (8)

// イベントが起動済みの時にワークに設定されている値
#define IS_ACTION_WK_VALUE  (2)

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
///	ギミックワーク
//=====================================
typedef struct {
  // sys
  HEAPID              heapID;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fld_wk;  
  EVENTWORK           *p_event_wk;

  // data
  BADGE_GATE_SV_WORK  *p_sv_work;
  MODEL               model[BADGEGATE_MDLID_MAX]; // モデル

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
}MONUMENT_START_WK;

//-------------------------------------
///	ゲート起動ワーク
//=====================================
typedef struct GATE_START_WK_tag
{
  // sys
  BADGEGATE_WK *p_gmk_wk; 
}GATE_START_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void setup_model_data( BADGEGATE_WK *p_wk );
static GMEVENT_RESULT MonumentStart_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static void SetAnmStopFlag( MODEL *p_model  ,u16 anmid ,BOOL is_stop );
static MODEL* GetModelData( BADGEGATE_WK *p_gmk_wk ,u32 model_id );
static GMEVENT_RESULT GateOpen_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static fx32 GetSynchroAnmFrame( BADGEGATE_WK *p_wk );
static void setup_anm_before_action( MODEL *p_model );
static void setup_anm_after_action( MODEL *p_model );
static BOOL get_is_action_end( u32 id, EVENTWORK *p_event_wk );

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
  p_wk->p_sv_work       = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_D26R0101 );
  p_wk->p_ex_obj        = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_event_wk      = GAMEDATA_GetEventWork( p_gamedata );

  // 3D拡張オブジェクトユニットの追加
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup, MMANAGER_FEXPOBJ_UID );
  
  // モデルデータのセットアップ
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
      p_model->unitID   = MMANAGER_FEXPOBJ_UID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, FALSE );
    }
  }

  // モデルステータス設定
  {
    int i;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < BADGEGATE_MDLID_MAX ; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          MMANAGER_FEXPOBJ_UID,
          p_model->modelID );

      // 位置を設定
      p_status->trans = d3obj_pos_table[i];

      // 見える状態に
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
          MMANAGER_FEXPOBJ_UID, p_model->modelID, FALSE );
      
      //カリングする
      FLD_EXP_OBJ_SetCulling( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, 0, TRUE);

      // アニメーション後かどうかを調べて、初期化を行う
      if( get_is_action_end( i, p_wk->p_event_wk ) == FALSE )
      {
        // アニメーション後の状態に初期化
        setup_anm_before_action( p_model );
      }
      else
      {
        // アニメーション前の状態に初期化
        setup_anm_after_action( p_model );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション前の状態に初期化する
 *
 *	@param	*p_model 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void setup_anm_before_action( MODEL *p_model )
{
  EXP_OBJ_ANM_CNT_PTR exobj_anm;

  // アニメーション有効設定、フレーム初期化
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA, TRUE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA ,0 );
  }
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA, TRUE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA ,0 );
  }
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA, FALSE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA ,0 );
  }

  // 通常状態用アニメーション初期化
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
  }
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
  }
  // ループ用アニメーション初期化
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション後の状態に初期化する
 *
 *	@param	*p_model 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void setup_anm_after_action( MODEL *p_model )
{
  EXP_OBJ_ANM_CNT_PTR exobj_anm;

  // アニメーション有効設定、フレーム初期化
  {
    fx32 last_frame;
    EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA );

    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA, TRUE);
    last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exobj_anm );
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA ,last_frame );
  }
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA, FALSE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA ,0 );
  }
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA, TRUE);
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA ,0 );
  }

  // 通常状態用アニメーション初期化
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
  }
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
  }
  // ループ用アニメーション初期化
  {
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, TRUE );
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
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MMANAGER_FEXPOBJ_UID );

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
  
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );
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
    p_wk->monument_id = monument_id;
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
  MONUMENT_START_WK *p_wk = p_wk_adrs;
  MODEL *p_model;
  
  // 像のモデルデータを取得
  p_model = GetModelData( p_wk->p_gmk_wk ,BADGEGATE_MDLID_MONUMENT_0 + p_wk->monument_id );

  switch( *p_seq )
  {
  case 0:
    {
        // 起動アニメーションを開始
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, FALSE );
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ITA, FALSE );
        (*p_seq)++;
    }
    break;
  case 1:
    {
      // モニュメントが開くアニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, TRUE );
        (*p_seq)++;
        // 同期タイミングかどうかをすぐに調べる
        return GMEVENT_RES_CONTINUE_DIRECT;
      }
    }
    break;
  case 2:
    {
      // ループアニメーションにきりかえる
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA ,GetSynchroAnmFrame( p_wk->p_gmk_wk ) );
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA, FALSE);
      FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA, TRUE);
      SetAnmStopFlag( p_model ,MDL_ANM_LOOP_ITA, FALSE );
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデルデータを取得する
 *
 *	@param	*p_gmk_wk 
 *	@param	model_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static MODEL* GetModelData( BADGEGATE_WK *p_gmk_wk ,u32 model_id )
{
  return &p_gmk_wk->model[model_id];
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
  
  exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, anmid );
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
    MODEL *p_model = GetModelData( p_wk ,BADGEGATE_MDLID_MONUMENT_0 + i );
    EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA );
    is_stop = FLD_EXP_OBJ_GetAnmStopFlg( exobj_anm );

    // 停止していないものをチェックする
    if( is_stop == FALSE )
    {
      fx32 anm_frame = FLD_EXP_OBJ_GetObjAnmFrm( p_model->p_ex_obj ,MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA );
      return anm_frame;
    }
  }

  // 全てのアニメーションが停止している場合はいつでも同期タイミング
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
  FIELDMAP_WORK   *p_fld_wk = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  // イベント作成
  GMEVENT *p_event = GMEVENT_Create( p_gamesys, NULL, GateOpen_Event, sizeof(GATE_START_WK) );
  GATE_START_WK *p_wk = GMEVENT_GetEventWork(p_event);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );

  // パラメータ設定
  {
    p_wk->p_gmk_wk    = GMK_TMP_WK_GetWork( p_fld_wk, BADGEGATE_WORK_ASSIGN_ID);
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
  GATE_START_WK *p_wk = p_wk_adrs;
  MODEL *p_model;
  
  // 門のモデルデータを取得
  p_model = GetModelData( p_wk->p_gmk_wk ,BADGEGATE_MDLID_GATE );

  switch( *p_seq )
  {
  case 0:
    {
        // 発光アニメーションを起動
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ITA, FALSE );
        (*p_seq)++;
    }
    break;
  case 1:
    {
      // 発光アニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
        // ループアニメーションにきりかえる
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ITA, TRUE );
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA ,GetSynchroAnmFrame( p_wk->p_gmk_wk ) );
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ITA, FALSE);
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_LOOP_ITA, TRUE);

        // 門が開くアニメーション,ループアニメーションを起動
        SetAnmStopFlag( p_model ,MDL_ANM_LOOP_ITA, FALSE );
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, FALSE );
        (*p_seq)++;
      }
    }
    break;
  case 2:
    {
      // 門が開くアニメーション終了待ち
      EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MMANAGER_FEXPOBJ_UID, p_model->modelID, MDL_ANM_ACTION_ICA );
      if( FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE )
      {
        SetAnmStopFlag( p_model ,MDL_ANM_ACTION_ICA, TRUE );
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

