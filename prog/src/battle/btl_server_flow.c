//=============================================================================================
/**
 * @file	btl_server_flow.c
 * @brief	ポケモンWB バトルシステム	サーバコマンド生成処理
 * @author	taya
 *
 * @date	2009.03.06	作成
 */
//=============================================================================================
#include <gflib.h>
#include "waza_tool\wazadata.h"
#include "waza_tool\wazano_def.h"

#include "btl_event.h"
#include "btl_server_cmd.h"
#include "btl_field.h"
#include "btl_server_local.h"
#include "btl_server.h"

#include "handler\hand_tokusei.h"

#include "btl_server_flow.h"


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

typedef struct {

	BTL_POKEPARAM*  inPokeParam;
	BTL_POKEPARAM*  outPokeParam;

}CHANGE_EVENT_PARAM;


/**
 *	アクション優先順記録構造体
 */
typedef struct {
	u8  clientID;		///< クライアントID
	u8  pokeIdx;		///< そのクライアントの、何体目？ 0～
}ACTION_ORDER_WORK;


// １度のワザ処理でダメージを受けたポケモンの情報を記憶しておく
typedef struct {

	BTL_POKEPARAM*	pp[ BTL_POS_MAX ];
	u8							count;
	u8							idx;

}TARGET_POKE_REC;

struct _BTL_SVFLOW_WORK {

	BTL_SERVER*             server;
	const BTL_MAIN_MODULE*  mainModule;
	BTL_POKE_CONTAINER*			pokeCon;
	BTL_SERVER_CMD_QUE*			que;
	u32											turnCount;
	u8                      numClient;

	ACTION_ORDER_WORK		actOrder[ BTL_POS_MAX ];
	TARGET_POKE_REC			targetPokemon;
	TARGET_POKE_REC			targetSubPokemon;
	TARGET_POKE_REC			damagedPokemon;


	u8					flowFlags[ FLOWFLG_BYTE_MAX ];

};

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


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static u8 sortClientAction( BTL_SVFLOW_WORK* wk, ACTION_ORDER_WORK* order );
static u8 countAlivePokemon( BTL_SVFLOW_WORK* wk );
static inline void FlowFlg_Set( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_Clear( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline BOOL FlowFlg_Get( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_ClearAll( BTL_SVFLOW_WORK* wk );
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk );
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk, BTL_POKEPARAM** bpp );
static inline void TargetPokeRec_Clear( TARGET_POKE_REC* rec );
static inline void TargetPokeRec_Add( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp );
static inline void TargetPokeRec_Remove( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp );
static inline BTL_POKEPARAM* TargetPokeRec_GetNext( TARGET_POKE_REC* rec );
static inline void TargetPokeRec_GetStart( TARGET_POKE_REC* rec );
static inline BTL_POKEPARAM* TargetPokeRec_Get( const TARGET_POKE_REC* rec, u32 idx );
static inline int TargetPokeRec_SeekFriendIdx( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, u32 start_idx );
static inline u32 TargetPokeRec_GetCount( const TARGET_POKE_REC* rec );
static inline u32 TargetPokeRec_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst );
static inline u32 TargetPokeRec_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst );
static void scput_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx );
static void scput_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx );
static void scput_Fight( BTL_SVFLOW_WORK* wk, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action );
static void cof_put_wazafail_msg_cmd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, u8 wazaFailReason );
static u8 flowsub_registerWazaTargets( BTL_SVFLOW_WORK* wk, BtlPokePos atPos, WazaID waza, const BTL_ACTION_PARAM* action, TARGET_POKE_REC* rec );
static BTL_POKEPARAM* svflowsub_get_opponent_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos, u8 pokeSideIdx );
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos );
static void flowsub_checkNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scput_Fight_Damage( BTL_SVFLOW_WORK* wk, WazaID waza, SVCL_WORK* atClient,
	 BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scput_Fight_Damage_side( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker,
	TARGET_POKE_REC* targets, fx32 dmg_ratio );
static void svflowsub_check_and_make_dead_command( BTL_SVFLOW_WORK* server, BTL_POKEPARAM* poke );
static void svflowsub_damage_singular( BTL_SVFLOW_WORK* wk,
			BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, fx32 targetDmgRatio );
static void svflowsub_damage_act_singular(  BTL_SVFLOW_WORK* wk,
		BTL_POKEPARAM* attacker,  BTL_POKEPARAM* defender,
		WazaID waza,   BtlTypeAff aff,   fx32 targetDmgRatio );
static void svflowsub_damage_plural( BTL_SVFLOW_WORK* wk, 
		BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets, const u8* aff, WazaID waza, fx32 dmg_ratio );
static void scput_Fight_Damage_After( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scput_Fight_Damage_After_Shrink( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static BOOL svflowsub_hitcheck_singular( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza );
static u16 svflowsub_damage_calc_core( BTL_SVFLOW_WORK* wk, 
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza,
		BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag );
static void svflowsub_reaction_damage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 damage );
static void svflowsub_set_waza_effect( BTL_SVFLOW_WORK* wk, u8 atPokeID, u8 defPokeID, WazaID waza );
static void scput_Fight_AddSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker );
static void scput_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static void scput_Fight_AddEffect_Target( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker );
static void scput_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action );
static void scput_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void svflowsub_MakeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
		WazaSick sick, BPP_SICK_CONT contParam, BOOL fAlmost );
static void scput_Fight_Weather( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, const BTL_ACTION_PARAM* action );
static void scput_Fight_Others( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scput_Fight_Others_SkillSwap( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec );
static void scput_TurnCheck( BTL_SVFLOW_WORK* wk );
static void scput_turncheck_sick( BTL_SVFLOW_WORK* wk );
static void scput_turncheck_weather( BTL_SVFLOW_WORK* wk );
static int scEvent_CalcWeatherDamage( BTL_SVFLOW_WORK* wk, BtlWeather weather, BTL_POKEPARAM* bpp, u8 *tok_cause_flg );
static inline int roundValue( int val, int min, int max );
static inline int roundMin( int val, int min );
static inline int roundMax( int val, int max );
static inline BOOL perOccur( u8 per );
static u16 scEvent_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckConf( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static u16 scEvent_CalcConfDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza );
static BOOL scEvent_checkHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_IchigekiCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckNotEffect_byType( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckCritical( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static void scEvent_WazaDamageEffect_Single( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff, u32 damage, BOOL criticalFlag );
static void scEvent_WazaDamageEffect_Double( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, WazaID waza, BtlTypeAff aff,
	u32 dmg1, u32 dmg2 );
static BtlTypeAff scEvent_checkAffinity( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, PokeType waza_type );
static void scEvent_decrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, u8 wazaIdx, TARGET_POKE_REC* rec );
static u8 scEvent_getHitPer( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckPluralHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u8* hitCount );
static u16 scEvent_getWazaPower( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static u16 scEvent_getAttackPower( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag );
static u16 scEvent_getDefenderGuard( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag );
static fx32 scEvent_CalcTypeMatchRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, PokeType waza_type );
static PokeType scEvent_getWazaPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static PokeTypePair scEvent_getAttackerPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static PokeTypePair scEvent_getDefenderPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender );
static void scEvent_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx );
static void scEvent_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx );
static void scEvent_AddShrink( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker, u8 per );
static void scEvent_CheckAddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker, WazaID waza );
static void scEvent_MakePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	PokeSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static void scEvent_MakeWazaSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static void scEvent_CheckAddEffect_Target( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker, WazaID waza );
static void scEvent_RankDown( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BppValueID statusType, u8 volume, BOOL fAlmost );
static void scEvent_RankUp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* pp, BppValueID statusType, u8 volume, BOOL fAlmost );
static BOOL scEvent_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn );




BTL_SVFLOW_WORK* BTL_SVFLOW_InitSystem( 
	BTL_SERVER* server, BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokeCon,
	BTL_SERVER_CMD_QUE* que, u32 numClient, HEAPID heapID )
{
	BTL_SVFLOW_WORK* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SVFLOW_WORK) );

	wk->server = server;
	wk->pokeCon = pokeCon;
	wk->mainModule = mainModule;
	wk->numClient = numClient;
	wk->turnCount = 0;
	wk->que = que;

	return wk;
}


void BTL_SVFLOW_QuitSystem( BTL_SVFLOW_WORK* wk )
{
	GFL_HEAP_FreeMemory( wk );
}


SvflowResult BTL_SVFLOW_Start_AfterPokemonIn( BTL_SVFLOW_WORK* wk )
{
	FRONT_POKE_SEEK_WORK fps;
	BTL_POKEPARAM* bpp;

	SCQUE_Init( wk->que );

	BTL_EVENTVAR_Push();
		FRONT_POKE_SEEK_InitWork( &fps, wk );
		while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
		{
			BTL_HANDLER_TOKUSEI_Add( bpp );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(bpp) );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_MEMBER_IN );
		}
	BTL_EVENTVAR_Pop();

	// @@@ しぬこともあるかも
	return SVFLOW_RESULT_DEFAULT;
}

SvflowResult BTL_SVFLOW_Start( BTL_SVFLOW_WORK* wk )
{
	BTL_POKEPARAM* bpp;
	SVCL_WORK* clwk;
	u8 numActPoke, alivePokeBefore, alivePokeAfter, clientID, pokeIdx, i;

	SCQUE_Init( wk->que );
	FlowFlg_ClearAll( wk );

	alivePokeBefore = countAlivePokemon( wk );

	numActPoke = sortClientAction( wk, wk->actOrder );
	for(i=0; i<numActPoke; i++)
	{
		clientID = wk->actOrder[i].clientID;
		pokeIdx  = wk->actOrder[i].pokeIdx;
		clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
		bpp = BTL_SVCL_GetFrontPokeData( clwk, pokeIdx );

		if( !BTL_POKEPARAM_IsDead(bpp) )
		{
			const BTL_ACTION_PARAM* action = BTL_SVCL_GetPokeAction( clwk, pokeIdx );

			BTL_POKEPARAM_SetTurnFlag( bpp, BPP_TURNFLG_ACTION_EXE );

			BTL_Printf("Client(%d) の %d 番目のポケモンのアクション\n", clientID, pokeIdx);
			switch( action->gen.cmd ){
			case BTL_ACTION_FIGHT:
				BTL_Printf("【たたかう】を処理。%d番目のワザを、%d番の相手に。\n", action->fight.wazaIdx, action->fight.targetPos);
				scput_Fight( wk, clientID, pokeIdx, action );
				break;
			case BTL_ACTION_ITEM:
				BTL_Printf("【どうぐ】を処理。アイテム%dを、%d番の相手に。\n", action->item.number, action->item.targetIdx);
				break;
			case BTL_ACTION_CHANGE:
				BTL_Printf("【ポケモン】を処理。位置%d <- ポケ%d \n", action->change.posIdx, action->change.memberIdx);
				scput_MemberOut( wk, clientID, action->change.posIdx );
				scput_MemberIn( wk, clientID, action->change.posIdx, action->change.memberIdx );
				break;
			case BTL_ACTION_ESCAPE:
				BTL_Printf("【にげる】を処理。\n");
				// @@@ 今は即座に逃げられるようにしている
				SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_EscapeSuccess, clientID );
				return SVFLOW_RESULT_BTL_QUIT;
			}
		}
	}

	// ターンチェック処理
	scput_TurnCheck( wk );

	alivePokeAfter = countAlivePokemon( wk );

	// 死んだポケモンがいる場合の処理
	BTL_Printf( "ポケモン数 %d -> %d ...\n", alivePokeBefore, alivePokeAfter );
	if( alivePokeBefore > alivePokeAfter )
	{
		return SVFLOW_RESULT_POKE_DEAD;
	}

	return SVFLOW_RESULT_DEFAULT;
}
//--------------------------------------------------------------------------
/**
 * １ターン分サーバコマンド生成（ポケモン選択後）
 *
 * @param   wk		
 *
 * @retval  SvflowResult		
 */
//--------------------------------------------------------------------------
SvflowResult BTL_SVFLOW_StartAfterPokeSelect( BTL_SVFLOW_WORK* wk )
{
	const BTL_ACTION_PARAM* action;
	SVCL_WORK* clwk;
	u16 clientID, posIdx;
	int i, j, actionCnt;

	BTL_Printf("ひんしポケモン入れ替え選択後のサーバーコマンド生成\n");

	SCQUE_Init( wk->que );

	for(i=0; i<wk->numClient; ++i)
	{
		clwk = BTL_SERVER_GetClientWork( wk->server, i );
		actionCnt = BTL_SVCL_GetNumActPoke( clwk );

		for(j=0; j<actionCnt; ++j)
		{
			action = BTL_SVCL_GetPokeAction( clwk, j );
			if( action->gen.cmd != BTL_ACTION_CHANGE ){ continue; }
			if( action->change.depleteFlag ){ continue; }

			BTL_Printf("クライアント(%d)のポケモン(位置%d) を、%d番目のポケといれかえる\n",
						i, action->change.posIdx, action->change.memberIdx );

			scput_MemberIn( wk, i, action->change.posIdx, action->change.memberIdx );
		}
	}

	{
		// @@@ いずれ「まきびし」とかで死ぬこともある
		return SVFLOW_RESULT_DEFAULT;
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
static u8 sortClientAction( BTL_SVFLOW_WORK* wk, ACTION_ORDER_WORK* order )
{
	/*
		行動優先順  ... 2BIT
		ワザ優先順  ... 6BIT
		素早さ      ... 16BIT
	*/
	#define MakePriValue( actPri, wazaPri, agility )	\
						( ((actPri)<<22) | ((wazaPri)<<16) | (agility) )

	SVCL_WORK* clwk;
	const BTL_ACTION_PARAM* actParam;
	const BTL_POKEPARAM* bpp;
	u32 pri[ BTL_POS_MAX ];
	u16 agility;
	u8  action, actionPri, wazaPri;
	u8  i, j, p, numPoke;

// 各ポケモンの行動プライオリティ値を生成
	for(i=0, p=0; i<wk->numClient; i++)
	{
		// クライアントごとの行動ポケモン数＆パラメータをアダプタから取得
		clwk = BTL_SERVER_GetClientWork( wk->server, i );

		numPoke = BTL_SVCL_GetNumActPoke( clwk );
		for(j=0; j<numPoke; j++)
		{
			bpp = BTL_SVCL_GetFrontPokeData( clwk, j );
			actParam = BTL_SVCL_GetPokeAction( clwk, j );

			// 行動による優先順（優先度高いほど数値大）
			switch( actParam->gen.cmd ){
			case BTL_ACTION_ESCAPE:	actionPri = 3; break;
			case BTL_ACTION_ITEM:		actionPri = 2; break;
			case BTL_ACTION_CHANGE:	actionPri = 1; break;
			case BTL_ACTION_FIGHT:	actionPri = 0; break;
			case BTL_ACTION_NULL:	continue;
			default:
				GF_ASSERT(0);
				actionPri = 0;
				break;
			}
			// ワザによる優先順
			if( actParam->gen.cmd == BTL_ACTION_FIGHT )
			{
				WazaID  w = BTL_POKEPARAM_GetWazaNumber( bpp, actParam->fight.wazaIdx );
				wazaPri = WAZADATA_GetPriority( w ) - WAZAPRI_MIN;
			}
			else
			{
				wazaPri = 0;
			}
			// すばやさ
			agility = scEvent_CalcAgility( wk, bpp );

			BTL_Printf("行動プライオリティ決定！ Client{%d-%d}'s actionPri=%d, wazaPri=%d, agility=%d\n",
					i, j, actionPri, wazaPri, agility );

			// プライオリティ値とクライアントIDを対にして配列に保存
			pri[p] = MakePriValue( actionPri, wazaPri, agility );
			order[p].clientID = i;
			order[p].pokeIdx = j;
			++p;
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
static u8 countAlivePokemon( BTL_SVFLOW_WORK* wk )
{
	SVCL_WORK* clwk;
	const BTL_POKEPARAM* bpp;
	u16 i, j;
	u8 cnt = 0;

	for(i=0; i<wk->numClient; ++i)
	{
		clwk = BTL_SERVER_GetClientWork( wk->server, i );
		for(j=0; j<clwk->numCoverPos; ++j)
		{
			bpp = BTL_SVCL_GetFrontPokeData( clwk, j );
			if( bpp != NULL )
			{
				if( !BTL_POKEPARAM_IsDead(bpp) )
				{
					cnt++;
				}
			}
		}
	}
	return cnt;
}

//----------------------------------------------------------------------------------------------
// サーバーフローフラグ処理
//----------------------------------------------------------------------------------------------
static inline void FlowFlg_Set( BTL_SVFLOW_WORK* wk, FlowFlag flg )
{
	wk->flowFlags[ flg ] = 1;
}
static inline void FlowFlg_Clear( BTL_SVFLOW_WORK* wk, FlowFlag flg )
{
	wk->flowFlags[ flg ] = 0;
}
static inline BOOL FlowFlg_Get( BTL_SVFLOW_WORK* wk, FlowFlag flg )
{
	return wk->flowFlags[ flg ];
}
static inline void FlowFlg_ClearAll( BTL_SVFLOW_WORK* wk )
{
	int i;
	for(i=0; i<NELEMS(wk->flowFlags); ++i)
	{
		wk->flowFlags[ i ] = 0;
	}
}
//----------------------------------------------------------------------------------------------
// 戦闘に出ているポケモンデータ順番アクセス処理
//----------------------------------------------------------------------------------------------

/**
 *	順番アクセス：ワーク初期化
 */
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk )
{
	fpsw->clientIdx = 0;
	fpsw->pokeIdx = 0;
	fpsw->endFlag = TRUE;

	{
		SVCL_WORK* cw;
		u8 i, j;

		for(i=0; i<BTL_CLIENT_MAX; ++i)
		{
			cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, i );
			if( cw == NULL ){ continue; }

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
/**
 *	順番アクセス：次のポケモンデータを取得（FALSEが返ったら終了）
 */
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk, BTL_POKEPARAM** bpp )
{
	if( fpsw->endFlag )
	{
		return FALSE;
	}
	else
	{
		BTL_POKEPARAM* nextPoke = NULL;
		SVCL_WORK* cw = BTL_SERVER_GetClientWork( wk->server, fpsw->clientIdx );

		*bpp = cw->frontMember[ fpsw->pokeIdx ];
		fpsw->pokeIdx++;

		while( fpsw->clientIdx < BTL_CLIENT_MAX )
		{
			cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, fpsw->clientIdx );
			if( cw )
			{
				while( fpsw->pokeIdx < cw->numCoverPos )
				{
					nextPoke = cw->frontMember[ fpsw->pokeIdx ];
					if( !BTL_POKEPARAM_IsDead(nextPoke) )
					{
						return TRUE;
					}
					fpsw->pokeIdx++;
				}
			}
			fpsw->clientIdx++;
			fpsw->pokeIdx = 0;
		}
		fpsw->endFlag = TRUE;
		return TRUE;
	}
}
//----------------------------------------------------------------------------------------------
// 対象ポケモン記録ワーク
//----------------------------------------------------------------------------------------------
static inline void TargetPokeRec_Clear( TARGET_POKE_REC* rec )
{
	rec->count = 0;
}
static inline void TargetPokeRec_Add( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp )
{
	if( rec->count < NELEMS(rec->pp) )
	{
		rec->pp[rec->count++] = pp;
	}
	else
	{
		GF_ASSERT(0);
	}
}
static inline void TargetPokeRec_Remove( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp )
{
	u32 i;
	for(i=0; i<rec->count; ++i)
	{
		if( rec->pp[i] == bpp )
		{
			u32 j;
			for(j=i+1; j<rec->count; ++j)
			{
				rec->pp[j-1] = rec->pp[j];
			}
			rec->count--;
			rec->idx--;
			break;
		}
	}
}
static inline BTL_POKEPARAM* TargetPokeRec_GetNext( TARGET_POKE_REC* rec )
{
	if( rec->idx < rec->count )
	{
		return rec->pp[ rec->idx++ ];
	}
	else
	{
		return NULL;
	}
}
static inline void TargetPokeRec_GetStart( TARGET_POKE_REC* rec )
{
	rec->idx = 0;
}
static inline BTL_POKEPARAM* TargetPokeRec_Get( const TARGET_POKE_REC* rec, u32 idx )
{
	if( idx < rec->count )
	{
		return (BTL_POKEPARAM*)(rec->pp[ idx ]);
	}
	return NULL;
}
// 指定ポケモンと同チームのポケモン位置インデックスを返す（見つからなければ-1）
static inline int TargetPokeRec_SeekFriendIdx( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, u32 start_idx )
{
	u8 pokeID1, pokeID2, i;

	pokeID1 = BTL_POKEPARAM_GetID( pp );
	for(i=start_idx; i<rec->count; ++i)
	{
		pokeID2 = BTL_POKEPARAM_GetID( rec->pp[i] );
		if( BTL_MAINUTIL_IsFriendPokeID(pokeID1, pokeID2) )
		{
			return i;
		}
	}
	return -1;
}
// ポケモン総数を返す
static inline u32 TargetPokeRec_GetCount( const TARGET_POKE_REC* rec )
{
	return rec->count;
}
// 指定ポケモンと同チームのポケモンのみを別ワークにコピー
static inline u32 TargetPokeRec_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst )
{
	BTL_POKEPARAM* bpp;
	u32 max, i;
	u8 ID1, ID2;

	TargetPokeRec_Clear( dst );
	ID1 = BTL_POKEPARAM_GetID( pp );

	max = TargetPokeRec_GetCount( rec );
	for(i=0; i<max; ++i)
	{
		bpp = TargetPokeRec_Get( rec, i );
		ID2 = BTL_POKEPARAM_GetID( bpp );
		if( BTL_MAINUTIL_IsFriendPokeID(ID1, ID2) )
		{
			TargetPokeRec_Add( dst, bpp );
		}
	}
	return TargetPokeRec_GetCount( dst );
}
// 指定ポケモンと敵チームのポケモンのみを別ワークにコピー
static inline u32 TargetPokeRec_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst )
{
	BTL_POKEPARAM* bpp;
	u32 max, i;
	u8 ID1, ID2;

	TargetPokeRec_Clear( dst );
	ID1 = BTL_POKEPARAM_GetID( pp );

	max = TargetPokeRec_GetCount( rec );
	for(i=0; i<max; ++i)
	{
		bpp = TargetPokeRec_Get( rec, i );
		ID2 = BTL_POKEPARAM_GetID( bpp );
		if( !BTL_MAINUTIL_IsFriendPokeID(ID1, ID2) )
		{
			TargetPokeRec_Add( dst, bpp );
		}
	}
	return TargetPokeRec_GetCount( dst );
}


//======================================================================================================
// サーバーフロー処理
//======================================================================================================

//-----------------------------------------------------------------------------------
// サーバーフロー：フロントメンバー入れ替え
//-----------------------------------------------------------------------------------
static void scput_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx )
{
	SVCL_WORK* clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
	clwk->frontMember[ posIdx ] = clwk->member[ nextPokeIdx ];
	scEvent_MemberIn( wk, clientID, posIdx, nextPokeIdx );
}

static void scput_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx )
{
	scEvent_MemberOut( wk, clientID, posIdx );
}

//-----------------------------------------------------------------------------------
// サーバーフロー：「たたかう」ルート
//-----------------------------------------------------------------------------------
static void scput_Fight( BTL_SVFLOW_WORK* wk, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action )
{
	SVCL_WORK *atClient;
	BTL_POKEPARAM  *attacker;
	WazaID  waza;
	u8 attacker_pokeID;

	atClient = BTL_SERVER_GetClientWork( wk->server, attackClientID );
	attacker = atClient->frontMember[posIdx];
	attacker_pokeID = BTL_POKEPARAM_GetID( attacker );

	waza = BTL_POKEPARAM_GetWazaNumber( attacker, action->fight.wazaIdx );

// こんらん自爆判定
	if( scEvent_CheckConf( wk, attacker ) )
	{
		u16 conf_dmg = scEvent_CalcConfDamage( wk, attacker );

		BTL_POKEPARAM_HpMinus( attacker, conf_dmg );
		SCQUE_PUT_OP_HpMinus( wk->que, atClient->myID, conf_dmg );
		SCQUE_PUT_ACT_ConfDamage( wk->que, BTL_POKEPARAM_GetID(attacker) );
		svflowsub_check_and_make_dead_command( wk, attacker );
		return;
	}
// ワザ出し成功判定
	if( !scEvent_CheckWazaExecute(wk, attacker, waza) ){
		return;
	}
// ワザを出すことに成功
	{
		WazaCategory  category = WAZADATA_GetCategory( waza );
		BtlPokePos    atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, attacker_pokeID );
		BTL_Printf("出すポケ位置=%d, 出すワザ=%d, カテゴリ=%d\n", atPos, waza, category);

		SCQUE_PUT_MSG_WAZA( wk->que, attacker_pokeID, action->fight.wazaIdx );

		// @@@ここに対象による無効化チェックを入れるかも…
		TargetPokeRec_Clear( &wk->damagedPokemon );
		TargetPokeRec_Clear( &wk->targetPokemon );

		flowsub_registerWazaTargets( wk, atPos, waza, action, &wk->targetPokemon );
		flowsub_checkNotEffect( wk, waza, attacker, &wk->targetPokemon );

		switch( category ){
		case WAZADATA_CATEGORY_SIMPLE_DAMAGE:
		case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
			scput_Fight_Damage( wk, waza, atClient, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_DAMAGE_EFFECT:
			scput_Fight_Damage( wk, waza, atClient, attacker, &wk->targetPokemon );
			scput_Fight_AddEffect_Target( wk, waza, attacker );
			break;
		case WAZADATA_CATEGORY_DAMAGE_SICK:
			scput_Fight_Damage( wk, waza, atClient, attacker, &wk->targetPokemon );
			scput_Fight_AddSick( wk, waza, attacker );
			break;
		case WAZADATA_CATEGORY_SIMPLE_EFFECT:
			scput_Fight_SimpleEffect( wk, waza, attacker, atPos, action );
			break;
		case WAZADATA_CATEGORY_SIMPLE_SICK:
			scput_Fight_SimpleSick( wk, waza, attacker, atPos, action );
			break;
		case WAZADATA_CATEGORY_ICHIGEKI:
			scput_Fight_Ichigeki( wk, waza, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_WEATHER:
			scput_Fight_Weather( wk, waza, attacker, action );
			break;
//	case WAZADATA_CATEGORY_BIND:
//	case WAZADATA_CATEGORY_GUARD:
//	case WAZADATA_CATEGORY_FIELD_EFFECT:
//	case WAZADATA_CATEGORY_SIDE_EFFECT:
		case WAZADATA_CATEGORY_OTHERS:
			scput_Fight_Others( wk, waza, attacker, &wk->targetPokemon );
			break;
		default:
//		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaAvoid );
			SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
			break;
		}
		scEvent_decrementPP( wk, attacker, action->fight.wazaIdx, &wk->damagedPokemon );
	}
}
// ワザだし失敗メッセージ表示コマンドをセット
static void cof_put_wazafail_msg_cmd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, u8 wazaFailReason )
{
	u8 pokeID = BTL_POKEPARAM_GetID( attacker );

	switch( wazaFailReason ){
	case SV_WAZAFAIL_NEMURI:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NemuriAct, pokeID );
		break;

	case SV_WAZAFAIL_MAHI:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_MahiAct, pokeID );
		break;

	case SV_WAZAFAIL_KOORI:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriAct, pokeID );
		break;

	case SV_WAZAFAIL_SHRINK:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_ShrinkExe, pokeID );
		break;

	default:
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail, pokeID );
		break;
	}
}

//--------------------------------------------------------------------------
/**
 * ワザ対象ポケモンのデータポインタを、TARGET_POKE_REC構造体にセット
 *
 * @param   wk		
 * @param   atPos			
 * @param   waza			
 * @param   action		
 * @param   rec				[out]
 *	
 * @retval  u8				対象ポケモン数
 */
//--------------------------------------------------------------------------
static u8 flowsub_registerWazaTargets( BTL_SVFLOW_WORK* wk, BtlPokePos atPos, WazaID waza, const BTL_ACTION_PARAM* action, TARGET_POKE_REC* rec )
{
	WazaTarget  targetType = WAZADATA_GetTarget( waza );
	BTL_POKEPARAM* bpp;

	TargetPokeRec_Clear( rec );

	// シングル
	if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE )
	{
		switch( targetType ){
		case WAZA_TARGET_SINGLE:				///< 自分以外の１体（選択）
		case WAZA_TARGET_SINGLE_ENEMY:	///< 敵１体（選択）
		case WAZA_TARGET_RANDOM:				///< 敵ランダム
		case WAZA_TARGET_ENEMY2:				///< 敵側２体
		case WAZA_TARGET_OTHER_ALL:			///< 自分以外全部
			bpp = svflowsub_get_opponent_pokeparam( wk, atPos, 0 );
			TargetPokeRec_Add( rec, bpp );
			break;

		case WAZA_TARGET_ONLY_USER:			///< 自分１体のみ
		case WAZA_TARGET_SINGLE_FRIEND:	///< 自分を含む味方１体
			bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, atPos );
			TargetPokeRec_Add( rec, bpp );
			break;

		default:
			return 0;
		}
		return 1;
	}
	// ダブル
	else
	{
		switch( targetType ){
		case WAZA_TARGET_SINGLE:				///< 自分以外の１体（選択）
			bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, action->fight.targetPos );
			TargetPokeRec_Add( rec, bpp );
			return 1;
		case WAZA_TARGET_SINGLE_ENEMY:	///< 敵１体（選択）
			bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, action->fight.targetPos );
			TargetPokeRec_Add( rec, bpp );
			return 1;
		case WAZA_TARGET_RANDOM:				///< 敵ランダム
			bpp = svflowsub_get_opponent_pokeparam( wk, atPos, GFL_STD_MtRand(1) );
			TargetPokeRec_Add( rec, bpp );
			return 1;

		case WAZA_TARGET_ENEMY2:				///< 敵側２体
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam(wk, atPos, 0) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam(wk, atPos, 1) );
			return 2;

		case WAZA_TARGET_OTHER_ALL:			///< 自分以外全部
			TargetPokeRec_Add( rec, svflowsub_get_next_pokeparam( wk, atPos ) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam( wk, atPos, 0 ) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam( wk, atPos, 1 ) );
			return 3;

		case WAZA_TARGET_ONLY_USER:			///< 自分１体のみ
			TargetPokeRec_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, atPos) );
			return 1;
		case WAZA_TARGET_SINGLE_FRIEND:	///< 自分を含む味方１体（選択）
			TargetPokeRec_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, action->fight.targetPos) );
			return 1;
		case WAZA_TARGET_OTHER_FRIEND:	///< 自分以外の味方１体
			TargetPokeRec_Add( rec, svflowsub_get_next_pokeparam( wk, atPos ) );
			return 1;
		default:
			return 0;
		}
	}
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定位置から見て対戦相手のポケモンデータを返す
//----------------------------------------------------------------------
static BTL_POKEPARAM* svflowsub_get_opponent_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos, u8 pokeSideIdx )
{
	SVCL_WORK* clwk;
	BtlPokePos targetPos;
	u8 clientID, pokeIdx;

	targetPos = BTL_MAIN_GetOpponentPokePos( wk->mainModule, pos, pokeSideIdx );
	BTL_MAIN_BtlPosToClientID_and_PokeIdx( wk->mainModule, targetPos, &clientID, &pokeIdx );

	clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

	return clwk->frontMember[ pokeIdx ];
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定位置から見て隣のポケモンデータを返す
//----------------------------------------------------------------------
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos )
{
	SVCL_WORK* clwk;
	BtlPokePos nextPos;
	u8 clientID, pokeIdx;

	nextPos = BTL_MAIN_GetNextPokePos( wk->mainModule, pos );
	BTL_MAIN_BtlPosToClientID_and_PokeIdx( wk->mainModule, nextPos, &clientID, &pokeIdx );
	clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

	return clwk->frontMember[ pokeIdx ];
}


//--------------------------------------------------------------------------
/**
 * ワザ発動前の、対象別無効化チェック
 *
 * @param   wk				
 * @param   attacker		[in] ワザを出すポケモン
 * @param   targets			[io] ワザ対象ポケモンコンテナ／無効になったポケモンはコンテナから削除される
 *
 */
//--------------------------------------------------------------------------
static void flowsub_checkNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	BTL_POKEPARAM* bpp;

	TargetPokeRec_GetStart( targets );

	// タイプ相性による無効化チェック
	while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
	{
		if( scEvent_CheckNotEffect_byType(wk, waza, attacker, bpp) )
		{
			TargetPokeRec_Remove( targets, bpp );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BTL_POKEPARAM_GetID(bpp) );
		}
	}

	// 攻撃ポケが必中状態なら、タイプ相性以外の無効化チェックはスキップする
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_MUSTHIT) )
	{
		TargetPokeRec_GetStart( targets );
		while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
		{
			if( scEvent_CheckNotEffect(wk, waza, attacker, bpp) )
			{
				TargetPokeRec_Remove( targets, bpp );
				SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BTL_POKEPARAM_GetID(bpp) );
			}
		}
	}
}
//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系
//----------------------------------------------------------------------
static void scput_Fight_Damage( BTL_SVFLOW_WORK* wk, WazaID waza, SVCL_WORK* atClient,
	 BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	u8 target_cnt;
	fx32 dmg_ratio;

	FlowFlg_Clear( wk, FLOWFLG_SET_WAZAEFFECT );

	target_cnt = TargetPokeRec_GetCount( targets );
	dmg_ratio = (target_cnt == 1)? BTL_CALC_DMG_TARGET_RATIO_NONE : BTL_CALC_DMG_TARGET_RATIO_PLURAL;

// 味方に対するダメージ
	if( TargetPokeRec_CopyFriends(targets, attacker, &wk->targetSubPokemon) )
	{
		scput_Fight_Damage_side( wk, waza, attacker, &wk->targetSubPokemon, dmg_ratio );
	}
// 敵に対するダメージ
	if( TargetPokeRec_CopyEnemys(targets, attacker, &wk->targetSubPokemon) )
	{
		scput_Fight_Damage_side( wk, waza, attacker, &wk->targetSubPokemon, dmg_ratio );
	}

	scput_Fight_Damage_After( wk, waza, attacker, &wk->damagedPokemon );
}
static void scput_Fight_Damage_side( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker,
	TARGET_POKE_REC* targets, fx32 dmg_ratio )
{
	BTL_POKEPARAM* bpp;
	u8 poke_cnt;

	// 命中チェック -> 命中しなかった場合は targets から除外
	TargetPokeRec_GetStart( targets );
	while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
	{
		if( !svflowsub_hitcheck_singular(wk, attacker, bpp, waza) )
		{
			TargetPokeRec_Remove( targets, bpp );
		}
	}
	// 回復チェック -> 特定タイプのダメージを無効化、回復してしまうポケを targets から除外
	TargetPokeRec_GetStart( targets );
	while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
	{
		if( scEvent_DmgToRecover(wk, bpp, waza) )
		{
			TargetPokeRec_Remove( targets, bpp );
		}
	}

	TargetPokeRec_GetStart( targets );
	poke_cnt = TargetPokeRec_GetCount( targets );
	if( poke_cnt == 1 )
	{
		bpp = TargetPokeRec_GetNext( targets );
		svflowsub_damage_singular( wk, attacker, bpp, waza, dmg_ratio );
	}
	else if( poke_cnt > 1 )
	{
		u8 aff[ BTL_POSIDX_MAX ];
		u8 critical[ BTL_POSIDX_MAX ];
		u8 f_same_aff;
		PokeType waza_type;
		u32 i, j;

		f_same_aff = TRUE;
		waza_type = WAZADATA_GetType( waza );
		for(i=0; i<poke_cnt; ++i)
		{
			bpp = TargetPokeRec_Get( targets, i );
			aff[i] = scEvent_checkAffinity( wk, bpp, waza_type );
			for(j=0; j<i; ++j){
				if( BTL_CALC_TypeAffAbout(aff[i]) != BTL_CALC_TypeAffAbout(aff[j]) ){
					f_same_aff = FALSE;
				}
			}
		}
		// 相性が１体でも別のヤツがいるなら個別の処理
		if( !f_same_aff )
		{
			for(i=0; i<poke_cnt; ++i){
				bpp = TargetPokeRec_Get( targets, i );
				svflowsub_damage_singular( wk, attacker, bpp, waza, dmg_ratio );
			}
		}
		// 相性一致なら、体力バーを同時に減らすようにコマンド生成する
		else
		{
			svflowsub_damage_plural( wk, attacker, targets, aff, waza, dmg_ratio );
		}
	}
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定ポケモンが死んでたら死亡処理コマンド生成
//----------------------------------------------------------------------
static void svflowsub_check_and_make_dead_command( BTL_SVFLOW_WORK* server, BTL_POKEPARAM* poke )
{
	if( !BTL_POKEPARAM_GetContFlag(poke, BPP_CONTFLG_DEAD_IGNORE) )
	{
		if( BTL_POKEPARAM_IsDead(poke) )
		{
			u8 pokeID = BTL_POKEPARAM_GetID( poke );

			BTL_HANDLER_TOKUSEI_Remove( poke );

			BTL_POKEPARAM_SetContFlag( poke, BPP_CONTFLG_DEAD_IGNORE );
			BTL_POKEPARAM_SetTurnFlag( poke, BPP_TURNFLG_DEAD );

			SCQUE_PUT_MSG_SET( server->que, BTL_STRID_SET_Dead, pokeID );
			SCQUE_PUT_ACT_Dead( server->que, pokeID );
		}
	}
}
//------------------------------------------------------------------
// サーバーフロー下請け：単体ダメージ処理（上位分岐）
//------------------------------------------------------------------
static void svflowsub_damage_singular( BTL_SVFLOW_WORK* wk,
			BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, fx32 targetDmgRatio )
{
	BtlTypeAff	aff;
	PokeType		waza_type;

	waza_type = scEvent_getWazaPokeType( wk, attacker, waza );
	aff = scEvent_checkAffinity( wk, defender, waza_type );

	svflowsub_damage_act_singular( wk, attacker, defender, waza, aff, targetDmgRatio );

	svflowsub_check_and_make_dead_command( wk, defender );
	svflowsub_check_and_make_dead_command( wk, attacker );
}
//------------------------------------------------------------------
// サーバーフロー下請け：単体ダメージ処理（下位）
//------------------------------------------------------------------
static void svflowsub_damage_act_singular(  BTL_SVFLOW_WORK* wk,
		BTL_POKEPARAM* attacker,  BTL_POKEPARAM* defender,
		WazaID waza,   BtlTypeAff aff,   fx32 targetDmgRatio )
{
	u8 plural_flag = FALSE;	// 複数回ヒットフラグ
	u8 deadFlag = FALSE;
	u8 atPokeID, defPokeID;
	u8 hit_count, critical_flag;
	u32 damage;
	int i;

	plural_flag = scEvent_CheckPluralHit( wk, attacker, waza, &hit_count );
	BTL_Printf("念のため表示 : ヒット回数=%d\n", hit_count);

	atPokeID = BTL_POKEPARAM_GetID( attacker );
	defPokeID = BTL_POKEPARAM_GetID( defender );

	for(i=0; i<hit_count; ++i)
	{
		critical_flag = scEvent_CheckCritical( wk, attacker, defender, waza );
		damage = svflowsub_damage_calc_core( wk, attacker, defender, waza, aff, targetDmgRatio, critical_flag );

		// デバッグを簡単にするため必ず大ダメージにする措置
		#ifdef PM_DEBUG
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
		{
			u8 pokeID = BTL_POKEPARAM_GetID(defender);
			if( pokeID >= BTL_PARTY_MEMBER_MAX )
			{
				damage = 999;
			}
		}
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
		{
			u8 pokeID = BTL_POKEPARAM_GetID(defender);
			if( pokeID < BTL_PARTY_MEMBER_MAX )
			{
				damage = 999;
			}
		}
		#endif

		BTL_POKEPARAM_HpMinus( defender, damage );
		SCQUE_PUT_OP_HpMinus( wk->que, defPokeID, damage );
		svflowsub_set_waza_effect( wk, atPokeID, defPokeID, waza );
		scEvent_WazaDamageEffect_Single( wk, attacker, defender, waza, aff, damage, critical_flag );
		svflowsub_reaction_damage( wk, attacker, waza, damage );

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

	TargetPokeRec_Add( &wk->damagedPokemon, defender );

	if( plural_flag )
	{
		// @@@ 「●●回あたった！」メッセージ
//			SCQUE_PUT_MSG_WazaHitCount( wk->que, i );	// @@@ あとで
	}
}
static void svflowsub_damage_plural( BTL_SVFLOW_WORK* wk, 
		BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets, const u8* aff, WazaID waza, fx32 dmg_ratio )
{
	BTL_POKEPARAM* bpp;
	u32 dmg;
	u8 critical_flg[ BTL_POSIDX_MAX ];
	u8 i, poke_cnt;

	poke_cnt = TargetPokeRec_GetCount( targets );
	GF_ASSERT(poke_cnt < BTL_POSIDX_MAX);

	for(i=0; i<poke_cnt; ++i)
	{
		bpp = TargetPokeRec_Get( targets, i );
		critical_flg[i] = scEvent_CheckCritical( wk, attacker, bpp, waza );
		dmg = svflowsub_damage_calc_core( wk, attacker, bpp, waza, aff[i], dmg_ratio, critical_flg[i] );
		BTL_POKEPARAM_HpMinus( bpp, dmg );
		SCQUE_PUT_OP_HpMinus( wk->que, BTL_POKEPARAM_GetID(bpp), dmg );
		TargetPokeRec_Add( &wk->damagedPokemon, bpp );
	}

	svflowsub_set_waza_effect( wk, BTL_POKEPARAM_GetID(attacker), BTL_POKEPARAM_GetID(bpp), waza );

	SCQUE_PUT_ACT_WazaDamagePlural( wk->que, poke_cnt, aff[0] );
	for(i=0; i<poke_cnt; ++i)
	{
		bpp = TargetPokeRec_Get( targets, i );
		SCQUE_PUT_ArgOnly( wk->que, BTL_POKEPARAM_GetID(bpp) );
	}
	for(i=0; i<poke_cnt; ++i)
	{
		if( critical_flg[i] ){
			bpp = TargetPokeRec_Get( targets, i );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_CriticalHit, BTL_POKEPARAM_GetID(bpp) );
		}
	}

	for(i=0; i<poke_cnt; ++i)
	{
		bpp = TargetPokeRec_Get( targets, i );
		svflowsub_check_and_make_dead_command( wk, bpp );
	}
	svflowsub_check_and_make_dead_command( wk, attacker );
}
//------------------------------------------------------------------
// サーバーフロー：ダメージ受け後の処理
//------------------------------------------------------------------
static void scput_Fight_Damage_After( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	scput_Fight_Damage_After_Shrink( wk, waza, attacker, targets );
}
//------------------------------------------------------------------
// サーバーフロー：ダメージ受け後の処理 > ひるみチェック
//------------------------------------------------------------------
static void scput_Fight_Damage_After_Shrink( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	u32 per = WAZADATA_GetShrinkPer( waza );
	if( per )
	{
		BTL_POKEPARAM* bpp;
		u32 cnt, i=0;

		while( (bpp = TargetPokeRec_Get(targets, i++)) != NULL )
		{
			if( !BTL_POKEPARAM_GetTurnFlag(bpp, BPP_TURNFLG_ACTION_EXE) )
			{
				scEvent_AddShrink( wk, bpp, attacker, per );
			}
		}
	}
}
//------------------------------------------------------------------
// サーバーフロー下請け：１体分攻撃ヒットチェック
//------------------------------------------------------------------
static BOOL svflowsub_hitcheck_singular( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza )
{
	u8 defPokeID = BTL_POKEPARAM_GetID( defender );

	// 既に対象が死んでいる
	if( BTL_POKEPARAM_IsDead(defender) )
	{
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_WazaAvoid, defPokeID );
		return FALSE;
	}
	// 命中率・回避率によるヒットチェックで外れた
	if( !scEvent_checkHit(wk, attacker, defender, waza) )
	{
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_WazaAvoid, defPokeID );
		return FALSE;
	}
	return TRUE;
}
//------------------------------------------------------------------
// サーバーフロー下請け： たたかう > ダメージワザ系 > ダメージ値計算
//------------------------------------------------------------------
static u16 svflowsub_damage_calc_core( BTL_SVFLOW_WORK* wk, 
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza,
		BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag )
{
	u32 rawDamage = 0;
	PokeType  waza_type = scEvent_getWazaPokeType( wk, attacker, waza );

	BTL_EVENTVAR_Push();
	BTL_EVENTVAR_SetValue( BTL_EVAR_TYPEAFF, typeAff );
	BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
	BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
	BTL_EVENTVAR_SetValue( BTL_EVAR_CRITICAL_FLAG, criticalFlag );

	{
		u16 atkPower, defGuard, wazaPower;

		// 各種パラメータから素のダメージ値計算
		wazaPower = scEvent_getWazaPower( wk, attacker, defender, waza );
		atkPower  = scEvent_getAttackPower( wk, attacker, defender, waza, criticalFlag );
		defGuard  = scEvent_getDefenderGuard( wk, attacker, defender, waza, criticalFlag );
		{
			u8 atkLevel = BTL_POKEPARAM_GetValue( attacker, BPP_LEVEL );
			rawDamage = wazaPower * atkPower * (atkLevel*2/5+2) / defGuard / 50;
		}
		rawDamage = (rawDamage * targetDmgRatio) >> FX32_SHIFT;
		// 天候補正
		{
			fx32 weatherDmgRatio = BTL_FIELD_GetWeatherDmgRatio( waza );
			if( weatherDmgRatio != BTL_CALC_DMG_WEATHER_RATIO_NONE )
			{
				u32 prevDmg = rawDamage;
				rawDamage = (rawDamage * weatherDmgRatio) >> FX32_SHIFT;
				BTL_Printf("天候による補正が発生, 補正率=%08x, dmg=%d->%d\n", weatherDmgRatio, prevDmg, rawDamage);
			}
		}
		// クリティカルで２倍
		if( criticalFlag )
		{
			rawDamage *= 2;
		}
		BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, rawDamage );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_DAMAGE_PROC1 );
		rawDamage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
		BTL_Printf("威力:%d, こうげき:%d, ぼうぎょ:%d,  ... 素ダメ:%d\n",
				wazaPower, atkPower, defGuard, rawDamage );
	}
	//ランダム補正
	{
		u16 ratio = 100 - GFL_STD_MtRand(16);
		rawDamage = (rawDamage * ratio) / 100;
		BTL_Printf("ランダム補正:%d%%  -> 素ダメ=%d\n", ratio, rawDamage);
	}
	// タイプ一致補正
	{
		fx32 ratio = scEvent_CalcTypeMatchRatio( wk, attacker, waza_type );
		rawDamage = (rawDamage * ratio) >> FX32_SHIFT;
	}
	// タイプ相性計算
	rawDamage = BTL_CALC_AffDamage( rawDamage, typeAff );
	BTL_Printf("タイプ相性:%02d -> ダメージ値：%d\n", typeAff, rawDamage);
	// やけど補正
	if(	(WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC)
	&&	(BTL_POKEPARAM_GetPokeSick(attacker) == POKESICK_YAKEDO)
	){
		rawDamage = (rawDamage * BTL_YAKEDO_DAMAGE_RATIO) / 100;
	}

	BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, rawDamage );
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_PROC2 );
	rawDamage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
	// 最低、１はダメージを与える
	if( rawDamage == 0 ){ rawDamage = 1; }

	BTL_EVENTVAR_Pop();
	return rawDamage;
}
//---------------------------------------------------------------------------------------------
// ワザダメージ処理後の反動処理
//---------------------------------------------------------------------------------------------
static void svflowsub_reaction_damage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 damage )
{
	u8 ratio = WAZADATA_GetReactionRatio( waza );
	if( ratio )
	{
		u8 pokeID = BTL_POKEPARAM_GetID( attacker );
		BTL_EVENTVAR_Push();
			BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, ratio );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, pokeID );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_REACTION );
		BTL_EVENTVAR_Pop();
		if( ratio )
		{
			damage = roundMin( (damage*ratio)/100, 1 );
			BTL_POKEPARAM_HpMinus( attacker, damage );
			SCQUE_PUT_OP_HpMinus( wk->que, pokeID, damage );
			SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_ReactionDmg, pokeID );
		}
	}
}
//---------------------------------------------------------------------------------------------
// １度のワザ処理にて、エフェクト発動コマンドを１回しか作成しないための仕組み
//---------------------------------------------------------------------------------------------
static void svflowsub_set_waza_effect( BTL_SVFLOW_WORK* wk, u8 atPokeID, u8 defPokeID, WazaID waza )
{
	if( FlowFlg_Get(wk, FLOWFLG_SET_WAZAEFFECT ) == FALSE )
	{
		BTL_Printf("ワザエフェクトコマンド生成しますよ, ワザナンバ%d\n", waza);
		SCQUE_PUT_ACT_WazaEffect( wk->que, atPokeID, defPokeID, waza );
		FlowFlg_Set( wk, FLOWFLG_SET_WAZAEFFECT );
	}
	else
	{
		BTL_Printf("ワザエフェクトコマンド生成済みなので無視, ワザナンバ%d\n", waza);
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：追加効果による状態異常
//---------------------------------------------------------------------------------------------
static void scput_Fight_AddSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker )
{
	BTL_POKEPARAM* receiver;
	u32 i=0;

	while( 1 )
	{
		receiver = TargetPokeRec_Get( &wk->damagedPokemon, i );
		if( receiver == NULL ){ break; }
		if( !BTL_POKEPARAM_IsDead(receiver) )
		{
			scEvent_CheckAddSick( wk, receiver, attacker, waza );
		}
		++i;
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接の状態異常
//---------------------------------------------------------------------------------------------
static void scput_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
	BTL_POKEPARAM* target;
	WAZA_SICKCONT_PARAM waza_contParam;
	WazaSick sick;
	u32 per, i = 0;

	flowsub_registerWazaTargets( wk, atPos, waza, action, &wk->targetPokemon );

	per = WAZADATA_GetHitRatio( waza );
	sick = WAZADATA_GetSick( waza, &waza_contParam );
	while( 1 )
	{
		target = TargetPokeRec_Get( &wk->targetPokemon, i++ );
		if( target == NULL ){ break; }
		if( svflowsub_hitcheck_singular(wk, attacker, target, waza) )
		{
			u8 atpokeID = BTL_POKEPARAM_GetID( attacker );
			u8 defpokeID = BTL_POKEPARAM_GetID( target );
			BPP_SICK_CONT cont;
			svflowsub_set_waza_effect( wk, atpokeID, defpokeID, waza );
			BTL_CALC_WazaSickContToBppSickCont( waza_contParam, attacker, &cont );
			svflowsub_MakeSick( wk, target, attacker, sick, cont, TRUE );
		}
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：追加効果によるランク効果
//---------------------------------------------------------------------------------------------
static void scput_Fight_AddEffect_Target( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker )
{
	BTL_POKEPARAM* receiver;
	u32 i=0;

	while( 1 )
	{
		receiver = TargetPokeRec_Get( &wk->damagedPokemon, i );
		if( receiver == NULL ){ break; }
		if( !BTL_POKEPARAM_IsDead(receiver) )
		{
			scEvent_CheckAddEffect_Target( wk, receiver, attacker, waza );
		}
		++i;
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接のランク効果
//---------------------------------------------------------------------------------------------
static void scput_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_ACTION_PARAM* action )
{
	BTL_POKEPARAM* target;
	WazaSick sick;
	u32 per, i = 0;

	flowsub_registerWazaTargets( wk, atPos, waza, action, &wk->targetPokemon );
	per = WAZADATA_GetHitRatio( waza );

	while( 1 )
	{
		target = TargetPokeRec_Get( &wk->targetPokemon, i++ );
		if( target == NULL ){ break; }
		if( svflowsub_hitcheck_singular(wk, attacker, target, waza) )
		{
			u8 atpokeID = BTL_POKEPARAM_GetID( attacker );
			u8 defpokeID = BTL_POKEPARAM_GetID( target );
			WazaRankEffect eff;
			u8 eff_cnt, e;
			int volume;

				svflowsub_set_waza_effect( wk, atpokeID, defpokeID, waza );

			eff_cnt = WAZADATA_GetRankEffectCount( waza );
			for(e=0; e<eff_cnt; ++e)
			{
				eff = WAZADATA_GetRankEffect( waza, e, &volume );
				if( volume < 0 )
				{
					scEvent_RankDown( wk, target, eff, -volume, TRUE );
				}
				else
				{
					scEvent_RankUp( wk, target, eff, volume, TRUE );
				}
			}
		}
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：一撃ワザ処理
//---------------------------------------------------------------------------------------------
static void scput_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	BTL_POKEPARAM* target;
	WazaSick sick;
	u32 i = 0;

	while( 1 )
	{
		target = TargetPokeRec_Get( targets, i++ );
		if( target == NULL ){ break; }
		if( !BTL_POKEPARAM_IsDead(target) )
		{
			u8 atkLevel = BTL_POKEPARAM_GetValue( attacker, BPP_LEVEL );
			u8 defLevel = BTL_POKEPARAM_GetValue( target, BPP_LEVEL );
			u8 targetPokeID = BTL_POKEPARAM_GetID( target );
			if( atkLevel < defLevel )
			{
				SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, targetPokeID );
			}
			else
			{
				if( scEvent_IchigekiCheck(wk, attacker, target, waza) )
				{
					svflowsub_set_waza_effect( wk, BTL_POKEPARAM_GetID(attacker), targetPokeID, waza );
					BTL_POKEPARAM_HpZero( target );
					SCQUE_PUT_OP_HpZero( wk->que, targetPokeID );
					SCQUE_PUT_ACT_WazaIchigeki( wk->que, targetPokeID );
				}
				else
				{
					SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_WazaAvoid, targetPokeID );
				}
			}
		}
	}
}

// ポケモン系・ワザ系による状態異常化イベントの呼び分け
static void svflowsub_MakeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
		WazaSick sick, BPP_SICK_CONT contParam, BOOL fAlmost )
{
	if( sick < POKESICK_MAX ){
		scEvent_MakePokeSick( wk, receiver, attacker, sick, contParam, fAlmost );
	}else{
		scEvent_MakeWazaSick( wk, receiver, attacker, sick, contParam, fAlmost );
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる天候の変化
//---------------------------------------------------------------------------------------------
static void scput_Fight_Weather( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, const BTL_ACTION_PARAM* action )
{
	BtlWeather  weather = WAZADATA_GetWeather( waza );
	u8 pokeID = BTL_POKEPARAM_GetID( attacker );

	if( scEvent_ChangeWeather( wk, weather, BTL_WEATHER_TURN_DEFAULT ) )
	{
		svflowsub_set_waza_effect( wk, pokeID, pokeID, waza );
		SCQUE_PUT_ACT_WeatherStart( wk->que, weather );
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：分類しきれないワザ
//---------------------------------------------------------------------------------------------
static void scput_Fight_Others( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
{
	switch( waza ){
	case WAZANO_SUKIRUSUWAPPU:
		scput_Fight_Others_SkillSwap( wk, attacker, targets );
		break;
	default:
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
		break;
	}
}
// スキルスワップ
static void scput_Fight_Others_SkillSwap( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec )
{
	BTL_POKEPARAM* target = TargetPokeRec_Get( targetRec, 0 );
	PokeTokusei atk_tok, tgt_tok;

	atk_tok = BTL_POKEPARAM_GetValue( attacker, BPP_TOKUSEI );
	tgt_tok = BTL_POKEPARAM_GetValue( target, BPP_TOKUSEI );

	if( (!BTL_CALC_TOK_CheckCant_Swap(atk_tok)) && (!BTL_CALC_TOK_CheckCant_Swap(tgt_tok)) )
	{
		u8 atkPokeID = BTL_POKEPARAM_GetID( attacker );
		u8 tgtPokeID = BTL_POKEPARAM_GetID( target );
		svflowsub_set_waza_effect( wk, atkPokeID, tgtPokeID, WAZANO_SUKIRUSUWAPPU );
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_SkillSwap, atkPokeID );

		BTL_POKEPARAM_ChangeTokusei( attacker, tgt_tok );
		BTL_POKEPARAM_ChangeTokusei( target, atk_tok );
		BTL_HANDLER_TOKUSEI_Swap( attacker, target );

		BTL_EVENTVAR_Push();
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, atkPokeID );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, tgtPokeID );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_SKILL_SWAP );
		BTL_EVENTVAR_Pop();
	}
	else
	{
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_STD_WazaFail );
	}
}


//==============================================================================
// サーバーフロー：ターンチェックルート
//==============================================================================
static void scput_TurnCheck( BTL_SVFLOW_WORK* wk )
{
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_TURNCHECK_BEGIN );

	scput_turncheck_sick( wk );
	scput_turncheck_weather( wk );

	BTL_EVENT_CallHandlers( wk, BTL_EVENT_TURNCHECK_END );

	// 全ポケモンのターンフラグをクリア
	{
		SVCL_WORK* cw;
		u32 i, j;
		for(i=0; i<BTL_CLIENT_MAX; ++i)
		{
			cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, i );
			if( cw == NULL ){ continue; }
			for(j=0; j<cw->memberCount; ++j)
			{
				BTL_POKEPARAM_ClearTurnFlag( cw->member[j] );
			}
		}
	}

	wk->turnCount++;
}
//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > 状態異常
//------------------------------------------------------------------
static void scput_turncheck_sick( BTL_SVFLOW_WORK* wk )
{
	FRONT_POKE_SEEK_WORK  fpsw;
	BTL_POKEPARAM* bpp;
	WazaSick sick;
	int damage;
	u8 pokeID;

	BTL_EVENTVAR_Push();

	FRONT_POKE_SEEK_InitWork( &fpsw, wk );

	while( FRONT_POKE_SEEK_GetNext( &fpsw, wk, &bpp ) )
	{
		pokeID = BTL_POKEPARAM_GetID( bpp );
		sick = BTL_POKEPARAM_GetPokeSick( bpp );
		damage = BTL_POKEPARAM_CalcSickDamage( bpp );
		if( sick != POKESICK_NULL && (damage!=0) )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );
			BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, damage );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_SICK_DAMAGE );
			damage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
			sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
			if( sick != POKESICK_NULL )
			{
				if( damage > 0 )
				{
					BTL_Printf("ポケ[%d]が状態異常(%d)で、%dのダメージ発生\n", pokeID, sick, damage );
					BTL_POKEPARAM_HpMinus( bpp, damage );
					SCQUE_PUT_OP_HpMinus( wk->que, pokeID, damage );
					SCQUE_PUT_SickDamage( wk->que, pokeID, sick, damage );
				}
				else if( damage < 0 )
				{
					int recover = -damage;
					BTL_POKEPARAM_HpPlus( bpp, recover );
					SCQUE_PUT_OP_HpPlus( wk->que, pokeID, recover );
					SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
				}
			}
		}
		BTL_POKEPARAM_WazaSick_TurnCheck( bpp );
		SCQUE_PUT_OP_WazaSickTurnCheck( wk->que, pokeID );
		svflowsub_check_and_make_dead_command( wk, bpp );
	}

	BTL_EVENTVAR_Pop();
}
//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > 天候
//------------------------------------------------------------------
static void scput_turncheck_weather( BTL_SVFLOW_WORK* wk )
{
	BtlWeather weather = BTL_FIELD_TurnCheckWeather();
	if( weather != BTL_WEATHER_NONE )
	{
		SCQUE_PUT_ACT_WeatherEnd( wk->que, weather );
		return;
	}
	else
	{
		FRONT_POKE_SEEK_WORK  fps;
		u8   pokeID[ BTL_POS_MAX ];
		u8   tok_pokeID[ BTL_POS_MAX ];
		s16  damage[ BTL_POS_MAX ];
		BTL_POKEPARAM* bpp;
		s16 dmg_tmp;
		u8 poke_cnt = 0;
		u8 tok_poke_cnt = 0;
		u8 tok_flg;

		weather = BTL_FIELD_GetWeather();
		FRONT_POKE_SEEK_InitWork( &fps, wk );
		while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
		{
			if( !BTL_POKEPARAM_IsDead(bpp) )
			{
				dmg_tmp = scEvent_CalcWeatherDamage( wk, weather, bpp, &tok_flg );
				if( dmg_tmp )
				{
					if( tok_flg )
					{
						tok_pokeID[ tok_poke_cnt++ ] = BTL_POKEPARAM_GetID( bpp );
					}
					damage[ poke_cnt ] = dmg_tmp;
					pokeID[ poke_cnt ] = BTL_POKEPARAM_GetID( bpp );
					++poke_cnt;
				}
			}
		}
		// ダメージ反映コマンド発行の前後に、とくせいによるケースはウィンドウ表示コマンド
		if( poke_cnt )
		{
			u8 i;
			for(i=0; i<tok_poke_cnt; ++i)
			{
				SCQUE_PUT_TOKWIN_IN( wk->que, tok_pokeID[i] );
			}

			for(i=0; i<poke_cnt; ++i){
				bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID[i] );
				if( damage[i] >= 0 ){
					BTL_POKEPARAM_HpMinus( bpp, damage[i] );
					SCQUE_PUT_OP_HpMinus( wk->que, pokeID[i], damage[i] );
				}else{
					BTL_POKEPARAM_HpPlus( bpp, -damage[i] );
					SCQUE_PUT_OP_HpPlus( wk->que, pokeID[i], -damage[i] );
				}
			}
			SCQUE_PUT_ACT_WeatherDamage( wk->que, weather, poke_cnt );
			for(i=0; i<poke_cnt; ++i)
			{
				SCQUE_PUT_ArgOnly( wk->que, pokeID[i] );
			}

			for(i=0; i<tok_poke_cnt; ++i)
			{
				SCQUE_PUT_TOKWIN_OUT( wk->que, tok_pokeID[i] );
			}
			// 死亡チェック
			for(i=0; i<poke_cnt; ++i)
			{
				svflowsub_check_and_make_dead_command( wk, BTL_POKECON_GetPokeParam(wk->pokeCon, pokeID[i]) );
			}
		}
	}
}
static int scEvent_CalcWeatherDamage( BTL_SVFLOW_WORK* wk, BtlWeather weather, BTL_POKEPARAM* bpp, u8 *tok_cause_flg )
{
	int default_damage = BTL_CALC_RecvWeatherDamage( bpp, weather );
	int damage;
	BTL_Printf( "天候ダメージチェックすれ\n");
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(bpp) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WEATHER, weather );
		BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE,  default_damage );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_WEATHER_DAMAGE );
		damage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
		*tok_cause_flg = ( default_damage != damage );
	BTL_EVENTVAR_Pop();
	return damage;
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
static inline int roundMin( int val, int min )
{
	if( val < min ){ val = min; }
	return val;
}
static inline int roundMax( int val, int max )
{
	if( val > max ){ val = max; }
	return val;
}
static inline BOOL perOccur( u8 per )
{
	return GFL_STD_MtRand(100) < per;
}



//---------------------------------------------------------------------------------------------
// コマンド出力  イベント群
//---------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------
/**
 * [Event] すばやさ取得
 *
 * @param   wk		
 * @param   attacker		
 *
 * @retval  u16		
 */
//--------------------------------------------------------------------------
static u16 scEvent_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
	u16 agi = BTL_POKEPARAM_GetValue( attacker, BPP_AGILITY );
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_AGILITY, agi );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, TRUE );	// まひ無視フラグとして使っている
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_AGILITY );
		agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY );
		if( BTL_POKEPARAM_GetPokeSick(attacker) == POKESICK_MAHI )
		{
			if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) )
			{
				agi = (agi * BTL_MAHI_AGILITY_RATIO) / 100;
			}
		}
	BTL_EVENTVAR_Pop();
	return agi;
}
//--------------------------------------------------------------------------
/**
 * [Event] こんらん自爆判定
 *
 * @param   wk		
 * @param   attacker		
 *
 * @retval  BOOL		自爆したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckConf( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_KONRAN) )
	{
		u8 pokeID = BTL_POKEPARAM_GetID( attacker );
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KonranAct, pokeID );
		{
			BOOL  fexe = ( GFL_STD_MtRand(100) < BTL_CONF_EXE_RATIO );
			return fexe;
		}
	}
	return FALSE;
}
//--------------------------------------------------------------------------
/**
 * [Event] こんらんダメージ計算
 *
 * @param   wk		
 * @param   attacker		
 *
 * @retval  u16		ダメージ値
 */
//--------------------------------------------------------------------------
static u16 scEvent_CalcConfDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
	u16 dmg = roundMin( BTL_POKEPARAM_GetValue(attacker, BPP_MAX_HP)/8, 1 );

	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_CONF_DMG, dmg );
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_CONF_DAMAGE );
	dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_CONF_DMG );

	BTL_EVENTVAR_Pop();
	return dmg;
}
//--------------------------------------------------------------------------
/**
 * 【Event】ワザ出し失敗判定 ＆ ねむり・まひ・こおりの回復チェック
 *
 * @param   wk				
 * @param   attacker	攻撃ポケモンパラメータ
 * @param   waza			出そうとしているワザ
 *
 * @retval  BOOL			ワザ出し失敗したらFALSE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza )
{
	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_REASON, SV_WAZAFAIL_NULL );
	BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
	do {
		WazaSick sick = BTL_POKEPARAM_GetPokeSick( attacker );
		u8 pokeID = BTL_POKEPARAM_GetID( attacker );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );

		// 状態異常による失敗チェック
		switch( sick ){
		case POKESICK_NEMURI:
			if( !BTL_POKEPARAM_Nemuri_CheckWake(attacker) ){
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_REASON, SV_WAZAFAIL_NEMURI );
			}else{
				BTL_POKEPARAM_CurePokeSick( attacker );
				SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );
				SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NemuriWake, pokeID );
			}
			break;
		case POKESICK_MAHI:
			if( GFL_STD_MtRand(100) < BTL_MAHI_EXE_RATIO )
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_REASON, SV_WAZAFAIL_MAHI );
			}
			break;
		case POKESICK_KOORI:
			if( GFL_STD_MtRand(100) >= BTL_KORI_MELT_RATIO ){
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_REASON, SV_WAZAFAIL_KOORI );
			}else{
				BTL_POKEPARAM_CurePokeSick( attacker );
				SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );
				SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriMelt, pokeID );
			}
			break;
		}
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_REASON) != SV_WAZAFAIL_NULL )
		{
			break;
		}
		// ひるみによる失敗チェック
		if( BTL_POKEPARAM_GetTurnFlag(attacker, BPP_TURNFLG_SHRINK) )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_REASON, SV_WAZAFAIL_SHRINK );
			break;
		}

	}while(0);

	BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXECUTE );
	{
		SV_WazaFailReason reason = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_REASON );
		if( reason != SV_WAZAFAIL_NULL )
		{
			cof_put_wazafail_msg_cmd( wk, attacker, reason );
		}
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXECUTE_FIX );
		BTL_EVENTVAR_Pop();
		return reason == SV_WAZAFAIL_NULL;
	}
}
//--------------------------------------------------------------------------
/**
 * [Event] 出したワザが対象に当たるか判定（一撃必殺以外のポケモン対象ワザ）
 *
 * @param   wk		
 * @param   attacker		
 * @param   defender		
 * @param   waza		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------------------
static BOOL scEvent_checkHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	if( WAZADATA_IsAlwaysHit(waza) )
	{
		return TRUE;
	}
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_MUSTHIT) )
	{
		return TRUE;
	}

	{
		u8 wazaHitRatio, per;
		s8 hitRank, avoidRank, totalRank;

		wazaHitRatio = scEvent_getHitPer(wk, attacker, defender, waza);

		BTL_EVENTVAR_Push();

			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_HIT_RANK, BTL_POKEPARAM_GetValue(attacker, BPP_HIT_RATIO) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_AVOID_RANK, BTL_POKEPARAM_GetValue(defender, BPP_HIT_RATIO) );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_RANK );

			hitRank = BTL_EVENTVAR_GetValue( BTL_EVAR_HIT_RANK );
			avoidRank = BTL_EVENTVAR_GetValue( BTL_EVAR_AVOID_RANK );

		BTL_EVENTVAR_Pop();

		totalRank = roundValue( BTL_CALC_HITRATIO_MID + avoidRank - hitRank, BTL_CALC_HITRATIO_MIN, BTL_CALC_HITRATIO_MAX );
		per = BTL_CALC_HitPer( wazaHitRatio, totalRank );

		return perOccur( per );
	}
}
//--------------------------------------------------------------------------
/**
 * [Event] 出したワザが対象に当たるか判定（一撃必殺ワザ）
 *
 * @param   wk		
 * @param   attacker		
 * @param   defender		
 * @param   waza		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------------------
static BOOL scEvent_IchigekiCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	BOOL ret = FALSE;
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, FALSE );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_ICHIGEKI_CHECK );
		if( !BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_FLAG) )
		{
			// とくせい等による強制無効化を受けない場合、必中有効。
			if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_MUSTHIT) ){
				ret = TRUE;
			}else{
				u8 per = WAZADATA_GetHitRatio( waza );
				u8 atLevel = BTL_POKEPARAM_GetValue( attacker, BPP_LEVEL );
				u8 defLevel = BTL_POKEPARAM_GetValue( defender, BPP_LEVEL );
				if( atLevel > defLevel )
				{
					per += (atLevel - defLevel);
					ret = perOccur( per );
				}
			}
		}
	BTL_EVENTVAR_Pop();
	return ret;
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ無効チェック（タイプ相性による無効化）
 *
 * @param   wk					
 * @param   waza				
 * @param   attacker		
 * @param   defender		
 *
 * @retval  BOOL		無効な場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckNotEffect_byType( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
	PokeType waza_type = WAZADATA_GetType( waza );
	PokeTypePair def_type = BTL_POKEPARAM_GetPokeType( defender );
	BOOL fNoEffect = (BTL_CALC_TypeAff( waza_type, def_type ) == BTL_TYPEAFF_0);

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_NOEFFECT_FLAG, fNoEffect );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_NOEFFECT_TYPE_CHECK );
		fNoEffect = BTL_EVENTVAR_GetValue( BTL_EVAR_NOEFFECT_FLAG );
	BTL_EVENTVAR_Pop();

	return fNoEffect;
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ無効チェック（とくせい等、個別ケースによる無効化）
 *
 * @param   wk					
 * @param   waza				
 * @param   attacker		
 * @param   defender		
 *
 * @retval  BOOL		無効な場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
	BOOL fNoEffect = FALSE;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_NOEFFECT_FLAG, fNoEffect );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_NOEFFECT_CHECK );
		fNoEffect = BTL_EVENTVAR_GetValue( BTL_EVAR_NOEFFECT_FLAG );
	BTL_EVENTVAR_Pop();

	return fNoEffect;
}
//--------------------------------------------------------------------------
/**
 * [Event] 特定タイプのダメージワザで回復してしまうポケの判定
 *
 * @param   wk				
 * @param   defender	
 * @param   waza			
 *
 * @retval  BOOL			
 */
//--------------------------------------------------------------------------
static BOOL scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza )
{
	BOOL ret = FALSE;
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, ret );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_DMG_TO_RECOVER );
		ret = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
	BTL_EVENTVAR_Pop();
	return ret;
}
//--------------------------------------------------------------------------
/**
 * [Event] クリティカルヒット発生チェック
 *
 * @param   wk		
 * @param   attacker		
 * @param   defender		
 * @param   waza		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckCritical( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	BOOL flag = FALSE;
	u16  rank = BTL_POKEPARAM_GetValue( attacker, BPP_CRITICAL_RATIO );
	rank += WAZADATA_GetCriticalRank( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, FALSE );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_CRITICAL_RANK, rank );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CRITICAL_CHECK );
		if( !BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG ) )
		{
			rank = roundMax( BTL_EVENTVAR_GetValue(BTL_EVAR_CRITICAL_RANK), BTL_CALC_CRITICAL_MAX );
			flag = BTL_CALC_CheckCritical( rank );
		}
	BTL_EVENTVAR_Pop();
	return flag;
}
//--------------------------------------------------------------------------
/**
 * [Event] 単体ワザダメージアクション処理
 *
 * @param   wk		
 *
 */
//--------------------------------------------------------------------------
static void scEvent_WazaDamageEffect_Single( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff, u32 damage, BOOL criticalFlag )
{
	u8 defPokeID = BTL_POKEPARAM_GetID( defender );
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, defPokeID );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, criticalFlag );
		SCQUE_PUT_ACT_WazaDamage( wk->que, defPokeID, aff, damage );
		if( criticalFlag )
		{
			SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_CriticalHit );
		}
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_AFTER );
	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] ２体同時ワザダメージアクション処理
 *
 * @param   wk		
 *
 */
//--------------------------------------------------------------------------
static void scEvent_WazaDamageEffect_Double( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, WazaID waza, BtlTypeAff aff,
	u32 dmg1, u32 dmg2 )
{
	u8 defPokeID1 = BTL_POKEPARAM_GetID( def1 );
	u8 defPokeID2 = BTL_POKEPARAM_GetID( def2 );

	SCQUE_PUT_ACT_WazaDamageDbl( wk->que, defPokeID1, defPokeID2, dmg1, dmg2, aff );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, FALSE );

		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, defPokeID1 );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_AFTER );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, defPokeID2 );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_AFTER );

	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザの相性計算
 *
 * @param   wk		
 * @param   defender		
 * @param   waza_type		
 *
 * @retval  BtlTypeAff		
 */
//--------------------------------------------------------------------------
static BtlTypeAff scEvent_checkAffinity( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, PokeType waza_type )
{
	PokeTypePair defPokeType = scEvent_getDefenderPokeType( wk, defender );
	return BTL_CALC_TypeAff( waza_type, defPokeType );;
}
//--------------------------------------------------------------------------
/**
 * [Event] 使ったワザのPPデクリメント
 *
 * @param   wk				
 * @param   attacker	攻撃ポケモンパラメータ
 * @param   wazaIdx		使ったワザインデックス
 * @param   rec				ダメージを受けたポケモンパラメータ群
 *
 */
//--------------------------------------------------------------------------
static void scEvent_decrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, u8 wazaIdx, TARGET_POKE_REC* rec )
{
	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, 1 );
	{
		u8 i = 0;
		BTL_POKEPARAM* bpp;
		BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGED_POKECNT, TargetPokeRec_GetCount(rec) );
		while( (bpp=TargetPokeRec_Get(rec, i)) != NULL )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DAMAGED1+i, BTL_POKEPARAM_GetID(bpp) );
			++i;
		}
	}
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECREMENT_PP );
	{
		u8 volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
		if( volume )
		{
			u8 pokeID = BTL_POKEPARAM_GetID( attacker );
			SCQUE_PUT_OP_PPMinus( wk->que, pokeID, wazaIdx, volume );
			BTL_POKEPARAM_PPMinus( attacker, wazaIdx, volume );
		}
	}
	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ的中率(0-100)取得
 *
 * @param   wk		
 * @param   attacker		
 * @param   defender		
 * @param   waza		
 *
 * @retval  u8		的中率 (0-100)
 */
//--------------------------------------------------------------------------
static u8 scEvent_getHitPer( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	u8 per = WAZADATA_GetHitRatio( waza );

	BTL_EVENTVAR_Push();
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_MIST )
		{
			per = (per * BTL_CALC_WEATHER_MIST_HITRATIO) >> FX32_SHIFT;
		}
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, per );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_RATIO );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
	BTL_EVENTVAR_Pop();

	return per;
}
//--------------------------------------------------------------------------
/**
 * [Event] 複数回ヒットワザのチェック処理
 *
 * @param   wk					
 * @param   attacker		
 * @param   waza		
 * @param   hitCount		[out] 実行するヒット回数（ランダム）
 *
 * @retval  BOOL		複数回ヒットするワザならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckPluralHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u8* hitCount )
{
	u8 max = WAZADATA_GetMaxHitCount( waza );

	if( max > 1 )
	{
		BTL_EVENTVAR_Push();
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_HITCOUNT_MAX, max );
			BTL_EVENTVAR_SetValue( BTL_EVAR_HITCOUNT, BTL_CALC_HitCountMax(max) );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_COUNT );
			*hitCount = BTL_EVENTVAR_GetValue( BTL_EVAR_HITCOUNT );
		BTL_EVENTVAR_Pop();
		return TRUE;
	}
	else
	{
		*hitCount = 1;
	}
	return FALSE;
}

// ワザ威力取得
static u16 scEvent_getWazaPower( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
	u16 power = WAZADATA_GetPower( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_POWER, power );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_POWER );
		power = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
	BTL_EVENTVAR_Pop();

	return power;
}


// 攻撃側能力値取得
static u16 scEvent_getAttackPower( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag )
{
	BppValueID vid;

	switch( WAZADATA_GetDamageType(waza) ){
	case WAZADATA_DMG_PHYSIC:		vid = BPP_ATTACK; break;
	case WAZADATA_DMG_SPECIAL:	vid = BPP_SP_ATTACK; break;
	default:
		GF_ASSERT(0);
		vid = BPP_ATTACK;
		break;
	}

	{
		u16 power;
		BTL_EVENTVAR_Push();
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, FALSE );		// 
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_ATTACKER_POWER_PREV );
			if( BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG ) )
			{
				BTL_POKEPARAM_GetValue_Base( attacker, vid );
			}
			else
			{
				if( criticalFlag ){
					power = BTL_POKEPARAM_GetValue_Critical( attacker, vid );
				}else{
					power = BTL_POKEPARAM_GetValue( attacker, vid );
				}
			}
			BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, power );
			BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_ATTACKER_POWER );
			power = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
		BTL_EVENTVAR_Pop();
		return power;
	}
}

// 防御側能力値取得
static u16 scEvent_getDefenderGuard( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag )
{
	BppValueID vid;

	switch( WAZADATA_GetDamageType(waza) ){
	case WAZADATA_DMG_PHYSIC:		vid = BPP_DEFENCE; break;
	case WAZADATA_DMG_SPECIAL:	vid = BPP_SP_DEFENCE; break;
	default:
		GF_ASSERT(0);
		vid = BPP_DEFENCE;
		break;
	}

	{
		u16 guard;

		BTL_EVENTVAR_Push();
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, FALSE );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_DEFENDER_GUARD_PREV );

			if( BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG) )
			{
				guard = BTL_POKEPARAM_GetValue_Base( defender, vid );
			}
			else
			{
				if( criticalFlag ){
					guard = BTL_POKEPARAM_GetValue_Critical( defender, vid );
				}else{
					guard = BTL_POKEPARAM_GetValue( defender, vid );
				}
			}

			// てんこう「すなあらし」の時、いわタイプのとくぼう1.5倍
			if(	(BTL_FIELD_GetWeather() == BTL_WEATHER_SAND)
			&&	(BTL_POKEPARAM_IsMatchType(defender, POKETYPE_IWA))
			&&	(vid == BPP_SP_DEFENCE)
			){
				guard = (guard * 192) / 128;
			}

			BTL_EVENTVAR_SetValue( BTL_EVAR_GUARD, guard );
			BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_DEFENDER_GUARD );
			guard = BTL_EVENTVAR_GetValue( BTL_EVAR_GUARD );

		BTL_EVENTVAR_Pop();
		return guard;
	}
}
//--------------------------------------------------------------------------
/**
 * [Event] 攻撃ポケとワザのタイプ一致補正率を計算
 *
 * @param   wk		
 * @param   attacker		
 * @param   waza_type		
 *
 * @retval  fx32		補正率
 */
//--------------------------------------------------------------------------
static fx32 scEvent_CalcTypeMatchRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, PokeType waza_type )
{
	BOOL f_match = BTL_POKEPARAM_IsMatchType( attacker, waza_type );
	fx32 ratio = (f_match)? FX32_CONST(1.5f) : FX32_CONST(1);

	BTL_EVENTVAR_Push();
	{
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, ratio );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, f_match );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_TYPEMATCH_RATIO );
		ratio = (fx32)BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
	}
	BTL_EVENTVAR_Pop();
	return ratio;
}
// ワザタイプ取得
static PokeType scEvent_getWazaPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
	PokeType type = WAZADATA_GetType( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_TYPE, WAZADATA_GetType(waza) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_TYPE );
		type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
	BTL_EVENTVAR_Pop();
	return type;
}
// 攻撃側タイプ取得
static PokeTypePair scEvent_getAttackerPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_POKE_TYPE, BTL_POKEPARAM_GetPokeType(attacker) );
	BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(attacker) );
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_ATTACKER_TYPE );

	{
		PokeTypePair type = BTL_EVENTVAR_GetValue( BTL_EVAR_POKE_TYPE );
		BTL_EVENTVAR_Pop();
		return type;
	}
}

// 防御側タイプ取得
static PokeTypePair scEvent_getDefenderPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender )
{
	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_POKE_TYPE, BTL_POKEPARAM_GetPokeType(defender) );
	BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(defender) );
	BTL_EVENT_CallHandlers( wk, BTL_EVENT_DEFENDER_TYPE );

	{
		PokeTypePair type = BTL_EVENTVAR_GetValue( BTL_EVAR_POKE_TYPE );
		BTL_EVENTVAR_Pop();
		return type;
	}
}

// メンバーバトル場から退出
static void scEvent_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, posIdx );
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_MEMBER_OUT );
		// ここで死ぬこともある
		if( !BTL_POKEPARAM_IsDead(bpp) )
		{
			SCQUE_PUT_ACT_MemberOut( wk->que, clientID, posIdx );
		}
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_MemberOut1, pokeID );
	BTL_EVENTVAR_Pop();

	BTL_HANDLER_TOKUSEI_Remove( bpp );
}

// メンバー新規参加
static void scEvent_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx )
{
	BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
	BTL_POKEPARAM* bpp;

	BTL_PARTY_SwapMembers( party, posIdx, nextPokeIdx );
	bpp = BTL_PARTY_GetMemberData( party, posIdx );
	{
		u8 pokeID = BTL_POKEPARAM_GetID(bpp);
		BTL_Printf("クライアント[%d] のメンバー[%d] が pos[%d] に出場でハンドラ追加\n",
					clientID, pokeID, posIdx);
	}
	BTL_HANDLER_TOKUSEI_Add( bpp );

	SCQUE_PUT_ACT_MemberIn( wk->que, clientID, posIdx, nextPokeIdx );
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(bpp) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_MEMBER_IN );
	BTL_EVENTVAR_Pop();

}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ追加効果により対象ポケモンをひるませる
 *
 * @param   wk					
 * @param   target			対象ポケモンデータ
 * @param   attacker		攻撃したポケモンデータ
 * @param   per					ひるみ確率
 *
 */
//--------------------------------------------------------------------------
static void scEvent_AddShrink( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker, u8 per )
{
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_SHRINK_CHECK );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
	BTL_EVENTVAR_Pop();

	if( perOccur(per) )
	{
		BTL_POKEPARAM_SetTurnFlag( target, BPP_TURNFLG_SHRINK );
	}
}
//--------------------------------------------------------------------------
/**
 * 【Event】ワザ追加効果による状態異常の発生チェック
 *
 * @param   wk		
 * @param   receiver		ダメージ受けた側
 * @param   attacker		ワザ出した側
 * @param   waza				出したワザ
 *
 */
//--------------------------------------------------------------------------
static void scEvent_CheckAddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker, WazaID waza )
{
	WAZA_SICKCONT_PARAM  waza_contParam;
	WazaSick sick = WAZADATA_GetSick( waza, &waza_contParam );
	u8 default_per = WAZADATA_GetSickPer( waza );
	u8 per = default_per;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(receiver) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, default_per );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_SICK );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
	BTL_EVENTVAR_Pop();

	if( perOccur(per) )
	{
		BPP_SICK_CONT cont;
		BTL_CALC_WazaSickContToBppSickCont( waza_contParam, attacker, &cont );
		svflowsub_MakeSick( wk, receiver, attacker, sick, cont, (per>=100) );
	}
}
//--------------------------------------------------------------------------
/**
 * 【Event】特定ポケモンをポケモン系の状態異常にする
 *
 * @param   wk					
 * @param   receiver		状態異常を受ける対象のポケモンパラメータ
 * @param   attacker		状態異常を与える側のポケモンパラメータ
 * @param   sick				状態異常ID
 * @param   fAlmost			ほぼ確定フラグ（※）
 *
 * ※ほぼ確定フラグとは
 *	プレイヤーがほぼ確実に状態異常になるはず、と思う状況でTRUEにする。
 *	追加効果なら効果確率100、ワザ直接効果ならワザ命中率100の場合。とくせい効果の場合は常にTRUE。
 *	これがTRUEの場合、状態異常を無効化するとくせいが発動した場合にそのことを表示するために使う。
 */
//--------------------------------------------------------------------------
static void scEvent_MakePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	PokeSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost )
{
	// てんこう「はれ」の時に「こおり」にはならない
	if( (BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE)
	&&  (sick == POKESICK_KOORI )
	){
		return;
	}
	// すでにポケモン系状態異常になっているなら、新たにポケモン系状態異常にはならない
	if( BTL_POKEPARAM_GetPokeSick(receiver) != POKESICK_NULL )
	{
		return;
	}

	{
		BTL_EVENTVAR_Push();
			if( attacker )
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
			}
			else
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEID_NULL );
			}
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(receiver) );
			BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
			BTL_EVENTVAR_SetValue( BTL_EVAR_SICK_CONT, sickCont.raw );
			BTL_EVENTVAR_SetValue( BTL_EVAR_ALMOST_FLAG, fAlmost );
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_MAKE_POKESICK );
			sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
			if( sick != WAZASICK_NULL )
			{
				u8 pokeID = BTL_POKEPARAM_GetID( receiver );
				sickCont.raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );
				BTL_POKEPARAM_SetWazaSick( receiver, sick, sickCont );
				SCQUE_PUT_OP_SetSick( wk->que, pokeID, sick, sickCont.raw );
				SCQUE_PUT_ACT_SickSet( wk->que, pokeID, sick );
			}
		BTL_EVENTVAR_Pop();
	}
}
//--------------------------------------------------------------------------
/**
 * 【Event】特定ポケモンをワザ系の状態異常にする
 *
 * @param   wk					
 * @param   receiver		状態異常を受ける対象のポケモンパラメータ
 * @param   attacker		状態異常を与える側のポケモンパラメータ
 * @param   sick				状態異常ID
 * @param   fAlmost			ほぼ確定フラグ（※）
 *
 * ※ほぼ確定フラグとは
 *	プレイヤーがほぼ確実に状態異常になるはず、と思う状況でTRUEにする。
 *	追加効果なら効果確率100、ワザ直接効果ならワザ命中率100の場合。とくせい効果の場合は常にTRUE。
 *	これがTRUEの場合、状態異常を無効化するとくせいが発動した場合にそのことを表示するために使う。
 */
//--------------------------------------------------------------------------
static void scEvent_MakeWazaSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost )
{
	BTL_EVENTVAR_Push();
		if( attacker )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		}
		else
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEID_NULL );
		}
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(receiver) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
		BTL_EVENTVAR_SetValue( BTL_EVAR_SICK_CONT, sickCont.raw );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ALMOST_FLAG, fAlmost );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_MAKE_WAZASICK );
		sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
		if( sick != WAZASICK_NULL )
		{
			u8 pokeID = BTL_POKEPARAM_GetID( receiver );
			sickCont.raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );
			BTL_POKEPARAM_SetWazaSick( receiver, sick, sickCont );
			SCQUE_PUT_OP_SetSick( wk->que, pokeID, sick, sickCont.raw );
//			SCQUE_PUT_ACT_SickSet( wk->que, pokeID, sick );
		}
	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * 【Event】ワザ追加効果によるランク効果の発生チェック
 *
 * @param   wk		
 * @param   receiver		ダメージ受けた側
 * @param   attacker		ワザ出した側
 * @param   waza				出したワザ
 *
 */
//--------------------------------------------------------------------------
static void scEvent_CheckAddEffect_Target( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker, WazaID waza )
{
	u8 default_per, per;

	default_per = WAZADATA_GetRankEffPer( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(receiver) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, default_per );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_RANK_TARGET );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
	BTL_EVENTVAR_Pop();

	if( perOccur(per) )
	{
		int  volume;
		WazaRankEffect  eff;
		u8 eff_cnt, i;

		eff_cnt = WAZADATA_GetRankEffectCount( waza );
		for(i=0; i<eff_cnt; ++i)
		{
			eff = WAZADATA_GetRankEffect( waza, i, &volume );
			if( volume < 0 )
			{
				scEvent_RankDown( wk, receiver, eff, -volume, (default_per>=100) );
			}
			else
			{
				scEvent_RankUp( wk, receiver, eff, volume, (default_per>=100) );
			}
		}
	}
}
//--------------------------------------------------------------------------
/**
 * [Event] 対象ポケモンの能力ランクを下げる
 *
 * @param   wk					
 * @param   pokeID			対象ポケモンのID
 * @param   pokePos			
 * @param   statusType	
 * @param   volume			
 * @param   fAlmost			ほぼ確定フラグ（とくせい等の例外的な場合を除き確実に効果を発生させるケースでTRUE）
 *
 */
//--------------------------------------------------------------------------
static void scEvent_RankDown( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BppValueID statusType, u8 volume, BOOL fAlmost )
{
	u8 pokeID = BTL_POKEPARAM_GetID(target);

	BTL_EVENTVAR_Push();

		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );
		BTL_EVENTVAR_SetValue( BTL_EVAR_STATUS_TYPE, statusType );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ALMOST_FLAG, fAlmost );

		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, FALSE );

		BTL_EVENT_CallHandlers( wk, BTL_EVENT_BEFORE_RANKDOWN );

		if( BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_FLAG) == FALSE )
		{
			volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
			BTL_POKEPARAM_RankDown(
					BTL_POKECON_GetPokeParam(wk->pokeCon, pokeID),
					statusType,
					volume
			);
			SCQUE_PUT_OP_RankDown( wk->que, pokeID, statusType, volume );
			SCQUE_PUT_ACT_RankDown( wk->que, pokeID, statusType, volume );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankdown_ATK, pokeID, statusType, volume );
		}

	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] 対象ポケモンの能力ランクを上げる
 *
 * @param   wk		
 * @param   pp		
 * @param   statusType	
 * @param   volume		
 * @param   fAlmost			ほぼ確定フラグ（とくせい等の例外的な場合を除き確実に効果を発生させるケースでTRUE）
 *
 */
//--------------------------------------------------------------------------
static void scEvent_RankUp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* pp, BppValueID statusType, u8 volume, BOOL fAlmost )
{
	BTL_EVENTVAR_Push();

	{
		u8 pokeID = BTL_POKEPARAM_GetID( pp );

		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );
		BTL_EVENTVAR_SetValue( BTL_EVAR_STATUS_TYPE, statusType );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ALMOST_FLAG, fAlmost );

		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, FALSE );

		BTL_EVENT_CallHandlers( wk, BTL_EVENT_BEFORE_RANKUP );

		if( BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_FLAG) == FALSE )
		{
			volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
			BTL_POKEPARAM_RankUp( pp, statusType, volume );
			SCQUE_PUT_OP_RankUp( wk->que, pokeID, statusType, volume );
			SCQUE_PUT_ACT_RankUp( wk->que, pokeID, statusType, volume );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankup_ATK, pokeID, statusType, volume );
		}
	}

	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * 【Event】天候の変化
 *
 * @param   wk			
 * @param   weather	天候
 * @param   turn		継続ターン数
 *
 * @retval  BOOL		変化した場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn )
{
	GF_ASSERT(weather != BTL_WEATHER_NONE);

	BTL_Printf("天候変化イベント発生 ... 天候->%d\n", weather);

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_WEATHER, weather );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHANGE );
		weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );
		if( weather != BTL_WEATHER_NONE )
		{
			BTL_Printf("   天候 -> %d へ変化\n", weather);
			BTL_FIELD_SetWeather( weather, turn );
		}
		else
		{
			BTL_Printf("   天候変化 打ち消し！\n");
		}
	BTL_EVENTVAR_Pop();
	return (weather != BTL_WEATHER_NONE);
}



//----------------------------------------------------------------------------------------------------------
// 以下、ハンドラからの応答受信関数とユーティリティ群
//----------------------------------------------------------------------------------------------------------

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
BtlPokePos BTL_SVFLOW_CheckExistFrontPokeID( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
	SVCL_WORK* clwk;
	int i;

	for(i=0; i<BTL_CLIENT_MAX; ++i)
	{
		clwk = BTL_SERVER_GetClientWork( wk->server, i );
		if( clwk->numCoverPos )
		{
			int p;
			for(p=0; p<clwk->numCoverPos; ++p)
			{
				if( (clwk->frontMember[p] != NULL)
				&&	(BTL_POKEPARAM_GetID(clwk->frontMember[p]) == pokeID)
				){
					return BTL_MAIN_GetClientPokePos( wk->mainModule, i, p );
				}
			}
		}
	}
	return BTL_POS_MAX;
}
//--------------------------------------------------------------------------------------
/**
 * 場に出ている全ポケモンのIDを配列に格納する
 *
 * @param   wk		
 * @param   dst		
 *
 * @retval  u8		場に出ているポケモン数
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFLOW_RECEPT_GetAllFrontPokeID( BTL_SVFLOW_WORK* wk, u8* dst )
{
	FRONT_POKE_SEEK_WORK fps;
	BTL_POKEPARAM* bpp;
	u8 cnt = 0;

	FRONT_POKE_SEEK_InitWork( &fps, wk );
	while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
	{
		dst[ cnt++ ] = BTL_POKEPARAM_GetID( bpp );
	}
	return cnt;
}
//--------------------------------------------------------------------------------------
/**
 * 場に出ている全ての相手側ポケモンIDを配列に格納する
 *
 * @param   wk		
 * @param   dst		
 *
 * @retval  u8		場に出ているポケモン数
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( BTL_SVFLOW_WORK* wk, u8 basePokeID, u8* dst )
{
	FRONT_POKE_SEEK_WORK fps;
	BTL_POKEPARAM* bpp;
	u8 cnt = 0;

	FRONT_POKE_SEEK_InitWork( &fps, wk );
	while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
	{
		if( !BTL_MAINUTIL_IsFriendPokeID( basePokeID, BTL_POKEPARAM_GetID(bpp) ) )
		{
			dst[ cnt++ ] = BTL_POKEPARAM_GetID( bpp );
		}
	}
	return cnt;
}

//--------------------------------------------------------------------------------------
/**
 * 指定のとくせいを持つポケモンが戦闘に出ているかチェック
 *
 * @param   wk		
 * @param   tokusei		
 *
 * @retval  BOOL		出ていたらTRUE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFLOW_RECEPT_CheckExistTokuseiPokemon( BTL_SVFLOW_WORK* wk, PokeTokusei tokusei )
{
	FRONT_POKE_SEEK_WORK fps;
	BTL_POKEPARAM* bpp;
	FRONT_POKE_SEEK_InitWork( &fps, wk );
	while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
	{
		if( BTL_POKEPARAM_GetValue(bpp, BPP_TOKUSEI) == tokusei )
		{
			return TRUE;
		}
	}
	return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 * 指定IDのポケモンパラメータを返す
 *
 * @param   wk		
 * @param   pokeID		
 *
 * @retval  const BTL_POKEPARAM*		
 */
//--------------------------------------------------------------------------------------
const BTL_POKEPARAM* BTL_SVFLOW_RECEPT_GetPokeParam( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
	return BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
}


//=============================================================================================
/**
 * [ハンドラ受信] とくせいウィンドウ表示イン
 *
 * @param   wk		
 * @param   pokeID		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_TokuseiWinIn( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
	SCQUE_PUT_TOKWIN_IN( wk->que, pokeID );
}
//=============================================================================================
/**
 * [ハンドラ受信] とくせいウィンドウ表示アウト
 *
 * @param   wk		
 * @param   pokeID		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_TokuseiWinOut( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
	SCQUE_PUT_TOKWIN_OUT( wk->que, pokeID );
}

void BTL_SERVER_RECTPT_StdMessage( BTL_SVFLOW_WORK* wk, u16 msgID )
{
	SCQUE_PUT_MSG_STD( wk->que, msgID );
}
void BTL_SERVER_RECTPT_SetMessage( BTL_SVFLOW_WORK* wk, u16 msgID, u8 pokeID )
{
	SCQUE_PUT_MSG_SET( wk->que, msgID, pokeID );
}
//=============================================================================================
/**
 * [ハンドラ受信] ステータスのランクダウン効果
 *
 * @param   wk			
 * @param   exPos					対象ポケモン位置
 * @param   statusType		ステータスタイプ
 * @param   volume		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_RankDownEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume )
{
	BTL_POKEPARAM* pp;
	u8 targetPos[ BTL_POSIDX_MAX ];
	u8 numPokemons, i;

	numPokemons = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, targetPos );
	BTL_Printf("ランクさげ効果：タイプ=%d,  対象ポケモン数=%d\n", statusType, numPokemons );
	for(i=0; i<numPokemons; ++i)
	{
		pp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos[i] );
		scEvent_RankDown( wk, pp, statusType, volume, TRUE );
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] ステータスのランクアップ効果
 *
 * @param   wk			
 * @param   exPos					対象ポケモン位置
 * @param   statusType		ステータスタイプ
 * @param   volume		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_RankUpEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume )
{
	BTL_POKEPARAM* pp;
	u8 targetPos[ BTL_POSIDX_MAX ];
	u8 numPokemons, i;

	numPokemons = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, targetPos );
	BTL_Printf("ランクあげ効果：タイプ=%d,  対象ポケモン数=%d\n", statusType, numPokemons );
	for(i=0; i<numPokemons; ++i)
	{
		pp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos[i] );
		if( !BTL_POKEPARAM_IsDead(pp) )
		{
			scEvent_RankUp( wk, pp, statusType, volume, TRUE );
		}
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] シンプルHP増減効果
 *
 * @param   wk						
 * @param   					対象ポケモン位置
 * @param   statusType		ステータスタイプ
 * @param   volume		
 *
 */
//=============================================================================================
void BTL_SERVER_RECEPT_HP_Add( BTL_SVFLOW_WORK* wk, u8 pokeID, int value )
{
	if( value )
	{
		BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
		if( value > 0 )
		{
			BTL_POKEPARAM_HpPlus( bpp, value );
			SCQUE_PUT_OP_HpPlus( wk->que, pokeID, value );
		}
		else
		{
			value *= -1;
			BTL_POKEPARAM_HpMinus( bpp, value );
			SCQUE_PUT_OP_HpMinus( wk->que, pokeID, value );
		}
		SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] 状態異常をくらわす処理
 *
 * @param   wk						
 * @param   targetPokeID	くらう相手のポケモンID
 * @param   attackPokeID	くらわせる側の（とくせい持ちなど）ポケモンID
 * @param   sick					
 * @param   fAlmost				
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_AddSick( BTL_SVFLOW_WORK* wk, u8 targetPokeID, u8 attackPokeID,
	WazaSick sick, BPP_SICK_CONT contParam, BOOL fAlmost )
{
	BTL_POKEPARAM* receiver = BTL_POKECON_GetPokeParam( wk->pokeCon, targetPokeID );
	if( !BTL_POKEPARAM_IsDead(receiver) )
	{
		BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, attackPokeID );
		svflowsub_MakeSick( wk, receiver, attacker, sick, contParam, fAlmost );
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] ポケモン系状態異常の回復処理
 *
 * @param   wk		
 * @param   pokeID		
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_CurePokeSick( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
	if( !BTL_POKEPARAM_IsDead(bpp) )
	{
		BTL_POKEPARAM_CurePokeSick( bpp );
		SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] ワザ系状態異常の回復処理
 *
 * @param   wk		
 * @param   pokeID		
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_CureWazaSick( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaSick sick )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
	if( !BTL_POKEPARAM_IsDead(bpp) )
	{
		BTL_POKEPARAM_CureWazaSick( bpp, sick );
		SCQUE_PUT_OP_CureWazaSick( wk->que, pokeID, sick );
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] 天候の変化効果
 *
 * @param   wk				
 * @param   weather		変化後の天候指定／天候をフラットにしたい場合は BTL_WEATHER_NONE
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather )
{
	if( weather != BTL_WEATHER_NONE )
	{
		if( scEvent_ChangeWeather( wk, weather, BTL_WEATHER_TURN_PERMANENT ) )
		{
			SCQUE_PUT_ACT_WeatherStart( wk->que, weather );
		}
	}
	else
	{
		BtlWeather prevWeather = BTL_FIELD_GetWeather();
		if( prevWeather != BTL_WEATHER_NONE )
		{
			BTL_FIELD_ClearWeather();
			SCQUE_PUT_ACT_WeatherEnd( wk->que, prevWeather );
		}
		else
		{
			GF_ASSERT(0);
		}
	}
}
//=============================================================================================
/**
 * [ハンドラ受信] とくせい「トレース」処理
 *
 * @param   wk						
 * @param   pokeID				トレース使う側ポケモンID
 * @param   targetPokeID	トレースされる側ポケモンID
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_TraceTokusei( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 targetPokeID )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
	BTL_POKEPARAM* bppTgt = BTL_POKECON_GetPokeParam( wk->pokeCon, targetPokeID );
	PokeTokusei tok = BTL_POKEPARAM_GetValue( bppTgt, BPP_TOKUSEI );

	BTL_Printf("トレースで書き換えるとくせい=%d\n", tok);
	BTL_POKEPARAM_ChangeTokusei( bpp, tok );
	SCQUE_PUT_ACT_TokTrace( wk->que, pokeID, targetPokeID, tok );
	BTL_HANDLER_TOKUSEI_Add( bpp );
}

