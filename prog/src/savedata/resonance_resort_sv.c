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

#include "debug/debug_nagihashi.h"
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


static RESORT_SETDATA Resonance_Resort_Sv_SetData_01( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type );
static RESORT_SETDATA Resonance_Resort_Sv_SetData_02( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type );

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
  //仕様書によると5800バイト->セーブをずらさないために固定になりました。
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
  RESORT_SETDATA ret = Resonance_Resort_Sv_SetData_01( p_wk, p_gamedata, cp_data_adrs, type );

  if( ret == RESORT_SETDATA_NEXT )
  {
    ret = Resonance_Resort_Sv_SetData_02( p_wk, p_gamedata, cp_data_adrs, type );
  }

  return ret;
}

static int s_merge_seq  = 0;
//----------------------------------------------------------------------------
/**
 *	@brief  分割セット処理開始
 *
 *	@param	RESONANCE_RESORT_SV *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESONANCE_RESORT_SV_SetDataStart( RESONANCE_RESORT_SV *p_wk )
{
  s_merge_seq  = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  分割セット処理メイン
 *
 *	@param	RESONANCE_RESORT_SV *p_wk
 *	@param	*p_gamedata
 *	@param	void *cp_data_adrs
 *	@param	type
 *	@param	*p_ret 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
BOOL RESONANCE_RESORT_SV_SetDataMain( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type, RESORT_SETDATA *p_ret )
{
  RESORT_SETDATA ret;
  switch( s_merge_seq )
  {
  case 0:
    ret = Resonance_Resort_Sv_SetData_01( p_wk, p_gamedata, cp_data_adrs, type );
    if( ret == RESORT_SETDATA_NEXT )
    {
      s_merge_seq++;
    }
    else
    {
      *p_ret = ret;
      return TRUE;
    }
    break;
  case 1:
    ret = Resonance_Resort_Sv_SetData_02( p_wk, p_gamedata, cp_data_adrs, type );
    *p_ret = ret;
    return TRUE; 

  default:
    GF_ASSERT(0);
  }


  return FALSE;
}



typedef BOOL (*IS_VALID)(  const void *cp_data_adrs );

typedef RESORT_MERGE_RESULT (*MERGE_RECV_BUFF)( RESORT_RECV_BUFF *p_wk, const void *cp_data_adrs );
typedef RESORT_MERGE_RESULT (*MERGE_PROXY_BUFF)( RESORT_PROXY_BUFF *p_wk, const void *cp_data_adrs );
typedef RESORT_ADD_RESULT (*ADD_RECV_DATA)( RESORT_RECV_BUFF *p_wk, const void *cp_data_adrs );
typedef RESORT_ADD_RESULT (*ADD_PROXY_DATA)( RESORT_PROXY_BUFF *p_wk, const void *cp_data_adrs );

static const struct
{
  IS_VALID          IsValid;
  MERGE_RECV_BUFF   MergeRecvBuff;
  MERGE_PROXY_BUFF  MergeProxyBuff;
  ADD_RECV_DATA     AddRecvData;
  ADD_PROXY_DATA    AddProxyData;
}sc_data_function_tbl[] =
{
  //ビーコンデータ 
  {
    (IS_VALID)RESORT_BEACON_DATA_IsValid,
    (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeBeacon,
    (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeBeacon,
    (ADD_RECV_DATA)RESORT_RECV_BUFF_AddBeacon,
    NULL,
  },
  //受信データ
  {
    (IS_VALID)RESORT_RECV_DATA_IsValid,
    (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeRecv,
    (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeRecv,
    (ADD_RECV_DATA)RESORT_RECV_BUFF_AddRecv,
    NULL,
  },
  //代理人データ 
  {
    (IS_VALID)RESORT_PROXY_DATA_IsValid,
    (MERGE_RECV_BUFF)RESORT_RECV_BUFF_MergeProxy,
    (MERGE_PROXY_BUFF)RESORT_PROXY_BUFF_MergeProxy,
    NULL,
    (ADD_PROXY_DATA)RESORT_PROXY_BUFF_Add,
  },
};

static RESORT_SETDATA Resonance_Resort_Sv_SetData_01( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type )
{
  RESORT_RECV_BUFF *p_sv_buff   =  RESONANCE_RESORT_SV_GetDataBuff( p_wk );
  RESORT_RECV_BUFF *p_temp_buff =  *(RESORT_RECV_BUFF **)GAMEDATA_GetResortBuffer( p_gamedata );
  RESORT_PROXY_BUFF *p_proxy_buff  =  RESONANCE_RESORT_SV_GetProxyBuff( p_wk );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_wk );
  RESORT_CITY_DATA *cp_city = RESONANCE_RESORT_SV_GetCityData( p_wk );

//DEBUG_TICK_PrintStart;
  //もしチュートリアル中で、代理人ならば、２こ開けておく
  if( RESORT_CITY_DATA_GetData(cp_city, RESORT_CITY_DATA_PARAM_IS_TUTORIAL, NULL ) 
      && type == RESORT_MERGE_TYPE_PROXY 
      && RESORT_PROXY_BUFF_GetCnt(p_proxy_buff) + 2 >= RESORT_PROXY_BUFF_GetMax(p_proxy_buff) )
  {
    return RESORT_SETDATA_NONE;
  }
//DEBUG_TICK_PrintEnd;


//DEBUG_TICK_PrintStart;
  //不正ならば何もしない
  if( !sc_data_function_tbl[type].IsValid( cp_data_adrs ) )
  {
    return RESORT_SETDATA_NONE;
  }
//DEBUG_TICK_PrintEnd;

//DEBUG_TICK_PrintStart;
  {
    int i;
    //バッファのマージ
    {
      //旅行者バッファマージ
      {
        RESORT_MERGE_RESULT ret = sc_data_function_tbl[type].MergeRecvBuff( p_sv_buff, cp_data_adrs );
        if( ret == RESORT_MERGE_RESULT_OK )
        {
          return RESORT_SETDATA_MERGE;
        }
        else if( ret == RESORT_MERGE_RESULT_DISABLE )
        {
          return RESORT_SETDATA_NONE;
        }
      }

      //拡張バッファマージ
      if( p_temp_buff )
      {
        RESORT_MERGE_RESULT ret = sc_data_function_tbl[type].MergeRecvBuff( p_temp_buff, cp_data_adrs );
        if( ret == RESORT_MERGE_RESULT_OK )
        {
          return RESORT_SETDATA_MERGE;
        }
        else if( ret == RESORT_MERGE_RESULT_DISABLE )
        {
          return RESORT_SETDATA_NONE;
        }
      }

      //代理人バッファマージ
      {
        RESORT_MERGE_RESULT ret = sc_data_function_tbl[type].MergeProxyBuff( p_proxy_buff, cp_data_adrs );
        if( ret == RESORT_MERGE_RESULT_OK )
        {
          return RESORT_SETDATA_MERGE;
        }
        else if( ret == RESORT_MERGE_RESULT_DISABLE )
        {
          return RESORT_SETDATA_NONE;
        }
      }
    }
  }
//DEBUG_TICK_PrintEnd;

  return RESORT_SETDATA_NEXT;
}

static RESORT_SETDATA Resonance_Resort_Sv_SetData_02( RESONANCE_RESORT_SV *p_wk, GAMEDATA *p_gamedata, const void *cp_data_adrs, RESORT_MERGE_TYPE type )
{
  RESORT_RECV_BUFF *p_sv_buff   =  RESONANCE_RESORT_SV_GetDataBuff( p_wk );
  RESORT_RECV_BUFF *p_temp_buff =  *(RESORT_RECV_BUFF **)GAMEDATA_GetResortBuffer( p_gamedata );
  RESORT_PROXY_BUFF *p_proxy_buff  =  RESONANCE_RESORT_SV_GetProxyBuff( p_wk );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_wk );
  RESORT_CITY_DATA *cp_city = RESONANCE_RESORT_SV_GetCityData( p_wk );


//DEBUG_TICK_PrintStart;
  //市民データのマージ  ※ビーコンがバッファに入った場合、先にバッファを調査した方が早くおわることがおおい
  {
    RESORT_MERGE_RESULT ret = RESORT_PEOPLE_ALL_DATA_MergeData( p_all_people, cp_data_adrs, type );
    if( ret == RESORT_MERGE_RESULT_OK )
    {
      return RESORT_SETDATA_MERGE;
    }
    else if( ret == RESORT_MERGE_RESULT_DISABLE )
    {
      return RESORT_SETDATA_NONE;
    }
  }
//DEBUG_TICK_PrintEnd;

//DEBUG_TICK_PrintStart;
  //バッファへのセット
  if( sc_data_function_tbl[type].AddRecvData )
  {
    RESORT_ADD_RESULT add = sc_data_function_tbl[type].AddRecvData( p_sv_buff, cp_data_adrs );
    if( (add == RESORT_ADD_RESULT_NG) && p_temp_buff )
    {
      add = sc_data_function_tbl[type].AddRecvData( p_temp_buff, cp_data_adrs );
    }
    if( add == RESORT_ADD_RESULT_OK )
    {
      return RESORT_SETDATA_SET;
    }
  }
  else
  {
    if( sc_data_function_tbl[type].AddProxyData( p_proxy_buff, cp_data_adrs ) == RESORT_ADD_RESULT_OK )
    {
      return RESORT_SETDATA_SET;
    }
  }
//DEBUG_TICK_PrintEnd;

  return RESORT_SETDATA_NONE;
}
