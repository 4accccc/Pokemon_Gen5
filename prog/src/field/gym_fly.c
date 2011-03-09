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
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl
#include "field_task_player_trans.h" // for FIELD_TASK_TransPlayer

// zone
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02

// event
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
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

#define FAN_NUM_MAX             (1)         // ファンの数
#define FAN_SPEED_LOW           (0x1000)    // 低速のアニメ速度
#define FAN_SPEED_MIDDLE        (0x2000)    // 中速のアニメ速度
#define FAN_SPEED_HIGH          (0x4000)    // 高速のアニメ速度
#define FAN_SPEED_SUPER         (0x7000)    // 超高速のアニメ速度(ショートカット用)
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

// map_array.cdatの配列の要素
#define MAP_ARRAY_ROW           (50)        // マップの縦MAX
#define MAP_ARRAY_COL           (23)        // マップの横MAX
enum{
  MAP_ARRAY_BLOW = 0,     // 吹き飛べる
  MAP_ARRAY_STOP,         // 吹き飛べない
  MAP_ARRAY_TIMECOUNT_START,  // 時間計測開始
  MAP_ARRAY_SHORTCUT,     // 帰り道ショートカット
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
  MMDL            *mmdl;        // 自機の動作モデル

  u16             time;         // 時間計測用
  u8              stay_walk_cnt;// 足踏みカウント
  BOOL            is_wall;      // 後ろがすぐ壁か？
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

// 吹き飛びの移動距離算出
static const u8 BLOW_EVENT_GetMoveDistance( FIELDMAP_WORK *p_fieldmap, s16 gx, s16 gz );

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

//-------------------------------------
///	セーブ情報
//=====================================
// GYM_FLY_SV_WORK
static void SAVE_SetTimeFlg( GYM_FLY_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetTimeFlg( const GYM_FLY_SV_WORK *cp_wk );

#ifdef GYM_FLY_DEBUG_MENU
//-------------------------------------
/// デバッグメニュー	
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int debug_windmeter_time1 = WINDMETER_MIDDLE_TIME;   // 低速から中速に変わるまでの時間
static int debug_windmeter_time2 = WINDMETER_HIGH_TIME;   // 中速から高速に変わるまでの時間
static u16 debug_blow_event_time_max = BLOW_EVENT_TIME;   // 吹き飛ばしイベントの最低経過時間
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
  p_wk->time      = 0;
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
  p_wk->fan_wk.anm_speed = FAN_SPEED_LOW;
  p_wk->fan_wk.anm_speed_max = FAN_SPEED_LOW;
  p_wk->fan_wk.parent_exp_obj = p_wk->exp_obj;

  // 風力計
  p_wk->windmeter_wk.anm_speed = WINDMETER_SPEED_LOW;
  p_wk->windmeter_wk.anm_speed_max = WINDMETER_SPEED_LOW;
  p_wk->windmeter_wk.parent_exp_obj = p_wk->exp_obj;
  
  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 環境音
  FSND_PlayEnvSE( p_wk->p_sound, GYM_FLY_SE_WIND );

  // セーブから取得
  p_wk->time_start = SAVE_GetTimeFlg( p_wk->p_sv_wk );

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

  // 環境音停止
  FSND_StopEnvSE( p_wk->p_sound, GYM_FLY_SE_WIND );

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
        p_wk->alpha[i] -= 2;
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
          if( i == 1 ){ p_wk->alpha_reset = FALSE; }
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

//=============================================================================
/**
 *      static関数
 */
//=============================================================================

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
    ICHI_Printf("風力計状態：高速\n");
    break;
  case WIND_STATE_SUPER:
    // 風エフェクト2,3(マップ)エフェクト
    {
      count   = 2;
      obj_idx = EFFECT_WIND_2;
    }
    break;;
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

  // 環境音のステータス変更
  {
    SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_WIND );
    int pitch = p_wk->state * 192;   // 64は半音
    int tempo = p_wk->state;
    u32 volume= 64 + p_wk->state * 20;
    PMSND_SetStatusSE_byPlayerID( player_ID, tempo, pitch, PMSND_NOEFFECT);
    PMSND_PlayerSetVolume( player_ID, volume );
  }
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
  work->time        = 0;
  work->stay_walk_cnt = 0;

  // 自機の1グリッド下が壁かチェック
  if( map_array[gz+1][gx] == MAP_ARRAY_STOP ){ work->is_wall = TRUE; }
  else                                       { work->is_wall = FALSE; }

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
        move_distance = BLOW_EVENT_GetMoveDistance( p_wk->p_fieldmap, gx, gz );
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

      // 風エフェクト(自機)エフェクト
      {
        // 位置
        GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI );
        MMDL_GetVectorPos( p_wk->mmdl, &status->trans );

        // 表示
        FLD_EXP_OBJ_SetVanish( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, FALSE );
        // アニメ
        {
          EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0 );
          FLD_EXP_OBJ_SetObjAnmFrm( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0, 0 );
          FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始
        }
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
        u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
        dir++;
        dir %= 4;
        MMDL_SetAcmd( p_wk->mmdl, (AC_DIR_U + dir) );
      }

      // 風エフェクト(自機)エフェクト
      {
        // 位置
        GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI );
        MMDL_GetVectorPos( p_wk->mmdl, &status->trans );
      }

      // SE
      {
        SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
        if( PMSND_CheckPlaySE_byPlayerID( player_ID ) == FALSE )
        {
          // SE ヒュー
          PMSND_PlaySE( GYM_FLY_SE_BLOW );
        }
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

          if( p_wk->is_wall == FALSE )
          {
            // SE びたーん
            PMSND_PlaySE( GYM_FLY_SE_WALL_HIT );
          }

          // 風エフェクト(自機)エフェクトOFF
          FLD_EXP_OBJ_SetVanish( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, TRUE );
          FLD_EXP_OBJ_ChgAnmStopFlg( FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0), TRUE );

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

      *seq = SEQ_MAIN;
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
        const u16 blow_event_time_max = debug_blow_event_time_max;
#else
        const u16 blow_event_time_max = BLOW_EVENT_TIME;
#endif

        if( p_wk->time >= blow_event_time_max )
        {
          ret = TRUE;
        }
      }

      // 次シーケンス移行チェック
      if( ret == TRUE )
      {
        *seq = SEQ_EXIT;
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
      WIND_ChangeState( p_wk->p_fieldmap, WIND_STATE_SUPER );
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

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        MMDL_OnStatusBit( p_wk->mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      // 風エフェクト(自機)エフェクト
      {
        // 位置
        GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI );
        MMDL_GetVectorPos( p_wk->mmdl, &status->trans );

        // 表示
        FLD_EXP_OBJ_SetVanish( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, FALSE );
        // アニメ
        {
          EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0 );
          FLD_EXP_OBJ_SetObjAnmFrm( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0, 0 );
          FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE ); //アニメ開始
        }
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
        u16 dir = FIELD_PLAYER_GetDir( p_wk->player );
        dir++;
        dir %= 4;
        MMDL_SetAcmd( p_wk->mmdl, (AC_DIR_U + dir) );
      }

      // 風エフェクト(自機)エフェクト
      {
        // 位置
        GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI );
        MMDL_GetVectorPos( p_wk->mmdl, &status->trans );
      }

      // SE
      {
        SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
        if( PMSND_CheckPlaySE_byPlayerID( player_ID ) == FALSE )
        {
          // SE ヒュー
          PMSND_PlaySE( GYM_FLY_SE_BLOW );
        }
      }

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
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {

          // SE 停止
          {
            SEPLAYER_ID player_ID = PMSND_GetSE_DefaultPlayerID( GYM_FLY_SE_BLOW );
            PMSND_StopSE_byPlayerID( player_ID );
          }

          // 風エフェクト(自機)エフェクトOFF
          FLD_EXP_OBJ_SetVanish( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, TRUE );
          FLD_EXP_OBJ_ChgAnmStopFlg( FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_FLY_UNIT_IDX, EFFECT_WIND_JIKI, 0), TRUE );

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
      GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
      GMEVENT* new_event;
      VecFx32 pos;
      pos.x = FX32_CONST( 78 * FIELD_CONST_GRID_SIZE );
      pos.y = FX32_CONST( 0 * FIELD_CONST_GRID_SIZE );
      pos.z = FX32_CONST( 269 * FIELD_CONST_GRID_SIZE );
      new_event = EVENT_ChangeMapPos( p_gsys, p_wk->p_fieldmap, ZONE_ID_C06, &pos, DIR_DOWN, FALSE );
      GMEVENT_ChangeEvent( event, new_event );
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
 *	@brief  吹き飛ばしの距離を計算
 *
 *	@param	FIELDMAP_WORK *p_fieldmap
 *	@param	s16 gx    自機グリッドX座標
 *	@param	s16 gz    自機グリッドZ座標
 *
 *	@return u8  移動距離(グリッド)
 */
//-----------------------------------------------------------------------------
static const u8 BLOW_EVENT_GetMoveDistance( FIELDMAP_WORK *p_fieldmap, s16 gx, s16 gz )
{
  u8 move_distance = 0;

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
  for( i=0; i<FAN_NUM_MAX; i++ )
  {
    // 座標セット
    VecFx32 pos[ FAN_NUM_MAX ] = {
      { FAN1_GX,  FAN1_GY,  FAN1_GZ },
      //{ FAN2_GX,  FAN2_GY,  FAN2_GZ },
      //{ FAN3_GX,  FAN3_GY,  FAN3_GZ },
    };

    u16   obj_idx = OBJ_FAN1 + i;    // OBJインデックス
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
  ///	風エフェクト(自機)の設定
  //
  {
    u16   obj_idx = EFFECT_WIND_JIKI;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, TRUE );  // 非表示
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, TRUE );
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, 0, 0);
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );   // ループ再生設定
    }
  }

  //=====================================
  ///	風エフェクト(マップ)の設定
  //
  for( i=EFFECT_WIND_1; i<=EFFECT_WIND_3; i++ )
  {
    u16   obj_idx = i;    // OBJインデックス
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx );

    status->trans.x = FX32_CONST( 11 * FIELD_CONST_GRID_SIZE );
    status->trans.y = FX32_CONST( 4 * FIELD_CONST_GRID_SIZE );
    status->trans.z = FX32_CONST( 24 * FIELD_CONST_GRID_SIZE );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_FLY_UNIT_IDX, obj_idx, FALSE );

    // 低速の風のみ表示
    if( i == EFFECT_WIND_1 )
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
    for( i=0; i<FAN_NUM_MAX; i++ )
    {
      u16   obj_idx = OBJ_FAN1 + i;    // OBJインデックス
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
// 風力変更の時間
static void DebugWin_Update_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeTime1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeTime2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeBlowTime( void *userWork, DEBUGWIN_ITEM *item );

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

#endif // GYM_FLY_DEBUG_MENU
