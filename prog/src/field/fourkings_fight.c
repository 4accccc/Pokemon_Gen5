//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_fight.c
 *	@brief  四天王部屋:格闘
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/fourkings_fight.naix"
#include "../../../resource/fldmapdata/eventdata/tmp/c09r0301.h"

// mine
#include "fourkings_sv.h"
#include "fourkings_fight.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FOURKINGS_FIGHT_UNIT_IDX        (0)
#define FOURKINGS_FIGHT_TMP_ASSIGN_ID   (1)

// 座標
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )
#define GRID_HALF_SIZE    ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

// ステージの座標
#define STAGE_X       (15*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)
#define STAGE_Y_HIGH  (22*FIELD_CONST_GRID_FX32_SIZE)
#define STAGE_Y_LOW   (12*FIELD_CONST_GRID_FX32_SIZE)
#define STAGE_Z       (7*FIELD_CONST_GRID_FX32_SIZE+GRID_HALF_SIZE)

#define STAGE_MOVE_SPD  (-7*FX32_ONE)    // 落下速度

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  FOURKINGS_SV_WORK   *p_sv_wk;       // セーブワーク
} FOURKINGS_FIGHT_TMP;

//-------------------------------------
///	ステージ落下イベント用ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ

  fx32                now_height;
  fx32                dst_height;
  fx32                add_val;
  MMDL                *mmdl;          // レンブ
} STAGE_FALL_EVENT_WORK;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び純
enum {
  RESID_DOOR_MDL = 0,       // ドアモデル    
  RESID_DOOR_ANM_ICA,       // ドアアニメ    
  RESID_LIGHT_MDL,          // ライトモデル
  RESID_LIGHT_ANM_ICA,      // ライトアニメ
  RESID_LIGHT_ANM_ITA,      // ライトアニメ
  RESID_LIGHT_ANM_ITP,      // ライトアニメ
  RESID_STAGE_MDL,          // ステージモデル
  RESID_STAIRS_MDL,          // 階段モデル
  RESID_STAIRS_01_ANM_ICA,   // 階段アニメ
  RESID_STAIRS_02_ANM_ICA,   // 階段アニメ
};

// OBJインデックス
enum {
  OBJ_DOOR,                 // ドア
  OBJ_LIGHT,                // ライト
  OBJ_STAGE,                // ステージ
  OBJ_STAIRS,                // 階段

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_door01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_door01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_light01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stage_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_FIGHT, NARC_fourkings_fight_stair02_nsbca, GFL_G3D_UTIL_RESARC },
};


// ANIME
static const GFL_G3D_UTIL_ANM anm_door[] =
{
  { RESID_DOOR_ANM_ICA, 0 },
};
#define ANM_DOOR_NUM  ( NELEMS(anm_door) )

static const GFL_G3D_UTIL_ANM anm_light[] =
{
  { RESID_LIGHT_ANM_ICA, 0 },
  { RESID_LIGHT_ANM_ITA, 0 },
  { RESID_LIGHT_ANM_ITP, 0 },
};
#define ANM_LIGHT_NUM  ( NELEMS(anm_light) )

static const GFL_G3D_UTIL_ANM anm_stair[] =
{
  { RESID_STAIRS_01_ANM_ICA, 0 },
  { RESID_STAIRS_02_ANM_ICA, 0 },
};
#define ANM_STAIRS_NUM  ( NELEMS(anm_stair) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_DOOR
  {
    RESID_DOOR_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_DOOR_MDL,       // テクスチャリソースID
    anm_door,             // アニメテーブル(複数指定のため)
    ANM_DOOR_NUM,         // アニメリソース数
  },
  // OBJ_LIGHT
  {
    RESID_LIGHT_MDL,       // モデルリソースID
    0,                     // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_MDL,       // テクスチャリソースID
    anm_light,             // アニメテーブル(複数指定のため)
    ANM_LIGHT_NUM,         // アニメリソース数
  },
  // OBJ_STAGE
  {
    RESID_STAGE_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_STAGE_MDL,      // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // OBJ_STAIRS
  {
    RESID_STAIRS_MDL,       // モデルリソースID
    0,                     // モデルデータID(リソース内部INDEX)
    RESID_STAIRS_MDL,       // テクスチャリソースID
    anm_stair,             // アニメテーブル(複数指定のため)
    ANM_STAIRS_NUM,         // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// INIT
// フィールド入りなおすたびに、セーブ状態による初期化
static void INIT_FieldRecover( FOURKINGS_FIGHT_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjDell( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjMain( FOURKINGS_FIGHT_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// DOOR
static void ANM_DOOR_LastFrame( FOURKINGS_FIGHT_TMP *p_wk );
// LIGHT
static void ANM_LIGHT_LastFrame( FOURKINGS_FIGHT_TMP *p_wk );
// STAIRS
static void ANM_STAIRS_Play( FOURKINGS_FIGHT_TMP *p_wk );
static void ANM_STAIRS_Vanish( FOURKINGS_FIGHT_TMP *p_wk );
static void ANM_STAIRS_LastFrame( FOURKINGS_FIGHT_TMP *p_wk );

// ステージ落下イベント
static GMEVENT * STAGE_FALL_CreateEvent( GAMESYS_WORK* gsys );
static GMEVENT_RESULT StageFallEvent( GMEVENT* event, int* seq, void* wk );

//-------------------------------------
///	セーブ情報
//=====================================
static void SAVE_SetPos1Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetPos1Flag( const FOURKINGS_SV_WORK *p_wk );
static void SAVE_SetPos2Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetPos2Flag( const FOURKINGS_SV_WORK *p_wk );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_FIGHT_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  // 汎用ワーク確保
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(FOURKINGS_FIGHT_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_sv_wk     = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_C09R0301 );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // セーブ状態別初期化
  INIT_FieldRecover( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_FIGHT_End(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_FIGHT_Move(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = STAGE_FALL_CreateEvent(gsys);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("ステージ落下イベント\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//-----------------------------------------------------------------------------
//          スクリプトコマンド関連
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  ドア開くアニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 */
//-----------------------------------------------------------------------------
void FOURKINGS_FIGHT_DoorAnm( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  // ドアアニメ再生
  ANM_Play( p_wk->exp_obj, OBJ_DOOR, 0, FALSE );

  // 下段のPOSを踏んだ
  SAVE_SetPos1Flag( p_wk->p_sv_wk, TRUE );

}
//----------------------------------------------------------------------------
/**
 *	@brief  階段アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id  0:表示  1:消滅
 */
//-----------------------------------------------------------------------------
void FOURKINGS_FIGHT_StairsAnm( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  GF_ASSERT_MSG( id<2, "_FOURKINGS_FIGHT_STAIRS 引数の指定が間違っている");

  if( id == 0 )
  {
    ANM_STAIRS_Play( p_wk );
  }
  else
  {
    ANM_STAIRS_Vanish( p_wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライト点灯アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 */
//-----------------------------------------------------------------------------
void FOURKINGS_FIGHT_LightAnm( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );
  int i;

  for( i=0; i<ANM_LIGHT_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, OBJ_LIGHT, i, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ステージ落下イベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* FOURKINGS_FIGHT_StageFall( GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_FIGHT_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  
  event = STAGE_FALL_CreateEvent( gsys );

  // 上段のPOSを踏んだ
  if( event != NULL )
  {
    SAVE_SetPos2Flag( p_wk->p_sv_wk, TRUE );
  }

  return event;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ状態による初期化
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void INIT_FieldRecover( FOURKINGS_FIGHT_TMP *p_wk )
{
  const BOOL pos1_flag = SAVE_GetPos1Flag( p_wk->p_sv_wk );
  const BOOL pos2_flag = SAVE_GetPos2Flag( p_wk->p_sv_wk );
  fx32  stage_y = STAGE_Y_HIGH;


  // セーブ状況別の状態
  // 何もPOS踏んでない
  // ⇒ ドア初期・階段初期・ライト初期
  // 下段POSふんだ
  // ⇒ ドア開く・階段競りあがる・ライト初期
  // 上段POSふんだ
  // ⇒ ドア開く・階段なくなる・ライト点滅・ステージ落下状態
  //

  // ドア表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_DOOR, FALSE );
  // ライト表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_LIGHT, FALSE );

  // 何も踏んでいない
  if( pos1_flag == FALSE && pos2_flag == FALSE )
  {
    // 階段表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_STAIRS, FALSE );
  }
  // 下段POS踏んだ
  else if( pos1_flag == TRUE && pos2_flag == FALSE )
  {
    // 階段
    ANM_STAIRS_LastFrame( p_wk );
    // ドア
    ANM_DOOR_LastFrame( p_wk );
  }
  // 上段POS踏んだ
  else if( pos2_flag == TRUE )
  {
    // ドア
    ANM_DOOR_LastFrame( p_wk );
    // ステージ
    stage_y = STAGE_Y_LOW;    // 落ちたとこにする
    // ライト
    ANM_LIGHT_LastFrame( p_wk );
  }

  
  // ステージの座標設定
  {
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_STAGE );

    // 座標
    status->trans.x = STAGE_X;
    status->trans.y = stage_y;
    status->trans.z = STAGE_Z;
  }
}

//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( FOURKINGS_FIGHT_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, FOURKINGS_FIGHT_UNIT_IDX);

  //=====================================
  // ドア
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_DOOR, ANM_DOOR_NUM );

  //=====================================
  // ライト
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT, ANM_LIGHT_NUM );

  //=====================================
  // ステージ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_STAGE, 0 );

  //=====================================
  // 階段
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_STAIRS, ANM_STAIRS_NUM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( FOURKINGS_FIGHT_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( FOURKINGS_FIGHT_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ初期設定
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj 3D拡張OBJポインタ
 *	@param  u16 obj_idx OBJインデックス
 *	@param	int anm_num アニメ数
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, MAP_HALF_SIZE,0,MAP_HALF_SIZE );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, TRUE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, FALSE );
  }
}

//=============================================================================
/**
 *          アニメ管理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 *	@param	loop      ループ再生フラグ(TRUEでループ
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ停止
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, 0 );
}

// DOOR
//----------------------------------------------------------------------------
/**
 *	@brief  ドアを開いた状態にする
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_DOOR_LastFrame( FOURKINGS_FIGHT_TMP *p_wk )
{
  u16 obj_idx = OBJ_DOOR;
  fx32 last_frame;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  int i;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_DOOR_NUM; i++ )
  {
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, TRUE );
    // 最終フレーム取得
    last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

    // ラストフレームにする
    FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, last_frame );
  }
}

// LIGHT
//----------------------------------------------------------------------------
/**
 *	@brief  ライト点灯状態にする
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_LIGHT_LastFrame( FOURKINGS_FIGHT_TMP *p_wk )
{
  u16 obj_idx = OBJ_LIGHT;
  fx32 last_frame;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  int i;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_LIGHT_NUM; i++ )
  {
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, TRUE );
    // 最終フレーム取得
    last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

    // ラストフレームにする
    FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, i, last_frame );
  }
}

// STAIRS
//----------------------------------------------------------------------------
/**
 *	@brief  階段競りあがるアニメ再生
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_STAIRS_Play( FOURKINGS_FIGHT_TMP *p_wk )
{
  u16 obj_idx = OBJ_STAIRS;

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  階段なくなるアニメ再生
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_STAIRS_Vanish( FOURKINGS_FIGHT_TMP *p_wk )
{
  u16 obj_idx = OBJ_STAIRS;

  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  階段が競りあがった状態で表示
 *
 *	@param	FOURKINGS_FIGHT_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_STAIRS_LastFrame( FOURKINGS_FIGHT_TMP *p_wk )
{
  fx32 last_frame;
  u16 obj_idx = OBJ_STAIRS;
  u16 anm_idx = 0;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 最終フレーム取得
  last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, FALSE );

  // ラストフレームにする
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, obj_idx, anm_idx, last_frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ステージ落下イベント作成
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * STAGE_FALL_CreateEvent( GAMESYS_WORK* gsys )
{
  GMEVENT*          event;
  STAGE_FALL_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, StageFallEvent, sizeof(STAGE_FALL_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  work->p_fieldmap  = p_fieldmap;

  // レンブのモデルを取得
  {
    MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    work->mmdl = MMDLSYS_SearchOBJID( mmdlsys, C09R0301_BIGFOUR3_01 );
    GF_ASSERT_MSG( work->mmdl, "レンブがみつからない\n");
  }

  // データ設定
  work->now_height = STAGE_Y_HIGH;
  work->dst_height = STAGE_Y_LOW;
  work->add_val = STAGE_MOVE_SPD;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ステージ落下イベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT StageFallEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  STAGE_FALL_EVENT_WORK *p_wk = (STAGE_FALL_EVENT_WORK*)wk;
  FOURKINGS_FIGHT_TMP *p_gmk_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, FOURKINGS_FIGHT_TMP_ASSIGN_ID );

  // 処理シーケンス
  enum{
    SEQ_START,
    SEQ_FALL,       // 落ち
    SEQ_END,
  };

  switch( *seq ){
  case SEQ_START:
    // ステージ表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_STAGE, FALSE );
    *seq = SEQ_FALL;
    break;
  // 落ち
  case SEQ_FALL:
    {
      // 高さ書き換え
      p_wk->now_height += p_wk->add_val;
      // 目的の高さに到達したか？
      if( p_wk->dst_height >= p_wk->now_height )
      {
        //目的高さで上書き
        p_wk->now_height = p_wk->dst_height;
        // 次のシーケンスへ
        *seq = SEQ_END;
      }
      // ステージ高さ更新
      {
        GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, FOURKINGS_FIGHT_UNIT_IDX, OBJ_STAGE );
        status->trans.y = p_wk->now_height;
      }
      // レンブの高さ更新
      {
        const VecFx32 *mmdl_pos;
        VecFx32 pos;
        int gy;
        //座標更新
        mmdl_pos = MMDL_GetVectorPosAddress( p_wk->mmdl );
        pos = *mmdl_pos;
        pos.y = p_wk->now_height;
        gy = SIZE_GRID_FX32( pos.y );
        MMDL_SetGridPosY( p_wk->mmdl, gy );
        MMDL_SetVectorPos( p_wk->mmdl, &pos );
      }
    }
    break;
  case SEQ_END:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *          セーブ情報
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント1起動フラグを設定
 *
 *	@param	p_wk  セーブワーク
 *	@param	flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static void SAVE_SetPos1Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag )
{
  p_wk->pos1_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント1起動フラグを取得
 *
 *	@param	p_wk  セーブワーク
 *
 *	@return flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetPos1Flag( const FOURKINGS_SV_WORK *p_wk )
{
  return p_wk->pos1_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント2起動フラグを設定
 *
 *	@param	p_wk  セーブワーク
 *	@param	flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static void SAVE_SetPos2Flag( FOURKINGS_SV_WORK *p_wk, BOOL flag )
{
  p_wk->pos2_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  POSイベント2起動フラグを取得
 *
 *	@param	p_wk  セーブワーク
 *
 *	@return flag  TRUEで起動した
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetPos2Flag( const FOURKINGS_SV_WORK *p_wk )
{
  return p_wk->pos2_flag;
}

