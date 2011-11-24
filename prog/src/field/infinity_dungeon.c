//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon.c
 *	@brief  無限ダンジョン
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fldmmdl.h"
#include "infinity_dungeon.h"
#include "infinity_dungeon_local_def.h"
#include "system/gfl_use.h"
#include "scrcmd_work.h"
#include "event_mapchange.h"
#include "event_field_fade.h"
#include "field/field_const.h"
#include "savedata/infinity_dungeon_save.h"
#include "../../../resource/fldmapdata/build_model/output/buildmodel_indoor.naix"
#include "../../../resource/fldmapdata/script/infinity_dungeon_scr_def.h"
#include "../../../resource/fldmapdata/script/trainer_def.h"
#include "field/infinity_dungeon_def.h"
#include "fld_btl_inst_tool.h"
#include "event_battle.h"
#include "system/main.h"  //HEAPID_FIELDMAP
#include "message.naix"
#include "tr_tool/ex_trainer_def.h"
#include "msg/script/msg_infinity_dungeon_scr.h"
#include "infinity_dungeon_gimmick.h"
#include "gmk_tmp_wk.h"
#include "../../../resource/fldmapdata/eventdata/define/total_header.h"
#include "infinity_dungeon_creator.h"
#include "infinity_dungeon_field.h"
#include "scrcmd_infinity_dungeon_nw.h"
#include "script_def.h"

#ifdef PM_DEBUG

// フィールドデバッグ
#include "field_debug.h" 

// デバッグ変数
extern BOOL g_debug_inf_is_map_info;
#endif

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	ヒントメッセージ取得用ワーク
//=====================================
typedef struct{
  const INF_DUNGEON_DATA      *p_dungeon_data;
  const INFINITYDUNGEON_SAVE  *p_save;
  u8                    talk_type;  // 会話タイプ
}TRHINT_WK;

// チュートリアルマップデータ
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tutorial_data.cdat"
#include "../../../resource/infinity_dungeon/data/infinity_dungeon_tutorial_data.h"

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  VecFx32 warp_pos;       // 遷移先でのマップ位置
  s32     zone_id;        // 遷移先ゾーンID
}NEXT_MAP_DATA;

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK          *p_gsys;
  GAMEDATA              *p_gdata;
  FIELDMAP_WORK         *p_field_work;
  INFINITYDUNGEON_SAVE  *p_save;
  FLDMAPPER             *p_g3d_mapper;
  FIELD_BMODEL_MAN      *p_bmodel_man;
  MMDLSYS               *p_mmdlsys;
  const INF_DUNGEON_DATA      *p_dungeon_data;
  HEAPID                heapID;

  // 移動タイプ
  u8                    map_change_type;

  // フェードタイプ
  u8                    fade_in_param;
  u8                    fade_out_param;

  // 移動データ
  u16                   warp_dir;
  NEXT_MAP_DATA         next_map_data;
  const u16             *p_zone_data;

  // フロア
  s32 now_room;
  s32 now_floor;
  s32 next_room;
  s32 next_floor;

  // エリア(フロアから算出)
  u16 now_area;

}MAP_CHAGE_EVENT_WORK;

// ヒントタイプごとのプロセステーブル
typedef u32 (*HINTMSGPROC)(const TRHINT_WK*);

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// 共通
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys);
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk );
static void GetFloorData( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static int GetSetTrainerIDTable( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context);
static u8 GetDungeonFloorFromArea( u8 now_floor ,u8 now_area );

// ゾーン依存
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk);
static u16 GetFieldDir( u16 warp_dir );

// 部屋データ
static const ROOM_DATA* GetNextRoomData( const MAP_CHAGE_EVENT_WORK *p_wk );
static const ROOM_DATA* GetNowRoomData( const MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン生成
static void SetBUildModel( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data);
static void AddDummyObj( const MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data);
static void SetDummyObj( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data);
static u32 GetRecoverTrainerID( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,GFL_STD_RandContext *rand_context );
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,u32 area_no,const int *room_id_array ,int cnt,const u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index );
static BOOL SetTutorialMap( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static void SetTutorialTrainerData( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data );
static s32 GetGateTrainerSetTrIndexArray( INF_DUNGEON_DATA  *p_dungeon_data ,s32 floor_no ,s32* id_pos_array, s32* room_id_array );

// グリッド
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );

// マップワープイベント
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk );

// フロア移動
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk );

// ダンジョン内トレーナー
static int GetRoomTrainerIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no);
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed );
static void SetTrainer( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data);
static u32 GetTrSeed( const INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos );
static s32 GetRecoverTrainerSetRoomIdArray( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,int *room_id_array ,u32 *member_num );

// フロア生成 
static BOOL IsMoveElevator( const MAP_CHAGE_EVENT_WORK *p_wk );

// ヒントメッセージ
static u32 gethint_gate_tr_dir( const TRHINT_WK *p_wk );
static u32 gethint_gate_tr_is_up_down( const TRHINT_WK *p_wk );
static u32 gethint_gate_tr_room_tr_num( const TRHINT_WK *p_wk );
static u32 gethint_gate_tr_sex( const TRHINT_WK *p_wk );
static u32 gethint_gate_tr_move_type( const TRHINT_WK *p_wk );
static u32 gethint_medic_dir( const TRHINT_WK *p_wk );
static u32 gethint_boss_monster( const TRHINT_WK *p_wk );
static u32 gethint_boss_info( const TRHINT_WK *p_wk );
static u32 gethint_no_hint( const TRHINT_WK *p_wk );

static u32 gethint_boss_floor( const TRHINT_WK *p_wk );
static u32 gethint_boss_dir( const TRHINT_WK *p_wk );

static u8  GetPosDirID( u8 x1, u8 y1, u8 x2 ,u8 y2 );
static u32 INFINITY_DUNGEON_GetBossRoom( const INF_DUNGEON_DATA *p_dungeon_data );


#ifdef PM_DEBUG
static void INF_DEBUG_OutPutFloorData( const INF_DUNGEON_DATA *p_wk,const ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save ,FIELDMAP_WORK *p_field_work );
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを生成する
 *
 *	@param	*p_wk
 *	@param	*p_save 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_MakeDungeonData( INF_DUNGEON_DATA *p_wk,GAMEDATA *p_gdata, HEAPID heapID )
{
  u32 master_seed;
  INFINITYDUNGEON_SAVE  *p_inf_save;
  SAVE_CONTROL_WORK     *p_save;
  FIELD_WFBC_CORE_TYPE wfbc_type;
 
  // 無限ダンジョンのデータを取得
  p_save = GAMEDATA_GetSaveControlWork( p_gdata );
  p_inf_save  = SaveData_GetINFINITYDUNGEON( p_save );
  
  // WFBCの状態を調べる
  wfbc_type = GAMEDATA_GetMyWFBCType( p_gdata );

  // 初期化
  GFL_STD_MemFill(p_wk, 0, sizeof(INF_DUNGEON_DATA) );

  // WFBCの状態からバージョンデータを決定する
  if( wfbc_type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    p_wk->version = VERSION_B2;
  }
  else
  {
    p_wk->version = VERSION_W2;
  }

  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_inf_save );
  INF_TOOL_SetDungeonSeed(&p_wk->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_inf_save , p_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを再計算する
 *
 *	@param	*p_wk
 *	@param	*p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INF_ReCalcDungeonData( INF_DUNGEON_DATA *p_wk,INFINITYDUNGEON_SAVE *p_save )
{
  u32 master_seed;

  // シードを生成
  master_seed = INFINITYDUNGEON_SAVE_GetRandomSeed( p_save );
  INF_TOOL_SetDungeonSeed(&p_wk->seed ,master_seed);

  // フロアデータの生成
  GetFloorData( p_save , p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのデータサイズを取得
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INF_GetDungeonDataSize()
{
  return sizeof(INF_DUNGEON_DATA);
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップ位置から次のマップを導き、ワープさせる
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	INF_DUNGEON_DATA *p_dungeon_data
 *	@param	warp_dir        ワープ方向  WARP_*
 *	@param	fade_in_param   フェードインパラメータ    SCR_INFDUNGEON_WARP_*
 *	@param	fade_out_param  フェードアウトパラメータ  SCR_INFDUNGEON_WARP_*
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_WarpNextMap( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys,const INF_DUNGEON_DATA *p_dungeon_data, u16 warp_dir ,u8 fade_in_param ,u8 fade_out_param )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK *p_wk;
 	
  // イベントの呼び出し
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_work, p_event );

  // パラメータの設定
	p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_dungeon_data    = p_dungeon_data;
  p_wk->warp_dir          = warp_dir;
  p_wk->map_change_type   = MAP_CHAGE_TYPE_ROOM;
  p_wk->fade_in_param     = fade_in_param;
  p_wk->fade_out_param    = fade_out_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief  次のフロアにワープ
 *
 *	@param	*p_scr_work
 *	@param	*p_gsys
 *	@param	floor_no 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_WarpNextFloor( SCRIPT_WORK *p_scr_work, GAMESYS_WORK *p_gsys ,INF_DUNGEON_DATA *p_dungeon_data ,int floor_no )
{
	GMEVENT *p_event = NULL;
  MAP_CHAGE_EVENT_WORK  *p_wk;
  SAVE_CONTROL_WORK     *p_save_data;
  GAMEDATA              *p_gdata;
  INFINITYDUNGEON_SAVE  *p_save;

  // セーブデータ取得
  p_gdata       = GAMESYSTEM_GetGameData( p_gsys );
  p_save_data   = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  
  // フロアIDを保存（これを行わないと、エリア番号を取得できない）
  INFINITYDUNGEON_SAVE_SetFloorID( p_save , floor_no );

  // イベント呼び出し
  {
    u8 area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
    p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
    SCRIPT_CallEvent( p_scr_work, p_event );
    p_wk = GMEVENT_GetEventWork( p_event );
    p_wk->next_floor = GetDungeonFloorFromArea( floor_no ,area_id );
    p_wk->map_change_type = MAP_CHAGE_TYPE_FOOR;
    p_wk->p_dungeon_data = p_dungeon_data;
    p_wk->warp_dir = WARP_DIR_DOWN;
    p_wk->fade_out_param = SCR_INFDUNGEON_WARP_FADE;
    p_wk->fade_in_param = SCR_INFDUNGEON_WARP_NO_FADE;
  }
  
  // データを生成するためにフロアを設定する
  {
    // 再計算
    INF_ReCalcDungeonData(p_dungeon_data,p_save); 
  }

  //OS_TFPrintf(1,"p_wk->next_floor:%u\n",p_wk->next_floor)
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  階数からエリア内の階数を取得する
 *
 *	@param	now_floor 
 *	@param	now_area 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetDungeonFloorFromArea( u8 now_floor ,u8 now_area )
{
  u8 start_floor = INFINITYDUNGEON_SAVE_GetAreaStartFloor( now_area );

  return now_floor - start_floor;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントデータから必要データを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitWorkData( MAP_CHAGE_EVENT_WORK *p_wk ,GAMESYS_WORK *p_gsys)
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );
  SAVE_CONTROL_WORK* p_save_data;
  p_wk->heapID        = HEAPID_FIELDMAP;
  p_wk->p_gsys        = p_gsys;
  p_wk->p_field_work  = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
  p_wk->p_gdata       = GAMESYSTEM_GetGameData( p_wk->p_gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_wk->p_gdata );
  p_wk->p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_wk->p_zone_data   = INF_TOOL_GetZoneIdTable( version );
  p_wk->now_room      = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
  p_wk->now_floor     = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  p_wk->p_g3d_mapper  = FIELDMAP_GetFieldG3Dmapper( p_wk->p_field_work );
  p_wk->p_bmodel_man  = FLDMAPPER_GetBuildModelManager( p_wk->p_g3d_mapper );
  p_wk->p_mmdlsys     = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
  p_wk->now_area      = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_wk->p_save );

  //OS_TFPrintf(1,"p_wk->now_floor:%u\n",p_wk->now_floor);
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップチェンジイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_CHAGE_EVENT_WORK *p_wk = wk;
  const ROOM_DATA *p_next_room;

  switch( *seq )
  {
  case 0:
    {
      // データ取得
      GAMESYS_WORK *p_gsys  = GMEVENT_GetGameSysWork( event );
      InitWorkData( p_wk ,p_gsys );

 
      // 動作モデル停止
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );

      // フェード
      if( p_wk->fade_out_param == SCR_INFDUNGEON_WARP_FADE )
      {
        // エレベーター移動の場合と、部屋移動でフェード方法を変える
        if( IsMoveElevator( p_wk ) )
        {
          // 遅いフェードアウト
          GFL_FADE_SetMasterBrightReq(
              GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FIELD_FADE_SLOW_WAIT );
        }
        else
        {
          // フェードアウト
          GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Black( p_wk->p_gsys, p_wk->p_field_work ,FIELD_FADE_WAIT ) );
        }
      }

      (*seq)++;
      break;
    }
  case 1:
    {
      // フェードチェック
      if( !GFL_FADE_CheckFade() )
      {

        // マップ移動
        NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

        // 無限ダンジョン関連の岩データ位置をリセットする
        INF_DUNGEON_FIELD_ResetPushRock( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );

        // 次ゾーンを設定
        SetNextZoneData( p_wk );

        // マップチェンジ
        GMEVENT_CallEvent(event,
            EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work, p_next_map_data->zone_id, &p_next_map_data->warp_pos,GetFieldDir( p_wk->warp_dir ) ));
        (*seq)++;
      }
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }
 
      // ギミックワークのポインタを再取得
      p_wk->p_dungeon_data  = INFINITY_DUNGEON_DataGet( p_wk->p_field_work );

      ONOUE_Printf("ワープ方向%d\n",p_wk->warp_dir);

      // 受付の場合は配置モデルを置かない
      if( p_wk->warp_dir != WARP_INFORMATION )
      {
        // 次の部屋データを取得する
        p_next_room = GetNextRoomData( p_wk );

        if( (p_next_room->room_type != ROOM_TYPE_BOSS) )
        {
          // 配置モデルを配置
          SetBUildModel(p_wk,p_next_room);

          // 封鎖用OBJ配置
          SetDummyObj(p_wk,p_next_room);

          // トレーナー配置
          SetTrainer(p_wk,p_next_room);
        }
      }
 
      // エレベーター移動の場合と、部屋移動でフェード方法を変える
      if( p_wk->fade_in_param == SCR_INFDUNGEON_WARP_FADE )
      {

        if( IsMoveElevator( p_wk ) )
        {
          //ＢＧ復帰
          INFINITY_DUNGEON_NW_BGSettingRecover( p_wk->p_field_work );

          // 遅いフェードアウト
          GFL_FADE_SetMasterBrightReq(
              GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FIELD_FADE_SLOW_WAIT );
        }
        else
        {
          GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black( p_wk->p_gsys, p_wk->p_field_work ,FIELD_FADE_WAIT ) );
        }
      }
      else
      {
        //ＢＧ復帰
        INFINITY_DUNGEON_NW_BGSettingRecover( p_wk->p_field_work );
      }

      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
 
#ifdef PM_DEBUG      
      {
        u8 now_floor;

        // MAP表示デバッグメニュー起動中かチェック
        if( g_debug_inf_is_map_info == TRUE )
        {
          FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( p_wk->p_field_work );
          FIELD_DEBUG_SetPrint(debug,FIELD_DEBUG_PRINT_TYPE_NONE);
        }

        // マップ状況をデバッグ表示
        now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
        INF_DEBUG_OutPutFloorData( p_wk->p_dungeon_data ,p_wk->p_dungeon_data->room_data[now_floor] ,p_wk->p_save ,p_wk->p_field_work);
      }
#endif

      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  次のゾーンデータを設定
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneData( MAP_CHAGE_EVENT_WORK *p_wk)
{
  if( p_wk->map_change_type == MAP_CHAGE_TYPE_ROOM )
  {
    if( p_wk->warp_dir != WARP_INFORMATION )
    {
      // 次のゾーンID設定
      SetNextZoneID( p_wk );

      // 次の場所設定
      SetNextPos( p_wk );
    }
    else
    {
      // 受付データを設定する
      SetReceptionWarpData( p_wk );

      // 受け付けエレベータの場所をセット
      SetReceptionElevatorPos( p_wk );
    }
  }
  else if( p_wk->map_change_type == MAP_CHAGE_TYPE_FOOR )
  {
    // 次のフロア設定
    SetNextFloor( p_wk );

    // 次の場所設定
    SetElevatorPos( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  次ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const ROOM_DATA* GetNextRoomData( const MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->next_floor ][p_wk->next_room];
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動前ルームデータ取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const ROOM_DATA* GetNowRoomData( const MAP_CHAGE_EVENT_WORK *p_wk )
{
  return &p_wk->p_dungeon_data->room_data[ p_wk->now_floor ][p_wk->now_room];
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワープ後のゾーンIDを取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextZoneID( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  const ROOM_DATA     *p_next_room_data;
  u16 warp_dir    = p_wk->warp_dir;
  u32 room_type;
  p_wk->next_room   = p_wk->now_room;
  p_wk->next_floor  = p_wk->now_floor;  // フロアは移動しない

  switch( warp_dir )
  {
  case WARP_DIR_UP:
    p_wk->next_room -= ROOM_MAX_X;
    break;
  case WARP_DIR_DOWN:
    p_wk->next_room += ROOM_MAX_X;
    break;
  case WARP_DIR_LEFT:
    p_wk->next_room --;
    break;
  case WARP_DIR_RIGHT:
    p_wk->next_room ++;
    break;
  defautl:
    GF_ASSERT(0);
    break;
  }

  // 範囲外の場合はとりあえず今の部屋に飛ぶ
  if( p_wk->next_room < 0 )
  {
    p_wk->next_room = p_wk->now_room;
  }
  else if( p_wk->next_room >= ROOM_MAX )
  {
    p_wk->next_room = p_wk->now_room;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  room_type = p_next_room_data->room_type;
  p_next_map_data->zone_id = p_wk->p_zone_data[ room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フロアのエレベーター部屋に移動
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextFloor( MAP_CHAGE_EVENT_WORK *p_wk )
{
  int i;
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  const ROOM_DATA *p_next_room_data;

  // 現在の部屋位置を初期化
  p_wk->next_room = 0;

  // エレベータ部屋を探す
  for( i = 0 ; i < ROOM_MAX ; i++ )
  {
    // 部屋タイプの取得
    const ROOM_DATA  *p_room_data;
    p_room_data = GetNextRoomData( p_wk );

    // エレベータ部屋が見つかるまでつづける
    if( p_room_data->room_type == ROOM_TYPE_ELEVATOR )
    {
      break;
    }

    // 次の部屋へ
    p_wk->next_room++;
  }

  // 次の部屋データを取得する
  p_next_room_data = GetNextRoomData( p_wk );

  // ループタイプに合わせたゾーンを設定する
  p_next_map_data->zone_id = p_wk->p_zone_data[ p_next_room_data->room_type ];
  INFINITYDUNGEON_SAVE_SetRoomID( p_wk->p_save , p_wk->next_room );

  // 次のフロアを設定する
  p_wk->now_floor = p_wk->next_floor;
  
  // エレベータ部屋が無かったら警告
  GF_ASSERT( p_next_room_data->room_type == ROOM_TYPE_ELEVATOR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動方向に対応した場所を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetNextPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  const GRID_DATA   *p_pos;
  const ROOM_DATA   *p_room_data;
  u32               room_type;
  u8 version        = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );
  
  // 部屋タイプの取得
  p_room_data = GetNextRoomData( p_wk );
  room_type = p_room_data->room_type;

  p_pos = INF_TOOL_GetWarpPosTable( version ,room_type ,p_wk->warp_dir  );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付に移動するためのデータを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionWarpData( MAP_CHAGE_EVENT_WORK *p_wk )
{
  // ループタイプに合わせたゾーンを設定する
  NEXT_MAP_DATA *p_next_map_data  = &p_wk->next_map_data;
  p_next_map_data->zone_id = p_wk->p_zone_data[ ROOM_TYPE_RECEPTION ];
  SetElevatorPos( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンごとのエレベーターの位置を設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );
  const GRID_DATA *p_pos = INF_TOOL_GetElevatorPos( version );

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  受付用エレベータの場所をセット
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetReceptionElevatorPos( MAP_CHAGE_EVENT_WORK *p_wk )
{
  NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;

  const GRID_DATA *p_pos;

  if( p_wk->now_area <= INF_LEFT_ELEVATOR_AREA )
  {
    p_pos = INF_TOOL_GetReceptionElevatorPos( RECEPTION_ELEVATOR_L );
  }
  else
  {
    p_pos = INF_TOOL_GetReceptionElevatorPos( RECEPTION_ELEVATOR_R );
  }

  SetGridPos( &p_next_map_data->warp_pos ,p_pos->x , p_pos->y , p_pos->z);
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標をFx32に変換してセットする
 *
 *	@param	*vec 
 *	@param	x     Xグリッド座標
 *	@param	y     Yグリッド座標
 *	@param	z     Xグリッド座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z )
{
  vec->x = (x * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->y = (y * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->z = (z * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のダンジョンバージョンを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 INFINITY_DUNGEON_GetDungeonColor( const INF_DUNGEON_DATA *p_wk )
{
  return p_wk->version;
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーデータ
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	area_no         エリア番号
 *	@param	seed            生成用シード
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainerData( INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no , u32 seed )
{
  int i,n;
  int tr_cnt = 0;
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  GFL_STD_RandContext rand_context;
  
  // 乱数を作成
  GFL_STD_RandInit( &rand_context, seed );
  
  // トレーナーIDテーブル取得
  tr_cnt = GetSetTrainerIDTable( p_dungeon_data , area_no ,trid_array, &rand_context  );

  // トレーナーIDのランダムデータを生成
  for( n = 0; n<INFINITYDUNGEON_FLOOR_MAX; n++)
  {
    // トレーナーデータ配列をランダム化
    INF_TOOL_RndArray( trid_array ,tr_cnt ,&rand_context);

    // 必要な数だけトレーナーIDを保存
    for( i = 0; i<INFINITY_DUNGEON_TRID_MAX; i++)
    {
      p_dungeon_data->tr_data[n].tr_id[i] = trid_array[i];
    }

    // 回復トレーナー部屋IDリストを初期化
    p_dungeon_data->recover_room_id[n] = ROOM_MAX;
  }

  // 回復トレーナー配置位置を決める
  {
    s32 cnt = 0;
    int room_id_array[ROOM_MAX];
    u32 member_num[ROOM_MAX];
    u32 recover_index = 0;
    u8 top_floor = INFINITYDUNGEON_SAVE_GetFloorNum( area_no );

    // 全フロアに配置する
    for( n = 0; n<top_floor; n++)
    {
      GFL_STD_MemFill(room_id_array, 0, sizeof(room_id_array) );
      GFL_STD_MemFill(member_num, 0, sizeof(member_num) );

      // 設置可能な部屋IDをリストにして返す
      cnt = GetRecoverTrainerSetRoomIdArray( p_dungeon_data ,n ,room_id_array, member_num );

      // 配置不能な場合対処
      GF_ASSERT_MSG(cnt!=0,"回復トレーナーが置ける場所がありません\n");
      if( cnt <= 0 ){ return; }

      SetRecoverTrainer( p_dungeon_data ,n,area_no ,room_id_array ,cnt ,member_num ,&rand_context ,&recover_index);
    }
  }

  // ゲートレーナーを配置する
  {
    s32 id_pos_array[INFINITY_DUNGEON_TRID_MAX];
    s32 room_id_array[INFINITY_DUNGEON_TRID_MAX];

    u8 top_floor = INFINITYDUNGEON_SAVE_GetFloorNum( area_no );
    s32 data_cnt = 0;
    u8 index_rand;
    
    // 配置階を決める
    p_dungeon_data->gate_trainer_floor = GFL_STD_Rand( &rand_context ,top_floor );

    // 設置可能な部屋IDをリストにして返す
    data_cnt = GetGateTrainerSetTrIndexArray( p_dungeon_data ,p_dungeon_data->gate_trainer_floor ,id_pos_array ,room_id_array );
    
    // 配置不能な場合対処
    GF_ASSERT_MSG(data_cnt!=0,"ゲートトレーナーが置ける場所がありません\n");
    if( data_cnt <= 0 ){ return; }
    
    // 配列から実際のトレーナー位置をひとつに決定
    index_rand = GFL_STD_Rand( &rand_context ,data_cnt );
    p_dungeon_data->gate_trainer_room_id = room_id_array[ index_rand ];
    p_dungeon_data->gate_trainer_id_pos = id_pos_array[ index_rand ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーに上書きできるトレーナーリストを作成する（トレーナーデータインデックス配列で返す）
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_no 
 *	@param	id_pos_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetGateTrainerSetTrIndexArray( INF_DUNGEON_DATA  *p_dungeon_data ,s32 floor_no ,s32* id_pos_array, s32* room_id_array )
{
  s32 i;
  s32 room_id;
  s32 cnt = 0;
  s32 boss_room_id = -1;

  for( room_id = 0; room_id<ROOM_MAX; room_id++)
  {
    u8 room_type = p_dungeon_data->room_data[floor_no][room_id].room_type;
    u8 pos_type = p_dungeon_data->room_data[floor_no][room_id].tr_pos_type;
    
    // ボス部屋には置けない、またボス部屋の位置も覚えておく
    if( room_type ==  ROOM_TYPE_BOSS){ 
      boss_room_id = room_id;
      continue;
    }

    // 通常部屋以外には配置しない
    if( room_type !=  ROOM_TYPE_DEFAULT){ continue; }

    // ボス部屋が見つかったときは、ボス部屋の下の部屋には置かない
    if( ( boss_room_id != -1 ) &&
        boss_room_id == (room_id + ROOM_MAX_X) ){ continue; }

    // トレーナーIDを保存する
    {
      u8 member_num = INF_TOOL_GetMemberNum(pos_type);

      // トレーナーインデックスを取得
      while( member_num > 0 )
      {
        BOOL is_recover = FALSE;
        s32 id_pos;
        u16 tr_id;
        member_num--;
        id_pos = GetRoomTrainerIDPos( p_dungeon_data ,floor_no ,room_id ,member_num );
        tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_dungeon_data,floor_no ,id_pos );

        // 回復トレーナー以外が配置対象
        for( i = 0; i < INFINITY_DUNGEON_RECOVERTR_MAX; i++ )
        {
          if( p_dungeon_data->recover_data[i] == tr_id)
          {
            is_recover = TRUE;
            break;
          }
        }

        // 回復トレーナーでない場合は保存
        if( is_recover == FALSE )
        {
          id_pos_array[ cnt ] = id_pos;
          room_id_array[ cnt ] = room_id;
          cnt ++;
          GF_ASSERT_MSG(cnt<INFINITY_DUNGEON_TRID_MAX,"配置人数が多すぎます(%d<%d)\n",cnt,INFINITY_DUNGEON_TRID_MAX);
        }
      }
    }
  }

  return cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナーの設置可能な部屋IDを返す
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_no 
 *	@param	*room_id_array 
 *	@param	*member_num 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetRecoverTrainerSetRoomIdArray( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,int *room_id_array ,u32 *member_num )
{
  int i;
  s32 cnt = 0;
  for( i = 0; i<ROOM_MAX; i++)
  {
    u8 pos_type = p_dungeon_data->room_data[floor_no][i].tr_pos_type;
    if( INF_TOOL_GetMemberNum(pos_type) > 0 )
    {
      room_id_array[cnt] = i;
      member_num[cnt] = INF_TOOL_GetMemberNum(pos_type);
      cnt++;
    }
  }

  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナー設置
 *
 *	@param	*p_dungeon_data ダンジョンデータ
 *	@param	floor_no        設置フロア
 *	@param	*room_id_array  配置可能部屋リスト
 *	@param	cnt             配置可能部屋数
 *	@param	*member_num     トレーナー数配列
 *	@param	*rand_context   
 *	@param	*recover_index  回復トレーナーインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetRecoverTrainer( INF_DUNGEON_DATA  *p_dungeon_data ,int floor_no ,u32 area_no,const int *room_id_array ,int cnt,const u32 *member_num , GFL_STD_RandContext *rand_context ,u32 *recover_index )
{
  int id_pos = 0;
  u16 tr_id;
  u32 room_id_index;
  u32 member_index;

  room_id_index = GFL_STD_Rand( rand_context ,cnt);
  member_index = GFL_STD_Rand( rand_context ,member_num[room_id_index]);
  id_pos = GetRoomTrainerIDPos( p_dungeon_data ,floor_no ,room_id_array[room_id_index] ,member_index);

  // エリア番号から、適切な回復トレーナーを抽選
  tr_id = GetRecoverTrainerID( p_dungeon_data ,area_no ,rand_context);

  p_dungeon_data->tr_data[floor_no].tr_id[id_pos] = tr_id;
  p_dungeon_data->recover_data[(*recover_index)] = tr_id;
  (*recover_index)++;
  p_dungeon_data->recover_room_id[floor_no] = room_id_array[room_id_index];
  OS_Printf("回復トレーナー部屋NO：%u\n",room_id_array[room_id_index]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアごとの回復トレーナーID取得
 *
 *	@param	*p_dungeon_data 
 *	@param	*rand_context 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetRecoverTrainerID( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,GFL_STD_RandContext *rand_context )
{
  const u32 dungeon_color = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );
  u32 start_trid = INF_TRID_START_RECOVERY;
  u32 trid_ofs   = GFL_STD_Rand( rand_context ,INF_TRID_RECOVERY_AREA_NUM);

  // 現在のエリアによって分岐
  switch( area_no )
  {
  case INFINITY_DUNGEON_AREA_ID_1:
  case INFINITY_DUNGEON_AREA_ID_2:
  case INFINITY_DUNGEON_AREA_ID_3:
  case INFINITY_DUNGEON_AREA_ID_4:
  case INFINITY_DUNGEON_AREA_ID_5:
    {
      if( dungeon_color == VERSION_B2 )
      {
        start_trid = INF_TRID_RECOVERY_START_AREA1_5_B;
      }
      else
      {
        start_trid = INF_TRID_RECOVERY_START_AREA1_5_W;
      }
    }
    break;
  case INFINITY_DUNGEON_AREA_ID_6:
  case INFINITY_DUNGEON_AREA_ID_7:
  case INFINITY_DUNGEON_AREA_ID_8:
  case INFINITY_DUNGEON_AREA_ID_9:
  case INFINITY_DUNGEON_AREA_ID_10:
    {
      if( dungeon_color == VERSION_B2 )
      {
        start_trid = INF_TRID_RECOVERY_START_AREA6_10_B;
      }
      else
      {
        start_trid = INF_TRID_RECOVERY_START_AREA6_10_W;
      }
    }
    break;
  default:
    GF_ASSERT(0);
    start_trid = INF_TRID_RECOVERY_START_AREA1_5_B;
    break;
  }

  // 回復トレーナーID
  start_trid += trid_ofs;

  return start_trid;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ブラック用トレーナーIDテーブルを取得
 *
 *	@param	*p_trid_array 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetSetTrainerIDTable( const INF_DUNGEON_DATA *p_dungeon_data ,u32 area_no ,u32 *p_trid_array ,GFL_STD_RandContext *rand_context)
{
  int i;
  u16 tr_id = 0;
  int cnt = 0;
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );
  u32 trid_array[INF_DUNGEON_TR_DATA_MAX];
  u32 tmp_trid_array[INF_DUNGEON_TR_DATA_MAX];
  u32 add_data_num = 0;

  // メモリをクリーン
  GFL_STD_MemFill( trid_array ,0 , sizeof(trid_array));

  // 使用できるトレーナーデータを決定
  for( i = 0 ; i < INF_TR_AREA_DATA_MAX; i++ )
  {
    const INF_TR_AREA_DATA* sc_tr_area_data = INF_TOOL_GetTrainerParam( i );
    GF_ASSERT_MSG(cnt<INF_DUNGEON_TR_DATA_MAX,"無限ダンジョンのトレーナー抽選に使用するトレーナー数が多すぎます(%d<%d)\n",cnt,INF_DUNGEON_TR_DATA_MAX);

    if( (area_no >= sc_tr_area_data->start_area ) &&
        (area_no <= sc_tr_area_data->end_area ) )
    {
      // バージョン指定なし=0
      if( (sc_tr_area_data->version == version) ||
          (sc_tr_area_data->version == 0) )
      {
        trid_array[add_data_num] = i;
        add_data_num++;
      }
    }
  }

  // トレーナータイプごとに抽選データを取得する
  {
    u8 tr_type;
    const u8 *p_trtype_cnt = INF_TOOL_GetTrTypeCntTable(area_no,version); 

    for( tr_type = 0; tr_type < INF_TR_TYPE_MAX; tr_type++ )
    {
      // 配置人数が0の場合のみテーブルに追加する作業を行う
      if( p_trtype_cnt[tr_type] > 0 )
      {
        int add_index = 0;
        int add_cnt = p_trtype_cnt[tr_type];
        int tmp_data_cnt = 0;
        GFL_STD_MemFill( tmp_trid_array ,0 , sizeof(tmp_trid_array));

        // タイプを絞り込んだテーブルを作成
        for( i = 0; i < add_data_num; i++ )
        {
          if( INF_TOOL_GetTrainerType( trid_array[i] ) == tr_type )
          {
            // トレーナーIDを一時保存
            tmp_trid_array[tmp_data_cnt] = trid_array[i];
            tmp_data_cnt++;
          }
        }
        // テーブル内をランダムにする
        INF_TOOL_RndArray( tmp_trid_array, tmp_data_cnt, rand_context );

        // テーブルに追加
        while( add_cnt > 0 )
        {
          p_trid_array[cnt] = tmp_trid_array[add_index];
          add_cnt--;    // 追加する数
          add_index++;  // 追加した位置
          cnt++;        // 出力トレーナー数
          GF_ASSERT_MSG(add_index<=tmp_data_cnt,"トレーナータイプ数の指定が間違っています[タイプインデックス->%dのトレーナーが足りない]\n",tr_type);
          ONOUE_Printf("tr_type => %u add\n",tr_type);
        }
      }
    }
  }
  
  return cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデルIDテーブルを取得
 *
 *	@param	INF_DUNGEON_DATA *p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
const BMODEL_ID* INFINITY_DUNGEON_GetBModelIdTable( const INF_DUNGEON_DATA *p_dungeon_data )
{
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );

  //-------------------------------------
  // 封鎖用モデルOBJ(黒)
  //=====================================
  static const BMODEL_ID sc_black_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj01_nsbmd, // 上
    NARC_output_buildmodel_indoor_mugenobj05_nsbmd, // 下
    NARC_output_buildmodel_indoor_mugenobj04_nsbmd, // 右
    NARC_output_buildmodel_indoor_mugenobj06_nsbmd, // 左
  };
  //-------------------------------------
  // 封鎖用モデルOBJ(白)
  //=====================================
  static const BMODEL_ID sc_white_bmodel_table[] = {
    NARC_output_buildmodel_indoor_mugenobj02_nsbmd, // 上
    NARC_output_buildmodel_indoor_mugenobj08_nsbmd, // 下
    NARC_output_buildmodel_indoor_mugenobj09_nsbmd, // 右
    NARC_output_buildmodel_indoor_mugenobj07_nsbmd, // 左
  };

  
  // モデルテーブル差し替え
  if( version == VERSION_B2 )
  {
    return sc_black_bmodel_table;
  }
  else
  {
    return sc_white_bmodel_table;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  配置モデルで通行禁止の場所をふさぐ
 *
 *	@param	*p_wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetBUildModel( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data)
{

  FIELD_BMODEL *p_bmdl;
  INF_GIMMICK_WK *p_gmk_wk;
  GFL_G3D_OBJSTATUS status;
  const BMODEL_ID *p_bmodel_id_table = INFINITY_DUNGEON_GetBModelIdTable( p_wk->p_dungeon_data );
  
  p_gmk_wk = GMK_TMP_WK_GetWork(p_wk->p_field_work, INF_GIMMICK_WORK_ASSIGN_ID);

  // 表示座標を初期化
  status.trans.x = 0;
  status.trans.y = 0;
  status.trans.z = 0;
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  GFL_CALC3D_MTX_CreateRot( 0, 0, 0, &status.rotate );

  
  // 配置
  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      // 上の部屋がボス部屋だった場合は設置しない
      BOOL is_boss = INFINITY_DUNGEON_GetUpPosRoomTypeIsBoss( p_wk->p_dungeon_data ,p_wk->next_floor ,p_wk->next_room) ;
      if( !is_boss )
      {
        if( !p_room_data->can_move[INF_MOVE_UP] )
        {
          p_bmdl = FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_UP], &status );
          // ギミックワークに生成した配置モデルを保存しておく
          INFINITY_DUNGEON_GMK_SetBModelData( p_gmk_wk,p_bmdl,INF_MOVE_UP);
        }
      }
    }

    // 下
    if( !p_room_data->can_move[INF_MOVE_DOWN] )
    {
      p_bmdl = FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_DOWN], &status );
      // ギミックワークに生成した配置モデルを保存しておく
      INFINITY_DUNGEON_GMK_SetBModelData( p_gmk_wk,p_bmdl,INF_MOVE_DOWN);
    }

    // 左
    if( !p_room_data->can_move[INF_MOVE_LEFT]  )
    {
      p_bmdl = FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_LEFT], &status );
      // ギミックワークに生成した配置モデルを保存しておく
      INFINITY_DUNGEON_GMK_SetBModelData( p_gmk_wk,p_bmdl,INF_MOVE_LEFT);
    }

    // 右
    if( !p_room_data->can_move[INF_MOVE_RIGHT] )
    {
      p_bmdl = FIELD_BMODEL_Create_Direct( p_wk->p_bmodel_man, p_bmodel_id_table[INF_MOVE_RIGHT], &status );
      // ギミックワークに生成した配置モデルを保存しておく
      INFINITY_DUNGEON_GMK_SetBModelData( p_gmk_wk,p_bmdl,INF_MOVE_RIGHT);
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトを追加する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetDummyObj( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data)
{
  // バージョン取得
  u8 version = INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data );

  // ボス部屋は封鎖しない
  if( p_room_data->room_type != ROOM_TYPE_BOSS )
  {
    // 上(エレベータ部屋には上封鎖オブジェクトを置くことがない)
    if( p_room_data->room_type != ROOM_TYPE_ELEVATOR )
    {
      // 上の部屋がボス部屋だった場合は設置しない
      BOOL is_boss = INFINITY_DUNGEON_GetUpPosRoomTypeIsBoss( p_wk->p_dungeon_data ,p_wk->next_floor ,p_wk->next_room) ;
      // ボスを倒しているか調べる
      BOOL is_gate_open = INFINITY_DUNGEON_GetGateOpen( p_wk->p_dungeon_data,p_wk->p_save );

      // ボス部屋に通じる道でかつ、ゲートトレーナーを倒していないときは
      // 道を封鎖する
      if( (!p_room_data->can_move[INF_MOVE_UP]) ||
          ((is_boss == TRUE) && ( is_gate_open==FALSE ) ) )
      {
        // ダミーオブジェクトで封鎖
        const GRID_DATA *p_grid_data = INF_TOOL_GetBmodelPos( version ,p_room_data->room_type ,WARP_DIR_UP );
        GRID_DATA gd = *p_grid_data;
        AddDummyObj( p_wk , &gd );  // 中
        gd.x = (*p_grid_data).x + 1;
        AddDummyObj( p_wk , &gd );  // 左
        gd.x = (*p_grid_data).x - 1;
        AddDummyObj( p_wk , &gd );  // 右
      }
    }

    // 下
    if( !p_room_data->can_move[INF_MOVE_DOWN] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = INF_TOOL_GetBmodelPos( version ,p_room_data->room_type ,WARP_DIR_DOWN );
      AddDummyObj( p_wk , p_grid_data );
    }

    // 左
    if( !p_room_data->can_move[INF_MOVE_LEFT] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = INF_TOOL_GetBmodelPos( version ,p_room_data->room_type ,WARP_DIR_LEFT );
      GRID_DATA gd = *p_grid_data;
      AddDummyObj( p_wk , &gd );  // 中
      gd.z = (*p_grid_data).z - 1;
      AddDummyObj( p_wk , &gd );  // 上
      gd.z = (*p_grid_data).z + 1;
      AddDummyObj( p_wk , &gd );  // 下
    }

    // 右
    if( !p_room_data->can_move[INF_MOVE_RIGHT] )
    {
      // ダミーオブジェクトで封鎖
      const GRID_DATA *p_grid_data = INF_TOOL_GetBmodelPos( version ,p_room_data->room_type ,WARP_DIR_RIGHT );
      GRID_DATA gd = *p_grid_data;
      AddDummyObj( p_wk , &gd );  // 中
      gd.z = (*p_grid_data).z - 1;
      AddDummyObj( p_wk , &gd );  // 上
      gd.z = (*p_grid_data).z + 1;
      AddDummyObj( p_wk , &gd );  // 下
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダミーオブジェクトの追加
 *
 *	@param	*p_wk 
 *	@param	x 
 *	@param	y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void AddDummyObj( const MAP_CHAGE_EVENT_WORK *p_wk , const GRID_DATA *p_grid_data)
{
  const NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
  MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys, p_grid_data->x, p_grid_data->z, DIR_UP,
      INFINITY_DUNGEON_BLOCK_OBJ_ID, NONDRAW, MV_DMY, p_next_map_data->zone_id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内復帰処理
 *
 *	@param	*p_gsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Recovery( GAMESYS_WORK *p_gsys ,const INF_DUNGEON_DATA  *p_dungeon_data)
{
  MAP_CHAGE_EVENT_WORK  *p_wk;
  const ROOM_DATA       *p_room_data;

  // ワーク確保
  p_wk = GFL_HEAP_AllocMemoryLo( HEAPID_FIELDMAP, sizeof( MAP_CHAGE_EVENT_WORK ) );

  // ダンジョンデータの設定
  p_wk->p_dungeon_data = p_dungeon_data;

  // ワーク初期化
  InitWorkData( p_wk ,p_gsys );

  // 配置モデルを配置
  p_room_data = GetNowRoomData( p_wk );
  SetBUildModel(p_wk,p_room_data);
 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド指定用方向定義に変換する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u16 GetFieldDir( u16 warp_dir )
{
  // 変換
  switch( warp_dir )
  {
  case WARP_DIR_UP:
    return DIR_UP;
  case WARP_DIR_DOWN:
    return DIR_DOWN;
  case WARP_DIR_LEFT:
    return DIR_LEFT;
  case WARP_DIR_RIGHT:
    return DIR_RIGHT;
  case WARP_INFORMATION:
    return WARP_DIR_DOWN;
  default:
    return WARP_DIR_DOWN;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーを配置する
 *
 *	@param	*p_wk 
 *	@param	p_room_data
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrainer( const MAP_CHAGE_EVENT_WORK *p_wk , const ROOM_DATA* p_room_data)
{
  int i;
  const INF_ROOM_SET_TR_PARAM *p_tr_room;

  if( INF_TOOL_GetMemberNum(p_room_data->tr_pos_type) > 0 )
  {
    p_tr_room = INF_TOOL_GetRoomTrParam(p_room_data->tr_pos_type);

    for( i= 0; i<INF_TOOL_GetMemberNum(p_room_data->tr_pos_type); i++ )
    {
      const INF_ROOM_TR_DATA *p_md = &p_tr_room->tr_move_data[i];
      const NEXT_MAP_DATA *p_next_map_data = &p_wk->next_map_data;
      const INF_MMDL_PARAM *p_mmdlp = INF_TOOL_GetMMDLParam(p_md->move_type);
      u16 tr_id;
      u8  id_pos;

      id_pos = GetRoomTrainerIDPos( p_wk->p_dungeon_data,p_wk->now_floor, p_room_data->room_id ,i );
      tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_wk->p_dungeon_data,p_wk->now_floor ,id_pos );
      if( tr_id != INF_DUNGEON_TR_ID_MAX )
      {
        u16 obj_code = FBI_TOOL_GetOBJCode( ARCID_INFINITY_DUGENON_TR , tr_id ,p_wk->heapID);
        MMDL *mmdl = MMDLSYS_AddMMdlParam( p_wk->p_mmdlsys,p_md->g_x, p_md->g_y, p_mmdlp->direction,
            id_pos + INFINITY_DUNGEON_TR_OBJ_ID_OFS , obj_code, p_mmdlp->move_code,
            p_next_map_data->zone_id );
        MMDL_SetEventType( mmdl , p_mmdlp->ev_type);
        MMDL_SetParam( mmdl, p_mmdlp->param0 , MMDL_PARAM_0 );
        MMDL_SetParam( mmdl, p_mmdlp->param1 , MMDL_PARAM_1 );
        MMDL_SetMoveLimitX( mmdl, p_mmdlp->lim_x );
        MMDL_SetMoveLimitZ( mmdl, p_mmdlp->lim_z );
        MMDL_SetEventID( mmdl, SCRID_INFINITY_DUNGEON_TALK_BATTLE_TRAINER );
        OS_Printf("配置TRID=%u\n",tr_id);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定部屋IDのトレーナーID格納位置取得
 *
 *	@param	*p_dungeon_data フロアデータ
 *	@param	floor_id      フロアID
 *	@param	room_id       部屋ID
 *	@param	room_tr_no    部屋内の相対ID（0～4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static int GetRoomTrainerIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u32 floor_id ,u32 room_id,u8 room_tr_no)
{
  int i;
  u32 tr_num = 0;

  for( i = 0; i<ROOM_MAX; i++ )
  {
    u32 tr_pos_type = p_dungeon_data->room_data[floor_id][i].tr_pos_type;
    
    // 指定の部屋まで移動した場合は終了する
    if( room_id <= i )
    {
      break;
    }

    // 人数加算
    if( INF_TOOL_GetMemberNum( tr_pos_type ) > 0 )
    {
      tr_num+=INF_TOOL_GetMemberNum( tr_pos_type );
    }
  }

  return tr_num+room_tr_no;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナー戦闘イベント
 *
 *	@param	*gsys
 *	@param	*fieldmap 
 *	@param	obj_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * INFINITY_DUNGEON_EVENT_TrainerBattle(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap ,u8 obj_id)
{
  GMEVENT   *event;
  GAMEDATA  *p_gdata;
  SAVE_CONTROL_WORK     *p_save_data;
  INFINITYDUNGEON_SAVE  *p_save;
  u32 area_no;

  BSUBWAY_TRAINER_ROM_DATA  *trd;
  BSUBWAY_PARTNER_DATA tr_data;
  HEAPID heapID = HEAPID_FIELDMAP;
  u16 tr_id;
  GFL_STD_RandContext rand_context;

  INF_DUNGEON_DATA *p_dungeon_data;
  u32 tr_seed;
  u32 floor_id;
  u8  version;
  u8  id_pos = obj_id;

  // セーブデータの取得
  p_gdata       = GAMESYSTEM_GetGameData( gsys );
  p_save_data         = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save        = SaveData_GetINFINITYDUNGEON( p_save_data );
   
  // 現在のエリア取得
  area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  // トレーナーシード
  p_dungeon_data  = INFINITY_DUNGEON_DataGet(fieldmap);
  floor_id        = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  tr_seed         = GetTrSeed( p_dungeon_data ,floor_id ,id_pos );

  // トレーナーID
  tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_dungeon_data ,floor_id ,id_pos );

  version = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );

  // 乱数を作成
  GFL_STD_RandInit( &rand_context, tr_seed );
  
  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,tr_id,NARC_message_infinity_trname_dat ,heapID);

  //ポケモンデータをセット
  FBI_TOOL_SetBSWayPokemonParam(
      trd, tr_id , tr_data.btpwd,2,
      ARCID_INFINITY_DUGENON_PD,NULL, NULL, NULL, &rand_context, INF_TOOL_GetPowRnd(area_no), heapID);

  GFL_HEAP_FreeMemory( trd );

  {
    // 手持ちパーティー生成
    BATTLE_SETUP_PARAM *dst;
    int i;
    POKEPARTY *party;
	  POKEMON_PARAM * pp;
    const POKEPARTY *myparty;
    u8 member_num;
    const INF_LEVEL_TABLE_PARAM *p_lv_param = INF_TOOL_GetLevelData( area_no );

    myparty = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) ); 
    member_num = PokeParty_GetPokeCount(myparty);
    party = PokeParty_AllocPartyWork( GFL_HEAP_LOWID(heapID) );
	  PokeParty_Init( party, member_num );
    for( i=0; i<member_num; i++ ){
      pp = PokeParty_GetMemberPointer( myparty, i );
      PokeParty_Add( party, pp );
    }

    // バトルパラメータの作成
    dst = FBI_TOOL_INFINITY_DUNGEON_CreateBattleParam(
        gsys, party, 
        member_num , 0 , 2,
        &tr_data,
        NULL ,TRID_OFFSET_INFINITY_DUNGEON ,
        p_lv_param->level_max ,
        p_lv_param->level_min , 0 ,version ,area_no, &rand_context);

    GFL_HEAP_FreeMemory(party);

    event = EVENT_InfinityDungeonTrainerBattle( gsys, fieldmap, dst );
  }
  return( event );
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのフロアデータを取得
 *
 *	@param	*p_save       無限ダンジョンセーブ
 *	@param	*p_dungeon_data フロアデータセット先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetFloorData( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  int i;
  u8 area_no = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
  const INF_DUNGEON_SEED *p_seed = &p_dungeon_data->seed;
  
  // @todo チュートリアルかどうか判定を分ける
  // チュートリアルMAPをセットする
  BOOL is_tutorial = SetTutorialMap( p_save ,p_dungeon_data );
  
  // チュートリアルMAPをセットしなかったときは、ダンジョンを自動生成
  if( is_tutorial == FALSE )
  {
    s32 fs_x,fs_y;
    // ダンジョンの広さを取得する
    INF_TOOL_GetFloorSize( area_no ,&fs_x ,&fs_y);
    // ダンジョンデータを生成(エリアに対応したサイズで生成する)
    INF_DCREATOR_SetDungeonData( p_dungeon_data ,area_no ,p_seed->dungeon_data_seed ,fs_x ,fs_y);
    // トレーナーデータをセット
    SetTrainerData( p_dungeon_data ,area_no ,p_seed->tr_data_seed );
  }
  else
  {
    // チュートリアル用トレーナーデータをセットする
    SetTutorialTrainerData( p_save ,p_dungeon_data );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルMAPをセットする
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SetTutorialMap( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  u8 dungeon_status;
  FIELD_WFBC_CORE_TYPE wfbc_type;

  // 両バージョンどちらかで、チュートリアルエリアを抜けているかどうかを調べる
  {
    u8 dungeon_status_b = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,FIELD_WFBC_CORE_TYPE_BLACK_CITY );
    u8 dungeon_status_w = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,FIELD_WFBC_CORE_TYPE_WHITE_FOREST );
    u8 now_area = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
    u8 max_status;

    // 白と黒、進んでいる方を調べる
    if( dungeon_status_b < dungeon_status_w )
    {
      max_status = dungeon_status_w;
    }
    else
    {
      max_status = dungeon_status_b;
    }

    // チュートリアルエリア2を超えているか、既にクリア済みのエリアの場合はチュートリアルデータに差換えない
    if( (max_status >= INF_TUTORIAL_DATA_CNT) || (max_status > now_area)  )
    {
      return FALSE;
    }
  }

  // バージョンからWFBCの状態を取得
  if( p_dungeon_data->version == VERSION_B2 )
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
  else
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
  }

  // ダンジョンの状態を取得
  dungeon_status = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,wfbc_type );

  {
    u8 tutorial_data_index = dungeon_status;
    const ROOM_DATA *p_w_tutorialmap_table[] =
    {
      sc_tutorial_rooo_data_w1,
      sc_tutorial_rooo_data_w2,
    };
    const ROOM_DATA *p_b_tutorialmap_table[] =
    {
      sc_tutorial_rooo_data_b1,
      sc_tutorial_rooo_data_b2,
    };

    // データをクリア
    GFL_STD_MemFill(p_dungeon_data->room_data, 0, sizeof(p_dungeon_data->room_data));

    // ダンジョンの状態を設定
    if( p_dungeon_data->version == VERSION_B2 )
    {
      GFL_STD_MemCopy( p_b_tutorialmap_table[tutorial_data_index], p_dungeon_data->room_data, sizeof(sc_tutorial_rooo_data_b1));
    }
    else
    {
      GFL_STD_MemCopy( p_w_tutorialmap_table[tutorial_data_index], p_dungeon_data->room_data, sizeof(sc_tutorial_rooo_data_b1));
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルトレーナーをセットする
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTutorialTrainerData( const INFINITYDUNGEON_SAVE *p_save ,INF_DUNGEON_DATA *p_dungeon_data )
{
  u8 dungeon_status;
  FIELD_WFBC_CORE_TYPE wfbc_type;

  // バージョンからWFBCの状態を取得
  if( p_dungeon_data->version == VERSION_B2 )
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
  else
  {
    wfbc_type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
  }

  // ダンジョンの状態を取得
  dungeon_status = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,wfbc_type );

  // チュートリアル用トレーナーをセットする
  {
    u8 tutorial_data_index = dungeon_status;

    // トレーナーデータ配列一覧
    const u16 *p_w_tutorialtr_table[][2] =
    {
      {
        sc_tutorial_tr_data_w1,
        sc_tutorial_rectr_data_w1,
      },
      {
        sc_tutorial_tr_data_w2,
        sc_tutorial_rectr_data_w2,
      },
    };
    const u16 *p_b_tutorialtr_table[][2] =
    {
      {
        sc_tutorial_tr_data_b1,
        sc_tutorial_rectr_data_b1,
      },
      {
        sc_tutorial_tr_data_b2,
        sc_tutorial_rectr_data_b2,
      },
    };
    
    // データをクリア
    GFL_STD_MemFill(p_dungeon_data->tr_data, 0, sizeof(p_dungeon_data->tr_data));
    GFL_STD_MemFill(p_dungeon_data->recover_data, 0, sizeof(p_dungeon_data->recover_data));

    // ダンジョンの通常トレーナーを設定
    if( p_dungeon_data->version == VERSION_B2 )
    {
      GFL_STD_MemCopy( p_b_tutorialtr_table[tutorial_data_index][0], &p_dungeon_data->tr_data[0], sizeof(DUNGEON_TRAINER));
      GFL_STD_MemCopy16( p_b_tutorialtr_table[tutorial_data_index][1], p_dungeon_data->recover_data, sizeof(p_dungeon_data->recover_data));
    }
    else
    {
      GFL_STD_MemCopy( p_w_tutorialtr_table[tutorial_data_index][0], &p_dungeon_data->tr_data[0], sizeof(DUNGEON_TRAINER));
      GFL_STD_MemCopy16( p_w_tutorialtr_table[tutorial_data_index][1], p_dungeon_data->recover_data, sizeof(p_dungeon_data->recover_data));
    }

    // 回復トレーナー位置を設定(1階のみ設定)
    {
      s32 i,n;
      GFL_STD_MemFill(p_dungeon_data->recover_room_id, 0, sizeof(p_dungeon_data->recover_room_id));
      for( i = 0; i < ROOM_MAX ;i++ )
      {
        ROOM_DATA *p_room_data = &p_dungeon_data->room_data[0][i];
        for( n= 0; n<INF_TOOL_GetMemberNum(p_room_data->tr_pos_type); n++ )
        {
          u16 tr_id;
          s32 id_pos;
          id_pos = GetRoomTrainerIDPos( p_dungeon_data,0, p_room_data->room_id ,n );
          tr_id = p_dungeon_data->tr_data[0].tr_id[id_pos];
          if( (tr_id >= INF_TRID_START_RECOVERY) 
              && (tr_id < (INF_TRID_START_RECOVERY+INF_TRID_CNT_RECOVERY)) )
          {
            p_dungeon_data->recover_room_id[0] = p_room_data->room_id;
          }
        }
      }
    }

    // ゲートトレーナーを設定する
    if( tutorial_data_index == 0 )
    {
      p_dungeon_data->gate_trainer_floor    = INF_TUTORIAL_FLOOR_NO_AREA_1;
      p_dungeon_data->gate_trainer_room_id  = INF_TUTORIAL_ROOM_ID_AREA_1;
      p_dungeon_data->gate_trainer_id_pos   = INF_TUTORIAL_TR_INDEX_AREA_1;
    }
    else
    {
      p_dungeon_data->gate_trainer_floor    = INF_TUTORIAL_FLOOR_NO_AREA_2;
      p_dungeon_data->gate_trainer_room_id  = INF_TUTORIAL_ROOM_ID_AREA_2;
      p_dungeon_data->gate_trainer_id_pos   = INF_TUTORIAL_TR_INDEX_AREA_2;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス部屋の存在するフロアを返す
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetBossFloor( const INF_DUNGEON_DATA *p_dungeon_data )
{
  s32 boss_room;
  s32 boss_floor;

  // ボスの存在するフロアを検索する
  for( boss_floor = 0; boss_floor<INFINITYDUNGEON_FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( p_dungeon_data->room_data[boss_floor][boss_room].room_type == ROOM_TYPE_BOSS )
      {
        return boss_floor;
      }
    }
  }

  GF_ASSERT_MSG(0,"ボス部屋が見つかりません");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス部屋の存在する部屋IDを返す
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 INFINITY_DUNGEON_GetBossRoom( const INF_DUNGEON_DATA *p_dungeon_data )
{
  s32 boss_room;
  s32 boss_floor;

  // ボスの存在するフロアを検索する
  for( boss_floor = 0; boss_floor<INFINITYDUNGEON_FLOOR_MAX; boss_floor++ )
  {
    for( boss_room = 0; boss_room<ROOM_MAX; boss_room++ )
    {
      if( p_dungeon_data->room_data[boss_floor][boss_room].room_type == ROOM_TYPE_BOSS )
      {
        return boss_room;
      }
    }
  }

  GF_ASSERT_MSG(0,"ボス部屋が見つかりません");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージを取得する
 *
 *	@param	*p_save 
 *	@param	*p_dungeon_data 
 *	@param	obj_code
 *	@param	id_pos
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetHintMessage( const INFINITYDUNGEON_SAVE *p_save ,const INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code,u8 id_pos)
{
  u8    talk_type;
  u8    hint_type;
  int   sub_str_num = 0;
  u32   ret_msg_id;
  BOOL  is_get_msg_id = FALSE;
  TRHINT_WK trhint_wk;
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_save );

  // ヒントメッセージ取得プログラム
  HINTMSGPROC hint_msg_proc_table[ INFTR_HINT_TYPE_MAX ] =
  {
    gethint_gate_tr_dir,
    gethint_gate_tr_is_up_down,
    gethint_gate_tr_room_tr_num,
    gethint_gate_tr_sex,
    gethint_gate_tr_move_type,
    gethint_medic_dir,
    gethint_boss_monster,
    gethint_boss_info,
    gethint_no_hint,
  };

  // OBJコードから会話パターンを選択
  talk_type = INF_TOOL_GetTrainerTalkType( obj_code );
  hint_type = INF_TOOL_GetTrainerHintType( p_dungeon_data, obj_code ,now_floor,id_pos);

  // ヒント取得ワークにパラメータを設定する
  {
    trhint_wk.p_dungeon_data = p_dungeon_data;
    trhint_wk.talk_type = talk_type;
    trhint_wk.p_save = p_save;
  }

  // ヒントメッセージ取得
  return hint_msg_proc_table[hint_type]( &trhint_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーの戦闘前メッセージを返す
 *
 *	@param	INFINITYDUNGEON_SAVE *p_save 
 *	@param	INF_DUNGEON_DATA *p_dungeon_data 
 *	@param	obj_code
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetGateBeforeBattleMessage( const INFINITYDUNGEON_SAVE *p_save ,const INF_DUNGEON_DATA *p_dungeon_data ,u16 obj_code ,BOOL is_before_battle)
{
  u8    talk_type;
  int   sub_str_num = 0;
  u32   ret_msg_id;
  BOOL  is_get_msg_id = FALSE;
  u8    now_floor;
  TRHINT_WK trhint_wk;

  // OBJコードから会話パターンを選択
  talk_type = INF_TOOL_GetTrainerTalkType( obj_code );

  if( is_before_battle == TRUE )
  {
    // 戦闘前メッセージを返す
    return msg_gate_start_01 + talk_type;
  }
  else
  {
    // 戦闘後メッセージを返す
    return msg_gate_end_01 + talk_type;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：上にいるかどうか
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_gate_tr_is_up_down( const TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // ボス部屋の存在するフロア
  u8 boss_floor = INFINITY_DUNGEON_GetBossFloor( p_wk->p_dungeon_data );

  // メッセージを取得
  u32 msg_id = msg_hint_updown_unknown_01;

  ONOUE_Printf("現在のフロア：%u,ボスのフロア%u\n",now_floor ,boss_floor );
  ONOUE_Printf("ヒント：上にいるか->");

  if( now_floor < boss_floor )
  {
    if( INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data ) == VERSION_B2 )
    {
      ONOUE_Printf("上にいる\n");
      msg_id = msg_hint_up_01;
    }
    else
    {
      ONOUE_Printf("ホワイトなので下にいる\n");
      msg_id = msg_hint_down_01;
    }
  }
  else if( now_floor > boss_floor )
  {
    if( INFINITY_DUNGEON_GetDungeonColor( p_wk->p_dungeon_data ) == VERSION_B2 )
    {
      ONOUE_Printf("下にいる\n");
      msg_id = msg_hint_down_01;
    }
    else
    {
      ONOUE_Printf("ホワイトなので上にいる\n");
      msg_id = msg_hint_up_01;
    }
  }
  else
  {
    ONOUE_Printf("同じ階にいる\n");
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスが何階にいるか
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_floor( const TRHINT_WK *p_wk )
{
  ONOUE_Printf("ヒント：どのフロアにいるか\n");
  return msg_hint_boss_floor_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスの方向
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_dir( const TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // ボス部屋の存在するフロア
  u8 boss_floor = INFINITY_DUNGEON_GetBossFloor( p_wk->p_dungeon_data );
  
  // ボスのいる部屋ID
  u8 boss_room = INFINITY_DUNGEON_GetBossRoom( p_wk->p_dungeon_data );
  // 主人公のいる部屋ID
  u8 now_room = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );

  // メッセージを取得
  u32 msg_id = msg_hint_dir_unknown_01;

  ONOUE_Printf("現在のフロア：%u,ボスのフロア%u\n",now_floor ,boss_floor );
  ONOUE_Printf("現在の部屋番号：%u,ボスの部屋番号%u\n",now_floor ,boss_room );
  if( now_floor == boss_floor )
  {
    u8 dir;
    u8 x1 = 0;
    u8 y1 = 0;
    u8 x2 = 0;
    u8 y2 = 0;
    if( now_room != 0 )
    {
      x1  = now_room % ROOM_MAX_X;
      y1  = now_room / ROOM_MAX_X;
    }
    if( boss_room != 0 )
    {
      x2  = boss_room % ROOM_MAX_X;
      y2  = boss_room / ROOM_MAX_X;
    }
    dir = GetPosDirID( x1, y1, x2, y2 );
    switch( dir )
    {
    case INFTR_DIR_U:  // 上
      msg_id = msg_hint_boss_dir_u_01;
      break;
    case INFTR_DIR_D:  // 下
      msg_id = msg_hint_boss_dir_d_01;
      break;
    case INFTR_DIR_L:  // 左
      msg_id = msg_hint_boss_dir_l_01;
      break;
    case INFTR_DIR_R:  // 右
      msg_id = msg_hint_boss_dir_r_01;
      break;
    case INFTR_DIR_LU:  // 上左
      msg_id = msg_hint_boss_dir_lu_01;
      break;
    case INFTR_DIR_RU:  // 上右
      msg_id = msg_hint_boss_dir_ru_01;
      break;
    case INFTR_DIR_LD:  // 下左
      msg_id = msg_hint_boss_dir_ld_01;
      break;
    case INFTR_DIR_RD:  // 下右
      msg_id = msg_hint_boss_dir_rd_01;
      break;
    default:
      GF_ASSERT(0);
      msg_id = msg_hint_boss_dir_u_01;
      break;
    }
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  2つの位置より、方向を導いて返す
 *
 *	@param	x1  基準X
 *	@param	y1  基準Y
 *	@param	x2  目標X
 *	@param	y2  目標Y
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetPosDirID( u8 x1, u8 y1, u8 x2 ,u8 y2 )
{
  if( x1 == x2 )
  {
    // 上下
    if( y1 > y2 )
    {
      return INFTR_DIR_U;
    }
    else
    {
      return INFTR_DIR_D;
    }
  }
  else if( y1 == y2 )
  {
    // 左右
    if( x1 < x2 )
    {
      return INFTR_DIR_R;
    }
    else
    {
      return INFTR_DIR_L;
    }
  }
  else
  {
    if( y1 > y2 )
    {
      // 上＋○
      if( x1 < x2 )
      {
        return INFTR_DIR_RU;
      }
      else
      {
        return INFTR_DIR_LU;
      }
    }
    else
    {
      // 下＋○
      if( x1 < x2 )
      {
        return INFTR_DIR_RD;
      }
      else
      {
        return INFTR_DIR_LD;
      }
    }
  }

  GF_ASSERT(0);
  return INFTR_DIR_U;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ボスのポケモン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_monster( const TRHINT_WK *p_wk )
{
  ONOUE_Printf("ヒント：ボスのポケモン\n");
  return msg_hint_boss_poke_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ドクター・ナースの方向
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_medic_dir( const TRHINT_WK *p_wk )
{
  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // 主人公のいる部屋ID
  u8 now_room = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );

  // 回復トレーナーのいる部屋ID
  u8 medic_room = p_wk->p_dungeon_data->recover_room_id[now_floor];

  // メッセージを取得
  u32 msg_id = msg_hint_dir_unknown_01;
  u8 dir;
  u8 x1 = 0;
  u8 y1 = 0;
  u8 x2 = 0;
  u8 y2 = 0;

  ONOUE_Printf("現在のフロア：%u,回復トレーナーのフロア%u\n",now_floor ,now_floor );
  ONOUE_Printf("現在の部屋番号：%u,回復トレーナーの部屋番号%u\n",now_floor ,medic_room );

  // 同じ部屋にいる場合は、専用メッセージ
  if( now_room == medic_room )
  {
    // 同じ部屋にいるときはとぼける
    return msg_hint_unknown_01 + p_wk->talk_type;
  }
 
  if( now_room != 0 )
  {
    x1  = now_room % ROOM_MAX_X;
    y1  = now_room / ROOM_MAX_X;
  }
  if( medic_room != 0 )
  {
    x2  = medic_room % ROOM_MAX_X;
    y2  = medic_room / ROOM_MAX_X;
  }

  GF_ASSERT( medic_room != ROOM_MAX );

  dir = GetPosDirID( x1, y1, x2, y2 );
  switch( dir )
  {
  case INFTR_DIR_U:  // 上
    msg_id = msg_hint_nurse_dir_u_01;
    break;
  case INFTR_DIR_D:  // 下
    msg_id = msg_hint_nurse_dir_d_01;
    break;
  case INFTR_DIR_L:  // 左
    msg_id = msg_hint_nurse_dir_l_01;
    break;
  case INFTR_DIR_R:  // 右
    msg_id = msg_hint_nurse_dir_r_01;
    break;
  case INFTR_DIR_LU:  // 上左
    msg_id = msg_hint_nurse_dir_lu_01;
    break;
  case INFTR_DIR_RU:  // 上右
    msg_id = msg_hint_nurse_dir_ru_01;
    break;
  case INFTR_DIR_LD:  // 下左
    msg_id = msg_hint_nurse_dir_ld_01;
    break;
  case INFTR_DIR_RD:  // 下右
    msg_id = msg_hint_nurse_dir_rd_01;
    break;
  default:
    GF_ASSERT(0);
    msg_id = msg_hint_nurse_dir_u_01;
    break;
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーの方向ヒント
 *
 *	@param	TRHINT_WK *p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_gate_tr_dir( const TRHINT_WK *p_wk )
{
  s32 i;

  // 現在のフロア
  u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
  // 主人公のいる部屋ID
  u8 now_room = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );

  // ゲートトレーナーのいる部屋ID
  u8 gate_tr_room = p_wk->p_dungeon_data->gate_trainer_room_id;

  // メッセージを取得
  u32 msg_id = msg_hint_dir_unknown_01;
  u8 dir;
  u8 x1 = 0;
  u8 y1 = 0;
  u8 x2 = 0;
  u8 y2 = 0;

  // 部屋データ取得
  const ROOM_DATA *p_room_data = p_wk->p_dungeon_data->room_data[now_floor];


  ONOUE_Printf("現在のフロア：%u,ゲートトレーナーのフロア%u\n",now_floor ,now_floor );
  ONOUE_Printf("現在の部屋番号：%u,ゲートトレーナーの部屋番号%u\n",now_floor ,gate_tr_room );

  // 違う階にいるときはとぼける
  if( p_wk->p_dungeon_data->gate_trainer_floor != now_floor )
  {
    // 同じ部屋にいるときはとぼける
    return msg_hint_unknown_01 + p_wk->talk_type;
  }

  // 同じ部屋にいる場合は、専用メッセージ
  if( now_room == gate_tr_room )
  {
    // 同じ部屋にいるときはとぼける
    return msg_hint_unknown_01 + p_wk->talk_type;
  }
 
  if( now_room != 0 )
  {
    x1  = now_room % ROOM_MAX_X;
    y1  = now_room / ROOM_MAX_X;
  }
  if( gate_tr_room != 0 )
  {
    x2  = gate_tr_room % ROOM_MAX_X;
    y2  = gate_tr_room / ROOM_MAX_X;
  }

  GF_ASSERT( gate_tr_room != ROOM_MAX );

  dir = GetPosDirID( x1, y1, x2, y2 );
  switch( dir )
  {
  case INFTR_DIR_U:  // 上
    msg_id = msg_hint_gate_dir_u_01;
    break;
  case INFTR_DIR_D:  // 下
    msg_id = msg_hint_gate_dir_d_01;
    break;
  case INFTR_DIR_L:  // 左
    msg_id = msg_hint_gate_dir_l_01;
    break;
  case INFTR_DIR_R:  // 右
    msg_id = msg_hint_gate_dir_r_01;
    break;
  case INFTR_DIR_LU:  // 上左
    msg_id = msg_hint_gate_dir_lu_01;
    break;
  case INFTR_DIR_RU:  // 上右
    msg_id = msg_hint_gate_dir_ru_01;
    break;
  case INFTR_DIR_LD:  // 下左
    msg_id = msg_hint_gate_dir_ld_01;
    break;
  case INFTR_DIR_RD:  // 下右
    msg_id = msg_hint_gate_dir_rd_01;
    break;
  default:
    GF_ASSERT(0);
    msg_id = msg_hint_gate_dir_u_01;
    break;
  }

  return msg_id + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーがいる部屋の人数
 *
 *	@param	TRHINT_WK *p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_gate_tr_room_tr_num( const TRHINT_WK *p_wk )
{
  s32 room_id = p_wk->p_dungeon_data->gate_trainer_room_id;
  s32 floor_id = p_wk->p_dungeon_data->gate_trainer_floor;
  u8 member_num = INF_TOOL_GetMemberNum(p_wk->p_dungeon_data->room_data[floor_id][room_id].tr_pos_type);

  // 0人のはありえない
  GF_ASSERT( member_num > 0 );
  // 4人以上はありえない
  GF_ASSERT( member_num < 4 );
  return msg_hint_gate_room_one_01 + (member_num-1);

}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーの性別
 *
 *	@param	TRHINT_WK *p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_gate_tr_sex( const TRHINT_WK *p_wk )
{
  // トレーナータイプ取得
  u8 floor_id = p_wk->p_dungeon_data->gate_trainer_floor;
  u8 id_pos = p_wk->p_dungeon_data->gate_trainer_id_pos;
  u16 tr_id = INFINITY_DUNGEON_GetTrIDByIDPos( p_wk->p_dungeon_data ,floor_id, id_pos );
  
	u16	tr_type;		//トレーナータイプ
  BSUBWAY_PARTNER_DATA tr_data;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
 
  //トレーナーデータセット
  trd = FBI_TOOL_AllocTrainerRomData(&tr_data ,ARCID_INFINITY_DUGENON_TR ,tr_id,NARC_message_infinity_trname_dat ,HEAPID_FIELDMAP);
  
  // トレーナータイプ取得
  tr_type = trd->tr_type;

  // 開放
  GFL_HEAP_FreeMemory( trd );

  if( TT_TrainerTypeSexGet( tr_type ) == PTL_SEX_MALE )
  {
    // 男
    return msg_hint_gate_gender_man_01 + p_wk->talk_type;
  }
  else
  {
    // 女
    return msg_hint_gate_gender_woman_01 + p_wk->talk_type;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーの移動方法
 *
 *	@param	TRHINT_WK *p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_gate_tr_move_type( const TRHINT_WK *p_wk )
{
  int n;
  u8 floor_id = p_wk->p_dungeon_data->gate_trainer_floor;
  u8 room_id = p_wk->p_dungeon_data->gate_trainer_room_id;
  const ROOM_DATA *p_room_data=&p_wk->p_dungeon_data->room_data[floor_id][room_id];
  const INF_ROOM_SET_TR_PARAM *p_tr_room;
  p_tr_room = INF_TOOL_GetRoomTrParam(p_room_data->tr_pos_type);


  // ゲートトレーナーの位置を調べる
  for( n= 0; n<INF_TOOL_GetMemberNum(p_room_data->tr_pos_type); n++ )
  {
    // 部屋のどの位置か調べる
    s32 id_pos = GetRoomTrainerIDPos( p_wk->p_dungeon_data ,floor_id ,room_id ,n );
    BOOL is_gate = INF_TOOL_DUNGEON_GetIsGateTrainer( p_wk->p_dungeon_data ,floor_id,id_pos );

    // ゲートトレーナーが見つかった場合
    if( is_gate == TRUE )
    {
      const INF_ROOM_TR_DATA *p_md = &p_tr_room->tr_move_data[n];
      const INF_MMDL_PARAM *p_mmdlp = INF_TOOL_GetMMDLParam(p_md->move_type);
      return p_mmdlp->hint_msg_id + p_wk->talk_type;
    }
  }

  // 発見できなかったときはとぼける
  GF_ASSERT_MSG(0,"ゲートトレーナーの移動タイプが発見できなかった\n");
  return msg_hint_unknown_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボス情報ヒント
 *
 *	@param	TRHINT_WK *p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_boss_info( const TRHINT_WK *p_wk )
{
  u8 now_area = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_wk->p_save );

  // 現在のエリアの階数によって分岐
  if( INFINITYDUNGEON_SAVE_GetFloorNum( now_area ) <= 1 )
  {
    // ボスの方向
    return gethint_boss_dir( p_wk );
  }
  else
  {
    // ボスのフロア
    return gethint_boss_floor( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメッセージ：ヒントなし
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 gethint_no_hint( const TRHINT_WK *p_wk )
{
  return msg_hint_unknown_01 + p_wk->talk_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのIDの場所からトレーナーIDを取得
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 INFINITY_DUNGEON_GetTrIDByIDPos( const INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u8 id_pos )
{
  return p_dungeon_data->tr_data[floor_id].tr_id[id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレーナーごとのシードを取得する
 *
 *	@param	*p_dungeon_data 
 *	@param	floor_id 
 *	@param	tr_id_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetTrSeed( const INF_DUNGEON_DATA *p_dungeon_data ,u8 floor_id ,u32 tr_id_pos )
{
  return p_dungeon_data->seed.tr_seed[floor_id][tr_id_pos];
}

//----------------------------------------------------------------------------
/**
 *	@brief  回復トレーナーの回復フラグを取得
 *
 *	@param	*p_dungeon_data 
 *	@param	tr_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_GetRecoverFlgPos( const INF_DUNGEON_DATA *p_dungeon_data ,u16 tr_id )
{
  u32 i;
  u32 master_seed;
  const u16 *p_trid;

  // トレーナーIDリストを取得
  p_trid = p_dungeon_data->recover_data;

  // 検索
  for( i= 0; i<INFINITY_DUNGEON_RECOVERTR_MAX; i++ )
  {
    ONOUE_Printf("trid =%d\n",p_trid[i]);
    if( p_trid[i] == tr_id )
    {
      return i;
    }
  }

  GF_ASSERT_MSG(0,"ID位置が発見できませんでした");
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータでの移動か調べる
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEの時エレベータでの移動
 */
//-----------------------------------------------------------------------------
static BOOL IsMoveElevator( const MAP_CHAGE_EVENT_WORK *p_wk )
{
  if( (p_wk->map_change_type == MAP_CHAGE_TYPE_FOOR) || (p_wk->warp_dir == WARP_INFORMATION) )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン内のトレーナー人数を調べる
 *
 *	@param	*p_dungeon_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 INFINITY_DUNGEON_TrainerCount( const INF_DUNGEON_DATA *p_dungeon_data )
{
  u32 member_num =0;
  int i,n;
  for( i = 0; i < INFINITYDUNGEON_FLOOR_MAX ;i++ )
  {
    for( n = 0; n < ROOM_MAX ;n++ )
    {
      u8 tr_pos_type = p_dungeon_data->room_data[i][n].tr_pos_type;
      if( tr_pos_type != INF_ROOM_POS_TYPE_NONE )
      {
          member_num += INF_TOOL_GetMemberNum(tr_pos_type);
      }
    }
  }

  ONOUE_Printf("無限ダンジョン内トレーナーの数を取得：%u\n",member_num);
  return member_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  一つ上の部屋タイプを取得す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL INFINITY_DUNGEON_GetUpPosRoomTypeIsBoss( const INF_DUNGEON_DATA *p_dungeon_data ,s32 now_floor ,s32 now_room )
{
  const ROOM_DATA *p_room;
  // 一つ上の部屋番号を取得
  s32 next_room = now_room - ROOM_MAX_X;

  if( now_room < 0 )
  {
    return FALSE;
  }

  p_room = &p_dungeon_data->room_data[ now_floor ][ next_room ];

  if( p_room->room_type == ROOM_TYPE_BOSS )
  {
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief ボス部屋に通じるゲートが開いているか調べる
 *
 *	@param	INF_DUNGEON_DATA *p_dungeon_data 
 *	@param	INFINITYDUNGEON_SAVE *p_save 
 *
 *	@retval TRUEの時に開いている
 */
//-----------------------------------------------------------------------------
BOOL INFINITY_DUNGEON_GetGateOpen( const INF_DUNGEON_DATA *p_dungeon_data ,const INFINITYDUNGEON_SAVE *p_save )
{
  s32 tr_id_pos = p_dungeon_data->gate_trainer_id_pos;
  s32 floor_id  = p_dungeon_data->gate_trainer_floor;
  return INFINITYDUNGEON_SAVE_GetTrainerFlag( p_save ,floor_id ,tr_id_pos);
}

#ifdef PM_DEBUG

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ文字列描画
 *
 *	@param	*debug 
 *	@param	*p_debug_draw_pos
 *	@param	*p_debug_draw_line
 *	@param	*str
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DDebugStr( FIELD_DEBUG_WORK *debug ,s32 *p_debug_draw_pos,s32 *p_debug_draw_line,const char *str )
{
  if( g_debug_inf_is_map_info == TRUE )
  {
    FIELD_DEBUG_DebugFont_Print(debug ,*p_debug_draw_pos,*p_debug_draw_line,str);

    // 文字数を数える
    while( (*str) != '\0' ){ (*p_debug_draw_pos)++; str++; };
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フロアデータのデバッグ出力
 *
 *	@param	*p_room_data 
 *	@param	p_save 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INF_DEBUG_OutPutFloorData( const INF_DUNGEON_DATA *p_wk,const ROOM_DATA *p_room_data ,INFINITYDUNGEON_SAVE *p_save ,FIELDMAP_WORK *p_field_work )
{
  int i,j,k,n;
  int draw;
  s32 dd_pos = 0;
  s32 dd_line = 0;
  char str_buf[256];
  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( p_field_work );

  if( g_debug_inf_is_map_info == TRUE )
  {
    FIELD_DEBUG_DebugFont_Print(debug ,0,0,"TEST");
  }
  
  OS_TFPrintf( 3,"\n");

  for( i = 0; i<ROOM_MAX_Y; i++ )
  {
    for( draw = 0 ; draw <= 4; draw++ )
    {
      // デバッグ表示を消す
      if( g_debug_inf_is_map_info == TRUE )
      {
        FIELD_DEBUG_DebugFont_ClearLine(debug,dd_line);
      }

      for( j = 0; j<ROOM_MAX_X; j++ )
      {
        int room_id = i*ROOM_MAX_X + j;
        switch( draw )
        {
          // 上の列
        case 0:
          {
            OS_TFPrintf( 3,"■■");
            DDebugStr(debug,&dd_pos,&dd_line,"OO");
            if( p_room_data[room_id].can_move[INF_MOVE_UP] == TRUE )
            {
              OS_TFPrintf( 3,"　");
              DDebugStr(debug,&dd_pos,&dd_line," ");
            }
            else
            {
              OS_TFPrintf( 3,"■");
              DDebugStr(debug,&dd_pos,&dd_line,"O");
            }

            OS_TFPrintf( 3,"■■");
            DDebugStr(debug,&dd_pos,&dd_line,"OO");
          }
          break;
        case 1:
          {
            BOOL is_gate = FALSE;
            u8 now_floor = INFINITYDUNGEON_SAVE_GetFloorID( p_save );

            // ゲートトレーナーがいるか調べる
            for( n= 0; n<INF_TOOL_GetMemberNum(p_room_data[room_id].tr_pos_type); n++ )
            {
              s32 id_pos = GetRoomTrainerIDPos( p_wk ,now_floor ,room_id ,n );
              is_gate = INF_TOOL_DUNGEON_GetIsGateTrainer( p_wk ,now_floor,id_pos );
              if( is_gate == TRUE ){break;}
            }

            if( p_room_data[room_id].room_type == ROOM_TYPE_BOSS)
            {
              OS_TFPrintf( 3,"■Ｂ　　■");
              DDebugStr(debug,&dd_pos,&dd_line,"OB  O");
            }
            else if( p_room_data[room_id].room_type == ROOM_TYPE_ELEVATOR)
            {
              OS_TFPrintf( 3,"■Ｅ　　■");
              DDebugStr(debug,&dd_pos,&dd_line,"OE  O");
            }
            else if( is_gate == TRUE )
            {
              OS_TFPrintf( 3,"■Ｇ　　■");
              DDebugStr(debug,&dd_pos,&dd_line,"OG  O");
            }
            else
            {
              OS_TFPrintf( 3,"■　　　■");
              DDebugStr(debug,&dd_pos,&dd_line,"O   O");
            }
          }
          break;
          // 真ん中の列
        case 2:
          {
            u8 now_room_id = INFINITYDUNGEON_SAVE_GetRoomID( p_save );
            if( p_room_data[room_id].can_move[INF_MOVE_LEFT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
              DDebugStr(debug,&dd_pos,&dd_line," ");
            }
            else
            {
              OS_TFPrintf( 3,"■");
              DDebugStr(debug,&dd_pos,&dd_line,"O");
            }

            // 現在地表示
            if( room_id == now_room_id )
            {
              OS_TFPrintf( 3,"　Ｐ　"); //部屋の中心
              DDebugStr(debug,&dd_pos,&dd_line," P ");
            }
            else
            {
              OS_TFPrintf( 3,"　　　"); //部屋の中心
              DDebugStr(debug,&dd_pos,&dd_line,"   ");
            }

            if( p_room_data[room_id].can_move[INF_MOVE_RIGHT] == TRUE )
            {
              OS_TFPrintf( 3,"　");
              DDebugStr(debug,&dd_pos,&dd_line," ");
            }
            else
            {
              OS_TFPrintf( 3,"■");
              DDebugStr(debug,&dd_pos,&dd_line,"O");
            }
          }
          break;
        case 3:
          {
            if( INF_TOOL_GetMemberNum(p_room_data[room_id].tr_pos_type) > 0 )
            {
              OS_TFPrintf( 3,"■%2d　%2d■",room_id,p_room_data[room_id].tr_pos_type);
              sprintf( str_buf,"O%2d O",room_id);
              DDebugStr(debug,&dd_pos,&dd_line,str_buf);
            }
            else
            {
              OS_TFPrintf( 3,"■%2d　×■",room_id,p_room_data[room_id].tr_pos_type);
              sprintf( str_buf,"O%2dxO",room_id);
              DDebugStr(debug,&dd_pos,&dd_line,str_buf);
            }
          }
          break;
          // 下の列
        case 4:
          {
            OS_TFPrintf( 3,"■■");
            DDebugStr(debug,&dd_pos,&dd_line,"OO");
            if( p_room_data[room_id].can_move[INF_MOVE_DOWN] == TRUE )
            {
              OS_TFPrintf( 3,"　");
              DDebugStr(debug,&dd_pos,&dd_line," ");
            }
            else
            {
              OS_TFPrintf( 3,"■");
              DDebugStr(debug,&dd_pos,&dd_line,"O");
            }

            OS_TFPrintf( 3,"■■");
            DDebugStr(debug,&dd_pos,&dd_line,"OO");
          }
          break;
        }
      }
      OS_TFPrintf( 3,"\n");
      dd_pos=0;
      dd_line++;
    }
  }
}


#endif
