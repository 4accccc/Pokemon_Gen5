//==============================================================================
/**
 * @file    g_power.h
 * @brief   GÉpÉèÅ[
 * @author  matsuda
 * @date    2010.02.08(åé)
 */
//==============================================================================
#pragma once

#include "buflen.h"
#include "field/intrude_common.h"
#include "field/field_comm/mission_types.h"
#include "poke_tool/pokeparty.h"


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern void GPower_SYSTEM_Init(void);
extern void GPOWER_SYSTEM_Update(void);
extern void GPOWER_Clear_AllPower(void);
extern void GPOWER_Set_OccurID(GPOWER_ID gpower_id, const POWER_CONV_DATA *powerdata);
extern GPOWER_ID GPOWER_Get_FinishWaitID(void);
extern void GPOWER_Set_Finish(GPOWER_ID gpower_id, const POWER_CONV_DATA *powerdata);
extern u32 GPOWER_Calc_Encount(u32 encount);
extern u32 GPOWER_Calc_Hatch(u32 add_count);
extern s32 GPOWER_Calc_Natsuki(s32 natsuki);
extern u32 GPOWER_Calc_Sale(u32 price);
extern u32 GPOWER_Calc_LargeAmountHappen(void);
extern s32 GPOWER_Calc_HPRestore(POKEPARTY *party);
extern s32 GPOWER_Calc_PPRestore(POKEPARTY *party);
extern u32 GPOWER_Calc_Exp(u32 exp);
extern u32 GPOWER_Calc_Money(u32 money);
extern u32 GPOWER_Calc_Capture(u32 hokakuritu);
extern void GPOWER_Calc_Distribution(void);

