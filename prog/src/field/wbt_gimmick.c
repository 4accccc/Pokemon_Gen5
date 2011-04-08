//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_gimmick.c
 *	@brief  WBT会場演出ギミック
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// system
#include "fieldmap.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "system/font2tex.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// script
#include "script_def.h"

// arc
#include "arc/arc_def.h"
#include "arc/fieldmap/wbt_gmk.naix"
#include "font/font.naix"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "wbt_gimmick.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define EXPOBJ_UNIT_IDX         (0)
#define EXPOBJ_UNIT_IDX_LOGO    (1)
#define EXPOBJ_UNIT_IDX_EFFECT  (2)

#define WBT_TMP_ASSIGN_ID   (1)

#define VISION_X  (368 << FX32_SHIFT)
#define VISION_Y  (0)
#define VISION_Z  (160 << FX32_SHIFT)
#define EFFECT_X  (256 << FX32_SHIFT)
#define EFFECT_X_MARGIN   (EFFECT_X*2)    // 差調整
#define EFFECT_Y  (0)
#define EFFECT_Z  (256 << FX32_SHIFT)


// script_def.hの優勝ビジョンの+1の値を入れる
// 要は、ギミックの初期化で読み込んでないリソースの始めの値。
#define TYPE_VISION_START      (SCR_WBT_VISION_ALL)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック一時ワーク
//=====================================
typedef struct {

  HEAPID    heapID;     // ヒープID

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ
  PLAYER_WORK         *p_player_wk;

  // ユニット追加フラグ
  BOOL      is_unit_logo;       // EXPOBJ_UNIT_IDX_LOGO
  BOOL      is_unit_effect;     // EXPOBJ_UNIT_IDX_EFFECT

  // 発動フラグ
  BOOL      move_e_win;         // 優勝エフェクト

} WBT_GMK_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
// naixの並び順と同じ
enum {
  RESID_V_VS_MDL = 0,       // VS
  RESID_V_VS_ITA,           // VS
  RESID_V_VS_NAME_MDL,      // VS 名前のところ
  RESID_V_WIN_MDL,          // 優勝
  RESID_V_WIN_ITA,          // 優勝
  RESID_V_WIN_NAME_MDL,     // 優勝 名前のところ

  // 優勝エフェクト
  RESID_E_WIN_A_MDL,        // 優勝 開始
  RESID_E_WIN_A_ICA,        // 優勝 開始
  RESID_E_WIN_A_ITA,        // 優勝 開始
  RESID_E_WIN_B_MDL,        // 優勝 ループ
  RESID_E_WIN_B_ICA,        // 優勝 ループ
  RESID_E_WIN_B_ITA,        // 優勝 ループ
  // ※ここから上の順番は変えない

  // トーナメントロゴ
  RESID_V_ALL_MDL,          // オールトーナメント
  RESID_V_ALL_ITA,          // オールトーナメント

  // エフェクト
  RESID_E_NORMAL_L_MDL,     // ノーマル
  RESID_E_NORMAL_L_IMA,     // ノーマル
  RESID_E_NORMAL_L_ICA,     // ノーマル
  RESID_E_NORMAL_L_ITA,     // ノーマル
  RESID_E_NORMAL_R_MDL,     // ノーマル
  RESID_E_NORMAL_R_IMA,     // ノーマル
  RESID_E_NORMAL_R_ICA,     // ノーマル
  RESID_E_NORMAL_R_ITA,     // ノーマル
};

// リソース指定の基準値
#define RESID_V_START_MDL   ( RESID_V_ALL_MDL )
#define RESID_V_START_ITA   ( RESID_V_ALL_ITA )
#define RESID_E_START_MDL   ( RESID_E_NORMAL_L_MDL )
#define RESID_E_START_IMA   ( RESID_E_NORMAL_L_IMA )
#define RESID_E_START_ICA   ( RESID_E_NORMAL_L_ICA )
#define RESID_E_START_ITA   ( RESID_E_NORMAL_L_ITA )

#define RES_V_NUM           (2)   // ビジョンのリソース数
#define RES_E_NUM           (4)   // エフェクトのリソース数
#define RES_E_WIN_NUM       (3)   // 優勝エフェクトのリソース数

// OBJインデックス
// ユニット別
enum {
  OBJ_V_VS = 0,       // VS
  OBJ_V_VS_NAME,      // VS 名前
  OBJ_V_WIN,          // 優勝ビジョン
  OBJ_V_WIN_NAME,     // 優勝ビジョン 名前
  OBJ_E_WIN_START,    // 優勝エフェクト開始
  OBJ_E_WIN_LOOP,     // 優勝エフェクトループ

  OBJ_V_MAX,
};

// トーナメントロゴ
enum {
  OBJ_LOGO = 0,       // ロゴ

  OBJ_LOGO_MAX,
};

// 通路エフェクト
enum {
  OBJ_EFFECT = 0,     // エフェクト

  OBJ_EFFECT_MAX,
};


static const GFL_G3D_UTIL_RES res_3d_table[] = 
{
  // VS   ビジョン
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_vs_name_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  // 優勝 ビジョン
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_v_win_name_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  // 優勝 エフェクト
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_a_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_WBT_GIMMICK_DATA, NARC_wbt_gmk_wbt_e_win_b_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_vs_table[] =
{
  { RESID_V_VS_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_v_win_table[] =
{
  { RESID_V_WIN_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_e_win_a_table[] =
{
  { RESID_E_WIN_A_ICA, 0 }, // nsbca
  { RESID_E_WIN_A_ITA, 0 }, // nsbta
};
static const GFL_G3D_UTIL_ANM anm_e_win_b_table[] =
{
  { RESID_E_WIN_B_ICA, 0 }, // nsbca
  { RESID_E_WIN_B_ITA, 0 }, // nsbta
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // VS
  {
    RESID_V_VS_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_VS_MDL,       // テクスチャリソースID
    anm_vs_table,         // アニメテーブル(複数指定のため)
    NELEMS(anm_vs_table), // アニメリソース数
  },
  // VS 名前
  {
    RESID_V_VS_NAME_MDL,  // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_VS_NAME_MDL,  // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // WIN
  {
    RESID_V_WIN_MDL,      // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_WIN_MDL,      // テクスチャリソースID
    anm_v_win_table,        // アニメテーブル(複数指定のため)
    NELEMS(anm_v_win_table),// アニメリソース数
  },
  // WIN 名前
  {
    RESID_V_WIN_NAME_MDL, // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_V_WIN_NAME_MDL, // テクスチャリソースID
    NULL,                 // アニメテーブル(複数指定のため)
    0,                    // アニメリソース数
  },
  // WIN エフェクト
  {
    RESID_E_WIN_A_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_WIN_A_MDL,    // テクスチャリソースID
    anm_e_win_a_table,    // アニメテーブル(複数指定のため)
    NELEMS(anm_e_win_a_table),// アニメリソース数
  },
  // WIN エフェクト
  {
    RESID_E_WIN_B_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_E_WIN_B_MDL,    // テクスチャリソースID
    anm_e_win_b_table,    // アニメテーブル(複数指定のため)
    NELEMS(anm_e_win_b_table),// アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				//リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				//オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//-------------------------------------
///	テクスチャデータ
//=====================================
typedef enum
{
  TEX_INDEX_RIVAL,    // 相手の名前
  TEX_INDEX_MY,       // 自分の名前
  TEX_INDEX_WIN,      // 優勝者の名前

  TEX_INDEX_MAX,
}TEX_INDEX;
static char* tex_name[ TEX_INDEX_MAX ] =
{
  "wbt_name01",
  "wbt_name02",
  "wbt_name03",
};
static char* plt_name[ TEX_INDEX_MAX ] =
{
  "wbt_name01_pl",
  "wbt_name02_pl",
  "wbt_name03_pl",
};

//=============================================================================
/**
 *          static関数宣言
 */
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk );

// 背景ビジョン
static void VISION_Setup( WBT_GMK_TMP *p_wk, u8 type );
static void VISION_SetupName( WBT_GMK_TMP *p_wk, u8 type, STRBUF *strbuf );
static BOOL VISION_Font2Tex( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, STRBUF *strbuf, HEAPID heapID );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, WBT_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(WBT_GMK_TMP) );

  p_wk->heapID      = HEAPID_FIELDMAP;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_player_wk = GAMEDATA_GetMyPlayerWork( p_gdata );

  p_wk->is_unit_logo  = FALSE;
  p_wk->is_unit_effect= FALSE;

  // ギミックは発動させない
  p_wk->move_e_win  = FALSE;

  // EXPOBJ初期化
  EXPOBJ_ObjSet( p_wk );

  ICHI_Printf("gimmick init\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, WBT_TMP_ASSIGN_ID);

  ICHI_Printf("gimmick end\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 背景ビジョン変更
 *	        優勝ビジョンなど
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 *	@param  strbuf      対戦相手名
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_VisionChange( FIELDMAP_WORK *p_fieldmap, u8 type, STRBUF *strbuf )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // 既存の表示ビジョンを非表示
  {
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS_NAME, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN_NAME, TRUE );
    if( p_wk->is_unit_logo )
    {
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, OBJ_LOGO, TRUE );
    }
  }

  // ビジョン別設定
  switch( type ){
  case SCR_WBT_VISION_VS:
  case SCR_WBT_VISION_WIN:
    // 名前張り付けのビジョンなら
    // リソースは読み込まない
    VISION_SetupName( p_wk, type, strbuf );
    break;

  default:
    // トーナメントロゴのビジョンなら
    // リソースの設定から行う
    VISION_Setup( p_wk, type );

    break;
  }

  ICHI_Printf("背景ビジョン変更 %d \n", type);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 通路エフェクト開始
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_EffectStart( FIELDMAP_WORK *p_fieldmap, u8 type, u8 pos )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  GFL_G3D_UTIL_RES    res_e_table[RES_E_NUM] = {0};
  GFL_G3D_UTIL_ANM    anm_e_table[RES_E_NUM-1] = {0};
  GFL_G3D_UTIL_OBJ    obj_e_table = {0};
  GFL_G3D_UTIL_SETUP  setup = {0};
  u8      res_cnt = 0;
  u8      anm_cnt = 0;
  // リソースIDの開始位置テーブル
  // ※並び替え不可
  const   res_start_tbl[ RES_E_NUM ] = {
    RESID_E_START_MDL,    // モデル
    RESID_E_START_IMA,    // IMA
    RESID_E_START_ICA,    // ICA
    RESID_E_START_ITA,    // ITA
  };

  // RES
  for( res_cnt=0; res_cnt<RES_E_NUM; res_cnt++ )
  {
    res_e_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
    res_e_table[ res_cnt ].datID   = res_start_tbl[ res_cnt ] + type + (pos*RES_E_NUM);
    res_e_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  }

  // ANIME
  anm_e_table[ anm_cnt ].anmresID = 1;
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table[ anm_cnt ].anmresID = 2;
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;
  anm_e_table[ anm_cnt ].anmresID = 3;
  anm_e_table[ anm_cnt ].anmdatID = 0;
  anm_cnt++;

  // OBJ
  obj_e_table.mdlresID = 0;
  obj_e_table.mdldatID = 0;
  obj_e_table.texresID = 0;
  obj_e_table.anmTbl   = anm_e_table;
  obj_e_table.anmCount = anm_cnt;

  // セットアップデータ
  setup.resTbl  = res_e_table;
  setup.resCount= res_cnt;
  setup.objTbl  = &obj_e_table;
  setup.objCount= 1;


  //必要なリソースの用意
  // 仮にすでにリソース設定された後なら削除
  if( p_wk->is_unit_effect )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT );
  }
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &setup, EXPOBJ_UNIT_IDX_EFFECT );
  p_wk->is_unit_effect = TRUE;    // 作成した
  
  //=====================================
  // Effectの設定
  {
    u16     obj_idx = OBJ_EFFECT;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx );

    // 座標
    status->trans.x = EFFECT_X + (pos*EFFECT_X_MARGIN); // 右側は、ずれているので調整
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<RES_E_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

  ICHI_Printf("通路エフェクト開始 %d pos:%d \n", type, pos);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ギミックスタート 優勝エフェクト開始
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_WinEffectStart( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // 優勝のアニメ再生
  {
    u16     obj_idx = OBJ_E_WIN_START;    // OBJインデックス
    u8      i;

    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    for( i=0; i<RES_E_WIN_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }
  // 一応ループの方を非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, TRUE );

  p_wk->move_e_win = TRUE;

  ICHI_Printf("優勝エフェクト開始\n");
}


//=============================================================================
/**
 *        static関数
 */
//=============================================================================
//=============================================================================
/**
 *        EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk )
{

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, EXPOBJ_UNIT_IDX);

  //=====================================
  // VSの設定
  {
    u16     obj_idx = OBJ_V_VS;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = VISION_X;
    status->trans.y = VISION_Y;
    status->trans.z = VISION_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }

  //=====================================
  // VS 名前の設定
  {
    u16     obj_idx = OBJ_V_VS_NAME;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );
    // 座標
    status->trans.x = VISION_X;
    status->trans.y = VISION_Y;
    status->trans.z = VISION_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
  }

  //=====================================
  // WINの設定
  {
    u16     obj_idx = OBJ_V_WIN;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = VISION_X;
    status->trans.y = VISION_Y;
    status->trans.z = VISION_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }
  //=====================================
  // WIN 名前の設定
  {
    u16     obj_idx = OBJ_V_WIN_NAME;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );
    // 座標
    status->trans.x = VISION_X;
    status->trans.y = VISION_Y;
    status->trans.z = VISION_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
  }

  //=====================================
  // WIN エフェクトの設定
  {
    u16     obj_idx = OBJ_E_WIN_START;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = EFFECT_X;
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングしない
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // アニメ再生
    for( i=0; i<RES_E_WIN_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }

  //=====================================
  // WIN エフェクトの設定
  {
    u16     obj_idx = OBJ_E_WIN_LOOP;    // OBJインデックス
    u8      i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = EFFECT_X;
    status->trans.y = EFFECT_Y;
    status->trans.z = EFFECT_Z;
    // カリングしない
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, TRUE );
    // アニメ再生
    for( i=0; i<RES_E_WIN_NUM-1; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX );
  if( p_wk->is_unit_logo )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO );
  }
  if( p_wk->is_unit_effect )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_EFFECT );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );


  // 優勝エフェクト
  if( p_wk->move_e_win == TRUE )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_START, 0 );
    // アニメ終了したら、ループ用に切り替え
    if( FLD_EXP_OBJ_ChkAnmEnd( exp_anm ) == TRUE )
    {
      u8 i;
      // 開始エフェクトは消す
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_START, TRUE );

      // ループエフェクトは表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, FALSE );
      // ループエフェクトアニメ設定
      for( i=0; i<RES_E_WIN_NUM-1; i++ )
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, i );
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_E_WIN_LOOP, i, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
      }

      // フラグOFF
      p_wk->move_e_win = FALSE;
    }
  }
}

//=============================================================================
/**
 *        背景ビジョン
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンセットアップ
 *
 *	@param	WBT_GMK_TMP *p_wk   ギミックワーク
 *	@param	type                ビジョン種類
 */
//-----------------------------------------------------------------------------
static void VISION_Setup( WBT_GMK_TMP *p_wk, u8 type )
{
  GFL_G3D_UTIL_RES    res_v_table[RES_V_NUM] = {0};
  GFL_G3D_UTIL_ANM    anm_v_table = {0};
  GFL_G3D_UTIL_OBJ    obj_v_table = {0};
  GFL_G3D_UTIL_SETUP  setup = {0};
  u8      res_cnt = 0;

  // リソースのインデックスをトーナメントビジョンの先頭まで移動
  type = type - TYPE_VISION_START;

  // RES
  res_v_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
  res_v_table[ res_cnt ].datID   = RESID_V_START_MDL + type;
  res_v_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  res_cnt++;
  res_v_table[ res_cnt ].arcive  = ARCID_WBT_GIMMICK_DATA;
  res_v_table[ res_cnt ].datID   = RESID_V_START_ITA + type;
  res_v_table[ res_cnt ].arcType = GFL_G3D_UTIL_RESARC;
  res_cnt++;

  // ANIME
  anm_v_table.anmresID = 1;
  anm_v_table.anmdatID = 0;

  // OBJ
  obj_v_table.mdlresID = 0;
  obj_v_table.mdldatID = 0;
  obj_v_table.texresID = 0;
  obj_v_table.anmTbl   = &anm_v_table;
  obj_v_table.anmCount = 1;

  // セットアップデータ
  setup.resTbl  = res_v_table;
  setup.resCount= res_cnt;
  setup.objTbl  = &obj_v_table;
  setup.objCount= 1;

  //必要なリソースの用意
  {
    // 仮にすでにリソース設定された後に呼ばれたら削除する
    if( p_wk->is_unit_logo )
    {
      FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO );
    }
  }
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &setup, EXPOBJ_UNIT_IDX_LOGO );
  p_wk->is_unit_logo = TRUE;    // 作成した


  //=====================================
  // LOGOの設定
  {
    u16     obj_idx = OBJ_LOGO;    // OBJインデックス
    VecFx32 pos = { VISION_X, VISION_Y, VISION_Z };

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, obj_idx );

    // 座標
    status->trans = pos;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX_LOGO, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

  ICHI_Printf("背景ビジョンリソース読み込み %d \n", type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンセットアップ
 *	        名前貼り付け有り
 *
 *	@param	WBT_GMK_TMP *p_wk   ギミックワーク
 *	@param	type                ビジョン種類
 *	@param	strbuf              対戦相手名
 */
//-----------------------------------------------------------------------------
static void VISION_SetupName( WBT_GMK_TMP *p_wk, u8 type, STRBUF *strbuf )
{
  u16   obj_idx;    // OBJインデックス

  // 名前貼り付けビジョンは
  // VSか優勝だけ
  if( type == SCR_WBT_VISION_WIN )
  {
    obj_idx = OBJ_V_WIN;
  }
  else
  {
    obj_idx = OBJ_V_VS;
  }


  // アニメ再生
  {
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

  // 名前の設定
  {
    BOOL ret;
    STRBUF  *str_player;

    str_player = MyStatus_CreateNameString( &p_wk->p_player_wk->mystatus, p_wk->heapID );

    switch( obj_idx ){
    case OBJ_V_VS:
      {
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_RIVAL, strbuf, p_wk->heapID );
        GF_ASSERT(ret);
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_MY, str_player, p_wk->heapID );
        GF_ASSERT(ret);

        FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_VS_NAME, FALSE );
      }
      break;

    case OBJ_V_WIN:
      {
        ret = VISION_Font2Tex( p_wk, TEX_INDEX_WIN, str_player, p_wk->heapID );
        GF_ASSERT(ret);

        FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, OBJ_V_WIN_NAME, FALSE );
      }
      break;
    }

    GFL_HEAP_FreeMemory( str_player );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビジョンに名前貼り付け
 *
 *	@param	const GFL_G3D_RES* g3Dtex
 *	@param	char* tex_name
 *	@param	char* plt_name
 *	@param	STRBUF* str
 *	@param	xpos
 *	@param	ypos
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL VISION_Font2Tex( WBT_GMK_TMP *p_wk, TEX_INDEX tex_index, STRBUF *strbuf, HEAPID heapID )
{
  F2T_WORK      f2t_wk;
  BOOL          ret;
  PRINTSYS_LSB  lsb;

  GFL_FONT*     fontHandle;

  GFL_G3D_OBJ   *p_obj;
  GFL_G3D_RND   *p_rnd;
  GFL_G3D_RES   *p_res;
  u8            obj_idx;

  u32           len = 0;

  // どのテクスチャに貼り付けるか
  // 文字も変える
  switch( tex_index ){
  case TEX_INDEX_MY:
    obj_idx = OBJ_V_VS_NAME;
    break;
    
  case TEX_INDEX_RIVAL:
    obj_idx = OBJ_V_VS_NAME;
    break;

  case TEX_INDEX_WIN:
    obj_idx = OBJ_V_WIN_NAME;
    break;
  }

  // 文字幅計算
  {
    //フォントハンドル作成
    fontHandle = GFL_FONT_Create
      (ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID);
    len = PRINTSYS_GetStrWidth(strbuf, fontHandle, 0);
    ICHI_Printf("nameLen %d \n", len );
    GFL_FONT_Delete(fontHandle);
  }

  p_obj = FLD_EXP_OBJ_GetUnitObj( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx );
  p_rnd = GFL_G3D_OBJECT_GetG3Drnd( p_obj );
  p_res = GFL_G3D_RENDER_GetG3DresTex( p_rnd );

  lsb = PRINTSYS_LSB_Make(1,2,0);

  // テクスチャに文字貼り付け
  // 76はテクスチャの見えてる範囲の幅
  ret = F2T_CopyString( p_res, tex_name[tex_index], NULL, plt_name[tex_index],
      strbuf, ((76/2) - (len/2)), 0, lsb, heapID, &f2t_wk );


  return ret;
}

