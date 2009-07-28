//======================================================================
/**
 * @file	scrcmd_trainer.h
 * @bfief	スクリプトコマンド：ミュージカル関連
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
#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_MUSICAL_H*/
