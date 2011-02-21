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


//TODO
static RESONANCE_RESORT_SV  s_sv  = {RESONANCE_RESORT_SV_RECV_BUFF_NUM,0,};

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
  return sizeof(RESONANCE_RESORT_SV);
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
  GFL_STD_MemClear( p_wk_adrs, RESONANCE_RESORT_SV_GetWorkSize() );

  //TODO
  RESORT_RECV_BUFF_InitInstance( RESONANCE_RESORT_SV_GetDataBuff(&s_sv), RESONANCE_RESORT_SV_RECV_BUFF_NUM );

  RESORT_PROXY_BUFF_InitInstance( RESONANCE_RESORT_SV_GetProxyBuff(&s_sv), RESONANCE_RESORT_SV_PROXY_BUFF_NUM );

  RESORT_CITY_DATA_InitInstance( RESONANCE_RESORT_SV_GetCityData(&s_sv) );

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
  return &s_sv;
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
