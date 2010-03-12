//==============================================================================
/**
 * @file    colosseum.h
 * @brief   コロシアムのヘッダ
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#pragma once

#include "savedata/mystatus.h"
#include "field\fieldmap_proc.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "net_app/union/comm_player.h"
#include "demo\comm_btl_demo.h"


//==============================================================================
//  型定義
//==============================================================================
///COMM_PLAYER_SYS構造体の不定形ポインタ型
typedef struct _COLOSSEUM_SYSTEM * COLOSSEUM_SYSTEM_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern COLOSSEUM_SYSTEM_PTR Colosseum_InitSystem(GAMEDATA *game_data, GAMESYS_WORK *gsys, MYSTATUS *myst, BOOL intrude);
extern void Colosseum_ExitSystem(COLOSSEUM_SYSTEM_PTR clsys);
extern void Colosseum_CommReadySet(COLOSSEUM_SYSTEM_PTR clsys, BOOL flag);
extern void Colosseum_Mine_SetStandingPostion(COLOSSEUM_SYSTEM_PTR clsys, int stand_position);
extern u8 Colosseum_Mine_GetStandingPostion(COLOSSEUM_SYSTEM_PTR clsys);
extern void Colosseum_Mine_SetAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int result);
extern u8 Colosseum_Mine_GetAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL Colosseum_Parent_SetStandingPosition(COLOSSEUM_SYSTEM_PTR clsys, int net_id, u8 stand_position);
extern void Colosseum_Parent_SendAnswerStandingPosition(COLOSSEUM_SYSTEM_PTR clsys);
extern void Colosseum_Parent_BattleReadyAnswer(COLOSSEUM_SYSTEM_PTR clsys);
extern void Colosseum_SetCommPlayerPos(COLOSSEUM_SYSTEM_PTR clsys, int net_id, const COMM_PLAYER_PACKAGE *pack);
extern BOOL Colosseum_GetCommPlayerPos(COLOSSEUM_SYSTEM_PTR clsys, int net_id, COMM_PLAYER_PACKAGE *dest);
extern u8 ColosseumTool_ReceiveCheck_PokeListSeletedNum(COLOSSEUM_SYSTEM_PTR clsys);
extern void ColosseumTool_Clear_ReceivePokeListSelected(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL Colosseum_AllReceiveCheck_Pokeparty(COLOSSEUM_SYSTEM_PTR clsys);
extern void Colosseum_Clear_ReceivePokeParty(COLOSSEUM_SYSTEM_PTR clsys, BOOL except_for_mine);
extern void Colosseum_SetupBattleDemoParent(COLOSSEUM_SYSTEM_PTR clsys, COMM_BTL_DEMO_PARAM *cbdp, HEAPID heap_id);
extern void Colosseum_DeleteBattleDemoParent(COMM_BTL_DEMO_PARAM *cbdp);

