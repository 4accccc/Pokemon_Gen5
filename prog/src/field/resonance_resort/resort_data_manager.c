//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_data_manager.c
 *	@brief  データマネージャー
 *	@author	Toru=Nagihashi
 *	@data		2011.02.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "system/prof_word.h"
#include "arc/arc_def.h"
#include "arc/script_message.naix"
#include "msg/script/msg_resort_scr.h"
#include "net/dwc_tool.h"
#include "savedata/resonance_resort_sv.h"
#include "resort_data_manager.h"
#include "system/gfl_use.h"
#include "field/resort_work.h"
#include "resort_debug.h"

#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_nagihashi_toru) | defined(DEBUG_ONLY_FOR_shimoyamada)
#define DEBUG_RESORT_PRINT_ON
#endif
#endif

#if defined(DEBUG_RESORT_PRINT_ON)
#define RESORT_Printf(...)  OS_TFPrintf(2,__VA_ARGS__)
#else
#define RESORT_Printf(...)  /* */
#endif



//=============================================================================
/**
 *	人物配置情報
 *	  このデータは保存されない
*/
//=============================================================================

//-------------------------------------
///	１つの場所に対する人物インデックス
//=====================================
typedef struct
{
  u8  people_index[RESORT_DATA_MANAGER_MAX];
  u8  people_buffer_type[RESORT_DATA_MANAGER_MAX];
}RESORT_DATA_MANAGER_INFO;

//-------------------------------------
///	データマネージャー
//=====================================
struct _RESORT_DATA_MANAGER
{
  RESORT_DATA_MANAGER_INFO  info[RESORT_PLACEID_MAX];
  RESORT_ABSTRUCT_MANAGER *p_abstruct;
};

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief 配置情報を作成
 *
 *	@param  const RESORT_DATA_MANAGER_INIT_PARAM *cp_param 初期化構造体
 *	@param	heapID  ヒープID
 *
 *	@return 配置情報
 */
//-----------------------------------------------------------------------------
RESORT_DATA_MANAGER * RESORT_DATA_MANAGER_Init( RESORT_ABSTRUCT_MANAGER *p_abstruct, HEAPID heapID )
{
  RESORT_DATA_MANAGER * p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_DATA_MANAGER) );
  p_wk->p_abstruct = p_abstruct;
  RESORT_DATA_MANAGER_Update( p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_Exit( RESORT_DATA_MANAGER *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  配置情報設定
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_Update( RESORT_DATA_MANAGER *p_wk )
{
  static const RESORT_DATA_BUFFER_TYPE sc_tbl[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_SECRETARY,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  };

  int i, j;
  RESORT_ABSTRUCT_ITERATOR ite;

  //クリア
  for( i = 0; i < RESORT_PLACEID_MAX; i++ )
  {
    RESORT_DATA_MANAGER_INFO *p_info = &p_wk->info[i];
    //人物IDクリア
    for( j = 0; j < RESORT_DATA_MANAGER_MAX; j++ )
    {
      p_info->people_buffer_type[j]  = RESORT_DATA_BUFFER_TYPE_NULL;
      p_info->people_index[j] = RESORT_DATA_MANAGER_INDEX_NULL;
    }
  }

  ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->p_abstruct, sc_tbl, NELEMS(sc_tbl) );
  while(1)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->p_abstruct, &ite, sc_tbl, NELEMS(sc_tbl) );
    if( p_data == NULL )
    {
      break;
    }
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      u16 peopleID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT( peopleID < RESORT_DATA_MANAGER_MAX );
      //登録されていないのを除く
      if( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX )
      {
        GF_ASSERT_MSG(p_wk->info[ placeID ].people_buffer_type[peopleID] == RESORT_DATA_BUFFER_TYPE_NULL, "assert placeID%d peopleID%d buffer%d\n", placeID, peopleID, p_wk->info[ placeID ].people_buffer_type[peopleID] );

        p_wk->info[ placeID ].people_buffer_type[ peopleID ]  = RESORT_ABSTRUCT_DATA_GetBufferType( p_data );
        p_wk->info[ placeID ].people_index[ peopleID ] = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_data );
        //RESORT_Printf( "placeID%d peopleID%d buffer_type%d bufferIndex%d\n", placeID, peopleID, p_wk->info[ placeID ].people_buffer_type[ peopleID ], p_wk->info[ placeID ].people_index[ peopleID ] );
      }
    }
  }

#ifdef PM_DEBUG
  RESORT_Printf( "配置更新▼\n" );
  for( i = RESORT_PLACEID_START; i < RESORT_PLACEID_MAX; i++ )
  {
    RESORT_DATA_MANAGER_INFO *p_info = &p_wk->info[i];
    for( j = 0; j < RESORT_DATA_MANAGER_MAX; j++ )
    {
      if( p_info->people_buffer_type[j] != RESORT_DATA_BUFFER_TYPE_NULL )
      {
        RESORT_Printf( "[%d]○t=%di=%d ", j, p_info->people_buffer_type[j], p_info->people_index[j] );
      }
      else
      {
        RESORT_Printf( "[%d]× ", j, p_info->people_buffer_type[j], p_info->people_index[j] );
      }
    }
    RESORT_Printf( "\n" );
  }
  RESORT_Printf( "―――――▲\n" );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  バッファタイプからデータタイプへの変換
 *
 *	@param	RESORT_DATA_BUFFER_TYPE type バッファタイプ
 *
 *	@return データタイプ
 */
//-----------------------------------------------------------------------------
u16 RESORT_DATA_BUFFER_TYPE_ToDataType( RESORT_DATA_BUFFER_TYPE type )
{
  static const RESORT_DATA_TYPE sc_to_data_type[] =
  {
    RESORT_DATA_TYPE_RESIDENT,
    RESORT_DATA_TYPE_SECRETARY,
    RESORT_DATA_TYPE_RECV,
    RESORT_DATA_TYPE_RECV,
    RESORT_DATA_TYPE_PROXY,
  };
  GF_ASSERT( type < NELEMS(sc_to_data_type ) );

  return sc_to_data_type[ type ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  データタイプからバッファタイプを取得
 *
 *	@param	RESORT_DATA_TYPE type   データタイプ
 *
 *	@return バッファタイプ
 */
//-----------------------------------------------------------------------------
RESORT_DATA_BUFFER_TYPE RESORT_DATA_TYPE_ToBufferType( RESORT_DATA_TYPE type )
{
  static const RESORT_DATA_BUFFER_TYPE sc_to_data_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_SECRETARY,
  };
  GF_ASSERT( type < NELEMS(sc_to_data_type ) );

  return sc_to_data_type[ type ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定の場所に存在しているかチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	placeID 場所ID
 *	@param	peopleID　人物ID  最大RESORT_DATA_MANAGER_MAX
 *
 *	@return TRUEで空  FALSEで存在
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsPlaceEmpty( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID )
{
  GF_ASSERT(peopleID<RESORT_DATA_MANAGER_MAX);
  GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );

  return cp_wk->info[ placeID ].people_index[ peopleID ] == RESORT_DATA_MANAGER_INDEX_NULL
    && cp_wk->info[ placeID ].people_buffer_type[ peopleID ] == RESORT_DATA_BUFFER_TYPE_NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief　配置データからAbstructDataを取得
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	placeID 場所ID
 *	@param	peopleID　人物ID  最大RESORT_DATA_MANAGER_MAX
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_DATA_MANAGER_GetAbstructDataByPlace( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID )
{
  RESORT_DATA_BUFFER_TYPE type;
  u16 index;

  GF_ASSERT( peopleID<RESORT_DATA_MANAGER_MAX);
  GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
 
  GF_ASSERT_MSG( !RESORT_DATA_MANAGER_IsPlaceEmpty( cp_wk, placeID, peopleID ), "空のデータをとってはいけない\n" );

  type  = cp_wk->info[ placeID ].people_buffer_type[ peopleID ];
  index = cp_wk->info[ placeID ].people_index[ peopleID ];

  if( type == RESORT_DATA_BUFFER_TYPE_NULL 
      || index == RESORT_DATA_MANAGER_INDEX_NULL )
  {
    GF_ASSERT(0);
    return NULL;
  }
  else
  {
    return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( cp_wk->p_abstruct, type, index );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief 開いている場所の領域を探す
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk　ワーク
 *	@param	placeID  場所
 *
 *	@return peopleID;
 */
//-----------------------------------------------------------------------------
u32 RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID )
{
  u16 peopleID;

  for( peopleID = 0; peopleID < RESORT_DATA_MANAGER_MAX; peopleID++ )
  {
    if( RESORT_DATA_MANAGER_IsPlaceEmpty( cp_wk, placeID, peopleID ) )
    {
      return peopleID;
    }
  }

  return RESORT_PEOPLE_ID_NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  おみやげアイテム抽選
 *
 *	@param	const RESORT_RECV_DATA *cp_wk 受信データ
 *	@param	RESORT_BINARY_PRESENT *cp_present_data  おみやげばいなり
 *	@param	RESORT_BINARY_CITY *cp_city_data  まちバイナリ
 *	@param	RESORT_CITY_DATA *cp_city_data  街データ
 *
 *	@return ０…アイテムもらえない ０…以外貰えるアイテム
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_DrawLotsPresentItem( const RESORT_ABSTRUCT_DATA *cp_wk, const RESORT_BINARY_PRESENT *cp_present_data, const RESORT_BINARY_CITY *cp_city_data, const RESORT_CITY_DATA *cp_city )
{
  //おみやげ受け取り確率
  u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_ITEM_GET, 100 );
  u32 lv = RESORT_CITY_DATA_GetData( cp_city, RESORT_CITY_DATA_PARAM_LV, NULL );
  u32 index = RESORT_BINARY_CITY_GetIndexByCityLv( cp_city_data, lv );
  u32 present_recv  = RESORT_BINARY_CITY_GetData( cp_city_data, index, RESORT_BINARY_CITY_PARAM_PRESENT_PER );

#ifdef PM_DEBUG
    if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_PRESENT) )
    {
      rnd = 0;
    }
    
    if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_PRESENT_NONE) )
    {
      return 0;
    }
#endif

  if( rnd < present_recv )
  {
    int i;

    u32 present_list  = RESORT_BINARY_CITY_GetData( cp_city_data, index, RESORT_BINARY_CITY_PARAM_PRESENT_LIST );
    u32 rnd_max = RESORT_BINARY_PRESENT_GetData( cp_present_data, present_list, RESORT_BINARY_PRESENT_PARAM_DENOMINATOR );


    //なにのおみやげを貰えるか
    u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_ITEM_KIND, rnd_max );


    u16 index = RESORT_BINARY_PRESENT_DrawLotsIndex( cp_present_data, present_list, rnd );

    if( index != RESORT_BINARY_PRESENT_PARAM_ITEM_MAX )
    {
      return RESORT_BINARY_PRESENT_GetData( cp_present_data, present_list, RESORT_BINARY_PRESENT_PARAM_ITEM_00+index*2 );
    }
  }

  return 0;
}

//=============================================================================
/**
 *					データ抽象型
*/
//=============================================================================
//-------------------------------------
///	関数型（ポインタを書き換えるため）
//=====================================
typedef void *(*InitFunction)( HEAPID );
typedef void (*ExitFunction)( void* );
typedef void (*InitInstanceFunction)( void* );
typedef BOOL (*IsEmptyFunction)( const void* );
typedef u32 (*GetDataFunction)( const void*, int, void* );
typedef void (*SetDataFunction)( void*, int, u32 );
typedef void (*InitRomFunction)( void*, const RESORT_ABSTRUCT_DATA_INIT*,HEAPID );
typedef u32 (*GetRndFunction)( const void*, RESORT_RND_ID, u32 );

//-------------------------------------
///	関数抽象ワーク
//=====================================
typedef struct
{
  InitFunction Init;
  ExitFunction Exit;
  InitInstanceFunction InitInstance;
  IsEmptyFunction IsEmpty;
  GetDataFunction GetData;
  SetDataFunction SetData;
  InitRomFunction InitData;
  GetRndFunction GetRnd;
} RESORT_ABSTRUCT_DATA_FUNCTION;
//-------------------------------------
///	メインワーク
//=====================================
struct _RESORT_ABSTRUCT_DATA
{
  void *p_data_adrs;
  const RESORT_ABSTRUCT_DATA_FUNCTION *cp_func;
  RESORT_DATA_BUFFER_TYPE buffer_type;
  u32 index;
  RESORT_DATA_TYPE type;
};

//-------------------------------------
//各データ操作関数
//=====================================
static const RESORT_ABSTRUCT_DATA_FUNCTION sc_abstruct_function[RESORT_DATA_TYPE_MAX]  =
{
  //RESORT_DATA_TYPE_RESIDENT
  {
    (InitFunction)RESORT_RECV_DATA_Init,
    (ExitFunction)RESORT_RECV_DATA_Exit,
    (InitInstanceFunction)RESORT_RECV_DATA_InitInstance,
    (IsEmptyFunction)RESORT_RECV_DATA_IsEmpty,
    (GetDataFunction)RESORT_RECV_DATA_GetData,
    (SetDataFunction)RESORT_RECV_DATA_SetData,
    (InitRomFunction)NULL,
    (GetRndFunction)RESORT_RECV_DATA_GetRnd,
  },
  //RESORT_DATA_TYPE_PROXY
  {
    (InitFunction)RESORT_PROXY_DATA_Init,
    (ExitFunction)RESORT_PROXY_DATA_Exit,
    (InitInstanceFunction)RESORT_PROXY_DATA_InitInstance,
    (IsEmptyFunction)RESORT_PROXY_DATA_IsEmpty,
    (GetDataFunction)RESORT_PROXY_DATA_GetData,
    (SetDataFunction)RESORT_PROXY_DATA_SetData,
    (InitRomFunction)NULL,
    (GetRndFunction)RESORT_PROXY_DATA_GetRnd,
  },
  //RESORT_DATA_TYPE_RECV
  {
    (InitFunction)RESORT_RECV_DATA_Init,
    (ExitFunction)RESORT_RECV_DATA_Exit,
    (InitInstanceFunction)RESORT_RECV_DATA_InitInstance,
    (IsEmptyFunction)RESORT_RECV_DATA_IsEmpty,
    (GetDataFunction)RESORT_RECV_DATA_GetData,
    (SetDataFunction)RESORT_RECV_DATA_SetData,
    (InitRomFunction)NULL,
    (GetRndFunction)RESORT_RECV_DATA_GetRnd,
  },
  //RESORT_DATA_TYPE_SECRETARY
  {
    (InitFunction)RESORT_SECRETARY_DATA_Init,
    (ExitFunction)RESORT_SECRETARY_DATA_Exit,
    (InitInstanceFunction)RESORT_SECRETARY_DATA_InitInstance,
    (IsEmptyFunction)RESORT_SECRETARY_DATA_IsEmpty,
    (GetDataFunction)RESORT_SECRETARY_DATA_GetData,
    (SetDataFunction)RESORT_SECRETARY_DATA_SetData,
    (InitRomFunction)NULL,
    (GetRndFunction)RESORT_SECRETARY_DATA_GetRnd,
  },
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  データ抽象操作  初期化
 *
 *	@param	HEAPID heapID   ヒープID
 *	@param	*p_data_adrs    データワーク NULLを指定すると内部でデータを作る
 *	@param	type            バッファータイプ
 *	@param  index           バッファのインデックス
 *
 *	@return　データ抽象操作
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_DATA_Init( HEAPID heapID, void *p_data_adrs, RESORT_DATA_BUFFER_TYPE type, u16 index )
{
  RESORT_ABSTRUCT_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_ABSTRUCT_DATA) );

  RESORT_ABSTRUCT_DATA_Setup( p_wk, p_data_adrs, type, index );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ抽象操作  破棄
 *
 *	@param	RESORT_ABSTRUCT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_DATA_Exit( RESORT_ABSTRUCT_DATA *p_wk )
{

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  一度初期化したものを再セットアップ
 *
 *	@param	RESORT_ABSTRUCT_DATA *p_wk　ワーク
 *	@param	*p_data_adrs    データワーク
 *	@param	type            タイプ
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_DATA_Setup( RESORT_ABSTRUCT_DATA *p_wk, void *p_data_adrs, RESORT_DATA_TYPE type, u32 index )
{
  GF_ASSERT( p_data_adrs != NULL );
  GF_ASSERT( type < RESORT_DATA_BUFFER_TYPE_MAX );
  p_wk->buffer_type = type;
  p_wk->index = index;
  p_wk->type = RESORT_DATA_BUFFER_TYPE_ToDataType(type);
  p_wk->cp_func   = &sc_abstruct_function[ p_wk->type ];

  if( p_data_adrs )
  {
    p_wk->p_data_adrs = p_data_adrs;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	RESORT_ABSTRUCT_DATA *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_DATA_InitInstance( RESORT_ABSTRUCT_DATA *p_wk )
{
  p_wk->cp_func->InitInstance( p_wk->p_data_adrs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  空かどうかチェック
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk   ワーク
 *
 *	@return TRUEならば空  FALSEで存在
 */
//-----------------------------------------------------------------------------
BOOL RESORT_ABSTRUCT_DATA_IsEmpty( const RESORT_ABSTRUCT_DATA *cp_wk )
{
  return cp_wk->cp_func->IsEmpty( cp_wk->p_data_adrs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *	@param	param 何を取得するか
 *	@param	*p_ret_adrs   受け取りバッファ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_GetData( const RESORT_ABSTRUCT_DATA *cp_wk, int param, void *p_ret_adrs )
{
  GF_ASSERT( cp_wk );
  return cp_wk->cp_func->GetData( cp_wk->p_data_adrs, param, p_ret_adrs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを設定
 *
 *	@param	RESORT_ABSTRUCT_DATA *p_wk  ワーク
 *	@param	param 何を設定するか
 *	@param	data  データ
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_DATA_SetData( RESORT_ABSTRUCT_DATA *p_wk, int param, u32 data )
{
  GF_ASSERT( p_wk );
  p_wk->cp_func->SetData( p_wk->p_data_adrs, param, data );
}
//----------------------------------------------------------------------------
/** 
 *	@brief  生データを取得
 *
 *	@param	RESORT_ABSTRUCT_DATA *p_wk  ワーク
 *
 *	@return 生データ
 */
//-----------------------------------------------------------------------------
void * RESORT_ABSTRUCT_DATA_GetDataAdrs( RESORT_ABSTRUCT_DATA *p_wk )
{
  return p_wk->p_data_adrs;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物タイプを取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk   ワーク
 *
 *	@return 人物タイプ
 */
//-----------------------------------------------------------------------------
RESORT_DATA_TYPE RESORT_ABSTRUCT_DATA_GetPeopleType( const RESORT_ABSTRUCT_DATA *cp_wk )
{
  return cp_wk->type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バッファタイプを取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *
 *	@return バッファタイプ
 */
//-----------------------------------------------------------------------------
RESORT_DATA_BUFFER_TYPE RESORT_ABSTRUCT_DATA_GetBufferType( const RESORT_ABSTRUCT_DATA *cp_wk )
{
  return cp_wk->buffer_type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファのインデックスを取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_GetBufferIndex( const RESORT_ABSTRUCT_DATA *cp_wk )
{
  return cp_wk->index;
}
//----------------------------------------------------------------------------
/**
 *	@brief  乱数を取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *	@param	id  乱数使用ID
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_GetRnd( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_RND_ID id, u32 range )
{
  return cp_wk->cp_func->GetRnd( cp_wk->p_data_adrs, id, range );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ２種類の種を足しあわせた乱数を取得
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk1  ワーク１
 *	@param	RESORT_ABSTRUCT_DATA *cp_wk2  ワーク２
 *	@param	id  乱数使用ID
 *	@param	range 範囲
 *
 *	@return 乱数
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_GetRndDouble( const RESORT_ABSTRUCT_DATA *cp_wk1, const RESORT_ABSTRUCT_DATA *cp_wk2, RESORT_RND_ID id, u32 range )
{
  u32 seed1 = RESORT_ABSTRUCT_DATA_GetData( cp_wk1, RESORT_DATA_PARAM_RND_SEED, NULL );
  u32 seed2 = RESORT_ABSTRUCT_DATA_GetData( cp_wk2, RESORT_DATA_PARAM_RND_SEED, NULL );
  return RESORT_DATA_GetRnd( seed1+seed2, id, range );
}

//=============================================================================
/**
 *					データ抽象型マネージャ
*/
//=============================================================================
//-------------------------------------
/// データ番号
//=====================================
enum
{
  DATA_INDEX_RESIDENT_START = 0,
  DATA_INDEX_RESIDENT_END   = DATA_INDEX_RESIDENT_START + RESORT_RESIDENT_DATA_NUM,
  DATA_INDEX_SECRETARY_START = DATA_INDEX_RESIDENT_END,
  DATA_INDEX_SECRETARY_END  = DATA_INDEX_SECRETARY_START + RESORT_SECRETARY_DATA_NUM,
  DATA_INDEX_TOURIST_START  = DATA_INDEX_SECRETARY_END,
  DATA_INDEX_TOURIST_END    = DATA_INDEX_TOURIST_START + RESONANCE_RESORT_SV_RECV_BUFF_NUM,
  DATA_INDEX_TOURIST_EX_START = DATA_INDEX_TOURIST_END,
  DATA_INDEX_TOURIST_EX_END = DATA_INDEX_TOURIST_EX_START + RESONANCE_RESORT_BUFF_RECV_EX_NUM,
  DATA_INDEX_PROXY_START    = DATA_INDEX_TOURIST_EX_END,
  DATA_INDEX_PROXY_END      = DATA_INDEX_PROXY_START + RESONANCE_RESORT_SV_PROXY_BUFF_NUM,
  DATA_INDEX_MAX            = DATA_INDEX_PROXY_END,
};

//-------------------------------------
///	データ抽象型
//=====================================
struct _RESORT_ABSTRUCT_MANAGER
{
  RESORT_ABSTRUCT_DATA  *p_data[ DATA_INDEX_MAX ];
};

//-------------------------------------
///	データ
//=====================================
//タイプごとの位置
static const struct
{
  u16 start;
  u16 end;
  u32 len;
} sc_type[] =
{
  {
    DATA_INDEX_RESIDENT_START,
    DATA_INDEX_RESIDENT_END,
    RESORT_RESIDENT_DATA_NUM,
  },
  {
    DATA_INDEX_SECRETARY_START,
    DATA_INDEX_SECRETARY_END,
    RESORT_SECRETARY_DATA_NUM,
  },
  {
    DATA_INDEX_TOURIST_START,
    DATA_INDEX_TOURIST_END,
    RESONANCE_RESORT_SV_RECV_BUFF_NUM,
  },
  {
    DATA_INDEX_TOURIST_EX_START,
    DATA_INDEX_TOURIST_EX_END,
    RESONANCE_RESORT_BUFF_RECV_EX_NUM,
  },
  {
    DATA_INDEX_PROXY_START,
    DATA_INDEX_PROXY_END,
    RESONANCE_RESORT_SV_PROXY_BUFF_NUM,
  },
};

SDK_COMPILER_ASSERT( RESORT_DATA_BUFFER_TYPE_MAX == NELEMS(sc_type) );

//----------------------------------------------------------------------------
/**
 *	@brief  マネージャー作成
 *
 *	@param	const RESORT_ALL_DATA_PARAM *cp_param 引数
 *	@param	heapID  ヒープID
 *
 *	@return マネージャー
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_MANAGER * RESORT_ABSTRUCT_MANAGER_Init( const RESORT_ALL_DATA_PARAM *cp_param, HEAPID heapID )
{
  int i;

  RESORT_ABSTRUCT_MANAGER *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_ABSTRUCT_MANAGER) );

  for( i = DATA_INDEX_RESIDENT_START; i < DATA_INDEX_RESIDENT_END; i++ )
  {
    u16 index = i-DATA_INDEX_RESIDENT_START;
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( cp_param->p_all_people, index );
    p_wk->p_data[ i ] = RESORT_ABSTRUCT_DATA_Init( heapID, p_data, RESORT_DATA_BUFFER_TYPE_RESIDENT, index );
  }
  for( i = DATA_INDEX_SECRETARY_START; i < DATA_INDEX_SECRETARY_END; i++ )
  {
    u16 index = i-DATA_INDEX_SECRETARY_START;
    RESORT_SECRETARY_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetSecretary( cp_param->p_all_people, index );
    p_wk->p_data[ i ] = RESORT_ABSTRUCT_DATA_Init( heapID, p_data, RESORT_DATA_BUFFER_TYPE_SECRETARY, index );
  }
  for( i = DATA_INDEX_TOURIST_START; i < DATA_INDEX_TOURIST_END; i++ )
  {
    u16 index = i-DATA_INDEX_TOURIST_START;
    RESORT_RECV_DATA *p_data = RESORT_RECV_BUFF_GetData(cp_param->p_recv_sv_buff, index );
    p_wk->p_data[ i ] = RESORT_ABSTRUCT_DATA_Init( heapID, p_data, RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF, index );
  }
  if( cp_param->p_recv_tmp_buff )
  {
    for( i = DATA_INDEX_TOURIST_EX_START; i < DATA_INDEX_TOURIST_EX_END; i++ )
    {
      u16 index = i-DATA_INDEX_TOURIST_EX_START;
      RESORT_RECV_DATA *p_data = RESORT_RECV_BUFF_GetData(cp_param->p_recv_tmp_buff, index );
      p_wk->p_data[ i ] = RESORT_ABSTRUCT_DATA_Init( heapID, p_data, RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, index );
    }
  }
  for( i = DATA_INDEX_PROXY_START; i < DATA_INDEX_PROXY_END; i++ )
  {
    u16 index = i-DATA_INDEX_PROXY_START;
    RESORT_PROXY_DATA *p_data = RESORT_PROXY_BUFF_GetData(cp_param->p_proxy_buff, index );
    p_wk->p_data[ i ] = RESORT_ABSTRUCT_DATA_Init( heapID, p_data, RESORT_DATA_BUFFER_TYPE_PROXY_BUFF, index );
  } 

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  マネージャー破棄
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_MANAGER_Exit( RESORT_ABSTRUCT_MANAGER * p_wk )
{
  int i;
  for( i = 0; i < DATA_INDEX_MAX; i++ )
  {
    if( p_wk->p_data[i] )
    {
      RESORT_ABSTRUCT_DATA_Exit( p_wk->p_data[i] );
    }
  }
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  イテレーター開始
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	RESORT_DATA_BUFFER_TYPE *cp_tbl サーチするタイプの配列
 *	@param	tbl_max   上記の配列要素数
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_ITERATOR RESORT_ABSTRUCT_MANAGER_BeginIterator( RESORT_ABSTRUCT_MANAGER * p_wk, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max )
{
  RESORT_ABSTRUCT_ITERATOR ite;
  GFL_STD_MemClear( &ite, sizeof( RESORT_ABSTRUCT_ITERATOR) );

  ite.tbl_index = 0;
  ite.current = sc_type[ cp_tbl[ 0 ] ].start;

  return ite;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イテレーター次
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	RESORT_DATA_BUFFER_TYPE *cp_tbl サーチするタイプの配列
 *	@param	tbl_max   上記の配列要素数
 *
 *	@return データ  NULLで終了
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_NextIterator( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_ABSTRUCT_ITERATOR *p_ite, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max )
{
  if( p_ite->tbl_index < tbl_max )
  {
    if( sc_type[ cp_tbl[p_ite->tbl_index] ].start <= p_ite->current && p_ite->current < sc_type[ cp_tbl[p_ite->tbl_index] ].end )
    {
      return p_wk->p_data[ p_ite->current++ ];
    }
    else
    {
      p_ite->tbl_index++;
      if( p_ite->tbl_index < tbl_max )
      {
        p_ite->current = sc_type[ cp_tbl[p_ite->tbl_index] ].start;
        return p_wk->p_data[ p_ite->current++ ];
      }
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ取得  アドレス版
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	void *cp_data_adrs              一致するアドレス
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_ABSTRUCT_MANAGER * p_wk, const void *cp_data_adrs )
{
  int i;
  for( i = 0; i < DATA_INDEX_MAX; i++ )
  {
    if( p_wk->p_data[i] )
    {
      if( cp_data_adrs == RESORT_ABSTRUCT_DATA_GetDataAdrs( p_wk->p_data[i] ) )
      {
        return p_wk->p_data[i];
      }
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ取得  インデックス版
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	type  どこのバッファか
 *	@param	index インデックス
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByIndex( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_DATA_BUFFER_TYPE type, u16 index )
{
  GF_ASSERT( type < NELEMS(sc_type) );
  GF_ASSERT( index < sc_type[type].len );
  return p_wk->p_data[ sc_type[type].start + index ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ取得  人物タイプ版
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	type    人物タイプ
 *	@param	index   インデックス
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetDataByPeopleIndex( RESORT_ABSTRUCT_MANAGER * p_wk, RESORT_DATA_TYPE type, u16 index )
{
  static const RESORT_DATA_BUFFER_TYPE sc_to_buffer_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    //RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    RESORT_DATA_BUFFER_TYPE_SECRETARY,
  };

  GF_ASSERT( type < NELEMS(sc_to_buffer_type) );

  if( type == RESORT_DATA_TYPE_RECV )
  {
    if( index < RESONANCE_RESORT_SV_RECV_BUFF_NUM )
    {
      type  = RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF;
    }
    else
    {
      type  = RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF;
      index -= RESONANCE_RESORT_SV_RECV_BUFF_NUM;
    }
  }
  else
  {
    type  = sc_to_buffer_type[ type ];
  }

  return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk, type, index );
}
//----------------------------------------------------------------------------
/**
 *	@brief  空のデータ取得
 *
 *	@param	RESORT_ABSTRUCT_MANAGER * p_wk  ワーク
 *	@param	type  どこのバッファか
 *
 *	@return 空データ
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_GetEmptyData( RESORT_ABSTRUCT_MANAGER * p_wk,RESORT_DATA_BUFFER_TYPE type )
{
  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk, &type, 1 );
  while(1)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk, &ite, &type, 1 );
    if( p_data == NULL )
    {
      return NULL;
    }
    if( RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      return p_data;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  不正文字  名前の不正文字は強制的に置きかける
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *
 *	@retval TRUE使用可能  FALSE使用不可
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_DATA_ModifyProfWord( RESORT_ABSTRUCT_DATA *p_wk, HEAPID heapID )
{
  {
    STRCODE src_name[PERSON_NAME_SIZE+EOM_SIZE];
    STRCODE dst_name[PERSON_NAME_SIZE+EOM_SIZE];

    PROF_WORD_WORK *p_prof = PROF_WORD_AllocWork( heapID );

    DWC_TOOL_SetBadNickName( dst_name, PERSON_NAME_SIZE+EOM_SIZE, heapID );

    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_wk) )
    {
      RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_NAME, src_name );
      if( PROF_WORD_CheckProfanityWordCode( p_prof, src_name, heapID ) )
      {
        RESORT_ABSTRUCT_DATA_SetData( p_wk, RESORT_DATA_PARAM_NAME, (u32)dst_name );
      }
    }

    PROF_WORD_FreeWork( p_prof );
  }
}
