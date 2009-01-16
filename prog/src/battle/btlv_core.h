//=============================================================================================
/**
 * @file	btlv_core.h
 * @brief	ポケモンWB バトル描画担当メインモジュール
 * @author	taya
 *
 * @date	2008.10.02	作成
 */
//=============================================================================================
#ifndef __BTLV_CORE_H__
#define __BTLV_CORE_H__


//----------------------------------------------------------------------
/**
 *	描画メインモジュールハンドラ型定義
 */
//----------------------------------------------------------------------
typedef  struct _BTLV_CORE		BTLV_CORE;


//----------------------------------------------------------------------
// 参照ヘッダ include 
//----------------------------------------------------------------------
#include "btl_main.h"
#include "btl_client.h"
#include "btl_action.h"
#include "btl_calc.h"
#include "btl_string.h"

//----------------------------------------------------------------------
/**
 *	描画コマンド
 */
//----------------------------------------------------------------------
typedef enum {

	BTLV_CMD_SETUP,
	BTLV_CMD_SELECT_ACTION,
	BTLV_CMD_SELECT_POKEMON,
	BTLV_CMD_MSG,

	BTLV_CMD_MAX,
	BTLV_CMD_NULL = BTLV_CMD_MAX,

}BtlvCmd;



//=============================================================================================
/**
 * 描画メインモジュールの生成
 *
 * @param   mainModule		システムメインモジュールのハンドラ
 * @param   heapID			生成先ヒープID
 *
 * @retval  BTLV_CORE*		生成された描画メインモジュールのハンドラ
 */
//=============================================================================================
extern BTLV_CORE*  BTLV_Create( BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, HEAPID heapID );

//=============================================================================================
/**
 * 描画メインモジュールの破棄
 *
 * @param   core		描画メインモジュールのハンドラ
 */
//=============================================================================================
extern void BTLV_Delete( BTLV_CORE* core );

//=============================================================================================
/**
 * 描画メインループ
 *
 * @param   core		描画メインモジュールのハンドラ
 */
//=============================================================================================
extern void BTLV_CORE_Main( BTLV_CORE* core );

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   wk			描画メインモジュールのハンドラ
 *
 * @retval  BOOL		終了していたらTRUE
 */
//=============================================================================================
extern void BTLV_StartCommand( BTLV_CORE* btlv, BtlvCmd cmd );

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   wk			描画メインモジュールのハンドラ
 *
 * @retval  BOOL		終了していたらTRUE
 */
//=============================================================================================
extern BOOL BTLV_WaitCommand( BTLV_CORE* btlv );


//=============================================================================================
/**
 * 
 *
 * @param   core		
 * @param   dst		
 *
 * @retval  extern void		
 */
//=============================================================================================
extern void BTLV_GetActionParam( const BTLV_CORE* core, BTL_ACTION_PARAM* dst );


extern void BTLV_StartMemberChangeAct( BTLV_CORE* wk, u8 clientID, u8 memberIdx );
extern BOOL BTLV_WaitMemberChangeAct( BTLV_CORE* wk );


//=============================================================================================
/**
 * 
 *
 * @param   wk		
 * @param   strID		
 * @param   clientID		
 *
 * @retval  extern void		
 */
//=============================================================================================
extern void BTLV_StartMsgStd( BTLV_CORE* wk, u16 strID, const int* args );
extern void BTLV_StartMsgSet( BTLV_CORE* wk, u16 strID, const int* args );
extern void BTLV_StartMsgWaza( BTLV_CORE* wk, BtlPokePos pokePos, u16 waza );
extern BOOL BTLV_WaitMsg( BTLV_CORE* wk );

extern void BTLV_StartWazaAct( BTLV_CORE* wk, BtlPokePos atPokePos, BtlPokePos defPokePos, u16 damage, WazaID waza, BtlTypeAff affinity );
extern BOOL BTLV_WaitWazaAct( BTLV_CORE* wk );
extern void BTLV_StartDeadAct( BTLV_CORE* wk, u8 clientID );
extern BOOL BTLV_WaitDeadAct( BTLV_CORE* wk );


extern void BTLV_StartTokWin( BTLV_CORE* wk, u8 clientID );
extern void BTLV_QuitTokWin( BTLV_CORE* wk, u8 clientID );
extern void BTLV_StartRankDownEffect( BTLV_CORE* wk, u8 clientID, BppValueID statusType );



//-----------------------------------------------------------
// 各種下請けから呼び出される関数

extern u8 BTLV_CORE_GetPlayerClientID( const BTLV_CORE* core );

#endif
