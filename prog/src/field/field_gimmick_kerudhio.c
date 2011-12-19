//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_kerudhio.c
 *	@brief  【配布】ケルディオイベント
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.12.19
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
#include "arc/fieldmap/kerudhio.naix"

// mine
#include "field_gimmick_kerudhio.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GIMMICK_UNIT_IDX        (0)
#define GIMMICK_TMP_ASSIGN_ID   (1)

// 座標
#define MAP_SIZE          (512 << FX32_SHIFT )
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )
#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

// 座標
// 気合のエフェクト
#define EFF_POS_X       ( 248 << FX32_SHIFT )
#define EFF_POS_Y       ( 0 )
#define EFF_POS_Z       ( 232 << FX32_SHIFT )
// ビリジオン
#define BIRIZION_POS_X  ( 280 << FX32_SHIFT )
#define BIRIZION_POS_Y  ( 0 )
#define BIRIZION_POS_Z  ( 232  << FX32_SHIFT )
// コバルオン
#define KOBARUON_POS_X  ( 248 << FX32_SHIFT )
#define KOBARUON_POS_Y  ( 48  << FX32_SHIFT )
#define KOBARUON_POS_Z  ( 232 << FX32_SHIFT )
// テラキオン
#define TERAKION_POS_X  ( 216 << FX32_SHIFT )
#define TERAKION_POS_Y  ( 0 )
#define TERAKION_POS_Z  ( 232 << FX32_SHIFT )


// ケルディオ演出イベント関係
#define KERUDHIO_EFFECT_WAIT    (120)     // 気合のエフェクト表示フレーム数
#define KERUDHIO_DELETE_WAIT    (40)      // 3匹同時に消えるまでの表示フレーム数

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

} KERUDHIO_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_EFFECT_01_MDL,      // 気合のエフェクト
  RESID_EFFECT_01_ANM_ITA,  // 気合のエフェクト アニメ
  RESID_BIRIZION_MDL,       // ビリジオン モデル
  RESID_KOBARUON_MDL,       // コバルオン モデル
  RESID_TERAKION_MDL,       // テラキオン モデル
  RESID_EFFECT_APPEAR_IMA,  // 出現アニメ
  RESID_EFFECT_APPEAR_IVA,  // 出現アニメ
  RESID_EFFECT_STAY_IMA,    // 待機アニメ
  RESID_EFFECT_STAY_IVA,    // 待機アニメ
  RESID_EFFECT_DISAPPEAR_ICA,// 消えるアニメ
  RESID_EFFECT_DISAPPEAR_IMA,// 消えるアニメ
  RESID_EFFECT_DISAPPEAR_IVA,// 消えるアニメ

  RESI_MAX,
};

// OBJインデックス
enum{
  OBJ_EFFECT_01,            // 気合のエフェクト
  OBJ_BIRIZION,             // ビリジオン
  OBJ_KOBARUON,             // コバルオン
  OBJ_TERAKION,             // テラキオン

  OBJ_NUM,
};


// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_effect_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_effect_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_bili_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_koba_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_tera_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_appear_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_appear_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_stay_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_stay_nsbva, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_KERUDHIO_GIMMICK, NARC_kerudhio_disappear_nsbva, GFL_G3D_UTIL_RESARC },
};


// ANIME
// ケルディオ気合
static const GFL_G3D_UTIL_ANM anm_effect_01[] =
{
  { RESID_EFFECT_01_ANM_ITA, 0 },
};
#define ANM_EFFECT_01_NUM  ( NELEMS(anm_effect_01) )

// 三銃士アニメ
static const GFL_G3D_UTIL_ANM anm_effect_02[] =
{
  { RESID_EFFECT_APPEAR_IMA, 0 },
  { RESID_EFFECT_APPEAR_IVA, 0 },
  { RESID_EFFECT_STAY_IMA, 0 },
  { RESID_EFFECT_STAY_IVA, 0 },
  { RESID_EFFECT_DISAPPEAR_ICA, 0 },
  { RESID_EFFECT_DISAPPEAR_IMA, 0 },
  { RESID_EFFECT_DISAPPEAR_IVA, 0 },
};
#define ANM_EFFECT_02_NUM  ( NELEMS(anm_effect_02) )

// アニメの順
enum{
  ANM_IDX_APPEAR_IMA,   // 出現
  ANM_IDX_APPEAR_IVA,   // 出現
  ANM_IDX_STAY_IMA,     // 待機
  ANM_IDX_STAY_IVA,     // 待機
  ANM_IDX_DISAPPEAR_ICA,// 消える
  ANM_IDX_DISAPPEAR_IMA,// 消える
  ANM_IDX_DISAPPEAR_IVA,// 消える
};


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_EFFECT_01
  {
    RESID_EFFECT_01_MDL,  // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_EFFECT_01_MDL,  // テクスチャリソースID
    anm_effect_01,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_01_NUM,    // アニメリソース数
  },
  // OBJ_BIRIZION
  {
    RESID_BIRIZION_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_BIRIZION_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
  // OBJ_KOBARUON
  {
    RESID_KOBARUON_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_KOBARUON_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
  // OBJ_TERAKION
  {
    RESID_TERAKION_MDL,   // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_TERAKION_MDL,   // テクスチャリソースID
    anm_effect_02,        // アニメテーブル(複数指定のため)
    ANM_EFFECT_02_NUM,    // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

// OBJとアニメ総数の対応テーブル
static const int anm_num_tbl[ OBJ_NUM ] = {
  ANM_EFFECT_01_NUM,
  ANM_EFFECT_02_NUM,
  ANM_EFFECT_02_NUM,
  ANM_EFFECT_02_NUM,
};

// 配置座標テーブル
static const VecFx32 pos_tbl[ OBJ_NUM ] = {
  { EFF_POS_X, EFF_POS_Y, EFF_POS_Z },
  { BIRIZION_POS_X, BIRIZION_POS_Y, BIRIZION_POS_Z },
  { KOBARUON_POS_X, KOBARUON_POS_Y, KOBARUON_POS_Z },
  { TERAKION_POS_X, TERAKION_POS_Y, TERAKION_POS_Z },
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( KERUDHIO_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( KERUDHIO_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( KERUDHIO_GIMMICK_TMP *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// 三銃士で共通のアニメ
// 登場アニメ
static void ANM_AppearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
static BOOL ANM_AppearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
// 待機アニメ
static void ANM_Stay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
// 消えるアニメ
static void ANM_DisappearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );
static BOOL ANM_DisappearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

///	ケルディオ演出イベント
// イベントワーク
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  int                 wait;           // ウェイト値
} KERUDHIO_EVENT_WORK;

static GMEVENT * KERUDHIO_CreateEvent( GAMESYS_WORK* gsys );
static GMEVENT_RESULT KerudhioEvent( GMEVENT* event, int* seq, void* wk );

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
void KERUDHIO_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  KERUDHIO_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(KERUDHIO_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
  OS_Printf("ケルディオギミック\n");
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void KERUDHIO_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  KERUDHIO_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void KERUDHIO_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  KERUDHIO_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      GMEVENT *event;
      event = KERUDHIO_CreateEvent(gsys);
      GAMESYSTEM_SetEvent(gsys, event);
      OS_Printf("ケルディオ演出\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      KERUDHIO_GIMMICK_EffectDelete(gsys);
      OS_Printf("エフェクト削除\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出イベント
 *
 *	@param	GAMESYS_WORK *gsys
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* KERUDHIO_GIMMICK_Event( GAMESYS_WORK *gsys )
{
  GMEVENT *event;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  KERUDHIO_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // イベント作成
  event = KERUDHIO_CreateEvent( gsys );
  
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  気合のエフェクトを削除
 *
 *	@param	GAMESYS_WORK *gsys 
 */
//-----------------------------------------------------------------------------
void KERUDHIO_GIMMICK_EffectDelete( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  KERUDHIO_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  int i;

  // 気合エフェクト 非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EFFECT_01, TRUE );
  // 全アニメ停止
  for( i=0; i<anm_num_tbl[ OBJ_EFFECT_01 ]; i++ )
  {
    ANM_Stop( p_wk->exp_obj, OBJ_EFFECT_01, i );
  }
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	KERUDHIO_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( KERUDHIO_GIMMICK_TMP *p_wk )
{
  u16 obj_idx;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ
  for( obj_idx=0; obj_idx<OBJ_NUM; obj_idx++ )
  {
    int anm_idx;
    int anm_num = anm_num_tbl[ obj_idx ];

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, pos_tbl[obj_idx].x, pos_tbl[obj_idx].y, pos_tbl[obj_idx].z );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ設定
    for( anm_idx=0; anm_idx<anm_num; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx, FALSE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	KERUDHIO_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( KERUDHIO_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	KERUDHIO_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( KERUDHIO_GIMMICK_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

//=============================================================================
/**
 *          ANIME
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param  anm_idx   アニメインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  int anm_num = anm_num_tbl[ obj_idx ];

  GF_ASSERT( anm_idx < anm_num );

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx );

  // 1回再生設定
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定

  // 有効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, anm_idx, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通 登場アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_AppearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_APPEAR_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_APPEAR_IVA );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  登場アニメ終了チェック
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_AppearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, ANM_IDX_APPEAR_IMA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, ANM_IDX_APPEAR_IVA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  待機アニメ
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 */
//-----------------------------------------------------------------------------
static void ANM_Stay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_STAY_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_STAY_IVA );
}

//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  消えるアニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 */
//-----------------------------------------------------------------------------
static void ANM_DisappearPlay( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;

  // 念のため全てのアニメ停止
  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Stop( exp_obj, obj_idx, i );
  }

  // 登場アニメ再生
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_ICA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_IMA );
  ANM_Play( exp_obj, obj_idx, ANM_IDX_DISAPPEAR_IVA );
}
//----------------------------------------------------------------------------
/**
 *	@brief  三銃士共通  消えるアニメ終了チェック
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_DisappearCheckEnd( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_ICA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_IMA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, ANM_IDX_DISAPPEAR_IVA );
  // 終了していないアニメを発見
  if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}

//=============================================================================
/**
 *          イベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出のイベント作成
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * KERUDHIO_CreateEvent( GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  KERUDHIO_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, KerudhioEvent, sizeof(KERUDHIO_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  work->p_fieldmap  = p_fieldmap;
  work->wait        = 0;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ演出のイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT KerudhioEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK  *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  KERUDHIO_EVENT_WORK *p_wk = (KERUDHIO_EVENT_WORK*)wk;
  KERUDHIO_GIMMICK_TMP *p_gmk_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  int i;

  // 処理シーケンス
  enum{
    SEQ_START,          // イベント開始
    SEQ_EFFECT_01,      // 気合のアニメ
    SEQ_EFFECT_01_WAIT, // 気合のアニメ待ち
    SEQ_TERAKION,       // テラキオン表示
    SEQ_TERAKION_WAIT,  // テラキオン表示待ち
    SEQ_BIRIJION,       // ビリジオン表示
    SEQ_BIRIJION_WAIT,  // ビリジオン表示待ち
    SEQ_KOBARUON,       // コバルオン表示
    SEQ_KOBARUON_WAIT,  // コバルオン表示待ち
    SEQ_ALL_DELETE,     // 3匹同時に消える
    SEQ_ALL_DELETE_WAIT,// 3匹同時に消える待ち
    SEQ_END,            // イベント終了
  };

  switch( *seq ){
  case SEQ_START:
    *seq = SEQ_EFFECT_01;
    break;

  case SEQ_EFFECT_01:      // 気合のアニメ
    {
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EFFECT_01, FALSE );
      // 全アニメ再生
      for( i=0; i<anm_num_tbl[ OBJ_EFFECT_01 ]; i++ )
      {
        ANM_Play( p_wk->exp_obj, OBJ_EFFECT_01, i );
      }
      p_wk->wait = KERUDHIO_EFFECT_WAIT;
    }
    *seq = SEQ_EFFECT_01_WAIT;
    break;

  case SEQ_EFFECT_01_WAIT: // 気合のアニメ待ち
    if( p_wk->wait-- <= 0 )
    {
      *seq = SEQ_TERAKION;
    }
    break;

  case SEQ_TERAKION:       // テラキオン表示
    ANM_AppearPlay( p_wk->exp_obj, OBJ_TERAKION );
    *seq = SEQ_TERAKION_WAIT;
    break;

  case SEQ_TERAKION_WAIT:  // テラキオン表示待ち
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_TERAKION ) )
    {
      ANM_Stay( p_wk->exp_obj, OBJ_TERAKION );
      *seq = SEQ_BIRIJION;
    }
    break;

  case SEQ_BIRIJION:       // ビリジオン表示
    ANM_AppearPlay( p_wk->exp_obj, OBJ_BIRIZION );
    *seq = SEQ_BIRIJION_WAIT;
    break;

  case SEQ_BIRIJION_WAIT:  // ビリジオン表示待ち
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_BIRIZION ) )
    {
      ANM_Stay( p_wk->exp_obj, OBJ_BIRIZION );
      *seq = SEQ_KOBARUON;
    }
    break;

  case SEQ_KOBARUON:       // コバルオン表示
    ANM_AppearPlay( p_wk->exp_obj, OBJ_KOBARUON );
    *seq = SEQ_KOBARUON_WAIT;
    break;

  case SEQ_KOBARUON_WAIT:  // コバルオン表示待ち
    if( ANM_AppearCheckEnd( p_wk->exp_obj, OBJ_KOBARUON ) )
    {
      ANM_Stay( p_wk->exp_obj, OBJ_KOBARUON );

      p_wk->wait = KERUDHIO_DELETE_WAIT;
      *seq = SEQ_ALL_DELETE;
    }
    break;

  case SEQ_ALL_DELETE:     // 3匹同時に消える
    if( p_wk->wait-- <= 0 )
    {
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_TERAKION );
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_BIRIZION );
      ANM_DisappearPlay( p_wk->exp_obj, OBJ_KOBARUON );
      *seq = SEQ_ALL_DELETE_WAIT;
    }
    break;

  case SEQ_ALL_DELETE_WAIT:// 3匹同時に消える待ち
    if( ANM_DisappearCheckEnd( p_wk->exp_obj, OBJ_TERAKION) )
    {
      // 非表示に
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_TERAKION, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BIRIZION, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_KOBARUON, TRUE );
      *seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}
