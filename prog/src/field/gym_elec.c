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

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_ELEC_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *    クラス前方宣言
 */
//=============================================================================
//-------------------------------------
///	モデル管理
//=====================================
typedef enum
{
  MODELID_SPOTLIGHT_START,
  MODELID_SPOTLIGHT_00  = MODELID_SPOTLIGHT_START,
  MODELID_SPOTLIGHT_01,
  MODELID_SPOTLIGHT_02,
  MODELID_SPOTLIGHT_END = MODELID_SPOTLIGHT_02,

  MODELID_MAX,

  MODELID_SPOTLIGHT_NUM = MODELID_SPOTLIGHT_END - MODELID_SPOTLIGHT_START + 1,
}MODELID;

typedef struct _MODEL_MANAGER MODEL_MANAGER;
typedef struct _MODEL MODEL;

static MODEL_MANAGER * MODEL_MANAGER_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID );
static void MODEL_MANAGER_Exit( MODEL_MANAGER *p_wk );
static void MODEL_MANAGER_Main( MODEL_MANAGER *p_wk );

static MODEL * MODEL_MANAGER_GetModel( MODEL_MANAGER *p_wk, MODELID modelID );
static void MODEL_SetVisible( MODEL *p_wk, BOOL is_visible );
static void MODEL_SetTrans( MODEL *p_wk, const VecFx32 * cp_trans );

//-------------------------------------
///	フォグでの影管理
//=====================================
#define FOG_SHADOW_FADE_SYNC  (60)

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
static void FOG_SHADOW_SetVisible( FOG_SHADOW* p_wk, BOOL is_visible );

static void FOG_SHADOW_DEBUG_SetSetupTbl( FOG_SHADOW *p_wk, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl );
static void FOG_SHADOW_DEBUG_SetMoveFlag( FOG_SHADOW *p_wk, BOOL is_move );
static void FOG_SHADOW_DEBUG_GetSetupTbl( const FOG_SHADOW *cp_wk, FOG_SHADOW_INIT_SETUP_TBL *p_tbl );

//-------------------------------------
///	スポットライト
//=====================================
typedef struct _SPOT_LIGHT_SET SPOT_LIGHT_SET;

static SPOT_LIGHT_SET* SPOT_LIGHT_SET_Init( u16 light_max, const MMDLSYS *cp_mmdlsys, MODEL_MANAGER *p_model_man, u16 start_idx, HEAPID heapID );
static void SPOT_LIGHT_SET_Exit( SPOT_LIGHT_SET* p_wk );
static void SPOT_LIGHT_SET_Main( SPOT_LIGHT_SET* p_wk );

static void SPOT_LIGHT_SET_Register( SPOT_LIGHT_SET *p_wk, u16 objID );
static void SPOT_LIGHT_SET_UnRegister( SPOT_LIGHT_SET *p_wk, u16 objID );

//-------------------------------------
///	ショーアップ（カミツレショーアップ演出）
//=====================================
static GMEVENT * SHOWUP_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow );

//-------------------------------------
///	フェードアウト(影がなくなる演出)
//=====================================
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow, s16 phase );

//-------------------------------------
///	セーブ情報
//=====================================
//GYM_ELEC_SV_WORK
static void GYM_ELEC_SV_SetEventSeq( GYM_ELEC_SV_WORK *p_wk, u16 seq );
static u16 GYM_ELEC_SV_GetEventSeq( const GYM_ELEC_SV_WORK *cp_wk );

#ifdef GYM_ELEC_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_tmp_adrs, HEAPID heapID );
static void DEBUGMENU_Exit( void );
#endif //GYM_ELEC_DEBUG_MENU

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
  FOG_SHADOW      *p_fog_shadow;  //フォグ影
  SPOT_LIGHT_SET      *p_spot_light;  //スポットライト管理
  MODEL_MANAGER   *p_model_man;   //モデル管理
  ISS_SWITCH_SYS  *p_iss_switch;  //スイッチiss
  GYM_ELEC_SV_WORK *p_sv;         //ギミックセーブワーク
}GYM_ELEC_TMP;

//-------------------------------------
///	private
//=====================================
static GYM_ELEC_SV_WORK * Gym_Elec_GetSaveWork( FIELDMAP_WORK *p_fieldmap );
static void Gym_Elec_SetEnvironment( GYM_ELEC_TMP *p_tmp, const GYM_ELEC_SV_WORK *cp_sv );
//=============================================================================
/**
 *    data
 */
//=============================================================================
//フォグの設定テーブル
static const FOG_SHADOW_INIT_SETUP_TBL  sc_fogshadow_init_tbl[] =
{
  {
    968,
    872,
    32647,
    32624,
  },
  {
    872,
    600,
    32690,
    32631,
  },
  {
    456,
    328,
    32661,
    32628,
  },
};

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
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);

  //汎用ワーク確保
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID, 
      heapID, sizeof(GYM_ELEC_TMP));

  //ギミックセーブワークを保存
  p_tmp->p_sv = Gym_Elec_GetSaveWork( p_fieldmap );

  //ISSを保存
  {
    ISS_SYS *p_iss  = GAMESYSTEM_GetIssSystem( FIELDMAP_GetGameSysWork(p_fieldmap) );
    p_tmp->p_iss_switch  = ISS_SYS_GetIssSwitchSystem( p_iss );
  }

  //モデル管理作成
  {
    FLD_EXP_OBJ_CNT_PTR p_ex_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
    p_tmp->p_model_man  = MODEL_MANAGER_Init( p_ex_obj, heapID );
  }

  //フォグ影作成
  {
    FIELD_FOG_WORK  *p_fog  = FIELDMAP_GetFieldFog(p_fieldmap);
    const FIELD_PLAYER* cp_player = FIELDMAP_GetFieldPlayer(p_fieldmap);
    p_tmp->p_fog_shadow = FOG_SHADOW_Init( p_fog, cp_player, sc_fogshadow_init_tbl, NELEMS(sc_fogshadow_init_tbl), 0, heapID );
  }

  //スポットライト作成
  {
    const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys(p_fieldmap);
    p_tmp->p_spot_light = SPOT_LIGHT_SET_Init( MODELID_SPOTLIGHT_NUM, cp_mmdlsys, p_tmp->p_model_man, MODELID_SPOTLIGHT_START, heapID );
  }

  //セーブワークの状態に環境を整える
  Gym_Elec_SetEnvironment( p_tmp, p_tmp->p_sv );

#ifdef GYM_ELEC_DEBUG_MENU
  //デバッグ作成
  DEBUGMENU_Init( p_tmp, heapID );
#endif 
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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

#ifdef GYM_ELEC_DEBUG_MENU
  //デバッグ破棄
  DEBUGMENU_Exit();
#endif 
  //スポットライト破棄
  SPOT_LIGHT_SET_Exit( p_tmp->p_spot_light );

  //フォグ影破棄
  FOG_SHADOW_Exit( p_tmp->p_fog_shadow );

  //モデル管理破棄
  MODEL_MANAGER_Exit( p_tmp->p_model_man );

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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

  //フォグ影メイン
  FOG_SHADOW_Main( p_tmp->p_fog_shadow );

  //スポットライトメイン
  SPOT_LIGHT_SET_Main( p_tmp->p_spot_light );

  //モデル管理メイン
  MODEL_MANAGER_Main( p_tmp->p_model_man );

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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

  return SHOWUP_CreateEvent( p_gamesys, p_tmp->p_fog_shadow );
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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);
  u16  event_seq = GYM_ELEC_SV_GetEventSeq( p_tmp->p_sv );

  GF_ASSERT_MSG( phase == 1 || phase == 2, "phaseは1か2じゃないとだめです%d\n",phase );

  //一人目と二人目に勝ったときしかこのイベントは反映しない
  if( event_seq < GYM_ELEC_EVENT_SEQ_TR03_BTL_END )
  {
    //現在のフェイズを保存
    GYM_ELEC_SV_SetEventSeq( p_tmp->p_sv, phase );

    //イベント作成
    return FADEOUT_CreateEvent( p_gamesys, p_tmp->p_fog_shadow, phase );
  }
  else
  {
    return NULL;
  }
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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

  if( is_visible )
  {
    SPOT_LIGHT_SET_Register( p_tmp->p_spot_light, objID );
  }
  else
  {
    SPOT_LIGHT_SET_UnRegister( p_tmp->p_spot_light, objID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  電気ジム  復帰処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------
void GYM_ELEC_Recover( FIELDMAP_WORK *p_fieldmap )
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
//----------------------------------------------------------------------------
/**
 *	@brief  環境設定
 *
 *	@param	GYM_ELEC_TMP *p_tmp       テンポラリワーク
 *	@param	GYM_ELEC_SV_WORK *cp_sv   セーブワーク
 */
//-----------------------------------------------------------------------------
static void Gym_Elec_SetEnvironment( GYM_ELEC_TMP *p_tmp, const GYM_ELEC_SV_WORK *cp_sv )
{
  u16  seq = GYM_ELEC_SV_GetEventSeq( cp_sv );

  switch( seq )
  {
  case GYM_ELEC_EVENT_SEQ_START:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 0 );
    break;
  case GYM_ELEC_EVENT_SEQ_TR01_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 1 );
    ISS_SWITCH_SYS_SwitchOn( p_tmp->p_iss_switch, SWITCH_01 );
    break;
  case GYM_ELEC_EVENT_SEQ_TR02_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 2 );
    ISS_SWITCH_SYS_SwitchOn( p_tmp->p_iss_switch, SWITCH_02 );
    break;
  case GYM_ELEC_EVENT_SEQ_TR03_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    /* カミツレイベントが起動した後になる */
    break;
  case GYM_ELEC_EVENT_SEQ_BOSS_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, FALSE );
    break;
  default:
    GF_ASSERT( 0 );
  }

}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    モデル管理
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID (0)

enum
{
  MODEL_RESID_SPOTLIGHT01_IMD,
  MODEL_RESID_MAX,
};

enum
{
  MODEL_ANMID_MAX,
};

//-------------------------------------
///	ワーク
//=====================================
//モデル操作ワーク
struct _MODEL
{
  s32                 unitID;
  MODELID             modelID;
  EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
};
//モデル管理ワーク
struct _MODEL_MANAGER
{
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  MODEL model[MODELID_MAX];
};
//-------------------------------------
///	データ
//=====================================
//リソース設定
static const GFL_G3D_UTIL_RES sc_g3d_util_res[MODEL_RESID_MAX] =
{
  //MODEL_RESID_SPOTLIGHT01_IMD  トレーナー用スポットライトモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_spot01_nsbmd, GFL_G3D_UTIL_RESARC
  },
};

//アニメ設定
//今はない
/*static const GFL_G3D_UTIL_ANM g3Dutil sc_g3d_util_anm[] = 
{
}*/

//3Dオブジェクト設定
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj[MODELID_MAX] =
{
  //MODELID_SPOTLIGHT_00 トレーナー用スポットライト
  {
    MODEL_RESID_SPOTLIGHT01_IMD, 	//モデルリソースID
		0, 							              //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT01_IMD, 	//テクスチャリソースID
		NULL,			                    //アニメテーブル(複数指定のため)
		0,	                          //アニメリソース数
  },
  //MODELID_SPOTLIGHT_01 トレーナー用スポットライト
  {
    MODEL_RESID_SPOTLIGHT01_IMD, 	//モデルリソースID
		0, 							              //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT01_IMD, 	//テクスチャリソースID
		NULL,			                    //アニメテーブル(複数指定のため)
		0,	                          //アニメリソース数
  },
  //MODELID_SPOTLIGHT_02 トレーナー用スポットライト
  {
    MODEL_RESID_SPOTLIGHT01_IMD, 	//モデルリソースID
		0, 							              //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT01_IMD, 	//テクスチャリソースID
		NULL,			                    //アニメテーブル(複数指定のため)
		0,	                          //アニメリソース数
  },
};

//全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup =
{
  sc_g3d_util_res,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj,  //オブジェクトテーブル
  MODELID_MAX,      //オブジェクト数
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
static MODEL_MANAGER * MODEL_MANAGER_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID )
{
  MODEL_MANAGER *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(MODEL_MANAGER));
  p_wk->p_ex_obj  = p_ex_obj;

  //読込み
  FLD_EXP_OBJ_AddUnit(p_wk->p_ex_obj, &sc_g3d_util_setup, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  
  //モデル初期化
  {
    int i;
    for( i = 0; i < MODELID_MAX; i++ )
    {
      MODEL *p_model    = MODEL_MANAGER_GetModel( p_wk, i );
      p_model->unitID   = MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_ex_obj;
    }
  }


  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  破棄
 *
 *	@param	MODEL_MANAGER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MODEL_MANAGER_Exit( MODEL_MANAGER *p_wk )
{
  //破棄
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  メイン処理
 *
 *	@param	MODEL_MANAGER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MODEL_MANAGER_Main( MODEL_MANAGER *p_wk )
{
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル管理  モデル１つを取得
 *
 *	@param	MODEL_MANAGER *p_wk   管理ワーク
 *	@param	modelID               モデルID
 *
 *	@return モデル
 */
//-----------------------------------------------------------------------------
static MODEL * MODEL_MANAGER_GetModel( MODEL_MANAGER *p_wk, MODELID modelID )
{
  GF_ASSERT( modelID < MODELID_MAX );
  return &p_wk->model[ modelID ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  表示設定
 *
 *	@param	MODEL *p_wk ワーク
 *	@param	is_visible  TRUEで表示FALSEで非表示
 */
//-----------------------------------------------------------------------------
static void MODEL_SetVisible( MODEL *p_wk, BOOL is_visible )
{
  FLD_EXP_OBJ_SetVanish( p_wk->p_ex_obj,
      p_wk->unitID, p_wk->modelID, !is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  位置設定
 *
 *	@param	MODEL *p_wk ワーク
 *	@param	VecFx32 * cp_trans  表示位置
 */
//-----------------------------------------------------------------------------
static void MODEL_SetTrans( MODEL *p_wk, const VecFx32 * cp_trans )
{
  GFL_G3D_OBJSTATUS *p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_ex_obj,
                                                p_wk->unitID,
                                                p_wk->modelID );
  p_status->trans = *cp_trans;
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
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,
  0x68,0x68,0x68,0x68,0x68,0x68,0x68,0x68,
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
  FIELD_FOG_SetSlope( p_wk->p_fog, FIELD_FOG_SLOPE_0x0040 );
  FIELD_FOG_SetOffset( p_wk->p_fog, 0 );
  FIELD_FOG_SetColorRgb( p_wk->p_fog, GX_RGB( 0,0,0 ) );  //色はzonelightで設定される
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
    //フォグ位置切り替え
    p_wk->now_move_info = p_wk->cp_tbl[ p_wk->phase ];

    //濃度をリセット
    GFL_STD_MemCopy( sc_start_fog_table, p_wk->fog_table, sizeof(u8)*FIELD_FOG_TBL_MAX );
    FIELD_FOG_SetTblAll( p_wk->p_fog, p_wk->fog_table );
    p_wk->is_next_phase   = FALSE;
  }

  //フォグがプレイヤーに合わせて動く処理
  if( p_wk->is_fog_move )
  {
    const FOG_SHADOW_INIT_SETUP_TBL *cp_info  = &p_wk->now_move_info;

    VecFx32 target_pos;
    s32 offset;

    FIELD_PLAYER_GetPos( p_wk->cp_player, &target_pos );
    
    //プレイヤー座標からオフセットを算出
    offset  = cp_info->fog_start_offset
      - ( cp_info->player_start_pos - (target_pos.z >> FX32_SHIFT))
      * (cp_info->fog_start_offset - cp_info->fog_end_offset)
      / (cp_info->player_start_pos - cp_info->player_end_pos);

    offset  = MATH_CLAMP( offset, 0, 0x7FFF );

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
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  影の表示設定
 *
 *	@param	FOG_SHADOW* p_wk  ワーク
 *	@param	is_visible        TRUE表示  FALSE非表示
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_SetVisible( FOG_SHADOW* p_wk, BOOL is_visible )
{
  FIELD_FOG_SetFlag( p_wk->p_fog, is_visible );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影デバッグ用  現在の設定テーブルを上書きセット
 *
 *	@param	FOG_SHADOW *p_wk  ワーク
 *	@param	FOG_SHADOW_INIT_SETUP_TBL *cp_tbl 設定テーブル１つ分
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_DEBUG_SetSetupTbl( FOG_SHADOW *p_wk, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl )
{
  p_wk->now_move_info = *cp_tbl;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影デバッグ用  現在の設定テーブルを取得
 *
 *	@param	FOG_SHADOW *p_wk  ワーク
 *	@param	FOG_SHADOW_INIT_SETUP_TBL *p_tbl 設定テーブル１つ分
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_DEBUG_GetSetupTbl( const FOG_SHADOW *cp_wk, FOG_SHADOW_INIT_SETUP_TBL *p_tbl )
{
  *p_tbl  = cp_wk->now_move_info;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影デバッグ用  フォグ影動作フラグを設定
 *
 *	@param	FOG_SHADOW *p_wk  ワーク
 *	@param	is_move   TRUEで動作（デフォルト）  FALSEで動作しない
 */
//-----------------------------------------------------------------------------
static void FOG_SHADOW_DEBUG_SetMoveFlag( FOG_SHADOW *p_wk, BOOL is_move )
{
  p_wk->is_fog_move = is_move;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    スポットライト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
typedef struct
{
  const MMDL    *cp_target;
  MODEL         *p_model;
  BOOL          is_use;
} SPOT_LIGHT;

//-------------------------------------
///	ワーク
//=====================================
struct _SPOT_LIGHT_SET
{
  const MMDLSYS *cp_mmdlsys;
  u16 search_idx;
  u16 light_max;
  SPOT_LIGHT  light[0];
};

//-------------------------------------
///	private
//=====================================
//スポットライトを取得
static SPOT_LIGHT * Spot_Light_Set_SearchByObjID( SPOT_LIGHT_SET *p_wk, u16 objID );
static SPOT_LIGHT * Spot_Light_Set_SearchEmpty( SPOT_LIGHT_SET *p_wk );

//スポットライト操作
static void Spot_Light_Init( SPOT_LIGHT *p_wk, MODEL *p_model );
static void Spot_Light_Main( SPOT_LIGHT *p_wk );
static void Spot_Light_Register( SPOT_LIGHT *p_wk, const MMDL *cp_target );
static void Spot_Light_UnRegister( SPOT_LIGHT *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  作成
 *  
 *  @param  light_max       ライト最大数
 *	@param	MMDLSYS         *p_mmdlsys  モデルシステム
 *	@param  MODEL_MANAGER   *p_model_man  ギミック用モデル管理
 *	@param  start_idx       ギミック用モデルに登録されているライトの開始位置（そこからライト最大数分読み込みます）
 *	@param	heapID          ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT_SET* SPOT_LIGHT_SET_Init( u16 light_max, const MMDLSYS *cp_mmdlsys, MODEL_MANAGER *p_model_man, u16 start_idx, HEAPID heapID )
{
  SPOT_LIGHT_SET  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(SPOT_LIGHT_SET) + sizeof(SPOT_LIGHT)*light_max );
  p_wk->cp_mmdlsys  = cp_mmdlsys;
  p_wk->light_max   = light_max;
  {
    int i;
    for( i = 0; i < light_max; i++ )
    {
      SPOT_LIGHT *p_light  = &p_wk->light[i];
      MODEL *p_model  = MODEL_MANAGER_GetModel( p_model_man, MODELID_SPOTLIGHT_00+i );
      Spot_Light_Init( p_light, p_model );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  破棄
 *
 *	@param	SPOT_LIGHT_SET* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_Exit( SPOT_LIGHT_SET* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  メイン処理
 *
 *	@param	SPOT_LIGHT_SET* p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_Main( SPOT_LIGHT_SET* p_wk )
{
  int i;
  for( i = 0; i < p_wk->light_max; i++ )
  {
    SPOT_LIGHT *p_light  = &p_wk->light[i];
    Spot_Light_Main( p_light );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  登録
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *	@param	objID   オブジェクトID
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_Register( SPOT_LIGHT_SET *p_wk, u16 objID )
{
  SPOT_LIGHT *p_light = Spot_Light_Set_SearchByObjID( p_wk, objID );

  //登録済みがなかったら登録
  if( p_light == NULL )
  {
    p_light = Spot_Light_Set_SearchEmpty( p_wk );

    Spot_Light_Register( p_light, MMDLSYS_SearchOBJID( p_wk->cp_mmdlsys, objID ) );
  }
  else
  {
    GF_ASSERT_MSG( 0, "ライトの数が足りません。現在の最大=%d", p_wk->light_max );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  解除
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *	@param	objID   オブジェクトID
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_UnRegister( SPOT_LIGHT_SET *p_wk, u16 objID )
{
  SPOT_LIGHT *p_light = Spot_Light_Set_SearchByObjID( p_wk, objID );

  //登録済みがあったら登録解除
  if( p_light != NULL )
  {
    Spot_Light_UnRegister( p_light );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  objIDからスポットライトを取得
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *	@param	objID   オブジェクトID
 *
 *	@return スポットライト
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT * Spot_Light_Set_SearchByObjID( SPOT_LIGHT_SET *p_wk, u16 objID )
{
  int i;
  for( i = 0; i < p_wk->light_max; i++ )
  {
    SPOT_LIGHT  *p_light  = &p_wk->light[i];
    if( p_light->cp_target != NULL && MMDL_GetOBJID( p_light->cp_target ) == objID )
    {
      return p_light;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  空のスポットライトを取得
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *
 *	@return スポットライト
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT * Spot_Light_Set_SearchEmpty( SPOT_LIGHT_SET *p_wk )
{
  int i;
  for( i = 0; i < p_wk->light_max; i++ )
  {
    SPOT_LIGHT  *p_light  = &p_wk->light[i];
    if( p_light->is_use == FALSE )
    {
      return p_light;
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライト初期化
 *
 *	@param	SPOT_LIGHT *p_wk  ワーク
 *	@param	*p_model          ライトで使用するモデル
 */
//-----------------------------------------------------------------------------
static void Spot_Light_Init( SPOT_LIGHT *p_wk, MODEL *p_model )
{
  GFL_STD_MemClear( p_wk, sizeof(SPOT_LIGHT) );
  p_wk->p_model = p_model;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライトメイン処理
 *
 *	@param	SPOT_LIGHT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Spot_Light_Main( SPOT_LIGHT *p_wk )
{
  if( p_wk->cp_target != NULL )
  {
    VecFx32 pos;
    MMDL_GetVectorPos( p_wk->cp_target, &pos );
    MODEL_SetTrans( p_wk->p_model, &pos );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ターゲットを登録
 *
 *	@param	SPOT_LIGHT *p_wk  ワーク
 *	@param	MMDL *cp_target   ターゲット  (NULLでもOK)
 */
//-----------------------------------------------------------------------------
static void Spot_Light_Register( SPOT_LIGHT *p_wk, const MMDL *cp_target )
{
  p_wk->cp_target = cp_target;
  p_wk->is_use  = TRUE;
  MODEL_SetVisible( p_wk->p_model, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  登録解除
 *
 *	@param	SPOT_LIGHT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Spot_Light_UnRegister( SPOT_LIGHT *p_wk )
{
  MODEL_SetVisible( p_wk->p_model, FALSE );
  p_wk->cp_target = NULL;
  p_wk->is_use    = FALSE;
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
  FOG_SHADOW    *p_fog_shadow;
  FIELD_CAMERA  *p_camera;
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
static GMEVENT * SHOWUP_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow )
{
  GMEVENT * p_event = GMEVENT_Create( p_gamesys, NULL, Showup_Event, sizeof(SHOWUP_EVENT_WORK) );
  SHOWUP_EVENT_WORK *p_wk = GMEVENT_GetEventWork(p_event);

  p_wk->p_fog_shadow   = p_fog_shadow;
  {
    FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
    p_wk->p_camera  = FIELDMAP_GetFieldCamera( p_fieldmap );
  }

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
    SEQ_CAMERA_INIT_START,
    SEQ_CAMERA_INIT_WAIT,
    SEQ_CAMERA_GO_START,
    SEQ_CAMERA_GO_WAIT,

    SEQ_CALL_SCRIPT,

    SEQ_FADE_START,
    SEQ_FADE_WAIT,

    SEQ_CAMERA_COME_START,
    SEQ_CAMERA_COME_WAIT,
    SEQ_CAMERA_END,

    SEQ_END,
  };

  SHOWUP_EVENT_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  //カメラ使用初期化
  case SEQ_CAMERA_INIT_START:
    //トレースをOFF
    FIELD_CAMERA_StopTraceRequest(p_wk->p_camera);
    (*p_seq)++;
    break;

  case SEQ_CAMERA_INIT_WAIT:
    if( FIELD_CAMERA_CheckTrace(p_wk->p_camera) == FALSE )
    {
      FIELD_CAMERA_SetRecvCamParam(p_wk->p_camera);
      FIELD_CAMERA_FreeTarget(p_wk->p_camera);
      (*p_seq)++;
    }
    break;

  //カメラカミツレへ
  case SEQ_CAMERA_GO_START:
    {

      FLD_CAM_MV_PARAM param;
      param.Core.AnglePitch = 0x2561;
      param.Core.AngleYaw   = 0x0;
      param.Core.Distance   = 0xe9*FX32_ONE;
      param.Core.TrgtPos.x  = 0xF8*FX32_ONE;
      param.Core.TrgtPos.y  = 0x48*FX32_ONE;
      param.Core.TrgtPos.z  = 0x2b*FX32_ONE;
      param.Chk.Shift = FALSE;
      param.Chk.Pitch = TRUE;
      param.Chk.Yaw   = TRUE;
      param.Chk.Dist  = TRUE;
      param.Chk.Fovy  = FALSE;
      param.Chk.Pos   = TRUE;
      FIELD_CAMERA_SetLinerParam(p_wk->p_camera, &param, 60);
    }
    (*p_seq)++;
    break;

  case SEQ_CAMERA_GO_WAIT:
    if( FIELD_CAMERA_CheckMvFunc(p_wk->p_camera ) == FALSE )
    {
      (*p_seq)++;
    }
    break;

  //カミツレ芝居
  case SEQ_CALL_SCRIPT:
    (*p_seq)++;
    break;
    
  //フェードイベント
  case SEQ_FADE_START:
    FOG_SHADOW_FadeOutRequest( p_wk->p_fog_shadow, FOG_SHADOW_FADE_SYNC );
    (*p_seq)++;
    break;

  case SEQ_FADE_WAIT:
    if( FOG_SHADOW_IsFadeEnd( p_wk->p_fog_shadow ) )
    {
      FOG_SHADOW_SetVisible( p_wk->p_fog_shadow, FALSE );
      (*p_seq)++;
    }
    break;

  //カメラ主人公位置へ
  case SEQ_CAMERA_COME_START:
    {

      FLD_CAM_MV_PARAM_CHK chk;
      chk.Shift = TRUE;
      chk.Pitch = TRUE;
      chk.Yaw = TRUE;
      chk.Dist = TRUE;
      chk.Fovy = TRUE;
      chk.Pos = TRUE;
      FIELD_CAMERA_RecvLinerParam(p_wk->p_camera, &chk, 60);
      (*p_seq)++;
    }
    break;
  case SEQ_CAMERA_COME_WAIT:
    if( FIELD_CAMERA_CheckMvFunc(p_wk->p_camera ) == FALSE )
    {
      (*p_seq)++;
    }
    break;

  //カメラ使用終了
  case SEQ_CAMERA_END:
    FIELD_CAMERA_ClearRecvCamParam(p_wk->p_camera);
    FIELD_CAMERA_BindDefaultTarget(p_wk->p_camera);
    FIELD_CAMERA_RestartTrace(p_wk->p_camera);
    (*p_seq)++;
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;
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
  FOG_SHADOW      *p_fog_shadow;
  ISS_SWITCH_SYS  *p_iss_switch;
  s16 phase;
  s16 dummy;
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
 *	@param  phase   フェイズ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow, s16 phase )
{
  GMEVENT * p_event = GMEVENT_Create( p_gamesys, NULL, Fadeout_Event, sizeof(FADEOUT_EVENT_WORK) );
  FADEOUT_EVENT_WORK *p_wk = GMEVENT_GetEventWork(p_event);
  p_wk->p_fog_shadow  = p_fog_shadow;
  {
    ISS_SYS *p_iss  = GAMESYSTEM_GetIssSystem( p_gamesys );
    p_wk->p_iss_switch  = ISS_SYS_GetIssSwitchSystem( p_iss );
  }
  p_wk->phase         = phase;

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
    SEQ_FADE_START,
    SEQ_FADE_WAIT,
    SEQ_FADE_RESET,
    SEQ_END,
  };

  FADEOUT_EVENT_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_FADE_START:
    ISS_SWITCH_SYS_SwitchOn( p_wk->p_iss_switch, p_wk->phase );
    FOG_SHADOW_FadeOutRequest( p_wk->p_fog_shadow, FOG_SHADOW_FADE_SYNC );
    (*p_seq)++;
    break;
  case SEQ_FADE_WAIT:
    if( FOG_SHADOW_IsFadeEnd( p_wk->p_fog_shadow ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADE_RESET:
    FOG_SHADOW_SetPhaseRequest( p_wk->p_fog_shadow, p_wk->phase );
    (*p_seq)++;
    break;
  case SEQ_END:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    セーブ情報
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *	@brief  イベントシーケンスを設定
 *
 *	@param	GYM_ELEC_SV_WORK *p_wk  ワーク
 *	@param	seq   イベントシーケンス
 */
//-----------------------------------------------------------------------------
static void GYM_ELEC_SV_SetEventSeq( GYM_ELEC_SV_WORK *p_wk, u16 seq )
{
  p_wk->event_seq = seq;
}
//----------------------------------------------------------------------------
/**
 *	@brief  イベントシーケンスを取得
 *
 *	@param	const GYM_ELEC_SV_WORK *cp_wk   ワーク
 *
 *	@return イベントシーケンス
 */
//-----------------------------------------------------------------------------
static u16 GYM_ELEC_SV_GetEventSeq( const GYM_ELEC_SV_WORK *cp_wk )
{
  return  cp_wk->event_seq;
}
#ifdef GYM_ELEC_DEBUG_MENU
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    デバッグメニュー
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	const
//=====================================
#define DEBUGWIN_GYM_ELEC (4)
#define DEBUGWIN_GYM_ELEC_SHADOW (5)

//-------------------------------------
///	global
//=====================================
static int s_fog_move_flag    = 1;
static int s_player_start_pos = 0;
static int s_player_end_pos   = 0;
static int s_fog_start_offset = 0;
static int s_fog_end_offset   = 0;

//-------------------------------------
///	private
//=====================================
static BOOL DebugWin_Util_ChangeData( int *p_param, int min, int max );
static void DebugWin_Update_EventSeq( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_EventSeq( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_FogMoveFlag( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_FogMoveFlag( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_PlayerStartPos( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_PlayerStartPos( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_PlayerEndPos( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_PlayerEndPos( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_FogStartOffset( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_FogStartOffset( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_FogEndOffset( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_FogEndOffset( void* userWork , DEBUGWIN_ITEM* item );

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yにデバッグメニュー追加
 *  @param  p_tmp         ギミックテンポラリワーク
 *	@param	HEAPID heapID 
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_tmp_adrs, HEAPID heapID )
{
  GYM_ELEC_TMP  *p_tmp  = p_tmp_adrs;

  //フォグ影現在値を取得
  {
    FOG_SHADOW_INIT_SETUP_TBL setup;
    FOG_SHADOW_DEBUG_GetSetupTbl( p_tmp->p_fog_shadow, &setup );
    s_player_start_pos  = setup.player_start_pos;
    s_player_end_pos    = setup.player_end_pos;
    s_fog_start_offset  = setup.fog_start_offset;
    s_fog_end_offset    = setup.fog_end_offset;
  }

  //デバッグメニューグループを追加
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GYM_ELEC , "でんきジム", heapID );
  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GYM_ELEC_SHADOW, "かげ", DEBUGWIN_GYM_ELEC, heapID );

  //シーケンス
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_EventSeq, 
                                DebugWin_Draw_EventSeq, 
                                p_tmp, 
                                DEBUGWIN_GYM_ELEC, 
                                heapID );
  //フォグ影移動フラグ
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_FogMoveFlag , 
                                DebugWin_Draw_FogMoveFlag , 
                                p_tmp->p_fog_shadow, 
                                DEBUGWIN_GYM_ELEC_SHADOW, 
                                heapID );
  //フォグ影プレイヤー開始位置
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_PlayerStartPos , 
                                DebugWin_Draw_PlayerStartPos , 
                                p_tmp->p_fog_shadow, 
                                DEBUGWIN_GYM_ELEC_SHADOW, 
                                heapID );
  //フォグ影プレイヤー終了位置
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_PlayerEndPos , 
                                DebugWin_Draw_PlayerEndPos , 
                                p_tmp->p_fog_shadow, 
                                DEBUGWIN_GYM_ELEC_SHADOW, 
                                heapID );
  //フォグ影フォグ開始位置
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_FogStartOffset , 
                                DebugWin_Draw_FogStartOffset , 
                                p_tmp->p_fog_shadow, 
                                DEBUGWIN_GYM_ELEC_SHADOW, 
                                heapID );
  //フォグ影フォグ終了位置
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_FogEndOffset , 
                                DebugWin_Draw_FogEndOffset , 
                                p_tmp->p_fog_shadow, 
                                DEBUGWIN_GYM_ELEC_SHADOW, 
                                heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup(DEBUGWIN_GYM_ELEC);
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ数値入力汎用関数
 *
 *	@param	*p_param  値のワーク
 *	@param	min   入力最小値
 *	@param	max   入力最大値
 *
 *	@return TRUE  更新した  FALSE更新していない
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_Util_ChangeData( int *p_param, int min, int max )
{ 
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
  { 
    value *= 10;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  { 
    value *= 100;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
  { 
    value *= 1000;
  }
  
  (*p_param)  +=  value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );


  if( value != 0 )
  { 
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  イベントシーケンス更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_EventSeq( void* userWork , DEBUGWIN_ITEM* item )
{
  GYM_ELEC_TMP *p_tmp  = userWork;
  int event_seq  = GYM_ELEC_SV_GetEventSeq( p_tmp->p_sv );
  if( DebugWin_Util_ChangeData( &event_seq, 0, GYM_ELEC_EVENT_SEQ_MAX-1 ) )
  {
    GYM_ELEC_SV_SetEventSeq( p_tmp->p_sv, event_seq );

    Gym_Elec_SetEnvironment( p_tmp, p_tmp->p_sv );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  イベントシーケンス描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_EventSeq( void* userWork , DEBUGWIN_ITEM* item )
{
  static const char* sc_label[GYM_ELEC_EVENT_SEQ_MAX] =
  {
    "しょき",
    "１かった",
    "２かった",
    "３かった",
    "ボスかった",
  };

  GYM_ELEC_TMP *p_tmp  = userWork;
  int event_seq  = GYM_ELEC_SV_GetEventSeq( p_tmp->p_sv );
  DEBUGWIN_ITEM_SetNameV( item , "じょうたい[%s]", sc_label[event_seq] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  動作フラグ更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogMoveFlag( void* userWork , DEBUGWIN_ITEM* item )
{
  FOG_SHADOW *p_fog_shadow  = userWork;
  if( DebugWin_Util_ChangeData( &s_fog_move_flag, 0, 1 ) )
  {
    FOG_SHADOW_DEBUG_SetMoveFlag( p_fog_shadow, s_fog_move_flag );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  動作フラグ描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogMoveFlag( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "フォグいどう[%d]", s_fog_move_flag );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  プレイヤー開始位置更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_PlayerStartPos( void* userWork , DEBUGWIN_ITEM* item )
{
  FOG_SHADOW *p_fog_shadow  = userWork;
  if( DebugWin_Util_ChangeData( &s_player_start_pos, -0xFFFF/2, 0xFFFF/2 ) )
  {
    FOG_SHADOW_INIT_SETUP_TBL setup;
    setup.player_start_pos  = s_player_start_pos;
    setup.player_end_pos    = s_player_end_pos;
    setup.fog_start_offset  = s_fog_start_offset;
    setup.fog_end_offset    = s_fog_end_offset;

    FOG_SHADOW_DEBUG_SetSetupTbl( p_fog_shadow, &setup );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  プレイヤー開始位置描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_PlayerStartPos( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "ざひょう かいし[%d]", s_player_start_pos );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  プレイヤー終了位置更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_PlayerEndPos( void* userWork , DEBUGWIN_ITEM* item )
{
  FOG_SHADOW *p_fog_shadow  = userWork;
  if( DebugWin_Util_ChangeData( &s_player_end_pos, -0xFFFF/2, 0xFFFF/2 ) )
  {
    FOG_SHADOW_INIT_SETUP_TBL setup;
    setup.player_start_pos  = s_player_start_pos;
    setup.player_end_pos    = s_player_end_pos;
    setup.fog_start_offset  = s_fog_start_offset;
    setup.fog_end_offset    = s_fog_end_offset;

    FOG_SHADOW_DEBUG_SetSetupTbl( p_fog_shadow, &setup );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  プレイヤー終了位置描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_PlayerEndPos( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "ざひょう おわり[%d]", s_player_end_pos );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フォグ開始位置更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogStartOffset( void* userWork , DEBUGWIN_ITEM* item )
{
  FOG_SHADOW *p_fog_shadow  = userWork;
  if( DebugWin_Util_ChangeData( &s_fog_start_offset, -0xFFFF/2, 0xFFFF/2 ) )
  {
    FOG_SHADOW_INIT_SETUP_TBL setup;
    setup.player_start_pos  = s_player_start_pos;
    setup.player_end_pos    = s_player_end_pos;
    setup.fog_start_offset  = s_fog_start_offset;
    setup.fog_end_offset    = s_fog_end_offset;

    FOG_SHADOW_DEBUG_SetSetupTbl( p_fog_shadow, &setup );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フォグ開始位置描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogStartOffset( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "フォグ かいし[%d]", s_fog_start_offset );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フォグ終了位置更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_FogEndOffset( void* userWork , DEBUGWIN_ITEM* item )
{
  FOG_SHADOW *p_fog_shadow  = userWork;
  if( DebugWin_Util_ChangeData( &s_fog_end_offset, -0xFFFF/2, 0xFFFF/2 ) )
  {
    FOG_SHADOW_INIT_SETUP_TBL setup;
    setup.player_start_pos  = s_player_start_pos;
    setup.player_end_pos    = s_player_end_pos;
    setup.fog_start_offset  = s_fog_start_offset;
    setup.fog_end_offset    = s_fog_end_offset;

    FOG_SHADOW_DEBUG_SetSetupTbl( p_fog_shadow, &setup );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フォグ影  フォグ終了位置描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_FogEndOffset( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "フォグ おわり[%d]", s_fog_end_offset );
}


#endif //GYM_ELEC_DEBUG_MENU
