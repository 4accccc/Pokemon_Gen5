///////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_city_sys.c
 * @brief  街ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
///////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_city_unit.h"
#include "iss_city_sys.h"
#include "arc/arc_def.h"
#include "sound/pm_sndsys.h"
#include "gamesystem/playerwork.h" 
#include "field/field_const.h"
#include "../field/field_sound.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//=================================================================================
// ■定数・マクロ
//=================================================================================
//#define DEBUG_PRINT_ON // デバッグ出力スイッチ
#define PRINT_DEST (1) // デバッグ情報の出力先

#define UNIT_IDX_NULL (0xff) // ユニット番号の無効値

#define MIN_TRACK_VOLUME    (0) // トラックボリュームの最小値
#define MAX_TRACK_VOLUME  (127) // トラックボリュームの最大値
#define MIN_SYSTEM_VOLUME   (0) // システムボリュームの最小値
#define MAX_SYSTEM_VOLUME (127) // システムボリュームの最大値
#define SYSTEM_VOLUME_FADEIN_SPEED  (2) // システムボリュームの変更速度 ( フェードイン )
#define SYSTEM_VOLUME_FADEOUT_SPEED (2) // システムボリュームの変更速度 ( フェードアウト )

// 操作トラック
#define TRACKBIT ((1<<(9-1))|(1<<(10-1))) // 9,10トラック

// システム状態
typedef enum {
  SYSTEM_STATE_WAIT,    // 待機
  SYSTEM_STATE_FADEIN,  // フェードイン
  SYSTEM_STATE_FADEOUT, // フェードアウト
} SYSTEM_STATE;


//=================================================================================
// ■街ISSシステム構造体
//=================================================================================
struct _ISS_CITY_SYS
{
	PLAYER_WORK* player; // 監視対象プレイヤー

	// システム
  BOOL         bootFlag;     // 起動しているかどうか
  SYSTEM_STATE state;        // 状態
  int          trackVolume;  // 操作トラックのボリューム
  int          systemVolume; // 上位ボリューム

	// ユニット
	ISS_C_UNIT** unit;		 // ユニット配列
	u8           unitNum;	 // ユニット数
	u8           unitIdx;	 // 動作中のユニット番号
};


//=================================================================================
// ◆関数インデックス
//=================================================================================
//---------------------------------------------------------------------------------
// ◇生成・破棄
//---------------------------------------------------------------------------------
// layer 2
static void InitializeSystem( ISS_CITY_SYS* system, HEAPID heapID, PLAYER_WORK* player ); // システムを初期化する
static void SetupSystem( ISS_CITY_SYS* system, HEAPID heapID ); // システムをセットアップする
static void CleanUpSystem( ISS_CITY_SYS* system ); // システムをクリーンアップする
// layer 1
static void LoadUnitData( ISS_CITY_SYS* system, HEAPID heapID ); // ユニットデータを読み込む
static void UnloadUnitData( ISS_CITY_SYS* system ); // ユニットデータを破棄する
//---------------------------------------------------------------------------------
// ◇ボリューム制御
//---------------------------------------------------------------------------------
// 上位ボリューム
static int GetSystemVolume( const ISS_CITY_SYS* system ); // 上位ボリュームを取得する
static void SetSystemVolume( ISS_CITY_SYS* system, int volume ); // 上位ボリュームを設定する
static BOOL SystemVolumeUp( ISS_CITY_SYS* system ); // 上位ボリュームを上げる
static BOOL SystemVolumeDown( ISS_CITY_SYS* system ); // 上位ボリュームを下げる
// トラックボリューム
static int GetTrackVolume( const ISS_CITY_SYS* system ); // トラックボリュームを取得する
static void SetTrackVolume( ISS_CITY_SYS* system, int volume ); // トラックボリュームを設定する
static BOOL UpdateTrackVolume( ISS_CITY_SYS* system ); // トラックボリュームを更新する
static int CalcTrackVolume( ISS_CITY_SYS* system ); // トラックボリュームを計算する
static int CalcEffectiveTrackVolume( const ISS_CITY_SYS* system ); // BGMの操作トラックに反映させるボリュームを算出する
static void ChangeBGMTrackVolume( const ISS_CITY_SYS* system ); // BGMのトラックボリュームを変更する
//---------------------------------------------------------------------------------
// ◇システム動作
//---------------------------------------------------------------------------------
static void SystemMain( ISS_CITY_SYS* system ); // システムのメイン動作関数
static void SystemMain_WAIT( ISS_CITY_SYS* system ); // システムのメイン動作関数 ( SYSTEM_STATE_WAIT )
static void SystemMain_FADEIN( ISS_CITY_SYS* system ); // システムのメイン動作関数 ( SYSTEM_STATE_FADEIN )
static void SystemMain_FADEOUT( ISS_CITY_SYS* system ); // システムのメイン動作関数 ( SYSTEM_STATE_FADEOUT )
//---------------------------------------------------------------------------------
// ◇システム制御
//---------------------------------------------------------------------------------
static SYSTEM_STATE GetSystemState( const ISS_CITY_SYS* system ); // システム状態を取得する
static void SetSystemState( ISS_CITY_SYS* system, SYSTEM_STATE state ); // システム状態を変更する
static void BootSystem( ISS_CITY_SYS* system ); // システムを起動する
static void StopSystem( ISS_CITY_SYS* system ); // システムを停止する
static void ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID ); // ゾーンの切り替え処理を行う
static u16 GetCurrentZoneID( const ISS_CITY_SYS* system ); // 現在のゾーンIDを取得する
static BOOL CheckSystemBoot( const ISS_CITY_SYS* system ); // システムが起動しているかどうかをチェックする
//---------------------------------------------------------------------------------
// ◇ユニット
//---------------------------------------------------------------------------------
static ISS_C_UNIT* GetCurrentUnit( const ISS_CITY_SYS* system ); // 現在のユニットを取得する
static u8 GetCurrentUnitIndex( const ISS_CITY_SYS* system ); // 現在のユニットインデックスを取得する
static BOOL CheckUnitExistNow( const ISS_CITY_SYS* system ); // 現在, ユニットが存在しているかどうかをチェックする
static BOOL CheckUnitExistAt( const ISS_CITY_SYS* system, u16 zoneID ); // 指定した場所にユニットが存在しているかどうかをチェックする
static BOOL ChangeUnit( ISS_CITY_SYS* system, u16 zoneID ); // ユニットを変更する
static u8 SearchUnit( const ISS_CITY_SYS* system, u16 zoneID ); // ユニットを検索する
static BOOL CheckUnitChange( const ISS_CITY_SYS* system, u16 zoneID ); // ユニットが変化するかどうかをチェックする


//=================================================================================
// ■公開関数の実装
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief  街ISSシステムを作成する
 *
 * @param  player  監視対象のプレイヤー
 * @param  heapID  使用するヒープID
 * 
 * @return 街ISSシステム
 */
//---------------------------------------------------------------------------------
ISS_CITY_SYS* ISS_CITY_SYS_Create( PLAYER_WORK* player, HEAPID heapID )
{
	ISS_CITY_SYS* system;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: create\n" );
#endif

	// システムを生成
	system = (ISS_CITY_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof(ISS_CITY_SYS) );

	// システムを初期化
  InitializeSystem( system, heapID, player );
  SetupSystem( system, heapID ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: create\n" );
#endif
	
	return system;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 街ISSシステムを破棄する
 *
 * @param system
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Delete( ISS_CITY_SYS* system )
{
#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: delete\n" );
#endif

  // システムをクリーンアップ
  CleanUpSystem( system );

	// 本体の破棄
	GFL_HEAP_FreeMemory( system );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: delete\n" );
#endif
}

//------------------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param system
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Main( ISS_CITY_SYS* system )
{
  // 起動していない
  if( CheckSystemBoot(system) == FALSE ) { return; }

  SystemMain( system );
}
	
//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_On( ISS_CITY_SYS* system )
{ 
  BootSystem( system );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param system
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Off( ISS_CITY_SYS* system )
{
  StopSystem( system );
}

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param system
 * @param nextZoneID 新しいゾーンID
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID )
{ 
  ZoneChange( system, nextZoneID );
}


//=================================================================================
// ■非公開関数の実装
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief システムを初期化する
 *
 * @param system
 * @param heapID システムが使用するヒープID
 * @param player 監視対象のプレイヤー ( 自機 )
 */
//---------------------------------------------------------------------------------
static void InitializeSystem( ISS_CITY_SYS* system, HEAPID heapID, PLAYER_WORK* player )
{
  GF_ASSERT( system );
  GF_ASSERT( player );

	system->player        = player;
  system->bootFlag      = FALSE;
  system->state         = SYSTEM_STATE_WAIT;
  system->trackVolume   = MIN_TRACK_VOLUME;
  system->systemVolume  = MIN_SYSTEM_VOLUME;
	system->unit          = NULL;
	system->unitNum       = 0;
  system->unitIdx       = UNIT_IDX_NULL;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: init system\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief システムをセットアップする 
 *
 * @param system
 * @param heapID
 */
//---------------------------------------------------------------------------------
static void SetupSystem( ISS_CITY_SYS* system, HEAPID heapID )
{
  GF_ASSERT( system );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: setup system\n" );
#endif 

	LoadUnitData( system, heapID ); // ユニット情報を読み込む

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: setup system\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief システムをクリーンアップする
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void CleanUpSystem( ISS_CITY_SYS* system )
{
  GF_ASSERT( system );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: clean up system\n" );
#endif

  UnloadUnitData( system ); // ユニットデータを破棄する

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: clean up system\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief ユニットデータを読み込む
 *
 * @param system
 * @param heapID
 */
//---------------------------------------------------------------------------------
static void LoadUnitData( ISS_CITY_SYS* system, HEAPID heapID )
{
  int dataNum;
  int datID;

  GF_ASSERT( system->unit == NULL );

  // ユニット数を取得
  dataNum = GFL_ARC_GetDataFileCnt( ARCID_ISS_CITY );
  system->unitNum = dataNum;

  // 各ユニットデータのワークを確保
  system->unit = GFL_HEAP_AllocMemory( heapID, dataNum * sizeof(ISS_C_UNIT*) );

  // 各ユニットデータを読み込む
  for( datID=0; datID < dataNum; datID++ )
  {
    system->unit[ datID ] = ISS_C_UNIT_Create( heapID, datID );
  } 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: load unit data\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief ユニットデータを破棄する
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void UnloadUnitData( ISS_CITY_SYS* system )
{
  int unitIdx;
  int unitNum;

  GF_ASSERT( system );
  GF_ASSERT( system->unit );

  unitNum = system->unitNum;

	// 各ユニットを破棄
  for( unitIdx=0; unitIdx < unitNum; unitIdx++ )
  {
    ISS_C_UNIT_Delete( system->unit[ unitIdx ] );
  }
	GFL_HEAP_FreeMemory( system->unit );
  system->unit = NULL;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: unload unit data\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief システム状態を取得する
 *
 * @param system
 *
 * @return 現在のシステムの状態
 */
//---------------------------------------------------------------------------------
static SYSTEM_STATE GetSystemState( const ISS_CITY_SYS* system )
{
  GF_ASSERT( system );
  return system->state;
}

//---------------------------------------------------------------------------------
/**
 * @brief システム状態を変更する
 *
 * @param system
 * @param state  設定する状態
 */
//---------------------------------------------------------------------------------
static void SetSystemState( ISS_CITY_SYS* system, SYSTEM_STATE state )
{
  GF_ASSERT( system );

  // 状態を更新
  system->state = state;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: set system state ==> " );
  switch( state ) {
  case SYSTEM_STATE_WAIT:    OS_TFPrintf( PRINT_DEST, "WAIT" );    break;
  case SYSTEM_STATE_FADEIN:  OS_TFPrintf( PRINT_DEST, "FADEIN" );  break;
  case SYSTEM_STATE_FADEOUT: OS_TFPrintf( PRINT_DEST, "FADEOUT" ); break;
  default: GF_ASSERT(0);
  }
  OS_TFPrintf( PRINT_DEST, "\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void BootSystem( ISS_CITY_SYS* system )
{
  // 起動済み
  if( CheckSystemBoot(system) ) { return; }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: boot system\n" );
#endif

  system->bootFlag = TRUE; // 起動
  SetTrackVolume( system, MIN_TRACK_VOLUME ); // トラックボリュームを最小にする
  ChangeBGMTrackVolume( system ); // BGMに反映させる

  // ユニットを変更
  ChangeUnit( system, GetCurrentZoneID(system) ); 

  // ユニットがある
  if( CheckUnitExistNow(system) == TRUE ) {
    SetSystemState( system, SYSTEM_STATE_FADEIN );
  }
  // ユニットがない
  else {
    SetSystemState( system, SYSTEM_STATE_WAIT );
  } 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: boot system\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void StopSystem( ISS_CITY_SYS* system )
{
  // 停止中
  if( CheckSystemBoot(system) == FALSE ) { return; }

  // 停止させる
  system->bootFlag = FALSE;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: stop system\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替え処理を行う
 *
 * @param system
 * @param nextZoneID 切り替え先のゾーンID
 */
//---------------------------------------------------------------------------------
static void ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID )
{
  // 停止中
  if( CheckSystemBoot(system) == FALSE ) { return; }

  // 現在, ユニットが存在する
  if( CheckUnitExistNow( system ) == TRUE ) {
    // 移動先にもユニットが存在する
    if( CheckUnitExistAt( system, nextZoneID ) == TRUE ) {
      // 曲の変更に対するリアクションは, システムのON/OFFで行う. 
      StopSystem( system );
      return;
    }
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: zone change\n" );
#endif

  // ユニットを変更
  ChangeUnit( system, nextZoneID );

  // ユニットが存在する
  if( CheckUnitExistNow( system ) == TRUE ) {
    SetSystemVolume( system, MIN_SYSTEM_VOLUME );
    SetSystemState( system, SYSTEM_STATE_FADEIN );
  }
  // ユニットが存在しない
  else {
    SetSystemState( system, SYSTEM_STATE_FADEOUT );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "--- ISS-C: zone change\n" );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを取得する
 *
 * @param system
 *
 * @return 現在のゾーンID
 */
//---------------------------------------------------------------------------------
static u16 GetCurrentZoneID( const ISS_CITY_SYS* system )
{
  GF_ASSERT( system );
  GF_ASSERT( system->player );

  return PLAYERWORK_getZoneID( system->player );
}

//---------------------------------------------------------------------------------
/**
 * @brief システムが起動しているかどうかをチェックする
 *
 * @param system
 *
 * @return システムが起動している場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL CheckSystemBoot( const ISS_CITY_SYS* system )
{
  return system->bootFlag;
}

//---------------------------------------------------------------------------------
/**
 * @brief システムのメイン動作
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void SystemMain( ISS_CITY_SYS* system )
{
  GF_ASSERT( system );

  switch( GetSystemState(system) ) {
  case SYSTEM_STATE_WAIT:    SystemMain_WAIT( system );    break;
  case SYSTEM_STATE_FADEIN:  SystemMain_FADEIN( system );  break;
  case SYSTEM_STATE_FADEOUT: SystemMain_FADEOUT( system ); break;
  default: GF_ASSERT(0);
  } 
}

//---------------------------------------------------------------------------------
/**
 * @brief システムのメイン動作関数 ( SYSTEM_STATE_WAIT )
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void SystemMain_WAIT( ISS_CITY_SYS* system )
{
  GF_ASSERT( system );
  GF_ASSERT( GetSystemState(system) == SYSTEM_STATE_WAIT ); 

  // トラックボリュームを更新
  if( UpdateTrackVolume( system ) ) {
    ChangeBGMTrackVolume( system ); // BGMに反映させる
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief システムのメイン動作関数 ( SYSTEM_STATE_FADEIN )
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void SystemMain_FADEIN( ISS_CITY_SYS* system )
{
  BOOL sysVolChanged;
  BOOL trkVolChanged;

  GF_ASSERT( system );
  GF_ASSERT( GetSystemState(system) == SYSTEM_STATE_FADEIN );

  // 上位ボリューム・トラックボリュームを更新
  sysVolChanged = SystemVolumeUp( system );
  trkVolChanged = UpdateTrackVolume( system );
  
  // ボリュームが変化
  if( sysVolChanged || trkVolChanged ) {
    ChangeBGMTrackVolume( system ); // BGMに反映させる
  }

  // フェードイン完了
  if( MAX_SYSTEM_VOLUME <= GetSystemVolume(system) ) {
    SetSystemState( system, SYSTEM_STATE_WAIT );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief システムのメイン動作関数 ( SYSTEM_STATE_FADEOUT )
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void SystemMain_FADEOUT( ISS_CITY_SYS* system )
{
  BOOL sysVolChanged;
  BOOL trkVolChanged;

  GF_ASSERT( system );
  GF_ASSERT( GetSystemState(system) == SYSTEM_STATE_FADEOUT );

  // 上位ボリューム・トラックボリュームを更新
  sysVolChanged = SystemVolumeDown( system );
  trkVolChanged = UpdateTrackVolume( system );
  
  // ボリュームが変化
  if( sysVolChanged || trkVolChanged ) {
    ChangeBGMTrackVolume( system ); // BGMに反映させる
  }

  // フェードアウト完了
  if( GetSystemVolume(system) <= MIN_SYSTEM_VOLUME ) {
    SetSystemState( system, SYSTEM_STATE_WAIT );
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 現在のユニットを取得する
 *
 * @param system
 *
 * @return 現在のユニット
 *         ユニットが存在しない場合 NULL
 */
//---------------------------------------------------------------------------------
static ISS_C_UNIT* GetCurrentUnit( const ISS_CITY_SYS* system )
{
  return system->unit[ GetCurrentUnitIndex(system) ];
}

//---------------------------------------------------------------------------------
/**
 * @brief 現在のユニットインデックスを取得する
 *
 * @param system 
 *
 * @return 現在のユニットのインデックス
 *         ユニットが存在しない場合, UNIT_IDX_NULL
 */
//---------------------------------------------------------------------------------
static u8 GetCurrentUnitIndex( const ISS_CITY_SYS* system )
{
  return system->unitIdx;
}

//---------------------------------------------------------------------------------
/**
 * @brief 現在, ユニットが存在しているかどうかをチェックする
 *
 * @param system
 * 
 * @return ユニットが存在している場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL CheckUnitExistNow( const ISS_CITY_SYS* system )
{
  if( GetCurrentUnitIndex(system) == UNIT_IDX_NULL ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定した場所にユニットが存在しているかどうかをチェックする
 *
 * @param system
 * @param zoneID チェックする場所を指定
 * 
 * @return ユニットが存在している場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL CheckUnitExistAt( const ISS_CITY_SYS* system, u16 zoneID )
{
  if( SearchUnit( system, zoneID ) == UNIT_IDX_NULL ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンに配置されたユニットに切り替える
 *
 * @param system
 * @param zoneID 切り替え先のゾーンID
 *
 * @return ユニットが変化した場合, TRUE
 *         そうでなければ　FALSE
 */
//---------------------------------------------------------------------------------
static BOOL ChangeUnit( ISS_CITY_SYS* system, u16 zoneID )
{
  int oldUnitIdx;
  int newUnitIdx;

  GF_ASSERT( system );

  oldUnitIdx = GetCurrentUnitIndex(system);
  newUnitIdx = SearchUnit( system, zoneID );

  // 変化なし
  if( oldUnitIdx == newUnitIdx ) { return FALSE; }

  // ユニットインデックスを変更
  system->unitIdx = newUnitIdx;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: change unit ==> %d\n", newUnitIdx );
#endif

  return TRUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief ユニットを検索する
 *
 * @param system
 * @param zoneID 検索対象のゾーン
 *
 * @return 指定したゾーンに対応するユニットのインデックス
 *         ユニットが存在しない場合 UNIT_IDX_NULL
 */
//---------------------------------------------------------------------------------
static u8 SearchUnit( const ISS_CITY_SYS* system, u16 zoneID )
{
	int unitIdx;
  int unitNum;

  GF_ASSERT( system );
  GF_ASSERT( system->unit );
  GF_ASSERT( 0 < system->unitNum );

  unitNum = system->unitNum;

  // 指定されたゾーンに該当するユニットを検索する
	for( unitIdx=0; unitIdx < unitNum; unitIdx++ )
	{
		// 発見
		if( ISS_C_UNIT_GetZoneID( system->unit[ unitIdx ] ) == zoneID )
		{ 
      return unitIdx;
		}
	}

  // 配置されていない
  return UNIT_IDX_NULL;
}

//---------------------------------------------------------------------------------
/**
 * @brief ユニットが変化するかどうかをチェックする
 *
 * @param system
 * @param zoneID チェックするゾーンを指定
 *
 * @return 指定したゾーンに移動した時に ユニットが変化する場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL CheckUnitChange( const ISS_CITY_SYS* system, u16 zoneID )
{
  int oldUnitIdx;
  int newUnitIdx;

  oldUnitIdx = GetCurrentUnitIndex(system);
  newUnitIdx = SearchUnit( system, zoneID );

  // 変化なし
  if(oldUnitIdx == newUnitIdx ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//---------------------------------------------------------------------------------
/**
 * @brief 上位ボリュームを取得する
 *
 * @param system
 *
 * @return 上位ボリューム値
 */
//---------------------------------------------------------------------------------
static int GetSystemVolume( const ISS_CITY_SYS* system )
{ 
  GF_ASSERT( system );

  return system->systemVolume;
}

//---------------------------------------------------------------------------------
/**
 * @brief 上位ボリュームを上げる
 *
 * @param system
 *
 * @return 上位ボリュームが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL SystemVolumeUp( ISS_CITY_SYS* system )
{
  int nextVolume;

  GF_ASSERT( system );

  // ボリュームを計算
  nextVolume = GetSystemVolume(system) + SYSTEM_VOLUME_FADEIN_SPEED;

  // 最大値補正
  if( MAX_SYSTEM_VOLUME <= nextVolume ) { nextVolume = MAX_SYSTEM_VOLUME; }

  // ボリューム変化なし
  if( nextVolume == GetSystemVolume(system) ) { return FALSE; }

  // 上位ボリュームを更新
  SetSystemVolume( system, nextVolume );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: system volume up ==> %d\n", nextVolume );
#endif

  return TRUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief 上位ボリュームを下げる
 *
 * @param system
 *
 * @return 上位ボリュームが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL SystemVolumeDown( ISS_CITY_SYS* system )
{
  int nextVolume;

  GF_ASSERT( system );

  // ボリュームを計算
  nextVolume = GetSystemVolume(system) - SYSTEM_VOLUME_FADEOUT_SPEED;

  // 最小値補正
  if( nextVolume <= MIN_SYSTEM_VOLUME ) { nextVolume = MIN_SYSTEM_VOLUME; }

  // ボリューム変化なし
  if( nextVolume == GetSystemVolume(system) ) { return FALSE; }

  // 上位ボリュームを更新
  SetSystemVolume( system, nextVolume );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: system volume down ==> %d\n", nextVolume );
#endif

  return TRUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief 上位ボリュームを設定する
 *
 * @param system
 * @param volume 設定するボリューム
 */
//---------------------------------------------------------------------------------
static void SetSystemVolume( ISS_CITY_SYS* system, int volume )
{
  GF_ASSERT( system );
  GF_ASSERT( MIN_SYSTEM_VOLUME <= volume );
  GF_ASSERT( volume <= MAX_SYSTEM_VOLUME );

  system->systemVolume = volume;
}

//---------------------------------------------------------------------------------
/**
 * @brief トラックボリュームを取得する
 *
 * @param system
 * 
 * @return トラックボリューム値
 */
//---------------------------------------------------------------------------------
static int GetTrackVolume( const ISS_CITY_SYS* system )
{
  GF_ASSERT( system );

  return system->trackVolume;
}

//---------------------------------------------------------------------------------
/**
 * @brief トラックボリュームを更新する
 *
 * @param system
 *
 * @return ボリュームが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//---------------------------------------------------------------------------------
static BOOL UpdateTrackVolume( ISS_CITY_SYS* system )
{
  int newVolume = 0;
  const VecFx32* playerPos;
  const ISS_C_UNIT* unit;

  GF_ASSERT( system );
  GF_ASSERT( CheckSystemBoot(system) );

  // ユニットが配置されていない
  if( CheckUnitExistNow(system) == FALSE ) { return FALSE; }

  // ユニットと自機の位置からボリュームを算出
  newVolume = CalcTrackVolume( system );

  // ボリューム変化なし
  if( newVolume == GetTrackVolume(system) ) { return FALSE; }

  // ボリュームを変更
  SetTrackVolume( system, newVolume ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: update track volume ==> %d\n", newVolume );
#endif
  return TRUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief トラックボリュームを計算する
 *
 * @param system
 *
 * @return ユニットと自機の位置から算出したトラックボリューム
 */
//---------------------------------------------------------------------------------
static int CalcTrackVolume( ISS_CITY_SYS* system )
{
  int volume;
  const VecFx32* playerPos;
  const ISS_C_UNIT* unit;

  GF_ASSERT( system );
  GF_ASSERT( system->player );
  GF_ASSERT( CheckUnitExistNow(system) == TRUE );

  // ユニットと自機の位置からボリュームを算出
  unit      = GetCurrentUnit( system );
  playerPos = PLAYERWORK_getPosition( system->player );
  volume    = ISS_C_UNIT_GetVolume( unit, playerPos );

  return volume;
}

//---------------------------------------------------------------------------------
/**
 * @brief トラックボリュームを設定する
 *
 * @param system
 * @param volume 設定するボリューム
 */
//---------------------------------------------------------------------------------
static void SetTrackVolume( ISS_CITY_SYS* system, int volume )
{
  GF_ASSERT( system );
  GF_ASSERT( MIN_TRACK_VOLUME <= volume );
  GF_ASSERT( volume <= MAX_TRACK_VOLUME );

  system->trackVolume = volume;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMのボリュームを変更する
 *
 * @param system
 */
//---------------------------------------------------------------------------------
static void ChangeBGMTrackVolume( const ISS_CITY_SYS* system )
{
  int volume;

  // ボリューム実効値を算出
  volume = CalcEffectiveTrackVolume( system );

  // BGMのトラックボリュームに反映
  PMSND_ChangeBGMVolume( TRACKBIT, volume );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( PRINT_DEST, "ISS-C: change BGM track volume ==> %d\n", volume );
#endif
}

//---------------------------------------------------------------------------------
/**
 * @brief BGMの操作トラックに反映させるボリュームを算出する
 *
 * @param system
 *
 * @return BGMに反映させるボリューム値
 */
//---------------------------------------------------------------------------------
static int CalcEffectiveTrackVolume( const ISS_CITY_SYS* system )
{
  int trackVolume;
  int systemVolume;
  int volume;

  GF_ASSERT( system );

  // ボリューム実効値を算出
  trackVolume  = GetTrackVolume(system);
  systemVolume = GetSystemVolume(system);
  volume       = trackVolume * systemVolume / MAX_SYSTEM_VOLUME;

  // ボリュームの値域チェック
  GF_ASSERT( MIN_TRACK_VOLUME <= trackVolume );
  GF_ASSERT( MIN_SYSTEM_VOLUME <= systemVolume );
  GF_ASSERT( 0 <= volume );
  GF_ASSERT( trackVolume <= MAX_TRACK_VOLUME );
  GF_ASSERT( systemVolume <= MAX_SYSTEM_VOLUME );
  GF_ASSERT( volume <= 127 );

  return volume;
}
