//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_esp.c
 *	@brief  四天王部屋:エスパー
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
#include "arc/fieldmap/fourkings_esp.naix"

// mine
#include "fourkings_sv.h"
#include "fourkings_esp.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FOURKINGS_ESP_UNIT_IDX        (0)
#define FOURKINGS_ESP_TMP_ASSIGN_ID   (1)

// 座標
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )

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
} FOURKINGS_ESP_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_CUBE_MDL = 0,      // キューブ
  RESID_CUBE_01_ANM_ICA,      // キューブ回転(ループ)
  RESID_CUBE_01_ANM_ITA,      // キューブ回転(ループ)
  RESID_CUBE_02_ANM_IMA,      // キューブ登場
  RESID_FLOWER_MDL,           // 花
  RESID_FLOWER_ANM_ICA,       // 花
  RESID_FLOWER_ANM_IMA,       // 花
  RESID_FLOWER_ANM_ITA,       // 花
  RESID_FLOWER_ANM_ITP,       // 花
  RESID_RIPPLE_MDL,           // 波紋
  RESID_RIPPLE_ANM_ITA,       // 波紋
  RESID_STAIRS_01_MDL,        // 滝
  RESID_STAIRS_01_ANM_ITA,    // 滝(ループ)
  RESID_STAIRS_02_ANM_ICA,    // 滝(開く)
  RESID_STAIRS_02_ANM_IMA,    // 滝(開く)
  RESID_STAIRS_03_MDL,        // 階段を隠している幕
  RESID_STAIRS_03_ANM_ICA,    // 幕(開く)
  RESID_STAIRS_03_ANM_ITA,    // 幕(開く)
  RESID_STAIRS_04_MDL,        // 階段
  RESID_STAIRS_04_ANM_ITA,    // 階段(ループ)
  RESID_STAIRS_05_ANM_IMA,    // 階段(消える)
};

// OBJインデックス
enum {
  OBJ_CUBE,                   // キューブ
  OBJ_FLOWER,                 // 花
  OBJ_RIPPLE,                 // 波紋
  OBJ_WATERFALL,              // 滝
  OBJ_CURTAIN,                // 階段を隠している幕
  OBJ_STAIRS,                 // 階段

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_cube01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_cube01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_cube01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_cube02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_flower_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_flower_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_flower_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_flower_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_flower_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_ripple_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_ripple_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs03_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs03_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs03_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs04_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs04_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_ESP, NARC_fourkings_esp_stairs05_nsbma, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_cube[] =
{
  { RESID_CUBE_01_ANM_ICA, 0 },
  { RESID_CUBE_01_ANM_ITA, 0 },
  { RESID_CUBE_02_ANM_IMA, 0 },
};
#define CUBE_ANM_NUM  ( NELEMS(anm_cube) )

static const GFL_G3D_UTIL_ANM anm_flower[] =
{
  { RESID_FLOWER_ANM_ICA, 0 },
  { RESID_FLOWER_ANM_IMA, 0 },
  { RESID_FLOWER_ANM_ITA, 0 },
  { RESID_FLOWER_ANM_ITP, 0 },
};
#define FLOWER_ANM_NUM  ( NELEMS(anm_flower) )

static const GFL_G3D_UTIL_ANM anm_ripple[] =
{
  { RESID_RIPPLE_ANM_ITA, 0 },
};
#define RIPPLE_ANM_NUM  ( NELEMS(anm_ripple) )

static const GFL_G3D_UTIL_ANM anm_waterfall[] =
{
  { RESID_STAIRS_01_ANM_ITA, 0 },
  { RESID_STAIRS_02_ANM_ICA, 0 },
  { RESID_STAIRS_02_ANM_IMA, 0 },
};
#define WATERFALL_ANM_NUM  ( NELEMS(anm_waterfall) )

static const GFL_G3D_UTIL_ANM anm_curtain[] =
{
  { RESID_STAIRS_03_ANM_ICA, 0 },
  { RESID_STAIRS_03_ANM_ITA, 0 },
};
#define CURTAIN_ANM_NUM  ( NELEMS(anm_curtain) )

static const GFL_G3D_UTIL_ANM anm_stairs[] =
{
  { RESID_STAIRS_04_ANM_ITA, 0 },
  { RESID_STAIRS_05_ANM_IMA, 0 },
};
#define STAIRS_ANM_NUM  ( NELEMS(anm_stairs) )


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_CUBE
  {
    RESID_CUBE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_CUBE_MDL,       // テクスチャリソースID
    anm_cube,             // アニメテーブル(複数指定のため)
    CUBE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FLOWER
  {
    RESID_FLOWER_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FLOWER_MDL,     // テクスチャリソースID
    anm_flower,           // アニメテーブル(複数指定のため)
    FLOWER_ANM_NUM,       // アニメリソース数
  },
  // OBJ_RIPPLE
  {
    RESID_RIPPLE_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_RIPPLE_MDL,     // テクスチャリソースID
    anm_ripple,           // アニメテーブル(複数指定のため)
    RIPPLE_ANM_NUM,       // アニメリソース数
  },
  // OBJ_WATERFALL
  {
    RESID_STAIRS_01_MDL,  // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_STAIRS_01_MDL,  // テクスチャリソースID
    anm_waterfall,        // アニメテーブル(複数指定のため)
    WATERFALL_ANM_NUM,    // アニメリソース数
  },
  // OBJ_CURTAIN
  {
    RESID_STAIRS_03_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_STAIRS_03_MDL,    // テクスチャリソースID
    anm_curtain,          // アニメテーブル(複数指定のため)
    CURTAIN_ANM_NUM,      // アニメリソース数
  },
  // OBJ_STAIRS
  {
    RESID_STAIRS_04_MDL,     // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_STAIRS_04_MDL,     // テクスチャリソースID
    anm_stairs,           // アニメテーブル(複数指定のため)
    STAIRS_ANM_NUM,       // アニメリソース数
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
// EXPOBJ
static void EXPOBJ_ObjSet( FOURKINGS_ESP_TMP *p_wk );
static void EXPOBJ_ObjDell( FOURKINGS_ESP_TMP *p_wk );
static void EXPOBJ_ObjMain( FOURKINGS_ESP_TMP *p_wk );

static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// CUBE
static void ANM_CUBE_Play( FOURKINGS_ESP_TMP *p_wk );
static BOOL ANM_CUBE_CheckEnd( FOURKINGS_ESP_TMP *p_wk );
static void ANM_CUBE_PlayLoop( FOURKINGS_ESP_TMP *p_wk );
// FLOWER
static void ANM_FLOWER_Play( FOURKINGS_ESP_TMP *p_wk );
// RIPPLE
static void ANM_RIPPLE_Play( FOURKINGS_ESP_TMP *p_wk );
// WATERFALL
static void ANM_WATERFALL_PlayLoop( FOURKINGS_ESP_TMP *p_wk );
static void ANM_WATERFALL_Play( FOURKINGS_ESP_TMP *p_wk );
static BOOL ANM_WATERFALL_CheckEnd( FOURKINGS_ESP_TMP *p_wk );
// CURTAIN
static void ANM_CURTAIN_Play( FOURKINGS_ESP_TMP *p_wk );
static BOOL ANM_CURTAIN_CheckEnd( FOURKINGS_ESP_TMP *p_wk );
// STAIRS
static void ANM_STAIRS_PlayLoop( FOURKINGS_ESP_TMP *p_wk );
static void ANM_STAIRS_Play( FOURKINGS_ESP_TMP *p_wk );


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
void FOURKINGS_ESP_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  // 汎用ワーク確保
  FOURKINGS_ESP_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, FOURKINGS_ESP_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(FOURKINGS_ESP_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_sv_wk     = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_C09R0401 );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 初期設定
  ANM_WATERFALL_PlayLoop( p_wk );
  ANM_STAIRS_PlayLoop( p_wk );
  // 幕表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, OBJ_CURTAIN, FALSE );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_ESP_End(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_ESP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_ESP_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, FOURKINGS_ESP_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_ESP_Move(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_ESP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_ESP_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      ANM_FLOWER_Play( p_wk );
      OS_Printf("花再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      ANM_RIPPLE_Play( p_wk );
      OS_Printf("波紋再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      ANM_CURTAIN_Play( p_wk );
      OS_Printf("幕再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      ANM_CUBE_Play( p_wk );
      OS_Printf("キューブ再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      ANM_CUBE_PlayLoop( p_wk );
      OS_Printf("キューブ再生ループ\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      ANM_WATERFALL_Play( p_wk );
      OS_Printf("滝開く再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      ANM_STAIRS_Play( p_wk );
      OS_Printf("階段消える再生\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
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
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( FOURKINGS_ESP_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, FOURKINGS_ESP_UNIT_IDX);

  //=====================================
  // キューブ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_CUBE, CUBE_ANM_NUM );

  //=====================================
  // 花
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_FLOWER, FLOWER_ANM_NUM );

  //=====================================
  // 波紋
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_RIPPLE, RIPPLE_ANM_NUM );

  //=====================================
  // 滝
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_WATERFALL, WATERFALL_ANM_NUM );

  //=====================================
  // 幕
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_CURTAIN, CURTAIN_ANM_NUM );

  //=====================================
  // 階段
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_STAIRS, STAIRS_ANM_NUM );

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( FOURKINGS_ESP_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( FOURKINGS_ESP_TMP *p_wk )
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
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, MAP_HALF_SIZE,0,MAP_HALF_SIZE );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, TRUE );
  // 非表示
  FLD_EXP_OBJ_SetVanish( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, TRUE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, i, FALSE );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, anm_idx, 0 );
}
//=============================================================================
/**
 *          キューブ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  キューブ登場アニメ再生
 *
 *	@param	*p_wk   ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_CUBE_Play( FOURKINGS_ESP_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_CUBE;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );

  // アニメ
  // 0 回転アニメ
  // 1 回転アニメ
  // 2 登場アニメ
  {
    ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
    ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
    ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  キューブ登場アニメ終了チェック
 *
 *	@param	*p_wk   ギミックワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_CUBE_CheckEnd( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_CUBE;

  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, 2 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}

//----------------------------------------------------------------------------
/**
 *	@brief  キューブ回転(ループ)アニメ再生
 *
 *	@param	*p_wk   ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_CUBE_PlayLoop( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_CUBE;

  // アニメ
  // 0 回転アニメ
  // 1 回転アニメ
  // 2 登場アニメ
  {
    ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
    ANM_Play( p_wk->exp_obj, obj_idx, 1, TRUE );
    ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  }
}

//=============================================================================
/**
 *          花
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  花アニメ再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_FLOWER_Play( FOURKINGS_ESP_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_FLOWER;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );
  
  for( i=0; i<FLOWER_ANM_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}

//=============================================================================
/**
 *          波紋
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  波紋アニメ再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_RIPPLE_Play( FOURKINGS_ESP_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_RIPPLE;
  
  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<RIPPLE_ANM_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}

//=============================================================================
/**
 *          滝
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  滝アニメ(ループ)再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_WATERFALL_PlayLoop( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_WATERFALL;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );

  // アニメ
  // 0 ループアニメ
  // 1 開くアニメ
  // 2 開くアニメ
  {
    ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
    ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
    ANM_Stop( p_wk->exp_obj, obj_idx, 2 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  滝開くアニメ再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_WATERFALL_Play( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_WATERFALL;

  // アニメ
  // 0 ループアニメ
  // 1 開くアニメ
  // 2 開くアニメ
  {
    ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
    ANM_Play( p_wk->exp_obj, obj_idx, 2, FALSE );
    ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  滝開くアニメ終了チェック
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_WATERFALL_CheckEnd( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_WATERFALL;

  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, 2 );

  return FLD_EXP_OBJ_ChkAnmEnd(exp_anm);
}

//=============================================================================
/**
 *          幕
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  幕開くアニメ再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_CURTAIN_Play( FOURKINGS_ESP_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_CURTAIN;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<CURTAIN_ANM_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  幕開くアニメ終了チェック
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_CURTAIN_CheckEnd( FOURKINGS_ESP_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_CURTAIN;

  for( i=0; i<CURTAIN_ANM_NUM; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, i );
    // 終了していないアニメを発見
    if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
    {
      return FALSE;
    }
  }

  return TRUE;
}

//=============================================================================
/**
 *          階段
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  階段アニメ(ループ)再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_STAIRS_PlayLoop( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_STAIRS;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_ESP_UNIT_IDX, obj_idx, FALSE );
  
  // 0 ループアニメ
  // 1 消えるアニメ
  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 ); 
}

//----------------------------------------------------------------------------
/**
 *	@brief  階段消えるアニメ再生
 *
 *	@param	FOURKINGS_ESP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void ANM_STAIRS_Play( FOURKINGS_ESP_TMP *p_wk )
{
  u16 obj_idx = OBJ_STAIRS;

  // 0 ループアニメ
  // 1 消えるアニメ
  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
}
