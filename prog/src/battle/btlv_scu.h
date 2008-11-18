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


#include  "btlv_core.h"


extern BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore, HEAPID heapID );
extern void BTLV_SCU_Delete( BTLV_SCU* wk );
extern void BTLV_SCU_Setup( BTLV_SCU* wk );
extern void BTLV_SCU_StartBtlIn( BTLV_SCU* wk );
extern BOOL BTLV_SCU_WaitBtlIn( BTLV_SCU* wk );
extern void BTLV_SCU_StartMsg( BTLV_SCU* wk, const STRBUF* str );
extern BOOL BTLV_SCU_WaitMsg( BTLV_SCU* wk );


#endif
