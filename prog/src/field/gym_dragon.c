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

#include "../../../resource/fldmapdata/eventdata/tmp/c08gym0101.h"  // obj_id
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_trace_data_id.h"  // データID
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_trace_data.cdat"  // sc_hero_move_data
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // SYS_FLAG_C08GYM_BUTTON_～
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_height_data.cdat"  // sc_ex_height_data
#include "../../../resource/fldmapdata/gimmick/gym_dragon/gym_dragon_gym_data_def.h"  // 
#include "gym_dragon.cdat"

#include "eventwork_def.h"  // LOCALWORK*

#include "tr_tool/trno_def.h" // TRID_*
#include "script_trainer.h"

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
  MODEL       model[MODELID_MAX]; // モデル
  
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
  BOOL          is_bark;

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
static void UpdateButtonStatus( GYM_BUTTON_SEQ button_seq ,EVENTWORK* p_event_wk );
static void SetWorkValue( EVENTWORK* p_event_wk ,u16 work_no ,u16 value );
static void SetExHeight( FIELDMAP_WORK* p_fld_wk );
static void SetButtonITPSeq( GYM_DRAGON_TMP *p_wk );
static void SetHeroDragonITPSeq( GYM_DRAGON_TMP *p_wk );
static void UpdateFrame( TRACE_MOVE_DATA *p_md );

#ifdef PM_DEBUG
#define DEBUGMENU_GYM_DRAGON (2)
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fld_wk, HEAPID heapID );
static void DEBUGMENU_Exit( void );

// デバッグパラメータ
static s32 debug_fog_slope_value  = FOG_SLOPE;
static s32 debug_fog_offset_value = FOG_OFFSET;
static s32 debug_fog_color_a_value  = FOG_COLOR_A;
static s32 debug_dragon_bark_value  = DRA_BARK_FRAME;
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

  p_wk->heapID      = HEAPID_FIELDMAP;
  p_wk->heapAnmID   = HEAPID_GYM_DRAGON_ANM;
  p_wk->p_fld_wk    = fieldWork;
  p_wk->p_sv_work   = GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_DRAGON );
  p_wk->p_ex_obj    = FIELDMAP_GetExpObjCntPtr( fieldWork );
  p_wk->p_event_wk  = GAMEDATA_GetEventWork( p_gamedata );

#ifdef PM_DEBUG
  // Rデバッグメニューの初期化
  DEBUGMENU_Init( p_wk ,p_wk->p_fld_wk ,p_wk->heapID);
#endif
 
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
    int i,n;
    EXP_OBJ_ANM_CNT_PTR exobj_anm;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0 ; i < MODELID_MAX ; i++ )
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
        // フレーム
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, n , 0);

        // ループ、再生設定
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, n );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
      }
    }
  }

  // 拡張高さの設定
  SetExHeight( p_wk->p_fld_wk );

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

  // ボタンを点灯状態にする
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
  MODEL *p_model    = &p_wk->model[MODELID_HERO_DRAGON];
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
  MODEL *p_model    = &p_wk->model[MODELID_HERO_DRAGON];
  {
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_ITP, FALSE);
    FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP, TRUE);
  }

   
  OS_Printf("現在のレイヤー：%u\n",p_wk->p_sv_work->now_floor_layer);
  OS_Printf("現在のロケーション：%u\n",p_wk->p_sv_work->now_location);

  // IPTのシーケンスを設定する
  {
    FLOOR_LAYER   now_layer = p_wk->p_sv_work->now_floor_layer;
    GYM_LOCATION  now_loc   = p_wk->p_sv_work->now_location;
    GYM_BUTTON_SEQ button_seq = GetNowButtonType( p_wk->p_event_wk, now_layer, now_loc );

    // フレーム
    FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,button_seq );
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
      for( i = 0; i < MODELID_MAX; i++ )
      {
        MODEL *p_model    = &p_wk->model[i];
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, FALSE );
      }
    }

    // モデル表示
    if( (cont & PAD_BUTTON_DEBUG) && (trg & PAD_BUTTON_SELECT) )
    {
      int i;
      for( i = 0; i < MODELID_MAX; i++ )
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
  case MOVE_EVENT_SEQ_START_SE:
    {
      PMSND_PlaySE( SEQ_SE_SW_DRGGYM_01 );
      (*p_seq) = MOVE_EVENT_SEQ_START_SE_WAIT;
    }
    break;
  case MOVE_EVENT_SEQ_START_SE_WAIT:
    {
      if( PMSND_CheckPlaySE() == FALSE )
      {
        // 移動中音を再生
        PMSND_PlaySE( SEQ_SE_SW_DRGGYM_02 );
        (*p_seq) = MOVE_EVENT_SEQ_INIT;
      }
    }
    break;
  case MOVE_EVENT_SEQ_INIT:
    {
      MOVE_EVENT_Init( p_wk );
      (*p_seq) = MOVE_EVENT_SEQ_MOVE;
    }
    break;
  case MOVE_EVENT_SEQ_MOVE:
    {
      int i;

      // フレームの加算
      p_wk->event_frame++;

      // トレーナー位置のアップデート
      MOVE_Event_UpdatePlayerPos( p_wk );
      MOVE_Event_UpdateEnemyTrainerPos( p_wk );

      // 終了チェック
      for( i = 0; i < ICA_MOVE_ID_MAX ; i++ )
      {
        TRACE_MOVE_DATA *p_md = &p_wk->move_data[i];
        if( p_md->is_move_end == FALSE )
        {
          return GMEVENT_RES_CONTINUE;
        }
      }
      // 全て終わっていたら移動音を止め、停止音を再生
      PMSND_StopSE();
      PMSND_PlaySE( SEQ_SE_SW_DRGGYM_03 );
      (*p_seq) = MOVE_EVENT_SEQ_END_SE_WAIT;
    }
    break;
  case MOVE_EVENT_SEQ_END_SE_WAIT:
    {
      if( PMSND_CheckPlaySE() == FALSE )
      {
        u16 layer2_value = *EVENTWORK_GetEventWorkAdrs( p_wk->p_event_wk, WK_SCENE_C08GYM_TURN_BTN_02 );
        u16 layer3_value = *EVENTWORK_GetEventWorkAdrs( p_wk->p_event_wk, WK_SCENE_C08GYM_TURN_BTN_03 );

        // ボタン起動イベントありの場合
        if( layer2_value == 1 )
        {
          GMEVENT *call_event;
          GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_wk->p_fld_wk );
          call_event = GYM_DRAGON_CreateButtonTurnOnEvent(gsys,1);
          GMEVENT_CallEvent( p_event, call_event );
          (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
        }
        else if( layer3_value == 1 )
        {
          GMEVENT *call_event;
          GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_wk->p_fld_wk );
          call_event = GYM_DRAGON_CreateButtonTurnOnEvent(gsys,2);
          GMEVENT_CallEvent( p_event, call_event );
          (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
        }
        // イベントなしの場合
        else
        {
          PMSND_PlaySE( SEQ_SE_SW_DRGGYM_07 );
          (*p_seq) = MOVE_EVENT_SEQ_BUTTON_SET;
        }
      }
    }
    break;
  case MOVE_EVENT_SEQ_BUTTON_SET:
    {
      SetButtonITPSeq( p_wk->p_gmkwk );
      (*p_seq) = MOVE_EVENT_SEQ_EXIT;
    }
    break;
  case MOVE_EVENT_SEQ_EXIT:
    {
      MOVE_EVENT_UpdateDragonGymData( p_wk );
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
  p_wk->event_frame = 0;
  p_wk->is_bark     = FALSE;
  
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
    p_md->move_data_frame_max = ICA_ANIME_GetMaxFrame( p_md->p_ica_move_data );
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
        MODEL *p_model    = p_md->p_model;
        // フレーム
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
      p_md->move_data_frame_max = ICA_ANIME_GetMaxFrame( p_md->p_ica_move_data );
      p_md->move_data_frame     = p_md->p_trace_data->start_frame;
      p_md->p_mmdl              = MMDLSYS_SearchOBJID( p_wk->p_mmdl_sys ,p_bev->en_obj_id );
      p_md->is_move_end         = FALSE;

      // 逆再生を調べる
      if( p_md->p_trace_data->start_frame < p_md->p_trace_data->end_frame )
      {
        p_md->is_reverse_move = FALSE;
        FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,FX32_ONE );
        OS_Printf("てきどらんごん再生\n");
      }
      else
      {
        p_md->is_reverse_move = TRUE;
        FLD_EXP_OBJ_SetUnitAddFrame( p_md->p_model->p_ex_obj ,GYM_DRAGON_UNIT_IDX ,-FX32_ONE );
        OS_Printf("てきどらんごん逆再生\n");
      }
 
      // 敵アニメーション再生
      for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
      {
        EXP_OBJ_ANM_CNT_PTR exobj_anm;
        
        MODEL *p_model    = p_md->p_model;
        // フレーム
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

  // 移動処理
  if( p_md->is_move_end == FALSE )
  {
    VecFx32 vec = {0,0,0};
    if ( ICA_ANIME_GetTranslateAt( p_md->p_ica_move_data, &vec, p_md->move_data_frame ) )
    {
      MtxFx43 trans;
      MODEL *p_model    = p_md->p_model;
      vec.x += sc_mdl_offset_table[p_md->model_id].x;
      vec.y += sc_mdl_offset_table[p_md->model_id].y;
      vec.z += sc_mdl_offset_table[p_md->model_id].z;
      
      FIELD_PLAYER_SetPos( p_wk->p_player, &vec );

      // フレームの更新
      UpdateFrame( p_md );

      // アニメーション停止
      if( p_md->is_move_end == TRUE )
      {
        for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
        {
          exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i );
          FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        }
        OS_Printf("アニメーション停止\n");
      }
    }

    // ドラゴンの咆哮音
    if( (p_wk->event_seq == GYM_DRAGON_EVENT_SEQ_UP_BUTTON) && 
        (p_wk->p_sv_work->now_floor_layer ==  FLOOR_LAYER_ARM))
    {
#ifdef PM_DEBUG
      if( p_md->p_trace_data->end_frame == (p_wk->event_frame + debug_dragon_bark_value) )
#else
      if( p_md->p_trace_data->end_frame == (p_wk->event_frame + DRA_BARK_FRAME) )
#endif
      {
        PMSND_PlaySE( SEQ_SE_SW_DRGGYM_04 );
        p_wk->is_bark = TRUE;
      }
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
    if(  p_md->move_data_frame_max <= p_md->move_data_frame )
    {
      p_md->is_move_end = TRUE;
    }
    else if(  p_md->p_trace_data->end_frame <= p_md->move_data_frame )
    {
      p_md->is_move_end = TRUE;
    }
  }
  else
  {
    // 逆再生
    p_md->move_data_frame--;
    if(  p_md->move_data_frame_max <= p_md->move_data_frame )
    {
      p_md->is_move_end = TRUE;
    }
    else if(  p_md->p_trace_data->end_frame >= p_md->move_data_frame )
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

  // 移動処理
  if( p_md->is_move_end == FALSE )
  {
    VecFx32 vec = {0,0,0};
    if ( ICA_ANIME_GetTranslateAt( p_md->p_ica_move_data, &vec, p_md->move_data_frame ) )
    {
      MtxFx43 trans;
      MODEL *p_model    = p_md->p_model;
      vec.x += sc_mdl_offset_table[p_md->model_id].x;
      vec.y += sc_mdl_offset_table[p_md->model_id].y;
      vec.z += sc_mdl_offset_table[p_md->model_id].z;
      //MMDL_SetVectorPos( p_md->p_mmdl ,&vec );

      // フレームの更新
      UpdateFrame( p_md );
      
      // アニメーション停止
      if( p_md->is_move_end == TRUE )
      {
        for( i = 0 ; i < sc_g3d_util_obj[p_md->model_id].anmCount ; i++ )
        {
          exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, i );
          FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        }
        OS_Printf("敵アニメーション停止\n");
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
  BUTTON_TURN_ON_EVENT_WK *p_wk = p_wk_adrs;
  MODEL *p_model    = &p_wk->p_gmkwk->model[MODELID_HERO_DRAGON];

  switch( (*p_seq) )
  {
  case TURNON_EVENT_SEQ_START:
    {
      // 待ち時間のクリア
      p_wk->wait_frame = 0;

      // ボタンの点灯する音
      PMSND_PlaySE( SEQ_SE_SW_DRGGYM_07 );
      
      if( p_wk->layer == 0 )
      {
        // 1階
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,GYM_BUTTON_EVENT1 );
      }
      else
      {
        // 2階、3回
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,GYM_BUTTON_EVENT2 );
      }

      (*p_seq) = TURNON_EVENT_SEQ_SE_START_WAIT;
    }
    break;
  case TURNON_EVENT_SEQ_SE_START_WAIT:
    {
      p_wk->wait_frame++;
      if( p_wk->wait_frame >= TURNON_EVENT_WAIT_FRAME )
      {
        (*p_seq) = TURNON_EVENT_SEQ_TURON_NOW_BUTTON;
      }
      OS_Printf("待ちフレーム%u\n",p_wk->wait_frame);
    }
    break;
  case TURNON_EVENT_SEQ_TURON_NOW_BUTTON:
    {
      // IPTのシーケンスを設定する
      FLOOR_LAYER   now_layer = p_wk->p_gmkwk->p_sv_work->now_floor_layer;
      GYM_LOCATION  now_loc   = p_wk->p_gmkwk->p_sv_work->now_location;
      GYM_BUTTON_SEQ button_seq = GetNowButtonType( p_wk->p_event_wk, now_layer, now_loc );

      // フレーム
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, GYM_DRAGON_UNIT_IDX, p_model->modelID, HERO_DRA_ANM_BUTTON_ITP ,button_seq );
    }

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
    return GMEVENT_RES_FINISH;
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
  DEBUGWIN_ITEM_SetNameV( item ,"DRA_BARK_FRAME[%d]" ,debug_dragon_bark_value );
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

