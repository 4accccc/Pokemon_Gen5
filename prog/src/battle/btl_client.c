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
enum {
	FRONTPOKE_EMPTY = 0xff,		///< 戦闘ポケのインデックスとして無効（死亡時などに使用）
};


/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*ClientSubProc)( BTL_CLIENT*, int* );
typedef BOOL (*ServerCmdProc)( BTL_CLIENT*, int*, const int* );

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
	u8					frontPokeIdx[ BTL_POSIDX_MAX ];
	u8					numCoverPos;	///< 担当する戦闘ポケモン数
	u8					procPokeIdx;	///< 処理中ポケモンインデックス
	BtlPokePos	basePos;			///< 戦闘ポケモンの位置ID

	BTL_ACTION_PARAM	actionParam[ BTL_POSIDX_MAX ];
	BTL_SERVER_CMD_QUE*	cmdQue;
	int					cmdArgs[ BTL_SERVERCMD_ARG_MAX ];
	ServerCmd			serverCmd;
	ServerCmdProc		scProc;
	int					scSeq;



	BTL_WAZA_EXE_PARAM	wazaExeParam[ BTL_POS_MAX ];

	u8	myID;
	u8	myType;
	u8	myState;

};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static ClientSubProc getSubProc( BTL_CLIENT* wk, BtlAdapterCmd cmd );
static BOOL SubProc_UI_NotifyPokeData( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_NotifyPokeData( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_Initialize( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_Initialize( BTL_CLIENT* wk, int *seq );
static BOOL SubProc_UI_SelectAction( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_SelectPokemon( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_ServerCmd( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq );
static BOOL scProc_DATA_WazaExe( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_DATA_MemberIn( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_Std( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_Set( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_Waza( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WazaDmg( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Dead( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_RankDownEffect( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_TOKWIN_In( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_TOKWIN_Out( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_HpMinus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_HpPlus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPMinus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPPlus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankDown( BTL_CLIENT* wk, int* seq, const int* args );



BTL_CLIENT* BTL_CLIENT_Create(
	BTL_MAIN_MODULE* mainModule, BtlCommMode commMode, GFL_NETHANDLE* netHandle,
	u16 clientID, u16 numCoverPos, BtlThinkerType clientType, HEAPID heapID )
{
	BTL_CLIENT* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_CLIENT) );
	int i;

	wk->myID = clientID;
	wk->myType = clientType;
	wk->adapter = BTL_ADAPTER_Create( netHandle, heapID, clientID );
	wk->myParty = BTL_MAIN_GetPartyDataConst( mainModule, clientID );
	wk->mainModule = mainModule;
	wk->numCoverPos = numCoverPos;
	wk->procPokeIdx = 0;
	wk->basePos = clientID & 1;
	for(i=0; i<NELEMS(wk->frontPokeIdx); i++)
	{
		wk->frontPokeIdx[i] = i;
	}
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
		BTL_Printf(" [CL] 画面構築します\n");
		BTLV_StartCommand( wk->viewCore, BTLV_CMD_SETUP );
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			BTL_Printf(" [CL] 画面構築おわりました\n");
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
		wk->procPokeIdx = 0;
		(*seq)++;
		/* fallthru */
	case 1:
		BTL_Printf(" [CL] アクション選択(%d体目）開始します\n", wk->procPokeIdx);
		BTLV_StartCommand( wk->viewCore, BTLV_CMD_SELECT_ACTION );
		(*seq)++;
		break;
	case 2:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			BTLV_GetActionParam( wk->viewCore, &wk->actionParam[wk->procPokeIdx] );
			wk->procPokeIdx++;
			if( wk->procPokeIdx >= wk->numCoverPos )
			{
				wk->returnDataPtr = &(wk->actionParam[0]);
				wk->returnDataSize = sizeof(wk->actionParam[0]) * wk->numCoverPos;
				return TRUE;
			}
			else
			{
				(*seq) = 1;
			}
		}
		break;
	}

	return FALSE;
}

static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq )
{
	const BTL_POKEPARAM* pp;
	u8 wazaCount, i;

	for(i=0; i<wk->numCoverPos; i++)
	{
		pp = BTL_CLIENT_GetFrontPokeData( wk, 0 );
		wazaCount = BTL_POKEPARAM_GetWazaCount( pp );
		BTL_ACTION_SetFightParam( &wk->actionParam[i], GFL_STD_MtRand(wazaCount), 0 );
	}
	wk->returnDataPtr = &(wk->actionParam[0]);
	wk->returnDataSize = sizeof(wk->actionParam[0]) * wk->numCoverPos;

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
			const BTL_POKEPARAM* frontPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, wk->frontPokeIdx[0] );
			if( BTL_POKEPARAM_GetValue(frontPoke, BPP_HP) == 0 )
			{
				BTL_Printf("[CL] myID=%d 先頭ポケが死んだのでポケモン選択\n", wk->myID);
				BTLV_StartCommand( wk->viewCore, BTLV_CMD_SELECT_POKEMON );
				(*seq)++;
			}
			else
			{
				BTL_ACTION_SetNULL( &wk->actionParam[0] );
				wk->returnDataPtr = &(wk->actionParam[0]);
				wk->returnDataSize = sizeof(wk->actionParam[0]);
				return TRUE;
			}
		}
		break;

	case 1:
		if( BTLV_WaitCommand(wk->viewCore) )
		{
			BTLV_GetActionParam( wk->viewCore, &wk->actionParam[0] );
			wk->returnDataPtr = &(wk->actionParam[0]);
			wk->returnDataSize = sizeof(wk->actionParam[0]);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq )
{
	const BTL_POKEPARAM* pp = BTL_PARTY_GetMemberDataConst( wk->myParty, wk->frontPokeIdx[0] );

	if( BTL_POKEPARAM_GetValue(pp, BPP_HP) == 0 )
	{
		u8 alivePokeIdx[ TEMOTI_POKEMAX ];
		u8 alivePokeCnt = 0;
		int max, i;

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
		wk->frontPokeIdx[0] = alivePokeIdx[i];

		BTL_ACTION_SetChangeParam( &wk->actionParam[0], wk->frontPokeIdx[0] );
	}
	else
	{
		BTL_Printf("[CL] id=%d : 先頭ポケ(%d)は生きてる→ポケモン選択待ち\n", wk->myID, wk->frontPokeIdx[0] );
		BTL_ACTION_SetNULL( &wk->actionParam[0] );
	}

	wk->returnDataPtr = &(wk->actionParam[0]);
	wk->returnDataSize = sizeof(wk->actionParam[0]);
	return TRUE;
}
//---------------------------------------------------
// サーバコマンド処理
//---------------------------------------------------
static BOOL SubProc_AI_ServerCmd( BTL_CLIENT* wk, int* seq )
{
	return TRUE;
}

static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq )
{
	static const struct {
		u32				cmd;
		ServerCmdProc	proc;
	}scprocTbl[] = {
		{	SC_DATA_WAZA_EXE,	scProc_DATA_WazaExe			},
		{	SC_DATA_MEMBER_IN,	scProc_DATA_MemberIn		},
		{	SC_MSG_STD,			scProc_MSG_Std				},
		{	SC_MSG_SET,			scProc_MSG_Set				},
		{	SC_MSG_WAZA,		scProc_MSG_Waza				},
		{	SC_ACT_WAZA_DMG,	scProc_ACT_WazaDmg			},
		{	SC_ACT_DEAD,		scProc_ACT_Dead				},
		{	SC_ACT_RANKDOWN,	scProc_ACT_RankDownEffect	},
		{	SC_TOKWIN_IN,		scProc_TOKWIN_In			},
		{	SC_TOKWIN_OUT,		scProc_TOKWIN_Out			},
		{	SC_OP_HP_MINUS,		scProc_OP_HpMinus			},
		{	SC_OP_HP_PLUS,		scProc_OP_HpPlus			},
		{	SC_OP_PP_MINUS,		scProc_OP_PPMinus			},
		{	SC_OP_PP_PLUS,		scProc_OP_PPPlus			},
		{	SC_OP_RANK_DOWN,	scProc_OP_RankDown			},
	};




restart:

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
		if( SCQUE_IsFinishRead(wk->cmdQue) )
		{
			BTL_Printf("[CL] サーバーコマンド読み終わりましたよっと\n");
			return TRUE;
		}
		(*seq)++;
		/* fallthru */
	case 2:
		{
			int i;

			wk->serverCmd = SCQUE_Read( wk->cmdQue, wk->cmdArgs );

			for(i=0; i<NELEMS(scprocTbl); i++)
			{
				if( scprocTbl[i].cmd == wk->serverCmd ){ break; }
			}

			if( i == NELEMS(scprocTbl) )
			{
				BTL_Printf("[CL] 用意されていないコマンド処理！\n");
				return TRUE;
			}

			BTL_Printf( "[CL] serverCmd=%d\n", wk->serverCmd );
			wk->scProc = scprocTbl[i].proc;
			wk->scSeq = 0;
			(*seq)++;
		}
		/* fallthru */
	case 3:
		if( wk->scProc(wk, &wk->scSeq, wk->cmdArgs) )
		{
			(*seq) = 1;
			goto restart;
		}
		break;
	}

	return FALSE;
}


// データ設定：ワザ発動
static BOOL scProc_DATA_WazaExe( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_WAZA_EXE_PARAM* wep;
	const BTL_POKEPARAM* poke;
	int i;
	u8 userPos;

	userPos = args[0];
	wep = &wk->wazaExeParam[ userPos ];
	wep->userPokeParam = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, userPos );
	poke = wep->userPokeParam;
	wep->waza = BTL_POKEPARAM_GetWazaNumber( poke, args[1] );
	wep->numTargetClients = args[2];

	for(i=0; i<wep->numTargetClients; i++)
	{
		wep->targetClientID[i] = args[3 + i];
	}

	return TRUE;
}

static BOOL scProc_DATA_MemberIn( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ){
	case 0:
		{
			u8 clientID = wk->cmdArgs[0];
			u8 posIdx = wk->cmdArgs[1];
			u8 memberIdx = wk->cmdArgs[2];

			BTL_Printf("[CL] MEMBER IN .... client=%d, memberIdx=%d\n", clientID, memberIdx);

			if( wk->myID == clientID )
			{
				wk->frontPokeIdx[ posIdx ] = memberIdx;
			}

			BTLV_StartMemberChangeAct( wk->viewCore, clientID, memberIdx );
			(*seq)++;
		}
		break;
	case 1:
		if( BTLV_WaitMemberChangeAct(wk->viewCore) )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL scProc_MSG_Std( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ){
	case 0:
		BTLV_StartMsgStd( wk->viewCore, args[0], &args[1] );
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitMsg(wk->viewCore) )
		{
			(*seq)++;
		}
		break;
	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL scProc_MSG_Set( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ){
	case 0:
		BTLV_StartMsgSet( wk->viewCore, args[0], &args[1] );
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitMsg(wk->viewCore) )
		{
			(*seq)++;
		}
		break;
	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL scProc_MSG_Waza( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ){
	case 0:
		{
			u8  pokePos = args[0];
			u16 wazaIdx  = args[1];
			const BTL_POKEPARAM* poke = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, pokePos );
			wazaIdx = BTL_POKEPARAM_GetWazaNumber( poke, wazaIdx );
			BTLV_StartMsgWaza( wk->viewCore, pokePos, wazaIdx );
		}
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitMsg(wk->viewCore) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL scProc_ACT_WazaDmg( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ) {
	case 0:
	{
		WazaID waza;
		u8 atPokePos, defPokePos, affinity, wazaIdx;
		u16 damage;
		const BTL_PARTY* party;
		const BTL_POKEPARAM* poke;

		atPokePos		= args[0];
		defPokePos	= args[1];
		damage		= args[2];
		wazaIdx		= args[3];
		affinity	= args[4];

		poke = BTL_MAIN_GetFrontPokeDataConst( wk->mainModule, atPokePos );
		waza = BTL_POKEPARAM_GetWazaNumber( poke, wazaIdx );

		BTL_Printf("[CL] WazaAct aff=%d, damage=%d\n", affinity, damage);

		BTLV_StartWazaAct( wk->viewCore, atPokePos, defPokePos, damage, waza, affinity );
		(*seq)++;
	}
	break;

	case 1:
		if( BTLV_WaitWazaAct(wk->viewCore) )
		{
			(*seq)++;
		}
		break;

	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
		{
			return TRUE;
		}
		break;
	}

	return FALSE;
}

static BOOL scProc_ACT_Dead( BTL_CLIENT* wk, int* seq, const int* args )
{
	switch( *seq ){
	case 0:
		BTLV_StartDeadAct( wk->viewCore, args[0] );
		(*seq)++;
		break;
	case 1:
		if( BTLV_WaitDeadAct(wk->viewCore) )
		{
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL scProc_ACT_RankDownEffect( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTLV_StartRankDownEffect( wk->viewCore, args[0], args[1] );
	return TRUE;
}


static BOOL scProc_TOKWIN_In( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTLV_StartTokWin( wk->viewCore, args[0] );
	return TRUE;
}

static BOOL scProc_TOKWIN_Out( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTLV_QuitTokWin( wk->viewCore, args[0] );
	return TRUE;
}

static BOOL scProc_OP_HpMinus( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, args[0] );
	BTL_POKEPARAM_HpMinus( pp, args[1] );
	return TRUE;
}
static BOOL scProc_OP_HpPlus( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, args[0] );
	BTL_POKEPARAM_HpPlus( pp, args[1] );
	return TRUE;
}
static BOOL scProc_OP_PPMinus( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, args[0] );
	BTL_POKEPARAM_PPMinus( pp, args[1], args[2] );
	return TRUE;
}
static BOOL scProc_OP_PPPlus( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, args[0] );
	BTL_POKEPARAM_PPPlus( pp, args[1], args[2] );
	return TRUE;
}
static BOOL scProc_OP_RankDown( BTL_CLIENT* wk, int* seq, const int* args )
{
	BTL_POKEPARAM* pp = BTL_MAIN_GetFrontPokeData( wk->mainModule, args[0] );
	TAYA_Printf("[CL] OP - RankDown PokePos=%d, ppDat=%p\n", args[0], pp);
	BTL_POKEPARAM_RankDown( pp, args[1], args[2] );
	return TRUE;
}













//------------------------------------------------------------------------------------------------------
// 外部モジュールからの情報取得処理
//------------------------------------------------------------------------------------------------------

u8 BTL_CLIENT_GetClientID( const BTL_CLIENT* client )
{
	return client->myID;
}

const BTL_PARTY* BTL_CLIENT_GetParty( const BTL_CLIENT* client )
{
	return client->myParty;
}

//=============================================================================================
/**
 * 現在、行動選択処理中のポケモンデータを取得
 *
 * @param   client		
 *
 * @retval  const BTL_POKEPARAM*		
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_CLIENT_GetProcPokeData( const BTL_CLIENT* client )
{
	return BTL_CLIENT_GetFrontPokeData( client, client->procPokeIdx );
}
//=============================================================================================
/**
 * 現在、行動選択処理中のポケモン位置IDを取得
 *
 * @param   client		
 *
 * @retval  BtlPokePos		
 */
//=============================================================================================
BtlPokePos BTL_CLIENT_GetProcPokePos( const BTL_CLIENT* client )
{
	return client->basePos + client->procPokeIdx*2;
}
//=============================================================================================
/**
 * 戦闘に出ているポケモンのポケモンデータを取得
 *
 * @param   client		
 * @param   posIdx		
 *
 * @retval  const BTL_POKEPARAM*		
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_CLIENT_GetFrontPokeData( const BTL_CLIENT* client, u8 posIdx )
{
	GF_ASSERT_MSG(posIdx<client->numCoverPos, "posIdx=%d, numCoverPos=%d", posIdx, client->numCoverPos);
	return BTL_PARTY_GetMemberDataConst( client->myParty, client->frontPokeIdx[posIdx] );
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


