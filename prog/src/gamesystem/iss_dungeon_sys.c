////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_dungeon_sys.h
 * @brief  ダンジョンISSユニット
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_dungeon_sys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"
#include "arc/arc_def.h"
#include "arc/iss.naix"
#include "arc/fieldmap/zone_id.h"	// TEMP:


//=====================================================================================================
/**
 * @brief 定数・マクロ
 */
//===================================================================================================== 

// 無効なゾーンID
#define INVALID_ZONE_ID (0xffff)

// ピッチ(キー)設定対象トラック
#define PITCH_TRACK_MASK (~((1<<(9-1)) | (1<<(10-1))))

// リバーブ設定
#define REVERB_SAMPLE_RATE (16000)	// サンプリングレート
#define REVERB_VOLUME      (63)		// ボリューム
#define REVERB_STOP_FRAME  (0)		// 停止までのフレーム数


//=====================================================================================================
/**
 * @brief BGMパラメータ
 */ 
//=====================================================================================================
typedef struct
{
	u16 zoneID;	// ゾーンID
	s16 pitch;	// ピッチ(キー)
	u16 tempo;	// テンポ
	u16 reverb;	// リバーブ
}
BGM_PARAM;

//------------------------------------------------------------------
/**
 * @brief BGMパラメータに関する, 非公開関数のプロトタイプ宣言
 */
//------------------------------------------------------------------

// 指定パラメータを反映させる
static void SetBGMStatus( const BGM_PARAM* p_param );


//=====================================================================================================
/**
 * @brief BGMパラメータに関する, 非公開関数の実装
 */ 
//=====================================================================================================

//------------------------------------------------------------------
/**
 * @brief 指定パラメータを反映させる
 *
 * @param 設定するパラメータ
 */
//------------------------------------------------------------------
static void SetBGMStatus( const BGM_PARAM* p_param )
{
	if( !p_param ) return;

	// 反映
	PMSND_SetStatusBGM( p_param->tempo, PMSND_NOEFFECT, 0 );
	NNS_SndPlayerSetTrackPitch( PMSND_GetBGMhandlePointer(), PITCH_TRACK_MASK, p_param->pitch );
	PMSND_ChangeCaptureReverb( p_param->reverb, REVERB_SAMPLE_RATE, REVERB_VOLUME, REVERB_STOP_FRAME );

	// DEBUG:
	OBATA_Printf( "Dungeon ISS SetBGMStatus\n" ); 
	OBATA_Printf( "pitch  = %d\n", p_param->pitch );
	OBATA_Printf( "tempo  = %d\n", p_param->tempo );
	OBATA_Printf( "reverb = %d\n", p_param->reverb );
}


//=====================================================================================================
/**
 * @brief ダンジョンISSデータ
 */
//=====================================================================================================
typedef struct
{
	u8         dataNum;	// 保持情報数
	BGM_PARAM* param;	// パラメータ配列
}
ISS_DATA;

//------------------------------------------------------------------
/**
 * @brief ダンジョンISSデータに関する, 非公開関数のプロトタイプ宣言
 */
//------------------------------------------------------------------

// リトルエンディアンのu16値取得関数
static u16 GetU16( u8* data, int pos );

// データを読み込む
static ISS_DATA* LoadIssData( HEAPID heap_id );

// データを破棄する
static void UnloadIssData( ISS_DATA* p_data );

// 指定ゾーンIDのBGMパラメータを取得する
static const BGM_PARAM* GetBGMParam( const ISS_DATA* p_data, u16 zone_id );


//=====================================================================================================
/** 
 * @brief ダンジョンISSデータに関する, 非公開関数の実装
 */
//=====================================================================================================

//---------------------------------------------------------------------------- 
/**
 * @brief data + pos の位置から始まる2バイトを, 
 *        リトルエンディアンで並んだu16のデータとして解釈し値を取得する
 *
 * @param data データ開始位置
 * @param pos  開始位置オフセット
 *
 * @return u16
 */
//---------------------------------------------------------------------------- 
static u16 GetU16( u8* data, int pos )
{
	u16 lower = (u16)( data[ pos ] );
	u16 upper = (u16)( data[ pos + 1 ] );
	u16 value = ( upper << 8 ) | lower;

	return value;
}

//----------------------------------------------------------------------------
/**
 * @brief データを読み込む
 *
 * @param heap_id 使用するヒープID
 *
 * @return 読み込んだデータ
 */
//----------------------------------------------------------------------------
static ISS_DATA* LoadIssData( HEAPID heap_id )
{
	int i;
	ISS_DATA* iss_data;
	u32 size;
	u8* p_data;
	u8 data_num;

	// 本体を作成
	iss_data = (ISS_DATA*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_DATA ) );

	// データ読み込み
	p_data = (u8*)GFL_ARC_UTIL_LoadEx( ARCID_ISS_UNIT, NARC_iss_dungeon_bin, FALSE, heap_id, &size );

	// データ数を取得
	data_num = p_data[0];

	// バッファ確保
	iss_data->dataNum = data_num; 
	iss_data->param   = (BGM_PARAM*)GFL_HEAP_AllocMemory( heap_id, sizeof( BGM_PARAM ) * data_num );

	// 各データを取得
	for( i=0; i<data_num; i++ )
	{
		int pos     = 1 + ( sizeof( u16 ) + sizeof( u16 ) ) * i;
		u16 zone_id = GetU16( p_data, pos );
		u16 offset  = GetU16( p_data, pos + 2 );

		// DEBUG:
		OBATA_Printf( "pos[%d]     = %d\n", i, pos );
		OBATA_Printf( "zone_id[%d] = %d\n", i, zone_id );
		OBATA_Printf( "offset[%d]  = %d\n", i, offset );

		iss_data->param[i].zoneID = GetU16( p_data, offset );
		iss_data->param[i].pitch  = (s16)GetU16( p_data, offset + 2 );
		iss_data->param[i].tempo  = GetU16( p_data, offset + 4 );
		iss_data->param[i].reverb = GetU16( p_data, offset + 6 );
	} 


	// DEBUG:
	OBATA_Printf( "------------------------LoadIssData\n" );
	OBATA_Printf( "data_num = %d\n", data_num );
	for( i=0; i<data_num; i++ )
	{ 
		OBATA_Printf( "data[%d].zone_id = %d\n", i, iss_data->param[i].zoneID ); 
		OBATA_Printf( "data[%d].pitch   = %d\n", i, iss_data->param[i].pitch ); 
		OBATA_Printf( "data[%d].tempo   = %d\n", i, iss_data->param[i].tempo ); 
		OBATA_Printf( "data[%d].reverb  = %d\n", i, iss_data->param[i].reverb ); 
	}

	// 読み込んだデータを返す
	return iss_data;
}

//----------------------------------------------------------------------------
/**
 * @brief データを破棄する
 *
 * @param p_data 破棄するデータ
 */
//----------------------------------------------------------------------------
static void UnloadIssData( ISS_DATA* p_data )
{ 
	// パラメータ配列を破棄
	GFL_HEAP_FreeMemory( p_data->param );

	// 本体を破棄
	GFL_HEAP_FreeMemory( p_data );
}

//----------------------------------------------------------------------------
/**
 * @brief 指定ゾーンIDのBGMパラメータを取得する
 *
 * @param p_data  検索元データ
 * @param zone_id 検索対象ゾーンID
 *
 * @return 指定ゾーンIDのパラメータ(存在しない場合, NULL)
 */
//----------------------------------------------------------------------------
static const BGM_PARAM* GetBGMParam( const ISS_DATA* p_data, u16 zone_id )
{
	int i;

	// 指定ゾーンIDを検索
	for( i=0; i<p_data->dataNum; i++ )
	{
		// 発見
		if( p_data->param[i].zoneID == zone_id )
		{
			return &p_data->param[i];
		}
	}

	// 発見できず
  OBATA_Printf( "指定ゾーンのBGMパラメータが設定されていません\n" );    // DEBUG:
	return NULL;
}


//=====================================================================================================
/**
 * @breif ダンジョンISSシステム構造体
 */
//=====================================================================================================
struct _ISS_DUNGEON_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象プレイヤー
	PLAYER_WORK* pPlayer;

	// 起動状態
	BOOL isActive; 

	// ゾーンID
	u16 currentZoneID;
	u16 nextZoneID;

	// データ
	ISS_DATA* pData;

	// 現在の設定データ
	const BGM_PARAM* pActiveParam;
};


//=====================================================================================================
/**
 * 公開関数の実装
 */
//===================================================================================================== 

//----------------------------------------------------------------------------
/**
 * @brief  ダンジョンISSシステムを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  heap_id  使用するヒープID
 * 
 * @return ダンジョンISSシステム
 */
//----------------------------------------------------------------------------
ISS_DUNGEON_SYS* ISS_DUNGEON_SYS_Create( PLAYER_WORK* p_player, HEAPID heap_id )
{
	ISS_DUNGEON_SYS* p_sys;

	// メモリ確保
	p_sys = (ISS_DUNGEON_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_DUNGEON_SYS ) );

	// 初期化
	p_sys->heapID        = heap_id;
	p_sys->pPlayer       = p_player;
	p_sys->isActive      = FALSE;
	p_sys->currentZoneID = INVALID_ZONE_ID;
	p_sys->nextZoneID    = INVALID_ZONE_ID;
	p_sys->pData         = LoadIssData( heap_id );
	p_sys->pActiveParam  = NULL;

	// 作成したダンジョンISSシステムを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  ダンジョンISSシステムを破棄する
 *
 * @param p_sys 破棄するダンジョンISSシステム
 */
//----------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Delete( ISS_DUNGEON_SYS* p_sys )
{
	// システム停止
	ISS_DUNGEON_SYS_Off( p_sys );
	
	// データを破棄
	UnloadIssData( p_sys->pData );

	// 本体を破棄
	GFL_HEAP_FreeMemory( p_sys );
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
		// 新ゾーンIDのBGMパラメータを検索
		p_sys->pActiveParam = GetBGMParam( p_sys->pData, p_sys->nextZoneID );

    // BGMパラメータが設定されていない場合 ==> システム停止
    if( p_sys->pActiveParam == NULL )
    {
      ISS_DUNGEON_SYS_Off( p_sys );
      return;
    }

		// BGMの設定を反映させる
		SetBGMStatus( p_sys->pActiveParam );

		// 更新
		p_sys->currentZoneID = p_sys->nextZoneID;
	}

	// DEBUG:
	OBATA_Printf( "Dungeon ISS is ON\n" );
} 

//---------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param p_sys        通知対象のダンジョンISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//---------------------------------------------------------------------------
void ISS_DUNGEON_SYS_ZoneChange( ISS_DUNGEON_SYS* p_sys, u16 next_zone_id )
{ 
	p_sys->nextZoneID = next_zone_id; 
}

//----------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param p_sys 起動するシステム
 */
//----------------------------------------------------------------------------
void ISS_DUNGEON_SYS_On( ISS_DUNGEON_SYS* p_sys )
{
	p_sys->isActive = TRUE;
	PMSND_EnableCaptureReverb( 0, REVERB_SAMPLE_RATE, REVERB_VOLUME, REVERB_STOP_FRAME );
	SetBGMStatus( p_sys->pActiveParam );
}

//----------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param p_sys 停止させるシステム
 */
//----------------------------------------------------------------------------
void ISS_DUNGEON_SYS_Off( ISS_DUNGEON_SYS* p_sys )
{
	// 停止し, パラメータをデフォルトに戻す
	p_sys->isActive = FALSE;
	PMSND_SetStatusBGM( 256, 0, 0 ); 
	PMSND_DisableCaptureReverb();

  // DEBUG:
  OBATA_Printf( "ISS Dungeon System Off: set default status\n" );
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
