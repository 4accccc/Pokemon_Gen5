//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_fly.c
 *	@brief  飛行ジム
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.02.24
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
#include "field_task.h"         // for FIELD_TASK_TransMMdl
#include "field_task_player_trans.h" // for FIELD_TASK_TransPlayer
#include "field_task_player_rot.h" // for FIELD_TASK_PlayerRotate
#include "field_task_camera_rot.h" // for FIELD_TASK_CameraRot_Pitch
#include "field_task_camera_zoom.h" // for FIELD_TASK_CameraLinearZoom

// zone
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C06

// event
#include "event_mapchange.h"  // for EVENT_ChangeMapPos
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// script
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c06gym0101_def.h"  //for SCRID_～

// arc
#include "arc/fieldmap/gym_fly.naix"
#include "../../../resource/fldmapdata/gimmick/gym_fly/gym_fly_local_def.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "gym_fly_sv.h"
#include "gym_fly_se_def.h"
#include "gym_fly.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_FLY_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_FLY_UNIT_IDX (0)
#define GYM_FLY_TMP_ASSIGN_ID  (1)

#define SE_PITCH_LOW            (-256)      // 低速のpitch    64*-4 -4音
#define SE_PITCH_MIDDLE         (128)       // 中速のpitch    64*2
#define SE_PITCH_HIGH           (256)       // 高速のpitch    64*4
#define SE_PITCH_SUPER          (768)       // 超高速のpitch  64*12
#define SE_PITCH_ADD            (16)

#define FAN_SPEED_LOW           (0x1000)    // 低速のアニメ速度
#define FAN_SPEED_MIDDLE        (0x2000)    // 中速のアニメ速度
#define FAN_SPEED_HIGH          (0x3000)    // 高速のアニメ速度
#define FAN_SPEED_SUPER         (0x4000)    // 超高速のアニメ速度(ショートカット用)
#define FAN_SPEED_ADD           (0x260)     // アニメ加算値

#define WINDMETER_NUM_MAX       (17)        // 風力計の数
#define WINDMETER_SPEED_LOW     (0x2000)    // 低速のアニメ速度
#define WINDMETER_SPEED_MIDDLE  (0x5000)    // 中速のアニメ速度
#define WINDMETER_SPEED_HIGH    (0xb000)    // 高速のアニメ速度
#define WINDMETER_SPEED_SUPER   (0xf000)    // 超高速のアニメ速度(ショートカット用)
#define WINDMETER_SPEED_ADD     (0x300)     // アニメ加算

#define BOX_NUM_MAX             (2)         // 貨物箱の数

// 吹き飛ばしイベント用
#define STAY_WALK_NUM           (4)         // 何歩足踏みするか
//#define SHAKE_CAM_WIDTH         (3)         // 横揺れ幅
//#define SHAKE_CAM_SYNC          (9)         // 1周期にかけるシンク数
//#define SHAKE_CAM_TIME          (2)         // 揺れ回数
//#define SHAKE_CAM_WAIT          (28)        // 待ちフレーム

// 安全地帯吹き飛ばしイベント
#define SHAKE_MMDL_WIDTH        (1)         // 横揺れ幅
#define SHAKE_MMDL_SYNC         (6)         // 1周期にかけるシンク数
#define SHAKE_MMDL_TIME         (2)         // ゆれ回数
#define SHAKE_WAIT_TIME1        (20)        // 1回目のゆれ開始までの待ち
#define SHAKE_WAIT_TIME2        (13)        // 2回目のゆれ開始までの待ち

// ショートカット吹き飛ばしイベント用
#define SHORTCUT_CAMERA_FRAME   (30)        // カメラ速度
#define SHORTCUT_CAMERA_PITCH   (0x20d8)    // カメラ角度
#define SHORTCUT_CAMERA_LEGTH   (0xD5 << FX32_SHIFT)  // カメラ距離

// map_array.cdatの配列の要素
#define MAP_ARRAY_ROW           (51)        // マップの縦MAX
#define MAP_ARRAY_COL           (23)        // マップの横MAX
// map_array.xlsの数字と同じ
enum{
  MAP_ARRAY_BLOW = 0,     // 吹き飛べる
  MAP_ARRAY_STOP,         // 吹き飛べない
  MAP_ARRAY_TIMECOUNT_START,  // 時間計測開始
  MAP_ARRAY_SHORTCUT,     // 帰り道ショートカット
  MAP_ARRAY_TIMECOUNT_STOP,// 時間計測をしない
};

//-------------------------------------
///	風力計状態
//=====================================
typedef enum {
  WIND_STATE_LOW,     // 低速
  WIND_STATE_MIDDLE,  // 中速
  WIND_STATE_HIGH,    // 高速
  WIND_STATE_SUPER,   // 超高速
}WIND_STATE;

//-------------------------------------
/// 吹き飛ばしのHIT種類	
//=====================================
typedef enum {
  BLOW_HIT_WALL,      // 壁にぶつかる
  BLOW_HIT_NPC,       // NPCにぶつかる

  BLOW_HIT_MAX,
} BLOW_HIT_TYPE;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ファン
//=====================================
typedef struct {
  fx32    anm_speed;      // アニメーション再生の速度
  fx32    anm_speed_max;  // アニメーション再生の目標速度

  FLD_EXP_OBJ_CNT_PTR parent_exp_obj;  // 拡張3Dモデルコントローラ
} FAN_WORK;

//-------------------------------------
///	風力計
//=====================================
typedef struct {
  fx32    anm_speed;      // アニメーション再生の速度
  fx32    anm_speed_max;  // アニメーション再生の目標速度

  FLD_EXP_OBJ_CNT_PTR parent_exp_obj;  // 拡張3Dモデルコントローラ
} WINDMETER_WORK;

//-------------------------------------
///	音管理
//=====================================
typedef struct {
  u8      seq;
  int     pitch;        // 現pitch
  int     pitch_max;    // pitchの目標
  BOOL    se_change;    // 風の音2種切り替え
} SOUND_WORK;


//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  HEAPID  heapID;     // ヒープID

  u8      state;          // 風速状態   WIND_STATE_XXX

  BOOL    time_start;     // 時間計測開始フラグ
  u16     time;           // 時間計測用

  s8      alpha[2];       // α値(0～31)
  BOOL    alpha_flag[2];  // α計算フラグ
  BOOL    alpha_reset;    // 初期化フラグ

  FAN_WORK            fan_wk;
  WINDMETER_WORK      windmeter_wk;
  SOUND_WORK          sound;

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ
  FIELD_PLAYER        *player;  // 自機
	FIELD_SOUND         *p_sound; // サウンド情報
  GYM_FLY_SV_WORK     *p_sv_wk; // セーブワーク

}GYM_FLY_TMP;

//-------------------------------------
///	吹き飛ばしイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機
  MMDL            *mmdl;        // 動作モデル

  BLOW_HIT_TYPE   hit_type;     // 衝突種類   BLOW_HIT_XXX
  u16             time;         // 時間計測用
  u8              stay_walk_cnt;// 足踏みカウント
  BOOL            is_wall;      // 後ろがすぐ壁か？
  u16             save_time;    // 時間保存用(安全地帯吹き飛ばしで使用)
} BLOW_EVENT_WORK;

//-------------------------------------
///	帰り道ショートカットイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;  // フィールドマップ
  FIELD_PLAYER    *player;      // 自機
  MMDL            *mmdl;        // 自機の動作モデル

  u8              stay_walk_cnt;// 足踏みカウント
} SHORTCUT_EVENT_WORK;


// 3Dリソース
#include "gym_fly.cdat"
// マップデータ
#include "../../../resource/fldmapdata/gimmick/gym_fly/map_array.cdat"

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================
// ジム初期化
static void GYM_FLY_Init( GYM_FLY_TMP *p_wk, const u16 c_time );
// 状態
static WIND_STATE WIND_GetState( const u16 c_time ); 
static void WIND_ChangeState( FIELDMAP_WORK *p_fieldmap, const WIND_STATE state );
// 吹き飛ばしイベント
static GMEVENT* WIND_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT BlowEvent( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT BlowEventSafe( GMEVENT* event, int* seq, void* wk );
// 帰り道ショートカットイベント
static GMEVENT* SHORTCUT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT ShortcutEvent( GMEVENT* event, int* seq, void* wk );
// スクリプト用吹き飛ばしイベント
static GMEVENT* SCRIPT_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const u16 obj_id );
static GMEVENT_RESULT ScriptBlowEvent( GMEVENT* event, int* seq, void* wk );

// 吹き飛びの移動距離算出
static const u8 BLOW_EVENT_GetMoveDistance( FIELDMAP_WORK *p_fieldmap, BLOW_HIT_TYPE *p_hit_type, s16 gx, s16 gz );

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_FLY_TMP *p_wk );
static void EXPOBJ_ObjDell( GYM_FLY_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_FLY_TMP *p_wk );

// FAN
static void FAN_Update( FAN_WORK *p_wk );
static BOOL FAN_CheckAnmSpeed( const FAN_WORK *cp_wk );

// WINDMETER
static void WINDMETER_Update( WINDMETER_WORK *p_wk );
static BOOL WINDMETER_CheckAnmSpeed( const WINDMETER_WORK *cp_wk );

// SOUND
static void SOUND_Update( SOUND_WORK *p_wk, const u16 c_time );
static void SOUND_ChangeSE( SOUND_WORK *p_wk );
static void SOUND_ChangePitch( SOUND_WORK *p_wk, const WIND_STATE state );

//-------------------------------------
///	セーブ情報
//=====================================
// GYM_FLY_SV_WORK
static void SAVE_SetTimeFlg( GYM_FLY_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetTimeFlg( const GYM_FLY_SV_WORK *cp_wk );
static void SAVE_SetTime( GYM_FLY_SV_WORK *p_wk, const u16 time );
static u16  SAVE_GetTime( const GYM_FLY_SV_WORK *cp_wk );

//=====================================
// フィールドタスク関係
//=====================================
// モデルの平行移動
typedef struct
{
  u8                  seq;  // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象フィールドマップ
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32        startPos;  // 移動開始位置
  VecFx32          endPos;  // 最終位置
  MMDL              *mmdl;  // 対象モデル

} TASK_WORK;
static FIELD_TASK_RETVAL  TransMMdl( void* wk );

// モデルの振動
typedef struct
{
  u8                  seq;  // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象フィールドマップ
  int               frame;  // 現在のフレーム数
  VecFx32         basePos;  // 移動開始位置
  MMDL              *mmdl;  // 対象モデル

  s16               width;  // ゆれ幅
  u8                 time;  // 振動回数
  u32                sync;  // 
  u32             nowSync;  // 

} MMDL_SHAKE_WORK;
static FIELD_TASK_RETVAL  ShakeMMdl( void* wk );

// カメラの振動
typedef struct {
  FIELDMAP_WORK*  fieldmap;
  FIELD_CAMERA*   camera;

  u8              seq;  // シーケンス
  CAM_SHAKE_PARAM shakeParam;
  const VecFx32   *WatchTarget;

} CAM_SHAKE_WORK;
static FIELD_TASK_RETVAL ShakeCamera( void* wk );


#ifdef GYM_FLY_DEBUG_MENU
//-------------------------------------
/// デバッグメニュー	
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int debug_windmeter_time1 = WINDMETER_MIDDLE_TIME;   // 低速から中速に変わるまでの時間
static int debug_windmeter_time2 = WINDMETER_HIGH_TIME;   // 中速から高速に変わるまでの時間
static u16 debug_blow_event_time_max = BLOW_EVENT_TIME;   // 吹き飛ばしイベントの最低経過時間
static u16 debug_safe_event_time_max = SAFE_EVENT_TIME;   // 安全地帯イベントの最低経過時間

static s16 debug_w = 0;
static s16 debug_h = 2;
static u32 debug_sync = 3;
static u8  debug_time = 4;
#endif // GYM_FLY_DEBUG_MENU

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
void GYM_FLY_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_FLY_TMP));

  p_wk->heapID    = HEAPID_FIELDMAP;
  p_wk->alpha[0]  = 0;
  p_wk->alpha[1]  = 0;
  p_wk->alpha_flag[0] = FALSE;
  p_wk->alpha_flag[1] = FALSE;
  p_wk->alpha_reset   = FALSE;
  p_wk->state     = WIND_STATE_LOW;
  p_wk->exp_obj   = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->player    = FIELDMAP_GetFieldPlayer( p_fieldmap );
  p_wk->p_sound   = GAMEDATA_GetFieldSound( p_gdata );
  p_wk->p_sv_wk   = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_GYM_FLY );

  // ファン
  p_wk->fan_wk.parent_exp_obj = p_wk->exp_obj;

  // 風力計
  p_wk->windmeter_wk.parent_exp_obj = p_wk->exp_obj;

  // サウンド
  p_wk->sound.seq = 0;
  p_wk->sound.se_change = FALSE;
  p_wk->sound.pitch     = SE_PITCH_LOW;
  p_wk->sound.pitch_max = SE_PITCH_LOW;

  // 環境音
  //FSND_PlayEnvSE( p_wk->p_sound, GYM_FLY_SE_FAN );
  PMSND_PlaySE( GYM_FLY_SE_FAN );
  PMSND_PlaySE( GYM_FLY_SE_WIND_NORMAL );

  // セーブから取得
  p_wk->time_start = SAVE_GetTimeFlg( p_wk->p_sv_wk );
  p_wk->time       = SAVE_GetTime( p_wk->p_sv_wk );

  // timeでワーク初期化
  GYM_FLY_Init( p_wk, p_wk->time );

#ifdef GYM_FLY_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, p_wk->heapID );
#endif // GYM_FLY_DEBUG_MENU
}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   p_fieldmap     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_FLY_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );

#ifdef GYM_FLY_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_FLY_DEBUG_MENU

  // セーブに設定
  SAVE_SetTimeFlg( p_wk->p_sv_wk, p_wk->time_start );
  SAVE_SetTime( p_wk->p_sv_wk, p_wk->time );

  // 環境音停止
  //FSND_StopEnvSE( p_wk->p_sound, GYM_FLY_SE_FAN );
  {
    SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_FAN );
    PMSND_StopSE_byPlayerID( player_ID );
    player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_WIND_NORMAL );
    PMSND_StopSE_byPlayerID( player_ID );
  }

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_FLY_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );
  u8 i;

  // 風のエフェクトのみアルファ適用
  for( i=0; i<2; i++ )
  {
    // α値計算
    {
      // 徐々に出す
      if( p_wk->alpha_flag[i] == TRUE )
      {
        p_wk->alpha[i] += 2;
        if( p_wk->alpha[i] > 31 )
        {
          p_wk->alpha[i] = 31;
          p_wk->alpha_flag[i] = FALSE;
        }
      }
      // 徐々に消す
      if( p_wk->alpha_reset == TRUE )
      {
        // まず強風消してから中風を消す
        // i=0だと中風 i=1だと強風
        if( i == 1 )
        {
          // 強風は無条件
          if( p_wk->alpha[i] > 0 ){ p_wk->alpha[i] -= 2; }

          if( p_wk->alpha[i] < 0 )
          {
            // 描画軽くするために、消していた風1を表示
            FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_1, FALSE );  // 表示
          }
        }
        if( i == 0 )
        {
          // 強風が消えてる場合のみ
          if( p_wk->alpha[1] == 0 )
          {
            if( p_wk->alpha[i] > 0 ){ p_wk->alpha[i] -= 4; }
          }
        }

        if( p_wk->alpha[i] < 0 )
        {
          p_wk->alpha[i] = 0;

          // 非表示
          FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_2+i, TRUE );
          {
            EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_2+i, 0 );
            FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE ); //アニメストップ
          }
          // フラグ初期化
          if( i == 0 ){ p_wk->alpha_reset = FALSE; }
        }
      }
    }

    // モデルに適用
    {
      GFL_G3D_OBJ *p_obj = FLD_EXP_OBJ_GetUnitObj( p_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_2 + i );
      NNSG3dResMdl *p_mdl = NNS_G3dRenderObjGetResMdl
        (GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(p_obj)) );
      NNS_G3dMdlSetMdlAlphaAll( p_mdl, p_wk->alpha[i] );
    }
  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );

  // サウンド更新
  SOUND_Update( &p_wk->sound, p_wk->time );
}


//----------------------------------------------------------------------------
/**
 *	@brief  時間計測・監視　イベント起動チェック
 *
 *	@param	GAMESYS_WORK *p_gamesys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_FLY_CheckEvent( GAMESYS_WORK *p_gamesys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  s16 gx, gy, gz;
  GMEVENT *event;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );

  if( p_wk == NULL ){ return NULL; }

  // 指定位置に来ていないなら時間開始しない
  if( p_wk->time_start == FALSE )
  {
    // 配列から自機座標のデータを取得
    if( map_array[gz][gx] == MAP_ARRAY_TIMECOUNT_START )
    {
      p_wk->time_start = TRUE;
    }
    return NULL;
  }
  
  // ジムリーダーの場所は時間計測をしない
  if( map_array[gz][gx] == MAP_ARRAY_TIMECOUNT_STOP )
  {
    return NULL;
  }

  // 強制脱出イベント監視
  if( map_array[gz][gx] == MAP_ARRAY_SHORTCUT )
  {
    event = SHORTCUT_CreateEvent( NULL, p_gamesys );
    return event;
  }

  // 時間経過
  p_wk->time++;
  
  {
    // 時間監視
    WIND_STATE tmp_state = WIND_GetState( p_wk->time );

    // 風力計の状態が変更したら
    if( p_wk->state != tmp_state )
    {
      WIND_ChangeState( p_fieldmap, tmp_state );

      // 高速の時のみ吹き飛ばしイベント作成
      if( p_wk->state == WIND_STATE_HIGH )
      {
        event = WIND_BLOW_CreateEvent( NULL, p_gamesys );
        return event;
      }
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベント
 *	        スクリプトコマンド用
 *
 *	@param	GAMESYS_WORK *p_gamesys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * GYM_FLY_BlowEvent( GAMESYS_WORK *p_gamesys, const u16 obj_id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_fieldmap );
  GMEVENT *event;

  event = SCRIPT_BLOW_CreateEvent( NULL, p_gamesys, obj_id );
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMDLの移動タスクを作成する
 *
 *  @param fieldmap 動作対象フィールドマップ
 *	@param mmdl     モデル
 *  @param frame    動作フレーム数
 *  @param pos      移動先座標
 *
 *	@return 作成したタスク
 *
 *	@note   現在飛行ジムでしか使われていないので飛行ジムのオーバーレイにあります。
 *	        他の場所でも使用する場合があれば、FIELD_MAPのオーバーレイに移した方がいい。
 */
//-----------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_TransMMdl( FIELDMAP_WORK* fieldmap, MMDL *mmdl, int frame, VecFx32* pos )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), TransMMdl );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->frame    = 0;
  work->endFrame = frame;
  VEC_Set( &work->endPos, pos->x, pos->y, pos->z );
  work->mmdl     = mmdl;

  return task;
}
//=============================================================================
//  タスク処理関数
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  モデルの平行移動処理
 */
//-----------------------------------------------------------------------------
static FIELD_TASK_RETVAL TransMMdl( void* wk )
{
  TASK_WORK*      work = (TASK_WORK*)wk;

  switch( work->seq )
  {
  case 0:
    MMDL_GetVectorPos( work->mmdl, &work->startPos );
    work->seq++;
    break;
  case 1:
    // 自機の座標を更新
    work->frame++;
    {
      VecFx32 v0, v1, v2;
      fx32 w1 = FX_F32_TO_FX32( (float)work->frame / (float)work->endFrame );
      fx32 w0 = FX32_ONE - w1;
      GFL_CALC3D_VEC_MulScalar( &work->startPos, w0, &v0 );
      GFL_CALC3D_VEC_MulScalar( &work->endPos,   w1, &v1 );
      VEC_Add( &v0, &v1, &v2 );
      MMDL_SetVectorPos( work->mmdl, &v2 ); // 移動
      MMDL_UpdateCurrentHeight( work->mmdl );   // 接地
    }
    if( work->endFrame <= work->frame )
    {
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMDLの振動タスクを作成する
 *
 *  @param fieldmap 動作対象フィールドマップ
 *	@param mmdl     モデル
 *	@param w        横揺れ幅
 *	@param sync     １周期にかけるシンク数 1以上
 *	@param time     振動回数
 *
 *	@return 作成したタスク
 *
 *	@note   現在飛行ジムでしか使われていないので飛行ジムのオーバーレイにあります。
 *	        他の場所でも使用する場合があれば、FIELD_MAPのオーバーレイに移した方がいい。
 */
//-----------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_ShakeMMdl( FIELDMAP_WORK* fieldmap, MMDL *mmdl, s16 w, u32 sync, u8 time )
{
  FIELD_TASK* task;
  MMDL_SHAKE_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(MMDL_SHAKE_WORK), ShakeMMdl );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->frame    = 0;
  work->mmdl     = mmdl;
  MMDL_GetVectorPos( mmdl, &work->basePos );

  work->width   = w;
  work->sync    = sync;
  work->time    = time;
  work->nowSync = 0;

  return task;
}
//=============================================================================
//  タスク処理関数
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  MMDL振動処理
 */
//-----------------------------------------------------------------------------
static FIELD_TASK_RETVAL ShakeMMdl( void* wk )
{
  MMDL_SHAKE_WORK*  work = (MMDL_SHAKE_WORK*)wk;
  BOOL end = FALSE;

  switch( work->seq ) {
  case 0:
    {
      //ターゲットを動かす
      int tmp;
      u16 rad;
      work->nowSync++;
      tmp = (work->nowSync * 0x10000) / work->sync;
      rad = tmp;

      if (work->nowSync >= work->sync)
      {
        work->time--;
        work->nowSync = 0;
      }
      if (work->time == 0)
      {
        work->nowSync = 0;
        end = TRUE;
      }

      if (end)
      {
        VecFx32 offset;
        VEC_Set( &offset, 0,0,0 );
        MMDL_SetVectorDrawOffsetPos( work->mmdl, &offset );
        //MMDL_SetVectorPos( work->mmdl, &work->basePos );

        work->seq++;
      }
      else
      {
        fx32 w;
        VecFx32 offset;
        w = work->width * FX_SinIdx(rad);
        offset.x = w;
        offset.y = 0;
        offset.z = 0;

        //座標セット
        MMDL_SetVectorDrawOffsetPos( work->mmdl, &offset );
        //MMDL_SetVectorPos( work->mmdl, &offset );
      }
    }
    break;
  case 1:
    return FIELD_TASK_RETVAL_FINISH;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動タスクを作成する
 *
 *	@param	fieldmap    動作対象フィールドマップ
 *	@param  shakeParam  振動パラメータ
 *
 *	@return 作成したタスク
 *
 */
//-----------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_ShakeCamera( FIELDMAP_WORK* fieldmap, const CAM_SHAKE_PARAM* shakeParam )
{
  FIELD_TASK* task;
  CAM_SHAKE_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(CAM_SHAKE_WORK), ShakeCamera );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap = fieldmap;
  work->camera   = FIELDMAP_GetFieldCamera( fieldmap );
  work->seq      = 0;
  work->shakeParam = *shakeParam;

  return task;
}
//=============================================================================
//  タスク処理関数
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ振動処理
 */
//-----------------------------------------------------------------------------
static FIELD_TASK_RETVAL ShakeCamera( void* wk )
{
  CAM_SHAKE_WORK*  work = (CAM_SHAKE_WORK*)wk;
  CAM_SHAKE_PARAM* param = &(work->shakeParam);
  BOOL end = FALSE;

  // 開始フレーム経過まで待つ
  if( param->WaitStartFrame > 0 )
  {
    param->WaitStartFrame--;
    return FIELD_TASK_RETVAL_CONTINUE;
  }

  switch( work->seq )
  {
  case 0:
    //トレース終了
    FIELD_CAMERA_StopTraceRequest(work->camera);
    work->seq++;
    /* break through */
  case 1:
    if ( FIELD_CAMERA_CheckTrace(work->camera) )
    {
      break;
    }
    {
      //復帰パラメータをセットする
      FIELD_CAMERA_SetRecvCamParam(work->camera);

      //カメラのターゲットを取得
      work->WatchTarget = FIELD_CAMERA_GetWatchTarget(work->camera);
      //バインド解除
      FIELD_CAMERA_FreeTarget(work->camera);
      
      //補間移動はカメラ位置不定型のみのサポート
      FIELD_CAMERA_ChangeMode( work->camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

      work->seq++;
    }
    /* break through */
  case 2:
    {
      //ターゲットを動かす
      int tmp;
      u16 rad;
      param->NowSync++;
      tmp = (param->NowSync * 0x10000) / param->Sync;
      rad = tmp;

      if (param->NowSync >= param->Sync)
      {
        param->Time--;
        param->NowSync = 0;
      }
      if (param->Time == 0)
      {
        param->NowSync = 0;
        end = TRUE;
      }

      if (end)
      {
        //カメラオフセットクリア
        {
          VecFx32 cam_ofs = {0,0,0};
          FIELD_CAMERA_SetCamPosOffset( work->camera, &cam_ofs );
        }
        //復帰パラメータをクリアする
        FIELD_CAMERA_ClearRecvCamParam(work->camera);

        //際バインド
        FIELD_CAMERA_BindTarget(work->camera, work->WatchTarget);
        //トレース再開
        FIELD_CAMERA_RestartTrace(work->camera);
        work->seq++;
      }
      else
      {
        fx32 w, h;
        VecFx32 target;
        VecFx32 cam_ofs;
        w = param->Width * FX_SinIdx(rad);
        h = param->Height * FX_SinIdx(rad);
        target = *work->WatchTarget;
        target.x += w;
        target.y += h;
        cam_ofs.x = w;
        cam_ofs.y = h;
        cam_ofs.z = h;

        //座標セット
        FIELD_CAMERA_SetTargetPos( work->camera, &target );
        //FIELD_CAMERA_SetCamPosOffset( work->camera, &cam_ofs );
      }
    }
    break;
  case 3:
    return FIELD_TASK_RETVAL_FINISH;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

//=============================================================================
/**
 *      static関数
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ジムの初期化
 *
 *	@param	GYM_FLY_TMP *p_wk
 *	@param	u16 c_time  経過時間
 */
//-----------------------------------------------------------------------------
static void GYM_FLY_Init( GYM_FLY_TMP *p_wk, const u16 c_time )
{
  WIND_STATE tmp_state = WIND_GetState( c_time );

  // ファンの速さ
  const fx32 fan_speed_tbl[] = {
    FAN_SPEED_LOW,
    FAN_SPEED_MIDDLE,
    FAN_SPEED_HIGH,
    FAN_SPEED_SUPER,
  };

  // 風力計の速さ
  const fx32 windmeter_speed_tbl[] = {
    WINDMETER_SPEED_LOW,
    WINDMETER_SPEED_MIDDLE,
    WINDMETER_SPEED_HIGH,
    WINDMETER_SPEED_SUPER,
  };

  // 状態更新
  p_wk->state = tmp_state;
  ICHI_Printf("初期化 %d \n", p_wk->state );

  // 状態別アニメの速さ
  p_wk->fan_wk.anm_speed     = fan_speed_tbl[ p_wk->state ];
  p_wk->fan_wk.anm_speed_max = fan_speed_tbl[ p_wk->state ];
  p_wk->windmeter_wk.anm_speed     = windmeter_speed_tbl[ p_wk->state ];
  p_wk->windmeter_wk.anm_speed_max = windmeter_speed_tbl[ p_wk->state ];
  
  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // SE ピッチ変更
  SOUND_ChangePitch( &p_wk->sound, tmp_state );
  p_wk->sound.pitch = p_wk->sound.pitch_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief  時間を見て風力計の状態を返す
 *
 *	@param	const u16 c_time 
 *
 *	@return その時間の風力計状態 WIND_STATE_XXX
 */
//-----------------------------------------------------------------------------
static WIND_STATE WIND_GetState( const u16 c_time )
{
  u16 windmeter_time1;    // 低速から中速の時間
  u16 windmeter_time2;    // 中速から高速の時間

#ifdef GYM_FLY_DEBUG_MENU
  windmeter_time1 = debug_windmeter_time1;
  windmeter_time2 = debug_windmeter_time2;
#else
  windmeter_time1 = WINDMETER_MIDDLE_TIME;
  windmeter_time2 = WINDMETER_HIGH_TIME;
#endif

  if( c_time >= windmeter_time2 )
  {
    return WIND_STATE_HIGH;
  }
  else if( c_time >= windmeter_time1 )
  {
    return WIND_STATE_MIDDLE;
  }
  else
  {
    return WIND_STATE_LOW;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風の状態変更
 *
 *	@param	FIELDMAP_WORK *p_fieldmap    フィールドワークポインタ
 *	@param	WIND_STATE state      変更後の状態
 */
//-----------------------------------------------------------------------------
static void WIND_ChangeState( FIELDMAP_WORK *p_fieldmap, const WIND_STATE state )
{
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);
  u8  i;
  u8  count;
  u8  obj_idx;

  // ファンの速さ
  const fx32 fan_speed_tbl[] = {
    FAN_SPEED_LOW,
    FAN_SPEED_MIDDLE,
    FAN_SPEED_HIGH,
    FAN_SPEED_SUPER,
  };

  // 風力計の速さ
  const fx32 windmeter_speed_tbl[] = {
    WINDMETER_SPEED_LOW,
    WINDMETER_SPEED_MIDDLE,
    WINDMETER_SPEED_HIGH,
    WINDMETER_SPEED_SUPER,
  };

  // 状態更新
  p_wk->state = state;

  // 状態別アニメの速さ
  p_wk->fan_wk.anm_speed_max = fan_speed_tbl[ p_wk->state ];
  p_wk->windmeter_wk.anm_speed_max = windmeter_speed_tbl[ p_wk->state ];
  
  // 状態別初期化
  switch( p_wk->state ){
  case WIND_STATE_LOW:
    {
      // αで透明にする
      p_wk->alpha_reset = TRUE;
    }

    // SEチェンジ
    SOUND_ChangeSE( &p_wk->sound );

    ICHI_Printf("風力計状態：低速\n");
    break;
  case WIND_STATE_MIDDLE:
    // 風エフェクト2(マップ)エフェクト
    {
      count   = 1;
      obj_idx = EFFECT_WIND_2;
    }
    ICHI_Printf("風力計状態：中速\n");
    break;
  case WIND_STATE_HIGH:
    // 風エフェクト3(マップ)エフェクト
    {
      count   = 1;
      obj_idx = EFFECT_WIND_3;
    }

    p_wk->time = 0;

    // SEチェンジ
    SOUND_ChangeSE( &p_wk->sound );

    // 描画軽くするために、風1は消す
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_1, TRUE );
    ICHI_Printf("風力計状態：高速\n");
    break;
  case WIND_STATE_SUPER:
    // 風エフェクト2,3(マップ)エフェクト
    {
      count   = 2;
      obj_idx = EFFECT_WIND_2;
    }
    break;
  default:
    GF_ASSERT(0);
  }

  // 風エフェクト(マップ)エフェクト設定
  // 中速、高速だと1回ループでそれぞれの風の設定
  // 超高速だと2回ループで両方の設定をする
  if( p_wk->state > WIND_STATE_LOW )
  {
    for( i=0; i<count; i++ )
    {
      // 表示
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );

      // アニメ
      {
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
        // アニメ再生開始してないなら開始
        if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == TRUE )
        {
          FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, 0 );
          FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始
        }
      }

      // α計算ON
      p_wk->alpha_flag[ (obj_idx-EFFECT_WIND_2)+i] = TRUE;
    }
  }

  // SE ピッチ変更
  SOUND_ChangePitch( &p_wk->sound, state );
}

//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベント作成
 *
 *	@param	GMEVENT* parent
 *	@param	gsys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* WIND_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*          event;
  BLOW_EVENT_WORK*  work;
  FIELD_PLAYER*     player;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  s16 gx, gy, gz;   // プレイヤーグリッド位置座標

  player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  // 場所によってイベント関数を変更
  {
    FIELD_PLAYER_GetGridPos( player, &gx, &gy, &gz );

    GF_ASSERT( gx < MAP_ARRAY_COL );
    GF_ASSERT( gz < MAP_ARRAY_ROW );

    // 配列から自機座標のデータを取得
    if( map_array[gz][gx] != MAP_ARRAY_BLOW )
    {
      event = GMEVENT_Create( gsys, parent, BlowEventSafe, sizeof(BLOW_EVENT_WORK) );
    }
    else
    {
      event = GMEVENT_Create( gsys, parent, BlowEvent, sizeof(BLOW_EVENT_WORK) );
    }
  }

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;
  work->mmdl        = FIELD_PLAYER_GetMMdl( player );
  work->hit_type    = BLOW_HIT_MAX;
  work->time        = 0;
  work->save_time   = 0;
  work->stay_walk_cnt = 0;

  // 自機の1グリッド下が壁・NPCかチェック
  if( map_array[gz+1][gx] == MAP_ARRAY_STOP ||
      MMDLSYS_SearchGridPos( FIELDMAP_GetMMdlSys( p_fieldmap ), gx, gz+1, FALSE ) )
  {
    work->is_wall = TRUE;
  }
  else
  {
    work->is_wall = FALSE;
  }


  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief 吹き飛ばしイベント関数
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT BlowEvent( GMEVENT* event, int* seq, void* wk )
{
  BLOW_EVENT_WORK *p_wk = (BLOW_EVENT_WORK*)wk;
  GYM_FLY_TMP     *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_WAIT,   // グリッド調度待ち
    //SEQ_CAMERA, // カメラ振動
    SEQ_STAY_WALK,// その場で足踏み
    SEQ_START,  // イベント開始 
    SEQ_STOP,   // 壁にぶつかった
    SEQ_SE_WAIT,// SE再生待ち
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
  };

  switch( *seq ){
  // グリッド調度待ち
  case SEQ_WAIT:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // ！マーク
      MMDL_SetAcmd( p_wk->mmdl, AC_MARK_GYOE ); 

      *seq = SEQ_STAY_WALK;
    }
    break;

#if 0
  // カメラ振動
  case SEQ_CAMERA:
    {
      // カメラ振動タスクを登録
      {
        FIELD_TASK* cam;
        FIELD_TASK_MAN* taskMan;

        CAM_SHAKE_PARAM shakeParam;
        shakeParam.Width           = SHAKE_CAM_WIDTH;
        shakeParam.Height          = 0;
        shakeParam.Sync            = SHAKE_CAM_SYNC;
        shakeParam.Time            = SHAKE_CAM_TIME;
        shakeParam.NowSync         = 0;
        shakeParam.WaitStartFrame  = SHAKE_CAM_WAIT;

        cam = FIELD_TASK_ShakeCamera( p_wk->p_fieldmap, &shakeParam );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );
      }
      *seq = SEQ_STAY_WALK;
    }
    break;
#endif

  // その場で足踏み
  case SEQ_STAY_WALK:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
      dir %= 4;
      MMDL_SetAcmd( p_wk->mmdl, (AC_STAY_WALK_U_4F + dir) );
      p_wk->stay_walk_cnt += 1;

      if( p_wk->stay_walk_cnt >= STAY_WALK_NUM )
      {
        *seq = SEQ_START;
      }
    }
    break;

  // イベント開始
  case SEQ_START:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      u8  move_distance = 0;    // 移動距離
      // 足元チェック
      // エクセル表のデータを自機のグリッド座標を使って参照する
      {
        s16 gx, gy, gz;
        FIELD_PLAYER_GetGridPos( p_wk->player, &gx, &gy, &gz );
        
        // 吹き飛び距離計算
        move_distance = BLOW_EVENT_GetMoveDistance( p_wk->p_fieldmap, &p_wk->hit_type, gx, gz );

        GF_ASSERT( p_wk->hit_type<BLOW_HIT_MAX );
      }

      // 移動タスク登録
      {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        VecFx32 down_pos;
        {
          FIELD_PLAYER_GetPos( p_wk->player, &down_pos );
          down_pos.z += GRID_TO_FX32(move_distance);
        }
        move = FIELD_TASK_TransPlayer( p_wk->p_fieldmap, move_distance*3, &down_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
      }

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        MMDL_OnStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      // カメラ振動タスクを登録
      if( p_wk->is_wall == FALSE )
      {
        FIELD_TASK* cam;
        FIELD_TASK_MAN* taskMan;

        CAM_SHAKE_PARAM shakeParam;
#ifdef GYM_FLY_DEBUG_MENU
        shakeParam.Width           = debug_w;
        shakeParam.Height          = debug_h;
        shakeParam.Sync            = debug_sync;
        shakeParam.Time            = debug_time;
#else
        shakeParam.Width           = 0;
        shakeParam.Height          = 2;
        shakeParam.Sync            = 3;
        shakeParam.Time            = 4;
#endif
        shakeParam.NowSync         = 0;
        shakeParam.WaitStartFrame  = move_distance*3 - 6; // -6でぶつかる前にタスク起動

        cam = FIELD_TASK_ShakeCamera( p_wk->p_fieldmap, &shakeParam );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );
      }

      // 下が壁だと再生しない
      if( p_wk->is_wall == FALSE )
      {
        // SE ヒュー
        PMSND_PlaySE( GYM_FLY_SE_BLOW );
      }

      *seq = SEQ_MAIN;
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      // 自機の向きを変える
      if( MMDL_CheckEndAcmd(p_wk->mmdl) )
      {
        u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
        dir++;
        dir %= 4;
        MMDL_SetAcmd( p_wk->mmdl, (AC_DIR_U + dir) );
      }

      // タスク終了チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        // カメラ振動タスクが残っているため1以下
        if( FIELD_TASK_MAN_GetTaskNum(taskMan) <= 1 ) {

          // SE 停止
          if( p_wk->is_wall == FALSE )
          {
            SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
            PMSND_StopSE_byPlayerID( player_ID );
          }

          // ぶつかる時のSE
          if( p_wk->is_wall == FALSE )
          {
            switch( p_wk->hit_type ){
            case BLOW_HIT_WALL:      // 壁にぶつかる
              // SE びたーん
              PMSND_PlaySE( GYM_FLY_SE_WALL_HIT );
              break;
            case BLOW_HIT_NPC:       // NPCにぶつかる
              // SE ぶう
              PMSND_PlaySE( GYM_FLY_SE_NPC_HIT );
              break;
            default:
              GF_ASSERT(0);
            }
          }

          *seq = SEQ_STOP;
        }
      }
    }
    break;

  // 壁にぶつかった
  case SEQ_STOP:
    {
      BOOL ret = FALSE;

      // 自機の向きを変える
      if( MMDL_CheckEndAcmd(p_wk->mmdl) )
      {
        u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
        dir++;
        dir %= 4;
        MMDL_SetAcmd( p_wk->mmdl, (AC_DIR_U + dir) );
      }

      // 時間計測
      // 一定時間は吹き飛ばしイベントを続ける
      {
#ifdef GYM_FLY_DEBUG_MENU
        const u16 blow_event_time_max = debug_blow_event_time_max;
#else
        const u16 blow_event_time_max = BLOW_EVENT_TIME;
#endif

        if( p_wk->time >= blow_event_time_max )
        {
          ret = TRUE;
        }
      }

      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) == FALSE ) {
          ret = FALSE;
        }
      }
      if( ret == TRUE )
      {
        // 移動しなかった場合は
        // SEを再生しない
        if( p_wk->is_wall == FALSE )
        {
          *seq = SEQ_SE_WAIT;
        }
        else
        {
          *seq = SEQ_EXIT;
        }
      }
    }
    break;

  // SE再生待ち
  case SEQ_SE_WAIT:
    {
      SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_WALL_HIT );
      if( PMSND_CheckPlaySE_byPlayerID( player_ID ) == FALSE )
      {
        *seq = SEQ_EXIT;
      }
      player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_NPC_HIT );
      if( PMSND_CheckPlaySE_byPlayerID( player_ID ) == FALSE )
      {
        *seq = SEQ_EXIT;
      }
    }
    break;

  // イベント終了
  case SEQ_EXIT:

    // 高さ取得禁止ステータスビットをOFFに戻す
    {
      MMDL_OffStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
    }

    // 上を向く
    MMDL_SetAcmd( p_wk->mmdl, AC_DIR_U );

    return GMEVENT_RES_FINISH;
  }

  // このイベントの時間カウント
  p_wk->time++;

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief 吹き飛ばしイベント関数
 *	       安全地帯にいる場合
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk 
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT BlowEventSafe( GMEVENT* event, int* seq, void* wk )
{
  BLOW_EVENT_WORK *p_wk = (BLOW_EVENT_WORK*)wk;
  GYM_FLY_TMP     *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_WAIT,   // グリッド調度待ち
    SEQ_START,   // イベント開始 
    SEQ_SHAKE_START,// 1回目のゆれ
    SEQ_SHAKE_WAIT, // 振動待ち
    SEQ_SHAKE_START2,// 2回目のゆれ
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
  };

  switch( *seq ){
  // グリッド調度待ち
  case SEQ_WAIT:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // ！マーク
      MMDL_SetAcmd( p_wk->mmdl, AC_MARK_GYOE ); 

      *seq = SEQ_START;
    }
    break;
  // イベント開始
  case SEQ_START:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // 波乗りのOBJコードに強制的に変更する
      int sex = FIELD_PLAYER_GetSex( p_wk->player );
      u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SAVEHERO );

      if( MMDL_GetOBJCode( p_wk->mmdl ) != code ){
        FIELD_PLAYER_ChangeOBJCode( p_wk->player, code );
      }

      p_wk->save_time = p_wk->time;

      *seq = SEQ_SHAKE_START;
    }
    break;

  case SEQ_SHAKE_START:// 1回目のゆれ
    {
      // 指定時間待つ
      if( (p_wk->time-p_wk->save_time) > SHAKE_WAIT_TIME1 )
      {
        // 振動タスク登録
        {
          FIELD_TASK* shake;
          FIELD_TASK_MAN* taskMan;
          shake = FIELD_TASK_ShakeMMdl( p_wk->p_fieldmap, p_wk->mmdl,
              SHAKE_MMDL_WIDTH,
              SHAKE_MMDL_SYNC,
              SHAKE_MMDL_TIME );
          taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
          FIELD_TASK_MAN_AddTask( taskMan, shake, NULL );
        }

        *seq = SEQ_SHAKE_WAIT;
      }
    }
    break;

  // 振動待ち
  case SEQ_SHAKE_WAIT:
    {
      // 次シーケンス移行チェック
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) )
      {
        p_wk->save_time = p_wk->time;
        *seq = SEQ_SHAKE_START2;
      }
    }
    break;

  case SEQ_SHAKE_START2:// 2回目のゆれ
    {
      // 指定時間待つ
      if( (p_wk->time-p_wk->save_time) > SHAKE_WAIT_TIME2 )
      {
        // 振動タスク登録
        {
          FIELD_TASK* shake;
          FIELD_TASK_MAN* taskMan;
          shake = FIELD_TASK_ShakeMMdl( p_wk->p_fieldmap, p_wk->mmdl,
              SHAKE_MMDL_WIDTH,
              SHAKE_MMDL_SYNC,
              SHAKE_MMDL_TIME );
          taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
          FIELD_TASK_MAN_AddTask( taskMan, shake, NULL );
        }

        *seq = SEQ_MAIN;
      }
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      BOOL  ret = FALSE;

      // 時間計測
      // 一定時間は吹き飛ばしイベントを続ける
      {
#ifdef GYM_FLY_DEBUG_MENU
        const u16 safe_event_time_max = debug_safe_event_time_max;
#else
        const u16 safe_event_time_max = SAFE_EVENT_TIME;
#endif

        if( p_wk->time >= safe_event_time_max )
        {
          ret = TRUE;
        }
      }

      // 次シーケンス移行チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan)  
            && ret == TRUE )
        {
          *seq = SEQ_EXIT;
        }
      }
    }
    break;

  // イベント終了
  case SEQ_EXIT:
    {
      // OBJコードを通常に戻す
      int sex = FIELD_PLAYER_GetSex( p_wk->player );
      u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );

      if( MMDL_GetOBJCode( p_wk->mmdl ) != code ){
        FIELD_PLAYER_ChangeOBJCode( p_wk->player, code );
      }
    }
    return GMEVENT_RES_FINISH;
  }

  // このイベントの時間カウント
  p_wk->time++;

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *      帰り道ショートカットイベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベント作成
 *
 *	@param	GMEVENT* parent
 *	@param	gsys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* SHORTCUT_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*          event;
  SHORTCUT_EVENT_WORK*  work;
  FIELD_PLAYER*     player;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  event = GMEVENT_Create( gsys, parent, ShortcutEvent, sizeof(SHORTCUT_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;
  work->mmdl        = FIELD_PLAYER_GetMMdl( player );
  work->stay_walk_cnt = 0;

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief 吹き飛ばしイベント関数
 *	       帰り道のショートカット
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk 
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT ShortcutEvent( GMEVENT* event, int* seq, void* wk )
{
  SHORTCUT_EVENT_WORK *p_wk = (SHORTCUT_EVENT_WORK*)wk;
  GYM_FLY_TMP     *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_WAIT,   // グリッド調度待ち
    SEQ_WIND_SPEEDUP,// 風力最大にUP
    SEQ_WIND_SPEEDUP_WAIT,// 風力最大にUP待ち
    SEQ_STAY_WALK,// その場で足踏み
    SEQ_START,  // イベント開始 
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
    SEQ_MAP_CHANGE, // マップチェンジイベント
  };

  switch( *seq ){
  // グリッド調度待ち
  case SEQ_WAIT:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      // ！マーク
      MMDL_SetAcmd( p_wk->mmdl, AC_MARK_GYOE ); 

      *seq = SEQ_WIND_SPEEDUP;
    }
    break;

  // 風力を最大に上げる
  case SEQ_WIND_SPEEDUP:
    {
      WIND_ChangeState( p_wk->p_fieldmap, WIND_STATE_HIGH );

      // カメラ回転タスクを登録
      {
        FIELD_TASK* pitch;
        FIELD_TASK* zoom;
        FIELD_TASK_MAN* taskMan;

        pitch = FIELD_TASK_CameraRot_Pitch( p_wk->p_fieldmap, SHORTCUT_CAMERA_FRAME, SHORTCUT_CAMERA_PITCH );
        zoom = FIELD_TASK_CameraLinearZoom( p_wk->p_fieldmap, SHORTCUT_CAMERA_FRAME, SHORTCUT_CAMERA_LEGTH );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, pitch, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, zoom, NULL );
      }
      *seq = SEQ_WIND_SPEEDUP_WAIT;
    }
    break;
  case SEQ_WIND_SPEEDUP_WAIT:
    {
      if( FAN_CheckAnmSpeed( &p_gym_wk->fan_wk ) == TRUE
          && WINDMETER_CheckAnmSpeed( &p_gym_wk->windmeter_wk ) == TRUE )
      {
        *seq = SEQ_STAY_WALK;
      }
    }
    break;
   
  // その場で足踏み
  case SEQ_STAY_WALK:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
      dir %= 4;
      MMDL_SetAcmd( p_wk->mmdl, (AC_STAY_WALK_U_4F + dir) );
      p_wk->stay_walk_cnt += 1;


      // 一定回数足踏みかつカメラ揺れ終了
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) 
            && ( p_wk->stay_walk_cnt >= STAY_WALK_NUM ) )
        {
          *seq = SEQ_START;
        }
      }
    }
    break;

  // イベント開始
  case SEQ_START:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      u8  move_distance = 0;    // 移動距離
      // 足元チェック
      // エクセル表のデータを自機のグリッド座標を使って参照する
      {
        s16 gx, gy, gz;
        FIELD_PLAYER_GetGridPos( p_wk->player, &gx, &gy, &gz );

        // 出口までの移動距離
        move_distance = (MAP_ARRAY_ROW-1)-gz;
      }

      // 移動タスク登録
      {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        VecFx32 down_pos;
        {
          FIELD_PLAYER_GetPos( p_wk->player, &down_pos );
          down_pos.z += GRID_TO_FX32(move_distance);
        }
        move = FIELD_TASK_TransPlayer( p_wk->p_fieldmap, move_distance*2, &down_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
      }

      // 回転タスク登録
      {
        FIELD_TASK* rot;
        FIELD_TASK_MAN* taskMan;
        // +10の意味はマップチェンジのフェードアウトぎりぎりまで回転しておく為
        rot = FIELD_TASK_PlayerRotate( p_wk->p_fieldmap, (move_distance*2+10), (move_distance*2+10)/4 );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, rot, NULL );
      }

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        MMDL_OnStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      // SE ヒュー
      PMSND_PlaySE( GYM_FLY_SE_BLOW );

      *seq = SEQ_MAIN;
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {

      // 箱衝突チェック
      {
        s16 gx, gy, gz;
        FIELD_PLAYER_GetGridPos( p_wk->player, &gx, &gy, &gz );

        if( gz == BOX1_GZ )
        {
          EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, OBJ_BOX_1, 0 );
          if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == TRUE )
          {
            FLD_EXP_OBJ_SetObjAnmFrm( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, OBJ_BOX_1, 0, 0 );
            FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始

            // SE 箱破壊
            PMSND_PlaySE( GYM_FLY_SE_BOX_BREAK );
          }
        }

        if( gz == BOX2_GZ )
        {
          EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, OBJ_BOX_2, 0 );
          if( FLD_EXP_OBJ_GetAnmStopFlg( exp_anm ) == TRUE )
          {
            FLD_EXP_OBJ_SetObjAnmFrm( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, OBJ_BOX_2, 0, 0 );
            FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始

            // SE 箱破壊
            PMSND_PlaySE( GYM_FLY_SE_BOX_BREAK );
          }
        }
      }

      // タスク終了チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );

        // 自機回転タスクが残っているため1以下
        if( FIELD_TASK_MAN_GetTaskNum(taskMan) <= 1 ) {

          // SE 停止
          {
            SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
            PMSND_StopSE_byPlayerID( player_ID );
          }


          // 高さ取得禁止ステータスビットをOFFに戻す
          {
            MMDL_OffStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
          }

          *seq = SEQ_MAP_CHANGE;
        }
      }
    }
    break;

  case SEQ_MAP_CHANGE: // マップチェンジイベント
    // マップチェンジ
    {
      // 通常のマップチェンジを使っていたが、
      // 外に出たときに、歩く演出がないので、
      // 本来のドアを出たときのマップチェンジと同じものを使用した
      {
        GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
        GMEVENT* new_event;
        EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( GAMESYSTEM_GetGameData(p_gsys) );
        VecFx32 pos;
        int     idx;
        LOC_EXIT_OFS        exit_ofs;
        const CONNECT_DATA  *cnct;

        FIELD_PLAYER_GetPos( p_wk->player, &pos );
        idx = EVENTDATA_SearchConnectIDByPos( evdata, &pos );
        cnct = EVENTDATA_GetConnectByID( evdata, idx);
        exit_ofs = CONNECTDATA_GetExitOfs( cnct, &pos);

        new_event = EVENT_ChangeMapByConnect( p_gsys, p_wk->p_fieldmap, cnct, exit_ofs );
        GMEVENT_ChangeEvent( event, new_event );
      }

      ICHI_Printf("マップチェンジイベント\n");
    }
    *seq = SEQ_EXIT;
    break;

  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  スクリプト用吹き飛ばしイベント
 *	        サングラスが吹き飛ぶ
 *
 *	@param	GMEVENT* parent
 *	@param	gsys 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* SCRIPT_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys, const u16 obj_id )
{
  GMEVENT*          event;
  BLOW_EVENT_WORK*  work;
  FIELD_PLAYER*     player;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS     *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  s16 gx, gy, gz;   // プレイヤーグリッド位置座標

  player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // イベント生成
  event = GMEVENT_Create( gsys, parent, ScriptBlowEvent, sizeof(BLOW_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys        = gsys;
  work->p_fieldmap  = p_fieldmap;
  work->player      = player;
  work->mmdl        = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );  // サングラス
  work->hit_type    = BLOW_HIT_MAX;
  work->time        = 0;
  work->save_time   = 0;
  work->stay_walk_cnt = 0;

  GF_ASSERT( work->mmdl );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプト用吹き飛ばしイベント関数
 *
 *	@param	GMEVENT* event
 *	@param	seq
 *	@param	wk 
 *
 *	@note   処理の流れ
 *	        風力を最大にUP→ガイドが飛ぶ→ぶつかる→自機1グリッド下がる
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ScriptBlowEvent( GMEVENT* event, int* seq, void* wk )
{
  BLOW_EVENT_WORK *p_wk = (BLOW_EVENT_WORK*)wk;
  GYM_FLY_TMP     *p_gym_wk = GMK_TMP_WK_GetWork( p_wk->p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);

  // 処理シーケンス
  enum{
    SEQ_WIND_SPEEDUP,// 風力最大にUP
    SEQ_WIND_SPEEDUP_WAIT,// 風力最大にUP待ち
    SEQ_START,  // イベント開始 
    SEQ_MAIN,   // メイン処理
    SEQ_PLAYER_MOVE,  // 自機移動
    SEQ_PLAYER_MOVE_WAIT,  // 自機移動待ち
    SEQ_EXIT,   // イベント終了
  };

  switch( *seq ){
  // 風力を最大に上げる
  case SEQ_WIND_SPEEDUP:
    {
      WIND_ChangeState( p_wk->p_fieldmap, WIND_STATE_HIGH );
      *seq = SEQ_WIND_SPEEDUP_WAIT;
    }
    break;
  // 風力を最大に上げる待ち
  case SEQ_WIND_SPEEDUP_WAIT:
    {
      if( FAN_CheckAnmSpeed( &p_gym_wk->fan_wk ) == TRUE
          && WINDMETER_CheckAnmSpeed( &p_gym_wk->windmeter_wk ) == TRUE )
      {
        *seq = SEQ_START;
      }
    }
    break;
  // イベント開始
  case SEQ_START:
    if( MMDL_CheckEndAcmd(p_wk->mmdl) )
    {
      u8  move_distance = 10;    // 移動距離

      // 移動タスク登録
      {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        VecFx32 down_pos;
        {
          MMDL_GetVectorPos( p_wk->mmdl, &down_pos );
          down_pos.z += GRID_TO_FX32(move_distance);
        }
        move = FIELD_TASK_TransMMdl( p_wk->p_fieldmap, p_wk->mmdl, move_distance*3, &down_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
      }

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        MMDL_OnStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      // SE ヒュー
      PMSND_PlaySE( GYM_FLY_SE_BLOW );

      *seq = SEQ_MAIN;
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      // 自機の向きを変える
      if( MMDL_CheckEndAcmd(p_wk->mmdl) )
      {
        u16 dir = MMDL_GetDirDisp( p_wk->mmdl );
        dir++;
        dir %= 4;
        MMDL_SetAcmd( p_wk->mmdl, (AC_DIR_U + dir) );
      }

      // タスク終了チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {

          // SE 停止
          {
            SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
            PMSND_StopSE_byPlayerID( player_ID );
          }

          // ぶつかる時のSE
          // SE ぶう
          PMSND_PlaySE( GYM_FLY_SE_NPC_HIT );

          // 風力を最低に戻す
          WIND_ChangeState( p_wk->p_fieldmap, WIND_STATE_LOW );

          *seq = SEQ_PLAYER_MOVE;
        }
      }
    }
    break;

  case SEQ_PLAYER_MOVE:  // 自機移動
    // 移動タスク登録
    {
      FIELD_TASK* move;
      FIELD_TASK_MAN* taskMan;
      VecFx32 down_pos;
      {
        FIELD_PLAYER_GetPos( p_wk->player, &down_pos );
        down_pos.z += GRID_TO_FX32(1);
      }
      move = FIELD_TASK_TransPlayer( p_wk->p_fieldmap, 2, &down_pos );
      taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, move, NULL );

      *seq = SEQ_PLAYER_MOVE_WAIT;
    }
    break;

  case SEQ_PLAYER_MOVE_WAIT:  // 自機移動待ち
    // タスク終了チェック
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {
        *seq = SEQ_EXIT;
      }
    }
    break;

  // イベント終了
  case SEQ_EXIT:

    // 高さ取得禁止ステータスビットをOFFに戻す
    {
      MMDL_OffStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
    }

    // モデルの座標更新
    {
      VecFx32 pos;
      MMDL_GetVectorPos( p_wk->mmdl, &pos );
      MMDL_InitPosition( p_wk->mmdl, &pos, DIR_DOWN );
    }

    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしの距離を計算
 *
 *	@param	FIELDMAP_WORK *p_fieldmap
 *	@param	s16 gx    自機グリッドX座標
 *	@param	s16 gz    自機グリッドZ座標
 *	@param  p_hit_type
 *
 *	@return u8  移動距離(グリッド)
 */
//-----------------------------------------------------------------------------
static const u8 BLOW_EVENT_GetMoveDistance( FIELDMAP_WORK *p_fieldmap, BLOW_HIT_TYPE *p_hit_type, s16 gx, s16 gz )
{
  u8 move_distance = 0;

  // 吹き飛び距離計算
  while( gz<MAP_ARRAY_ROW )
  {
    ICHI_Printf("自機GRID座標 x:%d z:%d ", gx, gz);
    ICHI_Printf("map_array %d \n", map_array[gz][gx] );

    // 1グリッド下
    gz += 1;

    // 配列から自機座標のデータを取得
    if( map_array[gz][gx] != MAP_ARRAY_BLOW )
    {
      // 吹き飛べないならストップ
      *p_hit_type = BLOW_HIT_WALL;
      break;
    }

    // NPCいるかチェック
    {
      MMDL *p_mmdl;
      // 動作モデルを検索
      p_mmdl = MMDLSYS_SearchGridPos(
          FIELDMAP_GetMMdlSys( p_fieldmap ),
          gx,
          gz,
          TRUE );

      // 動作モデルを発見
      if( p_mmdl )
      {
        *p_hit_type = BLOW_HIT_NPC;
        break;
      }
    }

    // 移動距離加算
    move_distance += 1;
  }

  return move_distance;
}

//=============================================================================
/**
 *      EXPOBJ
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	GYM_FLY_TMP *p_wk
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GYM_FLY_TMP *p_wk )
{
  u8    i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GYM_FLY_UNIT_IDX);

  //=====================================
  ///	ファンの設定
  //
  {
    u16   obj_idx = OBJ_FAN;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    status->trans.x = 0;
    status->trans.y = 0;
    status->trans.z = 0;
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, p_wk->fan_wk.anm_speed ); // アニメSPD
    }
  }

  //=====================================
  ///	風力計の設定
  //
  for( i=0; i<WINDMETER_NUM_MAX; i++ )
  {
    // 座標セット
    VecFx32 pos[ WINDMETER_NUM_MAX ] = {
      { WINDMETER1_GX,  WINDMETER1_GY,  WINDMETER1_GZ },
      { WINDMETER2_GX,  WINDMETER2_GY,  WINDMETER2_GZ },
      { WINDMETER3_GX,  WINDMETER3_GY,  WINDMETER3_GZ },
      { WINDMETER4_GX,  WINDMETER4_GY,  WINDMETER4_GZ },
      { WINDMETER5_GX,  WINDMETER5_GY,  WINDMETER5_GZ },
      { WINDMETER6_GX,  WINDMETER6_GY,  WINDMETER6_GZ },
      { WINDMETER7_GX,  WINDMETER7_GY,  WINDMETER7_GZ },
      { WINDMETER8_GX,  WINDMETER8_GY,  WINDMETER8_GZ },
      { WINDMETER9_GX,  WINDMETER9_GY,  WINDMETER9_GZ },
      { WINDMETER10_GX, WINDMETER10_GY, WINDMETER10_GZ },
      { WINDMETER11_GX, WINDMETER11_GY, WINDMETER11_GZ },
      { WINDMETER12_GX, WINDMETER12_GY, WINDMETER12_GZ },
      { WINDMETER13_GX, WINDMETER13_GY, WINDMETER13_GZ },
      { WINDMETER14_GX, WINDMETER14_GY, WINDMETER14_GZ },
      { WINDMETER15_GX, WINDMETER15_GY, WINDMETER15_GZ },
      { WINDMETER16_GX, WINDMETER16_GY, WINDMETER16_GZ },
      { WINDMETER17_GX, WINDMETER17_GY, WINDMETER17_GZ },
    };

    u16   obj_idx = OBJ_WINDMETER_1 + i;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    status->trans.x = FX32_CONST( pos[i].x * FIELD_CONST_GRID_SIZE );
    status->trans.y = FX32_CONST( pos[i].y * FIELD_CONST_GRID_SIZE );
    status->trans.z = FX32_CONST( pos[i].z * FIELD_CONST_GRID_SIZE );
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, p_wk->windmeter_wk.anm_speed ); // アニメSPD
    }
  }

  //=====================================
  ///	箱の設定
  //
  for( i=0; i<BOX_NUM_MAX; i++ )
  {
    // 座標セット
    VecFx32 pos[ BOX_NUM_MAX ] = {
      { BOX1_GX,  BOX1_GY,  BOX1_GZ },
      { BOX2_GX,  BOX2_GY,  BOX2_GZ },
    };

    u16   obj_idx = OBJ_BOX_1 + i;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    status->trans.x = FX32_CONST( pos[i].x * FIELD_CONST_GRID_SIZE );
    status->trans.y = FX32_CONST( pos[i].y * FIELD_CONST_GRID_SIZE );
    status->trans.z = FX32_CONST( pos[i].z * FIELD_CONST_GRID_SIZE );
    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );    // アニメ停止
    }
  }

  //=====================================
  ///	風エフェクト(マップ)の設定
  //
  for( i=EFFECT_WIND_1; i<=EFFECT_WIND_3; i++ )
  {
    u16   obj_idx = i;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    status->trans.x = 0;
    status->trans.y = 0;
    status->trans.z = 0;

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );

    // 低速の風のみ表示
    // 追加で、初期化状態が中速のときは中速風も表示
    if( i == EFFECT_WIND_1 
        || ( i == EFFECT_WIND_2 && p_wk->state == WIND_STATE_MIDDLE ) )
    {
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );  // 表示
    }
    else
    {
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );  // 表示
    }

    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	GYM_FLY_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GYM_FLY_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GYM_FLY_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GYM_FLY_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GYM_FLY_TMP *p_wk )
{
  // ファン
  FAN_Update( &p_wk->fan_wk );

  // 風力計
  WINDMETER_Update( &p_wk->windmeter_wk );

  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ファンの更新
 *
 *	@param	FAN_WORK *p_wk ワーク 
 */
//-----------------------------------------------------------------------------
static void FAN_Update( FAN_WORK *p_wk )
{
  u8 i;
  // アニメスピード計算
  if( p_wk->anm_speed != p_wk->anm_speed_max )
  {
    if( p_wk->anm_speed < p_wk->anm_speed_max )
    {
      p_wk->anm_speed += FAN_SPEED_ADD;
      if( p_wk->anm_speed > p_wk->anm_speed_max )
      {
        p_wk->anm_speed = p_wk->anm_speed_max;
      }
    }
    else
    {
      p_wk->anm_speed -= FAN_SPEED_ADD;
      if( p_wk->anm_speed < p_wk->anm_speed_max )
      {
        p_wk->anm_speed = p_wk->anm_speed_max;
      }
    }

    // ファンのOBJすべてのアニメを変更
    {
      u16   obj_idx = OBJ_FAN;    // OBJインデックス
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->parent_exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      // アニメスピード変更
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, p_wk->anm_speed );
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ファンのアニメ速度が最大か？
 *
 *	@param	const FAN_WORK *cp_wk 
 *
 *	@retval TRUE 最大
 *	@retval FALSE まだ上がる
 */
//-----------------------------------------------------------------------------
static BOOL FAN_CheckAnmSpeed( const FAN_WORK *cp_wk )
{
  if( cp_wk->anm_speed == cp_wk->anm_speed_max )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風力計の更新
 *
 *	@param	WINDMETER_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void WINDMETER_Update( WINDMETER_WORK *p_wk )
{
  u8 i;
  // アニメスピード計算
  if( p_wk->anm_speed != p_wk->anm_speed_max )
  {
    if( p_wk->anm_speed < p_wk->anm_speed_max )
    {
      p_wk->anm_speed += WINDMETER_SPEED_ADD;
      if( p_wk->anm_speed > p_wk->anm_speed_max )
      {
        p_wk->anm_speed = p_wk->anm_speed_max;
      }
    }
    else
    {
      p_wk->anm_speed -= WINDMETER_SPEED_ADD;
      if( p_wk->anm_speed < p_wk->anm_speed_max )
      {
        p_wk->anm_speed = p_wk->anm_speed_max;
      }
    }

    // ファンのOBJすべてのアニメを変更
    for( i=0; i<WINDMETER_NUM_MAX; i++ )
    {
      u16   obj_idx = OBJ_WINDMETER_1 + i;    // OBJインデックス
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->parent_exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      // アニメスピード変更
      FLD_EXP_OBJ_ChgAddFrame( exp_anm, p_wk->anm_speed );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風力計のアニメ速度が最大か？
 *
 *	@param	const WINDMETER_WORK *cp_wk 
 *
 *	@retval TRUE 最大
 *	@retval FALSE まだ上がる
 */
//-----------------------------------------------------------------------------
static BOOL WINDMETER_CheckAnmSpeed( const WINDMETER_WORK *cp_wk )
{
  if( cp_wk->anm_speed == cp_wk->anm_speed_max )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  音の更新
 *
 *	@param	SOUND_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void SOUND_Update( SOUND_WORK *p_wk, const u16 c_time )
{
  
  enum{
    SEQ_NORMAL_SE,    // 通常SE再生
    SEQ_TURN_SE,      // ターンSE再生
    SEQ_TURN_SE_WAIT, // ターンSE再生待ち
  };
  
  // 徐々にPitchを上げる
  if( p_wk->pitch != p_wk->pitch_max )
  {
    // pitch計算
    if( p_wk->pitch < p_wk->pitch_max )
    {
      p_wk->pitch += SE_PITCH_ADD;
      if( p_wk->pitch > p_wk->pitch_max )
      {
        p_wk->pitch = p_wk->pitch_max;
      }
    }
    else
    {
      p_wk->pitch -= SE_PITCH_ADD;
      if( p_wk->pitch < p_wk->pitch_max )
      {
        p_wk->pitch = p_wk->pitch_max;
      }
    }
  }

  switch( p_wk->seq ){
  case SEQ_NORMAL_SE:    // 通常SE再生
    {
      // pitch変更
      {
        // 風音
        SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_WIND_NORMAL );
        PMSND_SetStatusSE_byPlayerID( player_ID, PMSND_NOEFFECT, p_wk->pitch, PMSND_NOEFFECT);
      }

      // SEチェンジ
      if( p_wk->se_change == TRUE )
      {
        p_wk->se_change = FALSE;
        p_wk->seq = SEQ_TURN_SE;
      }
    }
    break;

  case SEQ_TURN_SE:      // ターンSE再生
    PMSND_PlaySE( GYM_FLY_SE_WIND_TURN );
    p_wk->seq = SEQ_TURN_SE_WAIT;
    break;

  case SEQ_TURN_SE_WAIT: // ターンSE再生待ち
    {
      if( p_wk->se_change == TRUE )
      {
        PMSND_PlaySE( GYM_FLY_SE_WIND_NORMAL );

        p_wk->se_change = FALSE;
        p_wk->seq = SEQ_NORMAL_SE;
      }
    }
    break;
  }

  

  // pitch変更
  {
    // 環境音
    SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_FAN );
    PMSND_SetStatusSE_byPlayerID( player_ID, PMSND_NOEFFECT, p_wk->pitch, PMSND_NOEFFECT);
  }


#ifdef PM_DEBUG
  // Rボタン押しながらでデバッグ
  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L)
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      PMSND_StopSE();
      ICHI_Printf("SE STOP!\n");
    }

    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
    {
      PMSND_PlaySE( GYM_FLY_SE_WIND_NORMAL );
      ICHI_Printf("SE H04 Play\n");
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      PMSND_PlaySE( GYM_FLY_SE_WIND_TURN );
      ICHI_Printf("SE H05 Play\n");
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
    {
      PMSND_PlaySE( GYM_FLY_SE_FAN );
      ICHI_Printf("SE H02 Play\n");
    }
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      PMSND_PlaySE( GYM_FLY_SE_BLOW );
      ICHI_Printf("SE H01 Play\n");
    }
  }
#endif

}

//----------------------------------------------------------------------------
/**
 *	@brief  音チェンジ
 *
 *	@param	SOUND_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void SOUND_ChangeSE( SOUND_WORK *p_wk )
{
  p_wk->se_change = TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ピッチ変更
 *
 *	@param	SOUND_WORK *p_wk
 *	@param	WIND_STATE state 
 */
//-----------------------------------------------------------------------------
static void SOUND_ChangePitch( SOUND_WORK *p_wk, const WIND_STATE state )
{
  const int pitch_tbl[] = {
    SE_PITCH_LOW,
    SE_PITCH_MIDDLE,
    SE_PITCH_HIGH,
    SE_PITCH_SUPER,
  };

  p_wk->pitch_max = pitch_tbl[ state ];

}

//=============================================================================
/**
 *    セーブ情報
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  タイム計測フラグを設定
 *
 *	@param	*p_wk 飛行ジムセーブワーク
 *	@param	flag  フラグ
 */
//-----------------------------------------------------------------------------
static void SAVE_SetTimeFlg( GYM_FLY_SV_WORK *p_wk, BOOL flag )
{
  p_wk->time_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイム計測フラグを取得
 *
 *	@param	*cp_wk 飛行ジムセーブワーク
 *
 *	@return flag  フラグ
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetTimeFlg( const GYM_FLY_SV_WORK *cp_wk )
{
  return cp_wk->time_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイム計測値を設定
 *
 *	@param	*p_wk 飛行ジムセーブワーク
 *	@param	time  タイム
 */
//-----------------------------------------------------------------------------
static void SAVE_SetTime( GYM_FLY_SV_WORK *p_wk, const u16 time )
{
  p_wk->time = time;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タイム計測値を取得
 *
 *	@param	*cp_wk 飛行ジムセーブワーク
 *
 *	@return time  タイム
 */
//-----------------------------------------------------------------------------
static u16  SAVE_GetTime( const GYM_FLY_SV_WORK *cp_wk )
{
  return cp_wk->time;
}

#ifdef GYM_FLY_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_GYM_FLY (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
static BOOL DebugWin_ChangeCamData( int *p_param, int min, int max );
// 風力変更の時間
static void DebugWin_Update_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeSafeTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeSafeTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeWidth( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeWidth( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeCamTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeCamTime( void *userWork, DEBUGWIN_ITEM *item );

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
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_FLY, "ひこうジム", heapID );

  // 風力変更の時間
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeTime1,
                              DebugWin_Draw_ChangeTime1,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeTime2,
                              DebugWin_Draw_ChangeTime2,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeBlowTime,
                              DebugWin_Draw_ChangeBlowTime,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeSafeTime,
                              DebugWin_Draw_ChangeSafeTime,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeWidth,
                              DebugWin_Draw_ChangeWidth,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeHeight,
                              DebugWin_Draw_ChangeHeight,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeSync,
                              DebugWin_Draw_ChangeSync,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeCamTime,
                              DebugWin_Draw_ChangeCamTime,
                              p_fieldmap,
                              DEBUGMENU_GYM_FLY,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_FLY );
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
    value = -10;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +10;
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
static BOOL DebugWin_ChangeCamData( int *p_param, int min, int max )
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
 *	@brief  低速から中速時間 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );
  int value = debug_windmeter_time1;

  if( DebugWin_ChangeData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_windmeter_time1 = value;
    DEBUGWIN_RefreshScreen();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  低速から中速時間 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );

  DEBUGWIN_ITEM_SetNameV( item, "ていそくじかん[%d]", debug_windmeter_time1 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  中速から高速時間 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );
  int value = debug_windmeter_time2;

  if( DebugWin_ChangeData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_windmeter_time2 = value;
    DEBUGWIN_RefreshScreen();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  中速から高速時間 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_FLY_TMP_ASSIGN_ID );

  DEBUGWIN_ITEM_SetNameV( item, "ちゅうそくじかん[%d]", debug_windmeter_time2 );
}


//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベントの継続時間  更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_blow_event_time_max;

  if( DebugWin_ChangeData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_blow_event_time_max = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  吹き飛ばしイベントの継続時間  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "ふきとばしじかん[%d]", debug_blow_event_time_max );
}

//----------------------------------------------------------------------------
/**
 *	@brief  安全イベントの継続時間  更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeSafeTime( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_safe_event_time_max;

  if( DebugWin_ChangeData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_safe_event_time_max = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  安全イベントの継続時間  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeSafeTime( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "あんぜんじかん[%d]", debug_safe_event_time_max );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれの横幅 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeWidth( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_w;

  if( DebugWin_ChangeCamData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_w = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれの横幅  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeWidth( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "CAMゆれ よこはば[%d]", debug_w );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれの縦幅 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_h;

  if( DebugWin_ChangeCamData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_h = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれの縦幅  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeHeight( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "CAMゆれ たてはば[%d]", debug_h );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ1回ゆれるシンク数 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_sync;

  if( DebugWin_ChangeCamData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_sync = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ1回ゆれるシンク数  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeSync( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "CAM１ゆれのシンク[%d]", debug_sync );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれ回数 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeCamTime( void *userWork, DEBUGWIN_ITEM *item )
{
  int value = debug_time;

  if( DebugWin_ChangeCamData( &value, 0, 600 ) )
  {
    // 更新あり
    debug_time = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラゆれの縦幅  描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeCamTime( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "CAMゆれのかいすう[%d]", debug_time );
}

#endif // GYM_FLY_DEBUG_MENU
