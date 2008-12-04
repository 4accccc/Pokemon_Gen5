//=============================================================================================
/**
 * @file	btl_client.c
 * @brief	ポケモンWB バトルシステム	クライアントモジュール
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#include <gflib.h>

#include "poke_tool/pokeparty.h"

#include "btl_common.h"
#include "btl_adapter.h"
#include "btl_action.h"
#include "btl_server_cmd.h"
#include "btlv_core.h"

#include "btl_client.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*ClientSubProc)( BTL_CLIENT*, int* );


/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/


//--------------------------------------------------------------
/**
 *	ワザ発動用パラメータ
 */
//--------------------------------------------------------------
struct _BTL_WAZA_EXE_PARAM {

	const BTL_POKEPARAM*	userPokeParam;
	WazaID	waza;			///< 使うワザナンバー
	u8		numTargetClients;				///< ターゲットとなるクライアント数
	u8		targetClientID[BTL_CLIENT_MAX];	///< ターゲットとなるクライアントID

};

//--------------------------------------------------------------
/**
 *	クライアントモジュール構造定義
 */
//--------------------------------------------------------------
struct _BTL_CLIENT {

	BTL_MAIN_MODULE*	mainModule;

	BTL_ADAPTER*	adapter;
	BTLV_CORE*		viewCore;

	ClientSubProc	subProc;
	int				subSeq;

	const void*			returnDataPtr;
	u32					returnDataSize;


	const BTL_PARTY*	myParty;
	u8					frontPokeIdx;

	BTL_ACTION_PARAM	actionParam;
	BTL_SERVER_CMD_QUE*	cmdQue;
	int					cmdArgs[ BTL_SERVERCMD_ARG_MAX ];
	ServerCmd			serverCmd;

	BTL_WAZA_EXE_PARAM	wazaExeParam[ BTL_CLIENT_MAX ];

	u8	myID;
	u8	myType;
	u8	myState;

};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BOOL SubProc_UI_NotifyPokeData( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_NotifyPokeData( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_Initialize( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_Initialize( BTL_CLIENT* wk, int *seq );
static BOOL SubProc_UI_SelectAction( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_SelectPokemon( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq );
static void BTL_ACTION_SetEscapeParam( BTL_ACTION_PARAM* p );
static void BTL_ACTION_SetNULL( BTL_ACTION_PARAM* p );
static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_ServerCmd( BTL_CLIENT* wk, int* seq );
static ClientSubProc getSubProc( BTL_CLIENT* wk, BtlAdapterCmd cmd );



BTL_CLIENT* BTL_CLIENT_Create(
	BTL_MAIN_MODULE* mainModule, BtlCommMode commMode, GFL_NETHANDLE* netHandle,
	u16 clientID, BtlThinkerType clientType, HEAPID heapID )
{
	BTL_CLIENT* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_CLIENT) );

	wk->myID = clientID;
	wk->myType = clientType;
	wk->adapter = BTL_ADAPTER_Create( commMode, netHandle, heapID, clientID );
	wk->myParty = BTL_MAIN_GetPartyDataConst( mainModule, clientID );
	wk->mainModule = mainModule;
	wk->frontPokeIdx = 0;
	wk->cmdQue = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SERVER_CMD_QUE) );

	wk->myState = 0;

	return wk;
}

void BTL_CLIENT_Delete( BTL_CLIENT* wk )
{
	GFL_HEAP_FreeMemory( wk->cmdQue );
	BTL_ADAPTER_Delete( wk->adapter );

	GFL_HEAP_FreeMemory( wk );
}

void BTL_CLIENT_AttachViewCore( BTL_CLIENT* wk, BTLV_CORE* viewCore )
{
	wk->viewCore = viewCore;
}


BTL_ADAPTER* BTL_CLIENT_GetAdapter( BTL_CLIENT* wk )
{
	GF_ASSERT(wk);
	return wk->adapter;
}


void BTL_CLIENT_Main( BTL_CLIENT* wk )
{
	switch( wk->myState ){
	case 0:
		{
			BtlAdapterCmd  cmd = BTL_ADAPTER_RecvCmd(wk->adapter);
			if( cmd != BTL_ACMD_NONE )
			{
				wk->subProc = getSubProc( wk, cmd );
				wk->subSeq = 0;
				wk->myState = 1;
				BTL_Printf("  [CL]  %d, コマンド%dを受け取りました\n", wk->myID, cmd);
			}
		}
		break;

	case 1:
		if( wk->subProc(wk, &wk->subSeq) )
		{
			BTL_ADAPTER_ReturnCmd( wk->adapter, wk->returnDataPtr, wk->returnDataSize );
			wk->myState = 0;
			BTL_Printf("  [CL]  %d, 返信しました\n", wk->myID );
		}
		break;
	}
}



//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

static BOOL SubProc_UI_NotifyPokeData( BTL_CLIENT* wk, int* seq )
{
//	wk->returnDataPtr = 
	return TRUE;
}
static BOOL SubProc_AI_NotifyPokeData( BTL_CLIENT* wk, int* seq )
{
	return TRUE;
}
static BOOL SubProc_UI_Initialize( BTL_CLIENT* wk, int* seq )
{
	switch( *seq ){
	case 0:
		BTLV_StartCommand( wk->viewCore, BTLV_CMD_SETUP );
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL SubProc_AI_Initialize( BTL_CLIENT* wk, int *seq )
{
	
	return TRUE;
}

//--------------
static BOOL SubProc_UI_SelectAction( BTL_CLIENT* wk, int* seq )
{
	switch( *seq ){
	case 0:
		BTLV_StartCommand( wk->viewCore, BTLV_CMD_SELECT_ACTION );
		(*seq)++;
		break;

	case 1:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			BTLV_GetActionParam( wk->viewCore, &wk->actionParam );
			wk->returnDataPtr = &(wk->actionParam);
			wk->returnDataSize = sizeof(wk->actionParam);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq )
{
	const BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, wk->myID );
	u8 wazaCount = BTL_POKEPARAM_GetWazaCount( pp );

	BTL_ACTION_SetFightParam( &wk->actionParam, GFL_STD_MtRand(wazaCount), 0 );
	wk->returnDataPtr = &(wk->actionParam);
	wk->returnDataSize = sizeof(wk->actionParam);

	return TRUE;
}
//---------------------------------------------------
// 自分のポケモンが死んでいたら次を選択する処理
//---------------------------------------------------
static BOOL SubProc_UI_SelectPokemon( BTL_CLIENT* wk, int* seq )
{
	switch( *seq ){
	case 0:
		{
			const BTL_POKEPARAM* frontPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, wk->frontPokeIdx );
			if( BTL_POKEPARAM_GetValue(frontPoke, BPP_HP) == 0 )
			{
				BTL_Printf("[CL] myID=%d 先頭ポケが死んだのでポケモン選択\n", wk->myID);
				BTLV_StartCommand( wk->viewCore, BTLV_CMD_SELECT_POKEMON );
				(*seq)++;
			}
			else
			{
				BTL_ACTION_SetNULL( &wk->actionParam );
				wk->returnDataPtr = &(wk->actionParam);
				wk->returnDataSize = sizeof(wk->actionParam);
				return TRUE;
			}
		}
		break;

	case 1:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			BTLV_GetActionParam( wk->viewCore, &wk->actionParam );
			wk->returnDataPtr = &(wk->actionParam);
			wk->returnDataSize = sizeof(wk->actionParam);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq )
{
	const BTL_POKEPARAM* pp = BTL_PARTY_GetMemberDataConst( wk->myParty, wk->frontPokeIdx );

	if( BTL_POKEPARAM_GetValue(pp, BPP_HP) == 0 )
	{
		u8 alivePokeIdx[ TEMOTI_POKEMAX ];
		u8 alivePokeCnt = 0;
		int max, i;

		BTL_Printf("[CL] id=%d : 先頭ポケ(%d)は死んだ→ポケモン選択開始\n", wk->myID, wk->frontPokeIdx);

		max = BTL_PARTY_GetMemberCount( wk->myParty );
		for(i=0; i<max; i++)
		{
			pp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
			if( BTL_POKEPARAM_GetValue(pp, BPP_HP) )
			{
				alivePokeIdx[ alivePokeCnt++ ] = i;
			}
		}

		GF_ASSERT(alivePokeCnt);

		i = GFL_STD_MtRand( alivePokeCnt );
		wk->frontPokeIdx = alivePokeIdx[i];

		BTL_ACTION_SetChangeParam( &wk->actionParam, wk->frontPokeIdx );
	}
	else
	{
		BTL_Printf("[CL] id=%d : 先頭ポケ(%d)は生きてる→ポケモン選択待ち\n", wk->myID, wk->frontPokeIdx);
		BTL_ACTION_SetNULL( &wk->actionParam );
	}

	wk->returnDataPtr = &(wk->actionParam);
	wk->returnDataSize = sizeof(wk->actionParam);
	return TRUE;
}
//---------------------------------------------------
// サーバコマンド処理
//---------------------------------------------------
static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq )
{
	switch( *seq ){
	case 0:
		{
			u16 cmdSize;
			const void* cmdBuf;

			cmdSize = BTL_ADAPTER_GetRecvData( wk->adapter, &cmdBuf );
			SCQUE_Setup( wk->cmdQue, cmdBuf, cmdSize );
			(*seq)++;
		}
		/* fallthru */
	case 1:
		if( !SCQUE_IsFinishRead(wk->cmdQue) )
		{
			wk->serverCmd = SCQUE_Read( wk->cmdQue, wk->cmdArgs );

			BTL_Printf( "[CL] serverCmd=%d\n", wk->serverCmd );

			if( wk->serverCmd == SC_DATA_WAZA_EXE )
			{
				BTL_WAZA_EXE_PARAM* wep;
				const BTL_POKEPARAM* poke;
				int i;
				u8 userClientID;

//				BTL_Printf(" WAZADATA PROC .... client=%d\n", wk->cmdArgs[0]);

				userClientID = wk->cmdArgs[0];
				wep = &wk->wazaExeParam[ userClientID ];
				wep->userPokeParam = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, userClientID );
				poke = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, userClientID );
				wep->waza = BTL_POKEPARAM_GetWazaNumber( poke, wk->cmdArgs[1] );
				wep->numTargetClients = wk->cmdArgs[2];
				for(i=0; i<wep->numTargetClients; i++)
				{
					wep->targetClientID[i] = wk->cmdArgs[3 + i];
				}
			}
			else if( wk->serverCmd == SC_DATA_MEMBER_IN )
			{
				u8 clientID = wk->cmdArgs[0];
				u8 memberIdx = wk->cmdArgs[1];

				BTL_Printf("[CL] MEMBER IN .... client=%d, memberIdx=%d\n", clientID, memberIdx);

				BTL_MAIN_SetFrontMemberIdx( wk->mainModule, clientID, memberIdx );
				if( wk->myID == clientID )
				{
					wk->frontPokeIdx = memberIdx;
				}

				BTLV_StartMemberChangeAct( wk->viewCore, clientID, memberIdx );
				(*seq) = 10;
			}
			else if( wk->serverCmd == SC_MSG_STD )
			{
				u16 msgID = wk->cmdArgs[0];
				BTLV_StartMsgStd( wk->viewCore, msgID, wk->cmdArgs );
				(*seq) = 2;
			}
			else if( wk->serverCmd == SC_MSG_SET )
			{
				u16 msgID = wk->cmdArgs[0];
				BTLV_StartMsgSet( wk->viewCore, msgID, &wk->cmdArgs[1] );
				(*seq) = 2;
			}
			else if( wk->serverCmd == SC_MSG_WAZA )
			{
				u8 clientID = wk->cmdArgs[0];
				u16 wazaIdx = wk->cmdArgs[1];
				const BTL_POKEPARAM* poke = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, clientID );
				wazaIdx = BTL_POKEPARAM_GetWazaNumber( poke, wazaIdx );
				BTLV_StartMsgWaza( wk->viewCore, clientID, wazaIdx );
				(*seq) = 4;
			}
			else if( wk->serverCmd == SC_ACT_WAZA_DMG )
			{
				WazaID waza;
				u8 atClientID, defClientID, affinity, wazaIdx;
				u16 damage;
				const BTL_PARTY* party;
				const BTL_POKEPARAM* poke;

				atClientID	= wk->cmdArgs[0];
				defClientID	= wk->cmdArgs[1];
				damage		= wk->cmdArgs[2];
				wazaIdx		= wk->cmdArgs[3];
				affinity	= wk->cmdArgs[4];

				//party = BTL_MAIN_GetPartyDataConst( wk->mainModule, atClientID );
				//poke = BTL_PARTY_GetMemberDataConst( party, 0 );
				poke = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, atClientID );
				waza = BTL_POKEPARAM_GetWazaNumber( poke, wazaIdx );

				BTL_Printf("[CL] WazaAct aff=%d, damage=%d\n", affinity, damage);

				BTLV_StartWazaAct( wk->viewCore, atClientID, defClientID, damage, waza, affinity );
				(*seq) = 5;
			}
			else if( wk->serverCmd == SC_ACT_DEAD )
			{
				BTLV_StartDeadAct( wk->viewCore, wk->cmdArgs[0] );
				(*seq)=6;
			}
			else if( wk->serverCmd == SC_TOKWIN_IN )
			{
				BTLV_StartTokWin( wk->viewCore, wk->cmdArgs[0] );
			}
			else if( wk->serverCmd == SC_TOKWIN_OUT )
			{
				BTLV_QuitTokWin( wk->viewCore, wk->cmdArgs[0] );
			}
			else if( wk->serverCmd == SC_ACT_RANKDOWN )
			{
				BTLV_StartRankDownEffect( wk->viewCore, wk->cmdArgs[0], wk->cmdArgs[1] );
			}
			else
			{
				BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, wk->cmdArgs[0] );

				BTL_Printf(" OP PROC .... cl=%d, val=%d\n", wk->cmdArgs[0], wk->cmdArgs[1] );

				switch( wk->serverCmd ){
				case SC_OP_HP_MINUS:		///< 【計算】HPマイナス  [ClientID, マイナス量]
					BTL_POKEPARAM_HpMinus( pp, wk->cmdArgs[1] );
					break;
				case SC_OP_HP_PLUS:			///< 【計算】HPプラス    [ClientID, プラス量]
				case SC_OP_PP_MINUS:		///< 【計算】PPマイナス  [ClientID, マイナス量]
				case SC_OP_PP_PLUS:			///< 【計算】PPプラス    [ClientID, プラス量]
					break;
				case SC_OP_RANK_DOWN:
					BTL_POKEPARAM_RankDown( pp, wk->cmdArgs[1], wk->cmdArgs[2] );
					break;
				default:
					GF_ASSERT(0);
					break;
				}
			}
		}
		else
		{
			BTL_Printf("[CL] サーバーコマンド読み終わりましたよっと\n");
			return TRUE;
		}
		break;

	case 2:
		if( BTLV_WaitMsg(wk->viewCore) )
		{
			(*seq)++;
		}
		break;
	case 3:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			(*seq)=1;
		}
		break;

	case 4:
		if( BTLV_WaitMsg(wk->viewCore) )
		{
			(*seq) = 1;
		}
		break;

	case 5:
		if( BTLV_WaitWazaAct(wk->viewCore) )
		{
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
			{
				(*seq)=1;
			}
		}
		break;

	case 6:
		if( BTLV_WaitDeadAct(wk->viewCore) )
		{
			(*seq)=1;
		}
		break;

	case 10:
		if( BTLV_WaitMemberChangeAct(wk->viewCore) )
		{
			(*seq)=1;
		}
		break;

	default:
		BTL_Printf("[CL] サーバーコマンド不正処理しましたよっと\n");
		return TRUE;
	}

	return FALSE;
}

static BOOL SubProc_AI_ServerCmd( BTL_CLIENT* wk, int* seq )
{
	return TRUE;
}

static ClientSubProc getSubProc( BTL_CLIENT* wk, BtlAdapterCmd cmd )
{
	static const struct {
		BtlAdapterCmd   cmd;
		ClientSubProc   procs[ BTL_THINKER_TYPE_MAX ];
	}procTbl[] = {

		{ BTL_ACMD_NOTIFY_POKEDATA,	{ SubProc_UI_NotifyPokeData, SubProc_AI_NotifyPokeData }, },
		{ BTL_ACMD_WAIT_INITIALIZE,	{ SubProc_UI_Initialize, SubProc_AI_Initialize } },
		{ BTL_ACMD_SELECT_ACTION,	{ SubProc_UI_SelectAction, SubProc_AI_SelectAction } },
		{ BTL_ACMD_SELECT_POKEMON,	{ SubProc_UI_SelectPokemon,SubProc_AI_SelectPokemon } },
		{ BTL_ACMD_SERVER_CMD,		{ SubProc_UI_ServerCmd, SubProc_AI_ServerCmd } },

	};

	int i;

	for(i=0; i<NELEMS(procTbl); i++)
	{
		if( procTbl[i].cmd == cmd )
		{
			return procTbl[i].procs[ wk->myType ];
		}
	}

	GF_ASSERT(0);
	return NULL;
}




//------------------------------------------------------------------------------------------------------
// 外部モジュールからの情報取得処理
//------------------------------------------------------------------------------------------------------

u8 BTL_CLIENT_GetClientID( const BTL_CLIENT* client )
{
	return client->myID;
}

u8 BTL_CLIENT_GetOpponentClientID( const BTL_CLIENT* client, u8 idx )
{
	return BTL_MAIN_GetOpponentClientID( client->mainModule, client->myID, idx );
}

const BTL_PARTY* BTL_CLIENT_GetParty( const BTL_CLIENT* client )
{
	return client->myParty;
}

u8 BTL_CLIENT_GetFrontPokeIdx( const BTL_CLIENT* client )
{
	return client->frontPokeIdx;
}

const BTL_POKEPARAM* BTL_CLIENT_GetFrontPokeParam( const BTL_CLIENT* client )
{
	return BTL_PARTY_GetMemberDataConst( client->myParty, client->frontPokeIdx );
}

const BTL_WAZA_EXE_PARAM* BTL_CLIENT_GetWazaExeParam( const BTL_CLIENT* client, u8 clientID )
{
	return &client->wazaExeParam[ clientID ];
}

//----------

const BTL_POKEPARAM*  BTL_CLIENT_WEP_GetUserPokeParam( const BTL_WAZA_EXE_PARAM* wep )
{
	return wep->userPokeParam;
}

WazaID  BTL_CLIENT_WEP_GetWazaNumber( const BTL_WAZA_EXE_PARAM* wep )
{
	return wep->waza;
}


