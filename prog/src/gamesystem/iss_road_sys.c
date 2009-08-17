/////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_road_sys.h
 * @brief  道路ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.29
 */
/////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_road_sys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"



//===========================================================================================
/**
 * @brief 定数
 */
//===========================================================================================
#define MIN_VOLUME     (0)	// 最低音量
#define MAX_VOLUME   (127)	// 最大音量
#define FADE_IN_SPEED  (8)	// フェード・イン速度
#define FADE_OUT_SPEED (8)	// フェード・アウト速度


//===========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//===========================================================================================

// 2つのベクトルが等しいかどうかを判定する
BOOL IsVecEqual( const VecFx32* p_vec1, const VecFx32* p_vec2 );



//===========================================================================================
/**
 * @breif 道路ISSシステム構造体
 */
//===========================================================================================
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


//===========================================================================================
/**
 * 公開関数の実装
 */
//===========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief  道路ISSシステムを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  heap_id  使用するヒープID
 * 
 * @return 道路ISSシステム
 */
//----------------------------------------------------------------------------
ISS_ROAD_SYS* ISS_ROAD_SYS_Create( PLAYER_WORK* p_player, HEAPID heap_id )
{
	ISS_ROAD_SYS* p_sys;

	// メモリ確保
	p_sys = (ISS_ROAD_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_ROAD_SYS ) );

	// 初期設定
	p_sys->heapID        = heap_id;
	p_sys->isActive      = FALSE;
	p_sys->volume        = MIN_VOLUME;
	p_sys->pPlayer       = p_player;
	p_sys->prevPlayerPos = *( PLAYERWORK_getPosition( p_player ) );
	
	// 作成した道路ISSシステムを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  道路ISSシステムを破棄する
 *
 * @param p_sys 破棄する道路ISSシステム 
 */
//----------------------------------------------------------------------------
void ISS_ROAD_SYS_Delete( ISS_ROAD_SYS* p_sys )
{
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_sys 操作対象のシステム
 */
//----------------------------------------------------------------------------
void ISS_ROAD_SYS_Update( ISS_ROAD_SYS* p_sys )
{
	const VecFx32* p_player_pos = NULL;

	// 起動していなければ, 何もしない
	if( p_sys->isActive != TRUE ) return;

	// 主人公の座標を取得
	p_player_pos = PLAYERWORK_getPosition( p_sys->pPlayer );

	// 音量調整
	if( IsVecEqual( p_player_pos, &p_sys->prevPlayerPos ) )
	{
		p_sys->volume -= FADE_OUT_SPEED;
		if( p_sys->volume < MIN_VOLUME ) p_sys->volume = MIN_VOLUME;
		OBATA_Printf( "Load ISS Volume DOWN\n" );
	}
	else
	{
		p_sys->volume += FADE_IN_SPEED;
		if( MAX_VOLUME < p_sys->volume ) p_sys->volume = MAX_VOLUME;
		OBATA_Printf( "Load ISS Volume UP\n" );
	}
	FIELD_SOUND_ChangeBGMActionVolume( p_sys->volume );

	// 記憶主人公座標を更新
	p_sys->prevPlayerPos = *p_player_pos;

	// DEBUG: デバッグ出力
  /*
	if( p_sys->isActive )
	{
		OBATA_Printf( "-----------------------\n" );
		OBATA_Printf( "Load ISS Unit is active\n" );
		OBATA_Printf( "volume = %d\n", p_sys->volume );
	}
  */
}

//----------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param p_sys 起動するシステム
 */
//----------------------------------------------------------------------------
void ISS_ROAD_SYS_On( ISS_ROAD_SYS* p_sys )
{
	p_sys->isActive = TRUE;
	p_sys->volume   = MIN_VOLUME;	// 音量を最小に戻す
}

//----------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param p_sys 停止させるシステム
 */
//----------------------------------------------------------------------------
void ISS_ROAD_SYS_Off( ISS_ROAD_SYS* p_sys )
{
	p_sys->isActive = FALSE;
}

//----------------------------------------------------------------------------
/**
 * @breif 動作状態を取得する
 *
 * @param p_sys 状態を調べるISSシステム
 * 
 * @return 動作中かどうか
 */
//----------------------------------------------------------------------------
BOOL ISS_ROAD_SYS_IsOn( const ISS_ROAD_SYS* p_sys )
{
	return p_sys->isActive;
}



//===========================================================================================
/**
 * @brief 非公開関数の実装
 */
//===========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief 2つのベクトルが等しいかどうかを判定する
 *
 * @param p_vec1 判定するベクトル1
 * @param p_vec2 判定するベクトル2
 */
//----------------------------------------------------------------------------
BOOL IsVecEqual( const VecFx32* p_vec1, const VecFx32* p_vec2 )
{
	return ( ( p_vec1->x == p_vec2->x ) &&
			 ( p_vec1->y == p_vec2->y ) &&
			 ( p_vec1->z == p_vec2->z ) );
}
