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
#include "arc/arc_def.h"
#include "arc/script_message.naix"

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
///	データマネージャー
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
  p_wk->param = *cp_param;
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
  const RESORT_DATA_MANAGER_INIT_PARAM *cp_param = &p_wk->param;
  int i, j;


  //クリア
  for( i = 0; i < RESORT_PLACEID_MAX; i++ )
  {
    RESORT_DATA_MANAGER_INFO *p_info = &p_wk->info[i];
    //人物IDクリア
    for( j = 0; j < RESORT_DATA_MANAGER_MAX; j++ )
    {
      p_info->people_type[j]  = RESORT_DATA_MANAGER_TYPE_NULL;
      p_info->people_index[j] = RESORT_DATA_MANAGER_INDEX_NULL;
    }
  }

  //配置情報検索
  //各バッファからデータを取得してどこにいるか調べる
  //全人物データにあるものは必ずpeopleIDとplaceIDがあるが、
  //バッファのはないものがある
  
  //住人
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( cp_param->p_all_people, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_data ) )
    {
      u16 peopleID = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
      GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
      p_wk->info[ placeID ].people_type[ peopleID ]  = RESORT_DATA_MANAGER_TYPE_RESIDENT;
      p_wk->info[ placeID ].people_index[ peopleID ] = i;
    }
  }

  //滞在者
  for( i = 0; i < RESORT_VISITOR_DATA_NUM; i++ )
  {
    RESORT_VISITOR_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetVisitor( cp_param->p_all_people, i );
    if( !RESORT_VISITOR_DATA_IsEmpty(p_data ) )
    {
      u16 peopleID = RESORT_VISITOR_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_VISITOR_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

      GF_ASSERT(p_wk->info[ placeID ].people_type[peopleID] == RESORT_DATA_MANAGER_TYPE_NULL );
      GF_ASSERT( RESORT_PLACEID_START <= placeID  && placeID < RESORT_PLACEID_MAX );
      p_wk->info[ placeID ].people_type[ peopleID ] = RESORT_DATA_MANAGER_TYPE_VISITOR;
      p_wk->info[ placeID ].people_index[ peopleID ] = i;
    }
  }

  //秘書
  for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
  {
    RESORT_SECRETARY_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetSecretary( cp_param->p_all_people, i );
    if( !RESORT_SECRETARY_DATA_IsEmpty( p_data ) )
    {
      u16 peopleID = RESORT_SECRETARY_DATA_GetData(p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_PLACEID placeID = RESORT_SECRETARY_DATA_GetData(p_data,RESORT_DATA_PARAM_PLACE_ID, NULL );

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
        u16 peopleID = RESORT_RECV_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
        RESORT_PLACEID placeID = RESORT_RECV_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

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
        u16 peopleID = RESORT_RECV_DATA_GetData( p_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
        RESORT_PLACEID placeID = RESORT_RECV_DATA_GetData( p_data, RESORT_DATA_PARAM_PLACE_ID, NULL );

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
        u16 peopleID = RESORT_PROXY_DATA_GetData(p_data,RESORT_DATA_PARAM_PEOPLE_ID, NULL );
        RESORT_PLACEID placeID = RESORT_PROXY_DATA_GetData(p_data,RESORT_DATA_PARAM_PLACE_ID, NULL );
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
 
  GF_ASSERT_MSG( !RESORT_DATA_MANAGER_IsPlaceEmpty( cp_wk, placeID, peopleID ), "空のデータをとってはいけない\n" );

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
    return RESORT_PEOPLE_ALL_DATA_GetResident( cp_wk->param.p_all_people, data_index );
  case RESORT_DATA_MANAGER_TYPE_VISITOR:
    return RESORT_PEOPLE_ALL_DATA_GetVisitor( cp_wk->param.p_all_people, data_index );
  case RESORT_DATA_MANAGER_TYPE_SECRETARY:
    return RESORT_PEOPLE_ALL_DATA_GetSecretary( cp_wk->param.p_all_people, data_index );
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
typedef enum
{
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR,
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_SECRETARY,
  RESORT_DATA_MANAGER_CHECKCONVERT_DST_RESIDENT,
}RESORT_DATA_MANAGER_CHECKCONVERT_DST;
static BOOL Resort_Data_Manager_CheckConvert( const RESORT_DATA_MANAGER *cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST param, RESORT_PLACEID placeID, u16 *p_peopleID, void **pp_dst )
{
  //場所の空きを検索
  *p_peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( cp_wk, placeID );

  //バッファの空きを検索
  switch( param )
  {
  case RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR:
    *pp_dst = RESORT_PEOPLE_ALL_DATA_SearchEmptyVisitor( cp_wk->param.p_all_people );
    break;
  case RESORT_DATA_MANAGER_CHECKCONVERT_DST_SECRETARY:
    *pp_dst = RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( cp_wk->param.p_all_people );
    break;
  case RESORT_DATA_MANAGER_CHECKCONVERT_DST_RESIDENT:
    *pp_dst = RESORT_PEOPLE_ALL_DATA_SearchEmptyResident( cp_wk->param.p_all_people );
    break;
  default:
    GF_ASSERT(0);
  }

  if( *p_peopleID == RESORT_PEOPLE_ID_NULL )
  {
    OS_Printf( "施設内位置が空いていない\n" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  if( *pp_dst == NULL )
  {
    OS_Printf( "バッファが空いていない" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から住民へ転職可能かチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src 受信データ
 *
 *	@return TRUEで可能  FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsRecvToResident( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src )
{
  //場所が開いているか？
  RESORT_PLACEID placeID = RESORT_PEOPLE_ALL_DATA_SearchEmptyInstitutionPlaceID( cp_wk->param.p_all_people );
  u16 peopleID;
  void *p_dst_adrs;
  if( placeID == RESORT_PLACEID_NONE )
  {
    OS_Printf( "施設内位置が空いていない\n" );
    return FALSE;
  }
  else
  {
    return Resort_Data_Manager_CheckConvert( cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_RESIDENT, placeID, &peopleID, &p_dst_adrs );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から住民へデータ変換
 *
 *	@param	const RESORT_DATA_MANAGER *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src 受信データ
 *	@param	type  転職タイプ　RESORT_RESIDENT_TYPE_xxx
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_RecvToResident( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src, u16 type )
{
  RESORT_PLACEID placeID = RESORT_PEOPLE_ALL_DATA_SearchEmptyInstitutionPlaceID( p_wk->param.p_all_people );
  u16 peopleID;
  void *p_dst_adrs;
  RESORT_RESIDENT_DATA *p_dst;

  if( placeID == RESORT_PLACEID_NONE )
  {
    //IsRecvToResidentでチェックしていない？
    OS_Printf( "場所が空いていない\n" );
    GF_ASSERT(0);
  }

  if( Resort_Data_Manager_CheckConvert( p_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_RESIDENT, placeID, &peopleID, &p_dst_adrs ) )
  {
    RESORT_INSTITUTION_DATA *p_institution;

    p_dst = p_dst_adrs;

    //データを転職
    RESORT_RESIDENT_DATA_SetRecv( p_dst, p_src, peopleID, placeID, type );

    //施設に情報を設定
    p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_dst );
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, type);

    //アップデート
    RESORT_DATA_MANAGER_Update( p_wk );
  }
  else
  {
    //IsRecvToResidentでチェックしていない？
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から滞在者になれるか
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src  受信データ
 *
 *	@return TRUEで可能  FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsRecvToVisitor( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src )
{
  RESORT_PLACEID placeID = RESORT_PEOPLE_ALL_DATA_SearchEmptyCondominiumPlaceID( cp_wk->param.p_all_people );
  u16 peopleID;
  void *p_dst;

  if( placeID == RESORT_PLACEID_NONE )
  {
    OS_Printf( "施設内位置が空いていない\n" );
    return FALSE;
  }
  else
  {
    return Resort_Data_Manager_CheckConvert( cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR, placeID, &peopleID, &p_dst );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から滞在者にデータ変換
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *	@param	*p_src  受信データ
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_RecvToVisitor( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src )
{
  RESORT_PLACEID placeID = RESORT_PEOPLE_ALL_DATA_SearchEmptyCondominiumPlaceID( p_wk->param.p_all_people );
  u16 peopleID;
  RESORT_VISITOR_DATA *p_dst;
  void *p_dst_adrs;

  if( placeID == RESORT_PLACEID_NONE )
  {
    //IsRecvToResidentでチェックしていない？
    OS_Printf( "場所が空いていない\n" );
    GF_ASSERT(0);
  }

  if( Resort_Data_Manager_CheckConvert( p_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR, placeID, &peopleID, &p_dst_adrs ) )
  {  
    p_dst = p_dst_adrs;
    
    //滞在者へ //TODO
    RESORT_VISITOR_DATA_SetRecv( p_dst, p_src, peopleID, placeID, 0 );

    //アップデート
    RESORT_DATA_MANAGER_Update( p_wk );
  }
  else
  {
    //IsRecvToResidentでチェックしていない？
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  旅行者を秘書に出来るかチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src
 *
 *	@return　TRUEで可能 FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsRecvToSecretary( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RECV_DATA *cp_src )
{
  RESORT_PLACEID placeID = RESORT_PLACEID_FIELD; //TODO
  u16 peopleID;
  void *p_dst;
  return Resort_Data_Manager_CheckConvert( cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_SECRETARY, placeID, &peopleID, &p_dst );
}

//----------------------------------------------------------------------------
/**
 *	@brief　旅行者を秘書にコンバート
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *	@param	*p_src  旅行者
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_RecvToSecretary( RESORT_DATA_MANAGER *p_wk, RESORT_RECV_DATA *p_src )
{
  RESORT_PLACEID placeID = RESORT_PLACEID_FIELD; //TODO
  u16 peopleID;
  RESORT_SECRETARY_DATA *p_dst;
  void *p_dst_adrs;

  if( Resort_Data_Manager_CheckConvert( p_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_SECRETARY, placeID, &peopleID, &p_dst_adrs ) )
  {
    p_dst = p_dst_adrs;

    //受信
    //TODO 0
    RESORT_SECRETARY_DATA_SetRecv( p_dst, p_src, peopleID, placeID, 0 );

    //更新
    RESORT_DATA_MANAGER_Update( p_wk );
  }
  else
  {
    //チェックしてない？
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  住民を滞在者に出来るかチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src
 *
 *	@return　TRUEで可能 FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsResidentToVisitor( const RESORT_DATA_MANAGER *cp_wk, const RESORT_RESIDENT_DATA *cp_src )
{
  RESORT_PLACEID placeID = RESORT_PLACEID_CONDOMINIUM_00; //TODO
  u16 peopleID;
  void *p_dst;
  return Resort_Data_Manager_CheckConvert( cp_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR, placeID, &peopleID, &p_dst );
}

//----------------------------------------------------------------------------
/**
 *	@brief　住民を滞在者にコンバート
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *	@param	*p_src  住民
 */
//-----------------------------------------------------------------------------
void RESORT_DATA_MANAGER_ResidentToVisitor( RESORT_DATA_MANAGER *p_wk, RESORT_RESIDENT_DATA *p_src )
{
  //コンドミニアムに滞在者は一人づつする処理をいれる
  RESORT_PLACEID placeID = RESORT_PLACEID_CONDOMINIUM_00; //TODO
  u16 peopleID;
  void *p_dst;
  if( Resort_Data_Manager_CheckConvert( p_wk, RESORT_DATA_MANAGER_CHECKCONVERT_DST_VISITOR, placeID, &peopleID, &p_dst ) )
  {
    //TODO 
    RESORT_VISITOR_DATA_SetRecv( p_dst, p_src, peopleID, placeID, 0 );

    //更新
    RESORT_DATA_MANAGER_Update( p_wk );
  }
  else
  {
    //チェックしてない？
    GF_ASSERT(0);
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
 *	@brief  住民データを探索
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	placeID　 場所ID
 *	@param	peopleID  人物ID
 *
 *	@return データアドレス
 */
//-----------------------------------------------------------------------------
RESORT_RESIDENT_DATA * RESORT_DATA_MANAGER_SearchResident( const RESORT_DATA_MANAGER *cp_wk, RESORT_PLACEID placeID, u16 peopleID )
{
  int i;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( cp_wk->param.p_all_people, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      RESORT_PLACEID placeID = RESORT_RESIDENT_DATA_GetData(p_resident, RESORT_DATA_PARAM_PLACE_ID, NULL );
      u16 peopleID = RESORT_RESIDENT_DATA_GetData(p_resident, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

      if( placeID == placeID
          && peopleID == peopleID )
      {
        return p_resident;
      }
    }
  }

  return NULL;
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
STRBUF * RESORT_DATA_MANAGER_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_RESIDENT_DATA *cp_data, HEAPID heapID )
{
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];

  const RESORT_BINARY_SHOP_DATA *cp_shop = RESORT_BINARY_SHOP_SearchDataByData( cp_wk, (RESORT_RESIDENT_DATA *)cp_data );

  WORDSET *p_wordset = WORDSET_Create( GFL_HEAP_LOWID(heapID) );

  GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
  u16 shop_name = RESORT_BINARY_SHOP_DATA_GetData( cp_shop, RESORT_BINARY_SHOP_PARAM_NAME );
  STRBUF *p_tmp = GFL_MSG_CreateString( p_msg, shop_name );
  STRBUF *p_name_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
  STRBUF *p_str = GFL_STR_CreateBuffer( 64, heapID );
  u32 sex = RESORT_RESIDENT_DATA_GetData( cp_data, RESORT_DATA_PARAM_SEX, NULL );
  RESORT_RESIDENT_DATA_GetData( cp_data, RESORT_DATA_PARAM_NAME, name );
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
  //RESORT_DATA_TYPE_VISITOR
  {
    (InitFunction)RESORT_VISITOR_DATA_Init,
    (ExitFunction)RESORT_VISITOR_DATA_Exit,
    (InitInstanceFunction)RESORT_VISITOR_DATA_InitInstance,
    (IsEmptyFunction)RESORT_VISITOR_DATA_IsEmpty,
    (GetDataFunction)RESORT_VISITOR_DATA_GetData,
    (SetDataFunction)RESORT_VISITOR_DATA_SetData,
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
 *	@param	type            タイプ
 *
 *	@return　データ抽象操作
 */
//-----------------------------------------------------------------------------
RESORT_ABSTRUCT_DATA * RESORT_ABSTRUCT_DATA_Init( HEAPID heapID, void *p_data_adrs, RESORT_DATA_TYPE type )
{
  RESORT_ABSTRUCT_DATA *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_ABSTRUCT_DATA) );

  RESORT_ABSTRUCT_DATA_Setup( p_wk, p_data_adrs, type );

  if( p_data_adrs == NULL )
  {
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
void RESORT_ABSTRUCT_DATA_Setup( RESORT_ABSTRUCT_DATA *p_wk, void *p_data_adrs, RESORT_DATA_TYPE type )
{
  GF_ASSERT( p_data_adrs != NULL );
  GF_ASSERT( p_wk->is_create == FALSE );
  GF_ASSERT( type < RESORT_DATA_TYPE_MAX );
  p_wk->cp_func   = &sc_abstruct_function[ type ];

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
 *	@brief  人物タイプからデータタイプへの変換
 *
 *	@param	u32 people_type 人物タイプ
 */
//-----------------------------------------------------------------------------
RESORT_DATA_TYPE PeopleTypeToResortDataType( u32 people_type )
{
  switch( people_type )
  {
  case RESORT_PEOPLE_TYPE_TOURIST:
    return RESORT_DATA_TYPE_RECV;
  case RESORT_PEOPLE_TYPE_PROXY:
    return RESORT_DATA_TYPE_PROXY;
  case RESORT_PEOPLE_TYPE_SECRETARY:
    return RESORT_DATA_TYPE_SECRETARY;
  case RESORT_PEOPLE_TYPE_RESIDENT:
    return RESORT_DATA_TYPE_RESIDENT;
  case RESORT_PEOPLE_TYPE_VISITOR:
    return RESORT_DATA_TYPE_VISITOR;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
