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
  RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE,
  RESORT_WORK_GET_PARAM_RNKDATA_NICKNAME,
  RESORT_WORK_GET_PARAM_RNKDATA_JA_TOURIST,
  RESORT_WORK_GET_PARAM_RNKDATA_BUY_MONEY,
  RESORT_WORK_GET_PARAM_RNKDATA_CROSSING,
  RESORT_WORK_GET_PARAM_RNKDATA_NET_BATTLE,
  RESORT_WORK_GET_PARAM_RNKDATA_GET_POKE,
  RESORT_WORK_GET_PARAM_RNKDATA_HATCH_EGG,
  RESORT_WORK_GET_PARAM_RNKDATA_MAX,

  RESORT_WORK_GET_PARAM_IS_MAPCHANGE  = RESORT_WORK_GET_PARAM_RNKDATA_MAX,
  RESORT_WORK_GET_PARAM_RECOMMENDED_TBL,
  RESORT_WORK_GET_PARAM_RECOMMENDED_CNT,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_01,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_02,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_03,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_04,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_05,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_06,
  RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_07,
  RESORT_WORK_GET_PARAM_MAX
} RESORT_WORK_GET_PARAM;

#define RESORT_WORK_RNKDATA_MAX (999999999)

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
extern void RESORT_WORK_SetData( RESORT_WORK *p_wk, RESORT_WORK_GET_PARAM param, u32 data );
