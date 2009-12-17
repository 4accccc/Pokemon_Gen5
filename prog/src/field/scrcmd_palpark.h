//======================================================================
/**
 * @file	scrcmd_musical.h
 * @brief	スクリプトコマンド：パルパーク関連
 * @author	Satoshi Nohara
 */
//======================================================================
#ifndef SCR_PALPARK_H
#define SCR_PALPARK_H

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に
extern VMCMD_RESULT EvCmdPalparkCall( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetPalparkValue( VMHANDLE *core, void *wk );

#endif	/* __ASM_NO_DEF_ */

#endif	/* SCR_PALPARK_H*/
