//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_rezi.c
 *	@brief  レジ３体がいる地底遺跡
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.16
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
#include "arc/fieldmap/rezi.naix"

// mine
#include "field_gimmick_rezi.h"

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

// ドア
#define DOOR_X            (13 * FIELD_CONST_GRID_FX32_SIZE )
#define DOOR_Y            ( 0 )
#define DOOR_Z            ( 2 * FIELD_CONST_GRID_FX32_SIZE )

// 眼
#define EYE_X             (14 * FIELD_CONST_GRID_FX32_SIZE )
#define EYE_Y             ( 0 )
#define EYE_Z             ( 7 * FIELD_CONST_GRID_FX32_SIZE )

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

} REZI_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_DOOR_MDL,       // ドア
  RESID_DOOR_ANM_ICA,   //
  RESID_EYE_01_MDL,     // 眼(赤)
  RESID_EYE_02_MDL,     // 眼(青)
  RESID_EYE_03_MDL,     // 眼(銀)
  RESID_EYE_04_MDL,     // 眼(灰)
};

// OBJインデックス
enum{
  OBJ_DOOR,             // ドア
  OBJ_EYE_1,            // 眼
  OBJ_EYE_2,            // 眼
  OBJ_EYE_3,            // 眼
  OBJ_EYE_4,            // 眼

  OBJ_NUM,
};


// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_door_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_door_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_eye_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_eye_02_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_eye_03_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_REZI_GIMMICK, NARC_rezi_reji_eye_04_nsbmd, GFL_G3D_UTIL_RESARC },
};


// ANIME
static const GFL_G3D_UTIL_ANM anm_door[] =
{
  { RESID_DOOR_ANM_ICA, 0 },
};
#define ANM_DOOR_NUM  ( NELEMS(anm_door) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_DOOR
  {
    RESID_DOOR_MDL,   // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_DOOR_MDL,   // テクスチャリソースID
    anm_door,         // アニメテーブル(複数指定のため)
    ANM_DOOR_NUM,     // アニメリソース数
  },
  // OBJ_EYE_1
  {
    RESID_EYE_01_MDL, // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_EYE_01_MDL, // テクスチャリソースID
    NULL,             // アニメテーブル(複数指定のため)
    0,                // アニメリソース数
  },
  // OBJ_EYE_2
  {
    RESID_EYE_02_MDL, // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_EYE_02_MDL, // テクスチャリソースID
    NULL,             // アニメテーブル(複数指定のため)
    0,                // アニメリソース数
  },
  // OBJ_EYE_3
  {
    RESID_EYE_03_MDL, // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_EYE_03_MDL, // テクスチャリソースID
    NULL,             // アニメテーブル(複数指定のため)
    0,                // アニメリソース数
  },
  // OBJ_EYE_4
  {
    RESID_EYE_04_MDL, // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_EYE_04_MDL, // テクスチャリソースID
    NULL,             // アニメテーブル(複数指定のため)
    0,                // アニメリソース数
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
static void EXPOBJ_ObjSet( REZI_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( REZI_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( REZI_GIMMICK_TMP *p_wk );

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
void REZI_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  REZI_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(REZI_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );
  OS_Printf("レジギミックセットアップ\n");
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void REZI_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  REZI_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

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
void REZI_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  REZI_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetDoorState( gsys, FALSE );
      REZI_GIMMICK_AnmDoorStart( gsys, TRUE );
      OS_Printf("ドア開く\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetDoorState( gsys, TRUE );
      REZI_GIMMICK_AnmDoorStart( gsys, FALSE );
      OS_Printf("ドア閉まる\n");
    }

    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetEye( gsys, 0 );
      OS_Printf("眼 0\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetEye( gsys, 1 );
      OS_Printf("眼 1\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetEye( gsys, 2 );
      OS_Printf("眼 2\n");
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      REZI_GIMMICK_SetEye( gsys, 3 );
      OS_Printf("眼 3\n");
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  眼の色設定
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	u8 id 
 *	        0:赤
 *	        1:青
 *	        2:銀
 *	        3:灰
 */
//-----------------------------------------------------------------------------
void REZI_GIMMICK_SetEye( GAMESYS_WORK *gsys, const u8 id )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  REZI_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // 一旦全て非表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EYE_1, TRUE );
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EYE_2, TRUE );
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EYE_3, TRUE );
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EYE_4, TRUE );


  // 指定したもの表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_EYE_1 + id, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ドアの開閉状態セット
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	is_open   TRUEで開いている状態
 *	                  FALSEで閉じている状態
 */
//-----------------------------------------------------------------------------
void REZI_GIMMICK_SetDoorState( GAMESYS_WORK *gsys, BOOL is_open )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  REZI_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  u16 obj_idx = OBJ_DOOR;
  fx32 last_frame;
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  int i;

  for( i=0; i<ANM_DOOR_NUM; i++ )
  {
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, TRUE );
    // 最終フレーム取得
    last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

    if( is_open == TRUE )
    {
      // ラストフレームにする
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, last_frame );
    }
    else
    {
      // ０フレームにする
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, 0 );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ドアの開閉アニメ開始
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	is_open   TRUEで開くアニメ
 *	                  FALSEで閉じるアニメ
 */
//-----------------------------------------------------------------------------
void REZI_GIMMICK_AnmDoorStart( GAMESYS_WORK *gsys, BOOL is_open )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  REZI_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  u16 obj_idx = OBJ_DOOR;
  int i;

  for( i=0; i<ANM_DOOR_NUM; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i );

    // 有効化
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, i, TRUE );

    if( is_open == TRUE )
    {
      // 順再生
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, FX32_ONE );
    }
    else
    {
      // 逆再生
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, -FX32_ONE );
    }

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );   // アニメ再生設定
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
 *	@param	REZI_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( REZI_GIMMICK_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ_DOOR
  {
    u16 obj_idx = OBJ_DOOR;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, DOOR_X, DOOR_Y, DOOR_Z );

    // カリング
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

    // アニメ再生
    for( i=0; i<ANM_DOOR_NUM; i++ )
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

  // OBJ_EYE
  for( i=OBJ_EYE_1; i<=OBJ_EYE_4; i++ )
  {
    u16 obj_idx = i;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, EYE_X, EYE_Y, EYE_Z );

    // カリング
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // なしのみひょうじ
    if( obj_idx == OBJ_EYE_4 ) {
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    } else {
      // 非表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	REZI_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( REZI_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	REZI_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( REZI_GIMMICK_TMP *p_wk )
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
  int anm_num = ANM_DOOR_NUM;

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

