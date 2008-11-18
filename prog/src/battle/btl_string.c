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

#include "print/wordset.h"

#include "btl_str.h"

#include "btl_common.h"
#include "btl_string.h"

#include "arc_def.h"
#include "message.naix"
#include "msg/msg_btl_std.h"
#include "msg/msg_btl_set.h"
#include "msg/msg_btl_attack.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/

enum {
	TMP_STRBUF_SIZE = 256,
};

//------------------------------------------------------
/**
 *	WORDSETバッファ用途
 */
//------------------------------------------------------
typedef enum {

	BUFIDX_POKE_1ST = 0,
	BUFIDX_POKE_2ND,
	BUFIDX_TOKUSEI,

}WordBufID;

//------------------------------------------------------
/**
 *	同内容メッセージセットの分類
 */
//------------------------------------------------------
typedef enum {

	SETSTR_MINE = 0,	///< 自分のポケ
	SETSTR_WILD,		///< 野生のポケ
	SETSTR_ENEMY,		///< 相手のポケ

	SETSTR_MAX,

}SetStrFormat;

//------------------------------------------------------
/**
 *	文字列リソースID
 */
//------------------------------------------------------
typedef enum {
	MSGSRC_STD = 0,
	MSGSRC_SET,
	MSGSRC_ATK,

	MSGDATA_MAX,
}MsgSrcID;


//------------------------------------------------------------------------------
/**
 *
 *	システムワーク
 *
 */
//------------------------------------------------------------------------------
static struct {

	const BTL_MAIN_MODULE*	mainModule;			///< メインモジュール
	const BTL_CLIENT*		client;				///< UIクライアント
	WORDSET*				wset;				///< WORDSET
	STRBUF*					tmpBuf;				///< 文字列一時展開用バッファ
	GFL_MSGDATA*			msg[ MSGDATA_MAX ];	///< メッセージデータハンドル

	HEAPID					heapID;				///< ヒープID
	u8						clientID;			///< UIクライアントID

}SysWork;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline void register_PokeNickname( u8 clientID, WordBufID bufID );
static inline SetStrFormat get_strFormat( u8 targetClientID );
static inline u16 get_atkStrID( u8 userClientID, WazaID wazaID );
static inline u16 get_setStrID( u8 targetClientID, u16 defaultStrID );
static void ms_encount( STRBUF* dst );
static void ms_select_action_ready( STRBUF* dst );
static void ms_escape( STRBUF* dst, const int* args );
static void ms_sp_waza_announce( STRBUF* dst, const int* args );
static void ms_sp_waza_avoid( STRBUF* dst, const int* args );
static void ms_sp_waza_dead( STRBUF* dst, const int* args );
static void ms_rankup( STRBUF* dst, const int* args );
static void ms_rankdown( STRBUF* dst, const int* args );
static void ms_rankdown_fail( STRBUF* dst, const int* args );




//=============================================================================================
/**
 * 文字列生成システム初期化（バトル起動直後に１回だけ呼ぶ）
 *
 * @param   mainModule		
 * @param   client		
 * @param   heapID		
 *
 */
//=============================================================================================
void BTL_STR_InitSystem( const BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, HEAPID heapID )
{
	static const u16 msgDataID[] = {
		NARC_message_btl_std_dat,
		NARC_message_btl_set_dat,
		NARC_message_btl_attack_dat,
	};

	int i;

	SysWork.mainModule = mainModule;
	SysWork.client = client;
	SysWork.heapID = heapID;

	SysWork.wset = WORDSET_Create( heapID );
	SysWork.tmpBuf = GFL_STR_CreateBuffer( heapID, TMP_STRBUF_SIZE );

	for(i=0; i<MSGDATA_MAX; i++)
	{
		SysWork.msg[i] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msgDataID[i], heapID );
	}
}

//=============================================================================================
/**
 * 文字列生成システム終了
 */
//=============================================================================================
void BTL_STR_QuitSystem( void )
{
	WORDSET_Delete( SysWork.wset );
	GFL_STR_DeleteBuffer( SysWork.tmpBuf );

	{
		int i;

		for(i=0; i<MSGDATA_MAX; i++)
		{
			GFL_MSG_Delete( SysWork.msg[i] );
		}
	}
}

//--------------------------------------------------------------------------------------

static inline void register_PokeNickname( u8 clientID, WordBufID bufID )
{
	const BTL_POKEPARAM* bpp;
	const POKEMON_PARAM* pp;

	bpp = BTL_MAIN_GetFrontPokeDataConst( SysWork.mainModule, clientID );
	pp = BTL_POKEPARAM_GetSrcData( bpp );

	WORDSET_RegisterPokeNickName( SysWork.wset, bufID, pp );
}

static inline SetStrFormat get_strFormat( u8 targetClientID )
{
	if( BTL_MAIN_IsOpponentClientID(SysWork.mainModule, SysWork.clientID, targetClientID) )
	{
		if( BTL_MAIN_GetCompetitor(SysWork.mainModule) == BTL_COMPETITOR_WILD )
		{
			return SETSTR_WILD;
		}
		else
		{
			return SETSTR_ENEMY;
		}
	}
	return SETSTR_MINE;
}

static inline u16 get_atkStrID( u8 userClientID, WazaID wazaID )
{
	return (wazaID * SETSTR_MAX) + get_strFormat( userClientID );
}

static inline u16 get_setStrID( u8 targetClientID, u16 defaultStrID )
{
	return defaultStrID + get_strFormat( targetClientID );
}

static inline u16 get_setPtnStrID( u8 targetClientID, u16 originStrID, u8 ptnNum )
{
	return originStrID + (ptnNum * SETSTR_MAX) + get_strFormat(targetClientID);
}

//--------------------------------------------------------------------------------------


//=============================================================================================
/**
 * 
 *
 * @param   buf			
 * @param   strID		
 *
 */
//=============================================================================================
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
		GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], BTLMSG_STD_NONE, buf );
	}
}



//----------------------------------------------------------------------

static void ms_encount( STRBUF* dst )
{
	u8 clientID = BTL_CLIENT_GetOpponentClientID( SysWork.client, 0 );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], BTLMSG_STD_WILD_APPEAR, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

static void ms_select_action_ready( STRBUF* dst )
{
	u8 clientID = BTL_CLIENT_GetClientID( SysWork.client );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], BTLMSG_STD_SELECT_ACTION, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}


//--------------------------------------------------------------
/**
 *	うまくにげきれた
 */
//--------------------------------------------------------------
static void ms_escape( STRBUF* dst, const int* args )
{
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], BTLMSG_STD_ESCAPE, dst );
}

//--------------------------------------------------------------
/**
 *	○○の××こうげき！  等
 */
//--------------------------------------------------------------
static void ms_sp_waza_announce( STRBUF* dst, const int* args )
{
	const BTL_WAZA_EXE_PARAM* wep;
	WazaID  waza;
	u16 strID;

	register_PokeNickname( args[0], BUFIDX_POKE_1ST );

	wep = BTL_CLIENT_GetWazaExeParam( SysWork.client, args[0] );
	waza = BTL_CLIENT_WEP_GetWazaNumber( wep );
	strID = get_atkStrID( args[0], waza );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_ATK], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//--------------------------------------------------------------
/**
 *	ワザがはずれた！
 */
//--------------------------------------------------------------
static void ms_sp_waza_avoid( STRBUF* dst, const int* args )
{
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], BTLMSG_STD_WAZA_AVOID, dst );
}

//--------------------------------------------------------------
/**
 *	××は倒れた！
 */
//--------------------------------------------------------------
static void ms_sp_waza_dead( STRBUF* dst, const int* args )
{
	u16 strID;

	register_PokeNickname( args[0], BUFIDX_POKE_1ST );
	strID = get_setStrID( args[0], BTLMSG_SET_DEAD_M );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//--------------------------------------------------------------
/**
 *	○○の××があがった！
 */
//--------------------------------------------------------------
static void ms_rankup( STRBUF* dst, const int* args )
{
	u16 strID;
	u8 clientID = args[0];
	u8 statusType = args[1];

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	strID = get_setPtnStrID( args[0], BTLMSG_SET_RANKUP_ATK_M, statusType );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );

//	sprintf( dst, "%sの%sがあがった！\n", BTRSTR_GetMonsName(monsno), BTRSTR_GetStatusTypeName(statusType) );

}
//--------------------------------------------------------------
/**
 *	○○の××がさがった！
 */
//--------------------------------------------------------------
static void ms_rankdown( STRBUF* dst, const int* args )
{
	u16 strID;
	u8 clientID = args[0];
	u8 statusType = args[1];

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	strID = get_setPtnStrID( args[0], BTLMSG_SET_RANKDOWN_ATK_M, statusType );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );

//	sprintf( dst, "%sの%sがさがった！\n", BTRSTR_GetMonsName(monsno), BTRSTR_GetStatusTypeName(statusType) );
}

//--------------------------------------------------------------
/**
 *	○○ののうりょくはさがらない！
 */
//--------------------------------------------------------------
static void ms_rankdown_fail( STRBUF* dst, const int* args )
{
	u16 strID;

	register_PokeNickname( args[0], BUFIDX_POKE_1ST );
	strID = get_setStrID( args[0], BTLMSG_SET_RANKDOWN_FAIL_M );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

