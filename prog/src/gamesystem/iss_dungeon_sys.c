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
#include "arc/iss.naix"
#include "gamesystem/pm_season.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"
#include "../../../resource/iss/dungeon/entry_table.cdat"


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
  if( param->reverb == 0 )
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
	int i;
	BGM_PARAMSET* paramset;

	// 本体を作成
	paramset = (BGM_PARAMSET*)GFL_HEAP_AllocMemory( heap_id, sizeof(BGM_PARAMSET) );

	// データ数を取得
	paramset->dataNum = NELEMS(entry_table);

	// バッファ確保
	paramset->param = GFL_HEAP_AllocMemory( heap_id, sizeof(BGM_PARAM) * paramset->dataNum );

	// 各データを取得
	for( i=0; i<paramset->dataNum; i++ )
	{
    GFL_ARC_LoadDataOfs( &paramset->param[i], 
        ARCID_ISS_DUNGEON, entry_table[i].datID, 0, sizeof(BGM_PARAM) );
	} 

	// DEBUG:
	OBATA_Printf( "ISS-D: Load BGM parameters\n" );
	OBATA_Printf( "- dataNum = %d\n", paramset->dataNum );
	for( i=0; i<paramset->dataNum; i++ )
	{ 
		OBATA_Printf( "- data[%d].zoneID = %d\n", i, paramset->param[i].zoneID ); 
		OBATA_Printf( "- data[%d].pitch  = %d, %d, %d, %d\n", i, 
        paramset->param[i].pitch[PMSEASON_SPRING], 
        paramset->param[i].pitch[PMSEASON_SUMMER], 
        paramset->param[i].pitch[PMSEASON_AUTUMN], 
        paramset->param[i].pitch[PMSEASON_WINTER] );
		OBATA_Printf( "- data[%d].tempo  = %d, %d, %d, %d\n", i,
        paramset->param[i].tempo[PMSEASON_SPRING], 
        paramset->param[i].tempo[PMSEASON_SUMMER], 
        paramset->param[i].tempo[PMSEASON_AUTUMN], 
        paramset->param[i].tempo[PMSEASON_WINTER] );
		OBATA_Printf( "- data[%d].reverb = %d, %d, %d, %d\n", i,
        paramset->param[i].reverb[PMSEASON_SPRING], 
        paramset->param[i].reverb[PMSEASON_SUMMER], 
        paramset->param[i].reverb[PMSEASON_AUTUMN], 
        paramset->param[i].reverb[PMSEASON_WINTER] );
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
 * @param  gdata    ゲームデータ
 * @param  p_player 監視対象のプレイヤー
 * @param  heap_id  使用するヒープID
 * 
 * @return ダンジョンISSシステム
 */
//-----------------------------------------------------------------------------------------
ISS_DUNGEON_SYS* ISS_DUNGEON_SYS_Create( GAMEDATA* gdata, 
                                         PLAYER_WORK* p_player, HEAPID heap_id )
{
	ISS_DUNGEON_SYS* p_sys;

	// メモリ確保
	p_sys = (ISS_DUNGEON_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_DUNGEON_SYS) );

	// 初期化
	p_sys->heapID        = heap_id;
  p_sys->gdata         = gdata;
	p_sys->pPlayer       = p_player;
	p_sys->isActive      = FALSE;
	p_sys->currentZoneID = INVALID_ZONE_ID;
	p_sys->nextZoneID    = INVALID_ZONE_ID;
	p_sys->paramset      = LoadParamset( heap_id );
	p_sys->pActiveParam  = NULL;
  // デフォルトパラメータ設定
  {
    int i;
    p_sys->defaultParam.zoneID = INVALID_ZONE_ID;
    for( i=0; i<PMSEASON_TOTAL; i++ )
    {
      p_sys->defaultParam.pitch[i]  = 0;
      p_sys->defaultParam.tempo[i]  = 256;
      p_sys->defaultParam.reverb[i] = 0;
    }
  }

	// DEBUG:
	OBATA_Printf( "ISS-D: Create\n" );

	// 作成したダンジョンISSシステムを返す
	return p_sys;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief  ダンジョンISSシステムを破棄する
 *
 * @param p_sys 破棄するダンジョンISSシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Delete( ISS_DUNGEON_SYS* p_sys )
{
	// システム停止
	ISS_DUNGEON_SYS_Off( p_sys );
	
	// データを破棄
	UnloadIssData( p_sys->paramset );

	// 本体を破棄
	GFL_HEAP_FreeMemory( p_sys );

	// DEBUG:
	OBATA_Printf( "ISS-D: Delete\n" );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_sys 動作対象のダンジョンISSシステム
 */
//----------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Update( ISS_DUNGEON_SYS* p_sys )
{
	// 起動していなければ, 何もしない
	if( p_sys->isActive != TRUE ) return; 

	// ゾーン切り替えが通知された場合
	if( p_sys->currentZoneID != p_sys->nextZoneID )
	{
		// 更新
		p_sys->currentZoneID = p_sys->nextZoneID;

		// 新ゾーンIDのBGMパラメータを検索
		p_sys->pActiveParam = GetBGMParam( p_sys->paramset, p_sys->nextZoneID );

    // BGMパラメータが設定されていない場合 ==> システム停止
    if( p_sys->pActiveParam == NULL )
    {
      ISS_DUNGEON_SYS_Off( p_sys );
      return;
    }

		// BGMの設定を反映させる
    {
      u8 season = GAMEDATA_GetSeasonID( p_sys->gdata );
      SetBGMStatus( p_sys->pActiveParam, season ); 
    }
	} 
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param p_sys        通知対象のダンジョンISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_ZoneChange( ISS_DUNGEON_SYS* p_sys, u16 next_zone_id )
{ 
	p_sys->nextZoneID = next_zone_id; 

  // DEBUG:
  OBATA_Printf( "ISS-D: ZoneChange\n" );
  OBATA_Printf( "- next zone id = %d\n", next_zone_id );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param p_sys 起動するシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_On( ISS_DUNGEON_SYS* p_sys )
{
  // 起動
	p_sys->isActive = TRUE;

  // パラメータ設定
  {
    u8 season = GAMEDATA_GetSeasonID( p_sys->gdata );
    SetBGMStatus( p_sys->pActiveParam, season ); 
  }

  // DEBUG:
  OBATA_Printf( "ISS-D: On\n" );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param p_sys 停止させるシステム
 */
//-----------------------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Off( ISS_DUNGEON_SYS* p_sys )
{
	// 停止
	p_sys->isActive = FALSE;

  // デフォルト・パラメータに戻す
  SetBGMStatus( &p_sys->defaultParam, 0 );

  // DEBUG:
  OBATA_Printf( "ISS-D: Off\n" );
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
BOOL ISS_DUNGEON_SYS_IsOn( const ISS_DUNGEON_SYS* p_sys )
{
	return p_sys->isActive; 
}
