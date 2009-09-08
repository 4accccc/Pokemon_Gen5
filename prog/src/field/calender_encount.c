#include "calender_encount.h"
#include "arc/arc_def.h"
#include "arc/calender.naix"


//==================================================================================
/**
 * @brief システムワーク
 */
//==================================================================================
struct _CALENDER_ENCOUNT_INFO
{
  u16 dataNum;    // データ数
  u8* month;      // 月配列
  u8* day;        // 日配列
  u8* encountNo;  // 出現率番号配列
};


//==================================================================================
/**
 * @brief 非公開関数のプロトタイプ宣言
 */
//==================================================================================

// 指定した2バイトをリトルエンディアンとして解釈し, 値を取得する
static u16 GetU16( u8* data, int pos );

// データを読み込む
static void LoadData( CALENDER_ENCOUNT_INFO* p_info, HEAPID heap_id );


//==================================================================================
/**
 * @brief 公開関数の定義
 */
//================================================================================== 

//---------------------------------------------------------------------------------
/**
 * @brief エンカウント・データを作成する
 *
 * @param heap_id 使用するヒープID
 *
 * @return 作成したエンカウント・データ
 */
//---------------------------------------------------------------------------------
CALENDER_ENCOUNT_INFO* CALENDER_ENCOUNT_INFO_Create( HEAPID heap_id )
{
  CALENDER_ENCOUNT_INFO* p_info;

  // ワークを作成
  p_info = (CALENDER_ENCOUNT_INFO*)GFL_HEAP_AllocMemory( heap_id, sizeof( CALENDER_ENCOUNT_INFO ) );

  // データを読み込む
  LoadData( p_info, heap_id );

  // 作成したデータを返す
  return p_info;
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定日の出現率変化番号を取得する
 *
 * @param p_info データ
 * @parma p_date 日付
 *
 * @return 指定した日付の出現率変化番号
 */
//---------------------------------------------------------------------------------
u8 CALENDER_ENCOUNT_INFO_GetEncountNo( CALENDER_ENCOUNT_INFO* p_info, const RTCDate* p_date )
{
  int i;
  u8 encount_no = ENCOUNT_NO_0;

  // 指定日を検索
  for( i=0; i<p_info->dataNum; i++ )
  {
    // 発見
    if( ( p_info->month[i] == p_date->month ) &&
        ( p_info->day[i]   == p_date->day ) )
    { 
      encount_no = p_info->encountNo[i];
      break;
    }
  }

  // 不正値を排除
  if( ENCOUNT_NO_MAX <= encount_no )
  {
    OBATA_Printf( "-----------------------------------\n" );
    OBATA_Printf( "不正な出現率変化番号(%d)を発見!!\n", encount_no );
    OBATA_Printf( "-----------------------------------\n" );
    encount_no = ENCOUNT_NO_0;
  }

  // DEBUG:
  OBATA_Printf( "%d/%d : encountNo = %d\n", p_date->month, p_date->day, encount_no );

  // 指定日のデータが存在しない ==> デフォルト値0番を返す
  return encount_no;
}

//---------------------------------------------------------------------------------
/**
 * @brief エンカウント・データを破棄する
 *
 * @param p_info 破棄するデータ
 */ 
//---------------------------------------------------------------------------------
void CALENDER_ENCOUNT_INFO_Delete( CALENDER_ENCOUNT_INFO* p_info )
{
  // 各配列を破棄
  GFL_HEAP_FreeMemory( p_info->month );
  GFL_HEAP_FreeMemory( p_info->day );
  GFL_HEAP_FreeMemory( p_info->encountNo );

  // 本体を破棄
  GFL_HEAP_FreeMemory( p_info );
}


//==================================================================================
/**
 * @brief 非公開関数の定義
 */
//==================================================================================

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

//---------------------------------------------------------------------------------
/**
 * @brief データを読み込む
 *
 * @param p_info  読み込んだデータの格納先
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------------
static void LoadData( CALENDER_ENCOUNT_INFO* p_info, HEAPID heap_id )
{
  int i;
  u8* data;
  int pos = 0; 

  // データ読み込み
  data = (u8*)GFL_ARC_LoadDataAlloc( ARCID_CALENDER, NARC_calender_encount_bin, heap_id );

  // データ数を取得
  p_info->dataNum = GetU16( data, pos );
  pos += 2;

  // バッファ確保
  p_info->month     = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->dataNum );
  p_info->day       = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->dataNum );
  p_info->encountNo = (u8*)GFL_HEAP_AllocMemory( heap_id, sizeof( u8 ) * p_info->dataNum );

  // 月・日・出現率変化番号をデータ数だけ取得
  for( i=0; i<p_info->dataNum; i++ )
  {
    p_info->month[i]     = data[ pos++ ];
    p_info->day[i]       = data[ pos++ ];
    p_info->encountNo[i] = data[ pos++ ];
  }

  // 後始末
  GFL_HEAP_FreeMemory( data );


  // DEBUG:
  OBATA_Printf( "dataNum = %d\n", p_info->dataNum );
  for( i=0; i<p_info->dataNum; i++ )
  {
    OBATA_Printf( "%d/%d = %d\%\n", p_info->month[i], p_info->day[i], p_info->encountNo[i] );
  } 
}
