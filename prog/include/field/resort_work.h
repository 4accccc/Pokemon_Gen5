//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_work.h
 *	@brief  リゾート常駐ワーク
 *	@author	Toru=Nagihashi
 *	@date		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	取得引数
//=====================================
typedef enum
{
  RESORT_WORK_GET_PARAM_RECORD_CAPTURE_POKE,
  RESORT_WORK_GET_PARAM_RECORD_FINISHING_SUCCESS,
  RESORT_WORK_GET_PARAM_RECORD_TAMAGO_HATCHING,
  RESORT_WORK_GET_PARAM_RECORD_BATTLE_COUNT,
  RESORT_WORK_GET_PARAM_RECORD_WALK_COUNT,
  RESORT_WORK_GET_PARAM_RECORD_TRADE_COUNT,
  RESORT_WORK_GET_PARAM_RECORD_SHOPPING_CNT,
  RESORT_WORK_GET_PARAM_RECORD_DENDOU_CNT,

  RESORT_WORK_GET_PARAM_RECORD_MAX
} RESORT_WORK_GET_PARAM;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	リゾート常駐ワーク
//=====================================
typedef struct _RESORT_WORK RESORT_WORK;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern RESORT_WORK* RESORT_WORK_Init( HEAPID heapID, SAVE_CONTROL_WORK *p_sv );
extern void RESORT_WORK_Exit( RESORT_WORK *p_wk );

extern void RESORT_WORK_ReLoadData( RESORT_WORK *p_wk, SAVE_CONTROL_WORK *p_sv );
extern u32 RESORT_WORK_GetData( const RESORT_WORK *cp_wk, RESORT_WORK_GET_PARAM param );
