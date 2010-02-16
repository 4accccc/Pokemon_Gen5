//======================================================================
/**
 * @file	scrcmd_musical.h
 * @brief	スクリプトコマンド：ミュージカル関連
 * @author	Satoshi Nohara
 */
//======================================================================
#ifndef SCR_MUSICAL_H
#define SCR_MUSICAL_H

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に
extern VMCMD_RESULT EvCmdMusicalCall( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMusicalFittingCall( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetMusicalValue( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetMusicalFanValue( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetMusicalWaitRoomValue( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdAddMusicalGoods( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdResetMusicalFanGiftFlg( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSelectMusicalPoke( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdMusicalWord( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMusicalTools( VMHANDLE *core, void *wk );

#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_MUSICAL_H*/
