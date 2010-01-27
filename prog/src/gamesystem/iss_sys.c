////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_sys.h
 * @brief  ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////
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


//==================================================================================
// ■定数
//==================================================================================
#define ISS_ON                       // ISS動作スイッチ
#define INVALID_BGM_NO (0xffffffff)  // 監視BGM番号の無効値


//==================================================================================
// ■ISSシステムワーク
//==================================================================================
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


//==================================================================================
// ■非公開関数のプロトタイプ宣言
//==================================================================================
static void BGMChangeCheck( ISS_SYS* sys );


//==================================================================================
// ■外部公開関数
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief ISSシステムを作成する
 *
 * @param gameData
 * @param heapID   使用するヒープID
 * 
 * @return ISSシステム
 */
//----------------------------------------------------------------------------------
ISS_SYS* ISS_SYS_Create( GAMEDATA* gameData, HEAPID heapID )
{
	ISS_SYS* sys;
	PLAYER_WORK* player;

  GF_ASSERT( gameData );

	player = GAMEDATA_GetMyPlayerWork( gameData );

	// システムワークを確保
	sys = (ISS_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof(ISS_SYS) );

	// 初期設定
	sys->heapID   = heapID;
	sys->gameData = gameData;
	sys->issC     = ISS_CITY_SYS_Create( player, heapID );
	sys->issR     = ISS_ROAD_SYS_Create( player, heapID );
	sys->issD     = ISS_DUNGEON_SYS_Create( gameData, player, heapID );
  sys->issZ     = ISS_ZONE_SYS_Create( heapID );
  sys->issS     = ISS_SWITCH_SYS_Create( heapID );
  sys->issB     = ISS_3DS_SYS_Create( heapID );
	sys->soundIdx = INVALID_BGM_NO;
	sys->frame    = 0;

	return sys;
}

//----------------------------------------------------------------------------------
/**
 * @brief ISSシステムを破棄する
 *
 * @param sys
 */
//----------------------------------------------------------------------------------
void ISS_SYS_Delete( ISS_SYS* sys )
{
  GF_ASSERT( sys );

	// 各ISSシステムを破棄
	ISS_CITY_SYS_Delete( sys->issC );
	ISS_ROAD_SYS_Delete( sys->issR );
	ISS_DUNGEON_SYS_Delete( sys->issD );
  ISS_ZONE_SYS_Delete( sys->issZ );
  ISS_SWITCH_SYS_Delete( sys->issS );
  ISS_3DS_SYS_Delete( sys->issB );

	// 本体を破棄
	GFL_HEAP_FreeMemory( sys );

#ifdef ISS_ON
	// 9, 10トラックの音量を元に戻す
  PMSND_ChangeBGMVolume( (1<<8)|(1<<9), 127 );
#endif
}

//----------------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param sys
 */
//----------------------------------------------------------------------------------
void ISS_SYS_Update( ISS_SYS* sys )
{ 
#ifdef ISS_ON
	// BGM変更チェック
	BGMChangeCheck( sys );

  // 各ISSの更新
	if( sys->frame++ % 2 == 0 )
  {
    ISS_ROAD_SYS_Update( sys->issR );  // 道路ISS(30フレームで動作)
    ISS_3DS_SYS_Main( sys->issB );     // 橋ISS(30フレームで動作)
  }
	ISS_CITY_SYS_Update( sys->issC );    // 街ISS 
	ISS_DUNGEON_SYS_Update( sys->issD ); // ダンジョンISS 
  ISS_ZONE_SYS_Update( sys->issZ );    // ゾーンISS 
  ISS_SWITCH_SYS_Update( sys->issS );  // スイッチISS
#endif
}
	

//----------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param sys
 * @param nextZoneID 新しいゾーンID
 */
//----------------------------------------------------------------------------------
void ISS_SYS_ZoneChange( ISS_SYS* sys, u16 nextZoneID )
{
#ifdef ISS_ON
	PLAYER_WORK* player;
	PLAYER_MOVE_FORM form;

	// 自機の状態を取得
	player = GAMEDATA_GetMyPlayerWork( sys->gameData );
	form   = PLAYERWORK_GetMoveForm( player ); 
	// 自転車に乗っている ==> ISS変更無し
	if( form == PLAYER_MOVE_FORM_CYCLE ) return;
  // なみのり中 ==> ISS変更無し
	if( form == PLAYER_MOVE_FORM_SWIM ) return;

  // 各ISSシステムにゾーンチェンジを通知
	ISS_CITY_SYS_ZoneChange( sys->issC, nextZoneID );    // 街ISS 
	ISS_DUNGEON_SYS_ZoneChange( sys->issD, nextZoneID ); // ダンジョンISS 
  ISS_SWITCH_SYS_ZoneChange( sys->issS, nextZoneID );  // スイッチISS
  ISS_ZONE_SYS_ZoneChange( sys->issZ, nextZoneID );    // ゾーンISS
  ISS_3DS_SYS_ZoneChange( sys->issB );                 // 橋ISS
#endif
}

//----------------------------------------------------------------------------------
/**
 * @brief スイッチISSシステムを取得する
 *
 * @param sys 取得対象システム
 *
 * @return スイッチISSシステム
 */
//----------------------------------------------------------------------------------
ISS_SWITCH_SYS* ISS_SYS_GetIssSwitchSystem( const ISS_SYS* sys )
{
  return sys->issS;
}

//------------------------------------------------------------------------------------
/**
 * @brief 3D ISSシステムを取得する
 *
 * @param sys 取得対象システム
 *
 * @return 3D ISSシステム
 */
//------------------------------------------------------------------------------------
ISS_3DS_SYS* ISS_SYS_GetIss3DSSystem( const ISS_SYS* sys )
{
  return sys->issB;
}


//==================================================================================
// ■非公開関数
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief BGMの変化を監視する
 *
 * @param 動かすシステム
 */
//----------------------------------------------------------------------------------
static void BGMChangeCheck( ISS_SYS* sys )
{
#ifdef ISS_ON
	u32 soundIdx, issType;
	BGM_INFO_SYS* bgmInfoSys = GAMEDATA_GetBGMInfoSys( sys->gameData );
	PLAYER_WORK*  player     = GAMEDATA_GetMyPlayerWork( sys->gameData );
  u16           zoneID     = PLAYERWORK_getZoneID( player );

	// 再生中のBGMのISSタイプを取得
	soundIdx = PMSND_GetBGMsoundNo();
	issType  = BGM_INFO_GetIssType( bgmInfoSys, soundIdx ); 

  // BGMに変化がなければ何もしない
  if( sys->soundIdx == soundIdx ){ return; }

  // 各ISSシステムの起動状態を変更
  switch( issType )
  {
  case ISS_TYPE_LOAD:  // 道路
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_ROAD_SYS_On( sys->issR );
      ISS_3DS_SYS_Off( sys->issB );
    break;
  case ISS_TYPE_CITY:  // 街
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_CITY_SYS_On( sys->issC );
      ISS_3DS_SYS_Off( sys->issB );
    break;
  case ISS_TYPE_DUNGEON:  // ダンジョン
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_DUNGEON_SYS_On( sys->issD );
      ISS_3DS_SYS_Off( sys->issB );
    break;
  case ISS_TYPE_ZONE:  // ゾーン
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_ZONE_SYS_On( sys->issZ, zoneID );
      ISS_3DS_SYS_Off( sys->issB );
      break;
  case ISS_TYPE_SWITCH:  // スイッチ
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_On( sys->issS );
      ISS_3DS_SYS_Off( sys->issB );
    break;
  case ISS_TYPE_BRIDGE:  // 橋
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_3DS_SYS_On( sys->issB );
      break;
  default:  // その他
      ISS_ROAD_SYS_Off( sys->issR );
      ISS_CITY_SYS_Off( sys->issC );
      ISS_DUNGEON_SYS_Off( sys->issD );
      ISS_ZONE_SYS_Off( sys->issZ );
      ISS_SWITCH_SYS_Off( sys->issS );
      ISS_3DS_SYS_Off( sys->issB );
    break;
  } 

  // BGM番号を記憶
  sys->soundIdx = soundIdx;
#endif
} 
