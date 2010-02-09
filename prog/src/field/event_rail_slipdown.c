//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_rail_slipdown.c
 *	@brief  チャンピオンロード専用　レールマップ　ずり落ちイベント
 *	@author	tomoya takahashi
 *	@date		2009.10.16
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "arc/fieldmap/zone_id.h"

#include "field/field_const.h"

#include "event_rail_slipdown.h"

#include "field_nogrid_mapper.h"

#include "fldeff_kemuri.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
enum
{
  EV_RAILSLIPDOWN_SARCH_LOCATION, // ロケーションＳＥＡＲＣＨ
  EV_RAILSLIPDOWN_JUMP_START,     // ジャンプ
  EV_RAILSLIPDOWN_JUMP_WAIT,
  EV_RAILSLIPDOWN_SLIPDOWN_START, // ずり落ち
  EV_RAILSLIPDOWN_SLIPDOWN_WAIT,
  EV_RAILSLIPDOWN_LANDING_START,  // 着地
  EV_RAILSLIPDOWN_LANDING_WAIT,
  EV_RAILSLIPDOWN_END,            // 終了
};


//-------------------------------------
///	ずり落ち方向
//  Yが1かわるごとのＸＺ方向の移動距離
//=====================================
#define RAILSLIPDOWN_MOVE_XZ  ( FX32_CONST(1.1250f) )

//-------------------------------------
///	1フレームでのずり落ち距離
//=====================================
#define RAILSLIPDOWN_ONE_DIST ( FX32_CONST(8) )


//-------------------------------------
///	ずり落ちSE再生
//=====================================
#define RAILSLIPDOWN_SE ( SEQ_SE_FLD_92 )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ずり落ち　イベントワーク
//=====================================
typedef struct 
{
  FLDNOGRID_MAPPER* p_mapper;
  FIELD_RAIL_WORK* p_player_rail;
  FIELD_PLAYER* p_player;
  FLDEFF_CTRL*  p_effctrl;

  VecFx32 start_pos;
  VecFx32 end_pos;
  VecFx16 way;
  s32 count;
  u32 count_max;

  RAIL_LOCATION target_location;
  
} EVENT_RAIL_SLIPDOWN_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// ずり落ち
static GMEVENT_RESULT EVENT_RailSlipDownMain(GMEVENT * p_event, int *  p_seq, void * p_work);



//----------------------------------------------------------------------------
/**
 *	@brief  チャンピオンマップ　レールマップ　ずり落ち処理
 *
 *	@param	gsys        ゲームシステム
 *	@param	fieldmap    フィールドマップ
 *
 *	@return 設定したイベント
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_RailSlipDown(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  GMEVENT *event;
  EVENT_RAIL_SLIPDOWN_WORK* p_slipdown;

  event = GMEVENT_Create(
    gsys, NULL, EVENT_RailSlipDownMain, sizeof(EVENT_RAIL_SLIPDOWN_WORK));

  // チャンピオンロードかチェック
  GF_ASSERT( FIELDMAP_GetZoneID( fieldmap ) == ZONE_ID_D09 );

  // レールマップかチェック
  GF_ASSERT( FIELDMAP_GetMapControlType( fieldmap ) == FLDMAP_CTRLTYPE_NOGRID );

  // ワークの初期化
  p_slipdown = GMEVENT_GetEventWork(event);

  p_slipdown->p_mapper          = FIELDMAP_GetFldNoGridMapper( fieldmap );
  p_slipdown->p_player          = FIELDMAP_GetFieldPlayer( fieldmap );
  p_slipdown->p_player_rail     = FIELD_PLAYER_GetNoGridRailWork( p_slipdown->p_player );
  p_slipdown->p_effctrl         = FIELDMAP_GetFldEffCtrl( fieldmap );

  // 自機の強制停止
  FIELD_PLAYER_ForceStop( p_slipdown->p_player );
  
  return event;
}









//-----------------------------------------------------------------------------
/**
 *      ｐｒｉｖａｔｅ関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  スリップイベント
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_RailSlipDownMain(GMEVENT * p_event, int *  p_seq, void * p_work)
{
  EVENT_RAIL_SLIPDOWN_WORK* p_slipdown;

  //ワークの初期化 
  p_slipdown = GMEVENT_GetEventWork(p_event);
  
  switch( *p_seq )
  {
  case EV_RAILSLIPDOWN_SARCH_LOCATION: // ロケーションＳＥＡＲＣＨ
    
    // サーチ
    // 今の位置から、角度？？で、移動する
    {
      VecFx16 front_way;
      VecFx32 now_pos;
      fx32 dist_y;
      const FIELD_RAIL_MAN* cp_railMan;
      BOOL result;

      MMDL_Rail_GetFrontWay( FIELD_PLAYER_GetMMdl(p_slipdown->p_player), &front_way );

      TOMOYA_Printf( "front_way x[0x%x] y[0x%x] z[0x%x]\n", front_way.x, front_way.y, front_way.z );
      // 平面の方向に変更
      front_way.y = 0;
      VEC_Fx16Normalize( &front_way, &front_way );


      FIELD_RAIL_WORK_GetPos( p_slipdown->p_player_rail, &now_pos );

//      TOMOYA_Printf( "nowpos x[%d] y[%d] z[%d]\n", FX_Whole(now_pos.x), FX_Whole(now_pos.y), FX_Whole(now_pos.z) );

      // その方向に,１グリッドサイズ移動し、
      // 下る
      p_slipdown->start_pos.x =  now_pos.x + FX_Mul( front_way.x, FIELD_CONST_GRID_FX32_SIZE );
      p_slipdown->start_pos.y =  now_pos.y + FX_Mul( front_way.y, FIELD_CONST_GRID_FX32_SIZE );
      p_slipdown->start_pos.z =  now_pos.z + FX_Mul( front_way.z, FIELD_CONST_GRID_FX32_SIZE );

/*      TOMOYA_Printf( "startpos x[%d] y[%d] z[%d]\n", FX_Whole(p_slipdown->start_pos.x), 
        FX_Whole(p_slipdown->start_pos.y), FX_Whole(p_slipdown->start_pos.z) );
//*/

      // 下り先
      dist_y = p_slipdown->start_pos.y + FX32_CONST(10.0f);
      p_slipdown->end_pos.y = -FX32_CONST(10.0f);
      p_slipdown->end_pos.x = p_slipdown->start_pos.x + FX_Mul( front_way.x, FX_Mul( RAILSLIPDOWN_MOVE_XZ, dist_y ) );
      p_slipdown->end_pos.z = p_slipdown->start_pos.z + FX_Mul( front_way.z, FX_Mul( RAILSLIPDOWN_MOVE_XZ, dist_y ) );

      /*
      TOMOYA_Printf( "endpos x[%d] y[%d] z[%d]\n", FX_Whole(p_slipdown->end_pos.x), 
          FX_Whole(p_slipdown->end_pos.y), FX_Whole(p_slipdown->end_pos.z) );
          //*/

      // くだり先をＳＥＡＲＣＨ
      cp_railMan = FLDNOGRID_MAPPER_GetRailMan( p_slipdown->p_mapper );
      result = FIELD_RAIL_MAN_Calc3DVecRailLocation( cp_railMan, 
          &p_slipdown->start_pos, &p_slipdown->end_pos,
          &p_slipdown->target_location, &p_slipdown->end_pos );

      // 下がないのであれば、何もしない
      if( !result )
      {
        OS_TPrintf( "slipdown 下のレールが見つかりません。\n" );
        //GF_ASSERT( result );
        return GMEVENT_RES_FINISH;
      }

     
      // 移動カウントを求める
      // 方向なども求める
      {
        VecFx32 way;
        fx32 slipdown_dist;
          
        VEC_Subtract( &p_slipdown->end_pos, &p_slipdown->start_pos, &way );
        slipdown_dist = VEC_Mag( &way );
        VEC_Normalize( &way, &way );
        VEC_Fx16Set( &p_slipdown->way, way.x, way.y, way.z ); 

        slipdown_dist = FX_Div( slipdown_dist, RAILSLIPDOWN_ONE_DIST ); 
        if( FX_Modf( slipdown_dist, &slipdown_dist ) == 0 )
        {
          p_slipdown->count_max = slipdown_dist>>FX32_SHIFT;
        }
        else
        {
          // 小数点切り上げ
          p_slipdown->count_max = slipdown_dist>>FX32_SHIFT;
          p_slipdown->count_max ++;
        }
      }

      p_slipdown->count = 0;

      PMSND_PlaySE( RAILSLIPDOWN_SE );

      (*p_seq) = EV_RAILSLIPDOWN_SLIPDOWN_START;
    }
    
    break;

  case EV_RAILSLIPDOWN_JUMP_START:     // ジャンプ
  case EV_RAILSLIPDOWN_JUMP_WAIT:
    break;

  case EV_RAILSLIPDOWN_SLIPDOWN_START: // ずり落ち
      (*p_seq) = EV_RAILSLIPDOWN_SLIPDOWN_WAIT;
  case EV_RAILSLIPDOWN_SLIPDOWN_WAIT:
    {
      VecFx32 pos;
      fx32 dist;

      p_slipdown->count ++;
      if( p_slipdown->count >= p_slipdown->count_max )
      {
        VEC_Set( &pos, p_slipdown->end_pos.x, p_slipdown->end_pos.y, p_slipdown->end_pos.z );

        // 完了
        (*p_seq) = EV_RAILSLIPDOWN_END;
      }
      else
      {
        dist = FX_Mul( FX32_CONST(p_slipdown->count), RAILSLIPDOWN_ONE_DIST );

        VEC_Set( &pos, 
            p_slipdown->start_pos.x + FX_Mul( p_slipdown->way.x, dist ),
            p_slipdown->start_pos.y + FX_Mul( p_slipdown->way.y, dist ),
            p_slipdown->start_pos.z + FX_Mul( p_slipdown->way.z, dist ) );


        // 土煙を出す
        if( (p_slipdown->count % 2) == 0 )
        {
          FLDEFF_KEMURI_SetMMdl( FIELD_PLAYER_GetMMdl( p_slipdown->p_player ), p_slipdown->p_effctrl );
        }
      }

      // 座標設定
      FIELD_PLAYER_SetPos( p_slipdown->p_player, &pos );    

    }
    break;
    
  case EV_RAILSLIPDOWN_LANDING_START:  // 着地
  case EV_RAILSLIPDOWN_LANDING_WAIT:
    break;
    
  case EV_RAILSLIPDOWN_END:            // 終了

    // プレイヤーID
    PMSND_StopSE_byPlayerID( PMSND_GetSE_DefaultPlayerID( RAILSLIPDOWN_SE ) );
    
    {
      VecFx32 pos;

      TOMOYA_Printf( "rail index %d\n", p_slipdown->target_location.rail_index );
      TOMOYA_Printf( "line_grid %d\n", p_slipdown->target_location.line_grid );
      TOMOYA_Printf( "width_grid %d\n", p_slipdown->target_location.width_grid );
      
      // レールロケーションを設定
      FIELD_PLAYER_SetNoGridLocation( p_slipdown->p_player, &p_slipdown->target_location );
      FIELD_PLAYER_GetNoGridPos( p_slipdown->p_player, &pos );
      // 座標をプレイヤーに設定
      FIELD_PLAYER_SetPos( p_slipdown->p_player, &pos );    
    }
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

