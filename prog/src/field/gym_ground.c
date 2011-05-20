//======================================================================
/**
 * @file    gym_ground.c
 * @brief   地面ジム
 * @author	Katsuhiko=Ichiraku
 * @date    2011.05.10
 */
//======================================================================
// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_task_player_trans.h" // for FIELD_TASK_TransPlayer

// zone
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C05

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gym_ground.naix"
#include "../../../resource/fldmapdata/gimmick/gym_ground/gym_ground_local_def.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_ground_sv.h"
#include "gym_ground_se_def.h"
#include "gym_ground.h"


//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_GROUND_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_GROUND_UNIT_IDX (0)
#define GYM_GROUND_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define LIFT_MOVE_NUM      (4)   // 昇降リフト数
#define LIFT_1_X      (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト1 X座標
#define LIFT_1_Z      (29 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト1 Z座標
#define LIFT_2_X      (46 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト2 X座標
#define LIFT_2_Z      (29 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト2 Z座標
#define LIFT_3_X      (16 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト3 X座標
#define LIFT_3_Z      (62 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト3 Z座標
#define LIFT_4_X      (46 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト4 X座標
#define LIFT_4_Z      (62 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // リフト4 Z座標

#define LIFT_YACON_X  (46 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE )    // ヤーコンリフト
#define LIFT_YACON_Z  (18 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // ヤーコンリフト
#define LIFT_ENT_X    (11 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // エントランスリフト
#define LIFT_ENT_Z    (82 * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE  )    // エントランスリフト

#define LIFT_NUM      (36)      // リフト数     

#define BELT_ANM_SPD  (0x1000)  // ベルトコンベアのアニメ速度

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
// ジム内部中の一時ワーク
//=====================================
typedef struct GYM_GROUND_TMP_tag
{
  HEAPID          heapID;       // ヒープID

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ
  FIELD_PLAYER        *player;  // 自機

  u8              lift_id;      // 現在リフトのID 左上から連番
  BOOL            lift_arrival[ LIFT_NUM ]; // リフト到達フラグ

} GYM_GROUND_TMP;

//-------------------------------------
///	ベルトコンベアイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機

  u8              belt_id;      // ベルトコンベア情報ID

} BELT_EVENT_WORK;

// 3Dリソース
#include "gym_ground.cdat"

//=============================================================================
// ベルトコンベア情報
//=============================================================================
// ベルトコンベア種類
typedef enum {
  BELT_TYPE_UD,     // ベルトコンベア(縦)
  BELT_TYPE_LR,     // ベルトコンベア(横)
  BELT_TYPE_SP_UD,  // ベルトコンベア(縦)
  BELT_TYPE_SP_LR,  // ベルトコンベア(横)
} BELT_TYPE;

// ベルトコンベアの長さ
// ジャンプ２回分で２グリッド少なくなる
#define BELT_UD_MOVE    (7)
#define BELT_LR_MOVE    (4)
#define BELT_SP_UD_MOVE (18)
#define BELT_SP_LR_MOVE (14)

// ベルトコンベア情報構造体
typedef struct {
  BELT_TYPE   type;   // 種類
  int         x;      // Xグリッド座標
  int         z;      // Zグリッド座標
  u16         dir;    // 向き
  u8          lift_id;// 移動後のリフトのID
} BELT_INFO;

#define BELT_ID_MAX     (OBJ_BELT_NUM*2)       // ベルトコンベア起動位置の数
// ベルトコンベア1つにつき2箇所起動位置がある
static const BELT_INFO BeltInfo[BELT_ID_MAX] = {
//  TYPE          X  Z  dir
//  座標が(X,0,Z)だと,dir方向に移動する
  { BELT_TYPE_UD, 6, 8, DIR_DOWN, 6 },// 1
  { BELT_TYPE_UD, 6,17, DIR_UP,   0 },// 1
  { BELT_TYPE_UD,16, 8, DIR_DOWN, 7 },// 2
  { BELT_TYPE_UD,16,17, DIR_UP,   1 },// 2
  { BELT_TYPE_UD,26, 8, DIR_DOWN, 8 },// 3
  { BELT_TYPE_UD,26,17, DIR_UP,   2 },// 3
  { BELT_TYPE_UD,36, 8, DIR_DOWN, 9 },// 4
  { BELT_TYPE_UD,36,17, DIR_UP,   3 },// 4
  { BELT_TYPE_UD, 6,19, DIR_DOWN,12 },// 5
  { BELT_TYPE_UD, 6,28, DIR_UP,   6 },// 5
  { BELT_TYPE_UD,16,19, DIR_DOWN,13 },// 6
  { BELT_TYPE_UD,16,28, DIR_UP,   7 },// 6
  { BELT_TYPE_UD,26,19, DIR_DOWN,14 },// 7
  { BELT_TYPE_UD,26,28, DIR_UP,   8 },// 7
  { BELT_TYPE_UD,56,19, DIR_DOWN,17 },// 8
  { BELT_TYPE_UD,56,28, DIR_UP,  11 },// 8
  { BELT_TYPE_UD, 6,30, DIR_DOWN,18 },// 9
  { BELT_TYPE_UD, 6,39, DIR_UP,  12 },// 9
  { BELT_TYPE_UD,26,30, DIR_DOWN,20 },// 10
  { BELT_TYPE_UD,26,39, DIR_UP,  14 },// 10
  { BELT_TYPE_UD,56,30, DIR_DOWN,23 },// 11
  { BELT_TYPE_UD,56,39, DIR_UP,  17 },// 11
  { BELT_TYPE_UD,16,41, DIR_DOWN,25 },// 12
  { BELT_TYPE_UD,16,50, DIR_UP,  19 },// 12
  { BELT_TYPE_UD,26,41, DIR_DOWN,26 },// 13
  { BELT_TYPE_UD,26,50, DIR_UP,  20 },// 13
  { BELT_TYPE_UD,56,41, DIR_DOWN,29 },// 14
  { BELT_TYPE_UD,56,50, DIR_UP,  23 },// 14
  { BELT_TYPE_UD, 6,52, DIR_DOWN,30 },// 15
  { BELT_TYPE_UD, 6,61, DIR_UP,  24 },// 15
  { BELT_TYPE_UD,16,52, DIR_DOWN,31 },// 16
  { BELT_TYPE_UD,16,61, DIR_UP,  25 },// 16
  { BELT_TYPE_UD,26,52, DIR_DOWN,32 },// 17
  { BELT_TYPE_UD,26,61, DIR_UP,  26 },// 17
  { BELT_TYPE_UD,36,52, DIR_DOWN,33 },// 18
  { BELT_TYPE_UD,36,61, DIR_UP,  27 },// 18
  { BELT_TYPE_UD,46,52, DIR_DOWN,34 },// 19
  { BELT_TYPE_UD,46,61, DIR_UP,  28 },// 19
  { BELT_TYPE_UD,56,52, DIR_DOWN,35 },// 20
  { BELT_TYPE_UD,56,61, DIR_UP,  29 },// 20

  { BELT_TYPE_LR, 8, 7, DIR_RIGHT, 1 },// 1
  { BELT_TYPE_LR,14, 7, DIR_LEFT,  0 },// 1
  { BELT_TYPE_LR,38, 7, DIR_RIGHT, 4 },// 2
  { BELT_TYPE_LR,44, 7, DIR_LEFT,  3 },// 2
  { BELT_TYPE_LR,48, 7, DIR_RIGHT, 5 },// 3
  { BELT_TYPE_LR,54, 7, DIR_LEFT,  4 },// 3
  { BELT_TYPE_LR, 8,18, DIR_RIGHT, 7 },// 4
  { BELT_TYPE_LR,14,18, DIR_LEFT,  6 },// 4
  { BELT_TYPE_LR,18,18, DIR_RIGHT, 8 },// 5
  { BELT_TYPE_LR,24,18, DIR_LEFT,  7 },// 5
  { BELT_TYPE_LR,28,18, DIR_RIGHT, 9 },// 6
  { BELT_TYPE_LR,34,18, DIR_LEFT,  8 },// 6
  { BELT_TYPE_LR, 8,29, DIR_RIGHT,13 },// 7
  { BELT_TYPE_LR,14,29, DIR_LEFT, 12 },// 7
  { BELT_TYPE_LR,18,29, DIR_RIGHT,14 },// 8
  { BELT_TYPE_LR,24,29, DIR_LEFT, 13 },// 8
  { BELT_TYPE_LR,28,29, DIR_RIGHT,15 },// 9
  { BELT_TYPE_LR,34,29, DIR_LEFT, 14 },// 9
  { BELT_TYPE_LR,38,29, DIR_RIGHT,16 },// 10
  { BELT_TYPE_LR,44,29, DIR_LEFT, 15 },// 10
  { BELT_TYPE_LR,48,29, DIR_RIGHT,17 },// 11
  { BELT_TYPE_LR,54,29, DIR_LEFT, 16 },// 11
  { BELT_TYPE_LR,38,40, DIR_RIGHT,22 },// 12
  { BELT_TYPE_LR,44,40, DIR_LEFT, 21 },// 12
  { BELT_TYPE_LR, 8,51, DIR_RIGHT,25 },// 13
  { BELT_TYPE_LR,14,51, DIR_LEFT, 24 },// 13
  { BELT_TYPE_LR,48,51, DIR_RIGHT,29 },// 14
  { BELT_TYPE_LR,54,51, DIR_LEFT, 28 },// 14
  { BELT_TYPE_LR,38,62, DIR_RIGHT,34 },// 15
  { BELT_TYPE_LR,44,62, DIR_LEFT, 33 },// 15
  { BELT_TYPE_LR,48,62, DIR_RIGHT,35 },// 16
  { BELT_TYPE_LR,54,62, DIR_LEFT, 34 },// 16

  { BELT_TYPE_SP_UD,36,19, DIR_DOWN,21 }, // 1
  { BELT_TYPE_SP_UD,36,39, DIR_UP,   9 }, // 1
  { BELT_TYPE_SP_UD,46,19, DIR_DOWN,22 }, // 2
  { BELT_TYPE_SP_UD,46,39, DIR_UP,  10 }, // 2

  { BELT_TYPE_SP_LR,18, 7, DIR_RIGHT, 3 }, // 1
  { BELT_TYPE_SP_LR,34, 7, DIR_LEFT,  1 }, // 1
  { BELT_TYPE_SP_LR,28,51, DIR_RIGHT,28 }, // 2
  { BELT_TYPE_SP_LR,44,51, DIR_LEFT, 26 }, // 2
  { BELT_TYPE_SP_LR,18,62, DIR_RIGHT,33 }, // 3
  { BELT_TYPE_SP_LR,34,62, DIR_LEFT, 31 }, // 3
};
#define BELT_ID_NULL    ( 0xFF )      // IDが見つからない


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_GROUND_TMP *p_wk );
static void EXPOBJ_ObjDell( GYM_GROUND_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_GROUND_TMP *p_wk );

// ベルトコンベアイベント
static GMEVENT* BELT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u16 belt_id );
static GMEVENT_RESULT BeltEvent( GMEVENT* event, int* seq, void* wk );

// 座標からベルトコンベアIDを調べる
static u8 BELT_GetId( const int x, const int z, const u16 dir ); 
static void BELT_AnmStart( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id, const u16 dir );
static void BELT_AnmStop( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id );

// リフト
static void LIFT_Init( GYM_GROUND_TMP *p_wk );
static void LIFT_SetAriivalFlag( GYM_GROUND_TMP *p_wk, const u8 lift_id );
static BOOL LIFT_CheckAllAriival( GYM_GROUND_TMP *p_wk );
static void LIFT_AllArrival( GYM_GROUND_TMP *p_wk );

#ifdef GYM_GROUND_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static fx32  debug_anm_spd = BELT_ANM_SPD;

#endif // GYM_GROUND_DEBUG_MENU

//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_GROUND_TMP));

  p_wk->heapID    = HEAPID_FIELDMAP;
  p_wk->exp_obj   = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->player    = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 【仮】セーブデータから取得する
  // リフト情報初期化
  LIFT_Init( p_wk );


#ifdef GYM_GROUND_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // GYM_GROUND_DEBUG_MENU
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);

#ifdef GYM_GROUND_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_GROUND_DEBUG_MENU

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_GROUND_SV_WORK *gmk_sv_work;
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);

  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  s16 gx, gy, gz;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );


#ifdef PM_DEBUG
  // ベルトコンベアイベント起動
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
      OS_Printf( "HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(p_wk->heapID) );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      GMEVENT *event;
      GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
      event = GYM_GROUND_CheckBeltEvent(gsys);
      GAMESYSTEM_SetEvent(gsys, event);
    }
    // リフトリセット
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
      u8 i;
      LIFT_Init( p_wk );
      for( i=0; i<OBJ_BELT_NUM; i++ )
      {
        u16 obj_idx = i;    // OBJインデックス
        // アニメ再生
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
        // 表示
        FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
      }
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );



}

//----------------------------------------------------------------------------
/**
 *	@brief  ベルトコンベアイベント
 *
 *	@param	*p_gamesys  ゲームシステムへのポインタ
 *	@param	x           自機X座標
 *	@param	z           自機Z座標
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_GROUND_CheckBeltEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  GMEVENT *event;
  s16 gx, gy, gz;
  u16 dir;
  u8  belt_id;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );
  // 自機向き
  dir = FIELD_PLAYER_GetDir( cp_player );

  belt_id = BELT_GetId( gx, gz, dir );
  
  if( belt_id != BELT_ID_NULL )
  {
    event = BELT_CreateEvent( NULL, p_gamesys, belt_id );
    return event;
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
 *	@param	GYM_GROUND_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GYM_GROUND_TMP *p_wk )
{
  u8 i;

  ICHI_Printf("じめんジム OBJの数 %d \n", OBJ_NUM);

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GYM_GROUND_UNIT_IDX);

  //=====================================
  // ベルトコンベアセット
  {
    // 座標セット
    VecFx32 pos[ OBJ_BELT_NUM ] = {
      { BELT_UD_1_X,  0,  BELT_UD_1_Z + GRID_HALF_SIZE },
      { BELT_UD_2_X,  0,  BELT_UD_2_Z + GRID_HALF_SIZE },
      { BELT_UD_3_X,  0,  BELT_UD_3_Z + GRID_HALF_SIZE },
      { BELT_UD_4_X,  0,  BELT_UD_4_Z + GRID_HALF_SIZE },
      { BELT_UD_5_X,  0,  BELT_UD_5_Z + GRID_HALF_SIZE },
      { BELT_UD_6_X,  0,  BELT_UD_6_Z + GRID_HALF_SIZE },
      { BELT_UD_7_X,  0,  BELT_UD_7_Z + GRID_HALF_SIZE },
      { BELT_UD_8_X,  0,  BELT_UD_8_Z + GRID_HALF_SIZE },
      { BELT_UD_9_X,  0,  BELT_UD_9_Z + GRID_HALF_SIZE },
      { BELT_UD_10_X, 0,  BELT_UD_10_Z + GRID_HALF_SIZE },
      { BELT_UD_11_X, 0,  BELT_UD_11_Z + GRID_HALF_SIZE },
      { BELT_UD_12_X, 0,  BELT_UD_12_Z + GRID_HALF_SIZE },
      { BELT_UD_13_X, 0,  BELT_UD_13_Z + GRID_HALF_SIZE },
      { BELT_UD_14_X, 0,  BELT_UD_14_Z + GRID_HALF_SIZE },
      { BELT_UD_15_X, 0,  BELT_UD_15_Z + GRID_HALF_SIZE },
      { BELT_UD_16_X, 0,  BELT_UD_16_Z + GRID_HALF_SIZE },
      { BELT_UD_17_X, 0,  BELT_UD_17_Z + GRID_HALF_SIZE },
      { BELT_UD_18_X, 0,  BELT_UD_18_Z + GRID_HALF_SIZE },
      { BELT_UD_19_X, 0,  BELT_UD_19_Z + GRID_HALF_SIZE },
      { BELT_UD_20_X, 0,  BELT_UD_20_Z + GRID_HALF_SIZE },
      { BELT_LR_1_X,  0,  BELT_LR_1_Z },
      { BELT_LR_2_X,  0,  BELT_LR_2_Z },
      { BELT_LR_3_X,  0,  BELT_LR_3_Z },
      { BELT_LR_4_X,  0,  BELT_LR_4_Z },
      { BELT_LR_5_X,  0,  BELT_LR_5_Z },
      { BELT_LR_6_X,  0,  BELT_LR_6_Z },
      { BELT_LR_7_X,  0,  BELT_LR_7_Z },
      { BELT_LR_8_X,  0,  BELT_LR_8_Z },
      { BELT_LR_9_X,  0,  BELT_LR_9_Z },
      { BELT_LR_10_X, 0,  BELT_LR_10_Z },
      { BELT_LR_11_X, 0,  BELT_LR_11_Z },
      { BELT_LR_12_X, 0,  BELT_LR_12_Z },
      { BELT_LR_13_X, 0,  BELT_LR_13_Z },
      { BELT_LR_14_X, 0,  BELT_LR_14_Z },
      { BELT_LR_15_X, 0,  BELT_LR_15_Z },
      { BELT_LR_16_X, 0,  BELT_LR_16_Z },
      { BELT_SP_UD_1_X,  0,  BELT_SP_UD_1_Z },
      { BELT_SP_UD_2_X,  0,  BELT_SP_UD_2_Z },
      { BELT_SP_LR_1_X,  0,  BELT_SP_LR_1_Z },
      { BELT_SP_LR_2_X,  0,  BELT_SP_LR_2_Z },
      { BELT_SP_LR_3_X,  0,  BELT_SP_LR_3_Z },
    };

    // ベルトコンベアの初期設定
    for( i=0; i<OBJ_BELT_NUM; i++ )
    {

      u16 obj_idx = i;    // OBJインデックス

      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx );

      // 座標
      status->trans.x = pos[i].x + GRID_HALF_SIZE;
      status->trans.y = pos[i].y;
      status->trans.z = pos[i].z + GRID_HALF_SIZE;

      // カリングする
      FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
      // アニメ再生
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
        // ICA
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
#ifdef GYM_GROUND_DEBUG_MENU
        FLD_EXP_OBJ_ChgAddFrame( exp_anm, debug_anm_spd ); // アニメSPD
#else
        FLD_EXP_OBJ_ChgAddFrame( exp_anm, BELT_ANM_SPD ); // アニメSPD
#endif
      }
    }
  }

  //=====================================
  // リフトライトアップ設定
  {
    // 基準の位置(左上)
    const int base_x = (6 * FIELD_CONST_GRID_FX32_SIZE );
    const int base_z = (7 * FIELD_CONST_GRID_FX32_SIZE );
    // 離れている距離
    const int base_w = (10 * FIELD_CONST_GRID_FX32_SIZE );
    const int base_h = (11 * FIELD_CONST_GRID_FX32_SIZE );

    // リフト本体
    for( i=0; i<LIFT_NUM; i++ )
    {
      u16 obj_idx = OBJ_LIFT_LIGHTUP_1 + i;    // OBJインデックス

      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx );

      // 座標
      status->trans.x = base_x + (base_w * (i%6));
      status->trans.y = 0;
      status->trans.z = base_z + (base_h * (i/6));

      // カリングする
      FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
      // 非表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );

      // アニメ再生
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
        // ITA
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
      }
    }
  }

  //=====================================
  // リフト設定
  {
    // 座標セット
    VecFx32 pos[ LIFT_MOVE_NUM ] = {
      { LIFT_1_X,  0,  LIFT_1_Z },
      { LIFT_2_X,  0,  LIFT_2_Z },
      { LIFT_3_X,  0,  LIFT_3_Z },
      { LIFT_4_X,  0,  LIFT_4_Z },
    };

    // リフト本体
    for( i=0; i<LIFT_MOVE_NUM; i++ )
    {
      u16 obj_idx = OBJ_LIFT_1 + i;    // OBJインデックス

      GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx );

      // 座標
      status->trans.x = pos[i].x;
      status->trans.y = pos[i].y;
      status->trans.z = pos[i].z;

      // カリングする
      FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );

      // アニメ再生
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
        // ITA
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
      }
    }
  }

  //=====================================
  // ヤーコンリフト設定
  {
    u16   obj_idx = OBJ_LIFT_YACON;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx );

    status->trans.x = LIFT_YACON_X;
    status->trans.y = 0;
    status->trans.z = LIFT_YACON_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );    // アニメ停止
    }
  }

  //=====================================
  // エントランスリフト設定
  {
    u16   obj_idx = OBJ_LIFT_ENT ;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx );

    status->trans.x = LIFT_ENT_X;
    status->trans.y = 0;
    status->trans.z = LIFT_ENT_Z;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );    // アニメ停止
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	GYM_GROUND_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GYM_GROUND_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GYM_GROUND_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GYM_GROUND_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GYM_GROUND_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ベルトコンベアイベント作成
 *
 *	@param	parent    親イベント
 *	@param	gsys      ゲームシステム
 *	@param	belt_id   ベルトコンベアID
 *	@param	dir       自機方向
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* BELT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, u16 belt_id )
{
  GMEVENT*          event;
  BELT_EVENT_WORK*  work;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER      *player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  event = GMEVENT_Create( gsys, parent, BeltEvent, sizeof(BELT_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;

  work->belt_id     = belt_id;

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ベルトコンベアイベント関数
 *
 *	@param	event   イベントポインタ
 *	@param	seq     シーケンス
 *	@param	wk      ワーク
 *
 *	@return GMEVENT_RESULT 
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT BeltEvent( GMEVENT* event, int* seq, void* wk )
{
  BELT_EVENT_WORK *p_wk = (BELT_EVENT_WORK*)wk;
  GYM_GROUND_TMP  *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_JUMP_IN,// ジャンプIN
    SEQ_INIT,   // イベント初期化
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
    SEQ_JUMP_OUT,// ジャンプOUT
    SEQ_JUMP_OUT_WAIT,// ジャンプOUT
  };

  switch( *seq ){
  // ジャンプ
  case SEQ_JUMP_IN:
    {
      // アニメ終了待ち
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        // ジャンプアニメ
        u16   dir   = FIELD_PLAYER_GetDir( p_wk->player );
        MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
        MMDL_SetAcmd( mmdl, AC_JUMP_U_1G_8F + dir ); 

        *seq = SEQ_INIT;
      }
    }
    break;
  // イベント初期化
  case SEQ_INIT:
    {
      // アニメ終了待ち
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        // 移動タスク登録
        {
          u8  move_distance;    // 移動距離
          FIELD_TASK* move;
          FIELD_TASK_MAN* taskMan;
          VecFx32 move_pos;

          // ベルトコンベアの種類により移動距離が違う
          switch( BeltInfo[ p_wk->belt_id ].type ){
          case BELT_TYPE_UD:    move_distance = BELT_UD_MOVE; break;
          case BELT_TYPE_LR:    move_distance = BELT_LR_MOVE; break;
          case BELT_TYPE_SP_UD: move_distance = BELT_SP_UD_MOVE; break;
          case BELT_TYPE_SP_LR: move_distance = BELT_SP_LR_MOVE; break;
          default:
            GF_ASSERT(0);
          }

          // 移動後位置計算
          {
            FIELD_PLAYER_GetPos( p_wk->player, &move_pos );
            switch( BeltInfo[ p_wk->belt_id ].dir ){
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

          move = FIELD_TASK_TransPlayer( p_wk->p_fieldmap, move_distance*4, &move_pos );
          taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
          FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
        }

        // ベルトコンベアアニメ開始
        BELT_AnmStart( p_gym_wk->exp_obj, p_wk->belt_id, BeltInfo[ p_wk->belt_id ].dir );

        *seq = SEQ_MAIN;
      }
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ){
        *seq = SEQ_EXIT;
      }
    }
    break;

    // イベント終了
  case SEQ_EXIT:
    {
      // ベルトコンベアアニメ停止
      BELT_AnmStop( p_gym_wk->exp_obj, p_wk->belt_id );

      // リフトID変更
      p_gym_wk->lift_id = BeltInfo[ p_wk->belt_id ].lift_id;
      ICHI_Printf("リフトID %d \n", p_gym_wk->lift_id);

      // リフト到着フラグセット
      LIFT_SetAriivalFlag( p_gym_wk, p_gym_wk->lift_id );

      *seq = SEQ_JUMP_OUT;
    }
  // ジャンプOUT
  case SEQ_JUMP_OUT:
    {
      // アニメ終了待ち
      if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
      {
        // ジャンプアニメ
        u16   dir   = FIELD_PLAYER_GetDir( p_wk->player );
        MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
        MMDL_SetAcmd( mmdl, AC_JUMP_U_1G_8F + dir ); 

        *seq = SEQ_JUMP_OUT_WAIT;
      }
    }
    break;
  case SEQ_JUMP_OUT_WAIT:
    // アニメ終了待ち
    if( FIELD_PLAYER_CheckAnimeEnd(p_wk->player) )
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  default:
    GF_ASSERT(0);
  }


  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定座標で、ベルトコンベアが作動するか調べる
 *
 *	@param	x     Xグリッド座標
 *	@param	z     Zグリッド座標
 *	@param	dir   自機の向き
 *
 *	@retval 作動するベルトコンベアがある場合はベルトID
 *  @retval 作動するベルトコンベアがない場合はBELT_ID_NULL
 */
//-----------------------------------------------------------------------------
static u8 BELT_GetId( const int x, const int z, const u16 dir )
{
  u8 i;

  for( i=0; i<BELT_ID_MAX; i++ )
  {
    if( BeltInfo[i].x == x &&
        BeltInfo[i].z == z &&
        BeltInfo[i].dir == dir )
    {
      return i;
    }
  }

  return BELT_ID_NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ベルトコンベア作動
 *
 *	@param	exp_anm   フィールド3D拡張OBJポインタ
 *	@param	belt_id ベルトコンベアID
 *	@param  dir     向き
 */
//-----------------------------------------------------------------------------
static void BELT_AnmStart( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id, const u16 dir )
{
  // OBJ1つにつきbelt_idが2つあるので2で割っている
  u16 obj_idx = belt_id/2;    // OBJインデックス

  EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );

  
  // アニメ再生開始してないなら開始
  if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == TRUE )
  {
    //FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, 0 );

    switch( dir ){
    case DIR_UP:
    case DIR_RIGHT:
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, FX32_ONE );
      break;
    case DIR_DOWN:
    case DIR_LEFT:
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, -FX32_ONE );
      break;
    default:
      GF_ASSERT(0);
    }

    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始
  }

  // SE 再生
  PMSND_PlaySE( GYM_GROUND_SE_BELT_CONVEYOR );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ベルトコンベア停止
 *
 *	@param	p_exp_obj   フィールド3D拡張OBJポインタ
 *	@param	belt_id ベルトコンベアID
 */
//-----------------------------------------------------------------------------
static void BELT_AnmStop( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id )
{
  // OBJ1つにつきbelt_idが2つあるので2で割っている
  u16 obj_idx = belt_id/2;    // OBJインデックス

  EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );

  
  // アニメ再生してるなら停止
  if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == FALSE )
  {
    //FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, 0 );

    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE ); //アニメ停止
  }
  
  // SE停止
  {
    SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_GROUND_SE_BELT_CONVEYOR );
    PMSND_StopSE_byPlayerID( player_ID );
  }
}

//=============================================================================
/**
 *            リフト
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  リフト情報をセーブデータから初期化する
 *
 *	@param	GYM_GROUND_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void LIFT_Init( GYM_GROUND_TMP *p_wk )
{
  p_wk->lift_id = 0;

  GFL_STD_MemClear( p_wk->lift_arrival, (sizeof(BOOL)*LIFT_NUM) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リフト到着フラグセット
 *
 *	@param	GYM_GROUND_TMP *p_wk
 *	@param	u8 lift_id 
 */
//-----------------------------------------------------------------------------
static void LIFT_SetAriivalFlag( GYM_GROUND_TMP *p_wk, const u8 lift_id )
{
  GF_ASSERT_MSG( lift_id < LIFT_NUM, "リフトIDが上限を超えている\n");

  // すでに到達しているなら何もしない
  if( p_wk->lift_arrival[ lift_id ] == TRUE )
  {
    return;
  }

  // フラグON
  p_wk->lift_arrival[ lift_id ] = TRUE;

  // リフトライトアップ
  {
    u16 obj_idx = OBJ_LIFT_LIGHTUP_1 + lift_id;    // OBJインデックス
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
    // 
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

  // 全リフト到着チェック
  if( LIFT_CheckAllAriival( p_wk ) == TRUE )
  {
    // 到着した
    LIFT_AllArrival( p_wk );
  }

  // SE再生
  PMSND_PlaySE( GYM_GROUND_SE_SWITCH );

  ICHI_Printf("リフトID %dの到着フラグセット\n", lift_id);
}


//----------------------------------------------------------------------------
/**
 *	@brief  全てのリフトに到着したかチェック
 *
 *	@param	GYM_GROUND_TMP *p_wk 
 *
 *	@return TRUE 到着した
 */
//-----------------------------------------------------------------------------
static BOOL LIFT_CheckAllAriival( GYM_GROUND_TMP *p_wk )
{
  u8 i;

  for( i=0; i<LIFT_NUM; i++ )
  {
    if( p_wk->lift_arrival[i] == FALSE )
    {
      return FALSE;
    }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全てのリフトに到着した時処理
 *
 *	@param	GYM_GROUND_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void LIFT_AllArrival( GYM_GROUND_TMP *p_wk )
{
  ICHI_Printf("全リフト到着した\n");
}

#ifdef GYM_GROUND_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_GYM_GROUND (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( fx32 *p_param, fx32 min, fx32 max );
// 音量の変更
static void DebugWin_Update_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item );

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
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_GROUND, "じめんジム", heapID );

  // 
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeVolume1,
                              DebugWin_Draw_ChangeVolume1,
                              p_fieldmap,
                              DEBUGMENU_GYM_GROUND,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_GROUND );
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
static BOOL DebugWin_ChangeData( fx32 *p_param, fx32 min, fx32 max )
{
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -0x100;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +0x100;
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
 *	@brief  音量1変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  fx32 value = debug_anm_spd;

  if( DebugWin_ChangeData( &value, FX32_ONE, 0xF000 ) )
  {
    // 更新あり
    debug_anm_spd = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  音量1変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ベルトアニメSPD[%x]", debug_anm_spd );
}


#endif // GYM_GROUND_DEBUG_MENU
