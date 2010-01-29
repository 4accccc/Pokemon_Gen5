//////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_road_sys.h
 * @brief  道路ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.29
 */
//////////////////////////////////////////////////////////////////////////////////
#include "iss_road_sys.h"
#include "sound/pm_sndsys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"


//================================================================================
// ■定数
//================================================================================
#define PRINT_DEST     (2)    // デバッグ情報の出力先
#define MIN_VOLUME     (0)	  // 最低音量
#define MAX_VOLUME     (127)	// 最大音量
#define FADE_IN_SPEED  (8)	  // フェード イン速度
#define FADE_OUT_SPEED (8)	  // フェード アウト速度
#define TRACKBIT ((1<<(9-1))|(1<<(10-1))) // 操作トラック(9,10)


//================================================================================
// ■非公開関数
//================================================================================
// システム制御
static void BootSystem( ISS_ROAD_SYS* system );
static void StopSystem( ISS_ROAD_SYS* system );
static void SystemMain( ISS_ROAD_SYS* system ); 
// 自機の監視
static BOOL CheckPlayerMove( ISS_ROAD_SYS* system ); 
// ボリューム制御
static void VolumeUp  ( ISS_ROAD_SYS* system );
static void VolumeDown( ISS_ROAD_SYS* system );
static void ChangeTrackVolume( const ISS_ROAD_SYS* system );


//================================================================================
// ■道路ISSシステム
//================================================================================
struct _ISS_ROAD_SYS
{
	BOOL boot;               // 起動しているかどうか
	int  targetTrackVolume;  // 操作対象トラックのボリューム

	PLAYER_WORK* player;        // 監視対象プレイヤー
	VecFx32      prevPlayerPos; // 前回監視時のプレイヤー座標
};


//================================================================================
// ■公開関数
//================================================================================

//--------------------------------------------------------------------------------
/**
 * @brief 道路ISSシステムを作成する
 *
 * @param player 監視対象のプレイヤー
 * @param heapID 使用するヒープID
 * 
 * @return 道路ISSシステム
 */
//--------------------------------------------------------------------------------
ISS_ROAD_SYS* ISS_ROAD_SYS_Create( PLAYER_WORK* player, HEAPID heapID )
{
	ISS_ROAD_SYS* system;

	// 生成
	system = (ISS_ROAD_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof( ISS_ROAD_SYS ) );

	// 初期化
	system->boot              = FALSE;
	system->targetTrackVolume = MIN_VOLUME;
	system->player            = player;
	system->prevPlayerPos     = *( PLAYERWORK_getPosition( player ) );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-R: create\n" );
	
	return system;
}

//--------------------------------------------------------------------------------
/**
 * @brief 道路ISSシステムを破棄する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Delete( ISS_ROAD_SYS* system )
{
	GFL_HEAP_FreeMemory( system );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-R: delete\n" );
}

//--------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param system 操作対象のシステム
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Update( ISS_ROAD_SYS* system )
{ 
  SystemMain( system );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_On( ISS_ROAD_SYS* system )
{
  BootSystem( system );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param system
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Off( ISS_ROAD_SYS* system )
{
  StopSystem( system );
}


//================================================================================
// ■非公開関数
//================================================================================

//--------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void BootSystem( ISS_ROAD_SYS* system )
{
  // 起動済み
  if( system->boot ){ return; }

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-R: boot\n" );

  // 起動
	system->boot = TRUE;
	system->targetTrackVolume = MIN_VOLUME;
  ChangeTrackVolume( system ); 
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void StopSystem( ISS_ROAD_SYS* system )
{
  // 停止済み
  if( system->boot == FALSE ){ return; }

  // 停止
	system->boot = FALSE; 

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-R: stop\n" );
} 

//--------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void SystemMain( ISS_ROAD_SYS* system )
{
	// 起動してない
	if( system->boot == FALSE ){ return; }

  // ボリューム更新
  if( CheckPlayerMove( system ) )
  {
    VolumeUp( system );
  }
  else
  {
    VolumeDown( system );
  }
}

//--------------------------------------------------------------------------------
/**
 * @brief プレイヤーが動いているかどうかを判定する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static BOOL CheckPlayerMove( ISS_ROAD_SYS* system )
{
  BOOL moving;
	const VecFx32* playerPos;

	// 主人公の座標を取得
	playerPos = PLAYERWORK_getPosition( system->player );

  // 動いているかどうか
  if( (playerPos->x == system->prevPlayerPos.x) &&
      (playerPos->y == system->prevPlayerPos.y) &&
      (playerPos->z == system->prevPlayerPos.z) )
  {
    moving = FALSE; 
  }
  else
  { 
    moving = TRUE; 
  }

  // 主人公の座標を記憶
  system->prevPlayerPos = *playerPos;

  return moving;
}

//--------------------------------------------------------------------------------
/**
 * @brief ボリュームを上げる
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void VolumeUp( ISS_ROAD_SYS* system )
{
  int nextVolume;

  // すでに最大
  if( MAX_VOLUME <= system->targetTrackVolume ){ return; }

  // 更新後のボリュームを決定
  nextVolume = system->targetTrackVolume + FADE_IN_SPEED;
  if( MAX_VOLUME < nextVolume ){ nextVolume = MAX_VOLUME; }

  // ボリュームUP
  system->targetTrackVolume = nextVolume;
  ChangeTrackVolume( system );
}

//--------------------------------------------------------------------------------
/**
 * @brief ボリュームを下げる
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void VolumeDown( ISS_ROAD_SYS* system )
{
  int nextVolume;

  // すでに最小
  if( system->targetTrackVolume <= MIN_VOLUME ){ return; }

  // 更新後のボリュームを決定
  nextVolume = system->targetTrackVolume - FADE_OUT_SPEED;
  if( nextVolume < MIN_VOLUME ){ nextVolume = MIN_VOLUME; }

  // ボリュームDOWN
  system->targetTrackVolume = nextVolume;
  ChangeTrackVolume( system );
}

//--------------------------------------------------------------------------------
/**
 * @brief 操作対象トラックのボリュームを更新する
 *
 * @param system
 */
//--------------------------------------------------------------------------------
static void ChangeTrackVolume( const ISS_ROAD_SYS* system )
{
  GF_ASSERT( system->boot );                             // 起動してない
  GF_ASSERT( MIN_VOLUME <= system->targetTrackVolume );  // 音量がおかしい
  GF_ASSERT( system->targetTrackVolume <= MAX_VOLUME );  // 音量がおかしい

  // ボリューム更新
  PMSND_ChangeBGMVolume( TRACKBIT, system->targetTrackVolume );

  // DEBUG:
  OS_TFPrintf( PRINT_DEST, "ISS-R: change volume ==> %d\n", system->targetTrackVolume );
}
