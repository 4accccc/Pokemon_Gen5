//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_gianthole.c
 *	@brief  ジャイアントホールの対決
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
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
#include "arc/fieldmap/gianthole.naix"

// mine
#include "field_gimmick_gianthole.h"
#include "field_gimmick_gianthole_se_def.h"

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

#define BLIZZARD_N_X      ( 248 << FX32_SHIFT )
#define BLIZZARD_N_Y      ( 0 )
#define BLIZZARD_N_Z      ( 312 << FX32_SHIFT )

//-------------------------------------
///	アニメ再生タイプ
//=====================================
typedef enum {
  ANIME_TYPE_NULL,
  ANIME_TYPE_FIRST_HALF,    // 前半
  ANIME_TYPE_SECOND_HALF,   // 後半
  ANIME_TYPE_N_POKE,        // レシゼクに乗ったN
} ANIME_TYPE;


// SE再生フレーム定義
enum{
  FRAME_SE_COLD       = 0,    // キュレム足元から冷気
  FRAME_SE_ICE_ROT_1  = 120,  // 氷の回転音(1段階目)
  FRAME_SE_ICE_ROT_2  = 290,  // 氷の回転音(3段階目)
  FRAME_SE_ICE_STOP   = 370,  // 氷が主人公の周囲に集まり一瞬とまる
  FRAME_SE_ICE_LEAVE  = 480,  // 氷が一瞬はなれる
  FRAME_SE_ICE_ATTACK = 498,  // 氷が主人公に襲い掛かる

  FRAME_SE_ICE_BREAK  = 5,    // 氷が砕け散る

  FRAME_SE_MAX = 500,       // 最長のアニメフレーム数
};

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

  ANIME_TYPE          anm_type;
  u32                 frame;


} GIANTHOLE_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_BLIZZARD_01_MDL,      // こごえるせかい 前半
  RESID_BLIZZARD_01_ANM_ICA,  // こごえるせかい 前半
  RESID_BLIZZARD_01_ANM_ITA,  // こごえるせかい 前半
  RESID_BLIZZARD_02_MDL,      // こごえるせかい 後半
  RESID_BLIZZARD_02_ANM_ICA,  // こごえるせかい 後半
  RESID_BLIZZARD_02_ANM_ITA,  // こごえるせかい 後半
  RESID_BLIZZARD_N_DOWN_MDL,  // ゼクロム(レシラム)に乗ったN下降
  RESID_BLIZZARD_N_DOWN_ANM_ICA,// 
  RESID_BLIZZARD_N_DOWN_ANM_ITP,// バージョンで違う
  RESID_BLIZZARD_N_UP_MDL,    // ゼクロム(レシラム)に乗ったN上昇
  RESID_BLIZZARD_N_UP_ANM_ICA,// 
  RESID_BLIZZARD_N_UP_ANM_ITP,// バージョンで違う
};

// OBJインデックス
enum{
  OBJ_BLIZZARD_01,            // こごえるせかい 前半
  OBJ_BLIZZARD_02,            // こごえるせかい 後半

  OBJ_BLIZZARD_N_DOWN,        // ゼクロム(レシラム)に乗ったN 下降
  OBJ_BLIZZARD_N_UP,          // ゼクロム(レシラム)に乗ったN 上昇

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_02_nsbta, GFL_G3D_UTIL_RESARC },

  // レシゼク N 下降
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_n_down_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_n_down_nsbca, GFL_G3D_UTIL_RESARC },
#if PM_VERSION == VERSION_B2
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_nz_down_nsbtp, GFL_G3D_UTIL_RESARC },
#else
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_nr_down_nsbtp, GFL_G3D_UTIL_RESARC },
#endif

  // レシゼク N 上昇
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_n_up_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_n_up_nsbca, GFL_G3D_UTIL_RESARC },
#if PM_VERSION == VERSION_B2
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_nz_up_nsbtp, GFL_G3D_UTIL_RESARC },
#else
  { ARCID_GIANTHOLE_GIMMICK, NARC_gianthole_blizzard_nr_up_nsbtp, GFL_G3D_UTIL_RESARC },
#endif
};

// ANIME
// こごえるせかい 前半
static const GFL_G3D_UTIL_ANM anm_blizzard_01[] =
{
  { RESID_BLIZZARD_01_ANM_ICA, 0 },
  { RESID_BLIZZARD_01_ANM_ITA, 0 },
};
#define ANM_BLIZZARD_01_NUM  ( NELEMS(anm_blizzard_01) )

// こごえるせかい 後半
static const GFL_G3D_UTIL_ANM anm_blizzard_02[] =
{
  { RESID_BLIZZARD_02_ANM_ICA, 0 },
  { RESID_BLIZZARD_02_ANM_ITA, 0 },
};
#define ANM_BLIZZARD_02_NUM  ( NELEMS(anm_blizzard_02) )

// ゼクロム(レシラム)に乗ったN 下降
static const GFL_G3D_UTIL_ANM anm_blizzard_n_down[] =
{
  { RESID_BLIZZARD_N_DOWN_ANM_ICA, 0 },
  { RESID_BLIZZARD_N_DOWN_ANM_ITP, 0 },
};
#define ANM_BLIZZARD_N_DOWN_NUM  ( NELEMS(anm_blizzard_n_down) )

// ゼクロム(レシラム)に乗ったN 上昇
static const GFL_G3D_UTIL_ANM anm_blizzard_n_up[] =
{
  { RESID_BLIZZARD_N_UP_ANM_ICA, 0 },
  { RESID_BLIZZARD_N_UP_ANM_ITP, 0 },
};
#define ANM_BLIZZARD_N_UP_NUM  ( NELEMS(anm_blizzard_n_up) )


// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_BLIZZARD_01
  {
    RESID_BLIZZARD_01_MDL,  // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_01_MDL,  // テクスチャリソースID
    anm_blizzard_01,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_01_NUM,    // アニメリソース数
  },
  // OBJ_BLIZZARD_02
  {
    RESID_BLIZZARD_02_MDL,  // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_02_MDL,  // テクスチャリソースID
    anm_blizzard_02,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_02_NUM,    // アニメリソース数
  },
  // OBJ_BLIZZARD_N_DOWN
  {
    RESID_BLIZZARD_N_DOWN_MDL,  // モデルリソースID
    0,                          // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_N_DOWN_MDL,  // テクスチャリソースID
    anm_blizzard_n_down,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_N_DOWN_NUM,    // アニメリソース数
  },
  // OBJ_BLIZZARD_N_UP
  {
    RESID_BLIZZARD_N_UP_MDL,  // モデルリソースID
    0,                        // モデルデータID(リソース内部INDEX)
    RESID_BLIZZARD_N_UP_MDL,  // テクスチャリソースID
    anm_blizzard_n_up,        // アニメテーブル(複数指定のため)
    ANM_BLIZZARD_N_UP_NUM,    // アニメリソース数
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
  ANM_BLIZZARD_01_NUM,
  ANM_BLIZZARD_02_NUM,
  ANM_BLIZZARD_N_DOWN_NUM,
  ANM_BLIZZARD_N_UP_NUM,
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GIANTHOLE_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( GIANTHOLE_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( GIANTHOLE_GIMMICK_TMP *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );


// ストーンを浮遊させる処理
// task
static void STONE_FLOAT_TASK_Add( GIANTHOLE_GIMMICK_TMP *p_wk, MMDL* mmdl, int obj_id );
// タスクワーク
typedef struct {
  fx32    now_sync;   // 現在シンク
  MMDL    *mmdl;      // 対象モデル
  FIELDMAP_WORK   *p_fieldmap;    // フィールドマップ
  int     obj_id;
} TASK_WORK;
static FIELD_TASK_RETVAL TaskFunc( void* wk );
#define ONE_CYCLE_SYNC    ( 35 )    // 1周期のシンク数
#define BASE_HEIGHT       ( 3 )     // ゆれ幅
#define OFFSET_HEIGHT     ( 0xD800 ) // 高さオフセット

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
void GIANTHOLE_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(GIANTHOLE_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

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
void GIANTHOLE_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

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
void GIANTHOLE_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      OS_Printf("デバッグ N下降\n");

      GIANTHOLE_GIMMICK_N_POKE_AnmStart( gsys, TRUE );
    }
    
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      OS_Printf("デバッグ N上昇\n");

      GIANTHOLE_GIMMICK_N_POKE_AnmStart( gsys, FALSE );
    }
  }
#endif

  // SE再生
  {
    if( p_wk->anm_type != ANIME_TYPE_NULL )
    {
      // こごえるせかい
      // 前半
      if( p_wk->anm_type == ANIME_TYPE_FIRST_HALF )
      {
        switch( p_wk->frame ){
        case FRAME_SE_COLD:       PMSND_PlaySE( GIANTHOLE_SE_COLD );      break;
        case FRAME_SE_ICE_ROT_1:  PMSND_PlaySE( GIANTHOLE_SE_ICE_ROT_1 ); break;
        case FRAME_SE_ICE_ROT_2:  PMSND_PlaySE( GIANTHOLE_SE_ICE_ROT_2 ); break;
        case FRAME_SE_ICE_STOP:   PMSND_PlaySE( GIANTHOLE_SE_ICE_STOP );  break;
        case FRAME_SE_ICE_LEAVE:  PMSND_PlaySE( GIANTHOLE_SE_ICE_LEAVE ); break;
        case FRAME_SE_ICE_ATTACK: PMSND_PlaySE( GIANTHOLE_SE_ICE_ATTACK );break;
        }
      }
      // こごえるせかい
      // 後半
      else if( p_wk->anm_type == ANIME_TYPE_SECOND_HALF )
      {
        switch( p_wk->frame ){
        case FRAME_SE_ICE_BREAK:  PMSND_PlaySE( GIANTHOLE_SE_ICE_BREAK ); break;
        }
      }
      // Nonゼクレシ
      else if( p_wk->anm_type == ANIME_TYPE_N_POKE )
      {
        // レシラムもゼクロムも羽ばたき音
        switch( p_wk->frame ){
        case 10:
        case 35:
        case 60:
        case 85:
          PMSND_PlaySE( GIANTHOLE_SE_RESIRAMU_FLY_01 );
          break;
        case 13:
        case 38:
        case 63:
        case 88:
          PMSND_PlaySE( GIANTHOLE_SE_RESIRAMU_FLY_02 );
          break;
        }
      }

      p_wk->frame++;

      // アニメが終了とする
      if( p_wk->frame > FRAME_SE_MAX ) { p_wk->anm_type = ANIME_TYPE_NULL; }
    }
  }


  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル表示
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	id  0:前半 1:後半
 */
//-----------------------------------------------------------------------------
void GIANTHOLE_GIMMICK_Disp( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  GF_ASSERT( id<2 );

  if( id == 0 )
  {
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_01, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_02, TRUE );
  }
  else
  {
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_02, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, OBJ_BLIZZARD_01, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	id  0:前半 1:後半
 */
//-----------------------------------------------------------------------------
void GIANTHOLE_GIMMICK_AnmStart( GAMESYS_WORK *gsys, int id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  GF_ASSERT( id<2 );

  // 初期化
  p_wk->anm_type = ANIME_TYPE_NULL;
  p_wk->frame = 0;

  if( id == 0 )
  {
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_01 );
    p_wk->anm_type = ANIME_TYPE_FIRST_HALF;
  }
  else
  {
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_02 );
    p_wk->anm_type = ANIME_TYPE_SECOND_HALF;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ストーン状態のゼクロム(レシラム)を浮遊させる
 *
 *	@param	GAMESYS_WORK *gsys 
 */
//-----------------------------------------------------------------------------
void GIANTHOLE_GIMMICK_StoneFloat( GAMESYS_WORK *gsys, int obj_id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  const MMDLSYS     *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );

  MMDL* mmdl = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );  // カトレア
  GF_ASSERT( mmdl );

  // タスク追加
  STONE_FLOAT_TASK_Add( p_wk, mmdl, obj_id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゼクロム(レシラム)に乗ったNのアニメ開始
 *
 *	@param	GAMESYS_WORK *gsys 
 *	@param  BOOL    is_down  TRUEで下降 FALSEで上昇
 */
//-----------------------------------------------------------------------------
void GIANTHOLE_GIMMICK_N_POKE_AnmStart( GAMESYS_WORK *gsys, BOOL is_down )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GIANTHOLE_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  p_wk->anm_type = ANIME_TYPE_N_POKE;
  p_wk->frame = 0;

  if( is_down == TRUE )
  {
    // 下降アニメ再生
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_N_DOWN );
  }
  else
  {
    // 上昇アニメ再生
    ANM_Play( p_wk->exp_obj, OBJ_BLIZZARD_N_UP );
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
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GIANTHOLE_GIMMICK_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // OBJ_BLIZZARD_01
  {
    u16 obj_idx = OBJ_BLIZZARD_01;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, MAP_HALF_SIZE, 0, MAP_HALF_SIZE );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_01_NUM; i++ )
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

  // OBJ_BLIZZARD_02
  {
    u16 obj_idx = OBJ_BLIZZARD_02;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, MAP_HALF_SIZE, 0, MAP_HALF_SIZE );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_02_NUM; i++ )
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

  // OBJ_BLIZZARD_N_DOWN
  {
    u16 obj_idx = OBJ_BLIZZARD_N_DOWN;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, BLIZZARD_N_X, BLIZZARD_N_Y, BLIZZARD_N_Z );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_N_DOWN_NUM; i++ )
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
  // OBJ_BLIZZARD_N_UP
  {
    u16 obj_idx = OBJ_BLIZZARD_N_UP;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    // 座標
    VEC_Set( &status->trans, BLIZZARD_N_X, BLIZZARD_N_Y, BLIZZARD_N_Z );

    // カリング
    // ※しない設定
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_BLIZZARD_N_UP_NUM; i++ )
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
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GIANTHOLE_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GIANTHOLE_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GIANTHOLE_GIMMICK_TMP *p_wk )
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


//=============================================================================
/**
 *          ストーン浮遊
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ストーン浮遊タスク追加
 */
//-----------------------------------------------------------------------------
static void STONE_FLOAT_TASK_Add( GIANTHOLE_GIMMICK_TMP *p_wk, MMDL* mmdl, int obj_id )
{
  FIELD_TASK      *task;
  FIELD_TASK_MAN  *taskMan;
  TASK_WORK       *work;
  HEAPID heap_id  = FIELDMAP_GetHeapID( p_wk->p_fieldmap );

  taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

  // タスク生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), TaskFunc );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->mmdl     = mmdl;
  work->now_sync = 0;
  work->p_fieldmap = p_wk->p_fieldmap;
  work->obj_id   = obj_id;

  FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
}

// タスク関数
static FIELD_TASK_RETVAL TaskFunc( void* wk )
{
  TASK_WORK* work = (TASK_WORK*)wk;
  fx32  h;  // ゆれ

  // 終了チェック
  {
    const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys( work->p_fieldmap );

    if( MMDLSYS_SearchOBJID( cp_mmdlsys, work->obj_id ) == NULL )
    {
      ICHI_Printf("ストーンタスク終了！\n");
      return FIELD_TASK_RETVAL_FINISH;
    }
  }

  // ゆれ計算
  {
    int   tmp;
    u16   rad;

    work->now_sync++;
    tmp = (work->now_sync * 0x10000) / ONE_CYCLE_SYNC;
    rad = tmp;

    h = BASE_HEIGHT * FX_SinIdx(rad);
  }

  // モデル更新
  {
    VecFx32 offset;
    offset.x = 0;
    offset.y = OFFSET_HEIGHT + h;
    offset.z = 0;
    MMDL_SetVectorDrawOffsetPos( work->mmdl, &offset );
  }

  // 自分からタスクを終了させることはない
  //
  return FIELD_TASK_RETVAL_CONTINUE;
}


