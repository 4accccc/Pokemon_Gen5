////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   iss_unit.h
 * @brief  ISSユニット
 * @author obata_toshihiro
 * @date   2009.07.16
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iss_unit.h"
#include "sound/pm_sndsys.h"
#include "arc/arc_def.h"
#include "arc/iss_unit.naix"
#include "arc/fieldmap/zone_id.h"


//=====================================================================================================
/**
 * @brief 定数・マクロ
 */
//===================================================================================================== 
// 1つのISSユニットが持つ音量空間の数
#define VOLUME_SPACE_NUM 4

// 音量を調整するトラックの指定用マスク
#define TRACK_MASK ( (1 << 9) | (1 << 10) )

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
 * @breif ISSユニット構造体
 */
//=====================================================================================================
struct _ISS_UNIT
{
	// 使用するヒープID
	HEAPID heapID;

	// 起動状態
	BOOL isActive;

	// ISSユニットの座標
	int x;
	int y;
	int z;
	
	// 音量空間
	VOLUME_SPACE volumeSpace[ VOLUME_SPACE_NUM ];
};

//-----------------------------------------------------------------
/**
 * 非公開関数のプロトタイプ宣言
 */
//-----------------------------------------------------------------
// 指定位置に該当する音量を取得する
static int GetVolumeByGrid( const ISS_UNIT* p_unit, int x, int y, int z );

// リトルエンディアンのu16値取得関数
static u16 GetU16( u8* data, int pos );

// 指定したゾーンに応じたユニットにセットアップする
static void Setup( ISS_UNIT* p_unit, u16 zone_id );


//=====================================================================================================
/**
 * 公開関数の実装
 */
//===================================================================================================== 

//----------------------------------------------------------------------------
/**
 * @brief  ISSユニットを作成する
 *
 * @param  zone_id ゾーンID
 * @param  heap_id 使用するヒープID
 * 
 * @return ISSユニット
 */
//----------------------------------------------------------------------------
ISS_UNIT* ISS_UNIT_Create( u16 zone_id, HEAPID heap_id )
{
	ISS_UNIT* p_unit;

	// メモリ確保
	p_unit = (ISS_UNIT*)GFL_HEAP_AllocMemory( heap_id, sizeof( ISS_UNIT ) );

	// 初期設定
	p_unit->heapID = heap_id;
	Setup( p_unit, zone_id );
	
	// 作成したISSユニットを返す
	return p_unit;
}

//----------------------------------------------------------------------------
/**
 * @brief  ISSユニットを破棄する
 *
 * @param p_unit 破棄するISSユニット 
 */
//----------------------------------------------------------------------------
void ISS_UNIT_Delete( ISS_UNIT* p_unit )
{
	GFL_HEAP_FreeMemory( p_unit );
}

//----------------------------------------------------------------------------
/**
 * @brief 音量を調整する
 *
 * @param p_unit 操作対象のユニット
 * @param x      主人公のx座標[グリッド]
 * @param y      主人公のy座標[グリッド]
 * @param z      主人公のz座標[グリッド]
 */
//----------------------------------------------------------------------------
void ISS_UNIT_Update( const ISS_UNIT* p_unit, int x, int y, int z )
{
	int volume;

	// 起動していなければ, 何もしない
	if( p_unit->isActive != TRUE ) return;

	// 音量を調整する
	volume = GetVolumeByGrid( p_unit, x, y, z );
	PMSND_ChangeBGMVolume( TRACK_MASK, volume );


	// DEBUG: デバッグ出力
	/*
	if( p_unit->isActive )
	{
		OBATA_Printf( "---------------------\n" );
		OBATA_Printf( "ISS Unit is active\n" );
		OBATA_Printf( "volume = %d\n", volume );
		OBATA_Printf( "x, y, z = %d, %d, %d\n", p_unit->x, p_unit->y, p_unit->z );
	}
	*/
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
	// アーカイブからデータを読み直す
	Setup( p_unit, next_zone_id );
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
 * @brief 音量を取得する
 *
 * @param p_unit 操作対象のユニット
 * @param x      主人公のx座標[グリッド]
 * @param y      主人公のy座標[グリッド]
 * @param z      主人公のz座標[グリッド]
 *
 * @return 指定した主人公座標における音量[0, 127]
 */
//---------------------------------------------------------------------------- 
static int GetVolumeByGrid( const ISS_UNIT* p_unit, int x, int y, int z )
{
	int i;
	int volume = 0;

	// 大きな空間から判定し, 属さない空間を見つけたら, 検索終了
	for( i=0; i<VOLUME_SPACE_NUM; i++ )
	{
		if( IsCover( &p_unit->volumeSpace[i], p_unit->x, p_unit->y, p_unit->z, x, y, z ) != TRUE )
		{
			break;
		}

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
 * @brief 指定したゾーンに応じたユニットにセットアップする
 *
 * @param p_unit  データを設定するユニット
 * @param zone_id ゾーンID
 */
//---------------------------------------------------------------------------- 
static void Setup( ISS_UNIT* p_unit, u16 zone_id )
{
	int i;
	int pos;		// 参照位置
	u8* header;		// ヘッダ部の先頭アドレス
	u16 offset;		// データ部先頭へのオフセット
	u8 table_size;	// 参照テーブルに登録されているデータの数

	// アーカイブデータの読み出し
	header = (u8*)GFL_ARC_UTIL_Load( ARCID_ISS_UNIT, NARC_iss_unit_iss_unit_bin, FALSE, p_unit->heapID );

	// 指定ゾーンIDがテーブルに登録されているかどうかを検索
	offset     = 0;
	pos        = 0;
	table_size = header[ pos++ ];	// テーブルサイズを取得
	for( i=0; i<table_size; i++ )
	{
		u16 id = GetU16( header, pos );
		pos += 2;
		
		// 指定ゾーンIDを発見したら, オフセットを取得し検索終了
		if( id == zone_id )
		{
			offset = GetU16( header, pos );
			break;
		} 
		pos += 2;
	}
	
	// ユニットのセットアップ
	if( 0 < offset )	// (指定ゾーンにISSユニットが存在する場合)
	{
		u8* data = header + offset;		// データ部の先頭アドレス
		p_unit->isActive = TRUE;
		p_unit->x = GetU16( data, 2 );
		p_unit->y = GetU16( data, 4 );
		p_unit->z = GetU16( data, 6 );
		p_unit->volumeSpace[0].volume = *( (u8*)( data +  8 ) );
		p_unit->volumeSpace[1].volume = *( (u8*)( data +  9 ) );
		p_unit->volumeSpace[2].volume = *( (u8*)( data + 10 ) );
		p_unit->volumeSpace[3].volume = *( (u8*)( data + 11 ) );
		p_unit->volumeSpace[0].x_range = *( (u8*)( data + 12 ) );
		p_unit->volumeSpace[1].x_range = *( (u8*)( data + 13 ) );
		p_unit->volumeSpace[2].x_range = *( (u8*)( data + 14 ) );
		p_unit->volumeSpace[3].x_range = *( (u8*)( data + 15 ) );
		p_unit->volumeSpace[0].y_range = *( (u8*)( data + 16 ) );
		p_unit->volumeSpace[1].y_range = *( (u8*)( data + 17 ) );
		p_unit->volumeSpace[2].y_range = *( (u8*)( data + 18 ) );
		p_unit->volumeSpace[3].y_range = *( (u8*)( data + 19 ) );
		p_unit->volumeSpace[0].z_range = *( (u8*)( data + 20 ) );
		p_unit->volumeSpace[1].z_range = *( (u8*)( data + 21 ) );
		p_unit->volumeSpace[2].z_range = *( (u8*)( data + 22 ) );
		p_unit->volumeSpace[3].z_range = *( (u8*)( data + 23 ) );
	}
	else		// (指定ゾーンにISSユニットが存在しない場合)
	{
		p_unit->isActive = FALSE;
	}
	
	// 後始末
	GFL_HEAP_FreeMemory( header );
}
