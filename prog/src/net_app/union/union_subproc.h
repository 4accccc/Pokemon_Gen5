//==============================================================================
/**
 * @file    union_subproc.h
 * @brief   
 * @author  matsuda
 * @date    2009.07.16(ñÿ)
 */
//==============================================================================
#pragma once


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern GMEVENT * UnionSubProc_Create(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldWork, UNION_SYSTEM_PTR unisys);
extern void UnionSubProc_EventSet(UNION_SYSTEM_PTR unisys, UNION_SUBPROC_ID sub_proc_id, void *parent_wk);
extern BOOL UnionSubProc_IsExits(UNION_SYSTEM_PTR unisys);

