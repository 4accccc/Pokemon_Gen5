//============================================================================
/**
 *  @file   zukan_search_engine.c
 *  @brief  図鑑検索エンジン
 *  @author Koji Kawada
 *  @data   2010.02.05
 *  @note   
 *  モジュール名：ZUKAN_SEARCH_ENGINE
 */
//============================================================================
//#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "savedata/zukan_savedata.h"
#include "poke_tool/poke_personal.h"

#include "zukan_search_engine.h"

// アーカイブ
#include "arc_def.h"
#include "zukan_data.naix"

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// monsno、formnoの取得
#define MONSNO(no) (no&0x03FF)
#define FORMNO(no) ((no&0xFC00)>>6)
/*
    |15 14 13 12 11 10|09 08 07 06 05 04 03 02 01 00|
    |     FORMNO      |           MONSNO            |
    |  0<=      <64   |        0<=      <1024       |
*/


// 頭文字番号
/*
# 頭文字番号
%initial_to_no_tbl =
(
  "ア"=>0,  "イ"=>1,  "ウ"=>2,  "エ"=>3,  "オ"=>4,
  "カ"=>5,  "キ"=>6,  "ク"=>7,  "ケ"=>8,  "コ"=>9,
  "サ"=>10, "シ"=>11, "ス"=>12, "セ"=>13, "ソ"=>14,
  "タ"=>15, "チ"=>16, "ツ"=>17, "テ"=>18, "ト"=>19,
  "ナ"=>20, "ニ"=>21, "ヌ"=>22, "ネ"=>23, "ノ"=>24,
  "ハ"=>25, "ヒ"=>26, "フ"=>27, "ヘ"=>28, "ホ"=>29,
  "マ"=>30, "ミ"=>31, "ム"=>32, "メ"=>33, "モ"=>34,
  "ヤ"=>35,           "ユ"=>36,           "ヨ"=>37,
  "ラ"=>38, "リ"=>39, "ル"=>40, "レ"=>41, "ロ"=>42,
  "ワ"=>43,                               "ヲ"=>44,
  "ン"=>45, 
);
$initial_to_no_unknown   = 46;  # 不明な頭文字のとき
*/


// 形番号
/*
%style_enum =
(
  "丸型"=>0,
  "脚型"=>1,
  "魚型"=>2,
  "昆虫型"=>3,
  "四足型"=>4,
  "四枚羽型"=>5,
  "集合型"=>6,
  "多足型"=>7,
  "直立胴型"=>8,
  "二足尻尾型"=>9,
  "二足人型"=>10,
  "二枚羽型"=>11,
  "腹這脚無型"=>12,
  "腕型"=>13,
);
$style_enum_empty   = 14;  # 入力されていないとき
$style_enum_unknown = 15;  # %style_enumに列挙されていない形のとき
*/


// 全ポケモン、全フォルムについて整列済みのデータ
static const int full_list_data[ZKNCOMM_LIST_SORT_ROW_MAX] =
{
	NARC_zukan_data_zkn_sort_chihou_dat,
	NARC_zukan_data_zkn_sort_aiueo_dat,
	NARC_zukan_data_zkn_sort_heavy_dat,
	NARC_zukan_data_zkn_sort_light_dat,
	NARC_zukan_data_zkn_sort_high_dat,
	NARC_zukan_data_zkn_sort_short_dat,
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static u16 Zukan_Search_Engine_CreateFullList( u8 mode, u8 row, HEAPID heap_id, u16** list );
static void Zukan_Search_Engine_CreateExData( HEAPID heap_id,
                                    u16* chihou_num,           u16** chihou_list,
                                    u16* initial_num,          u8**  initial_list,
                                    u16* style_num,            u8**  style_list,
                                    u16* next_form_pos_num,    u16** next_form_pos_list );

static u8 GetStyle( u8* style_list, u16* next_form_pos_list, u16 monsno, u16 formno );


#ifdef DEBUG_KAWADA
static void Zukan_Search_Engine_MakeTestData( ZUKAN_SAVEDATA* zkn_sv, HEAPID heap_id );
#endif


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
#define BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)       \
    {                                            \
      full_flag[i] = 1;                          \
      continue;                                  \
    }

//------------------------------------------------------------------
/**
 *  @brief          検索する 
 *
 *  @param[in]      zkn_sv      図鑑セーブデータ
 *  @param[in]      term        検索条件
 *  @param[in]      heap_id     ヒープID
 *  @param[out]     list        現在はmonsnoの列(戻り値0のときNULL)
 *
 *  @retval         listのサイズ 
 *
 *  @note           listは呼び出し元で解放して下さい。
 *
 */
//------------------------------------------------------------------
u16 ZUKAN_SEARCH_ENGINE_Search(
               const ZUKAN_SAVEDATA*      zkn_sv,
               const ZKNCOMM_LIST_SORT*   term,
               HEAPID                     heap_id,
               u16**                      list )
{
  // 全ポケモン、全フォルムについて整列済みのデータ
  u16  full_num;
  u16* full_list;
  u8*  full_flag;

  // ポケモンパーソナルにはない追加データ
  u16  chihou_num;
  u16* chihou_list;         // 地方図鑑番号の列((0<=monsno<=MONSNO_END)の(MONSNO_END+1)個のデータ)
  u16  initial_num;
  u8*  initial_list;        // 頭文字番号の列((0<=monsno<=MONSNO_END)の(MONSNO_END+1)個のデータ)
  u16  style_num;
  u8*  style_list;          // 形番号の列((0<=monsno<=MONSNO_END,TAMAGO,別フォルム)の(MONSNO_END+1+1+別フォルム数)のデータ)
  u16  next_form_pos_num;   // 次のフォルムのデータの位置を参照できる列
  u16* next_form_pos_list;  // ((0<=monsno<=MONSNO_END,TAMAGO,別フォルム)の(MONSNO_END+1+1+別フォルム数)のデータ)

  // ポケモンパーソナルデータ
  POKEMON_PERSONAL_DATA* ppd = NULL;
  
  // 検索結果のデータ
  u16  res_num;
  u16* res_list;

  // カウント
  u16 i;
  u16 j;

#ifdef DEBUG_KAWADA
  Zukan_Search_Engine_MakeTestData( (ZUKAN_SAVEDATA*)zkn_sv, heap_id );
#endif

#ifdef DEBUG_KAWADA
  OS_Printf( "ZUKAN_SEARCH_ENGINE : Search Start!\n" );
  OS_Printf( "MONSNO_END = %d\n", MONSNO_END );
#endif

  // 全ポケモン、全フォルムについて整列済みのデータを得る
  full_num = Zukan_Search_Engine_CreateFullList( term->mode, term->row, heap_id, &full_list );
  // フラグリスト
  full_flag = GFL_HEAP_AllocClearMemory( heap_id, sizeof(u8)*full_num );  // full_numが0ということはあり得ない

  // ポケモンパーソナルにはない追加データを得る
  Zukan_Search_Engine_CreateExData( heap_id,
                                    &chihou_num,           &chihou_list,
                                    &initial_num,          &initial_list,
                                    &style_num,            &style_list,
                                    &next_form_pos_num,    &next_form_pos_list );

  // 検索条件によって絞り込む
  for( i=0; i<full_num; i++ )
  {
    u16 monsno = MONSNO(full_list[i]);
    u16 formno = FORMNO(full_list[i]);

    u16 target_formno;

    GF_ASSERT_MSG( 1<=monsno && monsno<=MONSNO_END,  "ZUKAN_SEARCH_ENGINE : full_listのmonsnoが異常です。\n" );

    // monsnoのポケモンの現在着目しているフォルム番号を得る
    target_formno = 0;//GetCurrFormnoOfMonsno(zkn_sv, monsno);みたいな感じか？

    // 現在着目しているフォルムと一致していなければならない
    if( formno != target_formno )
      BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)

    // ソートが高さ、重さの場合はポケモンを捕まえていなければならない(捕まえないと高さ、重さが判明しないので)
    // 検索条件でポケタイプが設定されているときはポケモンを捕まえていなければならない(捕まえないとポケタイプが判明しないので)
    if(
           ( ZKNCOMM_LIST_SORT_ROW_WEIGHT_HI <= term->row && term->row <= ZKNCOMM_LIST_SORT_ROW_HEIGHT_LOW )
        || ( term->type1 != ZKNCOMM_LIST_SORT_NONE || term->type2 != ZKNCOMM_LIST_SORT_NONE )
    )
    {
      // 捕まえていなければならない
      if( !ZUKANSAVE_GetPokeGetFlag( zkn_sv, monsno ) )
        BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
    }

    // 地方図鑑の場合は地方図鑑に登場していなければならない
    if( term->mode == ZKNCOMM_LIST_SORT_MODE_LOCAL )
    {
      if( chihou_list[monsno] == 0 )
        BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
    }

    // 見つけていなければならない
    if( !ZUKANSAVE_GetPokeSeeFlag( zkn_sv, monsno ) )
      BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)

    // 頭文字が一致していなければならない
    if( term->name != ZKNCOMM_LIST_SORT_NONE )
    {
      if( term->name != initial_list[monsno] )
        BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
    }

    // ポケモンパーソナルデータを使用
    {
      u8 type1;
      u8 type2;
      u8 color;

      if( ppd ) POKE_PERSONAL_CloseHandle( ppd );  // 1つ前の後処理
      ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
     
      // タイプが一致していなければならない
      type1 = (u8)POKE_PERSONAL_GetParam( ppd, POKEPER_ID_type1 );
      type2 = (u8)POKE_PERSONAL_GetParam( ppd, POKEPER_ID_type2 );
      if( term->type1 != ZKNCOMM_LIST_SORT_NONE && term->type2 != ZKNCOMM_LIST_SORT_NONE )  // タイプ1に一致、タイプ2に一致
      {
        if(
            !(    ( type1 == term->type1 && type2 == term->type2 )
               || ( type1 == term->type2 && type2 == term->type1 )
            )
        )
          BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
      }
      else if( term->type1 != ZKNCOMM_LIST_SORT_NONE && term->type2 == ZKNCOMM_LIST_SORT_NONE )  // タイプ1に一致
      {
        if( !( type1 == term->type1 || type2 == term->type1 ) )
          BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
      }
      else if( term->type1 == ZKNCOMM_LIST_SORT_NONE && term->type2 != ZKNCOMM_LIST_SORT_NONE )  // タイプ2に一致
      {
        if( !( type1 == term->type2 || type2 == term->type2 ) )
          BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
      }

      // 色が一致していなければならない
      if( term->color != ZKNCOMM_LIST_SORT_NONE )
      {
        color = (u8)POKE_PERSONAL_GetParam( ppd, POKEPER_ID_color );
        if( color != term->color )
          BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
      }
    }
    
    // 形が一致していなければならない
    if( term->form != ZKNCOMM_LIST_SORT_NONE )
    {
      u8 style = GetStyle( style_list, next_form_pos_list, monsno, formno );
      if( style != term->form )
        BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
    }
  }

  if( ppd ) POKE_PERSONAL_CloseHandle( ppd );  // 最後の後処理

  // 検索結果
  {
    res_num = 0;
    for( i=0; i<full_num; i++ )
    {
      if( full_flag[i] == 0 ) res_num++;
    }
    if( res_num > 0 )
    {
      res_list = GFL_HEAP_AllocMemory( heap_id, sizeof(u16)*res_num );
      j = 0; 
      for( i=0; i<full_num; i++ )
      {
        if( full_flag[i] == 0 )
        {
          res_list[j] = MONSNO(full_list[i]);
          j++;
        }
      }
    }
    else
    {
      res_list = NULL;
    }
  }

  // 後始末
  GFL_HEAP_FreeMemory( full_list );
  GFL_HEAP_FreeMemory( full_flag );
  GFL_HEAP_FreeMemory( chihou_list );
  GFL_HEAP_FreeMemory( initial_list );
  GFL_HEAP_FreeMemory( style_list );
  GFL_HEAP_FreeMemory( next_form_pos_list );

#ifdef DEBUG_KAWADA
  OS_Printf( "ZUKAN_SEARCH_ENGINE : Search End!\n" );
#endif

#ifdef DEBUG_KAWADA
  {
    u16 i;
    OS_Printf( "ZUKAN_SEARCH_ENGINE_Search  Result\n" );
    OS_Printf( "num=%d\n", res_num );
    OS_Printf( "order  monsno\n" );
    for( i=0; i<res_num; i++ )
    {
      OS_Printf( "%3d    %3d\n", i, res_list[i] );
    }      
    OS_Printf( "End\n" );
  }
#endif

  // 返す
  *list = res_list;
  return res_num;
}

#undef BLOCK_FULL_FLAG_ON_AND_CONTINUE


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 全ポケモン、全フォルムについて整列済みのデータを得る(不要になったら呼び出し元でGFL_HEAP_FreeMemoryして下さい)
//=====================================
static u16 Zukan_Search_Engine_CreateFullList( u8 mode, u8 row, HEAPID heap_id, u16** list )
{
  u16  full_num;
  u16* full_list;
  if( mode == ZKNCOMM_LIST_SORT_MODE_ZENKOKU && row == ZKNCOMM_LIST_SORT_ROW_NUMBER )
  {
    // 全国図鑑番号 = monsno
    u16 i;
    full_list = GFL_HEAP_AllocMemory( heap_id, sizeof(u16)*MONSNO_END );
    for( i=0; i<MONSNO_END; i++ ) full_list[i] = i +1;
    full_num = MONSNO_END;
  }
  else
  {
    u32  size;
    full_list = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, full_list_data[row], FALSE, heap_id, &size );
    full_num = size / sizeof(u16);
  }
#ifdef DEBUG_KAWADA
  OS_Printf( "full_num = %d\n", full_num );
#endif
  GF_ASSERT_MSG( full_num >= MONSNO_END, "ZUKAN_SEARCH_ENGINE : 整列済みデータの個数が足りません。\n" );
  *list = full_list;
  return full_num;
}

//-------------------------------------
/// ポケモンパーソナルにはない追加データを得る(不要になったら呼び出し元でGFL_HEAP_FreeMemoryして下さい)
//=====================================
static void Zukan_Search_Engine_CreateExData( HEAPID heap_id,
                                    u16* chihou_num,           u16** chihou_list,
                                    u16* initial_num,          u8**  initial_list,
                                    u16* style_num,            u8**  style_list,
                                    u16* next_form_pos_num,    u16** next_form_pos_list )
{
  u32 size;
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_DATA, heap_id );
  
  *chihou_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_chihou_no_dat, FALSE, heap_id, &size );
  *chihou_num = size / sizeof(u16);

  *initial_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_initial_dat, FALSE, heap_id, &size );
  *initial_num = size / sizeof(u8);

  *style_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_style_dat, FALSE, heap_id, &size );
  *style_num = size / sizeof(u8);

  *next_form_pos_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_next_form_pos_dat, FALSE, heap_id, &size );
  *next_form_pos_num = size / sizeof(u16);

  GFL_ARC_CloseDataHandle( handle );

#ifdef DEBUG_KAWADA
  OS_Printf( "chihou_num = %d\n", *chihou_num );
  OS_Printf( "initial_num = %d\n", *initial_num );
  OS_Printf( "style_num = %d\n", *style_num );
  OS_Printf( "next_form_pos_num = %d\n", *next_form_pos_num );
#endif

  GF_ASSERT_MSG( *chihou_num == MONSNO_END+1, "ZUKAN_SEARCH_ENGINE : 地方図鑑番号の列の個数が異常です。\n" );
  GF_ASSERT_MSG( *initial_num == MONSNO_END+1, "ZUKAN_SEARCH_ENGINE : 頭文字番号の列の個数が異常です。\n" );
  GF_ASSERT_MSG( *style_num > MONSNO_END+1+1, "ZUKAN_SEARCH_ENGINE : 形番号の列の個数が足りません。\n" );
  GF_ASSERT_MSG( *next_form_pos_num > MONSNO_END+1+1, "ZUKAN_SEARCH_ENGINE : 次のフォルムのデータの位置を参照できる列の個数が足りません。\n" );
}

//-------------------------------------
/// 形番号を得る
//=====================================
static u8 GetStyle( u8* style_list, u16* next_form_pos_list, u16 monsno, u16 formno )
{
  u8  style;
  
  u16 formno_count = 0;
  u16 pos = monsno;

  while( formno_count != formno )
  {
    pos = next_form_pos_list[pos];
    if( pos == 0 ) break;
    formno_count++;
  }

  GF_ASSERT_MSG( pos > 0, "ZUKAN_SEARCH_ENGINE : フォルム番号が異常です。\n" );
  
  style = style_list[pos];
  return style;
}


#ifdef DEBUG_KAWADA
//-------------------------------------
/// テストデータを作成する
//=====================================
static void Zukan_Search_Engine_MakeTestData( ZUKAN_SAVEDATA* zkn_sv, HEAPID heap_id )
{
  u16 i;
  for( i=1; i<=MONSNO_END; i++ )
  {
    POKEMON_PARAM* pp = PP_Create( i, 0, 0, heap_id ); 
    ZUKANSAVE_SetPokeGet( zkn_sv, pp );
    GFL_HEAP_FreeMemory( pp );
  }
}
#endif

