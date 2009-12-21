//======================================================================
/**
 * @file	scrcmd_battle.h
 * @brief	スクリプトコマンド：勝敗処理など
 * @author  tamada GAMEFREAK inc.
 * @date  2009.10.23
 *
 * @note
 * 勝敗関連で、特に常駐領域にないと問題があるコマンドは
 * このファイルに所属させる。
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================

extern VMCMD_RESULT EvCmdTrainerFlagSet( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdTrainerFlagReset( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdTrainerFlagCheck( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdWildBattleRevengeCheck( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdWildLose( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWildLoseCheck( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdTrainerLose(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerLoseCheck(VMHANDLE *core,void *wk);

#if 0
extern VMCMD_RESULT EvCmdSeacretPokeRetryCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdHaifuPokeRetryCheck(VMHANDLE *core,void *wk);
#endif
