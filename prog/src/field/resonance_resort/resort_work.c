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
  u32 data[RESORT_WORK_GET_PARAM_RECORD_MAX];
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
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_CAPTURE_POKE ] = RECORD_Get(p_record, RECID_CAPTURE_POKE );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_FINISHING_SUCCESS ]  = RECORD_Get(p_record, RECID_FISHING_SUCCESS );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_TAMAGO_HATCHING ]  = RECORD_Get(p_record, RECID_TAMAGO_HATCHING );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_BATTLE_COUNT ] = RECORD_Get(p_record, RECID_BATTLE_COUNT );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_WALK_COUNT ]   = RECORD_Get(p_record, RECID_WALK_COUNT );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_TRADE_COUNT ]  = RECORD_Get(p_record, RECID_WIFI_TRADE )
    + RECORD_Get(p_record, RECID_COMM_TRADE )
    + RECORD_Get(p_record, RECID_IRC_TRADE );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_SHOPPING_CNT ] = RECORD_Get(p_record, RECID_SHOPPING_CNT );
  p_wk->data[ RESORT_WORK_GET_PARAM_RECORD_DENDOU_CNT ]   = RECORD_Get(p_record, RECID_DENDOU_CNT );
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
  GF_ASSERT( param < RESORT_WORK_GET_PARAM_RECORD_MAX );
  return cp_wk->data[ param ];
}
