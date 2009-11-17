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
#include "iss_dungeon_sys.h"
#include "iss_zone_sys.h"
#include "iss_switch_sys.h"
#include "../field/field_sound.h"
#include "sound/bgm_info.h"
#include "../../../resource/sound/bgm_info/iss_type.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "sound/pm_sndsys.h" 


//=========================================================================================
/**
 * @brief 定数・マクロ
 */
//=========================================================================================

// 再生中BGM番号の無効値
#define INVALID_BGM_NO (0xffff)


//=========================================================================================
/**
 * @brief ISSシステムワーク
 */
//=========================================================================================
struct _ISS_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象
	GAMEDATA* gdata;  // ゲームデータ

	// 自転車状態かどうか
	BOOL cycle;

	// なみのり状態かどうか
	BOOL surfing;

	// 各ISSシステム
	ISS_CITY_SYS*    issC;	// 街
	ISS_ROAD_SYS*    issR;	// 道路
	ISS_DUNGEON_SYS* issD;	// ダンジョン 
  ISS_ZONE_SYS*    issZ;  // ゾーン
  ISS_SWITCH_SYS*  issS;  // スイッチ

	// 再生中のBGM番号
	u16 bgmNo; 

	// フレームカウンタ
	u32 frame;
};


//=========================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//=========================================================================================

// 主人公の自転車状態を監視する
void CycleCheck( ISS_SYS* sys );

// 主人公のなみのり状態を監視する
void SurfingCheck( ISS_SYS* sys );

// BGMの変化を監視する
void BGMChangeCheck( ISS_SYS* sys );


//=========================================================================================
/**
 * @brief 公開関数の実装
 */
//=========================================================================================

//----------------------------------------------------------------------------
/**
 * @brief  ISSシステムを作成する
 *
 * @param gdata	  監視対象ゲームデータ
 * @param heap_id 使用するヒープID
 * 
 * @return ISSシステム
 */
//----------------------------------------------------------------------------
ISS_SYS* ISS_SYS_Create( GAMEDATA* gdata, HEAPID heap_id )
{
	ISS_SYS* sys;
	PLAYER_WORK* player;

	// 監視対象の主人公を取得
	player = GAMEDATA_GetMyPlayerWork( gdata );

	// メモリ確保
	sys = (ISS_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_SYS ) );

	// 初期設定
	sys->heapID  = heap_id;
	sys->gdata   = gdata;
	sys->cycle   = FALSE;
	sys->surfing = FALSE;
	sys->issC    = ISS_CITY_SYS_Create( player, heap_id );
	sys->issR    = ISS_ROAD_SYS_Create( player, heap_id );
	sys->issD    = ISS_DUNGEON_SYS_Create( gdata, player, heap_id );
  sys->issZ    = ISS_ZONE_SYS_Create( heap_id );
  sys->issS    = ISS_SWITCH_SYS_Create( heap_id );
	sys->bgmNo   = INVALID_BGM_NO;
	sys->frame   = 0;


	// 作成したISSシステムを返す
	return sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  ISSシステムを破棄する
 * * @param sys 破棄するISSシステム 
 */
//----------------------------------------------------------------------------
void ISS_SYS_Delete( ISS_SYS* sys )
{
	// 各ISSシステムを破棄
	ISS_CITY_SYS_Delete( sys->issC );
	ISS_ROAD_SYS_Delete( sys->issR );
	ISS_DUNGEON_SYS_Delete( sys->issD );
  ISS_ZONE_SYS_Delete( sys->issZ );
  ISS_SWITCH_SYS_Delete( sys->issS );

	// 本体を破棄
	GFL_HEAP_FreeMemory( sys );

	// 9, 10トラックの音量を元に戻す
  PMSND_ChangeBGMVolume( (1<<8)|(1<<9), 127 );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param sys 操作対象のシステム
 */
//----------------------------------------------------------------------------
void ISS_SYS_Update( ISS_SYS* sys )
{ 
	sys->frame++;

	// 自転車チェック
	CycleCheck( sys );

	// なみのりチェック
	SurfingCheck( sys );

	// BGM変更チェック
	BGMChangeCheck( sys );

	// 街ISS
	ISS_CITY_SYS_Update( sys->issC );

	// 道路ISS(主人公が30fpsで動作するため, 道路ISSもそれに合わせる)
	if( sys->frame % 2 == 0 ) ISS_ROAD_SYS_Update( sys->issR ); 
	
	// ダンジョンISS
	ISS_DUNGEON_SYS_Update( sys->issD );

  // ゾーンISS
  ISS_ZONE_SYS_Update( sys->issZ );

  // スイッチISS
  ISS_SWITCH_SYS_Update( sys->issS );
}
	

//---------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param sys       通知対象のISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//---------------------------------------------------------------------------
void ISS_SYS_ZoneChange( ISS_SYS* sys, u16 next_zone_id )
{
	// 自転車 or なみのり中なら, ISSに変更はない
	if( sys->cycle | sys->surfing ) return;

	// 街ISS
	ISS_CITY_SYS_ZoneChange( sys->issC, next_zone_id );

	// ダンジョンISS
	ISS_DUNGEON_SYS_ZoneChange( sys->issD, next_zone_id );

  // ゾーンISS
  ISS_ZONE_SYS_ZoneChange( sys->issZ, next_zone_id );

	// DEBUG:
	//OBATA_Printf( "ISS_SYS_ZoneChange()\n" );
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
void CycleCheck( ISS_SYS* sys )
{
	PLAYER_WORK*     player;
	PLAYER_MOVE_FORM form;

	// 自機の状態を取得
	player = GAMEDATA_GetMyPlayerWork( sys->gdata );
	form   = PLAYERWORK_GetMoveForm( player );

	// 自転車に乗るのを検出したら, ISSを停止
	if( ( sys->cycle == FALSE ) && ( form == PLAYER_MOVE_FORM_CYCLE ) )
	{
		sys->cycle = TRUE;
		ISS_CITY_SYS_Off( sys->issC );
		ISS_ROAD_SYS_Off( sys->issR );
		ISS_DUNGEON_SYS_Off( sys->issD );
		ISS_ZONE_SYS_Off( sys->issZ );
		ISS_SWITCH_SYS_Off( sys->issS );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( sys->cycle == TRUE ) && ( form != PLAYER_MOVE_FORM_CYCLE ) )
	{
		sys->cycle = FALSE;
		ISS_SYS_ZoneChange( sys, PLAYERWORK_getZoneID( player ) );
	}
}

//----------------------------------------------------------------------------
/**
 * @brief 主人公のなみのり状態を監視する
 */
//----------------------------------------------------------------------------
void SurfingCheck( ISS_SYS* sys )
{
	PLAYER_WORK*     player;
	PLAYER_MOVE_FORM form;

	// 自機の状態を取得
	player = GAMEDATA_GetMyPlayerWork( sys->gdata );
	form     = PLAYERWORK_GetMoveForm( player );

	// なみのり開始を検出したら, ISSを停止
	if( ( sys->surfing == FALSE ) && ( form == PLAYER_MOVE_FORM_SWIM ) )
	{
		sys->surfing = TRUE;
		ISS_CITY_SYS_Off( sys->issC );
		ISS_ROAD_SYS_Off( sys->issR );
		ISS_DUNGEON_SYS_Off( sys->issD );
		ISS_ZONE_SYS_Off( sys->issZ );
		ISS_SWITCH_SYS_Off( sys->issS );
	}

	// 自転車から降りるのを検出したら, ゾーン切り替え時と同じ処理
	if( ( sys->surfing == TRUE ) && ( form != PLAYER_MOVE_FORM_SWIM ) )
	{
		sys->surfing = FALSE;
		ISS_SYS_ZoneChange( sys, PLAYERWORK_getZoneID( player ) );
	}
} 

//----------------------------------------------------------------------------
/**
 * @brief BGMの変化を監視する
 *
 * @param 動かすシステム
 */
//----------------------------------------------------------------------------
void BGMChangeCheck( ISS_SYS* sys )
{
	int bgm_no, iss_type;
	BGM_INFO_SYS* bgm_info_sys = GAMEDATA_GetBGMInfoSys( sys->gdata );
	PLAYER_WORK* player = GAMEDATA_GetMyPlayerWork( sys->gdata );
  u16 zone_id = PLAYERWORK_getZoneID( player );

	// 再生中のBGMのISSタイプを取得
	bgm_no   = PMSND_GetBGMsoundNo();
	iss_type = BGM_INFO_GetIssType( bgm_info_sys, bgm_no ); 

  // BGMに変化がなければ何もしない
  if( sys->bgmNo == bgm_no ) return;

  // 各ISSシステムの起動状態を変更
  switch( iss_type )
  {
  case ISS_TYPE_LOAD:  // 道路
      ISS_ROAD_SYS_On( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
    break;
  case ISS_TYPE_CITY:  // 街
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_On( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
    break;
  case ISS_TYPE_DUNGEON:  // ダンジョン
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_On( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
    break;
  case ISS_TYPE_ZONE:  // ゾーン
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_On( sys->issZ, zone_id );
      ISS_SWITCH_SYS_Off( sys->issS );
      break;
  case ISS_TYPE_SWITCH:  // スイッチ
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_On( sys->issS );
    break;
  default:  // その他
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
    break;
    break;
  } 

  // BGM番号を記憶
  sys->bgmNo = bgm_no;
}

//----------------------------------------------------------------------------
/**
 * @brief スイッチISSシステムを取得する
 *
 * @param sys 取得対象システム
 *
 * @return スイッチISSシステム
 */
//----------------------------------------------------------------------------
ISS_SWITCH_SYS* ISS_SYS_GetISSSwitchSystem( const ISS_SYS* sys )
{
  return sys->issS;
}
