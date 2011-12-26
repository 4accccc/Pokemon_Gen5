//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_c12.c
 *	@brief  ヒオウギシティ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"
#include "gamesystem/pm_season.h"   //季節定義参照

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/intro_demo.naix"
#include "arc/fieldmap/ending.naix"

// mine
#include "field_gimmick_c12.h"

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

#define POS_X     ( 744 << FX32_SHIFT )
#define POS_Y     ( 16 << FX32_SHIFT )
#define POS_Z     ( 12216 << FX32_SHIFT )

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

  BOOL                bg_init;
  BOOL                bg_init_check;
} C12_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_INTRO_DEMO_SP_MDL,      // 春
  RESID_INTRO_DEMO_SP_ANM_ICA,  //
  RESID_INTRO_DEMO_SP_ANM_IMA,  //
  RESID_INTRO_DEMO_SU_MDL,      // 夏
  RESID_INTRO_DEMO_SU_ANM_ICA,  //
  RESID_INTRO_DEMO_SU_ANM_IMA,  //
  RESID_INTRO_DEMO_AU_MDL,      // 秋
  RESID_INTRO_DEMO_AU_ANM_ICA,  //
  RESID_INTRO_DEMO_AU_ANM_IMA,  //
  RESID_INTRO_DEMO_WI_MDL,      // 冬
  RESID_INTRO_DEMO_WI_ANM_ICA,  //
  RESID_INTRO_DEMO_WI_ANM_IMA,  //
};

// OBJインデックス
enum{
  OBJ_INTRO_DEMO_SP,            // 春
  OBJ_INTRO_DEMO_SU,            // 夏
  OBJ_INTRO_DEMO_AU,            // 秋
  OBJ_INTRO_DEMO_WI,            // 冬

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_sp_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_sp_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_sp_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_su_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_su_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_su_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_au_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_au_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_au_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_wi_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_wi_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_DEMO_GIMMICK, NARC_intro_demo_intro_gm1_wi_nsbma, GFL_G3D_UTIL_RESARC },
};

// ANIME
// 春
static const GFL_G3D_UTIL_ANM anm_intro_demo_sp[] =
{
  { RESID_INTRO_DEMO_SP_ANM_ICA, 0 },
  { RESID_INTRO_DEMO_SP_ANM_IMA, 0 },
};
#define ANM_INTRO_DEMO_SP_NUM  ( NELEMS(anm_intro_demo_sp) )
// 夏
static const GFL_G3D_UTIL_ANM anm_intro_demo_su[] =
{
  { RESID_INTRO_DEMO_SU_ANM_ICA, 0 },
  { RESID_INTRO_DEMO_SU_ANM_IMA, 0 },
};
#define ANM_INTRO_DEMO_SU_NUM  ( NELEMS(anm_intro_demo_su) )
// 秋
static const GFL_G3D_UTIL_ANM anm_intro_demo_au[] =
{
  { RESID_INTRO_DEMO_AU_ANM_ICA, 0 },
  { RESID_INTRO_DEMO_AU_ANM_IMA, 0 },
};
#define ANM_INTRO_DEMO_AU_NUM  ( NELEMS(anm_intro_demo_au) )
// 冬
static const GFL_G3D_UTIL_ANM anm_intro_demo_wi[] =
{
  { RESID_INTRO_DEMO_WI_ANM_ICA, 0 },
  { RESID_INTRO_DEMO_WI_ANM_IMA, 0 },
};
#define ANM_INTRO_DEMO_WI_NUM  ( NELEMS(anm_intro_demo_wi) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_INTRO_DEMO_SP
  {
    RESID_INTRO_DEMO_SP_MDL,// モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_INTRO_DEMO_SP_MDL,// テクスチャリソースID
    anm_intro_demo_sp,      // アニメテーブル(複数指定のため)
    ANM_INTRO_DEMO_SP_NUM  ,// アニメリソース数
  },
  // OBJ_INTRO_DEMO_SU
  {
    RESID_INTRO_DEMO_SU_MDL,// モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_INTRO_DEMO_SU_MDL,// テクスチャリソースID
    anm_intro_demo_su,      // アニメテーブル(複数指定のため)
    ANM_INTRO_DEMO_SU_NUM  ,// アニメリソース数
  },
  // OBJ_INTRO_DEMO_AU
  {
    RESID_INTRO_DEMO_AU_MDL,// モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_INTRO_DEMO_AU_MDL,// テクスチャリソースID
    anm_intro_demo_au,      // アニメテーブル(複数指定のため)
    ANM_INTRO_DEMO_AU_NUM  ,// アニメリソース数
  },
  // OBJ_INTRO_DEMO_WI
  {
    RESID_INTRO_DEMO_WI_MDL,// モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_INTRO_DEMO_WI_MDL,// テクスチャリソースID
    anm_intro_demo_wi,      // アニメテーブル(複数指定のため)
    ANM_INTRO_DEMO_WI_NUM  ,// アニメリソース数
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
  ANM_INTRO_DEMO_SP_NUM,
  ANM_INTRO_DEMO_SU_NUM,
  ANM_INTRO_DEMO_AU_NUM,
  ANM_INTRO_DEMO_WI_NUM,
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// BG
static void BG_Load( C12_GIMMICK_TMP *p_wk );
static void BG_Exit( C12_GIMMICK_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( C12_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( C12_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( C12_GIMMICK_TMP *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

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
void C12_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  C12_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(C12_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->bg_init     = FALSE;
  p_wk->bg_init_check = FALSE;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void C12_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  C12_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  if( p_wk->bg_init_check == TRUE )
  {
    // BG破棄
    BG_Exit( p_wk );
    ICHI_Printf("BG_Unload \n");
  }

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
void C12_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  C12_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  if( p_wk->bg_init == TRUE )
  {
    // BG初期化
    BG_Load( p_wk );

    p_wk->bg_init = FALSE;
    p_wk->bg_init_check = TRUE;
    ICHI_Printf("BG_Load \n");
  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys
 */
//-----------------------------------------------------------------------------
void C12_GIMMICK_AnmStart( GAMESYS_WORK *gsys )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  C12_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  u8  season;

  // 季節の取得
  season = GAMEDATA_GetSeasonID( p_gdata );

  switch( season ){
  case PMSEASON_SPRING:
  case PMSEASON_SUMMER:
  case PMSEASON_AUTUMN:
  case PMSEASON_WINTER:
    // 季節とOBJの順番は一緒
    ANM_Play( p_wk->exp_obj, season );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	GAMESYS_WORK *gsys
 */
//-----------------------------------------------------------------------------
void C12_GIMMICK_Init( GAMESYS_WORK *gsys )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  C12_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  
  // シネマ枠のBGを表示する
  p_wk->bg_init = TRUE;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//=============================================================================
/**
 *          BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シネマ枠のBG読み込み
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void BG_Load( C12_GIMMICK_TMP *p_wk )
{
  HEAPID heap_id = FIELDMAP_GetHeapID( p_wk->p_fieldmap );


  GFL_BG_FreeBGControl( FLDBG_MFRM_EFF2 );

#if 1
  // BGのセットアップ
  {
    //BG Frame
    GFL_BG_BGCNT_HEADER bgcntText = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, FLDBG_MFRM_EFF2_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( FLDBG_MFRM_EFF2, &bgcntText, GFL_BG_MODE_TEXT );
  }
#endif

  // BGリソース読み込み
  {
    ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ENDING_GIMMICK, GFL_HEAP_LOWID( heap_id ) );

    GFL_ARCHDL_UTIL_TransVramPalette( 
        ah, NARC_ending_ed_back_NCLR, PALTYPE_MAIN_BG, 0, 0, heap_id );
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_ending_ed_back_NCGR, FLDBG_MFRM_EFF2, 0, 0, FALSE, heap_id );
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_ending_ed_back_NSCR, FLDBG_MFRM_EFF2, 0, 0, FALSE, heap_id );

    GFL_ARC_CloseDataHandle( ah );

    GFL_BG_LoadScreenReq( FLDBG_MFRM_EFF2 );
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG破棄
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void BG_Exit( C12_GIMMICK_TMP *p_wk )
{
  GFL_BG_FreeBGControl( FLDBG_MFRM_EFF2 );
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
 *	@param	C12_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( C12_GIMMICK_TMP *p_wk )
{
  int obj_idx;
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  for( obj_idx=0; obj_idx<OBJ_NUM; obj_idx++ )
  {
    int anm_num = anm_num_tbl[ obj_idx ];
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, POS_X, POS_Y, POS_Z );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<anm_num; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, FALSE );
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	C12_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( C12_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	C12_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( C12_GIMMICK_TMP *p_wk )
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
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;
  int anm_num = anm_num_tbl[ obj_idx ];

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

    // 有効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, TRUE );
    // 頭だし
    FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, 0 );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
  }
}

