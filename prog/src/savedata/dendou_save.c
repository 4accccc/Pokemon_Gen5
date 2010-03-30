//==============================================================================
/**
 * @file    dendou_save.c
 * @brief   殿堂入りセーブデータ
 * @author  matsuda
 * @date    2010.03.12(金)
 *
 * @note    WBでは殿堂入りデータは外部セーブで内部セーブの２箇所にデータを持ちます。
 *          外部セーブに持つ従来の殿堂入りデータで１５個ぶんの殿堂入りデータを、
 *          内部セーブに一つだけ持つデータは初クリア時の「Ｎに勝利した」時のデータと
 *          して保持します。外部の殿堂入りは更新されていきますが、内部は一度きりの書き込みです
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/dendou_save.h"
#include "pm_define.h"



//==============================================================================
//  定数定義
//==============================================================================
enum {
  POKENAME_BUFSIZE = MONS_NAME_SIZE+EOM_SIZE,   // 11
  OYANAME_BUFSIZE = PERSON_NAME_SIZE+EOM_SIZE,  // 8
};

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static int inline recIndex_to_datIndex( const DENDOU_SAVEDATA* data, int recIndex );




//----------------------------------------------------------
/**
 *  殿堂入りポケモン個体データ（60 bytes）
 */
//----------------------------------------------------------
typedef struct {
  u16 monsno;       ///< ポケモンナンバー（０ならデータ全体が無効）
  u8  level;        ///< レベル
  u8  formNumber:6; ///< フォルム
  u8  sex:2;        ///< 性別
  u32 personalRandom;   ///< 個体乱数
  u32 idNumber;     ///< ID
  STRCODE nickname[ POKENAME_BUFSIZE ]; ///< ニックネーム
  STRCODE oyaname[ OYANAME_BUFSIZE ];   ///< おや名
  u16 waza[ 4 ];      ///< 技ナンバー
}DENDOU_POKEMON_DATA_INSIDE;


//----------------------------------------------------------
/**
 *  殿堂入り１件レコードデータ（364 bytes）(内部にも１件持ちます）
 */
//----------------------------------------------------------
struct DENDOU_RECORD{
  DENDOU_POKEMON_DATA_INSIDE  pokemon[ TEMOTI_POKEMAX ];  ///< ポケモンデータ
  u16     year;       ///< 殿堂入り日付（年） 2006 ～
  u8      month;        ///< 殿堂入り日付（月） 1～12
  u8      day;        ///< 殿堂入り日付（日） 1～31
};


//----------------------------------------------------------
/**
 *  殿堂入りセーブデータ本体（10920 bytes）
 */
//----------------------------------------------------------
struct _DENDOU_SAVEDATA {

  DENDOU_RECORD record[ DENDOU_RECORD_MAX ];

  u32   savePoint;    ///< record配列中の、次にセーブするindex
  u32   latestNumber; ///< 最新データの殿堂入り通算回数
};




//=============================================================================================
/**
 * @brief 殿堂入りデータセーブデータサイズを返す
 * @retval  u32   
 */
//=============================================================================================
u32 DendouData_GetWorkSize( void )
{
  return sizeof(DENDOU_SAVEDATA);
}

//=============================================================================================
/**
 * @brief   クリアポケモンセーブデータサイズを返す
 * @retval  u32   
 */
//=============================================================================================
u32 GameClearData_GetWorkSize( void )
{
  return sizeof(DENDOU_RECORD);
}

//------------------------------------------------------------------
/**
 * データ初期化
 *
 * @param   data    
 *
 */
//------------------------------------------------------------------
void DendouData_Init( void *work )
{
  GFL_STD_MemClear( work, sizeof(DENDOU_SAVEDATA) );
}

void GameClearData_Init( void *work )
{
  GFL_STD_MemClear( work, sizeof(DENDOU_RECORD) );
}


//=============================================================================================
/**
 * @brief 殿堂入りレコードデータへ手持ちポケモンを確認した上で書き込む
 *
 * @param   record    
 * @param   party   
 * @param   date    
 * @param   heap_id   
 */
//=============================================================================================
void DendouRecord_Add( DENDOU_RECORD* record, const POKEPARTY* party, const RTCDate* date, HEAPID heap_id )
{
  POKEMON_PARAM*  pp;
  STRBUF*         strbuf;
  int  pokeCount, i, p;
  BOOL fastFlag;

  GF_ASSERT(record!=NULL);

  pokeCount = PokeParty_GetPokeCount( party );

  strbuf = GFL_STR_CreateBuffer( POKENAME_BUFSIZE, heap_id );

  GFL_STD_MemClear( record->pokemon,  sizeof(DENDOU_POKEMON_DATA_INSIDE)*TEMOTI_POKEMAX );

  for(i=0, p=0; i<pokeCount; i++)
  {
    pp = PokeParty_GetMemberPointer( party, i );

    fastFlag = PP_FastModeOn( pp );

    if( PP_Get(pp, ID_PARA_tamago_flag, NULL) == FALSE )
    {
      record->pokemon[p].monsno = PP_Get( pp, ID_PARA_monsno, NULL );
      record->pokemon[p].level = PP_Get( pp, ID_PARA_level, NULL );
      record->pokemon[p].formNumber = PP_Get( pp, ID_PARA_form_no, NULL );
      record->pokemon[p].sex = PP_Get( pp, ID_PARA_sex, NULL );
      record->pokemon[p].personalRandom = PP_Get( pp, ID_PARA_personal_rnd, NULL );
      record->pokemon[p].idNumber = PP_Get( pp, ID_PARA_id_no, NULL );
      record->pokemon[p].waza[0] = PP_Get( pp, ID_PARA_waza1, NULL );
      record->pokemon[p].waza[1] = PP_Get( pp, ID_PARA_waza2, NULL );
      record->pokemon[p].waza[2] = PP_Get( pp, ID_PARA_waza3, NULL );
      record->pokemon[p].waza[3] = PP_Get( pp, ID_PARA_waza4, NULL );

      if(strbuf)
      {
        PP_Get( pp, ID_PARA_nickname, strbuf );
        GFL_STR_GetStringCode( strbuf, record->pokemon[p].nickname, POKENAME_BUFSIZE );

        PP_Get( pp, ID_PARA_oyaname, strbuf );
        GFL_STR_GetStringCode( strbuf, record->pokemon[p].oyaname, OYANAME_BUFSIZE );
      }
      else
      {
        record->pokemon[p].nickname[0] = GFL_STR_GetEOMCode();
        record->pokemon[p].oyaname[0]  = GFL_STR_GetEOMCode();
      }

      p++;
    }
    
    PP_FastModeOff( pp, fastFlag );
  }

  record->year = date->year;
  record->month = date->month;
  record->day = date->day;

  if(strbuf)
  {
    GFL_STR_DeleteBuffer( strbuf );
  }
  
}

//------------------------------------------------------------------
/**
 * 殿堂入りレコード(=クリア時データ)１件追加
 *
 * @param   data    殿堂入りセーブデータポインタ
 * @param   party   殿堂入りパーティーのデータ
 * @param   date    殿堂入り日付データ（NITROのデータそのまま）
 *
 */
//------------------------------------------------------------------
void DendouData_AddRecord( DENDOU_SAVEDATA* data, const POKEPARTY* party, const RTCDate* date, HEAPID heap_id )
{
  DENDOU_RECORD*  record;
  POKEMON_PARAM*  pp;
  STRBUF*         strbuf;
  int  pokeCount, i, p;
  BOOL fastFlag;

  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);

  // 書き込み場所算出（最大値(9999)以降は無視する）
  if( data->latestNumber >= DENDOU_NUMBER_MAX )
  {
    return;
  }
  record = &data->record[ data->savePoint ];

  // レコード１件書き込み
  DendouRecord_Add( record, party, date, heap_id );

  // 書き込みポイント進める
  if(++(data->savePoint) >= DENDOU_RECORD_MAX)
  {
    data->savePoint = 0;
  }
  data->latestNumber++;

}



//=============================================================================================
/**
 * @brief 殿堂レコード(=クリア時データ)の手持ちポケモンの数を返す
 *
 * @param   record    
 *
 * @retval  u32   手持ちポケモンの数
 */
//=============================================================================================
u32  DendouRecord_GetPokemonCount( const DENDOU_RECORD* record )
{
  int i;
  for(i=0; i<TEMOTI_POKEMAX; i++)
  {
    if(record->pokemon[i].monsno == 0)
    {
      break;
    }
  }
  return i;
  
}

//=============================================================================================
/**
 * @brief   殿堂レコード(=クリア時データ)のポケモン情報を返す
 *
 * @param   record      
 * @param   poke_pos    手持ちポケモンの位置（0-5）
 * @param   poke_data   情報を書き込むポインタ
 */
//=============================================================================================
void DendouRecord_GetPokemonData( const DENDOU_RECORD* record, int poke_pos, DENDOU_POKEMON_DATA* poke_data )
{
  const DENDOU_POKEMON_DATA_INSIDE*  recPoke;
  int i;
  GF_ASSERT( record != NULL);
  GF_ASSERT( poke_pos  < 6);

  recPoke = &record->pokemon[poke_pos];

  poke_data->monsno         = recPoke->monsno;
  poke_data->level          = recPoke->level;
  poke_data->personalRandom = recPoke->personalRandom;
  poke_data->idNumber       = recPoke->idNumber;
  poke_data->formNumber     = recPoke->formNumber;
  poke_data->sex            = recPoke->sex;

  GFL_STR_SetStringCodeOrderLength( poke_data->nickname, recPoke->nickname, POKENAME_BUFSIZE );
  GFL_STR_SetStringCodeOrderLength( poke_data->oyaname,  recPoke->oyaname,  OYANAME_BUFSIZE );

  for(i=0; i<4; i++)
  {
    poke_data->waza[i] = recPoke->waza[i];
  }
  
}


//=============================================================================================
/**
 * @brief   殿堂レコード(=クリア時データ)の日付を返す
 *
 * @param   record  
 * @param   date    日付を書き込むポインタ
 */
//=============================================================================================
void DendouRecord_GetDate( const DENDOU_RECORD* record, RTCDate* date )
{
  GF_ASSERT(record != NULL);

  date->year  = record->year;
  date->month = record->month;
  date->day   = record->day;

  // 曜日は保存しないが、一応クリアしておく
  date->week = 0;
  
}



//------------------------------------------------------------------
/**
 * 殿堂入りデータ内のレコード件数を返す
 *
 * @param   data  殿堂入りデータポインタ
 *
 * @retval  u32   件数
 */
//------------------------------------------------------------------
u32 DendouData_GetRecordCount( const DENDOU_SAVEDATA* data )
{
  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);

  if( data->latestNumber >= DENDOU_RECORD_MAX )
  {
    return DENDOU_RECORD_MAX;
  }
  return data->latestNumber;
}
//------------------------------------------------------------------
/**
 * 殿堂入りデータが、何回目の殿堂入りかを返す
 *
 * @param   data    殿堂入りデータポインタ
 * @param   index   レコードインデックス（0が最新で、順に旧くなっていく）
 *
 * @retval  u32     
 */
//------------------------------------------------------------------
u32 DendouData_GetRecordNumber( const DENDOU_SAVEDATA* data, int index )
{
  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);
  GF_ASSERT(index < DENDOU_RECORD_MAX);

  return data->latestNumber - index;
}
//------------------------------------------------------------------
/**
 * 殿堂入りデータから、指定の殿堂入りレコードのポケモン数取得
 *
 * @param   data    殿堂入りデータポインタ
 * @param   index   レコードインデックス（0が最新で、順に旧くなっていく）
 *
 * @retval  u32   殿堂入りパーティーのポケモン数
 */
//------------------------------------------------------------------
u32 DendouData_GetPokemonCount( const DENDOU_SAVEDATA* data, int index )
{
  u32 i;

  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);
  GF_ASSERT(index < DENDOU_RECORD_MAX);

  index = recIndex_to_datIndex( data, index );

  return DendouRecord_GetPokemonCount( &data->record[index] );
}
//------------------------------------------------------------------
/**
 * 殿堂入りデータから指定のレコードのポケモンデータ取得
 *
 * @param   data    殿堂入りデータポインタ
 * @param   index   レコードインデックス（0が最新で、順に旧くなっていく）
 * @param   poke_pos  何体目のポケモンか（0～5）
 * @param   poke_data ポケモンデータを受け取る構造体のポインタ
 */
//------------------------------------------------------------------
void DendouData_GetPokemonData( const DENDOU_SAVEDATA* data, int index, int poke_pos, DENDOU_POKEMON_DATA* poke_data )
{
  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);

  index = recIndex_to_datIndex( data, index );

  DendouRecord_GetPokemonData( &data->record[index], poke_pos, poke_data );
}
//------------------------------------------------------------------
/**
 * 殿堂入りデータから、指定のレコードの殿堂入り日付取得
 *
 * @param   data    殿堂入りデータポインタ
 * @param   index   レコードインデックス（0が最新で、順に旧くなっていく）
 * @param   date    日付データを受け取る構造体へのポインタ
 */
//------------------------------------------------------------------
void DendouData_GetDate( const DENDOU_SAVEDATA* data, int index, RTCDate* date )
{
  GF_ASSERT(data != NULL);
  GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);
  GF_ASSERT(index < DENDOU_RECORD_MAX);

  index = recIndex_to_datIndex( data, index );
  
  DendouRecord_GetDate( &data->record[index], date );
}

















//------------------------------------------------------------------
/**
 * レコードインデックス（新しい順インデックス）をデータ配列インデックスに変換
 *
 * @param   index   
 *
 * @retval  int   
 */
//------------------------------------------------------------------
static int inline recIndex_to_datIndex( const DENDOU_SAVEDATA* data, int recIndex )
{
  int datIndex = (data->savePoint - 1 - recIndex);
  if( datIndex < 0 )
  {
    datIndex += DENDOU_RECORD_MAX;
  }
  return datIndex;
}
