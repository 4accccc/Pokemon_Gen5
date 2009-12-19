//======================================================================
/**
 * @file	scrcmd_screeneffects.h
 * @brief	スクリプトコマンド：フィールド画面演出系
 * @date  2009.09.22
 * @author	tamada GAMEFREAK inc.
 *
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
extern VMCMD_RESULT EvCmdDispFadeStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdDispFadeCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFadeCheck( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdMapFade_BlackIn_Force( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_BlackIn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_BlackOut( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_WhiteIn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_WhiteOut( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdPokecenRecoverAnime( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPokecenPcOn( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPokecenPcRun( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPokecenPcOff( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdBModelDirectAnimeSetFinished( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelDirectAnimeSetLoop( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelDirectChangeViewFlag( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdBModelAnimeCreate( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelAnimeDelete( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelAnimeSet( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelAnimeStop( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdBModelAnimeWait( VMHANDLE * core, void *wk );



