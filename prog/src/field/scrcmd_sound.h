//======================================================================
/**
 * @file	scrcmd_sound.h
 * @bfief	スクリプトコマンド：サウンド関連
 * @author	Satoshi Nohara
 */
//======================================================================
#ifndef SCR_SOUND_H
#define SCR_SOUND_H

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に

//BGM
extern VMCMD_RESULT EvCmdBgmPlay( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmStop( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPlayerPause( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPlayCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmFadeOut( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmFadeIn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmNowMapPlay( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmForcePop( VMHANDLE *core, void *wk );

//SE
extern VMCMD_RESULT EvCmdSePlay( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSeStop( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSeWait(VMHANDLE *core, void *wk);

//ME
extern VMCMD_RESULT EvCmdMePlay(VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMeWait(VMHANDLE *core, void *wk );

#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_SOUND_H */
