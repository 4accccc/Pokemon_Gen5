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
#define MIN_VOLUME     (0)	// 最低音量
#define MAX_VOLUME   (127)	// 最大音量
#define FADE_IN_SPEED  (8)	// フェード・イン速度
#define FADE_OUT_SPEED (8)	// フェード・アウト速度
#define TRACKBIT ((1<<(9-1))|(1<<(10-1))) // 操作トラック(9,10)


//================================================================================
// ■非公開関数のプロトタイプ宣言
//================================================================================
static BOOL IsVecEqual( const VecFx32* p_vec1, const VecFx32* p_vec2 );
static void BootSystem( ISS_ROAD_SYS* sys );
static void StopSystem( ISS_ROAD_SYS* sys );
static void UpdateVolume( ISS_ROAD_SYS* sys );
static void AddVolume( ISS_ROAD_SYS* sys, int val );


//================================================================================
// ■道路ISSシステム構造体
//================================================================================
struct _ISS_ROAD_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 起動状態
	BOOL isActive;

	// 音量
	int volume;

	// 監視対象プレイヤー
	PLAYER_WORK* pPlayer;

	// 前回監視時のプレイヤー座標
	VecFx32 prevPlayerPos;
};


//================================================================================
// ■公開関数
//================================================================================

//--------------------------------------------------------------------------------
/**
 * @brief  道路ISSシステムを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  heap_id  使用するヒープID
 * 
 * @return 道路ISSシステム
 */
//--------------------------------------------------------------------------------
ISS_ROAD_SYS* ISS_ROAD_SYS_Create( PLAYER_WORK* p_player, HEAPID heap_id )
{
	ISS_ROAD_SYS* sys;

	// メモリ確保
	sys = (ISS_ROAD_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_ROAD_SYS ) );

	// 初期設定
	sys->heapID        = heap_id;
	sys->isActive      = FALSE;
	sys->volume        = MIN_VOLUME;
	sys->pPlayer       = p_player;
	sys->prevPlayerPos = *( PLAYERWORK_getPosition( p_player ) );

  // DEBUG:
  OBATA_Printf( "ISS-R: create\n" );
	
	// 作成した道路ISSシステムを返す
	return sys;
}

//--------------------------------------------------------------------------------
/**
 * @brief  道路ISSシステムを破棄する
 *
 * @param sys 破棄する道路ISSシステム 
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Delete( ISS_ROAD_SYS* sys )
{
	GFL_HEAP_FreeMemory( sys );

  // DEBUG:
  OBATA_Printf( "ISS-R: delete\n" );
}

//--------------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param sys 操作対象のシステム
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Update( ISS_ROAD_SYS* sys )
{ 
	// 起動してない
	if( sys->isActive != TRUE ){ return; }

  // 音量更新
  UpdateVolume( sys );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_On( ISS_ROAD_SYS* sys )
{
  BootSystem( sys );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param sys 停止させるシステム
 */
//--------------------------------------------------------------------------------
void ISS_ROAD_SYS_Off( ISS_ROAD_SYS* sys )
{
  StopSystem( sys );
}

//--------------------------------------------------------------------------------
/**
 * @breif 動作状態を取得する
 *
 * @param sys 状態を調べるISSシステム
 * 
 * @return 動作中かどうか
 */
//--------------------------------------------------------------------------------
BOOL ISS_ROAD_SYS_IsOn( const ISS_ROAD_SYS* sys )
{
	return sys->isActive;
}



//================================================================================
// ■非公開関数
//================================================================================

//--------------------------------------------------------------------------------
/**
 * @brief 2つのベクトルが等しいかどうかを判定する
 *
 * @param p_vec1 判定するベクトル1
 * @param p_vec2 判定するベクトル2
 *
 * @return ベクトルが等しいなら TRUE
 */
//--------------------------------------------------------------------------------
static BOOL IsVecEqual( const VecFx32* p_vec1, const VecFx32* p_vec2 )
{
	return ( ( p_vec1->x == p_vec2->x ) &&
			     ( p_vec1->y == p_vec2->y ) &&
			     ( p_vec1->z == p_vec2->z ) );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 道路ISSシステム
 */
//--------------------------------------------------------------------------------
static void BootSystem( ISS_ROAD_SYS* sys )
{
  // 起動中
  if( sys->isActive ){ return; }

  // 起動
	sys->isActive = TRUE;
	sys->volume   = MIN_VOLUME;	// 音量を最小に設定
  PMSND_ChangeBGMVolume( TRACKBIT, MIN_VOLUME );

  // DEBUG:
  OBATA_Printf( "ISS-R: boot\n" );
}

//--------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 道路ISSシステム
 */
//--------------------------------------------------------------------------------
static void StopSystem( ISS_ROAD_SYS* sys )
{
  // 停止中
  if( !sys->isActive ){ return; }

  // 停止
	sys->isActive = FALSE; 
  // 操作していたトラックを元に戻す
  //PMSND_ChangeBGMVolume( TRACKBIT, 127 ); 

  // DEBUG:
  OBATA_Printf( "ISS-R: stop\n" );
} 

//--------------------------------------------------------------------------------
/**
 * @brief 音量を更新する
 *
 * @param sys 道路ISSシステム
 */
//--------------------------------------------------------------------------------
static void UpdateVolume( ISS_ROAD_SYS* sys )
{
	const VecFx32* pos = NULL;
  int         volume = 0; // ボリューム変化量

  GF_ASSERT( sys->isActive );
  GF_ASSERT( sys->pPlayer );

	// 主人公の座標を取得
	pos = PLAYERWORK_getPosition( sys->pPlayer );

	// 音量変化量を決定
	if( IsVecEqual( pos, &sys->prevPlayerPos ) )
	{
		volume = -FADE_OUT_SPEED;
	}
	else
	{ 
		volume = FADE_IN_SPEED;
	}

  // 音量を変更
  AddVolume( sys, volume );

	// 記憶主人公座標を更新
	sys->prevPlayerPos = *pos; 
}

//--------------------------------------------------------------------------------
/**
 * @brief 音量を変更する
 *
 * @param sys ISS-Rシステム
 * @param val 変更量
 */
//--------------------------------------------------------------------------------
static void AddVolume( ISS_ROAD_SYS* sys, int val )
{
  int prev_volume; // 変化前のボリューム

  // 起動していない
  GF_ASSERT( sys->isActive );

  // 変化量ゼロ
  if( val == 0 ){ return; } 
  
  // 音量を更新
  prev_volume  = sys->volume;
  sys->volume += val;
  if( sys->volume < MIN_VOLUME ){ sys->volume = MIN_VOLUME; }  // 最小値補正
  if( sys->volume > MAX_VOLUME ){ sys->volume = MAX_VOLUME; }  // 最大値補正
  if( sys->volume == prev_volume ){ return; }                  // 変化無し
  PMSND_ChangeBGMVolume( TRACKBIT, sys->volume );

  // DEBUG: 
  OBATA_Printf( "ISS-R: change volume ==> %d\n", sys->volume );
}
