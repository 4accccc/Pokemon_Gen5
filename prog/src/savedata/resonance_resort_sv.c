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
  u8  data_buff[RESORT_DATA_BUFF_SIZE( RESONANCE_RESORT_SV_DATA_BUFF_NUM )];
};


//TODO
static RESONANCE_RESORT_SV  s_sv  = {RESONANCE_RESORT_SV_DATA_BUFF_NUM,0,};

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
  RESORT_DATA_BUFF_InitInstance( RESONANCE_RESORT_SV_GetDataBuff(&s_sv), RESONANCE_RESORT_SV_DATA_BUFF_NUM );
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
RESORT_DATA_BUFF * RESONANCE_RESORT_SV_GetDataBuff( RESONANCE_RESORT_SV *p_wk )
{
  return (void*)p_wk->data_buff;
}
