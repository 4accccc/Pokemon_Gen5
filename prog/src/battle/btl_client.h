//=============================================================================================
/**
 * @file	btl_client.h
 * @brief	ポケモンWB バトルシステム	クライアントモジュール
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#ifndef __BTL_CLIENT_H__
#define __BTL_CLIENT_H__

//--------------------------------------------------------------
/**
 *	クライアントモジュールハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_CLIENT		BTL_CLIENT;


//--------------------------------------------------------------
/**
 *	ワザ発動パラメータハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_WAZA_EXE_PARAM	BTL_WAZA_EXE_PARAM;

//----------------------------------------------------------------------
// 参照ヘッダ include 
//----------------------------------------------------------------------
#include "battle/battle.h"
#include "btl_main.h"
#include "btl_adapter.h"
#include "btlv_core.h"


extern BTL_CLIENT* BTL_CLIENT_Create(
	BTL_MAIN_MODULE* mainModule, BtlCommMode commMode,
	GFL_NETHANDLE* netHandle, u16 clientID, BtlThinkerType clientType, HEAPID heapID );

extern void BTL_CLIENT_Delete( BTL_CLIENT* wk );


extern BTL_ADAPTER* BTL_CLIENT_GetAdapter( BTL_CLIENT* client );

extern void BTL_CLIENT_AttachViewCore( BTL_CLIENT* wk, BTLV_CORE* viewCore );

extern void BTL_CLIENT_Main( BTL_CLIENT* client );


extern u8 BTL_CLIENT_GetClientID( const BTL_CLIENT* client );
extern u8 BTL_CLIENT_GetOpponentClientID( const BTL_CLIENT* client, u8 idx );
extern const BTL_PARTY* BTL_CLIENT_GetParty( const BTL_CLIENT* client );
extern u8 BTL_CLIENT_GetFrontPokeIdx( const BTL_CLIENT* client );
extern const BTL_POKEPARAM* BTL_CLIENT_GetFrontPokeParam( const BTL_CLIENT* client );

//---- wep ------
extern const BTL_WAZA_EXE_PARAM* BTL_CLIENT_GetWazaExeParam( const BTL_CLIENT* client, u8 clientID );
extern const BTL_POKEPARAM* BTL_CLIENT_WEP_GetUserPokeParam( const BTL_WAZA_EXE_PARAM* wep );
extern WazaID BTL_CLIENT_WEP_GetWazaNumber( const BTL_WAZA_EXE_PARAM* wep );



#endif
