///////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_sys.h
 * @brief  ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
///////////////////////////////////////////////////////////////////////////////
#include "iss_sys.h"
#include "iss_city_sys.h"
#include "iss_road_sys.h"
#include "iss_dungeon_sys.h"
#include "iss_zone_sys.h"
#include "iss_switch_sys.h"
#include "iss_3ds_sys.h"
#include "../field/field_sound.h"
#include "sound/bgm_info.h"
#include "../../../resource/sound/bgm_info/iss_type.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "sound/pm_sndsys.h" 


//=============================================================================
// ■定数
//=============================================================================
#define ISS_ENABLE   // ISS 動作スイッチ
#define ISS_R_ENABLE // ISS-R 動作スイッチ
#define ISS_C_ENABLE // ISS-C 動作スイッチ
#define ISS_D_ENABLE // ISS-D 動作スイッチ
#define ISS_Z_ENABLE // ISS-Z 動作スイッチ
#define ISS_S_ENABLE // ISS-S 動作スイッチ
#define ISS_B_ENABLE // ISS-B 動作スイッチ
//#define PRINT_ENABLE
#define PRINT_TARGET (1) // デバッグ情報の出力先
#define INVALID_BGM_NO (0xffffffff)  // 監視BGM番号の無効値


//=============================================================================
// ■関数インデックス
//=============================================================================
// ISSシステムの生成・破棄
static void InitISS( ISS_SYS* system ); // システムを初期化する
static void SetupISS( ISS_SYS* system, GAMEDATA* gameData, HEAPID heapID ); // システムをセットアップする
static void CleanUpISS( ISS_SYS* system ); // システムをクリーンアップする
// 各ISSの生成
static void CreateISS_R( ISS_SYS* system ); // 道ISSを生成する
static void CreateISS_C( ISS_SYS* system ); // 街ISSを生成する
static void CreateISS_D( ISS_SYS* system ); // ダンジョンISSを生成する
static void CreateISS_Z( ISS_SYS* system ); // ゾーンISSを生成する
static void CreateISS_S( ISS_SYS* system ); // スイッチISSを生成する
static void CreateISS_B( ISS_SYS* system ); // 橋ISSを生成する
// 各ISSの破棄
static void DeleteISS_R( ISS_SYS* system ); // 道ISSを破棄する
static void DeleteISS_C( ISS_SYS* system ); // 街ISSを破棄する
static void DeleteISS_D( ISS_SYS* system ); // ダンジョンISSを破棄する
static void DeleteISS_Z( ISS_SYS* system ); // ゾーンISSを破棄する
static void DeleteISS_S( ISS_SYS* system ); // スイッチISSを破棄する
static void DeleteISS_B( ISS_SYS* system ); // 橋ISSを破棄する
// 各ISSの動作
static void MainISS_R( ISS_SYS* system ); // 道ISSのメイン動作
static void MainISS_C( ISS_SYS* system ); // 街ISSのメイン動作
static void MainISS_D( ISS_SYS* system ); // ダンジョンISSのメイン動作
static void MainISS_Z( ISS_SYS* system ); // ゾーンISSのメイン動作
static void MainISS_S( ISS_SYS* system ); // スイッチISSのメイン動作
static void MainISS_B( ISS_SYS* system ); // 橋ISSのメイン動作
// 各ISSの起動
static void BootISS_R( ISS_SYS* system ); // 道ISSを起動する
static void BootISS_C( ISS_SYS* system ); // 街ISSを起動する
static void BootISS_D( ISS_SYS* system ); // ダンジョンISSを起動する
static void BootISS_Z( ISS_SYS* system ); // ゾーンISSを起動する
static void BootISS_S( ISS_SYS* system ); // スイッチISSを起動する
static void BootISS_B( ISS_SYS* system ); // 橋ISSを起動する
// 各ISSの停止
static void StopISS_R( ISS_SYS* system ); // 道ISSを停止する
static void StopISS_C( ISS_SYS* system ); // 街ISSを停止する
static void StopISS_D( ISS_SYS* system ); // ダンジョンISSを停止する
static void StopISS_Z( ISS_SYS* system ); // ゾーンISSを停止する
static void StopISS_S( ISS_SYS* system ); // スイッチISSを停止する
static void StopISS_B( ISS_SYS* system ); // 橋ISSを停止する
// ゾーンの変更通知
static void ZoneChangeISS_R( ISS_SYS* system, u16 nextZoneID ); // 道ISSにゾーンの変更を通知する
static void ZoneChangeISS_C( ISS_SYS* system, u16 nextZoneID ); // 街ISSにゾーンの変更を通知する
static void ZoneChangeISS_D( ISS_SYS* system, u16 nextZoneID ); // ダンジョンISSにゾーンの変更を通知する
static void ZoneChangeISS_Z( ISS_SYS* system, u16 nextZoneID ); // ゾーンISSにゾーンの変更を通知する
static void ZoneChangeISS_S( ISS_SYS* system, u16 nextZoneID ); // スイッチISSにゾーンの変更を通知する
static void ZoneChangeISS_B( ISS_SYS* system, u16 nextZoneID ); // 橋ISSにゾーンの変更を通知する
// 各ISSのON・OFF切り替え
static PLAYER_WORK* GetPlayerWork( const ISS_SYS* system ); // 自機を取得する
static u16 GetCurrentZoneID( const ISS_SYS* system ); // 現在のゾーンIDを取得する
static u32 GetPlayingBGM( void ); // 再生中のBGMを取得する
static u8 GetISSTypeOfPlayingBGM( const ISS_SYS* system ); // 再生中のBGMのISSタイプを取得する
static BOOL CheckBGMChange( ISS_SYS* system ); // BGMの変化をチェックする
static void BGMChange( ISS_SYS* system ); // BGM変更処理


//=============================================================================
// ■ISSシステムワーク
//=============================================================================
struct _ISS_SYS
{
	HEAPID    heapID;     // 使用するヒープID 
	GAMEDATA* gameData;   // ゲームデータ

	u32 soundIdx;  // 監視中のBGM番号
	u32 frame;     // フレームカウンタ

	// 各ISSシステム
	ISS_CITY_SYS*    issC;	// 街
	ISS_ROAD_SYS*    issR;	// 道路
	ISS_DUNGEON_SYS* issD;	// ダンジョン 
  ISS_ZONE_SYS*    issZ;  // ゾーン
  ISS_SWITCH_SYS*  issS;  // スイッチ
  ISS_3DS_SYS*     issB;  // 橋
};



//=============================================================================
// ■外部公開関数
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief ISSシステムを作成する
 *
 * @param gameData
 * @param heapID   使用するヒープID
 * 
 * @return ISSシステム
 */
//-----------------------------------------------------------------------------
ISS_SYS* ISS_SYS_Create( GAMEDATA* gameData, HEAPID heapID )
{
	ISS_SYS* system;

	// システムを生成
	system = (ISS_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SYS) );

  // システムを初期化・セットアップ
  InitISS( system );
  SetupISS( system, gameData, heapID );

	return system;
}

//-----------------------------------------------------------------------------
/**
 * @brief ISSシステムを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
void ISS_SYS_Delete( ISS_SYS* system )
{ 
  // システムをクリーンアップ
  CleanUpISS( system );

	// 本体を破棄
	GFL_HEAP_FreeMemory( system );
}

//-----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
void ISS_SYS_Update( ISS_SYS* system )
{ 
#ifdef ISS_ENABLE
	// BGMが変化
  if( CheckBGMChange( system ) == TRUE ) {
    BGMChange( system ); // BGM変更処理
    system->soundIdx = GetPlayingBGM(); // 新しいBGM番号を記憶
  }

  // 各ISSの更新
  MainISS_R( system ); // 道ISS
  MainISS_C( system ); // 街ISS
  MainISS_D( system ); // ダンジョンISS
  MainISS_Z( system ); // ゾーンISS
  MainISS_S( system ); // スイッチISS
  MainISS_B( system ); // 橋ISS

  // フレームカウンタを更新
  system->frame++;
#endif
}
	

//-----------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param system
 * @param nextZoneID 新しいゾーンID
 */
//-----------------------------------------------------------------------------
void ISS_SYS_ZoneChange( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_ENABLE
	PLAYER_MOVE_FORM form;

  // 自機のフォームを取得
	form = PLAYERWORK_GetMoveForm( GetPlayerWork(system) ); 

	// 自転車に乗っている ==> ISS変更無し
	if( form == PLAYER_MOVE_FORM_CYCLE ) { return; }

  // なみのり中 ==> ISS変更無し
	if( form == PLAYER_MOVE_FORM_SWIM ) { return; }

  // 各ISSシステムにゾーンの変更を通知
  ZoneChangeISS_R( system, nextZoneID ); // 道ISS
  ZoneChangeISS_C( system, nextZoneID ); // 街ISS
  ZoneChangeISS_D( system, nextZoneID ); // ダンジョンISS
  ZoneChangeISS_Z( system, nextZoneID ); // ゾーンISS
  ZoneChangeISS_S( system, nextZoneID ); // スイッチISS
  ZoneChangeISS_B( system, nextZoneID ); // 橋ISS
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSシステムを取得する
 *
 * @param system 取得対象システム
 *
 * @return スイッチISSシステム
 */
//-----------------------------------------------------------------------------
ISS_SWITCH_SYS* ISS_SYS_GetIssSwitchSystem( const ISS_SYS* system )
{
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  return system->issS;
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSシステムを取得する
 *
 * @param system 取得対象システム
 *
 * @return 橋ISSシステム
 */
//-----------------------------------------------------------------------------
ISS_3DS_SYS* ISS_SYS_GetIss3DSSystem( const ISS_SYS* system )
{
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  return system->issB;
}


//=============================================================================
// ■非公開関数
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief システムを初期化する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void InitISS( ISS_SYS* system )
{
  system->heapID   = 0;
  system->gameData = NULL;
  system->soundIdx = 0;
  system->frame    = 0;
  system->issC     = NULL;
  system->issR     = NULL;
  system->issD     = NULL;
  system->issZ     = NULL;
  system->issS     = NULL;
  system->issB     = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: init ISS\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief システムをセットアップする
 *
 * @param system
 * @param gameData
 * @param heapID
 */
//-----------------------------------------------------------------------------
static void SetupISS( ISS_SYS* system, GAMEDATA* gameData, HEAPID heapID )
{ 
  GF_ASSERT( system );
  GF_ASSERT( gameData );

	system->heapID   = heapID;
	system->gameData = gameData;
	system->soundIdx = INVALID_BGM_NO;
	system->frame    = 0;

  CreateISS_R( system );
  CreateISS_C( system );
  CreateISS_D( system );
  CreateISS_Z( system );
  CreateISS_S( system );
  CreateISS_B( system );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: setup ISS\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief システムをクリーンアップする
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CleanUpISS( ISS_SYS* system )
{
  GF_ASSERT( system );

  DeleteISS_R( system );
  DeleteISS_C( system );
  DeleteISS_D( system );
  DeleteISS_Z( system );
  DeleteISS_S( system );
  DeleteISS_B( system );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: clean up ISS\n" );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 道ISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_R( ISS_SYS* system )
{
  PLAYER_WORK* player;

#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->gameData );
  GF_ASSERT( system->issR == NULL );

	player = GetPlayerWork( system );
	system->issR = ISS_ROAD_SYS_Create( player, system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-R\n" );
#endif
#endif 
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_C( ISS_SYS* system )
{
#ifdef ISS_C_ENABLE
	PLAYER_WORK* player;
	player = GetPlayerWork( system );

  GF_ASSERT( system );
  GF_ASSERT( system->gameData );
  GF_ASSERT( system->issC == NULL ); 

	system->issC = ISS_CITY_SYS_Create( player, system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-C\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_D( ISS_SYS* system )
{
#ifdef ISS_D_ENABLE
	PLAYER_WORK* player;
	player = GetPlayerWork( system );

  GF_ASSERT( system );
  GF_ASSERT( system->gameData );
  GF_ASSERT( system->issD == NULL );

	system->issD = 
    ISS_DUNGEON_SYS_Create( system->gameData, player, system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-D\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_Z( ISS_SYS* system )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ == NULL );

  system->issZ = ISS_ZONE_SYS_Create( system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-Z\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_S( ISS_SYS* system )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS == NULL );

  system->issS = ISS_SWITCH_SYS_Create( system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-S\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void CreateISS_B( ISS_SYS* system )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB == NULL );

  system->issB = ISS_3DS_SYS_Create( system->heapID );

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: create ISS-B\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 道ISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
// 各ISSの破棄
static void DeleteISS_R( ISS_SYS* system )
{
#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issR );

	ISS_ROAD_SYS_Delete( system->issR );
  system->issR = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-R\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void DeleteISS_C( ISS_SYS* system )
{
#ifdef ISS_C_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issC );

	ISS_CITY_SYS_Delete( system->issC );
  system->issC = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-C\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void DeleteISS_D( ISS_SYS* system )
{
#ifdef ISS_D_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issD );

	ISS_DUNGEON_SYS_Delete( system->issD );
  system->issD = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-D\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void DeleteISS_Z( ISS_SYS* system )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ );

  ISS_ZONE_SYS_Delete( system->issZ );
  system->issZ = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-Z\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void DeleteISS_S( ISS_SYS* system )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  ISS_SWITCH_SYS_Delete( system->issS );
  system->issS = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-S\n" );
#endif
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void DeleteISS_B( ISS_SYS* system )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  ISS_3DS_SYS_Delete( system->issB );
  system->issB = NULL;

#ifdef PRINT_ENABLE
  OS_TFPrintf( PRINT_TARGET, "ISS: delete ISS-B\n" );
#endif
#endif
} 

//-----------------------------------------------------------------------------
/**
 * @brief 道ISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_R( ISS_SYS* system )
{
#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issR );

  // 秒間30フレームで動作させる
	if( system->frame % 2 == 0 ) {
    ISS_ROAD_SYS_Update( system->issR );
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_C( ISS_SYS* system )
{
#ifdef ISS_C_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issC );

  // 秒間30フレームで動作させる
	if( system->frame % 2 == 0 ) {
    ISS_CITY_SYS_Main( system->issC );
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_D( ISS_SYS* system )
{
#ifdef ISS_D_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issD );

	ISS_DUNGEON_SYS_Update( system->issD );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_Z( ISS_SYS* system )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ );

  ISS_ZONE_SYS_Update( system->issZ );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_S( ISS_SYS* system )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  ISS_SWITCH_SYS_Update( system->issS );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSのメイン動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void MainISS_B( ISS_SYS* system )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  // 秒間30フレームで動作させる
	if( system->frame % 2 == 0 ) {
    ISS_3DS_SYS_Main( system->issB );
  }
#endif
} 

//-----------------------------------------------------------------------------
/**
 * @brief 道ISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_R( ISS_SYS* system )
{
#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issR );

  ISS_ROAD_SYS_On( system->issR );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_C( ISS_SYS* system )
{
#ifdef ISS_C_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issC );

  ISS_CITY_SYS_On( system->issC );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_D( ISS_SYS* system )
{
#ifdef ISS_D_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issD );

  ISS_DUNGEON_SYS_On( system->issD );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_Z( ISS_SYS* system )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ );

  ISS_ZONE_SYS_On( system->issZ, GetCurrentZoneID( system ) );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_S( ISS_SYS* system )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  ISS_SWITCH_SYS_On( system->issS );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSを起動する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BootISS_B( ISS_SYS* system )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  ISS_3DS_SYS_On( system->issB );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 道ISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_R( ISS_SYS* system )
{
#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issR );

  ISS_ROAD_SYS_Off( system->issR );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_C( ISS_SYS* system )
{
#ifdef ISS_C_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issC );

  ISS_CITY_SYS_Off( system->issC );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_D( ISS_SYS* system )
{
#ifdef ISS_D_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issD );

  ISS_DUNGEON_SYS_Off( system->issD );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_Z( ISS_SYS* system )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ );

  ISS_ZONE_SYS_Off( system->issZ );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_S( ISS_SYS* system )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  ISS_SWITCH_SYS_Off( system->issS );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSを停止する
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void StopISS_B( ISS_SYS* system )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  ISS_3DS_SYS_Off( system->issB );
#endif
}


//-----------------------------------------------------------------------------
/**
 * @brief 道ISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_R( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_R_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issR );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 街ISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_C( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_C_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issC );

	ISS_CITY_SYS_ZoneChange( system->issC, nextZoneID );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ダンジョンISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_D( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_D_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issD );

	ISS_DUNGEON_SYS_ZoneChange( system->issD, nextZoneID );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief ゾーンISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_Z( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_Z_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issZ );

  ISS_ZONE_SYS_ZoneChange( system->issZ, nextZoneID );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief スイッチISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_S( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_S_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issS );

  ISS_SWITCH_SYS_ZoneChange( system->issS, nextZoneID );
#endif
}

//-----------------------------------------------------------------------------
/**
 * @brief 橋ISSにゾーンの変更を通知する
 *
 * @param system
 * @param nextZoneID 変更後のゾーンID
 */
//-----------------------------------------------------------------------------
static void ZoneChangeISS_B( ISS_SYS* system, u16 nextZoneID )
{
#ifdef ISS_B_ENABLE
  GF_ASSERT( system );
  GF_ASSERT( system->issB );

  ISS_3DS_SYS_ZoneChange( system->issB );
#endif
} 

//-----------------------------------------------------------------------------
/**
 * @brief 自機を取得する
 *
 * @parma system
 *
 * @return PLAYER_WORK
 */
//-----------------------------------------------------------------------------
static PLAYER_WORK* GetPlayerWork( const ISS_SYS* system )
{
  GF_ASSERT( system );
  GF_ASSERT( system->gameData );

	return GAMEDATA_GetMyPlayerWork( system->gameData );
}

//-----------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを取得する
 *
 * @parma system
 *
 * @return 現在の自機の位置から取得したゾーンID
 */
//-----------------------------------------------------------------------------
static u16 GetCurrentZoneID( const ISS_SYS* system )
{
  GF_ASSERT( system );

  return PLAYERWORK_getZoneID( GetPlayerWork(system) );
}

//-----------------------------------------------------------------------------
/**
 * @brief 再生中のBGMを取得する
 *
 * @return 現在再生中のBGMのシーケンス番号
 */
//-----------------------------------------------------------------------------
static u32 GetPlayingBGM( void )
{
  return PMSND_GetBGMsoundNo();
}

//-----------------------------------------------------------------------------
/**
 * @brief 再生中のBGMのISSタイプを取得する
 *
 * @param system
 *
 * @return 現在再生中のBGMのISSタイプ ( ISS_TYPE_xxxx )
 */
//-----------------------------------------------------------------------------
static u8 GetISSTypeOfPlayingBGM( const ISS_SYS* system )
{
	u8 type;
	BGM_INFO_SYS* BGMInfo;

  GF_ASSERT( system );
  GF_ASSERT( system->gameData );
  
	// 再生中のBGMのISSタイプを取得
	BGMInfo = GAMEDATA_GetBGMInfoSys( system->gameData ); 
	type    = BGM_INFO_GetIssType( BGMInfo, GetPlayingBGM() ); 

  return type;
}

//-----------------------------------------------------------------------------
/**
 * @brief BGMの変化をチェックする
 *
 * @param system
 *
 * @return BGMが変化した場合 TRUE, そうでなければ FALSE
 */
//-----------------------------------------------------------------------------
static BOOL CheckBGMChange( ISS_SYS* system )
{
  GF_ASSERT( system );

  // BGM変化なし
  if( system->soundIdx == GetPlayingBGM() ) { return FALSE; }

  // BGM変化あり
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief BGM変更処理
 *
 * @param system
 */
//-----------------------------------------------------------------------------
static void BGMChange( ISS_SYS* system )
{
	u8 ISSType;

	// 再生中のBGMのISSタイプを取得
	ISSType = GetISSTypeOfPlayingBGM( system );

  // 各ISSシステムを停止
  switch( ISSType ) {
  case ISS_TYPE_LOAD:
      StopISS_C( system ); StopISS_D( system ); StopISS_Z( system );
      StopISS_S( system ); StopISS_B( system ); break;
  case ISS_TYPE_CITY:
      StopISS_R( system ); StopISS_D( system ); StopISS_Z( system );
      StopISS_S( system ); StopISS_B( system ); break;
  case ISS_TYPE_DUNGEON:
      StopISS_R( system ); StopISS_C( system ); StopISS_Z( system );
      StopISS_S( system ); StopISS_B( system ); break;
  case ISS_TYPE_ZONE:
      StopISS_R( system ); StopISS_C( system ); StopISS_D( system );
      StopISS_S( system ); StopISS_B( system ); break;
  case ISS_TYPE_SWITCH:
      StopISS_R( system ); StopISS_C( system ); StopISS_D( system );
      StopISS_Z( system ); StopISS_B( system ); break;
  case ISS_TYPE_BRIDGE:
      StopISS_R( system ); StopISS_C( system ); StopISS_D( system );
      StopISS_Z( system ); StopISS_S( system ); break;
  default:
      StopISS_R( system ); StopISS_C( system ); StopISS_D( system );
      StopISS_Z( system ); StopISS_S( system ); StopISS_B( system );
      break;
  }

  // 再生中のBGMに対応するISSを起動
  switch( ISSType ) {
  case ISS_TYPE_LOAD:    BootISS_R( system ); break;
  case ISS_TYPE_CITY:    BootISS_C( system ); break;
  case ISS_TYPE_DUNGEON: BootISS_D( system ); break;
  case ISS_TYPE_ZONE:    BootISS_Z( system ); break;
  case ISS_TYPE_SWITCH:  BootISS_S( system ); break;
  case ISS_TYPE_BRIDGE:  BootISS_B( system ); break;
  } 
}
