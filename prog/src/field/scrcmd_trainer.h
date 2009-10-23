//======================================================================
/**
 * @file	scrcmd_trainer.h
 * @brief	スクリプトコマンド：トレーナー関連
 * @author	Satoshi Nohara
 */
//======================================================================
#ifndef SCR_TRAINER_H
#define SCR_TRAINER_H

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に
extern VMCMD_RESULT EvCmdEyeTrainerMoveSet(VMHANDLE *core,void *wk);
#if 0 //pl null
extern VMCMD_RESULT EvCmdEyeTrainerMoveCheck(VMHANDLE *core,void *wk);
#else //wb test
extern VMCMD_RESULT EvCmdEyeTrainerMoveSingle( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdEyeTrainerMoveDouble( VMHANDLE *core, void *wk );
#endif
extern VMCMD_RESULT EvCmdEyeTrainerTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdEyeTrainerIdGet(VMHANDLE *core,void *wk);

extern VMCMD_RESULT EvCmdTrainerIdGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerMultiBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerTalkTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdRevengeTrainerTalkTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerBgmSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerWin(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerLose(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerLoseCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmd2vs2BattleCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdDebugBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdDebugTrainerFlagSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdDebugTrainerFlagOnJump(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdBattleResultGet(VMHANDLE *core,void *wk);

extern VMCMD_RESULT EvCmdSeacretPokeRetryCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdHaifuPokeRetryCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdWildLoseCheck( VMHANDLE *core, void *wk );
#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_TRAINER_H */
