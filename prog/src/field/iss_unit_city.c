////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_unit_city.h
 * @brief  街ISSユニット
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_unit_city.h"
#include "field_sound.h"
#include "gamesystem/playerwork.h"
#include "field/field_const.h"		// FIELD_CONST_GRID_FX32_SIZE
#include "arc/arc_def.h"
#include "arc/iss_unit.naix"


//=====================================================================================================
/**
 * @brief 定数・マクロ
 */
//===================================================================================================== 
// 1つの街ISSユニットが持つ音量空間の数
#define VOLUME_SPACE_NUM 4

// 絶対値を取得する
#define ABS(n) ( (n)>0? (n) : -(n) )


//=====================================================================================================
/**
 * @brief 音量空間 構造体
 *	ある空間の大きさと、その空間内での音量を保持する
 */
//=====================================================================================================
typedef struct 
{
	u8 volume;	// 音量
	u8 x_range;	// x方向範囲
	u8 y_range; // y方向範囲
	u8 z_range; // z方向範囲
}
VOLUME_SPACE;

//-----------------------------------------------------------------
/**
 * 非公開関数のプロトタイプ宣言
 */
//-----------------------------------------------------------------
// 指定した座標が空間内にあるかどうかを判定する
static BOOL IsCover( const VOLUME_SPACE* p_vs, int cx, int cy, int cz, int x, int y, int z );


//=====================================================================================================
/**
 * @breif 街ISSユニット構造体
 */
//=====================================================================================================
typedef struct
{
	// 配置場所のゾーンID
	u16 zoneID;

	// 座標(単位:グリッド)
	int x;
	int y;
	int z;

	// 音量空間
	VOLUME_SPACE volumeSpace[ VOLUME_SPACE_NUM ];
}
UNIT;

//-----------------------------------------------------------------
/**
 * 非公開関数のプロトタイプ宣言
 */
//-----------------------------------------------------------------

// グリッド座標を求める
static void CalcGrid( const VecFx32* p_vec, int* dest_x, int* dest_y, int* dest_z ); 

// 指定位置に該当する音量を取得する
static int GetVolume( const UNIT* p_unit, const VecFx32* p_pos );


//=====================================================================================================
/**
 * @breif 街ISSユニット構造体
 */
//=====================================================================================================
struct _ISS_UNIT_CITY
{
	// 使用するヒープID
	HEAPID heapID;

	// 監視対象プレイヤー
	PLAYER_WORK* pPlayer;

	// 起動状態
	BOOL isActive;		// 動作中かどうか
	u8   activeUnitNo;	// 動作中のユニット番号

	// ISSユニット情報
	u8    unitNum;		// ユニット数
	UNIT* unitData;		// ユニット配列
};

//-----------------------------------------------------------------
/**
 * 非公開関数のプロトタイプ宣言
 */
//-----------------------------------------------------------------

// リトルエンディアンのu16値取得関数
static u16 GetU16( u8* data, int pos );

// 全ての街ISSユニットのデータを読み込む
static void LoadUnitData( ISS_UNIT_CITY* p_sys );



//=====================================================================================================
/**
 * 公開関数の実装
 */
//===================================================================================================== 

//----------------------------------------------------------------------------
/**
 * @brief  街ISSユニットを作成する
 *
 * @param  p_player 監視対象のプレイヤー
 * @param  zone_id  ゾーンID
 * @param  heap_id  使用するヒープID
 * 
 * @return 街ISSユニット
 */
//----------------------------------------------------------------------------
ISS_UNIT_CITY* ISS_UNIT_CITY_Create( PLAYER_WORK* p_player, u16 zone_id, HEAPID heap_id )
{
	ISS_UNIT_CITY* p_sys;

	// メモリ確保
	p_sys = (ISS_UNIT_CITY*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_UNIT_CITY ) );

	// 初期化
	p_sys->heapID       = heap_id;
	p_sys->pPlayer      = p_player;
	p_sys->isActive     = FALSE;
	p_sys->activeUnitNo = 0;
	p_sys->unitNum      = 0;
	p_sys->unitData     = NULL;

	// ユニット情報の読み込み
	LoadUnitData( p_sys );

	// 設定
	ISS_UNIT_CITY_ZoneChange( p_sys, zone_id );
	
	// 作成した街ISSユニットを返す
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 * @brief  街ISSユニットを破棄する
 *
 * @param p_sys 破棄する街ISSシステム
 */
//----------------------------------------------------------------------------
void ISS_UNIT_CITY_Delete( ISS_UNIT_CITY* p_sys )
{
	// 各ユニットを破棄
	GFL_HEAP_FreeMemory( p_sys->unitData );

	// 本体の破棄
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 * @brief プレイヤーを監視し, 音量を調整する
 *
 * @param p_sys 動作対象の街ISSシステム
 */
//----------------------------------------------------------------------------
void ISS_UNIT_CITY_Update( ISS_UNIT_CITY* p_sys )
{
	int volume;
	const VecFx32* p_pos = NULL;

	// 起動していなければ, 何もしない
	if( p_sys->isActive != TRUE ) return;

	// 音量を調整する
	p_pos  = PLAYERWORK_getPosition( p_sys->pPlayer );
	volume = GetVolume( &p_sys->unitData[ p_sys->activeUnitNo ], p_pos );
	FIELD_SOUND_ChangeBGMActionVolume( volume );

	// DEBUG: デバッグ出力
	if( p_sys->isActive )
	{
		UNIT* p_unit = &p_sys->unitData[ p_sys->activeUnitNo ];
		/*
		OBATA_Printf( "---------------------\n" );
		OBATA_Printf( "City ISS Unit is active\n" );
		OBATA_Printf( "active unit no = %d\n", p_sys->activeUnitNo );
		OBATA_Printf( "volume = %d\n", volume );
		OBATA_Printf( "x, y, z = %d, %d, %d\n", p_unit->x, p_unit->y, p_unit->z );
		*/
		OS_Printf( "---------------------\n" );
		OS_Printf( "City ISS Unit is active\n" );
		OS_Printf( "active unit no = %d\n", p_sys->activeUnitNo );
		OS_Printf( "volume = %d\n", volume );
		OS_Printf( "x, y, z = %d, %d, %d\n", p_unit->x, p_unit->y, p_unit->z );
	}
}
	

//---------------------------------------------------------------------------
/**
 * @brief ゾーン切り替えを通知する
 *
 * @param p_sys        通知対象の街ISSシステム
 * @param next_zone_id 新しいゾーンID
 */
//---------------------------------------------------------------------------
void ISS_UNIT_CITY_ZoneChange( ISS_UNIT_CITY* p_sys, u16 next_zone_id )
{
	int i;

	// DEBUG:
	{
		int i;
		OBATA_Printf( "next zone id = %d\n", next_zone_id );
		for( i=0; i<p_sys->unitNum; i++ )
		{
			OBATA_Printf( "zone if of unit[%d] = %d\n", i, p_sys->unitData[i].zoneID );
		}
	}
	
	// 新しいゾーンIDを持つユニットを探す
	for( i=0; i<p_sys->unitNum; i++ )
	{
		// 発見 ==> システム起動
		if( p_sys->unitData[i].zoneID == next_zone_id )
		{
			p_sys->isActive     = TRUE;
			p_sys->activeUnitNo = i;
			return;
		}
	}

	// 指定ゾーンIDにISSユニットが存在しない場合 ==> システム停止
	p_sys->isActive = FALSE;
}


//----------------------------------------------------------------------------
/**
 * @brief 動作状態を設定する
 *
 * @param active 動作させるかどうか
 */
//----------------------------------------------------------------------------
extern void ISS_UNIT_CITY_SetActive( ISS_UNIT_CITY* p_unit, BOOL active )
{
	p_unit->isActive = active;
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
extern BOOL ISS_UNIT_CITY_IsActive( const ISS_UNIT_CITY* p_unit )
{
	return p_unit->isActive;
}


//=====================================================================================================
/**
 * 非公開関数の実装
 */
//===================================================================================================== 

//---------------------------------------------------------------------------- 
/**
 * @brief 指定した座標が空間内にあるかどうかを判定する
 *
 * @param p_vs	判定対象の音量空間
 * @param cx	音量空間中心部のx座標
 * @param cy	音量空間中心部のy座標
 * @param cz	音量空間中心部のz座標
 * @param x		主人公のx座標
 * @param y		主人公のy座標
 * @param z		主人公のz座標
 *
 * @return (x, y, z) が p_vs空間の内部にある場合 TRUE
 */
//---------------------------------------------------------------------------- 
static BOOL IsCover( const VOLUME_SPACE* p_vs, int cx, int cy, int cz, int x, int y, int z )
{
	int min_x, min_y, min_z;
	int max_x, max_y, max_z;

	// 空間の最小値・最大値を求める
	min_x = cx - p_vs->x_range;
	min_y = cy - p_vs->y_range;
	min_z = cz - p_vs->z_range; 
	max_x = cx + p_vs->x_range;
	max_y = cy + p_vs->y_range;
	max_z = cz + p_vs->z_range;

	// 判定
	if( ( x < min_x ) | ( max_x < x ) |
		( y < min_y ) | ( max_y < y ) |
		( z < min_z ) | ( max_z < z ) )
	{
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 * @brief グリッド座標を求める
 *
 * @param p_vec  ワールド座標系
 * @param dest_x 求めたグリッドx座標の格納先
 * @param dest_y 求めたグリッドy座標の格納先
 * @param dest_z 求めたグリッドz座標の格納先
 */
//----------------------------------------------------------------------------
static void CalcGrid( const VecFx32* p_vec, int* dest_x, int* dest_y, int* dest_z )
{
	*dest_x = (int)( ( FX_Div( p_vec->x, FIELD_CONST_GRID_FX32_SIZE) & FX32_INT_MASK ) >> FX32_SHIFT );
	*dest_y = (int)( ( FX_Div( p_vec->y, FIELD_CONST_GRID_FX32_SIZE) & FX32_INT_MASK ) >> FX32_SHIFT );
	*dest_z = (int)( ( FX_Div( p_vec->z, FIELD_CONST_GRID_FX32_SIZE) & FX32_INT_MASK ) >> FX32_SHIFT );
} 

//---------------------------------------------------------------------------- 
/**
 * @brief 音量を取得する
 *
 * @param p_unit 操作対象のユニット
 * @param p_pos  主人公の座標[単位：ワールド座標]
 *
 * @return 指定した主人公座標における音量[0, 127]
 */
//---------------------------------------------------------------------------- 
static int GetVolume( const UNIT* p_unit, const VecFx32* p_pos )
{
	int i;
	int volume = 0;
	int x, y, z;

	// グリッド座標を計算
	CalcGrid( p_pos, &x, &y, &z );

	// 大きな空間から判定し, 属さない空間を見つけたら, 検索終了
	for( i=0; i<VOLUME_SPACE_NUM; i++ )
	{
		if( IsCover( &p_unit->volumeSpace[i], p_unit->x, p_unit->y, p_unit->z, x, y, z ) != TRUE )
		{
			break;
		}

		// 主人公が属する, 最も小さい空間に設定された音量を取得する
		volume = p_unit->volumeSpace[i].volume;
	}

	return volume;
}

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
 * @brief 全ての街ISSユニットデータを読み込む
 *
 * @param p_sys   データを設定する街ISSシステム
 * @param zone_id ゾーンID
 *
 * [バイナリ・データ・フォーマット]
 * 
 * [0] ユニット数
 * 
 * [1] ユニットのゾーンID
 * [2] 
 * [3] ユニットデータへの, 先頭からのオフセット
 * [4] 
 * 
 * [1]-[4]がユニット数だけ存在
 *
 * [x+ 0] ゾーンID 
 * [x+ 1] 
 * [x+ 2] x座標 
 * [x+ 3] 
 * [x+ 4] y座標 
 * [x+ 5] 
 * [x+ 6] z座標 
 * [x+ 7] 
 * [x+ 8] 音量1
 * [x+ 9] 音量2
 * [x+10] 音量3
 * [x+11] 音量4
 * [x+12] x範囲1 
 * [x+13] x範囲2 
 * [x+14] x範囲3 
 * [x+15] x範囲4 
 * [x+16] y範囲1 
 * [x+17] y範囲2 
 * [x+18] y範囲3 
 * [x+19] y範囲4 
 * [x+20] z範囲1 
 * [x+21] z範囲2 
 * [x+22] z範囲3 
 * [x+23] z範囲4 
 *
 * [x+0]-[x+23]がユニット数だけ存在
 */   
//---------------------------------------------------------------------------- 
static void LoadUnitData( ISS_UNIT_CITY* p_sys )
{
	int i;
	int pos = 0;	// 参照位置
	u8* header;		// ヘッダ部の先頭アドレス
	u8 unit_num;	// ユニット数
	u16* zone_id;	// 各ユニットのゾーンID
	u16* offset;	// 各ユニットデータ先頭部へのオフセット

	// アーカイブデータの読み込み
	header = (u8*)GFL_ARC_UTIL_Load( ARCID_ISS_UNIT, NARC_iss_unit_iss_unit_bin, FALSE, p_sys->heapID );

	// ユニット数を取得
	unit_num = header[ pos++ ];	

	// テーブルを作成
	zone_id = (u16*)GFL_HEAP_AllocMemoryLo( p_sys->heapID, sizeof( u16 ) * unit_num );
	offset  = (u16*)GFL_HEAP_AllocMemoryLo( p_sys->heapID, sizeof( u16 ) * unit_num );

	// テーブルデータを取得
	for( i=0; i<unit_num; i++ )
	{
		zone_id[i] = GetU16( header, pos );		pos += 2;	// ゾーンID
		offset[i]  = GetU16( header, pos );		pos += 2;	// オフセット
	}
	
	// 全ユニットのセットアップ
	p_sys->unitNum  = unit_num;
	p_sys->unitData = (UNIT*)GFL_HEAP_AllocMemory( p_sys->heapID, sizeof( UNIT ) * unit_num );
	for( i=0; i<unit_num; i++ )
	{
		u8* data = header + offset[i];				// データ部の先頭アドレス
		p_sys->unitData[i].zoneID = zone_id[i];
		p_sys->unitData[i].x = GetU16( data, 2 );
		p_sys->unitData[i].y = GetU16( data, 4 );
		p_sys->unitData[i].z = GetU16( data, 6 );
		p_sys->unitData[i].volumeSpace[0].volume = *( (u8*)( data +  8 ) );
		p_sys->unitData[i].volumeSpace[1].volume = *( (u8*)( data +  9 ) );
		p_sys->unitData[i].volumeSpace[2].volume = *( (u8*)( data + 10 ) );
		p_sys->unitData[i].volumeSpace[3].volume = *( (u8*)( data + 11 ) );
		p_sys->unitData[i].volumeSpace[0].x_range = *( (u8*)( data + 12 ) );
		p_sys->unitData[i].volumeSpace[1].x_range = *( (u8*)( data + 13 ) );
		p_sys->unitData[i].volumeSpace[2].x_range = *( (u8*)( data + 14 ) );
		p_sys->unitData[i].volumeSpace[3].x_range = *( (u8*)( data + 15 ) );
		p_sys->unitData[i].volumeSpace[0].y_range = *( (u8*)( data + 16 ) );
		p_sys->unitData[i].volumeSpace[1].y_range = *( (u8*)( data + 17 ) );
		p_sys->unitData[i].volumeSpace[2].y_range = *( (u8*)( data + 18 ) );
		p_sys->unitData[i].volumeSpace[3].y_range = *( (u8*)( data + 19 ) );
		p_sys->unitData[i].volumeSpace[0].z_range = *( (u8*)( data + 20 ) );
		p_sys->unitData[i].volumeSpace[1].z_range = *( (u8*)( data + 21 ) );
		p_sys->unitData[i].volumeSpace[2].z_range = *( (u8*)( data + 22 ) );
		p_sys->unitData[i].volumeSpace[3].z_range = *( (u8*)( data + 23 ) );
	}
	
	// 後始末
	GFL_HEAP_FreeMemory( zone_id );
	GFL_HEAP_FreeMemory( offset );
	GFL_HEAP_FreeMemory( header );
} 
