//==============================================================================
/**
 * @file    scrcmd_symbol.h
 * @brief   シンボルポケモン：スクリプトコマンド
 * @author  matsuda
 * @date    2010.03.16(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern VMCMD_RESULT EvCmdSymbolPokeBattle( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSymbolPokeSet( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdSymbolMapWarp( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSymbolMapMovePokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSymbolMapGetInfo( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSymbolMapSetMonsName( VMHANDLE * core, void * wk );
