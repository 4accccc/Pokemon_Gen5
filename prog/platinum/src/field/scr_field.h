//============================================================================================
/**
 * @file	scr_field.h
 * @bfief	スクリプトで使用するプログラム(非常駐フィールドプログラム)
 * @author	Satoshi Nohara
 * @date	07.12.04
 */
//============================================================================================
#ifndef SCR_FIELD_H
#define SCR_FIELD_H


//==============================================================================================
//
//	イベントウィンドウワークへの不完全型ポインタ
//
//==============================================================================================
typedef	struct _SEISEKI_WORK SEISEKI_WORK;


//============================================================================================
//
//	プログラムでのみ参照する定義
//
//============================================================================================
//__ASM_NO_DEF_が定義されている場合、アセンブラソースなので以下は無効にする
#ifndef	__ASM_NO_DEF_
extern BOOL EvCmdSeisekiBmpListStart( VM_MACHINE * core );
extern BOOL EvCmdTopBreederPowRndGet( VM_MACHINE * core );
extern BOOL EvCmdPokeFormChange( VM_MACHINE * core );
extern BOOL EvCmdMezameruPawaaTypeGet( VM_MACHINE * core );
extern BOOL EvCmdSetFavoritePoke( VM_MACHINE * core );
extern BOOL EvCmdGetFavoritePoke( VM_MACHINE * core );
//extern BOOL EvCmdRotomuFormCheck( VM_MACHINE * core );
extern BOOL EvCmdTemotiRotomuFormChangeGet( VM_MACHINE * core );
extern BOOL EvCmdRotomuFormWazaChange( VM_MACHINE * core );
extern BOOL EvCmdTemotiRotomuFormNoGet( VM_MACHINE * core );
extern BOOL EvCmdVillaListCheck( VM_MACHINE * core );
extern BOOL EvCmdImageClipTvSendFlagReset( VM_MACHINE * core );


#endif	/* __ASM_NO_DEF_ */


#endif	/* SCR_FIELD_H */


