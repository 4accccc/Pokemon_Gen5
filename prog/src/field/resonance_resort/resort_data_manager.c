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
#include "net/dwc_tool.h"
#include "savedata/resonance_resort_sv.h"
#include "resort_data_manager.h"
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

  RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->p_abstruct, sc_tbl, NELEMS(sc_tbl) );
  while(1)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->p_abstruct, sc_tbl, NELEMS(sc_tbl) );
    if( p_data == NULL )
    {
      break;
    }
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      u16 peopleID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

      //登録されていないのを除く
      if( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX )
      {
        GF_ASSERT_MSG(p_wk->info[ placeID ].people_buffer_type[peopleID] == RESORT_DATA_BUFFER_TYPE_NULL, "assert placeID%d peopleID%d buffer%d\n", placeID, peopleID, p_wk->info[ placeID ].people_buffer_type[peopleID] );

        p_wk->info[ placeID ].people_buffer_type[ peopleID ]  = RESORT_ABSTRUCT_DATA_GetBufferType( p_data );
        p_wk->info[ placeID ].people_index[ peopleID ] = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_data );
        OS_TFPrintf( 2, "placeID%d peopleID%d buffer_type%d bufferIndex%d\n", placeID, peopleID, p_wk->info[ placeID ].people_buffer_type[ peopleID ], p_wk->info[ placeID ].people_index[ peopleID ] );
      }
    }
  }
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
    RESORT_DATA_BUFFER_TYPE_SECRETARY,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
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
 *	@brief  旅行者から滞在者への変換チェックとそれぞれのバッファ取得
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src  旅行者
 *	@param	placeID  場所ID受け取り
 *	@param	*p_peopleID 人物ID受け取り
 *	@param  *pp_dst     滞在者受け取り
 *
 *	@return　TRUEで転職可能 FALSEで不可能
 */
//-----------------------------------------------------------------------------
extern BOOL RESORT_DATA_MANAGER_CheckConvert( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_BUFFER_TYPE type, RESORT_PLACEID placeID, u16 *p_peopleID, RESORT_ABSTRUCT_DATA **pp_dst )
{
  u16 peopleID;
  RESORT_ABSTRUCT_DATA *p_data;

  //場所の空きを検索
  peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( cp_wk, placeID );

  //バッファの空きを検索
  p_data = RESORT_ABSTRUCT_MANAGER_GetEmptyData( cp_wk->p_abstruct, type );

  if( peopleID == RESORT_PEOPLE_ID_NULL )
  {
    OS_Printf( "施設内位置が空いていない\n" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  if( p_data == NULL )
  {
    OS_Printf( "バッファが空いていない" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  if( p_peopleID )
  {
    *p_peopleID = peopleID;
  }
  if( pp_dst )
  {
    *pp_dst = p_data;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  転職可能かチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_ABSTRUCT_DATA *cp_src 受信データ
 *	@param  RESORT_DATA_TYPE dst_type 転職先タイプ
 *
 *	@return TRUEで可能  FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsJobChange( const RESORT_DATA_MANAGER *cp_wk, const RESORT_ABSTRUCT_DATA *cp_src, RESORT_DATA_TYPE dst_type )
{
  RESORT_DATA_BUFFER_TYPE type  = RESORT_ABSTRUCT_DATA_GetBufferType(cp_src);
  u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(cp_src);

  GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(cp_src)!=dst_type);

  return RESORT_DATA_MANAGER_CheckConvert( cp_wk, RESORT_DATA_TYPE_ToBufferType(dst_type), RESORT_MAP_GetPeoplePlace(people_type), NULL, NULL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から住民へデータ変換
 *
 *	@param	const RESORT_DATA_MANAGER *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src 受信データ
 *	@param	type  転職タイプ　RESORT_RESIDENT_TYPE_xxx
 *
 *	@retval インデックス
 */
//-----------------------------------------------------------------------------
u32 RESORT_DATA_MANAGER_Convert( RESORT_DATA_MANAGER *p_wk, RESORT_ABSTRUCT_DATA *p_src, RESORT_DATA_TYPE dst_type )
{
  u16 peopleID;
  RESORT_ABSTRUCT_DATA *p_dst_adrs = NULL;
  RESORT_PLACEID  placeID = RESORT_MAP_GetPeoplePlace(RESORT_ABSTRUCT_DATA_GetPeopleType(p_src));
  RESORT_DATA_TYPE src_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_src);

  if( src_type == dst_type )
  {
    //同じ場合は住人だけのはず
    RESORT_RESIDENT_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );

    GF_ASSERT( src_type == RESORT_DATA_TYPE_RESIDENT );

    peopleID  = RESORT_RECV_DATA_GetData( p_src_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

    RESORT_RESIDENT_DATA_ConvertRecv( p_src_data, p_src_data, peopleID, placeID );

    return RESORT_ABSTRUCT_DATA_GetBufferIndex( p_src );
  }
  else if( RESORT_DATA_MANAGER_CheckConvert( p_wk, RESORT_DATA_TYPE_ToBufferType(dst_type), placeID, &peopleID, &p_dst_adrs ) )
  {
    enum
    {
      JB_NONE,
      JB_RECV_RESI,
      JB_PROX_RESI,
      JB_RECV_SECR,
      JB_PROX_SECR,
    };

    static const sc_jobchnage_tbl[RESORT_DATA_TYPE_MAX][RESORT_DATA_TYPE_MAX] =
    { 
      //元RESI,   PROXY,        RECV,         SECR    //先
      {JB_NONE, JB_PROX_RESI, JB_RECV_RESI, JB_NONE}, //RESI
      {JB_NONE, JB_NONE,      JB_NONE,      JB_NONE}, //PROXY,
      {JB_NONE, JB_NONE,      JB_NONE,      JB_NONE}, //RECV,
      {JB_NONE, JB_PROX_SECR, JB_RECV_SECR, JB_NONE}, //SECR,
    };

    switch( sc_jobchnage_tbl[dst_type][src_type] )
    {
    case JB_RECV_RESI:
      {
        RESORT_RESIDENT_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        RESORT_RECV_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_RESIDENT_DATA_ConvertRecv( p_dst_data, p_src_data, peopleID, placeID );
      }
      break;
    case JB_PROX_RESI:
      {
        RESORT_RESIDENT_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        RESORT_PROXY_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_RESIDENT_DATA_ConvertProxy( p_dst_data, p_src_data, peopleID, placeID );
      }
      break;
    case JB_RECV_SECR:
      {
        RESORT_SECRETARY_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        u16 secretary_type  = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
        RESORT_RECV_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_SECRETARY_DATA_ConvertRecv( p_dst_data, p_src_data, peopleID, placeID, secretary_type );
      }
      break;
    case JB_PROX_SECR:
      {
        RESORT_SECRETARY_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        u16 secretary_type  = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
        RESORT_PROXY_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_SECRETARY_DATA_ConvertProxy( p_dst_data, p_src_data, peopleID, placeID, secretary_type );
      }
      break;
    default:
      GF_ASSERT_MSG(0,"src%d dst%d\n",src_type,dst_type);
      return 0;
    }

    //アップデート
    RESORT_DATA_MANAGER_Update( p_wk );

    return RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
  }
  else
  {
    //IsJobChangeでチェックしていない？
    GF_ASSERT(0);
    return 0;
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
 *	@brief  お店名を取得
 *
 *	@param	const RESORT_BINARY_SHOP_DATA *cp_wk  ROM内データ
 *	@param	RESORT_RESIDENT_DATA *cp_data メモリデータ
 *	@param	heapID ヒープID
 *
 *	@return お店名
 */
//-----------------------------------------------------------------------------
STRBUF * RESORT_DATA_MANAGER_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_ABSTRUCT_DATA *cp_data, u16 shopno, HEAPID heapID )
{
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];

  const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(cp_wk, shopno);

  WORDSET *p_wordset = WORDSET_Create( GFL_HEAP_LOWID(heapID) );

  GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
  u16 shop_name = RESORT_BINARY_SHOP_DATA_GetData( cp_shop, RESORT_BINARY_SHOP_PARAM_NAME );
  STRBUF *p_tmp = GFL_MSG_CreateString( p_msg, shop_name );
  STRBUF *p_name_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
  STRBUF *p_str = GFL_STR_CreateBuffer( 64, heapID );
  u32 sex = RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_SEX, NULL );
  RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_NAME, name );
  GFL_STR_SetStringCode(p_name_str, name );

  WORDSET_RegisterWord( p_wordset, 0, p_name_str, sex, TRUE, PM_LANG );
  WORDSET_ExpandStr( p_wordset, p_str, p_tmp );

  GFL_STR_DeleteBuffer( p_name_str );
  GFL_STR_DeleteBuffer( p_tmp );
  GFL_MSG_Delete( p_msg );
  WORDSET_Delete( p_wordset );

  return p_str;
}
//=============================================================================
/**
 *					データ抽象型
*/
//=============================================================================
//-------------------------------------
///	関数型（ポインタを書き換えるため）
//=====================================
typedef void *(*InitFunction)( HEAPID heapID );
typedef void (*ExitFunction)( void* );
typedef void (*InitInstanceFunction)( void* );
typedef BOOL (*IsEmptyFunction)( const void* );
typedef u32 (*GetDataFunction)( const void*, int, void* );
typedef void (*SetDataFunction)( void*, int, u32 );

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
} RESORT_ABSTRUCT_DATA_FUNCTION;
//-------------------------------------
///	メインワーク
//=====================================
struct _RESORT_ABSTRUCT_DATA
{
  void *p_data_adrs;
  const RESORT_ABSTRUCT_DATA_FUNCTION *cp_func;
  BOOL is_create;
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
  },
  //RESORT_DATA_TYPE_PROXY
  {
    (InitFunction)RESORT_PROXY_DATA_Init,
    (ExitFunction)RESORT_PROXY_DATA_Exit,
    (InitInstanceFunction)RESORT_PROXY_DATA_InitInstance,
    (IsEmptyFunction)RESORT_PROXY_DATA_IsEmpty,
    (GetDataFunction)RESORT_PROXY_DATA_GetData,
    (SetDataFunction)RESORT_PROXY_DATA_SetData,
  },
  //RESORT_DATA_TYPE_RECV
  {
    (InitFunction)RESORT_RECV_DATA_Init,
    (ExitFunction)RESORT_RECV_DATA_Exit,
    (InitInstanceFunction)RESORT_RECV_DATA_InitInstance,
    (IsEmptyFunction)RESORT_RECV_DATA_IsEmpty,
    (GetDataFunction)RESORT_RECV_DATA_GetData,
    (SetDataFunction)RESORT_RECV_DATA_SetData,
  },
  //RESORT_DATA_TYPE_SECRETARY
  {
    (InitFunction)RESORT_SECRETARY_DATA_Init,
    (ExitFunction)RESORT_SECRETARY_DATA_Exit,
    (InitInstanceFunction)RESORT_SECRETARY_DATA_InitInstance,
    (IsEmptyFunction)RESORT_SECRETARY_DATA_IsEmpty,
    (GetDataFunction)RESORT_SECRETARY_DATA_GetData,
    (SetDataFunction)RESORT_SECRETARY_DATA_SetData,
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

  if( p_data_adrs == NULL )
  {
    GF_ASSERT(0);
    p_wk->p_data_adrs = p_wk->cp_func->Init( heapID );
    p_wk->is_create = TRUE;
  }

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
  if( p_wk->is_create )
  {
    p_wk->is_create = FALSE;
    p_wk->cp_func->Exit( p_wk->p_data_adrs );
  }

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
  GF_ASSERT( p_wk->is_create == FALSE );
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
  u16 current;
  u16 tbl_index;
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
void RESORT_ABSTRUCT_MANAGER_BeginIterator( RESORT_ABSTRUCT_MANAGER * p_wk, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max )
{
  p_wk->tbl_index = 0;
  p_wk->current = sc_type[ cp_tbl[p_wk->tbl_index] ].start;
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
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_MANAGER_NextIterator( RESORT_ABSTRUCT_MANAGER * p_wk, const RESORT_DATA_BUFFER_TYPE *cp_tbl, u32 tbl_max )
{
  if( p_wk->tbl_index < tbl_max )
  {
    if( sc_type[ cp_tbl[p_wk->tbl_index] ].start <= p_wk->current && p_wk->current < sc_type[ cp_tbl[p_wk->tbl_index] ].end )
    {
      return p_wk->p_data[ p_wk->current++ ];
    }
    else
    {
      p_wk->tbl_index++;
      if( p_wk->tbl_index < tbl_max )
      {
        p_wk->current = sc_type[ cp_tbl[p_wk->tbl_index] ].start;
        return p_wk->p_data[ p_wk->current++ ];
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
  RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk, &type, 1 );
  while(1)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk, &type, 1 );
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
 *	@brief  不正文字
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_ABSTRUCT_MANAGER_CheckProfWord( RESORT_ABSTRUCT_MANAGER *p_wk, HEAPID heapID )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(prof_word) );

  {
    static const RESORT_DATA_BUFFER_TYPE sc_tbl[] =
    {
      RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
      RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    };

    STRCODE src_name[PERSON_NAME_SIZE+EOM_SIZE];
    STRCODE dst_name[PERSON_NAME_SIZE+EOM_SIZE];

    PROF_WORD_WORK *p_prof = PROF_WORD_AllocWork( heapID );

    DWC_TOOL_SetBadNickName( dst_name, PERSON_NAME_SIZE+EOM_SIZE, heapID );

    RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk, sc_tbl, NELEMS(sc_tbl) );
    while(1)
    {
      RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk, sc_tbl, NELEMS(sc_tbl) );
      if( p_data == NULL )
      {
        break;
      }

      if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
      {
        RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_NAME, src_name );
        if( PROF_WORD_CheckProfanityWordCode( p_prof, src_name, heapID ) )
        {
          RESORT_ABSTRUCT_DATA_SetData( p_data, RESORT_DATA_PARAM_NAME, (u32)dst_name );
        }
      }
    }

    PROF_WORD_FreeWork( p_prof );
  }

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(prof_word) );
}
