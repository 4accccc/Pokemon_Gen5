//======================================================================
/**
 * @file	scrcmd_trainer.h
 * @brief	スクリプトコマンド：トレーナー関連
 * @author	Satoshi Nohara
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdEyeTrainerMoveSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdEyeTrainerMove( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdTrainerBtlRuleGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdEyeTrainerMoveTypeGet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdEyeTrainerIdGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdEyeTrainerObjIdGet( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdTrainerIdGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerMultiBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerTalkTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdRevengeTrainerTalkTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerTypeGet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerBgmSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerWin(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmd2vs2BattleCheck(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdTrainerSpecialTypeGet( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdTrainerItemGet( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdDebugBattleSet(VMHANDLE *core,void *wk);
extern VMCMD_RESULT EvCmdBattleResultGet(VMHANDLE *core,void *wk);

extern VMCMD_RESULT EvCmdGetWheelTrainerObjID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetWheelTrainerTrID( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGetTradeAfterTrainerBattleSet( VMHANDLE *core, void *wk ); // 交換後トレーナーとの対戦



