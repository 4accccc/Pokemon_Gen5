//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_evil.c
 *	@brief  四天王部屋:悪
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
#include "arc/fieldmap/fourkings_evil.naix"

// mine
#include "fourkings_evil.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FOURKINGS_EVIL_UNIT_IDX        (0)
#define FOURKINGS_EVIL_TMP_ASSIGN_ID   (1)

// 爪座標
#define CLAW_X     (256 << FX32_SHIFT )
#define CLAW_Y     (0 << FX32_SHIFT )
#define CLAW_Z     (256 << FX32_SHIFT )

// 炎座標
#define FIRE1_X    (552 << FX32_SHIFT )   // 炎(手前右)
#define FIRE1_Y    (224 << FX32_SHIFT )   // 炎(手前右)
#define FIRE1_Z    (416 << FX32_SHIFT )   // 炎(手前右)
#define FIRE2_X    (456 << FX32_SHIFT )   // 炎(手前左)
#define FIRE2_Y    (224 << FX32_SHIFT )   // 炎(手前左)
#define FIRE2_Z    (416 << FX32_SHIFT )   // 炎(手前左)
#define FIRE3_X    (568 << FX32_SHIFT )   // 炎(奥右)
#define FIRE3_Y    (224 << FX32_SHIFT )   // 炎(奥右)
#define FIRE3_Z    (336 << FX32_SHIFT )   // 炎(奥右)
#define FIRE4_X    (440 << FX32_SHIFT )   // 炎(奥左)
#define FIRE4_Y    (224 << FX32_SHIFT )   // 炎(奥左)
#define FIRE4_Z    (336 << FX32_SHIFT )   // 炎(奥左)

#define FIRE5_X    (546 << FX32_SHIFT )   // 炎(階段下)
#define FIRE5_Y    (28 << FX32_SHIFT )    // 炎(階段下)
#define FIRE5_Z    (720 << FX32_SHIFT )   // 炎(階段下)
#define FIRE6_X    (462 << FX32_SHIFT )   // 炎(階段下)
#define FIRE6_Y    (28 << FX32_SHIFT )    // 炎(階段下)
#define FIRE6_Z    (720 << FX32_SHIFT )   // 炎(階段下)

#define FIRE_NUM    (6)   // 炎の個数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	アニメ管理構造体
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  BOOL                play_flg;       // 再生フラグ 
  u16                 id;             // どの炎 0:階段下 1:階段上
} ANM_CONTROL_WORK;

//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  ANM_CONTROL_WORK    anm_wk;         // アニメ管理構造体
} FOURKINGS_EVIL_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_CLAW_MDL = 0,     // 爪モデル
  RESID_CLAW_ANM_ICA_1,   // 爪開くアニメ
  RESID_CLAW_ANM_ICA_2,   // 爪閉じるアニメ
  RESID_FIRE_MDL,         // 炎モデル
  RESID_FIRE_ANM_ICA_1,   // 炎点火アニメ
  RESID_FIRE_ANM_ITP_1,   // 炎点火アニメ
  RESID_FIRE_ANM_ICA_2,   // 炎ループアニメ
  RESID_FIRE_ANM_ITP_2,   // 炎ループアニメ
};

// OBJインデックス
enum {
  OBJ_CLAW,               // 爪
  OBJ_FIRE_1,             // 炎(手前)
  OBJ_FIRE_2,             // 炎(手前)
  OBJ_FIRE_3,             // 炎(奥)
  OBJ_FIRE_4,             // 炎(奥)
  OBJ_FIRE_5,             // 炎(階段下)
  OBJ_FIRE_6,             // 炎(階段下)

  OBJ_NUM,
};
// 階段下と階段上の炎のオブジェ開始インデックス
static const int start_obj_tbl[] =
{
  OBJ_FIRE_5,   // 階段下
  OBJ_FIRE_1,   // 階段上
};
// 階段下と階段上の炎のオブジェ終了インデックス
static const int end_obj_tbl[] =
{
  OBJ_FIRE_6,   // 階段下
  OBJ_FIRE_4,   // 階段上
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_claw_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_claw01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_claw02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_fire_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_fire01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_fire01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_fire02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_FOURKINGS_EVIL, NARC_fourkings_evil_fire02_nsbtp, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_claw[] =
{
  { RESID_CLAW_ANM_ICA_1, 0 },
  { RESID_CLAW_ANM_ICA_2, 0 },
};
#define CLAW_ANM_NUM  ( NELEMS(anm_claw) )

static const GFL_G3D_UTIL_ANM anm_fire[] =
{
  { RESID_FIRE_ANM_ICA_1, 0 },
  { RESID_FIRE_ANM_ITP_1, 0 },
  { RESID_FIRE_ANM_ICA_2, 0 },
  { RESID_FIRE_ANM_ITP_2, 0 },
};
#define FIRE_ANM_NUM  ( NELEMS(anm_fire) )
#define FIRE_ANMIDX_IGNITION_START  (0)   // 点火の開始アニメインデックス
#define FIRE_ANMIDX_IGNITION_END    (1)   // 点火の終了アニメインデックス
#define FIRE_ANMIDX_LOOP_START      (2)   // ループ炎の開始アニメインデックス
#define FIRE_ANMIDX_LOOP_END        (3)   // ループ炎の終了アニメインデックス

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_CLAW
  {
    RESID_CLAW_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_CLAW_MDL,       // テクスチャリソースID
    anm_claw,             // アニメテーブル(複数指定のため)
    CLAW_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_1
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_2
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_3
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_4
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_5
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
  },
  // OBJ_FIRE_6
  {
    RESID_FIRE_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_FIRE_MDL,       // テクスチャリソースID
    anm_fire,             // アニメテーブル(複数指定のため)
    FIRE_ANM_NUM,         // アニメリソース数
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
static void EXPOBJ_ObjSet( FOURKINGS_EVIL_TMP *p_wk );
static void EXPOBJ_ObjDell( FOURKINGS_EVIL_TMP *p_wk );
static void EXPOBJ_ObjMain( FOURKINGS_EVIL_TMP *p_wk );

// アニメ管理
static void ANM_FIRE_Play( FOURKINGS_EVIL_TMP *p_gmk_wk, ANM_CONTROL_WORK *p_wk, const u16 id );
static void ANM_FIRE_Main( ANM_CONTROL_WORK *p_wk );
static BOOL ANM_FIRE_CheckEnd( ANM_CONTROL_WORK *p_wk );
static void ANM_FIRE_SetEnable( ANM_CONTROL_WORK *p_wk, u16 id, u16 anm_type );

static void ANM_CLAW_Play( FOURKINGS_EVIL_TMP *p_wk, const u16 anm_idx );

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
void FOURKINGS_EVIL_Setup(FIELDMAP_WORK *p_fieldmap)
{
  // 汎用ワーク確保
  FOURKINGS_EVIL_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(FOURKINGS_EVIL_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;

  p_wk->anm_wk.exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->anm_wk.play_flg= FALSE;
  p_wk->anm_wk.id      = 0;

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
void FOURKINGS_EVIL_End(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_EVIL_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void FOURKINGS_EVIL_Move(FIELDMAP_WORK *p_fieldmap)
{
  FOURKINGS_EVIL_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID );

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      ANM_FIRE_Play( p_wk, &p_wk->anm_wk, 0 );
      ANM_FIRE_Play( p_wk, &p_wk->anm_wk, 1 );
      OS_Printf("炎再生\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      ANM_CLAW_Play( p_wk, 0 );
      OS_Printf("爪開く\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      ANM_CLAW_Play( p_wk, 1 );
      OS_Printf("爪閉じる\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );

  // 炎アニメ管理
  ANM_FIRE_Main( &p_wk->anm_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief  炎のアニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id  0:階段下  1:階段上
 */
//-----------------------------------------------------------------------------
void FOURKINGS_EVIL_FireAnm( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_EVIL_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID );

  GF_ASSERT_MSG( id<2, "_FOURKINGS_EVIL_FIRE 引数の指定が間違っている");

  // 炎アニメ再生
  ANM_FIRE_Play( p_wk, &p_wk->anm_wk, id );
  ICHI_Printf("炎アニメ id %d \n", id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  爪のアニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id  0:開く  1:閉じる
 */
//-----------------------------------------------------------------------------
void FOURKINGS_EVIL_ClawAnm( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FOURKINGS_EVIL_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, FOURKINGS_EVIL_TMP_ASSIGN_ID );

  GF_ASSERT_MSG( id<2, "_FOURKINGS_EVIL_CLAW 引数の指定が間違っている");

  // 爪アニメ再生
  ANM_CLAW_Play( p_wk, id );
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	FOURKINGS_EVIL_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( FOURKINGS_EVIL_TMP *p_wk )
{
  u8 i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, FOURKINGS_EVIL_UNIT_IDX);

  //=====================================
  // 爪
  {
    u16 obj_idx = OBJ_CLAW;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx );

    status->trans.x = CLAW_X;
    status->trans.y = CLAW_Y;
    status->trans.z = CLAW_Z;

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<CLAW_ANM_NUM; i++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, i );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, i, FALSE );
    }
  }

  //=====================================
  // 炎
  for( i=0; i<FIRE_NUM; i++ )
  {
    VecFx32 pos[FIRE_NUM] = {
      { FIRE1_X, FIRE1_Y, FIRE1_Z },
      { FIRE2_X, FIRE2_Y, FIRE2_Z },
      { FIRE3_X, FIRE3_Y, FIRE3_Z },
      { FIRE4_X, FIRE4_Y, FIRE4_Z },
      { FIRE5_X, FIRE5_Y, FIRE5_Z },
      { FIRE6_X, FIRE6_Y, FIRE6_Z },
    };

    u16 obj_idx = OBJ_FIRE_1 + i;    // OBJインデックス
    u16 anm_idx;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx );

    status->trans.x = pos[i].x;
    status->trans.y = pos[i].y;
    status->trans.z = pos[i].z;

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( anm_idx=0; anm_idx<FIRE_ANM_NUM; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	FOURKINGS_EVIL_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( FOURKINGS_EVIL_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	FOURKINGS_EVIL_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( FOURKINGS_EVIL_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

//=============================================================================
/**
 *          アニメ管理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメ再生
 *
 *	@param	FOURKINGS_EVIL_TMP *p_gmk_wk  ギミックワーク
 *	@param	ANM_CONTROL_WORK   *p_wk      アニメ管理ワーク
 *	@param  u16 id  どこの炎を再生させる  0:階段下 1:階段上
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_Play( FOURKINGS_EVIL_TMP *p_gmk_wk, ANM_CONTROL_WORK *p_wk, const u16 id )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_gmk_wk->exp_obj;
  u8 i;
  u8 anm_idx;

  const int start_obj_idx = start_obj_tbl[ id ];  // OBJインデックス 開始値
  const int end_obj_idx = end_obj_tbl[ id ];    // OBJインデックス 終了値

  // 状態を初期化
  for( i=start_obj_idx; i<=end_obj_idx; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス
    // 表示
    FLD_EXP_OBJ_SetVanish( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, FALSE );
  }

  // 炎点火にする
  ANM_FIRE_SetEnable( p_wk, id, 0 );

  p_wk->exp_obj = exp_obj;
  p_wk->play_flg= TRUE;
  p_wk->id      = id;
}
//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメメイン
 *
 *	@param	ANM_CONTROL_WORK *p_wk  アニメ管理ワーク
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_Main( ANM_CONTROL_WORK *p_wk )
{
  u8 i;
  u8 anm_idx;

  if( p_wk->play_flg == FALSE )
  {
    // 有効でないなら戻る
    return;
  }

  if( ANM_FIRE_CheckEnd(p_wk) == FALSE )
  {
    // アニメ終了していないので戻る
    return;
  }

  // ループ炎にする
  ANM_FIRE_SetEnable( p_wk, p_wk->id, 1 );

  // 再生終了
  p_wk->play_flg = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  点火アニメ終了チェック
 *
 *	@param	ANM_CONTROL_WORK *p_wk  アニメ管理ワーク 
 *
 *	@return TRUEでアニメ終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_FIRE_CheckEnd( ANM_CONTROL_WORK *p_wk )
{
  u8 i;
  u8 anm_idx;

  const int start_obj_idx = start_obj_tbl[ p_wk->id ];  // OBJインデックス 開始値
  const int end_obj_idx = end_obj_tbl[ p_wk->id ];    // OBJインデックス 終了値

  for( i=start_obj_idx; i<=end_obj_idx; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    // 点火のアニメ終了チェック
    for( anm_idx=FIRE_ANMIDX_IGNITION_START; anm_idx<=FIRE_ANMIDX_IGNITION_END; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      fx32 frm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );

      frm = FLD_EXP_OBJ_GetObjAnmFrm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );
      // 終了していないアニメを発見
      if( FLD_EXP_OBJ_ChkAnmEnd(exp_anm) == FALSE )
      {
        return FALSE;
      }
    }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  炎アニメ  設定
 *
 *	@param	ANM_CONTROL_WORK *p_wk  アニメ管理ワーク
 *	@param	id        どこの炎  0:階段下  1:階段上
 *	@param	anm_type  アニメ種類  0:点火  1:ループ
 */
//-----------------------------------------------------------------------------
static void ANM_FIRE_SetEnable( ANM_CONTROL_WORK *p_wk, u16 id, u16 anm_type )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = p_wk->exp_obj;
  u8 i;
  u8 anm_idx;

  const int start_obj_idx = start_obj_tbl[ id ];  // OBJインデックス 開始値
  const int end_obj_idx = end_obj_tbl[ id ];    // OBJインデックス 終了値

  // アニメ種類
  // アニメの状態を初期化
  for( i=start_obj_idx; i<=end_obj_idx; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    for( anm_idx=0; anm_idx<FIRE_ANM_NUM; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );
      // 停止
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, FALSE );
      // 頭だし
      FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, 0 );
    }
  }

  // 指定したアニメを再生
  for( i=start_obj_idx; i<=end_obj_idx; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    if( anm_type == 0 )
    {
      // アニメ再生(点火)
      for( anm_idx=FIRE_ANMIDX_IGNITION_START; anm_idx<=FIRE_ANMIDX_IGNITION_END; anm_idx++ )
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm;
        exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );
        // 再生
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
        // 無効化解除
        FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, TRUE );
        // 頭だし
        FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, 0 );
      }
    }
    else
    {
      // アニメ再生(ループ)
      for( anm_idx=FIRE_ANMIDX_LOOP_START; anm_idx<=FIRE_ANMIDX_LOOP_END; anm_idx++ )
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm;
        exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );
        // 再生
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        // 無効化解除
        FLD_EXP_OBJ_ValidCntAnm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, TRUE );
        // 頭だし
        FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, 0 );
      }
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  爪開くアニメ再生
 *
 *	@param	FOURKINGS_EVIL_TMP *p_wk ギミックワーク
 *	@param  u16 anm_idx   アニメインデックス（0:開く 1:閉じる）
 */
//-----------------------------------------------------------------------------
static void ANM_CLAW_Play( FOURKINGS_EVIL_TMP *p_wk, const u16 anm_idx )
{
  u16 obj_idx = OBJ_CLAW;   // OBJインデックス
  int i;


  // 一旦全て停止
  for( i=0; i<CLAW_ANM_NUM; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, i );

    // 停止 
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, i, FALSE );
  }

  // 指定したアニメ再生
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx );
    // 再生
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
    // 無効化解除
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, TRUE );
    // 頭だし
    FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, FOURKINGS_EVIL_UNIT_IDX, obj_idx, anm_idx, 0 );
  }
}

