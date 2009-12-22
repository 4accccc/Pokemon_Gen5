//==============================================================================
/**
 * @file    scrcmd_intrude.h
 * @brief   スクリプトコマンド：侵入
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#ifndef __SCRCMD_INTRUDE_H__
#define __SCRCMD_INTRUDE_H__

//==============================================================================
//  外部関数宣言
//==============================================================================
#ifndef	__ASM_NO_DEF_ //アセンブラソースは無効に

extern VMCMD_RESULT EvCmdIntrudeMinimonoSetting( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdIntrudeMissionStart( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdIntrudeConnectMapSetting( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSymbolPokeSet( VMHANDLE *core, void *wk );

//--------------------------------------------------------------
//  fieldmapオーバーレイに配置
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdIntrudeMissionChoiceListReq( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetDisguiseCode( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPalaceInDisguise( VMHANDLE *core, void *wk );


#endif	/* __ASM_NO_DEF_ */

#endif	/* __SCRCMD_INTRUDE_H__*/
