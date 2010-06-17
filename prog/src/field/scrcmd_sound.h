//======================================================================
/**
 * @file	scrcmd_sound.h
 * @brief	スクリプトコマンド：サウンド関連
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
extern VMCMD_RESULT EvCmdBgmPlayEx( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPlaySilent( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPlayCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmNowMapPlay( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmNowMapPlayEx( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmFadeWait( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlayTempEventBGM( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPush( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmPop( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndBGMPushPop( SCREND_CHECK *end_check, int *seq ); 
extern VMCMD_RESULT EvCmdBgmWait( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmVolumeDown( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdBgmVolumeRecover( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndBGMVolumeDown( SCREND_CHECK *end_check, int *seq ); 

//EnvSe
extern VMCMD_RESULT EvCmdEnvSeBGMPlayClear( VMHANDLE *core, void *wk );
extern BOOL SCREND_CheckEndEnvSEBGMPlayPause( SCREND_CHECK *end_check, int *seq ); 

//SE
extern VMCMD_RESULT EvCmdSePlay( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSeStop( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSeWait(VMHANDLE *core, void *wk);

//ME
extern VMCMD_RESULT EvCmdMePlay(VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMeWait(VMHANDLE *core, void *wk );

//鳴き声
extern VMCMD_RESULT EvCmdVoicePlay(VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdVoicePlay_forMine(VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdVoiceWait(VMHANDLE *core, void *wk );

// ISS
extern VMCMD_RESULT EvCmdIssSwitchOn( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdIssSwitchOff( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdIssSwitchCheck( VMHANDLE* core, void* wk );

#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_SOUND_H */
