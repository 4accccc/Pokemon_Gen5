//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_champion.c
 *	@brief  チャンピオン部屋
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/champion.naix"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "field_gimmick_champion.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define CHAMPION_UNIT_IDX        (0)
#define CHAMPION_TMP_ASSIGN_ID   (1)

// 座標関係
#define MAP_SIZE          (512 << FX32_SHIFT )
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

} CHAMPION_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_ROOM_00_MDL,        // 照明点灯前マップモデル
  RESID_ROOM_01_MDL,        // 照明点灯後マップモデル
  RESID_RELIEF_00_MDL,      // 照明点灯前レリーフモデル
  RESID_RELIEF_01_MDL,      // 照明点灯後レリーフ
  RESID_RELIEF_01_ANM_ICA,  // 通常時レリーフ回転アニメ
};

// OBJインデックス
enum{
  OBJ_ROOM_1,               // マップ(点灯前)
  OBJ_ROOM_2,               // マップ(点灯後)
  OBJ_RELIEF_1,             // レリーフ(点灯前)
  OBJ_RELIEF_2,             // レリーフ(点灯後)
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_CHAMPION, NARC_champion_m_champ00_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_m_champ01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief00_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief_01_nsbca, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_relief[] =
{
  { RESID_RELIEF_01_ANM_ICA, 0 },
};
#define ANM_RELIEF_NUM  ( NELEMS(anm_relief) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_ROOM_1
  {
    RESID_ROOM_00_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_ROOM_00_MDL,      // テクスチャリソースID
    NULL,                   // アニメテーブル(複数指定のため)
    0,                      // アニメリソース数
  },
  // OBJ_ROOM_2
  {
    RESID_ROOM_01_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_ROOM_01_MDL,      // テクスチャリソースID
    NULL,                   // アニメテーブル(複数指定のため)
    0,                      // アニメリソース数
  },
  // OBJ_RELIEF_1
  {
    RESID_RELIEF_00_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_RELIEF_00_MDL,    // テクスチャリソースID
    anm_relief,             // アニメテーブル(複数指定のため)
    ANM_RELIEF_NUM,         // アニメリソース数
  },
  // OBJ_RELIEF_2
  {
    RESID_RELIEF_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_RELIEF_01_MDL,    // テクスチャリソースID
    anm_relief,             // アニメテーブル(複数指定のため)
    ANM_RELIEF_NUM,         // アニメリソース数
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
static void EXPOBJ_ObjSet( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjDell( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjMain( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, BOOL vanish );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

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
void CHAMPION_Setup( FIELDMAP_WORK *p_fieldmap )
{
  // 汎用ワーク確保
  CHAMPION_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(CHAMPION_TMP) );

  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;

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
void CHAMPION_End( FIELDMAP_WORK *p_fieldmap )
{
  CHAMPION_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void CHAMPION_Move( FIELDMAP_WORK *p_fieldmap )
{
  CHAMPION_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      OS_Printf("デバッグ 点灯前状態\n");

      // 表示設定
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_1, FALSE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_2, TRUE );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      OS_Printf("デバッグ 点灯後状態\n");

      // 表示設定
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_1, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_2, FALSE );
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
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( CHAMPION_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, CHAMPION_UNIT_IDX);

  //=====================================
  // マップ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_ROOM_1, 0, FALSE );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_ROOM_2, 0, TRUE );

  //=====================================
  // レリーフ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_RELIEF_1, ANM_RELIEF_NUM, FALSE );
  // ループアニメは再生しておく
  ANM_Play( p_wk->exp_obj, OBJ_RELIEF_1, 0, TRUE );

  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_RELIEF_2, ANM_RELIEF_NUM, TRUE );

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( CHAMPION_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, CHAMPION_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( CHAMPION_TMP *p_wk )
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
 *	@param  BOOL vanish 表示設定
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, BOOL vanish )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, CHAMPION_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, (MAP_HALF_SIZE), 0, (MAP_SIZE+MAP_HALF_SIZE) );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, CHAMPION_UNIT_IDX, obj_idx, TRUE );
  // 表示設定
  FLD_EXP_OBJ_SetVanish( exp_obj, CHAMPION_UNIT_IDX, obj_idx, vanish );

  // 
  if( anm_num == 0 )
  {
    // アニメがないなら設定しない
    return;
  }

  // アニメ再生
  //  1回再生 無効化
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, i, FALSE );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, 0 );
}


