///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_unit.h
 * @brief  ISSユニット
 * @author obata_toshihiro
 * @date   2009.07.16
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include "iss_unit.h"
#include "iss_unit_city.h"
#include "iss_unit_load.h"
#include "field_sound.h"
#include "sound/bgm_info.h"
#include "../../../resource/sound/bgm_info/iss_type.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"


//=========================================================================================
/**
 * @brief 定数・マクロ
 */
//=========================================================================================


//=========================================================================================
/**
 * @brief ISSシステムワーク
 */
//=========================================================================================
struct _ISS_UNIT
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象ゲームデータ
	GAMEDATA* pGameData;

	// 自転車状態かどうか
	BOOL cycle;

	// なみのり状態かどうか
	BOOL surfing;

	// 各ISSユニット
	ISS_UNIT_CITY* pCityUnit;	// 街
	ISS_UNIT_LOAD* pLoadUnit;	// 道路
};


//=========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//=========================================================================================

// 主人公の自転車状態を監視する
void CycleCheck( ISS_UNIT* p_sys );

// 主人公のなみのり状態を監視する
void SurfingCheck( ISS_UNIT* p_sys );


//=========================================================================================
/**
 * @brief 公開関数の実装
 */
//=========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief  ISSユニットを作成する
 *
 * @param p_gdata	監視対象ゲームデータ
 * @param zone_id   ゾーンID
 * @param heap_id   使用するヒープID
 * 
 * @return ISSユニット
 */
//----------------------------------------------------------------------------
ISS_UNIT* ISS_UNIT_Create( GAMEDATA* p_gdata, u16 zone_id, HEAPID heap_id )
{
	ISS_UNIT* p_sys;
	PLAYER_WORK* p_player;

	// 監視対象の主人公を取得
	p_player = GAMEDATA_GetMyPlayerWork( p_gdata );

	// メモリ確保
	p_sys = (ISS_UNIT*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_UNIT ) );

	// 初期設定
	p_sys->heapID      = heap_id;
	p_sys->pGameData   = p_gdata;
	p_sys->cycle       = FALSE;
	p_sys->surfing     = FALSE;
	p_sys->pCityUnit   = ISS_UNIT_CITY_Create( p_player, zone_id, heap_id );
	p_sys->pLoadUnit   = ISS_UNIT_LOAD_Create( p_player, zone_id, heap_id );
	ISS_UNIT_ZoneChange( p_sys, zone_id );
	
	// 作成したISSユニットを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  ISSユニットを破棄する
 * * @param p_unit 破棄するISSユニット 
 */
//----------------------------------------------------------------------------
void ISS_UNIT_Delete( ISS_UNIT* p_unit )
{
	// 各ISSユニットを破棄
	ISS_UNIT_CITY_Delete( p_unit->pCityUnit );
	ISS_UNIT_LOAD_Delete( p_unit->pLoadUnit );

	// 本体を破棄
	GFL_HEAP_FreeMemory( p_unit );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_unit 操作対象のユニット
 */
//----------------------------------------------------------------------------
void ISS_UNIT_Update( ISS_UNIT* p_unit )
{
	// 自転車チェック
	CycleCheck( p_unit );

	// なみのりチェック
	SurfingCheck( p_unit );

	// 街ISS
	ISS_UNIT_CITY_Update( p_unit->pCityUnit );

	// 道路ISS
	ISS_UNIT_LOAD_Update( p_unit->pLoadUnit );
}
	

//---------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param p_unit       通知対象のISSユニット
 * @param next_zone_id 新しいゾーンID
 */
//---------------------------------------------------------------------------
void ISS_UNIT_ZoneChange( ISS_UNIT* p_unit, u16 next_zone_id )
{
	PLAYER_WORK*     p_player;
	PLAYER_MOVE_FORM form;
	BGM_INFO_SYS*    p_bgm_info_sys;
	int              bgm_index;
	int              iss_type;

	// 自転車orなみのり中なら, ISSに変更はない
	if( p_unit->cycle | p_unit->surfing ) return;

	// 各オブジェクトを取得
	p_player       = GAMEDATA_GetMyPlayerWork( p_unit->pGameData );
	form           = PLAYERWORK_GetMoveForm( p_player );
	p_bgm_info_sys = GAMEDATA_GetBGMInfoSys( p_unit->pGameData );

	// 切り替え先ゾーンのISSタイプを取得
	bgm_index = FIELD_SOUND_GetFieldBGMNo( p_unit->pGameData, form, next_zone_id );
	iss_type  = BGM_INFO_GetIssType( p_bgm_info_sys, bgm_index );

	// 街ISS
	ISS_UNIT_CITY_ZoneChange( p_unit->pCityUnit, next_zone_id );

	// 道路ISS
	if( iss_type == ISS_TYPE_LOAD )
	{
		ISS_UNIT_LOAD_SetActive( p_unit->pLoadUnit, TRUE );
	}
	else
	{
		ISS_UNIT_LOAD_SetActive( p_unit->pLoadUnit, FALSE );
	}

	// DEBUG:
	OBATA_Printf( "ISS_UNIT_ZoneChange()\n" );
	OBATA_Printf( "seq = %d, iss_type = %d\n", bgm_index, iss_type );
}


//=========================================================================================
/**
 * @param 非公開関数の実装
 */
//=========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief 主人公の自転車状態を監視する
 */
//----------------------------------------------------------------------------
void CycleCheck( ISS_UNIT* p_sys )
{
	PLAYER_WORK*     p_player;
	PLAYER_MOVE_FORM form;

	// 自機の状態を取得
	p_player = GAMEDATA_GetMyPlayerWork( p_sys->pGameData );
	form     = PLAYERWORK_GetMoveForm( p_player );

	// 自転車に乗るのを検出したら, ISSを停止
	if( ( p_sys->cycle == FALSE ) && ( form == PLAYER_MOVE_FORM_CYCLE ) )
	{
		p_sys->cycle = TRUE;
		ISS_UNIT_CITY_SetActive( p_sys->pCityUnit, FALSE );
		ISS_UNIT_LOAD_SetActive( p_sys->pLoadUnit, FALSE );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( p_sys->cycle == TRUE ) && ( form != PLAYER_MOVE_FORM_CYCLE ) )
	{
		p_sys->cycle = FALSE;
		ISS_UNIT_ZoneChange( p_sys, PLAYERWORK_getZoneID( p_player ) );
	}
}

//----------------------------------------------------------------------------
/**
 * @brief 主人公のなみのり状態を監視する
 */
//----------------------------------------------------------------------------
void SurfingCheck( ISS_UNIT* p_sys )
{
	PLAYER_WORK*     p_player;
	PLAYER_MOVE_FORM form;

	// 自機の状態を取得
	p_player = GAMEDATA_GetMyPlayerWork( p_sys->pGameData );
	form     = PLAYERWORK_GetMoveForm( p_player );

	// なみのり開始を検出したら, ISSを停止
	if( ( p_sys->surfing == FALSE ) && ( form == PLAYER_MOVE_FORM_SWIM ) )
	{
		p_sys->surfing = TRUE;
		ISS_UNIT_CITY_SetActive( p_sys->pCityUnit, FALSE );
		ISS_UNIT_LOAD_SetActive( p_sys->pLoadUnit, FALSE );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( p_sys->surfing == TRUE ) && ( form != PLAYER_MOVE_FORM_SWIM ) )
	{
		p_sys->surfing = FALSE;
		ISS_UNIT_ZoneChange( p_sys, PLAYERWORK_getZoneID( p_player ) );
	}
}
