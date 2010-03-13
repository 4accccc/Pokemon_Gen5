////////////////////////////////////////////////////////////////////////////
/**
 * @file	 scrcmd_research.h
 * @brief	 スクリプトコマンド: すれ違い調査隊関連
 * @date   2010.03.09
 * @author obata
 */
////////////////////////////////////////////////////////////////////////////
#pragma once

extern VMCMD_RESULT EvCmdGetResearchRequestID( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetResearchQuestionID( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetResearchPassedTime( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdStartResearch( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdFinishResearch( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetTotalCountOfQuestion( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetStartCountOfQuestion( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetMajorityAnswerOfQuestion( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetThanksReceiveCount( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdGetSuretigaiCount( VMHANDLE *core, void *wk ); 
extern VMCMD_RESULT EvCmdSetMyAnswer( VMHANDLE *core, void *wk ); 
