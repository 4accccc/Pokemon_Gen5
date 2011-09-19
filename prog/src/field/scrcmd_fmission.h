//==============================================================================
/**
 * @file    scrcmd_fmission.h
 * @brief   スクリプトコマンド：フェスミッション関連(フィールド常駐)
 * @author  iwasawa 
 * @date    2011.02.27
 */
//==============================================================================

#pragma once

extern VMCMD_RESULT EvCmdFMissionEndForce( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionLogAction( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcEventClear( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdFMissionNpcParamGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcTradeItemGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcShoppingItemGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGetQuestionPoke( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFMissionNpcMemoryQuizParamGetAnswerPoke( VMHANDLE *core, void *wk );

