//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_c14.c
 *	@brief  セイガイハシティ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.12
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
#include "arc/fieldmap/gym_water.naix"  // 水ジムの水しぶきを流用

// mine
#include "field_gimmick_c14.h"

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

} C14_GIMMICK_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================

// リソースの並び順
enum{
  RESID_SPLASH_MDL,      // 水しぶき
  RESID_SPLASH_ANM_IMA,  //
  RESID_SPLASH_ANM_ITA,  //
};

// OBJインデックス
enum{
  OBJ_SPLASH,     // 水しぶき

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GYM_WATER, NARC_gym_water_le_a_shibu1_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_WATER, NARC_gym_water_le_a_shibu1_nsbma, GFL_G3D_UTIL_RESARC }, // IMA
  { ARCID_GYM_WATER, NARC_gym_water_le_a_shibu1_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_splash[] =
{
  { RESID_SPLASH_ANM_IMA, 0 }, // nsbma
  { RESID_SPLASH_ANM_ITA, 0 }, // nsbta
};
#define ANM_SPLASH_NUM  ( NELEMS(anm_splash) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_INTRO_DEMO_SP
  {
    RESID_SPLASH_MDL, // モデルリソースID
    0,                // モデルデータID(リソース内部INDEX)
    RESID_SPLASH_MDL, // テクスチャリソースID
    anm_splash,       // アニメテーブル(複数指定のため)
    ANM_SPLASH_NUM,   // アニメリソース数
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
static void EXPOBJ_ObjSet( C14_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjDell( C14_GIMMICK_TMP *p_wk );
static void EXPOBJ_ObjMain( C14_GIMMICK_TMP *p_wk );

// アニメ再生
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx );

// シズイのジャンプイベント処理
typedef struct {
  MMDL*     p_mmdl;   // 動作モデル
  s16       count;
  VecFx32   start;
  VecFx32   move;
  VecFx32   end;
} WATER_JUMP_EV_WORK;
static GMEVENT_RESULT EVENTFUNC_WaterJump(GMEVENT * event, int *seq, void*work);

//-------------------------------------
// ジャンプ系 高さYオフセット
//-------------------------------------
static const fx32 DATA_JumpOffsTbl[] =
{
  2*FX32_ONE,
  6*FX32_ONE,
  10*FX32_ONE,
  14*FX32_ONE,
  18*FX32_ONE,
  21*FX32_ONE,
  24*FX32_ONE,
  27*FX32_ONE,
  30*FX32_ONE,
  32*FX32_ONE,
  34*FX32_ONE,
  36*FX32_ONE,
  38*FX32_ONE,
  38*FX32_ONE,
  36*FX32_ONE,
  34*FX32_ONE,
  33*FX32_ONE,
  31*FX32_ONE,
  29*FX32_ONE,
  28*FX32_ONE,
  26*FX32_ONE,
  24*FX32_ONE,
  22*FX32_ONE,
};
#define EV_WATERJUMP_FRAME (NELEMS(DATA_JumpOffsTbl)-1)  // フレーム

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
void C14_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  C14_GIMMICK_TMP *p_wk;

  // 汎用ワーク確保
  GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(C14_GIMMICK_TMP) );

  p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  ICHI_Printf("c14ギミックセットアップ\n");
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void C14_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  C14_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

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
void C14_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  C14_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

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
void C14_GIMMICK_AnmStart( GAMESYS_WORK *gsys, int gx, int gy, int gz )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  C14_GIMMICK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  u16 obj_idx = OBJ_SPLASH;    // OBJインデックス

  // 位置
  {
    GFL_G3D_OBJSTATUS *status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GIMMICK_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = GRID_TO_FX32(gx);
    status->trans.y = GRID_TO_FX32(gy);
    status->trans.z = GRID_TO_FX32(gz);
    // 表示
    FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
  }

  // アニメ再生
  ANM_Play( p_wk->exp_obj, OBJ_SPLASH );

  // SE
  PMSND_PlaySE( SEQ_SE_SW_SHIZUI );

}

//----------------------------------------------------------------------------
/**
 *	@brief  シズイの水から飛び出してくる演出
 *
 *	@param	GAMESYS_WORK *gsys
 *	@param	obj_id    OBJID
 *	@param	gx        着地グリッドX
 *	@param	gz        着地グリッドZ
 *	@param	dir       着地後向き
 */
//-----------------------------------------------------------------------------
GMEVENT* C14_GIMMICK_ObjWaterJump( GAMESYS_WORK *gsys, u16 obj_id, int gx, int gz, u16 dir )
{
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );

  MMDL *mmdl = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );  // 
  VecFx32 before_pos; // 現在場所
  VecFx32 after_pos;

  GMEVENT * event;
  WATER_JUMP_EV_WORK *event_work;

  // 指定されたOBJIDは存在しない
  if( mmdl == NULL ) {
    GF_ASSERT(0);
    return NULL;
  }

  // スタート位置を設定
  {
    before_pos.x = GRID_TO_FX32( 791 ) + GRID_HALF_FX32;
    before_pos.y = GRID_TO_FX32( -5 );
    before_pos.z = GRID_TO_FX32( 151 ) + GRID_HALF_FX32;
    MMDL_SetVectorPos( mmdl, &before_pos  );
  }

  // 動作モデルを着地地点に移動
  {
    fx32    y = 0;

    after_pos.x = GRID_TO_FX32( gx ) + GRID_HALF_FX32;  // グリッドの中心
    after_pos.y = 250 << FX32_SHIFT;
    after_pos.z = GRID_TO_FX32( gz ) + GRID_HALF_FX32;  // グリッドの中心

    MMDL_GetMapPosHeight( mmdl, &after_pos, &y );
    after_pos.y = y;
  }

  // イベント作成
  event = GMEVENT_Create( gsys, NULL, EVENTFUNC_WaterJump, sizeof(WATER_JUMP_EV_WORK) );
  event_work = GMEVENT_GetEventWork( event );
  event_work->p_mmdl = mmdl;
  event_work->count = 0;
  event_work->start = before_pos;
  event_work->end   = after_pos;
  VEC_Subtract( &after_pos, &before_pos, &event_work->move );

  return event;
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
 *	@param	C14_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( C14_GIMMICK_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  // 水しぶき
  {
    u16 obj_idx = OBJ_SPLASH;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    int i;

    // 座標
    VEC_Set( &status->trans, 0, 0, 0 );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );

    // アニメ再生
    for( i=0; i<ANM_SPLASH_NUM; i++ )
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
 *	@param	C14_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( C14_GIMMICK_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	C14_GIMMICK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( C14_GIMMICK_TMP *p_wk )
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

  // 表示
  FLD_EXP_OBJ_SetVanish( exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_SPLASH_NUM; i++ )
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
 *          シズイ水ジャンプイベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  イベント関数
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENTFUNC_WaterJump(GMEVENT * event, int *seq, void*work)
{
  WATER_JUMP_EV_WORK *p_wk = work;
  VecFx32 pos;

  // シーケンス処理
  enum{
    SEQ_ARCH,
    SEQ_FALL,
  };

  // シーケンス別処理
  switch( *seq ){
  case SEQ_ARCH:
    // まずはARCH
    pos = p_wk->start;
    pos.y += DATA_JumpOffsTbl[ p_wk->count ];

    // 平行移動
    pos.x += FX_Div( FX_Mul( p_wk->move.x, p_wk->count<<FX32_SHIFT ), EV_WATERJUMP_FRAME << FX32_SHIFT );
    pos.z += FX_Div( FX_Mul( p_wk->move.z, p_wk->count<<FX32_SHIFT ), EV_WATERJUMP_FRAME << FX32_SHIFT );

    // 座標を更新
    MMDL_SetVectorPos( p_wk->p_mmdl, &pos );

    p_wk->count += 1;
    if( p_wk->count > EV_WATERJUMP_FRAME )
    {
      (*seq)++;
    }
    break;

  case SEQ_FALL:
    {
      // 落ちるだけ
      // 終点のYになるまで落とす。
      MMDL_GetVectorPos( p_wk->p_mmdl, &pos );
      if( (pos.y) > p_wk->end.y ){
        pos.y -= (FX32_ONE);

        MMDL_SetVectorPos( p_wk->p_mmdl, &pos );
      }else{
        pos.y = p_wk->end.y;
        pos.x = p_wk->end.x;
        pos.z = p_wk->end.z;

        MMDL_InitPosition( p_wk->p_mmdl, &pos, MMDL_GetDirDisp( p_wk->p_mmdl ) );
        //PMSND_PlaySE( SEQ_SE_FLD_10 );  // 着地
        // SE再生(着地音)
        PMSND_PlaySE( SEQ_SE_FLD_17 );

        return FIELD_TASK_RETVAL_FINISH;
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

