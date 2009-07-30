/////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_unit_load.h
 * @brief  道路ISSユニット
 * @author obata_toshihiro
 * @date   2009.07.29
 */
/////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_unit_load.h"
#include "field_sound.h"
#include "gamesystem/playerwork.h"



//===========================================================================================
/**
 * @brief 定数
 */
//===========================================================================================
#define MIN_VOLUME     (0)	// 最低音量
#define MAX_VOLUME   (127)	// 最大音量
#define FADE_IN_SPEED  (4)	// フェード・イン速度
#define FADE_OUT_SPEED (4)	// フェード・アウト速度


//===========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//===========================================================================================

// 2つのベクトルが等しいかどうかを判定する
BOOL IsVecEqual( const VecFx32* p_vec1, const VecFx32* p_vec2 );



//===========================================================================================
/**
 * @breif 道路ISSユニット構造体
 */
//===========================================================================================
struct _ISS_UNIT_LOAD
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
 * @brief  道路ISSユニットを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  zone_id  ゾーンID
 * @param  heap_id  使用するヒープID
 * 
 * @return 道路ISSユニット
 */
//----------------------------------------------------------------------------
ISS_UNIT_LOAD* ISS_UNIT_LOAD_Create( PLAYER_WORK* p_player, u16 zone_id, HEAPID heap_id )
{
	ISS_UNIT_LOAD* p_unit;

	// メモリ確保
	p_unit = (ISS_UNIT_LOAD*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_UNIT_LOAD ) );

	// 初期設定
	p_unit->heapID        = heap_id;
	p_unit->isActive      = FALSE;
	p_unit->volume        = MIN_VOLUME;
	p_unit->pPlayer       = p_player;
	p_unit->prevPlayerPos = *( PLAYERWORK_getPosition( p_player ) );
	
	// 作成した道路ISSユニットを返す
	return p_unit;
}

//----------------------------------------------------------------------------
/**
 * @brief  道路ISSユニットを破棄する
 *
 * @param p_unit 破棄する道路ISSユニット 
 */
//----------------------------------------------------------------------------
void ISS_UNIT_LOAD_Delete( ISS_UNIT_LOAD* p_unit )
{
	GFL_HEAP_FreeMemory( p_unit );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_unit 操作対象のユニット
 */
//----------------------------------------------------------------------------
void ISS_UNIT_LOAD_Update( ISS_UNIT_LOAD* p_unit )
{
	const VecFx32* p_player_pos = NULL;

	// 起動していなければ, 何もしない
	if( p_unit->isActive != TRUE ) return;

	// 主人公の座標を取得
	p_player_pos = PLAYERWORK_getPosition( p_unit->pPlayer );

	// 音量調整
	if( IsVecEqual( p_player_pos, &p_unit->prevPlayerPos ) )
	{
		p_unit->volume -= FADE_OUT_SPEED;
		if( p_unit->volume < MIN_VOLUME ) p_unit->volume = MIN_VOLUME;
	}
	else
	{
		p_unit->volume += FADE_IN_SPEED;
		if( MAX_VOLUME < p_unit->volume ) p_unit->volume = MAX_VOLUME;
	}
	FIELD_SOUND_ChangeBGMActionVolume( p_unit->volume );

	// 記憶主人公座標を更新
	p_unit->prevPlayerPos = *p_player_pos;

	// DEBUG: デバッグ出力
	if( p_unit->isActive )
	{
		OBATA_Printf( "-----------------------\n" );
		OBATA_Printf( "Load ISS Unit is active\n" );
		OBATA_Printf( "volume = %d\n", p_unit->volume );
	}
}

//----------------------------------------------------------------------------
/**
 * @brief 動作状態を設定する
 *
 * @param active 動作させるかどうか
 */
//----------------------------------------------------------------------------
extern void ISS_UNIT_LOAD_SetActive( ISS_UNIT_LOAD* p_unit, BOOL active )
{
	p_unit->isActive = active;
	p_unit->volume   = MIN_VOLUME;	// 音量を最小に戻す
}

//----------------------------------------------------------------------------
/**
 * @breif 動作状態を取得する
 *
 * @param p_unit 状態を調べるISSユニット
 * 
 * @return 動作中かどうか
 */
//----------------------------------------------------------------------------
extern BOOL ISS_UNIT_LOAD_IsActive( const ISS_UNIT_LOAD* p_unit )
{
	return p_unit->isActive;
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
