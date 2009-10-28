//==============================================================================
/**
 * @file    intrude_subdisp.h
 * @brief   êNì¸â∫âÊñ ÇÃÉwÉbÉ_
 * @author  matsuda
 * @date    2009.10.17(ìy)
 */
//==============================================================================
#pragma once


//==============================================================================
//  å^íËã`
//==============================================================================
typedef struct _INTRUDE_SUBDISP * INTRUDE_SUBDISP_PTR;


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern INTRUDE_SUBDISP_PTR INTRUDE_SUBDISP_Init(GAMESYS_WORK *gsys);
extern void INTRUDE_SUBDISP_Exit(INTRUDE_SUBDISP_PTR intsub);
extern void INTRUDE_SUBDISP_Update(INTRUDE_SUBDISP_PTR intsub);
extern void INTRUDE_SUBDISP_Draw(INTRUDE_SUBDISP_PTR intsub);

