//======================================================================
/**
 * @file  scrcmd_word.h
 * @brief  スクリプトコマンド：単語関連
 * @author	Satoshi Nohara
 */
//======================================================================
#ifndef SCR_WORD_H
#define SCR_WORD_H

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に
extern VMCMD_RESULT EvCmdPlayerName( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdItemName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdItemWazaName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWazaName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPokeTypeName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdMonsName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPocketName( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeMonsName(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeNickName(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSodateyaPokeMonsName(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSodateyaPokeNickName(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdNumber(VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdNickName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPlaceName( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdEasyTalkWordName( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdCountryName( VMHANDLE *core, void *wk );

#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_WORD_H */
