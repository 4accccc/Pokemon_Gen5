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

#define WINDMETER_NUM_MAX   (17)    // 風力計の数

//-------------------------------------
///	風力計状態
//=====================================
typedef enum {
  WINDMETER_STATE_LOW,     // 低速
  WINDMETER_STATE_MIDDLE,  // 中速
  WINDMETER_STATE_HIGH,    // 高速
}WINDMETER_STATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  HEAPID  heapID;     // ヒープID

  u16     time;       // 時間計測用
  u8      state;      // 風速状態   WINDMETER_STATE_XXX
  fx32    anm_speed;  // アニメーション再生の速度

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ

}GYM_FLY_TMP;

//-------------------------------------
///	吹き飛ばしイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK    *gsys;        // ゲームシステム
  FIELDMAP_WORK   *p_fieldmap;   // フィールドマップ
  FIELD_PLAYER    *player;      // 自機
  MMDL            *mmdl;        // 自機の動作モデル
} BLOW_EVENT_WORK;


#include "gym_fly.cdat"

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================
// WINDMETER 風力計
static WINDMETER_STATE WINDMETER_GetState( const u16 c_time ); 
static void WINDMETER_ChangeState( FIELDMAP_WORK *p_fieldmap, const WINDMETER_STATE state );
static GMEVENT* WINDMETER_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys );
static GMEVENT_RESULT BlowEvent( GMEVENT* event, int* seq, void* wk );

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_FLY_TMP *p_wk );
static void EXPOBJ_ObjDell( GYM_FLY_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_FLY_TMP *p_wk );

#ifdef GYM_FLY_DEBUG_MENU
//-------------------------------------
/// デバッグメニュー	
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int debug_windmeter_time1 = WINDMETER_MIDDLE_TIME;   // 低速から中速に変わるまでの時間
static int debug_windmeter_time2 = WINDMETER_HIGH_TIME;   // 中速から高速に変わるまでの時間
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
  //汎用ワーク確保
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_FLY_TMP));

  p_wk->heapID    = HEAPID_FIELDMAP;
  p_wk->time      = 0;
  p_wk->state     = WINDMETER_STATE_LOW;
  p_wk->anm_speed = FX32_ONE;
  p_wk->exp_obj   = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

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
  GMEVENT *event;

  if( p_wk == NULL ){ return NULL; }

  // 時間経過
  p_wk->time++;
  ICHI_Printf("*");
  
  {
    // 時間監視
    WINDMETER_STATE tmp_state = WINDMETER_GetState( p_wk->time );

    // 風力計の状態が変更したら
    if( p_wk->state != tmp_state )
    {
      WINDMETER_ChangeState( p_fieldmap, tmp_state );

      // 高速の時のみ吹き飛ばしイベント作成
      if( p_wk->state == WINDMETER_STATE_HIGH )
      {
        event = WINDMETER_BLOW_CreateEvent( NULL, p_gamesys );
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
 *	@return その時間の風力計状態 WINDMETER_STATE_XXX
 */
//-----------------------------------------------------------------------------
static WINDMETER_STATE WINDMETER_GetState( const u16 c_time )
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
    return WINDMETER_STATE_HIGH;
  }
  else if( c_time >= windmeter_time1 )
  {
    return WINDMETER_STATE_MIDDLE;
  }
  else
  {
    return WINDMETER_STATE_LOW;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  風力計の状態変更
 *
 *	@param	FIELDMAP_WORK *p_fieldmap    フィールドワークポインタ
 *	@param	WINDMETER_STATE state      変更後の状態
 */
//-----------------------------------------------------------------------------
static void WINDMETER_ChangeState( FIELDMAP_WORK *p_fieldmap, const WINDMETER_STATE state )
{
  GYM_FLY_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_FLY_TMP_ASSIGN_ID);

  p_wk->state = state;
  
  // 状態別初期化
  switch( p_wk->state ){
  case WINDMETER_STATE_LOW:
    p_wk->anm_speed = FX32_ONE;
    ICHI_Printf("風力計状態：低速\n");
    break;
  case WINDMETER_STATE_MIDDLE:
    p_wk->anm_speed = 0x2000;
    ICHI_Printf("風力計状態：中速\n");
    break;
  case WINDMETER_STATE_HIGH:
    p_wk->time = 0;
    p_wk->anm_speed = 0x4000;
    ICHI_Printf("風力計状態：高速\n");
    break;
  }

  // アニメスピード変更
  FLD_EXP_OBJ_SetUnitAddFrame( p_wk->exp_obj, GYM_FLY_UNIT_IDX, p_wk->anm_speed );
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
static GMEVENT* WINDMETER_BLOW_CreateEvent( GMEVENT* parent, GAMESYS_WORK* gsys )
{
  GMEVENT*          event;
  BLOW_EVENT_WORK*  work;
  FIELD_PLAYER*     player;
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  player = FIELDMAP_GetFieldPlayer( p_fieldmap );

  // 生成
  event = GMEVENT_Create( gsys, parent, BlowEvent, sizeof(BLOW_EVENT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys              = gsys;
  work->p_fieldmap          = p_fieldmap;
  work->player            = player;
  work->mmdl              = FIELD_PLAYER_GetMMdl( player );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief
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

  // 処理シーケンス
  enum{
    SEQ_WAIT,   // グリッド調度待ち
    SEQ_START,   // イベント開始 
    SEQ_MAIN,   // メイン処理
    SEQ_EXIT,   // イベント終了
  };

  switch( *seq )
  {
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
      // 足元チェック

      // 移動タスク登録
      {
        FIELD_TASK* move;
        FIELD_TASK_MAN* taskMan;
        VecFx32 down_pos;
        {
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
          FIELD_PLAYER_GetPos( player, &down_pos );
          down_pos.z += GRID_TO_FX32(5);
        }
        {
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
          s16 gx, gy, gz;
          FIELD_PLAYER_GetGridPos( player, &gx, &gy, &gz );
          ICHI_Printf("自機Grid座標 %d:%d:%d\n", gx, gy, gz);
        }
        move = FIELD_TASK_TransPlayer( p_wk->p_fieldmap, 5*4, &down_pos );
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, move, NULL );
      }

      // 高さを変更したくないので
      // 高さ取得禁止ステータスビットをONにする
      {
        FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
        FIELD_PLAYER_CORE* core = FIELD_PLAYER_GetCoreWk( player );
        MMDL* mmdl = FIELD_PLAYER_CORE_GetMMdl( core );
        MMDL_OnStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
      }

      *seq = SEQ_MAIN;
    }
    break;

  // メイン処理
  case SEQ_MAIN:
    {
      // 自機の向きを変える
      {
        int key = GFL_UI_KEY_GetCont();
        if( key & PAD_KEY_UP )   { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_U_4F ); }
        else if( key & PAD_KEY_DOWN ) { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_D_4F ); }
        else if( key & PAD_KEY_LEFT ) { MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_L_4F ); }
        else if( key & PAD_KEY_RIGHT ){ MMDL_SetAcmd( p_wk->mmdl, AC_STAY_WALK_R_4F ); }
        else
        {
          // キー入力がない場合は主人公の向きにその場歩き
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
          u16 dir = FIELD_PLAYER_GetDir( player );
          dir %= 4;
          MMDL_SetAcmd( p_wk->mmdl, (AC_STAY_WALK_U_4F + dir) );
        }
      }

      // タスク終了チェック
      {
        FIELD_TASK_MAN* taskMan;
        taskMan = FIELDMAP_GetTaskManager( p_wk->p_fieldmap );
        if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) {
          *seq = SEQ_EXIT;
        }
      }
    }
    break;

  // イベント終了
  case SEQ_EXIT:
    // 高さ取得禁止ステータスビットをOFFに戻す
    {
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
      FIELD_PLAYER_CORE* core = FIELD_PLAYER_GetCoreWk( player );
      MMDL* mmdl = FIELD_PLAYER_CORE_GetMMdl( core );
      MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_OFF );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
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
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
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


#endif // GYM_FLY_DEBUG_MENU
