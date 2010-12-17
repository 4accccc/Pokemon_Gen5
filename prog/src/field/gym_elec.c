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


#include "field_g3d_mapper.h"

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
//モデルID
typedef enum
{
  MODELID_SPOTLIGHT_NORMAL  = GYM_ELEC_SPOTLIGHT_NORMAL, //通常ライト
  MODELID_SPOTLIGHT_LEFT    = GYM_ELEC_SPOTLIGHT_LEFT,   //左から当たるライト
  MODELID_SPOTLIGHT_RIGHT   = GYM_ELEC_SPOTLIGHT_RIGHT,  //右から当たるライト
  MODELID_SPOTLIGHT_CENTER  = GYM_ELEC_SPOTLIGHT_CENTER, //上から当たるライト

  MODELID_SPOTLIGHT_END = GYM_ELEC_SPOTLIGHT_MAX,

  MODELID_FLOORLIGHT_FRONT = MODELID_SPOTLIGHT_END,  //通路ライト手前
  MODELID_FLOORLIGHT_MIDDLE,  //通路ライト真ん中
  MODELID_FLOORLIGHT_BACK,    //通路ライト奥

  MODELID_DISPLAY_BOSS,       //画面  カミツレ
  MODELID_DISPLAY_BOY,        //画面  男
  MODELID_DISPLAY_GIRL,       //画面  女

  MODELID_FLASH,              //カメラフラッシュ

  MODELID_MAX,
}MODELID;

//アニメID
typedef enum
{
  ANIMEID_START,

  //MODELID_FLOORLIGHT_FRONT用アニメID
  ANIMEID_FLOOR_LIGHT_01_ITA  = ANIMEID_START,
  ANIMEID_FLOOR_LIGHT_01_MAX,

  //MODELID_FLOORLIGHT_MIDDLE用アニメID
  ANIMEID_FLOOR_LIGHT_02_ITA  = ANIMEID_START,
  ANIMEID_FLOOR_LIGHT_02_MAX,

  //MODELID_FLOORLIGHT_MIDDLE用アニメID
  ANIMEID_FLOOR_LIGHT_03_ITA  = ANIMEID_START,
  ANIMEID_FLOOR_LIGHT_03_MAX,

  //MODELID_DISPLAY_BOSS用アニメID
  ANIMEID_DISPLAY_BOSS_ICA    = ANIMEID_START,
  ANIMEID_DISPLAY_BOSS_ITA,
  ANIMEID_DISPLAY_BOSS_MAX,

  //MODELID_DISPLAY_BOY用アニメID
  ANIMEID_DISPLAY_BOY_ICA     = ANIMEID_START,
  ANIMEID_DISPLAY_BOY_ITA,
  ANIMEID_DISPLAY_BOY_MAX,

  //MODELID_DISPLAY_GIRL用アニメID
  ANIMEID_DISPLAY_GIRL_ICA    = ANIMEID_START,
  ANIMEID_DISPLAY_GIRL_ITA,
  ANIMEID_DISPLAY_GIRL_MAX,

  //MODELID_FLASH用アニメID
  ANIMEID_FLASH_ICA           = ANIMEID_START,
  ANIMEID_FLASH_MAX,

}ANIMEID;

typedef struct _MODEL_MANAGER MODEL_MANAGER;
typedef struct _MODEL MODEL;

static MODEL_MANAGER * MODEL_MANAGER_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID );
static void MODEL_MANAGER_Exit( MODEL_MANAGER *p_wk );
static void MODEL_MANAGER_Main( MODEL_MANAGER *p_wk );

static MODEL * MODEL_MANAGER_GetModel( MODEL_MANAGER *p_wk, MODELID modelID );
static void MODEL_SetVisible( MODEL *p_wk, BOOL is_visible );
static BOOL MODEL_GetVisible( const MODEL *cp_wk );
static void MODEL_SetTrans( MODEL *p_wk, const VecFx32 * cp_trans );
static void MODEL_GetTrans( const MODEL *cp_wk, VecFx32 * p_trans );
static void MODEL_StartAnime( MODEL *p_wk, ANIMEID anmID );
static BOOL MODEL_IsAnimeStop( const MODEL *cp_wk, ANIMEID anmID );
static void MODEL_StartAnimeAll( MODEL *p_wk );
static BOOL MODEL_IsAnimeStopAll( const MODEL *cp_wk );

static const VecFx32 * MODEL_DEBUG_GetPosPointer( const MODEL *cp_wk );

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

static FOG_SHADOW* FOG_SHADOW_Init( FIELD_FOG_WORK* p_fog, const FIELD_CAMERA* cp_camera, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl, s32 tbl_max, s32 phase, HEAPID heapID );
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
typedef enum
{
  SPOT_LIGHT_TYPE_NORMAL,
  SPOT_LIGHT_TYPE_LEFT,
  SPOT_LIGHT_TYPE_RIGHT,
  SPOT_LIGHT_TYPE_CENTER,

  SPOT_LIGHT_TYPE_MAX,
}SPOT_LIGHT_TYPE;

typedef struct _SPOT_LIGHT_SET SPOT_LIGHT_SET;

static SPOT_LIGHT_SET* SPOT_LIGHT_SET_Init( const MMDLSYS *cp_mmdlsys, MODEL_MANAGER *p_model_man, HEAPID heapID );
static void SPOT_LIGHT_SET_Exit( SPOT_LIGHT_SET* p_wk );
static void SPOT_LIGHT_SET_Main( SPOT_LIGHT_SET* p_wk );

static void SPOT_LIGHT_SET_Register( SPOT_LIGHT_SET *p_wk, u16 objID, SPOT_LIGHT_TYPE type );
static void SPOT_LIGHT_SET_UnRegister( SPOT_LIGHT_SET *p_wk, u16 objID, SPOT_LIGHT_TYPE type );

//-------------------------------------
///	フロアのライト
//=====================================
typedef enum
{
  FLOOR_LIGHT_ID_FRONT,   //手前
  FLOOR_LIGHT_ID_MIDDLE,  //中
  FLOOR_LIGHT_ID_BACK,    //奥
  FLOOR_LIGHT_ID_MAX,
}FLOOR_LIGHT_ID;

typedef struct _FLOOR_LIGHT FLOOR_LIGHT;

static FLOOR_LIGHT* FLOOR_LIGHT_Init( MODEL_MANAGER *p_model_man, HEAPID heapID );
static void FLOOR_LIGHT_Exit( FLOOR_LIGHT *p_wk );
static void FLOOR_LIGHT_Main( FLOOR_LIGHT *p_wk );

static void FLOOR_LIGHT_Enable( FLOOR_LIGHT *p_wk, FLOOR_LIGHT_ID floorID );

//-------------------------------------
///	カミツレの後ろにある、画面
//=====================================
typedef struct _DISPLAY DISPLAY;

static DISPLAY* DISPLAY_Init( MODEL_MANAGER*p_model_man, HEAPID heapID );
static void DISPLAY_Exit( DISPLAY *p_wk );
static void DISPLAY_Main( DISPLAY *p_wk );

//static void DISPLAY_

//-------------------------------------
///	ショーアップ（カミツレショーアップ演出）
//=====================================
static GMEVENT * SHOWUP_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow );

//-------------------------------------
///	フェードアウト(影がなくなる演出)
//=====================================
typedef enum
{
  FADEOUT_PHASE_FRONT,  //手前が消えて、中間が出現
  FADEOUT_PHASE_MIDDLE, //中間が消えて、奥が出現
  FADEOUT_PHASE_BACK,   //奥が消える
}FADEOUT_PHASE;
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow, FLOOR_LIGHT *p_floor_light, FADEOUT_PHASE phase );

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
static void DEBUGMENU_Init( void *p_tmp_adrs, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
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
  SPOT_LIGHT_SET  *p_spot_light;  //スポットライト管理
  MODEL_MANAGER   *p_model_man;   //モデル管理
  ISS_SWITCH_SYS  *p_iss_switch;  //スイッチiss
  FLOOR_LIGHT     *p_floor_light; //通路ライト
  DISPLAY         *p_display;     //背後の巨大画面
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
    32650,
    32623,
  },
  {
    872,
    632,
    32692,
    32623,
  },
  {
    632,
    392,
    32687,
    32623,
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
    const FIELD_CAMERA* cp_camera = FIELDMAP_GetFieldCamera(p_fieldmap);
    p_tmp->p_fog_shadow = FOG_SHADOW_Init( p_fog, cp_camera, sc_fogshadow_init_tbl, NELEMS(sc_fogshadow_init_tbl), 0, heapID );
  }

  //スポットライト作成
  {
    const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys(p_fieldmap);
    p_tmp->p_spot_light = SPOT_LIGHT_SET_Init( cp_mmdlsys, p_tmp->p_model_man, heapID );
  }

  //通路ライト作成
  p_tmp->p_floor_light  = FLOOR_LIGHT_Init( p_tmp->p_model_man, heapID );

  //画面の作成
  p_tmp->p_display  = DISPLAY_Init( p_tmp->p_model_man, heapID );

  //セーブワークの状態に環境を整える
  Gym_Elec_SetEnvironment( p_tmp, p_tmp->p_sv );

#ifdef GYM_ELEC_DEBUG_MENU
  //デバッグ作成
  DEBUGMENU_Init( p_tmp, p_fieldmap, heapID );
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

  //反映されず残っている予約設定を全削除
  ISS_SWITCH_SYS_ClearReserve( p_tmp->p_iss_switch );

  //画面破棄
  DISPLAY_Exit( p_tmp->p_display );

  //通路ライト破棄
  FLOOR_LIGHT_Exit( p_tmp->p_floor_light);

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

  //通路ライトメイン
  FLOOR_LIGHT_Main( p_tmp->p_floor_light );

  //画面メイン
  DISPLAY_Main( p_tmp->p_display );

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
 *	@param  event_seq イベントシーケンス
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_ELEC_CreateFadeoutEvent( GAMESYS_WORK *p_gamesys, u16 event_seq )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

    //現在のフェイズを保存
  GYM_ELEC_SV_SetEventSeq( p_tmp->p_sv, event_seq );

  //よりイベントシーケンスを設定
  switch( event_seq )
  {
  case GYM_ELEC_EVENT_SEQ_START:
    OS_TPrintf( "開始時はフェードしません\n" );
    return NULL;
  case GYM_ELEC_EVENT_SEQ_TR01_BTL_END:
    return FADEOUT_CreateEvent( p_gamesys, p_tmp->p_fog_shadow, p_tmp->p_floor_light, FADEOUT_PHASE_FRONT );
  case GYM_ELEC_EVENT_SEQ_TR02_BTL_END:
    return FADEOUT_CreateEvent( p_gamesys, p_tmp->p_fog_shadow, p_tmp->p_floor_light, FADEOUT_PHASE_MIDDLE );
  case GYM_ELEC_EVENT_SEQ_TR03_BTL_END:
    return FADEOUT_CreateEvent( p_gamesys, p_tmp->p_fog_shadow, p_tmp->p_floor_light, FADEOUT_PHASE_BACK );
  case GYM_ELEC_EVENT_SEQ_BOSS_BTL_END:
    OS_TPrintf( "ボス終了時はフェードしません\n" );
    return NULL;
  default:
    GF_ASSERT( 0 );
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
void GYM_ELEC_SetSpotLight( FIELDMAP_WORK *p_fieldmap, BOOL is_visible, u16 objID, u16 type )
{
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);

  if( is_visible )
  {
    PMSND_PlaySE( GYM_ELEC_SE_SPOTON );
    SPOT_LIGHT_SET_Register( p_tmp->p_spot_light, objID, type );
  }
  else
  {
    SPOT_LIGHT_SET_UnRegister( p_tmp->p_spot_light, objID, type );
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
  GYM_ELEC_TMP *p_tmp = GMK_TMP_WK_GetWork(p_fieldmap, GYM_ELEC_TMP_ASSIGN_ID);
  Gym_Elec_SetEnvironment( p_tmp, p_tmp->p_sv );
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
 *	@param	GYM_ELEC_TMP *p_tmp     ギミックワーク
 *	@param	GYM_ELEC_SV_WORK *cp_sv   セーブワーク
 */
//-----------------------------------------------------------------------------
static void Gym_Elec_SetEnvironment( GYM_ELEC_TMP *p_tmp, const GYM_ELEC_SV_WORK *cp_sv )
{
  switch( GYM_ELEC_SV_GetEventSeq( cp_sv ) )
  {
  case GYM_ELEC_EVENT_SEQ_START:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 0 );
    break;
  case GYM_ELEC_EVENT_SEQ_TR01_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 1 );
    //本来ISS_SWITCH_SYS_SwitchOnを使うべきですが、Recoverでも予備、ISS_SWITCH_SYSが起動していないため、予約の方を使う
    ISS_SWITCH_SYS_ReserveSwitch( p_tmp->p_iss_switch, SWITCH_01, TRUE, GYM_ELEC_BGM_NORMAL );
    break;
  case GYM_ELEC_EVENT_SEQ_TR02_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, TRUE );
    FOG_SHADOW_SetPhaseRequest( p_tmp->p_fog_shadow, 2 );
    ISS_SWITCH_SYS_ReserveSwitch( p_tmp->p_iss_switch, SWITCH_01, TRUE, GYM_ELEC_BGM_NORMAL );
    ISS_SWITCH_SYS_ReserveSwitch( p_tmp->p_iss_switch, SWITCH_02, TRUE, GYM_ELEC_BGM_NORMAL );
    break;
  case GYM_ELEC_EVENT_SEQ_TR03_BTL_END:
    FOG_SHADOW_SetVisible( p_tmp->p_fog_shadow, FALSE );
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
  MODEL_RESID_SPOTLIGHT01_IMD,      //トレーナー用スポットライトモデル
  MODEL_RESID_SPOTLIGHT_CENTER_IMD, //カミツレ用用スポットライト中央モデル
  MODEL_RESID_SPOTLIGHT_LEFT_IMD,   //トレーナー用スポットライト左モデル
  MODEL_RESID_SPOTLIGHT_RIGHT_IMD,  //トレーナー用スポットライト右モデル
  MODEL_RESID_FLOORLIGHT_01_IMD,    //通路用０１ライトモデル
  MODEL_RESID_FLOORLIGHT_01_ITA,    //通路用０１ライトアニメ
  MODEL_RESID_FLOORLIGHT_02_IMD,    //通路用０２ライトモデル
  MODEL_RESID_FLOORLIGHT_02_ITA,    //通路用０２ライトアニメ
  MODEL_RESID_FLOORLIGHT_03_IMD,    //通路用０３ライトモデル
  MODEL_RESID_FLOORLIGHT_03_ITA,    //通路用０３ライトアニメ
  MODEL_RESID_DIPLAY_BOSS_IMD,      //背後の画面  カミツレモデル
  MODEL_RESID_DIPLAY_BOSS_ICA,      //背後の画面  カミツレアニメ
  MODEL_RESID_DIPLAY_BOSS_ITA,      //背後の画面  カミツレアニメ
  MODEL_RESID_DIPLAY_BOY_IMD,       //背後の画面  男モデル
  MODEL_RESID_DIPLAY_BOY_ICA,       //背後の画面  男アニメ
  MODEL_RESID_DIPLAY_BOY_ITA,       //背後の画面  男アニメ
  MODEL_RESID_DIPLAY_GIRL_IMD,      //背後の画面  女カミツレモデル
  MODEL_RESID_DIPLAY_GIRL_ICA,      //背後の画面  女アニメ
  MODEL_RESID_DIPLAY_GIRL_ITA,      //背後の画面  女アニメ
  MODEL_RESID_FLASH_IMD,            //フラッシュモデル
  MODEL_RESID_FLASH_ICA,            //フラッシュアニメ
  MODEL_RESID_MAX,
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
  BOOL                is_visible;
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
  //MODEL_RESID_SPOTLIGHT_CENTER_IMD, //カミツレ用用スポットライト中央モデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_spot02a_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_SPOTLIGHT_LEFT_IMD,   //トレーナー用スポットライト左モデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_spot02b_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_SPOTLIGHT_RIGHT_IMD,  //トレーナー用スポットライト右モデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_spot02c_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_01_IMD,//通路用０１ライトモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light01_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_01_ITA,//通路用０１ライトアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light01_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_02_IMD,//通路用０２ライトモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light02_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_02_ITA,//通路用０２ライトアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light02_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_03_IMD,//通路用０３ライトモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light03_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLOORLIGHT_03_ITA,//通路用０３ライトアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_stage_light03_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOSS_IMD,      //背後の画面  カミツレモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_kami_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOSS_ICA,      //背後の画面  カミツレアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_kami_nsbca, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOSS_ITA,      //背後の画面  カミツレアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_kami_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOY_IMD,       //背後の画面  男モデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_boy_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOY_ICA,       //背後の画面  男アニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_boy_nsbca, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_BOY_ITA,       //背後の画面  男アニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_boy_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_GIRL_IMD,      //背後の画面  女カミツレモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_girl_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_GIRL_ICA,      //背後の画面  女アニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_girl_nsbca, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_DIPLAY_GIRL_ITA,      //背後の画面  女アニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_display_girl_nsbta, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLASH_IMD,            //フラッシュモデル
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_flash01_nsbmd, GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_FLASH_ICA,            //フラッシュアニメ
  {
    ARCID_GYM_ELEC, NARC_gym_elec_c4g_flash01_nsbca, GFL_G3D_UTIL_RESARC
  },
};

//アニメ設定
//通路アニメ１用
static const GFL_G3D_UTIL_ANM sc_g3d_floorlight_01_anm[ANIMEID_FLOOR_LIGHT_01_MAX] = 
{
  MODEL_RESID_FLOORLIGHT_01_ITA,
};
//通路アニメ２用
static const GFL_G3D_UTIL_ANM sc_g3d_floorlight_02_anm[ANIMEID_FLOOR_LIGHT_02_MAX] = 
{
  MODEL_RESID_FLOORLIGHT_02_ITA,
};
//通路アニメ３用
static const GFL_G3D_UTIL_ANM sc_g3d_floorlight_03_anm[ANIMEID_FLOOR_LIGHT_03_MAX] = 
{
  MODEL_RESID_FLOORLIGHT_03_ITA,
};
//画面カミツレ
static const GFL_G3D_UTIL_ANM sc_g3d_display_boss_anm[ANIMEID_DISPLAY_BOSS_MAX] =
{
  MODEL_RESID_DIPLAY_BOSS_ICA,
  MODEL_RESID_DIPLAY_BOSS_ITA,
};
//画面男
static const GFL_G3D_UTIL_ANM sc_g3d_display_boy_anm[ANIMEID_DISPLAY_BOY_MAX] =
{
  MODEL_RESID_DIPLAY_BOY_ICA,
  MODEL_RESID_DIPLAY_BOY_ITA,
};
//画面女
static const GFL_G3D_UTIL_ANM sc_g3d_display_girl_anm[ANIMEID_DISPLAY_GIRL_MAX] =
{
  MODEL_RESID_DIPLAY_GIRL_ICA,
  MODEL_RESID_DIPLAY_GIRL_ITA,
};
//フラッシュ
static const GFL_G3D_UTIL_ANM sc_g3d_flash_anm[ANIMEID_FLASH_MAX] =
{
  MODEL_RESID_FLASH_ICA,
};

//3Dオブジェクト設定
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj[MODELID_MAX] =
{
  //MODELID_SPOTLIGHT_NORMAL 通常ライト
  {
    MODEL_RESID_SPOTLIGHT01_IMD, 	//モデルリソースID
		0, 							              //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT01_IMD, 	//テクスチャリソースID
		NULL,			                    //アニメテーブル(複数指定のため)
		0,	                          //アニメリソース数
  },
  //MODELID_SPOTLIGHT_LEFT,   //左から当たるライト
  {
    MODEL_RESID_SPOTLIGHT_LEFT_IMD, 	//モデルリソースID
		0, 							                  //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT_LEFT_IMD, 	//テクスチャリソースID
		NULL,			                        //アニメテーブル(複数指定のため)
		0,	                              //アニメリソース数
  },
  //MODELID_SPOTLIGHT_RIGHT,  //右から当たるライト
  {
    MODEL_RESID_SPOTLIGHT_RIGHT_IMD, 	//モデルリソースID
		0, 							                  //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT_RIGHT_IMD, 	//テクスチャリソースID
		NULL,			                        //アニメテーブル(複数指定のため)
		0,	                              //アニメリソース数
  },
  //MODELID_SPOTLIGHT_CENTER, //上から当たるライト
  {
    MODEL_RESID_SPOTLIGHT_CENTER_IMD, 	//モデルリソースID
		0, 							                    //モデルデータID(リソース内部INDEX)
		MODEL_RESID_SPOTLIGHT_CENTER_IMD, 	//テクスチャリソースID
		NULL,			                          //アニメテーブル(複数指定のため)
		0,	                                //アニメリソース数
  },
  //MODELID_FLOORLIGHT_FRONT  //通路ライト手前
  {
    MODEL_RESID_FLOORLIGHT_01_IMD,  //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_FLOORLIGHT_01_IMD,  //テクスチャリソースID
    sc_g3d_floorlight_01_anm,       //アニメテーブル(複数指定のため)
    ANIMEID_FLOOR_LIGHT_01_MAX,     //アニメリソース数
  },
  //MODELID_FLOORLIGHT_MIDDLE  //通路ライト中
  {
    MODEL_RESID_FLOORLIGHT_02_IMD,  //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_FLOORLIGHT_02_IMD,  //テクスチャリソースID
    sc_g3d_floorlight_02_anm,       //アニメテーブル(複数指定のため)
    ANIMEID_FLOOR_LIGHT_02_MAX,     //アニメリソース数
  },
  //MODELID_FLOORLIGHT_BACK  //通路ライト奥
  {
    MODEL_RESID_FLOORLIGHT_03_IMD,  //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_FLOORLIGHT_03_IMD,  //テクスチャリソースID
    sc_g3d_floorlight_03_anm,       //アニメテーブル(複数指定のため)
    ANIMEID_FLOOR_LIGHT_03_MAX,     //アニメリソース数
  },
  //MODELID_DISPLAY_BOSS,       //画面  カミツレ
  {
    MODEL_RESID_DIPLAY_BOSS_IMD,    //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_DIPLAY_BOSS_IMD,    //テクスチャリソースID
    NULL,//sc_g3d_display_boss_anm,        //アニメテーブル(複数指定のため)
    0//ANIMEID_DISPLAY_BOSS_MAX,       //アニメリソース数
  },
  //MODELID_DISPLAY_BOY,        //画面  男
  {
    MODEL_RESID_DIPLAY_BOY_IMD,     //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_DIPLAY_BOY_IMD,     //テクスチャリソースID
    NULL,//sc_g3d_display_boy_anm,        //アニメテーブル(複数指定のため)
    0//ANIMEID_DISPLAY_BOY_MAX,        //アニメリソース数
  },
  //MODELID_DISPLAY_GIRL,       //画面  女
  {
    MODEL_RESID_DIPLAY_GIRL_IMD,    //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_DIPLAY_GIRL_IMD,    //テクスチャリソースID
    NULL,//sc_g3d_display_girl_anm,        //アニメテーブル(複数指定のため)
    0//ANIMEID_DISPLAY_GIRL_MAX,       //アニメリソース数
  },
  //MODELID_FLASH,              //カメラフラッシュ
  {
    MODEL_RESID_FLASH_IMD,          //モデルリソースID
    0,                              //モデルデータID(リソース内部INDEX)
    MODEL_RESID_FLASH_IMD,          //テクスチャリソースID
    sc_g3d_flash_anm,               //アニメテーブル(複数指定のため)
    ANIMEID_FLASH_MAX,              //アニメリソース数
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
      MODEL_SetVisible( p_model, FALSE );
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
  p_wk->is_visible  = is_visible;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  表示取得
 *
 *	@param	const MODEL *cp_wk  ワーク
 *
 *	@return TRUEで表示中  FALSEで非表示中
 */
//-----------------------------------------------------------------------------
static BOOL MODEL_GetVisible( const MODEL *cp_wk )
{
  return cp_wk->is_visible;
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
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  位置取得
 *
 *	@param	const MODEL *cp_wk  ワーク
 *	@param	* p_trans   取得構造体
 */
//-----------------------------------------------------------------------------
static void MODEL_GetTrans( const MODEL *cp_wk, VecFx32 * p_trans )
{
  GFL_G3D_OBJSTATUS *p_status = FLD_EXP_OBJ_GetUnitObjStatus( cp_wk->p_ex_obj,
                                                cp_wk->unitID,
                                                cp_wk->modelID );
  *p_trans  = p_status->trans;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  アニメ開始
 *
 *	@param	MODEL *p_wk ワーク
 *	@param	anmID       アニメ番号
 */
//-----------------------------------------------------------------------------
static void MODEL_StartAnime( MODEL *p_wk, ANIMEID anmID )
{
  GF_ASSERT( anmID < sc_g3d_util_obj[ p_wk->modelID ].anmCount );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->p_ex_obj, p_wk->unitID, p_wk->modelID, 
      anmID, TRUE);
  {

    EXP_OBJ_ANM_CNT_PTR p_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, p_wk->unitID, p_wk->modelID, anmID );
    FLD_EXP_OBJ_ChgAnmStopFlg( p_anm, FALSE);
  }
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->p_ex_obj, p_wk->unitID, p_wk->modelID, anmID, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  アニメ終了検知
 *
 *	@param	const MODEL *cp_wk  ワーク
 *	@param	anmID               アニメ番号
 *
 *	@return TRUEアニメ終了  FALSEアニメ終了
 */
//-----------------------------------------------------------------------------
static BOOL MODEL_IsAnimeStop( const MODEL *cp_wk, ANIMEID anmID )
{
  GF_ASSERT( anmID < sc_g3d_util_obj[ cp_wk->modelID ].anmCount );
  {
    EXP_OBJ_ANM_CNT_PTR p_anm = FLD_EXP_OBJ_GetAnmCnt( cp_wk->p_ex_obj, cp_wk->unitID, cp_wk->modelID, anmID );
    return FLD_EXP_OBJ_ChkAnmEnd( p_anm );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  全アニメ開始
 *
 *	@param	MODEL *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MODEL_StartAnimeAll( MODEL *p_wk )
{
  int i;
  for( i = 0; i < sc_g3d_util_obj[ p_wk->modelID ].anmCount; i++ )
  {
    MODEL_StartAnime( p_wk, i );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作  全アニメ終了検知
 *
 *	@param	const MODEL *cp_wk  ワーク
 *
 *	@return TRUEアニメ終了  FALSEアニメ終了
 */
//-----------------------------------------------------------------------------
static BOOL MODEL_IsAnimeStopAll( const MODEL *cp_wk )
{
  int i;
  for( i = 0; i < sc_g3d_util_obj[ cp_wk->modelID ].anmCount; i++ )
  {
    if( !MODEL_IsAnimeStop( cp_wk, i ) )
    {
      return FALSE;
    }
  }
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作デバッグ用  座標ポインタを取得
 *
 *	@param	const MODEL *cp_wk  ワーク
 *
 *	@return 座標ポインタ
 */
//-----------------------------------------------------------------------------
static const VecFx32 * MODEL_DEBUG_GetPosPointer( const MODEL *cp_wk )
{
  GFL_G3D_OBJSTATUS *p_status = FLD_EXP_OBJ_GetUnitObjStatus( cp_wk->p_ex_obj,
                                                cp_wk->unitID,
                                                cp_wk->modelID );
  return &p_status->trans;
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
  const FIELD_CAMERA* cp_camera;

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

  fx32  offset;
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
 *  @param  cp_camera カメラ情報
 *	@param	const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl 初期化構造体(メモリ勿体ないのでアドレスのみしか保存しません)
 *	@param	tbl_max 構造体の数
 *	@param	phase   開始フェーズ
 *	@param	heapID  ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static FOG_SHADOW* FOG_SHADOW_Init( FIELD_FOG_WORK* p_fog, const FIELD_CAMERA* cp_camera, const FOG_SHADOW_INIT_SETUP_TBL *cp_tbl, s32 tbl_max, s32 phase, HEAPID heapID )
{
  FOG_SHADOW  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FOG_SHADOW) );

  //使用モジュールの登録
  p_wk->p_fog     = p_fog;
  p_wk->cp_camera = cp_camera;

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

  //フォグがカメラに合わせて動く処理
  if( p_wk->is_fog_move )
  {
    const FOG_SHADOW_INIT_SETUP_TBL *cp_info  = &p_wk->now_move_info;

    VecFx32 target_pos;
    s32 offset;

    FIELD_CAMERA_GetTargetPos( p_wk->cp_camera, &target_pos);
    
    //カメラターゲット座標からオフセットを算出
    offset  = cp_info->fog_start_offset
      - ( cp_info->player_start_pos - ((target_pos.z) >> FX32_SHIFT))
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
///	ライト単体
//=====================================
typedef struct
{
  const MMDL    *cp_target;
  MODEL         *p_model;
  BOOL          is_use;
} SPOT_LIGHT;

//-------------------------------------
///	ライト管理
//=====================================
struct _SPOT_LIGHT_SET
{
  const MMDLSYS *cp_mmdlsys;
  u32 search_idx;
  SPOT_LIGHT  light[SPOT_LIGHT_TYPE_MAX];
};

//-------------------------------------
///	private
//=====================================
//スポットライトを取得
static SPOT_LIGHT * Spot_Light_Set_SearchByType( SPOT_LIGHT_SET *p_wk, SPOT_LIGHT_TYPE type );
static SPOT_LIGHT * Spot_Light_Set_SearchEmpty( SPOT_LIGHT_SET *p_wk );

//スポットライト操作
static void Spot_Light_Init( SPOT_LIGHT *p_wk, MODEL *p_model );
static void Spot_Light_Main( SPOT_LIGHT *p_wk );
static void Spot_Light_Register( SPOT_LIGHT *p_wk, const MMDL *cp_target );
static void Spot_Light_UnRegister( SPOT_LIGHT *p_wk );
static BOOL Spot_Light_IsUse( const SPOT_LIGHT *cp_wk );

//-------------------------------------
///	data
//=====================================
static const MODELID sc_light_model_tbl[SPOT_LIGHT_TYPE_MAX] =
{
  //SPOT_LIGHT_TYPE_NORMAL,
  MODELID_SPOTLIGHT_NORMAL, //通常ライト
  //SPOT_LIGHT_TYPE_LEFT,
  MODELID_SPOTLIGHT_LEFT,   //左から当たるライト
  //SPOT_LIGHT_TYPE_RIGHT,
  MODELID_SPOTLIGHT_RIGHT,  //右から当たるライト
  //SPOT_LIGHT_TYPE_CENTER,
  MODELID_SPOTLIGHT_CENTER, //上から当たるライト
};

//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  作成
 *  
 *	@param	MMDLSYS         *p_mmdlsys  モデルシステム
 *	@param  MODEL_MANAGER   *p_model_man  ギミック用モデル管理
 *	@param	heapID          ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT_SET* SPOT_LIGHT_SET_Init( const MMDLSYS *cp_mmdlsys, MODEL_MANAGER *p_model_man, HEAPID heapID )
{
  SPOT_LIGHT_SET  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(SPOT_LIGHT_SET) );
  p_wk->cp_mmdlsys  = cp_mmdlsys;
  {
    int i;
    for( i = 0; i < SPOT_LIGHT_TYPE_MAX; i++ )
    {
      SPOT_LIGHT *p_light  = &p_wk->light[i];
      MODEL *p_model  = MODEL_MANAGER_GetModel( p_model_man, sc_light_model_tbl[i] );
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
  for( i = 0; i < SPOT_LIGHT_TYPE_MAX; i++ )
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
 *	@param  type    スポットライトのタイプ
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_Register( SPOT_LIGHT_SET *p_wk, u16 objID, SPOT_LIGHT_TYPE type )
{
  SPOT_LIGHT *p_light = Spot_Light_Set_SearchByType( p_wk, type );

  //登録済みがなかったら登録
  if( Spot_Light_IsUse( p_light ) )
  {
    GF_ASSERT_MSG( 0, "このライトは現在使用中です。objID=%d type=%d\n", objID, type );
  }
  else
  {
    Spot_Light_Register( p_light, MMDLSYS_SearchOBJID( p_wk->cp_mmdlsys, objID ) );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットライト  解除
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *	@param	objID   オブジェクトID
 *  @param  type    スポットライトのタイプ
 */
//-----------------------------------------------------------------------------
static void SPOT_LIGHT_SET_UnRegister( SPOT_LIGHT_SET *p_wk, u16 objID, SPOT_LIGHT_TYPE type )
{
  SPOT_LIGHT *p_light = Spot_Light_Set_SearchByType( p_wk, type );

  //登録済みがあったら登録解除
  if( Spot_Light_IsUse( p_light ) )
  {
    Spot_Light_UnRegister( p_light );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  typeからスポットライトを取得
 *
 *	@param	SPOT_LIGHT_SET *p_wk  ワーク
 *	@param	type  タイプ
 *
 *	@return スポットライト
 */
//-----------------------------------------------------------------------------
static SPOT_LIGHT * Spot_Light_Set_SearchByType( SPOT_LIGHT_SET *p_wk, SPOT_LIGHT_TYPE type )
{
  GF_ASSERT( type < SPOT_LIGHT_TYPE_MAX );
  return &p_wk->light[type];
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
  for( i = 0; i < SPOT_LIGHT_TYPE_MAX; i++ )
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
//----------------------------------------------------------------------------
/**
 *	@brief  使用フラグ取得
 *
 *	@param	const SPOT_LIGHT *cp_wk ワーク
 *
 *	@return TRUE使用中  FALSE未使用
 */
//-----------------------------------------------------------------------------
static BOOL Spot_Light_IsUse( const SPOT_LIGHT *cp_wk )
{
  return cp_wk->is_use;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    通路ライト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	const
//=====================================
//-------------------------------------
///	struct
//=====================================
struct _FLOOR_LIGHT
{
  MODEL *p_model[FLOOR_LIGHT_ID_MAX];
};
//-------------------------------------
///	data
//=====================================
static const struct
{
  MODELID modelID;
  VecFx32 init_pos;
}  sc_floor_model_tbl[FLOOR_LIGHT_ID_MAX]  =
{
  //FLOOR_LIGHT_ID_FRONT,   //手前
  {
    MODELID_FLOORLIGHT_FRONT,
    {
      FX32_CONST(256),
      FX32_CONST(24),
      FX32_CONST(799),
    }
  },
  //FLOOR_LIGHT_ID_MIDDLE,  //中
  {
    MODELID_FLOORLIGHT_MIDDLE,
  },
  //FLOOR_LIGHT_ID_BACK,    //奥
  {
    MODELID_FLOORLIGHT_BACK,
  }
};

//----------------------------------------------------------------------------
/**
 *	@brief  通路ライト管理  作成
 *
 *	@param	MODEL_MANAGER *p_model_man  モデル管理
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static FLOOR_LIGHT* FLOOR_LIGHT_Init( MODEL_MANAGER *p_model_man, HEAPID heapID )
{
  FLOOR_LIGHT *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLOOR_LIGHT) );

  {
    int i;
    for( i = 0; i < FLOOR_LIGHT_ID_MAX; i++ )
    {
      p_wk->p_model[i]  = MODEL_MANAGER_GetModel( p_model_man, sc_floor_model_tbl[i].modelID );
      MODEL_SetTrans( p_wk->p_model[i], &sc_floor_model_tbl[i].init_pos );
      MODEL_SetVisible( p_wk->p_model[i], TRUE );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  通路ライト管理  破棄
 *
 *	@param	FLOOR_LIGHT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void FLOOR_LIGHT_Exit( FLOOR_LIGHT *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  通路ライト管理  メイン処理
 *
 *	@param	FLOOR_LIGHT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void FLOOR_LIGHT_Main( FLOOR_LIGHT *p_wk )
{
}
//----------------------------------------------------------------------------
/**
 *	@brief  通路ライト管理  ライト点灯
 *
 *	@param	FLOOR_LIGHT *p_wk   ワーク
 *	@param	floorID             フロアーID
 */
//-----------------------------------------------------------------------------
static void FLOOR_LIGHT_Enable( FLOOR_LIGHT *p_wk, FLOOR_LIGHT_ID floorID )
{
  GF_ASSERT( floorID < FLOOR_LIGHT_ID_MAX );
  MODEL_SetVisible( p_wk->p_model[ floorID ], TRUE );
  MODEL_StartAnimeAll( p_wk->p_model[ floorID ] );
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *    画面
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	const
//=====================================
//-------------------------------------
///	struct
//=====================================
struct _DISPLAY
{
  int dummy;
};

//----------------------------------------------------------------------------
/**
 *	@brief  画面管理  作成
 *
 *	@param	MODEL_MANAGER*p_model_man モデル管理
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static DISPLAY* DISPLAY_Init( MODEL_MANAGER*p_model_man, HEAPID heapID )
{
  DISPLAY *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(DISPLAY) );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  画面管理  破棄
 *
 *	@param	DISPLAY *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DISPLAY_Exit( DISPLAY *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  画面管理  メイン処理
 *
 *	@param	DISPLAY *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DISPLAY_Main( DISPLAY *p_wk )
{
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
    SEQ_END,
  };

  SHOWUP_EVENT_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
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
  FLOOR_LIGHT     *p_floor_light;
  GAMESYS_WORK    *p_gamesys;
  FADEOUT_PHASE   phase;
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
 *	@param	*p_fog    フォグ影
 *	@param  *p_floor  通路ライト
 *	@param  phase     フェイズ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * FADEOUT_CreateEvent( GAMESYS_WORK *p_gamesys, FOG_SHADOW *p_fog_shadow, FLOOR_LIGHT *p_floor_light, FADEOUT_PHASE phase )
{
  GMEVENT * p_event = GMEVENT_Create( p_gamesys, NULL, Fadeout_Event, sizeof(FADEOUT_EVENT_WORK) );
  FADEOUT_EVENT_WORK *p_wk = GMEVENT_GetEventWork(p_event);
  p_wk->p_fog_shadow  = p_fog_shadow;
  {
    ISS_SYS *p_iss  = GAMESYSTEM_GetIssSystem( p_gamesys );
    p_wk->p_iss_switch  = ISS_SYS_GetIssSwitchSystem( p_iss );
  }
  p_wk->phase         = phase;
  p_wk->p_gamesys     = p_gamesys;
  p_wk->p_floor_light = p_floor_light;

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
    PMSND_PlaySE( GYM_ELEC_SE_CHEER );
    switch( p_wk->phase )
    {
    case FADEOUT_PHASE_FRONT:
      ISS_SWITCH_SYS_SwitchOn( p_wk->p_iss_switch, SWITCH_01 );
      FLOOR_LIGHT_Enable( p_wk->p_floor_light, FLOOR_LIGHT_ID_FRONT );
      break;
    case FADEOUT_PHASE_MIDDLE:
      ISS_SWITCH_SYS_SwitchOn( p_wk->p_iss_switch, SWITCH_02 );
      FLOOR_LIGHT_Enable( p_wk->p_floor_light, FLOOR_LIGHT_ID_MIDDLE );
      break;
    case FADEOUT_PHASE_BACK:
      {
        GMEVENT* p_subevent = EVENT_FSND_ChangeBGM( p_wk->p_gamesys, GYM_ELEC_BGM_BOSSAPPER, 0, 0 );
        GMEVENT_CallEvent( p_event, p_subevent );
      }
      FLOOR_LIGHT_Enable( p_wk->p_floor_light, FLOOR_LIGHT_ID_BACK );
      break;
    default:
      GF_ASSERT(0);
    }
    FOG_SHADOW_FadeOutRequest( p_wk->p_fog_shadow, FOG_SHADOW_FADE_FRAME );
    (*p_seq)++;
    break;
  case SEQ_FADE_WAIT:
    if( FOG_SHADOW_IsFadeEnd( p_wk->p_fog_shadow ) && !PMSND_CheckPlayingSEIdx(GYM_ELEC_SE_CHEER ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_FADE_RESET:
    switch( p_wk->phase )
    {
    case FADEOUT_PHASE_FRONT:
      FOG_SHADOW_SetPhaseRequest( p_wk->p_fog_shadow, 1 );
      break;
    case FADEOUT_PHASE_MIDDLE:
      FOG_SHADOW_SetPhaseRequest( p_wk->p_fog_shadow, 2 );
      break;
    case FADEOUT_PHASE_BACK:
      //最後だけ次の影を呼ばず、影を完全に消す
      FOG_SHADOW_SetVisible( p_wk->p_fog_shadow, FALSE );
      break;
    default:
      GF_ASSERT(0);
    }
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
#define DEBUGWIN_GYM_ELEC_MODEL  (6)

//-------------------------------------
///	global
//=====================================
static int s_fog_move_flag    = 1;
static int s_player_start_pos = 0;
static int s_player_end_pos   = 0;
static int s_fog_start_offset = 0;
static int s_fog_end_offset   = 0;

static struct
{
  int is_move;
  int index;
  int is_anime;
  MODEL_MANAGER *p_model_man;
  FIELD_CAMERA  *p_camera;
  BOOL  visible_save;
} s_model_wk  = {0};

//-------------------------------------
///	private
//=====================================
static BOOL DebugWin_Util_ChangeData( int *p_param, int min, int max );
//共通
static void DebugWin_Update_EventSeq( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_EventSeq( void* userWork , DEBUGWIN_ITEM* item );
//かげ
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
//モデル
static void DebugWin_Update_ModelMove( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_ModelMove( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_ModelIndex( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_ModelIndex( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Update_ModelAnime( void* userWork , DEBUGWIN_ITEM* item );
static void DebugWin_Draw_ModelAnime( void* userWork , DEBUGWIN_ITEM* item );

//モデル操作イベント
static GMEVENT_RESULT DebugMenu_ModelControl( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
static void DebugMenu_CallModelControllEvent( GAMESYS_WORK *p_gamesys );

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yにデバッグメニュー追加
 *  @param  p_tmp         ギミックテンポラリワーク
 *  @param  p_fieldmap    フィールド
 *	@param	HEAPID heapID   ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_tmp_adrs, FIELDMAP_WORK *p_fieldmap, HEAPID heapID )
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

  //モデルマネージャを取得
  s_model_wk.p_model_man  = p_tmp->p_model_man;
  s_model_wk.p_camera     = FIELDMAP_GetFieldCamera( p_fieldmap );

  //デバッグメニューグループを追加
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GYM_ELEC , "でんきジム", heapID );
  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GYM_ELEC_SHADOW, "かげ", DEBUGWIN_GYM_ELEC, heapID );
  DEBUGWIN_AddGroupToGroup( DEBUGWIN_GYM_ELEC_MODEL, "モデル", DEBUGWIN_GYM_ELEC, heapID );

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
  //モデル操作フラグ
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_ModelMove, 
                                DebugWin_Draw_ModelMove, 
                                p_fieldmap,
                                DEBUGWIN_GYM_ELEC_MODEL, 
                                heapID );
  //モデル操作インデックス
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_ModelIndex, 
                                DebugWin_Draw_ModelIndex, 
                                NULL,
                                DEBUGWIN_GYM_ELEC_MODEL, 
                                heapID );
  //モデルアニメさせるかどうか
  DEBUGWIN_AddItemToGroupEx( DebugWin_Update_ModelAnime,
                                DebugWin_Draw_ModelAnime,
                                NULL,
                                DEBUGWIN_GYM_ELEC_MODEL, 
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

//----------------------------------------------------------------------------
/**
 *	@brief  モデル  モデル操作フラグ更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ModelMove( void* userWork , DEBUGWIN_ITEM* item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;

  //フラグOFFのとき操作でき、
  //フラグONにした瞬間にイベント起動
  //フラグOFFのするためにはイベント終了しなければならない
  if( s_model_wk.is_move == 0 )
  {
    if( DebugWin_Util_ChangeData( &s_model_wk.is_move, 0, 1 ) )
    {
      DEBUGWIN_RefreshScreen();
      if( s_model_wk.is_move == 1 )
      {
        DebugMenu_CallModelControllEvent( FIELDMAP_GetGameSysWork(p_fieldmap) );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル  モデル操作フラグ描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ModelMove( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "そうさフラグ[%d]", s_model_wk.is_move );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル  操作するモデルインデックス更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ModelIndex( void* userWork , DEBUGWIN_ITEM* item )
{
  //操作始まったあとは変更不可
  if( s_model_wk.is_move == 0 )
  {
    if( DebugWin_Util_ChangeData( &s_model_wk.index, 0, MODELID_MAX-1 ) )
    {
      DEBUGWIN_RefreshScreen();
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル  操作するモデルインデックス描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ModelIndex( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "そうさモデル[%d]", s_model_wk.index );
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル  アニメさせるかどうか更新
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ModelAnime( void* userWork , DEBUGWIN_ITEM* item )
{
  //操作始まったあとは変更不可
  if( s_model_wk.is_move == 0 )
  {
    if( DebugWin_Util_ChangeData( &s_model_wk.is_anime, 0, 1 ) )
    {
      DEBUGWIN_RefreshScreen();
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル  アニメさせるかどうか描画
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ModelAnime( void* userWork , DEBUGWIN_ITEM* item )
{
  DEBUGWIN_ITEM_SetNameV( item , "アニメ[%d]", s_model_wk.is_anime );
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作用イベント
 *
 *	@param	GMEVENT *p_event  イベント
 *	@param	*p_seq            シーケンス
 *	@param	*p_wk_adrs        ワークアドレス
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT DebugMenu_ModelControl( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  static const fx32 sc_default_value  = FX32_CONST( 1 );
  VecFx32 value = {0};
  MODEL *p_model  = MODEL_MANAGER_GetModel( s_model_wk.p_model_man, s_model_wk.index );

  //移動操作
  if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
  {
    value.z = -sc_default_value;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
  {
    value.z = sc_default_value;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
  {
    value.x = -sc_default_value;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
  {
    value.x = sc_default_value;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    value.y = sc_default_value;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    value.y = -sc_default_value;
  }

  //速度操作
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    value.x *= 10;
    value.y *= 10;
    value.z *= 10;
  }

  //更新があれば反映
  if( value.x != 0 || value.y != 0 || value.z != 0 )
  {
    VecFx32 pos;
    MODEL_GetTrans( p_model, &pos );
    VEC_Add( &pos, &value, &pos );
    MODEL_SetTrans( p_model, &pos );

    OS_Printf( "○位置x%fy%fz%f\n", FX_FX32_TO_F32(pos.x), FX_FX32_TO_F32(pos.y),FX_FX32_TO_F32(pos.z));
  }

  //終了
  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
  {
    s_model_wk.is_move  = 0;
    FIELD_CAMERA_BindDefaultTarget(s_model_wk.p_camera);
    MODEL_SetVisible( p_model, s_model_wk.visible_save);
    OS_Printf( "▲モデル操作モード終了！！▲\n" );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル操作用イベントを呼ぶ
 *
 *	@param	GAMESYS_WORK *p_gamesys ゲームシステム
 */
//-----------------------------------------------------------------------------
static void DebugMenu_CallModelControllEvent( GAMESYS_WORK *p_gamesys )
{
  //モデル操作イベント作成
  GMEVENT *p_event  = GMEVENT_Create( p_gamesys, NULL, DebugMenu_ModelControl, 0);

  OS_Printf( "▼モデル操作モード開始！！▼\n" );
  OS_Printf( "↑…モデル奥に移動\n" );
  OS_Printf( "↓…モデル手前に移動\n" );
  OS_Printf( "←…モデル左に移動\n" );
  OS_Printf( "→…モデル右に移動\n" );
  OS_Printf( "L…モデル上に移動\n" );
  OS_Printf( "R…モデル下に移動\n" );
  OS_Printf( "X…押しながら移動で速度UP\n" );
  OS_Printf( "B…終了\n" );

  //カメラをモデルにバインドする
  {
    MODEL *p_model  = MODEL_MANAGER_GetModel( s_model_wk.p_model_man, s_model_wk.index );
    FIELD_CAMERA_BindTarget( s_model_wk.p_camera, MODEL_DEBUG_GetPosPointer(p_model) );
    s_model_wk.visible_save = MODEL_GetVisible( p_model );
    MODEL_SetVisible( p_model, TRUE );
    if( s_model_wk.is_anime )
    {
      MODEL_StartAnimeAll( p_model );
    }
  }

  //イベントを呼ぶ
  GAMESYSTEM_SetEvent( p_gamesys, p_event );
}

#endif //GYM_ELEC_DEBUG_MENU
