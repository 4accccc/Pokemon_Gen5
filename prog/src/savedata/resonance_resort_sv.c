//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort_sv.c
 *	@brief  レゾナンスリゾート用セーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "gamesystem/game_data.h"

//セーブ
#include "savedata/save_tbl.h"

//外部公開
#include "savedata/resonance_resort_sv.h"

//=============================================================================
/**
 *					レゾナンスリゾート用セーブデータ
*/
//=============================================================================
//-------------------------------------
///	セーブデータ
//=====================================
struct _RESONANCE_RESORT_SV
{
  u8  recv_buff[RESORT_RECV_BUFF_SIZE( RESONANCE_RESORT_SV_RECV_BUFF_NUM )];
  u8  proxy_buff[RESORT_PROXY_BUFF_SIZE( RESONANCE_RESORT_SV_PROXY_BUFF_NUM )];
  u8  people_all_data[RESORT_PEOPLE_ALL_DATA_SIZE];
  u8  city_data[RESORT_CITY_DATA_SIZE];
};

SDK_COMPILER_ASSERT( sizeof(RESONANCE_RESORT_SV) < 5800 );

//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのワークサイズ取得
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
u32  RESONANCE_RESORT_SV_GetWorkSize(void)
{
  //TODO 仕様書によると5800バイト
  return 5800;//sizeof(RESONANCE_RESORT_SV);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのワーク初期化
 *
 *	@param	void *p_wk_adrs ワーク
 */
//-----------------------------------------------------------------------------
void RESONANCE_RESORT_SV_Init( void *p_wk_adrs )
{
  GFL_STD_MemClear( p_wk_adrs, sizeof(RESONANCE_RESORT_SV) );

  RESORT_RECV_BUFF_InitInstance( RESONANCE_RESORT_SV_GetDataBuff(p_wk_adrs), RESONANCE_RESORT_SV_RECV_BUFF_NUM );

  RESORT_PROXY_BUFF_InitInstance( RESONANCE_RESORT_SV_GetProxyBuff(p_wk_adrs), RESONANCE_RESORT_SV_PROXY_BUFF_NUM );

  RESORT_PEOPLE_ALL_DATA_InitInstance( RESONANCE_RESORT_SV_GetPeopleAllData(p_wk_adrs) );

  RESORT_CITY_DATA_InitInstance( RESONANCE_RESORT_SV_GetCityData(p_wk_adrs) );

}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートのセーブワーク取得
 *
 *	@param	SAVE_CONTROL_WORK *p_sv   セーブデータ
 *
 *	@return レゾナンスリゾートのセーブワーク
 */
//-----------------------------------------------------------------------------
RESONANCE_RESORT_SV *RESONANCE_RESORT_SV_GetWork( SAVE_CONTROL_WORK *p_sv )
{
  return SaveControl_DataPtrGet( p_sv , GMDATA_ID_RESORT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  データバッファ取得
 *
 *	@param	RESONANCE_RESORT_SV *   ワーク
 *
 *	@return レゾナンスリゾートのデータバッファ取得
 */
//-----------------------------------------------------------------------------
RESORT_RECV_BUFF * RESONANCE_RESORT_SV_GetDataBuff( RESONANCE_RESORT_SV *p_wk )
{
  return (RESORT_RECV_BUFF *)p_wk->recv_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief　代理人バッファ取得
 *
 *	@param	RESONANCE_RESORT_SV *p_wk ワーク
 *
 *	@return　代理人バッファ
 */
//-----------------------------------------------------------------------------
RESORT_PROXY_BUFF * RESONANCE_RESORT_SV_GetProxyBuff( RESONANCE_RESORT_SV *p_wk )
{
  return (RESORT_PROXY_BUFF *)p_wk->proxy_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief　人物全データ取得
 *
 *	@param	RESONANCE_RESORT_SV *p_wk   ワーク
 *
 *	@return 人物全データ取得
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_ALL_DATA * RESONANCE_RESORT_SV_GetPeopleAllData( RESONANCE_RESORT_SV *p_wk )
{
  return (RESORT_PEOPLE_ALL_DATA *)p_wk->people_all_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief　街データ取得
 *
 *	@param	RESONANCE_RESORT_SV *p_wk   ワーク
 *
 *	@return 街データ取得
 */
//-----------------------------------------------------------------------------
RESORT_CITY_DATA * RESONANCE_RESORT_SV_GetCityData( RESONANCE_RESORT_SV *p_wk )
{
  return (RESORT_CITY_DATA *)p_wk->city_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  全データをチェックし、データをマージorセットor破棄する
 *
 *	@param	RESONANCE_RESORT_SV *p_wk ワーク
 *	@param	*p_gamedata               ゲームデータ
 *	@param	void *cp_data_adrs        データアドレス
 *	@param	type                      データ種類
 *
 *	@return TRUEデータマージorセット  FALSEデータ破棄
 */
//-----------------------------------------------------------------------------
RESORT_SETDATA RESONANCE_RESORT_SV_SetData( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type )
{

  typedef BOOL (*IS_VALID)(  const void *cp_data_adrs );

  typedef BOOL (*MERGE_RECV_BUFF)( RESORT_RECV_BUFF *p_wk, const void *cp_data_adrs );
  typedef BOOL (*MERGE_PROXY_BUFF)( RESORT_PROXY_BUFF *p_wk, const void *cp_data_adrs );
  typedef BOOL (*ADD_RECV_DATA)( RESORT_RECV_BUFF *p_wk, const void *cp_data_adrs );
  typedef BOOL (*ADD_PROXY_DATA)( RESORT_PROXY_BUFF *p_wk, const void *cp_data_adrs );

  //チェック順
  typedef enum
  {
    RESRT_MERGE_ORDER_PROXY_RECV,
    RESRT_MERGE_ORDER_RECV_PROXY,
    RESRT_MERGE_ORDER_MAX,
  }RESRT_MERGE_ORDER;

  static const struct
  {
    IS_VALID          IsValid;
    MERGE_RECV_BUFF   MergeRecvBuff;
    MERGE_PROXY_BUFF  MergeProxyBuff;
    ADD_RECV_DATA     AddRecvData;
    ADD_PROXY_DATA    AddProxyData;
    RESRT_MERGE_ORDER order;
  }sc_data_function_tbl[] =
  {
    //ビーコンデータ  代理人→ビーコン
    {
      (IS_VALID)RESORT_BEACON_DATA_IsValid,
      (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeBeacon,
      (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeBeacon,
      (ADD_RECV_DATA)RESORT_RECV_BUFF_AddBeacon,
      NULL,
      RESRT_MERGE_ORDER_PROXY_RECV,
    },
    //受信データ      代理人→ビーコン
    {
      (IS_VALID)RESORT_RECV_DATA_IsValid,
      (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeRecv,
      (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeRecv,
      (ADD_RECV_DATA)RESORT_RECV_BUFF_AddRecv,
      NULL,
      RESRT_MERGE_ORDER_PROXY_RECV,
    },
    //代理人データ    ビーコン→代理人
    {
      (IS_VALID)RESORT_PROXY_DATA_IsValid,
      (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeProxy,
      (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeProxy,
      NULL,
      (ADD_PROXY_DATA)RESORT_PROXY_BUFF_Add,
      RESRT_MERGE_ORDER_RECV_PROXY,
    },
  };

  RESORT_RECV_BUFF *p_sv_buff   =  RESONANCE_RESORT_SV_GetDataBuff( p_wk );
  RESORT_RECV_BUFF *p_temp_buff =  *(RESORT_RECV_BUFF **)GAMEDATA_GetResortBuffer( p_gamedata );
  RESORT_PROXY_BUFF *p_proxy_buff  =  RESONANCE_RESORT_SV_GetProxyBuff( p_wk );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_wk );
  RESORT_CITY_DATA *cp_city = RESONANCE_RESORT_SV_GetCityData( p_wk );


  //不正ならば何もしない
  if( !sc_data_function_tbl[type].IsValid( cp_data_adrs ) )
  {
    return RESORT_SETDATA_NONE;
  }

  //市民データのマージ
  if( RESORT_PEOPLE_ALL_DATA_MergeData( p_all_people, cp_data_adrs, type ) )
  {
    return RESORT_SETDATA_MERGE;
  }
  else
  {
    int i;
    //バッファのマージ
    //どちらのバッファを先にチェックするか順番が違う
    RESRT_MERGE_ORDER order = sc_data_function_tbl[type].order;
    for( i = 0; i < RESRT_MERGE_ORDER_MAX; i++ )
    {
      switch( order )
      {
      case RESRT_MERGE_ORDER_PROXY_RECV:
        if( sc_data_function_tbl[type].MergeRecvBuff( p_sv_buff, cp_data_adrs ) )
        {
          return RESORT_SETDATA_MERGE;
        }
        if( p_temp_buff )
        {
          if( sc_data_function_tbl[type].MergeRecvBuff( p_temp_buff, cp_data_adrs ) )
          {
            return RESORT_SETDATA_MERGE;
          }
        }
        order ^=  1;
        break;
      case RESRT_MERGE_ORDER_RECV_PROXY:
        if( sc_data_function_tbl[type].MergeProxyBuff( p_proxy_buff, cp_data_adrs ) )
        {
          return RESORT_SETDATA_MERGE;
        }
        order ^=  1;
        break;
      default:
        GF_ASSERT(0);
      }
    }
  }

  if( sc_data_function_tbl[type].AddRecvData )
  {
    BOOL is_set = sc_data_function_tbl[type].AddRecvData( p_sv_buff, cp_data_adrs );
    if( !is_set && p_temp_buff )
    {
      is_set = sc_data_function_tbl[type].AddRecvData( p_temp_buff, cp_data_adrs );
    }
    if( is_set )
    {
      return RESORT_SETDATA_SET;
    }
  }
  else
  {
    if( sc_data_function_tbl[type].AddProxyData( p_proxy_buff, cp_data_adrs ) )
    {
      return RESORT_SETDATA_SET;
    }
  }

  return RESORT_SETDATA_NONE;
}
