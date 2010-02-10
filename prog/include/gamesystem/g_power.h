//==============================================================================
/**
 * @file    g_power.h
 * @brief   Gパワー
 * @author  matsuda
 * @date    2010.02.08(月)
 */
//==============================================================================
#pragma once

#include "buflen.h"
#include "field/intrude_common.h"
#include "field/field_comm/mission_types.h"
#include "poke_tool/pokeparty.h"
#include "field/gpower_id.h"


//==============================================================================
//  定数定義
//==============================================================================
///一部のGパワー計算命令で、パワーが発動していない事を示す値
#define GPOWER_OCCUR_NONE       (0xff)


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  システム
//--------------------------------------------------------------
extern void GPower_SYSTEM_Init(void);
extern void GPOWER_SYSTEM_Update(void);

//--------------------------------------------------------------
//  発生とクリア
//--------------------------------------------------------------
extern void GPOWER_Clear_AllPower(void);
extern void GPOWER_Set_OccurID(GPOWER_ID gpower_id, const POWER_CONV_DATA *powerdata);
extern GPOWER_ID GPOWER_Check_OccurID(GPOWER_ID gpower_id, const POWER_CONV_DATA *powerdata);
extern GPOWER_ID GPOWER_Check_OccurType(GPOWER_TYPE type, const POWER_CONV_DATA *powerdata);
extern GPOWER_ID GPOWER_Get_FinishWaitID(void);
extern void GPOWER_Set_Finish(GPOWER_ID gpower_id, const POWER_CONV_DATA *powerdata);
extern POWER_CONV_DATA * GPOWER_PowerData_LoadAlloc(HEAPID heap_id);
extern void GPOWER_PowerData_Unload(POWER_CONV_DATA *powerdata);
extern GPOWER_TYPE GPOWER_ID_to_Type(const POWER_CONV_DATA *powerdata, GPOWER_ID gpower_id);

//--------------------------------------------------------------
//  影響の計算
//--------------------------------------------------------------
extern u32 GPOWER_Calc_Encount(u32 encount);
extern u32 GPOWER_Calc_Hatch(u32 add_count);
extern s32 GPOWER_Calc_Natsuki(s32 natsuki);
extern u32 GPOWER_Calc_Sale(u32 price);
extern s32 GPOWER_Calc_LargeAmountHappen(void);
extern s32 GPOWER_Calc_HPRestore(POKEPARTY *party);
extern s32 GPOWER_Calc_PPRestore(POKEPARTY *party);
extern u32 GPOWER_Calc_Exp(u32 exp);
extern u32 GPOWER_Calc_Money(u32 money);
extern u32 GPOWER_Calc_Capture(u32 hokakuritu);
extern void GPOWER_Calc_Distribution(void);

