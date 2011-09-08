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
  RESORT_PLACEID  placeID = RESORT_MAP_GetPeoplePlace(dst_type);
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


  if( rnd < present_recv )
  {
    int i;

    u32 present_list  = RESORT_BINARY_CITY_GetData( cp_city_data, index, RESORT_BINARY_CITY_PARAM_PRESENT_LIST );

    //なにのおみやげを貰えるか
    rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_ITEM_KIND, 100 );

    present_recv  = 0;
    for( i = 0; i < RESORT_BINARY_PRESENT_PARAM_ITEM_MAX; i++ )
    {
      u32 item = RESORT_BINARY_PRESENT_GetData( cp_present_data, present_list, RESORT_BINARY_PRESENT_PARAM_ITEM_00+i*2 );
      present_recv += RESORT_BINARY_PRESENT_GetData( cp_present_data, present_list, RESORT_BINARY_PRESENT_PARAM_PERCENT_00+i*2 );

      if( rnd < present_recv )
      {
        return item;
      }
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
 *	@brief  いきたい場所抽選
 *
 *	@param	const RESORT_RECV_DATA *cp_wk   ワーク
 *	@param	RESORT_BINARY_SPOT *cp_spot_data  いきたい場所バイナリ
 *	@param	RESORT_BINARY_CITY *cp_city_data  街バイナリ
 *	@param	RESORT_CITY_DATA *cp_city         街データ
 *
 *	@return いきたい場所ID
 */
//-----------------------------------------------------------------------------
u32 RESORT_ABSTRUCT_DATA_DrawLotsWhereToGo( const RESORT_ABSTRUCT_DATA *cp_wk, const RESORT_BINARY_SPOT *cp_spot_data, const RESORT_BINARY_CITY *cp_city_data, const RESORT_CITY_DATA *cp_city )
{
  u32 appeal_lv = 0;

  //街レベルから弱中強グループをランダム選出
  {
    u32 lv = RESORT_CITY_DATA_GetData( cp_city, RESORT_CITY_DATA_PARAM_LV, NULL);

    u32 index = RESORT_BINARY_CITY_GetIndexByCityLv( cp_city_data, lv );
    u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_WHERESPOT_LV, 100 );
    u32 percent = 0;

    int  i;
    for( i = 0; i < RESORT_APPEAL_LV_MAX; i++ )
    {
      percent += RESORT_BINARY_CITY_GetData( cp_city_data, index, RESORT_BINARY_CITY_PARAM_GROUP_00+i );
      if( percent > rnd )
      {
        appeal_lv = i;
        break;
      }
    }
    NAGI_Printf( "group%d idx%d lv%d\n", i, index, lv );
  }

  //選ばれたグループから行きたいお店メッセージをランダム選出
  {
    int rnd;
    int i;
    int cnt = 0;

    //i = 1なのは、0は何も無いを表すため
    for( i = 1; i < RESORT_BINARY_SPOT_GetMax(cp_spot_data); i++ )
    {
      u16 group = RESORT_BINARY_SPOT_GetData( cp_spot_data, i, RESORT_BINARY_SPOT_PARAM_GROUP );
      if( group == appeal_lv )
      {
        cnt++;
      }
    }

    GF_ASSERT( cnt != 0 );

    rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_WHERESPOT_KIND, cnt );

    cnt = 0;
    for( i = 1; i < RESORT_BINARY_SPOT_GetMax(cp_spot_data); i++ )
    {
      u16 group = RESORT_BINARY_SPOT_GetData( cp_spot_data, i, RESORT_BINARY_SPOT_PARAM_GROUP );
      if( group == appeal_lv )
      {
        if( cnt == rnd )
        {
          return i;
        }
        cnt++;
      }
    }
  }
  GF_ASSERT(0);
  return 1;
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

//----------------------------------------------------------------------------
/**
 *	@brief  プリセットデータから代理人を設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	type  代理人タイプ
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param	presetID  プリセットインデックス
 *	@param	heapID  テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void Resort_Data_Manager_SetProxyPreset( RESORT_PROXY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 presetID, HEAPID heapID )
{
  RESORT_PROXY_DATA_InitInstance( p_wk );

  {
    u32 name = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_NAME);
    u32 intro = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_INTRO);
    u32 phrase = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_PHRASE);
    u32 thanks = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_THANKS);
    u32 sex = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_SEX);
    u32 view = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_VIEW);
    u32 id = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_ID);
    u32 job = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_JOB);
    u32 spot = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_SPOT);

    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );

    {
      STRBUF *p_name = GFL_MSG_CreateString( p_msg, name );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_NAME, (u32)GFL_STR_GetStringCodePointer(p_name) );
      GFL_STR_DeleteBuffer( p_name );
    }
    {
      STRBUF *p_intro = GFL_MSG_CreateString( p_msg, intro );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_INTRODUCTION, (u32)GFL_STR_GetStringCodePointer(p_intro) );
      GFL_STR_DeleteBuffer( p_intro );
    }
    {
      STRBUF *p_phrase = GFL_MSG_CreateString( p_msg, phrase );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT6_PHRASE, (u32)GFL_STR_GetStringCodePointer(p_phrase) );
      GFL_STR_DeleteBuffer( p_phrase );
    }
    {
      STRBUF *p_thanks = GFL_MSG_CreateString( p_msg, thanks );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT7_THANKS, (u32)GFL_STR_GetStringCodePointer(p_thanks) );
      GFL_STR_DeleteBuffer( p_thanks );
    }
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_SEX, sex );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_OBJCODE, view );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_TRAINER_ID, id );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_WANT_JOB, job );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_WANT_SPOT, spot );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_TYPE, type );

    GFL_MSG_Delete( p_msg );
  }

  //日時
  {
    RTCDate date;
    GFL_RTC_GetDate(&date);
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_YEAR, date.year );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_MONTH, date.month );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_DAY, date.day );
  }

  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_RND_SEED, GFUser_GetPublicRand(GFL_STD_RAND_MAX) );
  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_VER, GET_VERSION() );
  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_LANG, PM_LANG );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ファン設定処理
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセットデータ
 *	@param	RESORT_RECV_DATA *cp_owner  自分のデータ
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_SetProxyFunRandom( RESORT_PROXY_DATA *p_wk, const RESORT_BINARY_PRESET *cp_preset, const RESORT_RECV_DATA *cp_owner, HEAPID heapID )
{
  u32 presetID  = 0;
  while(TRUE )
  {
    presetID = GFUser_GetPublicRand( RESORT_PRESET_ID_MAX );

    if( RESORT_BINARY_PRESET_GetData( cp_preset, presetID, RESORT_BINARY_PRESET_PARAM_TYPE ) == RESORT_PRESET_TYPE_RANDOM )
    {
      break;
    }
  }

  //プリセットを設定
  Resort_Data_Manager_SetProxyPreset( p_wk, RESORT_PROXY_TYPE_FUN, cp_preset, presetID, heapID );

  //ファンは主人公のフリーワードをコピーする
  {
    STRCODE strcode[ 64 ];

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_INTRODUCTION, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_INTRODUCTION, (u32)strcode );

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_EXT6_PHRASE, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT6_PHRASE, (u32)strcode );

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_EXT7_THANKS, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT7_THANKS, (u32)strcode );
  }

}


//----------------------------------------------------------------------------
/**
 *	@brief  秘書にプリセットを使用して設定
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param  preset_type タイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_SetSecretaryPreset( RESORT_SECRETARY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 preset_type, u16 peopleID, HEAPID heapID )
{
  u32 presetID  = 0;
  
  for( presetID = 0; presetID < RESORT_BINARY_GetMax(cp_preset); presetID++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, presetID, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      break;
    }
  }
  GF_ASSERT( presetID < RESORT_BINARY_GetMax(cp_preset) );

  {
    RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID ));
    Resort_Data_Manager_SetProxyPreset( p_proxy, 0, cp_preset, presetID, heapID );
    RESORT_SECRETARY_DATA_ConvertProxy( p_wk, p_proxy, peopleID, RESORT_PLACEID_RECEPTION, type );
    RESORT_PROXY_DATA_Exit(p_proxy);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人にプリセットを使用して設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	type  代理人タイプ
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param	preset_type プリセットタイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_SetProxyPreset( RESORT_PROXY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 preset_type, HEAPID heapID )
{
  u32 presetID  = 0;
  
  for( presetID = 0; presetID < RESORT_BINARY_GetMax(cp_preset); presetID++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, presetID, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      break;
    }
  }
  GF_ASSERT( presetID < RESORT_BINARY_GetMax(cp_preset) );

  //プリセットを設定
  Resort_Data_Manager_SetProxyPreset( p_wk, type, cp_preset, presetID, heapID );
}


//----------------------------------------------------------------------------
/**
 *	@brief  プリセットからデータを作成
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *	@param	*p_sys  リゾーとシステム
 *	@param	people_type 人物タイプ
 *	@param	preset_type プリセットタイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_SetPresetPeople( RESORT_DATA_MANAGER *p_wk, RESORT_PEOPLE_ALL_DATA *p_people_all, RESORT_PROXY_BUFF *p_proxy_buff, const RESORT_BINARY_PRESET *cp_preset, const MYSTATUS *cp_mystatus, u16 people_type, u16 preset_type, HEAPID heapID )
{
  switch( people_type )
  {
  case RESORT_PEOPLE_TYPE_PROXY:
    {
      static const struct 
      {
        u16 preset_type;
        u16 proxy_type;
      }sc_preset_type_to_proxy_type[] =
      {
        { RESORT_PRESET_TYPE_TUTORIAL1 , RESORT_PROXY_TYPE_TUTORIAL_01 },
        { RESORT_PRESET_TYPE_TUTORIAL2 , RESORT_PROXY_TYPE_TUTORIAL_02 },
        { RESORT_PRESET_TYPE_APPOINT,    RESORT_PROXY_TYPE_CELEB },
      };
      int i;
      BOOL is_set;
      u16 proxy_type  = 0xFF;

      for( i = 0; i < NELEMS(sc_preset_type_to_proxy_type); i++)
      {
        if( sc_preset_type_to_proxy_type[i].preset_type == preset_type )
        {
          proxy_type = sc_preset_type_to_proxy_type[i].proxy_type;
        }
      }
      GF_ASSERT( proxy_type != 0xFF );

      {
        RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID) );
        RESORT_DATA_MANAGER_SetProxyPreset( p_proxy, proxy_type, cp_preset, preset_type, heapID );

        //もしチュートリアル用ならばプレイヤーの初期お店になる
        if( RESORT_PRESET_TYPE_TUTORIAL1 == preset_type )
        {
          RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
          u32 want_job = RESORT_OWNER_DATA_GetWantJob( p_owner, cp_mystatus );
          RESORT_PROXY_DATA_SetData( p_proxy, RESORT_DATA_PARAM_WANT_JOB, want_job );
        }

        is_set = RESORT_PROXY_BUFF_Add( p_proxy_buff, p_proxy );
        GF_ASSERT( is_set );
        RESORT_PROXY_DATA_Exit( p_proxy );
      }
    }
    break;
  case RESORT_PEOPLE_TYPE_SECRETARY:
    {
      static const struct 
      {
        u16 preset_type;
        u16 sec_type;
      }sc_preset_type_to_sec_type[] =
      {
        { RESORT_PRESET_SECRETARY_01, RESORT_SECRETARY_TYPE_WORK },
        { RESORT_PRESET_SECRETARY_02, RESORT_SECRETARY_TYPE_MOVE },
        { RESORT_PRESET_SECRETARY_03, RESORT_SECRETARY_TYPE_CENTER },
        { RESORT_PRESET_SECRETARY_04, RESORT_SECRETARY_TYPE_PC },
      };
      int i;
      BOOL is_set;
      u16 sec_type  = 0xFF;

      for( i = 0; i < NELEMS(sc_preset_type_to_sec_type); i++)
      {
        if( sc_preset_type_to_sec_type[i].preset_type == preset_type )
        {
          sec_type = sc_preset_type_to_sec_type[i].sec_type;
        }
      }
      GF_ASSERT( sec_type != 0xFF );

      {
        RESORT_SECRETARY_DATA *p_empty;
        RESORT_SECRETARY_DATA *p_secretary = RESORT_SECRETARY_DATA_Init( GFL_HEAP_LOWID(heapID) );
        u16 peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( p_wk, RESORT_PLACEID_RECEPTION );
        RESORT_DATA_MANAGER_SetSecretaryPreset( p_secretary, sec_type, cp_preset, preset_type, peopleID, heapID );

        p_empty = RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( p_people_all );
        GF_ASSERT( p_empty );
        GFL_STD_MemCopy( p_secretary, p_empty, RESORT_SECRETARY_DATA_SIZE );
        RESORT_DATA_MANAGER_Update( p_wk );
        RESORT_SECRETARY_DATA_Exit( p_secretary );
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }
}

//=============================================================================
/**
 *					ランク閲覧
*/
//=============================================================================
enum
{
  RNKDATA_NUM = 9,
  RNKDATA_EMPTY = 0xFFFFFFFF,

  RNKDATA_W = 30,
  RNKDATA_H = 22,
};
//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウ作成
 *
 *	@param	FLDMSGBG *p_msgbg
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
FLDMSGWIN * RESORT_RANK_WND_Create( FLDMSGBG *p_msgbg, HEAPID heapID )
{
  FLDMSGWIN *p_fldmsgwin = FLDMSGWIN_Add(p_msgbg, NULL, 
      1, 1, RNKDATA_W, RNKDATA_H);

  return p_fldmsgwin;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウ描画
 *
 *	@param	HEAPID heapID 
 *
 *	@return RESORT_RANK_WND ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_RANK_WND_Write( FLDMSGWIN *p_fldmsgwin, RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID )
{

  struct
  {
    u32 index;
    u32 value;
  }rnkdata[ RNKDATA_NUM ];

  int i, j;
  int cnt = 0;

  RESORT_WORK *p_reso_wk = GAMEDATA_GetReosrtWork( p_gamedata );
  MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );

  GF_ASSERT( param < 8 );

  //ランキング作成

  //空に
  for( i = 0; i < RNKDATA_NUM; i++ )
  {
    rnkdata[i].index  = RNKDATA_EMPTY;
    rnkdata[i].value  = RNKDATA_EMPTY;
  }

  //データを集める
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_data) )
    {
      if( (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_02) && param < 4)
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_03) && param >= 4) )
      {
        rnkdata[cnt].index  = i;
        rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT2_RANK_00 + param, NULL );
        cnt++;
      }
    }
  }
  {
    rnkdata[cnt].index  = 8;
    rnkdata[cnt].value  = RESORT_WORK_GetData( p_reso_wk, RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE + param );
    cnt++;
  }
  //ソート
  for( i = 0; i < cnt-1; i++ )
  {
    for( j = cnt-1; j > i; j-- )
    {
      s32 idx1  = j-1;
      s32 idx2  = j;
      if( rnkdata[idx1].value < rnkdata[idx2].value )
      {
        s32 value = rnkdata[idx1].value;
        s32 index = rnkdata[idx1].index;
        rnkdata[idx1] = rnkdata[idx2];
        rnkdata[idx2].value = value;
        rnkdata[idx2].index = index;
      }
    }
  }
  {
    u32 w;
    STRBUF *p_name_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
    STRBUF *p_src_str = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(heapID) );
    STRBUF *p_dst_str = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(heapID) );
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );

    GFL_FONT *p_font = FLDMSGBG_GetFontHandle( p_msgbg );

    FLDMSGWIN_ClearWindow(p_fldmsgwin);

    //タイトル描画
    {
      GFL_MSG_GetString( p_msg, resort_rnkwnd_title_00 + param, p_dst_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, RNKDATA_W*8/2-w/2, 0, p_dst_str );
    }

    for( i = 0; i < cnt; i++ )
    {
      RESORT_RESIDENT_DATA *p_data;
      
      if( rnkdata[i].index == 8 )
      {
        p_data = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
      }
      else
      {
        p_data = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, rnkdata[i].index );
      }
      //順位
      GFL_MSG_GetString( p_msg, resort_rnkwnd_rank_00, p_src_str );
      WORDSET_RegisterNumber( p_wordset, 0, i+1, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 5*8-w, (3+i*2)*8, p_dst_str );

      //名前
      {
        if( rnkdata[i].index == 8 )
        {
          WORDSET_RegisterPlayerName( p_wordset, 0, p_mystatus );
        }
        else
        {
          STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
          u32 sex = RESORT_RESIDENT_DATA_GetData(p_data, RESORT_DATA_PARAM_SEX, NULL );
          RESORT_RESIDENT_DATA_GetData(p_data, RESORT_DATA_PARAM_NAME, name );
          GFL_STR_SetStringCode( p_name_str, name );
          GFL_MSG_GetString( p_msg, resort_rnkwnd_name_00, p_src_str );
          WORDSET_RegisterWord( p_wordset, 0, p_name_str, sex, TRUE, PM_LANG );
        }
        {
          WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
          w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
          FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 15*8-w, (3+i*2)*8, p_dst_str );
        }
      }

      //数
      GFL_MSG_GetString( p_msg, resort_rnkwnd_cnt_00, p_src_str );
      WORDSET_RegisterNumber( p_wordset, 0, rnkdata[i].value, 7, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 27*8-w, (3+i*2)*8, p_dst_str );
    }

    GFL_MSG_Delete( p_msg );
    GFL_STR_DeleteBuffer( p_dst_str );
    GFL_STR_DeleteBuffer( p_src_str );
    GFL_STR_DeleteBuffer( p_name_str );
  }
}


static u32 SexGetMsgID( u32 sex, u32 male, u32 female )
{
  return sex == PM_MALE ? male: female;
}
//----------------------------------------------------------------------------
/**
 *	@brief  MSGIDを取得
 *
 *	@param	u16 param 引数
 *	@param	*p_wordset  ワードセット
 *	@param	heapID  ひーぷID
 *
 *	@return MSGID
 */
//-----------------------------------------------------------------------------
u16 RESORT_SCRCMD_GetMsgID( u16 param, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_MANAGER *p_man, RESORT_CITY_DATA *p_city_data ,RESORT_BINARY_SPOT *p_spot, RESORT_BINARY_SHOP *p_shop, RESORT_BINARY_CITY *p_city, WORDSET *p_wordset, HEAPID heapID )
{
  u32 sex = PM_MALE;
  u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct);

  switch( param )
  {
  case RESORT_MSGID_RECENT_00:
  case RESORT_MSGID_RECENT_01:
  case RESORT_MSGID_RECENT_02:
  case RESORT_MSGID_RECENT_03:
    {
      u32 idx = RESORT_MSGID_RECENT_00 - param;
      u32 msg = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_00+idx, NULL );
      RESORT_DATE date = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx, NULL );
      
      //年月日を登録
      WORDSET_RegisterNumber( p_wordset, 0, RESORT_DATE_GET_YEAR(date), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 2, RESORT_DATE_GET_MONTH(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 4, RESORT_DATE_GET_DAY(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

      return SexGetMsgID( sex, resort_history_00_m, resort_history_00_f ) + msg;
    }
    break;
  case RESORT_MSGID_WHERETOGO:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST
        || people_type == RESORT_PEOPLE_TYPE_PROXY );
    {
      u32 spot = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, NULL );
      if( spot == 0 )
      {
        spot = RESORT_ABSTRUCT_DATA_DrawLotsWhereToGo( p_abstruct, p_spot, p_city, p_city_data );
        RESORT_ABSTRUCT_DATA_SetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, spot );
      }
      GF_ASSERT( spot > 0 );
      if( sex == PM_MALE )
      {
        return RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_M );
      }
      else
      {
        return RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_F );
      }
    }
    break;
  case RESORT_MSGID_APPEAL:
    {
      u16 want_job = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_JOB, NULL );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, want_job );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      return shop_type + SexGetMsgID( sex, resort_appeal_00_m, resort_appeal_00_f );
    }
    break;
  case RESORT_MSGID_SHOP_INFO:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_RESIDENT );
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_abstruct );
      RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution( p_resi );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      return shop_type + SexGetMsgID( sex, resort_shopinfo_00_m, resort_shopinfo_00_f );
    }
    break;
  case RESORT_MSGID_GOSSIP:
    return 0; //TODO
    break;
  case RESORT_MSGID_MEDAL_HAVE:
    return SexGetMsgID( sex, resort_history_mdl_01_m, resort_history_mdl_01_f );
    break;
  case RESORT_MSGID_MEDAL_NONE:
    return SexGetMsgID( sex, resort_history_mdl_02_m, resort_history_mdl_02_f );
    break;
  case RESORT_MSGID_RECORD_NEXT:
    return SexGetMsgID( sex, resort_history_msg_01_m, resort_history_msg_01_f );
    break;
  case RESORT_MSGID_RECORD_NONE:
    return SexGetMsgID( sex, resort_history_00_m, resort_history_00_f );
    break;
  case RESORT_TOURIST_GREET_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
    {
      RESORT_COMM_TYPE  comm  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_COMM_ID, NULL );
      u32 num  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXTEND_NUM, NULL );
      {

        if( comm == RESORT_COMM_TYPE_BEACON )
        {
          u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_07_m, resort_tourist_greet_07_f );
          GF_ASSERT( num > 0 );
          return msg_start + num - 1;

        }
        else if( comm == RESORT_COMM_TYPE_NULL )
        {
          //ファンかチュートリアル
          GF_ASSERT(0);
        }
        else
        {
          u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_00_m, resort_tourist_greet_00_f );
          return msg_start + comm - RESORT_COMM_TYPE_START;
        }
      }
    }
    break;
  case RESORT_TOURIST_CITY_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
    {
      u32 nation  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_NATION, NULL );
      u32 area  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_AREA, NULL );

      if( nation > 0 && area > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        WORDSET_RegisterLocalPlaceName( p_wordset, 2, nation, area );
        return SexGetMsgID( sex, resort_tourist_citymsg_02_m, resort_tourist_citymsg_02_m );
      }
      else if( nation > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        return SexGetMsgID( sex, resort_tourist_citymsg_03_m, resort_tourist_citymsg_03_f );
      }
      else
      {
        return SexGetMsgID( sex, resort_tourist_citymsg_01_m, resort_tourist_citymsg_01_f );
      }
    }
    break;
  case RESORT_TOURIST_SCOUT_YES_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_01_m, resort_tourist_scout_01_f );
    break;
  case RESORT_TOURIST_SCOUT_THANKS_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_02_m, resort_tourist_scout_02_f );
    break;
  case RESORT_TOURIST_SCOUT_NO_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_03_m, resort_tourist_scout_03_f );
    break;
  case RESORT_TOURIST_SCOUT_REFUSE_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_04_m, resort_tourist_scout_04_f );
    break;
  case RESORT_TOURIST_SPOT_GOING:
    return SexGetMsgID( sex, resort_resident_spot_go_00_m, resort_resident_spot_go_00_f );
    break;
  case RESORT_TOURIST_ITEM_MSGID:
    return SexGetMsgID( sex, resort_tourist_item_00_m, resort_tourist_item_00_f );
    break;
  case RESORT_TOURIST_BYE_MSGID:
    return SexGetMsgID( sex, resort_tourist_bye_00_m, resort_tourist_bye_00_f );
    break;
  case RESORT_TOURIST_JOY_MSGID:
    return SexGetMsgID( sex, resort_tourist_joy_00_m, resort_tourist_joy_00_f );
    break;
  case RESORT_RESIDENT_GREET_MSGID:
    return SexGetMsgID( sex, resort_resident_greet_00_m, resort_resident_greet_00_f );
    break;
  case RESORT_RESIDENT_BYE_MSGID:
    return SexGetMsgID( sex, resort_resident_bye_00_m, resort_resident_bye_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_MSGID:
    return SexGetMsgID( sex, resort_resident_change_00_m, resort_resident_change_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_CONFIM_MSGID:
    return SexGetMsgID( sex, resort_resident_change_con_00_m, resort_resident_change_con_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_OK_MSGID:
    return SexGetMsgID( sex, resort_resident_change_ok_00_m, resort_resident_change_ok_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_NG_MSGID:
    return SexGetMsgID( sex, resort_resident_change_no_00_m, resort_resident_change_no_00_f );
    break;
  case RESORT_SECRETARY_GREET_MSGID:
    return SexGetMsgID( sex, resort_secretary_greet_00_m, resort_secretary_greet_00_f );
    break;
  case RESORT_SECRETARY_BYE_MSGID:
    return SexGetMsgID( sex, resort_secretary_bye_00_m, resort_secretary_bye_00_f );
    break;
  case RESORT_SECRETARY_CITYLV_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_00_m, resort_secretary_talk_00_f );
    break;
  case RESORT_SECRETARY_SECSORT_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_04_m, resort_secretary_talk_04_f );
    break;
  case RESORT_SECRETARY_SHOPSORT_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_05_m, resort_secretary_talk_05_f );
    break;
  case RESORT_SECRETARY_NEON_OK_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_07_m, resort_secretary_talk_07_f );
    break;
  case RESORT_SECRETARY_NEON_SEL_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_06_m, resort_secretary_talk_06_f );
    break;
  case RESORT_SECRETARY_RANK_SEL_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_08_m, resort_secretary_talk_08_f );
    break;
  case RESORT_SECRETARY_SEL_GREET_MSGID:
    return SexGetMsgID( sex, resort_secretary_greet_01_m, resort_secretary_greet_01_m );
    break;
  case RESORT_SECRETARY_SEL_BYE_MSGID:
    return SexGetMsgID( sex, resort_secretary_bye_01_m, resort_secretary_bye_01_f );
    break;
  case RESORT_SECRETARY_SEL_SELECT:
    return SexGetMsgID( sex, resort_secretary_talk_03_m, resort_secretary_talk_03_f );
    break;
  case RESORT_SECRETARY_SEL_ONE:
    return SexGetMsgID( sex, resort_secretary_talk_01_m, resort_secretary_talk_01_f );
    break;
  case RESORT_SECRETARY_SEL_MANY:
    return SexGetMsgID( sex, resort_secretary_talk_02_m, resort_secretary_talk_02_f );
    break;
  case RESORT_RESIDENT_LV_MSGID:
    return SexGetMsgID( sex, resort_resident_lv_00_m, resort_resident_lv_00_f );
    break;

  case RESORT_TUTORIAL01_GREET_MSGID:
    return resort_tutorial_01_greet_00;
    break;

  case RESORT_TUTORIAL02_GREET_MSGID:
    return resort_tutorial_02_greet_00;
    break;
  case RESORT_FUN_GREET_MSGID:
    return SexGetMsgID( sex, resort_fun_greet_00_m, resort_fun_greet_00_f );
    break;

  case RESORT_MSGID_BOARD_00:
  case RESORT_MSGID_BOARD_01:
  case RESORT_MSGID_BOARD_02:
  case RESORT_MSGID_BOARD_03:
  case RESORT_MSGID_BOARD_04:
  case RESORT_MSGID_BOARD_05:
  case RESORT_MSGID_BOARD_06:
  case RESORT_MSGID_BOARD_07:
    {
      u32 index = param - RESORT_MSGID_BOARD_00;
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, index );
      const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
      u32 shop_kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
      {
        RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
        u16 shopno = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
        STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_abstruct, shopno, heapID );
        u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_SEX, NULL );

        WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );
        GFL_STR_DeleteBuffer(p_str);
      }
      return resort_board_00 + shop_kind;
    }
    break;
  }

  GF_ASSERT(0);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	u16 index
 *	@param	*p_resident
 *	@param	*p_city_data
 *	@param	*p_spot
 *	@param	*p_shop
 *	@param	*p_city
 *	@param	*p_wordset
 *	@param	*ret_grade_msg
 *	@param	*ret_eff_msg
 *	@param	*ret_lvup
 *	@param	ret_city_lvup 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
void RESORT_SCRCMD_CheckSpot( u16 index, RESORT_RESIDENT_DATA *p_resident, RESORT_CITY_DATA *p_city_data,RESORT_BINARY_SPOT *p_spot, RESORT_BINARY_SHOP *p_shop, RESORT_BINARY_CITY *p_city, WORDSET *p_wordset, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup )
{

  static const fx32 sc_grade[RESORT_GRADE_TYPE_MAX]  =
  {
    FX32_CONST(0.5),
    FX32_CONST(1.0),
    FX32_CONST(2.0),
  };

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );
  u32 next_exp;

  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_TYPE );
  u32 exp;
  int i;

  //何も起こらないときのメッセージ
  *ret_grade_msg  = resort_sys_08;
  *ret_lvup       = RESORT_SET_SPOT_SHOP_NONE;
  *ret_city_lvup  = RESORT_SET_SPOT_CITY_NONE;

  if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL) == PM_MALE )
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_EFF_MSG_M );
  }
  else
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_EFF_MSG_F );
  }

  if( lv >= RESORT_SHOP_LVMAX )
  {
    *ret_lvup = RESORT_SET_SPOT_SHOP_LVMAX;
    return;
  }

  for( i = 0; i < RESORT_BINARY_SPOT_ANSWER_MAX; i++ )
  {
    u16 type = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_TYPE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 data = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_DATA + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 grade = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_GRADE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 add_exp = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_BAESEXP );
    BOOL is_good = FALSE;

    add_exp = FX_Whole( sc_grade[ grade ] * add_exp );

    //条件にあうかチェック
    switch( type )
    {
    case RESORT_SPOT_TYPE_SHOPTYPE:
      if( shop_type == data )
      {
        is_good = TRUE;
      }
      break;

    case RESORT_SPOT_TYPE_SEX:
      if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL ) == data )
      {
        is_good = TRUE;
      }
      break;

    case RESORT_SPOT_TYPE_MOUTHCOMM_00:
    case RESORT_SPOT_TYPE_MOUTHCOMM_01:
    case RESORT_SPOT_TYPE_MOUTHCOMM_02:
    case RESORT_SPOT_TYPE_MOUTHCOMM_03:
    case RESORT_SPOT_TYPE_MOUTHCOMM_04:
    case RESORT_SPOT_TYPE_MOUTHCOMM_05:
    case RESORT_SPOT_TYPE_MOUTHCOMM_06:
    case RESORT_SPOT_TYPE_MOUTHCOMM_07:
    case RESORT_SPOT_TYPE_MOUTHCOMM_08:
    case RESORT_SPOT_TYPE_MOUTHCOMM_09:
    case RESORT_SPOT_TYPE_MOUTHCOMM_10:
    case RESORT_SPOT_TYPE_MOUTHCOMM_11:
    case RESORT_SPOT_TYPE_MOUTHCOMM_12:
    case RESORT_SPOT_TYPE_MOUTHCOMM_13:
    case RESORT_SPOT_TYPE_MOUTHCOMM_14:
    case RESORT_SPOT_TYPE_MOUTHCOMM_15:
      type = type - RESORT_SPOT_TYPE_MOUTHCOMM_00;
      if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT4_GOSSIP_00 + type, NULL ) == (data + 1) )
      {
        is_good = TRUE;
      } 
      break;
    case RESORT_SPOT_TYPE_NONE:
      is_good = FALSE;
      break;
    default:
      GF_ASSERT(0);
    }

    //施設に経験値が入る
    if( is_good )
    {
      *ret_grade_msg = resort_sys_03 + grade;
      WORDSET_RegisterNumber( p_wordset, 5, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      exp = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );
      next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );
      if( exp + add_exp >= next_exp )
      {
        const RESORT_BINARY_SHOP_DATA *cp_new_shop_data;
        u32 shopID;
        lv++;
        exp = 0;
        *ret_lvup = RESORT_SET_SPOT_SHOP_LVUP;
        RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, lv );

        //レベルアップしたら施設IDを変更
        shopID = RESORT_BINARY_SHOP_SearchShopID( p_shop, kind, lv, shop_type );
        RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, shopID );
        cp_new_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );

        WORDSET_RegisterNumber( p_wordset, 6, lv+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

        if( RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NAME ) != RESORT_BINARY_SHOP_DATA_GetData( cp_new_shop_data, RESORT_BINARY_SHOP_PARAM_NAME ) )
        {
          *ret_lvup = RESORT_SET_SPOT_SHOP_CHANGE;
        }
      }
      else
      {
        exp += add_exp;
      }
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP, exp );
    }

    //街に経験値が入る
    if( is_good )
    {
      u32 city_idx;
      lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      exp = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, NULL );

      if( lv < RESORT_BINARY_CITY_GetCityLvMax( p_city ) )
      {

        WORDSET_RegisterNumber( p_wordset, 7, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

        city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
        next_exp = RESORT_BINARY_CITY_GetData( p_city, city_idx, RESORT_BINARY_CITY_PARAM_EXP );
        if( exp + add_exp >= next_exp )
        {
          lv++;
          exp = 0;
          *ret_city_lvup = RESORT_SET_SPOT_CITY_LVUP;
          WORDSET_RegisterNumber( p_wordset, 8, lv+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        }
        else
        {
          exp += add_exp;
        }
        RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, lv );
        RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, exp );
      }
      else
      {
        *ret_city_lvup = RESORT_SET_SPOT_CITY_LVMAX;
      }
    }

    if( is_good )
    {
      break;
    }
  }
}
