//=============================================================================================
/**
 * @file	btl_server.c
 * @brief	ポケモンWB バトルシステム	サーバモジュール
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#include <gflib.h>
#include "waza_tool/wazadata.h"

#include "btl_common.h"
#include "btl_client.h"
#include "btl_adapter.h"
#include "btl_action.h"
#include "btl_pokeparam.h"
#include "btl_calc.h"
#include "btl_event.h"
#include "btl_util.h"
#include "btl_string.h"
#include "handler/hand_tokusei.h"

#include "btl_server_cmd.h"
#include "btl_server_local.h"
#include "btl_server.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	SERVER_CMD_SIZE_MAX = 1024,
	CLIENT_DISABLE_ID = 0xff,
};

enum {
	QUITSTEP_NONE = 0,		///
	QUITSTEP_REQ,
	QUITSTEP_CORE,
};


//--------------------------------------------------------
/**
 *		サーバフロー処理フラグ
 */
//--------------------------------------------------------
typedef enum {

	FLOWFLG_SET_WAZAEFFECT,

	FLOWFLG_MAX,
	FLOWFLG_BYTE_MAX = ((FLOWFLG_MAX/4)+((FLOWFLG_MAX&3)!=0)*4),

}FlowFlag;


//--------------------------------------------------------------
/**
 *	サーバメインループ関数型  - バトル終了時のみ TRUE を返す -
 */
//--------------------------------------------------------------
typedef BOOL (*ServerMainProc)(BTL_SERVER*, int*);



typedef struct {

	BTL_ADAPTER*		adapter;
	BTL_PARTY*			party;
	BTL_POKEPARAM*	member[ TEMOTI_POKEMAX ];
	BTL_POKEPARAM*	frontMember[ BTL_POSIDX_MAX ];
	u8				memberCount;
	u8				numCoverPos;
	u8				isLocalClient;
	u8				myID;

}CLIENT_WORK;

/**
 *	アクション優先順記録構造体
 */
typedef struct {
	u8  clientID;		///< クライアントID
	u8  pokeIdx;		///< そのクライアントの、何体目？ 0～
}ACTION_ORDER_WORK;


typedef struct {

	WazaID	waza;						///< 出したワザID

	u8		attackPokeID;				///< 攻撃側ポケID
	u8		defencePokeID;			///< 受け側ポケID（単体）
	BtlExPos	defencePokePos;	///< 受け側ポケ位置ID（範囲選択）

	u8		wazaExecuteFlag;	///< ワザ実行できたかフラグ
	u8		wazaHitRatio;			///< ワザ命中確率
//	u8		attackHitRank;
//	u8		defenceHitRank;
	u8		wazaPower;				///< ワザ威力
	u8		wazaDamageType;		///< ダメージタイプ（物理・特殊）
	u8		criticalRank;			///< クリティカル率
	u8		criticalFlag;			///< クリティカルフラグ
	u8		wazaPokeType;			///< ワザのポケタイプ
	u8		typeAff;					///< ワザと受け側ポケの相性
	PokeTypePair	attackerPokeType;	///< 攻撃側ポケタイプ
	PokeTypePair	defenderPokeType;	///< 受け側ポケタイプ
	u16		attackerPower;		///< 攻撃側の能力値（こうげきorとくこう）
	u16		defenderGuard;		///< 防御側の能力値（ぼうぎょorとくぼう）
	u16		damageDenom;			///< ダメージ計算用の母数
	u32		rawDamage;				///< タイプ計算前の素ダメージ
	u32		realDamage;				///< タイプ計算後の実ダメージ
	u32		refrectDamage;		///< 攻撃側の反動ダメージ
	fx32	typeMatchRatio;		///< 攻撃側ポケとワザのタイプ一致によるダメージ倍率(1.0 or 1.5）

	u8		wazaFailReason;		///< ワザ失敗理由
	u8		confFlag;					///< こんらん自滅フラグ
	u8		confDamage;				///< こんらん自滅ダメージ

	u8		hitCountMax;			///< 最大ヒット数
	u8		hitCountReal;			///< 実際ヒット数

	u8		decPP;						///< 減少PP値

	PokeSick	sick;					///< 状態異常
	int				sickDamage;		///< 状態異常ダメージ値

}FIGHT_EVENT_PARAM;

typedef struct {

	BTL_POKEPARAM*  inPokeParam;
	BTL_POKEPARAM*  outPokeParam;

}CHANGE_EVENT_PARAM;

//--------------------------------------------------------------
/**
 *	戦闘に出ているポケモンデータに順番にアクセスする処理のワーク
 */
//--------------------------------------------------------------
typedef struct {
	u8 clientIdx;
	u8 pokeIdx;
	u8 endFlag;
}FRONT_POKE_SEEK_WORK;


//--------------------------------------------------------------
/**
 *	
 */
//--------------------------------------------------------------
struct _BTL_SERVER {
	BTL_MAIN_MODULE*	mainModule;
	CLIENT_WORK				client[ BTL_CLIENT_MAX ];
//	u8					actionOrder[ BTL_POS_MAX ];
	ACTION_ORDER_WORK	actOrder[ BTL_POS_MAX ];
	u8					numActPokemon;
	u8					numClient;
	u8					pokeDeadFlag;
	u8					quitStep;
	u8					wazaEffectPuttedFlag;

	ServerMainProc		mainProc;
	int					seq;

	FIGHT_EVENT_PARAM		fightEventParams;
	CHANGE_EVENT_PARAM	changeEventParams;

	BTL_SERVER_CMD_QUE	queBody;
	BTL_SERVER_CMD_QUE*	que;

	int					eventArgs[ BTL_EVARG_MAX ];

	HEAPID				heapID;
	u8						flowFlags[ FLOWFLG_BYTE_MAX ];

};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline void setup_client_members( CLIENT_WORK* client, BTL_PARTY* party, u8 numCoverPos );
static void setMainProc( BTL_SERVER* sv, ServerMainProc mainProc );
static BOOL ServerMain_WaitReady( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectAction( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectPokemon( BTL_SERVER* server, int* seq );
static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd );
static void SetAdapterCmdEx( BTL_SERVER* server, BtlAdapterCmd cmd, const void* sendData, u32 dataSize );
static BOOL WaitAdapterCmd( BTL_SERVER* server );
static void ResetAdapterCmd( BTL_SERVER* server );
static inline void FlowFlg_Set( BTL_SERVER* server, FlowFlag flg );
static inline void FlowFlg_Clear( BTL_SERVER* server, FlowFlag flg );
static inline BOOL FlowFlg_Get( BTL_SERVER* server, FlowFlag flg );
static inline void FlowFlg_ClearAll( BTL_SERVER* server );
static u8 sortClientAction( BTL_SERVER* server, ACTION_ORDER_WORK* order );
static u8 countAlivePokemon( const BTL_SERVER* server );
static BOOL ServerFlow_Start( BTL_SERVER* server );
static BOOL ServerFlow_AfterPokeSelect( BTL_SERVER* server );
static void scput_MemberIn( BTL_SERVER* server, u8 clientID, u8 posIdx, u8 nextPokeIdx );
static void scput_MemberOut( BTL_SERVER* server, u8 clientID, const BTL_ACTION_PARAM* action );
static void scput_Fight( BTL_SERVER* server, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action );
static void cof_put_wazafail_msg_cmd( BTL_SERVER* server, const BTL_POKEPARAM* attacker, u8 wazaFailReason );
static void scput_Fight_Damage(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static BTL_POKEPARAM* svflowsub_get_opponent_pokeparam( BTL_SERVER* server, BtlPokePos pos, u8 pokeSideIdx );
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SERVER* server, BtlPokePos pos );
static void svflowsub_check_and_make_dead_command( BTL_SERVER* server, BTL_POKEPARAM* poke );
static void scput_Fight_Damage_Single(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static void scput_Fight_Damage_Enemy2(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static void scput_Fight_Damage_OtherAll(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static void svflowsub_damage_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza );
static void svflowsub_damage_enemy_all( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender1, BTL_POKEPARAM* defender2, WazaID waza );
static void svflowsub_damage_act_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff );
static void svflowsub_damage_act_enemy_all( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep,
		BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, BtlTypeAff aff1, BtlTypeAff aff2, WazaID waza );
static void svflowsub_damage_act_enemy_all_atonce( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep,
		BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, BtlTypeAff aff, WazaID waza );
static BOOL svflowsub_hitcheck_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza );
static u16 svflowsub_damage_calc_core( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff typeAff );
static void svflowsub_set_waza_effect( BTL_SERVER* server, u8 atPokeID, u8 defPokeID, WazaID waza );
static void scput_TurnCheck( BTL_SERVER* server );
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SERVER* server );
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SERVER* server, BTL_POKEPARAM** bpp );
static void scput_turncheck_sick( BTL_SERVER* server );
static inline int roundValue( int val, int min, int max );
static void initFightEventParams( FIGHT_EVENT_PARAM* fep );
static BOOL scEvent_CheckConf( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const CLIENT_WORK* atClient );
static u16 scEvent_CalcConfDamage( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckWazaExecute( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, const CLIENT_WORK* atClient );
static BOOL scEvent_checkHit( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BtlTypeAff scEvent_checkAffinity( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static void scEvent_decrementPP( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, u8 wazaIdx );
static u8 scEvent_getHitRatio( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza );
static u8 scEvent_GetHitCount( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, u8 hitCountMax );
static u8 scEvent_getCriticalRank( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, WazaID waza );
static u16 scEvent_getWazaPower( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza );
static u16 scEvent_getAttackPower( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, WazaID waza );
static u16 scEvent_getDefenderGuard( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* defender, WazaID waza );
static u16 scEvent_getDamageDenom( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep );
static PokeType scEvent_getWazaPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza );
static PokeTypePair scEvent_getAttackerPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker );
static PokeTypePair scEvent_getDefenderPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* defender );
static void scEvent_MemberOut( BTL_SERVER* server, CHANGE_EVENT_PARAM* cep, u8 clientID, u8 posIdx );
static void scEvent_MemberIn( BTL_SERVER* server, CHANGE_EVENT_PARAM* cep, u8 clientID, u8 posIdx, u8 nextPokeIdx );
static void scEvent_PokeComp( BTL_SERVER* server );
static void scEvent_RankDown( BTL_SERVER* server, u8 pokeID, BtlPokePos pokePos, BppValueID statusType, u8 volume );



//--------------------------------------------------------------------------------------
/**
 * サーバ生成
 *
 * @param   mainModule		バトルメインモジュールハンドラ
 * @param   heapID			生成用ヒープID
 *
 * @retval  BTL_SERVER*		生成されたサーバハンドラ
 */
//--------------------------------------------------------------------------------------
BTL_SERVER* BTL_SERVER_Create( BTL_MAIN_MODULE* mainModule, HEAPID heapID )
{
	BTL_SERVER* sv = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SERVER) );

	sv->mainModule = mainModule;
	sv->numClient = 0;
	sv->heapID = heapID;
	sv->que = &sv->queBody;
	sv->quitStep = QUITSTEP_NONE;

	{
		int i;
		for(i=0; i<BTL_CLIENT_MAX; ++i)
		{
			sv->client[i].myID = CLIENT_DISABLE_ID;
		}
	}

	setMainProc( sv, ServerMain_WaitReady );

	return sv;
}
//--------------------------------------------------------------------------------------
/**
 * サーバ削除
 *
 * @param   wk		
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_Delete( BTL_SERVER* wk )
{
	int i;
	for(i=0; i<BTL_CLIENT_MAX; ++i)
	{
		if( (wk->client[i].myID != CLIENT_DISABLE_ID)
		&&	(wk->client[i].isLocalClient == FALSE)
		){
			// 同一マシン上にあるクライアントでなければ、
			// サーバがアダプタを用意したハズなので自分で削除
			BTL_ADAPTER_Delete( wk->client[i].adapter );
		}
	}
	GFL_HEAP_FreeMemory( wk );
}
//--------------------------------------------------------------------------------------
/**
 * サーバと同一マシン上にあるクライアントとのアダプタを接続する
 *
 * @param   server			サーバハンドラ
 * @param   adapter			アダプタ（既にクライアントに関連付けられている）
 * @param   party				クライアントのパーティデータ
 * @param   clientID		クライアントID
 * @param   numCoverPos	クライアントが受け持つ戦闘中ポケモン数
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_AttachLocalClient( BTL_SERVER* server, BTL_ADAPTER* adapter, BTL_PARTY* party, u8 clientID, u8 numCoverPos )
{
	GF_ASSERT(clientID < BTL_CLIENT_MAX);
	GF_ASSERT(server->client[clientID].adapter==NULL);

	{
		CLIENT_WORK* client;

		client = &server->client[ clientID ];

		client->adapter = adapter;
		client->myID = clientID;
		client->isLocalClient = TRUE;

		setup_client_members( client, party, numCoverPos );
	}

	server->numClient++;
}
//--------------------------------------------------------------------------------------
/**
 * サーバと異なるマシン上にあるクライアントとのアダプタを生成する
 *
 * @param   server			サーバハンドラ
 * @param   commMode		通信モード
 * @param   netHandle		ネットワークハンドラ
 * @param   clientID		クライアントID
 * @param   numCoverPos	クライアントが受け持つ戦闘中ポケモン数
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_ReceptionNetClient( BTL_SERVER* server, BtlCommMode commMode, GFL_NETHANDLE* netHandle, BTL_PARTY* party, u8 clientID, u8 numCoverPos )
{
	GF_ASSERT(clientID < BTL_CLIENT_MAX);
	GF_ASSERT(server->client[clientID].adapter==NULL);

	{
		CLIENT_WORK* client;
		int i;

		client = &server->client[ clientID ];

		client->adapter = BTL_ADAPTER_Create( netHandle, server->heapID, clientID );
		client->myID = clientID;
		client->isLocalClient = FALSE;

		setup_client_members( client, party, numCoverPos );
	}

	server->numClient++;
}
//--------------------------------------------------------------------------------------
/**
 * 指定ポケIDを持つポケモンが戦闘に出ているかチェックし、出ていたらその戦闘位置を返す
 *
 * @param   server		
 * @param   pokeID		
 *
 * @retval  BtlPokePos		出ている場合は戦闘位置ID／出ていない場合はBTL_POS_MAX
 */
//--------------------------------------------------------------------------------------
BtlPokePos BTL_SERVER_CheckExistFrontPokeID( BTL_SERVER* server, u8 pokeID )
{
	int i;

	for(i=0; i<BTL_CLIENT_MAX; i++)
	{
		if( server->client[i].numCoverPos )
		{
			int p;
			for(p=0; p<server->client[i].numCoverPos; p++)
			{
				if( (server->client[i].frontMember[p] != NULL)
				&&	(BTL_POKEPARAM_GetID(server->client[i].frontMember[p]) == pokeID)
				){
					return BTL_MAIN_GetClientPokePos( server->mainModule, i, p );
				}
			}
		}
	}
	return BTL_POS_MAX;
}


//--------------------------------------------------------------------------
/**
 * クライアントワークのメンバーデータ部分初期化
 *
 * @param   client				クライアントワーク
 * @param   party					パーティデータ
 * @param   numCoverPos		クライアントが受け持つ戦闘中ポケモン数
 */
//--------------------------------------------------------------------------
static inline void setup_client_members( CLIENT_WORK* client, BTL_PARTY* party, u8 numCoverPos )
{
	u16 i;

	client->party = party;
	client->memberCount = BTL_PARTY_GetMemberCount( party );
	client->numCoverPos = numCoverPos;

	for(i=0; i<client->memberCount; i++)
	{
		client->member[i] = BTL_PARTY_GetMemberData( client->party, i );
	}
	for( ; i<NELEMS(client->member); i++)
	{
		client->member[i] = NULL;
	}

	{
		u16 numFrontPoke = (numCoverPos < client->memberCount)? numCoverPos : client->memberCount;
		for(i=0; i<numFrontPoke; i++)
		{
			client->frontMember[i] = client->member[i];
		}
		for( ; i<NELEMS(client->frontMember); i++)
		{
			client->frontMember[i] = NULL;
		}
	}
}


//--------------------------------------------------------------------------------------
/**
 * サーバメインループ
 *
 * @param   sv			サーバハンドラ
 *
 * @retval  BOOL		終了時 TRUE を返す
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_Main( BTL_SERVER* sv )
{
	if( sv->quitStep != QUITSTEP_CORE )
	{
		if( sv->mainProc( sv, &sv->seq ) )
		{
			SetAdapterCmd( sv, BTL_ACMD_QUIT_BTL );
			sv->quitStep = QUITSTEP_CORE;
		}
	}
	else
	{
		WaitAdapterCmd( sv );
	}
}

static void setMainProc( BTL_SERVER* sv, ServerMainProc mainProc )
{
	sv->mainProc = mainProc;
	sv->seq = 0;
}


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


static BOOL ServerMain_WaitReady( BTL_SERVER* server, int* seq )
{
	switch( *seq ){
	case 0:
		BTL_EVENT_InitSystem();
		BTL_Printf("イニシャライズコマンド発行\n");
		SetAdapterCmd( server, BTL_ACMD_WAIT_INITIALIZE );
		(*seq)++;
	case 1:
		if( WaitAdapterCmd(server) )
		{
			u16 i, p;

			ResetAdapterCmd( server );

			for(i=0; i<server->numClient; i++)
			{
				for(p=0; p<server->client[i].numCoverPos; p++)
				{
					if( server->client[i].frontMember[p] != NULL )
					{
						BTL_HANDLER_TOKUSEI_Add( server->client[i].frontMember[p] );
					}
				}
			}
			setMainProc( server, ServerMain_SelectAction );
		}
		break;
	}

	return FALSE;
}

static BOOL ServerMain_SelectAction( BTL_SERVER* server, int* seq )
{
	switch( *seq ){
	case 0:
		SCQUE_Init( server->que );
		scEvent_PokeComp( server );
		if( server->que->writePtr )
		{
			BTL_Printf("再生コマンド発行\n");
			SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
			(*seq) = 1;
		}
		else
		{
			(*seq)=2;
		}
		break;

	case 1:
		if( WaitAdapterCmd(server) )
		{
			ResetAdapterCmd( server );
			(*seq)++;
		}
		break;

	case 2:
		BTL_Printf("アクション選択コマンド発行\n");
		SetAdapterCmd( server, BTL_ACMD_SELECT_ACTION );
		(*seq)++;
		break;

	case 3:
		if( WaitAdapterCmd(server) )
		{
			ResetAdapterCmd( server );
			server->numActPokemon = sortClientAction( server, server->actOrder );
			BTL_Printf("全クライアントのアクションソート完了。処理アクション数=%d\n", server->numActPokemon);
			SCQUE_Init( server->que );
			server->pokeDeadFlag = ServerFlow_Start( server );
			if( server->pokeDeadFlag )
			{
				BTL_Printf(" **** ポケモンしにました\n");
			}
			SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
			(*seq)++;
		}
		break;

	case 4:
		if( WaitAdapterCmd(server) )
		{
			BTL_Printf("コマンド再生おわりました\n");
			BTL_MAIN_SyncServerCalcData( server->mainModule );
			ResetAdapterCmd( server );
			if( server->quitStep )
			{
				BTL_Printf("サーバー終了\n");
				return TRUE;
			}
			else if( server->pokeDeadFlag )
			{
				setMainProc( server, ServerMain_SelectPokemon );
			}
			else
			{
				(*seq) = 2;
			}
		}
		break;
	}
	return FALSE;
}

static BOOL ServerMain_SelectPokemon( BTL_SERVER* server, int* seq )
{
	switch( *seq ){
	case 0:
		{
			u8 loseClientCount;
			int i;

			loseClientCount = 0;

			for(i=0; i<server->numClient; i++)
			{
				if( BTL_PARTY_GetAliveMemberCount(server->client[i].party) == 0 )
				{
					loseClientCount++;
				}
			}

			// 続行
			if( loseClientCount == 0 )
			{
				BTL_Printf("負けたクライアントいないので次のポケモン選択へ\n");
				SetAdapterCmd( server, BTL_ACMD_SELECT_POKEMON );
				(*seq)++;
			}
			// 決着
			else
			{
				// 勝敗決定
				if( loseClientCount == 1 )
				{
					server->quitStep = QUITSTEP_REQ;
				}
				// 引き分け
				else
				{
					server->quitStep = QUITSTEP_REQ;
				}
				return TRUE;
			}
		}
		break;

	case 1:
		if( WaitAdapterCmd(server) )
		{
			ResetAdapterCmd( server );
			SCQUE_Init( server->que );
			server->pokeDeadFlag = ServerFlow_AfterPokeSelect( server );
			BTL_DUMP_Printf( "[SV]コマンド発信", server->que->buffer, server->que->writePtr );
			SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );

			(*seq)++;
		}
		break;

	case 2:
		if( WaitAdapterCmd(server) )
		{
			ResetAdapterCmd( server );
			BTL_MAIN_SyncServerCalcData( server->mainModule );

			if( server->quitStep )
			{
				return TRUE;
			}
			else if( server->pokeDeadFlag )
			{
				(*seq) = 0;
			}
			else
			{
				setMainProc( server, ServerMain_SelectAction );
			}
		}
		break;
	}

	return FALSE;
}



//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd )
{
	SetAdapterCmdEx( server, cmd, NULL, 0 );
}

static void SetAdapterCmdEx( BTL_SERVER* server, BtlAdapterCmd cmd, const void* sendData, u32 dataSize )
{
	int i;

	BTL_ADAPTERSYS_BeginSetCmd();

	for(i=0; i<server->numClient; i++)
	{
		BTL_ADAPTER_SetCmd( server->client[i].adapter, cmd, sendData, dataSize );
	}

	BTL_ADAPTERSYS_EndSetCmd();
}

static BOOL WaitAdapterCmd( BTL_SERVER* server )
{
	int i;
	BOOL ret = TRUE;
	for(i=0; i<server->numClient; i++)
	{
		if( !BTL_ADAPTER_WaitCmd( server->client[i].adapter ) )
		{
			ret = FALSE;
		}
	}
	return ret;
}

static void ResetAdapterCmd( BTL_SERVER* server )
{
	int i;
	for(i=0; i<server->numClient; i++)
	{
		BTL_ADAPTER_ResetCmd( server->client[i].adapter );
	}
}




//----------------------------------------------------------------------------------------------
// サーバーフローフラグ処理
//----------------------------------------------------------------------------------------------

static inline void FlowFlg_Set( BTL_SERVER* server, FlowFlag flg )
{
	server->flowFlags[ flg ] = 1;
}
static inline void FlowFlg_Clear( BTL_SERVER* server, FlowFlag flg )
{
	server->flowFlags[ flg ] = 0;
}
static inline BOOL FlowFlg_Get( BTL_SERVER* server, FlowFlag flg )
{
	return server->flowFlags[ flg ];
}
static inline void FlowFlg_ClearAll( BTL_SERVER* server )
{
	int i;
	for(i=0; i<NELEMS(server->flowFlags); ++i)
	{
		server->flowFlags[ i ] = 0;
	}
}


//--------------------------------------------------------------------------
/**
 * クライアントのアクションをチェックし、処理順序を確定
 *
 * @param   server		
 * @param   order		処理する順番にクライアントIDを並べ直して格納するための配列
 *
 */
//--------------------------------------------------------------------------
static u8 sortClientAction( BTL_SERVER* server, ACTION_ORDER_WORK* order )
{
	/*
		行動優先順  ... 2BIT
		ワザ優先順  ... 6BIT
		素早さ      ... 16BIT
	*/
	#define MakePriValue( actPri, wazaPri, agility )	\
						( ((actPri)<<22) | ((wazaPri)<<16) | (agility) )

	CLIENT_WORK* client;
	const BTL_ACTION_PARAM* actParam;
	u32 pri[ BTL_POS_MAX ];
	u16 agility;
	u8  action, actionPri, wazaPri;
	u8  i, j, p, numPoke;

// 各クライアントのプライオリティ値を生成
	for(i=0, p=0; i<server->numClient; i++)
	{
		client = &server->client[i];

		actParam = BTL_ADAPTER_GetReturnData( client->adapter );
		numPoke = BTL_ADAPTER_GetReturnDataCount( client->adapter );

		TAYA_Printf("[SV] アクションソート, CLIENT<%d> のカバー数は%d, 帰りデータ数は%d\n",
					i, client->numCoverPos, numPoke);

//		for(j=0; j<client->numCoverPos; j++)
		for(j=0; j<numPoke; j++)
		{
			// 行動による優先順（優先度高いほど数値大）
			switch( actParam->gen.cmd ){
			case BTL_ACTION_ESCAPE:	actionPri = 3; break;
			case BTL_ACTION_ITEM:	actionPri = 2; break;
			case BTL_ACTION_CHANGE:	actionPri = 1; break;
			case BTL_ACTION_FIGHT:	actionPri = 0; break;
			default:
				GF_ASSERT(0);
				actionPri = 0;
				break;
			}

			// ワザによる優先順
			if( actParam->gen.cmd == BTL_ACTION_FIGHT )
			{
				WazaID  w = BTL_POKEPARAM_GetWazaNumber( client->frontMember[j], actParam->fight.wazaIdx );
				wazaPri = WAZADATA_GetPriority( w ) - WAZAPRI_MIN;
			}
			else
			{
				wazaPri = 0;
			}

			// すばやさ
			agility = BTL_POKEPARAM_GetValue( client->frontMember[j], BPP_AGILITY );

			BTL_Printf("Client{%d-%d}'s actionPri=%d, wazaPri=%d, agility=%d\n",
					i, j, actionPri, wazaPri, agility );

			// プライオリティ値とクライアントIDを対にして配列に保存
			pri[p] = MakePriValue( actionPri, wazaPri, agility );
			order[p].clientID = i;
			order[p].pokeIdx = j;
			BTL_Printf("Client(%d) PriValue=0x%8x\n", i, pri[i]);

			actParam++;
			p++;
		}
	}
 
// プライオリティ値ソートに伴ってクライアントID配列もソート
	for(i=0; i<p; i++)
	{
		for(j=i+1; j<p; j++)
		{
			if( pri[i] < pri[j] )
			{
				u32 t = pri[i];
				pri[i] = pri[j];
				pri[j] = t;

				{
					ACTION_ORDER_WORK w = order[i];
					order[i] = order[j];
					order[j] = w;
				}
			}
		}
	}

// 全く同じプライオリティ値があったらランダムシャッフル
// @@@ 未実装

	return p;

}

// 今、戦闘に出ていて生きているポケモンの数をカウント
static u8 countAlivePokemon( const BTL_SERVER* server )
{
	const CLIENT_WORK* cl;
	u16 i, j;
	u8 cnt = 0;

	for(i=0; i<server->numClient; i++)
	{
		cl = &server->client[i];
		for(j=0; j<cl->numCoverPos; j++)
		{
			if( cl->frontMember[j] )
			{
				if( !BTL_POKEPARAM_IsDead(cl->frontMember[j]) )
				{
					cnt++;
				}
			}
		}
	}
	return cnt;
}
//--------------------------------------------------------------------------
/**
 * １ターン分サーバコマンド生成（行動選択後）
 *
 * @param   server		
 *
 * @retval  BOOL		ターン中、瀕死になったポケモンがいる場合はTRUE／それ以外FALSE
 */
//--------------------------------------------------------------------------
static BOOL ServerFlow_Start( BTL_SERVER* server )
{
	u16 clientID, pokeIdx;
	u8 numPokeBegin, numPokeAlive;
	u8 i;

	numPokeBegin = countAlivePokemon( server );
	FlowFlg_ClearAll( server );

	for(i=0; i<server->numActPokemon; i++)
	{
		clientID = server->actOrder[i].clientID;
		pokeIdx  = server->actOrder[i].pokeIdx;

		if( !BTL_POKEPARAM_IsDead(server->client[clientID].frontMember[pokeIdx]) )
		{
			const BTL_ACTION_PARAM* action = BTL_ADAPTER_GetReturnData( server->client[clientID].adapter );
			action += pokeIdx;

			BTL_Printf("Client(%d) の ", clientID);
			switch( action->gen.cmd ){
			case BTL_ACTION_FIGHT:
				BTL_Printf("【たたかう】を処理。%d番目のワザを、%d番の相手に。\n", action->fight.wazaIdx, action->fight.targetIdx);
				scput_Fight( server, clientID, pokeIdx, action );
				break;
			case BTL_ACTION_ITEM:
				BTL_Printf("【どうぐ】を処理。アイテム%dを、%d番の相手に。\n", action->item.number, action->item.targetIdx);
				break;
			case BTL_ACTION_CHANGE:
				BTL_Printf("【ポケモン】を処理。位置%d <- ポケ%d \n", action->change.posIdx, action->change.memberIdx);
				scput_MemberOut( server, clientID, action );
				scput_MemberIn( server, clientID, action->change.posIdx, action->change.memberIdx );
				break;
			case BTL_ACTION_ESCAPE:
				BTL_Printf("【にげる】を処理。\n");
				// @@@ 今は即座に逃げられるようにしている
				SCQUE_PUT_MSG_STD( server->que, BTL_STRID_STD_EscapeSuccess, clientID );
				server->quitStep = QUITSTEP_REQ;
				return FALSE;
			}
		}
	}

	// ターンチェック処理
	scput_TurnCheck( server );

	// 死んだポケモンがいる場合の処理
	numPokeAlive = countAlivePokemon( server );
	BTL_Printf( "ポケモン数 %d -> %d ...\n", numPokeBegin, numPokeAlive );
	if( numPokeBegin > numPokeAlive )
	{
		return TRUE;
	}

	return FALSE;
}
//--------------------------------------------------------------------------
/**
 * １ターン分サーバコマンド生成（ポケモン選択後）
 *
 * @param   server		
 *
 * @retval  BOOL		ターン中、瀕死になったポケモンがいる場合はTRUE／それ以外FALSE
 */
//--------------------------------------------------------------------------
static BOOL ServerFlow_AfterPokeSelect( BTL_SERVER* server )
{
	const BTL_ACTION_PARAM* action;
	CLIENT_WORK* client;
	u16 clientID, posIdx;
	int i, j, actionCnt;

	BTL_Printf("ひんしポケモン入れ替え選択後のサーバーコマンド生成\n");

	for(i=0; i<server->numClient; i++)
	{
		client = &server->client[i];
		action = BTL_ADAPTER_GetReturnData( client->adapter );
		actionCnt = BTL_ADAPTER_GetReturnDataCount( client->adapter );

		for(j=0; j<actionCnt; j++, action++)
		{
			if( action->gen.cmd != BTL_ACTION_CHANGE ){ continue; }
			if( action->change.depleteFlag ){ continue; }

			BTL_Printf("クライアント(%d)のポケモン(位置%d) を、%d番目のポケといれかえる\n",
						i, action->change.posIdx, action->change.memberIdx );

			scput_MemberIn( server, i, action->change.posIdx, action->change.memberIdx );
		}
	}

	// @@@ 今は確実にFALSEだが、入れ替えた時にまきびしとかで死ぬこともある
	return FALSE;
}

static void scput_MemberIn( BTL_SERVER* server, u8 clientID, u8 posIdx, u8 nextPokeIdx )
{
	server->client[clientID].frontMember[posIdx] = server->client[clientID].member[ nextPokeIdx ];
	scEvent_MemberIn( server, &server->changeEventParams, clientID, posIdx, nextPokeIdx );
}
static void scput_MemberOut( BTL_SERVER* server, u8 clientID, const BTL_ACTION_PARAM* action )
{
	scEvent_MemberOut( server, &server->changeEventParams, clientID, action->change.posIdx );
}
//==============================================================================
// サーバーフロー：「たたかう」ルート
//==============================================================================
static void scput_Fight( BTL_SERVER* server, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action )
{
	CLIENT_WORK *atClient;
	BTL_POKEPARAM  *attacker;
	FIGHT_EVENT_PARAM* fep;
	WazaID  waza;

	atClient = &server->client[ attackClientID ];
	attacker = atClient->frontMember[posIdx];

	waza = BTL_POKEPARAM_GetWazaNumber( attacker, action->fight.wazaIdx );

  fep = &server->fightEventParams;
	initFightEventParams( fep );
	fep->waza = waza;
	fep->attackPokeID = BTL_POKEPARAM_GetID( attacker );

// こんらん自爆判定
	if( scEvent_CheckConf( server, fep, attacker, atClient ) )
	{
		fep->confDamage = scEvent_CalcConfDamage( server, fep, attacker );
		{
			u16 hp = BTL_POKEPARAM_GetValue( attacker, BPP_HP );
			u8 deadFlag = FALSE;
			if( fep->confDamage >= hp )
			{
				fep->confDamage = hp;
				deadFlag = TRUE;
			}
			BTL_POKEPARAM_HpMinus( attacker, fep->confDamage );
			SCQUE_PUT_OP_HpMinus( server->que, atClient->myID, fep->confDamage );
//			SCQUE_PUT_MSG_ConfOn( server->que, atClient->myID );	@@@ メッセージまだです
			if( deadFlag )
			{
				u8 pokeID = BTL_POKEPARAM_GetID( attacker );
				SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_Dead, pokeID );
				SCQUE_PUT_ACT_Dead( server->que, pokeID );
				return;
			}
		}
	}

// ワザ出し成功判定
	if( !scEvent_CheckWazaExecute(server, fep, attacker, atClient ) )
	{
		cof_put_wazafail_msg_cmd( server, attacker, fep->wazaFailReason );
		return;
	}
	else
	{
		WazaCategory  category;
		BtlPokePos    atPos;

		category = WAZADATA_GetCategory( waza );
		atPos = BTL_MAIN_GetClientPokePos( server->mainModule, atClient->myID, posIdx );

		BTL_Printf("出すポケ位置=%d, 出すワザ=%d, カテゴリ=%d\n", atPos, waza, category);

		SCQUE_PUT_MSG_WAZA( server->que, atPos, action->fight.wazaIdx );

		switch( category ){
		case WAZADATA_CATEGORY_SIMPLE_DAMAGE:
		case WAZADATA_CATEGORY_DAMAGE_SICK:
		case WAZADATA_CATEGORY_DAMAGE_EFFECT:
		case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
			scput_Fight_Damage( server, fep, waza, atClient, attacker, atPos, action );
			break;

//	case WAZADATA_CATEGORY_ICHIGEKI:
//	case WAZADATA_CATEGORY_BIND:
//	case WAZADATA_CATEGORY_GUARD:
//	case WAZADATA_CATEGORY_FIELD_EFFECT:
//	case WAZADATA_CATEGORY_SIDE_EFFECT:
//	case WAZADATA_CATEGORY_OTHERS:
		default:
//		SCQUE_PUT_MSG_STD( server->que, BTL_STRID_STD_WazaAvoid );
			SCQUE_PUT_MSG_STD( server->que, BTL_STRID_STD_WazaFail );
			break;
		}
		scEvent_decrementPP( server, fep, attacker, action->fight.wazaIdx );
	}
}
// ワザだし失敗メッセージ表示コマンドをセット
static void cof_put_wazafail_msg_cmd( BTL_SERVER* server, const BTL_POKEPARAM* attacker, u8 wazaFailReason )
{
	u8 pokeID = BTL_POKEPARAM_GetID( attacker );

	switch( wazaFailReason ){
	case SV_WAZAFAIL_NEMURI:
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_NemuriAct, pokeID );
		break;

	case SV_WAZAFAIL_MAHI:
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_MahiAct, pokeID );
		break;

	case SV_WAZAFAIL_KOORI:
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_KoriAct, pokeID );
		break;

	default:
		SCQUE_PUT_MSG_STD( server->que, BTL_STRID_STD_WazaFail, pokeID );
		break;
	}
}
//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系
//----------------------------------------------------------------------
static void scput_Fight_Damage(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
	FlowFlg_Clear( server, FLOWFLG_SET_WAZAEFFECT );

	// シングル
	if( BTL_MAIN_GetRule(server->mainModule) == BTL_RULE_SINGLE )
	{
		scput_Fight_Damage_Single( server, fep, waza, atClient, attacker, atPos, action );
	}
	// ダブル
	else
	{
		WazaTarget  targetType = WAZADATA_GetTarget( waza );

		BTL_Printf("ダブルです。ワザナンバー=%d, ワザターゲットタイプ=%d\n", waza, targetType);

		switch( targetType ) {
		case WAZA_TARGET_SINGLE:				///< 自分以外の１体（選択）
		case WAZA_TARGET_SINGLE_ENEMY:	///< 敵１体（選択）
		case WAZA_TARGET_RANDOM:				///< 敵ランダム
			scput_Fight_Damage_Single( server, fep, waza, atClient, attacker, atPos, action );
			break;

		case WAZA_TARGET_ENEMY2:				///< 敵側２体
			scput_Fight_Damage_Enemy2( server, fep, waza, atClient, attacker, atPos, action );
			break;

		case WAZA_TARGET_OTHER_ALL:			///< 自分以外全部
			scput_Fight_Damage_OtherAll( server, fep, waza, atClient, attacker, atPos, action );
			break;

	//	case WAZA_TARGET_ONLY_USER:			///< 自分１体のみ
	//	case WAZA_TARGET_TEAM_USER:			///< 自分側陣地
	//	case WAZA_TARGET_TEAM_ENEMY:		///< 敵側陣地
	//	case WAZA_TARGET_FIELD:					///< 場に効く（天候系など）
		default:
//			GF_ASSERT_MSG(0, "illegal targetType!! waza=%d, targetType=%d", waza, targetType);
			return;
		}
	}
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定位置から見て対戦相手のポケモンデータを返す
//----------------------------------------------------------------------
static BTL_POKEPARAM* svflowsub_get_opponent_pokeparam( BTL_SERVER* server, BtlPokePos pos, u8 pokeSideIdx )
{
	BtlPokePos targetPos;
	u8 clientID, pokeIdx;

	targetPos = BTL_MAIN_GetOpponentPokePos( server->mainModule, pos, pokeSideIdx );
	BTL_MAIN_BtlPosToClientID_and_PokeIdx( server->mainModule, targetPos, &clientID, &pokeIdx );

	return server->client[ clientID ].frontMember[ pokeIdx ];
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定位置から見て隣のポケモンデータを返す
//----------------------------------------------------------------------
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SERVER* server, BtlPokePos pos )
{
	BtlPokePos nextPos;
	u8 clientID, pokeIdx;

	nextPos = BTL_MAIN_GetNextPokePos( server->mainModule, pos );
	BTL_MAIN_BtlPosToClientID_and_PokeIdx( server->mainModule, nextPos, &clientID, &pokeIdx );

	return server->client[ clientID ].frontMember[ pokeIdx ];
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定ポケモンが死んでたら死亡処理コマンド生成
//----------------------------------------------------------------------
static void svflowsub_check_and_make_dead_command( BTL_SERVER* server, BTL_POKEPARAM* poke )
{
	if( BTL_POKEPARAM_IsDead(poke) )
	{
		u8 pokeID = BTL_POKEPARAM_GetID( poke );
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_Dead, pokeID );
		SCQUE_PUT_ACT_Dead( server->que, pokeID );
	}
}


//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系 > 単体選択
//----------------------------------------------------------------------
static void scput_Fight_Damage_Single(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
//	BtlPokePos  defPos;
//	u8 defClientID;
	BTL_POKEPARAM* defender;

//	defPos = BTL_MAIN_GetClientPokePos( server->mainModule, atPos, action->fight.targetIdx );
//	BTL_MAIN_GetFrontPokeData( server->mainModule, defPos );
	defender = svflowsub_get_opponent_pokeparam( server, atPos, action->fight.targetIdx );

	svflowsub_damage_singular( server, fep, attacker, defender, waza );
}
//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系 > 相手２体対象
//----------------------------------------------------------------------
static void scput_Fight_Damage_Enemy2(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
	BTL_POKEPARAM *defpoke1, *defpoke2;

	defpoke1 = svflowsub_get_opponent_pokeparam( server, atPos, 0 );
	defpoke2 = svflowsub_get_opponent_pokeparam( server, atPos, 1 );

	svflowsub_damage_enemy_all( server, fep, attacker, defpoke1, defpoke2, waza );
}

//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系 > ３体（自分以外）対象
//----------------------------------------------------------------------
static void scput_Fight_Damage_OtherAll(
	 BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza, CLIENT_WORK* atClient,
	 BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
	BTL_POKEPARAM *defF, *defE1, *defE2;

	defF  = svflowsub_get_next_pokeparam( server, atPos );
	defE1 = svflowsub_get_opponent_pokeparam( server, atPos, 0 );
	defE2 = svflowsub_get_opponent_pokeparam( server, atPos, 1 );

	svflowsub_damage_singular( server, fep, attacker, defF, waza );
	svflowsub_damage_enemy_all( server, fep, attacker, defE1, defE2, waza );
}

//------------------------------------------------------------------
// サーバーフロー下請け：単体ダメージ処理（上位分岐）
//------------------------------------------------------------------
static void svflowsub_damage_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza )
{
	BtlTypeAff aff;

	if( !svflowsub_hitcheck_singular(server, fep, attacker, defender, waza) )
	{
		return;
	}

	aff = scEvent_checkAffinity( server, fep, attacker, defender, waza );

	svflowsub_damage_act_singular( server, fep, attacker, defender, waza, aff );

	svflowsub_check_and_make_dead_command( server, defender );
	svflowsub_check_and_make_dead_command( server, attacker );
}
//------------------------------------------------------------------
// サーバーフロー下請け：相手全体ダメージ処理（上位分岐）
//------------------------------------------------------------------
static void svflowsub_damage_enemy_all( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender1, BTL_POKEPARAM* defender2, WazaID waza )
{
	u8 hit1, hit2;
	u8 alive1, alive2;

	hit1 = svflowsub_hitcheck_singular(server, fep, attacker, defender1, waza);
	hit2 = svflowsub_hitcheck_singular(server, fep, attacker, defender2, waza);

	alive1 = !BTL_POKEPARAM_IsDead( defender1 );
	alive2 = !BTL_POKEPARAM_IsDead( defender2 );

	// ２体ともヒット
	if( hit1 && hit2 )
	{
		BtlTypeAff aff1, aff2;
		u8 about1, about2;

		aff1 = scEvent_checkAffinity( server, fep, attacker, defender1, waza );
		aff2 = scEvent_checkAffinity( server, fep, attacker, defender2, waza );

		about1 = BTL_CALC_TypeAffAbout( aff1 );
		about2 = BTL_CALC_TypeAffAbout( aff2 );

		// 相性が別々なら、個別の処理
		if( about1 != about2 )
		{
			BTL_Printf("２体ともにヒットし、相性べつべつ\n");
			svflowsub_damage_act_enemy_all( server, fep, attacker, defender1, defender2, aff1, aff2, waza );
		}
		// 相性も同じなら、体力バーを同時に減らすようにコマンド生成する
		else
		{
			BTL_Printf("２体ともにヒットし、相性も一緒\n");
			svflowsub_damage_act_enemy_all_atonce( server, fep, attacker, defender1, defender2, aff1, waza );
		}
	}
	// どっちかのみヒット
	else if( hit1 != !hit2 )
	{
		BTL_POKEPARAM* defender = (hit1)? defender1 : defender2;
		BtlTypeAff aff;

		aff = scEvent_checkAffinity( server, fep, attacker, defender, waza );
		svflowsub_damage_act_singular( server, fep, attacker, defender, waza, aff );
	}
	// ２体ともハズれ
	else
	{
		// return;
	}

	// 死亡チェック（ワザが当たらなくても反動などで死ぬケースはある）
	if( alive1 ){
		svflowsub_check_and_make_dead_command( server, defender1 );
	}
	if( alive2 ){
		svflowsub_check_and_make_dead_command( server, defender2 );
	}
	svflowsub_check_and_make_dead_command( server, attacker );

}
//------------------------------------------------------------------
// サーバーフロー下請け：単体ダメージ処理（下位）
//------------------------------------------------------------------
static void svflowsub_damage_act_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff )
{
	u8 pluralHit = FALSE;	// 複数回ヒットフラグ
	u8 deadFlag = FALSE;
	u8 atPokeID, defPokeID;
	int i;

	fep->hitCountMax = WAZADATA_GetMaxHitCount( waza );
	if( fep->hitCountMax > 1 )
	{
		fep->hitCountReal = scEvent_GetHitCount( server, fep, fep->hitCountMax );
		pluralHit = TRUE;
	}
	else
	{
		fep->hitCountReal = 1;
	}

	atPokeID = BTL_POKEPARAM_GetID( attacker );
	defPokeID = BTL_POKEPARAM_GetID( defender );

	for(i=0; i<fep->hitCountReal; ++i)
	{
		fep->realDamage = svflowsub_damage_calc_core( server, fep, attacker, defender, waza, aff );

		// デバッグを簡単にするため必ず大ダメージにする措置
		#ifdef PM_DEBUG
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
		{
			u8 pokeID = BTL_POKEPARAM_GetID(defender);
			if( pokeID >= BTL_PARTY_MEMBER_MAX )
			{
				fep->realDamage = 999;
			}
		}
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
		{
			u8 pokeID = BTL_POKEPARAM_GetID(defender);
			if( pokeID < BTL_PARTY_MEMBER_MAX )
			{
				fep->realDamage = 999;
			}
		}
		#endif

		BTL_POKEPARAM_HpMinus( defender, fep->realDamage );
		SCQUE_PUT_OP_HpMinus( server->que, defPokeID, fep->realDamage );
		svflowsub_set_waza_effect( server, atPokeID, defPokeID, waza );
		SCQUE_PUT_ACT_WazaDamage( server->que, defPokeID, fep->typeAff, fep->realDamage );
		BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_DMG_AFTER );

		BTL_Printf(" Waza Aff=%d, Damage=%d\n", fep->typeAff, fep->realDamage );

		if( BTL_POKEPARAM_IsDead(defender) )
		{
			deadFlag = TRUE;
		}
		if( BTL_POKEPARAM_IsDead(attacker) )
		{
			deadFlag = TRUE;
		}
		if( deadFlag ){ break; }
	}

	BTL_Printf("Damage Waza Exe %d times ... plural=%d, dead=%d\n",
			fep->hitCountReal, pluralHit, deadFlag);

	if( pluralHit )
	{
//			SCQUE_PUT_MSG_WazaHitCount( server->que, i );	// @@@ あとで
	}
}
//------------------------------------------------------------------
// サーバーフロー下請け：敵全体を順番にダメージ処理（下位）
//------------------------------------------------------------------
static void svflowsub_damage_act_enemy_all( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep,
		BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, BtlTypeAff aff1, BtlTypeAff aff2, WazaID waza )
{
	u8 atPokeID, defPokeID1, defPokeID2;
	u16 dmg1, dmg2;

	// 同時攻撃の時、ヒットカウントは１固定
	fep->hitCountMax = 1;
	fep->hitCountReal = 1;

	atPokeID = BTL_POKEPARAM_GetID( attacker );
	defPokeID1 = BTL_POKEPARAM_GetID( def1 );
	defPokeID2 = BTL_POKEPARAM_GetID( def2 );

	dmg1 = svflowsub_damage_calc_core( server, fep, attacker, def1, waza, aff1 );
	BTL_POKEPARAM_HpMinus( def1, dmg1 );
	SCQUE_PUT_OP_HpMinus( server->que, defPokeID1, fep->realDamage );

	dmg2 = svflowsub_damage_calc_core( server, fep, attacker, def2, waza, aff2 );
	BTL_POKEPARAM_HpMinus( def2, dmg2 );
	SCQUE_PUT_OP_HpMinus( server->que, defPokeID2, fep->realDamage );

	svflowsub_set_waza_effect( server, atPokeID, defPokeID1, waza );
	SCQUE_PUT_ACT_WazaDamage( server->que, defPokeID1, aff1, dmg1 );
	SCQUE_PUT_ACT_WazaDamage( server->que, defPokeID2, aff2, dmg2 );

	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_DMG_AFTER );
}
//------------------------------------------------------------------
// サーバーフロー下請け：敵全体を一斉にダメージ処理（下位）
//------------------------------------------------------------------
static void svflowsub_damage_act_enemy_all_atonce( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep,
		BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, BtlTypeAff aff, WazaID waza )
{
	u8 atPokeID, defPokeID1, defPokeID2;
	u16 dmg1, dmg2;

	// 同時攻撃の時、ヒットカウントは１固定
	fep->hitCountMax = 1;
	fep->hitCountReal = 1;

	atPokeID = BTL_POKEPARAM_GetID( attacker );
	defPokeID1 = BTL_POKEPARAM_GetID( def1 );
	defPokeID2 = BTL_POKEPARAM_GetID( def2 );

	dmg1 = svflowsub_damage_calc_core( server, fep, attacker, def1, waza, aff );
	BTL_POKEPARAM_HpMinus( def1, dmg1 );
	SCQUE_PUT_OP_HpMinus( server->que, defPokeID1, dmg1 );

	dmg2 = svflowsub_damage_calc_core( server, fep, attacker, def2, waza, aff );
	BTL_POKEPARAM_HpMinus( def2, dmg2 );
	SCQUE_PUT_OP_HpMinus( server->que, defPokeID2, dmg2 );

	svflowsub_set_waza_effect( server, atPokeID, defPokeID1, waza );
	SCQUE_PUT_ACT_WazaDamageDbl( server->que, defPokeID1, defPokeID2, dmg1, dmg2, aff );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_DMG_AFTER );
}

//------------------------------------------------------------------
// サーバーフロー下請け：１体分攻撃ヒットチェック
//------------------------------------------------------------------
static BOOL svflowsub_hitcheck_singular( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza )
{
	u8 defPokeID = BTL_POKEPARAM_GetID( defender );

	// 既に対象が死んでいる
	if( BTL_POKEPARAM_IsDead(defender) )
	{
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_WazaAvoid, defPokeID );
		return FALSE;
	}
	// 命中率・回避率によるヒットチェックで外れた
	if( !scEvent_checkHit(server, fep, attacker, defender, waza) )
	{
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_WazaAvoid, defPokeID );
		return FALSE;
	}
	return TRUE;
}
//------------------------------------------------------------------
// サーバーフロー下請け： たたかう > ダメージワザ系 > ダメージ値計算
//------------------------------------------------------------------
static u16 svflowsub_damage_calc_core( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff typeAff )
{
	fep->wazaDamageType = WAZADATA_GetDamageType( waza );

	GF_ASSERT(fep->wazaDamageType != WAZADATA_DMG_NONE);

	fep->criticalRank = scEvent_getCriticalRank( server, fep, attacker, waza );
	fep->criticalFlag = BTL_CALC_CheckCritical( fep->criticalRank );
	fep->wazaPower = scEvent_getWazaPower( server, fep, waza );
	fep->attackerPower = scEvent_getAttackPower( server, fep, attacker, waza );
	fep->defenderGuard = scEvent_getDefenderGuard( server, fep, defender, waza );

	fep->damageDenom = scEvent_getDamageDenom( server, fep );
	{
		u8 attackerLevel = BTL_POKEPARAM_GetValue( attacker, BPP_LEVEL );
		fep->rawDamage = fep->wazaPower * fep->attackerPower * (attackerLevel*2/5+2) / fep->defenderGuard;
		fep->rawDamage /= fep->damageDenom;
		if( fep->criticalFlag )
		{
			fep->rawDamage *= 2;
		}

		BTL_Printf("威力:%d, こうげき:%d, LV:%d, ぼうぎょ:%d, 分母:%d ... 素ダメ:%d\n",
				fep->wazaPower, fep->attackerPower, attackerLevel, fep->defenderGuard, fep->damageDenom, fep->rawDamage
		);
	}
	BTL_EVENT_CallHandlers( server, BTL_EVENT_DAMAGE_PROC1 );

	//ランダム補正
	{
		u16 ratio = 100 - GFL_STD_MtRand(16);
		fep->rawDamage = (fep->rawDamage * ratio) / 100;
		BTL_Printf("ランダム補正:%d%%  -> 素ダメ=%d\n", ratio, fep->rawDamage);
	}

	// タイプ一致補正
	fep->rawDamage = (fep->rawDamage * fep->typeMatchRatio) >> FX32_SHIFT;
	BTL_Printf("タイプ一致補正:%08x  -> 素ダメ=%d\n", fep->typeMatchRatio, fep->rawDamage);

	// タイプ相性計算
	fep->realDamage = BTL_CALC_AffDamage( fep->rawDamage, typeAff );

	// やけど補正
	if( fep->wazaDamageType == WAZADATA_DMG_PHYSIC )
	{
		if( BTL_POKEPARAM_GetPokeSick(attacker) == POKESICK_YAKEDO )
		{
			fep->realDamage = (fep->realDamage * BTL_YAKEDO_DAMAGE_RATIO) / 100;
		}
	}

	// 同時攻撃補正
	

	// 最低、１はダメージを与える
	if( fep->realDamage == 0 )
	{
		fep->realDamage = 1;
	}

	BTL_Printf("タイプ相性:%02d -> ダメージ値：%d\n", typeAff, fep->realDamage);

	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_DMG_PROC2 );

	return fep->realDamage;
}
//---------------------------------------------------------------------------------------------
// １度のワザ処理にて、エフェクト発動コマンドを１回しか作成しないための仕組み
//---------------------------------------------------------------------------------------------
static void svflowsub_set_waza_effect( BTL_SERVER* server, u8 atPokeID, u8 defPokeID, WazaID waza )
{
	if( FlowFlg_Get(server, FLOWFLG_SET_WAZAEFFECT ) == FALSE )
	{
		BTL_Printf("ワザエフェクトコマンド生成しますよ, ワザナンバ%d\n", waza);
		SCQUE_PUT_ACT_WazaEffect( server->que, atPokeID, defPokeID, waza );
		FlowFlg_Set( server, FLOWFLG_SET_WAZAEFFECT );
	}
	else
	{
		BTL_Printf("ワザエフェクトコマンド生成済みなので無視, ワザナンバ%d\n", waza);
	}
}

//==============================================================================
// サーバーフロー：ターンチェックルート
//==============================================================================
static void scput_TurnCheck( BTL_SERVER* server )
{
	scput_turncheck_sick( server );
//	scput_turncheck_weather( server );
}


static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SERVER* server )
{
	fpsw->clientIdx = 0;
	fpsw->pokeIdx = 0;
	fpsw->endFlag = TRUE;

	{
		CLIENT_WORK* cw;
		u8 i, j;

		for(i=0; i<BTL_CLIENT_MAX; ++i)
		{
			cw = &server->client[i];
			if( cw->myID == CLIENT_DISABLE_ID ){ continue; }

			for(j=0; j<cw->numCoverPos; ++j)
			{
				if( !BTL_POKEPARAM_IsDead(cw->frontMember[j]) )
				{
					fpsw->clientIdx = i;
					fpsw->pokeIdx = j;
					fpsw->endFlag = FALSE;
					return;
				}
			}
		}
	}
}
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SERVER* server, BTL_POKEPARAM** bpp )
{
	if( fpsw->endFlag )
	{
		return FALSE;
	}
	else
	{
		CLIENT_WORK* cw = &server->client[ fpsw->clientIdx ];
		BTL_POKEPARAM* nextPoke = NULL;

		*bpp = cw->frontMember[ fpsw->pokeIdx++ ];

		while( fpsw->clientIdx < BTL_CLIENT_MAX )
		{
			cw = &server->client[ fpsw->clientIdx ];
			while( fpsw->pokeIdx < cw->numCoverPos )
			{
				nextPoke = cw->frontMember[ fpsw->pokeIdx ];
				if( !BTL_POKEPARAM_IsDead(nextPoke) )
				{
					return TRUE;
				}
				fpsw->pokeIdx++;
			}
			fpsw->clientIdx++;
			fpsw->pokeIdx = 0;
		}
		fpsw->endFlag = TRUE;
		return TRUE;
	}
}


//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > 状態異常
//------------------------------------------------------------------
static void scput_turncheck_sick( BTL_SERVER* server )
{
	FIGHT_EVENT_PARAM* fep;
	FRONT_POKE_SEEK_WORK  fpsw;
	BTL_POKEPARAM* bpp;
	u8 pokeID;

	fep = &server->fightEventParams;
	FRONT_POKE_SEEK_InitWork( &fpsw, server );

	while( FRONT_POKE_SEEK_GetNext( &fpsw, server, &bpp ) )
	{
		pokeID = BTL_POKEPARAM_GetID( bpp );

		fep->sick = BTL_POKEPARAM_GetPokeSick( bpp );
		fep->sickDamage = BTL_POKEPARAM_CalcSickDamage( bpp );
		if( fep->sick != POKESICK_NULL && fep->sickDamage )
		{
			BTL_EVENT_CallHandlers( server, BTL_EVENT_SICK_DAMAGE );
			if( fep->sickDamage > 0 )
			{
				BTL_POKEPARAM_HpMinus( bpp, fep->sickDamage );
				SCQUE_PUT_OP_HpMinus( server->que, pokeID, fep->sickDamage );
				SCQUE_PUT_SickDamage( server->que, pokeID, fep->sick, fep->sickDamage );
			}
		}
		BTL_POKEPARAM_WazaSick_TurnCheck( bpp );
		SCQUE_PUT_OP_WazaSickTurnCheck( server->que, pokeID );
	}
}

//---------------------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------------------
static inline int roundValue( int val, int min, int max )
{
	if( val < min ){ val = min; }
	if( val > max ){ val = max; }
	return val;
}


//---------------------------------------------------------------------------------------------
// コマンド出力  第１階層
//---------------------------------------------------------------------------------------------

/*
	ワザだせるか判定 *
		＋メロメロ
		＋こんらん
		＋まひ
		＋ちょうはつ
		＋ひるみ
		＋ふういん
		→出せない場合は終わり

	ワザ対象チェック
		・対象がポケモン      →Ａへ
		・対象がポケモン以外  →Ｂへ

	Ａ・ポケモン対象ワザ
		・対象範囲チェック
			＋１体
			＋（ダブルのみ）自分以外全部
			＋（ダブルのみ）相手２体
			＋自分１体

			※対象人数分処理

			・ワザあたるか判定 
			   ・ワザ的中率取得		(*)
			   ・攻撃側命中率取得	(*)
			   ・防御側回避率取得	(*)
			   →当たらない場合はＡ１へ

			・ワザカテゴリチェック
				・ダメージワザ    →Ａ２へ
				・非ダメージワザ  →Ａ３へ


		Ａ１・ポケモン対象ワザはずれ処理
			・はずれハンドラ呼び出し(*)
			瀕死＆決着判定
			→終了

		Ａ２・ダメージワザヒット処理
			・クリティカル率取得		(*)
			・クリティカル判定
			・ワザ威力値取得			(*)
			・攻撃側能力値取得			(*)
			・攻撃側レベル取得
			・防御側能力値取得			(*)
			・ダメージ分母計算			(*)
			・クリティカル補正
			・タイプ計算前ダメージ補正	(*)
			・ランダム補正
			・ワザタイプ取得			(*)
			・攻撃ポケタイプ取得		(*)
			・防御ポケタイプ取得		(*)
			・タイプ一致補正
			・タイプ相性取得
			・タイプ相性補正
			・タイプ計算後ダメージ補正	(*)
			・最終補正					(*)
			・反動計算					(*)
			・瀕死＆決着判定１
			・ダメージ受けたハンドラ	(*)
			・瀕死＆決着判定２
			  ・決着なら→終了
			・自分への追加効果処理		(*)
			・相手への追加効果処理		(*)
			  ・瀕死になった   →Ｃ・ダメージ技による瀕死処理 へ
			  ・瀕死にならない →Ｄ・

		Ａ３・非ダメージワザヒット処理
			


	Ｂ・ポケモン以外対象ワザ
		・対象範囲チェック
			・陣営
			・場

	->tmp.c
*/

static void initFightEventParams( FIGHT_EVENT_PARAM* fep )
{
	GFL_STD_MemClear( fep, sizeof(*fep) );
}


//---------------------------------------------------------------------------------------------
// コマンド出力  イベント群
//---------------------------------------------------------------------------------------------

// こんらん自爆チェック
static BOOL scEvent_CheckConf( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const CLIENT_WORK* atClient )
{
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_KONRAN) )
	{
//		SCQUE_PUT_MSG_ConfAnnounce( server->que, atClient->myID );	// @@@ 
		fep->confFlag = ( GFL_STD_MtRand(100) < BTL_CALC_CONF_PER );
		return fep->confFlag;
	}
	return FALSE;
}

// 混乱ダメージ計算
static u16 scEvent_CalcConfDamage( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker )
{
	u16 dmg = BTL_POKEPARAM_GetValue(attacker, BPP_MAX_HP) / 8;

	fep->confDamage = dmg;

	BTL_EVENT_CallHandlers( server, BTL_EVENT_CALC_CONF_DAMAGE );
	return fep->confDamage;
}

// ワザだせるか判定
static BOOL scEvent_CheckWazaExecute( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, BTL_POKEPARAM* attacker, const CLIENT_WORK* atClient )
{
	// @@@ ひるみ、まひ、メロメロ等
	WazaSick  sick;
	fep->wazaExecuteFlag = TRUE;
	fep->wazaFailReason = SV_WAZAFAIL_NULL;

	do {
		sick = BTL_POKEPARAM_GetPokeSick( attacker );

		switch( sick ){
		case POKESICK_NEMURI:
			if( !BTL_POKEPARAM_Nemuri_CheckWake(attacker) )
			{
				fep->wazaExecuteFlag = FALSE;
				fep->wazaFailReason = SV_WAZAFAIL_NEMURI;
			}
			break;
		case POKESICK_MAHI:
			if( GFL_STD_MtRand(100) < BTL_MAHI_EXE_RATIO )
			{
				fep->wazaExecuteFlag = FALSE;
				fep->wazaFailReason = SV_WAZAFAIL_MAHI;
			}
			break;
		case POKESICK_KOORI:
			if( GFL_STD_MtRand(100) >= BTL_KORI_MELT_RATIO )
			{
				fep->wazaExecuteFlag = FALSE;
				fep->wazaFailReason = SV_WAZAFAIL_KOORI;
			}
			else
			{
				u8 pokeID = BTL_POKEPARAM_GetID( attacker );
				BTL_POKEPARAM_SetPokeSick( attacker, POKESICK_NULL, 0 );
				SCQUE_PUT_OP_SetSick( server->que, pokeID, POKESICK_NULL, 0 );
				SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_KoriMelt, pokeID );
			}
			break;
		}

	}while(0);

	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_EXECUTE );

	return fep->wazaExecuteFlag;
}

// ワザ当たるか判定
static BOOL scEvent_checkHit( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	if( WAZADATA_IsAlwaysHit(waza) )
	{
		return TRUE;
	}
	else
	{
		u8 wazaHitRatio, per;
		s8 hitRank, avoidRank, totalRank;

		wazaHitRatio = scEvent_getHitRatio(server, fep, waza);

		hitRank = BTL_POKEPARAM_GetValue( attacker, BPP_HIT_RATIO );
		avoidRank = BTL_POKEPARAM_GetValue( defender, BPP_AVOID_RATIO );
		totalRank = roundValue( BTL_CALC_HITRATIO_MID + avoidRank - hitRank, BTL_CALC_HITRATIO_MIN, BTL_CALC_HITRATIO_MAX );

		per = BTL_CALC_HitPer( wazaHitRatio, totalRank );

		return ( GFL_STD_MtRand(100) < (u32)per );
	}
}

// ワザ相性チェック
static BtlTypeAff scEvent_checkAffinity( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	// タイプ一致補正
	fep->wazaPokeType = scEvent_getWazaPokeType( server, fep, waza );
	fep->attackerPokeType = scEvent_getAttackerPokeType( server, fep, attacker );

	if( PokeTypePair_IsMatch(fep->attackerPokeType, fep->wazaPokeType) )
	{
		fep->typeMatchRatio = FX32_CONST(1.5f);
	}
	else
	{
		fep->typeMatchRatio = FX32_CONST(1.0f);
	}
	BTL_EVENT_CallHandlers( server, BTL_EVENT_TYPEMATCH_RATIO );

	// タイプ相性計算
	fep->defenderPokeType = scEvent_getDefenderPokeType( server, fep, defender );
	fep->typeAff = BTL_CALC_TypeAff( fep->wazaPokeType, fep->defenderPokeType );

	return fep->typeAff;
}

// 使ったワザのPPデクリメント
static void scEvent_decrementPP( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, u8 wazaIdx )
{
	fep->decPP = 1;

	BTL_EVENT_CallHandlers( server, BTL_EVENT_DECREMENT_PP );

	if( fep->decPP )
	{
		u8 pokeID = BTL_POKEPARAM_GetID( attacker );
		BTL_Printf("DECPP_ コマンドセット, value=%d\n", fep->decPP);
		SCQUE_PUT_OP_PPMinus( server->que, pokeID, wazaIdx, fep->decPP );
	}

}

// ワザ的中率取得
static u8 scEvent_getHitRatio( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza )
{
	fep->wazaHitRatio = WAZADATA_GetHitRatio( waza );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_HIT_RATIO );
	return fep->wazaHitRatio;
}

// ヒットする最大回数を取得
static u8 scEvent_GetHitCount( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, u8 hitCountMax )
{
	fep->hitCountMax = hitCountMax;
	fep->hitCountReal =  BTL_CALC_HitCountMax( hitCountMax );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_HIT_COUNT );
	return fep->hitCountReal;
}


// クリティカルランク取得
static u8 scEvent_getCriticalRank( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, WazaID waza )
{
	fep->criticalRank = BTL_POKEPARAM_GetValue( attacker, BPP_CRITICAL_RATIO );
	fep->criticalRank += WAZADATA_GetCriticalRank( waza );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_CRITICAL_RANK );

	if( fep->criticalRank > BTL_CALC_CRITICAL_MAX )
	{
		fep->criticalRank = BTL_CALC_CRITICAL_MAX;
	}

	return fep->criticalRank;
}

// ワザ威力取得
static u16 scEvent_getWazaPower( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza )
{
	fep->wazaPower = WAZADATA_GetPower( waza );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_POWER );
	return fep->wazaPower;
}

// 攻撃側能力値取得
static u16 scEvent_getAttackPower( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker, WazaID waza )
{
	BppValueID vid;

	switch( fep->wazaDamageType ){
	case WAZADATA_DMG_PHYSIC:	vid = BPP_ATTACK; break;
	case WAZADATA_DMG_SPECIAL:	vid = BPP_SP_ATTACK; break;
	default:
		GF_ASSERT(0);
		vid = BPP_ATTACK;
		break;
	}

	if( fep->criticalFlag )
	{
		fep->attackerPower = BTL_POKEPARAM_GetValueCriticalHit( attacker, vid );
	}
	else
	{
		fep->attackerPower = BTL_POKEPARAM_GetValue( attacker, vid );
	}

	BTL_EVENT_CallHandlers( server, BTL_EVENT_ATTACKER_POWER );

	return fep->attackerPower;
}

// 防御側能力値取得
static u16 scEvent_getDefenderGuard( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* defender, WazaID waza )
{
	BppValueID vid;

	switch( fep->wazaDamageType ){
	case WAZADATA_DMG_PHYSIC:	vid = BPP_DEFENCE; break;
	case WAZADATA_DMG_SPECIAL:	vid = BPP_SP_DEFENCE; break;
	default:
		GF_ASSERT(0);
		vid = BPP_ATTACK;
		break;
	}

	if( fep->criticalFlag )
	{
		fep->defenderGuard = BTL_POKEPARAM_GetValueCriticalHit( defender, vid );
	}
	else
	{
		fep->defenderGuard = BTL_POKEPARAM_GetValue( defender, vid );
	}

	BTL_EVENT_CallHandlers( server, BTL_EVENT_DEFENDER_GUARD );

	return fep->defenderGuard;
}

// ダメージ計算用分母取得
static u16 scEvent_getDamageDenom( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep )
{
	fep->damageDenom = 50;
	BTL_EVENT_CallHandlers( server, BTL_EVENT_DAMAGE_DENOM );
	fep->damageDenom += 2;

	return fep->damageDenom;
}

// ワザタイプ取得
static PokeType scEvent_getWazaPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, WazaID waza )
{
	fep->wazaPokeType = WAZADATA_GetType( waza );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_WAZA_TYPE );
	return fep->wazaPokeType;
}

// 攻撃側タイプ取得
static PokeTypePair scEvent_getAttackerPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* attacker )
{
	fep->attackerPokeType = BTL_POKEPARAM_GetPokeType( attacker );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_ATTACKER_TYPE );
	return fep->attackerPokeType;
}

// 防御側タイプ取得
static PokeTypePair scEvent_getDefenderPokeType( BTL_SERVER* server, FIGHT_EVENT_PARAM* fep, const BTL_POKEPARAM* defender )
{
	fep->defenderPokeType = BTL_POKEPARAM_GetPokeType( defender );
	BTL_EVENT_CallHandlers( server, BTL_EVENT_DEFENDER_TYPE );
	return fep->defenderPokeType;
}

// メンバーバトル場から退出
static void scEvent_MemberOut( BTL_SERVER* server, CHANGE_EVENT_PARAM* cep, u8 clientID, u8 posIdx )
{
	cep->outPokeParam = BTL_MAIN_GetClientPokeData( server->mainModule, clientID, posIdx );

	SCQUE_PUT_MSG_STD( server->que, BTL_STRID_STD_MemberOut1, clientID, posIdx );

	BTL_EVENT_CallHandlers( server, BTL_EVENT_MEMBER_OUT );
	// ここで死ぬこともある

	if( !BTL_POKEPARAM_IsDead(cep->outPokeParam) )
	{
		SCQUE_PUT_ACT_MemberOut( server->que, clientID, posIdx );
	}
}

// メンバー新規参加
static void scEvent_MemberIn( BTL_SERVER* server, CHANGE_EVENT_PARAM* cep, u8 clientID, u8 posIdx, u8 nextPokeIdx )
{
//	cep->inPokeParam = BTL_MAIN_GetPokeParam
		SCQUE_PUT_ACT_MemberIn( server->que, clientID, posIdx, nextPokeIdx );
//	SCQUE_PUT_ACT_MemberIn( server->que, clientID, memberIdx );
}

// ポケモン出そろった直後
static void scEvent_PokeComp( BTL_SERVER* server )
{
	BTL_EVENT_CallHandlers( server, BTL_EVENT_MEMBER_COMP );
}

// 能力を下げる
static void scEvent_RankDown( BTL_SERVER* server, u8 pokeID, BtlPokePos pokePos, BppValueID statusType, u8 volume )
{
	server->eventArgs[ BTL_EVARG_COMMON_POKEPOS ] = pokePos;
	server->eventArgs[ BTL_EVARG_COMMON_POKEID ] = pokeID;
	server->eventArgs[ BTL_EVARG_RANKDOWN_STATUS_TYPE ] = statusType;
	server->eventArgs[ BTL_EVARG_RANKDOWN_VOLUME ] = volume;
	server->eventArgs[ BTL_EVARG_RANKDOWN_FAIL_FLAG ] = FALSE;

	BTL_EVENT_CallHandlers( server, BTL_EVENT_BEFORE_RANKDOWN );

	if( server->eventArgs[ BTL_EVARG_RANKDOWN_FAIL_FLAG ] == FALSE )
	{
		volume = server->eventArgs[ BTL_EVARG_RANKDOWN_VOLUME ];
		BTL_POKEPARAM_RankDown(
//				server->client[targetClientID].frontMember,
				BTL_MAIN_GetFrontPokeData( server->mainModule, pokePos ),
				statusType, 
				volume
		);

		SCQUE_PUT_OP_RankDown( server->que, pokePos, statusType, volume );
		SCQUE_PUT_ACT_RankDown( server->que, pokePos, statusType, volume );
		SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_Rankdown_ATK, pokePos, statusType, volume );
	}
}

//----------------------------------------------------------------------------------------------------------
// 以下、ハンドラからの応答受信関数とユーティリティ群
//----------------------------------------------------------------------------------------------------------

int BTL_SERVER_RECEPT_GetEventArg( const BTL_SERVER* server, u8 idx )
{
	GF_ASSERT(idx < BTL_EVARG_MAX);
	return server->eventArgs[ idx ];
}

void BTL_SERVER_RECEPT_SetEventArg( BTL_SERVER* server, u8 idx, int value )
{
	GF_ASSERT(idx < BTL_EVARG_MAX);
	server->eventArgs[ idx ] = value;
}

//=============================================================================================
/**
 * とくせいウィンドウ表示イン
 *
 * @param   server		
 * @param   clientID		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_TokuseiWinIn( BTL_SERVER* server, u8 clientID )
{
	SCQUE_PUT_TOKWIN_IN( server->que, clientID );
}
//=============================================================================================
/**
 * とくせいウィンドウ表示アウト
 *
 * @param   server		
 * @param   pokeID		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_TokuseiWinOut( BTL_SERVER* server, u8 clientID )
{
	SCQUE_PUT_TOKWIN_OUT( server->que, clientID );
}

void BTL_SERVER_RECTPT_SetMessage( BTL_SERVER* server, u16 msgID, BtlPokePos pokePos )
{
	SCQUE_PUT_MSG_SET( server->que, msgID, pokePos );
}

//=============================================================================================
/**
 * [ハンドラ受信] ステータスのランクダウン効果
 *
 * @param   server			
 * @param   exPos					対象ポケモン位置
 * @param   statusType		ステータスタイプ
 * @param   volume		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_RankDownEffect( BTL_SERVER* server, BtlExPos exPos, BppValueID statusType, u8 volume )
{
	const BTL_POKEPARAM* pp;
	u8 targetPos[ BTL_POSIDX_MAX ];
	u8 numPokemons, pokeID, i;

	numPokemons = BTL_MAIN_ExpandBtlPos( server->mainModule, exPos, targetPos );
	BTL_Printf("ランクさげ効果：タイプ=%d,  対象ポケモン数=%d\n", statusType, numPokemons );
	for(i=0; i<numPokemons; i++)
	{
		pp = BTL_MAIN_GetFrontPokeData( server->mainModule, targetPos[i] );
		pokeID = BTL_POKEPARAM_GetID( pp );
		scEvent_RankDown( server, pokeID, targetPos[i], statusType, volume );
	}
}

//=============================================================================================
/**
 * [ハンドラ受信] 減少PP値の修正
 *
 * @param   server		
 * @param   volume		修正後のPP
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_SetDecrementPP( BTL_SERVER* server, u8 volume )
{
	server->fightEventParams.decPP = volume;
}


