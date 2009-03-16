//=============================================================================================
/**
 * @file	btl_event_factor.h
 * @brief	ポケモンWB バトルシステム	イベント＆ハンドラ処理（FACTOR側にのみ公開）
 * @author	taya
 *
 * @date	2008.10.23	作成
 */
//=============================================================================================
#ifndef __BTL_EVENT_FACTOR_H__
#define __BTL_EVENT_FACTOR_H__

#include "waza_tool\wazadata.h"

#include "btl_event.h"
#include "btl_server_flow.h"
#include "btl_server_local.h"
#include "btl_server.h"
#include "btl_pokeparam.h"


enum {
	EVENT_HANDLER_WORK_ELEMS = 8,
};

//--------------------------------------------------------------
/**
*	イベント実体型
*/
//--------------------------------------------------------------
typedef struct _BTL_EVENT_FACTOR	BTL_EVENT_FACTOR;

//--------------------------------------------------------------
/**
*	イベントハンドラ型
*
*	factor	自身削除用のハンドル
*	flowWk 	サーバフローワーク
*	pokeID	主体となるポケモンのID
*	work	ワーク用配列（要素数はEVENT_HANDLER_WORK_ELEMS, 初回呼び出し時ゼロクリアが保証される）
*/
//--------------------------------------------------------------
typedef void (*BtlEventHandler)( BTL_EVENT_FACTOR* factor, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );

//--------------------------------------------------------------
/**
*	イベントハンドラ＆タイプの関連付けテーブル
*/
//--------------------------------------------------------------
typedef struct {
	BtlEventType		eventType;
	BtlEventHandler		handler;
}BtlEventHandlerTable;




extern BTL_EVENT_FACTOR* BTL_EVENT_AddFactor( BtlEventFactor factorType, u16 subPri, u8 pokeID, const BtlEventHandlerTable* handlerTable );
extern BTL_EVENT_FACTOR* BTL_EVENT_SeekFactor( BtlEventFactor factorType, u8 pokeID );
extern void BTL_EVENT_RemoveFactor( BTL_EVENT_FACTOR* factor );

#endif
