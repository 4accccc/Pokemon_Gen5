///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  水流による強制移動イベント
 * @file   event_current.c
 * @author obata
 * @date   2009.11.27
 */
///////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_current.h"

#include "sound/pm_sndsys.h"
#include "field_player.h"
#include "fldmmdl.h"
#include "fldmmdl_code.h"
#include "map_attr_def.h"  // for MATTR_xxxx
#include "fldeff_namipoke.h"  // for NAMIPOKE_
#include "field_player_grid.h"  // for FIELD_PLAYER_GRID
#include "fieldmap_ctrl.h"  // for FIELDMAP_GetPlayerGrid


//=============================================================================================
// ■定数
//=============================================================================================
// 処理シーケンス
enum{
  SEQ_START,  // イベント開始
  SEQ_MAIN,   // メイン処理
  SEQ_EXIT,   // イベント終了
};


//=============================================================================================
// ■水流
//=============================================================================================
typedef struct 
{
  MAPATTR_VALUE attrval;  // 起動アトリビュート
  u16          moveAcmd;  // 自機に適用する移動アニメーションコマンド( AC_xxxx )
  u16           moveDir;  // 移動方向( DIR_xxxx )

} CURRENT_DATA;

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流データ
 */
//---------------------------------------------------------------------------------------------
static const CURRENT_DATA current[] = 
{
  {MATTR_CURRENT_LEFT,   AC_WALK_L_3F, DIR_LEFT},  //右水流・海
  {MATTR_CURRENT_RIGHT,  AC_WALK_R_3F, DIR_RIGHT}, //左水流・海
  {MATTR_CURRENT_UP,     AC_WALK_U_3F, DIR_UP},    //上水流・海
  {MATTR_CURRENT_DOWN,   AC_WALK_D_3F, DIR_DOWN},  //下水流・海
};

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流データを取得する
 *
 * @param attrval アトリビュート
 *
 * @return 指定したアトリビュートに対応する水流データ
 *         対応するデータが存在しない場合, NULL
 */
//---------------------------------------------------------------------------------------------
static const CURRENT_DATA* GetCurrentData( MAPATTR_VALUE attrval )
{
  int i;
  for(i=0; i<NELEMS(current); i++ )
  {
    if( current[i].attrval == attrval )
    {
      return &current[i];
    }
  }
  return NULL;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流アトリビュートかどうかを判定する
 *
 * @param attrval アトリビュート
 *
 * @return 指定アトリビュートが移動床なら TRUE, そうでないなら FALSE
 */
//---------------------------------------------------------------------------------------------
static BOOL IsCurrent( MAPATTR_VALUE attrval )
{
  return (GetCurrentData(attrval) != NULL);
}


//=============================================================================================
// ■岩
//=============================================================================================
typedef struct
{
  MAPATTR_VALUE attrval;  // アトリビュート
  u16          jumpAcmd;  // 自機に適用するジャンプアニメーションコマンド( AC_xxxx )

} ROCK_DATA;

//---------------------------------------------------------------------------------------------
/**
 * @brief 岩データ
 */
//---------------------------------------------------------------------------------------------
static const ROCK_DATA rock[] = 
{
  {MATTR_JUMP_RIGHT, AC_JUMP_R_3G_24F},  //右方向へジャンプ移動
  {MATTR_JUMP_LEFT,  AC_JUMP_L_3G_24F},  //左方向へジャンプ移動
  {MATTR_JUMP_UP,    AC_JUMP_U_3G_24F},  //上方向へジャンプ移動
  {MATTR_JUMP_DOWN,  AC_JUMP_D_3G_24F},  //下方向へジャンプ移動
};

//---------------------------------------------------------------------------------------------
/**
 * @brief 岩データを取得する
 *
 * @param attrval アトリビュート
 *
 * @return 指定したアトリビュートに対応する岩データ
 *         対応するデータが存在しない場合, NULL
 */
//---------------------------------------------------------------------------------------------
static const ROCK_DATA* GetRockData( MAPATTR_VALUE attrval )
{
  int i;
  for(i=0; i<NELEMS(rock); i++ )
  {
    if( rock[i].attrval == attrval )
    {
      return &rock[i];
    }
  }
  return NULL;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 岩アトリビュートかどうかを判定する
 *
 * @param attrval アトリビュート
 *
 * @return 指定アトリビュートが移動床なら TRUE, そうでないなら FALSE
 */
//---------------------------------------------------------------------------------------------
static BOOL IsRock( MAPATTR_VALUE attrval )
{
  return (GetRockData(attrval) != NULL);
}


//=============================================================================================
// ■イベントワーク
//=============================================================================================
typedef struct
{
  GAMESYS_WORK*          gsys;  // ゲームシステム
  FIELDMAP_WORK*     fieldmap;  // フィールドマップ
  FIELD_PLAYER*        player;  // 自機
  MMDL*                  mmdl;  // 自機の動作モデル
  const CURRENT_DATA* current;  // 適用中の水流データ
  const ROCK_DATA*       rock;  // 適用中の岩データ

} EVENT_WORK;


//=============================================================================================
// ■非公開関数のプロトタイプ宣言
//=============================================================================================
static u16 GetMoveAcmd( const EVENT_WORK* work );
static u16 GetJumpAcmd( const EVENT_WORK* work );
static u16 GetMoveDir( const EVENT_WORK* work );
static MAPATTR_VALUE GetNowAttribute( const EVENT_WORK* work );
static MAPATTR_VALUE GetFrontAttribute( const EVENT_WORK* work );
static void MoveStart( EVENT_WORK* work );
static void SplashCheck( EVENT_WORK* work );
static void CurrentCheck( EVENT_WORK* work );
static void RockCheck( EVENT_WORK* work );
static void ExitEvent( EVENT_WORK* work );


//=============================================================================================
// ■イベント処理関数
//=============================================================================================
static GMEVENT_RESULT AutoMove( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  // イベント開始
  case SEQ_START:
    // 足元が移動床でなければ終了
    if( !IsCurrent( GetNowAttribute(work) ) )
    {
      *seq = SEQ_EXIT;
      break;
    }
    // 波ポケ後部の水飛沫エフェクトOFF
    {
      FIELD_PLAYER_GRID* player;
      FLDEFF_TASK* task;
      player = FIELDMAP_GetPlayerGrid( work->fieldmap );
      task   = FIELD_PLAYER_GRID_GetEffectTaskWork( player );
      FLDEFF_NAMIPOKE_SetRippleEffect( task, FALSE );
    }
    *seq = SEQ_MAIN;
    break;
  // メイン処理
  case SEQ_MAIN:
    // 移動終了時
    if( MMDL_CheckEndAcmd(work->mmdl) ) // if(移動終了)
    {
      // 着水チェック
      SplashCheck( work );
      // 水流チェック
      CurrentCheck( work );
      // 岩チェック
      RockCheck( work );
      // 水流がなくなったら終了
      if( (work->current == NULL) && (work->rock == NULL) )
      {
        *seq = SEQ_EXIT;
        break;
      }
      // 移動開始
      MoveStart( work );
    }
    break;
  // イベント終了
  case SEQ_EXIT:
    ExitEvent( work );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//=============================================================================================
// ■外部公開関数
//=============================================================================================

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流移動イベントを作成する
 *
 * @param parent   親となるイベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 移動床イベント
 */
//---------------------------------------------------------------------------------------------
GMEVENT* EVENT_PlayerMoveOnCurrent( GMEVENT* parent, 
                                    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, parent, AutoMove, sizeof(EVENT_WORK) );
  // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->fieldmap = fieldmap;
  work->player   = FIELDMAP_GetFieldPlayer( fieldmap );
  work->mmdl     = FIELD_PLAYER_GetMMdl( work->player );
  work->current  = NULL;
  work->rock     = NULL;
  return event;
} 

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流アトリビュート判定
 *
 * @param attrval アトリビュート
 *
 * @return 指定したアトリビュートが水流なら TRUE, そうでないなら FALSE
 */
//---------------------------------------------------------------------------------------------
BOOL CheckAttributeIsCurrent( MAPATTR_VALUE attrval )
{
  return IsCurrent( attrval );
}


//=============================================================================================
// ■非公開関数のプロトタイプ宣言
//=============================================================================================

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の水流データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の水流データ(移動アニメーションコマンド AC_xxxx)
 */
//---------------------------------------------------------------------------------------------
static u16 GetMoveAcmd( const EVENT_WORK* work )
{
  if( !work->current ){ return 0; }
  return work->current->moveAcmd;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の水流データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の水流データ(ジャンプ・アニメーションコマンド AC_xxxx)
 */
//---------------------------------------------------------------------------------------------
static u16 GetJumpAcmd( const EVENT_WORK* work )
{
  if( !work->rock ){ return 0; }
  return work->rock->jumpAcmd;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の水流データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の水流データ(移動方向 DIR_xxxx)
 */
//---------------------------------------------------------------------------------------------
static u16 GetMoveDir( const EVENT_WORK* work )
{
  if( !work->current ){ return 0; }
  return work->current->moveDir;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 足元のアトリビュートを取得する
 *
 * @param work イベントワーク
 *
 * @return 主人公の足元のアトリビュート
 */
//---------------------------------------------------------------------------------------------
static MAPATTR_VALUE GetNowAttribute( const EVENT_WORK* work )
{
  VecFx32 pos;
  MAPATTR attr;
  MAPATTR_VALUE attrval;
  FLDMAPPER* mapper;

  // 足元のアトリビュートを取得
  FIELD_PLAYER_GetPos( work->player, &pos );
  mapper  = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
  attr    = MAPATTR_GetAttribute( mapper, &pos );
  attrval = MAPATTR_GetAttrValue( attr );
  return attrval;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 前方のアトリビュートを取得する
 *
 * @param work イベントワーク
 *
 * @return 主人公の1グリッド先のアトリビュート
 */
//---------------------------------------------------------------------------------------------
static MAPATTR_VALUE GetFrontAttribute( const EVENT_WORK* work )
{
  u16 dir;
  VecFx32 pos;
  MAPATTR attr;
  MAPATTR_VALUE attrval;
  FLDMAPPER* mapper;

  // 1グリッド先のアトリビュートを取得
  dir = MMDL_GetDirDisp( work->mmdl );
  FIELD_PLAYER_GetPos( work->player, &pos );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  mapper  = FIELDMAP_GetFieldG3Dmapper( work->fieldmap );
  attr    = MAPATTR_GetAttribute( mapper, &pos );
  attrval = MAPATTR_GetAttrValue( attr );
  return attrval;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 移動を開始する
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void MoveStart( EVENT_WORK* work )
{ 
  // アニメーションコマンドをセット
  if( work->rock ){ MMDL_SetAcmd( work->mmdl, GetJumpAcmd(work) ); }  // ジャンプ
  else if( work->current ){ MMDL_SetAcmd( work->mmdl, GetMoveAcmd(work) ); }  // 移動
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 着水チェック
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void SplashCheck( EVENT_WORK* work )
{
  if( work->rock )
  {
    FLDEFF_CTRL *fectrl;
    FLDEFF_PROCID proc_id;
    NAMIPOKE_EFFECT_TYPE type;
    VecFx32 pos;

    fectrl  = FIELDMAP_GetFldEffCtrl( work->fieldmap );
    proc_id = FLDEFF_PROCID_NAMIPOKE_EFFECT;

    // エフェクト登録
    if( FLDEFF_CTRL_CheckRegistEffect( fectrl, proc_id ) == FALSE )
    {
      FLDEFF_CTRL_RegistEffect( fectrl, &proc_id, 1 );
    }
    // エフェクトを表示
    type = NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH;
    FIELD_PLAYER_GetPos( work->player, &pos );
    FLDEFF_NAMIPOKE_EFFECT_SetEffectAlone( fectrl, type, &pos );
    // SE再生
    PMSND_PlaySE( SEQ_SE_FLD_83 );

    // ジャンプ終了
    work->rock = NULL;
  }
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 水流アトリビュートチェック
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void CurrentCheck( EVENT_WORK* work )
{
  MAPATTR_VALUE attrval;

  // 足元のアトリビュートを取得
  attrval = GetNowAttribute( work );

  // 参照する水流データを更新
  work->current = GetCurrentData( attrval );
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 岩アトリビュートチェック
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void RockCheck( EVENT_WORK* work )
{
  const ROCK_DATA* rock;
  MAPATTR_VALUE attrval;

  // 足元のアトリビュートを取得
  attrval = GetNowAttribute( work );
  
  // 該当する岩データを取得
  work->rock = GetRockData( attrval );
}

//---------------------------------------------------------------------------------------------
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void ExitEvent( EVENT_WORK* work )
{
  // 波ポケ後部の水飛沫エフェクトON
  {
    FIELD_PLAYER_GRID* player;
    FLDEFF_TASK* task;
    player = FIELDMAP_GetPlayerGrid( work->fieldmap );
    task   = FIELD_PLAYER_GRID_GetEffectTaskWork( player );
    FLDEFF_NAMIPOKE_SetRippleEffect( task, TRUE );
  }
}
