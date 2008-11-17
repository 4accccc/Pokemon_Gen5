//=============================================================================================
/**
 * @file	btl_string.c
 * @brief	ポケモンWB バトル 文字列生成処理
 * @author	taya
 *
 * @date	2008.10.06	作成
 */
//=============================================================================================
#include <gflib.h>

#include "btl_str.h"

#include "btl_common.h"
#include "btl_string.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	MSGDATA_1 = 0,
	MSGDATA_MAX,
};


static struct {

	const BTL_MAIN_MODULE*	mainModule;		///< メインモジュール
	const BTL_CLIENT*		client;			///< UIクライアント
	GFL_MSGDATA*			msg[ MSGDATA_MAX ];	///< メッセージデータハンドル
	HEAPID					heapID;			///< ヒープID
	

}SysWork;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void ms_encount( STRBUF* dst );
static void ms_select_action_ready( STRBUF* dst );
static void ms_escape( STRBUF* dst, const int* args );
static void ms_sp_waza_announce( STRBUF* dst, const int* args );
static void ms_sp_waza_avoid( STRBUF* dst, const int* args );
static void ms_sp_waza_dead( STRBUF* dst, const int* args );
static void ms_rankup( STRBUF* dst, const int* args );
static void ms_rankdown( STRBUF* dst, const int* args );
static void ms_rankdown_fail( STRBUF* dst, const int* args );




void BTL_STR_InitSystem( const BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, HEAPID heapID )
{
	SysWork.mainModule = mainModule;
	SysWork.client = client;
	SysWork.heapID = heapID;
}

void BTL_STR_MakeStringGeneric( STRBUF* buf, BtlGenStrID strID )
{
	static void (* const funcTbl[])( STRBUF* buf ) = {
		ms_encount,
		ms_select_action_ready,
	};

	GF_ASSERT(strID < NELEMS(funcTbl));

	funcTbl[strID]( buf );
}

void BTL_STR_MakeStringSpecific( STRBUF* buf, BtlSpStrID strID, const int* args )
{
	static void (* const funcTbl[])( STRBUF*, const int* ) = {
		ms_escape,
		NULL,
		NULL,
		ms_sp_waza_dead,
		NULL,
		ms_sp_waza_avoid,
		NULL,
		ms_sp_waza_announce,
		ms_rankdown,
		ms_rankup,
		ms_rankdown_fail,
	};

	GF_ASSERT(strID < NELEMS(funcTbl));

	if( funcTbl[strID ] != NULL )
	{
		funcTbl[strID]( buf, args );
	}
	else
	{
		sprintf( buf, "【用意されていない文字列 %d 】\n", strID );
	}
}


static void ms_encount( STRBUF* dst )
{
	const BTL_PARTY* party;
	const BTL_POKEPARAM* poke;
	u16 monsno;
	u8 clientID;

	clientID = BTL_CLIENT_GetOpponentClientID( SysWork.client, 0 );
	party = BTL_MAIN_GetPartyDataConst( SysWork.mainModule, clientID );
	poke = BTL_PARTY_GetMemberDataConst( party, 0 );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );

	sprintf( dst, "やせいの %s が飛び出した！\n", BTRSTR_GetMonsName(monsno) );
}

static void ms_select_action_ready( STRBUF* dst )
{
	const BTL_PARTY* party;
	const BTL_POKEPARAM* poke;
	u16 monsno;
	u8 clientID;

	clientID = BTL_CLIENT_GetClientID( SysWork.client );
	poke = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, clientID );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );

	sprintf( dst, "%s はどうする！？\n", BTRSTR_GetMonsName(monsno) );
}

//--------------------------------------------------------------
/**
 *	うまくにげきれた
 */
//--------------------------------------------------------------
static void ms_escape( STRBUF* dst, const int* args )
{
	sprintf( dst, "うまくにげきれた！\n" );
}

//--------------------------------------------------------------
/**
 *	○○の××こうげき！  等
 */
//--------------------------------------------------------------
static void ms_sp_waza_announce( STRBUF* dst, const int* args )
{
	const BTL_WAZA_EXE_PARAM* wep;
	const BTL_POKEPARAM* poke;
	WazaID  waza;
	u16 monsno;

	wep = BTL_CLIENT_GetWazaExeParam( SysWork.client, args[0] );
	poke = BTL_CLIENT_WEP_GetUserPokeParam( wep );
	waza = BTL_CLIENT_WEP_GetWazaNumber( wep );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );

	sprintf( dst, "%sの%s！\n", BTRSTR_GetMonsName(monsno), BTRSTR_GetWazaName(waza) );
}

//--------------------------------------------------------------
/**
 *	ワザがはずれた！
 */
//--------------------------------------------------------------
static void ms_sp_waza_avoid( STRBUF* dst, const int* args )
{
	sprintf( dst, "しかし　はずれた！\n");
}

//--------------------------------------------------------------
/**
 *	××は倒れた！
 */
//--------------------------------------------------------------
static void ms_sp_waza_dead( STRBUF* dst, const int* args )
{
	const BTL_POKEPARAM* poke;
	u16 monsno;

	poke = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, args[0] );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );

	sprintf( dst, "%sはたおれた！\n", BTRSTR_GetMonsName(monsno) );
}

//--------------------------------------------------------------
/**
 *	○○の××があがった！
 */
//--------------------------------------------------------------
static void ms_rankup( STRBUF* dst, const int* args )
{
	const BTL_POKEPARAM* poke;
	u16 monsno;
	u8 statusType;

	poke = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, args[0] );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );
	statusType = args[1];

	sprintf( dst, "%sの%sがあがった！\n", BTRSTR_GetMonsName(monsno), BTRSTR_GetStatusTypeName(statusType) );

}
//--------------------------------------------------------------
/**
 *	○○の××がさがった！
 */
//--------------------------------------------------------------
static void ms_rankdown( STRBUF* dst, const int* args )
{
	const BTL_POKEPARAM* poke;
	u16 monsno;
	u8 statusType;

	poke = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, args[0] );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );
	statusType = args[1];

	sprintf( dst, "%sの%sがさがった！\n", BTRSTR_GetMonsName(monsno), BTRSTR_GetStatusTypeName(statusType) );
}

//--------------------------------------------------------------
/**
 *	○○ののうりょくはさがらない！
 */
//--------------------------------------------------------------
static void ms_rankdown_fail( STRBUF* dst, const int* args )
{
	const BTL_POKEPARAM* poke;
	u16 monsno;

	poke = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, args[0] );
	monsno = BTL_POKEPARAM_GetMonsNo( poke );

	sprintf( dst, "%sの　のうりょくは　さがらない！\n", BTRSTR_GetMonsName(monsno) );
}

