//==============================================================================
/**
 * @file    bingo_system.h
 * @brief   ビンゴシステムのヘッダ
 * @author  matsuda
 * @date    2009.10.11(日)
 */
//==============================================================================
#pragma once

#include "bingo_types.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BINGO_SYSTEM * Bingo_GetBingoSystemWork(INTRUDE_COMM_SYS_PTR intcomm);
extern void Bingo_InitBingoSystem(BINGO_SYSTEM *bingo);
extern void Bingo_Clear_BingoBattleBeforeBuffer(BINGO_SYSTEM *bingo);
extern void Bingo_Clear_BingoIntrusionBeforeBuffer(BINGO_SYSTEM *bingo);
extern void Bingo_SendUpdate(INTRUDE_COMM_SYS_PTR intcomm, BINGO_SYSTEM *bingo);
extern void Bingo_Req_IntrusionPlayer(INTRUDE_COMM_SYS_PTR intcomm, BINGO_SYSTEM *bingo, int net_id);
extern void Bingo_Parent_EntryIntusion(BINGO_SYSTEM *bingo, int net_id);
extern BINGO_INTRUSION_ANSWER Bingo_GetBingoIntrusionAnswer(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Bingo_GetBingoIntrusionParam(INTRUDE_COMM_SYS_PTR intcomm);

