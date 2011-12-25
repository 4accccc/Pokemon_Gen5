//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_ending.c
 *	@brief  エンディング
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.12.06
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
#include "arc/fieldmap/ending.naix"

// mine
#include "field_gimmick_ending.h"

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

// 花火座標
#define FIREWORKS_POS_X    ( 512 << FX32_SHIFT )
#define FIREWORKS_POS_Y    ( 768 << FX32_SHIFT )
#define FIREWORKS_POS_Z    ( -256 << FX32_SHIFT )

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
} ENDING_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_FIREWORKS_MDL,       // 花火
  RESID_FIREWORKS_ANM_ICA,   //
  RESID_FIREWORKS_ANM_IMA,   //
  RESID_FIREWORKS_ANM_ITA,   //
};

// OBJインデックス
enum{
  OBJ_FIREWORKS,             // 花火

  OBJ_NUM,
};


// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_ENDING_GIMMICK, NARC_ending_ed_hanabi01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_ENDING_GIMMICK, NARC_ending_ed_hanabi01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_ENDING_GIMMICK, NARC_ending_ed_hanabi01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_ENDING_GIMMICK, NARC_ending_ed_hanabi01_nsbta, GFL_G3D_UTIL_RESARC },
};


// ANIME
static const GFL_G3D_UTIL_ANM anm_fireworks[] =
{
  { RESID_FIREWORKS_ANM_ICA, 0 },
  { RESID_FIREWORKS_ANM_IMA, 0 },
  { RESID_FIREWORKS_ANM_ITA, 0 },
};
#define ANM_FIREWORKS_NUM  ( NELEMS(anm_fireworks) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_FIREWORKS
  {
    RESID_FIREWORKS_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_FIREWORKS_MDL,    // テクスチャリソースID
    anm_fireworks,          // アニメテーブル(複数指定のため)
    ANM_FIREWORKS_NUM,      // アニメリソース数
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

// BG
static void BG_Load( ENDING_GIMMICK_TMP *p_wk );
static void BG_Exit( ENDING_GIMMICK_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( ENDING_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( ENDING_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( ENDING_GIMMICK_TMP *p_wk );

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
void ENDING_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(ENDING_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->bg_init     = FALSE;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // ライトを夜に設定する
  {
    FIELD_LIGHT *p_light = FIELDMAP_GetFieldLight( p_fieldmap );
    FIELD_LIGHT_COLORFADE_StartOneWay( p_light, GX_RGB(11, 11, 16), 1 );
  }

  OS_Printf("エンディングギミックセットアップ\n");
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void ENDING_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // BG破棄
  BG_Exit( p_wk );
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
void ENDING_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  if( p_wk->bg_init == FALSE )
  {
    // BG初期化
    BG_Load( p_wk );

    p_wk->bg_init = TRUE;
    ICHI_Printf("BG_Load \n");
  }
  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  花火再生
 *
 *	@param	GAMESYS_WORK *gsys
 */
//-----------------------------------------------------------------------------
void ENDING_GIMMICK_FireWorks( GAMESYS_WORK *gsys )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  ENDING_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // 花火再生
  ANM_Play( p_wk->exp_obj, OBJ_FIREWORKS );

  ICHI_Printf("花火再生\n");
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
static void BG_Load( ENDING_GIMMICK_TMP *p_wk )
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
static void BG_Exit( ENDING_GIMMICK_TMP *p_wk )
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
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( ENDING_GIMMICK_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ_FIREWORKS
  {
    u16 obj_idx = OBJ_FIREWORKS;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, FIREWORKS_POS_X, FIREWORKS_POS_Y, FIREWORKS_POS_Z );

    // カリング
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<ANM_FIREWORKS_NUM; i++ )
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
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( ENDING_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	ENDING_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( ENDING_GIMMICK_TMP *p_wk )
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
 *	@brief  花火アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx )
{
  int i;
  int anm_num = ANM_FIREWORKS_NUM;

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
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
  }
}


