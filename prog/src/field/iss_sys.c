///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_sys.h
 * @brief  ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include "iss_sys.h"
#include "iss_city_sys.h"
#include "iss_road_sys.h"
#include "field_sound.h"
#include "sound/bgm_info.h"
#include "../../../resource/sound/bgm_info/iss_type.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "sound/pm_sndsys.h" // TEST:


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
struct _ISS_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象ゲームデータ
	GAMEDATA* pGameData;

	// 自転車状態かどうか
	BOOL cycle;

	// なみのり状態かどうか
	BOOL surfing;

	// 各ISSシステム
	ISS_CITY_SYS* pIssCitySys;	// 街
	ISS_ROAD_SYS* pIssRoadSys;	// 道路

	// 再生中のBGM番号
	int bgmNo; 
};


//=========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//=========================================================================================

// 主人公の自転車状態を監視する
void CycleCheck( ISS_SYS* p_sys );

// 主人公のなみのり状態を監視する
void SurfingCheck( ISS_SYS* p_sys );

// 戦闘エンカウントの発生を監視する
void BattleCheck( ISS_SYS* p_sys );


//=========================================================================================
/**
 * @brief 公開関数の実装
 */
//=========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief  ISSシステムを作成する
 *
 * @param p_gdata	監視対象ゲームデータ
 * @param zone_id   ゾーンID
 * @param heap_id   使用するヒープID
 * 
 * @return ISSシステム
 */
//----------------------------------------------------------------------------
ISS_SYS* ISS_SYS_Create( GAMEDATA* p_gdata, u16 zone_id, HEAPID heap_id )
{
	ISS_SYS* p_sys;
	PLAYER_WORK* p_player;

	// 監視対象の主人公を取得
	p_player = GAMEDATA_GetMyPlayerWork( p_gdata );

	// メモリ確保
	p_sys = (ISS_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_SYS ) );

	// 初期設定
	p_sys->heapID      = heap_id;
	p_sys->pGameData   = p_gdata;
	p_sys->cycle       = FALSE;
	p_sys->surfing     = FALSE;
	p_sys->pIssCitySys = ISS_CITY_SYS_Create( p_player, zone_id, heap_id );
	p_sys->pIssRoadSys = ISS_ROAD_SYS_Create( p_player, zone_id, heap_id );
	p_sys->bgmNo       = 0;
	ISS_SYS_ZoneChange( p_sys, zone_id );
	
	// 作成したISSシステムを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  ISSシステムを破棄する
 * * @param p_sys 破棄するISSシステム 
 */
//----------------------------------------------------------------------------
void ISS_SYS_Delete( ISS_SYS* p_sys )
{
	// 各ISSシステムを破棄
	ISS_CITY_SYS_Delete( p_sys->pIssCitySys );
	ISS_ROAD_SYS_Delete( p_sys->pIssRoadSys );

	// 本体を破棄
	GFL_HEAP_FreeMemory( p_sys );

	// 9, 10トラックの音量を元に戻す
	FIELD_SOUND_ChangeBGMActionVolume( 127 );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_sys 操作対象のシステム
 */
//----------------------------------------------------------------------------
void ISS_SYS_Update( ISS_SYS* p_sys )
{
	// 自転車チェック
	CycleCheck( p_sys );

	// なみのりチェック
	SurfingCheck( p_sys );

	// 戦闘チェック
	BattleCheck( p_sys );

	// 街ISS
	ISS_CITY_SYS_Update( p_sys->pIssCitySys );

	// 道路ISS
	ISS_ROAD_SYS_Update( p_sys->pIssRoadSys ); 
}
	

//---------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param p_sys       通知対象のISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//---------------------------------------------------------------------------
void ISS_SYS_ZoneChange( ISS_SYS* p_sys, u16 next_zone_id )
{
	PLAYER_WORK*     p_player;
	PLAYER_MOVE_FORM form;
	BGM_INFO_SYS*    p_bgm_info_sys;
	int              iss_type;

	// 自転車orなみのり中なら, ISSに変更はない
	if( p_sys->cycle | p_sys->surfing ) return;

	// 各オブジェクトを取得
	p_player       = GAMEDATA_GetMyPlayerWork( p_sys->pGameData );
	form           = PLAYERWORK_GetMoveForm( p_player );
	p_bgm_info_sys = GAMEDATA_GetBGMInfoSys( p_sys->pGameData );

	// 切り替え先ゾーンのISSタイプを取得
	p_sys->bgmNo = FIELD_SOUND_GetFieldBGMNo( p_sys->pGameData, form, next_zone_id );
	iss_type     = BGM_INFO_GetIssType( p_bgm_info_sys, p_sys->bgmNo );

	// 街ISS
	ISS_CITY_SYS_ZoneChange( p_sys->pIssCitySys, next_zone_id );

	// 道路ISS
	if( iss_type == ISS_TYPE_LOAD )
	{
		ISS_ROAD_SYS_SetActive( p_sys->pIssRoadSys, TRUE );
	}
	else
	{
		ISS_ROAD_SYS_SetActive( p_sys->pIssRoadSys, FALSE );
	}

	// DEBUG:
	OBATA_Printf( "ISS_SYS_ZoneChange()\n" );
	OBATA_Printf( "seq = %d, iss_type = %d\n", p_sys->bgmNo, iss_type );
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
void CycleCheck( ISS_SYS* p_sys )
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
		ISS_CITY_SYS_SetActive( p_sys->pIssCitySys, FALSE );
		ISS_ROAD_SYS_SetActive( p_sys->pIssRoadSys, FALSE );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( p_sys->cycle == TRUE ) && ( form != PLAYER_MOVE_FORM_CYCLE ) )
	{
		p_sys->cycle = FALSE;
		ISS_SYS_ZoneChange( p_sys, PLAYERWORK_getZoneID( p_player ) );
	}
}

//----------------------------------------------------------------------------
/**
 * @brief 主人公のなみのり状態を監視する
 */
//----------------------------------------------------------------------------
void SurfingCheck( ISS_SYS* p_sys )
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
		ISS_CITY_SYS_SetActive( p_sys->pIssCitySys, FALSE );
		ISS_ROAD_SYS_SetActive( p_sys->pIssRoadSys, FALSE );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( p_sys->surfing == TRUE ) && ( form != PLAYER_MOVE_FORM_SWIM ) )
	{
		p_sys->surfing = FALSE;
		ISS_SYS_ZoneChange( p_sys, PLAYERWORK_getZoneID( p_player ) );
	}
}

//----------------------------------------------------------------------------
/**
 * @brief 戦闘エンカウントの発生を監視する
 */
//----------------------------------------------------------------------------
void BattleCheck( ISS_SYS* p_sys )
{
	PLAYER_WORK*     p_player;
	PLAYER_MOVE_FORM form;
	BGM_INFO_SYS*    p_bgm_info_sys;
	int              iss_type;

	if( !ISS_ROAD_SYS_IsActive( p_sys->pIssRoadSys ) ) return;
	
	// 各オブジェクトを取得
	p_player       = GAMEDATA_GetMyPlayerWork( p_sys->pGameData );
	form           = PLAYERWORK_GetMoveForm( p_player );
	p_bgm_info_sys = GAMEDATA_GetBGMInfoSys( p_sys->pGameData );

	// 切り替え先ゾーンのISSタイプを取得
	p_sys->bgmNo = PMSND_GetNextBGMsoundNo();
	iss_type     = BGM_INFO_GetIssType( p_bgm_info_sys, p_sys->bgmNo ); 

	// 道路ISSのBGMでなかったら, 道路ISSを停止する
	if( iss_type != ISS_TYPE_LOAD )
	{
		ISS_ROAD_SYS_SetActive( p_sys->pIssRoadSys, FALSE );
		FIELD_SOUND_ChangeBGMActionVolume( 127 );
	}

}
