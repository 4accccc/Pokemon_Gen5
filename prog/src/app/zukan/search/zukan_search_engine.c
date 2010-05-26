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
#define FORMNO(no) ((no&0xFC00)>>10)
#define MONSNO_FORMNO(monsno,formno)  (((formno)<<10)&(monsno))
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


// 分割検索エンジンのシーケンス
typedef enum
{
  ZKN_SCH_EGN_DIV_SEQ_OPEN_HANDLE,
  ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_0,
  ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_1,
  ZKN_SCH_EGN_DIV_SEQ_CHECK_POKE,
  ZKN_SCH_EGN_DIV_SEQ_FINISH,
  ZKN_SCH_EGN_DIV_SEQ_END,
}
ZKN_SCH_EGN_DIV_SEQ;

// 1フレームで処理するポケモンの匹数
#define ZKN_SCH_EGN_DIV_FRAME_NUM_FOR_POKE (110)  // zknsearch_seq.c MainSeq_StartSort でポケモンの処理に使えそうなフレーム数(だいたい)
#define ZKN_SCH_EGN_DIV_POKE_TOTAL_NUM     (750)  // フォルム数も加えた総数(だいたい)
#define ZKN_SCH_EGN_DIV_POKE_NUM_PER_FRAME (7)    // ZKN_SCH_EGN_DIV_POKE_TOTAL_NUM / ZKN_SCH_EGN_DIV_FRAME_NUM_FOR_POKE より大きい値


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 分割検索エンジンワーク
//=====================================
struct  _ZKN_SCH_EGN_DIV_WORK
{
  const ZUKAN_SAVEDATA*      zkn_sv; 
  const ZKNCOMM_LIST_SORT*   term;    // 呼び出し元にある実体へのポインタを保持しているだけ
 
  u16                        seq;
  ARCHANDLE*                 handle;
  u16                        search_idx;

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
  POKEMON_PERSONAL_DATA* ppd;

  // 検索結果のデータ
  u16  res_num;
  u16* res_list;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static u16 Zukan_Search_Engine_CreateFullList( u8 mode, u8 row, HEAPID heap_id, u16** list );
static u16 Zukan_Search_Engine_CreateFullListHandle(
    ARCHANDLE* handle,
    u8 mode, u8 row, HEAPID heap_id, u16** list );
static void Zukan_Search_Engine_CreateExData( HEAPID heap_id,
                                    u16* chihou_num,           u16** chihou_list,
                                    u16* initial_num,          u8**  initial_list,
                                    u16* style_num,            u8**  style_list,
                                    u16* next_form_pos_num,    u16** next_form_pos_list );
static void Zukan_Search_Engine_CreateExDataHandle(
                                    ARCHANDLE* handle,
                                    HEAPID heap_id,
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

    // 見つけていなければならない
    if( !ZUKANSAVE_GetPokeSeeFlag( zkn_sv, monsno ) )
      BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)

    // monsnoのポケモンの現在着目しているフォルム番号を得る
    {
      u32 sex;
      BOOL rare;
      u32 form;
      ZUKANSAVE_GetDrawData( (ZUKAN_SAVEDATA*)zkn_sv, monsno, &sex, &rare, &form, heap_id );
      target_formno = (u16)form;
    }

    // フォルム違いを考慮しない整列済みのデータの場合
    if( full_num == MONSNO_END )
    {
      formno = target_formno;  // フォルムが一致するようにしておく
    }

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
      if( chihou_list[monsno] == POKEPER_CHIHOU_NO_NONE )
        BLOCK_FULL_FLAG_ON_AND_CONTINUE(i)
    }

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


//------------------------------------------------------------------
/**
 *  @brief          分割検索エンジンワークの生成
 *
 *  @param[in]      zkn_sv      図鑑セーブデータ
 *  @param[in]      term        検索条件            // 呼び出し元はこれを削除しないこと
 *  @param[in]      heap_id     ヒープID
 *
 *  @retval         初期化済みの図鑑検索エンジンワーク
 *
 *  @note           
 *
 */
//------------------------------------------------------------------
ZKN_SCH_EGN_DIV_WORK*  ZUKAN_SEARCH_ENGINE_DivInit(
               const ZUKAN_SAVEDATA*      zkn_sv,
               const ZKNCOMM_LIST_SORT*   term,      // 呼び出し元はこれを削除しないこと
               HEAPID                     heap_id
)
{
  ZKN_SCH_EGN_DIV_WORK* work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(ZKN_SCH_EGN_DIV_WORK) );
  
  work->zkn_sv = zkn_sv;
  work->term   = term;

  work->seq        = ZKN_SCH_EGN_DIV_SEQ_OPEN_HANDLE;
  work->handle     = NULL;
  work->search_idx = 0;

  // ポケモンパーソナルデータ
  work->ppd = NULL;

  // 検索結果のデータ
  work->res_num  = 0;
  work->res_list = NULL;

  return work;
}

//------------------------------------------------------------------
/**
 *  @brief          分割検索エンジンワークの破棄
 *
 *  @param[in]      work        図鑑検索エンジンワーク
 *
 *  @retval         なし
 *
 *  @note           
 *
 */
//------------------------------------------------------------------
void                   ZUKAN_SEARCH_ENGINE_DivExit(
               ZKN_SCH_EGN_DIV_WORK*      work
)
{
  GFL_HEAP_FreeMemory( work );
}


#define BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)       \
    {                                                 \
      work->full_flag[i] = 1;                         \
      continue;                                       \
    }

//------------------------------------------------------------------
/**
 *  @brief          分割検索する(1フレームに少しずつ検索する)
 *
 *  @param[in]      work      図鑑検索エンジンワーク
 *  @param[in]      heap_id   ヒープID
 *  @param[out]     num       見付けた匹数                 // 戻り値がZKN_SCH_EGN_DIV_STATE_FINISHになったら有効
 *  @param[out]     list      monsnoの列(num=0のときNULL)  // 戻り値がZKN_SCH_EGN_DIV_STATE_FINISHになったら有効
 *
 *  @retval         分割検索エンジンの状態
 *
 *  @note           
 *
 */
//------------------------------------------------------------------
ZKN_SCH_EGN_DIV_STATE  ZUKAN_SEARCH_ENGINE_DivSearch(  // 1フレームに1回呼び出す
               ZKN_SCH_EGN_DIV_WORK*      work,
               HEAPID                     heap_id,
               u16*                       num,      // 戻り値がZKN_SCH_EGN_DIV_STATE_FINISHになったら有効
               u16**                      list      // 戻り値がZKN_SCH_EGN_DIV_STATE_FINISHになったら有効
)
{
  switch(work->seq)
  {
  case ZKN_SCH_EGN_DIV_SEQ_OPEN_HANDLE:
    {
#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_OPEN_HANDLE\n" );
#endif

      work->handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_DATA, heap_id );

      work->seq = ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_0;
    }
    break;
  case ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_0:
    {
#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_0\n" );
#endif

      // 全ポケモン、全フォルムについて整列済みのデータを得る
      work->full_num = Zukan_Search_Engine_CreateFullListHandle( work->handle, work->term->mode, work->term->row, heap_id, &(work->full_list) );
      // フラグリスト
      work->full_flag = GFL_HEAP_AllocClearMemory( heap_id, sizeof(u8)*work->full_num );  // full_numが0ということはあり得ない

      work->seq = ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_1;
    }
    break;
  case ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_1:
    {
#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_LOAD_FILE_1\n" );
#endif

      // ポケモンパーソナルにはない追加データを得る
      Zukan_Search_Engine_CreateExDataHandle(
                                        work->handle,
                                        heap_id,
                                        &(work->chihou_num),           &(work->chihou_list),
                                        &(work->initial_num),          &(work->initial_list),
                                        &(work->style_num),            &(work->style_list),
                                        &(work->next_form_pos_num),    &(work->next_form_pos_list) );

      work->seq = ZKN_SCH_EGN_DIV_SEQ_CHECK_POKE;
    }
    break;
  case ZKN_SCH_EGN_DIV_SEQ_CHECK_POKE:
    {
      // カウント
      u16 i;

      u16 start_idx;  // start_idx<= <end_idx
      u16 end_idx;

#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_CHECK_POKE %3d\n", work->search_idx );
#endif

      if( work->search_idx >= work->full_num )
      {
        work->seq = ZKN_SCH_EGN_DIV_SEQ_FINISH;
        break;
      }

      start_idx = work->search_idx;
      end_idx   = start_idx + ZKN_SCH_EGN_DIV_POKE_NUM_PER_FRAME;
      if( end_idx > work->full_num )
      {
        end_idx = work->full_num;
      }
      work->search_idx = end_idx;  // 次の検索開始位置を設定しておく

      // 検索条件によって絞り込む
      for( i=start_idx; i<end_idx; i++ )
      {
        u16 monsno = MONSNO(work->full_list[i]);
        u16 formno = FORMNO(work->full_list[i]);
    
        u16 target_formno;
    
        GF_ASSERT_MSG( 1<=monsno && monsno<=MONSNO_END,  "ZUKAN_SEARCH_ENGINE : full_listのmonsnoが異常です。\n" );
   
#ifdef DEBUG_KAWADA
        if( monsno == 487 )
        {
          OS_Printf( "monsno=%d, formno=%d\n", monsno, formno );
        }
#endif

        // 見つけていなければならない
        if( !ZUKANSAVE_GetPokeSeeFlag( work->zkn_sv, monsno ) )
          BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
    
        // monsnoのポケモンの現在着目しているフォルム番号を得る
        {
          u32 sex;
          BOOL rare;
          u32 form;
          ZUKANSAVE_GetDrawData( (ZUKAN_SAVEDATA*)work->zkn_sv, monsno, &sex, &rare, &form, heap_id );
          target_formno = (u16)form;
        }
   
        // フォルム違いを考慮しない整列済みのデータの場合
        if( work->full_num == MONSNO_END )
        {
          formno = target_formno;  // フォルムが一致するようにしておく
        }

        // 現在着目しているフォルムと一致していなければならない
        if( formno != target_formno )
          BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
    
        // ソートが高さ、重さの場合はポケモンを捕まえていなければならない(捕まえないと高さ、重さが判明しないので)
        // 検索条件でポケタイプが設定されているときはポケモンを捕まえていなければならない(捕まえないとポケタイプが判明しないので)
        if(
               ( ZKNCOMM_LIST_SORT_ROW_WEIGHT_HI <= work->term->row && work->term->row <= ZKNCOMM_LIST_SORT_ROW_HEIGHT_LOW )
            || ( work->term->type1 != ZKNCOMM_LIST_SORT_NONE || work->term->type2 != ZKNCOMM_LIST_SORT_NONE )
        )
        {
          // 捕まえていなければならない
          if( !ZUKANSAVE_GetPokeGetFlag( work->zkn_sv, monsno ) )
            BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
        }
    
        // 地方図鑑の場合は地方図鑑に登場していなければならない
        if( work->term->mode == ZKNCOMM_LIST_SORT_MODE_LOCAL )
        {
          if( work->chihou_list[monsno] == POKEPER_CHIHOU_NO_NONE )
            BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
        }
    
        // 頭文字が一致していなければならない
        if( work->term->name != ZKNCOMM_LIST_SORT_NONE )
        {
          if( work->term->name != work->initial_list[monsno] )
            BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
        }
    
        // ポケモンパーソナルデータを使用
        {
          u8 type1;
          u8 type2;
          u8 color;
    
          if( work->ppd ) POKE_PERSONAL_CloseHandle( work->ppd );  // 1つ前の後処理
          work->ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
         
          // タイプが一致していなければならない
          type1 = (u8)POKE_PERSONAL_GetParam( work->ppd, POKEPER_ID_type1 );
          type2 = (u8)POKE_PERSONAL_GetParam( work->ppd, POKEPER_ID_type2 );
          if( work->term->type1 != ZKNCOMM_LIST_SORT_NONE && work->term->type2 != ZKNCOMM_LIST_SORT_NONE )  // タイプ1に一致、タイプ2に一致
          {
            if(
                !(    ( type1 == work->term->type1 && type2 == work->term->type2 )
                   || ( type1 == work->term->type2 && type2 == work->term->type1 )
                )
            )
              BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
          }
          else if( work->term->type1 != ZKNCOMM_LIST_SORT_NONE && work->term->type2 == ZKNCOMM_LIST_SORT_NONE )  // タイプ1に一致
          {
            if( !( type1 == work->term->type1 || type2 == work->term->type1 ) )
              BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
          }
          else if( work->term->type1 == ZKNCOMM_LIST_SORT_NONE && work->term->type2 != ZKNCOMM_LIST_SORT_NONE )  // タイプ2に一致
          {
            if( !( type1 == work->term->type2 || type2 == work->term->type2 ) )
              BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
          }
    
          // 色が一致していなければならない
          if( work->term->color != ZKNCOMM_LIST_SORT_NONE )
          {
            color = (u8)POKE_PERSONAL_GetParam( work->ppd, POKEPER_ID_color );
            if( color != work->term->color )
              BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
          }
        }
        
        // 形が一致していなければならない
        if( work->term->form != ZKNCOMM_LIST_SORT_NONE )
        {
          u8 style = GetStyle( work->style_list, work->next_form_pos_list, monsno, formno );
          if( style != work->term->form )
            BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE(i)
        }
      }
    }
    break;
  case ZKN_SCH_EGN_DIV_SEQ_FINISH:
    {
      // 検索結果のデータ
      u16  res_num;
      u16* res_list;
    
      // カウント
      u16 i;
      u16 j;

#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_FINISH\n" );
#endif

      if( work->ppd ) POKE_PERSONAL_CloseHandle( work->ppd );  // 最後の後処理
    
      // 検索結果
      {
        res_num = 0;
        for( i=0; i<work->full_num; i++ )
        {
          if( work->full_flag[i] == 0 ) res_num++;
        }
        if( res_num > 0 )
        {
          res_list = GFL_HEAP_AllocMemory( heap_id, sizeof(u16)*res_num );
          j = 0; 
          for( i=0; i<work->full_num; i++ )
          {
            if( work->full_flag[i] == 0 )
            {
              res_list[j] = MONSNO(work->full_list[i]);
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
      GFL_HEAP_FreeMemory( work->full_list );
      GFL_HEAP_FreeMemory( work->full_flag );
      GFL_HEAP_FreeMemory( work->chihou_list );
      GFL_HEAP_FreeMemory( work->initial_list );
      GFL_HEAP_FreeMemory( work->style_list );
      GFL_HEAP_FreeMemory( work->next_form_pos_list );
    
      // 返す
      work->res_list = res_list;
      work->res_num  = res_num;

      GFL_ARC_CloseDataHandle( work->handle );
      
      work->seq = ZKN_SCH_EGN_DIV_SEQ_END;
    }
    break;
  case ZKN_SCH_EGN_DIV_SEQ_END:
  default:
    {
#ifdef DEBUG_KAWADA
      OS_Printf( "ZKN_SCH_EGN_DIV_SEQ_END\n" );
#endif

      *list = work->res_list;
      *num  = work->res_num;

      return ZKN_SCH_EGN_DIV_STATE_FINISH;
    }
    break;
  }

  return ZKN_SCH_EGN_DIV_STATE_CONTINUE;
}

#undef BLOCK_WORK_FULL_FLAG_ON_AND_CONTINUE


//------------------------------------------------------------------
/**
 *  @brief          全国/地方図鑑の番号順リストを得る 
 *
 *  @param[in]      mode        検索条件
 *  @param[in]      heap_id     ヒープID
 *  @param[out]     list        monsnoの列
 *
 *  @retval         listのサイズ 
 *
 *  @note           listは呼び出し元で解放して下さい。
 *  @note           見た捕まえたに関係なく、その図鑑に登場する全ポケモンをリストにします。
 *
 */
//------------------------------------------------------------------
u16 ZUKAN_GetNumberRow(
               u8       mode,  // ZKNCOMM_LIST_SORT_MODE_ZENKOKU / ZKNCOMM_LIST_SORT_MODE_LOCAL
               HEAPID   heap_id,
               u16**    list )
{
  
  if( mode == ZKNCOMM_LIST_SORT_MODE_ZENKOKU )
  {
    // 全国図鑑番号 = monsno
    u16  full_num;
    u16* full_list;
    u16  i;
    
    full_list = GFL_HEAP_AllocMemory( heap_id, sizeof(u16)*MONSNO_END );
    for( i=0; i<MONSNO_END; i++ ) full_list[i] = i +1;
    full_num = MONSNO_END;
  
#ifdef DEBUG_KAWADA
    {
      u16 i;
      OS_Printf( "ZUKAN_GetNumberRow  Result\n" );
      OS_Printf( "num=%d\n", full_num );
      OS_Printf( "order  monsno\n" );
      for( i=0; i<full_num; i++ )
      {
        OS_Printf( "%3d    %3d\n", i, full_list[i] );
      }      
      OS_Printf( "End\n" );
    }
#endif

    *list = full_list;
    return full_num;
  }
  else
  {
    u16* chihou_appear_list;
    u16  chihou_appear_count;

    u16  chihou_appear_num;
    u16* zenkoku_to_chihou_list = POKE_PERSONAL_GetZenkokuToChihouArray( heap_id, &chihou_appear_num );

    u16  full_num;
    u16* full_list;
    u32  size;
    u16  i; 

    full_list = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, NARC_zukan_data_zkn_sort_chihou_dat, FALSE, heap_id, &size );
    full_num = size / sizeof(u16);

    chihou_appear_list = GFL_HEAP_AllocMemory( heap_id, sizeof(u16)*chihou_appear_num );

    chihou_appear_count = 0;
    for( i=0; i<full_num; i++ )
    {
      u16  monsno = full_list[i];
      if( zenkoku_to_chihou_list[monsno] != POKEPER_CHIHOU_NO_NONE )
      {
        chihou_appear_list[chihou_appear_count] = monsno;
        chihou_appear_count++;
      }
    }

    GFL_HEAP_FreeMemory( full_list );
    GFL_HEAP_FreeMemory( zenkoku_to_chihou_list );

#ifdef DEBUG_KAWADA
    {
      u16 i;
      OS_Printf( "ZUKAN_GetNumberRow  Result\n" );
      OS_Printf( "num=%d\n", chihou_appear_num );
      OS_Printf( "order  monsno\n" );
      for( i=0; i<chihou_appear_num; i++ )
      {
        OS_Printf( "%3d    %3d\n", i, chihou_appear_list[i] );
      }      
      OS_Printf( "End\n" );
    }
#endif

    *list = chihou_appear_list;
    return chihou_appear_num;
  }
}


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
/// 全ポケモン、全フォルムについて整列済みのデータを得る(不要になったら呼び出し元でGFL_HEAP_FreeMemoryして下さい)
//=====================================
static u16 Zukan_Search_Engine_CreateFullListHandle(
    ARCHANDLE* handle,
    u8 mode, u8 row, HEAPID heap_id, u16** list )
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
    full_list = GFL_ARCHDL_UTIL_LoadEx( handle, full_list_data[row], FALSE, heap_id, &size );
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
  
  *chihou_list = POKE_PERSONAL_GetZenkokuToChihouArray( heap_id, NULL );
  *chihou_num = MONSNO_END+1;

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
/// ポケモンパーソナルにはない追加データを得る(不要になったら呼び出し元でGFL_HEAP_FreeMemoryして下さい)
//=====================================
static void Zukan_Search_Engine_CreateExDataHandle(
                                    ARCHANDLE* handle,
                                    HEAPID heap_id,
                                    u16* chihou_num,           u16** chihou_list,
                                    u16* initial_num,          u8**  initial_list,
                                    u16* style_num,            u8**  style_list,
                                    u16* next_form_pos_num,    u16** next_form_pos_list )
{
  u32 size;
  
  *chihou_list = POKE_PERSONAL_GetZenkokuToChihouArray( heap_id, NULL );
  *chihou_num = MONSNO_END+1;

  *initial_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_initial_dat, FALSE, heap_id, &size );
  *initial_num = size / sizeof(u8);

  *style_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_style_dat, FALSE, heap_id, &size );
  *style_num = size / sizeof(u8);

  *next_form_pos_list = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_data_zkn_next_form_pos_dat, FALSE, heap_id, &size );
  *next_form_pos_num = size / sizeof(u16);

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

