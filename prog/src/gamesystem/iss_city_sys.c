/////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_city_sys.c
 * @brief  街ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
/////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_city_sys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"
#include "../../include/field/field_const.h"		// FIELD_CONST_GRID_FX32_SIZE
#include "arc/arc_def.h"
#include "field/field_const.h"
#include "iss_city_unit.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"
#include "sound/pm_sndsys.h"


//===========================================================================================
// ■定数・マクロ
//===========================================================================================
// デバッグ情報の出力先
#define PRINT_DEST (1)

// 無効ユニット番号
// ( 街ISSのBGMが鳴っているが, ISSユニットが配置されていない状況. )
#define UNIT_NOT_FOUND (0xff)

#define MIN_VOLUME               (0)   // 最小ボリューム
#define MAX_VOLUME               (127) // 最大ボリューム
#define MASTER_VOLUME_FADE_SPEED (2)   // マスターボリュームのフェード速度

// 操作トラック
#define TRACKBIT ((1<<(9-1))|(1<<(10-1))) // 9,10トラック


//===========================================================================================
// ■街ISSシステム構造体
//===========================================================================================
struct _ISS_CITY_SYS
{
	HEAPID       heapID; 
	PLAYER_WORK* player; // 監視対象プレイヤー

	// システム
	BOOL boot;		               // 起動中かどうか
	u8   activeUnitIdx;	         // 動作中のユニット番号
  int  trackVolume;            // 操作トラックのボリューム
  int  masterVolume;           // マスターボリューム ( 設定値 )
  int  practicalMasterVolume;  // マスターボリューム ( 実行値 )

	// ユニット
	ISS_C_UNIT** unit;		 // ユニット配列
	u8           unitNum;	 // ユニット数
};


//===========================================================================================
// ■非公開関数のプロトタイプ宣言
//===========================================================================================
// 生成/破棄
static void InitializeSystem( ISS_CITY_SYS* system );
static void LoadUnitData    ( ISS_CITY_SYS* system );
static void UnloadUnitData  ( ISS_CITY_SYS* system );
// システム制御
static void BootSystem( ISS_CITY_SYS* system );
static void StopSystem( ISS_CITY_SYS* system );
static void ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID );
static void SystemMain( ISS_CITY_SYS* system );
// ユニット制御
static void ChangeUnit( ISS_CITY_SYS* system, u16 zoneID ); 
// ボリューム制御 ( マスターボリューム )
static BOOL UpdateMasterVolume( ISS_CITY_SYS* system );
static void SetMasterVolume   ( ISS_CITY_SYS* system, int volume );
// ボリューム制御 ( トラックボリューム )
static BOOL UpdateTrackVolume   ( ISS_CITY_SYS* system );
static void ChangeBGMTrackVolume( const ISS_CITY_SYS* system );


//===========================================================================================
// ■公開関数の実装
//===========================================================================================

//-------------------------------------------------------------------------------------------
/**
 * @brief  街ISSシステムを作成する
 *
 * @param  player  監視対象のプレイヤー
 * @param  heapID  使用するヒープID
 * 
 * @return 街ISSシステム
 */
//-------------------------------------------------------------------------------------------
ISS_CITY_SYS* ISS_CITY_SYS_Create( PLAYER_WORK* player, HEAPID heapID )
{
	ISS_CITY_SYS* system;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: create\n" );

	// 生成
	system = (ISS_CITY_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof( ISS_CITY_SYS ) );

	// 初期化
  InitializeSystem( system );
	system->heapID = heapID;
	system->player = player;

	// ユニット情報の読み込み
	LoadUnitData( system );
	
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
  // ユニットデータの破棄
  UnloadUnitData( system );

	// 本体の破棄
	GFL_HEAP_FreeMemory( system );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: delete\n" );
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


//===========================================================================================
// ■非公開関数の実装
//===========================================================================================

//-------------------------------------------------------------------------------------------
/**
 * @brief システムを初期化する
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void InitializeSystem( ISS_CITY_SYS* system )
{
	system->heapID                = 0;
	system->player                = NULL;
	system->boot                  = FALSE;
	system->activeUnitIdx         = UNIT_NOT_FOUND;
  system->masterVolume          = MAX_VOLUME;
  system->practicalMasterVolume = MAX_VOLUME;
	system->unitNum               = 0;
	system->unit                  = NULL;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief ユニットデータを読み込む
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void LoadUnitData( ISS_CITY_SYS* system )
{
  int datnum;
  int datid;

  GF_ASSERT( system->unit == NULL );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: load unit data\n" );

  // ユニット数 取得
  datnum          = GFL_ARC_GetDataFileCnt( ARCID_ISS_CITY );
  system->unitNum = datnum;

  // 各ユニットデータ ワーク生成
  system->unit = GFL_HEAP_AllocMemory( system->heapID, datnum * sizeof(ISS_C_UNIT*) );

  // 各ユニットデータを読み込む
  for( datid=0; datid < datnum; datid++ )
  {
    system->unit[ datid ] = ISS_C_UNIT_Create( system->heapID, datid );
  } 
}

//-------------------------------------------------------------------------------------------
/**
 * @brief ユニットデータを破棄する
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void UnloadUnitData( ISS_CITY_SYS* system )
{
  int unitIdx;
  int unitNum;

  GF_ASSERT( system->unit );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: unload unit data\n" );

  unitNum = system->unitNum;

	// 各ユニットを破棄
  for( unitIdx=0; unitIdx < unitNum; unitIdx++ )
  {
    ISS_C_UNIT_Delete( system->unit[ unitIdx ] );
  }
	GFL_HEAP_FreeMemory( system->unit );
  system->unit = NULL;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void BootSystem( ISS_CITY_SYS* system )
{
  // 起動中
  if( system->boot ){ return; }

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: boot\n" );

  // 起動
  system->boot        = TRUE;
  system->trackVolume = MIN_VOLUME; 

  // ユニット検索
  {
    u16 zoneID;

    zoneID = PLAYERWORK_getZoneID( system->player );
    ChangeUnit( system, zoneID ); 
  } 

  // 音量を調整
  UpdateTrackVolume( system );
  ChangeBGMTrackVolume( system );
}

//-------------------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void StopSystem( ISS_CITY_SYS* system )
{
  // 停止中
  if( system->boot == FALSE ){ return; }

  // 停止
	system->boot          = FALSE;
  system->activeUnitIdx = UNIT_NOT_FOUND;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: stop\n" );
} 

//-------------------------------------------------------------------------------------------
/**
 * @brief ゾーンの切り替えを通知する
 *
 * @param system
 * @param nextZoneID 切り替え先のゾーンID
 */
//-------------------------------------------------------------------------------------------
static void ZoneChange( ISS_CITY_SYS* system, u16 nextZoneID )
{
  // 停止中
  if( system->boot == FALSE ){ return; }

  // ユニットを切り替える
  ChangeUnit( system, nextZoneID );

  // マスターボリューム設定
  if( system->activeUnitIdx == UNIT_NOT_FOUND )
  {
    SetMasterVolume( system, MIN_VOLUME );
  }
  else
  {
    SetMasterVolume( system, MAX_VOLUME );
  }

  // 音量を調整
  if( UpdateTrackVolume( system ) )
  {
    ChangeBGMTrackVolume( system ); 
  }
}

//-------------------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void SystemMain( ISS_CITY_SYS* system )
{
  BOOL changeMasterVolume;
  BOOL changeTrackVolume;

  // 停止中
  if( system->boot == FALSE ){ return; }

  // マスターボリューム更新
  changeMasterVolume = UpdateMasterVolume( system );

  // ボリューム調整
  changeTrackVolume = UpdateTrackVolume( system );
  
  // ボリュームが変化
  if( changeMasterVolume || changeTrackVolume )
  {
    ChangeBGMTrackVolume( system );
  }
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンに配置されたユニットに切り替える
 *
 * @param system
 * @param zoneID 切り替え先のゾーンID
 */
//-------------------------------------------------------------------------------------------
static void ChangeUnit( ISS_CITY_SYS* system, u16 zoneID )
{
	int unitIdx, unitNum;

  // 起動していない
  GF_ASSERT( system->boot );

  unitNum = system->unitNum;

  // 検索
	for( unitIdx=0; unitIdx < unitNum; unitIdx++ )
	{
		// 発見
		if( ISS_C_UNIT_GetZoneID( system->unit[ unitIdx ] ) == zoneID )
		{ 
			system->activeUnitIdx = unitIdx;

      // DEBUG:
      OS_TFPrintf( PRINT_DEST, "ISS-C: change unit index ==> %d\n", unitIdx );
      return;
		}
	}

  // 配置されていない
  system->activeUnitIdx = UNIT_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief マスターボリュームを更新する
 *
 * @param system
 *
 * @return マスターボリュームが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------------------
static BOOL UpdateMasterVolume( ISS_CITY_SYS* system )
{
  int nowVolume;
  int destVolume;
  int nextVolume;

  // 起動していない
  GF_ASSERT( system->boot );

  // 現在値/目標値
  nowVolume  = system->practicalMasterVolume;
  destVolume = system->masterVolume;

  // フェード完了済み
  if( nowVolume == destVolume ){ return FALSE; }

  // フェード
  if( destVolume < nowVolume )
  {
    nextVolume = nowVolume - MASTER_VOLUME_FADE_SPEED;
  }
  else
  {
    nextVolume = nowVolume + MASTER_VOLUME_FADE_SPEED;
  }

  // 最小/最大補正
  if( nextVolume < MIN_VOLUME ){ nextVolume = MIN_VOLUME; }
  if( MAX_VOLUME < nextVolume ){ nextVolume = MAX_VOLUME; }

  // マスター更新
  system->practicalMasterVolume = nextVolume;

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: update master volume => %d\n", nextVolume );

  return TRUE;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief マスターボリュームを設定する
 *
 * @param system
 * @param volume 設定するボリューム
 */
//-------------------------------------------------------------------------------------------
static void SetMasterVolume( ISS_CITY_SYS* system, int volume )
{
  GF_ASSERT( system->boot );
  GF_ASSERT( MIN_VOLUME <= volume );
  GF_ASSERT( volume <= MAX_VOLUME );

  system->masterVolume = volume;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief トラックボリュームを調整する
 *
 * @param system
 *
 * @return ボリュームが変化した場合 TRUE
 *         そうでなければ FALSE
 */
//-------------------------------------------------------------------------------------------
static BOOL UpdateTrackVolume( ISS_CITY_SYS* system )
{
  int newVolume = 0;
  const VecFx32* playerPos;

  // 起動していない
  GF_ASSERT( system->boot );  

  // ユニットが配置されていない
  if( system->activeUnitIdx == UNIT_NOT_FOUND ){ return FALSE; }

  // ユニットと自機の位置からボリュームを算出
  playerPos = PLAYERWORK_getPosition( system->player );
  newVolume = ISS_C_UNIT_GetVolume( system->unit[ system->activeUnitIdx ], playerPos );

  // ボリュームが変化した場合
  if( system->trackVolume != newVolume )
  {
    system->trackVolume = newVolume; 
    return TRUE;
  }
  return FALSE;
}

//-------------------------------------------------------------------------------------------
/**
 * @brief BGMのボリュームを変更する
 *
 * @param system
 */
//-------------------------------------------------------------------------------------------
static void ChangeBGMTrackVolume( const ISS_CITY_SYS* system )
{
  int trackVolume;
  int masterVolume;
  int volume;

  // 実ボリューム値を算出
  trackVolume  = system->trackVolume;
  masterVolume = system->practicalMasterVolume;
  volume       = trackVolume * masterVolume / MAX_VOLUME;

  // ボリュームの値域チェック
  GF_ASSERT( MIN_VOLUME   <= trackVolume );
  GF_ASSERT( MIN_VOLUME   <= masterVolume );
  GF_ASSERT( MIN_VOLUME   <= volume );
  GF_ASSERT( trackVolume  <= MAX_VOLUME );
  GF_ASSERT( masterVolume <= MAX_VOLUME );
  GF_ASSERT( volume       <= MAX_VOLUME );

  // トラックボリューム変更
  PMSND_ChangeBGMVolume( TRACKBIT, volume );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-C: change BGM track volume ==> %d\n", volume );
}
