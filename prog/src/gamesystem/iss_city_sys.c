/////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_city_sys.c
 * @brief  街ISSシステム
 * @author obata_toshihiro
 * @date   2009.07.16
 */
/////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_city_sys.h"
#include "../field/field_sound.h"
#include "gamesystem/playerwork.h"
#include "../../include/field/field_const.h"		// FIELD_CONST_GRID_FX32_SIZE
#include "arc/arc_def.h"
#include "field/field_const.h"
#include "iss_city_unit.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//===========================================================================================
// ■定数・マクロ
//===========================================================================================

// 無効ユニット番号(街ISSのBGMが鳴っているが, ISSユニットが配置されていない状況)
#define INVALID_UNIT_NO (0xff)

// 最大音量
#define MAX_VOLUME (127)
#define INVALID_VOLUME (0xffff)

// 操作トラック
#define TRACKBIT ((1<<(9-1))|(1<<(10-1))) // 9,10トラック


//===========================================================================================
// ■街ISSシステム構造体
//===========================================================================================
struct _ISS_CITY_SYS
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象プレイヤー
	PLAYER_WORK* pPlayer;

	// 起動状態
	BOOL isActive;		  // 動作中かどうか
	u8   activeUnitNo;	// 動作中のユニット番号
  int  volume;        // 音量

	// ユニット情報
	u8        unitNum;		// ユニット数
	ISS_C_UNIT** unit;		// ユニット配列
};


//===========================================================================================
// ■非公開関数のプロトタイプ宣言
//===========================================================================================
static void BootSystem( ISS_CITY_SYS* sys );
static void StopSystem( ISS_CITY_SYS* sys );
static void LoadUnitData( ISS_CITY_SYS* sys );
static void UpdateVolume( ISS_CITY_SYS* sys );
static void ChangeUnit( ISS_CITY_SYS* sys, u16 zone_id );


//===========================================================================================
// ■公開関数の実装
//===========================================================================================

//-------------------------------------------------------------------------------------------
/**
 * @brief  街ISSシステムを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  heap_id  使用するヒープID
 * 
 * @return 街ISSシステム
 */
//-------------------------------------------------------------------------------------------
ISS_CITY_SYS* ISS_CITY_SYS_Create( PLAYER_WORK* p_player, HEAPID heap_id )
{
	ISS_CITY_SYS* sys;

	// メモリ確保
	sys = (ISS_CITY_SYS*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_CITY_SYS ) );

	// 初期化
	sys->heapID       = heap_id;
	sys->pPlayer      = p_player;
	sys->isActive     = FALSE;
	sys->activeUnitNo = INVALID_UNIT_NO;
  sys->volume       = INVALID_VOLUME;
	sys->unitNum      = 0;
	sys->unit         = NULL;

	// ユニット情報の読み込み
	LoadUnitData( sys );

  // DEBUG:
  OBATA_Printf( "ISS-C: create\n" );
	
	// 作成した街ISSシステムを返す
	return sys;
}

//------------------------------------------------------------------------------------------
/**
 * @brief  街ISSシステムを破棄する
 *
 * @param sys 破棄する街ISSシステム
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Delete( ISS_CITY_SYS* sys )
{
  int i;

	// 各ユニットを破棄
  for( i=0; i<sys->unitNum; i++ )
  {
    ISS_C_UNIT_Delete( sys->unit[i] );
  }
	GFL_HEAP_FreeMemory( sys->unit );

	// 本体の破棄
	GFL_HEAP_FreeMemory( sys );

  // DEBUG:
  OBATA_Printf( "ISS-C: delete\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param sys 動作対象の街ISSシステム
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Update( ISS_CITY_SYS* sys )
{
  // 停止中
	if( sys->isActive != TRUE ){ return; }

  // 音量を調整する
  UpdateVolume( sys );
}
	
//------------------------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param sys        通知対象の街ISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_ZoneChange( ISS_CITY_SYS* sys, u16 next_zone_id )
{ 
  // 停止中
  if( sys->isActive != TRUE ){ return; }

  // ユニットを切り替える
  ChangeUnit( sys, next_zone_id );

  // 音量を調整
  UpdateVolume( sys );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 起動するシステム
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_On( ISS_CITY_SYS* sys )
{ 
  BootSystem( sys );
}

//------------------------------------------------------------------------------------------
/**
 * @brief システムを停止させる
 *
 * @param sys 停止させるシステム
 */
//------------------------------------------------------------------------------------------
void ISS_CITY_SYS_Off( ISS_CITY_SYS* sys )
{
  StopSystem( sys );
}

//------------------------------------------------------------------------------------------
/**
 * @breif 動作状態を取得する
 *
 * @param sys 状態を調べるISSシステム
 * 
 * @return 動作中かどうか
 */
//------------------------------------------------------------------------------------------
BOOL ISS_CITY_SYS_IsOn( const ISS_CITY_SYS* sys )
{
	return sys->isActive;
}


//===========================================================================================
// ■非公開関数の実装
//===========================================================================================

//-------------------------------------------------------------------------------------------
/**
 * @brief システムを起動する
 *
 * @param sys 街ISSシステム
 */
//-------------------------------------------------------------------------------------------
static void BootSystem( ISS_CITY_SYS* sys )
{
  // 起動中
  if( sys->isActive ){ return; }

  // システム起動
  sys->isActive = TRUE;
  sys->volume   = INVALID_VOLUME;
  // DEBUG:
  OBATA_Printf( "ISS-C: boot\n" );

  // ユニット検索
  {
    u16 zone_id;
    zone_id = PLAYERWORK_getZoneID( sys->pPlayer );
    ChangeUnit( sys, zone_id ); 
  } 
}

//-------------------------------------------------------------------------------------------
/**
 * @brief システムを停止する
 *
 * @param sys 街ISSシステム
 */
//-------------------------------------------------------------------------------------------
static void StopSystem( ISS_CITY_SYS* sys )
{
  // 停止中
  if( !sys->isActive ){ return; }

  // 停止
	sys->isActive     = FALSE;
  sys->activeUnitNo = INVALID_UNIT_NO;
  // 操作していたトラックを元に戻す
  //PMSND_ChangeBGMVolume( TRACKBIT, 127 ); 

  // DEBUG:
  OBATA_Printf( "ISS-C: stop\n" );
}

//-------------------------------------------------------------------------------------------
/**
 * @brief ユニットデータを読み込む
 *
 * @param sys 読み込んだデータを保持するシステム
 */
//-------------------------------------------------------------------------------------------
static void LoadUnitData( ISS_CITY_SYS* sys )
{
  int datid;

  // 登録されているユニットの数を取得
  sys->unitNum = GFL_ARC_GetDataFileCnt( ARCID_ISS_CITY );

  // 各ユニットを作成
  sys->unit = GFL_HEAP_AllocMemory( sys->heapID, sys->unitNum * sizeof(ISS_C_UNIT*) );

  // 各ユニットデータを読み込む
  for( datid=0; datid<sys->unitNum; datid++ )
  {
    sys->unit[datid] = ISS_C_UNIT_Create( sys->heapID, datid );
  } 
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 音量を調整する
 *
 * @param sys 調整を行う
 */
//-------------------------------------------------------------------------------------------
static void UpdateVolume( ISS_CITY_SYS* sys )
{
  int new_volume = 0;
  const VecFx32* pos;

  // 起動していない
  GF_ASSERT( sys->isActive );  

  // ユニットが配置されていない
  if( sys->activeUnitNo == INVALID_UNIT_NO ){ return; }

  // ユニットと自機の位置からボリュームを算出
  pos        = PLAYERWORK_getPosition( sys->pPlayer );
  new_volume = ISS_C_UNIT_GetVolume( sys->unit[sys->activeUnitNo], pos );

  // 音量が変化した場合
  if( sys->volume != new_volume )
  {
    // 更新
    sys->volume = new_volume;
    PMSND_ChangeBGMVolume( TRACKBIT, new_volume );
    // DEBUG:
    OBATA_Printf( "ISS-C: update volume ==> %d\n", new_volume );
  }
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンに配置されたユニットに切り替える
 *
 * @param sys     ユニットを切り替えるシステム
 * @param zone_id 切り替え先のゾーンID
 */
//-------------------------------------------------------------------------------------------
static void ChangeUnit( ISS_CITY_SYS* sys, u16 zone_id )
{
	int unit_idx;

  // 起動していない
  GF_ASSERT( sys->isActive );

  // 検索
	for( unit_idx=0; unit_idx<sys->unitNum; unit_idx++ )
	{
		// 発見 ==> ユニット番号を更新
		if( ISS_C_UNIT_GetZoneID( sys->unit[unit_idx] ) == zone_id )
		{ 
			sys->activeUnitNo = unit_idx;
      // DEBUG:
      OBATA_Printf( "ISS-C: change unit index ==> %d\n", sys->activeUnitNo );
      return;
		}
	}

  // 配置されていない
  sys->activeUnitNo = INVALID_UNIT_NO;
}
