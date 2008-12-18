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

#include "btl_common.h"
#include "btl_string.h"

#include "arc_def.h"
#include "message.naix"
#include "msg/msg_btl_std.h"
#include "msg/msg_btl_set.h"
#include "msg/msg_btl_attack.h"
#include "msg/msg_btl_ui.h"
#include "msg/msg_wazaname.h"

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

	SETTYPE_MINE = 0,	///< 自分のポケ
	SETTYPE_WILD,		///< 野生のポケ
	SETTYPE_ENEMY,		///< 相手のポケ

	SETTYPE_MAX,

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
	MSGSRC_UI,
	MSGSRC_TOKUSEI,

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
static inline u16 get_setPtnStrID( u8 targetClientID, u16 originStrID, u8 ptnNum );
static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID );
static void ms_encount( STRBUF* dst, BtlStrID_STD strID );
static void ms_put_single( STRBUF* dst, BtlStrID_STD strID );
static void ms_put_single_enemy( STRBUF* dst, BtlStrID_STD strID );
static void ms_select_action_ready( STRBUF* dst, BtlStrID_STD strID );
static void ms_sp_waza_dead( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args );
static void ms_set_rankdown_fail( STRBUF* dst, u16 strID, const int* args );




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
		NARC_message_btl_ui_dat,
		NARC_message_tokusei_dat,
	};

	int i;

	SysWork.mainModule = mainModule;
	SysWork.client = client;
	SysWork.heapID = heapID;

	SysWork.wset = WORDSET_Create( heapID );
	SysWork.tmpBuf = GFL_STR_CreateBuffer( TMP_STRBUF_SIZE, heapID );

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
			return SETTYPE_WILD;
		}
		else
		{
			return SETTYPE_ENEMY;
		}
	}
	return SETTYPE_MINE;
}

static inline u16 get_atkStrID( u8 userClientID, WazaID wazaID )
{
	return (wazaID * SETTYPE_MAX) + get_strFormat( userClientID );
}

static inline u16 get_setStrID( u8 targetClientID, u16 defaultStrID )
{
	return defaultStrID + get_strFormat( targetClientID );
}

static inline u16 get_setPtnStrID( u8 targetClientID, u16 originStrID, u8 ptnNum )
{
	return originStrID + (ptnNum * SETTYPE_MAX) + get_strFormat(targetClientID);
}

//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------

//=============================================================================================
/**
 * 標準メッセージの生成
 *
 * ※標準メッセージ：対象者なし、あるいは対象が誰であっても一定のフォーマットで生成される文字列
 *
 * @param   buf			
 * @param   strID		
 *
 */
//=============================================================================================
void BTL_STR_MakeStringStd( STRBUF* buf, BtlStrID_STD strID )
{
	static void (* const funcTbl[])( STRBUF* buf, BtlStrID_STD strID ) = {
		NULL,

		ms_encount,
		ms_put_single,
		ms_put_single_enemy,
		ms_select_action_ready,

		ms_std_simple,
		ms_std_simple,
		ms_std_simple,
		ms_std_simple,
		ms_std_simple,
		ms_std_simple,
		ms_std_simple,
	};

	if( strID && strID < NELEMS(funcTbl) )
	{
		funcTbl[strID]( buf, strID );
	}
	else
	{
//		GF_ASSERT_MSG(0, " unknown strID=%d\n", strID);
		GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], 0, buf );
	}

}
static void ms_std_simple( STRBUF* dst, BtlStrID_STD strID )
{
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, dst );
}

static void ms_encount( STRBUF* dst, BtlStrID_STD strID )
{
	u8 clientID = BTL_CLIENT_GetOpponentClientID( SysWork.client, 0 );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

static void ms_put_single( STRBUF* dst, BtlStrID_STD strID )
{
	u8 clientID = BTL_CLIENT_GetClientID( SysWork.client );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

static void ms_put_single_enemy( STRBUF* dst, BtlStrID_STD strID )
{
	u8 clientID = BTL_CLIENT_GetClientID( SysWork.client );
	clientID = BTL_MAIN_GetOpponentClientID( SysWork.mainModule, clientID, 0 );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

static void ms_select_action_ready( STRBUF* dst, BtlStrID_STD strID )
{
	u8 clientID = BTL_CLIENT_GetClientID( SysWork.client );

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

//----------------------------------------------------------------------

//=============================================================================================
/**
 * セットメッセージの生成
 * ※セットメッセージ：自分側，敵側（やせいの），敵側（あいての）が必ず３つセットになった文字列
 *
 * @param   buf		
 * @param   strID		
 * @param   args		
 *
 */
//=============================================================================================
void BTL_STR_MakeStringSet( STRBUF* buf, BtlStrID_SET strID, const int* args )
{
	static const struct {
		u16		strID;
		void	(* func)( STRBUF*, u16, const int* );
	}funcTbl[] = {
		{ BTL_STRID_SET_Dead,			ms_sp_waza_dead			},
		{ BTL_STRID_SET_Rankup_ATK,		ms_set_rankup			},
		{ BTL_STRID_SET_Rankdown_ATK,	ms_set_rankdown			},
		{ BTL_STRID_SET_RankdownFail,	ms_set_rankdown_fail	},
	};

	int i;

	for(i=0; i<NELEMS(funcTbl); i++)
	{
		if( funcTbl[i].strID == strID )
		{
			funcTbl[i].func( buf, strID, args );
			return;
		}
	}
	GFL_MSG_GetString( SysWork.msg[MSGSRC_STD], 0, buf );
}


//--------------------------------------------------------------
/**
 *	××は倒れた！
 */
//--------------------------------------------------------------
static void ms_sp_waza_dead( STRBUF* dst, u16 strID, const int* args )
{
	register_PokeNickname( args[0], BUFIDX_POKE_1ST );
	strID = get_setStrID( args[0], strID );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *	○○の××があがった！
 */
//--------------------------------------------------------------
static void ms_set_rankup( STRBUF* dst, u16 strID, const int* args )
{
	u8 clientID = args[0];
	u8 statusType = args[1];

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	strID = get_setPtnStrID( args[0], strID, statusType );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *	○○の××がさがった！
 */
//--------------------------------------------------------------
static void ms_set_rankdown( STRBUF* dst, u16 strID, const int* args )
{
	u8 clientID = args[0];
	u8 statusType = args[1];

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );
	strID = get_setPtnStrID( args[0], strID, statusType );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}
//--------------------------------------------------------------
/**
 *	○○ののうりょくはさがらない！
 */
//--------------------------------------------------------------
static void ms_set_rankdown_fail( STRBUF* dst, u16 strID, const int* args )
{
	register_PokeNickname( args[0], BUFIDX_POKE_1ST );
	strID = get_setStrID( args[0], strID );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_SET], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}


//=============================================================================================
/**
 * ワザメッセージの生成
 * ※ワザメッセージ：○○の××こうげき！とか。セットメッセージと同様、必ず３つセット。
 *
 * @param   buf		
 * @param   strID		
 * @param   args		
 *
 */
//=============================================================================================
void BTL_STR_MakeStringWaza( STRBUF* dst, u8 clientID, u16 waza )
{
	u16 strID;

	register_PokeNickname( clientID, BUFIDX_POKE_1ST );

	strID = get_setStrID( clientID, waza * SETTYPE_MAX );
	GFL_MSG_GetString( SysWork.msg[MSGSRC_ATK], strID, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}






//=============================================================================================
/**
 * 
 *
 * @param   dst			
 * @param   strID		
 *
 */
//=============================================================================================
void BTL_STR_GetUIString( STRBUF* dst, u16 strID )
{
	GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], strID, dst );
}

//--------------------------------------------------------------------



u16 BTL_STR_GetRankUpStrID( u8 statusType, u8 volume )
{
	GF_ASSERT(statusType < BPP_RANKTYPE_MAX);

	if( volume == 1 )
	{
		return BTL_STRID_SET_Rankup_ATK + (statusType * SETTYPE_MAX);
	}
	else
	{
		// @@@ ぐーんと
		return BTL_STRID_SET_Rankup_ATK + (statusType * SETTYPE_MAX);
	}
}

u16 BTL_STR_GetRankDownStrID( u8 statusType, u8 volume )
{
	GF_ASSERT(statusType < BPP_RANKTYPE_MAX);

	if( volume == 1 )
	{
		return BTL_STRID_SET_Rankdown_ATK + (statusType * SETTYPE_MAX);
	}
	else
	{
		// @@@ ぐーんと
		return BTL_STRID_SET_Rankdown_ATK + (statusType * SETTYPE_MAX);
	}
}



//--------------------------------------------------------------------
// この下はちょう仮作成。いずれ消えますたぶん…
//--------------------------------------------------------------------

void BTL_STR_MakeWazaUIString( STRBUF* dst, u16 wazaID, u8 wazaPP, u8 wazaPPMax )
{
	WORDSET_RegisterWazaName( SysWork.wset, 0, wazaID );
	WORDSET_RegisterNumber( SysWork.wset, 1, wazaPP, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber( SysWork.wset, 2, wazaPPMax, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], BTLMSG_UI_SEL_WAZA, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}


void BTL_STR_MakeStatusWinStr( STRBUF* dst, const BTL_POKEPARAM* bpp, u16 hp )
{
	u16 hpMax = BTL_POKEPARAM_GetValue( bpp, BPP_MAX_HP );

	WORDSET_RegisterPokeNickName( SysWork.wset, 0, BTL_POKEPARAM_GetSrcData(bpp) );
	WORDSET_RegisterNumber( SysWork.wset, 1, hp, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterNumber( SysWork.wset, 2, hpMax, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );

	GFL_MSG_GetString( SysWork.msg[MSGSRC_UI], BTLSTR_UI_StatWinForm, SysWork.tmpBuf );
	WORDSET_ExpandStr( SysWork.wset, dst, SysWork.tmpBuf );
}

