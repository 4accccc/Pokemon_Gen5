//////////////////////////////////////////////////////////////////////////
/**
 * @brief  全接続チェックイベント
 * @file   event_debug_all_connect_check.c
 * @author obata
 * @date   2010.04.21
 */
//////////////////////////////////////////////////////////////////////////
#ifdef PM_DEBUG
#include <gflib.h>
#include "system/main.h" // for HEAPID_xxxx
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"
#include "sound/wb_sound_data.sadl" // for SEQ_ME_BADGE
#include "sound/pm_sndsys.h" // for PMSND_xxxx

#include "fieldmap.h"
#include "event_debug_all_connect_check.h"
#include "event_mapchange.h"
#include "eventdata_local.h"
#include "field_sound.h"

#include "../../../resource/fldmapdata/zonetable/zone_id.h" // for ZONE_ID_xxxx


// デバッグ出力先
#define PRINT_TARGET (3)


//========================================================================
// ■チェック対象外のゾーンID
//========================================================================
static const u16 UncheckZoneList[] = 
{
  ZONE_ID_UNION,
  ZONE_ID_CLOSSEUM,
  ZONE_ID_CLOSSEUM02,
  ZONE_ID_PLD10,
  ZONE_ID_C04R0202, // ミュージカル: 正しい手順を経て入室する必要がある
  ZONE_ID_BC10,     // ブラックシティ
  ZONE_ID_WC10,     // ホワイトフォレスト
  ZONE_ID_C09P01,   // リーグフロント四天王分岐部屋 ( INIT_LABEL でスクリプトを予約するため )
};

//========================================================================
// ■イベント動作モード
//========================================================================
typedef enum {
	EVMODE_ONE_CHECK, // 現在のゾーンのみをチェック
	EVMODE_ALL_CHECK, // すべてのゾーンをチェック
} EVMODE;


//========================================================================
// ■イベントワーク
//========================================================================
typedef struct {

  GAMESYS_WORK*     gameSystem;
  GAMEDATA*         gameData;
  EVENTDATA_SYSTEM* eventDataSystem;
	EVMODE mode; // 動作モード
  u32 start_zone_id; // スタート地点のゾーンID

  // チェックデータ
  u16 check_zone_id; // チェック対象のゾーンID
  u32 check_exit_id; // チェック対象の出入り口ID
  u16 check_offset;  // チェック対象のオフセット

} CHECK_WORK;


//========================================================================
// ■prototype
//========================================================================
static GMEVENT_RESULT AllConnectCheckEvent( GMEVENT* event, int* seq, void* wk ); // 全接続チェックイベント
static u16 GetCurrentZoneID( const CHECK_WORK* work ); // 現在のゾーンIDを取得する
static u32 GetConnectNum( const CHECK_WORK* work ); // 接続データの数を取得する
static BOOL CheckGridMap( const CHECK_WORK* work ); // グリッドマップかどうかをチェックする
// マップチェンジ
static GMEVENT* GetMapChangeEvent_Connect( const CHECK_WORK* work ); // 接続によるマップチェンジイベントを取得する
static GMEVENT* GetMapChangeEvent_Return( const CHECK_WORK* work ); // チェック対象ゾーンに戻るマップチェンジイベントを取得する
// チェック対象の接続データ
static const CONNECT_DATA* GetConnectData( const CHECK_WORK* work ); // チェック対象の接続データを取得する 
static LOC_EXIT_OFS GetExitOffset( const CHECK_WORK* work ); // チェック対象のロケーションオフセットを取得する
static u16 GetConnectOffsetSize( const CHECK_WORK* work ); // チェック対象のオフセット最大値を取得する
static u16 GetConnectOffsetSize_GRID( const CHECK_WORK* work ); // チェック対象のオフセット最大値を取得する ( グリッドマップ用 )
static u16 GetConnectOffsetSize_RAIL( const CHECK_WORK* work ); // チェック対象のオフセット最大値を取得する ( レールマップ用 )
static u16 GetConnectZoneID( const CHECK_WORK* work ); // チェック対象の接続先ゾーンIDを取得する
static BOOL CheckSpecialConnect( const CHECK_WORK* work ); // チェック対象の出入り口が特殊接続出入り口かどうかを判定する
static BOOL CheckClosedConnect( const CHECK_WORK* work ); // チェック対象の出入り口が無効出入り口かどうかをチェックする
// チェック対象の更新
static void UpdateCheckZoneID( CHECK_WORK* work ); // チェック対象のゾーンIDを更新する
static void UpdateCheckExitID( CHECK_WORK* work ); // チェック対象の出入り口IDを更新する
static void UpdateCheckOffset( CHECK_WORK* work ); // チェック対象のオフセットを更新する
static void ResetCheckExitID( CHECK_WORK* work ); // チェック対象の出入り口IDをリセットする
static void ResetCheckOffset( CHECK_WORK* work ); // チェック対象のオフセットをリセットする
static u16 GetCheckZoneID( const CHECK_WORK* work ); // チェック対象のゾーンIDを取得する
static u32 GetCheckExitID( const CHECK_WORK* work ); // チェック対象の出入り口IDを取得する
static u16 GetCheckOffset( const CHECK_WORK* work ); // チェック対象のオフセットを取得する
// ユーティリティ
static u16 ConnectData_GetExitDirToDir( const CONNECT_DATA * cp_data ); // EXIT_DIR_xxxx を DIR_xxxx に変換する
static BOOL CheckValidZone( u16 zone_id ); // チェックが有効なゾーンかどうかを判定する
// デバッグ出力
static void DebugPrint_Start( const CHECK_WORK* work );
static void DebugPrint_AllConnect( const CHECK_WORK* work );
static void DebugPrint_CheckConnect( const CHECK_WORK* work );
static void DebugPrint_DetectSpecialConnect( const CHECK_WORK* work );
static void DebugPrint_Finish( const CHECK_WORK* work );
// ヒープ残量チェック
static void HeapFreeSizeCheck( HEAPID heap_id, u32 check_size );


//========================================================================
// ■public functions
//========================================================================

//------------------------------------------------------------------------
/**
 * @brief 全接続チェックイベントを生成する
 *
 * @param gameSystem
 * @param wk
 *
 * @return 生成したイベント
 */
//------------------------------------------------------------------------
GMEVENT* EVENT_DEBUG_AllConnectCheck( GAMESYS_WORK* gameSystem, void* wk )
{
  GMEVENT* event;
  CHECK_WORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( 
      gameSystem, NULL, AllConnectCheckEvent, sizeof(CHECK_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(CHECK_WORK) );
  work->gameSystem      = gameSystem;
  work->gameData        = gameData;
	work->eventDataSystem = GAMEDATA_GetEventData( gameData );
	work->mode            = EVMODE_ALL_CHECK;

  return event;
} 

//------------------------------------------------------------------------
/**
 * @brief 現マップの接続チェックイベントを生成する
 *
 * @param gameSystem
 * @param wk
 *
 * @return 生成したイベント
 */
//------------------------------------------------------------------------
GMEVENT* EVENT_DEBUG_NowConnectCheck( GAMESYS_WORK* gameSystem, void* wk )
{
  GMEVENT* event;
  CHECK_WORK* work;
  GAMEDATA* gameData;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( 
      gameSystem, NULL, AllConnectCheckEvent, sizeof(CHECK_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(CHECK_WORK) );
  work->gameSystem      = gameSystem;
  work->gameData        = gameData;
	work->eventDataSystem = GAMEDATA_GetEventData( gameData );
	work->mode            = EVMODE_ONE_CHECK;

  return event;
} 

//========================================================================
// ■private functions
//========================================================================

// イベントのシーケンス番号
enum {
  CHECK_SEQ_INIT,           // イベント初期化
  CHECK_SEQ_SEARCH_CONNECT, // 接続データ検索
  CHECK_SEQ_JUMP_NEXT_ZONE, // 次のゾーンに飛ぶ
  CHECK_SEQ_NEW_ZONE_START, // 新しいゾーンのチェック開始
  CHECK_SEQ_CONNECT_TEST,   // 接続テスト
  CHECK_SEQ_RETURN,         // チェック対象ゾーンへ戻る
  CHECK_SEQ_TO_NEXT_CHECK,  // 次のチェックへ
  CHECK_SEQ_FINISH,         // イベント終了
};


//------------------------------------------------------------------------
/**
 * @brief 全接続チェックイベント
 */
//------------------------------------------------------------------------
static GMEVENT_RESULT AllConnectCheckEvent( GMEVENT* event, int* seq, void* wk )
{
  CHECK_WORK*    work       = wk;
  GAMESYS_WORK*  gameSystem = work->gameSystem;
  GAMEDATA*      gameData   = work->gameData;
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );

  // ヒープ残量チェック
  HeapFreeSizeCheck( HEAPID_PROC, 0x8000 );

  switch( *seq ) {
  // イベント初期化
  case CHECK_SEQ_INIT:
    DebugPrint_Start( work );
    work->start_zone_id = GetCurrentZoneID( work ); // スタート地点を記憶
    work->check_zone_id = work->start_zone_id;
    *seq = CHECK_SEQ_NEW_ZONE_START;
    break;

  // 接続データ検索
  case CHECK_SEQ_SEARCH_CONNECT:
    // 全ての出入り口のチェックが完了
    if( GetConnectNum(work) <= work->check_exit_id ) {
			if( work->mode == EVMODE_ONE_CHECK ) {
				*seq = CHECK_SEQ_FINISH; // 終了
			}
			else {
				*seq = CHECK_SEQ_JUMP_NEXT_ZONE; // 次のゾーンへ
			}
    }
    else {
      *seq = CHECK_SEQ_CONNECT_TEST;
    }
    break;

  // 次のゾーンに飛ぶ
  case CHECK_SEQ_JUMP_NEXT_ZONE:
    ResetCheckOffset( work );
    ResetCheckExitID( work );
    UpdateCheckZoneID( work ); 

    // 一周
    if( GetCheckZoneID(work) == work->start_zone_id ) {
      *seq = CHECK_SEQ_FINISH;
    }
    // チェック対象ゾーン
    else if( CheckValidZone(GetCheckZoneID(work)) ) {
      GMEVENT_CallEvent( event, GetMapChangeEvent_Return(work) );
      *seq = CHECK_SEQ_NEW_ZONE_START;
    }
    // チェック対象外のゾーン
    else {
      *seq = CHECK_SEQ_JUMP_NEXT_ZONE; // 次のゾーンへ
    }
    break;

  // 新しいゾーンのチェック開始
  case CHECK_SEQ_NEW_ZONE_START:
    // チェック対象外のゾーンの場合
    if( CheckValidZone(GetCheckZoneID(work)) == FALSE ) {
      *seq = CHECK_SEQ_JUMP_NEXT_ZONE; // 次のゾーンへ
    }
    // 接続がない場合
    else if( GetConnectNum(work) == 0 ) {
      *seq = CHECK_SEQ_JUMP_NEXT_ZONE; // 次のゾーンへ
    }
    else {
      *seq = CHECK_SEQ_CONNECT_TEST;
    }
    DebugPrint_AllConnect( work );
    break; 

  // 接続テスト
  case CHECK_SEQ_CONNECT_TEST:
    DebugPrint_CheckConnect( work );

    // 特殊接続出入口の場合
    if( CheckSpecialConnect(work) ) {
      DebugPrint_DetectSpecialConnect( work );
      GF_ASSERT(0); // WB には存在しないはず!!
      *seq = CHECK_SEQ_TO_NEXT_CHECK;
    }
    // 無効出入口の場合
    else if( CheckClosedConnect(work) ) {
      *seq = CHECK_SEQ_TO_NEXT_CHECK;
    }
    // 通常の出入り口の場合
    else {
      GMEVENT_CallEvent( event, GetMapChangeEvent_Connect(work) );
      *seq = CHECK_SEQ_RETURN;
    }
    break; 

  // チェック対象ゾーンへ戻る
  case CHECK_SEQ_RETURN:
    GMEVENT_CallEvent( event, GetMapChangeEvent_Return(work) );
    (*seq)++;
    break;

  // チェック対象更新処理
  case CHECK_SEQ_TO_NEXT_CHECK:
    UpdateCheckOffset( work );
    // 全てのオフセットのチェックが完了
    if( GetConnectOffsetSize(work) <= work->check_offset ) {
      // 次の出入り口へ
      ResetCheckOffset( work );
      UpdateCheckExitID( work ); 
    }
    *seq = CHECK_SEQ_SEARCH_CONNECT;
    break;

  // イベント終了
  case CHECK_SEQ_FINISH:
    DebugPrint_Finish( work );
		PMSND_PlaySE( SEQ_SE_DECIDE4 );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを取得する
 *
 * @return 現在のマップのゾーンID
 */
//------------------------------------------------------------------------
static u16 GetCurrentZoneID( const CHECK_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gameSystem;
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );

  return FIELDMAP_GetZoneID( fieldmap );
}

//------------------------------------------------------------------------
/**
 * @brief 接続データの数を取得する
 *
 * @param work
 *
 * @return 現在のマップに存在する接続データの数
 */
//------------------------------------------------------------------------
static u32 GetConnectNum( const CHECK_WORK* work )
{
  return EVENTDATA_GetConnectEventNum( work->eventDataSystem );
}

//------------------------------------------------------------------------
/**
 * @brief グリッドマップかどうかをチェックする
 *
 * @param work
 *
 * @return 現在のマップがグリッドマップなら TRUE
 *         そうでないなら FALSE
 */
//------------------------------------------------------------------------
static BOOL CheckGridMap( const CHECK_WORK* work )
{
  FIELDMAP_WORK* fieldmap;
  FLDMAP_BASESYS_TYPE type;

  fieldmap = GAMESYSTEM_GetFieldMapWork( work->gameSystem );
  type     = FIELDMAP_GetBaseSystemType( fieldmap );

  if( type == FLDMAP_BASESYS_GRID ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//------------------------------------------------------------------------
/**
 * @brief 接続によるマップチェンジイベントを取得する
 *
 * @param work
 *
 * @return チェック対象の接続データによるマップチェンジイベント
 */
//------------------------------------------------------------------------
static GMEVENT* GetMapChangeEvent_Connect( const CHECK_WORK* work )
{
  GMEVENT* event;
  FIELDMAP_WORK* fieldmap;
  const CONNECT_DATA* connect;
  LOC_EXIT_OFS offset;

  fieldmap = GAMESYSTEM_GetFieldMapWork( work->gameSystem );
  connect  = GetConnectData( work );
  offset   = GetExitOffset( work );

  event = EVENT_ChangeMapByConnect( work->gameSystem, fieldmap, connect, offset ); 
  return event;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象ゾーンに戻るマップチェンジイベントを取得する
 *
 * @param work
 *
 * @return チェック対象ゾーンに戻るマップチェンジイベント
 */
//------------------------------------------------------------------------
static GMEVENT* GetMapChangeEvent_Return( const CHECK_WORK* work )
{
  GAMESYS_WORK*  gameSystem = work->gameSystem;
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );

  GMEVENT* event;
  
  event = DEBUG_EVENT_QuickChangeMapDefaultPos( 
      gameSystem, fieldmap, work->check_zone_id );

  return event;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の接続データを取得する 
 *
 * @param work
 */
//------------------------------------------------------------------------
static const CONNECT_DATA* GetConnectData( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;

  connect = EVENTDATA_GetConnectByID( work->eventDataSystem, work->check_exit_id );
  return connect;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のロケーションオフセットを取得する
 *
 * @param work
 *
 * @return チェック対象のオフセットデータ
 */
//------------------------------------------------------------------------
static LOC_EXIT_OFS GetExitOffset( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;
  LOC_EXIT_OFS offset;
  u16 size, ofs, dir;

  connect = GetConnectData( work );
  size    = GetConnectOffsetSize( work );
  ofs     = GetCheckOffset( work );
  dir     = ConnectData_GetExitDirToDir( connect );

  offset = LOC_EXIT_OFS_DEF( size, ofs, dir );
  return offset;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセット最大値を取得する
 *
 * @param work
 *
 * @return チェック対象の接続データの最大オフセット値
 */
//------------------------------------------------------------------------
static u16 GetConnectOffsetSize( const CHECK_WORK* work )
{
  // グリッドマップの場合
  if( CheckGridMap(work) ) {
    return GetConnectOffsetSize_GRID( work );
  }
  // レールマップの場合
  else {
    return GetConnectOffsetSize_RAIL( work );
  }
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセット最大値を取得する ( グリッドマップ用 )
 *
 * @param work
 *
 * @return チェック対象の接続データの最大オフセット値
 */
//------------------------------------------------------------------------
static u16 GetConnectOffsetSize_GRID( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;
  const CONNECT_DATA_GPOS* gpos;
  u16 size;

  GF_ASSERT( CheckGridMap(work) );

  connect = GetConnectData( work );
  gpos    = (const CONNECT_DATA_GPOS*)(connect->pos_buf);

  if( 1 < gpos->sizex ) {
    size = gpos->sizex; // x 方向に幅がある
  }
  else if( 1 < gpos->sizez ) {
    size = gpos->sizez; // z 方向に幅がある
  }
  else {
    size = 1; // 幅なし
  } 

  return size;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセット最大値を取得する ( レールマップ用 )
 *
 * @param work
 *
 * @return チェック対象の接続データの最大オフセット値
 */
//------------------------------------------------------------------------
static u16 GetConnectOffsetSize_RAIL( const CHECK_WORK* work )
{ 
  const CONNECT_DATA* connect;
  const CONNECT_DATA_RPOS* rpos;
  u16 size;

  GF_ASSERT( CheckGridMap(work) == FALSE );
  
  connect = GetConnectData( work );
  rpos    = (const CONNECT_DATA_RPOS *)(connect->pos_buf);

  if (rpos->side_grid_size > 1 ) {
    size = rpos->side_grid_size;
  }
  else if (rpos->front_grid_size > 1 ) {
    size = rpos->front_grid_size;
  }
  else {
    size = 1;
  } 

  return size;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の接続先ゾーンIDを取得する
 *
 * @param work
 *
 * @return チェック対象の接続でつながっている場所のゾーンID
 */
//------------------------------------------------------------------------
static u16 GetConnectZoneID( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;

  connect = GetConnectData( work );
  return connect->link_zone_id;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の出入り口が特殊接続出入り口かどうかを判定する
 *
 * @param work
 *
 * @return チェック対象の出入り口が特殊接続出入口なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------
static BOOL CheckSpecialConnect( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;

  connect = GetConnectData( work );

  return CONNECTDATA_IsSpecialExit( connect );
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の出入り口が無効出入り口かどうかをチェックする
 *
 * @param work
 *
 * @return チェック対象の出入り口が無効出入り口なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------
static BOOL CheckClosedConnect( const CHECK_WORK* work )
{
  const CONNECT_DATA* connect;

  connect = GetConnectData( work );

  return CONNECTDATA_IsClosedExit( connect );
}


//------------------------------------------------------------------------
/**
 * @brief チェック対象のゾーンIDを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------
static void UpdateCheckZoneID( CHECK_WORK* work )
{
  work->check_zone_id++;

  if( ZONE_ID_MAX <= work->check_zone_id ) {
    work->check_zone_id = 0;
  }
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の出入り口IDを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------
static void UpdateCheckExitID( CHECK_WORK* work )
{
  work->check_exit_id++;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセットを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------
static void UpdateCheckOffset( CHECK_WORK* work )
{
  work->check_offset++;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の出入り口IDをリセットする
 *
 * @param work
 */
//------------------------------------------------------------------------
static void ResetCheckExitID( CHECK_WORK* work )
{
  work->check_exit_id = 0;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセットをリセットする
 *
 * @param work
 */
//------------------------------------------------------------------------
static void ResetCheckOffset( CHECK_WORK* work )
{
  work->check_offset = 0;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のゾーンIDを取得する
 *
 * @param work
 */
//------------------------------------------------------------------------
static u16 GetCheckZoneID( const CHECK_WORK* work )
{
  return work->check_zone_id;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象の出入り口IDを取得する
 *
 * @param work
 */
//------------------------------------------------------------------------
static u32 GetCheckExitID( const CHECK_WORK* work )
{
  return work->check_exit_id;
}

//------------------------------------------------------------------------
/**
 * @brief チェック対象のオフセットを取得する
 *
 * @param work
 */
//------------------------------------------------------------------------
static u16 GetCheckOffset( const CHECK_WORK* work )
{
  return work->check_offset;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ExitDirからDirを取得
 *
 *	@param	cp_data 
 */
//-----------------------------------------------------------------------------
static u16 ConnectData_GetExitDirToDir( const CONNECT_DATA * cp_data )
{
  u16 dir;

  switch( cp_data->exit_dir ) {
	case EXIT_DIR_UP:    dir = DIR_UP;    break;
	case EXIT_DIR_DOWN:  dir = DIR_DOWN;  break;
	case EXIT_DIR_LEFT:  dir = DIR_LEFT;  break;
	case EXIT_DIR_RIGHT: dir = DIR_RIGHT; break;
  default:             dir = DIR_DOWN;  break;
  } 
  return dir;
}

//------------------------------------------------------------------------
/**
 * @brief チェックが有効なゾーンかどうかを判定する
 *
 * @param zone_id
 *
 * @return 指定したゾーンに対するチェックが有効なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------
static BOOL CheckValidZone( u16 zone_id )
{
  int i;

  for( i=0; i<NELEMS(UncheckZoneList); i++ )
  {
    // チェック対象外リスト内に発見
    if( UncheckZoneList[i] == zone_id ) { return FALSE; }
  }

  return TRUE;
}

//------------------------------------------------------------------------
/**
 * @brief デバッグ出力 ( 全接続チェック開始 )
 */
//------------------------------------------------------------------------
static void DebugPrint_Start( const CHECK_WORK* work )
{
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
  OS_TFPrintf( PRINT_TARGET, "ALL-CONNECT-CHECK: Start!!\n" );
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力 ( すべての接続データ )
 */
//------------------------------------------------------------------------
static void DebugPrint_AllConnect( const CHECK_WORK* work )
{
  OS_TFPrintf( PRINT_TARGET, "---------------------------------------------\n" );
  OS_TFPrintf( PRINT_TARGET, "ALL-CONNECT-CHECK: ZoneID[%d], ConnectNum[%d]\n", GetCheckZoneID(work), GetConnectNum(work) );
  OS_TFPrintf( PRINT_TARGET, "---------------------------------------------\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力 ( 全接続チェック開始 )
 */
//------------------------------------------------------------------------
static void DebugPrint_CheckConnect( const CHECK_WORK* work )
{
  OS_TFPrintf( PRINT_TARGET, 
      "ALL-CONNECT-CHECK: ZoneID[%d]==>[%d], ExitID[%d], Offset[%d/%d]\n", 
      GetCheckZoneID(work), GetConnectZoneID(work), GetCheckExitID(work), GetCheckOffset(work), GetConnectOffsetSize(work) );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力 ( 特殊接続出入口を検出 )
 */
//------------------------------------------------------------------------
static void DebugPrint_DetectSpecialConnect( const CHECK_WORK* work )
{
  OS_TFPrintf( PRINT_TARGET, "********************************************\n" );
  OS_TFPrintf( PRINT_TARGET, "ALL-CONNECT-CHECK: SpecialConnect Detedted!!\n" );
  OS_TFPrintf( PRINT_TARGET, "********************************************\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力 ( 全接続チェック開始 )
 */
//------------------------------------------------------------------------
static void DebugPrint_Finish( const CHECK_WORK* work )
{
  OS_TFPrintf( PRINT_TARGET, "===========================\n" );
  OS_TFPrintf( PRINT_TARGET, "ALL-CONNECT-CHECK: Finish!!\n" );
  OS_TFPrintf( PRINT_TARGET, "===========================\n" );
}

//------------------------------------------------------------------------
/**
 * @brief ヒープ残量チェック
 *
 * @param heap_id    チェックするヒープ
 * @param check_size チェック残量
 */
//------------------------------------------------------------------------
static void HeapFreeSizeCheck( HEAPID heap_id, u32 check_size )
{
  if( GFL_HEAP_GetHeapFreeSize( HEAPID_PROC ) < check_size ) {
    OS_Printf( "***************************************\n" );
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    OS_Printf( "***************************************\n" );
    GF_ASSERT(0);
  }
}


#endif // PM_DEBUG
