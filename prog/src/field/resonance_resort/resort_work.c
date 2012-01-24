//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_work.c
 *	@brief  リゾート常駐ワーク
 *	@author	Toru=Nagihashi
 *	@data		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "savedata/record.h"
#include "savedata/misc.h"
#include "field/resort_work.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	リゾートワーク
//=====================================
struct _RESORT_WORK
{
  u32 data[RESORT_WORK_GET_PARAM_MAX];
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  リゾートワーク初期化
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	*p_sv   セーブデータ
 *
 *	@return リゾートワーク
 */
//-----------------------------------------------------------------------------
RESORT_WORK* RESORT_WORK_Init( HEAPID heapID, SAVE_CONTROL_WORK *p_sv )
{
  RESORT_WORK* p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_WORK) );
  RESORT_WORK_ReLoadData( p_wk, p_sv );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リゾートワーク破棄
 *
 *	@param	RESORT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_WORK_Exit( RESORT_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ再読み込み
 *
 *	@param	RESORT_WORK *p_wk ワーク
 *	@param	*p_sv   セーブデータ
 */
//-----------------------------------------------------------------------------
void RESORT_WORK_ReLoadData( RESORT_WORK *p_wk, SAVE_CONTROL_WORK *p_sv )
{
  RECORD *p_record = SaveData_GetRecord(p_sv);
  const MISC *cp_misc = SaveData_GetMiscConst(p_sv);

  {
    u64 value = 0;
    value += RECORD_Get(p_record, RECID_COMM_TRADE );
    value += RECORD_Get(p_record, RECID_WIFI_TRADE );
    value += RECORD_Get(p_record, RECID_IRC_TRADE );
    p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE ] = MATH_CLAMP( value, 0, RESORT_WORK_RNKDATA_MAX);
  }

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_NICKNAME ]  =
    RECORD_Get(p_record, RECID_NICKNAME );

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_JA_TOURIST ]  =
    RECORD_Get(p_record, RECID_RESORT_GUID_CNT ) 
    + RECORD_Get(p_record, RECID_RESORT_SHOP_MAKE_CNT );

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_BUY_MONEY ] =
    RECORD_Get(p_record, RECID_SHOPPING_MONEY );

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_CROSSING ]  =
    MISC_CrossComm_GetSuretigaiCount( cp_misc );

  {
    u64 value = 0;
    value += RECORD_Get(p_record, RECID_COMM_BATTLE );
    value += RECORD_Get(p_record, RECID_WIFI_BATTLE );
    value += RECORD_Get(p_record, RECID_IRC_BATTLE );
    p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_NET_BATTLE ]  = MATH_CLAMP( value, 0, RESORT_WORK_RNKDATA_MAX);
  }

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_GET_POKE ] =
    RECORD_Get(p_record, RECID_CAPTURE_POKE );

  p_wk->data[ RESORT_WORK_GET_PARAM_RNKDATA_HATCH_EGG ] =
    RECORD_Get(p_record, RECID_TAMAGO_HATCHING );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	const RESORT_WORK *cp_wk  ワーク
 *	@param	param   取得するデータ
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_WORK_GetData( const RESORT_WORK *cp_wk, RESORT_WORK_GET_PARAM param )
{
  GF_ASSERT( param < RESORT_WORK_GET_PARAM_MAX );
  return cp_wk->data[ param ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ設定
 *
 *	@param	RESORT_WORK *p_wk ワーク
 *	@param	param 設定するデータ
 *	@param	data データ
 */
//-----------------------------------------------------------------------------
void RESORT_WORK_SetData( RESORT_WORK *p_wk, RESORT_WORK_GET_PARAM param, u32 data )
{
  GF_ASSERT( param < RESORT_WORK_GET_PARAM_MAX );
  p_wk->data[ param ] = data;
}
