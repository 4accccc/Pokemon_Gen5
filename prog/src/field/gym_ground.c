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
#include "system/wipe.h"    //for WIPE_SYS_

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

// script
#include "../../../resource/fldmapdata/flagwork/work_define.h"

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
#define LIFT_MOVE_SPD ( LIFT_MOVE_SPEED * FX32_ONE )              // 昇降リフトY座標加算値
#define LIFT_Y        ( HEIGHT1 * FIELD_CONST_GRID_FX32_SIZE )    // 昇降リフトが上がった時の高さ
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

#define BELT_ANM_SPD  (0x1200)  // ベルトコンベアのアニメ速度

// 最初のリフトの位置
// 最初のみ、歩いてライトが光る
#define FIRST_LIFT_X  ( 6 )     // Xグリッド座標
#define FIRST_LIFT_Z  ( 63 )    // Zグリッド座標

// WBの出口リフト移動から移植
#define F0_HEIGHT ( HEIGHT0*FIELD_CONST_GRID_FX32_SIZE )
#define EXIT_LIFT_MOVE_SPD (2*FX32_ONE)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

// WBのソースから移植
typedef struct SHAKE_WORK_tag
{
  u8 Seq;
  u8 AddCount;
  u8 ShakeCount;
  u8 ShakeMargin;
  fx32 AddVal;
  FIELD_CAMERA *Camera;
  const VecFx32 *Watch;
  VecFx32 OrgOffset;
}SHAKE_WORK;

//-------------------------------------
// ジム内部中の一時ワーク
//=====================================
typedef struct GYM_GROUND_TMP_tag
{
  HEAPID          heapID;       // ヒープID

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER        *player;  // 自機

  BOOL            lift_arrival[ LIFT_NUM ]; // リフト到達フラグ
  BOOL            first_lift;   // 最初のリフトに到達したか
  BOOL            fog_off;      // フォグをOFFにするフラグ
  BOOL            all_arrival;  // 全リフト到達フラグ

  // 出口リフト用
  const VecFx32 *watch;

} GYM_GROUND_TMP;

//-------------------------------------
///	ベルトコンベアイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機

  u8              belt_id;      // ベルトコンベア情報ID
  u8              move_frame;
  u8              move_frame_num;
} BELT_EVENT_WORK;

//-------------------------------------
///	昇降リフトイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機

  int             lift_id;      // 昇降リフトID 0-3
  u8              way;
  fx32            now_height;   // 現在位置
  fx32            dst_height;   // 移動後位置
  fx32            add_val;      // 加算値
} LIFT_EVENT_WORK;

//-------------------------------------
///	出口リフトイベントワーク
//=====================================
typedef struct {
  // WBの出口リフト処理と同じ
  fx32          now_height;
  fx32          dst_height;
  fx32          add_val;
  SHAKE_WORK    ShakeWork;
  BOOL          exit;

  int           obj_idx;    // 出口リフト or ヤーコンリフト
  MMDL          *yacon_mmdl;// ヤーコンのMMDL
} EXIT_LIFT_EVENT_WORK;


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
  u8          x;      // Xグリッド座標
  u8          z;      // Zグリッド座標
  u8          dir;    // 向き
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
static void BELT_AnmStart( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id, const u8 dir );
static void BELT_AnmStop( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id );

// リフト
static void LIFT_Init( GYM_GROUND_TMP *p_wk );
static void LIFT_SetAriivalFlag( GYM_GROUND_TMP *p_wk, const u8 lift_id );
static BOOL LIFT_CheckAllAriival( GYM_GROUND_TMP *p_wk );
static void LIFT_AllArrival( GYM_GROUND_TMP *p_wk );

// スクリプトコマンド用
// 昇降リフト
static GMEVENT* LIFT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const int lift_id, const int way );
static GMEVENT_RESULT LiftEvent( GMEVENT* event, int* seq, void* wk );
static void LIFT_EVENT_SetPos( LIFT_EVENT_WORK *p_wk );
static BOOL LIFT_EVENT_CheckArrive( LIFT_EVENT_WORK *p_wk );

// 出口リフト
static GMEVENT* EXIT_LIFT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const BOOL exit, const BOOL yacon );
static GMEVENT_RESULT ExitLiftEvent( GMEVENT* event, int* seq, void* wk );
static BOOL EXIT_LIFT_CheckArrive( EXIT_LIFT_EVENT_WORK *p_wk );
static void InitLiftShake(  const fx32 inAddVal,
                            const BOOL inStart,
                            FIELD_CAMERA *camera,
                            SHAKE_WORK *outWork );
static BOOL ShakeLift( SHAKE_WORK *work);
static void SetExitLiftDefaultSt(FLD_EXP_OBJ_CNT_PTR ptr);


//-------------------------------------
///	セーブ情報 ギミック外でも破棄されない
//=====================================
static void SAVE_SetLiftArrival( GIMMICKWORK* p_gmkwk, const BOOL *arrival );
static void SAVE_GetLiftArrival( GIMMICKWORK* p_gmkwk, BOOL *arrival );


#ifdef GYM_GROUND_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static fx32  debug_anm_spd = BELT_ANM_SPD;
static int   debug_height1 = HEIGHT1;
static int   debug_lift_move_speed = LIFT_MOVE_SPEED;

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
  GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
  GYM_GROUND_TMP *p_wk;

  ICHI_Printf( "汎用ワーク確保前 HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP) );

  //汎用ワーク確保
  p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_GROUND_TMP));

  p_wk->heapID    = HEAPID_FIELDMAP;
  p_wk->p_fieldmap= p_fieldmap;
  p_wk->exp_obj   = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->player    = FIELDMAP_GetFieldPlayer( p_fieldmap );
  p_wk->all_arrival = FALSE;  // 全リフト到達していたら、LIFT_InitでTRUEに変わる

  ICHI_Printf( "EXP_OBJ確保前 HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(p_wk->heapID) );
  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // セーブデータ取得
  SAVE_GetLiftArrival( p_gmkwk, p_wk->lift_arrival );

  // リフト情報初期化
  LIFT_Init( p_wk );

  ICHI_Printf( "EXP_OBJ確保後 HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(p_wk->heapID) );

#ifdef GYM_GROUND_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // GYM_GROUND_DEBUG_MENU
  ICHI_Printf( "セットアップ完了後HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(p_wk->heapID) );
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
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA*     p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);

#ifdef GYM_GROUND_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_GROUND_DEBUG_MENU

  // セーブデータ保存
  SAVE_SetLiftArrival( p_gmkwk, p_wk->lift_arrival );

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

  // 最初のリフトのみチェックする
  if( p_wk->first_lift == FALSE )
  {
    if( gx == FIRST_LIFT_X && gz == FIRST_LIFT_Z )
    {
      // 最初リフト到達フラグを立てる
      p_wk->first_lift = TRUE;
      // リフト到着フラグセット
      LIFT_SetAriivalFlag( p_wk, 30 );
    }
  }

  // ギミックのセットアップでフォグをOFFにしても、
  // その後の処理で、ONにされてしまうため、
  // ここでフォグをOFFにする
  if( p_wk->fog_off == TRUE )
  {
    FIELD_FOG_WORK *fog = FIELDMAP_GetFieldFog( p_wk->p_fieldmap );
    if( FIELD_FOG_GetFlag( fog ) == TRUE )
    {
      FIELD_FOG_SetFlag( fog, FALSE );
    }
    else
    {
      p_wk->fog_off = FALSE;
      LIFT_AllArrival( p_wk );
    }
  }

#ifdef PM_DEBUG
  // ベルトコンベアイベント起動
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
      OS_Printf( "HeapFreeSize %x \n",GFL_HEAP_GetHeapFreeSize(p_wk->heapID) );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      // ヤーコンリフト以外到着
      u8 i;
      for( i=0; i<LIFT_NUM; i++ )
      {
        p_wk->lift_arrival[i] = TRUE;
      }
      p_wk->lift_arrival[10] = FALSE;
    }
    // リフトリセット
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
      GFL_STD_MemClear( p_wk->lift_arrival, (sizeof(BOOL)*LIFT_NUM) );
      {
        GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
        GAMEDATA*     p_gdata = GAMESYSTEM_GetGameData( p_gsys );
        GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
        u16 *ex_wk;
        ex_wk = GIMMICKWORK_GetExWork( p_gmkwk, FLD_GIMMICK_GYM_GROUND );
        GFL_STD_MemClear( ex_wk, (sizeof(u8)*5) );
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

  // スクリプト関係
  // WK_SCENE_C05GYM0101_SHORTCUTが1の時は、POSイベントが発生するので、
  // ベルトコンベアのイベントはしない
  {
    GAMEDATA   *gdata = GAMESYSTEM_GetGameData( p_gamesys );
    EVENTWORK* p_evwk = GAMEDATA_GetEventWork( gdata );
    u16 *wk_shortcut = EVENTWORK_GetEventWorkAdrs( p_evwk, WK_SCENE_C05GYM0101_SHORTCUT );

    if( *wk_shortcut == 1 )
    {
      return NULL;
    }
  }

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

//----------------------------------------------------------------------------
/**
 *	@brief  昇降リフトイベント作成
 *
 *	@param	*p_gamesys    ゲームシステムポインタ
 *	@param	lift_id       リフトID(0～3 左上から右下の連番)
 *  @param  way           移動方向 0=上がる  1=下がる
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT *GYM_GROUND_CreateLiftMoveEvent( GAMESYS_WORK *p_gamesys, const int lift_id, const int way )
{
  GMEVENT*    event;

  if( lift_id >= LIFT_MOVE_NUM )
  {
    GF_ASSERT_MSG( 0, "LIFT_ID OVER %d\n", lift_id );
  }
  GF_ASSERT_MSG( way<2,"WAY の値が違います\n");

  event = LIFT_CreateEvent( NULL, p_gamesys, lift_id, way );
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  出口昇降イベント作成
 *
 *	@param	*p_gamesys    ゲームシステムポインタ
 *	@param	exit          ジムから出るとき TRUE
 *	@param  yacon         ヤーコンリフトのとき TRUE
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT *GYM_GROUND_CreateExitLiftMoveEvent( GAMESYS_WORK *p_gamesys, const BOOL exit, const BOOL yacon )
{
  GMEVENT *event;
  event = EXIT_LIFT_CreateEvent( NULL, p_gamesys, exit, yacon );
  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  入り口位置にリフトと自機を移動させる
 *
 *	@param	*p_gamesys    ゲームシステムポインタ
 */
//-----------------------------------------------------------------------------
void GYM_GROUND_SetEnterPos( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  GYM_GROUND_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( p_fieldmap );
  
  //現在ウォッチターゲットを保存
  p_wk->watch = FIELD_CAMERA_GetWatchTarget(camera);
  //カメラのバインドを切る
  FIELD_CAMERA_FreeTarget(camera);
  
  //対象リフト高さ更新
  {
    int obj_idx = OBJ_LIFT_ENT;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_GROUND_UNIT_IDX, obj_idx);
    status->trans.y = F0_HEIGHT;
  }
  //自機の高さを更新
  {
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    pos.y = F0_HEIGHT;
    FIELD_PLAYER_SetPos( fld_player, &pos );
  }
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
        FLD_EXP_OBJ_ChgAddFrame( exp_anm, BELT_ANM_SPD ); // アニメSPD
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
      status->trans.x = base_x + (base_w * (i%6)) + GRID_HALF_SIZE;
      status->trans.y = 0;
      status->trans.z = base_z + (base_h * (i/6)) + GRID_HALF_SIZE;

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
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメ再生設定
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
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );    // アニメ停止
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
  work->move_frame  = 0;
  work->move_frame_num = 0;

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
    SEQ_FADE_OUT, // フェードアウト
    SEQ_FADE_OUT_WAIT, // フェードアウト待ち
    SEQ_FADE_IN_WAIT, // フェードイン待ち
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
          p_wk->move_frame_num = move_distance*4;

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
      // 移動タスク終了待ち
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ){
        *seq = SEQ_EXIT;
      }

      // 立体交差ベルトコンベアは、自力で座標を下げる
      // 立体交差マップをなくした為。
      if( BeltInfo[ p_wk->belt_id ].type == BELT_TYPE_SP_LR ||
          BeltInfo[ p_wk->belt_id ].type == BELT_TYPE_SP_UD )
      {

        p_wk->move_frame++;
        // 長さは3グリッド
        if( p_wk->move_frame == 4*3 )
        {
          // 高さを変更したくないので
          // 高さ取得禁止ステータスビットをONにする
          {
            MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
            MMDL_OnStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
          }
        }
        if( p_wk->move_frame == p_wk->move_frame_num - 4*3 )
        {
          // 高さ取得禁止ステータスビットをOFFに戻す
          {
            MMDL  *mmdl = FIELD_PLAYER_GetMMdl( p_wk->player );
            MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
          }
        }

      }
    }
    break;

    // イベント終了
  case SEQ_EXIT:
    {
      // ベルトコンベアアニメ停止
      BELT_AnmStop( p_gym_wk->exp_obj, p_wk->belt_id );

      // リフトID変更
      {
        const u8 lift_id = BeltInfo[ p_wk->belt_id ].lift_id;
        ICHI_Printf("リフトID %d \n", lift_id);

        // リフト到着フラグセット
        LIFT_SetAriivalFlag( p_gym_wk, lift_id );
      }

      *seq = SEQ_JUMP_OUT;
    }
    break;

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
      // 全リフト到着チェック
      if( p_gym_wk->all_arrival == FALSE &&
          LIFT_CheckAllAriival( p_gym_wk ) == TRUE )
      {
        *seq = SEQ_FADE_OUT;
      }
      else
      {
        return GMEVENT_RES_FINISH;
      }
    }
    break;

  // フェードアウト
  case SEQ_FADE_OUT:
    // ホワイトフェードアウト
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
        WIPE_FADE_WHITE, 10, 4, p_gym_wk->heapID );
    *seq = SEQ_FADE_OUT_WAIT;
    break;
  // フェードアウト待ち
  case SEQ_FADE_OUT_WAIT:
    {
      if( WIPE_SYS_EndCheck() == TRUE )
      {
        // 到着した
        LIFT_AllArrival( p_gym_wk );

        // ホワイトフェードイン
        WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
            WIPE_FADE_WHITE, WIPE_DEF_DIV, WIPE_DEF_SYNC, p_gym_wk->heapID );
        *seq = SEQ_FADE_IN_WAIT;
      }
    }
    break;
  // フェードイン待ち
  case SEQ_FADE_IN_WAIT:
    {
      if( WIPE_SYS_EndCheck() == TRUE )
      {
        return GMEVENT_RES_FINISH;
      }
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
static void BELT_AnmStart( FLD_EXP_OBJ_CNT_PTR exp_obj, const u16 belt_id, const u8 dir )
{
  // OBJ1つにつきbelt_idが2つあるので2で割っている
  u16 obj_idx = belt_id/2;    // OBJインデックス

  EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );

  
  // アニメ再生開始してないなら開始
  if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == TRUE )
  {
    //FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, 0 );
    fx32  anm_spd;

    switch( dir ){
    case DIR_UP:
    case DIR_RIGHT:
#ifdef GYM_GROUND_DEBUG_MENU
      anm_spd = debug_anm_spd;
#else
      anm_spd = BELT_ANM_SPD;
#endif
      break;
    case DIR_DOWN:
    case DIR_LEFT:
#ifdef GYM_GROUND_DEBUG_MENU
      anm_spd = -debug_anm_spd;
#else
      anm_spd = -BELT_ANM_SPD;
#endif
      break;
    default:
      GF_ASSERT(0);
    }

    FLD_EXP_OBJ_ChgAddFrame( exp_anm, anm_spd ); // アニメSPD
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
  u8 i;

  // 到達フラグがTRUEなら設定
  for( i=0; i<LIFT_NUM; i++ )
  {
    if( p_wk->lift_arrival[i] == TRUE )
    {
      // リフトライトアップ
      u16 obj_idx = OBJ_LIFT_LIGHTUP_1 + i;    // OBJインデックス
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, FALSE );
      // 
      // アニメ再生
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0 );
        FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
      }

      // 最初のリフトチェック
      if( i == 30 )
      {
        p_wk->first_lift = TRUE;
      }
    }
  }
  
  // 全リフト到着チェック
  // フォグをOFF
  if( LIFT_CheckAllAriival( p_wk ) == TRUE ){ p_wk->fog_off = TRUE; }
  else                                      { p_wk->fog_off = FALSE; }
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
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

  // SE再生
  PMSND_PlaySE( GYM_GROUND_SE_SWITCH );

  // ※セーブ
  {
    GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
    GAMEDATA*     p_gdata = GAMESYSTEM_GetGameData( p_gsys );
    GIMMICKWORK   *p_gmkwk = GAMEDATA_GetGimmickWork(p_gdata);
    SAVE_SetLiftArrival( p_gmkwk, p_wk->lift_arrival );
  }

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

  // フォグをOFFにする
  {
    FIELD_FOG_WORK *fog = FIELDMAP_GetFieldFog( p_wk->p_fieldmap );
    FIELD_FOG_SetFlag( fog, FALSE );
  }

  // ライトアップも消す
  {
    u8 i;
    for( i=0; i<LIFT_NUM; i++ )
    {
      u16 obj_idx = OBJ_LIFT_LIGHTUP_1 + i;    // OBJインデックス

      // 非表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, TRUE );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, FALSE );
    }
  }

  // フラグを立てる
  p_wk->all_arrival = TRUE;

  ICHI_Printf("全リフト到着した\n");
}


//=============================================================================
/**
 *          スクリプトコマンド用
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  昇降リフトイベント作成
 *
 *	@param	parent    親イベント
 *	@param	gsys      ゲームシステムポインタ
 *	@param	lift_id   リフトID 0-3
 *  @param  way       移動方向 0=上がる  1=下がる
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* LIFT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const int lift_id, const int way )
{
  GMEVENT         *event;
  LIFT_EVENT_WORK *work;
  FIELD_PLAYER    *p_player;
  FIELDMAP_WORK   *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント作成
  event = GMEVENT_Create( gsys, parent, LiftEvent, sizeof(LIFT_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = p_player;
  work->lift_id     = lift_id;
  work->way         = way;

  // 上がる
  if( way == 0 )
  {
    work->now_height  = 0;
#ifdef GYM_GROUND_DEBUG_MENU
    work->dst_height  = ( debug_height1 * FIELD_CONST_GRID_FX32_SIZE );
    work->add_val     = ( debug_lift_move_speed * FX32_ONE );
#else
    work->dst_height  = LIFT_Y;
    work->add_val     = LIFT_MOVE_SPD;
#endif
  }
  // 下がる 
  else
  {
    work->dst_height  = 0;
#ifdef GYM_GROUND_DEBUG_MENU
    work->now_height  = ( debug_height1 * FIELD_CONST_GRID_FX32_SIZE );
    work->add_val     = -( debug_lift_move_speed * FX32_ONE );
#else
    work->now_height  = LIFT_Y;
    work->add_val     = -LIFT_MOVE_SPD;
#endif
  }

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  昇降リフトイベント
 *
 *	@param	* event イベント
 *	@param	seq     シーケンス
 *	@param	wk      ワーク
 *
 *	@return GMEVENT_RESULT 
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT LiftEvent( GMEVENT* event, int* seq, void* wk )
{
  LIFT_EVENT_WORK *p_wk = (LIFT_EVENT_WORK*)wk;
  GYM_GROUND_TMP  *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID );
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  // 処理シーケンス
  enum{
    SEQ_INIT,     // 初期化
    SEQ_INIT_WAIT,// 初期化待ち
    SEQ_MOVE,     // リフト下がる
    SEQ_EXIT,     // イベント終了
  };

  switch( *seq ){
  // 初期化
  case SEQ_INIT:
    {
      // アニメ再生
      if( p_wk->way == 0 )
      {
        int obj_idx = OBJ_LIFT_1 + p_wk->lift_id;
        EXP_OBJ_ANM_CNT_PTR anm;
        anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0);
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, FALSE);
      }

      *seq = SEQ_INIT_WAIT;
    }
    break;

  // 初期化待ち
  case SEQ_INIT_WAIT:
    {
      // SE再生
      PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE);    // リフト移動音
      *seq = SEQ_MOVE;
    }
  /* break throw */
  // リフト上がる
  case SEQ_MOVE: 
    {
      // 高さ加算
      p_wk->now_height += p_wk->add_val;
      // 到達チェック
      if( LIFT_EVENT_CheckArrive(p_wk) )
      {
        //目的高さで上書き
        p_wk->now_height = p_wk->dst_height;

        *seq = SEQ_EXIT;
      }

      // 自機とリフトの位置更新
      LIFT_EVENT_SetPos( p_wk );
    }
    break;

  // イベント終了
  case SEQ_EXIT:
    {
      // アニメ停止
      if( p_wk->way == 1 )
      {
        int obj_idx = OBJ_LIFT_1 + p_wk->lift_id;
        EXP_OBJ_ANM_CNT_PTR anm;
        anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0);
        FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, GYM_GROUND_UNIT_IDX, obj_idx, 0, 0 );
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, TRUE);
      }
      // SE停止
      {
        SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_GROUND_SE_LIFT_MOVE );
        PMSND_StopSE_byPlayerID( player_ID );
      }
    }
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  位置(リフトと自機)の更新
 *
 *	@param	LIFT_EVENT_WORK *p_wk   イベントワーク
 */
//-----------------------------------------------------------------------------
static void LIFT_EVENT_SetPos( LIFT_EVENT_WORK *p_wk )
{
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );

  // 対象リフト高さ更新
  {
    int obj_idx = OBJ_LIFT_1 + p_wk->lift_id;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_GROUND_UNIT_IDX, obj_idx);
    status->trans.y = p_wk->now_height;
  }

  // 対象フォグ高さ更新
  {
    const int obj_tbl[LIFT_MOVE_NUM] = {
      OBJ_LIFT_LIGHTUP_14,
      OBJ_LIFT_LIGHTUP_17,
      OBJ_LIFT_LIGHTUP_32,
      OBJ_LIFT_LIGHTUP_35,
    };
    int obj_idx = obj_tbl[ p_wk->lift_id ];
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_GROUND_UNIT_IDX, obj_idx);
    status->trans.y = p_wk->now_height;
  }

  //自機の高さを更新
  {
    VecFx32 pos;
    FIELD_PLAYER_GetPos( p_wk->player, &pos );
    pos.y = p_wk->now_height;
    FIELD_PLAYER_SetPos( p_wk->player, &pos );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  リフト到達チェック
 *
 *	@param	LIFT_WORK *p_wk 
 *
 *	@return TRUEで到達
 */
//-----------------------------------------------------------------------------
static BOOL LIFT_EVENT_CheckArrive( LIFT_EVENT_WORK *p_wk )
{
  if (p_wk->add_val < 0){
    if ( p_wk->dst_height >= p_wk->now_height ) return TRUE;
  }else{
    if ( p_wk->dst_height <= p_wk->now_height ) return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  出口リフトイベント作成
 *
 *	@param	parent  親イベント
 *	@param	gsys    ゲームシステムポインタ
 *	@param	exit    ジムから出るとき TRUE
 *	@param  yacon   ヤーコンリフトのとき TRUE
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* EXIT_LIFT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const BOOL exit, const BOOL yacon )
{
  GMEVENT * event;
  EXIT_LIFT_EVENT_WORK  *work;
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント作成
  event = GMEVENT_Create( gsys, parent, ExitLiftEvent, sizeof(EXIT_LIFT_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );

  work->exit = exit;
  // 出口リフトかヤーコンリフト
  work->obj_idx = (yacon == TRUE) ? OBJ_LIFT_YACON : OBJ_LIFT_ENT;

  // ヤーコンの場合モデルを取得しておく
  if( yacon == TRUE )
  {
    MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    work->yacon_mmdl = MMDLSYS_SearchGridPos( mmdlsys, 46, 18, TRUE );
    GF_ASSERT_MSG( work->yacon_mmdl, "ヤーコンがみつからない\n");
  }
  else
  {
    work->yacon_mmdl = NULL;
  }

  //対象リフトの状態を取得
  if ( exit )
  {
    //基準位置→移動後位置
    work->now_height = 0;
    work->dst_height = F0_HEIGHT;
  }
  else
  {
    //移動後位置→基準位置
    work->now_height = F0_HEIGHT;
    work->dst_height = 0;
  }

  // アニメ設定
  {
    FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
    EXP_OBJ_ANM_CNT_PTR anm;
    anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, work->obj_idx, 0);
    // アニメ再生
    FLD_EXP_OBJ_ChgAnmStopFlg(anm, FALSE);
  }

  if ( work->dst_height - work->now_height < 0 ) work->add_val = -EXIT_LIFT_MOVE_SPD;
  else work->add_val = EXIT_LIFT_MOVE_SPD;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  出口リフトイベント
 *
 *	@param	* event イベント
 *	@param	seq     シーケンス
 *	@param	wk      ワーク
 *
 *	@return GMEVENT_RESULT 
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ExitLiftEvent( GMEVENT* event, int* seq, void* wk )
{
  EXIT_LIFT_EVENT_WORK *p_wk = (EXIT_LIFT_EVENT_WORK*)wk;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD_EXP_OBJ_CNT_PTR exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  GYM_GROUND_TMP *p_gym_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_GROUND_TMP_ASSIGN_ID);
  FIELD_CAMERA *camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  switch( *seq ){
  case 0:
    //@note Exit が FLASEのときは GYM_GROUND_SetEnterPos関数で処理を行っているはずなので、フックする
    if (p_wk->exit)
    {
      //現在ウォッチターゲットを保存
      p_gym_wk->watch = FIELD_CAMERA_GetWatchTarget(camera);
      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(camera);
    }    
    //リフト移動ＳＥ
    if ( !p_wk->exit ) PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE_IN);
    else PMSND_PlaySE(GYM_GROUND_SE_LIFT_MOVE);
    (*seq)++;
    break;
  case 1:
    {
      //変動後の高さに書き換え
      p_wk->now_height += p_wk->add_val;
      //目的の高さに到達したか？
      if(  EXIT_LIFT_CheckArrive(p_wk) )
      {
        //目的高さで上書き
        p_wk->now_height = p_wk->dst_height;
        //リフト振動セットアップ
        if (!p_wk->exit)
        {
          InitLiftShake( p_wk->add_val, FALSE, camera, &p_wk->ShakeWork);
          //リフト停止ＳＥ
          PMSND_PlaySE(GYM_GROUND_SE_LIFT_STOP);
        }
        //次のシーケンスへ
        (*seq)++;
      }
      //対象リフト高さ更新
      {
        GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exp_obj, GYM_GROUND_UNIT_IDX, p_wk->obj_idx);
        status->trans.y = p_wk->now_height;
      }
      //自機の高さを更新
      {
        VecFx32 pos;
        FIELD_PLAYER *fld_player;
        fld_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
        FIELD_PLAYER_GetPos( fld_player, &pos );
        pos.y = p_wk->now_height;
        FIELD_PLAYER_SetPos( fld_player, &pos );
      }
      // ヤーコンの高さを更新
      if( p_wk->yacon_mmdl != NULL )
      {
        const VecFx32 *mmdl_pos;
        VecFx32 pos;
        int gy;
        //座標更新
        mmdl_pos = MMDL_GetVectorPosAddress( p_wk->yacon_mmdl );
        pos = *mmdl_pos;
        pos.y = p_wk->now_height;
        gy = SIZE_GRID_FX32( pos.y );
        MMDL_SetGridPosY( p_wk->yacon_mmdl, gy );
        MMDL_SetVectorPos( p_wk->yacon_mmdl, &pos );
      }
    }
    break;
  case 2:
    if (p_wk->exit) return GMEVENT_RES_FINISH;
    else
    {
      //振動終わるまで処理をフック
      {
        SHAKE_WORK *shake = &p_wk->ShakeWork;
        if ( !ShakeLift(shake) ) break;
      }

      //カメラを再バインド
      if ( (p_gym_wk->watch != NULL) ){
        FIELD_CAMERA_BindTarget(camera, p_gym_wk->watch);
      }
      //アニメ開始
      {
        EXP_OBJ_ANM_CNT_PTR anm;
        int anm_idx;
        anm_idx = 0;
        anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, p_wk->obj_idx, anm_idx );
        //アニメ停止解除
        FLD_EXP_OBJ_ChgAnmStopFlg(anm, 0);
      }
      //次のシーケンスへ
      (*seq)++;
    }
    break;
  case 3:
    {
      EXP_OBJ_ANM_CNT_PTR anm;
      int anm_idx;
      anm_idx = 0;
      anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, GYM_GROUND_UNIT_IDX, p_wk->obj_idx, anm_idx );
      //アニメ待ち
      if( FLD_EXP_OBJ_ChkAnmEnd(anm) ){
        SetExitLiftDefaultSt(exp_obj);
        return GMEVENT_RES_FINISH;
      }
    }
  }

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  出口リフト到達チェック
 *
 *	@param	*p_wk ワーク 
 *
 *	@return TRUEで到達
 */
//-----------------------------------------------------------------------------
static BOOL EXIT_LIFT_CheckArrive( EXIT_LIFT_EVENT_WORK *p_wk )
{
  if (p_wk->add_val < 0){
    if ( p_wk->dst_height >= p_wk->now_height ) return TRUE;
  }else{
    if ( p_wk->dst_height <= p_wk->now_height ) return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * リフト振動初期化
 * @param   inAddVal       昇降速度
 * @param   inStart     起動時 OR 到着時 TRUEで起動時
 * @param   camera      フィールドカメラポインタ
 * @param   outWork     格納バッファ
 * @return  none
 */
//--------------------------------------------------------------
static void InitLiftShake(  const fx32 inAddVal,
                            const BOOL inStart,
                            FIELD_CAMERA *camera,
                            SHAKE_WORK *outWork )
{
  SHAKE_WORK *work;

  work = outWork;
  work->Seq = 0;
  work->AddCount = 0;
  work->Camera = camera;
  //現在ウォッチターゲットを保存
  work->Watch = FIELD_CAMERA_GetWatchTarget(camera);
  //復帰オフセット値保存
  FIELD_CAMERA_GetTargetOffset( camera, &work->OrgOffset );

  if (inStart)
  {
    work->AddVal = LIFT_START_SHAKE_VAL*FX32_ONE;
    work->ShakeCount = LIFT_START_SHAKE_COUNT;
  }
  else
  {
    work->AddVal = LIFT_SHAKE_VAL*FX32_ONE;
    work->ShakeCount = LIFT_SHAKE_COUNT;
  }
  //進行方向で加算値変更
  if (inAddVal < 0) work->AddVal *= -1;
}

//--------------------------------------------------------------
/**
 * リフト振動関数
 * @param	  work    振動ワークポインタ
 * @return  BOOL    TRUEで振動終了
 */
//--------------------------------------------------------------
static BOOL ShakeLift( SHAKE_WORK *work)
{
  switch(work->Seq){
  case 0:
    //カメラのバインドを切る
    FIELD_CAMERA_FreeTarget(work->Camera);
    
    work->Seq++;
    //none break
  case 1:
    if (work->ShakeMargin % SHAKE_MARGIN == 0)
    {
      //値加算
      {
        VecFx32 offset;
        FIELD_CAMERA_GetTargetOffset( work->Camera, &offset );
        offset.y += work->AddVal;
        FIELD_CAMERA_SetTargetOffset( work->Camera, &offset );
      }
      //値逆転
      work->AddVal *= -1;
      //加算回数インクリメント
      work->AddCount++;
      //終了判定
      if ( work->AddCount >= work->ShakeCount*2 ){
        //カメラを再バインド
        if ( (work->Watch != NULL) ) FIELD_CAMERA_BindTarget(work->Camera, work->Watch);

        //オフセット戻し
        FIELD_CAMERA_SetTargetOffset( work->Camera, &work->OrgOffset );

        return TRUE;
      }
    }
    work->ShakeMargin = (work->ShakeMargin+1)%SHAKE_MARGIN;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * 出口リフトアニメ初期状態セット
 * @param	  FLD_EXP_OBJ_CNT_PTR ptr   拡張ＯＢＪコントローラポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void SetExitLiftDefaultSt(FLD_EXP_OBJ_CNT_PTR ptr)
{
  EXP_OBJ_ANM_CNT_PTR anm;
  int obj_idx;
  int anm_idx;
  obj_idx = OBJ_LIFT_ENT;
  anm_idx = 0;

  FLD_EXP_OBJ_ValidCntAnm(ptr, GYM_GROUND_UNIT_IDX, obj_idx, 0, TRUE);
  //頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx, 0 );
  //停止にしておく
  anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_GROUND_UNIT_IDX, obj_idx, anm_idx);
  FLD_EXP_OBJ_ChgAnmStopFlg(anm, TRUE);
}

//=============================================================================
/**
 *            セーブ情報
 */
//=============================================================================
#ifdef PM_DEBUG
const int BitSize = sizeof(u8) * 8; // 整数型のビットサイズを算出
static void dtob(u8 x) {
  int bit = 1, i;
  char c[BitSize];
 
  for (i = 0; i < BitSize; i++) {
    if (x & bit)
      c[i] = '1';
    else
      c[i] = '0';
    bit <<= 1;
  }
  // 計算結果の表示
  for ( i = BitSize - 1; i >= 0; i-- ) {
      ICHI_Printf("%c",c[i]);
  }
}
#endif
//----------------------------------------------------------------------------
/**
 *	@brief  リフト到達フラグを設定
 *
 *	@param	GIMMICKWORK* p_gmkwk    ギミックワーク
 *	@param	BOOL *arrival           到達フラグ配列
 *
 *	@note   リフト到達フラグ : BOOL flg[36] 
 *	        をu8[5]にビットフラグでセーブする
 */
//-----------------------------------------------------------------------------
static void SAVE_SetLiftArrival( GIMMICKWORK* p_gmkwk, const BOOL *arrival )
{
  u8 i;
  u8 *ex_wk;
  u8 idx = 0;

  ex_wk = GIMMICKWORK_GetExWork( p_gmkwk, FLD_GIMMICK_GYM_GROUND );

  for( i=0; i<LIFT_NUM; i++ )
  {
    if( i % 8 == 0 ) idx++;

    // 到達していたら、ビットをたてる
    if( arrival[i] == TRUE )
    {
      ex_wk[ idx ] |= (1 << (i%8));
    }
  }

#ifdef PM_DEBUG
  ICHI_Printf("セーブ リフト到達フラグ \n 2進数：");
  for( i=0; i<5; i++ ) dtob( ex_wk[i] );
  ICHI_Printf("\n");
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  リフト到達フラグを取得
 *
 *	@param	GIMMICKWORK* p_gmkwk    ギミックワーク
 *	@param	*arrival                到達フラグ配列
 */
//-----------------------------------------------------------------------------
static void SAVE_GetLiftArrival( GIMMICKWORK* p_gmkwk, BOOL *arrival )
{
  u8 i;
  u8 *ex_wk;
  u8 idx = 0;

  ex_wk = GIMMICKWORK_GetExWork( p_gmkwk, FLD_GIMMICK_GYM_GROUND );

#ifdef PM_DEBUG
  ICHI_Printf("セーブ リフト到達フラグ \n 2進数：");
  for( i=0; i<5; i++ ) dtob( ex_wk[i] );
  ICHI_Printf("\n");
#endif

  for( i=0; i<LIFT_NUM; i++ )
  {
    if( i % 8 == 0 ) idx++;

    if( ex_wk[ idx ] & (1 << (i%8)) ){ arrival[i] = TRUE; }
    else                             { arrival[i] = FALSE; }
  }
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
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
static BOOL DebugWin_ChangeDataHex( fx32 *p_param, fx32 min, fx32 max );
// 音量の変更
static void DebugWin_Update_ChangeAnmSpd( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeAnmSpd( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeHeight1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeHeight1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeLiftMoveSpd( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeLiftMoveSpd( void *userWork, DEBUGWIN_ITEM *item );
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
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeAnmSpd,
                              DebugWin_Draw_ChangeAnmSpd,
                              p_fieldmap,
                              DEBUGMENU_GYM_GROUND,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeHeight1,
                              DebugWin_Draw_ChangeHeight1,
                              p_fieldmap,
                              DEBUGMENU_GYM_GROUND,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeLiftMoveSpd,
                              DebugWin_Draw_ChangeLiftMoveSpd,
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
static BOOL DebugWin_ChangeDataHex( fx32 *p_param, fx32 min, fx32 max )
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
 *	@brief  ベルトアニメスピード変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeAnmSpd( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  fx32 value = debug_anm_spd;

  if( DebugWin_ChangeDataHex( &value, FX32_ONE, 0xF000 ) )
  {
    // 更新あり
    debug_anm_spd = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ベルトアニメスピード変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeAnmSpd( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ベルトアニメSPD[%x]", debug_anm_spd );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ベルトアニメスピード変更 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeHeight1( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_height1;

  if( DebugWin_ChangeData( &value, 0, 10 ) )
  {
    // 更新あり
    debug_height1 = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ベルトアニメスピード変更 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeHeight1( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "リフトたかさ[%d]", debug_height1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ベルトアニメスピード変更 更新
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeLiftMoveSpd( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_lift_move_speed;

  if( DebugWin_ChangeData( &value, 0, 30 ) )
  {
    // 更新あり
    debug_lift_move_speed = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ベルトアニメスピード変更 描画
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeLiftMoveSpd( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "リフトそくど[%d]", debug_lift_move_speed );
}

#endif // GYM_GROUND_DEBUG_MENU
