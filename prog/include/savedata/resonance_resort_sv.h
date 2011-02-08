//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort_sv.h
 *	@brief  レゾナンスリゾート用セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//セーブデータ関係
#include "savedata/save_control.h"

//リゾートのデータ
#include "field/resonance_resort_data.h"

//=============================================================================
/**
 *					レゾナンスリゾート用セーブデータ
*/
//=============================================================================
#define RESONANCE_RESORT_SV_DATA_BUFF_NUM (8)

//-------------------------------------
///	セーブデータ
//=====================================
typedef struct _RESONANCE_RESORT_SV RESONANCE_RESORT_SV;

//-------------------------------------
///	外部公開
//=====================================
extern u32  RESONANCE_RESORT_SV_GetWorkSize(void);
extern void RESONANCE_RESORT_SV_Init( void *p_wk_adrs );
extern RESONANCE_RESORT_SV *RESONANCE_RESORT_SV_GetWork( SAVE_CONTROL_WORK *p_sv );

extern RESORT_DATA_BUFF * RESONANCE_RESORT_SV_GetDataBuff( RESONANCE_RESORT_SV *p_wk );
