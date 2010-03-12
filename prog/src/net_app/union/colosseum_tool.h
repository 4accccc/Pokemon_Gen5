//==============================================================================
/**
 * @file    colosseum_tool.h
 * @brief   コロシアムツール類：オーバーレイに配置
 * @author  matsuda
 * @date    2009.07.18(土)
 */
//==============================================================================
#pragma once

#include "demo/comm_btl_demo.h"

//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL ColosseumTool_AllReceiveCheck_BasicStatus(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_AllReceiveCheck_TrainerCard(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_ReceiveCheck_StandPos(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_ReceiveCheck_StandingPos(COLOSSEUM_SYSTEM_PTR clsys);
extern void ColosseumTool_Clear_ReceiveStandingPos(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_CheckStandingPosition(FIELDMAP_WORK *fieldWork, int entry_num, int *stand_pos);
extern BOOL ColosseumTool_AllReceiveCheck_Leave(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_ReceiveCheck_Leave(COLOSSEUM_SYSTEM_PTR clsys);
extern BOOL ColosseumTool_CheckWayOut(FIELDMAP_WORK *fieldWork);
extern void ColosseumTool_CommPlayerUpdate(COLOSSEUM_SYSTEM_PTR clsys);
