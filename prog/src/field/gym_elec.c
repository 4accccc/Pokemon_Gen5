//======================================================================
/**
 * @file    gym_elec.c
 * @brief   SWAN電気ジム
 * @author  Toru=Nagihashi
 * @date    10.12.08
 */
//======================================================================
//field
#include "fieldmap.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

//script
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c04gym0101_def.h"  //for SCRID_～

//archive
#include "arc/fieldmap/gym_elec.naix" 
#include "../../../resource/fldmapdata/gimmick/gym_denki/gym_elec_local_def.h"

//SE
#include "sound/pm_sndsys.h"
#include "gym_elec_se_def.h"

//mine
#include "gym_elec_sv.h"
#include "gym_elec.h"


//=============================================================================
/**
 *    前方宣言
 */
//=============================================================================
//-------------------------------------
///	モデル管理
//=====================================
typedef struct _MODEL MODEL;

static MODEL * MODEL_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID );
static void MODEL_Exit( MODEL *p_wk );
static void MODEL_Main( MODEL *p_wk );

//-------------------------------------
///	フォグでの影管理
//=====================================
typedef struct 
{
  s32 player_start_pos;
  s32 player_end_pos;
  s32 fog_start_offset;
  s32 fog_end_offset;
}FOG_SHADOW_INIT_SETUP_TBL;

typedef struct _FOG_SHADOW FOG_SHADOW;

static FOG_SHADOW* FOG_SHADOW_Init( FIELD_FOG_WORK* p_fog, const FIELD_PLAYER* cp_player, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl, s32 tbl_max, s32 phase, HEAPID heapID );
static void FOG_SHADOW_Exit( FOG_SHADOW* p_wk );
static void FOG_SHADOW_Main( FOG_SHADOW* p_wk );
static void FOG_SHADOW_FadeOutRequest( FOG_SHADOW* p_wk, s16 sync );
static BOOL FOG_SHADOW_IsFadeEnd( const FOG_SHADOW* cp_wk );
static void FOG_SHADOW_SetPhaseRequest( FOG_SHADOW* p_wk, s32 phase );

//-------------------------------------
///	スポットライト
//=====================================
typedef struct _SPOT_LIGHT SPOT_LIGHT;

static SPOT_LIGHT* SPOT_LIGHT_Init( const MMDLSYS *cp_mmdlsys, HEAPID heapID );
static void SPOT_LIGHT_Exit( SPOT_LIGHT* p_wk );
static void SPOT_LIGHT_Main( SPOT_LIGHT* p_wk );

static void SPOT_LIGHT_SetVisible( SPOT_LIGHT *p_wk, BOOL visible );
static void SPOT_LIGHT_SetTarget( SPOT_LIGHT *p_wk, u16 objID );

//-------------------------------------
///	ショーアップ（カミツレショーアップ演出）
//=====================================
static GMEVENT * SHOWUP_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog );

//-------------------------------------
///	フェードアウト(影がなくなる演出)
//=====================================
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    電気ジムギミック  
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define GYM_ELEC_TMP_ASSIGN_ID  (0)

//-------------------------------------
///	ジム内部中のワーク
//=====================================
typedef struct 
{
  FOG_SHADOW  *p_fog_shadow;
  SPOT_LIGHT  *p_spot_light;
}GYM_ELEC_TMP;

//-------------------------------------
///	private
//=====================================
//ギミック
static GYM_ELEC_SV_WORK * Gym_Elec_GetSaveWork( FIELDMAP_WORK *p_fieldmap );

//=============================================================================
/**
 *    public
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GYM_ELEC_TMP *p_tmp;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  GYM_ELEC_SV_WORK *p_sv = Gym_Elec_GetSaveWork( p_fieldmap );

  //汎用ワーク確保
  p_tmp  = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID, 
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_ELEC_TMP));

}
//--------------------------------------------------------------
/**
 * @brief       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_End(FIELDMAP_WORK *p_fieldmap)
{
  FLD_EXP_OBJ_CNT_PTR p_ex_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  GYM_ELEC_SV_WORK *p_sv = Gym_Elec_GetSaveWork( p_fieldmap );
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);


  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

}

//--------------------------------------------------------------
/**
 * @frief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ELEC_Move(FIELDMAP_WORK *p_fieldmap)
{
  FLD_EXP_OBJ_CNT_PTR p_ex_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  GYM_ELEC_SV_WORK *p_sv = Gym_Elec_GetSaveWork( p_fieldmap );
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

}

//--------------------------------------------------------------
/**
 * @brief   電気ジムSE初期化
 * @param   gsys            ゲームシステムポインタ
 */
//--------------------------------------------------------------
void GYM_ELEC_InitSe( GAMESYS_WORK *p_gamesys )
{
}
//----------------------------------------------------------------------------
/**
 *	@brief  ショーアップイベント作成
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステム
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_ELEC_CreateShowupEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_ELEC_SV_WORK *p_sv = Gym_Elec_GetSaveWork( p_fieldmap );
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

 // return SHOWUP_CreateEvent( p_gamesys, FOG_SHADOW *p_fog );
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  影フェードアウトイベント作成
 *
 *	@param	GAMESYS_WORK *gsys ゲームシステム
 *	@param  phase 影のフェイズ
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_ELEC_CreateFadeoutEvent( GAMESYS_WORK *p_gamesys, s16 phase )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_ELEC_SV_WORK *p_sv = Gym_Elec_GetSaveWork( p_fieldmap );
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

  //return FADEOUT_CreateEvent( p_gamesys, FOG_SHADOW *p_fog );
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト操作
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	is_visible  TRUE表示ON FALSE表示OFF
 *	@param  objID       スポットライトを当てるOBJID
 */
//-----------------------------------------------------------------------------
void GYM_ELEC_SetSpotLight( FIELDMAP_WORK *p_fieldmap, BOOL is_visible, u16 objID )
{
}
//=============================================================================
/**
 *  ギミックプライベート
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ギミックセーブワークを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return ギミックセーブワーク
 */
//-----------------------------------------------------------------------------
static GYM_ELEC_SV_WORK * Gym_Elec_GetSaveWork( FIELDMAP_WORK *p_fieldmap )
{
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( p_fieldmap ) );
  GIMMICKWORK *p_gmkwork = GAMEDATA_GetGimmickWork(p_gamedata);
  return GIMMICKWORK_Get( p_gmkwork, FLD_GIMMICK_GYM_ELEC );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    モデル管理
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define MODEL_FLD_EXP_OBJ_UNIT_ID (0)

//-------------------------------------
///	ワーク
//=====================================
struct _MODEL
{
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
};
//-------------------------------------
///	データ
//=====================================
static const GFL_G3D_UTIL_SETUP Setup = {
  0
  /*
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
  */
};

//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  作成
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR p_ex_obj  拡張OBJシステム
 *	@param	heapID  ヒープID 
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static MODEL * MODEL_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID )
{
  MODEL *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(MODEL));
  p_wk->p_ex_obj  = p_ex_obj;

  FLD_EXP_OBJ_AddUnit(p_wk->p_ex_obj, &Setup, MODEL_FLD_EXP_OBJ_UNIT_ID );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  破棄
 *
 *	@param	MODEL *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MODEL_Exit( MODEL *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MODEL_FLD_EXP_OBJ_UNIT_ID );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  メイン処理
 *
 *	@param	MODEL *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MODEL_Main( MODEL *p_wk )
{
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    フォグでの影
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	ワーク
//=====================================
struct _FOG_SHADOW
{
  FIELD_FOG_WORK* p_fog;
  const FIELD_PLAYER* cp_player;

  BOOL  is_fog_move;
  s32   phase;
  u8    fog_table[FIELD_FOG_TBL_MAX];

  BOOL  is_fadeout;
  s16   fadeout_sync_now;
  s16   fadeout_sync_max;

  BOOL  is_next_phase;
  FOG_SHADOW_INIT_SETUP_TBL now_move_info;

  const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl;
  s32 tbl_max;
};

//-------------------------------------
///	data
//=====================================
//フォグ初期濃度
static const u8 sc_start_fog_table[FIELD_FOG_TBL_MAX]  =
{
  /*
  0x7,0x9,0xb,0xe,0x10,0x12,0x15,0x17,
  0x1a,0x1d,0x20,0x23,0x26,0x29,0x2c,0x2e,
  0x31,0x34,0x37,0x3a,0x3d,0x40,0x43,0x47,
  0x4b,0x50,0x54,0x59,0x5d,0x61,0x66,0x67,
  */

  0x15,0x17,0x19,0x1b,0x1d,0x1f,0x20,0x22,
  0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,
  0x34,0x36,0x39,0x3b,0x3d,0x3f,0x41,0x43,
  0x46,0x48,0x4b,0x4d,0x4f,0x53,0x58,0x5a,
};

//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  作成
 *
 *  @param  p_fog   フォグ管理システム
 *  @param  cp_player プレイヤー情報
 *	@param	const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl 初期化構造体(メモリ勿体ないのでアドレスのみしか保存しません)
 *	@param	tbl_max 構造体の数
 *	@param	phase   開始フェーズ
 *	@param	heapID  ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static FOG_SHADOW* FOG_SHADOW_Init( FIELD_FOG_WORK* p_fog, const FIELD_PLAYER* cp_player, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl, s32 tbl_max, s32 phase, HEAPID heapID )
{
  FOG_SHADOW  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FOG_SHADOW) );

  //使用モジュールの登録
  p_wk->p_fog     = p_fog;
  p_wk->cp_player = cp_player;

  //設定情報の登録
  p_wk->cp_tbl  = cp_tbl;
  p_wk->tbl_max = tbl_max;

  //初期フォグ濃度の設定
  GFL_STD_MemCopy( sc_start_fog_table, p_wk->fog_table, sizeof(u8)*FIELD_FOG_TBL_MAX );

  //フォグ初期設定
  FIELD_FOG_SetBlendMode( p_wk->p_fog, FIELD_FOG_BLEND_COLOR_ALPHA );
  FIELD_FOG_SetSlope( p_wk->p_fog, FIELD_FOG_SLOPE_0x0020 );
  FIELD_FOG_SetOffset( p_wk->p_fog, 0 );
  FIELD_FOG_SetColorRgb( p_wk->p_fog, GX_RGB( 0,0,0 ) );
  FIELD_FOG_SetTblAll( p_wk->p_fog, p_wk->fog_table );
  FIELD_FOG_SetFlag( p_wk->p_fog, TRUE );

  //フォグ動作開始
  p_wk->is_fog_move = TRUE;

  //現在のフェーズを設定
  FOG_SHADOW_SetPhaseRequest( p_wk, phase );

  //設定を反映させるために一度呼ぶ
  FOG_SHADOW_Main( p_wk );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  破棄
 *
 *	@param	FOG_SHADOW* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_Exit( FOG_SHADOW* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  メイン処理
 *
 *	@param	FOG_SHADOW* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_Main( FOG_SHADOW* p_wk )
{
  //フォグの位置が切り替わる処理
  if( p_wk->is_next_phase )
  {
    p_wk->now_move_info = p_wk->cp_tbl[ p_wk->phase ];
    p_wk->is_next_phase   = FALSE;
  }

  //フォグがプレイヤーに合わせて動く処理
  if( p_wk->is_fog_move )
  {
    const FOG_SHADOW_INIT_SETUP_TBL *cp_info  = &p_wk->now_move_info;

    VecFx32 target_pos;
    s32 offset;

    FIELD_PLAYER_GetPos( p_wk->cp_player, &target_pos );
    
    offset  = cp_info->fog_start_offset
      - ( cp_info->player_start_pos - (target_pos.x >> FX32_SHIFT))
      * (cp_info->fog_start_offset - cp_info->fog_end_offset)
      / (cp_info->player_start_pos - cp_info->player_end_pos);

    FIELD_FOG_SetOffset( p_wk->p_fog, offset );
  }

  //フォグのフェードアウト処理
  if( p_wk->is_fadeout )
  {
    int i;
    s16 fog;
    for( i = 0; i< FIELD_FOG_TBL_MAX; i++ )
    {
      fog = sc_start_fog_table[i]
        - sc_start_fog_table[ i] * p_wk->fadeout_sync_now / p_wk->fadeout_sync_max;
      p_wk->fog_table[i]  = MATH_CLAMP( fog, 0, 255 );
    }
    
    FIELD_FOG_SetTblAll( p_wk->p_fog, p_wk->fog_table );
    if( p_wk->fadeout_sync_now++ > p_wk->fadeout_sync_max )
    {
      p_wk->is_fadeout  = FALSE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フェードアウトリクエスト
 *
 *	@param	FOG_SHADOW* p_wk ワーク
 *	@param  sync        フェードアウトにかかるシンク
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_FadeOutRequest( FOG_SHADOW* p_wk, s16 sync )
{
  p_wk->is_fadeout        = TRUE;
  p_wk->fadeout_sync_now  = 0;
  p_wk->fadeout_sync_max  = sync;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フェードアウト終了検知
 *
 *	@param	const FOG_SHADOW* p_wk ワーク
 *
 *	@return TRUEフェード終了  FALSEフェード中
 */
//-----------------------------------------------------------------------------
static BOOL FOG_SHADOW_IsFadeEnd( const FOG_SHADOW* cp_wk )
{
  return cp_wk->is_fadeout == FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  次フェーズへの進行リクエスト
 *
 *	@param	FOG_SHADOW* p_wk  ワーク
 *	@param	phase   次フェーズ番号
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_SetPhaseRequest( FOG_SHADOW* p_wk, s32 phase )
{
  GF_ASSERT( phase < p_wk->tbl_max );
  p_wk->is_next_phase = TRUE;
  p_wk->phase         = phase;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    スポットライト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	ワーク
//=====================================
struct _SPOT_LIGHT
{
  const MMDLSYS *cp_mmdlsys;
  MMDL * p_target;
};

//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  作成
 *
 *	@param	MMDLSYS *p_mmdlsys  モデルシステム
 *	@param	heapID              ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT* SPOT_LIGHT_Init( const MMDLSYS *cp_mmdlsys, HEAPID heapID )
{
  SPOT_LIGHT  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(SPOT_LIGHT) );
  p_wk->cp_mmdlsys  = cp_mmdlsys;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  破棄
 *
 *	@param	SPOT_LIGHT* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_Exit( SPOT_LIGHT* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  メイン処理
 *
 *	@param	SPOT_LIGHT* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_Main( SPOT_LIGHT* p_wk )
{
  if( p_wk->p_target != NULL )
  {
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  表示設定
 *
 *	@param	SPOT_LIGHT *p_wk  ワーク
 *	@param	visible           TRUE表示ON　  FALSE表示OFF
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SetVisible( SPOT_LIGHT *p_wk, BOOL visible )
{
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  スポットライトをあてるOBJIDを設定
 *
 *	@param	SPOT_LIGHT *p_wk  ワーク
 *	@param	objID             スポットライトを当てるターゲットOBJID
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SetTarget( SPOT_LIGHT *p_wk, u16 objID )
{
  p_wk->p_target  = MMDLSYS_SearchOBJID( p_wk->cp_mmdlsys, objID );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    ショーアップ
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	ワーク
//=====================================
typedef struct
{
  FOG_SHADOW * p_fog;
}SHOWUP_EVENT_WORK;

//-------------------------------------
///	private
//=====================================
static GMEVENT_RESULT Showup_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *	@brief  ショーアップイベント作成
 *
 *	@param	GAMESYS_WORK *p_gamesys イベント作成用ゲームシステム
 *	@param	*p_fog                  フォグ影システム
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * SHOWUP_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog )
{
  GMEVENT * p_event = GMEVENT_Create( p_gamesys, NULL, Showup_Event, sizeof(SHOWUP_EVENT_WORK) );
  SHOWUP_EVENT_WORK *p_wk = GMEVENT_GetEventWork(p_event);

  p_wk->p_fog = p_fog;

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ショーアップ  メインイベント
 *
 *	@param	GMEVENT* p_event  イベント
 *	@param	p_seq       シーケンス
 *	@param	p_wk_adrs   ワーク
 *
 *	@return イベント終了値
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT Showup_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  enum
  {
    SEQ_,
  };

  SHOWUP_EVENT_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case 0:
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    フェードアウト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	ワーク
//=====================================
typedef struct
{
  FOG_SHADOW * p_fog;
}FADEOUT_EVENT_WORK;

//-------------------------------------
///	private
//=====================================
static GMEVENT_RESULT Fadeout_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs );


//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウトイベント
 *
 *	@param	GAMESYS_WORK *p_gamesys ゲームシステム
 *	@param	*p_fog  フォグ影
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog )
{
  GMEVENT * p_event = GMEVENT_Create( p_gamesys, NULL, Showup_Event, sizeof(FADEOUT_EVENT_WORK) );
  FADEOUT_EVENT_WORK *p_wk = GMEVENT_GetEventWork(p_event);

  p_wk->p_fog = p_fog;

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト  メインイベント
 *
 *	@param	GMEVENT* p_event  イベント
 *	@param	p_seq       シーケンス
 *	@param	p_wk_adrs   ワーク
 *
 *	@return イベント終了値
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT Fadeout_Event( GMEVENT* p_event, int* p_seq, void* p_wk_adrs )
{
  enum
  {
    SEQ_,
  };

  FADEOUT_EVENT_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case 0:
    break;
  }

  return GMEVENT_RES_CONTINUE;
}
