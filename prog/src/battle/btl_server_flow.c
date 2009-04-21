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
#include "handler\hand_item.h"

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

	EVENT_WORK_DEPTH = 16,
	EVENT_WORK_TOTAL_SIZE = 512,

	STRID_NULL = 0xffff,

}FlowFlag;

/**
 *	ふきとばし系のワザ処理パターン
 */
typedef enum {
	PUSHOUT_EFFECT_ESCAPE,		///< 逃げたのと同じ（戦闘を終了する）
	PUSHOUT_EFFECT_CHANGE,		///< 相手を入れ替えさせる（手持ちがいなければ失敗する）
	PUSHOUT_EFFECT_MUSTFAIL,	///< 必ず失敗する
}PushOutEffect;


/**
 *	アクション優先順記録構造体
 */
typedef struct {
	u8  clientID;		///< クライアントID
	u8  pokeIdx;		///< そのクライアントの、何体目？ 0～
}ACTION_ORDER_WORK;


/**
 * 対象となるポケモンパラメータを記録しておくためのワーク
 */
typedef struct {

	BTL_POKEPARAM*	pp[ BTL_POS_MAX ];
	u16							damage[ BTL_POS_MAX ];
	u8							count;
	u8							idx;

}TARGET_POKE_REC;

/**
 *	戦闘に出ているポケモンデータに順番にアクセスする処理のワーク
 */
typedef struct {
	u8 clientIdx;
	u8 pokeIdx;
	u8 endFlag;
}FRONT_POKE_SEEK_WORK;


typedef struct {

	u8		work[ EVENT_WORK_TOTAL_SIZE ];
	u16		size[ EVENT_WORK_DEPTH ];
	u8		sp;

}BTL_EVENT_WORK_STACK;

//-----------------------------------------------------
/**
 *	メインワーク
 */
//-----------------------------------------------------
struct _BTL_SVFLOW_WORK {

	BTL_SERVER*             server;
	const BTL_MAIN_MODULE*  mainModule;
	BTL_POKE_CONTAINER*			pokeCon;
	BTL_SERVER_CMD_QUE*			que;
	u32											turnCount;
	SvflowResult						flowResult;
	u8                      numClient;
	u8			wazaEff_EnableFlag;
	u8			wazaEff_TargetPokeID;

	ACTION_ORDER_WORK		actOrder[ BTL_POS_MAX ];
	TARGET_POKE_REC			targetPokemon;
	TARGET_POKE_REC			targetSubPokemon;
	TARGET_POKE_REC			damagedPokemon;
	SVFL_WAZAPARAM			wazaParam;

	BTL_EVENT_WORK_STACK	eventWork;

	u8					flowFlags[ FLOWFLG_BYTE_MAX ];
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static u8 sortClientAction( BTL_SVFLOW_WORK* wk, ACTION_ORDER_WORK* order );
static u8 countAlivePokemon( BTL_SVFLOW_WORK* wk );
static inline void FlowFlg_Set( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_Clear( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline BOOL FlowFlg_Get( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_ClearAll( BTL_SVFLOW_WORK* wk );
static void wazaEff_SetTarget( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target );
static void wazaEff_SetNoTarget( BTL_SVFLOW_WORK* wk );
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk );
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk, BTL_POKEPARAM** bpp );
static inline void TargetPokeRec_Clear( TARGET_POKE_REC* rec );
static inline void TargetPokeRec_AddWithDamage( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp, u16 damage );
static inline void TargetPokeRec_Add( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp );
static inline void TargetPokeRec_Remove( TARGET_POKE_REC* rec, BTL_POKEPARAM* bpp );
static inline BTL_POKEPARAM* TargetPokeRec_GetNext( TARGET_POKE_REC* rec );
static inline void TargetPokeRec_GetStart( TARGET_POKE_REC* rec );
static inline BTL_POKEPARAM* TargetPokeRec_Get( const TARGET_POKE_REC* rec, u32 idx );
static inline u32 TargetPokeRec_GetDamage( const TARGET_POKE_REC* rec, u32 idx );
static inline int TargetPokeRec_SeekFriendIdx( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, u32 start_idx );
static inline u32 TargetPokeRec_GetCount( const TARGET_POKE_REC* rec );
static u32 TargetPokeRec_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst );
static u32 TargetPokeRec_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst );
static void TargetPokeRec_RemoveDeadPokemon( TARGET_POKE_REC* rec );
static BOOL scput_Nigeru( BTL_SVFLOW_WORK* wk, u8 clientID, u8 pokeIdx );
static void scproc_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 pokeIdx, BOOL fBtlStart );
static void scproc_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx );
static void scproc_Fight( BTL_SVFLOW_WORK* wk, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action );
static u8 flowsub_registerWazaTargets( BTL_SVFLOW_WORK* wk, BtlPokePos atPos, WazaID waza, const BTL_ACTION_PARAM* action, TARGET_POKE_REC* rec );
static void flowsub_checkNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static BTL_POKEPARAM* svflowsub_get_opponent_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos, u8 pokeSideIdx );
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos );
static void scproc_Fight_Damage( BTL_SVFLOW_WORK* wk, WazaID waza, SVCL_WORK* atClient,
	 BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scproc_Fight_Damage_side( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker,
	TARGET_POKE_REC* targets, fx32 dmg_ratio );
static void scproc_Fight_Damage_side_single( BTL_SVFLOW_WORK* wk,
			BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, fx32 targetDmgRatio );
static void svflowsub_damage_act_singular(  BTL_SVFLOW_WORK* wk,
		BTL_POKEPARAM* attacker,  BTL_POKEPARAM* defender,
		WazaID waza,   BtlTypeAff aff,   fx32 targetDmgRatio );
static void scproc_Fight_damage_side_plural( BTL_SVFLOW_WORK* wk, 
		BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets, const u8* aff, WazaID waza, fx32 dmg_ratio );
static void scproc_WazaAdditionalEffect( BTL_SVFLOW_WORK* wk, WazaID waza,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage );
static void scproc_WazaDamageReaction( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk );
static void scproc_WazaDamageReaction_Sick( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk );
static void scproc_WazaDamageReaction_Rank( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk );
static void scproc_WazaDamageReaction_Damage( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk );
static BOOL scproc_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static BOOL addsick_core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost, BOOL* fCarry, BOOL* fItemResponce );
static BOOL scEvent_AddSick_IsMustFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaSick sick );
static void scEvent_AddSick_Check( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
		WazaSick sick, BTL_EVWK_ADDSICK* evwk );
static void scproc_UseItem( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_Fight_Damage_After( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void scput_Fight_Damage_After_Shrink( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static BOOL svflowsub_hitcheck_singular( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza );
static u16 scEvent_CalcDamage( BTL_SVFLOW_WORK* wk, 
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza,
		BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag );
static void scproc_Damage_Kickback( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 wazaDamage );
static BOOL scproc_SimpleDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage, u16 strID );
static BOOL scEvent_CheckEnableSimpleDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 damage );
static BOOL scEvent_SimpleDamage_Reaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scproc_Fight_Damage_AddSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static void scproc_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec );
static void scproc_Fight_Damage_AddEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static void scproc_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec );
static BOOL scproc_WazaRankEffect_Common( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL fAlmost );
static BOOL scproc_RankEffectCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume, BOOL fAlmost );
static void scproc_Fight_Drain( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage );
static void scproc_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets );
static void svflowsub_MakeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
		WazaSick sick, BPP_SICK_CONT contParam, BOOL fAlmost );
static void scproc_Fight_PushOut( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec );
static PushOutEffect check_pushout_effect( BTL_SVFLOW_WORK* wk );
static u8 get_pushout_nextpoke_idx( BTL_SVFLOW_WORK* wk, const SVCL_WORK* clwk );
static BOOL scEvent_CheckPushOutFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target );
static void scproc_Fight_Weather( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, const BTL_ACTION_PARAM* action );
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
static void eventWork_Init( BTL_EVENT_WORK_STACK* stack );
static void* eventWork_Push( BTL_EVENT_WORK_STACK* stack, u32 size );
static void eventWork_Pop( BTL_EVENT_WORK_STACK* stack, void* adrs );
static void scPut_ConfCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_ConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage );
static void scPut_CurePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scPut_WazaExecuteFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause );
static void scPut_WazaFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scPut_WazaAvoid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza );
static void scput_WazaNoEffect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender );
static void scput_DmgToRecover( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BTL_EVWK_DMG_TO_RECOVER* evwk );
static void scPut_CheckDeadCmd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke );
static void scPut_WazaEffectOn( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaID waza );
static void scPut_WazaDamageSingle( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* defender, BtlTypeAff aff, u32 damage, BOOL critical_flag );
static void scPut_WazaDamagePlural( BTL_SVFLOW_WORK* wk, u32 poke_cnt, BtlTypeAff aff, 
	BTL_POKEPARAM** bpp, const u16* damage, const u8* critical_flag );
static u32 scEvent_CalcKickBack( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u32 damage );
static void scPut_TokWin_In( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_TokWin_Out( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_RankEffectLimit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect effect, int volume );
static void scPut_RankEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume );
static void scPut_RankEffectFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
	WazaRankEffect effect, const BTL_EVWK_CHECK_RANKEFF* evwk );
static void scPut_Ichigeki( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target );
static void scPut_SimpleHp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, int value );
static void scPut_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaSick sick, BPP_SICK_CONT sickCont );
static void scPut_AddSickFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaSick sick );
static void scPut_Messae_Set( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 strID );
static u16 scEvent_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckConf( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static u16 scEvent_CalcConfDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static SV_WazaFailCause scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
	WazaID waza, u8* curedFlag );
static void scEvent_CheckWazaParam( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* param );
static BOOL scEvent_checkHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_IchigekiCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckNotEffect_byType( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, u8 lv,
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza, BTL_EVWK_DMG_TO_RECOVER* evwk );
static BOOL scEvent_CheckCritical( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_WazaDamageReaction( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff,
	u32 damage, BOOL criticalFlag, BTL_EVWK_DAMAGE_REACTION* evwk );
static void scEvent_WazaDamageEffect_Double( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* def1, BTL_POKEPARAM* def2, WazaID waza, BtlTypeAff aff,
	u32 dmg1, u32 dmg2 );
static BtlTypeAff scEvent_checkWazaDamageAffinity( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, PokeType waza_type );
static void scEvent_decrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, u8 wazaIdx, TARGET_POKE_REC* rec );
static u8 scEvent_getHitPer( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckPluralHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u8* hitCount );
static u16 scEvent_getWazaPower( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static u16 scEvent_getAttackPower( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag );
static u16 scEvent_getDefenderGuard( BTL_SVFLOW_WORK* wk,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL criticalFlag );
static fx32 scEvent_CalcTypeMatchRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, PokeType waza_type );
static PokeTypePair scEvent_getDefenderPokeType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender );
static void scEvent_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx );
static void scEvent_MemberIn( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scEvent_AddShrink( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker, u8 per );
static BOOL scEvent_CheckAddSickPer( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static void scEvent_MakePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	PokeSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static void scEvent_MakeWazaSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* receiver, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static BOOL scEvent_CheckAddRankEffectOccur( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static void scEvent_GetWazaRankEffectValue( BTL_SVFLOW_WORK* wk, WazaID waza, u32 idx,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaRankEffect* effect, int* volume );
static BOOL scEvent_CheckRankEffectSuccess( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
	WazaRankEffect effect, int volume, BTL_EVWK_CHECK_RANKEFF* evwk );
static int scEvent_CalcDrainVolume( BTL_SVFLOW_WORK* wk, WazaID waza,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, int damage );
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
	wk->flowResult = SVFLOW_RESULT_DEFAULT;
	wk->que = que;

	eventWork_Init( &wk->eventWork );

	return wk;
}


void BTL_SVFLOW_QuitSystem( BTL_SVFLOW_WORK* wk )
{
	GFL_HEAP_FreeMemory( wk );
}


SvflowResult BTL_SVFLOW_Start_AfterPokemonIn( BTL_SVFLOW_WORK* wk )
{
	SVCL_WORK* cw;
	u32 i, posIdx;

	SCQUE_Init( wk->que );

	for(i=0; i<BTL_CLIENT_MAX; ++i)
	{
		cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, i );
		if( cw == NULL ){ continue; }

		for(posIdx=0; posIdx<cw->numCoverPos; ++posIdx)
		{
			if( !BTL_POKEPARAM_IsDead(cw->member[posIdx]) )
			{
				// @@@ 先々は、戦闘開始時のMemberInと途中入れ替えのMemberInでは演出を別ける必要アリ。
				scproc_MemberIn( wk, i, posIdx, posIdx, TRUE );
			}
		}
	}

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
	BTL_EVENT_StartTurn();
	wk->flowResult =  SVFLOW_RESULT_DEFAULT;

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
				scproc_Fight( wk, clientID, pokeIdx, action );
				break;
			case BTL_ACTION_ITEM:
				BTL_Printf("【どうぐ】を処理。アイテム%dを、%d番の相手に。\n", action->item.number, action->item.targetIdx);
				break;
			case BTL_ACTION_CHANGE:
				BTL_Printf("【ポケモン】を処理。位置%d <- ポケ%d \n", action->change.posIdx, action->change.memberIdx);
				scproc_MemberOut( wk, clientID, action->change.posIdx );
				scproc_MemberIn( wk, clientID, action->change.posIdx, action->change.memberIdx, FALSE );
				break;
			case BTL_ACTION_ESCAPE:
				BTL_Printf("【にげる】を処理。\n");
				if( scput_Nigeru( wk, clientID, pokeIdx ) ){
					SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_EscapeSuccess );
					wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
				}	else {
					SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_EscapeFail);
				}
				break;
			}

			if( wk->flowResult !=  SVFLOW_RESULT_DEFAULT )
			{
				return wk->flowResult;
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
		return SVFLOW_RESULT_POKE_CHANGE;
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

			scproc_MemberIn( wk, i, action->change.posIdx, action->change.memberIdx, FALSE );
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

static void wazaEff_SetTarget( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target )
{
	wk->wazaEff_EnableFlag = TRUE;
	wk->wazaEff_TargetPokeID = BTL_POKEPARAM_GetID( target );
}
static void wazaEff_SetNoTarget( BTL_SVFLOW_WORK* wk )
{
	wk->wazaEff_EnableFlag = TRUE;
	wk->wazaEff_TargetPokeID = BTL_POKEID_NULL;
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
				if( !BTL_POKEPARAM_IsDead(cw->member[j]) )
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

		*bpp = cw->member[ fpsw->pokeIdx ];
		fpsw->pokeIdx++;

		while( fpsw->clientIdx < BTL_CLIENT_MAX )
		{
			cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, fpsw->clientIdx );
			if( cw )
			{
				while( fpsw->pokeIdx < cw->numCoverPos )
				{
					nextPoke = cw->member[ fpsw->pokeIdx ];
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
static inline void TargetPokeRec_AddWithDamage( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp, u16 damage )
{
	if( rec->count < NELEMS(rec->pp) )
	{
		rec->pp[rec->count] = pp;
		rec->damage[rec->count] = damage;
		rec->count++;
	}
	else
	{
		GF_ASSERT(0);
	}
}
static inline void TargetPokeRec_Add( TARGET_POKE_REC* rec, BTL_POKEPARAM* pp )
{
	TargetPokeRec_AddWithDamage( rec, pp, 0 );
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
static inline u32 TargetPokeRec_GetDamage( const TARGET_POKE_REC* rec, u32 idx )
{
	GF_ASSERT(idx < rec->count);
	return rec->damage[ idx ];
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
static u32 TargetPokeRec_CopyFriends( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst )
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
static u32 TargetPokeRec_CopyEnemys( const TARGET_POKE_REC* rec, const BTL_POKEPARAM* pp, TARGET_POKE_REC* dst )
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
// 死んでるポケモンを削除
static void TargetPokeRec_RemoveDeadPokemon( TARGET_POKE_REC* rec )
{
	BTL_POKEPARAM* bpp;

	TargetPokeRec_GetStart( rec );

	// タイプ相性による無効化チェック
	while( (bpp = TargetPokeRec_GetNext(rec)) != NULL )
	{
		if( BTL_POKEPARAM_IsDead(bpp) )
		{
			TargetPokeRec_Remove( rec, bpp );
		}
	}
}


//======================================================================================================
// サーバーフロー処理
//======================================================================================================


//-----------------------------------------------------------------------------------
// サーバーフロー：「にげる」
//-----------------------------------------------------------------------------------
static BOOL scput_Nigeru( BTL_SVFLOW_WORK* wk, u8 clientID, u8 pokeIdx )
{
	SVCL_WORK* clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
	BTL_POKEPARAM* bpp = clwk->member[ pokeIdx ];

	BOOL fEscape = TRUE;	// @@@ 今はすばやさ計算をせず誰でも逃げられるようにしている

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(bpp) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, fEscape );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_ESCAPE );
		fEscape = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
	BTL_EVENTVAR_Pop();

	#ifdef PM_DEBUG
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
		fEscape = TRUE;
	}
	#endif

	return fEscape;
}
//-----------------------------------------------------------------------------------
// サーバーフロー：メンバーを場に登場させる
//-----------------------------------------------------------------------------------
static void scproc_MemberIn( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 pokeIdx, BOOL fBtlStart )
{
	SVCL_WORK* clwk;
	BTL_PARTY* party;
	BTL_POKEPARAM* bpp;

	party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
	clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

	GF_ASSERT(pokeIdx < clwk->numCoverPos);

	if( posIdx != pokeIdx )
	{
		BTL_PARTY_SwapMembers( party, posIdx, pokeIdx );
	}
	bpp = BTL_PARTY_GetMemberData( party, posIdx );

	clwk->frontMember[ posIdx ] = bpp;

	BTL_HANDLER_TOKUSEI_Add( bpp );
	BTL_HANDLER_ITEM_Add( bpp );
	BTL_POKEPARAM_SetAppearTurn( bpp, wk->turnCount );

	SCQUE_PUT_OP_MemberIn( wk->que, clientID, posIdx, pokeIdx, wk->turnCount );
	if( !fBtlStart )
	{
		SCQUE_PUT_ACT_MemberIn( wk->que, clientID, posIdx, pokeIdx, wk->turnCount );
	}

	scEvent_MemberIn( wk, bpp );
}


//-----------------------------------------------------------------------------------
// サーバーフロー：メンバーを場から退場させる
//-----------------------------------------------------------------------------------
static void scproc_MemberOut( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx )
{
	scEvent_MemberOut( wk, clientID, posIdx );
}

//-----------------------------------------------------------------------------------
// サーバーフロー：「たたかう」ルート
//-----------------------------------------------------------------------------------
static void scproc_Fight( BTL_SVFLOW_WORK* wk, u8 attackClientID, u8 posIdx, const BTL_ACTION_PARAM* action )
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
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_KONRAN) )
	{
		scPut_ConfCheck( wk, attacker );
		if( scEvent_CheckConf(wk, attacker ) )
		{
			u16 conf_dmg = scEvent_CalcConfDamage( wk, attacker );
			scPut_ConfDamage( wk, attacker, conf_dmg );
			return;
		}
	}
// ワザ出し成功判定
	{
		SV_WazaFailCause  fail_cause;
		u8 cured_flag;

		fail_cause = scEvent_CheckWazaExecute( wk, attacker, waza, &cured_flag );
		if( cured_flag )
		{
			scPut_CurePokeSick( wk, attacker );
		}
		if( fail_cause != SV_WAZAFAIL_NULL )
		{
			scPut_WazaExecuteFail( wk, attacker, waza, fail_cause );
			return;
		}
	}
// ワザを出すことに成功
	{
		WazaCategory  category = WAZADATA_GetCategory( waza );
		BtlPokePos    atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, attacker_pokeID );
		u16  que_reserve_pos;
		BTL_Printf("出すポケ位置=%d, 出すワザ=%d, カテゴリ=%d\n", atPos, waza, category);

		TargetPokeRec_Clear( &wk->damagedPokemon );
		TargetPokeRec_Clear( &wk->targetPokemon );

		scEvent_CheckWazaParam( wk, waza, attacker, &wk->wazaParam );
		SCQUE_PUT_MSG_WAZA( wk->que, attacker_pokeID, action->fight.wazaIdx );
		que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_WAZA_EFFECT );
		wk->wazaEff_EnableFlag = FALSE;
		wk->wazaEff_TargetPokeID = BTL_POKEID_NULL;

		// @@@ ここに、対象ポケによらない無効化チェックを入れるかも…
		flowsub_registerWazaTargets( wk, atPos, waza, action, &wk->targetPokemon );
		flowsub_checkNotEffect( wk, waza, attacker, &wk->targetPokemon );

		switch( category ){
		case WAZADATA_CATEGORY_SIMPLE_DAMAGE:
		case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
		case WAZADATA_CATEGORY_DAMAGE_EFFECT:
		case WAZADATA_CATEGORY_DAMAGE_SICK:
		case WAZADATA_CATEGORY_DRAIN:
			scproc_Fight_Damage( wk, waza, atClient, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_SIMPLE_EFFECT:
			scproc_Fight_SimpleEffect( wk, waza, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_SIMPLE_SICK:
			scproc_Fight_SimpleSick( wk, waza, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_ICHIGEKI:
			scproc_Fight_Ichigeki( wk, waza, attacker, &wk->targetPokemon );
			break;
		case WAZADATA_CATEGORY_WEATHER:
			scproc_Fight_Weather( wk, waza, attacker, action );
			break;
		case WAZADATA_CATEGORY_PUSHOUT:
			scproc_Fight_PushOut( wk, waza, attacker, &wk->targetPokemon );
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

		if( wk->wazaEff_EnableFlag )
		{
			u8 atPokeID = BTL_POKEPARAM_GetID( attacker );
			SCQUE_PUT_ReservedPos( wk->que, que_reserve_pos, SC_ACT_WAZA_EFFECT, atPokeID, wk->wazaEff_TargetPokeID, waza );
		}

		scEvent_decrementPP( wk, attacker, action->fight.wazaIdx, &wk->damagedPokemon );
	}
}
//--------------------------------------------------------------------------
/**
 * 対象ポケモンのデータポインタを、TARGET_POKE_REC構造体にセット
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
	BTL_POKEPARAM* attacker = BTL_POKECON_GetFrontPokeData( wk->pokeCon, atPos );

	TargetPokeRec_Clear( rec );

	// シングル
	if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE )
	{
		switch( targetType ){
		case WAZA_TARGET_OTHER_SELECT:	///< 自分以外の１体（選択）
		case WAZA_TARGET_ENEMY_SELECT:	///< 敵１体（選択）
		case WAZA_TARGET_ENEMY_RANDOM:	///< 敵ランダム
		case WAZA_TARGET_ENEMY_ALL:			///< 敵側２体
		case WAZA_TARGET_OTHER_ALL:			///< 自分以外全部
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam(wk, atPos, 0) );
			break;

		case WAZA_TARGET_USER:			///< 自分１体のみ
		case WAZA_TARGET_FRIEND_USER_SELECT:	///< 自分を含む味方１体
			TargetPokeRec_Add( rec, attacker );
			break;

		default:
			return 0;
		}
		return 1;
	}
	// ダブル
	else
	{
		BTL_POKEPARAM* bpp = NULL;

		switch( targetType ){
		case WAZA_TARGET_OTHER_SELECT:				///< 自分以外の１体（選択）
			bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, action->fight.targetPos );
			TargetPokeRec_Add( rec, bpp );
			return 1;
		case WAZA_TARGET_ENEMY_SELECT:				///< 敵１体（選択）
			bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, action->fight.targetPos );
			TargetPokeRec_Add( rec, bpp );
			return 1;
		case WAZA_TARGET_ENEMY_RANDOM:				///< 敵ランダム
			bpp = svflowsub_get_opponent_pokeparam( wk, atPos, GFL_STD_MtRand(1) );
			TargetPokeRec_Add( rec, bpp );
			return 1;

		case WAZA_TARGET_ENEMY_ALL:						///< 敵側全体
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam(wk, atPos, 0) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam(wk, atPos, 1) );
			return 2;

		case WAZA_TARGET_OTHER_ALL:						///< 自分以外全部
			TargetPokeRec_Add( rec, svflowsub_get_next_pokeparam( wk, atPos ) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam( wk, atPos, 0 ) );
			TargetPokeRec_Add( rec, svflowsub_get_opponent_pokeparam( wk, atPos, 1 ) );
			return 3;

		case WAZA_TARGET_USER:			///< 自分１体のみ
			TargetPokeRec_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, atPos) );
			return 1;
		case WAZA_TARGET_FRIEND_USER_SELECT:	///< 自分を含む味方１体（選択）
			TargetPokeRec_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, action->fight.targetPos) );
			return 1;
		case WAZA_TARGET_FRIEND_SELECT:				///< 自分以外の味方１体
			TargetPokeRec_Add( rec, svflowsub_get_next_pokeparam(wk, atPos) );
			return 1;
		default:
			return 0;
		}

		if( bpp )
		{
			u8 targetPokeID = BTL_POKEID_NULL;
			BTL_EVENTVAR_Push();
				BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
				BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_TYPE, wk->wazaParam.wazaType );
				BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEID_NULL );
				BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECIDE_TARGET );
				targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
			BTL_EVENTVAR_Pop();
			if( targetPokeID != BTL_POKEID_NULL )
			{
				bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, targetPokeID );
			}
			TargetPokeRec_Add( rec, bpp );
			return 1;
		}
		else
		{
			return 0;
		}
	}
}
//--------------------------------------------------------------------------
/**
 * ワザ発動前の、対象別無効化チェック
 *
 * @param   wk				
 * @param   attacker	[in] ワザを出すポケモン
 * @param   targets		[io] ワザ対象ポケモンコンテナ／無効になったポケモンはコンテナから削除される
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

	// Lv1 無効化チェック（攻撃ポケが必中状態なら負ける）
	if( BTL_POKEPARAM_CheckSick(attacker, WAZASICK_MUSTHIT) )
	{
		TargetPokeRec_GetStart( targets );
		while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
		{
			if( scEvent_CheckNotEffect(wk, waza, 0, attacker, bpp) )
			{
				TargetPokeRec_Remove( targets, bpp );
				SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BTL_POKEPARAM_GetID(bpp) );
			}
		}
	}

	// Lv2 無効化チェック（攻撃ポケが必中状態でも無効化）
	TargetPokeRec_GetStart( targets );
	while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
	{
		if( scEvent_CheckNotEffect(wk, waza, 1, attacker, bpp) )
		{
			TargetPokeRec_Remove( targets, bpp );
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BTL_POKEPARAM_GetID(bpp) );
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
	u8 clientID, posIdx;

	targetPos = BTL_MAIN_GetOpponentPokePos( wk->mainModule, pos, pokeSideIdx );
	BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, targetPos, &clientID, &posIdx );

	clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

	return clwk->frontMember[ posIdx ];
}
//----------------------------------------------------------------------
// サーバーフロー下請け：指定位置から見て隣のポケモンデータを返す
//----------------------------------------------------------------------
static BTL_POKEPARAM* svflowsub_get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos )
{
	SVCL_WORK* clwk;
	BtlPokePos nextPos;
	u8 clientID, posIdx;

	nextPos = BTL_MAIN_GetNextPokePos( wk->mainModule, pos );
	BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, nextPos, &clientID, &posIdx );
	clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

	return clwk->frontMember[ posIdx ];
}
//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ系
//----------------------------------------------------------------------
static void scproc_Fight_Damage( BTL_SVFLOW_WORK* wk, WazaID waza, SVCL_WORK* atClient,
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
		TargetPokeRec_RemoveDeadPokemon( &wk->targetSubPokemon );
		scproc_Fight_Damage_side( wk, waza, attacker, &wk->targetSubPokemon, dmg_ratio );
	}
// 敵に対するダメージ
	if( TargetPokeRec_CopyEnemys(targets, attacker, &wk->targetSubPokemon) )
	{
		// @@@ 本当は死んでたら別ターゲットにしないとダメかも
		TargetPokeRec_RemoveDeadPokemon( &wk->targetSubPokemon );
		scproc_Fight_Damage_side( wk, waza, attacker, &wk->targetSubPokemon, dmg_ratio );
	}

	scproc_Fight_Damage_After( wk, waza, attacker, &wk->damagedPokemon );
}
static void scproc_Fight_Damage_side( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker,
	TARGET_POKE_REC* targets, fx32 dmg_ratio )
{
	BTL_POKEPARAM* bpp;
	u8 poke_cnt;

	// 命中チェック -> 命中しなかった場合は targets から除外
	TargetPokeRec_GetStart( targets );
	while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
	{
		if( !scEvent_checkHit(wk, attacker, bpp, waza) )
		{
			scPut_WazaAvoid( wk, bpp, waza );
			TargetPokeRec_Remove( targets, bpp );
		}
	}
	// 回復チェック -> 特定タイプのダメージを無効化、回復してしまうポケを targets から除外
	{
		BTL_EVWK_DMG_TO_RECOVER* ewk = eventWork_Push( &wk->eventWork, sizeof(BTL_EVWK_DMG_TO_RECOVER) );
		TargetPokeRec_GetStart( targets );
		while( (bpp = TargetPokeRec_GetNext(targets)) != NULL )
		{
			if( scEvent_DmgToRecover(wk, bpp, waza, ewk) )
			{
				scput_DmgToRecover( wk, bpp, ewk );
				TargetPokeRec_Remove( targets, bpp );
			}
		}
		eventWork_Pop( &wk->eventWork, ewk );
	}

	TargetPokeRec_GetStart( targets );
	poke_cnt = TargetPokeRec_GetCount( targets );
	if( poke_cnt == 1 )
	{
		bpp = TargetPokeRec_GetNext( targets );
		scproc_Fight_Damage_side_single( wk, attacker, bpp, waza, dmg_ratio );
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
			aff[i] = scEvent_checkWazaDamageAffinity( wk, bpp, waza_type );
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
				scproc_Fight_Damage_side_single( wk, attacker, bpp, waza, dmg_ratio );
			}
		}
		// 相性一致なら、体力バーを同時に減らすようにコマンド生成する
		else
		{
			scproc_Fight_damage_side_plural( wk, attacker, targets, aff, waza, dmg_ratio );
		}
	}
}
//------------------------------------------------------------------
// サーバーフロー下請け：単体ダメージ処理（上位分岐）
//------------------------------------------------------------------
static void scproc_Fight_Damage_side_single( BTL_SVFLOW_WORK* wk,
			BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, fx32 targetDmgRatio )
{
	BtlTypeAff aff = scEvent_checkWazaDamageAffinity( wk, defender, wk->wazaParam.wazaType );

	svflowsub_damage_act_singular( wk, attacker, defender, waza, aff, targetDmgRatio );

	scPut_CheckDeadCmd( wk, defender );
	scPut_CheckDeadCmd( wk, attacker );
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
	u8 hit_count, critical_flag, item_use_flag;
	u32 damage;
	int i;
	BTL_EVWK_DAMAGE_REACTION* evwk;

	evwk = eventWork_Push( &wk->eventWork, sizeof(BTL_EVWK_DAMAGE_REACTION) );
	plural_flag = scEvent_CheckPluralHit( wk, attacker, waza, &hit_count );
	BTL_Printf("念のため表示 : ヒット回数=%d\n", hit_count);

	atPokeID = BTL_POKEPARAM_GetID( attacker );
	defPokeID = BTL_POKEPARAM_GetID( defender );

	wazaEff_SetTarget( wk, defender );

	for(i=0; i<hit_count; ++i)
	{
		critical_flag = scEvent_CheckCritical( wk, attacker, defender, waza );
		damage = scEvent_CalcDamage( wk, attacker, defender, waza, aff, targetDmgRatio, critical_flag );

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

		scPut_WazaDamageSingle( wk, defender, aff, damage, critical_flag );
		scproc_WazaAdditionalEffect( wk, waza, attacker, defender, damage );

		item_use_flag = scEvent_WazaDamageReaction( wk, attacker, defender, waza, aff, damage, critical_flag, evwk );
		if( item_use_flag ){
			scproc_UseItem( wk, defender );
		}
		scproc_WazaDamageReaction( wk, evwk );

		scproc_Damage_Kickback( wk, attacker, waza, damage );

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

	TargetPokeRec_AddWithDamage( &wk->damagedPokemon, defender, damage );

	if( plural_flag )
	{
		// @@@ 「●●回あたった！」メッセージ
//			SCQUE_PUT_MSG_WazaHitCount( wk->que, i );	// @@@ あとで
	}

	eventWork_Pop( &wk->eventWork, evwk );
}

static void scproc_Fight_damage_side_plural( BTL_SVFLOW_WORK* wk, 
		BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets, const u8* aff, WazaID waza, fx32 dmg_ratio )
{
	BTL_POKEPARAM *bpp[ BTL_POSIDX_MAX ];
	u16 dmg[ BTL_POSIDX_MAX ];
	u8  critical_flg[ BTL_POSIDX_MAX ];
	u8 i, poke_cnt;

	poke_cnt = TargetPokeRec_GetCount( targets );
	GF_ASSERT(poke_cnt < BTL_POSIDX_MAX);

	wazaEff_SetNoTarget( wk );

	for(i=0; i<poke_cnt; ++i)
	{
		bpp[i] = TargetPokeRec_Get( targets, i );
		critical_flg[i] = scEvent_CheckCritical( wk, attacker, bpp[i], waza );
		dmg[i] = scEvent_CalcDamage( wk, attacker, bpp[i], waza, aff[i], dmg_ratio, critical_flg[i] );
		TargetPokeRec_AddWithDamage( &wk->damagedPokemon, bpp[i], dmg[i] );
	}

	scPut_WazaDamagePlural( wk, poke_cnt, aff[0], bpp, dmg, critical_flg );

	for(i=0; i<poke_cnt; ++i)
	{
		scproc_WazaAdditionalEffect( wk, waza, attacker, bpp[i], dmg[i] );
	}

	for(i=0; i<poke_cnt; ++i)
	{
		scPut_CheckDeadCmd( wk, bpp[i] );
	}
	scPut_CheckDeadCmd( wk, attacker );
}
//--------------------------------------------------------------------------
/**
 * ワザダメージ後の追加効果処理
 *
 * @param   wk					
 * @param   waza				使用ワザ
 * @param   attacker		攻撃ポケモンパラメータ
 * @param   defender		防御ポケモンパラメータ
 * @param   damage			与えたダメージ
 *
 */
//--------------------------------------------------------------------------
static void scproc_WazaAdditionalEffect( BTL_SVFLOW_WORK* wk, WazaID waza,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage )
{
	WazaCategory category = WAZADATA_GetCategory( waza );

	if( category == WAZADATA_CATEGORY_DRAIN )
	{
		scproc_Fight_Drain( wk, waza, attacker, defender, damage );
	}
	else
	{
		if( !BTL_POKEPARAM_IsDead(defender) )
		{
			switch( category ){
			case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
				scproc_Fight_Damage_AddEffect( wk, waza, attacker, attacker );
				break;
			case WAZADATA_CATEGORY_DAMAGE_EFFECT:
				scproc_Fight_Damage_AddEffect( wk, waza, attacker, defender );
				break;
			case WAZADATA_CATEGORY_DAMAGE_SICK:
				scproc_Fight_Damage_AddSick( wk, waza, attacker, defender );
				break;
			}
		}
	}
}


//--------------------------------------------------------------------------
/**
 * ワザによるダメージ後の各種効果
 */
//--------------------------------------------------------------------------
static void scproc_WazaDamageReaction( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk )
{
	switch( evwk->reaction ){
	case BTL_EV_AFTER_DAMAGED_REACTION_SICK:				///< 対象ポケモンを状態異常にする
		scproc_WazaDamageReaction_Sick( wk, evwk );
		break;
	case BTL_EV_AFTER_DAMAGED_REACTION_RANK:				///< 対象ポケモンのランク増減効果
		scproc_WazaDamageReaction_Rank( wk, evwk );
		break;
	case BTL_EV_AFTER_DAMAGED_REACTION_DAMAGE:			///< 対象ポケモンにダメージを与える
		scproc_WazaDamageReaction_Damage( wk, evwk );
		break;
	case BTL_EV_AFTER_DAMAGED_REACTION_POKETYPE:		///< 対象ポケモンのタイプ変更
		break;
	default:
		GF_ASSERT(0);
		break;
	}
}

static void scproc_WazaDamageReaction_Sick( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk )
{
	BTL_POKEPARAM* targetPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, evwk->targetPokeID );
	BTL_POKEPARAM* damagedPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, evwk->damagedPokeID );

	if( !scEvent_AddSick_IsMustFail(wk, targetPoke, evwk->sick) )
	{
		u16 sc_reserved_pos;
		u8  add_succeed_flag;
		u8  sc_reserved_flag = FALSE;

		if( evwk->tokFlag )
		{
			sc_reserved_pos = SCQUE_RESERVE_Pos( wk->que, SC_TOKWIN_IN );
			sc_reserved_flag = TRUE;
		}

		add_succeed_flag = scproc_AddSick( wk, targetPoke, damagedPoke, evwk->sick, evwk->sickCont, FALSE );

		if( add_succeed_flag )
		{
			if( sc_reserved_flag )
			{
				SCQUE_PUT_ReservedPos( wk->que, sc_reserved_pos, SC_TOKWIN_IN, evwk->damagedPokeID );
				SCQUE_PUT_TOKWIN_OUT( wk->que, evwk->damagedPokeID );
			}
		}
	}
}

static void scproc_WazaDamageReaction_Rank( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk )
{
	BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, evwk->targetPokeID );
	u16 sc_reserved_pos;
	u8 sc_reserved_flag;
	u8 succeed;

	if( evwk->tokFlag ){
		sc_reserved_pos = SCQUE_RESERVE_Pos( wk->que, SC_TOKWIN_IN );
		sc_reserved_flag = TRUE;
	}

	succeed = scproc_RankEffectCore( wk, target, evwk->rankType, evwk->rankVolume, TRUE );

	if( succeed ){
		SCQUE_PUT_ReservedPos( wk->que, sc_reserved_pos, SC_TOKWIN_IN, evwk->damagedPokeID );
		SCQUE_PUT_TOKWIN_OUT( wk->que, evwk->damagedPokeID );
	}
}

static void scproc_WazaDamageReaction_Damage( BTL_SVFLOW_WORK* wk, BTL_EVWK_DAMAGE_REACTION* evwk )
{
	BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, evwk->targetPokeID );
	u16 sc_reserved_pos;
	u8 sc_reserved_flag;
	u8 succeed;

	if( evwk->tokFlag ){
		sc_reserved_pos = SCQUE_RESERVE_Pos( wk->que, SC_TOKWIN_IN );
		sc_reserved_flag = TRUE;
	}

	succeed = scproc_SimpleDamage( wk, target, evwk->damage, STRID_NULL );

	if( succeed ){
		SCQUE_PUT_ReservedPos( wk->que, sc_reserved_pos, SC_TOKWIN_IN, evwk->damagedPokeID );
		SCQUE_PUT_TOKWIN_OUT( wk->que, evwk->damagedPokeID );
	}
}

//--------------------------------------------------------------------------
/**
 * [proc] 状態異常共通処理
 *
 * @param   wk					
 * @param   target			
 * @param   attacker		
 * @param   sick				
 * @param   sickCont		
 * @param   fAlmost			
 *
 * @retval  BOOL				状態異常にできた場合 TRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost )
{
	BOOL fItemUseTgt, fCarry, fSucceed;

	fSucceed = addsick_core( wk, target, attacker, sick, sickCont, fAlmost, &fCarry, &fItemUseTgt );
	if( fCarry )
	{
		BOOL fItemUseAtk;

		addsick_core( wk, attacker, target, sick, sickCont, TRUE, &fCarry, &fItemUseAtk );
		if( fItemUseAtk )
		{
			scproc_UseItem( wk, attacker );
		}
	}

	if( fItemUseTgt )
	{
		scproc_UseItem( wk, target );
	}

	return fSucceed;
}
//--------------------------------------------------------------------------
/**
 * 状態異常コア部分
 *
 * @param   wk					
 * @param   target						状態異常をくらうポケ
 * @param   sick							状態異常ID
 * @param   sickCont					状態異常継続パラメータ
 * @param   fAlmost						ほぼ確定フラグ
 * @param   fCarry						[out]くらった場合、相手にも同じ異常をうつすならTRUE
 * @param   fItemResponce			[out]くらった場合、アイテム使用が発生するならTRUE
 *
 * @retval  BOOL		状態異常にした場合、TRUE
 */
//--------------------------------------------------------------------------
static BOOL addsick_core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
	WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost, BOOL* fCarry, BOOL* fItemResponce )
{
	BTL_EVWK_ADDSICK* evwk = eventWork_Push( &wk->eventWork, sizeof(BTL_EVWK_ADDSICK) );
	BOOL succeed = FALSE;

	scEvent_AddSick_Check( wk, target, sick, evwk );
	*fItemResponce = FALSE;
	*fCarry = FALSE;

	if( evwk->reaction != BTL_EV_SICK_REACTION_DISCARD )
	{
		scPut_AddSick( wk, target, sick, sickCont );
		*fItemResponce = evwk->fItemResponce;
		*fCarry = (evwk->reaction == BTL_EV_SICK_REACTION_REFRECT);
		succeed = TRUE;
	}
	else
	{
		BOOL fTokWin = (fAlmost && evwk->fDiscardByTokusei);
		if( fTokWin ){
			scPut_TokWin_In( wk, target );
		}

		scPut_AddSickFail( wk, target, evwk->discardSickType );

		if( fTokWin ){
			scPut_TokWin_Out( wk, target );
		}
	}

	eventWork_Pop( &wk->eventWork, evwk );
	return succeed;
}

/*
typedef struct {

	BtlEvSickReaction		reaction;
	WazaSick						discardSickType;
	u8									fDiscardByTokusei;
	u8									discardPokeID;
	u8									fItemResponce;

}BTL_EVWK_ADDSICK;
*/
static BOOL scEvent_AddSick_IsMustFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaSick sick )
{
	// てんこう「はれ」の時に「こおり」にはならない
	if( (BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE)
	&&  (sick == POKESICK_KOORI )
	){
		return TRUE;
	}
	// すでにポケモン系状態異常になっているなら、新たにポケモン系状態異常にはならない
	if( BTL_POKEPARAM_GetPokeSick(target) != POKESICK_NULL )
	{
		return TRUE;
	}
	return FALSE;
}
static void scEvent_AddSick_Check( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
		WazaSick sick, BTL_EVWK_ADDSICK* evwk )
{
	BtlEventType event = (sick < POKESICK_MAX)? BTL_EVENT_MAKE_POKESICK : BTL_EVENT_MAKE_WAZASICK;

	evwk->reaction = BTL_EV_SICK_REACTION_NONE;
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WORK_ADRS, (int)evwk );
		BTL_EVENT_CallHandlers( wk, event );
	BTL_EVENTVAR_Pop();
}
//------------------------------------------------------------------
// サーバーフロー：アイテム使用
//------------------------------------------------------------------
static void scproc_UseItem( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
	// @@@ 未実装
}

//------------------------------------------------------------------
// サーバーフロー：ダメージ受け後の処理
//------------------------------------------------------------------
static void scproc_Fight_Damage_After( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
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
static u16 scEvent_CalcDamage( BTL_SVFLOW_WORK* wk, 
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza,
		BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag )
{
	u32 rawDamage = 0;

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
		fx32 ratio = scEvent_CalcTypeMatchRatio( wk, attacker, wk->wazaParam.wazaType );
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
static void scproc_Damage_Kickback( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 wazaDamage )
{
	u32 damage = scEvent_CalcKickBack( wk, attacker, waza, wazaDamage );
	if( damage )
	{
		scproc_SimpleDamage( wk, attacker, damage, BTL_STRID_SET_ReactionDmg );
	}
}
//---------------------------------------------------------------------------------------------
// ワザ以外のダメージ共通処理
//---------------------------------------------------------------------------------------------
static BOOL scproc_SimpleDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage, u16 strID )
{
	if( scEvent_CheckEnableSimpleDamage(wk, bpp, damage) )
	{
		int value = -damage;

		scPut_SimpleHp( wk, bpp, value );

		if( strID != STRID_NULL )
		{
			scPut_Messae_Set( wk, bpp, strID );
		}

		if( scEvent_SimpleDamage_Reaction(wk, bpp) )
		{
			scproc_UseItem( wk, bpp );
		}

		scPut_CheckDeadCmd( wk, bpp );

		// @@@ 成功したらTRUE（今は常にTRUEにしている）
		return TRUE;
	}
	return FALSE;
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ以外のダメージ有効／無効チェック
 *
 * @param   wk		
 * @param   bpp		
 * @param   damage	ダメージ量
 *
 * @retval  BOOL		有効ならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckEnableSimpleDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 damage )
{
	BOOL flag;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(bpp) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, TRUE );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_SIMPLE_DAMAGE_ENABLE );
		flag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
	BTL_EVENTVAR_Pop();

	return flag;
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ以外のダメージ後、反応チェック（現状、アイテム使用のみ）
 *
 * @param   wk		
 * @param   bpp		
 *
 * @retval  BOOL		アイテム使用する場合、TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_SimpleDamage_Reaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
	BOOL flag;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(bpp) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ITEMUSE_FLAG, TRUE );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_SIMPLE_DAMAGE_REACTION );
		flag = BTL_EVENTVAR_GetValue( BTL_EVAR_ITEMUSE_FLAG );
	BTL_EVENTVAR_Pop();

	return flag;

}


//---------------------------------------------------------------------------------------------
// サーバーフロー：追加効果による状態異常
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_AddSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
	WAZA_SICKCONT_PARAM  waza_contParam;
	WazaSick sick;
	u32 i=0;

	sick = WAZADATA_GetSick( waza, &waza_contParam );

	if(	(!BTL_POKEPARAM_IsDead(target))
	&&	(!scEvent_AddSick_IsMustFail(wk, target, sick))
	){
		if( scEvent_CheckAddSickPer(wk, waza, attacker, target) )
		{
			BPP_SICK_CONT sickCont;
			BTL_CALC_WazaSickContToBppSickCont( waza_contParam, attacker, &sickCont );
			scproc_AddSick( wk, target, attacker, sick, sickCont, FALSE );
		}
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接の状態異常
//---------------------------------------------------------------------------------------------
static void scproc_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec )
{
	BTL_POKEPARAM* target;
	WAZA_SICKCONT_PARAM waza_contParam;
	WazaSick sick;
	u32 i = 0;

	sick = WAZADATA_GetSick( waza, &waza_contParam );

	while( (target = TargetPokeRec_GetNext(targetRec)) != NULL )
	{
		if( scEvent_AddSick_IsMustFail(wk, target, sick) ){
			TargetPokeRec_Remove( targetRec, target );
		}
	}

	if( TargetPokeRec_GetCount(targetRec) )
	{
		TargetPokeRec_GetStart( targetRec );
		while( (target = TargetPokeRec_GetNext(targetRec)) != NULL )
		{
			if( scEvent_checkHit(wk, attacker, target, waza) )
			{
				BPP_SICK_CONT sickCont;
				BTL_CALC_WazaSickContToBppSickCont( waza_contParam, attacker, &sickCont );
				if( scproc_AddSick( wk, target, attacker, sick, sickCont, TRUE ) )
				{
					wazaEff_SetTarget( wk, target );
				}
				else
				{
					// @@@ 効果がない理由を言わねばならん…
				}
			}
			else
			{
				scPut_WazaAvoid( wk, target, waza );
			}
		}
	}
	else
	{
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：追加効果によるランク効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_AddEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
	if( !BTL_POKEPARAM_IsDead(target) )
	{
		if( scEvent_CheckAddRankEffectOccur(wk, waza, attacker, target) )
		{
			scproc_WazaRankEffect_Common( wk, waza, attacker, target, FALSE );
		}
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接のランク効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec )
{
	BTL_POKEPARAM* target;
	u32 i = 0;

	TargetPokeRec_GetStart( targetRec );
	while( (target = TargetPokeRec_GetNext(targetRec)) != NULL )
	{
		if( scEvent_checkHit(wk, attacker, target, waza) )
		{
			if( scproc_WazaRankEffect_Common(wk, waza, attacker, target, TRUE) )
			{
				wazaEff_SetTarget( wk, target );
			}
		}
		else
		{
			scPut_WazaAvoid( wk, target, waza );
		}
	}
}
//--------------------------------------------------------------------------
/**
 * ワザによる（直接・追加共通）ランク増減効果の処理
 *
 * @param   wk				
 * @param   waza			
 * @param   attacker	
 * @param   target		
 *
 * @retval  BOOL		ランク増減効果が発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_WazaRankEffect_Common( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL fAlmost )
{
	BOOL ret = FALSE;
	u32 eff_cnt, i;

	eff_cnt = WAZADATA_GetRankEffectCount( waza );
	for(i=0; i<eff_cnt; ++i)
	{
		WazaRankEffect  effect;
		int  volume;

		scEvent_GetWazaRankEffectValue( wk, waza, i, attacker, target, &effect, &volume );
		if( scproc_RankEffectCore(wk, target, effect, volume, fAlmost) )
		{
			ret = TRUE;
		}
	}
	return ret;
}
//--------------------------------------------------------------------------
/**
 * ランク増減効果のコア（ワザ以外もこれを呼び出す）
 *
 * @param   wk		
 * @param   target		
 * @param   effect		
 * @param   volume		
 * @param   fAlmost		
 *
 * @retval  BOOL		ランク増減効果が発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_RankEffectCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume, BOOL fAlmost )
{
	if( !BTL_POKEPARAM_IsRankEffectValid(target, effect, volume) )
	{
		if( fAlmost )
		{
			scPut_RankEffectLimit( wk, target, effect, volume );
			return FALSE;
		}
	}

	{
		BTL_EVWK_CHECK_RANKEFF* evwk = (BTL_EVWK_CHECK_RANKEFF*)eventWork_Push( &wk->eventWork, sizeof(BTL_EVWK_CHECK_RANKEFF) );
		BOOL ret = TRUE;

		if( scEvent_CheckRankEffectSuccess(wk, target, effect, volume, evwk) )
		{
			BTL_Printf("ランク効果：type=%d, volume=%d\n", effect, volume );
			scPut_RankEffect( wk, target, effect, volume );
		}
		else
		{
			if( fAlmost )
			{
				if( evwk->failTokuseiFlag ){
					scPut_TokWin_In( wk, target );
				}
				scPut_RankEffectFail( wk, target, effect, evwk );
				if( evwk->failTokuseiFlag ){
					scPut_TokWin_Out( wk, target );
				}
			}
			ret = FALSE;
		}

		eventWork_Pop( &wk->eventWork, evwk );
		return ret;
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザダメージに応じたHP吸い取り効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Drain( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage )
{
	BTL_POKEPARAM* target;
	int value;
	u32 i;
	u8 atkPokeID;

	atkPokeID = BTL_POKEPARAM_GetID( attacker );

	value = scEvent_CalcDrainVolume( wk, waza, attacker, defender, damage );
	if( value > 0 )
	{
		scPut_SimpleHp( wk, attacker, value );
	}
	else
	{
		scproc_SimpleDamage( wk, attacker, -value, STRID_NULL );
	}
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：一撃ワザ処理
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targets )
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
				scput_WazaNoEffect( wk, target );
			}
			else
			{
				if( scEvent_IchigekiCheck(wk, attacker, target, waza) )
				{
					wazaEff_SetTarget( wk, target );
					scPut_Ichigeki( wk, target );
				}
				else
				{
					scPut_WazaAvoid( wk, target, waza );
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
// サーバーフロー：ふきとばしワザ処理
//---------------------------------------------------------------------------------------------
static void scproc_Fight_PushOut( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, TARGET_POKE_REC* targetRec )
{
	PushOutEffect eff = check_pushout_effect( wk );

	if( eff == PUSHOUT_EFFECT_MUSTFAIL ){
		scPut_WazaFail( wk, attacker, waza );
	}else{

		BTL_POKEPARAM* target = TargetPokeRec_Get( targetRec, 0 );

		if( BTL_POKEPARAM_IsDead(target) ){
			scPut_WazaFail( wk, attacker, waza );
			return;
		}
		if( scEvent_CheckPushOutFail(wk, attacker, target) ){
			return;
		}
		if( !scEvent_checkHit(wk, attacker, target, waza) ){
			scPut_WazaAvoid( wk, attacker, waza );
		}

		{
			BtlPokePos targetPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, BTL_POKEPARAM_GetID(target) );
			u8 clientID, posIdx;
			BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, targetPos, &clientID, &posIdx );

			if( eff == PUSHOUT_EFFECT_CHANGE )
			{
				SVCL_WORK* clwk;
				clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
				if( BTL_PARTY_GetMemberCount(clwk->party) <= clwk->numCoverPos )
				{
					scPut_WazaFail( wk, attacker, waza );
					return;
				}
				else
				{
					u8 nextPokeIdx = get_pushout_nextpoke_idx( wk, clwk );
					SCQUE_PUT_ACT_MemberOut( wk->que, clientID, posIdx );
					scproc_MemberIn( wk, clientID, posIdx, nextPokeIdx, FALSE );
				}
			}
			else
			{
				SCQUE_PUT_ACT_MemberOut( wk->que, clientID, posIdx );
				wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
			}
		}
	}
}
//--------------------------------------------------------------------------
/**
 * 「ふきとばし」系のワザ効果をバトルのルール等から判定する
 *
 * @retval  BOOL	必ず失敗するケースでTRUE
 */
//--------------------------------------------------------------------------
static PushOutEffect check_pushout_effect( BTL_SVFLOW_WORK* wk )
{
	BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );
	BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk->mainModule );

	switch( rule ){
	case BTL_COMPETITOR_WILD:
		return (competitor == BTL_COMPETITOR_WILD)? PUSHOUT_EFFECT_ESCAPE : PUSHOUT_EFFECT_MUSTFAIL;

	default:
		return PUSHOUT_EFFECT_CHANGE;
	}
}
//--------------------------------------------------------------------------
/**
 * 「ふきとばし」系のワザで強制入れかえが発生した時、
 * 次に出すポケモンのパーティ内インデックスをランダムで決める
 *
 * @param   wk		
 * @param   clwk		
 *
 * @retval  u8		
 */
//--------------------------------------------------------------------------
static u8 get_pushout_nextpoke_idx( BTL_SVFLOW_WORK* wk, const SVCL_WORK* clwk )
{
	const BTL_POKEPARAM* bpp;
	u32 i, count, members;
	u8 list[ BTL_PARTY_MEMBER_MAX ];

	members = BTL_PARTY_GetMemberCount( clwk->party );
	for(i=clwk->numCoverPos, count=0; i<members; ++i)
	{
		bpp = BTL_PARTY_GetMemberDataConst( clwk->party, i );
		if( !BTL_POKEPARAM_IsDead(bpp) ){
			list[count++] = i;
		}
	}
	GF_ASSERT( count );
	return clwk->numCoverPos + GFL_STD_MtRand( count );
}

//--------------------------------------------------------------------------
/**
 * [Event] ふきとばしワザ失敗チェック
 *
 * @param   wk				
 * @param   attacker	
 * @param   target		
 *
 * @retval  BOOL			失敗ならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckPushOutFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target )
{
	BOOL failFlag = FALSE;
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, failFlag );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_PUSHOUT );
		failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
	BTL_EVENTVAR_Pop();
	return failFlag;
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる天候の変化
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Weather( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, const BTL_ACTION_PARAM* action )
{
	BtlWeather  weather = WAZADATA_GetWeather( waza );

	if( scEvent_ChangeWeather( wk, weather, BTL_WEATHER_TURN_DEFAULT ) )
	{
		scPut_WazaEffectOn( wk, attacker, attacker, waza );
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
		scPut_WazaEffectOn( wk, attacker, target, WAZANO_SUKIRUSUWAPPU );
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

	if( wk->turnCount < BTL_TURNCOUNT_MAX ){
		wk->turnCount++;
	}
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
		scPut_CheckDeadCmd( wk, bpp );
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
				scPut_CheckDeadCmd( wk, BTL_POKECON_GetPokeParam(wk->pokeCon, pokeID[i]) );
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
//	数値関連関数群
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
//	イベントワーク確保
//---------------------------------------------------------------------------------------------
static void eventWork_Init( BTL_EVENT_WORK_STACK* stack )
{
	stack->sp = 0;
}
static void* eventWork_Push( BTL_EVENT_WORK_STACK* stack, u32 size )
{
	GF_ASSERT( stack->sp < EVENT_WORK_DEPTH );

	while( size % 4 ){ ++size; }

	{
		u32 usedSize;
		u32 i;

		for(i=0, usedSize=0; i<stack->sp; ++i)
		{
			usedSize += stack->size[ i ];
		}

		if( (usedSize + size) < sizeof(stack->work) )
		{
			void* adrs;
			adrs = &stack->work[ usedSize ];
			stack->size[ stack->sp ] = size;
			stack->sp++;
			return adrs;
		}
		else
		{
			GF_ASSERT(0);
			return NULL;
		}
	}
}

static void eventWork_Pop( BTL_EVENT_WORK_STACK* stack, void* adrs )
{
	GF_ASSERT( stack->sp );

	stack->sp--;

	{
		u32 used_size, i;

		for(i=0, used_size=0; i<stack->sp; ++i)
		{
			used_size += stack->size[ i ];
		}

		if( adrs != &(stack->work[used_size]) )
		{
			GF_ASSERT(0);	// 最後に確保されたワーク以外の領域が先に解放されようとしている
		}
	}
}
//---------------------------------------------------------------------------------------------
//
// コマンド出力層
// サーバーコマンドをバッファに生成 ＆ 必要ならそれに伴うポケモンパラメータ操作を行う
// イベント層の関数呼び出しは行われない
//
//---------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
 * 「○○はこんらんしている」表示
 */
//--------------------------------------------------------------------------
static void scPut_ConfCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
	SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KonranAct, BTL_POKEPARAM_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * こんらんによる自爆ダメージ処理
 */
//--------------------------------------------------------------------------
static void scPut_ConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage )
{
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );

	BTL_POKEPARAM_HpMinus( bpp, damage );
	SCQUE_PUT_OP_HpMinus( wk->que, pokeID, damage );
	SCQUE_PUT_ACT_ConfDamage( wk->que, pokeID );

	scPut_CheckDeadCmd( wk, bpp );
}
//--------------------------------------------------------------------------
/**
 * ワザ出し判定時のポケモン系状態異常回復処理
 */
//--------------------------------------------------------------------------
static void scPut_CurePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );
	PokeSick sick = BTL_POKEPARAM_GetPokeSick( bpp );

	BTL_POKEPARAM_CurePokeSick( bpp );
	SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );

	switch( sick ){
	case POKESICK_NEMURI:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NemuriWake, pokeID );
		break;
	case POKESICK_KOORI:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriMelt, pokeID );
		break;
	default:
		GF_ASSERT_MSG(0, "illegal sick ID=%d\n", sick);
	}
}
//--------------------------------------------------------------------------
/**
 * ワザ出し失敗メッセージ表示
 */
//--------------------------------------------------------------------------
static void scPut_WazaExecuteFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause )
{
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );

	switch( cause ){
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
	case SV_WAZAFAIL_NAMAKE:
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Namake, pokeID );
		break;
	default:
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail, pokeID );
		break;
	}
}
//--------------------------------------------------------------------------
/**
 * 「しかしうまくきまらなかった」表示
 */
//--------------------------------------------------------------------------
static void scPut_WazaFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
	SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
}
//--------------------------------------------------------------------------
/**
 * 「○○には当たらなかった」表示
 */
//--------------------------------------------------------------------------
static void scPut_WazaAvoid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza )
{
	SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_WazaAvoid, BTL_POKEPARAM_GetID(defender) );
}
//--------------------------------------------------------------------------
/**
 * 「○○にはこうかがないようだ」表示
 */
//--------------------------------------------------------------------------
static void scput_WazaNoEffect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender )
{
	SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BTL_POKEPARAM_GetID(defender) );
}
//--------------------------------------------------------------------------
/**
 * [Put] ダメージ受け->回復化処理
 */
//--------------------------------------------------------------------------
static void scput_DmgToRecover( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BTL_EVWK_DMG_TO_RECOVER* evwk )
{
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );

	if( evwk->tokFlag ){
		SCQUE_PUT_TOKWIN_IN( wk->que, pokeID );
	}

	if( evwk->recoverHP != 0 )
	{
		wazaEff_SetTarget( wk, bpp );
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_SET_HP_Recover, pokeID );
		BTL_POKEPARAM_HpPlus( bpp, evwk->recoverHP );
		SCQUE_PUT_OP_HpPlus( wk->que, pokeID, evwk->recoverHP );
		SCQUE_PUT_ACT_SimpleHP( wk->que, evwk->pokeID );
	}
	else
	{
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_SET_NoEffect, pokeID );
	}

	if( evwk->tokFlag ){
		SCQUE_PUT_TOKWIN_OUT( wk->que, evwk->pokeID );
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] 対象ポケモンが死んでいたら死亡処理＆必要コマンド生成
 */
//--------------------------------------------------------------------------
static void scPut_CheckDeadCmd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke )
{
	if( !BTL_POKEPARAM_GetContFlag(poke, BPP_CONTFLG_DEAD_IGNORE) )
	{
		if( BTL_POKEPARAM_IsDead(poke) )
		{
			u8 pokeID = BTL_POKEPARAM_GetID( poke );

			BTL_HANDLER_TOKUSEI_Remove( poke );

			BTL_POKEPARAM_SetContFlag( poke, BPP_CONTFLG_DEAD_IGNORE );
			BTL_POKEPARAM_SetTurnFlag( poke, BPP_TURNFLG_DEAD );

			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Dead, pokeID );
			SCQUE_PUT_ACT_Dead( wk->que, pokeID );
		}
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ワザエフェクト発動
 */
//--------------------------------------------------------------------------
static void scPut_WazaEffectOn( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaID waza )
{
	// １つのワザ処理ごとに１回しか作らない
	if( FlowFlg_Get(wk, FLOWFLG_SET_WAZAEFFECT ) == FALSE )
	{
		u8 atPokeID = BTL_POKEPARAM_GetID( attacker );
		u8 defPokeID = BTL_POKEPARAM_GetID( target );
		BTL_Printf("ワザエフェクトコマンド生成しますよ, ワザナンバ%d\n", waza);
		SCQUE_PUT_ACT_WazaEffect( wk->que, atPokeID, defPokeID, waza );
		FlowFlg_Set( wk, FLOWFLG_SET_WAZAEFFECT );
	}
	else
	{
		BTL_Printf("ワザエフェクトコマンド生成済みなので無視, ワザナンバ%d\n", waza);
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ワザによるダメージ（単体）
 */
//--------------------------------------------------------------------------
static void scPut_WazaDamageSingle( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* defender, BtlTypeAff aff, u32 damage, BOOL critical_flag )
{
	u8 pokeID = BTL_POKEPARAM_GetID( defender );

	BTL_POKEPARAM_HpMinus( defender, damage );
	SCQUE_PUT_OP_HpMinus( wk->que, pokeID, damage );
	SCQUE_PUT_ACT_WazaDamage( wk->que, pokeID, aff, damage );
	if( critical_flag )
	{
		SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_CriticalHit );
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ワザによるダメージ（複数体）
 */
//--------------------------------------------------------------------------
static void scPut_WazaDamagePlural( BTL_SVFLOW_WORK* wk, u32 poke_cnt, BtlTypeAff aff, 
	BTL_POKEPARAM** bpp, const u16* damage, const u8* critical_flag )
{
	u32 i;

	for(i=0; i<poke_cnt; ++i)
	{
		BTL_POKEPARAM_HpMinus( bpp[i], damage[i] );
		SCQUE_PUT_OP_HpMinus( wk->que, BTL_POKEPARAM_GetID(bpp[i]), damage[i] );
	}

	SCQUE_PUT_ACT_WazaDamagePlural( wk->que, poke_cnt, aff );
	for(i=0; i<poke_cnt; ++i)
	{
		SCQUE_PUT_ArgOnly( wk->que, BTL_POKEPARAM_GetID(bpp[i]) );
	}
	for(i=0; i<poke_cnt; ++i)
	{
		if( critical_flag[i] ){
			SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_CriticalHit, BTL_POKEPARAM_GetID(bpp[i]) );
		}
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ワザによるダメージを与えた後の反動計算
 */
//--------------------------------------------------------------------------
static u32 scEvent_CalcKickBack( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u32 damage )
{
	u8 ratio = WAZADATA_GetReactionRatio( waza );
	u8 pokeID = BTL_POKEPARAM_GetID( attacker );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, ratio );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, pokeID );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_REACTION );
		ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
	BTL_EVENTVAR_Pop();

	if( ratio )
	{
		damage = roundMin( (damage*ratio)/100, 1 );
		return damage;
	}
	return 0;
}
//--------------------------------------------------------------------------
/**
 * [Put] とくせいウィンドウ表示開始
 */
//--------------------------------------------------------------------------
static void scPut_TokWin_In( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
	SCQUE_PUT_TOKWIN_IN( wk->que, BTL_POKEPARAM_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * [Put] とくせいウィンドウ消去開始
 */
//--------------------------------------------------------------------------
static void scPut_TokWin_Out( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
	SCQUE_PUT_TOKWIN_OUT( wk->que, BTL_POKEPARAM_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * [Put] これ以上、能力ランクがあがらない（さがらない）メッセージ
 */
//--------------------------------------------------------------------------
static void scPut_RankEffectLimit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect effect, int volume )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );

	if( volume > 0 ){
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_RankupMax_ATK, pokeID, effect );
	}else{
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_RankdownMin_ATK, pokeID, effect );
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ランク増減効果が発生
 */
//--------------------------------------------------------------------------
static void scPut_RankEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );

	if( volume > 0 )
	{
		volume = BTL_POKEPARAM_RankUp( target, effect, volume );
		SCQUE_PUT_OP_RankUp( wk->que, pokeID, effect, volume );
		SCQUE_PUT_ACT_RankUp( wk->que, pokeID, effect, volume );
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankup_ATK, pokeID, effect, volume );
	}
	else
	{
		volume *= -1;
		volume = BTL_POKEPARAM_RankDown( target, effect, volume );
		BTL_Printf(" RANK DOWN vol=%d\n", volume);
		SCQUE_PUT_OP_RankDown( wk->que, pokeID, effect, volume );
		SCQUE_PUT_ACT_RankDown( wk->que, pokeID, effect, volume );
		SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankdown_ATK, pokeID, effect, volume );
	}
}
//--------------------------------------------------------------------------
/**
 * [Put] ランク増減効果失敗メッセージ
 */
//--------------------------------------------------------------------------
static void scPut_RankEffectFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
	WazaRankEffect effect, const BTL_EVWK_CHECK_RANKEFF* evwk )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );
	u16 strID = BTL_STRID_SET_RankdownFail;

	if( effect == evwk->failSpecificType )
	{
		switch( effect ){
		case WAZA_RANKEFF_ATTACK:	strID = BTL_STRID_SET_RankdownFail_ATK; break;
		case WAZA_RANKEFF_HIT:		strID = BTL_STRID_SET_RankdownFail_HIT; break;
		}
	}
	BTL_SERVER_RECTPT_SetMessage( wk, strID, pokeID );
}
//--------------------------------------------------------------------------
/**
 * [Put] 一撃必殺ワザ成功
 */
//--------------------------------------------------------------------------
static void scPut_Ichigeki( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );
	BTL_POKEPARAM_HpZero( target );
	SCQUE_PUT_OP_HpZero( wk->que, pokeID );
	SCQUE_PUT_ACT_WazaIchigeki( wk->que, pokeID );
}
//--------------------------------------------------------------------------
/**
 * [Put] シンプルHP増減エフェクト
 */
//--------------------------------------------------------------------------
static void scPut_SimpleHp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, int value )
{
	u8 pokeID = BTL_POKEPARAM_GetID( bpp );

	if( value > 0 )
	{
		BTL_POKEPARAM_HpPlus( bpp, value );
		SCQUE_PUT_OP_HpPlus( wk->que, pokeID, value );
	}
	else if( value < 0 )
	{
		value *= -1;
		BTL_POKEPARAM_HpMinus( bpp, value );
		SCQUE_PUT_OP_HpMinus( wk->que, pokeID, value );
	}
	SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
}
//--------------------------------------------------------------------------
/**
 * [Put] 状態異常化
 */
//--------------------------------------------------------------------------
static void scPut_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaSick sick, BPP_SICK_CONT sickCont )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );

	BTL_POKEPARAM_SetWazaSick( target, sick, sickCont );
	SCQUE_PUT_OP_SetSick( wk->que, pokeID, sick, sickCont.raw );
	SCQUE_PUT_ACT_SickSet( wk->que, pokeID, sick );
}
static void scPut_AddSickFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaSick sick )
{
	u8 pokeID = BTL_POKEPARAM_GetID( target );
	SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, pokeID );
}
//--------------------------------------------------------------------------
/**
 * [Put] セットメッセージ表示
 */
//--------------------------------------------------------------------------
static void scPut_Messae_Set( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 strID )
{
	SCQUE_PUT_MSG_SET( wk->que, strID, BTL_POKEPARAM_GetID(bpp) );
}


//---------------------------------------------------------------------------------------------
//
// 【イベント層】
//
// メイン処理層での判定に必要な各種条件の決定を行う。
// ハンドラ類の呼び出しはイベント層からのみ行われる。
// ポケモンパラメータの操作は行わない。
// コマンド出力は行わない。
//
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
		return BTL_CALC_IsOccurPer( BTL_CONF_EXE_RATIO );
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
 * @param   attacker		攻撃ポケモンパラメータ
 * @param   waza				出そうとしているワザ
 * @param   curedFlag		[out] ねむり・まひ・こおり が回復した場合TRUE／それ以外FALSEが入る
 *
 * @retval  SV_WazaFailCause		
 */
//--------------------------------------------------------------------------
static SV_WazaFailCause scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
	WazaID waza, u8* curedFlag )
{
	*curedFlag = FALSE;

	BTL_EVENTVAR_Push();

	BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_NULL );
	BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
	do {
		WazaSick sick = BTL_POKEPARAM_GetPokeSick( attacker );
		u8 pokeID = BTL_POKEPARAM_GetID( attacker );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, pokeID );

		// 状態異常による失敗チェック
		switch( sick ){
		case POKESICK_NEMURI:
			if( !BTL_POKEPARAM_Nemuri_CheckWake(attacker) ){
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_NEMURI );
			}else{
				*curedFlag = TRUE;
			}
			break;
		case POKESICK_MAHI:
			if( GFL_STD_MtRand(100) < BTL_MAHI_EXE_RATIO )
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_MAHI );
			}
			break;
		case POKESICK_KOORI:
			if( GFL_STD_MtRand(100) >= BTL_KORI_MELT_RATIO ){
				BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_KOORI );
			}else{
				*curedFlag = TRUE;
			}
			break;
		}
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_CAUSE) != SV_WAZAFAIL_NULL )
		{
			break;
		}
		// ひるみによる失敗チェック
		if( BTL_POKEPARAM_GetTurnFlag(attacker, BPP_TURNFLG_SHRINK) )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_SHRINK );
			break;
		}

	}while(0);

	BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXECUTE );
	{
		SV_WazaFailCause cause = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_CAUSE );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXECUTE_FIX );
		BTL_EVENTVAR_Pop();
		return cause;
	}
}
//--------------------------------------------------------------------------
/**
 * [Event] 発動するワザのパラメータを構造体にセット
 *
 * @param   wk		
 * @param   waza		
 * @param   attacker		
 * @param   [out] param
 *
 */
//--------------------------------------------------------------------------
static void scEvent_CheckWazaParam( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* param )
{
	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_TYPE, WAZADATA_GetType(waza) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_USER_TYPE, BTL_POKEPARAM_GetPokeType(attacker) );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_PARAM );
		param->wazaType = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
		param->userType = BTL_EVENTVAR_GetValue( BTL_EVAR_USER_TYPE );
	BTL_EVENTVAR_Pop();
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
	PokeType waza_type = wk->wazaParam.wazaType;
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
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, WazaID waza, u8 lv,
		const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
	BOOL fNoEffect = FALSE;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_NOEFFECT_FLAG, fNoEffect );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_NOEFFECT_CHECK_L1+lv );
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
static BOOL scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza, BTL_EVWK_DMG_TO_RECOVER* evwk )
{
	BOOL ret = FALSE;

	evwk->pokeID = BTL_POKEPARAM_GetID( defender );
	evwk->recoverHP = 0;
	evwk->tokFlag = FALSE;
	evwk->rankEffect = WAZA_RANKEFF_NULL;
	evwk->rankVolume = 0;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, evwk->pokeID );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WORK_ADRS, (int)evwk );
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
 * [Event] ワザダメージ受けた後の反応処理
 *
 * @param   wk						
 * @param   attacker			
 * @param   defender			
 * @param   waza					
 * @param   aff						
 * @param   damage				
 * @param   criticalFlag	
 * @param   evwk					[out] 反応詳細情報
 *
 * @retval  BOOL		アイテム反応フラグ（TRUEならアイテムを使う）
 */
//--------------------------------------------------------------------------
static BOOL scEvent_WazaDamageReaction( BTL_SVFLOW_WORK* wk,
	BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, WazaID waza, BtlTypeAff aff,
	u32 damage, BOOL criticalFlag, BTL_EVWK_DAMAGE_REACTION* evwk )
{
	u8 defPokeID = BTL_POKEPARAM_GetID( defender );
	u8 item_use_flag = FALSE;

	evwk->reaction = BTL_EV_AFTER_DAMAGED_REACTION_NONE;
	evwk->damagedPokeID = defPokeID;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, defPokeID );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_TYPE, wk->wazaParam.wazaType );
		BTL_EVENTVAR_SetValue( BTL_EVAR_CRITICAL_FLAG, criticalFlag );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ITEMUSE_FLAG, item_use_flag );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WORK_ADRS, (int)evwk );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_AFTER );
		item_use_flag = BTL_EVENTVAR_GetValue( BTL_EVAR_ITEMUSE_FLAG );
	BTL_EVENTVAR_Pop();

	return item_use_flag;
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
static BtlTypeAff scEvent_checkWazaDamageAffinity( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, PokeType waza_type )
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
//--------------------------------------------------------------------------
/**
 * [Event] メンバーを場に登場させる
 *
 * @param   wk		
 * @param   bpp		
 *
 */
//--------------------------------------------------------------------------
static void scEvent_MemberIn( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
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
 * @param   waza				
 * @param   attacker		
 * @param   defender		
 *
 * @retval  BOOL		発生する場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckAddSickPer( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
	WAZA_SICKCONT_PARAM  waza_contParam;
	WazaSick sick = WAZADATA_GetSick( waza, &waza_contParam );
	u8 per = WAZADATA_GetSickPer( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(defender) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_SICK );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
	BTL_EVENTVAR_Pop();

	return  perOccur(per);
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
 *  ワザ直接効果なら常にTRUE。追加効果なら効果確率100の時にTRUE。とくせい効果の場合は常にFALSE。
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
 *  ワザ直接効果なら常にTRUE。追加効果なら効果確率100の時にTRUE。とくせい効果の場合は常にFALSE。
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
 * 【Event】ワザによる追加ランク効果の発生チェック
 *
 * @param   wk		
 * @param   attacker		
 * @param   target		
 * @param   waza		
 *
 * @retval  BOOL		発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckAddRankEffectOccur( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
	u8 per = WAZADATA_GetRankEffPer( waza );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_RANK_TARGET );
		per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
	BTL_EVENTVAR_Pop();

	return perOccur(per);
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザによる（直接・追加）ランク増減効果を取得
 *
 * @param   wk				
 * @param   waza			
 * @param   idx				
 * @param   attacker	
 * @param   target		
 * @param   effect		[out] 効果種類
 * @param   volume		[out] 効果値
 *
 */
//--------------------------------------------------------------------------
static void scEvent_GetWazaRankEffectValue( BTL_SVFLOW_WORK* wk, WazaID waza, u32 idx,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaRankEffect* effect, int* volume )
{
	*effect = WAZADATA_GetRankEffect( waza, idx, volume );

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_STATUS_TYPE, *effect );
		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, *volume );
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, FALSE );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_GET_RANKEFF_VALUE );
		*volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
	BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] ランク増減効果の最終成否チェック
 *
 * @param   wk				
 * @param   target		
 * @param   effect		
 * @param   volume		
 * @param   evwk			[out]
 *
 * @retval  BOOL			成功する場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckRankEffectSuccess( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
	WazaRankEffect effect, int volume, BTL_EVWK_CHECK_RANKEFF* evwk )
{
	evwk->failFlag = FALSE;
	evwk->failTokuseiFlag = FALSE;
	evwk->failSpecificType = WAZA_RANKEFF_NULL;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_STATUS_TYPE, effect );
		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );
		BTL_EVENTVAR_SetValue( BTL_EVAR_WORK_ADRS, (int)evwk );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_GET_RANKEFF_VALUE );
	BTL_EVENTVAR_Pop();

	return evwk->failFlag == FALSE;
}//--------------------------------------------------------------------------
/**
 * [Event] ドレイン攻撃のドレインHP量を計算
 *
 * @param   wk		
 * @param   waza		
 * @param   attacker		
 * @param   target		
 * @param   damage		与えたダメージ
 *
 * @retval  int			回復量（マイナスなら逆にダメージを受ける）
 */
//--------------------------------------------------------------------------
static int scEvent_CalcDrainVolume( BTL_SVFLOW_WORK* wk, WazaID waza,
	const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, int damage )
{
	int volume = damage * WAZADATA_GetDrainRatio(waza) / 100;

	BTL_EVENTVAR_Push();
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BTL_POKEPARAM_GetID(attacker) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEPARAM_GetID(target) );
		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );
		BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_DRAIN_VOLUME );
		volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
	BTL_EVENTVAR_Pop();
	return volume;
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
			BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHANGE_AFTER );
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
 * 現在の経過ターン数を返す
 *
 * @param   wk		
 *
 * @retval  u16		
 */
//=============================================================================================
u16 BTL_SVFLOW_GetTurnCount( BTL_SVFLOW_WORK* wk )
{
	return wk->turnCount;
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
void BTL_SERVER_RECTPT_SetMessageEx( BTL_SVFLOW_WORK* wk, u16 msgID, u8 pokeID, int arg )
{
	SCQUE_PUT_MSG_SET( wk->que, msgID, pokeID, arg );
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
void BTL_SERVER_RECEPT_RankDownEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 val, BOOL fAlmost )
{
	BTL_POKEPARAM* pp;
	u8 targetPos[ BTL_POSIDX_MAX ];
	u8 numPokemons, i;
	int volume = val * -1;

	numPokemons = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, targetPos );
	for(i=0; i<numPokemons; ++i)
	{
		pp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos[i] );
		if( !BTL_POKEPARAM_IsDead(pp) )
		{
			scproc_RankEffectCore( wk, pp, statusType, volume, fAlmost );
		}
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
void BTL_SERVER_RECEPT_RankUpEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume, BOOL fAlmost )
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
			scproc_RankEffectCore( wk, pp, statusType, volume, fAlmost );
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
 * [ハンドラ受信] ポケモンのタイプ変化（純粋タイプのみ）
 *
 * @param   wk			
 * @param   pokeID	
 * @param   type		
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_ChangePokeType( BTL_SVFLOW_WORK* wk, u8 pokeID, PokeType type )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
	SCQUE_PUT_OP_ChangePokeType( wk->que, pokeID, type );
	BTL_POKEPARAM_ChangePokeType( bpp, PokeTypePair_MakePure(type) );
}
//=============================================================================================
/**
 * [ハンドラ受信] ポケモンのフォルム変化
 *
 * @param   wk			
 * @param   pokeID	
 * @param   formNo	
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_ChangePokeForm( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 formNo )
{
	BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
	SCQUE_PUT_OP_ChangePokeForm( wk->que, pokeID, formNo );
	BTL_POKEPARAM_ChangeForm( bpp, formNo );
}
//=============================================================================================
/**
 * [ハンドラ受信] 逃げ交換禁止コードの追加を全クライアントに通知
 *
 * @param   wk						
 * @param   clientID			禁止コードを発行したポケモンID
 * @param   code					禁止コード
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_CantEscapeAdd( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code )
{
	SCQUE_PUT_OP_CantEscape_Add( wk->que, pokeID, code );
}
//=============================================================================================
/**
 * [ハンドラ受信] 逃げ交換禁止コードの削除を全クライアントに通知
 *
 * @param   wk						
 * @param   clientID			禁止コードを発行したポケモンID
 * @param   code					禁止コード
 *
 */
//=============================================================================================
void BTL_SVFLOW_RECEPT_CantEscapeSub( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code )
{
	SCQUE_PUT_OP_CantEscape_Sub( wk->que, pokeID, code );
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

