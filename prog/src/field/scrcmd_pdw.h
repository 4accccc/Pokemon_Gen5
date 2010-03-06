//======================================================================
/**
 * @file scr_pdw.c
 * @brief スクリプトコマンド　PDW関連
 * @author	Ariizumi Nobuhiko
 * @date	10/03/06
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に
extern VMCMD_RESULT EvCmdPDW_CommonTools( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPDW_FurnitureTools( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPDW_SetFurnitureWord( VMHANDLE *core, void *wk );

#endif	/* __ASM_NO_DEF_ */

