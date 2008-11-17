//=============================================================================================
/**
 * @file	btl_string.h
 * @brief	ポケモンWB バトル 文字列生成処理
 * @author	taya
 *
 * @date	2008.10.06	作成
 */
//=============================================================================================
#ifndef __BTL_STRING_H__
#define __BTL_STRING_H__

//--------------------------------------------------------------
/**
 *	対象クライアントを考慮せずに生成できる文字列ID群
 */
//--------------------------------------------------------------
typedef enum {

	BTL_STRFMT_ENCOUNT,
	BTL_STRFMT_SELECT_ACTION_READY,

	BTL_STRID_GEN_MAX,

}BtlGenStrID;

//--------------------------------------------------------------
/**
 *	対象クライアントにより生成される内容が異なる文字列ID群
 */
//--------------------------------------------------------------
typedef enum {
	BTL_STRID_ESCAPE,			///< 【メッセージ】 逃げ出した [ClientID]
	BTL_STRID_CONF_ANNOUNCE,	///< 【メッセージ】 こんらんしている [ClientID]
	BTL_STRID_CONF_ON,			///< 【メッセージ】 わけもわからず [ClientID]
	BTL_STRID_DEAD,				///< 【メッセージ】 たおれた [ClientID]
	BTL_STRID_WAZA_FAIL,		///< 【メッセージ】 ワザが出せなかった[ ClientID, Reason ]
	BTL_STRID_WAZA_AVOID,		///< 【メッセージ】 けど　はずれた！[ ClientID ]
	BTL_STRID_WAZA_HITCOUNT,	///< 【メッセージ】 ○○回あたった！[ hitCount ]
	BTL_STRID_WAZA_ANNOUNCE,	///<  ○○の××こうげき！
	BTL_STRID_RANKDOWN,			///< ○○の××がさがった！[ ClientID, StatusType ]
	BTL_STRID_RANKUP,			///< ○○の××があがった！[ ClientID, StatusType ]
	BTL_STRID_RANKDOWN_FAIL,	///< ○○はのうりょくがさがらない！ [ClientID]

}BtlSpStrID;

#include "btl_main.h"
#include "btl_client.h"


extern void BTL_STR_InitSystem( const BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, HEAPID heapID );
extern void BTL_STR_MakeStringGeneric( STRBUF* buf, BtlGenStrID strID );
extern void BTL_STR_MakeStringSpecific( STRBUF* buf, BtlSpStrID strID, const int* args );


#endif

