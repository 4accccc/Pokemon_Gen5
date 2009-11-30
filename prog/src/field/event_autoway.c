///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  移動床による強制移動イベント
 * @file   event_autoway.c
 * @author obata
 * @date   2009.11.27
 *
 * @todo SEをあてる
 */
///////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_autoway.h"

#include "field_player.h"
#include "fldmmdl.h"
#include "fldmmdl_code.h"
#include "map_attr_def.h"  // for MATTR_xxxx


//=============================================================================================
// ■定数
//=============================================================================================
// 処理シーケンス
enum
{
  SEQ_START,  // イベント開始
  SEQ_MAIN,   // メイン処理
  SEQ_EXIT,   // イベント終了
};


//=============================================================================================
// ■移動床
//=============================================================================================
typedef struct 
{
  MAPATTR_VALUE attrval;  // 起動アトリビュート
  u16              acmd;  // 自機に適用するアニメーションコマンド( AC_xxxx )
  u16           moveDir;  // 移動方向( DIR_xxxx )
  u16           rotWait;  // 回転時の向き変更間隔

} AUTOWAY;

//---------------------------------------------------------------------------------------------
/**
 * @brief 移動床データ
 */
//---------------------------------------------------------------------------------------------
static const AUTOWAY autoway[] = 
{
  {MATTR_MOVEF_LEFT,  AC_WALK_L_8F, DIR_LEFT,  2},
  {MATTR_MOVEF_RIGHT, AC_WALK_R_8F, DIR_RIGHT, 2},
  {MATTR_MOVEF_UP,    AC_WALK_U_8F, DIR_UP,    2},
  {MATTR_MOVEF_DOWN,  AC_WALK_D_8F, DIR_DOWN,  2},
};

//---------------------------------------------------------------------------------------------
/**
 * @brief 移動データを取得する
 *
 * @param attrval アトリビュート
 *
 * @return 指定したアトリビュートに対応する移動データ
 *         対応するデータが存在しない場合, NULL
 */
//---------------------------------------------------------------------------------------------
static const AUTOWAY* GetAutoWay( MAPATTR_VALUE attrval )
{
  int i;
  for(i=0; i<NELEMS(autoway); i++ )
  {
    if( autoway[i].attrval == attrval )
    {
      return &autoway[i];
    }
  }
  return NULL;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 移動床アトリビュートかどうかを判定する
 *
 * @param attrval アトリビュート
 *
 * @return 指定アトリビュートが移動床なら TRUE, そうでないなら FALSE
 */
//---------------------------------------------------------------------------------------------
static BOOL IsAutoWay( MAPATTR_VALUE attrval )
{
  return (GetAutoWay(attrval) != NULL);
}


//=============================================================================================
// ■イベントワーク
//=============================================================================================
typedef struct
{
  GAMESYS_WORK*      gsys;  // ゲームシステム
  FIELDMAP_WORK* fieldmap;  // フィールドマップ
  FIELD_PLAYER*    player;  // 自機
  MMDL*              mmdl;  // 自機の動作モデル
  const AUTOWAY*  autoWay;  // 適用中の移動床データ
  u16              rotDir;  // 自機の向き
  u32            rotCount;  // 回転カウンタ

} EVENT_WORK;


//=============================================================================================
// ■非公開関数のプロトタイプ宣言
//=============================================================================================
static u16 GetAcmd( const EVENT_WORK* work );
static u16 GetMoveDir( const EVENT_WORK* work );
static u16 GetRotWait( const EVENT_WORK* work );
static MAPATTR_VALUE GetAttribute( const EVENT_WORK* work );
static void UpdateRotDir( EVENT_WORK* work );
static void MoveStart( EVENT_WORK* work );
static void AttributeCheck( EVENT_WORK* work );
static BOOL HitCheck( EVENT_WORK* work );
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
    if( !IsAutoWay( GetAttribute(work) ) )
    {
      *seq = SEQ_EXIT;
      break;
    }
    *seq = SEQ_MAIN;
    break;
  // メイン処理
  case SEQ_MAIN:
    // 回転
    UpdateRotDir( work );
    // 移動終了時
    if( MMDL_CheckEndAcmd(work->mmdl) ) // if(移動終了)
    {
      // アトリビュートチェック
      AttributeCheck( work );
      // 衝突チェック
      if( HitCheck(work) )  // if(衝突)
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
 * @brief 移動床イベントを作成する
 *
 * @param parent   親となるイベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 移動床イベント
 */
//---------------------------------------------------------------------------------------------
GMEVENT* EVENT_PlayerMoveOnAutoWay( GMEVENT* parent, 
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
  work->autoWay  = 0;
  work->rotDir   = MMDL_GetDirDisp( work->mmdl );
  work->rotCount = 0;
  return event;
} 


//=============================================================================================
// ■非公開関数のプロトタイプ宣言
//=============================================================================================

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の移動データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の移動データ(アニメーションコマンド AC_xxxx)
 */
//---------------------------------------------------------------------------------------------
static u16 GetAcmd( const EVENT_WORK* work )
{
  if( !work->autoWay ){ return 0; }
  return work->autoWay->acmd;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の移動データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の移動データ(移動方向 DIR_xxxx)
 */
//---------------------------------------------------------------------------------------------
static u16 GetMoveDir( const EVENT_WORK* work )
{
  if( !work->autoWay ){ return 0; }
  return work->autoWay->moveDir;
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 現在参照中の移動データを取得する
 *
 * @param work イベントワーク
 *
 * @return 現在参照中の移動データ(回転待ち時間)
 */
//---------------------------------------------------------------------------------------------
static u16 GetRotWait( const EVENT_WORK* work )
{
  if( !work->autoWay ){ return 0; }
  return work->autoWay->rotWait;
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
static MAPATTR_VALUE GetAttribute( const EVENT_WORK* work )
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
 * @brief 自機の向きを更新する
 *
 * @param work    イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void UpdateRotDir( EVENT_WORK* work )
{ 
  if( ++work->rotCount % GetRotWait(work) == 0 )
  {
    // 向きを変更
    work->rotDir = (work->rotDir + 1) % DIR_MAX4;
    // 動作モデルに反映
    MMDL_OffStatusBit( work->mmdl, MMDL_STABIT_PAUSE_DIR );
    MMDL_SetDirDisp( work->mmdl, work->rotDir );
    MMDL_OnStatusBit( work->mmdl, MMDL_STABIT_PAUSE_DIR );
  }
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
  MMDL_SetAcmd( work->mmdl, GetAcmd(work) ); 

  // 移動アニメーションによる向きの変更を一時停止
  MMDL_OnStatusBit( work->mmdl, MMDL_STABIT_PAUSE_DIR );
}

//---------------------------------------------------------------------------------------------
/**
 * @brief アトリビュートチェック
 *
 * @param work イベントワーク
 */
//---------------------------------------------------------------------------------------------
static void AttributeCheck( EVENT_WORK* work )
{
  const AUTOWAY* new_way;
  MAPATTR_VALUE attrval;

  // 足元のアトリビュートを取得
  attrval = GetAttribute( work );

  // 参照する移動データを更新
  new_way = GetAutoWay( attrval );
  if( new_way ){ work->autoWay = new_way; }
}

//---------------------------------------------------------------------------------------------
/**
 * @brief 衝突チェック
 *
 * @param work イベントワーク
 *
 * @return 衝突したら TRUE, そうでなければ FALSE
 */
//---------------------------------------------------------------------------------------------
static BOOL HitCheck( EVENT_WORK* work )
{
  // 進行方向のグリッドをチェック
  u32 hitbit = MMDL_HitCheckMoveDir( work->mmdl, GetMoveDir(work) );
  return (hitbit != MMDL_MOVEHITBIT_NON);
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
  // 変更したステータスビットを元に戻す
  MMDL_OffStatusBit( work->mmdl, MMDL_STABIT_PAUSE_DIR );

  // 進行方向を向く
  if( work->autoWay )
  {
    MMDL_SetDirDisp( work->mmdl, GetMoveDir(work) );
  }
}
