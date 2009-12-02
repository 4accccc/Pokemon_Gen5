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

extern VMCMD_RESULT EvCmdMapFade_BlackIn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_BlackOut( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_WhiteIn( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMapFade_WhiteOut( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdPokecenRecoverAnime( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdDoorAnimeSetOpened( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdDoorAnimeCreate( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdDoorAnimeDelete( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdDoorAnimeSet( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdDoorAnimeWait( VMHANDLE * core, void *wk );



