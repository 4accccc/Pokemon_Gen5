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
  u8  people_type[RESORT_DATA_MANAGER_MAX];
}RESORT_DATA_MANAGER_INFO;

//-------------------------------------
///	辞書データ
//=====================================
struct _RESORT_DATA_MANAGER
{
  RESORT_DATA_MANAGER_INFO  info[RESORT_PLACEID_MAX];
  RESORT_DATA_MANAGER_INIT_PARAM param;
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
extern RESORT_DATA_MANAGER * RESORT_DATA_MANAGER_Init( const RESORT_DATA_MANAGER_INIT_PARAM *cp_param, HEAPID heapID )
{
  RESORT_DATA_MANAGER * p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_DATA_MANAGER) );
  RESORT_DATA_MANAGER_Setup( p_wk, cp_param );
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
 *	@param	RESORT_DATA_MANAGER_INIT_PARAM *cp_param 初期化酵素いたい
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_Setup( RESORT_DATA_MANAGER *p_wk, const RESORT_DATA_MANAGER_INIT_PARAM *cp_param )
{
  int i, j;

  p_wk->param = *cp_param;

  //クリア
  for( i = 0; i < RESORT_PLACEID_MAX; i++ )
  {
    RESORT_DATA_MANAGER_INFO *p_info = &p_wk->info[i];
    for( j = 0; j < RESORT_DATA_MANAGER_MAX; j++ )
    {
      p_info->people_type[j]  = RESORT_DATA_MANAGER_TYPE_NULL;
      p_info->people_index[j] = RESORT_DATA_MANAGER_INDEX_NULL;
    }
  }

  //配置情報検索
  //各バッファからデータを取得してどこにいるか調べる
  //全人物データにあるものは必ずpeopleIDとplaceIDがあるが、
  //バッファにあるものはないものがある
  
  //住人
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( cp_param->p_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_data ) )
    {
      u16 peopleID = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
      GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
      p_wk->info[ placeID ].people_type[ peopleID ]  = RESORT_DATA_MANAGER_TYPE_RESIDENT;
      p_wk->info[ placeID ].people_index[ peopleID ] = i;
    }
  }

  //滞在者
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetVisitor( cp_param->p_all, i );
    if( !RESORT_VISITOR_DATA_IsEmpty(p_data ) )
    {
      u16 peopleID = RESORT_VISITOR_DATA_GetData( p_data, RESORT_VISITOR_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_VISITOR_DATA_GetData( p_data, RESORT_VISITOR_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
      GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
      p_wk->info[ placeID ].people_type[ peopleID ] = RESORT_DATA_MANAGER_TYPE_VISITOR;
      p_wk->info[ placeID ].people_index[ peopleID ] = i;
    }
  }

  //秘書
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetSecretary( cp_param->p_all, i );
    if( !RESORT_SECRETARY_DATA_IsEmpty( p_data ) )
    {
      u16 peopleID = RESORT_SECRETARY_DATA_GetData(p_data, RESORT_SECRETARY_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_SECRETARY_DATA_GetData(p_data,RESORT_SECRETARY_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
      GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
      p_wk->info[ placeID ].people_type[peopleID] = RESORT_DATA_MANAGER_TYPE_SECRETARY;
      p_wk->info[ placeID ].people_index[peopleID] = i;
    }
  }

  //セーブ受信バッファ
  if( cp_param->p_recv_sv_buff )
  {
    for( i = 0; i < RESORT_RECV_BUFF_GetMax(cp_param->p_recv_sv_buff); i++ )
    {
      RESORT_RECV_DATA *p_data = RESORT_RECV_BUFF_GetData(cp_param->p_recv_sv_buff, i );
      if( !RESORT_RECV_DATA_IsEmpty(p_data ) )
      {
        u16 peopleID = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PEOPLE_ID, NULL );
        RESORT_PLACEID placeID = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PLACE_ID, NULL );

        if( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX )
        {
          GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );

          p_wk->info[ placeID ].people_type[peopleID] = RESORT_DATA_MANAGER_TYPE_RECV_SV_BUFF;
          p_wk->info[ placeID ].people_index[peopleID] = i;
        }
      }
    }
  }

  //一時受信バッファ
  if( cp_param->p_recv_tmp_buff )
  {
    for( i = 0; i < RESORT_RECV_BUFF_GetMax(cp_param->p_recv_tmp_buff); i++ )
    {
      RESORT_RECV_DATA *p_data = RESORT_RECV_BUFF_GetData(cp_param->p_recv_tmp_buff, i );
      if( !RESORT_RECV_DATA_IsEmpty(p_data ) )
      {
        u16 peopleID = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PEOPLE_ID, NULL );
        RESORT_PLACEID placeID = RESORT_RECV_DATA_GetData( p_data, RESORT_RECV_DATA_PARAM_PLACE_ID, NULL );

        if( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX )
        {
          GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
          p_wk->info[ placeID ].people_type[peopleID] = RESORT_DATA_MANAGER_TYPE_RECV_TMP_BUFF;
          p_wk->info[ placeID ].people_index[peopleID] = i;
        }
      }
    }
  }
  //代理人バッファ
  if( cp_param->p_proxy_buff )
  {
    for( i = 0; i < RESORT_PROXY_BUFF_GetMax(cp_param->p_proxy_buff); i++ )
    {
      RESORT_PROXY_DATA *p_data = RESORT_PROXY_BUFF_GetData( cp_param->p_proxy_buff, i );
      if( !RESORT_PROXY_DATA_IsEmpty(p_data ) )
      {
        u16 peopleID = RESORT_PROXY_DATA_GetData(p_data,RESORT_PROXY_DATA_PARAM_PEOPLEID );
        RESORT_PLACEID placeID = RESORT_PROXY_DATA_GetData(p_data,RESORT_PROXY_DATA_PARAM_PLACEID );
if( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX )
        {
          GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
          p_wk->info[ placeID ].people_type[peopleID]   = RESORT_DATA_MANAGER_TYPE_PROXY_BUFF;
          p_wk->info[ placeID ].people_index[peopleID]  = i;
        }
      }
    }
  }
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
    && cp_wk->info[ placeID ].people_type[ peopleID ] == RESORT_DATA_MANAGER_TYPE_NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief　配置データを取得
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	placeID 場所ID
 *	@param	peopleID　人物ID  最大RESORT_DATA_MANAGER_MAX
 *	@param	*p_type タイプ受け取り
 *	@param	*p_peopleID   人物ID
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_GetPlaceData( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u32 peopleID, RESORT_DATA_MANAGER_TYPE *p_type, u16 *p_data_idx )
{
  GF_ASSERT( peopleID<RESORT_DATA_MANAGER_MAX);
  GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
 
  *p_type       = cp_wk->info[ placeID ].people_type[ peopleID ];
  *p_data_idx   = cp_wk->info[ placeID ].people_index[ peopleID ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイプごとのデータアドレスを上k取り
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	type    タイプ
 *	@param	index   インデックス
 *
 *	@return タイプごとのデータアドレス
 */
//-----------------------------------------------------------------------------
void *RESORT_DATA_MANAGER_GetDataAdrs( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_MANAGER_TYPE type, u32 data_index )
{
  switch( type )
  {
  case RESORT_DATA_MANAGER_TYPE_RESIDENT:
    return RESORT_PEOPLE_ALL_DATA_GetResident( cp_wk->param.p_all, data_index );
  case RESORT_DATA_MANAGER_TYPE_VISITOR:
    return RESORT_PEOPLE_ALL_DATA_GetVisitor( cp_wk->param.p_all, data_index );
  case RESORT_DATA_MANAGER_TYPE_SECRETARY:
    return RESORT_PEOPLE_ALL_DATA_GetSecretary( cp_wk->param.p_all, data_index );
  case RESORT_DATA_MANAGER_TYPE_RECV_SV_BUFF:
    if( cp_wk->param.p_recv_sv_buff )
    {
      return RESORT_RECV_BUFF_GetData( cp_wk->param.p_recv_sv_buff, data_index );
    }
    return NULL;
  case RESORT_DATA_MANAGER_TYPE_RECV_TMP_BUFF:
    if( cp_wk->param.p_recv_tmp_buff )
    {
      return RESORT_RECV_BUFF_GetData( cp_wk->param.p_recv_tmp_buff, data_index );
    }
    return NULL;
  case RESORT_DATA_MANAGER_TYPE_PROXY_BUFF:
    if( cp_wk->param.p_proxy_buff )
    {
      return RESORT_PROXY_BUFF_GetData( cp_wk->param.p_proxy_buff, data_index );
    }
    return NULL;
  default:
    GF_ASSERT(0);
    return NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  場所タイプから人物タイプを取得
 *
 *	@param	RESORT_DATA_MANAGER_TYPE type   場所タイプ
 *
 *	@return 人物タイプ
 */
//-----------------------------------------------------------------------------
u32 RESORT_DATA_MANAGER_GetPeopleType( RESORT_DATA_MANAGER_TYPE type )
{
  switch( type )
  {
  case RESORT_DATA_MANAGER_TYPE_RESIDENT:
    return RESORT_PEOPLE_TYPE_RESIDENT;
  case RESORT_DATA_MANAGER_TYPE_VISITOR:
    return RESORT_PEOPLE_TYPE_VISITOR;
  case RESORT_DATA_MANAGER_TYPE_SECRETARY:
    return RESORT_PEOPLE_TYPE_SECRETARY;
  case RESORT_DATA_MANAGER_TYPE_RECV_SV_BUFF:
    return RESORT_PEOPLE_TYPE_TOURIST;
  case RESORT_DATA_MANAGER_TYPE_RECV_TMP_BUFF:
    return RESORT_PEOPLE_TYPE_TOURIST;
  case RESORT_DATA_MANAGER_TYPE_PROXY_BUFF:
    return RESORT_PEOPLE_TYPE_PROXY;
  default:
    GF_ASSERT(0);
    return NULL;
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
