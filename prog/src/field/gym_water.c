//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_water.c
 *	@brief  水ジム
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gym_water.naix"
#include "../../../resource/fldmapdata/gimmick/gym_water/gym_water_local_def.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_water.h"
#include "gym_water_se_def.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_WATER_DEBUG_MENU
#endif //PM_DEBUG


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_WATER_UNIT_IDX (0)
#define GYM_WATER_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define HASU_NUM   (11);   // ハスの個数

#define HASU1_X    (10 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハス1　X座標
#define HASU1_Y    (0  * FIELD_CONST_GRID_FX32_SIZE)       // ハス1　Y座標
#define HASU1_Z    (64 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE)       // ハス1　Z座標

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ジム内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  BOOL                is_hasu_ride;   // ハスの上かフラグ
  u8                  ex_height_idx;  // 拡張高さインデックス         
} GYM_WATER_TMP;

//-------------------------------------
///	ハス移動ワーク
//=====================================
typedef struct {

  // 移動用
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32        startPos;  // 移動開始位置
  VecFx32          endPos;  // 最終位置

  // 上下ゆれ用
  s16              Height;  // ゆれ高さ
  u32                Sync;  // 1周期するのにかかるシンク数
  u32             NowSync;  // 現在のシンク
} HASU_MOVE_WORK;

//-------------------------------------
///	ハスイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機

  HASU_MOVE_WORK  move_wk;      // 移動ワーク

  u8              id;           // ハスID
  u16             obj_idx;      // OBJインデックス

} HASU_EVENT_WORK;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_HASU_MDL = 0,     // ハス
};

// OBJインデックス
enum {
  OBJ_HASU_1,

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GYM_WATER, NARC_gym_water_hasu01_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // ハス1
  {
    RESID_HASU_MDL,       // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_HASU_MDL,       // テクスチャリソースID
    NULL,         // アニメテーブル(複数指定のため)
    0, // アニメリソース数
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
 *          データ
 */
//=============================================================================
//-------------------------------------
///	ハスイベント起動位置パラメータ
//=====================================
typedef struct {
  u8    gx;       // 起動Xグリッド座標
  u8    gz;       // 起動Zグリッド座標
  u8    move_dir; // 移動方向
  u8    move_distance;  // 移動距離
} HASU_POS_PARAM;

static const HASU_POS_PARAM HasuPosParam[ HASU_POS_NUM ] = {
  { HASU_POS_0_X, HASU_POS_0_Z, HASU_POS_0_DIR, HASU_POS_0_MOVE },
  { HASU_POS_1_X, HASU_POS_1_Z, HASU_POS_1_DIR, HASU_POS_1_MOVE },
  { HASU_POS_2_X, HASU_POS_2_Z, HASU_POS_2_DIR, HASU_POS_2_MOVE },
  { HASU_POS_3_X, HASU_POS_3_Z, HASU_POS_3_DIR, HASU_POS_3_MOVE },
  { HASU_POS_4_X, HASU_POS_4_Z, HASU_POS_4_DIR, HASU_POS_4_MOVE },
  { HASU_POS_5_X, HASU_POS_5_Z, HASU_POS_5_DIR, HASU_POS_5_MOVE },
  { HASU_POS_6_X, HASU_POS_6_Z, HASU_POS_6_DIR, HASU_POS_6_MOVE },
  { HASU_POS_7_X, HASU_POS_7_Z, HASU_POS_7_DIR, HASU_POS_7_MOVE },
  { HASU_POS_8_X, HASU_POS_8_Z, HASU_POS_8_DIR, HASU_POS_8_MOVE },
  { HASU_POS_9_X, HASU_POS_9_Z, HASU_POS_9_DIR, HASU_POS_9_MOVE },
  { HASU_POS_10_X, HASU_POS_10_Z, HASU_POS_10_DIR, HASU_POS_10_MOVE },
  { HASU_POS_11_X, HASU_POS_11_Z, HASU_POS_11_DIR, HASU_POS_11_MOVE },
  { HASU_POS_12_X, HASU_POS_12_Z, HASU_POS_12_DIR, HASU_POS_12_MOVE },
  { HASU_POS_13_X, HASU_POS_13_Z, HASU_POS_13_DIR, HASU_POS_13_MOVE },
  { HASU_POS_14_X, HASU_POS_14_Z, HASU_POS_14_DIR, HASU_POS_14_MOVE },
  { HASU_POS_15_X, HASU_POS_15_Z, HASU_POS_15_DIR, HASU_POS_15_MOVE },
  { HASU_POS_16_X, HASU_POS_16_Z, HASU_POS_16_DIR, HASU_POS_16_MOVE },
  { HASU_POS_17_X, HASU_POS_17_Z, HASU_POS_17_DIR, HASU_POS_17_MOVE },
  { HASU_POS_18_X, HASU_POS_18_Z, HASU_POS_18_DIR, HASU_POS_18_MOVE },
  { HASU_POS_19_X, HASU_POS_19_Z, HASU_POS_19_DIR, HASU_POS_19_MOVE },
  { HASU_POS_20_X, HASU_POS_20_Z, HASU_POS_20_DIR, HASU_POS_20_MOVE },
  { HASU_POS_21_X, HASU_POS_21_Z, HASU_POS_21_DIR, HASU_POS_21_MOVE },
  { HASU_POS_22_X, HASU_POS_22_Z, HASU_POS_22_DIR, HASU_POS_22_MOVE },
  { HASU_POS_23_X, HASU_POS_23_Z, HASU_POS_23_DIR, HASU_POS_23_MOVE },
  { HASU_POS_24_X, HASU_POS_24_Z, HASU_POS_24_DIR, HASU_POS_24_MOVE },
  { HASU_POS_25_X, HASU_POS_25_Z, HASU_POS_25_DIR, HASU_POS_25_MOVE },
  { HASU_POS_26_X, HASU_POS_26_Z, HASU_POS_26_DIR, HASU_POS_26_MOVE },
  { HASU_POS_27_X, HASU_POS_27_Z, HASU_POS_27_DIR, HASU_POS_27_MOVE },
  { HASU_POS_28_X, HASU_POS_28_Z, HASU_POS_28_DIR, HASU_POS_28_MOVE },
  { HASU_POS_29_X, HASU_POS_29_Z, HASU_POS_29_DIR, HASU_POS_29_MOVE },
  { HASU_POS_30_X, HASU_POS_30_Z, HASU_POS_30_DIR, HASU_POS_30_MOVE },
  { HASU_POS_31_X, HASU_POS_31_Z, HASU_POS_31_DIR, HASU_POS_31_MOVE },
  { HASU_POS_32_X, HASU_POS_32_Z, HASU_POS_32_DIR, HASU_POS_32_MOVE },
  { HASU_POS_33_X, HASU_POS_33_Z, HASU_POS_33_DIR, HASU_POS_33_MOVE },
  { HASU_POS_34_X, HASU_POS_34_Z, HASU_POS_34_DIR, HASU_POS_34_MOVE },
  { HASU_POS_35_X, HASU_POS_35_Z, HASU_POS_35_DIR, HASU_POS_35_MOVE },
};


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_WATER_TMP *p_wk );
static void EXPOBJ_ObjDell( GYM_WATER_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_WATER_TMP *p_wk );


// HASU
static BOOL HASU_CheckPos( FLD_EXP_OBJ_CNT_PTR exp_obj, const fx32 x, const fx32 z, u16 *p_obj_idx );
static BOOL HASU_GetCenterPos( u8 id, int *p_gx, int *p_gz );
static int HASU_GetMoveDistance( GYM_WATER_TMP *p_wk, u8 id );
static void HASU_SetExHeight_DirMax4( GYM_WATER_TMP *p_wk );

// HASUイベント
static GMEVENT * HASU_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u8 id, u16 obj_idx );
static GMEVENT_RESULT HasuEvent( GMEVENT* event, int* seq, void* wk );

// HASU移動
static void HASU_EVENT_MoveInit( HASU_EVENT_WORK *p_wk, int frame, VecFx32* pos );
static BOOL HASU_EVENT_MoveMain( HASU_EVENT_WORK *p_wk );


// EX_HEIGHT 拡張高さ
static BOOL EX_HEIGHT_CheckHasuRide( GYM_WATER_TMP *p_wk, const int gx, const int gz );
static void EX_HEIGHT_Reset( GYM_WATER_TMP *p_wk ); 
static void EX_HEIGHT_Set( GYM_WATER_TMP *p_wk, u8 id );
static void EX_HEIGHT_Set2( GYM_WATER_TMP *p_wk, u16 obj_idx );

#ifdef GYM_WATER_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int  debug_frame = HASU_MOVE_SPEED;
static s16  debug_height = HASU_SHAKE_HEIGHT;
static u32  debug_sync = HASU_SHAKE_SYNC;

#endif // GYM_WATER_DEBUG_MENU


//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief     セットアップ関数
 * @param	    p_fieldmap     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_WATER_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_WATER_TMP));
  
  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;

  p_wk->is_hasu_ride  = FALSE;
  p_wk->ex_height_idx = 0;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

#ifdef GYM_WATER_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // GYM_WATER_DEBUG_MENU
}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   p_fieldmap     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_WATER_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );

#ifdef GYM_WATER_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_WATER_DEBUG_MENU

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_WATER_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_WATER_TMP_ASSIGN_ID );
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // ハスの上にいる場合は降りたチェック
  if( p_wk->is_hasu_ride == TRUE )
  {
    s16 gx, gy, gz;
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( cp_player );

    // 自機過去グリッド位置
    // ※過去座標でないと、一瞬拡張高さの消えた状態が出来てしまう
    gx = MMDL_GetOldGridPosX( mmdl);
    gz = MMDL_GetOldGridPosZ( mmdl);

    if( EX_HEIGHT_CheckHasuRide( p_wk, gx, gz ) == FALSE )
    {
      // ハスの上に乗っていない
      p_wk->is_hasu_ride = FALSE;

      // 拡張高さリセット
      EX_HEIGHT_Reset( p_wk );

      ICHI_Printf("ハスの上から降りた\n");
    }
  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベントチェック
 *
 *	@param	*p_gamesys  ゲームシステムへのポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_WATER_CheckHasuEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_WATER_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );

  GMEVENT *event;
  s16 gx, gy, gz;
  u8  i;
  u16 obj_idx = 0;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );

  for( i=0; i<HASU_POS_NUM; i++ )
  {
    // 起動位置チェック
    if( gx == HasuPosParam[i].gx && gz == HasuPosParam[i].gz )
    {
      // DIR方向にハスがあるか調べる
      fx32 x;
      fx32 z;
      int center_gx;
      int center_gz;

      // 起動位置のハスIDの中央グリッド座標を取得
      HASU_GetCenterPos( i, &center_gx, &center_gz );

      x = GRID_TO_FX32(center_gx);
      z = GRID_TO_FX32(center_gz);

      // 半グリッドずれを調整
      x += GRID_HALF_SIZE;
      z += GRID_HALF_SIZE;

      // ハス存在チェック
      if( HASU_CheckPos( p_wk->exp_obj, x, z, &obj_idx ) )
      {
        ICHI_Printf("はす %d イベント起動\n", i );

        event = HASU_CreateEvent( NULL, p_gamesys, i, obj_idx );
        return event;
      }
    }
  }

  return NULL;
}

//=============================================================================
/**
 *      static関数
 */
//=============================================================================
//=============================================================================
/**
 *      EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GYM_WATER_TMP *p_wk )
{
  u8 i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GYM_WATER_UNIT_IDX);

  //=====================================
  // ハスの初期設定
  for( i=OBJ_HASU_1; i<OBJ_NUM; i++ )
  {
    // 座標セット
    VecFx32 pos[ OBJ_NUM ] = {
      { HASU1_X,  HASU1_Y,  HASU1_Z },
    };

    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = pos[i].x;
    status->trans.y = pos[i].y;
    status->trans.z = pos[i].z;

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, FALSE );

#if 0
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0 );
      // ICA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 0, TRUE );
      // ITA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx, 1, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
#endif
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GYM_WATER_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GYM_WATER_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GYM_WATER_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GYM_WATER_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}


//=============================================================================
// ハスイベント
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステム
 *	@param	id      ハスID
 *	@param  obj_idx OBJインデックス
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * HASU_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u8 id, u16 obj_idx )
{
  GMEVENT*          event;
  HASU_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER      *player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  event = GMEVENT_Create( gsys, parent, HasuEvent, sizeof(HASU_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;

  work->id          = id;
  work->obj_idx     = obj_idx;

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハスイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT HasuEvent( GMEVENT* event, int* seq, void* wk )
{
  HASU_EVENT_WORK *p_wk = (HASU_EVENT_WORK*)wk;
  GYM_WATER_TMP  *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_WATER_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_START,
    SEQ_JUMP,
    SEQ_MOVE_INIT,
    SEQ_MOVE,
    SEQ_END,
  };

  switch( *seq ){
  case SEQ_START:
    {
      // 拡張高さリセット
      EX_HEIGHT_Reset( p_gym_wk );

      // 拡張高さセット
      EX_HEIGHT_Set( p_gym_wk, p_wk->id );

      *seq = SEQ_JUMP;
    }
    break;

  case SEQ_JUMP:
    {
      // アニメ終了待ち
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        // ジャンプアニメ
        u16   dir   = FIELD_PLAYER_GetDir( p_wk->player );
        MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
        MMDL_SetAcmd( mmdl, AC_JUMP_U_2G_16F + dir ); 

        *seq = SEQ_MOVE_INIT;
      }
    }
    break;

  case SEQ_MOVE_INIT:
    {
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        int move_distance = HASU_GetMoveDistance( p_gym_wk, p_wk->id );
        VecFx32 move_pos;
        // 実際の移動するのは3グリッド少ない
        move_distance -= 3;
        ICHI_Printf("移動距離 %d\n", move_distance);

        // 移動後位置計算
        {
          FIELD_PLAYER_GetPos( p_wk->player, &move_pos );
          switch( HasuPosParam[ p_wk->id ].move_dir ){
          case DIR_UP:
            move_pos.z -= GRID_TO_FX32(move_distance);
            break;
          case DIR_DOWN:
            move_pos.z += GRID_TO_FX32(move_distance);
            break;
          case DIR_LEFT:
            move_pos.x -= GRID_TO_FX32(move_distance);
            break;
          case DIR_RIGHT:
            move_pos.x += GRID_TO_FX32(move_distance);
            break;
          default:
            GF_ASSERT(0);
          }
        }

#ifdef GYM_WATER_DEBUG_MENU
        HASU_EVENT_MoveInit( p_wk, move_distance*debug_frame, &move_pos );
#else
        HASU_EVENT_MoveInit( p_wk, move_distance*HASU_MOVE_SPEED, &move_pos );
#endif

        *seq = SEQ_MOVE;
      }
    }
    break;

  case SEQ_MOVE:
    {
      // ハス移動終了チェック
      if( HASU_EVENT_MoveMain( p_wk ) == TRUE )
      {
        // 拡張高さリセット
        EX_HEIGHT_Reset( p_gym_wk );

        // 拡張高さ更新(自分のいるハス)
        EX_HEIGHT_Set2( p_gym_wk, p_wk->obj_idx );

        // 周りのハスを調べて、拡張高さ更新
        HASU_SetExHeight_DirMax4( p_gym_wk );

        // ハスの上フラグを立てる
        p_gym_wk->is_hasu_ride = TRUE;

        *seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    {
      return GMEVENT_RES_FINISH;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *        HASU
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  指定位置にハスがあるかチェック
 *
 *	@param	exp_obj 拡張3Dオブジェポインタ
 *	@param	fx32 x  X座標
 *	@param	fx32 z  Z座標
 *	@param  *obj_idx  ハスがあった場合そのOBJインデックス
 *
 *	@return TRUEだと存在する
 */
//-----------------------------------------------------------------------------
static BOOL HASU_CheckPos( FLD_EXP_OBJ_CNT_PTR exp_obj, const fx32 x, const fx32 z, u16 *p_obj_idx )
{
  u8 i;

  for( i=OBJ_HASU_1; i<OBJ_NUM; i++ )
  {
    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

    if( status->trans.x == x && status->trans.z == z )
    {
      *p_obj_idx = obj_idx;

      // 存在する
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハスIDの中央位置を求める
 *
 *	@param	id      ハスID 
 *	@param	*p_gx   グリッドX座標を入れるポインタ
 *	@param	*p_gz   グリッドZ座標を入れるポインタ
 *
 *	@return 取得できたらTRUE
 */
//-----------------------------------------------------------------------------
static BOOL HASU_GetCenterPos( u8 id, int *p_gx, int *p_gz )
{
  int x, z;

  // ハスの中心までの差分
  const int diff = 2;   // 2グリッド

  if( id > HASU_POS_NUM )
  {
    GF_ASSERT_MSG( 0, "ハスIDが上限を超えています\n");
    return FALSE;
  }

  x = HasuPosParam[ id ].gx;
  z = HasuPosParam[ id ].gz;

  switch( HasuPosParam[ id ].move_dir ){
  case DIR_UP:    z -= diff; break;
  case DIR_DOWN:  z += diff; break;
  case DIR_LEFT:  x -= diff; break;
  case DIR_RIGHT: x += diff; break;
  default:
    GF_ASSERT(0);
  }

  *p_gx = x;
  *p_gz = z;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス起動IDからハスの移動距離を求める
 *	        途中にハスがある場合はそのハスまでの距離を求める
 *
 *	@param	p_wk  ギミックワーク
 *	@param	id    ハスID
 *
 *	@return 移動距離(グリッド)
 */
//-----------------------------------------------------------------------------
static int HASU_GetMoveDistance( GYM_WATER_TMP *p_wk, u8 id )
{
  int i;
  u16 obj_idx = 0;
  int start_gx;
  int start_gz;

  HASU_GetCenterPos( id, &start_gx, &start_gz );

  for( i=1; i< HasuPosParam[ id ].move_distance; i++ )
  {
    fx32 x;
    fx32 z;
    int gx = start_gx;
    int gz = start_gz;

    switch( HasuPosParam[ id ].move_dir ){
    case DIR_UP:    gz -= i; break;
    case DIR_DOWN:  gz += i; break;
    case DIR_LEFT:  gx -= i; break;
    case DIR_RIGHT: gx += i; break;
    }

    x = GRID_TO_FX32(gx);
    z = GRID_TO_FX32(gz);

    // 半グリッドずれを調整
    x += GRID_HALF_SIZE;
    z += GRID_HALF_SIZE;

    if( HASU_CheckPos( p_wk->exp_obj, x, z, &obj_idx ) )
    {
      ICHI_Printf("途中にハス %d が見つかった\n", obj_idx );
      return i;
    }
  }

  // 見つからない場合は設定された距離
  return HasuPosParam[ id ].move_distance;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機の位置から四方向を調べてハスがあると拡張高さを設定する
 *
 *	@param	GYM_WATER_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void HASU_SetExHeight_DirMax4( GYM_WATER_TMP *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );

  const int merge = (3 * FIELD_CONST_GRID_FX32_SIZE);
  VecFx32 pos;
  u16 obj_idx = 0;
  int i;

  // 自機位置取得
  FIELD_PLAYER_GetPos( cp_player, &pos );

  // 4方向チェック
  for( i=0; i<DIR_MAX4; i++ )
  {
    fx32 x = pos.x;
    fx32 z = pos.z;

    switch( i ){
    case DIR_UP:    z -= merge; break;
    case DIR_DOWN:  z += merge; break;
    case DIR_LEFT:  x -= merge; break;
    case DIR_RIGHT: x += merge; break;
    default: GF_ASSERT(0);
    }

    // 半グリッドずれを調整
    x += GRID_HALF_SIZE;
    z += GRID_HALF_SIZE;

    if( HASU_CheckPos( p_wk->exp_obj, x, z, &obj_idx ) )
    {
      ICHI_Printf("周りにハス %d が見つかった\n", obj_idx );
      // 拡張高さセット
      EX_HEIGHT_Set2( p_wk, obj_idx );
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス移動初期化
 *
 *	@param	p_wk    イベントワーク
 *	@param	frame   動作フレーム数
 *	@param	pos     移動先座標
 */
//-----------------------------------------------------------------------------
static void HASU_EVENT_MoveInit( HASU_EVENT_WORK *p_wk, int frame, VecFx32* pos )
{
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;

  // 初期化
  p_move_wk->frame = 0;
  p_move_wk->endFrame = frame;
  VEC_Set( &p_move_wk->endPos, pos->x, pos->y, pos->z );
  FIELD_PLAYER_GetPos( p_wk->player, &p_move_wk->startPos );

#ifdef GYM_WATER_DEBUG_MENU
  p_move_wk->Height = debug_height;
  p_move_wk->Sync   = debug_sync;
#else
  p_move_wk->Height = HASU_SHAKE_HEIGHT;
  p_move_wk->Sync   = HASU_SHAKE_SYNC;
#endif
  p_move_wk->NowSync= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハス移動メイン
 *
 *	@param	p_wk    イベントワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL HASU_EVENT_MoveMain( HASU_EVENT_WORK *p_wk )
{
  HASU_MOVE_WORK* p_move_wk = &p_wk->move_wk;
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  VecFx32 out_pos;      // 移動後位置
  fx32    h;            // ハスゆれ値
  BOOL    end = FALSE;

  p_move_wk->frame++;
 
  // 終了チェック
  if( p_move_wk->endFrame <= p_move_wk->frame )
  {
    end = TRUE;
  }

  //=====================================
  // 移動
  {
    VecFx32 v0, v1;
    fx32 w1 = FX_F32_TO_FX32( (float)p_move_wk->frame / (float)p_move_wk->endFrame );
    fx32 w0 = FX32_ONE - w1;

    GFL_CALC3D_VEC_MulScalar( &p_move_wk->startPos, w0, &v0 );
    GFL_CALC3D_VEC_MulScalar( &p_move_wk->endPos,   w1, &v1 );
    VEC_Add( &v0, &v1, &out_pos );
  }

  //=====================================
  // ゆれ
  {
    int tmp;
    u16 rad;
    p_move_wk->NowSync++;
    tmp = (p_move_wk->NowSync * 0x10000) / p_move_wk->Sync;
    rad = tmp;
    h = p_move_wk->Height * FX_SinIdx(rad);

    // ゆれセット
    if( end )
    {
      // オフセット初期化
      VecFx32 offset;
      MMDL    *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
      VEC_Set( &offset, 0,0,0 );
      MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    }
    else
    {
      VecFx32 offset;
      MMDL    *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
      VEC_Set( &offset, 0,0,0 );
      offset.y = h;
      MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    }
  }

  //=====================================
  // 自機更新
  FIELD_PLAYER_SetPos( p_wk->player, &out_pos );   // 移動

  // ハス更新
  {
    int obj_idx = p_wk->obj_idx;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_WATER_UNIT_IDX, obj_idx);
    out_pos.y += h;
    status->trans = out_pos;
  }

  // 終了チェック
  return end;
}

//=============================================================================
/**
 *        EX_HEIGHT 拡張高さ関連
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハス(拡張高さ)の上に乗っているか
 *
 *	@param	p_wk  ギミックワーク 
 *	@param  gx    チェックするグリッドX座標
 *	@param  gz    チェックするグリッドZ座標
 *
 *	@return TRUEで乗っている
 */
//-----------------------------------------------------------------------------
static BOOL EX_HEIGHT_CheckHasuRide( GYM_WATER_TMP *p_wk, const int gx, const int gz )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  BOOL ret;
  int  i;

#if 1
  // 拡張高さヒットチェック
  for( i=0; i<p_wk->ex_height_idx; i++ )
  {
    ret = EXH_HitCheckHeight(	gx, gz, ex_height_list, i );

    if( ret )
    {
      return TRUE;
    }
  }

  return FALSE;
#endif

#if 0
  // ハス起動位置にいるかチェック
  for( i=0; i<HASU_POS_NUM; i++ )
  {
    if( gx == HasuPosParam[i].gx && gz == HasuPosParam[i].gz )
    {
      return FALSE;
    }
  }

  return TRUE;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  設定されている拡張高さをリセット
 *
 *	@param	p_wk  ギミックワーク 
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Reset( GYM_WATER_TMP *p_wk )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  int num = EXH_GetDatNum( ex_height_list );
  int i;

  for( i=0; i<num; i++ )
  {
    EXH_SetHeightValid( i, FALSE, ex_height_list );
  }

  p_wk->ex_height_idx = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定
 *
 *	@param	p_wk  ギミックワーク
 *	@param	id    ハスID
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Set( GYM_WATER_TMP *p_wk, u8 id )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  // ハスの中央グリッド
  int center_gx;
  int center_gz;
  HASU_GetCenterPos( id, &center_gx, &center_gz );

  // 1 縦3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      center_gx,   // 領域開始グリッドX座標
      center_gz-1,   // 領域開始グリッドZ座標
      1,      // 領域グリッド横幅
      3,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);


  // 1 横3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      center_gx-1,   // 領域開始グリッドX座標
      center_gz,   // 領域開始グリッドZ座標
      3,      // 領域グリッド横幅
      1,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);

}

//----------------------------------------------------------------------------
/**
 *	@brief  拡張高さを設定(OBJインデックス指定版)
 *
 *	@param	p_wk    ギミックワーク
 *	@param	obj_idx OBJインデックス
 */
//-----------------------------------------------------------------------------
static void EX_HEIGHT_Set2( GYM_WATER_TMP *p_wk, u16 obj_idx )
{
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( p_wk->p_fieldmap );
  EHL_PTR	ex_height_list = FLDMAPPER_GetExHegihtPtr( mapper );

  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_WATER_UNIT_IDX, obj_idx );

  int gx;
  int gz;

  gx = FX32_TO_GRID( status->trans.x - GRID_HALF_SIZE );
  gz = FX32_TO_GRID( status->trans.z - GRID_HALF_SIZE );

  // 1 縦3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      gx,     // 領域開始グリッドX座標
      gz-1,   // 領域開始グリッドZ座標
      1,      // 領域グリッド横幅
      3,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);


  // 1 横3グリッド高さ
  EXH_SetUpHeightData( p_wk->ex_height_idx++,
      gx-1,   // 領域開始グリッドX座標
      gz,     // 領域開始グリッドZ座標
      3,      // 領域グリッド横幅
      1,      // 領域グリッド縦幅
      0,      // 領域内高さ
      (((MAPATTR_FLAGBIT_SHADOW)<<16)|MATTR_NORMAL), // アトリビュート
      ex_height_list);

}

#ifdef GYM_WATER_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_GYM_WATER (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
// 音量の変更
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yにデバッグメニュー追加
 *
 *	@param	*p_wk       ギミックワーク
 *	@param	*p_fieldmap フィールドマップワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID )
{

  // デバッグメニューグループ追加
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_WATER, "みずジム", heapID );

  // 
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeData,
                              DebugWin_Draw_ChangeData,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeHeight,
                              DebugWin_Draw_ChangeHeight,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeSync,
                              DebugWin_Draw_ChangeSync,
                              p_fieldmap,
                              DEBUGMENU_GYM_WATER,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_WATER );
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ数値入力
 *
 *	@param	*p_param  変更する値のポインタ
 *	@param	min       入力する時の最小値
 *	@param	max       入力する時の最大値
 *
 *	@retval TRUE 入力あり
 *	@retval FALSE 入力なし
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_ChangeData( int *p_param, int min, int max )
{
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }

  (*p_param) += value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );

  if( value == 0 )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハス速度変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_frame;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_frame = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハス速度変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeData( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ハスそくど[%d]", debug_frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゆれ高さ 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_height;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_height = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれ高さ 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ゆれたかさ[%d]", debug_height );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれシンク 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_sync;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_sync = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゆれシンク 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ゆれシンク[%d]", debug_sync );
}

#endif // GYM_WATER_DEBUG_MENU

