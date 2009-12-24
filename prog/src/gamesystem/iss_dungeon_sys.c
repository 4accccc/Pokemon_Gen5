////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_dungeon_sys.h
 * @brief  ダンジョンISSユニット
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_dungeon_sys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"
#include "arc/arc_def.h"
#include "gamesystem/pm_season.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//==========================================================================================
// ■定数・マクロ
//========================================================================================== 
// 無効なゾーンID
#define INVALID_ZONE_ID (0xffff)

// ピッチ(キー)設定対象トラック
#define PITCH_TRACK_MASK (~((1<<(9-1)) | (1<<(10-1))))  // 9, 10トラック

// リバーブ設定
#define REVERB_SAMPLE_RATE (16000)	// サンプリングレート
#define REVERB_VOLUME      (63)		  // ボリューム
#define REVERB_STOP_FRAME  (0)		  // 停止までのフレーム数


//========================================================================================== 
// ■BGMパラメータ
//========================================================================================== 
typedef struct
{
	u16 zoneID;	                // ゾーンID
  u16 padding;
	s16 pitch[PMSEASON_TOTAL];	// ピッチ(キー)
	u16 tempo[PMSEASON_TOTAL];	// テンポ
	u16 reverb[PMSEASON_TOTAL];	// リバーブ

} BGM_PARAM; 

//------------------------------------------------------------------------------------------
/**
 * @brief 指定パラメータを反映させる
 *
 * @param param  設定するパラメータ
 * @param season 季節を指定
 */
//------------------------------------------------------------------------------------------
static void SetBGMStatus( const BGM_PARAM* param, u8 season )
{
	if( !param ) return;
  if( PMSEASON_TOTAL <= season ) return;

  // テンポ
	PMSND_SetStatusBGM( param->tempo[season], PMSND_NOEFFECT, 0 ); 
  // ピッチ
	NNS_SndPlayerSetTrackPitch( 
      PMSND_GetBGMhandlePointer(), PITCH_TRACK_MASK, param->pitch[season] ); 
  // リバーブ
  if( param->reverb[season] == 0 )
  {
    PMSND_DisableCaptureReverb();
  }
  else
  {
    PMSND_EnableCaptureReverb( 
        param->reverb[season], REVERB_SAMPLE_RATE, REVERB_VOLUME, REVERB_STOP_FRAME );
  }

	// DEBUG:
	OBATA_Printf( "ISS-D: Set BGM status\n" );
	OBATA_Printf( "- pitch  = %d\n", param->pitch[season] );
	OBATA_Printf( "- tempo  = %d\n", param->tempo[season] );
	OBATA_Printf( "- reverb = %d\n", param->reverb[season] );
}


//========================================================================================== 
// ■ダンジョンISSデータ
//========================================================================================== 
typedef struct
{
	u8         dataNum;	// 保持情報数
	BGM_PARAM* param;	  // パラメータ配列

} BGM_PARAMSET;


//-----------------------------------------------------------------------------------------
/**
 * @brief データを読み込む
 *
 * @param heap_id 使用するヒープID
 *
 * @return 読み込んだデータ
 */
//-----------------------------------------------------------------------------------------
static BGM_PARAMSET* LoadParamset( HEAPID heap_id )
{
	int dat_id;
	BGM_PARAMSET* paramset;

	// 本体を作成
	paramset = (BGM_PARAMSET*)GFL_HEAP_AllocMemory( heap_id, sizeof(BGM_PARAMSET) );

	// データ数を取得
	paramset->dataNum = GFL_ARC_GetDataFileCnt( ARCID_ISS_DUNGEON );

	// バッファ確保
	paramset->param = GFL_HEAP_AllocMemory( heap_id, sizeof(BGM_PARAM) * paramset->dataNum );

	// 各データを取得
	for( dat_id=0; dat_id<paramset->dataNum; dat_id++ )
	{
    GFL_ARC_LoadDataOfs( &paramset->param[dat_id], 
        ARCID_ISS_DUNGEON, dat_id, 0, sizeof(BGM_PARAM) );
	} 

	// DEBUG:
	OBATA_Printf( "ISS-D: Load BGM parameters\n" );
	OBATA_Printf( "- dataNum = %d\n", paramset->dataNum );
	for( dat_id=0; dat_id<paramset->dataNum; dat_id++ )
	{ 
		OBATA_Printf( "- data[%d].zoneID = %d\n", dat_id, paramset->param[dat_id].zoneID ); 
		OBATA_Printf( "- data[%d].pitch  = %d, %d, %d, %d\n", dat_id, 
        paramset->param[dat_id].pitch[PMSEASON_SPRING], 
        paramset->param[dat_id].pitch[PMSEASON_SUMMER], 
        paramset->param[dat_id].pitch[PMSEASON_AUTUMN], 
        paramset->param[dat_id].pitch[PMSEASON_WINTER] );
		OBATA_Printf( "- data[%d].tempo  = %d, %d, %d, %d\n", dat_id,
        paramset->param[dat_id].tempo[PMSEASON_SPRING], 
        paramset->param[dat_id].tempo[PMSEASON_SUMMER], 
        paramset->param[dat_id].tempo[PMSEASON_AUTUMN], 
        paramset->param[dat_id].tempo[PMSEASON_WINTER] );
		OBATA_Printf( "- data[%d].reverb = %d, %d, %d, %d\n", dat_id,
        paramset->param[dat_id].reverb[PMSEASON_SPRING], 
        paramset->param[dat_id].reverb[PMSEASON_SUMMER], 
        paramset->param[dat_id].reverb[PMSEASON_AUTUMN], 
        paramset->param[dat_id].reverb[PMSEASON_WINTER] );
	}

	// 読み込んだデータを返す
	return paramset;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief データを破棄する
 *
 * @param paramset 破棄するデータ
 */
//-----------------------------------------------------------------------------------------
static void UnloadIssData( BGM_PARAMSET* paramset )
{ 
	// パラメータ配列を破棄
	GFL_HEAP_FreeMemory( paramset->param );

	// 本体を破棄
	GFL_HEAP_FreeMemory( paramset );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定ゾーンIDのBGMパラメータを取得する
 *
 * @param paramset 検索元データ
 * @param zone_id  検索対象ゾーンID
 *
 * @return 指定ゾーンIDのパラメータ(存在しない場合, NULL)
 */
//-----------------------------------------------------------------------------------------
static const BGM_PARAM* GetBGMParam( const BGM_PARAMSET* paramset, u16 zone_id )
{
	int i;

	// 指定ゾーンIDを検索
	for( i=0; i<paramset->dataNum; i++ )
	{
		// 発見
		if( paramset->param[i].zoneID == zone_id )
		{
			return &paramset->param[i];
		}
	}

	// DEBUG: BGMパラメータ発見できず
  OBATA_Printf( "ISS-D: BGM param is not found\n" );
	return NULL;
}


//========================================================================================== 
// ■ダンジョンISSシステム管理ワーク
//========================================================================================== 
struct _ISS_DUNGEON_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象
	PLAYER_WORK* player; // プレイヤー
  GAMEDATA* gdata;  // ゲームデータ

	// 起動状態
	BOOL isActive; 

	// ゾーンID
	u16 currentZoneID;
	u16 nextZoneID;

	// データ
	BGM_PARAMSET* paramset;

	// 現在の設定データ
	const BGM_PARAM* pActiveParam;

  // デフォルト・パラメータ
  BGM_PARAM defaultParam;
};


//========================================================================================== 
// ■公開関数
//========================================================================================== 

//-----------------------------------------------------------------------------------------
/**
 * @brief  ダンジョンISSシステムを作成する
 *
 * @param  gdata   ゲームデータ
 * @param  player  監視対象のプレイヤー
 * @param  heap_id 使用するヒープID
 * 
 * @return ダンジョンISSシステム
 */
//-----------------------------------------------------------------------------------------
ISS_DUNGEON_SYS* ISS_DUNGEON_SYS_Create( GAMEDATA* gdata, 
                                         PLAYER_WORK* player, HEAPID heap_id )
{
	ISS_DUNGEON_SYS* sys;

	// メモリ確保
	sys = (ISS_DUNGEON_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_DUNGEON_SYS) );

	// 初期化
	sys->heapID        = heap_id;
  sys->gdata         = gdata;
	sys->player        = player;
	sys->isActive      = FALSE;
	sys->currentZoneID = INVALID_ZONE_ID;
	sys->nextZoneID    = INVALID_ZONE_ID;
	sys->paramset      = LoadParamset( heap_id );
	sys->pActiveParam  = NULL;
  // デフォルトパラメータ設定
  {
    int i;
    sys->defaultParam.zoneID = INVALID_ZONE_ID;
    for( i=0; i<PMSEASON_TOTAL; i++ )
    {
      sys->defaultParam.pitch[i]  = 0;
      sys->defaultParam.tempo[i]  = 256;
      sys->defaultParam.reverb[i] = 0;
    }
  }

	// DEBUG:
	OBATA_Printf( "ISS-D: Create\n" );

	// 作成したダンジョンISSシステムを返す
	return sys;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief  ダンジョンISSシステムを破棄する
 *
 * @param sys 破棄するダンジョンISSシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Delete( ISS_DUNGEON_SYS* sys )
{
	// システム停止
	ISS_DUNGEON_SYS_Off( sys );
	
	// データを破棄
	UnloadIssData( sys->paramset );

	// 本体を破棄
	GFL_HEAP_FreeMemory( sys );

	// DEBUG:
	OBATA_Printf( "ISS-D: Delete\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param sys 動作対象のダンジョンISSシステム
 */
//----------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Update( ISS_DUNGEON_SYS* sys )
{
	// 起動していなければ, 何もしない
	if( sys->isActive != TRUE ) return; 

	// ゾーン切り替えが通知された場合
	if( sys->currentZoneID != sys->nextZoneID )
	{
		// 更新
		sys->currentZoneID = sys->nextZoneID;

		// 新ゾーンIDのBGMパラメータを検索
		sys->pActiveParam = GetBGMParam( sys->paramset, sys->nextZoneID );

    // BGMパラメータが設定されていない場合 ==> システム停止
    if( sys->pActiveParam == NULL )
    {
      ISS_DUNGEON_SYS_Off( sys );
      return;
    }

		// BGMの設定を反映させる
    {
      u8 season = GAMEDATA_GetSeasonID( sys->gdata );
      SetBGMStatus( sys->pActiveParam, season ); 
    }
	} 
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param sys        通知対象のダンジョンISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_ZoneChange( ISS_DUNGEON_SYS* sys, u16 next_zone_id )
{ 
	sys->nextZoneID = next_zone_id; 

  // DEBUG:
  OBATA_Printf( "ISS-D: ZoneChange\n" );
  OBATA_Printf( "- next zone id = %d\n", next_zone_id );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_On( ISS_DUNGEON_SYS* sys )
{
  // 起動済み
  if( sys->isActive ){ return; }

  // 起動
	sys->isActive = TRUE;

#if 0
  // パラメータ設定
  {
    u8 season = GAMEDATA_GetSeasonID( sys->gdata );
    SetBGMStatus( sys->pActiveParam, season ); 
  }
#endif

  // DEBUG:
  OBATA_Printf( "ISS-D: On\n" );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param sys 停止させるシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Off( ISS_DUNGEON_SYS* sys )
{
  // 停止済み
  if( !sys->isActive ){ return; }

	// 停止
	sys->isActive = FALSE;

  // デフォルト・パラメータに戻す
  SetBGMStatus( &sys->defaultParam, 0 );

  // DEBUG:
  OBATA_Printf( "ISS-D: Off\n" );
}

//----------------------------------------------------------------------------
/**
 * @breif 動作状態を取得する
 *
 * @param sys 状態を調べるISSシステム
 * 
 * @return 動作中かどうか
 */
//----------------------------------------------------------------------------
BOOL ISS_DUNGEON_SYS_IsOn( const ISS_DUNGEON_SYS* sys )
{
	return sys->isActive; 
}
