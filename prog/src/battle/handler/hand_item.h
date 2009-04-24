//=============================================================================================
/**
 * @file	hand_item.h
 * @brief	ポケモンWB バトルシステム	イベントファクター（アイテム）追加処理
 * @author	taya
 *
 * @date	2009.04.21	作成
 */
//=============================================================================================
#ifndef	__HAND_ITEM_H__
#define	__HAND_ITEM_H__


#include "..\btl_pokeparam.h"
#include "..\btl_event_factor.h"
#include "..\btl_server_local.h"

extern BTL_EVENT_FACTOR*  BTL_HANDLER_ITEM_Add( const BTL_POKEPARAM* pp );
extern void BTL_HANDLER_ITEM_Remove( const BTL_POKEPARAM* pp );



#endif	/* #ifndef __HAND_ITEM_H__ */
