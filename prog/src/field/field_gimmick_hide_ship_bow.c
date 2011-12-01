//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_hide_ship_bow.c
 *	@brief  アジト船(船首)ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.26
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
#include "arc/fieldmap/hide_ship_engine.naix"

// sound
#include "sound/pm_sndsys.h"

// script
#include "../../../resource/fldmapdata/flagwork/work_define.h"

// mine
#include "field_gimmick_hide_ship_bow.h"
#include "field_gimmick_hide_ship_engine_se_def.h"    // 使いまわし

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define HIDE_SHIP_UNIT_IDX        (0)
#define HIDE_SHIP_TMP_ASSIGN_ID   (1)

// 座標関係
#define GRID_HALF_SIZE  ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)
// ボタン
#define BUTTON_NUM      ( 4 )   // ボタン数
#define BUTTON_1_X      ( 6 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン1 X座標
#define BUTTON_1_Z      ( 7 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン1 Z座標
#define BUTTON_2_X      ( 6 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン2 X座標
#define BUTTON_2_Z      (12 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン2 Z座標
#define BUTTON_3_X      (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン3 X座標
#define BUTTON_3_Z      ( 7 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン3 Z座標
#define BUTTON_4_X      (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン4 X座標
#define BUTTON_4_Z      (12 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE ) // ボタン4 Z座標
// ゲート
#define GATE_NUM        ( 4 )   // ゲート数
#define GATE_1_X        (11 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート1 X座標
#define GATE_1_Z        (7 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート1 Z座標
#define GATE_2_X        (11 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート2 X座標
#define GATE_2_Z        (6 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート2 Z座標
#define GATE_3_X        (11 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート3 X座標
#define GATE_3_Z        (5 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート3 Z座標
#define GATE_4_X        (11 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE) // ゲート4 X座標
#define GATE_4_Z        (4 * FIELD_CONST_GRID_FX32_SIZE ) // ゲート4 Z座標


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

} HIDE_SHIP_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_BUTTON_MDL = 0,         // ボタンモデル
  RESID_BUTTON_ANM_OFF,         // ボタンOFFアニメ
  RESID_BUTTON_ANM_ON,          // ボタンONアニメ
  RESID_GATE_MDL,               // ゲートモデル
  RESID_GATE_ANM_ITA,           // ゲート常時アニメ
  RESID_GATE_ANM_OFF,           // ゲート解除アニメ
};

// OBJインデックス
enum{
  OBJ_BUTTON_1,                 // ボタン
  OBJ_BUTTON_2,                 // ボタン
  OBJ_BUTTON_3,                 // ボタン
  OBJ_BUTTON_4,                 // ボタン
  OBJ_GATE_1,                   // ゲート
  OBJ_GATE_2,                   // ゲート
  OBJ_GATE_3,                   // ゲート
  OBJ_GATE_4,                   // ゲート

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_off_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_button01_on_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_HIDE_SHIP_ENGINE, NARC_hide_ship_engine_as_gate01_off_nsbma, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_button[] =
{
  { RESID_BUTTON_ANM_OFF, 0 },
  { RESID_BUTTON_ANM_ON, 0 },
};
#define ANM_BUTTON_NUM  ( NELEMS(anm_button) )

static const GFL_G3D_UTIL_ANM anm_gate[] =
{
  { RESID_GATE_ANM_ITA, 0 },
  { RESID_GATE_ANM_OFF, 0 },
};
#define ANM_GATE_NUM  ( NELEMS(anm_gate) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BUTTON_1
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_2
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_3
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_BUTTON_4
  {
    RESID_BUTTON_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BUTTON_MDL,       // テクスチャリソースID
    anm_button,             // アニメテーブル(複数指定のため)
    ANM_BUTTON_NUM,         // アニメリソース数
  },
  // OBJ_GATE_1
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_2
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_3
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
  },
  // OBJ_GATE_4
  {
    RESID_GATE_MDL,         // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_GATE_MDL,         // テクスチャリソースID
    anm_gate,               // アニメテーブル(複数指定のため)
    ANM_GATE_NUM,           // アニメリソース数
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
static void EXPOBJ_ObjSet( HIDE_SHIP_TMP *p_wk );
static void EXPOBJ_ObjDell( HIDE_SHIP_TMP *p_wk );
static void EXPOBJ_ObjMain( HIDE_SHIP_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// BUTTONアニメ
static void ANM_BUTTON_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 id );
static void ANM_BUTTON_ON_Play( HIDE_SHIP_TMP *p_wk, u16 id );
static void ANM_BUTTON_SET_Off( HIDE_SHIP_TMP *p_wk, u16 id );
static void ANM_BUTTON_SET_On( HIDE_SHIP_TMP *p_wk, u16 id );

// GATE
static void ANM_GATE_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 id );
static void ANM_GATE_SET_Off( HIDE_SHIP_TMP *p_wk, u16 id );
static void ANM_GATE_SET_On( HIDE_SHIP_TMP *p_wk, u16 id );


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
void HIDE_SHIP_BOW_Setup( FIELDMAP_WORK *p_fieldmap )
{

  // 汎用ワーク確保
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(HIDE_SHIP_TMP) );

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
void HIDE_SHIP_BOW_End( FIELDMAP_WORK *p_fieldmap )
{
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void HIDE_SHIP_BOW_Move( FIELDMAP_WORK *p_fieldmap )
{
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタンの状態設定
 *
 *	@param	GAMESYS_WORK *gsys    ゲームシステムポインタ
 *	@param	id    ボタンID
 *	@param	flag  状態  FALSE:踏んでいない  TRUE:踏んだ
 */
//-----------------------------------------------------------------------------
void HIDE_SHIP_BOW_ButtonSet( GAMESYS_WORK *gsys, int id, BOOL flag )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  if( flag == FALSE )
  {
    ANM_GATE_SET_On( p_wk, id );
    ANM_BUTTON_SET_Off( p_wk, id );
  }
  else
  {
    ANM_GATE_SET_Off( p_wk, id );
    ANM_BUTTON_SET_On( p_wk, id );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン押下
 *
 *	@param	GAMESYS_WORK *gsys    ゲームシステムポインタ
 *	@param	id    ボタンID
 */
//-----------------------------------------------------------------------------
void HIDE_SHIP_BOW_ButtonOff( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  ANM_BUTTON_ON_Play( p_wk, id );

  // SE 再生
  PMSND_PlaySE( HIDE_SHIP_SE_BTN );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲート解除アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *	@param	id      ゲートID  
 */
//-----------------------------------------------------------------------------
void HIDE_SHIP_BOW_GateOff( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HIDE_SHIP_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, HIDE_SHIP_TMP_ASSIGN_ID );

  ANM_GATE_OFF_Play( p_wk, id );

  // SE 再生
  PMSND_PlaySE( HIDE_SHIP_SE_GATE_OFF );
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
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( HIDE_SHIP_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, HIDE_SHIP_UNIT_IDX);

  //=====================================
  // ボタン
  {
    int i;
    VecFx32 pos[ BUTTON_NUM ] = {
      { BUTTON_1_X, 0, BUTTON_1_Z },
      { BUTTON_2_X, 0, BUTTON_2_Z },
      { BUTTON_3_X, 0, BUTTON_3_Z },
      { BUTTON_4_X, 0, BUTTON_4_Z },
    };

    for( i=0; i<BUTTON_NUM; i++ )
    {
      int obj_idx = OBJ_BUTTON_1 + i;
      
      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_BUTTON_NUM, pos[i] );

      // ホワイトバージョンならボタンはないので非表示
      if( GET_VERSION() == VERSION_W2 )
      {
        FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
      }
    }
  }

  //=====================================
  // ゲート
  {
    int i;
    VecFx32 pos[ GATE_NUM ] = {
      { GATE_1_X, 0, GATE_1_Z },
      { GATE_2_X, 0, GATE_2_Z },
      { GATE_3_X, 0, GATE_3_Z },
      { GATE_4_X, 0, GATE_4_Z },
    };

    for( i=0; i<GATE_NUM; i++ )
    {
      int obj_idx = OBJ_GATE_1 + i;
      
      // 汎用設定
      EXPOBJ_ObjSetOne( p_wk->exp_obj, obj_idx, ANM_GATE_NUM, pos[i] );
      // ループアニメは再生しておく
      ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( HIDE_SHIP_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	HIDE_SHIP_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( HIDE_SHIP_TMP *p_wk )
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
 *	@param  VecFx32 pos 座標
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, VecFx32 pos )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, pos.x, pos.y, pos.z );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, TRUE );
  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, FALSE );

  // アニメ再生
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, i, FALSE );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  オフアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 idx   ボタンインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 obj_idx = OBJ_BUTTON_1 + id;

  GF_ASSERT( id < BUTTON_NUM );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  オンアニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 idx   ボタンインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_ON_Play( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 obj_idx = OBJ_BUTTON_1 + id;

  GF_ASSERT( id < BUTTON_NUM );

  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンをオフ状態にする
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 idx   ボタンインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_SET_Off( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 anm_idx = 0;
  u16 obj_idx = OBJ_BUTTON_1 + id;
  fx32 last_frame;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 最終フレーム取得
  last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

  // ラストフレームにする
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, last_frame );

  // 他のアニメはSTOP
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンをオン状態にする
 *
 *	@param	HIDE_SHIP_TMP *p_wk   ギミックワーク
 *	@param  u16 idx   ボタンインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_BUTTON_SET_On( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 anm_idx = 1;
  u16 obj_idx = OBJ_BUTTON_1 + id;
  fx32 last_frame;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 最終フレーム取得
  last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

  // ラストフレームにする
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, anm_idx, last_frame );

  // 他のアニメはSTOP
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
}

//=============================================================================
/**
 *          GATE
 *          アニメインデックス
 *          0:常時アニメ
 *          1:解除アニメ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ゲート解除アニメ再生
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	id    ゲートID
 */
//-----------------------------------------------------------------------------
static void ANM_GATE_OFF_Play( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 obj_idx = OBJ_GATE_1 + id;

  GF_ASSERT( id < GATE_NUM );

  // 常時アニメを維持しつつ再生する
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲートをOFF状態にする
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	id    ゲートID
 */
//-----------------------------------------------------------------------------
static void ANM_GATE_SET_Off( HIDE_SHIP_TMP *p_wk, u16 id )
{
  fx32 last_frame;
  u16 obj_idx = OBJ_GATE_1 + id;
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  GF_ASSERT( id < GATE_NUM );

  exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, 1 );

  FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, 1, TRUE );
  // 最終フレーム取得
  last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

  // ラストフレームにする
  FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, HIDE_SHIP_UNIT_IDX, obj_idx, 1, last_frame );
  // 常時はSTOP
  ANM_Stop( p_wk->exp_obj, obj_idx, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲートをON状態にする
 *
 *	@param	HIDE_SHIP_TMP *p_wk ギミックワーク
 *	@param	id    ゲートID
 */
//-----------------------------------------------------------------------------
static void ANM_GATE_SET_On( HIDE_SHIP_TMP *p_wk, u16 id )
{
  u16 obj_idx = OBJ_GATE_1 + id;

  GF_ASSERT( id < GATE_NUM );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, TRUE );
  ANM_Stop( p_wk->exp_obj, obj_idx, 1 );
}


