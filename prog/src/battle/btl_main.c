//=============================================================================================
/**
 * @file	btl_main.c
 * @brief	ポケモンWB バトルシステムメインモジュール
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "battle/battle.h"
#include "poke_mcss.h"

#include "btl_common.h"
#include "btl_server.h"
#include "btl_client.h"
#include "btl_adapter.h"
#include "btl_pokeparam.h"
#include "btl_net.h"
#include "btl_util.h"

#include "btlv_core.h"

#include "btl_main.h"




enum {
	PARENT_HEAP_ID = GFL_HEAPID_APP,

	BTL_COMMITMENT_POKE_MAX = BTL_CLIENT_MAX * TEMOTI_POKEMAX,
};



typedef BOOL (*pMainLoop)(BTL_MAIN_MODULE*);

typedef struct {
	u8	competitor;
	u8	rule;
	u8	engine;
	u8	comm_mode;
}BTL_BASIC_PARAMS;


struct _BTL_PARTY {
	BTL_POKEPARAM*  member[ TEMOTI_POKEMAX ];
	u8  memberCount;
};



struct _BTL_MAIN_MODULE {

	const BATTLE_SETUP_PARAM*	setupParam;
	BTL_BASIC_PARAMS			basicParam;

	BTLV_CORE*		viewCore;
	BTL_SERVER*		server;
	BTL_CLIENT*		client[ BTL_CLIENT_MAX ];

	// サーバが計算時に書き換えても良い一時使用パラメータ領域と、
	// サーバコマンドを受け取ったクライアントが実際に書き換えるパラメータ領域
	BTL_PARTY		party[ BTL_CLIENT_MAX ];
	BTL_POKEPARAM*	pokeParam[ BTL_COMMITMENT_POKE_MAX ];
	BTL_PARTY		partyForServerCalc[ BTL_CLIENT_MAX ];
	BTL_POKEPARAM*	pokeParamForServerCalc[ BTL_COMMITMENT_POKE_MAX ];

	u8				numClients;
	u8				myClientID;
	u8				myOrgPos;

	BTL_PROC		subProc;
	pMainLoop		mainLoop;


	HEAPID			heapID;

};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void setSubProcForSetup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static void setSubProcForClanup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static BOOL setup_alone_single( int* seq, void* work );
static BOOL cleanup_alone_single( int* seq, void* work );
static BOOL setup_comm_single( int* seq, void* work );
static BOOL setup_alone_double( int* seq, void* work );
static BOOL cleanup_alone_double( int* seq, void* work );
static void setupPokeParams( BTL_PARTY* dstParty, BTL_POKEPARAM** dstParams, const POKEPARTY* party, u8 pokeID_Origin );
static void cleanupPokeParams( BTL_PARTY* party );
static BOOL MainLoop_StandAlone( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk );
static u8 expandClientID_single( const BTL_MAIN_MODULE* wk, BtlExClientType exType, u8 clientID, u8* dst );
static u8 expandClientID_double( const BTL_MAIN_MODULE* wk, BtlExClientType exType, u8 clientID, u8* dst );
static inline u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos );
static inline void btlPos_to_cliendID_and_posIdx( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos, u8* clientID, u8* posIdx );
static void BTL_PARTY_Initialize( BTL_PARTY* party );
static void BTL_PARTY_Cleanup( BTL_PARTY* party );
static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member );


static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	switch( *seq ){
	case 0:
		{
			BTL_MAIN_MODULE* wk;
			const BATTLE_SETUP_PARAM* setup_param = pwk;

			GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_SYSTEM, 0x28000 );
			GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_NET,     0x8000 );
			GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_VIEW,   0xa0000 );

			wk = GFL_PROC_AllocWork( proc, sizeof(BTL_MAIN_MODULE), HEAPID_BTL_SYSTEM );
			GFL_STD_MemClear32( wk, sizeof(BTL_MAIN_MODULE) );

			wk->heapID = HEAPID_BTL_SYSTEM;
			wk->setupParam = setup_param;

			BTL_NET_InitSystem( setup_param->netHandle, HEAPID_BTL_NET );
			BTL_ADAPTERSYS_Init( setup_param->commMode );

			setSubProcForSetup( &wk->subProc, wk, setup_param );
			(*seq)++;
		}
		break;

	case 1:
		{
			BTL_MAIN_MODULE* wk = mywk;
			if( BTL_UTIL_CallProc(&wk->subProc) )
			{
				return GFL_PROC_RES_FINISH;
			}
		}
		break;

	}

	return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	BTL_MAIN_MODULE* wk = mywk;

	if( wk->mainLoop( wk ) )
	{
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;

}

static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	BTL_MAIN_MODULE* wk = mywk;

	switch( *seq ){
	case 0:
		setSubProcForClanup( &wk->subProc, wk, wk->setupParam );
		(*seq)++;
		break;

	case 1:
		if( BTL_UTIL_CallProc(&wk->subProc) )
		{
			BTL_ADAPTERSYS_Quit();
			BTL_NET_QuitSystem();
			(*seq)++;
		}
		break;

	case 2:
		GFL_PROC_FreeWork( proc );
		GFL_HEAP_DeleteHeap( HEAPID_BTL_VIEW );
		GFL_HEAP_DeleteHeap( HEAPID_BTL_NET );
		GFL_HEAP_DeleteHeap( HEAPID_BTL_SYSTEM );
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//======================================================================================================
//======================================================================================================

// 各種モジュール生成＆関連付けルーチンを設定
static void setSubProcForSetup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param )
{
	// @@@ 本来は setup_param を参照して各種初期化処理ルーチンを決定する
	if( setup_param->commMode == BTL_COMM_NONE )
	{
		switch( setup_param->rule ){
		case BTL_RULE_SINGLE:
			BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
			break;
		case BTL_RULE_DOUBLE:
			BTL_UTIL_SetupProc( bp, wk, setup_alone_double, NULL );
			break;
		default:
			GF_ASSERT(0);
			BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
			break;
		}
	}
	else
	{
		BTL_UTIL_SetupProc( bp, wk, setup_comm_single, NULL );
	}
}

// 各種モジュール解放＆削除処理ルーチンを設定
static void setSubProcForClanup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param )
{
	// @@@ 本来は setup_param を参照して各種初期化処理ルーチンを決定する
	if( setup_param->commMode == BTL_COMM_NONE )
	{
		switch( setup_param->rule ){
		case BTL_RULE_SINGLE:
			BTL_UTIL_SetupProc( bp, wk, cleanup_alone_single, NULL );
			break;
		case BTL_RULE_DOUBLE:
			BTL_UTIL_SetupProc( bp, wk, cleanup_alone_double, NULL );
			break;
		default:
			GF_ASSERT(0);
			BTL_UTIL_SetupProc( bp, wk, cleanup_alone_single, NULL );
			break;
		}
	}
	else
	{
		BTL_UTIL_SetupProc( bp, wk, cleanup_alone_single, NULL );
	}

}

//--------------------------------------------------------------------------
/**
 * スタンドアローン／シングルバトル
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_single( int* seq, void* work )
{
	// server*1, client*2
	BTL_MAIN_MODULE* wk = work;
	const BATTLE_SETUP_PARAM* sp = wk->setupParam;

	// バトル用ポケモンパラメータ＆パーティデータを生成
	setupPokeParams( &wk->party[0], &wk->pokeParam[0], sp->partyPlayer, 0 );
	setupPokeParams( &wk->party[1], &wk->pokeParam[TEMOTI_POKEMAX], sp->partyEnemy1, TEMOTI_POKEMAX );

	setupPokeParams( &wk->partyForServerCalc[0], &wk->pokeParamForServerCalc[0], sp->partyPlayer, 0 );
	setupPokeParams( &wk->partyForServerCalc[1], &wk->pokeParamForServerCalc[TEMOTI_POKEMAX], sp->partyEnemy1, TEMOTI_POKEMAX );

	// Server 作成
	wk->server = BTL_SERVER_Create( wk, wk->heapID );

	// Client 作成
	wk->client[0] = BTL_CLIENT_Create( wk, BTL_COMM_NONE, sp->netHandle, 0, 1, BTL_THINKER_UI, wk->heapID );
	wk->client[1] = BTL_CLIENT_Create( wk, BTL_COMM_NONE, sp->netHandle, 1, 1, BTL_THINKER_AI, wk->heapID );
	wk->numClients = 2;
	wk->myClientID = 0;
	wk->myOrgPos = BTL_POS_1ST_0;

	// Server に Client を接続
	BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), &wk->partyForServerCalc[0], 0 );
	BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), &wk->partyForServerCalc[1], 1 );

	// 描画エンジン生成
	wk->viewCore = BTLV_Create( wk, wk->client[0], HEAPID_BTL_VIEW );

	// プレイヤークライアントに描画エンジンを関連付ける
	BTL_CLIENT_AttachViewCore( wk->client[0], wk->viewCore );

	wk->mainLoop = MainLoop_StandAlone;

	return TRUE;
}
static BOOL cleanup_alone_single( int* seq, void* work )
{
	BTL_MAIN_MODULE* wk = work;
	const BATTLE_SETUP_PARAM* sp = wk->setupParam;

	BTLV_Delete( wk->viewCore );

	BTL_CLIENT_Delete( wk->client[0] );
	BTL_CLIENT_Delete( wk->client[1] );
	BTL_SERVER_Delete( wk->server );

	cleanupPokeParams( &wk->party[0] );
	cleanupPokeParams( &wk->party[1] );
	cleanupPokeParams( &wk->partyForServerCalc[0] );
	cleanupPokeParams( &wk->partyForServerCalc[1] );


	return TRUE;
}
//--------------------------------------------------------------------------
/**
 * 通信／シングルバトル
 */
//--------------------------------------------------------------------------
static BOOL setup_comm_single( int* seq, void* work )
{
	BTL_MAIN_MODULE* wk = work;
	const BATTLE_SETUP_PARAM* sp = wk->setupParam;

	switch( *seq ){
	case 0:
		if( BTL_NET_IsServerDetermained() )
		{
			BTL_Printf("サーバ決定しましたよ\n");
			if( BTL_NET_IsServer() )
			{
				// サーバーマシンが、各参加者にクライアントIDを割り振る
				u8 clientID_0, clientID_1;

				clientID_0 = 0;
				clientID_1 = 1;

				BTL_Printf("ワシ、サーバーです。\n");

				BTL_NET_NotifyClientID( clientID_0, &clientID_0, 1 );
				BTL_NET_NotifyClientID( clientID_1, &clientID_1, 1 );
			}
			else
			{
				BTL_Printf("ワシ、サーバーではない。\n");
			}
			(*seq)++;
		}
		break;
	case 1:
		#ifdef PM_DEBUG
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
		{
			BTL_Printf("クライアントIDの確定待ちでーす\n");
		}
		#endif
		// 自分のクライアントIDが確定
		if( BTL_NET_IsClientIdDetermined() )
		{
			BTL_Printf("クライアントIDが確定した→タイミングシンクロ\n");
			BTL_NET_TimingSyncStart( BTL_NET_TIMING_CLIENTID_DETERMINE );
			(*seq)++;
		}
		break;
	case 2:
		if( BTL_NET_IsTimingSync(BTL_NET_TIMING_CLIENTID_DETERMINE) )
		{
			BTL_Printf("タイミングシンクロしました。\n");
			BTL_NET_StartNotifyPartyData( sp->partyPlayer );
			(*seq)++;
		}
		break;
	case 3:
		// パーティデータ相互受信を完了
		if( BTL_NET_IsCompleteNotifyPartyData() )
		{
			BTL_Printf("パーティデータ相互受信できました。\n");
			setupPokeParams( &wk->party[0], &wk->pokeParam[0], BTL_NET_GetPartyData(0), 0 );
			setupPokeParams( &wk->party[1], &wk->pokeParam[TEMOTI_POKEMAX], BTL_NET_GetPartyData(1), TEMOTI_POKEMAX );
			(*seq)++;
		}
		break;
	case 4:
		if( BTL_NET_IsServer() )
		{
			setupPokeParams( &wk->partyForServerCalc[0], &wk->pokeParamForServerCalc[0], BTL_NET_GetPartyData(0), 0 );
			setupPokeParams( &wk->partyForServerCalc[1], &wk->pokeParamForServerCalc[TEMOTI_POKEMAX], BTL_NET_GetPartyData(1), TEMOTI_POKEMAX );
		}
		(*seq)++;
		break;
	case 5:
		BTL_NET_EndNotifyPartyData();
		(*seq)++;
		break;
	case 6:
		wk->numClients = 2;
		wk->myClientID = GFL_NET_GetNetID( sp->netHandle );
		wk->myOrgPos = (wk->myClientID == 0)? BTL_POS_1ST_0 : BTL_POS_2ND_0;

		// 自分がサーバ
		if( BTL_NET_IsServer() )
		{
			u8 netID = GFL_NET_GetNetID( sp->netHandle );

			BTL_Printf("サーバ用のパーティデータセット\n");

			wk->server = BTL_SERVER_Create( wk, wk->heapID );
			wk->client[netID] = BTL_CLIENT_Create( wk, sp->commMode, sp->netHandle, netID, 1, BTL_THINKER_UI, wk->heapID );
			BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[netID]), &wk->partyForServerCalc[netID], netID );

			BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, &wk->partyForServerCalc[!netID], !netID );

			// 描画エンジン生成
			wk->viewCore = BTLV_Create( wk, wk->client[netID], HEAPID_BTL_VIEW );
			BTL_CLIENT_AttachViewCore( wk->client[netID], wk->viewCore );

			wk->mainLoop = MainLoop_Comm_Server;
		}
		// 自分がサーバではない
		else
		{
			u8 netID = GFL_NET_GetNetID( sp->netHandle );

			BTL_Printf("サーバではない用のパーティデータセット\n");

			wk->client[ netID ] = BTL_CLIENT_Create( wk, sp->commMode, sp->netHandle, netID, 1, BTL_THINKER_UI, wk->heapID );

			// 描画エンジン生成
			wk->viewCore = BTLV_Create( wk, wk->client[netID], HEAPID_BTL_VIEW );
			BTL_CLIENT_AttachViewCore( wk->client[netID], wk->viewCore );

			wk->mainLoop = MainLoop_Comm_NotServer;
		}
		(*seq)++;
		break;
	case 7:
		BTL_Printf("セットアップ完了\n");
		return TRUE;
	}
	return FALSE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／ダブルバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_double( int* seq, void* work )
{
	// server*1, client*2
	BTL_MAIN_MODULE* wk = work;
	const BATTLE_SETUP_PARAM* sp = wk->setupParam;

	// バトル用ポケモンパラメータ＆パーティデータを生成
	setupPokeParams( &wk->party[0], &wk->pokeParam[0], sp->partyPlayer, 0 );
	setupPokeParams( &wk->party[1], &wk->pokeParam[TEMOTI_POKEMAX], sp->partyEnemy1, TEMOTI_POKEMAX );

	setupPokeParams( &wk->partyForServerCalc[0], &wk->pokeParamForServerCalc[0], sp->partyPlayer, 0 );
	setupPokeParams( &wk->partyForServerCalc[1], &wk->pokeParamForServerCalc[TEMOTI_POKEMAX], sp->partyEnemy1, TEMOTI_POKEMAX );

	// Server 作成
	wk->server = BTL_SERVER_Create( wk, wk->heapID );

	// Client 作成
	wk->client[0] = BTL_CLIENT_Create( wk, BTL_COMM_NONE, sp->netHandle, 0, 2, BTL_THINKER_UI, wk->heapID );
	wk->client[1] = BTL_CLIENT_Create( wk, BTL_COMM_NONE, sp->netHandle, 1, 2, BTL_THINKER_AI, wk->heapID );
	wk->numClients = 2;

	// Server に Client を接続
	BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), &wk->partyForServerCalc[0], 0 );
	BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), &wk->partyForServerCalc[1], 1 );

	// 描画エンジン生成
	wk->viewCore = BTLV_Create( wk, wk->client[0], HEAPID_BTL_VIEW );

	// プレイヤークライアントに描画エンジンを関連付ける
	BTL_CLIENT_AttachViewCore( wk->client[0], wk->viewCore );

	wk->mainLoop = MainLoop_StandAlone;

	return TRUE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／ダブルバトル：クリーンアップ
 */
//--------------------------------------------------------------------------
static BOOL cleanup_alone_double( int* seq, void* work )
{
	BTL_MAIN_MODULE* wk = work;
	const BATTLE_SETUP_PARAM* sp = wk->setupParam;

	BTLV_Delete( wk->viewCore );

	BTL_CLIENT_Delete( wk->client[0] );
	BTL_CLIENT_Delete( wk->client[1] );
	BTL_SERVER_Delete( wk->server );

	cleanupPokeParams( &wk->party[0] );
	cleanupPokeParams( &wk->party[1] );
	cleanupPokeParams( &wk->partyForServerCalc[0] );
	cleanupPokeParams( &wk->partyForServerCalc[1] );


	return TRUE;
}

//--------------------------------------------------------------------------
/**
 * バトル用ポケモンパラメータ＆パーティデータを生成
 *
 * @param   dstParty		[out] パーティデータ生成先アドレス
 * @param   dstParams		[out] ポケモンパラメータ生成先配列アドレス
 * @param   party				[in] 生成元のフィールド用ポケモンパーティデータ
 *
 */
//--------------------------------------------------------------------------
static void setupPokeParams( BTL_PARTY* dstParty, BTL_POKEPARAM** dstParams, const POKEPARTY* party, u8 pokeID_Origin )
{
	int i, max = PokeParty_GetPokeCount( party );

	BTL_PARTY_Initialize( dstParty );
	for(i=0; i<max; i++)
	{
		dstParams[i] = BTL_POKEPARAM_Create( PokeParty_GetMemberPointer(party, i), pokeID_Origin+i, HEAPID_BTL_SYSTEM );
		BTL_PARTY_AddMember( dstParty, dstParams[i] );
	}
}

static void cleanupPokeParams( BTL_PARTY* party )
{
	BTL_PARTY_Cleanup( party );
}


//======================================================================================================
//======================================================================================================


static BOOL MainLoop_StandAlone( BTL_MAIN_MODULE* wk )
{
	int i;

	if( BTL_SERVER_Main( wk->server ) )
	{
		BTL_Printf("server end msg recved\n");
		return TRUE;
	}

	for(i=0; i<2; i++)
	{
		BTL_CLIENT_Main( wk->client[i] );
	}

	BTLV_CORE_Main( wk->viewCore );

	return FALSE;
}


static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk )
{
	int i;

	if( BTL_SERVER_Main( wk->server ) )
	{
		BTL_Printf("server end msg recved\n");
		return TRUE;
	}

	for(i=0; i<2; i++)
	{
		if( wk->client[i] )
		{
			BTL_CLIENT_Main( wk->client[i] );
		}
	}

	BTLV_CORE_Main( wk->viewCore );

	return FALSE;
}

static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk )
{
	int i;

	for(i=0; i<2; i++)
	{
		if( wk->client[i] )
		{
			BTL_CLIENT_Main( wk->client[i] );
		}
	}

	BTLV_CORE_Main( wk->viewCore );

	return FALSE;
}


//--------------------------------------------------------------
/**
 *	Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA BtlProcData = {
	BTL_PROC_Init,
	BTL_PROC_Main,
	BTL_PROC_Quit,
};





//======================================================================================================
// クライアント等からの問い合わせ対応
//======================================================================================================


//=============================================================================================
/**
 * 戦闘のルール指定子を返す
 *
 * @param   wk		
 *
 * @retval  BtlRule		
 */
//=============================================================================================
BtlRule BTL_MAIN_GetRule( const BTL_MAIN_MODULE* wk )
{
	return wk->setupParam->rule;
}


//=============================================================================================
/**
 * 対戦相手タイプを返す
 *
 * @param   wk		
 *
 * @retval  BtlCompetitor		
 */
//=============================================================================================
BtlCompetitor BTL_MAIN_GetCompetitor( const BTL_MAIN_MODULE* wk )
{
	return wk->setupParam->competitor;
}

//=============================================================================================
/**
 * 特殊クライアントID指定子を、実際の対象クライアントIDに変換
 *
 * @param   wk						メインモジュールハンドラ
 * @param   exID					特殊クライアントID指定子
 * @param   dst						[out] 対象クライアントIDを格納するバッファ
 *
 * @retval  u8		対象クライアント数
 */
//=============================================================================================
u8 BTL_MAIN_ExpandClientID( const BTL_MAIN_MODULE* wk, BtlExClientID exID, u8* dst )
{
	u8 exType = EXID_GET_TYPE( exID );
	u8 clientID = EXID_GET_CLIENTID( exID );

	if( exType == BTL_EXCLIENT_DEFAULT )
	{
		dst[0] = clientID;
		return 1;
	}

	switch( wk->setupParam->rule ){
	case BTL_RULE_SINGLE:
		return expandClientID_single( wk, exType, clientID, dst );
	case BTL_RULE_DOUBLE:
	default:
		return expandClientID_double( wk, exType, clientID, dst );
	}
}
// シングル用
static u8 expandClientID_single( const BTL_MAIN_MODULE* wk, BtlExClientType exType, u8 clientID, u8* dst )
{
	switch( exType ){
	default:
		GF_ASSERT(0);
		/* fallthru */
	case BTL_EXCLIENT_ENEMY_ALL:
	case BTL_EXCLIENT_ENEMY_RANDOM:
	case BTL_EXCLIENT_WITHOUT_ME:
		dst[0] = BTL_MAIN_GetOpponentClientID( wk, clientID, 0 );
		break;
	}
	return 1;
}
// ダブル用
static u8 expandClientID_double( const BTL_MAIN_MODULE* wk, BtlExClientType exType, u8 clientID, u8* dst )
{
	switch( exType ){
	default:
		GF_ASSERT(0);
		/* fallthru */
	case BTL_EXCLIENT_ENEMY_ALL:
		dst[0] = BTL_MAIN_GetOpponentClientID( wk, clientID, 0 );
		dst[1] = BTL_MAIN_GetOpponentClientID( wk, clientID, 1 );
		return 2;
	case BTL_EXCLIENT_ENEMY_RANDOM:
		dst[0] = BTL_MAIN_GetOpponentClientID( wk, clientID, GFL_STD_MtRand(1) );
		return 1;
	case BTL_EXCLIENT_WITHOUT_ME:
		dst[0] = BTL_MAIN_GetOpponentClientID( wk, clientID, 0 );
		dst[1] = BTL_MAIN_GetOpponentClientID( wk, clientID, 1 );
		dst[2] = BTL_MAIN_GetFriendClientID( wk, clientID, 1 );
		return 3;
	}
}

//=============================================================================================
/**
 * 相手方クライアントIDを返す
 *
 * @param   wk				
 * @param   clientID		
 * @param   idx				
 *
 * @retval  u8				
 */
//=============================================================================================
u8 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx )
{
	switch( wk->setupParam->rule ){
	case BTL_RULE_SINGLE:
		GF_ASSERT(idx==0);
		GF_ASSERT(clientID<2);
		return !clientID;

	case BTL_RULE_DOUBLE:
		GF_ASSERT(idx<2);
		GF_ASSERT(clientID<4);
		return !(clientID&1) + (idx*2);

	default:
		GF_ASSERT(0);
		return !(clientID&1);
	}
}
//=============================================================================================
/**
 * 味方クライアントIDを返す
 *
 * @param   wk			
 * @param   cliendID	
 * @param   idx			
 *
 * @retval  u8			
 */
//=============================================================================================
u8 BTL_MAIN_GetFriendClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx )
{
	switch( wk->setupParam->rule ){
	case BTL_RULE_SINGLE:
		GF_ASSERT(idx==0);
		GF_ASSERT(clientID<2);
		return clientID;

	case BTL_RULE_DOUBLE:
		GF_ASSERT(idx<2);
		GF_ASSERT(clientID<4);
		return (clientID&1) + (idx*2);

	default:
		GF_ASSERT(0);
		return (clientID&1);
	}
}

//=============================================================================================
/**
 * ２つのクライアントIDが対戦相手同士のものかどうかを判別
 *
 * @param   wk		
 * @param   clientID1		
 * @param   clientID2		
 *
 * @retval  BOOL		対戦相手同士ならTRUE
 */
//=============================================================================================
BOOL BTL_MAIN_IsOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID1, u8 clientID2 )
{
	return (clientID1&1) != (clientID2&1);
}



//=============================================================================================
/**
 * 指定クライアントのパーティデータを返す
 *
 * @param   wk		
 * @param   clientID		
 *
 * @retval  const POKEPARTY*		
 */
//=============================================================================================
const BTL_PARTY* BTL_MAIN_GetPartyDataConst( const BTL_MAIN_MODULE* wk, u32 clientID )
{
	return &wk->party[ clientID ];
}
BTL_PARTY* BTL_MAIN_GetPartyData( BTL_MAIN_MODULE* wk, u32 clientID )
{
	return &wk->party[ clientID ];
}


//--------------------------------------------------------------------------
/**
 * 戦闘位置->クライアントIDに 変換
 *
 * @param   wk		
 * @param   btlPos		
 *
 * @retval  inline u8		
 */
//--------------------------------------------------------------------------
static inline u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos )
{
	// @@@ 本当はマルチなどの場合にこれじゃダメです
	return btlPos & 1;
}
//--------------------------------------------------------------------------
/**
 * 戦闘位置->クライアントID，位置インデクス（クライアントごと0オリジン，連番）に変換
 *
 * @param   wk				[in] メインモジュールのハンドラ
 * @param   btlPos		[in] 戦闘位置
 * @param   clientID	[out]
 * @param   posIdx		[out]
 *
 */
//--------------------------------------------------------------------------
static inline void btlPos_to_cliendID_and_posIdx( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos, u8* clientID, u8* posIdx )
{
	*clientID = btlPos_to_clientID( wk, btlPos );

	// @@@ 本当はマルチなどの場合にこれじゃダメです
	*posIdx = btlPos / 2;
}

//=============================================================================================
/**
 * 戦闘に出ているポケモンデータの取得
 *
 * @param   wk		メインモジュールのハンドラ
 * @param   pos		立ち位置
 *
 * @retval  BTL_POKEPARAM*		
 */
//=============================================================================================
BTL_POKEPARAM* BTL_MAIN_GetFrontPokeData( BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
	u8 clientID, posIdx;

	btlPos_to_cliendID_and_posIdx( wk, pos, &clientID, &posIdx );

	GF_ASSERT_MSG(wk->client[clientID], "btlPos=%d, clientID=%d", pos, clientID);

	// @@@ このキャストは良くない…  この関数の呼び出し先はクライアントのダメージ計算ルーチンなので、
	// それらの処理はmainか、あるいは別の下請けモジュールに任せるようにした方がいいかも。
	return (BTL_POKEPARAM*)BTL_CLIENT_GetFrontPokeData( wk->client[clientID], posIdx );
}

//=============================================================================================
/**
 * 戦闘に出ているポケモンデータの取得 ( const )
 *
 * @param   wk		メインモジュールのハンドラ
 * @param   pos		立ち位置
 *
 * @retval  const BTL_POKEPARAM*		
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_MAIN_GetFrontPokeDataConst( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
	u8 clientID, posIdx;

	btlPos_to_cliendID_and_posIdx( wk, pos, &clientID, &posIdx );

	GF_ASSERT_MSG(wk->client[clientID], "btlPos=%d, clientID=%d", pos, clientID);

	TAYA_Printf("[GetFrontPokeDataConst] BtlPos=%d -> clientID:%d, posIdx:%d\n", pos, clientID, posIdx);

	return BTL_CLIENT_GetFrontPokeData( wk->client[clientID], posIdx );
}


//=============================================================================================
/**
 * 戦闘位置->クライアントID変換
 *
 * @param   wk		
 * @param   pos		
 *
 */
//=============================================================================================
u8 BTL_MAIN_BtlPosToClientID( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
	return btlPos_to_clientID( wk, pos );
}

//=============================================================================================
/**
 * 戦闘位置 -> 描画位置変換
 *
 * @param   wk		
 * @param   pos		
 *
 * @retval  u8		
 */
//=============================================================================================
u8 BTL_MAIN_BtlPosToViewPos( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
	u8 isPlayerSide;

	isPlayerSide = ( (wk->myOrgPos&1) == (pos&1) );

	if( wk->setupParam->rule == BTL_RULE_SINGLE )
	{
		return (isPlayerSide)? POKE_MCSS_POS_AA : POKE_MCSS_POS_BB;
	}
	else
	{
		static const u8 vpos[2][BTL_POSIDX_MAX] = {
			{ POKE_MCSS_POS_B, POKE_MCSS_POS_D, POKE_MCSS_POS_F },
			{ POKE_MCSS_POS_A, POKE_MCSS_POS_C, POKE_MCSS_POS_E },
		};
		u8 clientID, posIdx;

		btlPos_to_cliendID_and_posIdx( wk, pos, &clientID, &posIdx );
		return vpos[ isPlayerSide ][ posIdx ];
	}
}


static void BTL_PARTY_Initialize( BTL_PARTY* party )
{
	int i;
	party->memberCount = 0;
	for(i=0; i<TEMOTI_POKEMAX; i++)
	{
		party->member[i] = NULL;
	}
}

static void BTL_PARTY_Cleanup( BTL_PARTY* party )
{
	int i;
	for(i=0; i<party->memberCount; i++)
	{
		BTL_POKEPARAM_Delete( party->member[i] );
	}
	BTL_PARTY_Initialize( party );
}

static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member )
{
	GF_ASSERT(party->memberCount < TEMOTI_POKEMAX);
	party->member[party->memberCount++] = member;
}

u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party )
{
	return party->memberCount;
}

u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party )
{
	int i, cnt;
	for(i=0, cnt=0; i<party->memberCount; i++)
	{
		if( BTL_POKEPARAM_GetValue(party->member[i], BPP_HP) )
		{
			cnt++;
		}
	}
	return cnt;
}


BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx )
{
	return party->member[ idx ];
}
const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx )
{
	return party->member[ idx ];
}

const BTL_POKEPARAM* BTL_MAIN_GetPokeParam( const BTL_MAIN_MODULE* wk, u8 pokeID )
{
	GF_ASSERT(pokeID<BTL_COMMITMENT_POKE_MAX);
	GF_ASSERT(wk->pokeParam[pokeID]);

	return wk->pokeParam[ pokeID ];
}



//=============================================================================================
/**
 * クライアント計算結果完了後のデータを、サーバ計算用データに上書きする
 *
 * @param   wk		
 *
 */
//=============================================================================================
void BTL_MAIN_SyncServerCalcData( BTL_MAIN_MODULE* wk )
{
	GFL_STD_MemCopy32( wk->pokeParam, wk->pokeParamForServerCalc, sizeof(wk->pokeParamForServerCalc) );
}

