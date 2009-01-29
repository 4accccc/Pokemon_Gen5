//=============================================================================================
/**
 * @file	btlv_scu.h
 * @brief	ポケモンWB バトル 描画 上画面制御モジュール
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================
#ifndef __BTLV_SCU_H__
#define __BTLV_SCU_H__

//--------------------------------------------------------------
/**
 *	モジュールハンドラ型
 */
//--------------------------------------------------------------
typedef struct _BTLV_SCU	BTLV_SCU;



#include <tcbl.h>
#include "print/gf_font.h"
#include  "btl_main.h"
#include  "btlv_core.h"


extern BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore, const BTL_MAIN_MODULE* mainModule, GFL_TCBLSYS* tcbl, GFL_FONT* defaultFont, HEAPID heapID );
extern void BTLV_SCU_Delete( BTLV_SCU* wk );
extern void BTLV_SCU_Setup( BTLV_SCU* wk );
extern void BTLV_SCU_StartBtlIn( BTLV_SCU* wk );
extern BOOL BTLV_SCU_WaitBtlIn( BTLV_SCU* wk );
extern void BTLV_SCU_StartMsg( BTLV_SCU* wk, const STRBUF* str );
extern BOOL BTLV_SCU_WaitMsg( BTLV_SCU* wk );
extern void BTLV_SCU_DispTokWin( BTLV_SCU* wk, u8 clientID );
extern void BTLV_SCU_HideTokWin( BTLV_SCU* wk, u8 clientID );
extern void BTLV_SCU_StartWazaAct( BTLV_SCU* wk, BtlPokePos atPos, BtlPokePos defPos, WazaID waza, BtlTypeAff affinity );
extern BOOL BTLV_SCU_WaitWazaAct( BTLV_SCU* wk );
extern void BTLV_SCU_StartDeadAct( BTLV_SCU* wk, BtlPokePos pos );
extern BOOL BTLV_SCU_WaitDeadAct( BTLV_SCU* wk );
extern void BTLV_SCU_StartPokeIn( BTLV_SCU* wk, BtlPokePos pokePos, u8 clientID, u8 memberIdx );
extern BOOL BTLV_SCU_WaitPokeIn( BTLV_SCU* wk );
extern void BTLV_SCU_StartMemberOutAct( BTLV_SCU* wk, u8 clientID, u8 memberIdx, BtlPokePos pos );
extern BOOL BTLV_SCU_WaitMemberOutAct( BTLV_SCU* wk );



#endif
