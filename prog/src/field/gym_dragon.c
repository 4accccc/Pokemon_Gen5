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

#include "field/field_const.h"          //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_dragon.naix"
#include "system/main.h"                //for HEAPID_FIELDMAP
#include "script.h"                     //for SCRIPT_CallScript

#include "sound/pm_sndsys.h"
#include "gym_dragon_se_def.h"
#include "system/ica_anime.h"

#include "../../../resource/fldmapdata/eventdata/tmp/c08gym0101.h"                      // obj_id
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_trace_data_id.h"   // データID
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_trace_data.cdat"   // sc_hero_move_data
#include "../../../resource/fldmapdata/flagwork/flag_define.h"                          // SYS_FLAG_C08GYM_BUTTON_～
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_height_data.cdat"  // sc_ex_height_data
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_gym_data_def.h"    // 外部指定パラメータ
#include "gym_dragon.cdat"

#include "eventwork_def.h"  // LOCALWORK*

#include "tr_tool/trno_def.h" // TRID_*
#include "script_trainer.h"
#include "field_task_camera_shake.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_DRAGON_UNIT_IDX (0)
#define GYM_DRAGON_TMP_ASSIGN_ID  (1)
#define GYM_DRAGON_ANM_HEAP_SIZE (0x24000) // 追従アニメーションデータ用ヒープ


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
  HEAPID              heapAnmID;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fld_wk;  
  EVENTWORK           *p_event_wk;

  // data
  GYM_DRAGON_SV_WORK  *p_sv_work;
  MODEL               model[GYM_DRAGON_MODELID_MAX]; // モデル
  
}GYM_DRAGON_TMP;

//-------------------------------------
///	ICA追従データ
//=====================================
typedef struct {
  u8            model_id;
  MMDL          *p_mmdl;
  s32           move_data_frame;
  s32           move_data_frame_max;
  ICA_ANIME     *p_ica_move_data;
  MODEL         *p_model; 
  const GYM_DRAGON_TRACE_DATA *p_trace_data;
  BOOL          is_move_end;
  BOOL          is_reverse_move;
} TRACE_MOVE_DATA;

//-------------------------------------
///	プレイヤー移動ワーク
//=====================================
typedef struct PLAYER_MOVE_WORK_tag
{
  // sys
  GYM_DRAGON_TMP    *p_gmkwk;
  FIELDMAP_WORK     *p_fld_wk;
  EVENTWORK         *p_event_wk;
  MMDLSYS           *p_mmdl_sys;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  MODEL *p_en_dra_model;

  // data
  FIELD_PLAYER  *p_player;
  u16           event_seq;
  const BUTTON_EVENT_DATA *p_button_evdata;
  GYM_DRAGON_SV_WORK  *p_sv_work;
  u32           event_frame;
  s32           bark_seq;
  u32           cam_type_tmp;

  TRACE_MOVE_DATA move_data[ICA_MOVE_ID_MAX];

}PLAYER_MOVE_WORK;

//-------------------------------------
///	プレイヤー移動ワーク
//=====================================
typedef struct BUTTON_TURN_ON_EVENT_WK_tag
{
  // sys
  GYM_DRAGON_TMP    *p_gmkwk;
  EVENTWORK         *p_event_wk;

  // data
  u16               layer;
  u32               wait_frame;
}BUTTON_TURN_ON_EVENT_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
typedef BOOL (*UPLAYERCECHK)(EVENTWORK*);

static BOOL CheckCanUpLayer( EVENTWORK* p_event_wk,FLOOR_LAYER layer_id );
static BOOL CheckUpLayerTail(EVENTWORK* p_wk);
static BOOL CheckUpLayerFoot(EVENTWORK* p_wk);
static BOOL CheckUpLayerArm(EVENTWORK* p_wk);
static BOOL CheckUpLayerHead(EVENTWORK* p_wk);

static void setup_model_data( GYM_DRAGON_TMP *p_wk );
static GYM_BUTTON_SEQ GetNowButtonType( EVENTWORK* p_event_wk ,u16 layer ,u16 loc );
static GMEVENT * CreateButtonTurnOnEvent( GAMESYS_WORK *p_gamesys, u16 layer );
static GMEVENT * CreateButtonEvent_UDLR( GAMESYS_WORK *p_gamesys, u16 event_seq );
static GMEVENT_RESULT PlayerMove_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static GMEVENT_RESULT ButtonActive_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static GMEVENT_RESULT ButtonTurnOn_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );
static void MOVE_EVENT_Init( PLAYER_MOVE_WORK *p_wk );
static void MOVE_EVENT_Exit( PLAYER_MOVE_WORK *p_wk );
static void MOVE_Event_UpdatePlayerPos( PLAYER_MOVE_WORK *p_wk );
static void MOVE_Event_UpdateEnemyTrainerPos( PLAYER_MOVE_WORK *p_wk );
static void MOVE_EVENT_UpdateDragonGymData( PLAYER_MOVE_WORK *p_wk );
static void MOVE_Event_DragonBarkMain( PLAYER_MOVE_WORK *p_wk );
static void MOVE_Event_StopEffectMain( PLAYER_MOVE_WORK *p_wk );
static void UpdateButtonStatus( GYM_BUTTON_SEQ button_seq ,EVENTWORK* p_event_wk );
static void SetWorkValue( EVENTWORK* p_event_wk ,u16 work_no ,u16 value );
static void SetExHeight( FIELDMAP_WORK* p_fld_wk );
static void SetButtonITPSeq( GYM_DRAGON_TMP *p_wk );
static void SetHeroDragonITPSeq( GYM_DRAGON_TMP *p_wk );
static void UpdateFrame( TRACE_MOVE_DATA *p_md );
static void CallShakeCamera_Bark( FIELDMAP_WORK *p_fld_wk );
static void CallShakeCamera_Bump( FIELDMAP_WORK *p_fld_wk );
static u32 get_anmend_frame( TRACE_MOVE_DATA *p_md );

#ifdef PM_DEBUG
#define DEBUGMENU_GYM_DRAGON (2)
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fld_wk, HEAPID heapID );
static void DEBUGMENU_Exit( void );

// デバッグパラメータ
static s32 debug_fog_slope_value  = FOG_SLOPE;
static s32 debug_fog_offset_value = FOG_OFFSET;
static s32 debug_fog_color_a_value  = FOG_COLOR_A;
static s32 debug_dragon_bark_value  = DRA_BARK_FRAME_HERO;
#endif

//-------------------------------------
///	上レイヤーへ行けるか判定関数テーブル
//=====================================
UPLAYERCECHK sc_uplayer_check[] = {
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
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  GIMMICKWORK *p_gmkwork = GAMEDATA_GetGimmickWork(p_gamedata);

#ifdef PM_DEBUG
  // 空きヒープサイズ
  {
    u32 heap_size = GFI_HEAP_GetHeapFreeSize( FIELDMAP_GetHeapID(fieldWork) );
    OS_Printf("開始前残りヒープサイズ:%x\n",heap_size);
  }
#endif

  //汎用ワーク確保
  p_wk = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_DRAGON_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_DRAGON_TMP));

  // アニメーション用ヒープ生成
  GFL_HEAP_CreateHeap( HEAPID_FIELDMAP, HEAPID_GYM_DRAGON_ANM, GYM_DRAGON_ANM_HEAP_SIZE );

  // パラメータの初期化
  p_wk->heapID          = HEAPID_FIELDMAP;
  p_wk->heapAnmID       = HEAPID_GYM_DRAGON_ANM;
  p_wk->p_fld_wk        = fieldWork;
  p_wk->p_sv_work       = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_DRAGON );
  p_wk->p_ex_obj        = FIELDMAP_GetExpObjCntPtr( fieldWork );
  p_wk->p_event_wk      = GAMEDATA_GetEventWork( p_gamedata );

#ifdef PM_DEBUG
  // Rデバッグメニューの初期化
  DEBUGMENU_Init( p_wk ,p_wk->p_fld_wk ,p_wk->heapID);
#endif
 
  // 3D拡張オブジェクトユニットの追加
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup, GYM_DRAGON_UNIT_IDX );
 
  // 拡張高さの設定
  SetExHeight( p_wk->p_fld_wk );

  // モデルデータのセットアップ
  setup_model_data( p_wk );

  // フォグの設定
  {
    FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_wk->p_fld_wk );
    FIELD_FOG_SetBlendMode( fog, FIELD_FOG_BLEND_COLOR_ALPHA );
    FIELD_FOG_SetColorA( fog, FOG_COLOR_A );
    FIELD_FOG_SetTblAll( fog, DATA_FogTable );
    FIELD_FOG_SetSlope( fog, FOG_SLOPE );
    FIELD_FOG_SetOffset( fog, FOG_OFFSET );
    FIELD_FOG_SetFlag( fog, TRUE );
    FIELD_FOG_Reflect( fog );
  }

  // ボタンを点灯状態にする(ボタンITPのシーケンスはここで設定される)
  SetButtonITPSeq( p_wk );

#ifdef PM_DEBUG
  // 空きヒープサイズ
  {
    u32 heap_size = GFI_HEAP_GetHeapFreeSize( p_wk->heapID );
    OS_Printf("開始後残りヒープサイズ:%x\n",heap_size);
  }
#endif

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
static void setup_model_data( GYM_DRAGON_TMP *p_wk )
{
  //モデル初期化
  {
    int i;
    for( i = 0; i < GYM_DRAGON_MODELID_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];
      p_model->unitID   = GYM_DRAGON_UNIT_IDX;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, FALSE );
    }
  }

  // モデルステータス設定
  {
    int i,n;
    EXP_OBJ_ANM_CNT_PTR exobj_anm;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < GYM_DRAGON_MODELID_MAX ; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];

      // ステータス取得
      p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
          GYM_DRAGON_UNIT_IDX,
          p_model->modelID );

      // 位置を設定
      p_status->trans = sc_mdl_offset_table[i];

      // 見える状態に
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
          GYM_DRAGON_UNIT_IDX, p_model->modelID, FALSE );

      // アニメーション初期化
      for( n = 0 ; n < sc_g3d_util_obj[i].anmCount ; n++ )
      {
        // 有効にする
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, n, TRUE);

        // ICAアニメーションのフレームを復帰
        {
          fx32 frame  = p_wk->p_sv_work->model_frame[p_model->modelID];
          FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, n ,frame );
        }

        // ループ、再生設定
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, n );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
      }

    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機ドラゴン用ITPを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetHeroDragonITPSeq( GYM_DRAGON_TMP *p_wk )
{
  // アニメーションの有効状態を設定する
  MODEL *p_model    = &p_wk->model[GYM_DRAGON_MODELID_HERO_DRAGON];
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_ITP, TRUE);
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP, FALSE);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のボタン状態に合わせて、ITPを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetButtonITPSeq( GYM_DRAGON_TMP *p_wk )
{
  // アニメーションの有効状態を設定する
  MODEL *p_model    = &p_wk->model[GYM_DRAGON_MODELID_HERO_DRAGON];
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_ITP, FALSE);
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP, TRUE);
  }

  // IPTのシーケンスを設定する
  {
    FLOOR_LAYER   now_layer = p_wk->p_sv_work->now_floor_layer;
    GYM_LOCATION  now_loc   = p_wk->p_sv_work->now_location;
    GYM_BUTTON_SEQ button_seq = GetNowButtonType( p_wk->p_event_wk, now_layer, now_loc );

    // フレーム
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,FX32_CONST(button_seq) );
  }
}

//-----------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定する
 *
 *	@param	p_fld_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetExHeight( FIELDMAP_WORK* p_fld_wk )
{
  int i;
  FLDMAPPER* p_mapper = FIELDMAP_GetFieldG3Dmapper( p_fld_wk );
  EHL_PTR p_ex_height = FLDMAPPER_GetExHegihtPtr( p_mapper );

  for(i = 0;i < EX_HEIGHT_DATA_NUM;i++){
    EXH_SetUpHeightData( i,
      sc_ex_height_data[i].x, sc_ex_height_data[i].z,
      sc_ex_height_data[i].xw, sc_ex_height_data[i].zw,
			sc_ex_height_data[i].h, EX_HEIGHT_ATTRIBUTE, p_ex_height );
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
  int i;
  GYM_DRAGON_TMP *p_wk = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

#ifdef PM_DEBUG
  // デバッグメニューの開放
  DEBUGMENU_Exit();
#endif

  // 破棄
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, GYM_DRAGON_UNIT_IDX );
  
  // アニメーション用ヒープを削除
  GFL_HEAP_DeleteHeap( p_wk->heapAnmID );
  
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
  GYM_DRAGON_TMP *p_wk = GMK_TMP_WK_GetWork(fieldWork, GYM_DRAGON_TMP_ASSIGN_ID);

  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );

#ifdef PM_DEBUG
  {
    // 強制ボタンイベント生成
    int cont = GFL_UI_KEY_GetCont();
    int trg = GFL_UI_KEY_GetTrg();
    GMEVENT *call_event;
    GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldWork );
    
    // モデル非表示
    if( (cont & PAD_BUTTON_DEBUG) && (trg & PAD_BUTTON_START) )
    {
      int i;
      for( i = 0; i < GYM_DRAGON_MODELID_MAX; i++ )
      {
        MODEL *p_model    = &p_wk->model[i];
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, FALSE );
      }
    }

    // モデル表示
    if( (cont & PAD_BUTTON_DEBUG) && (trg & PAD_BUTTON_SELECT) )
    {
      int i;
      for( i = 0; i < GYM_DRAGON_MODELID_MAX; i++ )
      {
        MODEL *p_model    = &p_wk->model[i];
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, TRUE );
      }
    }

    if ( GAMESYSTEM_GetEvent( gsys ) == NULL )
    {
      if( (cont & PAD_BUTTON_L) && (trg & PAD_KEY_UP) )
      {
        if( p_wk->p_sv_work->now_floor_layer < 3 )
        {
          call_event = GYM_DRAGON_CreateButtonEvent(gsys,GYM_DRAGON_EVENT_SEQ_UP_BUTTON);
          GAMESYSTEM_SetEvent( gsys ,call_event);
        }
      }
      if( (cont & PAD_BUTTON_L) && (trg & PAD_KEY_DOWN) )
      {
        if( p_wk->p_sv_work->now_floor_layer > 0 )
        {
          call_event = GYM_DRAGON_CreateButtonEvent(gsys,GYM_DRAGON_EVENT_SEQ_DOWN_BUTTON);
          GAMESYSTEM_SetEvent( gsys ,call_event);
        }
      }
      if( (cont & PAD_BUTTON_L) && (trg & PAD_KEY_LEFT) )
      {
        if( p_wk->p_sv_work->now_location != GYM_LOCATION_LEFT )
        {
          call_event = GYM_DRAGON_CreateButtonEvent(gsys,GYM_DRAGON_EVENT_SEQ_LEFT_BUTTON);
          GAMESYSTEM_SetEvent( gsys ,call_event);
        }
      }
      if( (cont & PAD_BUTTON_L) && (trg & PAD_KEY_RIGHT) )
      {
        if( p_wk->p_sv_work->now_location != GYM_LOCATION_RIGHT )
        {
          call_event = GYM_DRAGON_CreateButtonEvent(gsys,GYM_DRAGON_EVENT_SEQ_RIGHT_BUTTON);
          GAMESYSTEM_SetEvent( gsys ,call_event);
        }
      }
    }
  }
#endif
  
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
 *	@brief 上のレイヤーに上がれるかチェックする(TAIL)
 *
 *	@param	p_wk      イベントワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL CheckUpLayerTail(EVENTWORK* p_wk)
{
  u16 wk_value = *EVENTWORK_GetEventWorkAdrs( p_wk, WK_SCENE_C08GYM_TURN_BTN_01 );
  BOOL tr_ret = SCRIPT_CheckEventFlagTrainer( p_wk ,TRID_VETERANM_09);

  if( ( tr_ret == TRUE) && (wk_value == 2) )
  {
    return TRUE;
  }
  return FALSE;
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
  BOOL tr1_ret;
  BOOL tr2_ret;
  u16 wk_value = *EVENTWORK_GetEventWorkAdrs( p_wk, WK_SCENE_C08GYM_TURN_BTN_02 );
  tr1_ret = SCRIPT_CheckEventFlagTrainer( p_wk ,TRID_VETERANM_10);
  tr2_ret = SCRIPT_CheckEventFlagTrainer( p_wk ,TRID_VETERANW_07);
  if( ((tr1_ret == TRUE )  || (tr2_ret == TRUE )) && wk_value == 2 )
  {
    return TRUE;
  }
  return FALSE;
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
  BOOL tr1_ret;
  BOOL tr2_ret;
  u16 wk_value = *EVENTWORK_GetEventWorkAdrs( p_wk, WK_SCENE_C08GYM_TURN_BTN_03 );
  tr1_ret = SCRIPT_CheckEventFlagTrainer( p_wk ,TRID_VETERANM_11);
  tr2_ret = SCRIPT_CheckEventFlagTrainer( p_wk ,TRID_VETERANW_08);
  if( ((tr1_ret == TRUE )  || (tr2_ret == TRUE )) && wk_value == 2 )
  {
    return TRUE;
  }
  return FALSE;
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
  // これ以上、上にはいけない
  return FALSE;
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
 *	@brief  ジム内ボタン再設定イベント
 *
 *	@param	*p_gamesys
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void GYM_DRAGON_RefreshButton( GAMESYS_WORK *p_gamesys )
{
  GAMEDATA    *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  EVENTWORK   *p_event_wk = GAMEDATA_GetEventWork( p_gamedata );
  GIMMICKWORK *p_gmkwork  = GAMEDATA_GetGimmickWork( p_gamedata );
  GYM_DRAGON_SV_WORK *p_sv_work = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_DRAGON );

  FLOOR_LAYER   now_layer = p_sv_work->now_floor_layer;
  GYM_LOCATION  now_loc   = p_sv_work->now_location;
  GYM_BUTTON_SEQ button_seq = GetNowButtonType( p_event_wk, now_layer, now_loc );
  
  // 表示するべきボタン状況を取得し、その状況に合わせてフラグを操作
  UpdateButtonStatus( button_seq ,p_event_wk);
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
 *	@brief  ボタン点灯イベント
 *
 *	@param	*p_gamesys
 *	@param	layer 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_DRAGON_CreateButtonTurnOnEvent( GAMESYS_WORK *p_gamesys, u16 layer )
{
  FIELDMAP_WORK   *p_fld_wk = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_DRAGON_TMP  *p_gmkwk = GMK_TMP_WK_GetWork( p_fld_wk, GYM_DRAGON_TMP_ASSIGN_ID);

  // イベント作成
  GMEVENT *p_event = GMEVENT_Create( p_gamesys, NULL, ButtonTurnOn_Event, sizeof(BUTTON_TURN_ON_EVENT_WK) );
  BUTTON_TURN_ON_EVENT_WK *p_wk = GMEVENT_GetEventWork(p_event);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  GIMMICKWORK *p_gmkwork = GAMEDATA_GetGimmickWork(p_gamedata);

  // パラメータ設定
  {
    // ボタンデータの取得
    p_wk->layer         = layer;
    // ギミックワークの取得
    p_wk->p_gmkwk           = p_gmkwk;
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
  // 移動開始音
  case MOVE_EVENT_SEQ_START_SE:
    {
      PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE_START );
      (*p_seq) = MOVE_EVENT_SEQ_START_SE_WAIT;
    }
    break;

  // 移動中音再生
  case MOVE_EVENT_SEQ_START_SE_WAIT:
    {
      if( PMSND_CheckPlaySE() == FALSE )
      {
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE );
        (*p_seq) = MOVE_EVENT_SEQ_INIT;
      }
    }
    break;

  // ギミックの初期化
  case MOVE_EVENT_SEQ_INIT:
    {
      MOVE_EVENT_Init( p_wk );
      (*p_seq) = MOVE_EVENT_SEQ_MOVE;
    }
    // through break;

  // 移動シーケンスメイン
  case MOVE_EVENT_SEQ_MOVE:
    {
      int i;

      // フレームの加算
      p_wk->event_frame++;

      // トレーナー位置のアップデート
      MOVE_Event_UpdatePlayerPos( p_wk );

      // 敵トレーナー位置のアップデート
      MOVE_Event_UpdateEnemyTrainerPos( p_wk );

      // ドラゴンのほえる処理
      MOVE_Event_DragonBarkMain( p_wk );

      // 停止演出
      MOVE_Event_StopEffectMain( p_wk );

      // アニメーション終了チェック
      for( i = 0; i < ICA_MOVE_ID_MAX ; i++ )
      {
        TRACE_MOVE_DATA *p_md = &p_wk->move_data[i];
        if( p_md->is_move_end == FALSE )
        {
          return GMEVENT_RES_CONTINUE;
        }
      }

      // SE終了チェック
      if( PMSND_CheckPlaySE() == TRUE )
      {
        return GMEVENT_RES_CONTINUE;
      }

      (*p_seq) = MOVE_EVENT_SEQ_CALL_BUTTON_EVENT;
    }
    break;
  
  // ボタン点灯イベント開始シーケンス
  case MOVE_EVENT_SEQ_CALL_BUTTON_EVENT:
    {
      // ボタン起動イベント設定
      u16 layer2_value = *EVENTWORK_GetEventWorkAdrs( p_wk->p_event_wk, WK_SCENE_C08GYM_TURN_BTN_02 );
      u16 layer3_value = *EVENTWORK_GetEventWorkAdrs( p_wk->p_event_wk, WK_SCENE_C08GYM_TURN_BTN_03 );
      GMEVENT       *call_event;
      GAMESYS_WORK  *gsys = FIELDMAP_GetGameSysWork( p_wk->p_fld_wk );

      // 2階のボタンイ点灯イベント
      if( layer2_value == 1 )
      {
        call_event = GYM_DRAGON_CreateButtonTurnOnEvent(gsys,1);
        GMEVENT_CallEvent( p_event, call_event );
        (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
      }
      // 3階のボタンイ点灯イベント
      else if( layer3_value == 1 )
      {
        call_event = GYM_DRAGON_CreateButtonTurnOnEvent(gsys,2);
        GMEVENT_CallEvent( p_event, call_event );
        (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
      }
      // イベントなしの場合
      else
      {
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_SWITCH_TURNON );
        (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
      }
    }
    break;

  // ボタン設定シーケンス
  case MOVE_EVENT_SEQ_BUTTON_SET:
    {
      MOVE_EVENT_UpdateDragonGymData( p_wk );
      SetButtonITPSeq( p_wk->p_gmkwk );
      (*p_seq) = MOVE_EVENT_SEQ_EXIT;
    }
    break;

  // 移動イベント終了シーケンス
  case MOVE_EVENT_SEQ_EXIT:
    {
      MOVE_EVENT_Exit(p_wk);
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
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( p_wk->p_fld_wk ) );
  
  p_wk->p_player      = FIELDMAP_GetFieldPlayer( p_wk->p_fld_wk );
  p_wk->p_ex_obj      = p_wk->p_gmkwk->p_ex_obj;
  p_wk->p_mmdl_sys    = GAMEDATA_GetMMdlSys( p_gamedata );
  p_wk->event_frame   = 0;
  p_wk->bark_seq      = 0;
  
  // 自機ドラゴン用ITPを設定する
  SetHeroDragonITPSeq( p_wk->p_gmkwk );

  // 自機ICA移動データ
  {
    int i;
    u16 trace_data_id = p_wk->p_button_evdata->pl_trace_data_id;
    TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_PLAYER];
    u8  move_data_index;
    p_md->model_id            = MODEL_RESID_HERO_DRAGON;
    p_md->p_trace_data        = &sc_trace_data[trace_data_id];
    move_data_index           = p_md->p_trace_data->data_index;
    p_md->p_ica_move_data     = ICA_ANIME_CreateAlloc( p_wk->p_gmkwk->heapAnmID, ARCID_GYM_DRAGON,sc_hero_move_data[move_data_index] );
    p_md->p_model             = &p_wk->p_gmkwk->model[p_md->model_id];
    p_md->move_data_frame_max = ICA_ANIME_GetMaxFrame( p_md->p_ica_move_data ) - 1;
    p_md->move_data_frame     = p_md->p_trace_data->start_frame;
    p_md->p_mmdl              = FIELD_PLAYER_GetMMdl( p_wk->p_player );
    p_md->is_move_end         = FALSE;

    // 逆再生を調べる
    if( p_md->p_trace_data->start_frame < p_md->p_trace_data->end_frame )
    {
      p_md->is_reverse_move = FALSE;
      FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,FX32_ONE );
    }
    else
    {
      p_md->is_reverse_move = TRUE;
      FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,-FX32_ONE );
    }

    // 自機アニメーション再生
    for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
    {
      // ボタン用ITPは再生しない
      if( i != HERO_DRA_ANM_BUTTON_ITP )
      {
        MODEL *p_model  = p_md->p_model;
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i ,FX32_CONST(p_md->move_data_frame) );
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );

      }
    }
  }

  // 敵ICA移動データ
  {
    const BUTTON_EVENT_DATA *p_bev = p_wk->p_button_evdata;
    if( p_bev->is_enemy_move == TRUE )
    {
      int i;
      u16 trace_data_id = p_bev->en_trace_data_id;
      TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_ENEMY];
      u8  move_data_index;
      p_md->model_id            = p_bev->en_model_id;
      p_md->p_trace_data = &sc_trace_data[trace_data_id];
      move_data_index = p_md->p_trace_data->data_index;
      p_md->p_ica_move_data     = ICA_ANIME_CreateAlloc( p_wk->p_gmkwk->heapAnmID,
          ARCID_GYM_DRAGON,sc_hero_move_data[move_data_index] );
      p_md->p_model             = &p_wk->p_gmkwk->model[p_md->model_id];
      p_md->move_data_frame_max = ICA_ANIME_GetMaxFrame( p_md->p_ica_move_data ) - 1;
      p_md->move_data_frame     = p_md->p_trace_data->start_frame;
      p_md->p_mmdl              = MMDLSYS_SearchOBJID( p_wk->p_mmdl_sys ,p_bev->en_obj_id );
      p_md->is_move_end         = FALSE;

      // 逆再生を調べる
      if( p_md->p_trace_data->start_frame < p_md->p_trace_data->end_frame )
      {
        p_md->is_reverse_move = FALSE;
        FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,FX32_ONE );
      }
      else
      {
        p_md->is_reverse_move = TRUE;
        FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,-FX32_ONE );
      }
 
      // 敵アニメーション再生
      for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
      {
        EXP_OBJ_ANM_CNT_PTR exobj_anm;
        MODEL *p_model    = p_md->p_model;

        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i ,FX32_CONST(p_md->move_data_frame) );
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
      }      
    }
    else{
      // 再生しないときは、アニメーションが終了したことにする
      TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_ENEMY];
      p_md->is_move_end = TRUE;
      p_md->p_ica_move_data     = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤー移動イベント終了
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_EVENT_Exit( PLAYER_MOVE_WORK *p_wk )
{
  int i;

  // ICAアニメーションのフレーム保存
  for( i = 0; i < GYM_DRAGON_MODELID_MAX ; i++ )
  {
    MODEL *p_model  = &p_wk->p_gmkwk->model[i];
    fx32 frame = FLD_EXP_OBJ_GetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, 0 , 0);
    p_wk->p_sv_work->model_frame[p_model->modelID] = frame;
  }
  
  // ICAアニメの開放
  for( i = 0; i < ICA_MOVE_ID_MAX ; i++ )
  {
    if( p_wk->move_data[i].p_ica_move_data != NULL )
    {
      ICA_ANIME_Delete( p_wk->move_data[i].p_ica_move_data );
    }
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
static void MOVE_Event_UpdatePlayerPos( PLAYER_MOVE_WORK *p_wk )
{
  int i;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_PLAYER];
  fx32 frame;
  
  // アニメーションフレームの初期化
  frame = FLD_EXP_OBJ_GetObjAnmFrm(p_md->p_model->p_ex_obj,GYM_DRAGON_UNIT_IDX,0,0);

 
  // アニメーション停止
  if( p_md->is_move_end == TRUE )
  {
    for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
    {
      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_md->p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_md->p_model->modelID, i );
      FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
    }
  }

  // 移動処理
  if( p_md->is_move_end == FALSE )
  {
    VecFx32 vec = {0,0,0};
    if ( ICA_ANIME_GetTranslateAt( p_md->p_ica_move_data, &vec, p_md->move_data_frame + 1 ) ) // 0フレーム目はスタート位置なのでスキップ
    {
      MtxFx43 trans;
      MODEL *p_model    = p_md->p_model;
        
      vec.x += sc_mdl_offset_table[p_md->model_id].x;
      vec.y += sc_mdl_offset_table[p_md->model_id].y;
      vec.z += sc_mdl_offset_table[p_md->model_id].z;

      // プレイヤー位置を設定
      FIELD_PLAYER_SetPos( p_wk->p_player, &vec );

      // フレームの更新
      UpdateFrame( p_md );
    }
  }
 
}

//----------------------------------------------------------------------------
/**
 *	@brief  ドラゴン咆哮音処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_Event_DragonBarkMain( PLAYER_MOVE_WORK *p_wk )
{
  // 自機の追従フレームを基準にする
  TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_PLAYER];
  s32 end_frame;

  // 最上階に移動するときのみ処理を行う
  if( (p_wk->event_seq != GYM_DRAGON_EVENT_SEQ_UP_BUTTON) || 
      (p_wk->p_sv_work->now_floor_layer !=  FLOOR_LAYER_ARM))
  {
    return;
  }
 
  // 終了フレームの取得
  end_frame = get_anmend_frame( p_md );

  // 画面ゆれ
  if( end_frame == (p_wk->event_frame + DRA_BARK_CAMERA_SHAKE_HERO) )
  {
    CallShakeCamera_Bark( p_wk->p_fld_wk );
  }
  if( end_frame == (p_wk->event_frame + DRA_BARK_CAMERA_SHAKE_ENEMY) )
  {
    CallShakeCamera_Bark( p_wk->p_fld_wk );
  }

  // カメラ切り替え
  if( end_frame == (p_wk->event_frame + DRA_BARK_EFFECT_CAM_START) )
  {
    // 演出用カメラに切り替え
    FIELD_CAMERA *p_cam = FIELDMAP_GetFieldCamera( p_wk->p_fld_wk );
    p_wk->cam_type_tmp = FIELD_CAMERA_GetCameraType( p_cam );
  
    // デフォルトカメラを切り替え
    FIELD_CAMERA_SetCameraType( p_cam, GYM_DRAGON_EFFECT_CAM_NO );  // デフォルトカメラの切り替え
    FIELD_CAMERA_RestartTrace(p_cam);                               // トレース配列をクリア

    ONOUE_Printf("カメラを演出用に変更\n");
  }
  if( end_frame == (p_wk->event_frame + DRA_BARK_EFFECT_CAM_END) )
  {
    // 演出用カメラから通常カメラに戻す
    FIELD_CAMERA *p_cam = FIELDMAP_GetFieldCamera( p_wk->p_fld_wk );
    FIELD_CAMERA_SetCameraType( p_cam, p_wk->cam_type_tmp );        // カメラタイプを復帰
    FIELD_CAMERA_RestartTrace(p_cam);                               // トレース配列をクリア
    ONOUE_Printf("カメラを元に戻す\n");
  }

  // SE再生
  switch( p_wk->bark_seq )
  {
  case 0:
    {
      // 移動音の停止シーケンス
      if( end_frame == (p_wk->event_frame + DRA_BARK_MOVE_STOP_SE_FRAME) )
      {
        // 移動音の停止
        PMSND_StopSE();
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE_STOP );
        p_wk->bark_seq++;
        ONOUE_Printf("ドラゴン咆哮：移動停止音の再生\n");
      }
    }
    break;
  case 1: // 自機ドラゴンと敵ドラゴンが咆哮するまでこのシーケンスを行う
  case 2:
    {
      // 自機ドラゴンの咆哮
#ifdef PM_DEBUG
      if( end_frame == (p_wk->event_frame + debug_dragon_bark_value) )
#else
      if( end_frame == (p_wk->event_frame + DRA_BARK_FRAME_HERO) )
#endif
      {
        // 咆える音再生
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_BARK );
        p_wk->bark_seq++;
        ONOUE_Printf("ドラゴン咆哮：自機ドラゴン咆哮音の再生\n");
      }

      // 敵ドラゴンの咆哮
      if( end_frame == (p_wk->event_frame + DRA_BARK_FRAME_ENEMY) )
      {
        // 咆える音再生
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_BARK );
        p_wk->bark_seq++;
        ONOUE_Printf("ドラゴン咆哮：敵ドラゴン咆哮音の再生\n");
      }
    }
    break;
  case 3:
    {
      // 移動開始音再生
      if( end_frame == (p_wk->event_frame + DRA_BARK_MOVE_START_SE_FRAME) )
      {
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE_START );
        p_wk->bark_seq++;
        ONOUE_Printf("ドラゴン咆哮：移動開始音の再生\n");
      }
    }
    break;
  case 4:
    {
      if( PMSND_CheckPlaySE() == FALSE )
      {
        // 移動中音を再生
        PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE );
        p_wk->bark_seq++;
        ONOUE_Printf("ドラゴン咆哮：移動中音の再生\n");
      }
    }
    break;
  }
}
 
//----------------------------------------------------------------------------
/**
 *	@brief  終了フレームの取得
 *
 *	@param	*p_md 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 get_anmend_frame( TRACE_MOVE_DATA *p_md )
{
  // 逆再生を考慮する
  if( p_md->is_reverse_move == FALSE )
  {
    return p_md->p_trace_data->end_frame - p_md->p_trace_data->start_frame;
  }
  else
  {
    return p_md->p_trace_data->start_frame - p_md->p_trace_data->end_frame;
  }

  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動イベント呼び出し(ドラゴン咆哮時)
 *
 *	@param	*p_fld_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CallShakeCamera_Bark( FIELDMAP_WORK *p_fld_wk )
{
  FIELD_TASK* cam;
  FIELD_TASK_MAN* taskMan;

  CAM_SHAKE_PARAM shakeParam;
  GFL_STD_MemFill(&shakeParam, 0, sizeof(shakeParam)); // パラメータの初期化
  shakeParam.Width           = DRA_BARK_CAMERA_SHAKE_W;
  shakeParam.Height          = DRA_BARK_CAMERA_SHAKE_H;
  shakeParam.Sync            = DRA_BARK_CAMERA_SHAKE_SYNC;
  shakeParam.Time            = DRA_BARK_CAMERA_SHAKE_TIME;
  shakeParam.ShakeType       = CAM_SHAKE_TYPE_TARGET_POS;

  cam = FIELD_TASK_ShakeCamera( p_fld_wk, &shakeParam );
  taskMan = FIELDMAP_GetTaskManager( p_fld_wk );
  FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );

  ONOUE_Printf("カメラゆれ再生（ドラゴン咆哮）\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動イベント呼び出し(衝突時)
 *
 *	@param	*p_fld_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CallShakeCamera_Bump( FIELDMAP_WORK *p_fld_wk )
{
  FIELD_TASK* cam;
  FIELD_TASK_MAN* taskMan;

  CAM_SHAKE_PARAM shakeParam;
  GFL_STD_MemFill(&shakeParam, 0, sizeof(shakeParam)); // パラメータの初期化
  shakeParam.Width           = DRA_BUMP_CAMERA_SHAKE_W;
  shakeParam.Height          = DRA_BUMP_CAMERA_SHAKE_H;
  shakeParam.Sync            = DRA_BUMP_CAMERA_SHAKE_SYNC;
  shakeParam.Time            = DRA_BUMP_CAMERA_SHAKE_TIME;
  shakeParam.ShakeType       = CAM_SHAKE_TYPE_TARGET_POS;

  cam = FIELD_TASK_ShakeCamera( p_fld_wk, &shakeParam );
  taskMan = FIELDMAP_GetTaskManager( p_fld_wk );
  FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );
  
  ONOUE_Printf("カメラゆれ再生（ドラゴン衝突）\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動終了エフェクト再生メイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_Event_StopEffectMain( PLAYER_MOVE_WORK *p_wk )
{
  // 自機の追従フレームを基準にする
  u32 end_frame;
  TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_PLAYER];
  const BUTTON_EVENT_DATA *p_bev = p_wk->p_button_evdata;

  // アニメーション終了フレームの取得
  end_frame = get_anmend_frame( p_md );

  // 衝突判定
  if( p_bev->is_bump == FALSE )
  {
    // 移動停止音
    if( end_frame == ( p_wk->event_frame + DRA_STOP_SE_FRAME ) )
    {
      PMSND_StopSE();
      PMSND_PlaySE( GYM_DRAGON_SE_SEQ_MOVE_STOP );
      ONOUE_Printf("停止音の再生\n");
    }
  }
  else
  {
    // 衝突音再生
    if( end_frame == ( p_wk->event_frame + DRA_BUMP_SE_FRAME ) )
    {
      PMSND_StopSE();
      PMSND_PlaySE( GYM_DRAGON_SE_SEQ_BUMP );
      ONOUE_Printf("衝突音の再生\n");
    }

    // カメラゆれ
    if( end_frame == ( p_wk->event_frame + DRA_BUMP_CAMERA_SHAKE_FRAME ) )
    {
      CallShakeCamera_Bump( p_wk->p_fld_wk );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションフレームの更新
 *
 *	@param	*p_md 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UpdateFrame( TRACE_MOVE_DATA *p_md )
{
  if( p_md->is_reverse_move == FALSE )
  {
    // 通常再生
    p_md->move_data_frame++;
    if(  p_md->p_trace_data->end_frame <= p_md->move_data_frame )
    {
      p_md->is_move_end = TRUE;
    }
  }
  else
  {
    // 逆再生
    p_md->move_data_frame--;
    if(  p_md->p_trace_data->end_frame >= p_md->move_data_frame )
    {
      p_md->is_move_end = TRUE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  敵トレーナー移動イベント
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MOVE_Event_UpdateEnemyTrainerPos( PLAYER_MOVE_WORK *p_wk )
{
  int i;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  TRACE_MOVE_DATA *p_md = &p_wk->move_data[ICA_MOVE_ID_ENEMY];
  const BUTTON_EVENT_DATA *p_bev = p_wk->p_button_evdata;

  if( p_bev->is_enemy_move == TRUE )
  {
    // アニメーション停止
    if( p_md->is_move_end == TRUE )
    {
      for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
      {
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_md->p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_md->p_model->modelID, i );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
      }
    }

    // 移動処理
    if( p_md->is_move_end == FALSE )
    {
      VecFx32 vec = {0,0,0};
      if ( ICA_ANIME_GetTranslateAt( p_md->p_ica_move_data, &vec, p_md->move_data_frame + 1 ) ) // 0フレーム目はスタート位置なのでスキップ
      {
        MtxFx43 trans;
        MODEL *p_model    = p_md->p_model;

        // 実座標の設定
        vec.x += sc_mdl_offset_table[p_md->model_id].x;
        vec.y += sc_mdl_offset_table[p_md->model_id].y;
        vec.z += sc_mdl_offset_table[p_md->model_id].z;
        MMDL_SetVectorPos( p_md->p_mmdl ,&vec );

        // グリッド座標の設定
        MMDL_SetGridPosX( p_md->p_mmdl ,FX32_TO_GRID(vec.x) );
        MMDL_SetGridPosY( p_md->p_mmdl ,FX32_TO_GRID(vec.y) );
        MMDL_SetGridPosZ( p_md->p_mmdl ,FX32_TO_GRID(vec.z) );

        // フレームの更新
        UpdateFrame( p_md );
      }
    }
  }
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

  // 表示するべきボタン状況を取得し、その状況に合わせてフラグを操作
  {
    FLOOR_LAYER   now_layer = p_wk->p_sv_work->now_floor_layer;
    GYM_LOCATION  now_loc   = p_wk->p_sv_work->now_location;
    button_seq = GetNowButtonType( p_wk->p_event_wk, now_layer, now_loc );
    UpdateButtonStatus( button_seq ,p_wk->p_event_wk);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン点灯イベントメイン
 *
 *	@param	p_event
 *	@param	p_seq
 *	@param	p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ButtonTurnOn_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  fx32 anm_start_frame;
  fx32 anm_end_frame;
  BUTTON_TURN_ON_EVENT_WK *p_wk = p_wk_adrs;
  MODEL *p_model                = &p_wk->p_gmkwk->model[GYM_DRAGON_MODELID_HERO_DRAGON];
  EXP_OBJ_ANM_CNT_PTR exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP );

  switch( (*p_seq) )
  {
  case TURNON_EVENT_SEQ_START:
    {
      // 待ち時間のクリア
      p_wk->wait_frame = 0;

      // ボタンの点灯する音
      PMSND_PlaySE( GYM_DRAGON_SE_SEQ_SWITCH_TURNON );

      // アニメーションを有効に
      {
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_ITP, FALSE);
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP, TRUE);
      }

      // 点灯アニメーション格納アニメーションを取得
      anm_start_frame = FX32_CONST(GYM_BUTTON_EVENT);
      if( p_wk->layer != FLOOR_LAYER_TAIL )
      {
        // 1階以外はパターン２のアニメーションを使用する
        anm_start_frame += FX32_CONST( TURNON_EVENT_WAIT_FRAME1 );
      }

      // 点灯アニメーションの再生
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,anm_start_frame );
      FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );

      (*p_seq) = TURNON_EVENT_SEQ_SE_START_WAIT;
    }
    break;
  case TURNON_EVENT_SEQ_SE_START_WAIT:
    {
      p_wk->wait_frame++;
      
      // アニメーション終了フレームの取得
      if( p_wk->layer != FLOOR_LAYER_TAIL )
      {
        anm_end_frame = TURNON_EVENT_WAIT_FRAME1;
      }
      else
      {
        anm_end_frame = TURNON_EVENT_WAIT_FRAME2;
      }

      // アニメーション終了判定
      if( p_wk->wait_frame >= anm_end_frame )
      {
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        (*p_seq) = TURNON_EVENT_SEQ_TURON_NOW_BUTTON;
      }
    }
    break;
  case TURNON_EVENT_SEQ_TURON_NOW_BUTTON:
    {
      // シーンワークを2(イベント再生済み)に設定
      switch(p_wk->layer)
      {
      case 0:
        SetWorkValue( p_wk->p_event_wk ,WK_SCENE_C08GYM_TURN_BTN_01 ,2);
        break;
      case 1:
        SetWorkValue( p_wk->p_event_wk ,WK_SCENE_C08GYM_TURN_BTN_02 ,2);
        break;
      case 2:
        SetWorkValue( p_wk->p_event_wk ,WK_SCENE_C08GYM_TURN_BTN_03 ,2);
        break;
      }

      // IPTのシーケンスを設定する
      SetButtonITPSeq( p_wk->p_gmkwk );

      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

#ifdef PM_DEBUG

//----------------------------------------------------------------------------
/**
 *	@brief  パラメターを操作する
 *
 *	@param	p_value     値代入先
 *	@param	min         最小
 *	@param	max         最大
 *	@param	add_value1  左右キーでの速度
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_ValueUpdate( s32* p_value ,s32 min, s32 max ,s32 add_value1)
{
  BOOL is_change = FALSE;
  int rep = GFL_UI_KEY_GetRepeat();
  
  if( rep & PAD_KEY_RIGHT  )
  {
    (*p_value) += add_value1;
    is_change = TRUE;
  }
  if( rep & PAD_KEY_LEFT  )
  {
    (*p_value) -= add_value1;
    is_change = TRUE;
  }

  // 最大チェック
  (*p_value) %= max;

  // 最小チェック
  if( (*p_value) < min )
  {
    (*p_value) += max;
  }

  return is_change;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ドラゴンの咆哮タイミングを描画
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_DragonBarkFrame( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item ,"DRA_BARK_FRAME_HERO[%d]" ,debug_dragon_bark_value );
}

//----------------------------------------------------------------------------
/**
 *	@brief  咆哮音再生タイミングを調節
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_DragonBarkFrame( void *userWork, DEBUGWIN_ITEM *item )
{
  s32 *p_value = &debug_dragon_bark_value;
  FIELDMAP_WORK *p_fld_wk = userWork;

  if( DebugWin_ValueUpdate( p_value, 0, 9999 ,1 ) )
  {
    // 描画更新
    DEBUGWIN_RefreshScreen();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグのカラーAを描画
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogCollar( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item ,"FOG_COLOR_A[%d]" ,debug_fog_color_a_value );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグの色を調節
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogCollar( void *userWork, DEBUGWIN_ITEM *item )
{
  s32 *p_value = &debug_fog_color_a_value;
  FIELDMAP_WORK *p_fld_wk = userWork;

  if( DebugWin_ValueUpdate( p_value, 0, 256 ,1 ) )
  {
    // 描画更新
    DEBUGWIN_RefreshScreen();
    
    // 反映
    {
      FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_fld_wk );
      FIELD_FOG_SetColorA( fog, (*p_value) );
      FIELD_FOG_Reflect( fog );
    }

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグのかかり具合を描画
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogSlope( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item ,"FOG_SLOPE[%d]" ,debug_fog_slope_value );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグのかかりぐあいを調節
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogSlope( void *userWork, DEBUGWIN_ITEM *item )
{
  s32 *p_value = &debug_fog_slope_value;
  FIELDMAP_WORK *p_fld_wk = userWork;

  if( DebugWin_ValueUpdate( p_value, 0, FIELD_FOG_SLOPE_MAX ,1 ) )
  {
    // 描画更新
    DEBUGWIN_RefreshScreen();
    
    // 反映
    {
      FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_fld_wk );
      FIELD_FOG_SetSlope( fog, (*p_value) );
      FIELD_FOG_Reflect( fog );
    }

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグのオフセットを描画
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogOffset( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item ,"FOG_OFFSET[%d]" ,debug_fog_offset_value );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグのオフセットを調節
 *
 *	@param	*userWork
 *	@param	*item 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogOffset( void *userWork, DEBUGWIN_ITEM *item )
{
  s32 *p_value = &debug_fog_offset_value;
  FIELDMAP_WORK *p_fld_wk = userWork;

  if( DebugWin_ValueUpdate( p_value, 0, 0xFFFFF ,0x10 ) )
  {
    // 描画更新
    DEBUGWIN_RefreshScreen();
    
    // 反映
    {
      FIELD_FOG_WORK  *fog  = FIELDMAP_GetFieldFog( p_fld_wk );
      FIELD_FOG_SetOffset( fog, (*p_value) );
      FIELD_FOG_Reflect( fog );
    }

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ドラゴンジム内デバッグメニュー
 *
 *	@param	*p_wk
 *	@param	*p_fld_wk
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fld_wk, HEAPID heapID )
{
  // デバッグメニューグループ追加
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_DRAGON, "ドラゴンジム", heapID );

  // フォグのカラー
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_FogCollar,
                              DebugWin_Draw_FogCollar,
                              p_fld_wk,
                              DEBUGMENU_GYM_DRAGON,
                              heapID );

  // フォグのかかり具合
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_FogSlope,
                              DebugWin_Draw_FogSlope,
                              p_fld_wk,
                              DEBUGMENU_GYM_DRAGON,
                              heapID );
  
  // フォグのオフセット
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_FogOffset,
                              DebugWin_Draw_FogOffset,
                              p_fld_wk,
                              DEBUGMENU_GYM_DRAGON,
                              heapID );

  // ドラゴンの泣き声
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_DragonBarkFrame,
                              DebugWin_Draw_DragonBarkFrame,
                              p_fld_wk,
                              DEBUGMENU_GYM_DRAGON,
                              heapID );

}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー終了処理
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_DRAGON );
}
#endif

