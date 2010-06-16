//=============================================================================================
/**
 * @file  btl_server_flow.c
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理
 * @author  taya
 *
 * @date  2009.03.06  作成
 */
//=============================================================================================
#include <gflib.h>

#include "sound\pm_sndsys.h"
#include "waza_tool\wazadata.h"
#include "waza_tool\wazano_def.h"
#include "item\item.h"
#include "item\itemtype_def.h"
#include "poke_tool\shinka_check.h"
#include "gamesystem\g_power.h"

#include "btl_event.h"
#include "btl_server_cmd.h"
#include "btl_field.h"
#include "btl_sick.h"
#include "btl_pospoke_state.h"
#include "btl_pokeset.h"
#include "btl_shooter.h"
#include "btl_calc.h"
#include "btl_tables.h"
#include "btlv\btlv_effect.h"

#include "handler\hand_tokusei.h"
#include "handler\hand_item.h"
#include "handler\hand_waza.h"
#include "handler\hand_side.h"
#include "handler\hand_pos.h"

#include "btl_server_local.h"
#include "btl_server.h"
#include "btl_server_flow_sub.h"
#include "btl_hem.h"

#include "btl_server_flow.h"
#include "btl_server_flow_local.h"

#include "debug/debug_hudson.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void clearWorks( BTL_SVFLOW_WORK* wk );
static void clearPokeCantActionFlag( BTL_SVFLOW_WORK* wk );
static u32 ActOrderProc_Main( BTL_SVFLOW_WORK* wk, u32 startOrderIdx );
static BOOL ActOrderProc_OnlyPokeIn( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction );
static BOOL scproc_CheckShowdown( BTL_SVFLOW_WORK* wk );
static BOOL CheckPlayerSideAlive( BTL_SVFLOW_WORK* wk );
static void scproc_countup_shooter_energy( BTL_SVFLOW_WORK* wk );
static BOOL reqChangePokeForServer( BTL_SVFLOW_WORK* wk, CLIENTID_REC* rec );
static void scproc_BeforeFirstFight( BTL_SVFLOW_WORK* wk );
static inline u32 ActPri_Make( u8 actPri, u8 wazaPri, u8 spPri, u16 agility );
static inline u8 ActPri_GetWazaPri( u32 priValue );
static inline u8 ActPri_GetSpPri( u32 priValue );
static inline u32 ActPri_SetSpPri( u32 priValue, u8 spPri );
static u8 sortClientAction( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction, ACTION_ORDER_WORK* order, u32 orderMax );
static void sortActionSub( ACTION_ORDER_WORK* order, u32 numOrder );
static u8 scEvent_GetWazaPriority( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* bpp );
static u16 scEvent_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BOOL fTrickRoomEnable );
static inline WazaID ActOrder_GetWazaID( const ACTION_ORDER_WORK* wk );
static ACTION_ORDER_WORK* ActOrderTool_SearchByPokeID( BTL_SVFLOW_WORK* wk, u8 pokeID );
static ACTION_ORDER_WORK* ActOrderTool_SearchByWazaID( BTL_SVFLOW_WORK* wk, WazaID waza, u8 startIndex );
static ACTION_ORDER_WORK* ActOrderTool_SearchForCombiWaza( BTL_SVFLOW_WORK* wk, WazaID waza, u8 pokeID, BtlPokePos targetPos );
static u8 ActOrderTool_GetIndex( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder );
static int ActOrderTool_Intr( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder, u32 intrIndex );
static void ActOrderTool_SendToLast( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder );
static void ActOrder_Proc( BTL_SVFLOW_WORK* wk, ACTION_ORDER_WORK* actOrder );
static void scproc_ActStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlAction actionCmd );
static void scEvent_ActProcStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlAction actionCmd );
static void scEvent_ActProcEnd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static SabotageType CheckSabotageType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL ActOrder_IntrProc( BTL_SVFLOW_WORK* wk, u8 intrPokeID );
static BOOL ActOrder_IntrReserve( BTL_SVFLOW_WORK* wk, u8 intrPokeID );
static BOOL ActOrder_IntrReserveByWaza( BTL_SVFLOW_WORK* wk, WazaID waza );
static BOOL ActOrder_SendLast( BTL_SVFLOW_WORK* wk, u8 pokeID );
static void ActOrder_ForceDone( BTL_SVFLOW_WORK* wk, u8 pokeID );
static inline void FlowFlg_Set( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_Clear( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline BOOL FlowFlg_Get( BTL_SVFLOW_WORK* wk, FlowFlag flg );
static inline void FlowFlg_ClearAll( BTL_SVFLOW_WORK* wk );
static void wazaEffCtrl_Init( WAZAEFF_CTRL* ctrl );
static void wazaEffCtrl_Setup( WAZAEFF_CTRL* ctrl, BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* rec );
static inline void wazaEffCtrl_SetEnable( WAZAEFF_CTRL* ctrl );
static inline BOOL wazaEffCtrl_IsEnable( const WAZAEFF_CTRL* ctrl );
static inline void wazaEffCtrl_SetEffectIndex( WAZAEFF_CTRL* ctrl, u8 index );
static inline BOOL IsBppExist( const BTL_POKEPARAM* bpp );
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk );
static void FRONT_POKE_SEEK_AddRotationBack( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk );
static BOOL FRONT_POKE_SEEK_GetNext( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk, BTL_POKEPARAM** bpp );
static void scproc_Move( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_MoveCore( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx1, u8 posIdx2, BOOL fActCmd );
static BOOL scproc_NigeruCmd_Root( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static BOOL scproc_NigeruCmdSub( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fSkipNigeruCalc );
static BOOL scEvent_SkipNigeruCalc( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL scproc_NigeruCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fForceNigeru );
static BOOL scEvent_CheckNigeruForbid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL scEvent_NigeruExMessage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scproc_MemberInCore( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx );
static void scproc_MemberInForChange( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 next_poke_idx, BOOL fPutMsg );
static BOOL scproc_AfterMemberIn( BTL_SVFLOW_WORK* wk );
static void scPut_MemberOutMessage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_MemberChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, u8 nextPokeIdx );
static BOOL scproc_MemberOutForChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, BOOL fIntrDisable );
static void scproc_MemberOutCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, u16 effectNo );
static void scEvent_MemberOutFixed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke );
static void scproc_Fight( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_ACTION_PARAM* action );
static void scproc_MagicCoat_Root( BTL_SVFLOW_WORK* wk, WazaID actWaza );
static void wazaRobParam_Init( WAZA_ROB_PARAM* param );
static void wazaRobParam_Add( WAZA_ROB_PARAM* param, u8 robberPokeID, u8 targetPokeID, BtlPokePos targetPos );
static void scproc_WazaRobRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID actWaza, BTL_POKESET* defaultTarget );
static void scEvent_CheckCombiWazaExe( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* wazaParam );
static void scproc_WazaExe_Decide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, BtlEventType evType );
static void scEvent_WazaExeDecide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, BtlEventType evType  );
static BOOL scproc_Fight_CheckCombiWazaReady( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlPokePos targetPos );
static BOOL scproc_Fight_CheckDelayWazaSet( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlPokePos targetPos );
static BOOL scEvent_CheckDelayWazaSet( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BtlPokePos targetPos );
static void scproc_Fight_DecideDelayWaza( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static void scEvent_DecideDelayWaza( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static void scproc_StartWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scEvent_StartWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scproc_EndWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaEnable );
static void scEvent_EndWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaEnable );
static void scEvent_WazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* robPoke, const BTL_POKEPARAM* orgAtkPoke, WazaID waza );
static void scEvent_WazaReflect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* robPoke, const BTL_POKEPARAM* orgAtkPoke, WazaID waza );
static BOOL scproc_Check_WazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza,
  BTL_POKESET* rec, WAZA_ROB_PARAM* robParam );
static BOOL scEvent_CheckWazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza,
  BTL_POKESET* targetRec, u8* robberPokeID, u8* robTargetPokeID );
static BOOL scproc_Fight_CheckReqWazaFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID orgWazaID );
static void scPut_ReqWazaEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, BtlPokePos targetPos );
static void scproc_WazaExeRecordUpdate( BTL_SVFLOW_WORK* wk, WazaID waza );
static BOOL scproc_Fight_WazaExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BTL_POKESET* targetRec );
static void scproc_CheckTripleFarPokeAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static void scproc_MigawariExclude( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
    const BTL_POKEPARAM* attacker, BTL_POKESET* target, BOOL fDamage );
static BOOL scEvent_CheckMigawariExclude( BTL_SVFLOW_WORK* wk,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL fDamage );
static void scproc_WazaExe_Effective( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza );
static void scproc_WazaExe_NotEffective( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza );
static void scproc_WazaExe_Done( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza );
static void scEvent_WazaExeEnd_Common( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza, BtlEventType eventID );
static BOOL IsMustHit( const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target );
static void flowsub_checkWazaAffineNoEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, BTL_DMGAFF_REC* affRec );
static void flowsub_checkNotEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static BOOL scproc_checkNoEffect_sub( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, BtlEventType eventID );
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BtlEventType eventID,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* customMsg, BOOL* fNoReaction );
static void flowsub_CheckPokeHideAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static void flowsub_checkWazaAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static BOOL scEvent_SkipAvoidCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam );
static BOOL IsTripleFarPos( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaID waza );
static BOOL scEvent_CheckHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam );
static void scPut_WazaEffect( BTL_SVFLOW_WORK* wk, WazaID waza, WAZAEFF_CTRL* effCtrl, u32 que_reserve_pos );
static TameWazaResult scproc_Fight_TameWazaExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, const BTL_POKESET* targetRec, WazaID waza );
static BOOL scproc_TameStartTurn( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_POKESET* targetRec, WazaID waza );
static void scproc_TameLockClear( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker );
static BOOL scproc_FreeFall_Start( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL* fFailMsgDisped );
static inline BOOL checkFreeFallUsing( const BTL_POKEPARAM* bpp );
static void scproc_FreeFall_CheckRelease( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fReleaseProc );
static BOOL scproc_Fight_CheckWazaExecuteFail_1st( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u8 fWazaLock );
static BOOL scproc_Fight_CheckWazaExecuteFail_2nd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaLock );
static BOOL scEvent_ExeFailThrew( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause );
static SV_WazaFailCause scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlEventType eventID );
static BOOL scproc_Fight_CheckConf( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker );
static BOOL scproc_Fight_CheckMeroMero( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker );
static BOOL scproc_PokeSickCure_WazaCheck( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza );
static void scproc_WazaExecuteFailed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, SV_WazaFailCause fail_cause );
static void scPut_WazaExecuteFailMsg( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause );
static void scPut_SetBppCounter( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppCounter counterID, u8 value );
static void scproc_decrementPPUsedWaza( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  WazaID waza, u8 wazaIdx, BTL_POKESET* rec );
static u32 scEvent_DecrementPPVolume( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker,
  u8 wazaIdx, WazaID waza, BTL_POKESET* rec );
static BOOL scproc_decrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 wazaIdx, u8 volume );
static void scPut_DecrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, u8 wazaIdx, u8 vol );
static void scproc_Fight_Damage_Root( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
   BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec, BOOL fDelayAttack );
static void BTL_CALCDAMAGE_Set( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKESET* target,
  const SVFL_WAZAPARAM* wazaParam, const BTL_DMGAFF_REC* affRec, fx32 dmg_ratio, BTL_CALC_DAMAGE_REC* dmgRec );
static u32 BTL_CALCDAMAGE_GetCount( const BTL_CALC_DAMAGE_REC* dmgRec );
static u32 BTL_CALCDAMAGE_GetMigawariHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry, u16* dmgAry, BtlTypeAff* affAry, u8* criticalFlagAry );
static u32 BTL_CALCDAMAGE_GetAllHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry );
static u32 BTL_CALCDAMAGE_GetRealHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry, u16* dmgAry, BtlTypeAff* affAry, u8* criticalFlagAry, u8* koraeCauseAry );
static u32 BTL_CALCDAMAGE_GetDamageSum( const BTL_CALC_DAMAGE_REC* rec );
static u32 scproc_Fight_Damage_SingleCount( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec, BOOL fDelayAttack );
static u32 scproc_Fight_Damage_PluralCount( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec );
static BOOL HITCHECK_IsFirstTime( const HITCHECK_PARAM* param );
static BOOL HITCHECK_IsPluralHitWaza( const HITCHECK_PARAM* param );
static BOOL HITCHECK_CheckWazaEffectPuttable( HITCHECK_PARAM* param );
static void HITCHECK_SetPluralHitAffinity( HITCHECK_PARAM* param, BtlTypeAff affinity );
static u32 scproc_Fight_Damage_side( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker,
  BTL_POKESET* targets, const BTL_CALC_DAMAGE_REC* dmgRec, HITCHECK_PARAM* hitCheckParam, fx32 dmg_ratio, FLAG_SET flagSet );
static u32 scproc_Fight_damage_side_core( BTL_SVFLOW_WORK* wk,
    BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_CALC_DAMAGE_REC* dmgRec,
    const SVFL_WAZAPARAM* wazaParam, HITCHECK_PARAM* hitCheckParam, fx32 dmg_ratio, FLAG_SET flagSet );
static void scproc_Fight_Damage_ToRecover( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, BTL_POKESET* targets );
static void scPut_DamageAff( BTL_SVFLOW_WORK* wk, BtlTypeAff affinity );
static u32 MarumeDamage( const BTL_POKEPARAM* bpp, u32 damage );
static void scproc_Fight_Damage_Determine( BTL_SVFLOW_WORK* wk,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam );
static void scEvent_WazaDamageDetermine( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam );
static BppKoraeruCause scEvent_CheckKoraeru( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BOOL fWazaDamage, u16* damage );
static void scproc_Koraeru( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppKoraeruCause cause );
static void scEvent_KoraeruExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppKoraeruCause cause );
static void scproc_PrevWazaDamage( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, u32 poke_cnt, BTL_POKEPARAM** bppAry );
static void scEvent_PrevWazaDamage( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, u32 poke_cnt, BTL_POKEPARAM** bppAry );
static void wazaDmgRec_Add( BTL_SVFLOW_WORK* wk, BtlPokePos atkPos, const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, u16 damage );
static void scproc_WazaAdditionalEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage, BOOL fMigawriHit );
static void scproc_WazaDamageSideAfter( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, u32 damage );
static void scEvent_WazaDamageSideAfter( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u32 damage );
static void scproc_CheckItemReaction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BtlItemReaction reactionType );
static void scEvent_CheckItemReaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlItemReaction reactionType );
static void scproc_Fight_DamageProcStart( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam );
static void scEvent_DamageProcStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam );
static void scproc_Fight_DamageProcEnd( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets, u32 dmgTotal, BOOL fDelayAttack );
static void scproc_CheckShrink( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender );
static void scproc_Fight_Damage_KoriCure( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static BOOL scproc_AddShrinkCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, u32 per );
static void scproc_Fight_Damage_Drain( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static void scproc_Damage_Drain( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage );
static BOOL scproc_DrainCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, u16 drainHP );
static void scEvent_DamageProcEnd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BTL_POKESET* targets,
  const SVFL_WAZAPARAM* wazaParam, BOOL fDelayAttack );
static void scEvent_DamageProcEndSub( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BTL_POKESET* targets,
  const SVFL_WAZAPARAM* wazaParam, BOOL fDelayAttack, BOOL fRealHitOnly, BtlEventType eventID );
static BOOL scEvent_CalcDamage( BTL_SVFLOW_WORK* wk,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam,
    BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag, BOOL fSimurationMode, u16* dstDamage );
static void scproc_Fight_Damage_Kickback( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 wazaDamage );
static BOOL scproc_SimpleDamage_CheckEnable( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage );
static BOOL scproc_SimpleDamage_Core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage, BTL_HANDEX_STR_PARAMS* str );
static BOOL scproc_UseItemEquip( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static BOOL scEvent_CheckItemEquipFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 itemID );
static void scproc_ConsumeItem( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scPut_ConsumeItem( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_ItemChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 nextItemID );
static void scproc_KillPokemon( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_Fight_Damage_AddSick( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static WazaSick scEvent_CheckWazaAddSick( BTL_SVFLOW_WORK* wk, WazaID waza,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BPP_SICK_CONT* pSickCont );
static void scproc_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static BOOL scproc_Fight_WazaSickCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target,
  WazaID waza, WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost );
static WazaSick scEvent_DecideSpecialWazaSick( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* str );
static void scEvent_GetWazaSickAddStr( BTL_SVFLOW_WORK* wk, WazaSick sick,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* str );
static void scEvent_WazaSickCont( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target,
  WazaSick sick, BPP_SICK_CONT* sickCont );
static BOOL scproc_AddSickRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost, BOOL fDefaultMsgEnable );
static BOOL scproc_AddSickCheckFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BtlSickOverWriteMode overWriteMode, BOOL fDispFailResult );
static BtlAddSickFailCode addsick_check_fail_std( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaSick sick, BPP_SICK_CONT sickCont, BtlSickOverWriteMode overWriteMode );
static void scproc_AddSickCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BOOL fDefaultMsgEnable, const BTL_HANDEX_STR_PARAMS* exStr );
static BtlWeather scEvent_GetWeather( BTL_SVFLOW_WORK* wk );
static fx32 svEvent_GetWaitRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL scEvent_WazaSick_CheckFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaSick sick  );
static void scEvent_AddSick_Failed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker, WazaSick sick );
static void scEvent_PokeSickFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker,
  PokeSick sick, BPP_SICK_CONT sickCont );
static void scEvent_WazaSickFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker, WazaSick sick );
static void scEvent_IekiFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target );
static void scproc_Fight_Damage_AddEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static BOOL scEvent_CheckAddRankEffectOccur( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target );
static void scproc_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static BOOL scproc_WazaRankEffect_Common( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL fAlmost );
static void scEvent_GetWazaRankEffectValue( BTL_SVFLOW_WORK* wk, WazaID waza, u32 idx,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaRankEffect* effect, int* volume );
static BOOL scproc_RankEffectCore( BTL_SVFLOW_WORK* wk, u8 atkPokeID, BTL_POKEPARAM* target,
  WazaRankEffect effect, int volume, u8 wazaUsePokeID, u16 itemID, u32 rankEffSerial, BOOL fAlmost, BOOL fStdMsg );
static int scEvent_RankValueChange( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect rankType,
  u8 wazaUsePokeID, u16 itemID, int volume );
static void scproc_Fight_EffectSick( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static void scproc_Fight_SimpleRecover( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static BOOL scproc_RecoverHP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 recoverHP, BOOL fDispFailMsg );
static BOOL scproc_RecoverHP_CheckFailBase( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL scproc_RecoverHP_CheckFailSP( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fDispFailMsg );
static void scproc_RecoverHP_Core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 recoverHP );
static void scproc_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static void scproc_Ichigeki_Succeed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam, BtlTypeAffAbout affAbout );
static void scproc_Ichigeki_Korae( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam,
    BtlTypeAffAbout affAbout, BppKoraeruCause korae_cause, u16 damage );
static void scproc_Ichigeki_Migawari( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam, BtlTypeAffAbout affAbout );
static void scproc_Fight_PushOut( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static BOOL scproc_PushOutCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target,
  BOOL fForceChange, BOOL* fFailMsgDisped, u16 effectNo, BOOL fIgnoreLevel, BTL_HANDEX_STR_PARAMS* succeedMsg );
static PushOutEffect check_pushout_effect( BTL_SVFLOW_WORK* wk );
static int get_pushout_nextpoke_idx( BTL_SVFLOW_WORK* wk, const SVCL_WORK* clwk );
static BOOL scEvent_CheckPushOutFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target );
static void scput_Fight_FieldEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker );
static BOOL scproc_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn );
static BOOL scproc_ChangeWeatherCheck( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn );
static void scproc_ChangeWeatherCore( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn );
static void scproc_ChangeWeatherAfter( BTL_SVFLOW_WORK* wk, BtlWeather weather );
static u8 scEvent_WazaWeatherTurnUp( BTL_SVFLOW_WORK* wk, BtlWeather weather, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8* turn );
static BOOL scproc_FieldEffectCore( BTL_SVFLOW_WORK* wk, BtlFieldEffect effect, BPP_SICK_CONT contParam, BOOL fAddDependTry );
static void scEvent_FieldEffectCall( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scput_Fight_Uncategory( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets );
static void scput_Fight_Uncategory_SkillSwap( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec );
static void scproc_KintyoukanMoved( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bppMoved );
static BOOL scproc_Migawari_Create( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static u16 scproc_Migawari_Damage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, u16 damage,
  BtlTypeAff aff, u8 fCritical, const SVFL_WAZAPARAM* wazaParam );
static void scproc_Migawari_Delete( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scproc_WazaDamageReaction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam, BtlTypeAff affinity, u32 damage, BOOL critical_flag, BOOL fMigawari );
static void scEvent_WazaDamageReaction( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BtlTypeAff aff,
  u32 damage, BOOL fCritical, BOOL fMigawari );
static BOOL scEvent_UnCategoryWaza( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targets, BOOL* fFailMsgEnable );
static void StoreFrontPokeOrderAgility( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet );
static BOOL scproc_TurnCheck( BTL_SVFLOW_WORK* wk );
static void scproc_CheckResetMove( BTL_SVFLOW_WORK* wk );
static void   scproc_turncheck_CommSupport( BTL_SVFLOW_WORK* wk );
static BOOL scproc_turncheck_sub( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet, BtlEventType event_type );
static void scEvent_TurnCheck( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlEventType event_type );
static BOOL scproc_turncheck_sick( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet );
static u32 scEvent_SickDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sickID, u32 damage );
static void scproc_turncheck_side( BTL_SVFLOW_WORK* wk );
static void scproc_turncheck_side_callback( BtlSide side, BtlSideEffect sideEffect, void* arg );
static void scPut_SideEffectOffMsg( BTL_SVFLOW_WORK* wk, BtlSideEffect sideEffect, BtlSide side );
static void scproc_turncheck_field( BTL_SVFLOW_WORK* wk );
static void turncheck_field_callback( BtlFieldEffect effect, void* arg );
static void scproc_FieldEff_End( BTL_SVFLOW_WORK* wk, BtlFieldEffect effect );
static BOOL scproc_turncheck_weather( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet );
static int scEvent_CheckWeatherReaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlWeather weather, u32 damage );
static void scPut_WeatherDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BtlWeather weather, int damage );
static BOOL checkPokeDeadFlagAllOn( BTL_SVFLOW_WORK* wk, u8 pokeID );
static BOOL scproc_CheckDeadCmd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke );
static u32 checkExistEnemyMaxLevel( BTL_SVFLOW_WORK* wk );
static void scproc_ClearPokeDependEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke );
static void scEvent_BeforeDead( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BOOL scproc_CheckExpGet( BTL_SVFLOW_WORK* wk );
static BOOL scproc_GetExp( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* deadPoke );
static BOOL getexp_make_cmd( BTL_SVFLOW_WORK* wk, BTL_PARTY* party, const CALC_EXP_WORK* calcExp );
static BOOL scproc_TameHideCancel( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag hideContFlag );
static inline int roundValue( int val, int min, int max );
static inline int roundMin( int val, int min );
static inline int roundMax( int val, int max );
static inline BOOL perOccur( BTL_SVFLOW_WORK* wk, u8 per );
static void eventWork_Init( BTL_EVENT_WORK_STACK* stack );
static void* eventWork_Push( BTL_EVENT_WORK_STACK* stack, u32 size );
static void eventWork_Pop( BTL_EVENT_WORK_STACK* stack, void* adrs );
static void scPut_EffectByPokePos( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 effectNo );
static void scPut_CantAction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scPut_ConfCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_MeromeroAct( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_ConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage );
static void scPut_CurePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sick, BOOL fStdMsg );
static void scPut_WazaMsg( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scPut_CureSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sick, BPP_SICK_CONT* oldCont );
static WazaSick trans_sick_code( const BTL_POKEPARAM* bpp, BtlWazaSickEx* exCode );
static void scPut_WazaFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static void scPut_WazaAvoid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender, WazaID waza );
static void scput_WazaNoEffect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender );
static void scput_WazaNoEffectIchigeki( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender );
static void scPut_WazaDamagePlural( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  u32 poke_cnt, const BtlTypeAff* aff, BTL_POKEPARAM** bpp, const u16* damage, const u8* critical_flag, BOOL fTargetPlural );
static void scPut_WazaAffinityMsg( BTL_SVFLOW_WORK* wk, u32 poke_cnt, const BtlTypeAff* aff, BTL_POKEPARAM** bpp,  BOOL fTargetPlural );
static void scPut_CriticalMsg( BTL_SVFLOW_WORK* wk, u32 poke_cnt, BTL_POKEPARAM** bpp, const u8* critical_flg, BOOL fTargetPlural );
static void scPut_Koraeru( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BppKoraeruCause cause );
static void scPut_TokWin_In( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_TokWin_Out( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scPut_RankEffectLimit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect effect, int volume );
static void scPut_RankEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume, u16 itemID, BOOL fStdMsg );
static void scPut_SimpleHp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, int value, BOOL fEffectEnable );
static void scPut_KillPokemon( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 effType );
static void scPut_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaSick sick, BPP_SICK_CONT sickCont );
static void scPut_AddSickFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, BtlAddSickFailCode failCode, WazaSick sick );
static void scPut_Message_Set( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 strID );
static void scPut_Message_StdEx( BTL_SVFLOW_WORK* wk, u16 strID, u32 argCnt, const int* args );
static void scPut_Message_SetEx( BTL_SVFLOW_WORK* wk, u16 strID, u32 argCnt, const int* args );
static void scPut_DecreaseHP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 value );
static void scPut_RecoverPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 wazaIdx, u8 volume, BOOL fOrgWaza );
static void scPut_UseItemAct( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp );
static void scPut_SetContFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag flag );
static void scPut_ResetContFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag flag );
static void scPut_SetTurnFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppTurnFlag flag );
static void scPut_ResetTurnFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppTurnFlag flag );
static void scPut_IllusionSet( BTL_SVFLOW_WORK* wk, CLIENTID_REC* rec );
static u32 scEvent_MemberChangeIntr( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* outPoke );
static BOOL scEvent_GetReqWazaParam( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  WazaID orgWazaid, BtlPokePos orgTargetPos, REQWAZA_WORK* reqWaza );
static u8 scEvent_CheckSpecialActPriority( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker );
static void scEvent_BeforeFight( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, WazaID waza );
static u32 scproc_CalcConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckWazaMsgCustom( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker,
  WazaID orgWazaID, WazaID actWazaID, BTL_HANDEX_STR_PARAMS* str );
static void scEvent_GetWazaParam( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* param );
static void scEvent_CheckWazaExeFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp,
  WazaID waza, SV_WazaFailCause cause );
static BtlWazaForceEnableMode scEvent_WazaExecuteStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BTL_POKESET* rec );
static BOOL scEvent_CheckMamoruBreak( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckTameFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* target );
static BOOL scEvent_CheckTameTurnSkip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static BOOL scEvent_TameStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* targetRec, WazaID waza, u8* hideTargetPokeID, BOOL* fFailMsgDisped );
static void scEvent_TameSkip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza );
static BOOL scEvent_TameRelease( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* rec, WazaID waza );
static BOOL scEvent_CheckPokeHideAvoid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_IsCalcHitCancel( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static u32 scEvent_getHitPer( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam );
static BOOL scEvent_IchigekiCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static BOOL scEvent_CheckDmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam );
static void scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL scEvent_CheckCritical( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza );
static u32 scEvent_CalcKickBack( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u32 damage, BOOL* fMustEnable );
static void scEvent_ItemEquip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scEvent_ItemEquipTmp( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u8 atkPokeID );
static BtlTypeAff scProc_checkWazaDamageAffinity( BTL_SVFLOW_WORK* wk,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BOOL fNoEffectMsg );
static BOOL scproc_CheckFloating( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fHikouCheck );
static BOOL scEvent_CheckFloating( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fHikouCheck );
static void scproc_WazaNoEffectByFlying( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scEvent_WazaNoEffectByFlying( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static BtlTypeAff scEvent_CheckDamageAffinity( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, PokeType waza_type, PokeType poke_type );
static void scproc_ViewEffect( BTL_SVFLOW_WORK* wk, u16 effectNo, BtlPokePos pos_from, BtlPokePos pos_to, BOOL fQueResereved, u32 reservedPos );
static BOOL scEvent_DecrementPP_Reaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, u8 wazaIdx );
static BOOL scEvent_HitCheckParam( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, HITCHECK_PARAM* param );
static u16 scEvent_getWazaPower( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam );
static BOOL scEvent_CheckEnableSimpleDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 damage );
static u16 scEvent_getAttackPower( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BOOL criticalFlag );
static u16 scEvent_getDefenderGuard( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam, BOOL criticalFlag );
static fx32 scEvent_CalcTypeMatchRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, PokeType waza_type );
static void scEvent_AfterMemberIn( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static u32 scEvent_GetWazaShrinkPer( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker );
static BOOL scEvent_CheckShrink( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, u32 per );
static void scEvent_FailShrink( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target );
static BOOL scEvent_CheckRankEffectSuccess( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaRankEffect effect, u8 wazaUsePokeID, int volume, u32 wazaRankEffSerial );
static void scEvent_RankEffect_Failed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 wazaRankEffSerial );
static void scEvent_RankEffect_Fix( BTL_SVFLOW_WORK* wk, u8 atkPokeID,
  const BTL_POKEPARAM* bpp, WazaRankEffect rankType, int volume );
static void scEvent_WazaRankEffectFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaID wazaID, WazaRankEffect effectID, int volume );
static u16 scEvent_RecalcDrainVolume( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, u16 volume );
static void scEvent_AfterChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather );
static u32 scEvent_CalcRecoverHP( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* bpp );
static BOOL scEvent_CheckItemSet( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 itemID );
static void scEvent_ItemSetDecide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 nextItemID );
static void scEvent_ItemSetFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp );
static void scEvent_ChangeTokuseiBefore( BTL_SVFLOW_WORK* wk, u8 pokeID, u16 prev_tokuseiID, u16 next_tokuseiID );
static void scEvent_ChangeTokuseiAfter( BTL_SVFLOW_WORK* wk, u8 pokeID );
static void scEvent_CheckSideEffectParam( BTL_SVFLOW_WORK* wk, u8 userPokeID, BtlSideEffect effect, BtlSide side, BPP_SICK_CONT* cont );
static void scEvent_NotifyAirLock( BTL_SVFLOW_WORK* wk );
static void AffCounter_Clear( WAZA_AFF_COUNTER* cnt );
static void AffCounter_CountUp( WAZA_AFF_COUNTER* cnt, BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BtlTypeAff affinity );
static void relivePokeRec_Init( BTL_SVFLOW_WORK* wk );
static void relivePokeRec_Add( BTL_SVFLOW_WORK* wk, u8 pokeID );
static BOOL relivePokeRec_CheckNecessaryPokeIn( BTL_SVFLOW_WORK* wk );
static void PSetStack_Init( BTL_SVFLOW_WORK* wk );
static void PSetStack_Push( BTL_SVFLOW_WORK* wk );
static void PSetStack_Pop( BTL_SVFLOW_WORK* wk );
static void psetstack_setup( BTL_SVFLOW_WORK* wk, u32 sp, BOOL fClear );
static HandExResult scproc_HandEx_Result( BTL_SVFLOW_WORK* wk );
static void HandEx_Exe( BTL_SVFLOW_WORK* wk, BTL_HANDEX_PARAM_HEADER* handEx_header );
static u8 scproc_HandEx_UseItemAct( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_TokWinIn( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_TokWinOut( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_ItemEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_recoverHP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_drain( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_damage( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_shiftHP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_recoverPP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_decrementPP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_cureSick( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_addSick( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_rankEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID );
static u8 scproc_HandEx_setRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_recoverRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_resetRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setStatus( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_kill( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_changeType( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_message( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setTurnFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_resetTurnFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setContFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_resetContFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_sideEffectAdd( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_sideEffectRemove( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_addFieldEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_removeFieldEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_changeWeather( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static BOOL handexSub_putString( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_STR_PARAMS* strParam );
static u8 scproc_HandEx_PosEffAdd( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_tokuseiChange( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_swapItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_CheckItemEquip( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_useItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_ItemSP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_consumeItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_updateWaza( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_counter( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_delayWazaDamage( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_quitBattle( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_changeMember( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_batonTouch( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_addShrink( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_relive( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setWeight( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_pushOut( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_intrPoke( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_intrWaza( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_sendLast( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_swapPoke( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_hensin( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_fakeBreak( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_juryokuCheck( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_TameHideCancel( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_effectByPos( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_changeForm( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setWazaEffectIndex( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );
static u8 scproc_HandEx_setWazaEffectEnable( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header );




BTL_SVFLOW_WORK* BTL_SVFLOW_InitSystem(
  BTL_SERVER* server, BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokeCon,
  BTL_SERVER_CMD_QUE* que, BtlBagMode bagMode, HEAPID heapID )
{
  BTL_SVFLOW_WORK* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SVFLOW_WORK) );

  wk->server = server;
  wk->pokeCon = pokeCon;
  wk->mainModule = mainModule;
  wk->numActOrder = 0;
  wk->turnCount = 0;
  wk->flowResult = SVFLOW_RESULT_DEFAULT;
  wk->que = que;
  wk->heapID = heapID;
  wk->bagMode = bagMode;
  wk->sinkaArcHandle = SHINKA_GetArcHandle( heapID );

  clearWorks( wk );

  return wk;
}


void BTL_SVFLOW_ResetSystem( BTL_SVFLOW_WORK* wk )
{
  clearWorks( wk );
}

static void clearWorks( BTL_SVFLOW_WORK* wk )
{
  BTL_EVENT_InitSystem();
  BTL_FIELD_Init( BTL_WEATHER_NONE );

  BTL_WAZAREC_Init( &wk->wazaRec );
  BTL_DEADREC_Init( &wk->deadRec );
  {
    BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );
    BTL_POSPOKE_InitWork( &wk->pospokeWork, wk->mainModule, wk->pokeCon, rule );
  }

  GFL_STD_MemClear( wk->pokeDeadFlag, sizeof(wk->pokeDeadFlag) );
  GFL_STD_MemClear( wk->pokeInFlag, sizeof(wk->pokeInFlag) );
  GFL_STD_MemClear( wk->memberChangeCount, sizeof(wk->memberChangeCount) );

  AffCounter_Clear( &wk->affCounter );
  PSetStack_Init( wk );
  BTL_ESCAPEINFO_Clear( &wk->escInfo );

  BTL_Hem_Init( &wk->HEManager );
  eventWork_Init( &wk->eventWork );
  BTL_HANDLER_SIDE_InitSystem();

  wk->nigeruCount = 0;
  wk->wazaEffIdx = 0;
  wk->fMemberOutIntr = FALSE;
  wk->fWinBGMPlayWild = FALSE;
  wk->fEscMsgDisped = FALSE;
  wk->fMemberOutIntr = FALSE;
  wk->cmdBuildStep = 0;
  wk->thruDeadMsgPokeID = BTL_POKEID_NULL;
  wk->prevExeWaza = WAZANO_NULL;
  wk->getPokePos = BTL_POS_NULL;
  wk->simulationCounter = 0;
  wk->turnCheckStep = 0;
  wk->wazaRankEffSerial = 0;
}

void BTL_SVFLOW_QuitSystem( BTL_SVFLOW_WORK* wk )
{
  GFL_ARC_CloseDataHandle( wk->sinkaArcHandle );
  GFL_HEAP_FreeMemory( wk );
}




//--------------------------------------------------------------------------
/**
 * サーバコマンド生成（バトル開始直後）
 *
 * @param   wk
 *
 * @retval  SvflowResult
 */
//--------------------------------------------------------------------------
SvflowResult BTL_SVFLOW_StartBtlIn( BTL_SVFLOW_WORK* wk )
{
  SVCL_WORK* cw;
  u32 i, posIdx;

  SCQUE_Init( wk->que );

  {
    const BTL_FIELD_SITUATION* fSit = BTL_MAIN_GetFieldSituation( wk->mainModule );
    if( fSit->weather != BTL_WEATHER_NONE )
    {
      scproc_ChangeWeather( wk, fSit->weather, BTL_WEATHER_TURN_PERMANENT );
    }
  }

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, i );
    if( cw == NULL ){ continue; }

    for(posIdx=0; posIdx<cw->numCoverPos; ++posIdx)
    {
      BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberData(cw->party, posIdx);
      if( (bpp!=NULL) && !BPP_IsDead(bpp) )
      {
        scproc_MemberInCore( wk, i, posIdx, posIdx );
      }
    }
    // ローテーションの時は画面に出る３体を全て図鑑に登録
    if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
    {
      for(posIdx=0; posIdx<BTL_ROTATION_VISIBLE_POS_NUM; ++posIdx)
      {
        BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberData(cw->party, posIdx);
        if( (bpp!=NULL) && !BPP_IsDead(bpp) )
        {
          BTL_MAIN_RegisterZukanSeeFlag( wk->mainModule, i, bpp );
        }
      }
    }
  }

  scproc_AfterMemberIn( wk );

  return SVFLOW_RESULT_DEFAULT;
}

void BTL_SVFLOW_StartTurn_Boot( BTL_SVFLOW_WORK* wk )
{
  wk->cmdBuildStep = 0;
  wk->numEndActOrder = 0;
}

//=============================================================================================
/**
 * サーバコマンド生成（通常の１ターン分コマンド生成）
 *
 * @param   wk
 *
 * @retval  SvflowResult
 */
//=============================================================================================
SvflowResult BTL_SVFLOW_StartTurn( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction )
{
  wk->flowResult =  SVFLOW_RESULT_DEFAULT;
  SCQUE_Init( wk->que );

  if( wk->cmdBuildStep == 0 )
  {
    relivePokeRec_Init( wk );
    BTL_DEADREC_StartTurn( &wk->deadRec );
    FlowFlg_ClearAll( wk );
    BTL_EVENT_StartTurn();

    BTL_SERVER_InitChangePokemonReq( wk->server );
    BTL_EVENTVAR_CheckStackCleared();

    clearPokeCantActionFlag( wk );

    wk->numActOrder = sortClientAction( wk, clientAction, wk->actOrder, NELEMS(wk->actOrder) );
    wk->cmdBuildStep = 1;
  }

  wk->numEndActOrder = ActOrderProc_Main( wk, wk->numEndActOrder );

  BTL_N_Printf( DBGSTR_SVFL_TurnStart_Result, wk->numEndActOrder, wk->numActOrder );

  return wk->flowResult;
}
/**
 *  全ポケモンの反動で動けない状態フラグをクリア
 */
static void clearPokeCantActionFlag( BTL_SVFLOW_WORK* wk )
{
  FRONT_POKE_SEEK_WORK  fps;
  BTL_POKEPARAM* bpp;

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext(&fps, wk, &bpp) )
  {
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_CANT_ACTION) ){
      scPut_ResetContFlag( wk, bpp, BPP_CONTFLG_CANT_ACTION );
    }
  }
}
/**
 *
 */
void BTL_SVFLOW_ContinueAfterPokeChange_Boot( BTL_SVFLOW_WORK* wk )
{
  wk->cmdBuildStep = 0;
}
//=============================================================================================
/**
 * サーバコマンド生成（ターン途中のポケモン入れ替え選択後、継続）
 *
 * @param   wk
 *
 * @retval  SvflowResult
 */
//=============================================================================================
SvflowResult BTL_SVFLOW_ContinueAfterPokeChange( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction )
{
  wk->flowResult = SVFLOW_RESULT_DEFAULT;
  SCQUE_Init( wk->que );

  if( wk->cmdBuildStep == 0 )
  {
    BTL_N_Printf( DBGSTR_SVFL_StartAfterPokeChange );

    BTL_SERVER_InitChangePokemonReq( wk->server );
    BTL_EVENTVAR_CheckStackCleared();

    wk->actOrderStep = 0;
    wk->cmdBuildStep = 1;

    if( ActOrderProc_OnlyPokeIn(wk, clientAction) ){
      wk->flowResult = SVFLOW_RESULT_LEVELUP;
      return wk->flowResult;
    }
  }

  wk->numEndActOrder = ActOrderProc_Main( wk, wk->numEndActOrder );
  return wk->flowResult;
}
void BTL_SVFLOW_StartAfterPokeIn_Boot( BTL_SVFLOW_WORK* wk )
{
  wk->cmdBuildStep = 0;
}

//=============================================================================================
/**
 * サーバコマンド生成（ポケモン死亡・生き返りによるターン最初の入場処理）
 *
 * @param   wk
 *
 * @retval  SvflowResult
 */
//=============================================================================================
SvflowResult BTL_SVFLOW_StartAfterPokeIn( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction )
{
  const BTL_ACTION_PARAM* action;
  u32 i;
  u8 numDeadPoke;

  wk->flowResult =  SVFLOW_RESULT_DEFAULT;
  SCQUE_Init( wk->que );

  if( wk->cmdBuildStep == 0 )
  {
    BTL_SERVER_InitChangePokemonReq( wk->server );
    BTL_EVENTVAR_CheckStackCleared();

    BTL_N_Printf( DBGSTR_SVFL_StartAfterPokeIn );

    numDeadPoke = BTL_DEADREC_GetCount( &wk->deadRec, 0 );

    wk->numActOrder = sortClientAction( wk, clientAction, wk->actOrder, NELEMS(wk->actOrder) );
    wk->numEndActOrder = 0;

    // まずは「ポケモン」コマンドからの入れ替え処理
    for(i=0; i<wk->numActOrder; i++)
    {
      action = &wk->actOrder[i].action;
      if( action->gen.cmd != BTL_ACTION_CHANGE ){ continue; }
      if( action->change.depleteFlag ){ continue; }

      if( !BPP_IsDead(wk->actOrder[i].bpp) )
      {
        BTL_N_Printf( DBGSTR_SVFL_AfterPokeIn_Alive,
                wk->actOrder[i].clientID, action->change.posIdx, action->change.memberIdx );

        scproc_MemberChange( wk, wk->actOrder[i].bpp, action->change.memberIdx );
        wk->actOrder[i].fDone = TRUE;
      }
    }
    // 続けて空きを埋める繰り出し
    for(i=0; i<wk->numActOrder; i++)
    {
      action = &wk->actOrder[i].action;
      if( action->gen.cmd != BTL_ACTION_CHANGE ){ continue; }
      if( action->change.depleteFlag ){ continue; }

      if( BPP_IsDead(wk->actOrder[i].bpp) )
      {
        BTL_N_Printf( DBGSTR_SVFL_AfterPokeIn_Dead,
                action->change.posIdx, action->change.memberIdx );

        scproc_MemberInForChange( wk, wk->actOrder[i].clientID, action->change.posIdx, action->change.memberIdx, TRUE );
        wk->actOrder[i].fDone = TRUE;
      }
    }
    // 新ローテーションバトルでは空きを埋めるローテーションが発生する
    #ifdef ROTATION_NEW_SYSTEM
    if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
    {
      for(i=0; i<wk->numActOrder; i++)
      {
        action = &wk->actOrder[i].action;
        if( action->gen.cmd != BTL_ACTION_ROTATION ){ continue; }
        if( action->change.depleteFlag ){ continue; }

        ActOrder_Proc( wk, &wk->actOrder[i] );
      }
    }
    #endif

    wk->cmdBuildStep = 1;

  }

  scproc_AfterMemberIn( wk );
  scproc_CheckExpGet( wk );
  reqChangePokeForServer( wk, &wk->clientIDRec );

  {
    u8 numDeadPokeAfter = BTL_DEADREC_GetCount( &wk->deadRec, 0 );
    if( numDeadPoke == numDeadPokeAfter )
    {
      scproc_CheckResetMove( wk );
      return SVFLOW_RESULT_DEFAULT;
    }
    else{
      if( scproc_CheckShowdown(wk) == FALSE)
      {
        scPut_IllusionSet( wk, &wk->clientIDRec );
        return SVFLOW_RESULT_POKE_COVER;
      }
      else
      {
        return SVFLOW_RESULT_BTL_SHOWDOWN;
      }
    }
  }
}


//----------------------------------------------------------------------------------
/**
 * コマンド生成コア（選択アクション配列から）
 *
 * @param   wk
 * @param   startOrderIdx   開始アクションIndex
 *
 * @retval  u32   処理し終えたアクション数
 */
//----------------------------------------------------------------------------------
static u32 ActOrderProc_Main( BTL_SVFLOW_WORK* wk, u32 startOrderIdx )
{
  u32 i;
  u8 fExpGet, fShowDown;

  BTL_N_Printf( DBGSTR_SVFL_ActOrderMainStart, startOrderIdx );

  for(i=startOrderIdx; i<wk->numActOrder; i++)
  {
    ActOrder_Proc( wk, &wk->actOrder[i] );

    // レベルアップコマンド発生
    fExpGet = scproc_CheckExpGet( wk );

    // 決着（大爆発など同時全滅のケースは、死亡レコードを見れば解決する）
    fShowDown = scproc_CheckShowdown( wk );

    if( fShowDown ){
      wk->flowResult = SVFLOW_RESULT_BTL_SHOWDOWN;
      return i+1;
    }

    if( wk->flowResult == SVFLOW_RESULT_POKE_GET ){
      return i+1;
    }

    if( wk->flowResult == SVFLOW_RESULT_POKE_CHANGE ){
      // 途中交代しようとしてるなら死亡ポケの分はリクエスト出しちゃだめ
      return i+1;
    }

    if( fExpGet ){
      wk->flowResult = SVFLOW_RESULT_LEVELUP;
      return i+1;
    }

    // @todo 逃げる確定したら本来は「にげる」コマンドだけを処理すべき
    if( wk->flowResult == SVFLOW_RESULT_BTL_QUIT ){
      continue;
    }

    GF_ASSERT_MSG( wk->flowResult==SVFLOW_RESULT_DEFAULT, "result=%d\n", wk->flowResult );
    BTL_N_Printf( DBGSTR_SVFL_QueWritePtr, wk->que->writePtr );
  }

  // ターンチェック
  if( wk->flowResult == SVFLOW_RESULT_DEFAULT )
  {
    u8 numDeadPoke;

    // ターンチェック処理
    fExpGet = scproc_TurnCheck( wk );
    fShowDown = scproc_CheckShowdown( wk );

    if( fShowDown ){
      wk->flowResult = SVFLOW_RESULT_BTL_SHOWDOWN;
      return wk->numActOrder;
    }
    if( fExpGet ){
      wk->flowResult = SVFLOW_RESULT_LEVELUP;
      return wk->numActOrder;
    }

    numDeadPoke = BTL_DEADREC_GetCount( &wk->deadRec, 0 );
    // 死亡・生き返りなどでポケ交換の必要があるかチェック
    if( relivePokeRec_CheckNecessaryPokeIn(wk)
    ||  (numDeadPoke != 0)
    ){
      reqChangePokeForServer( wk, &wk->clientIDRec );
      scPut_IllusionSet( wk, &wk->clientIDRec );

      wk->flowResult = SVFLOW_RESULT_POKE_COVER;
      return wk->numActOrder;
    }

    wk->flowResult = SVFLOW_RESULT_DEFAULT;
  }

  return wk->numActOrder;
}
//----------------------------------------------------------------------------------
/**
 * コマンド生成（ポケモン入場アクションのみ処理）
 *
 * @param   wk
 *
 * @retval  BOOL  入場後の処理でポケモンが死亡->経験値取得でレベルアップした場合にTRUE
 */
//----------------------------------------------------------------------------------
static BOOL ActOrderProc_OnlyPokeIn( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction )
{
  SVCL_WORK* clwk;
  BTL_ACTION_PARAM action;
  u32 i, j, actionCnt;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    clwk = BTL_SERVER_GetClientWork( wk->server, i );
    if( clwk == NULL ){
      continue;
    }

    actionCnt = BTL_SVCL_ACTION_GetNumActPoke( clientAction, i );
    for(j=0; j<actionCnt; ++j)
    {
      action = BTL_SVCL_ACTION_Get( clientAction, i, j );
      if( action.gen.cmd != BTL_ACTION_CHANGE ){ continue; }
      if( action.change.depleteFlag ){ continue; }

      BTL_N_Printf( DBGSTR_SVFL_PokeChangeRootInfo,
            i, action.change.posIdx, action.change.memberIdx );

      scproc_MemberInForChange( wk, i, action.change.posIdx, action.change.memberIdx, TRUE );
    }
  }
  scproc_AfterMemberIn( wk );
  return scproc_CheckExpGet( wk );
//  return scproc_CheckShowdown( wk );
}
//----------------------------------------------------------------------------------
/**
 * 現在のパーティデータ比較し、決着がついたか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_CheckShowdown( BTL_SVFLOW_WORK* wk )
{
  u8  pokeExist[2];
  u32 i;

  pokeExist[0] = pokeExist[1] = FALSE;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk->mainModule, i) )
    {
      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, i );
      u8 side = BTL_MAIN_GetClientSide( wk->mainModule, i );
      u8 aliveCnt = BTL_PARTY_GetAliveMemberCount( party );
      if( aliveCnt )
      {
        BTL_N_Printf( DBGSTR_SVFL_ClientPokeStillAlive, i, side, aliveCnt );
        pokeExist[ side ] = TRUE;
      }
      else{
        BTL_N_Printf( DBGSTR_SVFL_ClientPokeDeadAll, i, side);
      }
    }
  }

  if( (pokeExist[0] == FALSE) || (pokeExist[1] == FALSE) ){
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 *
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL CheckPlayerSideAlive( BTL_SVFLOW_WORK* wk )
{
  u32 i;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk->mainModule, i) )
    {
      u8 side = BTL_MAIN_GetClientSide( wk->mainModule, i );
      u8 playerSide = BTL_MAIN_GetClientSide( wk->mainModule, BTL_MAIN_GetPlayerClientID(wk->mainModule) );

      if( side == playerSide )
      {
        BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, i );
        if( BTL_PARTY_GetAliveMemberCount(party) )
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

//=============================================================================================
/**
 * サーバコマンド生成（次のポケモンを出す or にげる -> にげるを選んだ時の処理）
 *
 * @param   wk
 *
 * @retval  BOOL    逃げることが出来たらTRUE
 */
//=============================================================================================
BOOL BTL_SVFLOW_MakePlayerEscapeCommand( BTL_SVFLOW_WORK* wk )
{
  u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
  BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, 0 );

  SCQUE_Init( wk->que );

  if( scproc_NigeruCmd_Root(wk, bpp) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//=============================================================================================
/**
 * シューターチャージコマンド生成
 *
 * @param   wk
 *
 * @retval  BOOL    コマンド生成したらTRUE
 */
//=============================================================================================
BOOL BTL_SVFLOW_MakeShooterChargeCommand( BTL_SVFLOW_WORK* wk )
{
  SCQUE_Init( wk->que );

  // シューターのエネルギーチャージ
  scproc_countup_shooter_energy( wk );

  return wk->que->writePtr != 0;
}
//----------------------------------------------------------------------------------
/**
 * シューターのエネルギーチャージ計算＆各クライアントに通知
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void scproc_countup_shooter_energy( BTL_SVFLOW_WORK* wk )
{
  if( wk->bagMode == BBAG_MODE_SHOOTER )
  {
    u32 i, count;
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( !BTL_SERVER_IsClientEnable(wk->server, i) ){
        continue;
      }
      // トリプルバトル以外は毎ターン１ずつ固定
      if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_TRIPLE ){
        count = 1;
      // トリプルバトルの場合は自分の空き位置＋１
      }else{
        u8 emptyCnt;
        u8 emptyPos[ BTL_POSIDX_MAX ];

        emptyCnt = BTL_POSPOKE_GetClientEmptyPos( &wk->pospokeWork, i, emptyPos );
        count = emptyCnt + 1;
      }

      #ifdef PM_DEBUG
      if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_SHOOTER_MODE) ){
        count = BTL_SHOOTER_ENERGY_MAX;
      }
      #endif
      SCQUE_PUT_OP_ShooterCharge( wk->que, i, count );
    }
  }
}

//=============================================================================================
/**
 * ローテーションコマンド生成（ローテーションバトルのみ）
 *
 * @param   wk
 * @param   clientID
 * @param   dir
 */
//=============================================================================================
void BTL_SVFLOW_MakeRotationCommand( BTL_SVFLOW_WORK* wk, u8 clientID, BtlRotateDir dir )
{
  BTL_Printf("クライアント[%d]は回転方向 %d へローテ\n", clientID, dir);

  if( (dir != BTL_ROTATEDIR_STAY)
  &&  (dir != BTL_ROTATEDIR_NONE)
  ){
    BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
    BTL_POKEPARAM *outPoke, *inPoke;

    SCQUE_PUT_ACT_Rotation( wk->que, clientID, dir );

    BTL_PARTY_RotateMembers( party, dir, &outPoke, &inPoke );
    if( !BPP_IsDead(outPoke) ){
      scproc_ClearPokeDependEffect( wk, outPoke );
    }
    if( !BPP_IsDead(inPoke) ){
      BTL_HANDLER_TOKUSEI_Add( inPoke );
      BTL_HANDLER_ITEM_Add( inPoke );
    }
    BTL_POSPOKE_Rotate( &wk->pospokeWork, dir, clientID, inPoke, wk->pokeCon );
  }
}
//----------------------------------------------------------------------------------
/**
 * サーバに対し、各クライアントの空き位置を通知
 *
 * @param   wk
 *
 * @retval  BOOL  誰かが死んでいて交換する必要ある場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL reqChangePokeForServer( BTL_SVFLOW_WORK* wk, CLIENTID_REC* rec )
{
  u8 posAry[ BTL_POSIDX_MAX ];
  u8 empty_pos_cnt, clientID, i;
  u8 result = FALSE;

  rec->count = 0;

  for(clientID=0; clientID<BTL_CLIENT_MAX; ++clientID)
  {
    empty_pos_cnt = BTL_POSPOKE_GetClientEmptyPos( &wk->pospokeWork, clientID, posAry );
    if( empty_pos_cnt )
    {
      // 空いている位置を全てサーバへ通知
      BTL_N_Printf( DBGSTR_SVFL_ChangePokeReqInfo, clientID, empty_pos_cnt );
      for(i=0; i<empty_pos_cnt; ++i)
      {
        BTL_SERVER_RequestChangePokemon( wk->server, posAry[i] );
        BTL_N_PrintfSimple( DBGSTR_csv, posAry[i] );
      }
      BTL_N_PrintfSimple( DBGSTR_LF );

      rec->clientID[ rec->count++ ] = clientID;
      result = TRUE;
    }
  }
  return result;
}
//=============================================================================================
/**
 * 逃げたクライアントIDを取得
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
const BTL_ESCAPEINFO* BTL_SVFLOW_GetEscapeInfoPointer( const BTL_SVFLOW_WORK* wk )
{
  return &(wk->escInfo);
}
//=============================================================================================
/**
 * 捕獲したポケモンの位置を取得
 *
 * @param   wk
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_SVFLOW_GetCapturedPokePos( const BTL_SVFLOW_WORK* wk )
{
  return wk->getPokePos;
}
//----------------------------------------------------------------------------------
/**
 * [Proc] ターン最初の「たたかう」コマンド処理前に実行する処理
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void scproc_BeforeFirstFight( BTL_SVFLOW_WORK* wk )
{
  u32 hem_state, i;
  WazaID waza;

  hem_state = BTL_Hem_PushState( &wk->HEManager );
  for(i=0; i<wk->numActOrder; ++i)
  {
    waza = ActOrder_GetWazaID( &wk->actOrder[i] );
    if( waza != WAZANO_NULL )
    {
      if( BTL_HANDLER_Waza_Add(wk->actOrder[i].bpp, waza) )
      {
        scEvent_BeforeFight(wk, wk->actOrder[i].bpp, waza );
        BTL_HANDLER_Waza_Remove( wk->actOrder[i].bpp, waza );
      }
    }
  }
//  scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
static inline u32 ActPri_Make( u8 actPri, u8 wazaPri, u8 spPri, u16 agility )
{
  /*
    行動優先順  ...  3 BIT
    ワザ優先順  ...  6 BIT
    特殊優先順  ...  3 BIT
    素早さ      ... 16 BIT
  */

  return ( ((actPri&0x07)<<25) | ((wazaPri&0x3f)<<19) | ((spPri&0x07)<<16) | (agility&0xffff) );
}
static inline u8 ActPri_GetWazaPri( u32 priValue )
{
  return ((priValue >>19) & 0x3f);
}
static inline u8 ActPri_GetSpPri( u32 priValue )
{
  return ((priValue >> 16) & 0x07);
}
static inline u32 ActPri_SetSpPri( u32 priValue, u8 spPri )
{
  priValue &= ~(0x07 << 16);
  priValue |= ((spPri & 0x07) << 16);
  return priValue;
}

//----------------------------------------------------------------------------------
/**
 * ポケモンごとのアクションをチェックし、処理順序を確定
 *
 * @param   wk
 * @param   clientAction    クライアントアクションデータハンドラ
 * @param   order           処理する順番にクライアントIDを並べ直して格納するための配列
 * @param   orderMax        配列 order の要素数
 *
 * @retval  u8    処理するポケモン数
 */
//----------------------------------------------------------------------------------
static u8 sortClientAction( BTL_SVFLOW_WORK* wk, const BTL_SVCL_ACTION* clientAction, ACTION_ORDER_WORK* order, u32 orderMax )
{
  SVCL_WORK* clwk;
  const BTL_ACTION_PARAM* actParam;
  const BTL_POKEPARAM* bpp;
  u32   hem_state;
  u16 agility;
  u8  actionPri, wazaPri;
  u8  i, j, p, numAction, pokeIdx, action;

// 全ポケモンの行動内容をワークに格納
  for(i=0, p=0; i<BTL_CLIENT_MAX; ++i)
  {
    clwk = BTL_SERVER_GetClientWork( wk->server, i );
    if( clwk == NULL ){ continue; }

    numAction = BTL_SVCL_ACTION_GetNumActPoke( clientAction, i );
    pokeIdx = 0;
    BTL_N_Printf( DBGSTR_SVFL_ActionSortInfo, i, numAction );
    for(j=0; j<numAction; j++)
    {
      order[p].action = BTL_SVCL_ACTION_Get( clientAction, i, j );
      if( BTL_ACTION_IsDeplete(&order[p].action) ){
        break;
      }

      action = BTL_ACTION_GetAction( &order[p].action );

      switch( action ){
      case BTL_ACTION_ROTATION:
        {
          pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( order[p].action.rotation.dir );
          order[p].bpp = BTL_PARTY_GetMemberData( clwk->party, pokeIdx );
          BTL_N_Printf ( DBGSTR_SVFL_ActionSortRotation, pokeIdx );
        }
        break;

      case BTL_ACTION_CHANGE:
        order[p].bpp = BTL_PARTY_GetMemberData( clwk->party, order[p].action.change.posIdx );
        ++pokeIdx;
        break;

      default:
        order[p].bpp = BTL_PARTY_GetMemberData( clwk->party, pokeIdx );
        BTL_N_Printf ( DBGSTR_SVFL_ActionSortGen, pokeIdx, BPP_GetID(order[p].bpp), action );
        ++pokeIdx;
        break;
      }

      order[p].clientID = i;
      order[p].fDone = FALSE;
      order[p].fIntrCheck = FALSE;
      ++p;
    }
  }

// 各ポケモンの行動プライオリティ値を生成
  numAction = p;
  for(i=0; i<numAction; ++i)
  {
    hem_state = BTL_Hem_PushState( &wk->HEManager );
    bpp = order[i].bpp;
    actParam = &(order[i].action);

    // 行動による優先順（優先度高いほど数値大）
    switch( actParam->gen.cmd ){
    case BTL_ACTION_ESCAPE:
      if( (BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_WILD)
      &&  (order[i].clientID == BTL_CLIENT_ENEMY1)
      ){
        // 野生ポケモンの「にげる」は「たたかう」と同じプライオリティに（移動ポケ用）
        actionPri = 0;
      }else{
        actionPri = 4;
      }
      break;
    case BTL_ACTION_CHANGE:   actionPri = 3; break;
    case BTL_ACTION_ITEM:     actionPri = 2; break;
    case BTL_ACTION_ROTATION: actionPri = 1; break;
    case BTL_ACTION_SKIP:     actionPri = 0; break;
    case BTL_ACTION_MOVE:     actionPri = 0; break;
    case BTL_ACTION_FIGHT:    actionPri = 0; break;
    case BTL_ACTION_NULL: continue;
    case BTL_ACTION_RECPLAY_TIMEOVER: continue;
    default:
      GF_ASSERT(0);
      actionPri = 0;
      break;
    }
    // 「たたかう」場合はワザによる優先順、アイテム装備による優先フラグを全て見る
    if( actParam->gen.cmd == BTL_ACTION_FIGHT )
    {
      WazaID  waza;
      waza = ActOrder_GetWazaID( &order[i] );
      BTL_Printf("ポケ[%d]のワザ優先チェック .. waza=%d\n", BPP_GetID(bpp), waza);
      wazaPri = scEvent_GetWazaPriority( wk, waza, bpp );

    // 「ムーブ」または「はんどうで動けない」場合、ワザ優先度フラットで計算
    }
    else if( (actParam->gen.cmd == BTL_ACTION_MOVE) || (actParam->gen.cmd == BTL_ACTION_SKIP) )
    {
      wazaPri = 0 - WAZAPRI_MIN;

    // それ以外は行動優先順のみで判定
    }else{
      wazaPri = 0;
    }

    // すばやさ
    agility = scEvent_CalcAgility( wk, bpp, TRUE );

    BTL_Printf("行動プライオリティ決定！ Client{%d-%d}'s actionPri=%d, wazaPri=%d, agility=%d\n",
        i, j, actionPri, wazaPri, agility );

    BTL_Hem_PopState( &wk->HEManager, hem_state );

    // プライオリティ値とクライアントIDを対にして配列に保存
    order[i].priority = ActPri_Make( actionPri, wazaPri, BTL_SPPRI_DEFAULT, agility );
  }

  // プライオリティ値によるソート
  sortActionSub( order, numAction );

  // この時点での処理順をワークに記憶する
  for(i=0; i<numAction; ++i){
    order[i].defaultIdx = i;
  }

  // プライオリティ操作イベント呼び出し
  for(i=0; i<numAction; ++i)
  {
    if( (order[i].action.gen.cmd == BTL_ACTION_FIGHT)
    ||  (order[i].action.gen.cmd == BTL_ACTION_MOVE)
    ){
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      u8 spPri = scEvent_CheckSpecialActPriority( wk, order[i].bpp );
      order[i].priority = ActPri_SetSpPri( order[i].priority, spPri );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
  }

  // 再度、プライオリティ値によるソート
  sortActionSub( order, numAction );

  return p;
}
//----------------------------------------------------------------------------------
/**
 * アクション内容ワークをプライオリティ値降順にソートする
 *
 * @param   order
 * @param   numOrder
 */
//----------------------------------------------------------------------------------
static void sortActionSub( ACTION_ORDER_WORK* order, u32 numOrder )
{
  ACTION_ORDER_WORK tmp;
  u32 i, j;

  for(i=0; i<numOrder; ++i)
  {
    for(j=i+1; j<numOrder; ++j)
    {
      if( order[i].priority > order[j].priority ){
        continue;
      }
      if( order[i].priority == order[j].priority ){ // 全く同じプライオリティ値があったらランダム
        if( BTL_CALC_GetRand(2) == 0 ){
          continue;
        }
      }

      tmp = order[i];
      order[i] = order[j];
      order[j] = tmp;
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * [Event] ワザプライオリティ取得
 * ※ワザデータに設定されている min ～ max の値ではなく、0オリジンの値に直したもの。
 *
 * @param   wk
 * @param   waza
 * @param   bpp
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 scEvent_GetWazaPriority( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* bpp )
{
  u8 pri = WAZADATA_GetParam( waza, WAZAPARAM_PRIORITY ) - WAZAPRI_MIN;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_PRI, pri );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_GET_WAZA_PRI );
    pri = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_PRI );
  BTL_EVENTVAR_Pop();

  return pri;
}
//--------------------------------------------------------------------------
/**
 * [Event] すばやさ取得
 *
 * @param   wk
 * @param   attacker
 * @param   fTrickRoomEnable
 *
 * @retval  u16
 */
//--------------------------------------------------------------------------
static u16 scEvent_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BOOL fTrickRoomEnable )
{
  u32 agi = BPP_GetValue( attacker, BPP_AGILITY );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_AGILITY, agi );
    BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, TRUE ); // まひチェックフラグとして使っている
    BTL_EVENTVAR_SetValue( BTL_EVAR_TRICK_FLAG, FALSE );
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.1f), FX32_CONST(32) );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_AGILITY );

    agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY );
    {
      fx32 ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
      agi = BTL_CALC_MulRatio( agi, ratio );
    }
    if( BPP_GetPokeSick(attacker) == POKESICK_MAHI )
    {
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) ){
        agi = (agi * BTL_MAHI_AGILITY_RATIO) / 100;
      }
    }
    if( agi > BTL_CALC_AGILITY_MAX ){
      agi = BTL_CALC_AGILITY_MAX;
    }

    // トリックルームでひっくりかえる
    if( fTrickRoomEnable )
    {
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_TRICK_FLAG) ){
        agi = BTL_CALC_AGILITY_MAX - agi;
      }
    }

  BTL_EVENTVAR_Pop();

  return agi;
}

// アクション内容から、優先順位計算に使用するワザIDを取得（ワザ使わない場合はWAZANO_NULL）
static inline WazaID ActOrder_GetWazaID( const ACTION_ORDER_WORK* wk )
{
  if( wk->action.gen.cmd == BTL_ACTION_FIGHT )
  {
    return wk->action.fight.waza;
  }
  return WAZANO_NULL;
}
// 指定ポケモンのアクション内容データポインタを取得
static ACTION_ORDER_WORK* ActOrderTool_SearchByPokeID( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u32 i;
  for(i=0; i<wk->numActOrder; ++i){
    if( BPP_GetID(wk->actOrder[i].bpp) == pokeID ){
      return &(wk->actOrder[i]);
    }
  }
  return NULL;
}
// 未実行の指定ワザ使用アクション内容データポインタを取得
static ACTION_ORDER_WORK* ActOrderTool_SearchByWazaID( BTL_SVFLOW_WORK* wk, WazaID waza, u8 startIndex )
{
  u32 i;
  for(i=startIndex; i<wk->numActOrder; ++i)
  {
    if( wk->actOrder[i].fDone == FALSE )
    {
      if( (BTL_ACTION_GetAction(&wk->actOrder[i].action) == BTL_ACTION_FIGHT)
      &&  (BTL_ACTION_GetWazaID(&wk->actOrder[i].action) == waza)
      ){
        return &wk->actOrder[i];
      }
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------
/**
 * 合体ワザ対象となるアクション内容データポインタを取得（存在しない場合NULL）
 *
 * @param   wk
 * @param   waza        対象ワザID
 * @param   pokeID      先に合体ワザを打つポケID（このポケの味方のみが対象となる）
 * @param   targetPos   先に合体ワザを打つポケが狙っている位置（同位置を狙うワザのみ対象となる）
 *
 * @retval  ACTION_ORDER_WORK*
 */
//----------------------------------------------------------------------------------
static ACTION_ORDER_WORK* ActOrderTool_SearchForCombiWaza( BTL_SVFLOW_WORK* wk, WazaID waza, u8 pokeID, BtlPokePos targetPos )
{
  ACTION_ORDER_WORK* p = ActOrderTool_SearchByWazaID( wk, waza, 0 );
  while( p != NULL )
  {
    if( BTL_MAINUTIL_IsFriendPokeID(pokeID, BPP_GetID(p->bpp)) )
    {
      return p;
    }

    {
      u8 idx = ActOrderTool_GetIndex( wk, p );
      p = ActOrderTool_SearchByWazaID( wk, waza, idx + 1 );
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------
/**
 * アクション内容データポインタから自身のIndexを取得
 *
 * @param   wk
 * @param   actOrder
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 ActOrderTool_GetIndex( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder )
{
  int i;
  for(i=0; i<wk->numActOrder; ++i)
  {
    if( &(wk->actOrder[i]) == actOrder ){
      return i;
    }
  }
  GF_ASSERT(0);
  return wk->numActOrder;
}
//----------------------------------------------------------------------------------
/**
 * 指定アクションを、指定index以降の未実行列の先頭に割り込ませる
 *
 * @param   wk
 * @param   actOrder
 * @param   intrIndex   割り込み許可する最小のindex（これ以降に割り込み許可）
 *
 * @retval  int   割り込んだIndex （割り込めなければ-1)
 */
//----------------------------------------------------------------------------------
static int ActOrderTool_Intr( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder, u32 intrIndex )
{
  int prevOrder, topOrder, i;

  // 自分自身の元々のIndexと、未実行列の先頭Indexをサーチ
  prevOrder = topOrder = -1;
  for(i=intrIndex; i<wk->numActOrder; ++i)
  {
    if( &(wk->actOrder[i]) == actOrder ){
      prevOrder = i;
      break;
    }
  }
  for(i=intrIndex; i<wk->numActOrder; ++i)
  {
    if( wk->actOrder[i].fDone == FALSE){
      topOrder = i;
      break;
    }
  }

  // どちらも有効値＆自分自身が未実行の先頭より後の順番なら並べ替え
  if( (prevOrder>=0) && (topOrder>=0)
  &&  (prevOrder > topOrder)
  ){
    wk->actOrderTmp = *actOrder;
    while( prevOrder > topOrder ){
      wk->actOrder[prevOrder] = wk->actOrder[prevOrder-1];
      --prevOrder;
    }
    wk->actOrder[topOrder] = wk->actOrderTmp;
    return topOrder;
  }

  return -1;
}
//----------------------------------------------------------------------------------
/**
 * 指定アクションを、最後に回す
 *
 * @param   wk
 * @param   actOrder
 */
//----------------------------------------------------------------------------------
static void ActOrderTool_SendToLast( BTL_SVFLOW_WORK* wk, const ACTION_ORDER_WORK* actOrder )
{
  // 自分自身の元々のIndex
  int prevOrder = -1;
  int i;

  for(i=0; i<wk->numActOrder; ++i)
  {
    if( &(wk->actOrder[i]) == actOrder ){
      prevOrder = i;
      break;
    }
  }

  if( prevOrder >= 0 )
  {
    wk->actOrderTmp = *actOrder;
    for(i=prevOrder; i<(wk->numActOrder-1); ++i)
    {
      wk->actOrder[i] = wk->actOrder[i+1];
    }
    wk->actOrder[i] = wk->actOrderTmp;
  }
}

//----------------------------------------------------------------------------------
/**
 *  アクション内容の実行
 *
 * @param   wk
 * @param   actOrder
 */
//----------------------------------------------------------------------------------
static void ActOrder_Proc( BTL_SVFLOW_WORK* wk, ACTION_ORDER_WORK* actOrder )
{
  if( actOrder->fDone == FALSE )
  {
    BTL_POKEPARAM* bpp = actOrder->bpp;
    BTL_ACTION_PARAM action = actOrder->action;

    PSetStack_Push( wk );
    actOrder->fDone = TRUE;
    BTL_N_Printf( DBGSTR_SVFL_ActOrderStart, BPP_GetID(bpp), BPP_GetMonsNo(bpp), actOrder );

    do {
      // 既に誰かの逃げが確定している時、逃げコマンド処理以外は実行しない
      if( (wk->flowResult == SVFLOW_RESULT_BTL_QUIT)
      &&  (action.gen.cmd != BTL_ACTION_ESCAPE)
      ){
        break;
      }
      // 死んでたら実行しない
      if( BPP_IsDead(bpp) ){
        break;
      }
      // ローテーション以外で、場にいないポケは実行しない
      if( (action.gen.cmd != BTL_ACTION_ROTATION)
      &&  (!BTL_POSPOKE_IsExist(&wk->pospokeWork, BPP_GetID(bpp)))
      ){
        break;
      }
      // フリーフォール状態のポケは実行しない
      if( BPP_CheckSick(bpp, WAZASICK_FREEFALL) ){
        break;
      }

      scproc_ActStart( wk, bpp, actOrder->action.gen.cmd );
      BPP_TURNFLAG_Set( bpp, BPP_TURNFLG_ACTION_START );

      switch( action.gen.cmd ){
      #ifdef ROTATION_NEW_SYSTEM
      case BTL_ACTION_ROTATION:
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION ){
          BTL_N_Printf( DBGSTR_SVFL_ActOrder_Rotation, actOrder->clientID, action.rotation.dir);
          BTL_SVFLOW_MakeRotationCommand( wk, actOrder->clientID, action.rotation.dir );
        }
        else{
          GF_ASSERT(0);
        }
        break;
      #endif
      case BTL_ACTION_FIGHT:
        if( !FlowFlg_Get(wk, FLOWFLG_FIRST_FIGHT) ){
          scproc_BeforeFirstFight( wk );
          FlowFlg_Set( wk, FLOWFLG_FIRST_FIGHT );
        }
        BTL_N_Printf( DBGSTR_SVFL_ActOrder_Fight, BPP_GetID(bpp), action.fight.waza, action.fight.targetPos);
        wk->currentSabotageType = CheckSabotageType( wk, bpp );
        scproc_Fight( wk, bpp, &action );
        break;
      case BTL_ACTION_ITEM:
        BTL_N_Printf( DBGSTR_SVFL_ActOrder_Item, action.item.number, action.item.targetIdx);
//        scproc_TrainerItem_Root( wk, bpp, action.item.number, action.item.param, action.item.targetIdx );
        {
          TrItemResult result = BTL_SVFSUB_TrainerItemProc( wk, bpp, action.item.number,
                    action.item.param, action.item.targetIdx );

          if( result == TRITEM_RESULT_ESCAPE )
          {
            if( scproc_NigeruCore(wk, bpp, TRUE) ){
              wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
            }
          }
        }
        break;
      case BTL_ACTION_CHANGE:
        BTL_Printf( DBGSTR_SVFL_ActOrder_Change, action.change.posIdx, action.change.memberIdx);
        scproc_MemberChange( wk, bpp, action.change.memberIdx );
        scproc_AfterMemberIn( wk );
        break;
      case BTL_ACTION_ESCAPE:
        BTL_N_Printf( DBGSTR_SVFL_ActOrder_Escape );
        if( scproc_NigeruCmd_Root( wk, bpp ) ){
          wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
        }
        break;
      case BTL_ACTION_RECPLAY_TIMEOVER:
        wk->flowResult = SVFLOW_RESULT_BTL_SHOWDOWN;
        break;
      case BTL_ACTION_MOVE:
        scproc_Move( wk, bpp );
        break;
      case BTL_ACTION_SKIP:
        scPut_CantAction( wk, bpp );
        break;
      case BTL_ACTION_NULL:
        BTL_N_Printf( DBGSTR_SVFL_ActOrder_Dead );
        break;
      }

      if( (action.gen.cmd == BTL_ACTION_FIGHT) || (action.gen.cmd == BTL_ACTION_ITEM) ){
        BPP_TURNFLAG_Set( bpp, BPP_TURNFLG_ACTION_DONE );
        scPut_SetContFlag( wk, bpp, BPP_CONTFLG_ACTION_DONE );
      }

      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        scEvent_ActProcEnd( wk, bpp );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
      }

    }while(0);

    PSetStack_Pop( wk );
    BTL_N_Printf( DBGSTR_SVFL_ActOrderEnd, BPP_GetID(bpp), BPP_GetMonsNo(bpp), actOrder );
  }
}
//----------------------------------------------------------------------------------
/**
 * アクション実行開始通知処理
 *
 * @param   wk
 * @param   bpp
 * @param   actionCmd
 */
//----------------------------------------------------------------------------------
static void scproc_ActStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlAction actionCmd )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_ActProcStart( wk, bpp, actionCmd );
//  scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] アクション実行開始
 *
 * @param   wk
 * @param   bpp
 * @param   actionCmd
 */
//----------------------------------------------------------------------------------
static void scEvent_ActProcStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlAction actionCmd )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ACTION, actionCmd );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ACTPROC_START );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] アクション実行の終了
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scEvent_ActProcEnd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ACTPROC_END );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * 命令無視チェック
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  SabotageType
 */
//----------------------------------------------------------------------------------
static SabotageType CheckSabotageType( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor(wk->mainModule);

  if( ((competitor == BTL_COMPETITOR_TRAINER) || (competitor == BTL_COMPETITOR_WILD))
  &&  (BTL_MAINUTIL_PokeIDtoClientID(BPP_GetID(bpp)) == (BTL_MAIN_GetPlayerClientID(wk->mainModule)))
  ){
    const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
    const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );
    if( !PP_IsMatchOya(pp, status) )
    {
      u8 badgeCnt = BTL_MAIN_GetPlayerBadgeCount( wk->mainModule );

      if( badgeCnt < 8 )
      {
        u16 lv_poke = BPP_GetValue( bpp, BPP_LEVEL );
        u16 lv_border = (1 + badgeCnt) * 10;
        u16 rand;

        if( lv_poke <= lv_border ){
          return SABOTAGE_NONE;
        }
        rand = BTL_CALC_GetRand( (lv_poke + lv_border + 1) );
        if( rand < lv_border ){
          return SABOTAGE_NONE;
        }

        if( BPP_CheckSick(bpp, WAZASICK_NEMURI) ){
          return SABOTAGE_DONT_ANY;
        }

        rand = BTL_CALC_GetRand( (lv_poke + lv_border + 1) );
        if( rand < lv_border ){
          if( BPP_WAZA_GetUsableCount(bpp) > 1 ){
            return SABOTAGE_OTHER_WAZA;
          }
        }

        rand = BTL_CALC_GetRand( 256 );
        if( rand < (lv_poke - lv_border) )
        {
          if( !scEvent_WazaSick_CheckFail(wk, bpp, bpp, POKESICK_NEMURI) ){
            return SABOTAGE_GO_SLEEP;
          }
        }

        rand -= (lv_poke - lv_border);
        if( rand < (lv_poke - lv_border) ){
          return SABOTAGE_CONFUSE;
        }

        return SABOTAGE_DONT_ANY;
      }
    }
  }
  return SABOTAGE_NONE;

}

//--------------------------------------------------------------
/**
 *  アクションの割り込み実行（ポケモンID指定）
 *  ※割り込んで即座に実行（現状は“おいうち”にのみ使用）
 */
//--------------------------------------------------------------
static BOOL ActOrder_IntrProc( BTL_SVFLOW_WORK* wk, u8 intrPokeID )
{
  ACTION_ORDER_WORK* actOrder = ActOrderTool_SearchByPokeID( wk, intrPokeID );
  if( actOrder && (actOrder->fDone == FALSE) ){
    BTL_N_Printf( DBGSTR_SVFL_ActIntr, actOrder, BPP_GetID(actOrder->bpp));
    ActOrder_Proc( wk, actOrder );
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 *  アクションの割り込み予約（ポケモンID指定）
 *  ※割り込んで、現在処理しているアクションの直後に実行
 * （現状は“おさきにどうぞ”と合体ワザにのみ使用）
 */
//--------------------------------------------------------------
static BOOL ActOrder_IntrReserve( BTL_SVFLOW_WORK* wk, u8 intrPokeID )
{
  ACTION_ORDER_WORK* actOrder = ActOrderTool_SearchByPokeID( wk, intrPokeID );
  if( actOrder && (actOrder->fDone == FALSE) ){
    if( ActOrderTool_Intr(wk, actOrder, 0) >= 0 ){
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 *  アクションの割り込み実行（使用ワザ指定）
 *  ※割り込んで、現在処理しているアクションの直後に実行（現状は“りんしょう”にのみ使用）
 */
//--------------------------------------------------------------
static BOOL ActOrder_IntrReserveByWaza( BTL_SVFLOW_WORK* wk, WazaID waza )
{
  ACTION_ORDER_WORK* actOrder = ActOrderTool_SearchByWazaID( wk, waza, 0 );
  int intrIdx = 0;
  BOOL result = FALSE;
  while( actOrder )
  {
    intrIdx = ActOrderTool_Intr( wk, actOrder, intrIdx );
    if( intrIdx >= 0 ){
      actOrder = ActOrderTool_SearchByWazaID( wk, waza, ++intrIdx );
      result = TRUE;
    }else{
      break;
    }
  }
  return result;
}
//--------------------------------------------------------------
/**
 *  アクションを最後に回す（ポケモンID指定）
 */
//--------------------------------------------------------------
static BOOL ActOrder_SendLast( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  ACTION_ORDER_WORK* actOrder = ActOrderTool_SearchByPokeID( wk, pokeID );
  if( actOrder && (actOrder->fDone == FALSE) ){
    ActOrderTool_SendToLast( wk, actOrder );
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 *  未実行アクションを実行済みにする（ポケモンID指定）
 */
//--------------------------------------------------------------
static void ActOrder_ForceDone( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  ACTION_ORDER_WORK* actOrder = ActOrderTool_SearchByPokeID( wk, pokeID );
  if( actOrder )
  {
    actOrder->fDone = TRUE;
  }

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

/**
 *  ワザエフェクト発動管理：初期化
 */
static void wazaEffCtrl_Init( WAZAEFF_CTRL* ctrl )
{
  ctrl->attackerPos = BTL_POS_NULL;
  ctrl->targetPos = BTL_POS_NULL;
  ctrl->effectIndex = 0;
  ctrl->fEnable = FALSE;
  ctrl->fDone = FALSE;
}
/**
 *  ワザエフェクト発動管理：基本パラメータ設定
 */
static void wazaEffCtrl_Setup( WAZAEFF_CTRL* ctrl, BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* rec )
{
  u32 target_max = BTL_POKESET_GetCountMax( rec );

  ctrl->attackerPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(attacker) );
  ctrl->targetPos = BTL_POS_NULL;
  ctrl->fEnable = FALSE;
  ctrl->fDone = FALSE;

  // ターゲットが単体なら明確なターゲット位置情報を記録
  if( target_max == 1 )
  {
    if( BTL_POKESET_GetCount(rec) )
    {
      const BTL_POKEPARAM* bpp = BTL_POKESET_Get( rec, 0 );
      ctrl->targetPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(bpp) );
    }
  }
}

/**
 *  ワザエフェクト発動管理：発動確定
 */
static inline void wazaEffCtrl_SetEnable( WAZAEFF_CTRL* ctrl )
{
  if( ctrl->fEnable == FALSE )
  {
    ctrl->fEnable = TRUE;
  }
}
/**
 *  ワザエフェクト発動管理：有効で、既に発動したことを通知（複数回ヒット）
 */
static inline void wazaEffCtrl_SetEnableDummy( WAZAEFF_CTRL* ctrl )
{
  if( ctrl->fEnable == FALSE )
  {
    ctrl->fEnable = TRUE;
    ctrl->fDone = TRUE;
  }
}
/**
 *  ワザエフェクト発動管理：発動確定するかチェック
 */
static inline BOOL wazaEffCtrl_IsEnable( const WAZAEFF_CTRL* ctrl )
{
  return ctrl->fEnable;
}
/**
 *  ワザエフェクト発動管理：既にエフェクトコマンド生成済みチェック
 */
static inline BOOL wazaEffCtrl_IsDone( const WAZAEFF_CTRL* ctrl )
{
  return ctrl->fDone;
}

static inline void wazaEffCtrl_SetEffectIndex( WAZAEFF_CTRL* ctrl, u8 index )
{
  ctrl->effectIndex = index;
}

/**
 *  ポケモン存在＆生存チェック一時関数
 */
static inline BOOL IsBppExist( const BTL_POKEPARAM* bpp )
{
  if( bpp != NULL ){
    return !BPP_IsDead(bpp);
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------------
// 戦闘に出ているポケモンデータ順番アクセス処理
//----------------------------------------------------------------------------------------------

/**
 *  順番アクセス：ワーク初期化
 */
static void FRONT_POKE_SEEK_InitWork( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk )
{
  fpsw->clientIdx = 0;
  fpsw->pokeIdx = 0;
  fpsw->endFlag = TRUE;
  fpsw->rotationFlag = FALSE;

  {
    SVCL_WORK* cw;
    u8 i, j;

    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, i );
      if( cw == NULL ){ continue; }

      for(j=0; j<cw->numCoverPos; ++j)
      {
        if( IsBppExist( BTL_PARTY_GetMemberDataConst(cw->party, j)) )
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
 *  順番アクセス：ローテーションバトル時の後衛ポケモンも追加
 */
static void FRONT_POKE_SEEK_AddRotationBack( FRONT_POKE_SEEK_WORK* fpsw, BTL_SVFLOW_WORK* wk )
{
  if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
  {
    fpsw->rotationFlag = TRUE;
  }
}
/**
 *  順番アクセス：次のポケモンデータを取得（FALSEが返ったら終了）
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

    *bpp = BTL_PARTY_GetMemberData( cw->party, fpsw->pokeIdx );
    fpsw->pokeIdx++;

    while( fpsw->clientIdx < BTL_CLIENT_MAX )
    {
      cw = BTL_SERVER_GetClientWorkIfEnable( wk->server, fpsw->clientIdx );
      if( cw )
      {
        u8 posMax = (fpsw->rotationFlag == FALSE)? cw->numCoverPos : BTL_ROTATION_VISIBLE_POS_NUM;
        while( fpsw->pokeIdx < posMax )
        {
          nextPoke = BTL_PARTY_GetMemberData( cw->party, fpsw->pokeIdx );
          if( IsBppExist(nextPoke) )
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
//======================================================================================================
// サーバーフロー処理
//======================================================================================================

//-----------------------------------------------------------------------------------
// サーバーフロー：「ムーブ」
//-----------------------------------------------------------------------------------
static void scproc_Move( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  BTL_PARTY* party;
  u8 clientID;
  u8 pokeID;
  int posIdx;

  pokeID = BPP_GetID( bpp );
  clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
  party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
  posIdx = BTL_PARTY_FindMember( party, bpp );
  if( (posIdx == 0) || (posIdx == 2) )
  {
    scproc_MoveCore( wk, clientID, posIdx, 1, TRUE );
    scPut_Message_Set( wk, bpp, BTL_STRID_SET_TripleMove );
  }
  else
  {
    GF_ASSERT_MSG(0, "clientID=%d, pokeID=%d, posIdx=%d\n", clientID, pokeID, posIdx);
  }
}
/**
 *
 */
static void scproc_MoveCore( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx1, u8 posIdx2, BOOL fActCmd )
{
  BtlPokePos pos1, pos2;

  pos1 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx1 );
  pos2 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx2 );

  BTL_POSPOKE_Swap( &wk->pospokeWork, pos1, pos2 );
  {
    BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
    BTL_PARTY_SwapMembers( party, posIdx1, posIdx2 );
  }

  if( fActCmd ){
    SCQUE_PUT_ACT_MemberMove( wk->que, clientID, pos1, pos2 );
  }
}

//-----------------------------------------------------------------------------------
// サーバーフロー：「にげる」
//-----------------------------------------------------------------------------------
/**
 * 逃げる処理ルート
 *
 *  @retval 逃げられたらTRUE, 逃げ失敗でFALSE
 */
static BOOL scproc_NigeruCmd_Root( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( BPP_GetID(bpp) );
  u8 playerClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );

  BOOL result = scproc_NigeruCmdSub( wk, bpp, FALSE );

  if( result )
  {
    // 逃げられたレコードは外部で処理 2010.06.15  taya
    #if 0
    if( !BTL_MAINUTIL_IsFriendClientID(clientID, playerClientID) ){
      BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_NIGERARETA );
    }
    #endif
    return TRUE;
  }
  else
  {
    if( clientID == playerClientID ){
      BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_NIGERU_SIPPAI );
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_EscapeFail );
    }else{
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_EscapeFailed, BPP_GetID(bpp) );
    }
  }

  if( clientID == playerClientID )
  {
    if( ++(wk->nigeruCount) > NIGERU_COUNT_MAX ){
      wk->nigeruCount = NIGERU_COUNT_MAX;
    }
  }

  return result;
}
/**
 * 逃げるコマンド選択処理下請け
 *
 *  @retval 逃げられたらTRUE, 逃げ失敗でFALSE
 */
static BOOL scproc_NigeruCmdSub( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fSkipNigeruCalc )
{
  BOOL fForceNigeru = FALSE;

  if( BTL_MAIN_GetEscapeMode(wk->mainModule) == BTL_ESCAPE_MODE_WILD )
  {
    u8 escClientID = BTL_MAINUTIL_PokeIDtoClientID( BPP_GetID(bpp) );
    if( escClientID == BTL_MAIN_GetPlayerClientID(wk->mainModule) )
    {
      fForceNigeru = scEvent_SkipNigeruCalc( wk, bpp );
      if( fForceNigeru ){
        fSkipNigeruCalc = TRUE;
      }
    }
    // 野生ポケモンは逃げ計算を免除
    else{
      fSkipNigeruCalc = TRUE;
    }
  }
  else{
    fSkipNigeruCalc = TRUE;
    fForceNigeru = TRUE;
  }

  #ifdef PM_DEBUG
  // L or R 押しっぱなしで強制的に逃げる
  if( ( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) || ( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) ){
    fSkipNigeruCalc = TRUE;
    fForceNigeru = TRUE;
  }
  #endif

  #ifdef DEBUG_ONLY_FOR_hudson
  // 逃げれるようにする
  if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) ||
      HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) ||
      HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) )
  {
    fSkipNigeruCalc = TRUE;
    fForceNigeru = TRUE;
  }
  #endif //DEBUG_ONLY_FOR_hudson

  if( !fSkipNigeruCalc )
  {
    const BTL_POKEPARAM* bppEnemy = BTL_POKECON_GetClientPokeDataConst( wk->pokeCon, BTL_CLIENTID_SA_ENEMY1, 0 );
    if( bppEnemy )
    {
      u16 myAgi = BPP_GetValue_Base( bpp, BPP_AGILITY );
      u16 enemyAgi = BPP_GetValue_Base( bppEnemy, BPP_AGILITY );

      if( myAgi <= enemyAgi )
      {
        u32 value = ((myAgi << FX32_SHIFT) / enemyAgi * 128) >> FX32_SHIFT;
        value += (wk->nigeruCount * 30 );
        if( BTL_CALC_GetRand(256) >= value ){
          return FALSE;
        }
      }
    }
  }

  return scproc_NigeruCore( wk, bpp, fForceNigeru );
}
//----------------------------------------------------------------------------------
/**
 * [Event] 逃げる時の確率計算処理をスキップするか判定
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL    スキップするならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_SkipNigeruCalc( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  if( !BPP_IsDead(bpp) )
  {
    BOOL result = FALSE;
    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, result );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_SKIP_NIGERU_CALC );
      result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
    BTL_EVENTVAR_Pop();
    return result;
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * 逃げる処理コア
 *
 * @param   wk
 * @param   bpp
 * @param   fNigeruCmd
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_NigeruCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fForceNigeru )
{
  u8 escapeClientID = BTL_MAINUTIL_PokeIDtoClientID( BPP_GetID(bpp) );

  // 野生戦は逃げる判定
  if( BTL_MAIN_GetEscapeMode(wk->mainModule) == BTL_ESCAPE_MODE_WILD )
  {
    u8 playerClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
    u8 fEscapeEnemy = ( !BTL_MAINUTIL_IsFriendClientID( escapeClientID, playerClientID) );

    // 強制的に逃がす場合はチェックを行わない
    if( !fForceNigeru )
    {
      // 逃げ禁止チェック
      if( !BPP_IsDead(bpp) )
      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        BOOL fForbid = scEvent_CheckNigeruForbid( wk, bpp );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
        if( fForbid ){
          return FALSE;
        }
      }
    }

    // ここまで来たら逃げ確定
    if( !(wk->fEscMsgDisped) )
    {
      // 特殊な逃げメッセージチェック
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      BOOL fSpMsgDisped = scEvent_NigeruExMessage( wk, bpp );

      // 何もなければ標準メッセージ
      if( !fSpMsgDisped )
      {
        if( fEscapeEnemy )
        {
          // 本来なら野生戦以外はココにこないが、デバッグLR逃げだと来て見栄えが悪いので括る。
          if( BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_WILD ){
            SCQUE_PUT_MSG_STD_SE( wk->que, BTL_STRID_STD_EscapeWild, SEQ_SE_NIGERU, BPP_GetID(bpp) );
          }
        }
        else
        {
          SCQUE_PUT_MSG_STD_SE( wk->que, BTL_STRID_STD_EscapeSuccess, SEQ_SE_NIGERU );
        }
      }
      BTL_Hem_PopState( &wk->HEManager, hem_state );

      wk->fEscMsgDisped = TRUE;
    }

  }

  // サブウェイ戦・通信対戦などは無条件に逃げる
  BTL_ESCAPEINFO_Add( &wk->escInfo, escapeClientID );
  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 逃げる封じチェック処理
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL  逃げる封じが発動していたらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckNigeruForbid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BOOL result = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, result );
    BTL_SICKEVENT_CheckEscapeForbit( wk, bpp );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_NIGERU_FORBID );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  return result;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 逃げることが成功したときに特殊メッセージを表示するか判定
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL    特殊メッセージを表示するならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_NigeruExMessage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BOOL result = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, result );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_NIGERU_EXMSG );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();
  return result;
}


//----------------------------------------------------------------------------------
/**
 * メンバー入場コア
 *
 * @param   wk
 * @param   clientID      クライアントID
 * @param   posIdx        入場位置 index
 * @param   nextPokeIdx   入場させるポケモンの現パーティ内 index
 */
//----------------------------------------------------------------------------------
static void scproc_MemberInCore( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 nextPokeIdx )
{
  BTL_PARTY* party;
  BTL_POKEPARAM* bpp;
  u8 pokeID;

  party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );

  {
    SCQUE_PUT_OP_SetIllusionForParty( wk->que, clientID );
    BTL_MAIN_SetIllusionForParty(  wk->mainModule, party, clientID );
  }

  if( posIdx != nextPokeIdx ){
    // クライアントへの通知は SCQUE_PUT_OP_MemberIn
    BTL_PARTY_SwapMembers( party, posIdx, nextPokeIdx );
  }
  bpp = BTL_PARTY_GetMemberData( party, posIdx );
  pokeID = BPP_GetID( bpp );
//  TAYA_Printf("MemberIn pokeID=%d\n", pokeID);

  BTL_MAIN_RegisterZukanSeeFlag( wk->mainModule, clientID, bpp );

  BTL_HANDLER_TOKUSEI_Add( bpp );
  BTL_HANDLER_ITEM_Add( bpp );
  BPP_SetAppearTurn( bpp, wk->turnCount );
  BPP_Clear_ForIn( bpp );
  wk->pokeInFlag[ pokeID ] = TRUE;

  SCQUE_PUT_OP_MemberIn( wk->que, clientID, posIdx, nextPokeIdx, wk->turnCount );
  {
    BtlPokePos  pos = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx );
    BTL_POSPOKE_PokeIn( &wk->pospokeWork, pos, pokeID, wk->pokeCon );
  }
}
//----------------------------------------------------------------------------------
/**
 * メンバー入れ替え
 *
 * @param   wk
 * @param   clientID
 * @param   posIdx
 * @param   next_poke_idx
 * @param   fPutMsg
 */
//----------------------------------------------------------------------------------
static void scproc_MemberInForChange( BTL_SVFLOW_WORK* wk, u8 clientID, u8 posIdx, u8 next_poke_idx, BOOL fPutMsg )
{
  scproc_MemberInCore( wk, clientID, posIdx, next_poke_idx );
  SCQUE_PUT_ACT_MemberIn( wk->que, clientID, posIdx, next_poke_idx, fPutMsg );
}


//----------------------------------------------------------------------------------
/**
 * サーバーフロー：新しく場に入場したポケモン処理
 *
 * @param   wk
 *
 * @retval  BOOL    とくせい・アイテムなど、何らかの効果が発生したらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_AfterMemberIn( BTL_SVFLOW_WORK* wk )
{
  FRONT_POKE_SEEK_WORK fps;
  BTL_POKESET* pokeSet;
  BTL_POKEPARAM* bpp;
  u32 hem_state;
  u8  pokeID, fMemberInEffect = FALSE;

  pokeSet = &wk->pokesetMemberInProc;
  BTL_POKESET_Clear( pokeSet );
  FRONT_POKE_SEEK_InitWork( &fps, wk );

  while( FRONT_POKE_SEEK_GetNext(&fps, wk, &bpp) )
  {
    u8 pokeID = BPP_GetID( bpp );
    if( wk->pokeInFlag[ pokeID ] )
    {
      BTL_POKESET_Add( pokeSet, bpp );
      wk->pokeInFlag[ pokeID ] = FALSE;
    }
  }
  BTL_POKESET_SortByAgility( pokeSet, wk );


  BTL_POKESET_SeekStart( pokeSet );
  while( (bpp = BTL_POKESET_SeekNext(pokeSet)) != NULL )
  {
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_AfterMemberIn( wk, bpp );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
      if( scproc_HandEx_Result(wk) != HandExResult_NULL ){
        fMemberInEffect = TRUE;
      }
    }
  }


  return fMemberInEffect;
}
//----------------------------------------------------------------------------------
/**
 * [Put] メンバー交替時の「○○ もどれ！」などのメッセージ表示コマンド
 *
 * @param   wk
 * @param   bpp   引っ込めるポケモンパラメータ
 */
//----------------------------------------------------------------------------------
static void scPut_MemberOutMessage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  u8 pokeID = BPP_GetID( bpp );
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );

  SCQUE_PUT_ACT_MemberOutMsg( wk->que, clientID, pokeID );
}
//-----------------------------------------------------------------------------------
// サーバーフロー：メンバー交替
//-----------------------------------------------------------------------------------
static void scproc_MemberChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, u8 nextPokeIdx )
{
  scPut_MemberOutMessage( wk, outPoke );

  if( scproc_MemberOutForChange(wk, outPoke, FALSE) )
  {
    BtlPokePos pos;
    u8 clientID, posIdx;
    u8 outPokeID = BPP_GetID( outPoke );

    pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, outPokeID );
    BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, pos, &clientID, &posIdx );

    if( wk->memberChangeCount[clientID] < MEMBER_CHANGE_COUNT_MAX ){
      wk->memberChangeCount[clientID]++;
    }

    scproc_MemberInForChange( wk, clientID, posIdx, nextPokeIdx, TRUE );
  }
}
//----------------------------------------------------------------------------------
/**
 * メンバーを場から退場させる（入れ替え処理前）
 *
 * @param   wk
 * @param   bpp           退場させるポケモンパラメータ
 * @param   fIntrDisable  割り込みアクション（おいうちなど）を禁止するフラグ
 *
 * @retval  BOOL    退場できたらTRUE／割り込まれて死んだりしたらFALSE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_MemberOutForChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, BOOL fIntrDisable )
{
  // 割り込みチェック
  if( !fIntrDisable )
  {
    u32 intrPokeCount = scEvent_MemberChangeIntr( wk, outPoke );
    if( intrPokeCount )
    {
      u32 i;
      wk->fMemberOutIntr = TRUE;
      for(i=0; i<intrPokeCount; ++i)
      {
        ActOrder_IntrProc( wk, wk->MemberOutIntrPokeID[i] );
        if( BPP_IsDead(outPoke) ){
          break;
        }
      }
      wk->fMemberOutIntr = FALSE;
    }
  }

  if( !BPP_IsDead(outPoke) )
  {
    scproc_MemberOutCore( wk, outPoke, BTLEFF_POKEMON_MODOSU );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * メンバー退場（入れ替え・吹き飛ばし・死亡）共通処理
 *
 * @param   wk
 * @param   outPoke
 * @param   effectNo   エフェクトナンバー(0=エフェクトなし）
 *
 */
//----------------------------------------------------------------------------------
static void scproc_MemberOutCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke, u16 effectNo )
{
  u8 pokeID = BPP_GetID( outPoke );


  {
    BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );
    if( pos != BTL_POS_NULL )
    {
      SCQUE_PUT_ACT_MemberOut( wk->que, pos, effectNo );
    }
  }

  ActOrder_ForceDone( wk, pokeID );

  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    scEvent_MemberOutFixed( wk, outPoke );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }

  scproc_ClearPokeDependEffect( wk, outPoke );
  BPP_Clear_ForOut( outPoke );
  SCQUE_PUT_OP_OutClear( wk->que, BPP_GetID(outPoke) );

  BTL_POSPOKE_PokeOut( &wk->pospokeWork, pokeID );
}
//----------------------------------------------------------------------------------
/**
 * [Event] メンバー退場の確定
 *
 * @param   wk
 * @param   outPoke
 */
//----------------------------------------------------------------------------------
static void scEvent_MemberOutFixed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* outPoke )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(outPoke) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_MEMBER_OUT_FIXED );
  BTL_EVENTVAR_Pop();
}

//-----------------------------------------------------------------------------------
// サーバーフロー：「たたかう」ルート
//-----------------------------------------------------------------------------------
static void scproc_Fight( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_ACTION_PARAM* action )
{
  REQWAZA_WORK  reqWaza;
  WazaID  orgWaza, actWaza;
  BtlPokePos  orgTargetPos, actTargetPos;
  BppContFlag  tameFlag;
  FLAG_SET  wazaFlags;
  u8 fWazaEnable, fWazaLock, fReqWaza, fPPDecrement, orgWazaIdx;

  tameFlag = BPP_CONTFLAG_CheckWazaHide( attacker );

  wazaEffCtrl_Init( wk->wazaEffCtrl );
  wazaRobParam_Init( wk->magicCoatParam );

  wazaFlags.raw = 0;
  reqWaza.wazaID = WAZANO_NULL;
  reqWaza.targetPos = BTL_POS_NULL;

  /* アンコール状態で違うワザを選んでいた場合は強制書き換え */
  if( BPP_CheckSick(attacker, WAZASICK_ENCORE) )
  {
    if( action->fight.waza != WAZANO_WARUAGAKI )
    {
      WazaID encoreWaza = BPP_SICKCONT_GetParam( BPP_GetSickCont(attacker, WAZASICK_ENCORE) );
      if( (encoreWaza != action->fight.waza)
      &&  (BPP_WAZA_GetPP_ByNumber(attacker, encoreWaza))
      ){
        action->fight.waza = encoreWaza;
      }
    }
  }

  orgWaza = action->fight.waza;
  orgWazaIdx = BPP_WAZA_SearchIdx( attacker, orgWaza );
  actWaza = orgWaza;
  orgTargetPos = action->fight.targetPos;
  actTargetPos = orgTargetPos;
  fWazaEnable = FALSE;
  fPPDecrement = FALSE;
  fWazaLock = BPP_CheckSick(attacker, WAZASICK_WAZALOCK) || BPP_CheckSick(attacker, WAZASICK_TAMELOCK);

  BTL_HANDLER_Waza_Add( attacker, orgWaza );
  scproc_StartWazaSeq( wk, attacker, orgWaza );

  do {

    // ワザ出し失敗判定１（ポケモン系状態異常＆こんらん、メロメロ、ひるみ）
    if( scproc_Fight_CheckWazaExecuteFail_1st(wk, attacker, orgWaza, fWazaLock ) ){ break; }

    // 派生ワザ呼び出しパラメータチェック＆失敗判定
    if( !scEvent_GetReqWazaParam(wk, attacker, orgWaza, orgTargetPos, &reqWaza) )
    {
      scPut_WazaMsg( wk, attacker, orgWaza );
      scEvent_GetWazaParam( wk, orgWaza, attacker, wk->wazaParamOrg );
      BTL_SVFSUB_RegisterTargets( wk, attacker, orgTargetPos, wk->wazaParamOrg, wk->psetTargetOrg );
      scproc_decrementPPUsedWaza( wk, attacker, orgWaza, orgWazaIdx, wk->psetTargetOrg );
      fPPDecrement = TRUE;
      scproc_WazaExecuteFailed( wk, attacker, orgWaza, SV_WAZAFAIL_OTHER );
      break;
    }

    // 派生ワザ呼び出しする場合、メッセージ出力＆ワザ出し確定イベント
    fReqWaza = ( reqWaza.wazaID != WAZANO_NULL );
    if( fReqWaza )
    {
      scPut_WazaMsg( wk, attacker, orgWaza );
      scPut_ReqWazaEffect( wk, attacker, orgWaza, orgTargetPos );
      BTL_HANDLER_Waza_Add( attacker, reqWaza.wazaID );
      actWaza = reqWaza.wazaID;
      actTargetPos = reqWaza.targetPos;
    }else{
      actWaza = orgWaza;
      actTargetPos = orgTargetPos;
    }

    // ワザパラメータ確定
    scEvent_GetWazaParam( wk, orgWaza, attacker, wk->wazaParamOrg );
    if( fReqWaza )
    {
      scEvent_GetWazaParam( wk, actWaza, attacker, wk->wazaParam );
      wk->wazaParam->fReqWaza = TRUE;
      wk->wazaParam->orgWazaID = orgWaza;
    }
    else{
      *(wk->wazaParam) = *(wk->wazaParamOrg);
    }

    // ワザ対象をワークに取得
    BTL_POKESET_Clear( wk->psetTargetOrg );
    BTL_POKESET_Clear( wk->psetTarget );
    BTL_SVFSUB_RegisterTargets( wk, attacker, actTargetPos, wk->wazaParam, wk->psetTargetOrg );
    {
      u8 defTargetCnt = BTL_POKESET_GetCountMax( wk->psetTargetOrg );
      BTL_POKESET_RemoveDeadPoke( wk->psetTargetOrg );
      BTL_POKESET_CopyAlive( wk->psetTargetOrg, wk->psetTarget );
      BTL_POKESET_SetDefaultTargetCount( wk->psetTarget, defTargetCnt );
      TAYA_Printf("TargetCntMax=%d\n", BTL_POKESET_GetCount(wk->psetTarget) );
    }

    // 使ったワザのPP減らす（前ターンからロックされている場合は減らさない）
    if( (!fWazaLock) )
    {
      if( orgWazaIdx != PTL_WAZA_MAX )
      {
        scproc_decrementPPUsedWaza( wk, attacker, orgWaza, orgWazaIdx, wk->psetTargetOrg );
        fPPDecrement = TRUE;
      }
    }

    // ワザメッセージ出力確定
    scproc_WazaExe_Decide( wk, attacker, wk->wazaParamOrg, BTL_EVENT_WAZA_CALL_DECIDE );

    // 合体ワザ（後発）の発動チェック
    scEvent_CheckCombiWazaExe( wk, attacker, wk->wazaParam );

    // ワザメッセージ出力
    if( scEvent_CheckWazaMsgCustom(wk, attacker, orgWaza, actWaza, &wk->strParam) ){
      // 他ワザ呼び出し時など、ワザ名メッセージをカスタマイズした場合
      handexSub_putString( wk, &wk->strParam );
      HANDEX_STR_Clear( &wk->strParam );
    }else{
      scPut_WazaMsg( wk, attacker, actWaza );
    }

    // ワザ出し失敗判定２（ＰＰ減少後）
    if( scproc_Fight_CheckWazaExecuteFail_2nd(wk, attacker, actWaza, fWazaLock )){
       break;
    }

    // 遅延発動ワザの準備処理
    if( scproc_Fight_CheckDelayWazaSet(wk, attacker, actWaza, actTargetPos) ){ break; }

    // 合体ワザ（先発）の準備処理
    if( scproc_Fight_CheckCombiWazaReady(wk, attacker, actWaza, actTargetPos) ){ break; }

    // ここまで来たらワザが出ることは確定
    BTL_WAZAREC_Add( &wk->wazaRec, actWaza, wk->turnCount, BPP_GetID(attacker) );
    scproc_WazaExe_Decide( wk, attacker, wk->wazaParam, BTL_EVENT_WAZA_EXE_DECIDE );

    // ワザ乗っ取り判定
    if( scproc_Check_WazaRob(wk, attacker, actWaza, wk->psetTarget, wk->wazaRobParam) )
    {
      BTL_N_Printf( DBGSTR_SVFL_YokodoriDetermine, BPP_GetID(attacker), actWaza );
      scproc_WazaRobRoot( wk, attacker, actWaza, wk->psetTarget );
    }
    // 乗っ取られず通常処理
    else
    {
      fWazaEnable = scproc_Fight_WazaExe( wk, attacker, actWaza, wk->psetTarget );
    }

    wk->prevExeWaza = actWaza;

  }while(0);

  // ワザプロセス修了した
  BPP_TURNFLAG_Set( attacker, BPP_TURNFLG_WAZAPROC_DONE );

  // PP消費したらワザ使用記録を更新
  if( fPPDecrement )
  {
    BPP_UpdateWazaProcResult( attacker, actTargetPos, fWazaEnable, wk->wazaParam->wazaType, actWaza, orgWaza );
    SCQUE_PUT_OP_UpdateWazaProcResult( wk->que, BPP_GetID(attacker), actTargetPos, fWazaEnable,
                            wk->wazaParam->wazaType, actWaza, orgWaza );
  }

  // 溜めワザ解放ターンの失敗に対処
  if( (tameFlag != BPP_CONTFLG_NULL) && (BPP_CONTFLAG_CheckWazaHide(attacker) != BPP_CONTFLG_NULL) ){
    scproc_TameLockClear( wk, attacker );
  }
  if( BPP_TURNFLAG_Get(attacker, BPP_TURNFLG_TAMEHIDE_OFF )
  ){
    SCQUE_PUT_ACT_TameWazaHide( wk->que, BPP_GetID(attacker), FALSE );
    scproc_FreeFall_CheckRelease( wk, attacker, FALSE );
  }

  scproc_EndWazaSeq( wk, attacker, actWaza, fWazaEnable );

  if( reqWaza.wazaID != WAZANO_NULL ){
    BTL_HANDLER_Waza_Remove( attacker, reqWaza.wazaID );
  }
  BTL_HANDLER_Waza_Remove( attacker, orgWaza );

  scproc_MagicCoat_Root( wk, actWaza );
}
//----------------------------------------------------------------------------------
/**
 * マジックコート処理ルート
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void scproc_MagicCoat_Root( BTL_SVFLOW_WORK* wk, WazaID actWaza )
{
  BTL_POKEPARAM *robPoke, *targetPoke;
  u32 i;

  BTL_N_Printf( DBGSTR_SVFL_MagicCoatStart, wk->magicCoatParam->robberCount);

  for(i=0; i<wk->magicCoatParam->robberCount; ++i)
  {
    robPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, wk->magicCoatParam->robberPokeID[i] );
    targetPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, wk->magicCoatParam->targetPokeID[i] );

    BTL_N_Printf( DBGSTR_SVFL_MagicCoatReflect,
      wk->magicCoatParam->robberPokeID[i],
      wk->magicCoatParam->targetPokeID[i],
      wk->magicCoatParam->targetPos[i] );

    if( BPP_IsFightEnable(robPoke) )
    {
      // 跳ね返し確定イベント
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_WazaReflect( wk, robPoke, targetPoke, actWaza );
//      scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
      BTL_Hem_PopState( &wk->HEManager, hem_state );

      // その後、跳ね返し処理
      // ワザパラメータ差し替え
      scEvent_GetWazaParam( wk, actWaza, robPoke, wk->wazaParam );
      wk->wazaParam->fMagicCoat = TRUE;
      BTL_SVFSUB_RegisterTargets( wk, robPoke, wk->magicCoatParam->targetPos[i], wk->wazaParam, wk->psetRobTarget );

      BTL_HANDLER_Waza_Add( robPoke, actWaza );
      scproc_Fight_WazaExe( wk, robPoke, actWaza, wk->psetRobTarget );
//      scproc_EndWazaSeq( wk, robPoke, actWaza );

      BTL_HANDLER_Waza_RemoveForce( robPoke, actWaza );
    }
  }
}


static void wazaRobParam_Init( WAZA_ROB_PARAM* param )
{
  u32 i;
  param->robberCount = 0;
  for(i=0; i<NELEMS(param->robberPokeID); ++i){
    param->robberPokeID[i] = BTL_POKEID_NULL;
  }
}
static void wazaRobParam_Add( WAZA_ROB_PARAM* param, u8 robberPokeID, u8 targetPokeID, BtlPokePos targetPos )
{
  if( param->robberCount < (NELEMS(param->robberPokeID)) )
  {
    param->robberPokeID[ param->robberCount ] = robberPokeID;
    param->targetPokeID[ param->robberCount ] = targetPokeID;
    param->targetPos[ param->robberCount ] = targetPos;
    param->robberCount++;
  }
}

//----------------------------------------------------------------------------------
/**
 * ワザ乗っ取り処理ルート
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   defaultTarget
 */
//----------------------------------------------------------------------------------
static void scproc_WazaRobRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID actWaza, BTL_POKESET* defaultTarget )
{
  BtlPokePos robTargetPos;

  // 誰かが乗っ取った
  if( wk->wazaRobParam->robberCount )
  {
    u8 robPokeID = wk->wazaRobParam->robberPokeID[ 0 ];
    BTL_POKEPARAM* robPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, robPokeID );

    // 乗っ取り（乗っ取ったポケが主体になり、指定されたターゲットにワザを出す）
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_WazaRob( wk, robPoke, attacker, actWaza );
      BTL_Hem_PopState( &wk->HEManager, hem_state );

      // ワザパラメータ差し替え
      scEvent_GetWazaParam( wk, actWaza, robPoke, wk->wazaParam );
      BTL_SVFSUB_RegisterTargets( wk, robPoke, wk->wazaRobParam->targetPos[0], wk->wazaParam, wk->psetRobTarget );

      BTL_N_Printf( DBGSTR_SVFL_YokodoriExe, robPokeID, wk->wazaRobParam->targetPos[0] );

      // かいふくふうじチェック
      if( BPP_CheckSick(robPoke, WAZASICK_KAIHUKUHUUJI)
      &&  (WAZADATA_GetFlag(actWaza, WAZAFLAG_KaifukuHuuji))
      ){
        scPut_WazaExecuteFailMsg( wk, robPoke, actWaza, SV_WAZAFAIL_KAIHUKUHUUJI );
        return;
      }

      BTL_HANDLER_Waza_Add( robPoke, actWaza );
      scproc_Fight_WazaExe( wk, robPoke, actWaza, wk->psetRobTarget );
      BTL_HANDLER_Waza_RemoveForce( robPoke, actWaza );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] 合体ワザの成立チェック
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_CheckCombiWazaExe( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* wazaParam )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_COMBIWAZA_CHECK );
    wazaParam->wazaType = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * ワザだし確定イベント呼び出し処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExe_Decide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, BtlEventType evType )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  scEvent_WazaExeDecide( wk, attacker, wazaParam, evType );
//  scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザだし確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaExeDecide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, BtlEventType evType  )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENT_CallHandlers( wk, evType );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * 必要なら、合体ワザの発動準備処理を行う
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   targetPos
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_CheckCombiWazaReady( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlPokePos targetPos )
{
  static const WazaID CombiWazaTbl[] = {
    WAZANO_HONOONOTIKAI,  WAZANO_MIZUNOTIKAI,  WAZANO_KUSANOTIKAI,
  };
  u32 i;

  for(i=0; i<NELEMS(CombiWazaTbl); ++i)
  {
    if( CombiWazaTbl[i] == waza ){
      BTL_N_Printf( DBGSTR_SVFL_CombiWazaCheck, waza);
      break;
    }
  }
  if( (i != NELEMS(CombiWazaTbl))
  &&  (!BPP_TURNFLAG_Get(attacker, BPP_TURNFLG_COMBIWAZA_READY))
  &&  (!BPP_CombiWaza_IsSetParam(attacker))
  ){
    enum {
      COMBI_MAX = BTL_POSIDX_MAX - 1,   // 合体ワザを一緒に撃ってくれる論理上の最大ポケ数
    };
    ACTION_ORDER_WORK*  orderWork[ COMBI_MAX ];
    u32 orderCnt = 0;
    u8 pokeID = BPP_GetID( attacker );

    for(i=0; i<NELEMS(CombiWazaTbl); ++i)
    {
      if( CombiWazaTbl[i] != waza )
      {
        orderWork[ orderCnt ] = ActOrderTool_SearchForCombiWaza( wk, CombiWazaTbl[i], pokeID, targetPos );
        if( orderWork[ orderCnt ] != NULL ){
          BTL_N_Printf( DBGSTR_SVFL_CombiWazaFound,
                    targetPos, CombiWazaTbl[i], BPP_GetID(orderWork[orderCnt]->bpp) );
          if( ++orderCnt >= COMBI_MAX ){ break; }
        }
      }
    }

    if( orderCnt )
    {
      BTL_POKEPARAM* combiPoke;
      u8 idx, idxMin, combiPokeID;

      idxMin = BTL_POS_MAX;
      combiPoke = orderWork[ 0 ]->bpp;

      for(i=0; i<orderCnt; ++i)
      {
        idx = ActOrderTool_GetIndex( wk, orderWork[i] );
        if( idx < idxMin ){
          idxMin = idx;
          combiPoke = orderWork[i]->bpp;
        }
      }

      combiPokeID = BPP_GetID( combiPoke );
      BTL_N_Printf( DBGSTR_SVFL_CombiDecide, pokeID, combiPokeID );

      BPP_TURNFLAG_Set( attacker, BPP_TURNFLG_COMBIWAZA_READY );
      BPP_CombiWaza_SetParam( combiPoke, pokeID, waza );
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_CombiWazaReady, pokeID, combiPokeID );

      ActOrder_IntrReserve( wk, combiPokeID );
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 必要なら、遅延発動ワザの準備処理を行う。
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   targetPos
 *
 * @retval  BOOL      遅延発動ワザ準備処理を行った場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_CheckDelayWazaSet( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlPokePos targetPos )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  BOOL result = scEvent_CheckDelayWazaSet( wk, attacker, targetPos );

  if( result )
  {
    BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );
    if( scproc_HandEx_Result(wk) == HandExResult_Enable )
    {
      SCQUE_PUT_ACT_WazaEffect( wk->que, atPos, targetPos, waza, BTLV_WAZAEFF_DELAY_START );
      scproc_Fight_DecideDelayWaza( wk, attacker );
    }
    else{
      scPut_WazaFail( wk, attacker, waza );
    }
  }

  BTL_Hem_PopState( &wk->HEManager, hem_state );
  return result;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 遅延ワザ準備チェック
 *
 * @param   wk
 * @param   attacker
 * @param   targetPos
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckDelayWazaSet( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BtlPokePos targetPos )
{
  BOOL result;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEPOS, targetPos );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_DELAY_WAZA );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return result;
}
//----------------------------------------------------------------------------------
/**
 * 遅延発動ワザの実行確定（ワザエフェクト表示後）
 *
 * @param   wk
 * @param   attacker
 */
//----------------------------------------------------------------------------------
static void scproc_Fight_DecideDelayWaza( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

    scEvent_DecideDelayWaza( wk, attacker );
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );

  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] 遅延ワザ発動確定
 *
 * @param   wk
 * @param   attacker
 */
//----------------------------------------------------------------------------------
static void scEvent_DecideDelayWaza( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECIDE_DELAY_WAZA );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * ワザシーケンス開始処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_StartWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_StartWazaSeq( wk, attacker, waza );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザシーケンス開始
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_StartWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASEQ_START );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * ワザシーケンス終了処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_EndWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaEnable )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_EndWazaSeq( wk, attacker, waza, fWazaEnable );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザシーケンス終了
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_EndWazaSeq( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaEnable )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_GEN_FLAG, fWazaEnable );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASEQ_END );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ乗っ取り確定
 *
 * @param   wk
 * @param   robPoke       乗っ取った側
 * @param   orgAtkPoke    元々、ワザを出した側
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* robPoke, const BTL_POKEPARAM* orgAtkPoke, WazaID waza )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(robPoke) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(orgAtkPoke) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASEQ_ROB );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] マジックコート跳ね返し確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaReflect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* robPoke, const BTL_POKEPARAM* orgAtkPoke, WazaID waza )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(robPoke) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(orgAtkPoke) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASEQ_REFRECT );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * わざ乗っ取りチェック
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   rec       [io] 本来のターゲット群。乗っ取ったポケはここから除外される。
 * @param   targetPos [out] 乗っ取った場合、その対象位置を保持
 *
 * @retval  u8    乗っ取ったポケモンのID／乗っ取ったポケがいない場合はBTL_POKEID_NULL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Check_WazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza,
  BTL_POKESET* rec, WAZA_ROB_PARAM* robParam )
{
  u8 robberPokeID = BTL_POKEID_NULL;
  u8 robTargetPokeID = BTL_POKEID_NULL;

  scEvent_CheckWazaRob( wk, attacker, waza, rec, &robberPokeID, &robTargetPokeID );
  if( robberPokeID != BTL_POKEID_NULL )
  {
    if( robTargetPokeID != BTL_POKEID_NULL ){
      robParam->targetPos[0] = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, robTargetPokeID );
    }else{
      robParam->targetPos[0] = BTL_POS_NULL;
    }

    BTL_N_Printf( DBGSTR_SVFL_YokodoriInfo, robTargetPokeID, robParam->targetPos[0]);

    robParam->robberPokeID[0] = robberPokeID;
    robParam->robberCount = 1;

    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 自分以外のポケが出したワザを乗っ取る判定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   targetPos
 * @param   robberPokeID    [out] 乗っ取ったポケモンID
 * @param   robTargetPokeID [out] 乗っ取った場合の対象ポケモンID
 * @param   strParam        [out] 乗っ取った場合のメッセージパラメータ
 *
 * @retval  BOOL    乗っ取ったらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckWazaRob( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza,
  BTL_POKESET* targetRec, u8* robberPokeID, u8* robTargetPokeID )
{
  u32 targetCnt = BTL_POKESET_GetCount( targetRec );
  u32 i;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, targetCnt );
    {
      BTL_POKEPARAM* bpp;
      for(i=0; i<targetCnt; ++i)
      {
        bpp = BTL_POKESET_Get( targetRec, i );
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(bpp) );
      }
    }
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_POKEID, BTL_POKEID_NULL );
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BTL_POKEID_NULL );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_WAZA_ROB );

    *robberPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
    *robTargetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );

  BTL_EVENTVAR_Pop();

  return (*robberPokeID) != BTL_POKEID_NULL;
}

// 他ワザ呼び出しするワザで、かつ失敗する場合のみTRUE
static BOOL scproc_Fight_CheckReqWazaFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID orgWazaID )
{
  BOOL failFlag = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, orgWazaID );
    BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, failFlag );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_REQWAZA_CHECKFAIL );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  return failFlag;
}
// 他ワザ呼び出しするワザのエフェクトを発動
static void scPut_ReqWazaEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, BtlPokePos targetPos )
{
  BtlPokePos  atkPos;
  u8 pokeID = BPP_GetID( bpp );

  atkPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );

  SCQUE_PUT_ACT_WazaEffect( wk->que, atkPos, targetPos, waza, 0 );
}

//----------------------------------------------------------------------------------
/**
 * ワザ出しレコード更新
 *
 * @param   wk
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExeRecordUpdate( BTL_SVFLOW_WORK* wk, WazaID waza )
{
  if( waza == WAZANO_HANERU ){
    BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_WAZA_HANERU );
    return;
  }
  if( waza == WAZANO_WARUAGAKI ){
    BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_WAZA_WARUAGAKI );
    return;
  }
}
//----------------------------------------------------------------------------------
/**
 * 「たたかう」-> ワザ出し成功以降の処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 *
 * @retval  BOOL  ワザが無効に終わった場合FALSE／それ以外はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_WazaExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BTL_POKESET* targetRec )
{
  WazaCategory  category = WAZADATA_GetCategory( waza );
  u8 pokeID = BPP_GetID( attacker );
  u8 fEnable = TRUE;
  u8 fDamage = FALSE;

  u16  que_reserve_pos;

  BTL_N_Printf( DBGSTR_SVFL_WazaExeStart, BPP_GetID(attacker), waza );

  // レコードデータ更新
  scproc_WazaExeRecordUpdate( wk, waza );

  // １ターン溜めワザの発動チェック
  if( WAZADATA_GetFlag(waza, WAZAFLAG_Tame) )
  {
    TameWazaResult tameResult = scproc_Fight_TameWazaExe( wk, attacker, targetRec, waza );
    switch( tameResult ){
    case TAMEWAZA_START_FAIL:    ///< 溜め開始ターン：失敗
      return FALSE;
    case TAMEWAZA_START_OK:      ///< 溜め開始ターン：成功
      return TRUE;
    case TAMEWAZA_RELEASE_FAIL:  ///< 溜め解放ターン：失敗
      return FALSE;
    case TAMEWAZA_RELEASE_OK:    ///< 溜め解放ターン：成功
    default:
      break;
    }
  }

  // ダメージ受けポケモンワークをクリアしておく
  BTL_POKESET_Clear( wk->psetDamaged );

  // ワザエフェクトコマンド生成用にバッファ領域を予約しておく
  que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_WAZA_EFFECT );
  wazaEffCtrl_Setup( wk->wazaEffCtrl, wk, attacker, targetRec );

  // ワザ出し処理開始イベント
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    BtlWazaForceEnableMode enableMode = scEvent_WazaExecuteStart( wk, attacker, waza, targetRec );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
    if( enableMode != BTL_WAZAENABLE_NONE )
    {
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
      if( enableMode == BTL_WAZAENABLE_QUIT ){
        return TRUE;
      }
    }
  }

  // 死んでるポケモンは除外
  BTL_POKESET_RemoveDeadPoke( targetRec );
  // 最初は居たターゲットが残っていない->うまく決まらなかった、終了
  if( BTL_POKESET_IsRemovedAll(targetRec) ){
    scPut_WazaFail( wk, attacker, waza );
    fEnable = FALSE;
  }

  if( fEnable )
  {
    BOOL fMigawariHit = FALSE;

    switch( category ){
    case WAZADATA_CATEGORY_SIMPLE_DAMAGE:
    case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
    case WAZADATA_CATEGORY_DAMAGE_EFFECT:
    case WAZADATA_CATEGORY_DAMAGE_SICK:
    case WAZADATA_CATEGORY_DRAIN:
      fDamage = TRUE;
    }

    // トリプル遠隔位置によるハズレ
    scproc_CheckTripleFarPokeAvoid( wk, wk->wazaParam, attacker, targetRec );

    // タイプによる無効化チェック
    if( fDamage || (category == WAZADATA_CATEGORY_ICHIGEKI) ){
      flowsub_checkWazaAffineNoEffect( wk, wk->wazaParam, attacker, targetRec, &wk->dmgAffRec );
      fMigawariHit = TRUE;
    }

    // 対象ごとの無効チェック->回避チェック（原因表示はその先に任せる）

    if( category != WAZADATA_CATEGORY_ICHIGEKI ){  // 一撃ワザは独自の回避判定を行うので
      flowsub_CheckPokeHideAvoid( wk, wk->wazaParam, attacker, targetRec );
    }

    flowsub_checkNotEffect( wk, wk->wazaParam, attacker, targetRec );

    if( category != WAZADATA_CATEGORY_ICHIGEKI ){  // 一撃ワザは独自の回避判定を行うので
      flowsub_checkWazaAvoid( wk, wk->wazaParam, attacker, targetRec );
    }

    // 最初は居たターゲットが残っていない -> 無効イベント呼び出し後終了
    if( BTL_POKESET_IsRemovedAll(targetRec) )
    {
      scproc_WazaExe_NotEffective( wk, pokeID, waza );
      fEnable = FALSE;
    }
    else
    {
      // ダメージワザ以外のみがわり除外チェック
      scproc_MigawariExclude( wk, wk->wazaParam, attacker, targetRec, fMigawariHit );
      // ターゲットが残っていない -> 無効イベント呼び出し後終了
      if( BTL_POKESET_IsRemovedAll(targetRec) )
      {
        scPut_WazaFail( wk, attacker, waza );
        scproc_WazaExe_NotEffective( wk, pokeID, waza );
        fEnable = FALSE;
      }
    }
  }

  if( fEnable )
  {
    BTL_Printf("ワザ=%d, カテゴリ=%d\n", wk->wazaParam.wazaID, category );

    if( fDamage ){
      scproc_Fight_Damage_Root( wk, wk->wazaParam, attacker, targetRec, &wk->dmgAffRec, FALSE );
    }
    else{

      switch( category ){
      case WAZADATA_CATEGORY_SIMPLE_EFFECT:
        scproc_Fight_SimpleEffect( wk, wk->wazaParam, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_SIMPLE_SICK:
        scproc_Fight_SimpleSick( wk, waza, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_EFFECT_SICK:
        scproc_Fight_EffectSick( wk, wk->wazaParam, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_ICHIGEKI:
        scproc_Fight_Ichigeki( wk, wk->wazaParam, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_PUSHOUT:
        scproc_Fight_PushOut( wk, waza, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_SIMPLE_RECOVER:
        scproc_Fight_SimpleRecover( wk, waza, attacker, targetRec );
        break;
      case WAZADATA_CATEGORY_FIELD_EFFECT:
        scput_Fight_FieldEffect( wk, wk->wazaParam, attacker );
        break;
      case WAZADATA_CATEGORY_SIDE_EFFECT:
      case WAZADATA_CATEGORY_OTHERS:
        scput_Fight_Uncategory( wk, wk->wazaParam, attacker, targetRec );
        break;
      default:
        break;
      }
    }

    // ワザ効果あり→エフェクトコマンド生成などへ
    if( wazaEffCtrl_IsEnable(wk->wazaEffCtrl) )
    {
      if( !wazaEffCtrl_IsDone(wk->wazaEffCtrl) )
      {
        scPut_WazaEffect( wk, waza, wk->wazaEffCtrl, que_reserve_pos );
      }

      BTL_WAZAREC_SetEffectiveLast( &wk->wazaRec );

      // 反動で動けなくなる処理
      if( !BPP_IsDead(attacker) ){
        if( WAZADATA_GetFlag(waza, WAZAFLAG_Tire) ){
          scPut_SetContFlag( wk, attacker, BPP_CONTFLG_CANT_ACTION );
        }
      }

      scproc_WazaExe_Effective( wk, pokeID, waza );

    }else{
      scproc_WazaExe_NotEffective( wk, pokeID, waza );
      fEnable = FALSE;
    }

  }
  else{
    // 無効でもエフェクトのみ発動する（じばく・だいばくはつ対応）
    if( wazaEffCtrl_IsEnable(wk->wazaEffCtrl) ){
      scPut_WazaEffect( wk, waza, wk->wazaEffCtrl, que_reserve_pos );
    }
  }

  scproc_WazaExe_Done( wk, pokeID, waza );
  scproc_CheckDeadCmd( wk, attacker );

  return fEnable;
}
//----------------------------------------------------------------------------------
/**
 * トリプル遠隔位置によるハズレチェック
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   targetRec
 */
//----------------------------------------------------------------------------------
static void scproc_CheckTripleFarPokeAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM* bpp;

  BTL_POKESET_SeekStart( targetRec );
  while( (bpp = BTL_POKESET_SeekNext(targetRec) ) != NULL )
  {
    if( IsTripleFarPos(wk, attacker, bpp, wazaParam->wazaID) ){
      BTL_POKESET_Remove( targetRec, bpp );
      scPut_WazaAvoid( wk, bpp, wazaParam->wazaID );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * みがわり中のポケモンをターゲットから除外
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   target
 * @param   fDamage
 */
//----------------------------------------------------------------------------------
static void scproc_MigawariExclude( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
    const BTL_POKEPARAM* attacker, BTL_POKESET* target, BOOL fDamage )
{
  BTL_POKEPARAM* bpp;

  BTL_POKESET_SeekStart( target );
  while( (bpp = BTL_POKESET_SeekNext(target) ) != NULL )
  {
    if( (attacker != bpp)
    &&  (BPP_MIGAWARI_IsExist(bpp))
    ){
      if( (!fDamage) && (WAZADATA_GetFlag(wazaParam->wazaID, WAZAFLAG_MigawariThru)==FALSE) )
      {
        // ワザパラメータとしてみがわり貫通フラグを持つため、不要になった
//      if( scEvent_CheckMigawariExclude(wk, attacker, bpp, wazaParam->wazaID, fDamage) ){
        BTL_POKESET_Remove( target, bpp );
      }
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] みがわり中ポケモンをワザ対象から除外するかチェック
 *
 * @param   wk
 * @param   attacker    攻撃ポケ
 * @param   defender    ターゲットポケ（みがわり中）
 * @param   fDamage     ダメージを与えるワザか？
 *
 * @retval  BOOL    除外する場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckMigawariExclude( BTL_SVFLOW_WORK* wk,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza, BOOL fDamage )
{
  BOOL fExclude = (!fDamage); // デフォルトではダメージワザなら除外せず、それ以外は除外する

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_NOEFFECT_FLAG, fExclude );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_MIGAWARI_EXCLUDE );
    fExclude = BTL_EVENTVAR_GetValue( BTL_EVAR_NOEFFECT_FLAG );
  BTL_EVENTVAR_Pop();

  return fExclude;
}


//----------------------------------------------------------------------------------
/**
 * ワザ出した->有効だった時のイベント呼び出し
 *
 * @param   wk
 * @param   pokeID
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExe_Effective( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_WazaExeEnd_Common( wk, pokeID, waza, BTL_EVENT_WAZA_EXECUTE_EFFECTIVE );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * ワザ出した->無効だった時のイベント呼び出し
 *
 * @param   wk
 * @param   pokeID  ワザを出したポケモンID
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExe_NotEffective( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_WazaExeEnd_Common( wk, pokeID, waza, BTL_EVENT_WAZA_EXECUTE_NO_EFFECT );
  BTL_Hem_PopState( &wk->HEManager, hem_state );

  BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_WAZA_MUKOU );

  if( BTL_MAINUTIL_PokeIDtoClientID(pokeID) == BTL_MAIN_GetPlayerClientID(wk->mainModule) ){
    BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_WAZA_MUKOU );
  }
}
//----------------------------------------------------------------------------------
/**
 * ワザ出し最終イベント呼び出し
 *
 * @param   wk
 * @param   pokeID
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExe_Done( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_WazaExeEnd_Common( wk, pokeID, waza, BTL_EVENT_WAZA_EXECUTE_DONE );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ出し終了共通
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaExeEnd_Common( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza, BtlEventType eventID )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, pokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENT_CallHandlers( wk, eventID );
  BTL_EVENTVAR_Pop();
}


//----------------------------------------------------------------------------------
/**
 * 必中状態かどうかチェック
 *
 * @param   attacker
 * @param   target
 *
 * @retval  BOOL    必中状態ならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL IsMustHit( const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target )
{
  if( (BPP_GetValue(attacker, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_NOOGAADO)
  ||  (BPP_GetValue(target, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_NOOGAADO)
  ){
    return TRUE;
  }
  if( BPP_CheckSick(attacker, WAZASICK_MUSTHIT) ){
    return TRUE;
  }
  if( BPP_CheckSick(attacker, WAZASICK_MUSTHIT_TARGET) )
  {
    u8 targetPokeID = BPP_GetSickParam( attacker, WAZASICK_MUSTHIT_TARGET );
    TAYA_Printf("pokeID=%d は、%d をロックオンしてる\n", BPP_GetID(attacker), targetPokeID );
    if( targetPokeID == BPP_GetID(target) ){
      return TRUE;
    }
  }

  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * ワザ発動前の、相性による無効化チェック（ダメージワザのみ）
 *
 * @param   wk
 * @param   attacker  [in] ワザを出すポケモン
 * @param   targets   [io] ワザ対象ポケモンコンテナ／無効になったポケモンはコンテナから削除される
 *
 */
//--------------------------------------------------------------------------
static void flowsub_checkWazaAffineNoEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, BTL_DMGAFF_REC* affRec )
{
  BTL_POKEPARAM* bpp;
  BtlTypeAff aff;

  BTL_DMGAFF_REC_Init( affRec );

  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets) ) != NULL )
  {
    aff = scProc_checkWazaDamageAffinity( wk, attacker, bpp, wazaParam, TRUE );
    BTL_DMGAFF_REC_Add( affRec, BPP_GetID(bpp), aff );
    if( aff == BTL_TYPEAFF_0 )
    {
      BTL_POKESET_Remove( targets, bpp );
      AffCounter_CountUp( &wk->affCounter, wk, attacker, bpp, BTL_TYPEAFF_0 );
    }
  }
}
//--------------------------------------------------------------------------
/**
 * ワザ発動前の、対象別無効化チェック
 *
 * @param   wk
 * @param   attacker  [in] ワザを出すポケモン
 * @param   targets   [io] ワザ対象ポケモンコンテナ／無効になったポケモンはコンテナから削除される
 *
 */
//--------------------------------------------------------------------------
static void flowsub_checkNotEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  BTL_POKEPARAM* bpp;

  // Lv1 無効化チェック（攻撃ポケが必中状態なら無効化できない）
  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( !IsMustHit(attacker, bpp) )
    {
      if( scproc_checkNoEffect_sub(wk, wazaParam, attacker, bpp, BTL_EVENT_NOEFFECT_CHECK_L1) ){
        BTL_POKESET_Remove( targets, bpp );
      }
    }
  }

  // Lv2 無効化チェック（攻撃ポケが必中状態でも無効化）
  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( scproc_checkNoEffect_sub(wk, wazaParam, attacker, bpp, BTL_EVENT_NOEFFECT_CHECK_L2) ){
      BTL_POKESET_Remove( targets, bpp );
    }
  }

  // まもるチェック
  if( WAZADATA_GetFlag(wazaParam->wazaID, WAZAFLAG_Mamoru) )
  {
    BTL_POKESET_SeekStart( targets );
    while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
    {
      if( BPP_TURNFLAG_Get(bpp, BPP_TURNFLG_MAMORU) )
      {
        // まもる無効化されなければターゲットから除外
        if( !scEvent_CheckMamoruBreak(wk, attacker, bpp, wazaParam->wazaID) ){
          BTL_POKESET_Remove( targets, bpp );
          SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Mamoru, BPP_GetID(bpp) );
        }
      }
    }
  }

  // Lv3 無効化チェック（攻撃ポケが必中状態でも無効化＆「まもる」より低プライオリティ）
  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( scproc_checkNoEffect_sub(wk, wazaParam, attacker, bpp, BTL_EVENT_NOEFFECT_CHECK_L3) ){
      BTL_POKESET_Remove( targets, bpp );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * ワザ無効チェック＆原因表示
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   target
 * @param   eventID
 *
 * @retval  BOOL    無効だったらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_checkNoEffect_sub( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, BtlEventType eventID )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  BOOL fNoReaction = FALSE;
  BOOL fNoEffect;

  fNoEffect = scEvent_CheckNotEffect( wk, wazaParam, eventID, attacker, target, &wk->strParam, &fNoReaction );
  if( fNoEffect )
  {
    // 文字を出すだけのリアクション
    if( HANDEX_STR_IsEnable(&wk->strParam) ){
      handexSub_putString( wk, &wk->strParam );
      HANDEX_STR_Clear( &wk->strParam );
    }
    else{
      // HandExを利用したリアクション
      if( !scproc_HandEx_Result( wk ) )
      {
        if( !fNoReaction )
        {
          // 何もリアクションなければ「効果がないようだ…」
          SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BPP_GetID(target) );
        }
      }
    }
  }

  BTL_Hem_PopState( &wk->HEManager, hem_state );
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
 * @param   customMsg   無効化した場合に特定のメッセージを出力する場合のパラメータ
 *
 * @retval  BOOL    無効な場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckNotEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BtlEventType eventID,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* customMsg, BOOL* fNoReaction )
{
  BOOL fNoEffect = FALSE;
  HANDEX_STR_Clear( customMsg );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_MAGICCOAT_FLAG, wazaParam->fMagicCoat );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WORK_ADRS, (int)customMsg );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_NOEFFECT_FLAG, fNoEffect );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, eventID );
    fNoEffect = BTL_EVENTVAR_GetValue( BTL_EVAR_NOEFFECT_FLAG );
    (*fNoReaction) = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return fNoEffect;
}
//--------------------------------------------------------------------------
/**
 *  「そらをとぶ」など画面から一時的に消えている状態で外れるケースをチェック
 *  命中しなかった場合は targets から除外
 */
//--------------------------------------------------------------------------
static void flowsub_CheckPokeHideAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  BTL_POKEPARAM* defender;

  BTL_POKESET_SeekStart( targets );

  while( (defender = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( IsMustHit(attacker, defender) ){
      continue;
    }
    if( scEvent_CheckPokeHideAvoid(wk, attacker, defender, wazaParam->wazaID) )
    {
      BTL_POKESET_Remove( targets, defender );
      scPut_WazaAvoid( wk, defender, wazaParam->wazaID );
    }
  }
}

//--------------------------------------------------------------------------
/**
 * ワザ命中判定 -> 命中しなかった場合は targets から除外
 *
 * @param   wk
 * @param   attacker  [in] ワザを出すポケモン
 * @param   targets   [io] ワザ対象ポケモンコンテナ／無効になったポケモンはコンテナから削除される
 *
 */
//--------------------------------------------------------------------------
static void flowsub_checkWazaAvoid( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  BTL_POKEPARAM* bpp;

  // 対象が最初からワザを打ったポケモン１体のワザは命中率などによる判定を行わない
  if( (BTL_POKESET_GetCountMax(targets) == 1)
  &&  (BTL_POKESET_Get(targets,0) == attacker)
  ){
    return;
  }

  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( scEvent_SkipAvoidCheck(wk, attacker, bpp, wazaParam) ){
      continue;
    }
    if( IsTripleFarPos(wk, attacker, bpp, wazaParam->wazaID)
    ||  !scEvent_CheckHit(wk, attacker, bpp, wazaParam)
    ){
      BTL_POKESET_Remove( targets, bpp );
      scPut_WazaAvoid( wk, bpp, wazaParam->wazaID );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ命中判定をスキップするかチェック
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 *
 * @retval  BOOL    スキップする場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_SkipAvoidCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam )
{
  BOOL fSkip = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_SKIP_AVOID_CHECK );
    fSkip = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return fSkip;
}

//----------------------------------------------------------------------------------
/**
 * トリプル時、当たらない位置のポケモンかどうかを判定
 *
 * @param   wk
 * @param   attacker    攻撃するポケ
 * @param   target      攻撃されるポケ
 * @param   waza        ワザID
 *
 * @retval  BOOL        トリプルで当たらない位置ならTRUE／それ以外FALSE
 */
//----------------------------------------------------------------------------------
static BOOL IsTripleFarPos( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaID waza )
{
  if( (BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE)
  &&  (WAZADATA_GetFlag(waza, WAZAFLAG_TripleFar) == FALSE)
  ){
    BtlPokePos  atkPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );
    BtlPokePos  tgtPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(target) );

    if( !BTL_MAINUTIL_CheckTripleHitArea(atkPos, tgtPos) ){
      return TRUE;
    }
  }
  return FALSE;
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
 * @retval  BOOL  当たる場合はTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckHit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam )
{
  if( IsMustHit(attacker, defender) ){
    return TRUE;
  }

//  if( scEvent_CheckPokeHideAvoid(wk, attacker, defender, wazaParam->wazaID) ){
//    return FALSE;
//  }

  if( scEvent_IsCalcHitCancel(wk, attacker, defender, wazaParam->wazaID) ){
    return TRUE;
  }

  if( BPP_CheckSick(defender, WAZASICK_TELEKINESIS) ){
    return TRUE;
  }

  {
    u8 fAvoidFlat;
    s8 hitRank, avoidRank, totalRank;
    u32 totalPer, wazaHitPer;
    fx32 ratio;

    wazaHitPer = scEvent_getHitPer(wk, attacker, defender, wazaParam);

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FLAT_FLAG, FALSE );
      BTL_EVENTVAR_SetValue( BTL_EVAR_HIT_RANK, BPP_GetValue(attacker, BPP_HIT_RATIO) );
      BTL_EVENTVAR_SetValue( BTL_EVAR_AVOID_RANK, BPP_GetValue(defender, BPP_AVOID_RATIO) );
      BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.01f), FX32_CONST(32) );
      BTL_SICKEVENT_CheckHitRatio( wk, attacker, defender );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_RANK );

      hitRank = BTL_EVENTVAR_GetValue( BTL_EVAR_HIT_RANK );
      avoidRank = BTL_EVENTVAR_GetValue( BTL_EVAR_AVOID_RANK );
      fAvoidFlat = BTL_EVENTVAR_GetValue( BTL_EVAR_FLAT_FLAG );
      // 「みやぶる」をされたポケモンは回避率上昇を無視
      if( ((BPP_CheckSick(defender, WAZASICK_MIYABURU)) && (avoidRank > BTL_CALC_HITRATIO_MID))
      ||  (fAvoidFlat)
      ){
        avoidRank = BTL_CALC_HITRATIO_MID;
      }

      ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );

    BTL_EVENTVAR_Pop();

    totalRank = roundValue( (int)(BTL_CALC_HITRATIO_MID + hitRank - avoidRank), BTL_CALC_HITRATIO_MIN, BTL_CALC_HITRATIO_MAX );
    totalPer  = BTL_CALC_HitPer( wazaHitPer, totalRank );

    BTL_N_Printf( DBGSTR_SVFL_HitCheckInfo1, BPP_GetID(attacker), hitRank, wazaHitPer );
    BTL_N_Printf( DBGSTR_SVFL_HitCheckInfo2, BPP_GetID(defender), avoidRank );
    BTL_N_Printf( DBGSTR_SVFL_HitCheckInfo3, totalRank, totalPer, ratio );

    totalPer = BTL_CALC_MulRatio( totalPer, ratio );
    if( totalPer > 100 ){
      totalPer = 100;
    }
    if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_HIT100PER) ){
      totalPer = 100;
    }
    BTL_N_Printf( DBGSTR_SVFL_HitCheckInfo4, totalPer);

    return perOccur( wk, totalPer );
  }
}
//----------------------------------------------------------------------------------
/**
 * ワザ効果あることが確定→必要な演出コマンドの生成
 *
 * @param   wk
 * @param   waza
 * @param   defPokeID
 * @param   que_reserve_pos
 */
//----------------------------------------------------------------------------------
static void scPut_WazaEffect( BTL_SVFLOW_WORK* wk, WazaID waza, WAZAEFF_CTRL* effCtrl, u32 que_reserve_pos )
{
  SCQUE_PUT_ReservedPos( wk->que, que_reserve_pos, SC_ACT_WAZA_EFFECT,
          effCtrl->attackerPos, effCtrl->targetPos, waza, effCtrl->effectIndex );

  BTL_N_Printf( DBGSTR_SVFL_PutWazaEffect, que_reserve_pos, waza, effCtrl->attackerPos, effCtrl->targetPos, effCtrl->effectIndex );

  effCtrl->effectIndex = BTLV_WAZAEFF_INDEX_DEFAULT;
}
//----------------------------------------------------------------------------------
/**
 * 必要なら、１ターン溜めワザの溜めターン処理を行う。
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 *
 * @retval  BOOL      溜めターン処理を行った場合TRUE
 */
//----------------------------------------------------------------------------------
static TameWazaResult scproc_Fight_TameWazaExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, const BTL_POKESET* targetRec, WazaID waza )
{
  if( !BPP_CheckSick(attacker, WAZASICK_TAMELOCK) )
  {
    BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );

    // 溜めターン失敗判定
    if( scEvent_CheckTameFail(wk, attacker, targetRec) )
    {
      scPut_WazaFail( wk, attacker, waza );
      return TAMEWAZA_START_FAIL;
    }

    // 溜めターンスキップ判定
    if( !scEvent_CheckTameTurnSkip(wk, attacker, waza) )
    {
      if( scproc_TameStartTurn(wk, attacker, atPos, targetRec, waza) )
      {
        BPP_SICK_CONT  sickCont = BPP_SICKCONT_MakeTurnParam( 2, waza );
        scPut_AddSick( wk, attacker, WAZASICK_TAMELOCK, sickCont );
      }
      // スキップしない時はここでreturn
      return TAMEWAZA_START_OK;
    }

    if( !scproc_TameStartTurn(wk, attacker, atPos, targetRec, waza) ){
      return TAMEWAZA_START_FAIL;
    }
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_TameSkip( wk, attacker, waza );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
  }

  {
    BOOL fSucceed;
    wazaEffCtrl_SetEffectIndex( wk->wazaEffCtrl, BTLV_WAZAEFF_TAME_RELEASE );
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      fSucceed = scEvent_TameRelease( wk, attacker, targetRec, waza );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
    BPP_TURNFLAG_Set( attacker, BPP_TURNFLG_TAMEHIDE_OFF );
    scproc_TameLockClear( wk, attacker );

    return (fSucceed)? TAMEWAZA_RELEASE_OK : TAMEWAZA_RELEASE_FAIL;
  }
}
//----------------------------------------------------------------------------------
/**
 * 溜め開始ターン処理
 *
 * @param   wk
 * @param   attacker
 * @param   atPos
 * @param   targetRec
 * @param   waza
 */
//----------------------------------------------------------------------------------
static BOOL scproc_TameStartTurn( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos atPos, const BTL_POKESET* targetRec, WazaID waza )
{
  BOOL fSuccess, fFailMsgDisped = FALSE;
  u8 hideTargetPokeID = BTL_POKEID_NULL;

  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  fSuccess = scEvent_TameStart( wk, attacker, targetRec, waza, &hideTargetPokeID, &fFailMsgDisped );

  if( fSuccess )
  {
    BtlPokePos targetPos = wk->defaultTargetPos;
    if( BTL_POKESET_GetCount(targetRec) )
    {
      BTL_POKEPARAM* target = BTL_POKESET_Get( targetRec, 0 );
      targetPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(target) );
    }
    SCQUE_PUT_ACT_WazaEffect( wk->que, atPos, targetPos, waza, BTLV_WAZAEFF_TAME_START );
//    scEvent_TameStartEffect( wk, atPos, waza );
  }
  BTL_Hem_PopState( &wk->HEManager, hem_state );

  if( fSuccess )
  {
    if( BPP_CONTFLAG_CheckWazaHide(attacker) != BPP_CONTFLG_NULL ){
      SCQUE_PUT_ACT_TameWazaHide( wk->que, BPP_GetID(attacker), TRUE );
    }
    if( hideTargetPokeID != BTL_POKEID_NULL ){
      SCQUE_PUT_ACT_TameWazaHide( wk->que, hideTargetPokeID, TRUE );
    }
  }
  else
  {
    if( !fFailMsgDisped ){
      scPut_WazaFail( wk, attacker, waza );
    }
  }

  return fSuccess;
}
static void scproc_TameLockClear( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker )
{
  if( BPP_CheckSick(attacker, WAZASICK_TAMELOCK) )
  {
    scPut_CureSick( wk, attacker, WAZASICK_TAMELOCK, NULL );
    BPP_TURNFLAG_Set( attacker, BPP_TURNFLG_TAMEHIDE_OFF );
  }

  {
    BppContFlag contFlag = BPP_CONTFLAG_CheckWazaHide( attacker );
    while( contFlag != BPP_CONTFLG_NULL )
    {
      scPut_ResetContFlag( wk, attacker, contFlag );
      contFlag = BPP_CONTFLAG_CheckWazaHide( attacker );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * フリーフォール溜めターン開始処理
 *
 * @param   wk
 * @param   attacker
 * @param   target
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_FreeFall_Start( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL* fFailMsgDisped )
{
  *fFailMsgDisped = FALSE;

  // 対象が死んでる・みがわり・ワザ効果で場から消えてる場合は失敗
  if( (!BPP_IsDead(target))
  &&  (!BPP_MIGAWARI_IsExist(target))
  &&  (BPP_CONTFLAG_CheckWazaHide(target) == BPP_CONTFLG_NULL)
  ){
    // さらに守ってなければ成功
    if( !BPP_TURNFLAG_Get(target, BPP_TURNFLG_MAMORU) )
    {
      BPP_SICK_CONT  cont;
      u8 counterValue = BPP_PokeIDtoFreeFallCounter( BPP_GetID(target) );

      scPut_SetContFlag( wk, attacker, BPP_CONTFLG_SORAWOTOBU );
      scPut_SetContFlag( wk, target, BPP_CONTFLG_SORAWOTOBU );
      scPut_SetBppCounter( wk, attacker, BPP_COUNTER_FREEFALL, counterValue );

      cont = BPP_SICKCONT_MakePoke( BPP_GetID(attacker) );
      scproc_AddSickCore( wk, target, attacker, WAZASICK_FREEFALL, cont, FALSE, NULL );

      return TRUE;
    }
    // 守った場合はメッセージ表示
    else
    {
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Mamoru, BPP_GetID(target) );
      *fFailMsgDisped = TRUE;
    }
  }
  return FALSE;
}
/**
 *  フリーフォール（攻撃側）で空中に居る状態かどうか判定
 */
static inline BOOL checkFreeFallUsing( const BTL_POKEPARAM* bpp )
{
  u8 capturedPokeID = BPP_FreeFallCounterToPokeID( BPP_COUNTER_Get(bpp, BPP_COUNTER_FREEFALL) );
//  if( BPP_FreeFallCounterToPokeID( BPP_COUNTER_Get(bpp, BPP_COUNTER_FREEFALL) != BTL_POKEID_NULL ) ){
  TAYA_Printf("pokeID-%d, captureCheck ... %d\n", BPP_GetID(bpp), capturedPokeID);
  if( capturedPokeID != BTL_POKEID_NULL ){
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * フリーフォールでポケモンをつかんだ状態ならリリースする処理
 *
 * @param   fldSit
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scproc_FreeFall_CheckRelease( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BOOL fReleaseProc )
{
  u8 capturedPokeID = BPP_FreeFallCounterToPokeID( BPP_COUNTER_Get(bpp, BPP_COUNTER_FREEFALL) );
  if( capturedPokeID != BTL_POKEID_NULL )
  {
    BTL_POKEPARAM* capturedBpp = BTL_POKECON_GetPokeParam( wk->pokeCon, capturedPokeID );

    if( BPP_CheckSick(capturedBpp, WAZASICK_FREEFALL) )
    {
      scPut_CureSick( wk, capturedBpp, WAZASICK_FREEFALL, NULL );
      scPut_ResetContFlag( wk, capturedBpp, BPP_CONTFLG_SORAWOTOBU );

      if( BTL_POSPOKE_IsExist(&wk->pospokeWork, capturedPokeID) ){
        SCQUE_PUT_ACT_TameWazaHide( wk->que, capturedPokeID, FALSE );
        scPut_Message_Set( wk, capturedBpp, BTL_STRID_SET_FreeFall_End );
      }
    }

    scPut_SetBppCounter( wk, bpp, BPP_COUNTER_FREEFALL, 0 );
    if( !fReleaseProc )
    {
      if( !BPP_IsDead(bpp) ){
        SCQUE_PUT_ACT_TameWazaHide( wk->que, BPP_GetID(bpp), FALSE );
      }
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * ワザ出し失敗チェック
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 *
 * @retval  SV_WazaFailCause    失敗した場合失敗コード／成功の場合 SV_WAZAFAIL_NULL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_CheckWazaExecuteFail_1st( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u8 fWazaLock )
{
  SV_WazaFailCause  cause = SV_WAZAFAIL_NULL;
  PokeSick sick = POKESICK_NULL;
  BOOL fWazaMelt = FALSE;

  do {

    // 命令無視判定
    if( wk->currentSabotageType == SABOTAGE_DONT_ANY ){
      cause = SV_WAZAFAIL_SABOTAGE;
      break;
    }
    if( wk->currentSabotageType == SABOTAGE_GO_SLEEP ){
      cause = SV_WAZAFAIL_SABOTAGE_GO_SLEEP;
      break;
    }

     // PPがゼロ
    if( !fWazaLock )
    {
      u8 wazaIdx = BPP_WAZA_SearchIdx( attacker, waza );
      if( (wazaIdx != PTL_WAZA_MAX)
      &&  (BPP_WAZA_GetPP(attacker, wazaIdx) == 0)
      ){
        cause = SV_WAZAFAIL_PP_ZERO;
        break;
      }
    }

    // ねむり・こおり等の解除チェック
    fWazaMelt = scproc_PokeSickCure_WazaCheck( wk, attacker, waza );

    // ポケモン系 状態異常による失敗チェック
    sick = BPP_GetPokeSick( attacker );
    if( sick == POKESICK_NEMURI ){
      if( !scEvent_ExeFailThrew(wk, attacker, waza, SV_WAZAFAIL_NEMURI) ){
        cause = SV_WAZAFAIL_NEMURI;
        break;
      }
    }
    if( (sick == POKESICK_KOORI) && (!fWazaMelt) ){
      cause = SV_WAZAFAIL_KOORI;
      break;
    }

    // イベント失敗チェックレベル１
    cause = scEvent_CheckWazaExecute( wk, attacker, waza, BTL_EVENT_WAZA_EXECUTE_CHECK_1ST );
    if( cause != SV_WAZAFAIL_NULL ){  break; }


    // 集中切れ（きあいパンチ失敗）による失敗チェック
    if( BPP_TURNFLAG_Get(attacker, BPP_TURNFLG_KIAI_SHRINK) ){
      cause = SV_WAZAFAIL_KIAI_SHRINK;
      break;
    }
    // ひるみによる失敗チェック
    if( BPP_TURNFLAG_Get(attacker, BPP_TURNFLG_SHRINK) ){
      cause = SV_WAZAFAIL_SHRINK;
      break;
    }

    // かなしばり判定
    if( BPP_CheckSick(attacker, WAZASICK_KANASIBARI) )
    {
      if( waza != WAZANO_WARUAGAKI )
      {
        BPP_SICK_CONT  sickCont = BPP_GetSickCont( attacker, WAZASICK_KANASIBARI );
        if( BPP_SICKCONT_GetParam(sickCont) == waza ){
          cause = SV_WAZAFAIL_KANASIBARI;
          break;
        }
      }
    }

    // かいふくふうじチェック
    if( BPP_CheckSick(attacker, WAZASICK_KAIHUKUHUUJI)
    &&  (WAZADATA_GetFlag(waza, WAZAFLAG_KaifukuHuuji))
    ){
      cause = SV_WAZAFAIL_KAIHUKUHUUJI;
      break;
    }

    // じゅうりょくチェック
    if( BTL_FIELD_CheckEffect(BTL_FLDEFF_JURYOKU)
    &&  (WAZADATA_GetFlag(waza, WAZAFLAG_Flying))
    ){
      cause = SV_WAZAFAIL_JURYOKU;
      break;
    }

    // わるあがき以外をチェック
    if( waza != WAZANO_WARUAGAKI )
    {
      // アンコールによる失敗チェック
      if( (BPP_CheckSick(attacker, WAZASICK_ENCORE))
      &&  (BPP_GetPrevOrgWazaID(attacker) != waza)
      ){
        cause = SV_WAZAFAIL_WAZALOCK;
        break;
      }
      // ワザロック（こだわり）による失敗チェック
      if( (BPP_CheckSick(attacker, WAZASICK_WAZALOCK))
      &&  (BPP_GetPrevWazaID(attacker) != waza)
      ){
        cause = SV_WAZAFAIL_WAZALOCK;
        break;
      }
      // ちょうはつによる失敗チェック
      if( BPP_CheckSick(attacker, WAZASICK_TYOUHATSU)
      &&  !WAZADATA_IsDamage(waza)
      ){
        cause = SV_WAZAFAIL_TYOUHATSU;
        break;
      }
      // いちゃもんによる失敗チェック
      // 2010.06.06  GSまでの挙動としては、いちゃもんを先制で受けてもワザは出せるらしいのでチェックをはずす
      #if 0
      if( BPP_CheckSick(attacker, WAZASICK_ICHAMON)
      &&  (BPP_GetPrevOrgWazaID(attacker) == waza)
      ){
        cause = SV_WAZAFAIL_ICHAMON;
        break;
      }
      #endif
      // ふういんによる失敗チェック
      if( BTL_FIELD_CheckEffect(BTL_FLDEFF_FUIN) )
      {
        if( BTL_FIELD_CheckFuin(wk->pokeCon, attacker, waza) )
        {
          cause = SV_WAZAFAIL_FUUIN;
          break;
        }
      }
    }/* waza != WAZANO_WARUAGAKI */


    // こんらん自爆判定
    if( scproc_Fight_CheckConf(wk, attacker) ){
      cause = SV_WAZAFAIL_KONRAN;
      break;
    }

    // マヒ判定
    if( sick == POKESICK_MAHI ){
      if( BTL_CALC_IsOccurPer(BTL_MAHI_EXE_PER) ){
       cause = SV_WAZAFAIL_MAHI;
        break;
      }
    }
    // メロメロ判定
    if( scproc_Fight_CheckMeroMero(wk, attacker) ){
      cause = SV_WAZAFAIL_MEROMERO;
      break;
    }

  }while( 0 );

  if( cause != SV_WAZAFAIL_NULL ){
    BTL_N_Printf( DBGSTR_SVFL_WazaExeFail_1, BPP_GetID(attacker), cause);
    scproc_WazaExecuteFailed( wk, attacker, waza, cause );
    return TRUE;
  }
  else
  {
    // 眠ってるのに失敗せず->眠りエフェクトだけ表示する
    if( sick == POKESICK_NEMURI )
    {
      scPut_WazaExecuteFailMsg( wk, attacker, waza, SV_WAZAFAIL_NEMURI );
      scPut_EffectByPokePos( wk, attacker, BTLEFF_NEMURI );
    }
    // ワザでこおりが溶ける
    else if( fWazaMelt && (sick == POKESICK_KOORI ) ){
      scPut_CurePokeSick( wk, attacker, sick, FALSE );
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriMeltWaza, BPP_GetID(attacker), waza );
    }
  }
  return FALSE;
}
static BOOL scproc_Fight_CheckWazaExecuteFail_2nd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BOOL fWazaLock )
{
  if( waza != WAZANO_WARUAGAKI )
  {
    SV_WazaFailCause  cause;

    cause = scEvent_CheckWazaExecute( wk, attacker, waza, BTL_EVENT_WAZA_EXECUTE_CHECK_2ND );
    if( cause != SV_WAZAFAIL_NULL )
    {
      BTL_N_Printf( DBGSTR_SVFL_WazaExeFail_2, BPP_GetID(attacker), cause);
      scproc_WazaExecuteFailed( wk, attacker, waza, cause );
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event]  特定ワザ出し失敗要因を無視するか判定
 *
 * @param   wk
 * @param   bpp
 * @param   cause
 * @param   waza
 *
 * @retval  BOOL    無視する場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_ExeFailThrew( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause )
{
  BOOL fThrew = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_FAIL_CAUSE, cause );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_FAIL_THREW );
    fThrew = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  return fThrew;
}
//--------------------------------------------------------------------------
/**
 * 【Event】ワザ出し失敗判定
 *
 * @param   wk
 * @param   attacker    攻撃ポケモンパラメータ
 * @param   waza        出そうとしているワザ
 *
 * @retval  SV_WazaFailCause
 */
//--------------------------------------------------------------------------
static SV_WazaFailCause scEvent_CheckWazaExecute( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, BtlEventType eventID )
{
  SV_WazaFailCause cause ;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_NULL );
    BTL_EVENT_CallHandlers( wk, eventID );
    cause = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_CAUSE );
  BTL_EVENTVAR_Pop();

  return cause;
}
//----------------------------------------------------------------------------------
/**
 * こんらん自爆チェック＆処理
 *
 * @param   wk
 * @param   attacker
 *
 * @retval  BOOL    自爆した場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_CheckConf( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker )
{
  if( wk->currentSabotageType == SABOTAGE_CONFUSE ){
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_Sabotage1, BPP_GetID(attacker) );
    return TRUE;
  }

  if( BPP_CheckSick(attacker, WAZASICK_KONRAN) )
  {
    if( BPP_CheckKonranWakeUp(attacker) ){
      scPut_CureSick( wk, attacker, WAZASICK_KONRAN, NULL );
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KonranCure, BPP_GetID(attacker) );
      return FALSE;
    }
    scPut_ConfCheck( wk, attacker );
    return BTL_CALC_IsOccurPer( BTL_CONF_EXE_RATIO );
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * メロメロによるワザだし失敗判定
 *
 * @param   wk
 * @param   attacker
 *
 * @retval  BOOL    失敗した場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Fight_CheckMeroMero( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker )
{
  if( BPP_CheckSick(attacker, WAZASICK_MEROMERO) )
  {
    scPut_MeromeroAct( wk, attacker );
    if( BTL_CALC_IsOccurPer(BTL_MEROMERO_EXE_PER) )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * ねむり・こおり等、アクション選択後に解除チェックを行う処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static BOOL scproc_PokeSickCure_WazaCheck( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza )
{
  PokeSick sick = BPP_GetPokeSick( attacker );
  BOOL f_cured = FALSE;
  BOOL f_wazaMelt = FALSE;

  switch( sick ){
  case POKESICK_NEMURI:
    if( BPP_CheckNemuriWakeUp(attacker) ){
      f_cured = TRUE;
    }
    break;
  case POKESICK_KOORI:
    if( WAZADATA_GetFlag(waza, WAZAFLAG_KooriMelt) ){
      f_wazaMelt = TRUE;
    }
    else if( BTL_CALC_IsOccurPer(BTL_KORI_MELT_PER) )
    {
      f_cured = TRUE;
    }
    break;
  }

  if( f_cured ){
    scPut_CurePokeSick( wk, attacker, sick, TRUE );
  }

  return f_wazaMelt;
}
//----------------------------------------------------------------------------------
/**
 * [Proc] ワザ出し失敗時処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   fail_cause
 */
//----------------------------------------------------------------------------------
static void scproc_WazaExecuteFailed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, SV_WazaFailCause fail_cause )
{
  // こんらんなら自爆ダメージチェック
  if( fail_cause == SV_WAZAFAIL_KONRAN )
  {
    scproc_CalcConfDamage( wk, attacker );
    if( scproc_CheckDeadCmd(wk, attacker) ){
      if( scproc_CheckShowdown(wk) ){
        return;
      }
    }
  }
  else
  {
    // 失敗メッセージ
    scPut_WazaExecuteFailMsg( wk, attacker, waza, fail_cause );

    // 演出
    switch( fail_cause ){
    case SV_WAZAFAIL_NEMURI:     scPut_EffectByPokePos( wk, attacker, BTLEFF_NEMURI ); break;
    case SV_WAZAFAIL_MAHI:       scPut_EffectByPokePos( wk, attacker, BTLEFF_MAHI ); break;
    case SV_WAZAFAIL_KOORI:      scPut_EffectByPokePos( wk, attacker, BTLEFF_KOORI ); break;
//    case SV_WAZAFAIL_MEROMERO:   scPut_EffectByPokePos( wk, attacker, BTLEFF_MEROMERO ); break;

    case SV_WAZAFAIL_SABOTAGE_GO_SLEEP:
      {
        BPP_SICK_CONT cont = BTL_CALC_MakeDefaultPokeSickCont( POKESICK_NEMURI );

        SCQUE_PUT_ACT_TameWazaHide( wk->que, BPP_GetID(attacker), FALSE );
        scPut_AddSick( wk, attacker, POKESICK_NEMURI, cont );
        scPut_EffectByPokePos( wk, attacker, BTLEFF_NEMURI );
      }
      break;
    }
  }

  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    scEvent_CheckWazaExeFail( wk, attacker, waza, fail_cause );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }
}
//--------------------------------------------------------------------------
/**
 * ワザ出し失敗メッセージ表示
 */
//--------------------------------------------------------------------------
static void scPut_WazaExecuteFailMsg( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaID waza, SV_WazaFailCause cause )
{
  u8 pokeID = BPP_GetID( bpp );

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
  case SV_WAZAFAIL_KIAI_SHRINK:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KiaiShrink, pokeID );
    break;
  case SV_WAZAFAIL_MEROMERO:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_MeromeroExe, pokeID );
    break;
  case SV_WAZAFAIL_KANASIBARI:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KanasibariWarn, pokeID, waza );
    break;
  /*
  case SV_WAZAFAIL_NAMAKE:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Namake, pokeID );
    break;
  */
  case SV_WAZAFAIL_ENCORE:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Namake, pokeID );
    break;
  case SV_WAZAFAIL_TYOUHATSU:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_ChouhatuWarn, pokeID, waza );
    break;
  case SV_WAZAFAIL_ICHAMON:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_IchamonWarn, pokeID, waza );
    break;
  case SV_WAZAFAIL_FUUIN:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_FuuinWarn, pokeID, waza );
    break;
  case SV_WAZAFAIL_KAIHUKUHUUJI:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KaifukuFujiExe, pokeID, waza );
    break;
  case SV_WAZAFAIL_HPFULL:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_HPFull, pokeID );
    break;
  case SV_WAZAFAIL_FUMIN:
    SCQUE_PUT_TOKWIN_IN( wk->que, pokeID );
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Fumin, pokeID );
    SCQUE_PUT_TOKWIN_OUT( wk->que, pokeID );
    break;
  case SV_WAZAFAIL_JURYOKU:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_JyuryokuWazaFail, pokeID, waza );
    break;
  case SV_WAZAFAIL_SABOTAGE:
    if( BPP_CheckSick(bpp, WAZASICK_NEMURI) ){
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_Sabotage_Sleeping, pokeID  );
    }else{
      u8 rnd = BTL_CALC_GetRand( 4 );
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_Sabotage1 + rnd, pokeID  );
    }
    break;
  case SV_WAZAFAIL_SABOTAGE_GO_SLEEP:
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_Sabotage_GoSleep, pokeID  );
    break;

  case SV_WAZAFAIL_TOKUSEI:
  case  SV_WAZAFAIL_NO_REACTION:
    // とくせいの場合、各ハンドラに任せる
    break;

  default:
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail, pokeID );
    break;
  }
}
//----------------------------------------------------------------------------------
/**
 * [Put] ポケモン内部カウンタ値をセット
 *
 * @param   wk
 * @param   bpp
 * @param   counterID
 * @param   value
 */
//----------------------------------------------------------------------------------
static void scPut_SetBppCounter( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppCounter counterID, u8 value )
{
  SCQUE_PUT_OP_SetPokeCounter( wk->que, BPP_GetID(bpp), counterID, value );
  BPP_COUNTER_Set( bpp, counterID, value );
}

//--------------------------------------------------------------------------
/**
 * [Proc] 使ったワザのPP減少
 *
 * @param   wk
 * @param   attacker  攻撃ポケモンパラメータ
 * @param   wazaIdx   使ったワザインデックス
 * @param   rec       ダメージを受けたポケモンパラメータ群
 *
 */
//--------------------------------------------------------------------------
static void scproc_decrementPPUsedWaza( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  WazaID waza, u8 wazaIdx, BTL_POKESET* rec )
{
  u8 vol = scEvent_DecrementPPVolume( wk, attacker, wazaIdx, waza, rec );
  scproc_decrementPP( wk, attacker, wazaIdx, vol );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ使用後のデクリメントするPP値を取得
 *
 * @param   wk
 * @param   attacker  ワザ使用ポケモン
 * @param   rec       ワザによってダメージを受けたポケモンパラメータ群
 *
 * @retval  u32       デクリメントPP値
 */
//----------------------------------------------------------------------------------
static u32 scEvent_DecrementPPVolume( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker,
  u8 wazaIdx, WazaID waza, BTL_POKESET* rec )
{
  u32 volume = 1;

  BTL_EVENTVAR_Push();
    {
      u32 i = 0;
      BTL_POKEPARAM* bpp;
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, BTL_POKESET_GetCount(rec) );

      while( (bpp=BTL_POKESET_Get(rec, i)) != NULL )
      {
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(bpp) );
        ++i;
      }
    }
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_IDX, wazaIdx );
//    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE ); // 真ワザワーク対象フラグとする
    BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECREMENT_PP );
    volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );

  BTL_EVENTVAR_Pop();

  return volume;
}
//----------------------------------------------------------------------------------
/**
 * [Proc] ワザのPP減少
 *
 * @param   wk
 * @param   bpp       対象ポケモンパラメータ
 * @param   wazaIdx   減少させるワザインデックス
 * @param   volume    減少量
 *
 * @retval  BOOL      減少させたらTRUE／死亡時など減少させられなかったらFALSE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_decrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 wazaIdx, u8 volume )
{
  u8 restPP;

  restPP = BPP_WAZA_GetPP( bpp, wazaIdx );

  if( volume >= restPP ){
    volume = restPP;
  }

  if( volume )
  {
    scPut_DecrementPP( wk, bpp, wazaIdx, volume );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Put] 使ったワザのPPデクリメント
 *
 * @param   wk
 * @param   attacker
 * @param   wazaIdx
 * @param   vol
 */
//----------------------------------------------------------------------------------
static void scPut_DecrementPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, u8 wazaIdx, u8 vol )
{
  if( !BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_PP_CONST) )
  {
    u8 pokeID = BPP_GetID( attacker );

    BPP_WAZA_DecrementPP( attacker, wazaIdx, vol );
    SCQUE_PUT_OP_PPMinus( wk->que, pokeID, wazaIdx, vol );

    BPP_WAZA_SetUsedFlag( attacker, wazaIdx );
    SCQUE_PUT_OP_WazaUsed( wk->que, pokeID, wazaIdx );
  }
}

//----------------------------------------------------------------------
// サーバーフロー：「たたかう」> ダメージワザ
//----------------------------------------------------------------------
static void scproc_Fight_Damage_Root( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
   BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec, BOOL fDelayAttack )
{
  u32  dmg_sum = 0;

  FlowFlg_Clear( wk, FLOWFLG_SET_WAZAEFFECT );
  scproc_Fight_DamageProcStart( wk, attacker, wazaParam );

  // ダメージ受けポケモンワークをクリアしておく
  BTL_POKESET_Clear( wk->psetDamaged );

  scEvent_HitCheckParam( wk, attacker, wazaParam->wazaID, wk->hitCheckParam );

  // ダメージワザ回復化 コイツは現在「プレゼント」でしか使われていない
  scproc_Fight_Damage_ToRecover( wk, attacker, wazaParam, targets );

  if( BTL_POKESET_GetCount(targets) )
  {
    if( HITCHECK_IsPluralHitWaza(wk->hitCheckParam)
    &&  (BTL_POKESET_GetCountMax(targets) == 1)
    ){
      dmg_sum = scproc_Fight_Damage_PluralCount( wk, wazaParam, attacker, targets, affRec );
    }
    else{
      dmg_sum = scproc_Fight_Damage_SingleCount( wk, wazaParam, attacker, targets, affRec, fDelayAttack );
    }

    if( dmg_sum ){
      scproc_Fight_Damage_Kickback( wk, attacker, wazaParam->wazaID, dmg_sum );
    }
  }

  scproc_Fight_DamageProcEnd( wk, wazaParam, attacker, wk->psetDamaged, dmg_sum, fDelayAttack );
}

//----------------------------------------------------------------------------------
/**
 * ダメージ計算結果をワークに取得
 *
 * @param   wk
 * @param   attacker
 * @param   target
 * @param   wazaParam
 * @param   affRec
 * @param   dmg_ratio
 * @param   dmgRec
 */
//----------------------------------------------------------------------------------
static void BTL_CALCDAMAGE_Set( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKESET* target,
  const SVFL_WAZAPARAM* wazaParam, const BTL_DMGAFF_REC* affRec, fx32 dmg_ratio, BTL_CALC_DAMAGE_REC* dmgRec )
{
  BTL_POKEPARAM* bpp;
  u32 c;
  u16 damage;

  dmgRec->real_hit_count = 0;
  dmgRec->migawari_hit_count = 0;
  c = 0;

  BTL_POKESET_SeekStart( target );
  while( (bpp = BTL_POKESET_SeekNext(target) ) != NULL )
  {
    dmgRec->record[c].pokeID = BPP_GetID( bpp );
    dmgRec->record[c].fCritical = scEvent_CheckCritical( wk, attacker, bpp, wazaParam->wazaID );
    dmgRec->record[c].affine = BTL_DMGAFF_REC_Get( affRec, dmgRec->record[c].pokeID );
    dmgRec->record[c].fFixDamage = scEvent_CalcDamage( wk, attacker, bpp, wazaParam, dmgRec->record[c].affine,
        dmg_ratio, dmgRec->record[c].fCritical, FALSE, &damage );
    dmgRec->record[c].damage = damage;

    // 固定ダメージなら相性等倍、クリティカル無しとして処理
    if( dmgRec->record[c].fFixDamage )
    {
      dmgRec->record[c].fCritical = FALSE;
      dmgRec->record[c].affine = BTL_TYPEAFF_100;
    }

    dmgRec->record[c].fMigawari = BPP_MIGAWARI_IsExist( bpp );
    if( dmgRec->record[c].fMigawari == FALSE )
    {
      u16 damage = MarumeDamage( bpp, dmgRec->record[c].damage );
      dmgRec->record[c].koraeru = scEvent_CheckKoraeru( wk, attacker, bpp, TRUE, &damage );
      dmgRec->record[c].damage = damage;
      dmgRec->real_hit_count++;
    }
    else
    {
      dmgRec->record[c].damage = damage;
      dmgRec->record[c].koraeru = BPP_KORAE_NONE;
      dmgRec->migawari_hit_count++;
    }
//    TAYA_Printf("ダメージ記録：ポケ(%d)に対するダメージ=%d\n", BPP_GetID(bpp), dmgRec->record[c].damage );

    AffCounter_CountUp( &wk->affCounter, wk, attacker, bpp, dmgRec->record[c].affine );

    ++c;
  }
  dmgRec->total_hit_count = c;

  #if 0
  BTL_POKEPARAM* bpp;

  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets) ) != NULL )
  {
  }
    bpp[i] = BTL_POKESET_Get( targets, i );
    critical_flg[i] = scEvent_CheckCritical( wk, attacker, bpp[i], wazaParam->wazaID );
    fFixDamage = scEvent_CalcDamage( wk, attacker, bpp[i], wazaParam, affAry[i], dmg_ratio, critical_flg[i], TRUE, &dmg[i] );
    if( fFixDamage ){
      affAry[i] = BTL_TYPEAFF_100;
      critical_flg[i] = FALSE;
    }
    dmg_tmp = dmg[i];
    if( !BPP_MIGAWARI_IsExist(bpp[i]) ){
      dmg[i] = MarumeDamage( bpp[i], dmg[i] );
      koraeru_cause[i] = scEvent_CheckKoraeru( wk, attacker, bpp[i], TRUE, &dmg[i] );
    }
    dmg_sum += dmg[i];
    if( dmg_tmp ){
      AffCounter_CountUp( &wk->affCounter, wk, attacker, bpp[i], affAry[i] );
    }
#endif
}
//----------------------------------------------------------------------------------
/**
 * ダメージ計算結果に格納されたターゲットポケモン数を取得（実体・みがわりの合計）
 *
 * @param   dmgRec
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static u32 BTL_CALCDAMAGE_GetCount( const BTL_CALC_DAMAGE_REC* dmgRec )
{
  return dmgRec->total_hit_count;
}
//----------------------------------------------------------------------------------
/**
 * みがわりヒットしたポケモンの情報のみを配列に出力
 *
 * @retval  u32   みがわりヒットしたポケモン数
 */
//----------------------------------------------------------------------------------
static u32 BTL_CALCDAMAGE_GetMigawariHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry, u16* dmgAry, BtlTypeAff* affAry, u8* criticalFlagAry )
{
  u32 i, c;
  for(i=0, c=0; i<rec->total_hit_count; ++i)
  {
    if( rec->record[i].fMigawari )
    {
      bppAry[c] = BTL_POKECON_GetPokeParam( wk->pokeCon, rec->record[i].pokeID );
      dmgAry[c] = rec->record[i].damage;
      affAry[c] = rec->record[i].affine;
      criticalFlagAry[c] = rec->record[i].fCritical;
      ++c;
    }
  }
  return c;
}
//----------------------------------------------------------------------------------
/**
 * みがわり・実体問わずヒットしたポケモンの情報を配列に出力
 *
 * @retval  u32   ヒットしたポケモン数
 */
//----------------------------------------------------------------------------------
static u32 BTL_CALCDAMAGE_GetAllHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry )
{
  u32 i;
  for(i=0; i<rec->total_hit_count; ++i)
  {
    bppAry[i] = BTL_POKECON_GetPokeParam( wk->pokeCon, rec->record[i].pokeID );
  }
  return rec->total_hit_count;
}
//----------------------------------------------------------------------------------
/**
 * 実体ヒットしたポケモンの情報のみを配列に出力
 *
 * @retval  u32   ヒットしたポケモン数
 */
//----------------------------------------------------------------------------------
static u32 BTL_CALCDAMAGE_GetRealHitPoke( BTL_SVFLOW_WORK* wk, const BTL_CALC_DAMAGE_REC* rec,
  BTL_POKEPARAM** bppAry, u16* dmgAry, BtlTypeAff* affAry, u8* criticalFlagAry, u8* koraeCauseAry )
{
  u32 i, c;
  for(i=0, c=0; i<rec->total_hit_count; ++i)
  {
    if( rec->record[i].fMigawari == FALSE )
    {
      bppAry[c] = BTL_POKECON_GetPokeParam( wk->pokeCon, rec->record[i].pokeID );
      dmgAry[c] = rec->record[i].damage;
      affAry[c] = rec->record[i].affine;
      criticalFlagAry[c] = rec->record[i].fCritical;
      koraeCauseAry[c] = rec->record[i].koraeru;;
      ++c;
    }
  }
  return c;
}
//----------------------------------------------------------------------------------
/**
 * ダメージ合計を取得
 *
 * @param   rec
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static u32 BTL_CALCDAMAGE_GetDamageSum( const BTL_CALC_DAMAGE_REC* rec )
{
  u32 i, sum = 0;
  for(i=0; i<rec->total_hit_count; ++i)
  {
    sum += rec->record[i].damage;
  }
  return sum;
}



//------------------------------------------------------------------------------
/**
* １回ヒットワザ（対象は１体以上）
*/
//------------------------------------------------------------------------------
static u32 scproc_Fight_Damage_SingleCount( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec, BOOL fDelayAttack )
{
  FLAG_SET flagSet;
  fx32 dmg_ratio = (BTL_POKESET_GetCountMax(targets) == 1)? BTL_CALC_DMG_TARGET_RATIO_NONE : BTL_CALC_DMG_TARGET_RATIO_PLURAL;
  u32 dmg_sum = 0;


  // 複数対象のワザか判定
  flagSet.hitPluralPoke = (BTL_POKESET_GetCount(targets) > 1);
  flagSet.delayAttack = fDelayAttack;

  // 敵・味方を別々のSetにコピー
  BTL_POKESET_CopyFriends( targets, attacker, wk->psetFriend );
  BTL_POKESET_CopyEnemys( targets, attacker, wk->psetEnemy );

  // ダメージ計算結果をワークに保存
  BTL_CALCDAMAGE_Set( wk, attacker, wk->psetFriend, wazaParam, affRec, dmg_ratio, wk->calcDmgFriend );
  BTL_CALCDAMAGE_Set( wk, attacker, wk->psetEnemy, wazaParam, affRec, dmg_ratio, wk->calcDmgEnemy );

//  scproc_SingleCount_DamageDetermine( wk, attacker,

  if( BTL_POKESET_GetCount( wk->psetFriend ) )
  {
    dmg_sum += scproc_Fight_Damage_side( wk, wazaParam, attacker, wk->psetFriend, wk->calcDmgFriend,
                  wk->hitCheckParam, dmg_ratio, flagSet );
    if( dmg_sum ){
      BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_TEMOTI_MAKIZOE );
    }
  }
  if( BTL_POKESET_GetCount( wk->psetEnemy ) )
  {
    dmg_sum += scproc_Fight_Damage_side( wk, wazaParam, attacker, wk->psetEnemy, wk->calcDmgEnemy,
      wk->hitCheckParam, dmg_ratio, flagSet );
  }

  wazaEffCtrl_SetEnable( wk->wazaEffCtrl );

  return dmg_sum;
}
//------------------------------------------------------------------------------
/**
* 複数回ヒットワザ（対象は１体のみ）
*/
//------------------------------------------------------------------------------
static u32 scproc_Fight_Damage_PluralCount( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_DMGAFF_REC* affRec )
{
  BTL_POKEPARAM* bpp;
  u32 i, hitCount;
  u32 dmg_sum = 0;
  WazaSick  pokeSick;
  BtlPokePos  targetPos = BTL_POS_NULL;
  FLAG_SET flagSet;

  bpp = BTL_POKESET_Get( targets, 0 );
  wk->thruDeadMsgPokeID = BPP_GetID( bpp );

  // 複数回ヒットはエフェクトにカメラ演出を入れず、ズームインコマンドを生成する
  if( BTL_MAIN_IsWazaEffectEnable(wk->mainModule) )
  {
    targetPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(bpp) );
    if( targetPos != BTL_POS_NULL ){
      SCQUE_PUT_ACT_EffectByPos( wk->que, targetPos, BTLEFF_ZOOM_IN );
    }
  }

  flagSet.raw = 0;

  for(i=0, hitCount=0; i<wk->hitCheckParam->countMax; ++i)
  {
    pokeSick = BPP_GetPokeSick( attacker );

    // ワザエフェクトコマンド生成
    wazaEffCtrl_SetEnableDummy( wk->wazaEffCtrl );
    SCQUE_PUT_ACT_WazaEffect( wk->que,
          wk->wazaEffCtrl->attackerPos, wk->wazaEffCtrl->targetPos, wazaParam->wazaID, 0 );


    BTL_CALCDAMAGE_Set( wk, attacker, targets, wazaParam, affRec, BTL_CALC_DMG_TARGET_RATIO_NONE, wk->calcDmgEnemy );

    dmg_sum += scproc_Fight_Damage_side( wk, wazaParam, attacker, targets, wk->calcDmgEnemy,
                    wk->hitCheckParam, BTL_CALC_DMG_TARGET_RATIO_NONE, flagSet );
    ++hitCount;

    if( BPP_IsDead(bpp) ){ break; }
    if( BPP_IsDead(attacker) ){ break; }

    // 攻撃したことにより眠ってしまったらブレイク
    if( (BPP_GetPokeSick(attacker) == POKESICK_NEMURI)
    &&  (pokeSick != POKESICK_NEMURI)
    ){
      break;
    }
    // 毎回ヒットチェックを行うワザは外れたらブレイク
    if( wk->hitCheckParam->fCheckEveryTime ){
      if( !scEvent_CheckHit(wk, attacker, bpp, wazaParam) ){ break; }
    }
  }

  if( hitCount>0 ){
    scPut_DamageAff( wk, wk->hitCheckParam->pluralHitAffinity );
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_Hit_PluralTimes, hitCount );
  }

  if( BPP_IsDead(bpp) ){
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Dead, BPP_GetID(bpp) );
  }

  // カメラズームインをリセット
  if( targetPos != BTL_POS_NULL ){
    SCQUE_PUT_ACT_EffectSimple( wk->que, BTLEFF_CAMERA_INIT );
  }

  wk->thruDeadMsgPokeID = BTL_POKEID_NULL;

  return dmg_sum;
}

/**
 *  最初のヒットか判定
 */
static BOOL HITCHECK_IsFirstTime( const HITCHECK_PARAM* param )
{
  return param->count == 0;
}
/**
 *  複数回ヒットするワザか判定
 */
static BOOL HITCHECK_IsPluralHitWaza( const HITCHECK_PARAM* param )
{
  return param->fPluralHitWaza;
}
/**
 *
 */
static BOOL HITCHECK_CheckWazaEffectPuttable( HITCHECK_PARAM* param )
{
  if( param->fPutEffectCmd == FALSE ){
    param->fPutEffectCmd = TRUE;
    return TRUE;
  }
  return FALSE;
}
/**
 *
 */
static void HITCHECK_SetPluralHitAffinity( HITCHECK_PARAM* param, BtlTypeAff affinity )
{
  if( param->pluralHitAffinity == BTL_TYPEAFF_100 ){
    param->pluralHitAffinity = affinity;
  }
}

//----------------------------------------------------------------------------------
/**
 * １陣営に対するダメージ処理
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   targets
 * @param   dmg_ratio
 *
 * @retval  u32  発生したダメージ量
 */
//----------------------------------------------------------------------------------
static u32 scproc_Fight_Damage_side( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker,
  BTL_POKESET* targets, const BTL_CALC_DAMAGE_REC* dmgRec, HITCHECK_PARAM* hitCheckParam, fx32 dmg_ratio, FLAG_SET flagSet )
{
  BtlTypeAff affAry[ BTL_POSIDX_MAX ];
  BTL_POKEPARAM* bpp;
  u8 damaged_poke_cnt;

  damaged_poke_cnt = BTL_CALCDAMAGE_GetCount( dmgRec );

  if( damaged_poke_cnt )
  {
    u32 dmg_sum = scproc_Fight_damage_side_core( wk, attacker, targets, dmgRec, wazaParam,
                  hitCheckParam, dmg_ratio, flagSet );
    scproc_WazaDamageSideAfter( wk, attacker, wazaParam, dmg_sum );
    return dmg_sum;
  }

  return 0;
}
/**
 *  １陣営ダメージ処理コア
 */
static u32 scproc_Fight_damage_side_core( BTL_SVFLOW_WORK* wk,
    BTL_POKEPARAM* attacker, BTL_POKESET* targets, const BTL_CALC_DAMAGE_REC* dmgRec,
    const SVFL_WAZAPARAM* wazaParam, HITCHECK_PARAM* hitCheckParam, fx32 dmg_ratio, FLAG_SET flagSet )
{
  static BTL_POKEPARAM *bpp[ BTL_POSIDX_MAX ];
  static u16 dmg[ BTL_POSIDX_MAX ];
  static BtlTypeAff affAry[ BTL_POSIDX_MAX ];
  static u8  critical_flg[ BTL_POSIDX_MAX ];
  static u8  koraeru_cause[ BTL_POSIDX_MAX ];

  u32 dmg_sum, dmg_tmp;
  u8  poke_cnt, fFixDamage;
  int i;
  BtlPokePos atkPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(attacker) );

  dmg_sum = BTL_CALCDAMAGE_GetDamageSum( dmgRec );

  // 身代わりダメージ処理
  poke_cnt = BTL_CALCDAMAGE_GetMigawariHitPoke( wk, dmgRec, bpp, dmg, affAry, critical_flg );
  for(i=0; i<poke_cnt; ++i)
  {
    if( BPP_MIGAWARI_IsExist(bpp[i]) )
    {
      u16 add_damage = scproc_Migawari_Damage( wk, attacker, bpp[i], dmg[i], affAry[i], critical_flg[i], wazaParam );
      BTL_POKESET_AddWithDamage( wk->psetDamaged, bpp[i], add_damage, TRUE );  // ダメージ0として記録
      dmg_sum -= (dmg[i] - add_damage);
    }
  }

  // ダメージ確定イベントコール（複数回ヒットワザなら初回のみ）
  if( HITCHECK_IsFirstTime(hitCheckParam) )
  {
    poke_cnt = BTL_CALCDAMAGE_GetAllHitPoke( wk, dmgRec, bpp );
    for(i=0; i<poke_cnt; ++i){
      scproc_Fight_Damage_Determine( wk, attacker, bpp[i], wazaParam );
    }
  }
  hitCheckParam->count++;

  // 実体ヒットした分をコピー（みがわりだけだったらここでリターン）
  poke_cnt = BTL_CALCDAMAGE_GetRealHitPoke( wk, dmgRec, bpp, dmg, affAry, critical_flg, koraeru_cause );
  if( poke_cnt == 0 ){
    return dmg_sum;
  }

  // ダメージコマンド出力
  scproc_PrevWazaDamage( wk, wazaParam, attacker, poke_cnt, bpp );
  scPut_WazaDamagePlural( wk, wazaParam, poke_cnt, affAry, bpp, dmg, critical_flg, flagSet.hitPluralPoke );

  if( !HITCHECK_IsPluralHitWaza(hitCheckParam) ){
    // １回ヒットなら相性メッセージ即表示
    scPut_WazaAffinityMsg( wk, poke_cnt, affAry, bpp, flagSet.hitPluralPoke );
  }else{
    // 複数回ヒットワザの場合は相性値を記録するだけ
    HITCHECK_SetPluralHitAffinity( hitCheckParam, affAry[0] );
  }
  scPut_CriticalMsg( wk, poke_cnt, bpp, critical_flg, flagSet.hitPluralPoke );

  // ダメージ記録
  for(i=0; i<poke_cnt; ++i)
  {
    BTL_POKESET_AddWithDamage( wk->psetDamaged, bpp[i], dmg[i], FALSE );
    wazaDmgRec_Add( wk, atkPos, attacker, bpp[i], wazaParam, dmg[i] );
    BPP_TURNFLAG_Set( bpp[i], BPP_TURNFLG_DAMAGED );
  }

  // こらえた反応
  for(i=0; i<poke_cnt; ++i)
  {
    if( koraeru_cause[i] != BPP_KORAE_NONE ){
      scproc_Koraeru( wk, bpp[i], koraeru_cause[i] );
    }
  }

  // 追加効果、リアクション処理
  for(i=0; i<poke_cnt; ++i)
  {
    scproc_CheckShrink( wk, wazaParam, attacker, bpp[i] );
    scproc_Damage_Drain( wk, wazaParam, attacker, bpp[i], dmg[i] );
    scproc_WazaAdditionalEffect( wk, wazaParam, attacker, bpp[i], dmg[i], FALSE );
    scproc_WazaDamageReaction( wk, attacker, bpp[i], wazaParam, affAry[i], dmg[i], critical_flg[i], FALSE );
  }

  // ひんしチェック
  for(i=0; i<poke_cnt; ++i){
    scproc_CheckDeadCmd( wk, bpp[i] );
  }
  scproc_CheckDeadCmd( wk, attacker );

  return dmg_sum;
}
//----------------------------------------------------------------------------------
/**
 * ダメージ無効化（＆回復化）してしまうポケモンの処理。処理後、該当ポケモンをtargetsから除外
 *
 * @param   wk
 * @param   attacker
 * @param   wazaParam
 * @param   targets
 */
//----------------------------------------------------------------------------------
static void scproc_Fight_Damage_ToRecover( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, BTL_POKESET* targets )
{
  BTL_POKEPARAM* bpp;

  // 回復チェック -> 特定タイプのダメージを無効化、回復してしまうポケを targets から除外
  BTL_POKESET_SeekStart( targets );
  while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
  {
    if( scEvent_CheckDmgToRecover(wk, attacker, bpp, wazaParam) )
    {
      u32 hem_state;

      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );

      hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_DmgToRecover( wk, attacker, bpp );
//      scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );

      BTL_Hem_PopState( &wk->HEManager, hem_state );
      BTL_POKESET_Remove( targets, bpp );
    }
  }
}

/**
 *  効果は○○だ　メッセージ出力
 */
static void scPut_DamageAff( BTL_SVFLOW_WORK* wk, BtlTypeAff affinity )
{
  if( affinity < BTL_TYPEAFF_100 ){
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_AffBad );
  }
  else if(affinity > BTL_TYPEAFF_100){
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_AffGood );
  }
}
/**
 * 最高で残りＨＰの範囲に収まるようにダメージ最終補正
 */
static u32 MarumeDamage( const BTL_POKEPARAM* bpp, u32 damage )
{
  u32 hp = BPP_GetValue( bpp, BPP_HP );
  if( damage > hp ){
    damage = hp;
  }
  return damage;
}

//----------------------------------------------------------------------------------
/**
 * ワザによるダメージを与えることが確定した（みがわりも含む）
 *
 * @param   wk
 * @param   attacker
 * @param   wazaParam
 * @param   targets
 */
//----------------------------------------------------------------------------------
static void scproc_Fight_Damage_Determine( BTL_SVFLOW_WORK* wk,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  scEvent_WazaDamageDetermine( wk, attacker, defender, wazaParam );

  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによるダメージを与えることが確定した
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaDamageDetermine( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_DETERMINE );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ・混乱自爆によるダメージを「こらえる」かどうかチェック
 * ※「こらえる」= HPが1残る
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   fWazaDamage   ワザによるダメージの場合TRUE／混乱ダメージならFALSEを指定
 * @param   damage
 *
 * @retval  BppKoraeruCause
 */
//----------------------------------------------------------------------------------
static BppKoraeruCause scEvent_CheckKoraeru( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BOOL fWazaDamage, u16* damage )
{
  if( BPP_GetValue(defender, BPP_HP) > (*damage) )
  {
    return BPP_KORAE_NONE;
  }
  else
  {
    BppKoraeruCause  cause = BPP_KORAE_NONE;

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_GEN_FLAG,  fWazaDamage );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_KORAERU_CAUSE, cause );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_KORAERU_CHECK );
      cause = BTL_EVENTVAR_GetValue( BTL_EVAR_KORAERU_CAUSE );
    BTL_EVENTVAR_Pop();

    if( cause != BPP_KORAE_NONE ){
      *damage = BPP_GetValue(defender, BPP_HP) - 1;
    }

    return cause;
  }
}
//----------------------------------------------------------------------------------
/**
 * 「こらえる」原因表示＆アイテム消費等の処理
 *
 * @param   wk
 * @param   bpp
 * @param   cause
 */
//----------------------------------------------------------------------------------
static void scproc_Koraeru( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppKoraeruCause cause )
{
  u8 pokeID = BPP_GetID( bpp );

  switch( cause ){
  case BPP_KORAE_WAZA_DEFENDER:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Koraeru_Exe, pokeID );
    break;

  case BPP_KORAE_ITEM:
  default:
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_KoraeruExe( wk, bpp, cause );
//      scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
    break;
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] こらえる動作発動
 *
 * @param   wk
 * @param   bpp
 * @param   cause
 */
//----------------------------------------------------------------------------------
static void scEvent_KoraeruExe( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppKoraeruCause cause )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_KORAERU_EXE );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * ワザダメージ演出直後（みがわりは除外されている）
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   poke_cnt
 * @param   bppAry
 */
//----------------------------------------------------------------------------------
static void scproc_PrevWazaDamage( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, u32 poke_cnt, BTL_POKEPARAM** bppAry )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  scEvent_PrevWazaDamage( wk, wazaParam, attacker, poke_cnt, bppAry );
//  scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );

  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザダメージ演出直後（みがわりは除外されている）
 *
 * @param   wk
 * @param   wazaParam
 * @param   attacker
 * @param   poke_cnt
 * @param   bppAry
 */
//----------------------------------------------------------------------------------
static void scEvent_PrevWazaDamage( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, u32 poke_cnt, BTL_POKEPARAM** bppAry )
{
  u32 i;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, poke_cnt );
    for(i=0; i<poke_cnt; ++i)
    {
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(bppAry[i]) );
    }
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_PREV_WAZA_DMG );
  BTL_EVENTVAR_Pop();
}


//----------------------------------------------------------------------------------
/**
 * ワザダメージレコードを追加
 *
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 * @param   damage
 */
//----------------------------------------------------------------------------------
static void wazaDmgRec_Add( BTL_SVFLOW_WORK* wk, BtlPokePos atkPos, const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, u16 damage )
{
  BPP_WAZADMG_REC rec;
  u8 atkPokeID = BPP_GetID( attacker );
  u8 defPokeID = BPP_GetID( defender );

  BPP_WAZADMG_REC_Setup( &rec, atkPokeID, atkPos, wazaParam->wazaID, wazaParam->wazaType, damage );
  BPP_WAZADMGREC_Add( defender, &rec );

  SCQUE_OP_AddWazaDmgRec( wk->que, defPokeID, atkPokeID, atkPos, wazaParam->wazaType, wazaParam->wazaID, damage );
}

//--------------------------------------------------------------------------
/**
 * ワザダメージ後の追加効果処理
 *
 * @param   wk
 * @param   waza        使用ワザ
 * @param   attacker    攻撃ポケモンパラメータ
 * @param   defender    防御ポケモンパラメータ
 * @param   damage      与えたダメージ
 *
 */
//--------------------------------------------------------------------------
static void scproc_WazaAdditionalEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage, BOOL fMigawriHit )
{
  WazaCategory category = WAZADATA_GetCategory( wazaParam->wazaID );

  switch( category ){
  case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
    scproc_Fight_Damage_AddEffect( wk, wazaParam, attacker, attacker );
    break;
  case WAZADATA_CATEGORY_DAMAGE_EFFECT:
    if( !fMigawriHit ){
      scproc_Fight_Damage_AddEffect( wk, wazaParam, attacker, defender );
    }
    break;
  case WAZADATA_CATEGORY_DAMAGE_SICK:
    if( !fMigawriHit ){
      scproc_Fight_Damage_AddSick( wk, wazaParam, attacker, defender );
    }
    break;
  }
}
//----------------------------------------------------------------------------------
/**
 * ワザダメージ１回１陣営毎の終了処理
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 * @param   affinity
 * @param   damage
 * @param   critical_flag
 */
//----------------------------------------------------------------------------------
static void scproc_WazaDamageSideAfter( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, u32 damage )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  scEvent_WazaDamageSideAfter( wk, attacker, wazaParam, damage );
//  scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザダメージを与え終わった後の追加処理
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza
 * @param   aff
 * @param   damage
 *
 */
//--------------------------------------------------------------------------
static void scEvent_WazaDamageSideAfter( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u32 damage )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE, damage );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZADMG_SIDE_AFTER );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * アイテム反応チェック
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scproc_CheckItemReaction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BtlItemReaction reactionType )
{
  if( BPP_GetItem(bpp) != ITEM_DUMMY_DATA )
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

    BTL_N_Printf( DBGSTR_SVFL_CheckItemReaction, BPP_GetID(bpp));

    scEvent_CheckItemReaction( wk, bpp, reactionType );
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event]アイテム反応チェック
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scEvent_CheckItemReaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlItemReaction reactionType )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ITEM_REACTION, reactionType );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_ITEM_REACTION );
  BTL_EVENTVAR_Pop();
}
//------------------------------------------------------------------
// サーバーフロー：ダメージワザシーケンス開始
//------------------------------------------------------------------
static void scproc_Fight_DamageProcStart( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_DamageProcStart( wk, attacker, wazaParam );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ダメージワザ処理開始
 *
 * @param   wk
 * @param   attacker
 * @param   targets
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_DamageProcStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam )
{
  BTL_EVENTVAR_Push();

    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DAMAGEPROC_START );

  BTL_EVENTVAR_Pop();
}

//------------------------------------------------------------------
// サーバーフロー：ダメージワザシーケンス終了
//------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 *
 *
 * @param   wk
 * @param   wazaParam   ワザパラメータ
 * @param   attacker    攻撃ポケ
 * @param   targets     ダメージを受けたポケモン群（みがわり除く）
 * @param   dmg_sum     みがわり分も含めたダメージトータル
 */
//----------------------------------------------------------------------------------
static void scproc_Fight_DamageProcEnd( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets, u32 dmgTotal, BOOL fDelayAttack )
{
  scproc_Fight_Damage_KoriCure( wk, wazaParam, attacker, targets );

  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    scEvent_DamageProcEnd( wk, attacker, targets, wazaParam, fDelayAttack );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }
}
//------------------------------------------------------------------
// サーバーフロー：ダメージ受け後の処理 > ひるみチェック
//------------------------------------------------------------------
static void scproc_CheckShrink( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender )
{
  u32 waza_per = scEvent_GetWazaShrinkPer( wk, wazaParam->wazaID, attacker );

  {
    scproc_AddShrinkCore( wk, defender, waza_per );
  }
}
//------------------------------------------------------------------
// サーバーフロー：ダメージ受け後の処理 > ほのおワザで「こおり」が治る処理
// 追加効果処理の後に置くこと。でないと「こおりがとけた」→「やけどになった」が起きてしまう。
//------------------------------------------------------------------
static void scproc_Fight_Damage_KoriCure( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  if( wazaParam->wazaType == POKETYPE_HONOO )
  {
    BTL_POKEPARAM* bpp;

    BTL_POKESET_SeekStart( targets );
    while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
    {
      if( (BPP_GetPokeSick(bpp) == POKESICK_KOORI)
      &&  (!BPP_MIGAWARI_IsExist(bpp))
      ){
        scPut_CurePokeSick( wk, bpp, POKESICK_KOORI, TRUE );
      }
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * 対象ポケモンにひるみ効果を与える
 *
 * @param   wk
 * @param   target
 * @param   per     確率（パーセンテージ）
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_AddShrinkCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, u32 per )
{
  BOOL fShrink;


  if( BPP_TURNFLAG_Get(target, BPP_TURNFLG_KIAI_READY) )
  {
    BPP_TURNFLAG_Set( target, BPP_TURNFLG_KIAI_SHRINK );
    return TRUE;
  }
  else
  {
    fShrink = scEvent_CheckShrink( wk, target, per );
  }

  if( fShrink )
  {
    BPP_TURNFLAG_Set( target, BPP_TURNFLG_SHRINK );
    return TRUE;
  }
  else if( per >= 100 )
  {
    // 確率100％なのに失敗したら原因表示へ
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    scEvent_FailShrink( wk, target );
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }
  return FALSE;
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザダメージに応じたHP吸い取り効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_Drain( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  u32 total_damage = 0;
  u32 hem_state;

  if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_DRAIN )
  {
    BTL_POKEPARAM* bpp;
    u32 damage, recoverHP;
    u32 i;

    hem_state = BTL_Hem_PushState( &wk->HEManager );
    BTL_POKESET_SeekStart( targets );
    while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
    {
      damage = BTL_POKESET_GetDamage( targets, bpp );
      total_damage += damage;
      recoverHP = (WAZADATA_GetParam(waza, WAZAPARAM_DAMAGE_RECOVER_RATIO) * damage) / 100;

      if( recoverHP > 0 )
      {
        if( scproc_DrainCore(wk, attacker, bpp, recoverHP) )
        {
          scPut_Message_Set( wk, bpp, BTL_STRID_SET_Drain );
        }
      }
    }
  }
}

static void scproc_Damage_Drain( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, u32 damage )
{
  if( WAZADATA_GetCategory(wazaParam->wazaID) == WAZADATA_CATEGORY_DRAIN )
  {
    u32 ratio = WAZADATA_GetParam( wazaParam->wazaID, WAZAPARAM_DAMAGE_RECOVER_RATIO );
    u32 recoverHP = BTL_CALC_MulRatioInt( damage, ratio );

    if( scproc_DrainCore(wk, attacker, defender, recoverHP) )
    {
      scPut_Message_Set( wk, defender, BTL_STRID_SET_Drain );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * HP吸い取り処理コア
 *
 * @param   wk
 * @param   attacker    吸い取る側ポケID
 * @param   target      吸い取られる側ポケID（NULLでも可：「ねをはる」を大きなねっこ対象にするため利用）
 * @param   drainHP
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scproc_DrainCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, u16 drainHP )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  BOOL result = FALSE;

  drainHP = scEvent_RecalcDrainVolume( wk, attacker, target, drainHP );
  if( drainHP > 0 )
  {
    if( !scproc_RecoverHP_CheckFailBase(wk, attacker) )
    {
      result = scproc_RecoverHP( wk, attacker, drainHP, TRUE );
    }
  }
//  else{
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
//  }
  BTL_Hem_PopState( &wk->HEManager, hem_state );
  return result;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ダメージワザ処理終了
 *
 * @param   wk
 * @param   attacker
 * @param   targets
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_DamageProcEnd( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BTL_POKESET* targets,
  const SVFL_WAZAPARAM* wazaParam, BOOL fDelayAttack )
{
  scEvent_DamageProcEndSub( wk, attacker, targets, wazaParam, fDelayAttack, TRUE,  BTL_EVENT_DAMAGEPROC_END_HIT_REAL );
  scEvent_DamageProcEndSub( wk, attacker, targets, wazaParam, fDelayAttack, FALSE, BTL_EVENT_DAMAGEPROC_END_HIT );

  // ダメージ反応アイテム
  {
    BTL_POKEPARAM* bpp;
    BTL_POKESET_SeekStart( targets );
    while( (bpp = BTL_POKESET_SeekNext(targets)) != NULL )
    {
      scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_HP );
    }
  }

  scEvent_DamageProcEndSub( wk, attacker, targets, wazaParam, fDelayAttack, TRUE,  BTL_EVENT_DAMAGEPROC_END );
}
/**
 *  Event] ダメージワザ処理終了（下請け）
 */
static void scEvent_DamageProcEndSub( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, BTL_POKESET* targets,
  const SVFL_WAZAPARAM* wazaParam, BOOL fDelayAttack, BOOL fRealHitOnly, BtlEventType eventID )
{
  const BTL_POKEPARAM* bpp;
  u32 damage, damage_sum, target_cnt, hit_cnt, i;

  target_cnt = BTL_POKESET_GetCount( targets );
  hit_cnt = damage_sum = 0;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DELAY_ATTACK_FLAG, fDelayAttack );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );

    for(i=0; i<target_cnt; ++i)
    {
      bpp = BTL_POKESET_Get( targets, i );
      if( fRealHitOnly ){
        damage = BTL_POKESET_GetDamageReal( targets, bpp );
      }else{
        damage = BTL_POKESET_GetDamage( targets, bpp );
      }

      if( damage )
      {
        damage_sum += damage;
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+hit_cnt, BPP_GetID(bpp) );
        ++hit_cnt;
      }
    }
    if( hit_cnt )
    {
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, hit_cnt );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE, damage_sum );
      BTL_EVENT_CallHandlers( wk, eventID );
    }
  BTL_EVENTVAR_Pop();
}

//------------------------------------------------------------------
// サーバーフロー下請け： たたかう > ダメージワザ系 > ダメージ値計算
//------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * [Event] ダメージ計算
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 * @param   typeAff
 * @param   targetDmgRatio
 * @param   criticalFlag
 * @param   dstDamage       [out] 計算結果
 *
 * @retval  BOOL     固定ダメージ値（防御・攻撃等の値と関係なく与えるダメージ）の場合、TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CalcDamage( BTL_SVFLOW_WORK* wk,
    const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam,
    BtlTypeAff typeAff, fx32 targetDmgRatio, BOOL criticalFlag, BOOL fSimurationMode, u16* dstDamage )
{
  enum {
    PRINT_FLG = TRUE,
  };
  WazaDamageType dmgType = WAZADATA_GetDamageType( wazaParam->wazaID );
  u32  rawDamage = 0;
  BOOL fFixDamage = FALSE;

  BTL_EVENTVAR_Push();

  BTL_EVENTVAR_SetConstValue( BTL_EVAR_TYPEAFF, typeAff );
  BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
  BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
  BTL_EVENTVAR_SetConstValue( BTL_EVAR_CRITICAL_FLAG, criticalFlag );
  BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
  BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, dmgType );
  BTL_EVENTVAR_SetValue( BTL_EVAR_FIX_DAMAGE, 0 );

  BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_PROC1 );
  rawDamage = BTL_EVENTVAR_GetValue( BTL_EVAR_FIX_DAMAGE );

  if( rawDamage )
  {
    fFixDamage = TRUE;
  }
  else
  {
    u32 fxDamage;
    u16 atkPower, defGuard, wazaPower;
    u8 atkLevel;
    // 各種パラメータから素のダメージ値計算
    wazaPower = scEvent_getWazaPower( wk, attacker, defender, wazaParam );
    atkPower  = scEvent_getAttackPower( wk, attacker, defender, wazaParam, criticalFlag );
    defGuard  = scEvent_getDefenderGuard( wk, attacker, defender, wazaParam, criticalFlag );
    {
      atkLevel = BPP_GetValue( attacker, BPP_LEVEL );
      fxDamage = BTL_CALC_DamageBase( wazaPower, atkPower, atkLevel, defGuard );

      BTL_N_PrintfEx(PRINT_FLG, DBGSTR_CALCDMG_WazaParam, wazaParam->wazaID, wazaParam->wazaType);
      BTL_N_PrintfEx(PRINT_FLG, DBGSTR_CALCDMG_BaseDamage, fxDamage);
    }
    // ワザ対象数による補正
    if( targetDmgRatio != BTL_CALC_DMG_TARGET_RATIO_NONE ){
      fxDamage  = BTL_CALC_MulRatio( fxDamage, targetDmgRatio );
      BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_RangeHosei, fxDamage, targetDmgRatio);
    }
    // 天候補正
    {
      BtlWeather weather = scEvent_GetWeather( wk );
      fx32 weatherDmgRatio = BTL_CALC_GetWeatherDmgRatio( weather, wazaParam->wazaType );
      if( weatherDmgRatio != BTL_CALC_DMG_WEATHER_RATIO_NONE )
      {
        u32 prevDmg = fxDamage;
        fxDamage = BTL_CALC_MulRatio( fxDamage, weatherDmgRatio );
        BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_WeatherHosei, weatherDmgRatio, prevDmg, fxDamage);
      }
    }
    // 素ダメージ値を確定
    BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_RAW_DAMAGE,
        wazaPower, atkLevel, atkPower, defGuard, fxDamage );

    // クリティカルで２倍
    if( criticalFlag ){
      fxDamage *= 2;
      BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_Critical, fxDamage);
    }
    //ランダム補正(100～85％)
    if( !BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_DMG_RAND_OFF) )
    {
      u16 ratio;
      if( !fSimurationMode ){
        ratio = 100 - BTL_CALC_GetRand( 16 );
      }else{
        ratio = 85;
      }
      fxDamage = (fxDamage * ratio) / 100;
      BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_RandomHosei, ratio, fxDamage);
    }
    // タイプ一致補正
    if( wazaParam->wazaType != POKETYPE_NULL )
    {
      fx32 ratio = scEvent_CalcTypeMatchRatio( wk, attacker, wazaParam->wazaType );
      fxDamage = BTL_CALC_MulRatio( fxDamage, ratio );
      if( ratio != FX32_ONE ){
        BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_TypeMatchHosei, (ratio*100>>FX32_SHIFT), fxDamage);
      }
    }
    // タイプ相性計算
    fxDamage = BTL_CALC_AffDamage( fxDamage, typeAff );
    BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_TypeAffInfo, typeAff, fxDamage);
    // やけど補正
    if( (dmgType == WAZADATA_DMG_PHYSIC)
    &&  (BPP_GetPokeSick(attacker) == POKESICK_YAKEDO)
    &&  (BPP_GetValue(attacker, BPP_TOKUSEI_EFFECTIVE) != POKETOKUSEI_KONJOU)
    ){
      fxDamage = (fxDamage * BTL_YAKEDO_DAMAGE_RATIO) / 100;
    }
    rawDamage = fxDamage;
    if( rawDamage == 0 ){ rawDamage = 1; }

    // 各種ハンドラによる倍率計算
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.01f), FX32_CONST(32) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, rawDamage );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_PROC2 );
    {
      fx32 ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
      rawDamage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
      rawDamage = BTL_CALC_MulRatio( rawDamage, ratio );
      BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CALCDMG_DamageResult,
            BPP_GetID(defender), ratio, BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE ), rawDamage);
    }
  } /* if( rawDamage == 0 ) */

  BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_PROC_END );

  BTL_EVENTVAR_Pop();

  *dstDamage = rawDamage;
  return fFixDamage;
}
//---------------------------------------------------------------------------------------------
// ワザダメージ処理後の反動処理（ダメージ×反動率）
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_Kickback( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, WazaID waza, u32 wazaDamage )
{
  if( !BPP_IsDead(attacker) )
  {
    u32  damage;
    BOOL fMustEnable;

    damage = scEvent_CalcKickBack( wk, attacker, waza, wazaDamage, &fMustEnable );

    if( damage )
    {
      HANDEX_STR_Setup( &wk->strParam, BTL_STRTYPE_SET, BTL_STRID_SET_ReactionDmg );
      HANDEX_STR_AddArg( &wk->strParam, BPP_GetID(attacker) );
      if( (fMustEnable)
      ||  (scproc_SimpleDamage_CheckEnable(wk, attacker, damage))
      ){
        scproc_SimpleDamage_Core( wk, attacker, damage, &wk->strParam );
      }
    }
  }
}
//---------------------------------------------------------------------------------------------
// ワザ以外のダメージ共通処理
//---------------------------------------------------------------------------------------------
/**
 *  ワザ以外ダメージ：有効チェック（有効ならTRUE）
 */
static BOOL scproc_SimpleDamage_CheckEnable( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage )
{
  if( !scEvent_CheckEnableSimpleDamage(wk, bpp, damage) )
  {
    return FALSE;
  }
  return TRUE;
}
/**
 *  ワザ以外ダメージ：実行
 */
static BOOL scproc_SimpleDamage_Core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage, BTL_HANDEX_STR_PARAMS* str )
{
  int value = -damage;

  if( value )
  {
    scPut_SimpleHp( wk, bpp, value, TRUE );
    BPP_TURNFLAG_Set( bpp, BPP_TURNFLG_DAMAGED );

    if( str != NULL ){
      handexSub_putString( wk, str );
      HANDEX_STR_Clear( str );
    }

    scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_HP );

//    TAYA_Printf("BPP ID=%d, HP=%d, ADRS=%p\n", BPP_GetID(bpp), BPP_GetValue(bpp,BPP_HP), bpp);

    if( scproc_CheckDeadCmd(wk, bpp) ){
      if( scproc_CheckShowdown(wk) ){
        return TRUE;
      }
    }

    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Proc] 装備アイテム使用
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL  装備アイテム使用したらTRUE／装備なし・使用できない場合FALSE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_UseItemEquip( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  u32 itemID = BPP_GetItem( bpp );
  u32 hem_state_1st;
  BOOL result = FALSE;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, BPP_GetID(bpp)) )
  {
    hem_state_1st = BTL_Hem_PushState( &wk->HEManager );
    result = !scEvent_CheckItemEquipFail( wk, bpp, itemID );
    if( result )
    {
      u32 hem_state_2nd;
      u16 que_reserve_pos;
      u8 fConsume = FALSE;

      que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_KINOMI );
//      scPut_UseItemAct( wk, bpp );

      hem_state_2nd = BTL_Hem_PushStateUseItem( &wk->HEManager, itemID );
        scEvent_ItemEquip( wk, bpp );
        if( scproc_HandEx_Result(wk) != HandExResult_Enable ){
          result = FALSE;
        }
      BTL_Hem_PopState( &wk->HEManager, hem_state_2nd );

      if( result )
      {
        SCQUE_PUT_ReservedPos( wk->que, que_reserve_pos, SC_ACT_KINOMI, BPP_GetID(bpp) );

        if( BTL_CALC_ITEM_GetParam(itemID, ITEM_PRM_ITEM_SPEND) )
        {
          scproc_ItemChange( wk, bpp, ITEM_DUMMY_DATA );
          scproc_ConsumeItem( wk, bpp );
        }
      }
    }

    BTL_Hem_PopState( &wk->HEManager, hem_state_1st );
  }
  return result;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 装備アイテム使用可否チェック
 *
 * @param   wk
 * @param   bpp
 * @param   itemID
 *
 * @retval  BOOL    特殊な原因で使用できなかったらTRUE／それ以外FALSE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckItemEquipFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 itemID )
{
  BOOL failFlag = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ITEM, itemID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_ITEMEQUIP_FAIL );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  return failFlag;
}
//----------------------------------------------------------------------------------
/**
 * 装備アイテム消費処理
 *
 * @param   wk
 * @param   bpp
 * @param   itemID
 */
//----------------------------------------------------------------------------------
static void scproc_ConsumeItem( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  u16 itemID = BPP_GetItem( bpp );

  BPP_ConsumeItem( bpp );
  SCQUE_PUT_OP_ConsumeItem( wk->que, BPP_GetID(bpp) );

  scPut_SetTurnFlag( wk, bpp, BPP_TURNFLG_ITEM_CONSUMED );
}
//----------------------------------------------------------------------------------
// アイテム書き換え共通処理
//----------------------------------------------------------------------------------
static void scproc_ItemChange( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 nextItemID )
{
  u8 pokeID = BPP_GetID( bpp );
  u16 prevItemID = BPP_GetItem( bpp );
  u32 hem_state;

  // アイテム書き換え確定ハンドラ呼び出し
  hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_ItemSetDecide( wk, bpp, nextItemID );
  BTL_Hem_PopState( &wk->HEManager, hem_state );

  BTL_HANDLER_ITEM_Remove( bpp );
  SCQUE_PUT_OP_SetItem( wk->que, pokeID, nextItemID );
  BPP_SetItem( bpp, nextItemID );

  // アイテム書き換え完了ハンドラ呼び出し
  if( nextItemID != ITEM_DUMMY_DATA ){
    BTL_HANDLER_ITEM_Add( bpp );
  }

  hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_ItemSetFixed( wk, bpp );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}


//---------------------------------------------------------------------------------------------
// ポケモンを強制的に瀕死にする
//---------------------------------------------------------------------------------------------
static void scproc_KillPokemon( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  scPut_KillPokemon( wk, bpp, 0 );
  scproc_CheckDeadCmd( wk, bpp );
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ダメージワザの追加効果による状態異常
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_AddSick( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
  WazaSick sick;
  BPP_SICK_CONT sick_cont;
  u32 i=0;

  sick = scEvent_CheckWazaAddSick( wk, wazaParam->wazaID, attacker, target, &sick_cont );
  if( sick != WAZASICK_NULL )
  {
    if( !BPP_IsDead(target) ){
      scproc_Fight_WazaSickCore( wk,  attacker, target, wazaParam->wazaID, sick, sick_cont, FALSE );
    }
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
 * @param   [out] pSickCont   継続効果
 *
 * @retval  WazaSick          状態異常ID
 */
//--------------------------------------------------------------------------
static WazaSick scEvent_CheckWazaAddSick( BTL_SVFLOW_WORK* wk, WazaID waza,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BPP_SICK_CONT* pSickCont )
{
  BPP_SICK_CONT sickCont;
  WazaSick sick = WAZADATA_GetParam( waza, WAZAPARAM_SICK );
  WAZA_SICKCONT_PARAM  waza_contParam = WAZADATA_GetSickCont( waza );
  u8 per = WAZADATA_GetParam( waza, WAZAPARAM_SICK_PER );
  u8 fFail = FALSE;

  BTL_CALC_WazaSickContToBppSickCont( waza_contParam, attacker, &sickCont );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );

    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, fFail );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_SICK_CONT, sickCont.raw );
    BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_SICK );

    sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
    per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    if( sick == WAZASICK_SPECIAL_CODE ){
      sick = WAZASICK_NULL;
    }
    sickCont.raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );

  BTL_EVENTVAR_Pop();

  if( !fFail )
  {
    if( sick != WAZASICK_NULL )
    {
      if( perOccur(wk, per) ){
        *pSickCont = sickCont;
        return sick;
      }
      // デバッグ機能により必ず発生
      if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_MUST_TUIKA) ){
        *pSickCont = sickCont;
        return sick;
      }
    }
  }
  return  WAZASICK_NULL;
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接の状態異常
//---------------------------------------------------------------------------------------------
static void scproc_Fight_SimpleSick( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM* target;
  WazaSick sick;
  WAZA_SICKCONT_PARAM contParam;
  BOOL fSucceed;
  u32  targetCnt;

  sick = WAZADATA_GetParam( waza, WAZAPARAM_SICK );
  contParam = WAZADATA_GetSickCont( waza );
  fSucceed = FALSE;

  targetCnt = BTL_POKESET_GetCount( targetRec );

  if( targetCnt )
  {
    BTL_POKESET_SeekStart( targetRec );
    while( (target = BTL_POKESET_SeekNext(targetRec)) != NULL )
    {
      BPP_SICK_CONT sickCont;
      BTL_CALC_WazaSickContToBppSickCont( contParam, attacker, &sickCont );
      if( scproc_Fight_WazaSickCore( wk, attacker, target, waza, sick, sickCont, TRUE) ){
        wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
        fSucceed = TRUE;  // ターゲットが居て、１体でも状態異常にかかれば成功
      }
    }
  }
  else{
    // 既にターゲットが無くなっている->しかしうまく決まらなかった
    scPut_WazaFail( wk, attacker, waza );
  }
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる状態異常共通
//---------------------------------------------------------------------------------------------
static BOOL scproc_Fight_WazaSickCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target,
  WazaID waza, WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost )
{
  HANDEX_STR_Clear( &wk->strParam );

  if( sick == WAZASICK_SPECIAL_CODE )
  {
    sick = scEvent_DecideSpecialWazaSick( wk, attacker, target, &wk->strParam );
    if( (sick == WAZASICK_NULL) || (sick == WAZASICK_SPECIAL_CODE) ){
      return FALSE;
    }
  }
  else{
    scEvent_GetWazaSickAddStr( wk, sick, attacker, target, &wk->strParam );
  }
  scEvent_WazaSickCont( wk, attacker, target, sick, &sickCont );

  {
    // 特殊メッセージが何も用意されていなければ、scproc_AddSick 内で、
    // できるだけ標準メッセージを出すようにしている。
    BOOL fDefaultMsg = (HANDEX_STR_IsEnable(&wk->strParam) == FALSE );
    if( scproc_AddSickRoot(wk, target, attacker, sick, sickCont, fAlmost, fDefaultMsg) )
    {
      if( !fDefaultMsg ){
        handexSub_putString( wk, &wk->strParam );
        HANDEX_STR_Clear( &wk->strParam );
      }
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによる特殊処理の状態異常ID＆成功時メッセージを決定する
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   str
 *
 * @retval  WazaSick
 */
//----------------------------------------------------------------------------------
static WazaSick scEvent_DecideSpecialWazaSick( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* str )
{
  WazaSick sickID;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WORK_ADRS, (int)(str) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, WAZASICK_NULL );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASICK_SPECIAL );
    sickID = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
  BTL_EVENTVAR_Pop();
  return sickID;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによる通常の状態異常に対する成功時メッセージを決定する
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   str
 */
//----------------------------------------------------------------------------------
static void scEvent_GetWazaSickAddStr( BTL_SVFLOW_WORK* wk, WazaSick sick,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BTL_HANDEX_STR_PARAMS* str )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WORK_ADRS, (int)(str) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sick );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASICK_STR );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによる状態異常の継続パラメータ調整
 *
 * @param   wk
 * @param   attacker
 * @param   target
 * @param   sick
 * @param   sickCont    [io]調整前パラメータ／調整後の結果も格納
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaSickCont( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target,
  WazaSick sick, BPP_SICK_CONT* sickCont )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sick );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SICK_CONT, sickCont->raw );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASICK_PARAM );
    sickCont->raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );
  BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [proc] 状態異常処理ルート
 *
 * @param   wk
 * @param   target
 * @param   attacker
 * @param   sick
 * @param   sickCont
 * @param   fAlmost            失敗した時に原因メッセージを表示する
 *
 * @retval  BOOL       成功した場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_AddSickRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BOOL fAlmost, BOOL fDefaultMsgEnable )
{
  if( scproc_AddSickCheckFail(wk, target, attacker, sick, sickCont, BTL_SICK_OW_NONE, fAlmost) == FALSE )
  {
    scproc_AddSickCore( wk, target, attacker, sick, sickCont, fDefaultMsgEnable, NULL );
    return TRUE;
  }
  return FALSE;
}
/**
* 状態異常成否チェック
*/
static BOOL scproc_AddSickCheckFail( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BtlSickOverWriteMode overWriteMode, BOOL fDispFailResult )
{
  // 基本ルールによる失敗
  BtlAddSickFailCode fail_code = addsick_check_fail_std( wk, target, sick, sickCont, overWriteMode );
  if( fail_code != BTL_ADDSICK_FAIL_NULL )
  {
    BTL_N_Printf( DBGSTR_SVFL_AddSickFailCode, fail_code );
    if( fDispFailResult ){
      scPut_AddSickFail( wk, target, fail_code, sick );
    }
    return TRUE;
  }
  // とくせいなど特殊条件による失敗
  else
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

    BOOL fFail = scEvent_WazaSick_CheckFail( wk, attacker, target, sick );

    if( fFail )
    {
      if( fDispFailResult ){
        scEvent_AddSick_Failed( wk, target, attacker, sick );
//        scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
      }
    }

    BTL_Hem_PopState( &wk->HEManager, hem_state );
    return fFail;
  }
}
/**
 *  状態異常失敗チェック（基本ルール）
 */
static BtlAddSickFailCode addsick_check_fail_std( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaSick sick, BPP_SICK_CONT sickCont, BtlSickOverWriteMode overWriteMode )
{
  // すでに同じ状態異常になっているなら失敗
  if( BPP_CheckSick(target, sick) )
  {
    if( overWriteMode != BTL_SICK_OW_STRONG ){
      return BTL_ADDSICK_FAIL_ALREADY;
    }
  }

  // すでに基本状態異常になっているなら、他の基本状態異常にはならない
  if( (sick < POKESICK_MAX)
  &&  (BPP_GetPokeSick(target) != POKESICK_NULL)
  ){
    if( overWriteMode == BTL_SICK_OW_NONE ){
      return BTL_ADDSICK_FAIL_OTHER;
    }
  }

  // てんこう「はれ」の時に「こおり」にはならない
  if( (scEvent_GetWeather(wk) == BTL_WEATHER_SHINE)
  &&  (sick == POKESICK_KOORI)
  ){
    return BTL_ADDSICK_FAIL_OTHER;
  }

  // はがね or どくタイプは、「どく」にならない
  if( sick==WAZASICK_DOKU )
  {
    PokeTypePair type = BPP_GetPokeType( target );
    if( PokeTypePair_IsMatch(type, POKETYPE_HAGANE)
    ||  PokeTypePair_IsMatch(type, POKETYPE_DOKU)
    ){
      return BTL_ADDSICK_FAIL_NO_EFFECT;
    }
  }

  // ほのおタイプは、「やけど」にならない
  if( sick==WAZASICK_YAKEDO )
  {
    PokeTypePair type = BPP_GetPokeType( target );
    if( PokeTypePair_IsMatch(type, POKETYPE_HONOO) ){
      return BTL_ADDSICK_FAIL_NO_EFFECT;
    }
  }

  // こおりタイプは、「こおり」にならない
  if( sick==WAZASICK_KOORI )
  {
    PokeTypePair type = BPP_GetPokeType( target );
    if( PokeTypePair_IsMatch(type, POKETYPE_KOORI) ){
      return BTL_ADDSICK_FAIL_NO_EFFECT;
    }
  }

  // くさタイプは、「やどりぎのタネ」にならない
  if( sick==WAZASICK_YADORIGI )
  {
    PokeTypePair type = BPP_GetPokeType( target );
    if( PokeTypePair_IsMatch(type, POKETYPE_KUSA) ){
      return BTL_ADDSICK_FAIL_NO_EFFECT;
    }
  }

  // 基本状態異常の時には「あくび」にならない
  if( sick == WAZASICK_AKUBI )
  {
    if( BPP_GetPokeSick(target) != POKESICK_NULL ){
      return BTL_ADDSICK_FAIL_OTHER;
    }
  }

  // マルチタイプは「いえき」にならない
  if( sick == WAZASICK_IEKI )
  {
    if( BPP_GetValue(target, BPP_TOKUSEI) == POKETOKUSEI_MARUTITAIPU ){
      return BTL_ADDSICK_FAIL_OTHER;
    }
  }

  return BTL_ADDSICK_FAIL_NULL;
}
//----------------------------------------------------------------------------------
/**
 * 状態異常処理コア（確定後）
 *
 * @param   wk
 * @param   target
 * @param   attacker
 * @param   sick
 * @param   sickCont
 * @param   fDefaultMsgEnable
 */
//----------------------------------------------------------------------------------
static void scproc_AddSickCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, BTL_POKEPARAM* attacker,
  WazaSick sick, BPP_SICK_CONT sickCont, BOOL fDefaultMsgEnable, const BTL_HANDEX_STR_PARAMS* exStr )
{
  scPut_AddSick( wk, target, sick, sickCont );

  // エフェクトの用意されている異常はそれを表示
  switch( sick ){
  case WAZASICK_DOKU:     scPut_EffectByPokePos( wk, target, BTLEFF_DOKU ); break;
  case WAZASICK_YAKEDO:   scPut_EffectByPokePos( wk, target, BTLEFF_YAKEDO ); break;
  case WAZASICK_MAHI:     scPut_EffectByPokePos( wk, target, BTLEFF_MAHI ); break;
  case WAZASICK_KOORI:    scPut_EffectByPokePos( wk, target, BTLEFF_KOORI ); break;
  case WAZASICK_NEMURI:   scPut_EffectByPokePos( wk, target, BTLEFF_NEMURI ); break;
  case WAZASICK_KONRAN:   scPut_EffectByPokePos( wk, target, BTLEFF_KONRAN ); break;
  case WAZASICK_MEROMERO: scPut_EffectByPokePos( wk, target, BTLEFF_MEROMERO ); break;
  }

  // 状態異常確定標準メッセージ出力
  if( fDefaultMsgEnable )
  {
    BTL_SICK_MakeDefaultMsg( sick, sickCont, target, &wk->strParam );
    handexSub_putString( wk, &wk->strParam );
    HANDEX_STR_Clear( &wk->strParam );
  }
  else if( exStr != NULL ){
    handexSub_putString( wk, exStr );
  }

  // シェイミがランドフォルムに戻る
  if( (sick == WAZASICK_KOORI)
  &&  (BPP_GetMonsNo(target) == MONSNO_SHEIMI)
  &&  (BPP_GetValue(target,BPP_FORM) == FORMNO_SHEIMI_SKY)
  ){
    BPP_ChangeForm( target, FORMNO_SHEIMI_LAND );
    SCQUE_PUT_ACT_ChangeForm( wk->que, BPP_GetID(target), FORMNO_SHEIMI_LAND );
    scPut_Message_Set( wk, target, BTL_STRID_SET_ChangeForm );
  }

  // 状態異常確定イベントコール
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

    if( BTL_CALC_IsBasicSickID(sick) ){
      scEvent_PokeSickFixed( wk, target, attacker, sick, sickCont );
    }else if( sick == WAZASICK_IEKI )
    {
      scEvent_IekiFixed( wk, target );
      if( BPP_GetValue(target, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_KINCHOUKAN )
      {
        scproc_KintyoukanMoved( wk, target );
      }
    }else {
      scEvent_WazaSickFixed( wk, target, attacker, sick );
    }
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }

  // アイテム反応イベントへ
  scproc_CheckItemReaction( wk, target, BTL_ITEMREACTION_SICK );
}
//----------------------------------------------------------------------------------
/**
 * [Event] 天候チェック
 *
 * @param   wk
 *
 * @retval  BtlWeather
 */
//----------------------------------------------------------------------------------
static BtlWeather scEvent_GetWeather( BTL_SVFLOW_WORK* wk )
{
  BOOL fDisable = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHECK );
    fDisable = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  if( fDisable ){
    return BTL_WEATHER_NONE;
  }

  return BTL_FIELD_GetWeather();
}
//----------------------------------------------------------------------------------
/**
 * [Event] 体重変化倍率取得（ライトメタル・ヘヴィメタルなど）
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static fx32 svEvent_GetWaitRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  fx32 ratio;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.1), FX32_CONST(32) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEIGHT_RATIO );
    ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
  BTL_EVENTVAR_Pop();

  return ratio;
}

//----------------------------------------------------------------------------------
/**
 * [Event] 状態異常を失敗するケースのチェック
 *
 * @param   wk
 * @param   target
 * @param   sick
 *
 * @retval  BOOL    失敗する場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_WazaSick_CheckFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaSick sick  )
{
  BOOL fFail = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, (attacker!=NULL)? BPP_GetID(attacker) : BTL_POKEID_NULL );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sick );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, fFail );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADDSICK_CHECKFAIL );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  return fFail;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 状態異常失敗が確定した
 *
 * @param   wk
 * @param   target
 * @param   sick
 */
//----------------------------------------------------------------------------------
static void scEvent_AddSick_Failed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker, WazaSick sick )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, sick );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADDSICK_FAILED );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ポケモン系状態異常確定
 *
 * @param   wk
 * @param   target
 * @param   attacker
 * @param   sick
 */
//----------------------------------------------------------------------------------
static void scEvent_PokeSickFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker,
  PokeSick sick, BPP_SICK_CONT sickCont )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    {
      u8 atkPokeID = (attacker!=NULL)? BPP_GetID(attacker) : BTL_POKEID_NULL;
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, atkPokeID );
    }
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sick );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICK_CONT, sickCont.raw );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_POKESICK_FIXED );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ系状態異常確定
 *
 * @param   wk
 * @param   target
 * @param   attacker
 * @param   sick
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaSickFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, const BTL_POKEPARAM* attacker, WazaSick sick )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    {
      u8 atkPokeID = (attacker!=NULL)? BPP_GetID(attacker) : BTL_POKEID_NULL;
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, atkPokeID );
    }
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sick );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZASICK_FIXED );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] いえきによる特性無効化の確定
 *
 * @param   wk
 * @param   target
 */
//----------------------------------------------------------------------------------
static void scEvent_IekiFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(target) );
    BTL_EVENT_ForceCallHandlers( wk, BTL_EVENT_IEKI_FIXED );
  BTL_EVENTVAR_Pop();
}


//---------------------------------------------------------------------------------------------
// サーバーフロー：ダメージワザの追加効果によるランク効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Damage_AddEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
  if( !BPP_IsDead(target) )
  {
    if( scEvent_CheckAddRankEffectOccur(wk, wazaParam, attacker, target) )
    {
      scproc_WazaRankEffect_Common( wk, wazaParam, attacker, target, FALSE );
    }
  }
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
 * @retval  BOOL    発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckAddRankEffectOccur( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target )
{
  u8 per = WAZADATA_GetParam( wazaParam->wazaID, WAZAPARAM_RANKPER_1 );
  u8 failFlag;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ADD_RANK_TARGET );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
  BTL_EVENTVAR_Pop();

  if( !failFlag )
  {
    if( perOccur(wk, per) ){
      return TRUE;
    }
    // デバッグ機能により必ず発生
    if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_MUST_TUIKA) ){
      return TRUE;
    }
  }

  return FALSE;
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：ワザによる直接のランク効果
//---------------------------------------------------------------------------------------------
static void scproc_Fight_SimpleEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM* target;
  u32 i = 0;

  BTL_POKESET_SeekStart( targetRec );
  while( (target = BTL_POKESET_SeekNext(targetRec)) != NULL )
  {
    if( scproc_WazaRankEffect_Common(wk, wazaParam, attacker, target, TRUE) )
    {
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
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
 * @retval  BOOL    ランク増減効果が発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_WazaRankEffect_Common( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, BOOL fAlmost )
{
  BOOL ret = FALSE;
  u32 eff_cnt, i;
  u8 atkPokeID = BPP_GetID( attacker );

  wk->wazaRankEffSerial++;

  eff_cnt = WAZADATA_GetRankEffectCount( wazaParam->wazaID );
//  BTL_Printf("ワザ:%dのエフェクト数=%d\n", wazaParam->wazaID, eff_cnt);
  for(i=0; i<eff_cnt; ++i)
  {
    WazaRankEffect  effect;
    int  volume;
    BOOL fEffective = FALSE;

    scEvent_GetWazaRankEffectValue( wk, wazaParam->wazaID, i, attacker, target, &effect, &volume );
    if( effect != WAZA_RANKEFF_NULL )
    {
      if( effect != WAZA_RANKEFF_MULTI5 )
      {
        fEffective = scproc_RankEffectCore( wk, atkPokeID, target, effect, volume, atkPokeID,
                ITEM_DUMMY_DATA, wk->wazaRankEffSerial, fAlmost, TRUE );
      }
      else
      {
        u8 e;
        for( e=WAZA_RANKEFF_ORIGIN; e<WAZA_RANKEFF_BASE_MAX; ++e )
        {
          if( scproc_RankEffectCore(wk, atkPokeID, target, e, volume, atkPokeID,
              ITEM_DUMMY_DATA, wk->wazaRankEffSerial, fAlmost, TRUE)
          ){
            fEffective = TRUE;
          }
        }
      }
      if( fEffective )
      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        scEvent_WazaRankEffectFixed( wk, target, wazaParam->wazaID, effect, volume );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
        ret = TRUE;
      }
    }
  }
  return ret;
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
 * @param   effect    [out] 効果種類
 * @param   volume    [out] 効果値
 *
 */
//--------------------------------------------------------------------------
static void scEvent_GetWazaRankEffectValue( BTL_SVFLOW_WORK* wk, WazaID waza, u32 idx,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, WazaRankEffect* effect, int* volume )
{
  *effect = WAZADATA_GetRankEffect( waza, idx, volume );
  BTL_Printf("ワザ[%d]のランク効果[%d] type=%d, volume=%d\n", waza, idx, *effect, *volume);

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_STATUS_TYPE, *effect );
    BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, *volume );
    BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, 1 );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_GET_RANKEFF_VALUE );
    *effect = BTL_EVENTVAR_GetValue( BTL_EVAR_STATUS_TYPE );
    *volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    {
      u8 ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
      if( ratio > 1 ){
        (*volume) *= ratio;
      }
    }
  BTL_EVENTVAR_Pop();

  if( *effect == WAZA_RANKEFF_SP ){
    *effect = WAZA_RANKEFF_NULL;
  };
}
//--------------------------------------------------------------------------
/**
 * ランク増減効果のコア（ワザ以外もこれを呼び出す）
 *
 * @param   wk
 * @param   target
 * @param   effect
 * @param   volume
 * @param   wazaUsePokeID    ワザによるランク増減ならワザ使用者ポケID（それ以外は BTL_POKEID_NULL）
 * @param   itemID    アイテム使用によるランク増減ならアイテムID指定（それ以外は ITEM_DUMMY_DATA を指定）
 * @param   fAlmost   特殊要因で効果失敗した時、そのメッセージを表示する
 * @param   fStdMsg   標準メッセージを出力する（○○は××があがった！など）
 *
 * @retval  BOOL    ランク増減効果が発生したらTRUE
 */
//--------------------------------------------------------------------------
static BOOL scproc_RankEffectCore( BTL_SVFLOW_WORK* wk, u8 atkPokeID, BTL_POKEPARAM* target,
  WazaRankEffect effect, int volume, u8 wazaUsePokeID, u16 itemID, u32 rankEffSerial, BOOL fAlmost, BOOL fStdMsg )
{
  volume = scEvent_RankValueChange( wk, target, effect, wazaUsePokeID, itemID, volume );
  BTL_N_Printf( DBGSTR_SVFL_RankEffCore, BPP_GetID(target), effect, volume, atkPokeID );

  // すでに限界ならそれを表示
  if( !BPP_IsRankEffectValid(target, effect, volume) ){
    if( fAlmost ){
      scPut_RankEffectLimit( wk, target, effect, volume );
    }
    return FALSE;
  }

  // ターゲットがみがわり状態の時、自分以外からの作用は受けない
  if( BPP_MIGAWARI_IsExist(target) )
  {
    u8 targetPokeID = BPP_GetID( target );
    if( atkPokeID != targetPokeID )
    {
      if( fAlmost ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, targetPokeID );
      }
      return FALSE;
    }
  }

  {
    BOOL ret = TRUE;

    if( scEvent_CheckRankEffectSuccess(wk, target, effect, atkPokeID, volume, rankEffSerial) )
    {
      // ここまで来たらランク効果発生
      scPut_RankEffect( wk, target, effect, volume, itemID, fStdMsg );
      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        scEvent_RankEffect_Fix( wk, atkPokeID, target, effect, volume );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
      }
    }
    else
    {
      BTL_Printf("ランク効果失敗\n");
      if( fAlmost )
      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        scEvent_RankEffect_Failed( wk, target, rankEffSerial );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
      }
      ret = FALSE;
    }

    return ret;
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] ランク効果の増減値を変化させる
 *
 * @param   wk
 * @param   target
 * @param   rankType
 * @param   wazaUsePokeID
 * @param   itemID
 * @param   volume
 *
 * @retval  int   変化後の増減値
 */
//----------------------------------------------------------------------------------
static int scEvent_RankValueChange( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect rankType,
  u8 wazaUsePokeID, u16 itemID, int volume )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, wazaUsePokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_STATUS_TYPE, rankType );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_VOLUME, volume );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_RANKVALUE_CHANGE );
    volume  = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
  BTL_EVENTVAR_Pop();
  return volume;
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：ランク効果＆状態異常を同時に与えるワザ
//---------------------------------------------------------------------------------------------
static void scproc_Fight_EffectSick( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  WazaSick sick;
  WAZA_SICKCONT_PARAM contParam;
  BPP_SICK_CONT sickCont;
  BTL_POKEPARAM* target;

  sick = WAZADATA_GetParam( wazaParam->wazaID, WAZAPARAM_SICK );
  contParam = WAZADATA_GetSickCont( wazaParam->wazaID );

  BTL_CALC_WazaSickContToBppSickCont( contParam, attacker, &sickCont );

  BTL_POKESET_SeekStart( targetRec );
  while( (target = BTL_POKESET_SeekNext(targetRec)) != NULL )
  {
    if( scproc_WazaRankEffect_Common(wk, wazaParam, attacker, target, TRUE) ){
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
    }
    if( scproc_Fight_WazaSickCore(wk, attacker, target, wazaParam->wazaID, sick, sickCont, FALSE) ){
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
    }
  }
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：HP回復
//---------------------------------------------------------------------------------------------
static void scproc_Fight_SimpleRecover( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM* target;
  u8  pokeID;

  BTL_POKESET_SeekStart( targetRec );
  while( (target = BTL_POKESET_SeekNext(targetRec)) != NULL )
  {
    pokeID = BPP_GetID( target );
    if( !scproc_RecoverHP_CheckFailBase(wk, target) )
    {
      u32 recoverHP = scEvent_CalcRecoverHP( wk, waza, target );
      if( scproc_RecoverHP(wk, target, recoverHP, TRUE) )
      {
        wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_HP_Recover, pokeID );
      }
    }
    else
    {
      if( BPP_IsHPFull(target) ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_HPFull, pokeID );
      }
    }
  }
}
/**
 *  HP回復ルート
 */
static BOOL scproc_RecoverHP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 recoverHP, BOOL fDispFailMsg )
{
  if( !scproc_RecoverHP_CheckFailSP(wk, bpp, fDispFailMsg) )
  {
    scproc_RecoverHP_Core( wk, bpp, recoverHP );
    return TRUE;
  }
  return FALSE;
}
/**
 *  HP回復可否チェック（基本条件）
 */
static BOOL scproc_RecoverHP_CheckFailBase( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  if( !BPP_IsFightEnable(bpp) ){
    return TRUE;
  }
  if( BPP_IsHPFull(bpp) ){
    return TRUE;
  }
  return FALSE;
}
/**
 *  HP回復可否チェック（特殊条件）
 */
static BOOL scproc_RecoverHP_CheckFailSP( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fDispFailMsg )
{
  if( BPP_CheckSick(bpp, WAZASICK_KAIHUKUHUUJI) )
  {
    if( fDispFailMsg ){
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KaifukuFujiExe, BPP_GetID(bpp) );
    }
    return TRUE;
  }
  return FALSE;
}
/**
 *  HP回復処理コア
 */
static void scproc_RecoverHP_Core( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 recoverHP )
{
  u8 pokeID = BPP_GetID( bpp );
  BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID );
  if( pos != BTL_POS_NULL ){
    SCQUE_PUT_ACT_EffectByPos( wk->que, pos, BTLEFF_HP_RECOVER );
  }
  scPut_SimpleHp( wk, bpp, recoverHP, TRUE );
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：一撃ワザ処理
//---------------------------------------------------------------------------------------------
static void scproc_Fight_Ichigeki( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  BTL_POKEPARAM* target;
  u8 atkLevel, defLevel, targetPokeID;
  BtlTypeAff aff;
  u32 i = 0;

  atkLevel = BPP_GetValue( attacker, BPP_LEVEL );

  while( 1 )
  {
    target = BTL_POKESET_Get( targets, i++ );
    if( target == NULL ){ break; }
    if( BPP_IsDead(target) ){ continue; }

    targetPokeID = BPP_GetID( target );

    if( !IsMustHit(attacker, target) )
    {
      // そらをとぶなどによるハズレ
      if( scEvent_CheckPokeHideAvoid(wk, attacker, target, wazaParam->wazaID) ){
        scPut_WazaAvoid( wk, target, wazaParam->wazaID );
        continue;
      }
    }

    // トリプル遠隔地によるハズレ
    if( IsTripleFarPos(wk, attacker, target, wazaParam->wazaID) ){
      scPut_WazaAvoid( wk, target, wazaParam->wazaID );
      continue;
    }

    // 自分よりレベルが高いポケモンには無効
    defLevel = BPP_GetValue( target, BPP_LEVEL );
    if( atkLevel < defLevel ){
      scput_WazaNoEffectIchigeki( wk, target );
      continue;
    }

    // ワザ相性による無効化
    aff = BTL_DMGAFF_REC_Get( &wk->dmgAffRec, targetPokeID );
    if( aff == BTL_TYPEAFF_0 ){
      scput_WazaNoEffectIchigeki( wk, target );
      continue;
    }

    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

      if( scEvent_IchigekiCheck(wk, attacker, target, wazaParam->wazaID) )
      {
        BtlPokePos atkPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, BPP_GetID(attacker) );
        u16  damage = BPP_GetValue( target, BPP_HP );
        BtlTypeAffAbout  affAbout = BTL_CALC_TypeAffAbout( aff );

        wazaEffCtrl_SetEnable( wk->wazaEffCtrl );

        if( BPP_MIGAWARI_IsExist(target) )
        {
          scproc_Ichigeki_Migawari( wk, target, wazaParam, affAbout );
        }
        else
        {
          BppKoraeruCause korae_cause = scEvent_CheckKoraeru( wk, attacker, target, TRUE, &damage );

          if( korae_cause == BPP_KORAE_NONE ){
            scproc_Ichigeki_Succeed( wk, target, wazaParam, affAbout );
          }else{
            scproc_Ichigeki_Korae( wk, target, wazaParam, affAbout, korae_cause, damage );
          }
        }

        wazaDmgRec_Add( wk, atkPos, attacker, target, wazaParam, damage );

        scproc_CheckDeadCmd( wk, target );
        scproc_WazaDamageReaction( wk, attacker, target, wazaParam, aff, damage, FALSE, FALSE );
      }
      else
      {
        if( !scproc_HandEx_Result(wk) ){
          scPut_WazaAvoid( wk, target, wazaParam->wazaID );
        }
      }
       BTL_Hem_PopState( &wk->HEManager, hem_state );
    }

  }/* while(1) */
}
//--------------------------------------------------------------------------
/**
 * 一撃必殺ワザ成功
 */
//--------------------------------------------------------------------------
static void scproc_Ichigeki_Succeed( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam, BtlTypeAffAbout affAbout )
{
  u8 pokeID = BPP_GetID( target );

  BPP_HpZero( target );
  SCQUE_PUT_OP_HpZero( wk->que, pokeID );

  SCQUE_PUT_ACT_WazaDamage( wk->que, pokeID, affAbout, wazaParam->wazaID );
  SCQUE_PUT_ACT_WazaIchigeki( wk->que, pokeID );
}
//--------------------------------------------------------------------------
/**
 * 一撃必殺ワザこらえた
 */
//--------------------------------------------------------------------------
static void scproc_Ichigeki_Korae( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam,
    BtlTypeAffAbout affAbout, BppKoraeruCause korae_cause, u16 damage )
{
  scPut_DecreaseHP( wk, target, damage );

//  SCQUE_PUT_ACT_SimpleHP( wk->que, BPP_GetID(target) );
  SCQUE_PUT_ACT_WazaDamage( wk->que, BPP_GetID(target), affAbout, wazaParam->wazaID );

  scproc_Koraeru( wk, target, korae_cause );
}
//----------------------------------------------------------------------------------
/**
 * 一撃必殺ワザ みがわり破壊
 */
//----------------------------------------------------------------------------------
static void scproc_Ichigeki_Migawari( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, const SVFL_WAZAPARAM* wazaParam, BtlTypeAffAbout affAbout )
{
  u8 pokeID = BPP_GetID( target );

  SCQUE_PUT_ACT_WazaIchigeki( wk->que, pokeID );
  SCQUE_PUT_ACT_WazaDamage( wk->que, pokeID, affAbout, wazaParam->wazaID );
  scproc_Migawari_Delete( wk, target );

}

//---------------------------------------------------------------------------------------------
// サーバーフロー：ふきとばしワザ処理
//---------------------------------------------------------------------------------------------
static void scproc_Fight_PushOut( BTL_SVFLOW_WORK* wk, WazaID waza, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM *target;
  BOOL fFailMsgDisped = FALSE;

  BTL_POKESET_SeekStart( targetRec );
  while( (target = BTL_POKESET_SeekNext(targetRec)) != NULL )
  {
    if( scproc_PushOutCore(wk, attacker, target, FALSE, &fFailMsgDisped, 0, FALSE, NULL) )
    {
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
    }
    else if( fFailMsgDisped == FALSE ){
      scPut_WazaFail( wk, attacker, waza );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * ふきとばし処理コア
 *
 * @param   wk
 * @param   attacker
 * @param   target
 * @param   fForceChange  強制的に入れ替えモードで実行（FALSEならルール等に応じて自動判別）
 * @param   effectNo      成功時エフェクトナンバー（0 = 標準エフェクト）
 * @param   fIgnoreLevel  レベル差による失敗判定を行わない
 * @param   fFailMsgDisped  [out] 特殊な失敗条件発生、原因を表示したらTRUE
 *
 * @retval  BOOL    成功時TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_PushOutCore( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target,
  BOOL fForceChange, BOOL* fFailMsgDisped, u16 effectNo, BOOL fIgnoreLevel, BTL_HANDEX_STR_PARAMS* succeedMsg )
{
  PushOutEffect   eff;

  *fFailMsgDisped = FALSE;

  if( fForceChange ){
    eff = PUSHOUT_EFFECT_CHANGE;
  }else{
    eff = check_pushout_effect( wk );
  }

  if( eff == PUSHOUT_EFFECT_MUSTFAIL ){
    return FALSE;
  }else{

    BtlPokePos targetPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(target) );

    // 対象が場にいなければ失敗
    if( targetPos == BTL_POS_NULL ){
      return FALSE;
    }

    // 対象が死んでたら失敗
    if( BPP_IsDead(target) ){
      return FALSE;
    }

    // フリーフォールで捕まれてる場合は失敗
    if( BPP_CheckSick(target, WAZASICK_FREEFALL) ){
      return FALSE;
    }
    // フリーフォールで捕んでいる場合も失敗
    if( checkFreeFallUsing(target) ){
      return FALSE;
    }

    // 特殊要因で失敗
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      u8 fFailed = scEvent_CheckPushOutFail( wk, attacker, target );
      if( fFailed )
      {
        if( scproc_HandEx_Result(wk) != HandExResult_NULL ){
          (*fFailMsgDisped) = TRUE;
        }
      }
      BTL_Hem_PopState( &wk->HEManager, hem_state );

      if( fFailed ){
        return FALSE;
      }
    }

    // 通常処理
    {
      u8 clientID, posIdx;
      BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, targetPos, &clientID, &posIdx );

      // 強制入れ替え効果
      if( eff == PUSHOUT_EFFECT_CHANGE )
      {
        SVCL_WORK* clwk;
        int nextPokeIdx;

        clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
        nextPokeIdx = get_pushout_nextpoke_idx( wk, clwk );

        if( nextPokeIdx >= 0 )
        {
          BTL_POKEPARAM* nextPoke = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, nextPokeIdx );
          u8 nextPokeID = BPP_GetID( nextPoke );

          scproc_MemberOutCore( wk, target, effectNo );
          if( succeedMsg != NULL ){
            handexSub_putString( wk, succeedMsg );
          }
          scproc_MemberInForChange( wk, clientID, posIdx, nextPokeIdx, FALSE );

          SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_PushOutChange, nextPokeID );
          scproc_AfterMemberIn( wk );
        }
        else{
          return FALSE;
        }
      }
      // バトル離脱効果
      else
      {
        u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( BPP_GetID(attacker) );

        // 対象の方がレベル上だったら失敗
        if( !fIgnoreLevel )
        {
          u8 atkLevel = BPP_GetValue( attacker, BPP_LEVEL );
          u8 tgtLevel = BPP_GetValue( target, BPP_LEVEL );
          if( tgtLevel > atkLevel ){
            return FALSE;
          }
        }

        BTL_ESCAPEINFO_Add( &wk->escInfo, clientID );

        scproc_MemberOutCore( wk, target, effectNo );
        wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
      }
    }
    return TRUE;
  }
}
//--------------------------------------------------------------------------
/**
 * 「ふきとばし」系のワザ効果をバトルのルール等から判定する
 */
//--------------------------------------------------------------------------
static PushOutEffect check_pushout_effect( BTL_SVFLOW_WORK* wk )
{
  BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk->mainModule );

  switch( rule ){
  case BTL_RULE_SINGLE:
    if( competitor == BTL_COMPETITOR_WILD ){
      return PUSHOUT_EFFECT_ESCAPE;
    }else{
      return PUSHOUT_EFFECT_CHANGE;
    }

  default:
    return PUSHOUT_EFFECT_CHANGE;
  }
}
//--------------------------------------------------------------------------
/**
 * 「ふきとばし」系のワザで強制入れかえが発生した時、
 * 次に出すポケモンのパーティ内インデックスをランダムで決める
 * 出せるポケモンが控えに居ない場合、-1 を返す
 *
 * @param   wk
 * @param   clwk
 *
 * @retval  int  次に出すポケモンのパーティ内Index : 出せるポケが居ない場合 -1
 */
//--------------------------------------------------------------------------
static int get_pushout_nextpoke_idx( BTL_SVFLOW_WORK* wk, const SVCL_WORK* clwk )
{
  const BTL_POKEPARAM* bpp;
  u32 i, startIdx, count, members;
  u8 list[ BTL_PARTY_MEMBER_MAX ];

  members = BTL_PARTY_GetMemberCount( clwk->party );
  startIdx = BTL_MAIN_GetClientBenchPosIndex( wk->mainModule, clwk->myID );

  for(i=startIdx, count=0; i<members; ++i)
  {
    bpp = BTL_PARTY_GetMemberDataConst( clwk->party, i );
    if( BPP_IsFightEnable(bpp) ){
      list[count++] = i;
    }
  }

  if( count )
  {
    i = BTL_CALC_GetRand( count );
    return list[ i ];
  }

  return -1;
}

//--------------------------------------------------------------------------
/**
 * [Event] ふきとばしワザ失敗チェック
 *
 * @param   wk
 * @param   attacker
 * @param   target
 *
 * @retval  BOOL      失敗ならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckPushOutFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target )
{
  BOOL failFlag = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, failFlag );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_PUSHOUT );
    BTL_SICKEVENT_CheckPushOutFail( wk, target );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  return failFlag;
}
//---------------------------------------------------------------------------------------------
// サーバーフロー：フィールドエフェクト効果
//---------------------------------------------------------------------------------------------
static void scput_Fight_FieldEffect( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker )
{
  BtlWeather  weather = WAZADATA_GetWeather( wazaParam->wazaID );
  if( weather != BTL_WEATHER_NONE )
  {
    u8 turn_upcnt = scEvent_WazaWeatherTurnUp( wk, weather, attacker );

    if( scproc_ChangeWeather( wk, weather, BTL_WEATHER_TURN_DEFAULT+turn_upcnt ) ){
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
    }else{
      scPut_WazaFail( wk, attacker, wazaParam->wazaID );
    }
  }
  else
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    BOOL fSucceed;
    scEvent_FieldEffectCall( wk, attacker, wazaParam->wazaID );
    fSucceed = (scproc_HandEx_Result( wk ) == HandExResult_Enable);
    BTL_Hem_PopState( &wk->HEManager, hem_state );

    if( fSucceed ){
      wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
    }else{
      scPut_WazaFail( wk, attacker, wazaParam->wazaID );
    }
  }
}
/**
 *  天候変化処理
 */
static BOOL scproc_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn )
{
  if( scproc_ChangeWeatherCheck(wk, weather, turn) )
  {
    scproc_ChangeWeatherCore( wk, weather, turn );
    return TRUE;
  }
  return FALSE;
}
/**
 *  天候変化可否チェック
 */
static BOOL scproc_ChangeWeatherCheck( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn )
{
  if( weather >= BTL_WEATHER_MAX ){
    return FALSE;
  }

  if( BTL_FIELD_GetWeather() == weather )
  {
    if( (turn != BTL_WEATHER_TURN_PERMANENT)
    ||  (BTL_FIELD_GetWeatherTurn() == BTL_WEATHER_TURN_PERMANENT)
    ){
      return FALSE;
    }
  }

  return TRUE;
}
/**
 *  天候変化コア
 */
static void scproc_ChangeWeatherCore( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8 turn )
{
  BTL_FIELD_SetWeather( weather, turn );
  SCQUE_PUT_ACT_WeatherStart( wk->que, weather );
  scproc_ChangeWeatherAfter( wk, weather );
}
/**
 *  天候変化直後イベント呼び出し
 */
static void scproc_ChangeWeatherAfter( BTL_SVFLOW_WORK* wk, BtlWeather weather )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_AfterChangeWeather( wk, weather );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによる天候変化のターン数増加チェック
 *
 * @param   wk
 * @param   weather
 * @param   wk
 * @param   weather
 * @param   turn
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 scEvent_WazaWeatherTurnUp( BTL_SVFLOW_WORK* wk, BtlWeather weather, const BTL_POKEPARAM* attacker )
{
  u8 cnt;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WEATHER, weather );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_TURN_COUNT, 0 );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_WEATHER_TURNCNT );
    cnt = BTL_EVENTVAR_GetValue( BTL_EVAR_TURN_COUNT );
  BTL_EVENTVAR_Pop();
  return cnt;
}


//--------------------------------------------------------------------------
/**
 * 【Event】天候の変化チェック
 *
 * @param   wk
 * @param   weather 天候
 * @param   turn    [io]継続ターン数
 *
 * @retval  BOOL    変化する場合はTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather, u8* turn )
{
  GF_ASSERT(weather != BTL_WEATHER_NONE);

  {
    BOOL fFail;

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetValue( BTL_EVAR_WEATHER, weather );
      BTL_EVENTVAR_SetValue( BTL_EVAR_TURN_COUNT, *turn );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHANGE );
      weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );
      *turn = BTL_EVENTVAR_GetValue( BTL_EVAR_TURN_COUNT );
      fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    BTL_EVENTVAR_Pop();
    return !fFail;
  }
}
static BOOL scproc_FieldEffectCore( BTL_SVFLOW_WORK* wk, BtlFieldEffect effect, BPP_SICK_CONT contParam, BOOL fAddDependTry )
{
  if( BTL_FIELD_AddEffect(effect, contParam) )
  {
    SCQUE_PUT_OP_AddFieldEffect( wk->que, effect, contParam.raw );
    return TRUE;
  }
  else if( fAddDependTry )
  {
    u8 dependPokeID = BPP_SICKCONT_GetPokeID( contParam );
    if( dependPokeID != BTL_POKEID_NULL )
    {
      BTL_FIELD_AddDependPoke( effect, dependPokeID );
      SCQUE_PUT_OP_AddFieldEffectDepend( wk->que, effect, dependPokeID );
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] フィールドエフェクト追加処理呼び出し
 *
 * @param   wk
 * @param   attacker
 * @param   target
 *
 */
//----------------------------------------------------------------------------------
static void scEvent_FieldEffectCall( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  BOOL failFlag = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_FIELD_EFFECT_CALL );
  BTL_EVENTVAR_Pop();
}

//---------------------------------------------------------------------------------------------
// サーバーフロー：分類しきれないワザ
//---------------------------------------------------------------------------------------------
static void scput_Fight_Uncategory( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam, BTL_POKEPARAM* attacker, BTL_POKESET* targets )
{
  BTL_POKEPARAM* bpp;

  // ターゲットは居たはずだがモロモロの理由で残っていない場合は終了
  if( BTL_POKESET_IsRemovedAll(targets) ){
    return;
  }

  switch( wazaParam->wazaID ){
  case WAZANO_SUKIRUSUWAPPU:
    scput_Fight_Uncategory_SkillSwap( wk, attacker, targets );
    break;
  case WAZANO_MIGAWARI:
    if( scproc_Migawari_Create(wk, attacker) )
    {
      wazaEffCtrl_SetEnableDummy( wk->wazaEffCtrl );
    }
    break;
  default:
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      HandExResult result = HandExResult_NULL;
      BOOL fFailMsgEnable;

      if( scEvent_UnCategoryWaza(wk, wazaParam, attacker, targets, &fFailMsgEnable) )
      {
        result = scproc_HandEx_Result( wk );
        if( result == HandExResult_Enable ){
          wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
        }
      }

      if( (result == HandExResult_NULL) || (result == HandExResult_Fail) )
      {
        if( fFailMsgEnable ){
          SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
        }
      }

      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
    break;
  }
}
// スキルスワップ
static void scput_Fight_Uncategory_SkillSwap( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKESET* targetRec )
{
  BTL_POKEPARAM* target = BTL_POKESET_Get( targetRec, 0 );
  PokeTokusei atk_tok, tgt_tok;

  atk_tok = BPP_GetValue( attacker, BPP_TOKUSEI );
  tgt_tok = BPP_GetValue( target, BPP_TOKUSEI );

  if( (atk_tok != tgt_tok)
  &&  (!BTL_TABLES_CheckSkillSwapFailTokusei(atk_tok))
  &&  (!BTL_TABLES_CheckSkillSwapFailTokusei(tgt_tok))
  ){
    u8 atkPokeID = BPP_GetID( attacker );
    u8 tgtPokeID = BPP_GetID( target );

    wazaEffCtrl_SetEnable( wk->wazaEffCtrl );

    SCQUE_PUT_ACTOP_SwapTokusei( wk->que, atkPokeID, tgtPokeID, tgt_tok, atk_tok );
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_SkillSwap, atkPokeID );

    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_ChangeTokuseiBefore( wk, atkPokeID, atk_tok, tgt_tok );
      scEvent_ChangeTokuseiBefore( wk, tgtPokeID, tgt_tok, atk_tok );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }
    BPP_ChangeTokusei( attacker, tgt_tok );
    BPP_ChangeTokusei( target, atk_tok );
    BTL_HANDLER_TOKUSEI_Swap( attacker, target );


    SCQUE_PUT_TOKWIN_OUT( wk->que, atkPokeID );
    SCQUE_PUT_TOKWIN_OUT( wk->que, tgtPokeID );


    if( atk_tok != tgt_tok )
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_ChangeTokuseiAfter( wk, atkPokeID );
      scEvent_ChangeTokuseiAfter( wk, tgtPokeID );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }

    // ぶきよう・きんちょうかんが入れ替わったらアイテム発動チェック
    if( !BPP_CheckSick(attacker, WAZASICK_IEKI) )
    {
      if( atk_tok == POKETOKUSEI_BUKIYOU ){
        scproc_CheckItemReaction( wk, attacker, BTL_ITEMREACTION_GEN );
      }
      if( atk_tok == POKETOKUSEI_KINCHOUKAN ){
        scproc_KintyoukanMoved( wk, attacker );
      }
    }
    if( !BPP_CheckSick(target, WAZASICK_IEKI) )
    {
      if( tgt_tok == POKETOKUSEI_BUKIYOU ){
        scproc_CheckItemReaction( wk, target, BTL_ITEMREACTION_GEN );
      }
      if( tgt_tok == POKETOKUSEI_KINCHOUKAN ){
        scproc_KintyoukanMoved( wk, target );
      }
    }
  }
  else{
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
  }
}
/**
 *  とくせい「きんちょうかん」を持つポケモンが場から消えた・またはとくせいを失った時の処理
 */
static void scproc_KintyoukanMoved( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bppMoved )
{
  FRONT_POKE_SEEK_WORK  fps;
  BTL_POKEPARAM* bpp;

  u8 movedPokeID = BPP_GetID( bppMoved );
  u8 pokeID;

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext(&fps, wk, &bpp) )
  {
    pokeID = BPP_GetID( bpp );
    if( !BTL_MAINUTIL_IsFriendPokeID(movedPokeID, pokeID) )
    {
      scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_GEN );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * みがわり - 作成
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static BOOL scproc_Migawari_Create( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  if( !BPP_MIGAWARI_IsExist(bpp) )
  {
    int migawariHP = BTL_CALC_QuotMaxHP( bpp, 4 );
    if( BPP_GetValue(bpp, BPP_HP) > migawariHP )
    {
      BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos(&wk->pospokeWork, BPP_GetID(bpp) );
      if( pos != BTL_POS_NULL )
      {
        scPut_SimpleHp( wk, bpp, -migawariHP, TRUE );
        scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_HP );

        BPP_MIGAWARI_Create( bpp, migawariHP );
        SCQUE_PUT_OP_MigawariCreate( wk->que, BPP_GetID(bpp), migawariHP );
        SCQUE_PUT_ACT_MigawariCreate( wk->que, pos );
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_MigawariAppear, BPP_GetID(bpp) );
        return TRUE;
      }
    }
    // 失敗メッセージ
    scPut_Message_StdEx( wk, BTL_STRID_STD_MigawariFail, 0, NULL );
  }
  else{
    // すでに出ていたメッセージ
    scPut_Message_Set( wk, bpp, BTL_STRID_SET_MigawariExist );
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * みがわり - ダメージ処理
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  u16 実際に与えたダメージ値
 */
//----------------------------------------------------------------------------------
static u16 scproc_Migawari_Damage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* target, u16 damage,
  BtlTypeAff aff, u8 fCritical, const SVFL_WAZAPARAM* wazaParam )
{

  SCQUE_PUT_ACT_MigawariDamage( wk->que, BPP_GetID(target), aff, wazaParam->wazaID );
  scPut_Message_Set( wk, target, BTL_STRID_SET_MigawariDamage );
  scPut_WazaAffinityMsg( wk, 1, &aff, &target, FALSE );
  scPut_CriticalMsg( wk, 1, &target, &fCritical, FALSE );

  if( BPP_MIGAWARI_AddDamage(target, &damage) )
  {
    scproc_Migawari_Delete( wk, target );
  }

  scproc_Damage_Drain( wk, wazaParam, attacker, target, damage );
  scproc_WazaAdditionalEffect( wk, wazaParam, attacker, target, damage, TRUE );
  scproc_WazaDamageReaction( wk, attacker, target, wazaParam, aff, damage, fCritical, TRUE );

  return damage;
}
//----------------------------------------------------------------------------------
/**
 * みがわり - 削除処理
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scproc_Migawari_Delete( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  u8 pokeID = BPP_GetID( bpp );
  BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID );

  scPut_Message_Set( wk, bpp, BTL_STRID_SET_MigawariDestract );

  BPP_MIGAWARI_Delete( bpp );
  SCQUE_PUT_OP_MigawariDelete( wk->que, pokeID );
  SCQUE_PUT_ACT_MigawariDelete( wk->que, pos );
}
//----------------------------------------------------------------------------------
/**
 * ワザダメージに対する反応処理
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 * @param   affinity
 * @param   damage
 * @param   critical_flag
 */
//----------------------------------------------------------------------------------
static void scproc_WazaDamageReaction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam, BtlTypeAff affinity, u32 damage, BOOL critical_flag, BOOL fMigawari )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );

  scEvent_WazaDamageReaction( wk, attacker, defender, wazaParam, affinity, damage, critical_flag, fMigawari );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
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
 *
 */
//--------------------------------------------------------------------------
static void scEvent_WazaDamageReaction( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BtlTypeAff aff,
  u32 damage, BOOL fCritical, BOOL fMigawari )
{
  u8 defPokeID = BPP_GetID( defender );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, defPokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ORG_WAZAID, wazaParam->orgWazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TYPEAFF, aff );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, wazaParam->damageType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_CRITICAL_FLAG, fCritical );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_MIGAWARI_FLAG, fMigawari );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_RINPUNGUARD_FLG, FALSE );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_REACTION_PREV );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_REACTION );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_DMG_REACTION_L2 );

  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * [Event] 未分類ワザ処理コール
 *
 * @param   wk
 * @param   waza
 * @param   attacker
 * @param   targets
 *
 * @retval  BOOL      反応したワザハンドラがあればTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_UnCategoryWaza( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  const BTL_POKEPARAM* attacker, BTL_POKESET* targets, BOOL* fFailMsgEnable )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    {
      BTL_POKEPARAM* bpp;
      u8 targetMax, targetCnt, i;

      targetMax = BTL_POKESET_GetCountMax( targets );
      targetCnt = BTL_POKESET_GetCount( targets );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, targetCnt );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, TRUE );


      BTL_N_Printf( DBGSTR_SVFL_UncategoryWazaInfo, BPP_GetID(attacker), targetCnt, targetMax);

      for(i=0; i<targetCnt; ++i){
        bpp = BTL_POKESET_Get( targets, i );
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(bpp) );
      }
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
      if( targetMax )
      {
        if( targetCnt ){
          BTL_EVENT_CallHandlers( wk, BTL_EVENT_UNCATEGORIZE_WAZA );
        }
      }
      else{
        BTL_EVENT_CallHandlers( wk, BTL_EVENT_UNCATEGORIZE_WAZA_NO_TARGET );
      }

      (*fFailMsgEnable) = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );

    }
  BTL_EVENTVAR_Pop();

  return (scproc_HandEx_Result(wk) != HandExResult_NULL);
}


//==============================================================================
// サーバーフロー：ターンチェックルート
//==============================================================================

/**
 *  POKESETに全戦闘ポケを格納 -> 素早さ順ソート
 */
static void StoreFrontPokeOrderAgility( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet )
{
  FRONT_POKE_SEEK_WORK  fps;
  BTL_POKEPARAM* bpp;

  BTL_POKESET_Clear( pokeSet );

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext(&fps, wk, &bpp) ){
    BTL_POKESET_Add( pokeSet, bpp );
  }
  BTL_POKESET_SortByAgility( pokeSet, wk );
}

/**
 *  @retval BOOL  レベルアップして処理途中で抜けたらTRUE
 */
static BOOL scproc_TurnCheck( BTL_SVFLOW_WORK* wk )
{
  // このPOKESETに対象ポケモンを格納する
  BTL_POKESET* pokeSet = wk->psetTarget;
  BOOL fExpGet = FALSE;

  if( wk->turnCheckStep == 0 )
  {
    // POKESET格納->素早さ順ソート
    StoreFrontPokeOrderAgility( wk, pokeSet );
    wk->turnCheckStep = 1;

    scproc_turncheck_CommSupport( wk );
  }

  switch( wk->turnCheckStep ){
  case 1:
    wk->turnCheckStep++;
    SCQUE_PUT_ACT_MsgWinHide( wk->que, 0 );
    if( scproc_turncheck_weather(wk, pokeSet) ){
      fExpGet = TRUE;
      break;
    }
    if( scproc_CheckShowdown(wk) ){ return FALSE; }
    /* fallthru */
  case 2:
    wk->turnCheckStep++;
    if( scproc_turncheck_sub(wk, pokeSet, BTL_EVENT_TURNCHECK_BEGIN) ){
      fExpGet = TRUE;
      break;
    }
    if( scproc_CheckShowdown(wk) ){
      return FALSE;
     }
    /* fallthru */
  case 3:
    wk->turnCheckStep++;
    if( scproc_turncheck_sick(wk, pokeSet) ){
      fExpGet = TRUE;
      break;
    }
    if( scproc_CheckShowdown(wk) ){ return FALSE; }
    /* fallthru */

  case 4:
    wk->turnCheckStep++;
    scproc_turncheck_side( wk );
    scproc_turncheck_field( wk );
    /* fallthru */
  case 5:
    wk->turnCheckStep++;
    if( scproc_turncheck_sub(wk, pokeSet, BTL_EVENT_TURNCHECK_END) ){
      fExpGet = TRUE;
      break;
    }
    if( scproc_CheckShowdown(wk) ){ return FALSE; }
    /* fallthru */
  case 6:
    scproc_turncheck_sub(wk, pokeSet, BTL_EVENT_TURNCHECK_DONE );

    // 全ポケモンのターンフラグをクリア
    wk->turnCheckStep++;
    {
      BTL_POKEPARAM* bpp;
      BTL_POKESET_SeekStart( pokeSet );
      while( (bpp = BTL_POKESET_SeekNext(pokeSet)) != NULL )
      {
        BPP_TurnCheck( bpp );
        BPP_CombiWaza_ClearParam( bpp );
        SCQUE_PUT_OP_TurnCheck( wk->que, BPP_GetID(bpp) );
      }
    }

    // トリプルバトル時、リセットムーブ処理
    scproc_CheckResetMove( wk );

    // 分離イベントファクター削除
    BTL_EVENT_RemoveIsolateFactors();

    if( wk->turnCount < BTL_TURNCOUNT_MAX ){
      wk->turnCount++;
    }
    wk->turnCheckStep = 0;
    wk->simulationCounter = 0;

#ifdef PM_DEBUG
//    OS_SetPrintOutput_Arm9( 1 );
//    TAYA_Printf( " ============= turn END =============\n" );
//    OS_SetPrintOutput_Arm9( 0 );
#endif
    return FALSE;
  }

  return fExpGet;
}
/**
 *  ターンチェック：トリプル時リセットムーブ処理
 */
static void scproc_CheckResetMove( BTL_SVFLOW_WORK* wk )
{
  if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE )
  {
    u8 clientID_1 = BTL_MAIN_GetPlayerClientID( wk->mainModule );
    u8 clientID_2 = BTL_MAIN_GetOpponentClientID( wk->mainModule, clientID_1, 0 );

    BTL_PARTY* party_1 = BTL_POKECON_GetPartyData( wk->pokeCon, clientID_1 );
    BTL_PARTY* party_2 = BTL_POKECON_GetPartyData( wk->pokeCon, clientID_2 );

    if( (BTL_PARTY_GetAliveMemberCount(party_1) == 1)
    &&  (BTL_PARTY_GetAliveMemberCount(party_2) == 1)
    ){
      BTL_POKEPARAM  *bpp1 = BTL_PARTY_GetAliveTopMember( party_1 );
      BTL_POKEPARAM  *bpp2 = BTL_PARTY_GetAliveTopMember( party_2 );

      u8 pokeID_1 = BPP_GetID( bpp1 );
      u8 pokeID_2 = BPP_GetID( bpp2 );

      BtlPokePos  pos1 = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID_1 );
      BtlPokePos  pos2 = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID_2 );

      if( (pos1!=BTL_POS_NULL) && (pos2!=BTL_POS_NULL) )
      {
        u8 posIdx1 = BTL_MAIN_BtlPosToPosIdx( wk->mainModule, pos1 );
        u8 posIdx2 = BTL_MAIN_BtlPosToPosIdx( wk->mainModule, pos2 );

        if( (posIdx1 == posIdx2) && (!BTL_MAINUTIL_IsTripleCenterPos(pos1) ) )
        {
          SCQUE_PUT_ACT_TripleResetMove( wk->que, clientID_1, posIdx1, clientID_2, posIdx2 );
          scproc_MoveCore( wk, clientID_1, posIdx1, 1, FALSE );
          scproc_MoveCore( wk, clientID_2, posIdx2, 1, FALSE );
        }
      }
    }
  }
}

/**
 *  ターンチェック：通信侵入者によるサポート
 */
static void   scproc_turncheck_CommSupport( BTL_SVFLOW_WORK* wk )
{
  COMM_PLAYER_SUPPORT* supportHandle = BTL_MAIN_GetCommSupportHandle( wk->mainModule );
  if( supportHandle )
  {
    SUPPORT_TYPE support_type;

// テスト用に強制ＯＮしたい時に使おう
//    COMM_PLAYER_SUPPORT_SetParam( supportHandle, SUPPORT_TYPE_RECOVER_HALF, BTL_MAIN_GetPlayerStatus(wk->mainModule) );

    support_type = COMM_PLAYER_SUPPORT_GetSupportType( supportHandle );
    if( support_type != SUPPORT_TYPE_NULL && support_type != SUPPORT_TYPE_USED )
    {
      u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      SVCL_WORK* clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
      u8 pokeIDAry[ BTL_POSIDX_MAX ];
      u8 cnt, i;
      BTL_POKEPARAM* bpp;

      // 生きててHPが満タンではないポケのみ列挙
      for(i=0, cnt=0; i<clwk->numCoverPos; ++i)
      {
        bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, i );
        if( IsBppExist(bpp) && !BPP_IsHPFull(bpp) )
        {
          pokeIDAry[ cnt++ ] = BPP_GetID( bpp );
        }
      }
      if( cnt )
      {
        u32 recoverHP;
        u8 pokeID = pokeIDAry[ GFL_STD_MtRand(cnt) ];
        bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
        recoverHP = BPP_GetValue( bpp, BPP_MAX_HP );
        if( support_type == SUPPORT_TYPE_RECOVER_HALF ){
          recoverHP /= 2;
        }
        if( recoverHP )
        {
          if( scproc_RecoverHP(wk, bpp, recoverHP, FALSE) )
          {
            SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_CommSupport, BTL_CLIENTID_COMM_SUPPORT, pokeID );
            COMM_PLAYER_SUPPORT_SetUsed( supportHandle );
          }
        }
      }
    }
  }

}
/**
 *  ターンチェック：汎用
 *
 * @retval  BOOL    誰かが死亡->レベルアップ発生した場合にTRUE
 */
static BOOL scproc_turncheck_sub( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet, BtlEventType event_type )
{
  BTL_POKEPARAM* bpp;
  BTL_POKESET_SeekStart( pokeSet );

  scEvent_TurnCheck( wk, BTL_POKEID_NULL, event_type );

  while( (bpp = BTL_POKESET_SeekNext(pokeSet)) != NULL )
  {
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_TurnCheck( wk, BPP_GetID(bpp), event_type );
      BTL_Hem_PopState( &wk->HEManager, hem_state );

      scproc_CheckDeadCmd( wk, bpp );
      if( scproc_CheckShowdown(wk) ){
        break;
      }
    }
  }

  return scproc_CheckExpGet( wk );
}
static void scEvent_TurnCheck( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlEventType event_type )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, pokeID );
    BTL_EVENT_CallHandlers( wk, event_type );
  BTL_EVENTVAR_Pop();
}

//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > 状態異常
//------------------------------------------------------------------
static BOOL scproc_turncheck_sick( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet )
{
  BTL_POKEPARAM* bpp;
  u32 hem_state;

  BTL_POKESET_SeekStart( pokeSet );
  while( (bpp = BTL_POKESET_SeekNext(pokeSet)) != NULL )
  {
    hem_state = BTL_Hem_PushState( &wk->HEManager );

    BPP_WazaSick_TurnCheck( bpp, BTL_SICK_TurnCheckCallback, wk );
    SCQUE_PUT_OP_WazaSickTurnCheck( wk->que, BPP_GetID(bpp) );

//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }

  return scproc_CheckExpGet( wk );
}
//--------------------------------------------------------------------------------
/**
 * ターンチェックで状態異常によるダメージが発生するたびにコールバックされる
 *
 * @param   wk
 * @param   bpp
 * @param   sickID
 * @param   damage
 */
//--------------------------------------------------------------------------------
void BTL_SVF_SickDamageRecall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sickID, u32 damage )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  BOOL fEnable = FALSE;
  damage = scEvent_SickDamage( wk, bpp, sickID, damage );
  if( damage )
  {
    if( scproc_SimpleDamage_CheckEnable(wk, bpp, damage) )
    {
      fEnable = TRUE;
      switch( sickID ){
      case WAZASICK_DOKU:
        scPut_EffectByPokePos( wk, bpp, BTLEFF_DOKU );
        break;
      case WAZASICK_YAKEDO:
        scPut_EffectByPokePos( wk, bpp, BTLEFF_YAKEDO );
        break;
      case WAZASICK_NOROI:
        scPut_EffectByPokePos( wk, bpp, BTLEFF_NOROI );
        break;
      case WAZASICK_AKUMU:
        scPut_EffectByPokePos( wk, bpp, BTLEFF_AKUMU );
        break;
      }
      HANDEX_STR_Clear( &wk->strParam );
      BTL_SICK_MakeSickDamageMsg( &wk->strParam, bpp, sickID );
      scproc_SimpleDamage_Core( wk, bpp, damage, &wk->strParam );
    }
  }

  if(!fEnable){
//    scproc_HandEx_Root( wk, ITEM_DUMMY_DATA );
  }

  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] 状態異常によるターンチェックダメージ
 *
 * @param   wk
 * @param   bpp
 * @param   sickID
 * @param   damage
 */
//----------------------------------------------------------------------------------
static u32 scEvent_SickDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sickID, u32 damage )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SICKID, sickID );
    BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, damage );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_SICK_DAMAGE );
    damage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
  BTL_EVENTVAR_Pop();
  return damage;
}

//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > サイドエフェクト
//------------------------------------------------------------------
static void scproc_turncheck_side( BTL_SVFLOW_WORK* wk )
{
  BTL_HANDLER_SIDE_TurnCheck( scproc_turncheck_side_callback, wk );
}
static void scproc_turncheck_side_callback( BtlSide side, BtlSideEffect sideEffect, void* arg )
{
  BTL_SVFLOW_WORK* wk = arg;
  scPut_SideEffectOffMsg( wk, sideEffect, side );
}
static void scPut_SideEffectOffMsg( BTL_SVFLOW_WORK* wk, BtlSideEffect sideEffect, BtlSide side )
{
  int strID = -1;

  switch( sideEffect ){
  case BTL_SIDEEFF_REFRECTOR:       strID = BTL_STRID_STD_ReflectorOff; break;
  case BTL_SIDEEFF_HIKARINOKABE:    strID = BTL_STRID_STD_HikariNoKabeOff; break;
  case BTL_SIDEEFF_SINPINOMAMORI:   strID = BTL_STRID_STD_SinpiNoMamoriOff; break;
  case BTL_SIDEEFF_SIROIKIRI:       strID = BTL_STRID_STD_SiroiKiriOff; break;
  case BTL_SIDEEFF_OIKAZE:          strID = BTL_STRID_STD_OikazeOff; break;
  case BTL_SIDEEFF_OMAJINAI:        strID = BTL_STRID_STD_OmajinaiOff; break;
  case BTL_SIDEEFF_MAKIBISI:        strID = BTL_STRID_STD_MakibisiOff; break;
  case BTL_SIDEEFF_DOKUBISI:        strID = BTL_STRID_STD_DokubisiOff; break;
  case BTL_SIDEEFF_STEALTHROCK:     strID = BTL_STRID_STD_StealthRockOff; break;
  case BTL_SIDEEFF_RAINBOW:         strID = BTL_STRID_STD_RainbowOff; break;
  case BTL_SIDEEFF_BURNING:         strID = BTL_STRID_STD_BurningOff; break;
  case BTL_SIDEEFF_MOOR:            strID = BTL_STRID_STD_MoorOff; break;
  default:
    break;
  }

  if( strID >= 0 )
  {
    SCQUE_PUT_MSG_STD( wk->que, strID, side );
  }
}

//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > フィールドエフェクト
//------------------------------------------------------------------
static void scproc_turncheck_field( BTL_SVFLOW_WORK* wk )
{
  BTL_FIELD_TurnCheck( turncheck_field_callback, wk );
}
static void turncheck_field_callback( BtlFieldEffect effect, void* arg )
{
  BTL_SVFLOW_WORK* wk = arg;

  scproc_FieldEff_End( wk, effect );
}

static void scproc_FieldEff_End( BTL_SVFLOW_WORK* wk, BtlFieldEffect effect )
{
  int strID = -1;
  switch( effect ){
  case BTL_FLDEFF_TRICKROOM:   strID = BTL_STRID_STD_TrickRoomOff; break; ///< トリックルーム
  case BTL_FLDEFF_JURYOKU:     strID = BTL_STRID_STD_JyuryokuOff; break;  ///< じゅうりょく
  case BTL_FLDEFF_WONDERROOM:  strID = BTL_STRID_STD_WonderRoom_End; break;
  case BTL_FLDEFF_MAGICROOM:   strID = BTL_STRID_STD_MagicRoom_End; break;
  }

  if( strID >= 0 )
  {
    SCQUE_PUT_MSG_STD( wk->que, strID );
  }
  SCQUE_PUT_OP_RemoveFieldEffect( wk->que, effect );

  // マジックルームが切れたらアイテム反応チェック
  if( effect == BTL_FLDEFF_MAGICROOM )
  {
    BTL_POKEPARAM* bpp;

    StoreFrontPokeOrderAgility( wk, wk->psetTmp );
    BTL_POKESET_SeekStart( wk->psetTmp );
    while( (bpp = BTL_POKESET_SeekNext(wk->psetTmp)) != NULL )
    {
      if( BPP_IsFightEnable(bpp) )
      {
        scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_GEN );
      }
    }
  }
}

//------------------------------------------------------------------
// サーバーフロー下請け： ターンチェック > 天候
//------------------------------------------------------------------
static BOOL scproc_turncheck_weather( BTL_SVFLOW_WORK* wk, BTL_POKESET* pokeSet )
{
  BtlWeather weather = BTL_FIELD_TurnCheckWeather();
  // ターンチェックにより天候が終わった
  if( weather != BTL_WEATHER_NONE )
  {
    SCQUE_PUT_ACT_WeatherEnd( wk->que, weather );
    scproc_ChangeWeatherAfter( wk, BTL_WEATHER_NONE );
    return FALSE;
  }
  // 天候が継続
  else
  {
    BTL_POKEPARAM* bpp;

    weather = scEvent_GetWeather( wk );
    BTL_POKESET_SeekStart( pokeSet );
    while( (bpp = BTL_POKESET_SeekNext(pokeSet)) != NULL )
    {
      if( (!BPP_IsDead(bpp))
      &&  (!BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_ANAWOHORU))
      &&  (!BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_DIVING))
      ){
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        int  damage = BTL_CALC_RecvWeatherDamage( bpp, weather );
        damage = scEvent_CheckWeatherReaction( wk, bpp, weather, damage );
        if( !scproc_HandEx_Result(wk) )
        {
          if( damage ){
            scPut_WeatherDamage( wk, bpp, weather, damage );
          }
        }
        BTL_Hem_PopState( &wk->HEManager, hem_state );
        scproc_CheckDeadCmd( wk, bpp );
      }
    }
    return scproc_CheckExpGet( wk );
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] 天候リアクションチェック
 *
 * @param   wk
 * @param   weather
 */
//----------------------------------------------------------------------------------
static int scEvent_CheckWeatherReaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BtlWeather weather, u32 damage )
{
  u8  failFlag;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WEATHER, weather );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE,  damage );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_REACTION );
    damage = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  if( failFlag ){
    return 0;
  }
  return damage;
}
//----------------------------------------------------------------------------------
/**
 * [Put]天候によるダメージ処理
 *
 * @param   wk
 * @param   bpp
 * @param   weather
 * @param   damage
 */
//----------------------------------------------------------------------------------
static void scPut_WeatherDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BtlWeather weather, int damage )
{
  u8 pokeID = BPP_GetID( bpp );

  switch( weather ){
  case BTL_WEATHER_SAND:
    HANDEX_STR_Setup( &wk->strParam, BTL_STRTYPE_SET, BTL_STRID_SET_WeatherDmg_Sand );
    HANDEX_STR_AddArg( &wk->strParam, pokeID );
    break;
  case BTL_WEATHER_SNOW:
    HANDEX_STR_Setup( &wk->strParam, BTL_STRTYPE_SET, BTL_STRID_SET_WeatherDmg_Snow );
    HANDEX_STR_AddArg( &wk->strParam, pokeID );
    break;
  default:
    HANDEX_STR_Clear( &wk->strParam );
    break;
  }

  BTL_Printf("ダメージ値=%d\n", damage);
  if( damage > 0 )
  {
    if( scproc_SimpleDamage_CheckEnable(wk, bpp, damage) )
    {
      handexSub_putString( wk, &wk->strParam );
      scproc_SimpleDamage_Core( wk, bpp, damage, NULL );
    }
  }
}


//----------------------------------------------------------------------------------
/**
 * 指定ポケモンを含むチームが全てひん死処理に入ったかチェック（BGM再生判断）
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL checkPokeDeadFlagAllOn( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
  BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
  u32 members = BTL_PARTY_GetMemberCount( party );
  u32 i;
  for(i=0; i<members; ++i)
  {
    pokeID = BPP_GetID( BTL_PARTY_GetMemberData(party, i) );
    if( wk->pokeDeadFlag[ pokeID ] == 0 ){
      return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------------
/**
 * 対象ポケモンが死んでいたら死亡処理＆必要コマンド生成
 */
//--------------------------------------------------------------------------
static BOOL scproc_CheckDeadCmd( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke )
{
  u8 pokeID = BPP_GetID( poke );

  if( wk->pokeDeadFlag[pokeID] == 0 )
  {
    if( BPP_IsDead(poke) )
    {
      BTL_N_Printf( DBGSTR_SVFL_PokeDead, pokeID);
      wk->pokeDeadFlag[pokeID] = 1;
      BTL_DEADREC_Add( &wk->deadRec, pokeID );

      // ラストのシン・ムは「○○は　たおれた」メッセージを表示しない
      // 複数回攻撃を受けている最中のポケモンも同様
      if( ((BTL_MAIN_GetSetupStatusFlag(wk->mainModule, BTL_STATUS_FLAG_LEGEND_EX) == FALSE) ||
          (BTL_MAINUTIL_PokeIDtoClientID(pokeID) == BTL_CLIENT_PLAYER))
      &&  (wk->thruDeadMsgPokeID != pokeID)
      ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Dead, pokeID );
      }

      SCQUE_PUT_ACT_Dead( wk->que, pokeID );
      scproc_ClearPokeDependEffect( wk, poke );
      BPP_Clear_ForDead( poke );

      // 野生戦でプレイヤー勝利ならこの時点でBGM再生コマンド
      if( (BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_WILD)
      &&  (wk->fWinBGMPlayWild == FALSE)
      &&  scproc_CheckShowdown(wk)
      &&  checkPokeDeadFlagAllOn(wk, pokeID)
      &&  CheckPlayerSideAlive(wk)
      ){
        u16 WinBGM = BTL_MAIN_GetWinBGMNo( wk->mainModule );
        SCQUE_PUT_ACT_PlayWinBGM( wk->que, WinBGM );
        wk->fWinBGMPlayWild = TRUE;
      }

      // プレイヤーのポケモンが死んだ時になつき度計算
      if( BTL_MAINUTIL_PokeIDtoClientID(pokeID) == BTL_MAIN_GetPlayerClientID(wk->mainModule) )
      {
        int deadPokeLv = BPP_GetValue( poke, BPP_LEVEL );
        int enemyMaxLv = checkExistEnemyMaxLevel( wk );
        BOOL fLargeDiffLevel = ( (deadPokeLv + 30) <= enemyMaxLv );

        BTL_N_Printf( DBGSTR_SVFL_DeadDiffLevelCheck, deadPokeLv, enemyMaxLv);
        BTL_MAIN_ReflectNatsukiDead( wk->mainModule, poke, fLargeDiffLevel );
      }

      // 経験値取得 -> 退場の順にしないと経験値計算でおかしくなります
//      scproc_GetExp( wk, poke );
      BTL_POSPOKE_PokeOut( &wk->pospokeWork, pokeID );

      {
        u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
        u8 playerClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
        if( clientID == playerClientID )
        {
          BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_TEMOTI_KIZETU );
        }
        else if( !BTL_MAINUTIL_IsFriendClientID(clientID, playerClientID) )
        {
          BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_KILL_POKE );
          BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_DAYCNT_KILL );
        }
      }

      return TRUE;
    }
  }
  else
  {
    BTL_N_Printf( DBGSTR_SVFL_DeadAlready, pokeID);
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 相手サイドの場に出ているポケモンの内、最大レベルを取得
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static u32 checkExistEnemyMaxLevel( BTL_SVFLOW_WORK* wk )
{
  u32 maxLv = 1;
  u32 posEnd = BTL_MAIN_GetEnablePosEnd( wk->mainModule );
  u8  playerClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
  u8  clientID;
  u32 pos;

  for(pos=0; pos<=posEnd; ++pos)
  {
    clientID = BTL_MAIN_BtlPosToClientID( wk->mainModule, pos );

    if( !BTL_MAINUTIL_IsFriendClientID(playerClientID, clientID) )
    {
      u8 pokeID = BTL_POSPOKE_GetExistPokeID( &wk->pospokeWork, pos );
      if( pokeID != BTL_POKEID_NULL )
      {
        const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
        u32 lv = BPP_GetValue( bpp, BPP_LEVEL );
        if( lv > maxLv ){
          maxLv = lv;
        }
      }
    }
  }

  return maxLv;
}

//--------------------------------------------------------------------------
/**
 * 特定ポケモン依存の状態異常・サイドエフェクト等、各種ハンドラをクリアする
 */
//--------------------------------------------------------------------------
static void scproc_ClearPokeDependEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* poke )
{
  FRONT_POKE_SEEK_WORK fps;
  BTL_POKEPARAM* bpp;
  u8 dead_pokeID = BPP_GetID( poke );

  scEvent_BeforeDead( wk, poke );

  scproc_FreeFall_CheckRelease( wk, poke, FALSE );

  BTL_HANDLER_TOKUSEI_Remove( poke );
  BTL_HANDLER_ITEM_Remove( poke );
  BTL_HANDLER_Waza_RemoveForceAll( poke );

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  FRONT_POKE_SEEK_AddRotationBack( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
  {
    BPP_CureWazaSickDependPoke( bpp, dead_pokeID );
    SCQUE_PUT_OP_CureSickDependPoke( wk->que, BPP_GetID(bpp), dead_pokeID );
  }
  BTL_FIELD_RemoveDependPokeEffect( dead_pokeID );
  SCQUE_PUT_OP_DeleteDependPokeFieldEffect( wk->que, dead_pokeID );

  if( !scproc_CheckShowdown(wk) )
  {
    u16 tokusei = BPP_GetValue( poke, BPP_TOKUSEI_EFFECTIVE );
    if( tokusei == POKETOKUSEI_KINCHOUKAN )
    {
      scproc_KintyoukanMoved( wk, poke );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] 死亡直前通知
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scEvent_BeforeDead( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_NOTIFY_DEAD );
  BTL_EVENTVAR_Pop();
}


//----------------------------------------------------------------------------------
/**
 * 死亡ポケレコードを見て必要なら経験値取得の処理を行う
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static BOOL scproc_CheckExpGet( BTL_SVFLOW_WORK* wk )
{
  BOOL result = FALSE;
  if( BTL_MAIN_IsExpSeqEnable(wk->mainModule) )
  {
    u32 i, deadPokeCnt = BTL_DEADREC_GetCount( &wk->deadRec, 0 );

    for(i=0; i<deadPokeCnt; ++i)
    {
      if( !BTL_DEADREC_GetExpCheckedFlag(&wk->deadRec, 0, i) )
      {
        u8 pokeID = BTL_DEADREC_GetPokeID( &wk->deadRec, 0, i );
        const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );

        if( scproc_GetExp(wk, bpp) ){
          result = TRUE;
        }

        BTL_DEADREC_SetExpCheckedFlag( &wk->deadRec, 0, i );
      }
    }
  }
  return result;
}
//----------------------------------------------------------------------------------
/**
 * 経験値取得処理コマンド生成
 *
 * @param   wk
 * @param   deadPoke
 *
 * @retval  BOOL    経験値取得処理によりレベルアップしたら（取得条件を満たしていたら）TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_GetExp( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* deadPoke )
{
  u8 deadPokeID = BPP_GetID( deadPoke );
  if( BTL_MAINUTIL_PokeIDtoSide(deadPokeID) == BTL_SIDE_2ND )
  {
    BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, BTL_MAIN_GetPlayerClientID(wk->mainModule) );

    BTL_SVFSUB_CalcExp( wk, party, deadPoke, wk->calcExpWork );
    return getexp_make_cmd( wk, party, wk->calcExpWork );
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 経験値計算結果を元にコマンド生成
 *
 * @param   wk
 * @param   party     プレイヤー側パーティ
 * @param   calcExp   経験値計算結果ワーク
 *
 * @retval BOOL  レベルアップしたらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL getexp_make_cmd( BTL_SVFLOW_WORK* wk, BTL_PARTY* party, const CALC_EXP_WORK* calcExp )
{
  BTL_POKEPARAM* bpp;
  u32 i;
  BOOL result = FALSE;

  for(i=0; i<BTL_PARTY_MEMBER_MAX; ++i)
  {
    if( calcExp[i].exp )
    {
      BTL_POKEPARAM* bpp;


      bpp = BTL_PARTY_GetMemberData( party, i );
      if( BPP_GetValue(bpp, BPP_LEVEL) < PTL_LEVEL_MAX )
      {
        u32 exp   = calcExp[i].exp;
        u16 strID = (calcExp[i].fBonus)? BTL_STRID_STD_GetExp_Bonus : BTL_STRID_STD_GetExp;
        u8  pokeID = BPP_GetID( bpp );

//        BTL_Printf("経験値はいったメッセージ :strID=%d, pokeID=%d, exp=%d\n", strID, pokeID, exp);
        SCQUE_PUT_MSG_STD( wk->que, strID, pokeID, exp );

        {
          u32 restExp = exp;
          while(1){
            if( !BPP_AddExp(bpp, &restExp, &wk->lvupInfo) ){
              break;
            }
          }
        }
        SCQUE_PUT_ACT_AddExp( wk->que, pokeID, exp );

        BTL_MAIN_RECORDDATA_Add( wk->mainModule, RECID_DAYCNT_EXP, exp );

        result = TRUE;

      }
    }
  }

  return result;
}

/**
 *  そらをとぶ状態をキャンセル
 */
static BOOL scproc_TameHideCancel( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag hideContFlag )
{
  if( BPP_CONTFLAG_Get(bpp, hideContFlag) )
  {
    u8 pokeID = BPP_GetID( bpp );

    scPut_ResetContFlag( wk, bpp, hideContFlag );
    if( BPP_CheckSick(bpp, WAZASICK_TAMELOCK) ) {
      scPut_CureSick( wk, bpp, WAZASICK_TAMELOCK, NULL );
    }
    SCQUE_PUT_ACT_TameWazaHide( wk->que, pokeID, FALSE );

    ActOrder_ForceDone( wk, pokeID );
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------------------------
//  数値関連関数群
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
static inline BOOL perOccur( BTL_SVFLOW_WORK* wk, u8 per )
{
  return BTL_CALC_GetRand( 100 ) < per;
}
//---------------------------------------------------------------------------------------------
//  イベントワーク確保
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
      GF_ASSERT(0); // 最後に確保されたワーク以外の領域が先に解放されようとしている
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
 * [Put] 指定ポケモン位置にエフェクト発動
 */
//--------------------------------------------------------------------------
static void scPut_EffectByPokePos( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 effectNo )
{
  u8 pokeID = BPP_GetID( bpp );
  BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID );
  if( pos != BTL_POS_NULL )
  {
    SCQUE_PUT_ACT_EffectByPos( wk->que, pos, effectNo );
  }
}

//--------------------------------------------------------------------------
/**
 * [Put] 「○○は　はんどうでうごけない」表示
 */
//--------------------------------------------------------------------------
static void scPut_CantAction( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  scPut_Message_Set( wk, bpp, BTL_STRID_SET_CantAction );
}
//--------------------------------------------------------------------------
/**
 * 「○○はこんらんしている」表示
 */
//--------------------------------------------------------------------------
static void scPut_ConfCheck( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  scPut_EffectByPokePos( wk, bpp, BTLEFF_KONRAN );
  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KonranAct, BPP_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * 「○○は△△にメロメロだ」表示
 */
//--------------------------------------------------------------------------
static void scPut_MeromeroAct( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  u8 target_pokeID = BPP_GetSickParam( bpp, WAZASICK_MEROMERO );
  scPut_EffectByPokePos( wk, bpp, BTLEFF_MEROMERO );
  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_MeromeroAct, BPP_GetID(bpp), target_pokeID );
}
//--------------------------------------------------------------------------
/**
 * こんらんによる自爆ダメージ処理
 */
//--------------------------------------------------------------------------
static void scPut_ConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 damage )
{
  u8 pokeID = BPP_GetID( bpp );

  // こんらんはマジックガードの影響を受けないのでSimpleHPを直接呼んでいる  taya
  SCQUE_PUT_ACT_ConfDamage( wk->que, pokeID );
  scPut_SimpleHp( wk, bpp, (int)(-damage), TRUE );
  SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_KonranExe, pokeID );

}
//--------------------------------------------------------------------------
/**
 * ワザ出し判定時のポケモン系状態異常回復処理
 */
//--------------------------------------------------------------------------
static void scPut_CurePokeSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sick, BOOL fStdMsg )
{
  u8 pokeID = BPP_GetID( bpp );

  BPP_CurePokeSick( bpp );
  SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );
  SCQUE_PUT_ACT_SickIcon( wk->que, pokeID, POKESICK_NULL );

  if( fStdMsg )
  {
    switch( sick ){
    case POKESICK_NEMURI:
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NemuriWake, pokeID );
      break;
    case POKESICK_KOORI:
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriMelt, pokeID );
      break;
    default:
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_PokeSick_Cure, pokeID );
      break;
    }
  }
}
//--------------------------------------------------------------------------
/**
 * ワザメッセージ表示（○○の××こうげき！など）
 */
//--------------------------------------------------------------------------
static void scPut_WazaMsg( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  SCQUE_PUT_MSG_WAZA( wk->que, BPP_GetID(attacker), waza );
}
//----------------------------------------------------------------------------------
/**
 * [Put]状態異常回復処理のコマンド生成
 *
 * @param   wk
 * @param   bpp       対象ポケモン
 * @param   sick_id   状態異常コード
 * @param   oldCont   [out] 回復前の継続パラメータ（不要ならNULL)
 */
//----------------------------------------------------------------------------------
static void scPut_CureSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, WazaSick sick, BPP_SICK_CONT* oldCont )
{
  if( sick != WAZASICK_NULL )
  {
    u8 pokeID = BPP_GetID( bpp );

    if( oldCont ){
      *oldCont = BPP_GetSickCont( bpp, sick );
    }

    if( sick < POKESICK_MAX )
    {
      BPP_CurePokeSick( bpp );
      SCQUE_PUT_OP_CurePokeSick( wk->que, pokeID );
      if ( BTL_POSPOKE_GetPokeExistPos(&wk->pospokeWork, pokeID) != BTL_POS_NULL ){
        SCQUE_PUT_ACT_SickIcon( wk->que, pokeID, POKESICK_NULL );
      }
    }
    else
    {
      BPP_CureWazaSick( bpp, sick );
      SCQUE_PUT_OP_CureWazaSick( wk->que, pokeID, sick );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * 拡張状態異常コードを、該当するコードに変換する
 *
 * @param   bpp
 * @param   exCode    [io]
 *
 * @retval  WazaSick    該当コード／該当コードなしの場合、WAZASICK_NULLを返す
 */
//----------------------------------------------------------------------------------
static WazaSick trans_sick_code( const BTL_POKEPARAM* bpp, BtlWazaSickEx* exCode )
{
  WazaSick result = WAZASICK_NULL;
  PokeSick poke_sick = BPP_GetPokeSick( bpp );

  switch( (*exCode) ){
  case WAZASICK_EX_POKEFULL:
    // ポケモン系状態異常になっているならそれを治す
    if( poke_sick != WAZASICK_NULL ){
      result = poke_sick;
    }
    *exCode = WAZASICK_NULL;
    break;

  case WAZASICK_EX_POKEFULL_PLUS:
    // ポケモン系状態異常になっているならそれを治す
    if( poke_sick != WAZASICK_NULL ){
      result = poke_sick;
      *exCode = WAZASICK_KONRAN;
      break;
    }
    if( BPP_CheckSick(bpp, WAZASICK_KONRAN) ){
      result = WAZASICK_KONRAN;
    }
    *exCode = WAZASICK_NULL;
    break;

  case WAZASICK_EX_MENTAL:  // メンタルハーブ
    {
      result = BTL_CALC_CheckMentalSick( bpp );
      if( result == WAZASICK_NULL ){
        *exCode = WAZASICK_NULL;
      }
      return result;
    }
    break;

  case WAZASICK_NULL:
    break;

  default:
    if( BPP_CheckSick(bpp, *exCode) ){
      result = (*exCode);
    }
    *exCode = WAZASICK_NULL;
    break;
  }

  return result;
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
  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_WazaAvoid, BPP_GetID(defender) );
}
//--------------------------------------------------------------------------
/**
 * 「○○にはこうかがないようだ」表示
 */
//--------------------------------------------------------------------------
static void scput_WazaNoEffect( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender )
{
  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, BPP_GetID(defender) );
}
//--------------------------------------------------------------------------
/**
 * 「○○には  ぜんぜんきいてない」表示
 */
//--------------------------------------------------------------------------
static void scput_WazaNoEffectIchigeki( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* defender )
{
  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NotEffect_Ichigeki, BPP_GetID(defender) );
}

//--------------------------------------------------------------------------
/**
 * [Put] ワザによるダメージコマンド生成
 */
//--------------------------------------------------------------------------
static void scPut_WazaDamagePlural( BTL_SVFLOW_WORK* wk, const SVFL_WAZAPARAM* wazaParam,
  u32 poke_cnt, const BtlTypeAff* aff, BTL_POKEPARAM** bpp, const u16* damage, const u8* critical_flag, BOOL fTargetPlural )
{
  BtlTypeAffAbout  affAbout;
  u8               affGoodCnt, affBadCnt;
  u32 i;

  affGoodCnt = affBadCnt = 0;

  // HP減少コマンド生成＆効果音用の相性判定
  for(i=0; i<poke_cnt; ++i)
  {
    scPut_DecreaseHP( wk, bpp[i], damage[i] );
    if( aff[i] > BTL_TYPEAFF_100 ){
      ++affGoodCnt;
    }
    if( aff[i] < BTL_TYPEAFF_100 ){
      ++affBadCnt;
    }
  }

  // ダメージエフェクトコマンド生成
  if( affGoodCnt ){
    affAbout = BTL_TYPEAFF_ABOUT_ADVANTAGE;
  }else{
    affAbout = (affBadCnt)? BTL_TYPEAFF_ABOUT_DISADVANTAGE : BTL_TYPEAFF_ABOUT_NORMAL;
  }
  SCQUE_PUT_ACT_WazaDamagePlural( wk->que, poke_cnt, affAbout, wazaParam->wazaID );
  BTL_N_Printf( DBGSTR_SVFL_WazaDmgCmd, wazaParam->wazaID, poke_cnt );
  for(i=0; i<poke_cnt; ++i)
  {
    SCQUE_PUT_ArgOnly( wk->que, BPP_GetID(bpp[i]) );
    BTL_N_PrintfSimple( DBGSTR_csv, BPP_GetID(bpp[i]) );
  }
  BTL_N_PrintfSimple( DBGSTR_LF );
}
//--------------------------------------------------------------------------
/**
 * [Put] 相性メッセージ生成
 */
//--------------------------------------------------------------------------
static void scPut_WazaAffinityMsg( BTL_SVFLOW_WORK* wk, u32 poke_cnt, const BtlTypeAff* aff, BTL_POKEPARAM** bpp,  BOOL fTargetPlural )
{
  u8               affGoodCnt, affBadCnt;
  u32 i;

  affGoodCnt = affBadCnt = 0;

  // 効果音用の相性判定
  for(i=0; i<poke_cnt; ++i)
  {
    if( aff[i] > BTL_TYPEAFF_100 ){ ++affGoodCnt; }
    if( aff[i] < BTL_TYPEAFF_100 ){ ++affBadCnt; }
  }

  // 相性メッセージコマンド生成
  if( fTargetPlural )
  {
    u8 pokeID[ BTL_POSIDX_MAX ];
    u8 c;

    if( affGoodCnt )
    {
      for(i=0, c=0; i<poke_cnt; ++i)
      {
        if( aff[i] > BTL_TYPEAFF_100 ){
          pokeID[c++] = BPP_GetID( bpp[i] );
        }
      }
      switch( affGoodCnt ){
      case 1: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffGood_1, pokeID[0] ); break;
      case 2: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffGood_2, pokeID[0], pokeID[1] ); break;
      case 3: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffGood_3, pokeID[0], pokeID[1], pokeID[2] ); break;
      }
    }
    if( affBadCnt )
    {
      for(i=0, c=0; i<poke_cnt; ++i)
      {
        if( aff[i] < BTL_TYPEAFF_100 ){
          pokeID[c++] = BPP_GetID( bpp[i] );
        }
      }
      switch( affBadCnt ){
      case 1: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffBad_1, pokeID[0] ); break;
      case 2: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffBad_2, pokeID[0], pokeID[1] ); break;
      case 3: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_AffBad_3, pokeID[0], pokeID[1], pokeID[2] ); break;
      }
    }
  }
  else
  {
    if( affGoodCnt ){
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_AffGood );
    }else if( affBadCnt ){
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_AffBad );
    }
  }
}
//--------------------------------------------------------------------------
/**
 * [Put] クリティカルメッセージコマンド生成
 */
//--------------------------------------------------------------------------
static void scPut_CriticalMsg( BTL_SVFLOW_WORK* wk, u32 poke_cnt, BTL_POKEPARAM** bpp, const u8* critical_flg, BOOL fTargetPlural )
{
  u32 i;

  for(i=0; i<poke_cnt; ++i)
  {
    if( critical_flg[i] )
    {
      if( fTargetPlural ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_CriticalHit, BPP_GetID(bpp[i]) );
      }else{
        SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_CriticalHit );
      }
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * [Put] ワザダメージを「こらえた」時のメッセージ表示
 *
 * @param   wk
 * @param   bpp
 * @param   cause
 */
//----------------------------------------------------------------------------------
static void scPut_Koraeru( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BppKoraeruCause cause )
{
  u8 pokeID = BPP_GetID( bpp );

  switch( cause ){
  case BPP_KORAE_WAZA_DEFENDER:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Koraeru_Exe, pokeID );
    break;
  case BPP_KORAE_ITEM:
    {
      u16 itemID = BPP_GetItem( bpp );
      if( itemID ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoraeItem, pokeID, itemID );
      }
    }
  }
}
//--------------------------------------------------------------------------
/**
 * [Put] とくせいウィンドウ表示開始
 */
//--------------------------------------------------------------------------
static void scPut_TokWin_In( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  SCQUE_PUT_TOKWIN_IN( wk->que, BPP_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * [Put] とくせいウィンドウ消去開始
 */
//--------------------------------------------------------------------------
static void scPut_TokWin_Out( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  SCQUE_PUT_TOKWIN_OUT( wk->que, BPP_GetID(bpp) );
}
//--------------------------------------------------------------------------
/**
 * [Put] これ以上、能力ランクがあがらない（さがらない）メッセージ
 */
//--------------------------------------------------------------------------
static void scPut_RankEffectLimit( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, WazaRankEffect effect, int volume )
{
  u8 pokeID = BPP_GetID( target );

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
static void scPut_RankEffect( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaRankEffect effect, int volume, u16 itemID, BOOL fStdMsg )
{
  u8 pokeID = BPP_GetID( target );

  if( volume > 0 )
  {
    volume = BPP_RankUp( target, effect, volume );
    SCQUE_PUT_OP_RankUp( wk->que, pokeID, effect, volume );
    SCQUE_PUT_ACT_RankUp( wk->que, pokeID, effect, volume );
    if( fStdMsg )
    {
      if( itemID == ITEM_DUMMY_DATA ){
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankup_ATK, pokeID, effect, volume );
      }else{
        SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_UseItem_Rankup_ATK, pokeID, itemID, effect, volume );
      }
    }
  }
  else
  {
    volume *= -1;
    volume = BPP_RankDown( target, effect, volume );
    SCQUE_PUT_OP_RankDown( wk->que, pokeID, effect, volume );
    SCQUE_PUT_ACT_RankDown( wk->que, pokeID, effect, volume );
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_Rankdown_ATK, pokeID, effect, volume );
  }
}
//--------------------------------------------------------------------------
/**
 * [Put] シンプルHP増減エフェクト
 */
//--------------------------------------------------------------------------
static void scPut_SimpleHp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, int value, BOOL fEffectEnable )
{
  u8 pokeID = BPP_GetID( bpp );

  if( value > 0 )
  {
    BPP_HpPlus( bpp, value );
    SCQUE_PUT_OP_HpPlus( wk->que, pokeID, value );
  }
  else if( value < 0 )
  {
    value *= -1;
    scPut_DecreaseHP( wk, bpp, value );
  }

  // 場にいないポケも回復させることがあるため
  if( fEffectEnable
  &&  (BTL_POSPOKE_GetPokeExistPos(&wk->pospokeWork, pokeID) != BTL_POS_NULL)
  ){
    SCQUE_PUT_ACT_SimpleHP( wk->que, pokeID );
  }
  else{
    BTL_Printf("エフェクト有効フラグ=%d, エフェクトを生成しません\n", fEffectEnable);
  }
}
//--------------------------------------------------------------------------
/**
 * [Put] ポケモン強制ひん死
 */
//--------------------------------------------------------------------------
static void scPut_KillPokemon( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 effType )
{
  u16 minusHP = BPP_GetValue( bpp, BPP_HP );
  u8 pokeID = BPP_GetID( bpp );

  BPP_HpZero( bpp );
  SCQUE_PUT_OP_HpZero( wk->que, pokeID );
  SCQUE_PUT_ACT_Kill( wk->que, pokeID, effType );
}
//--------------------------------------------------------------------------
/**
 * [Put] 状態異常化
 */
//--------------------------------------------------------------------------
static void scPut_AddSick( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* target, WazaSick sick, BPP_SICK_CONT sickCont )
{
  u8 pokeID = BPP_GetID( target );

  BPP_SetWazaSick( target, sick, sickCont );
  SCQUE_PUT_OP_SetSick( wk->que, pokeID, sick, sickCont.raw );

  // 状態異常アイコン付加
  if( sick < POKESICK_MAX ){
    if( (sick == POKESICK_DOKU) && BPP_SICKCONT_IsMoudokuCont(sickCont) ){
      sick = POKESICK_MAX;
    }
    SCQUE_PUT_ACT_SickIcon( wk->que, pokeID, sick );
  }
}
/**
 *  状態異常化の失敗原因を表示
 */
static void scPut_AddSickFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, BtlAddSickFailCode failCode, WazaSick sick )
{
  u8 pokeID = BPP_GetID( target );

  switch( failCode ){
  case BTL_ADDSICK_FAIL_ALREADY:
    switch( sick ){
    case WAZASICK_DOKU:   SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_DokuAlready, pokeID ); break;
    case WAZASICK_YAKEDO: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_YakedoAlready, pokeID ); break;
    case WAZASICK_MAHI:   SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_MahiAlready, pokeID ); break;
    case WAZASICK_NEMURI: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NemuriAlready, pokeID ); break;
    case WAZASICK_KOORI:  SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KoriAlready, pokeID ); break;
    case WAZASICK_KONRAN: SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KonranAlready, pokeID ); break;
    default:
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
    }
    return;

  case BTL_ADDSICK_FAIL_NO_EFFECT:
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_NoEffect, pokeID );
    return;

  default:
    break;
  }

  SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_WazaFail );
}
//--------------------------------------------------------------------------
/**
 * [Put] セットメッセージ表示
 */
//--------------------------------------------------------------------------
static void scPut_Message_Set( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 strID )
{
  u8 pokeID = BPP_GetID( bpp );
  SCQUE_PUT_MSG_SET( wk->que, strID, pokeID );
}
//--------------------------------------------------------------------------
/**
 * [Put] 標準メッセージ表示（引数拡張）
 */
//--------------------------------------------------------------------------
static void scPut_Message_StdEx( BTL_SVFLOW_WORK* wk, u16 strID, u32 argCnt, const int* args )
{
  switch( argCnt ){
  case 0:
  default:
    SCQUE_PUT_MSG_STD( wk->que, strID );
    break;
  case 1:
    SCQUE_PUT_MSG_STD( wk->que, strID, args[0] );
    break;
  case 2:
    SCQUE_PUT_MSG_STD( wk->que, strID, args[0], args[1] );
    break;
  case 3:
    SCQUE_PUT_MSG_STD( wk->que, strID, args[0], args[1], args[2] );
    break;
  case 4:
    SCQUE_PUT_MSG_STD( wk->que, strID, args[0], args[1], args[2], args[3] );
    break;
  }
}
//--------------------------------------------------------------------------
/**
 * [Put] セットメッセージ表示（引数拡張）
 */
//--------------------------------------------------------------------------
static void scPut_Message_SetEx( BTL_SVFLOW_WORK* wk, u16 strID, u32 argCnt, const int* args )
{
  switch( argCnt ){
  case 0:
  default:
    SCQUE_PUT_MSG_SET( wk->que, strID );
    break;
  case 1:
    SCQUE_PUT_MSG_SET( wk->que, strID, args[0] );
    break;
  case 2:
    SCQUE_PUT_MSG_SET( wk->que, strID, args[0], args[1] );
    break;
  case 3:
    SCQUE_PUT_MSG_SET( wk->que, strID, args[0], args[1], args[2] );
    break;
  case 4:
    SCQUE_PUT_MSG_SET( wk->que, strID, args[0], args[1], args[2], args[3] );
    break;
  }
}
//----------------------------------------------------------------------------------
/**
 * [Put] ポケモンHPを減らす
 *
 * @param   wk
 * @param   bpp
 * @param   value
 */
//----------------------------------------------------------------------------------
static void scPut_DecreaseHP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u32 value )
{
  if( !BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_HP_CONST) )
  {
    BPP_HpMinus( bpp, value );
    SCQUE_PUT_OP_HpMinus( wk->que, BPP_GetID(bpp), value );
  }
}
//----------------------------------------------------------------------------------
/**
 * [Put] PP回復
 *
 * @param   wk
 * @param   bpp
 * @param   wazaIdx
 * @param   volume
 * @param   itemID    アイテムを使った効果の場合はアイテムID
 */
//----------------------------------------------------------------------------------
static void scPut_RecoverPP( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u8 wazaIdx, u8 volume, BOOL fOrgWaza )
{
  u8 pokeID = BPP_GetID( bpp );
  WazaID waza;

  if( fOrgWaza )
  {
    waza = BPP_WAZA_IncrementPP_Org( bpp, wazaIdx, volume );
    SCQUE_PUT_OP_PPPlus_Org( wk->que, pokeID, wazaIdx, volume );
  }
  else
  {
    waza = BPP_WAZA_IncrementPP( bpp, wazaIdx, volume );
    SCQUE_PUT_OP_PPPlus( wk->que, pokeID, wazaIdx, volume );
  }
}
//----------------------------------------------------------------------------------
/**
 * [Put] きのみを食べるアクション
 */
//----------------------------------------------------------------------------------
static void scPut_UseItemAct( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  SCQUE_PUT_ACT_KINOMI( wk->que, BPP_GetID(bpp) );
}
//----------------------------------------------------------------------------------
/**
 * [Put] 継続フラグセット
 *
 * @param   wk
 * @param   bpp
 * @param   flag
 */
//----------------------------------------------------------------------------------
static void scPut_SetContFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag flag )
{
  BPP_CONTFLAG_Set( bpp, flag );
  SCQUE_PUT_OP_SetContFlag( wk->que, BPP_GetID(bpp), flag );
}
//----------------------------------------------------------------------------------
/**
 * [Put] 継続フラグクリア
 *
 * @param   wk
 * @param   bpp
 * @param   flag
 */
//----------------------------------------------------------------------------------
static void scPut_ResetContFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppContFlag flag )
{
  BPP_CONTFLAG_Clear( bpp, flag );
  SCQUE_PUT_OP_ResetContFlag( wk->que, BPP_GetID(bpp), flag );
}
//----------------------------------------------------------------------------------
/**
 * [Put] ターンフラグセット
 *
 * @param   wk
 * @param   bpp
 * @param   flag
 */
//----------------------------------------------------------------------------------
static void scPut_SetTurnFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppTurnFlag flag )
{
  BPP_TURNFLAG_Set( bpp, flag );
  SCQUE_PUT_OP_SetTurnFlag( wk->que, BPP_GetID(bpp), flag );
}
//----------------------------------------------------------------------------------
/**
 * [Put] ターンフラグリセット
 *
 * @param   wk
 * @param   bpp
 * @param   flag
 */
//----------------------------------------------------------------------------------
static void scPut_ResetTurnFlag( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, BppTurnFlag flag )
{
  BPP_TURNFLAG_ForceOff( bpp, flag );
  SCQUE_PUT_OP_ResetTurnFlag( wk->que, BPP_GetID(bpp), flag );
}
//----------------------------------------------------------------------------------
/**
 * [Put] イリュージョン再設定
 *
 * @param   wk
 * @param   rec
 */
//----------------------------------------------------------------------------------
static void scPut_IllusionSet( BTL_SVFLOW_WORK* wk, CLIENTID_REC* rec )
{
  u32 i;
  BTL_PARTY* party;
  for(i=0; i<rec->count; ++i)
  {
    party = BTL_POKECON_GetPartyData( wk->pokeCon, rec->clientID[i] );
    SCQUE_PUT_OP_SetIllusionForParty( wk->que, rec->clientID[i] );
    BTL_MAIN_SetIllusionForParty(  wk->mainModule, party, rec->clientID[i] );
  }
}

//---------------------------------------------------------------------------------------------
//
// 【Event】
//
// メイン処理層での判定に必要な各種条件の決定を行う。
// ハンドラ類の呼び出しはイベント層からのみ行われる。
// ポケモンパラメータの操作は行わない。
// コマンド出力は行わない。
//
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/**
 * [Event] メンバー入れ替え前に特定ポケモンのアクションを割り込ませるかチェック
 *
 * @param   wk
 * @param   outPoke
 *
 * @retval  u32        割り込みたいポケモン数
 */
//----------------------------------------------------------------------------------
static u32 scEvent_MemberChangeIntr( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* outPoke )
{
  u32 i;

  // １度、未処理ポケモンの全ワザハンドラを登録しておく
  for(i=0; i<wk->numActOrder; ++i)
  {
    if( (wk->actOrder[i].bpp != outPoke) && (wk->actOrder[i].fDone == FALSE) )
    {
      WazaID waza = ActOrder_GetWazaID( &wk->actOrder[i] );
      if( waza != WAZANO_NULL ){
        if( BTL_HANDLER_Waza_Add( wk->actOrder[i].bpp, waza ) ){
          wk->actOrder[i].fIntrCheck = TRUE;
        }
      }
    }
  }

  wk->MemberOutIntrPokeCount = 0;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1, BPP_GetID(outPoke) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_MENBERCHANGE_INTR );
  BTL_EVENTVAR_Pop();

  // 未処理ポケモンの全ワザハンドラを削除
  for(i=0; i<wk->numActOrder; ++i)
  {
    if( wk->actOrder[i].fIntrCheck)
    {
      WazaID waza = ActOrder_GetWazaID( &wk->actOrder[i] );
      if( waza != WAZANO_NULL ){
        BTL_HANDLER_Waza_RemoveForce( wk->actOrder[i].bpp, waza );
      }
      wk->actOrder[i].fIntrCheck = FALSE;
    }
  }

  return wk->MemberOutIntrPokeCount;
}

//----------------------------------------------------------------------------------
/**
 * [Event] 他のワザを呼び出すワザなら、呼び出すワザのパラメータをセットする
 *
 * @param   wk
 * @param   attacker
 * @param   orgWazaid
 * @param   orgTargetPos
 * @param   reqWaza         [io] 既にアクション処理順序が設定されている
 *
 * @retval  BOOL    他のワザを呼び出すワザ自体を出すのに失敗する場合FALSE／それ以外TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_GetReqWazaParam( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker,
  WazaID orgWazaid, BtlPokePos orgTargetPos, REQWAZA_WORK* reqWaza )
{
  BOOL failFlag = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEPOS_ORG, orgTargetPos );
    BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, WAZANO_NULL );
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEPOS, BTL_POS_NULL );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, failFlag );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_REQWAZA_PARAM );

    reqWaza->wazaID = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    reqWaza->targetPos = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEPOS );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );

  BTL_EVENTVAR_Pop();

  return !failFlag;
}
//----------------------------------------------------------------------------------
/**
 * [Event] “たたかう”を選んだときの特殊優先度を判定
 *
 * @param   wk
 * @param   attacker
 *
 */
//----------------------------------------------------------------------------------
static u8 scEvent_CheckSpecialActPriority( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker )
{
  u8 spPri;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SP_PRIORITY, BTL_SPPRI_DEFAULT );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_SP_PRIORITY );
    spPri = BTL_EVENTVAR_GetValue( BTL_EVAR_SP_PRIORITY );
  BTL_EVENTVAR_Pop();

  return spPri;
}

//----------------------------------------------------------------------------------
/**
 * [Event] ターン最初のワザ処理シーケンス直前
 *
 * @param   wk
 * @param   bpp
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_BeforeFight( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, WazaID waza )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_BEFORE_FIGHT );
  BTL_EVENTVAR_Pop();
}
//--------------------------------------------------------------------------
/**
 * [Event] こんらんダメージ計算
 *
 * @param   wk
 * @param   attacker
 *
 * @retval  u16   ダメージ値
 */
//--------------------------------------------------------------------------
static u32 scproc_CalcConfDamage( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker )
{
  u32 atk, guard, level, ratio;
  u16 dmg;
  BppKoraeruCause  korae_cause;

  atk = BPP_GetValue( attacker, BPP_ATTACK );
  guard = BPP_GetValue( attacker, BPP_DEFENCE );
  level = BPP_GetValue( attacker, BPP_LEVEL );

  dmg = BTL_CALC_DamageBase( 40, atk, level, guard );

  ratio = 100 - BTL_CALC_GetRand( 16 );
  dmg = (dmg * ratio) / 100;
  if( dmg == 0 ){
    dmg = 1;
  }

  korae_cause = scEvent_CheckKoraeru( wk, attacker, attacker, FALSE, &dmg );
  scPut_ConfDamage( wk, attacker, dmg );

  if( korae_cause != BPP_KORAE_NONE ){
    scproc_Koraeru( wk, attacker, korae_cause );
  }
  return dmg;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 他ワザ呼び出し時など、独自のワザメッセージを出力するか判定
 *
 * @param   wk
 * @param   attacker    ワザを出すポケモン
 * @param   orgWazaID   呼び出し元のワザID（ex. ゆびをふる）
 * @param   actWazaID   呼び出し後のワザID
 * @param   str         [out] 独自ワザメッセージパラメータを格納
 *
 * @retval  BOOL    独自ワザメッセージを出力する場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckWazaMsgCustom( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker,
  WazaID orgWazaID, WazaID actWazaID, BTL_HANDEX_STR_PARAMS* str )
{
  BOOL result;

  HANDEX_STR_Clear( str );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, actWazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WORK_ADRS, (int)str );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_REQWAZA_MSG );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return result;
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
static void scEvent_GetWazaParam( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker, SVFL_WAZAPARAM* param )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_WAZA_TYPE, WAZADATA_GetType(waza) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_USER_TYPE, BPP_GetPokeType(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE_TYPE, WAZADATA_GetDamageType(waza) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_TARGET_TYPE, WAZADATA_GetParam(waza, WAZAPARAM_TARGET) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FLAT_FLAG, FALSE );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_PARAM );

    param->wazaID      = waza;
    param->orgWazaID   = waza;
    param->wazaType    = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
    param->userType    = BTL_EVENTVAR_GetValue( BTL_EVAR_USER_TYPE );
    param->damageType  = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE_TYPE );
    param->targetType  = BTL_EVENTVAR_GetValue( BTL_EVAR_TARGET_TYPE );
    param->flags = 0;
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_FLAT_FLAG) ){
      param->wazaType = POKETYPE_NULL;
    }

    /*
      if( waza != WAZANO_NOROI ){
        param->targetType = WAZADATA_GetParam( waza, WAZAPARAM_TARGET );
      }else{
        param->targetType = BTL_CALC_GetNoroiTargetType( attacker );
      }
    */

  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザだし失敗確定
 *
 * @param   wk
 * @param   bpp
 * @param   waza
 * @param   cause
 */
//----------------------------------------------------------------------------------
static void scEvent_CheckWazaExeFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp,
  WazaID waza, SV_WazaFailCause cause )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_FAIL_CAUSE, cause );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXECUTE_FAIL );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ出し成功確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 *
 * @retval  BtlWazaForceEnableMode  特殊条件により、強制的に有効と判断（エフェクト発動）するためのパターン指定子
 */
//----------------------------------------------------------------------------------
static BtlWazaForceEnableMode scEvent_WazaExecuteStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, BTL_POKESET* rec )
{
  BOOL fQuit = FALSE;
  u32 targetCnt = BTL_POKESET_GetCount( rec );
  BtlWazaForceEnableMode  enableMode;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, targetCnt );
    {
      u32 i;
      const BTL_POKEPARAM* bpp;
      for(i=0; i<targetCnt; ++i){
        bpp = BTL_POKESET_Get( rec, i );
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(bpp) );
      }
    }
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_ENABLE_MODE, BTL_WAZAENABLE_NONE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_EXE_START );
    enableMode = BTL_EVENTVAR_GetValue( BTL_EVAR_ENABLE_MODE );
  BTL_EVENTVAR_Pop();

  return enableMode;
}
//----------------------------------------------------------------------------------
/**
 * [Event] まもる・みきりの無効化チェック
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza
 *
 * @retval  BOOL    無効化する場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckMamoruBreak( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
  BOOL result = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, result );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_MAMORU_BREAK );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();
  return result;
}
//----------------------------------------------------------------------------------
/**
 * 溜めターン失敗判定
 *
 * @param   wk
 * @param   attacker
 * @param   target
 *
 * @retval  BOOL    失敗ならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckTameFail( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* targetRec )
{
  BOOL fFail = FALSE;
  const BTL_POKEPARAM* target = BTL_POKESET_Get( targetRec, 0 );


  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, (target!=NULL)? BPP_GetID(target) : BTL_POKEID_NULL );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_TAMETURN_FAIL );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();
  return fFail;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 溜めターンスキップ判定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 *
 * @retval  BOOL    溜めターンスキップする場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckTameTurnSkip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  BOOL result = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, waza );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, result );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_TAMETURN_SKIP );
    result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();
  return result;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 溜めターン発動確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_TameStart( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* targetRec, WazaID waza, u8* hideTargetPokeID, BOOL* fFailMsgDisped )
{
  const BTL_POKEPARAM* target;
  u32 targetCnt, i;
  BOOL fFail = FALSE;

  targetCnt = BTL_POKESET_GetCount( targetRec );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, targetCnt );
    if( targetCnt )
    {
      u32 i;
      for(i=0; i<targetCnt; ++i)
      {
        target = BTL_POKESET_Get( targetRec, i );
        BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(target) );
      }
    }
    else{
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1, BTL_POKEID_NULL );
    }
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, fFail );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_MSG_FLAG, FALSE );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TAME_START );

    if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) ){
      *hideTargetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_TARGET1 );
    }else{
      *hideTargetPokeID = BTL_POKEID_NULL;
    }
    *fFailMsgDisped = BTL_EVENTVAR_GetValue( BTL_EVAR_MSG_FLAG );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );

  BTL_EVENTVAR_Pop();

  return !fFail;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 溜めターンスキップ確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static void scEvent_TameSkip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TAME_SKIP );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] 溜めターン発動確定
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_TameRelease( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKESET* rec, WazaID waza )
{
  const BTL_POKEPARAM* target;
  u32 targetCnt, i;
  u8 fFail = FALSE;

  targetCnt = BTL_POKESET_GetCount( rec );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TARGET_POKECNT, targetCnt );
    for(i=0; i<targetCnt; ++i)
    {
      target = BTL_POKESET_Get( rec, i );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_TARGET1+i, BPP_GetID(target) );
    }
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TAME_RELEASE );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  return (!fFail);
}
//----------------------------------------------------------------------------------
/**
 * [Event] そらをとぶ、ダイビング等、場から隠れているポケモンへのヒットチェック
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza
 *
 * @retval  BOOL    隠れているためヒットしない場合にTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckPokeHideAvoid( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
  BOOL avoidFlag = FALSE;

  BppContFlag  hideState = BPP_CONTFLAG_CheckWazaHide( defender );
  if( hideState != BPP_CONTFLG_NULL )
  {
    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKE_HIDE, hideState );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_AVOID_FLAG, TRUE );

      BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_POKE_HIDE );
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_AVOID_FLAG) ){
        avoidFlag = TRUE;
      }
    BTL_EVENTVAR_Pop();
  }

  return avoidFlag;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザの命中チェック時、確率計算をスキップして命中させるかチェック
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza
 *
 * @retval  BOOL        確率計算をスキップする場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_IsCalcHitCancel( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, WazaID waza )
{
  BOOL result = WAZADATA_IsAlwaysHit( waza );
  if( !result )
  {
    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_HIT_CANCEL );
      result = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
    BTL_EVENTVAR_Pop();
  }
  return result;
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
 * @retval  u32    命中率 (じゅうりょく等、命中率上昇の効果を受けて100超過する場合もあり得る）
 */
//--------------------------------------------------------------------------
static u32 scEvent_getHitPer( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam )
{
  u32 per = WAZADATA_GetParam( wazaParam->wazaID, WAZAPARAM_HITPER );
  fx32 ratio = FX32_CONST(1);

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, wazaParam->damageType );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_HIT_PER, per );
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.1), FX32_CONST(32) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_RATIO );
    per = BTL_EVENTVAR_GetValue( BTL_EVAR_HIT_PER );
    ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
  BTL_EVENTVAR_Pop();

  per = BTL_CALC_MulRatio( per, ratio );

  return per;
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
  u8 atkLevel = BPP_GetValue( attacker, BPP_LEVEL );
  u8 defLevel = BPP_GetValue( defender, BPP_LEVEL );

  // 攻撃側レベルが防御側レベル未満なら常に失敗
  if( atkLevel < defLevel )
  {
    return FALSE;
  }

  {
    BOOL ret = FALSE;
    BOOL fFail = FALSE;

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_ICHIGEKI_CHECK );
      fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    BTL_EVENTVAR_Pop();

    // とくせい等による強制無効化があれば失敗
    if( fFail ){
      return FALSE;
    }

    if( IsMustHit(attacker, defender) ){
      ret = TRUE;
    }else{
      u8 per = WAZADATA_GetParam( waza, WAZAPARAM_HITPER );
      BTL_Printf("デフォ命中率=%d\n", per);
      per += (atkLevel - defLevel);
      ret = perOccur( wk, per );
      BTL_Printf("レベル補正命中率=%d, hit=%d\n", per, ret);
    }
    return ret;
  }
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
static BOOL scEvent_CheckDmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam )
{
  BOOL ret = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, ret );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DMG_TO_RECOVER_CHECK );
    ret = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();
  return ret;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ダメージワザ回復化決定
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 */
//----------------------------------------------------------------------------------
static void scEvent_DmgToRecover( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DMG_TO_RECOVER_FIX );
  BTL_EVENTVAR_Pop();
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
  u16  rank = WAZADATA_GetParam( waza, WAZAPARAM_CRITICAL_RANK );
  rank += BPP_GetCriticalRank( attacker );

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_CRITICAL_RANK, rank );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CRITICAL_CHECK );
    if( !BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG ) )
    {
      if( WAZADATA_IsMustCritical(waza) ){
        flag = TRUE;
      }else{
        rank = roundMax( BTL_EVENTVAR_GetValue(BTL_EVAR_CRITICAL_RANK), BTL_CALC_CRITICAL_MAX );
        flag = BTL_CALC_CheckCritical( rank );
      }

      if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_MUST_CRITICAL) ){
        flag = TRUE;
      }

    }
    else{
      flag = FALSE;
    }
  BTL_EVENTVAR_Pop();
  return flag;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによるダメージを与えた後の反動計算（ダメージ×反動率）
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   damage
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static u32 scEvent_CalcKickBack( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, u32 damage, BOOL* fMustEnable )
{
  u8 pokeID = BPP_GetID( attacker );
  u8 ratio = WAZADATA_GetParam( waza, WAZAPARAM_DAMAGE_REACTION_RATIO );
  u8 ratio_ex = 0;
  u8 fail_flag = FALSE;
  u8 anti_fail_flag = FALSE;
  SV_ReactionType  reactionType = SV_REACTION_NONE;

  *fMustEnable = FALSE;

  if( ratio )
  {
    reactionType = SV_REACTION_DAMAGE;
  }
  else
  {
    ratio = WAZADATA_GetParam( waza, WAZAPARAM_HP_REACTION_RATIO );
    if(  ratio ){
      reactionType = SV_REACTION_HP;
    }
  }

  //ドレイン系は反動にならないようにする
  if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_DRAIN )
  {
    return 0;
  }

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, pokeID );
    BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, ratio );
    BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO_EX, 0 );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_KICKBACK );

    ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
    fail_flag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    anti_fail_flag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
    ratio_ex = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO_EX );
  BTL_EVENTVAR_Pop();

  if( !anti_fail_flag )
  {
    if( fail_flag == TRUE ){
      return 0;
    }
  }
  ratio += ratio_ex;

  if( ratio )
  {
    if( reactionType == SV_REACTION_HP )
    {
      u32 maxHP = BPP_GetValue( attacker, BPP_MAX_HP );
      damage = roundMin( BTL_CALC_MulRatioInt(maxHP, ratio), 1 );
    }
    else{
      damage = roundMin( BTL_CALC_MulRatioInt(damage, ratio), 1 );
    }

    (*fMustEnable) = anti_fail_flag;

    return damage;
  }
  return 0;
}
//----------------------------------------------------------------------------------
/**
 * [Event] 装備アイテム使用
 *
 * @param   wk
 * @param   bpp
 * @param   evwk
 */
//----------------------------------------------------------------------------------
static void scEvent_ItemEquip( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_USE_ITEM );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] 一時的に登録したアイテムの使用（ついばむなど）
 *
 * @param   wk
 * @param   bpp
 * @param   evwk
 */
//----------------------------------------------------------------------------------
static void scEvent_ItemEquipTmp( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u8 atkPokeID )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID_ATK, atkPokeID );
    BTL_EVENT_ForceCallHandlers( wk, BTL_EVENT_USE_ITEM_TMP );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * ワザの相性計算（ダメージ・一撃ワザのみ）
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   wazaParam
 * @param   fNoEffectMsg    効果が無いワザの場合、それを示すメッセージ表示を行う
 *
 * @retval  BtlTypeAff
 */
//----------------------------------------------------------------------------------
static BtlTypeAff scProc_checkWazaDamageAffinity( BTL_SVFLOW_WORK* wk,
  BTL_POKEPARAM* attacker, BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BOOL fNoEffectMsg )
{
  BOOL fFloating = FALSE;

  // 無属性の攻撃（わるあがき）は、常に等倍でヒット
  if( wazaParam->wazaType == POKETYPE_NULL ){
    return BTL_TYPEAFF_100;
  }

  {
    BtlTypeAff affinity;
    PokeTypePair defPokeTypePair = BPP_GetPokeType( defender );
    PokeType  defPokeType = PokeTypePair_GetType1( defPokeTypePair );

    affinity = scEvent_CheckDamageAffinity( wk, attacker, defender, wazaParam->wazaType, defPokeType );
    if( !PokeTypePair_IsPure(defPokeTypePair) )
    {
      PokeTypePair  type2 = PokeTypePair_GetType2( defPokeTypePair );
      BtlTypeAff    aff2 = scEvent_CheckDamageAffinity( wk, attacker, defender, wazaParam->wazaType, type2 );
      affinity = BTL_CALC_TypeAffMul( affinity, aff2 );
    }

    // 相性有効
    if( affinity != BTL_TYPEAFF_0 )
    {
      // 地面ワザ以外はそのまま
      if( wazaParam->wazaType != POKETYPE_JIMEN ){
        return affinity;
      }
      // 地面ワザでも「ふゆう状態」でなければそのまま
      else
      {
        fFloating = scproc_CheckFloating( wk, defender, FALSE );
        if( !fFloating ){
          return affinity;
        }
        // ふゆう状態なら相性無効
        affinity = BTL_TYPEAFF_0;
      }
    }
    // 相性無効
    else
    {
      // 地面ワザなら受けたポケ=飛行タイプのはずなので、「ふゆう状態」チェック
      if( wazaParam->wazaType == POKETYPE_JIMEN )
      {
        // 「ふゆう状態」でないなら等倍ヒット
        if( !scproc_CheckFloating(wk, defender, TRUE) ){
          return BTL_TYPEAFF_100;
        }
      }
    }

    if( affinity == BTL_TYPEAFF_0 )
    {
      if( fNoEffectMsg )
      {
        if( !fFloating ){
          scput_WazaNoEffect( wk, defender );
        }
        else{
          scproc_WazaNoEffectByFlying( wk, defender );
        }
      }
    }

    return affinity;
  }
}
//----------------------------------------------------------------------------------
/**
 * ポケモン単体ふゆう状態のチェック
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static BOOL scproc_CheckFloating( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fHikouCheck )
{
  // じゅうりょくが効いていたら誰も浮けないのでチェックしない
  if( !BTL_FIELD_CheckEffect(BTL_FLDEFF_JURYOKU) )
  {
    if( scEvent_CheckFloating(wk, bpp, fHikouCheck) ){
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ふゆう状態チェック
 *
 * @param   wk
 * @param   bpp
 * @param   fHikouCheck   タイプ「ひこう」をふゆう要素としてチェックするか？
 *
 * @retval  BOOL    有効であればTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckFloating( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fHikouCheck )
{
  // ここでは"じゅうりょく"の有無をチェックしない。
  // （じゅうりょく発動直後に「じめんにおちた」テキスト表示のチェックに使いたいので）
  {
    u8 floatFlag = (fHikouCheck)? BPP_IsMatchType( bpp, POKETYPE_HIKOU ) : FALSE;
    u8 failFlag = FALSE;

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, floatFlag );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, failFlag );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_FLYING );
      BTL_SICKEVENT_CheckFlying( wk, bpp );
      floatFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
      failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    BTL_EVENTVAR_Pop();

    if( failFlag ){
      return FALSE;
    }
    return floatFlag;
  }
}
//----------------------------------------------------------------------------------
/**
 * ふゆう状態で地面ワザ無効だった時のメッセージ表示
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scproc_WazaNoEffectByFlying( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  scEvent_WazaNoEffectByFlying( wk, bpp );

  if( scproc_HandEx_Result(wk) != HandExResult_Enable ){
    scput_WazaNoEffect( wk, bpp );
  }

  BTL_Hem_PopState( &wk->HEManager, hem_state );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ふゆう状態による地面ワザ無効化反応
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaNoEffectByFlying( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_NOEFF_BY_FLYING );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * [Event] ワザ相性計算
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza_type
 * @param   poke_type
 *
 * @retval  BtlTypeAff
 */
//----------------------------------------------------------------------------------
static BtlTypeAff scEvent_CheckDamageAffinity( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, PokeType waza_type, PokeType poke_type )
{
  BOOL  flatFlag, flatMasterFlag;
  BtlTypeAff affinity;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKE_TYPE, poke_type );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, waza_type );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FLAT_FLAG, FALSE );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FLATMASTER_FLAG, FALSE );

    BTL_SICKEVENT_CheckNotEffectByType( wk, defender );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_AFFINITY );

    flatFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FLAT_FLAG );
    flatMasterFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FLATMASTER_FLAG );
  BTL_EVENTVAR_Pop();

  if( flatMasterFlag ){
    return BTL_TYPEAFF_100;
  }

  affinity = BTL_CALC_TypeAff( waza_type, poke_type );
  if( (affinity == BTL_TYPEAFF_0) && (flatFlag) ){
    return BTL_TYPEAFF_100;
  }

  return affinity;
}


//----------------------------------------------------------------------------------
/**
 * エフェクト生成
 *
 * @param   wk
 * @param   effectNo    エフェクトナンバー
 * @param   pos_from    エフェクト開始位置（不要ならBTL_POS_NULL)
 * @param   pos_to      エフェクト終了位置（不要ならBTL_POS_NULL)
 * @param   fQueResereved   予約済みQue領域を利用するフラグ
 * @param   reservedPos     予約済みQue位置
 */
//----------------------------------------------------------------------------------
static void scproc_ViewEffect( BTL_SVFLOW_WORK* wk, u16 effectNo, BtlPokePos pos_from, BtlPokePos pos_to, BOOL fQueResereved, u32 reservedPos )
{
  enum {
    EFF_SIMPLE = 0, // 位置指定なし
    EFF_POS,        // 開始位置のみ指定
    EFF_VECTOR,     // 開始・終端位置ともに指定
  };

  int effType;

  if( pos_to != BTL_POS_NULL ){
    effType = EFF_VECTOR;
  }else if( pos_from != BTL_POS_NULL ) {
    effType = EFF_POS;
  }else{
    effType = EFF_SIMPLE;
  }

  // 予約領域へ書き込み
  if( fQueResereved )
  {
    switch( effType ){
    case EFF_SIMPLE:
      SCQUE_PUT_ReservedPos( wk->que, reservedPos, SC_ACT_EFFECT_SIMPLE, effectNo );
      break;

    case EFF_POS:
      SCQUE_PUT_ReservedPos( wk->que, reservedPos, SC_ACT_EFFECT_BYPOS,
          pos_from, effectNo );
      break;

    case EFF_VECTOR:
      SCQUE_PUT_ReservedPos( wk->que, reservedPos, SC_ACT_EFFECT_BYVECTOR,
          pos_from, pos_to, effectNo );
      break;
    }
  }
  // 最後尾に追記
  else
  {
    switch( effType ){
    case EFF_SIMPLE:
      SCQUE_PUT_ACT_EffectSimple( wk->que, effectNo );
      break;

    case EFF_POS:
      SCQUE_PUT_ACT_EffectByPos( wk->que, pos_from, effectNo );
      break;

    case EFF_VECTOR:
      SCQUE_PUT_ACT_EffectByVector( wk->que, pos_from, pos_to, effectNo );
      break;
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ使用後、PPをデクリメントした後の反応
 *
 * @param   wk
 * @param   attacker
 *
 * @retval  BOOL    アイテム使用反応があればTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_DecrementPP_Reaction( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, u8 wazaIdx )
{
  BOOL flag = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID,   BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_IDX, wazaIdx );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, flag );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECREMENT_PP_DONE );
    flag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return flag;
}
//--------------------------------------------------------------------------
/**
 * [Event] 複数回ヒットワザのチェック処理
 *
 * @param   wk
 * @param   attacker
 * @param   waza
 * @param   param            [out] ヒットチェックパラメータ
 *
 * @retval  BOOL    複数回ヒットするワザならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_HitCheckParam( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, WazaID waza, HITCHECK_PARAM* param )
{
  u8 max = WAZADATA_GetParam( waza, WAZAPARAM_HITCOUNT_MAX );

  param->count = 0;
  param->pluralHitAffinity = BTL_TYPEAFF_100;

  if( max > 1 )
  {
    u8 hit_cnt = BTL_CALC_HitCountStd( max );

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_HITCOUNT_MAX, max );
      BTL_EVENTVAR_SetValue( BTL_EVAR_HITCOUNT, hit_cnt );
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );   // 最大までヒットさせるフラグ
      BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_AVOID_FLAG, FALSE ); // １回ヒットごとに命中判定を行うフラグ

      BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_HIT_COUNT );

      // 強制的に最大回数までヒット
      if( ( max <= 5)
      &&  ( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) )
      ){
        param->countMax = max;
        param->fCheckEveryTime = FALSE;
      }
      else
      {
        param->countMax = BTL_EVENTVAR_GetValue( BTL_EVAR_HITCOUNT );
        param->fCheckEveryTime = BTL_EVENTVAR_GetValue( BTL_EVAR_AVOID_FLAG );
      }
      param->fPluralHitWaza = TRUE;

    BTL_EVENTVAR_Pop();

    return TRUE;
  }
  else
  {
    param->countMax = 1;
    param->fCheckEveryTime = FALSE;
    param->fPluralHitWaza = FALSE;
    return FALSE;
  }
}

//----------------------------------------------------------------------------------
/**
 * [Event] ワザ威力取得
 *
 * @param   wk
 * @param   attacker
 * @param   defender
 * @param   waza
 * @param   wazaParam
 *
 * @retval  u16   ワザ威力値
 */
//----------------------------------------------------------------------------------
static u16 scEvent_getWazaPower( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam )
{
  u16 power = WAZADATA_GetPower( wazaParam->wazaID );
  fx32 ratio;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, wazaParam->damageType );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_WAZA_POWER, power );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_POWER_BASE );

    BTL_EVENTVAR_SetMulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.0), FX32_CONST(0.01f), FX32_CONST(512) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_POWER );
    power = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
    ratio = (fx32)BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER_RATIO );
  BTL_EVENTVAR_Pop();

  power = BTL_CALC_MulRatio( power, ratio );
  BTL_N_Printf( DBGSTR_SVFL_WazaPower, wazaParam->wazaID, power, ratio);
  return power;
}
//--------------------------------------------------------------------------
/**
 * [Event] ワザ以外のダメージ有効／無効チェック
 *
 * @param   wk
 * @param   bpp
 * @param   damage  ダメージ量
 *
 * @retval  BOOL    有効ならTRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckEnableSimpleDamage( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 damage )
{
  BOOL flag;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, TRUE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_SIMPLE_DAMAGE_ENABLE );
    flag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();

  return flag;
}

// 攻撃側能力値取得
static u16 scEvent_getAttackPower( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, const SVFL_WAZAPARAM* wazaParam, BOOL criticalFlag )
{
  BppValueID vid = (WAZADATA_GetDamageType(wazaParam->wazaID) == WAZADATA_DMG_SPECIAL)? BPP_SP_ATTACK : BPP_ATTACK;

  {
    const BTL_POKEPARAM* calc_attacker = attacker;
    fx32 ratio;
    u16 power;

    BTL_EVENTVAR_Push();
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
      BTL_EVENTVAR_SetValue( BTL_EVAR_SWAP_POKEID, BTL_POKEID_NULL );
      BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, FALSE );    //
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_ATTACKER_POWER_PREV );

      {
        u8 swapPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_SWAP_POKEID );
        if( swapPokeID != BTL_POKEID_NULL ){
          calc_attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, swapPokeID );
        }
      }

      // クリティカルチェック
      if( BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG ) ){
        power = BPP_GetValue_Base( calc_attacker, vid );
      }
      else
      {
        if( criticalFlag ){
          power = BPP_GetValue_Critical( calc_attacker, vid );
        }else{
          power = BPP_GetValue( calc_attacker, vid );
        }
      }

      BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
      BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, wazaParam->damageType );
      BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, power );
      BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_ONE, FX32_CONST(0.1), FX32_CONST(32) );
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_ATTACKER_POWER );
      power = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
      ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
      power = BTL_CALC_MulRatio( power, ratio );
      BTL_N_Printf( DBGSTR_SVFL_AtkPower, power, criticalFlag, ratio);
    BTL_EVENTVAR_Pop();
    return power;
  }
}

// 防御側能力値取得
static u16 scEvent_getDefenderGuard( BTL_SVFLOW_WORK* wk,
  const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender,
  const SVFL_WAZAPARAM* wazaParam, BOOL criticalFlag )
{
  BppValueID vid = (WAZADATA_GetDamageType(wazaParam->wazaID) == WAZADATA_DMG_SPECIAL)? BPP_SP_DEFENCE : BPP_DEFENCE;
  WazaDamageType dmgType = wazaParam->damageType;
  fx32 ratio = FX32_CONST(1);
  u16 guard;
  u8 forceFlatFlag;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_VID, vid );
    BTL_EVENTVAR_SetValue( BTL_EVAR_VID_SWAP_CNT, 0 );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DEFENDER_GUARD_PREV );

    // 特殊要因によって防御・特防をスワップして評価する
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_VID_SWAP_CNT) & 1 )
    {
      vid = (vid == BPP_DEFENCE)? BPP_SP_DEFENCE : BPP_DEFENCE;
      dmgType = (vid == BPP_DEFENCE)? WAZADATA_DMG_PHYSIC : WAZADATA_DMG_SPECIAL;
    }
    forceFlatFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );
  BTL_EVENTVAR_Pop();


  if( forceFlatFlag ){
    guard = BPP_GetValue_Base( defender, vid );
  }else{
    if( criticalFlag ){
      guard = BPP_GetValue_Critical( defender, vid );
    }else{
      guard = BPP_GetValue( defender, vid );
    }
  }

  // てんこう「すなあらし」の時、いわタイプのとくぼう1.5倍
  if( (scEvent_GetWeather(wk) == BTL_WEATHER_SAND)
  &&  (BPP_IsMatchType(defender, POKETYPE_IWA))
  &&  (vid == BPP_SP_DEFENCE)
  ){
    guard = BTL_CALC_MulRatio( guard, FX32_CONST(1.5) );
  }

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(defender) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_DAMAGE_TYPE, dmgType );
    BTL_EVENTVAR_SetValue( BTL_EVAR_GUARD, guard );
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, ratio, FX32_CONST(0.1), FX32_CONST(32) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DEFENDER_GUARD );
    guard = BTL_EVENTVAR_GetValue( BTL_EVAR_GUARD );
    ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
  BTL_EVENTVAR_Pop();

  guard = BTL_CALC_MulRatio( guard, ratio );
  BTL_N_Printf( DBGSTR_SVFL_DefGuard, guard, criticalFlag, ratio);
  return guard;
}
//--------------------------------------------------------------------------
/**
 * [Event] 攻撃ポケとワザのタイプ一致補正率を計算
 *
 * @param   wk
 * @param   attacker
 * @param   waza_type
 *
 * @retval  fx32    補正率
 */
//--------------------------------------------------------------------------
static fx32 scEvent_CalcTypeMatchRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, PokeType waza_type )
{
  BOOL f_match = BPP_IsMatchType( attacker, waza_type );
  fx32 ratio = FX32_CONST(1);

  BTL_EVENTVAR_Push();
  {
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_GEN_FLAG, f_match );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TYPEMATCH_CHECK );
    f_match = BTL_EVENTVAR_GetValue( BTL_EVAR_GEN_FLAG );

    if( f_match ){
      ratio = FX32_CONST(1.5f);
    }

    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TYPEMATCH_FLAG, f_match );
    BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, ratio );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TYPEMATCH_RATIO );

    ratio = (fx32)BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
  }
  BTL_EVENTVAR_Pop();
  return ratio;
}
//--------------------------------------------------------------------------
/**
 * [Event] メンバーを場に登場させた直後
 *
 * @param   wk
 * @param   bpp
 * @param   fAllIn  全員一斉入場処理フラグ（バトル開幕１回のみ）
 *
 */
//--------------------------------------------------------------------------
static void scEvent_AfterMemberIn( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_N_PrintfEx( PRINT_CHANNEL_PRESSURE, DBGSTR_SVFL_MemberInEventBegin, BPP_GetID(bpp) );
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_MEMBER_IN );
  BTL_EVENTVAR_Pop();
  BTL_N_PrintfEx( PRINT_CHANNEL_PRESSURE, DBGSTR_SVFL_MemberInEventEnd, BPP_GetID(bpp) );
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザ追加効果によるひるみ確率取得
 *
 * @param   wk
 * @param   waza
 * @param   attacker
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static u32 scEvent_GetWazaShrinkPer( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* attacker )
{
  BOOL fFail = FALSE;
  u32 per = WAZADATA_GetParam( waza, WAZAPARAM_SHRINK_PER );
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, fFail );
    BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_SHRINK_PER );
    fFail = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
  BTL_EVENTVAR_Pop();
  if( fFail ){
    return 0;
  }
  return per;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ひるみ発生判定
 *
 * @param   wk
 * @param   target
 * @param   waza    ワザ追加効果による場合ワザID（それ以外WAZANO_NULL）
 * @param   per     ひるみ確率
 *
 * @retval  BOOL    ひるむならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckShrink( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target, u32 per )
{
  BOOL fail_flag = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, BPP_GetID(target) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, fail_flag );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_SHRINK_CHECK );

    per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
    fail_flag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  if( fail_flag ){
    return FALSE;
  }
  else if( per > 0 )
  {
    if( BTL_SVFTOOL_GetDebugFlag(wk, BTL_DEBUGFLAG_MUST_TUIKA) ){
      return TRUE;
    }

    return perOccur( wk, per );
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ひるみ失敗原因表示
 *
 * @param   wk
 * @param   target
 */
//----------------------------------------------------------------------------------
static void scEvent_FailShrink( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetValue( BTL_EVAR_POKEID, BPP_GetID(target) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_SHRINK_FAIL );
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
 * @param   wazaRankEffSerial   ワザによる効果の場合、そのワザ１発ごとのシリアルナンバー(1～）
 *
 * @retval  BOOL      成功する場合TRUE
 */
//--------------------------------------------------------------------------
static BOOL scEvent_CheckRankEffectSuccess( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaRankEffect effect, u8 wazaUsePokeID, int volume, u32 wazaRankEffSerial )
{
  BOOL failFlag = FALSE;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, wazaUsePokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_STATUS_TYPE, effect );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_VOLUME, volume );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_SERIAL, wazaRankEffSerial );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, failFlag );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_RANKEFF_LAST_CHECK );
    failFlag = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  return !failFlag;
//  return evwk->failFlag == FALSE;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ランク増減効果の失敗確定
 *
 * @param   wk
 * @param   bpp
 * @param   wazaRankEffSerial   ワザによる効果の場合、そのワザ１発ごとのシリアルナンバー(1～）
 */
//----------------------------------------------------------------------------------
static void scEvent_RankEffect_Failed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u32 wazaRankEffSerial )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_SERIAL, wazaRankEffSerial );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_RANKEFF_FAILED );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ランク増減効果の成功確定
 *
 * @param   wk
 * @param   atkPokeID   効果を発生させたワザ・とくせいの主体ポケID（居ない場合BTL_POKEID_NULL）
 * @param   bpp
 * @param   rankType
 * @param   volume
 */
//----------------------------------------------------------------------------------
static void scEvent_RankEffect_Fix( BTL_SVFLOW_WORK* wk, u8 atkPokeID,
  const BTL_POKEPARAM* bpp, WazaRankEffect rankType, int volume )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, atkPokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_STATUS_TYPE, rankType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_VOLUME, volume );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_RANKEFF_FIXED );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによるランク増減効果の成功確定
 *
 * @param   wk
 * @param   target
 * @param   wazaID
 * @param   effectID
 * @param   volume
 */
//----------------------------------------------------------------------------------
static void scEvent_WazaRankEffectFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* target,
  WazaID wazaID, WazaRankEffect effectID, int volume )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(target) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_STATUS_TYPE, effectID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_VOLUME, volume );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WAZA_RANKEFF_FIXED );
  BTL_EVENTVAR_Pop();
}

//--------------------------------------------------------------------------
/**
 * [Event] ドレイン攻撃のドレインHP量を再計算
 *
 * @param   wk
 * @param   attacker
 * @param   target
 * @param   volume     デフォルト回復量
 *
 * @retval  int     回復量
 */
//--------------------------------------------------------------------------
static u16 scEvent_RecalcDrainVolume( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* target, u16 volume )
{
  fx32 ratio;
  u8   targetPokeID = (target!=NULL)? BPP_GetID(target) : BTL_POKEID_NULL;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_DEF, targetPokeID );
    BTL_EVENTVAR_SetMulValue( BTL_EVAR_RATIO, FX32_CONST(1), FX32_CONST(0.1), FX32_CONST(32) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );

    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CALC_DRAIN );
    volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
    if( volume != 0 ){
      volume = BTL_CALC_MulRatio_OverZero( volume, ratio );
    }
  BTL_EVENTVAR_Pop();

  return volume;
}
//----------------------------------------------------------------------------------
/**
 * 【Event】天候の変化後イベント
 *
 * @param   wk
 * @param   weather
 */
//----------------------------------------------------------------------------------
static void scEvent_AfterChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather )
{
  BOOL fDisable = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WEATHER, weather );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHANGE );
    fDisable = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
    if( !fDisable ){
      BTL_EVENT_CallHandlers( wk, BTL_EVENT_WEATHER_CHANGE_AFTER );
    }
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザによるHP回復量計算
 *
 * @param   wk
 * @param   waza
 * @param   bpp
 *
 * @retval  u32
 */
//----------------------------------------------------------------------------------
static u32 scEvent_CalcRecoverHP( BTL_SVFLOW_WORK* wk, WazaID waza, const BTL_POKEPARAM* bpp )
{
  u32 base_ratio = WAZADATA_GetParam( waza, WAZAPARAM_HP_RECOVER_RATIO );
  fx32 ex_ratio;

  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, 0 );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_RECOVER_HP_RATIO );
    ex_ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
  BTL_EVENTVAR_Pop();

  {
    u32 maxHP = BPP_GetValue( bpp, BPP_MAX_HP );
    u32 volume;

//    TAYA_Printf("maxHP=%d, ex_ratio=%08x, baes_ratio=%08x\n", maxHP, ex_ratio, base_ratio );

    if( ex_ratio ){
      volume = BTL_CALC_MulRatio( maxHP, ex_ratio );
    }else{
      volume = BTL_CALC_MulRatioInt( maxHP, base_ratio );
    }
    if( volume == 0 ){
      volume = 1;
    }else if( volume > maxHP ){
      volume = maxHP;
    }

    return volume;
  }
}
//----------------------------------------------------------------------------------
/**
 * [Event] アイテムをセット（あるいは消去）されるチェック
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL    失敗する場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL scEvent_CheckItemSet( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 itemID )
{
  BOOL failed = FALSE;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ITEM, itemID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FAIL_FLAG, failed );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ITEMSET_CHECK );
    failed = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_FLAG );
  BTL_EVENTVAR_Pop();

  return failed;
}
//----------------------------------------------------------------------------------
/**
 * [Event] アイテム書き換え決定
 *
 * @param   wk
 * @param   bpp
 * @param   nextItemID   書き換え後のアイテムナンバー
 */
//----------------------------------------------------------------------------------
static void scEvent_ItemSetDecide( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, u16 nextItemID )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_ITEM, nextItemID );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ITEMSET_DECIDE );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------
/**
 * [Event] アイテム書き換え完了
 *
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void scEvent_ItemSetFixed( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, BPP_GetID(bpp) );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_ITEMSET_FIXED );
  BTL_EVENTVAR_Pop();
}


//----------------------------------------------------------------------------------
/**
 * [Event] とくせい変更が確定（変更の直前）
 *
 * @param   wk
 * @param   pokeID
 * @param   tokuseiID
 */
//----------------------------------------------------------------------------------
static void scEvent_ChangeTokuseiBefore( BTL_SVFLOW_WORK* wk, u8 pokeID, u16 prev_tokuseiID, u16 next_tokuseiID )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, pokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TOKUSEI_PREV, prev_tokuseiID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_TOKUSEI_NEXT, next_tokuseiID );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHANGE_TOKUSEI_BEFORE );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] とくせいが変更された
 *
 * @param   wk
 * @param   pokeID
 */
//----------------------------------------------------------------------------------
static void scEvent_ChangeTokuseiAfter( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, pokeID );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHANGE_TOKUSEI_AFTER );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] サイドエフェクト継続パラメータ調整
 *
 * @param   wk
 * @param   param->effect
 * @param   param->side
 * @param   &param->cont
 */
//----------------------------------------------------------------------------------
static void scEvent_CheckSideEffectParam( BTL_SVFLOW_WORK* wk, u8 userPokeID, BtlSideEffect effect, BtlSide side, BPP_SICK_CONT* cont )
{
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID, userPokeID );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SIDE, side );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_SIDE_EFFECT, effect );
    BTL_EVENTVAR_SetValue( BTL_EVAR_SICK_CONT, cont->raw );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_CHECK_SIDEEFF_PARAM );
    cont->raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );
  BTL_EVENTVAR_Pop();
}
//----------------------------------------------------------------------------------
/**
 * [Event] エアロック開始通知
 *
 * @param   wk
 * @param   param->effect
 * @param   param->side
 * @param   &param->cont
 */
//----------------------------------------------------------------------------------
static void scEvent_NotifyAirLock( BTL_SVFLOW_WORK* wk )
{
  BTL_EVENTVAR_Push();
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_NOTIFY_AIRLOCK );
  BTL_EVENTVAR_Pop();
}

//----------------------------------------------------------------------------------------------------------
// 相性ヒットカウンタ
//----------------------------------------------------------------------------------------------------------

/**
 *  クリア
 */
static void AffCounter_Clear( WAZA_AFF_COUNTER* cnt )
{
  GFL_STD_MemClear( cnt, sizeof(WAZA_AFF_COUNTER) );
}

static void AffCounter_CountUp( WAZA_AFF_COUNTER* cnt, BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BtlTypeAff affinity )
{
  u8 atkPokeID = BPP_GetID( attacker );
  u8 defPokeID = BPP_GetID( defender );

  u8 atkClientID = BTL_MAINUTIL_PokeIDtoClientID( atkPokeID );
  u8 defClientID = BTL_MAINUTIL_PokeIDtoClientID( defPokeID );

  u8 fEnemyAttack = !BTL_MAINUTIL_IsFriendPokeID( atkPokeID, defPokeID );

  u16* pCnt = NULL;

  // プレイヤーが攻撃側
  if( atkClientID == BTL_CLIENT_PLAYER )
  {
    if( fEnemyAttack )
    {
      if( affinity == BTL_TYPEAFF_0 )
      {
        pCnt = &(cnt->putVoid);
      }
      else if( affinity > BTL_TYPEAFF_100 )
      {
        pCnt = &(cnt->putAdvantage);
      }
      else if( affinity < BTL_TYPEAFF_100 )
      {
        pCnt = &(cnt->putDisadvantage);
      }
    }
  }
  // プレイヤーが防御側
  else if( defClientID == BTL_CLIENT_PLAYER )
  {
    if( fEnemyAttack )
    {
      if( affinity == BTL_TYPEAFF_0 )
      {
        pCnt = &(cnt->recvVoid);
      }
      else if( affinity > BTL_TYPEAFF_100 )
      {
        pCnt = &(cnt->recvAdvantage);
      }
      else if( affinity < BTL_TYPEAFF_100 )
      {
        pCnt = &(cnt->recvDisadvantage);
      }
    }
  }

  if( pCnt != NULL )
  {
    if( (*pCnt) < AFFCOUNTER_MAX ){
      ++(*pCnt);
    }
  }
}
/**
 *  攻撃相性カウンタ：「効果がなかった」ワザを出した回数
 */
u16 BTL_SVF_GetTypeAffCnt_PutVoid( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.putVoid;
}
/**
 *  攻撃相性カウンタ：「効果はバツグン」ワザを出した回数
 */
u16 BTL_SVF_GetTypeAffCnt_PutAdvantage( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.putAdvantage;
}
/**
 *  攻撃相性カウンタ：「効果はいまひとつ」ワザを出した回数
 */
u16 BTL_SVF_GetTypeAffCnt_PutDisadvantage( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.putDisadvantage;
}
/**
 *  攻撃相性カウンタ：「効果がなかった」ワザを受けた回数
 */
u16 BTL_SVF_GetTypeAffCnt_RecvVoid( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.recvVoid;
}
/**
 *  攻撃相性カウンタ：「効果はバツグン」ワザを受けた回数
 */
u16 BTL_SVF_GetTypeAffCnt_RecvAdvantage( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.recvAdvantage;
}
/**
 *  攻撃相性カウンタ：「効果はいまひとつ」ワザを受けた回数
 */
u16 BTL_SVF_GetTypeAffCnt_RecvDisadvantage( const BTL_SVFLOW_WORK* wk )
{
  return wk->affCounter.recvDisadvantage;
}


//----------------------------------------------------------------------------------------------------------
// 以下、ハンドラからの応答受信関数とユーティリティ群
//----------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール]指定IDのポケモンパラメータを返す
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  const BTL_POKEPARAM*
 */
//--------------------------------------------------------------------------------------
const BTL_POKEPARAM* BTL_SVFTOOL_GetPokeParam( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  return BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定ポケIDを持つポケモンが戦闘に出ているかチェックし、出ていたらその戦闘位置を返す
 *                    （ローテーションの場合は前衛のみ）
 *
 * @param   server
 * @param   pokeID
 *
 * @retval  BtlPokePos    出ている場合は戦闘位置ID／出ていない場合はBTL_POS_MAX
 */
//--------------------------------------------------------------------------------------
BtlPokePos BTL_SVFTOOL_GetExistFrontPokePos( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, pokeID );
  if( BTL_MAIN_IsFrontPos(wk->mainModule, pos) ){
    return pos;
  }
  return BTL_POS_NULL;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定ポケIDを持つポケモンが最後に出ているた位置IDを返す
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos
 */
//--------------------------------------------------------------------------------------
BtlPokePos BTL_SVFTOOL_GetPokeLastPos( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  return BTL_POSPOKE_GetPokeLastPos( &wk->pospokeWork, pokeID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定位置に存在しているポケIDを返す（存在していない場合は BTL_POKEID_NULL）
 *
 * @param   wk
 * @param   pos
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetExistPokeID( BTL_SVFLOW_WORK* wk, BtlPokePos pos )
{
  return BTL_POSPOKE_GetExistPokeID( &wk->pospokeWork, pos );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 場に出ている全ての相手側ポケモンIDを配列に格納する
 *
 * @param   wk
 * @param   dst
 *
 * @retval  u8    場に出ているポケモン数
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetAllOpponentFrontPokeID( BTL_SVFLOW_WORK* wk, u8 basePokeID, u8* dst )
{
  FRONT_POKE_SEEK_WORK fps;
  BTL_POKEPARAM* bpp;
  u8 cnt = 0;

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
  {
    if( !BTL_MAINUTIL_IsFriendPokeID( basePokeID, BPP_GetID(bpp) ) )
    {
      dst[ cnt++ ] = BPP_GetID( bpp );
    }
  }
  return cnt;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定のとくせいを持つポケモンが戦闘に出ているかチェック
 *
 * @param   wk
 * @param   tokusei
 *
 * @retval  BOOL    出ていたらTRUE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_CheckExistTokuseiPokemon( BTL_SVFLOW_WORK* wk, PokeTokusei tokusei )
{
  FRONT_POKE_SEEK_WORK fps;
  BTL_POKEPARAM* bpp;
  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
  {
    if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == tokusei )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 *  * [ハンドラ用ツール] 相性計算シミュレート結果を返す
 *
 * @param   wk
 * @param   atkPokeID
 * @param   defPokeID
 * @param   waza
 *
 * @retval  BtlTypeAff
 */
//--------------------------------------------------------------------------------------
BtlTypeAff BTL_SVFTOOL_SimulationAffinity( BTL_SVFLOW_WORK* wk, u8 atkPokeID, u8 defPokeID, WazaID waza )
{
  BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, atkPokeID );
  BTL_POKEPARAM* defender = BTL_POKECON_GetPokeParam( wk->pokeCon, defPokeID );
  SVFL_WAZAPARAM  wazaParam;
  BtlTypeAff result;

  scEvent_GetWazaParam( wk, waza, attacker, &wazaParam );
  result = scProc_checkWazaDamageAffinity( wk, attacker, defender, &wazaParam, FALSE );

  return result;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] ダメージ計算シミュレート結果を返す
 *
 * @param   flowWk
 * @param   atkPokeID       攻撃ポケID
 * @param   defPokeID       防御ポケID
 * @param   waza            ワザ
 * @param   fAffinity       相性計算するか（FALSEなら等倍で計算）
 * @param   fEnableRand     乱数有効
 *
 * @retval  u32
 */
//--------------------------------------------------------------------------------------
u32 BTL_SVFTOOL_SimulationDamage( BTL_SVFLOW_WORK* wk, u8 atkPokeID, u8 defPokeID, WazaID waza, BOOL fAffinity, BOOL fEnableRand )
{
  if( ( waza != WAZANO_NULL )
  &&  ( WAZADATA_IsDamage(waza) )
  ){
    const BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, atkPokeID );
    const BTL_POKEPARAM* defender = BTL_POKECON_GetPokeParam( wk->pokeCon, defPokeID );

    BtlTypeAff  aff;
    BOOL  critical_flag;
    SVFL_WAZAPARAM  wazaParam;
    u16  damage;

    wk->simulationCounter++;

    //
    if( fAffinity ){
      aff = BTL_SVFTOOL_SimulationAffinity( wk, atkPokeID, defPokeID, waza );
    }else{
      aff = BTL_TYPEAFF_100;
    }

    scEvent_GetWazaParam( wk, waza, attacker, &wazaParam );

    scEvent_CalcDamage( wk, attacker, defender, &wazaParam, aff,
      BTL_CALC_DMG_TARGET_RATIO_NONE, FALSE, !fEnableRand, &damage );

    wk->simulationCounter--;


    return damage;
  }
  else{
    return 0;
  }
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] シミュレーション用呼び出しかどうかを判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsSimulationMode( BTL_SVFLOW_WORK* wk )
{
  return wk->simulationCounter != 0;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] ペラップボイスランク取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  u32
 */
//--------------------------------------------------------------------------------------
u32 BTL_SVFTOOL_GetPerappVoicePower( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
  return BTL_MAIN_GetPerappVoicePower( wk->mainModule, clientID );
}

//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 自クライアントが担当している戦闘位置の数を返す
 *
 * @param   wk
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetClientCoverPosCount( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  SVCL_WORK* clwk;
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );

  clwk = BTL_SERVER_GetClientWork( wk->server, clientID );
  return clwk->numCoverPos;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 自クライアントパーティ内、控えの開始Indexを取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetMyBenchIndex( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_ROTATION ){
    return BTL_SVFTOOL_GetClientCoverPosCount( wk, pokeID );
  }
  else{
    return BTL_ROTATION_FRONTPOS_NUM + BTL_ROTATION_BACKPOS_NUM;
  }
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 自クライアントパーティに、（戦闘可能な）控えのポケモンがいるか
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsExistBenchPoke( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_PARTY* party = BTL_SVFTOOL_GetPartyData( wk, pokeID );
  u8 startIdx = BTL_SVFTOOL_GetMyBenchIndex( wk, pokeID );
  if( BTL_PARTY_GetAliveMemberCountRear(party, startIdx) ){
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 既にメンバー入れ替えアクションを実行しようとしているポケモンがいるか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_ReserveMemberChangeAction( BTL_SVFLOW_WORK* wk )
{
  if( wk->fMemberChangeReserve ){
    return FALSE;
  }
  wk->fMemberChangeReserve = TRUE;
  return TRUE;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 該当位置にいる生存しているポケモンIDを配列に格納＆数を返す
 *
 * @param   wk
 * @param   exPos
 * @param   dst_pokeID    [out]格納先配列
 *
 * @retval  u8    格納した数
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_ExpandPokeID( BTL_SVFLOW_WORK* wk, BtlExPos exPos, u8* dst_pokeID )
{
  u8 pos[ BTL_POS_MAX ];
  BTL_POKEPARAM* bpp;
  u8 max, num, i;

  max = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, pos );

  for(i=0, num=0; i<max; ++i)
  {
    bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, pos[i] );
    if( IsBppExist(bpp) ){
      dst_pokeID[num++] = BPP_GetID( bpp );
    }
  }
  return num;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定ポケモンを含むパーティデータを取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  const BTL_PARTY*
 */
//--------------------------------------------------------------------------------------
const BTL_PARTY* BTL_SVFTOOL_GetPartyData( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
  return BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] マルチ時、味方クライアントのパーティデータ取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  const BTL_PARTY*  味方クライアントのパーティデータ（非マルチ時はNULL）
 */
//--------------------------------------------------------------------------------------
const BTL_PARTY* BTL_SVFTOOL_GetFriendPartyData( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );
  u8 friendClientID = BTL_MAIN_GetFriendCleintID( wk->mainModule, clientID );
  if( BTL_MAIN_IsExistClient(wk->mainModule, friendClientID) )
  {
    return BTL_POKECON_GetPartyDataConst( wk->pokeCon, friendClientID );
  }
  return NULL;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定ポケモンの戦闘位置を返す
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos
 */
//--------------------------------------------------------------------------------------
BtlPokePos BTL_SVFTOOL_PokeIDtoPokePos( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  return BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定位置のポケモンIDを返す
 *
 * @param   wk
 * @param   pokePos
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_PokePosToPokeID( BTL_SVFLOW_WORK* wk, u8 pokePos )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pokePos );
  return BPP_GetID( bpp );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定ポケモンが「ゆびをふる」など他のワザを呼び出した際、ターゲットを自動で決める
 * ※ターゲット選択の必要が無いワザの場合、BTL_POS_NULL が返る
 *
 * @param   wk
 * @param   pokeID
 * @param   waza
 *
 * @retval  BtlPokePos
 */
//--------------------------------------------------------------------------------------
BtlPokePos BTL_SVFTOOL_ReqWazaTargetAuto( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  BtlPokePos targetPos;

  targetPos = BTL_CALC_DecideWazaTargetAuto( wk->mainModule, wk->pokeCon, bpp, waza );
  return targetPos;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 現ターンに選択されたアクション内容を取得
 *
 * @param   wk
 * @param   pokeID
 * @param   dst       [out]
 *
 * @retval  BOOL    正しく取得できたらTRUE（現ターン、参加していないポケなどが指定されたらFALSE）
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_GetThisTurnAction( BTL_SVFLOW_WORK* wk, u8 pokeID, BTL_ACTION_PARAM* dst )
{
  u32 i;
  for(i=0; i<wk->numActOrder; ++i)
  {
    if( BPP_GetID(wk->actOrder[i].bpp) == pokeID ){
      *dst = wk->actOrder[i].action;
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 自分のアクションが全体の何番目かを取得
 *
 * @param   wk
 * @param   pokeID
 * @param   myOrder       [out] 自分の順位
 * @param   totalAction   [out] 全アクション数
 *
 * @retval  BOOL    正しく取得できたらTRUE（現ターン、参加していないポケなどが指定されたらFALSE）
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_GetMyActionOrder( BTL_SVFLOW_WORK* wk, u8 pokeID, u8* myOrder, u8* totalAction )
{
  u32 i;
  for(i=0; i<wk->numActOrder; ++i)
  {
    if( BPP_GetID(wk->actOrder[i].bpp) == pokeID ){
      *myOrder = i;
      *totalAction = wk->numActOrder;
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 直前に出されたワザを取得
 *
 * @param   wk
 *
 * @retval  WazaID
 */
//--------------------------------------------------------------------------------------
WazaID BTL_SVFTOOL_GetPrevExeWaza( BTL_SVFLOW_WORK* wk )
{
  return wk->prevExeWaza;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 全ハンドラ共有のテンポラリワーク先頭アドレスを返す
 *
 * @param   wk
 *
 * @retval  void*
 */
//--------------------------------------------------------------------------------------
void* BTL_SVFTOOL_GetTmpWork( BTL_SVFLOW_WORK* wk, u32 size )
{
  GF_ASSERT(size < sizeof(wk->handlerTmpWork) );
  return wk->handlerTmpWork;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 出たワザレコードのポインタを取得
 *
 * @param   wk
 *
 * @retval  const BTL_WAZAREC*
 */
//--------------------------------------------------------------------------------------
const BTL_WAZAREC* BTL_SVFTOOL_GetWazaRecord( BTL_SVFLOW_WORK* wk )
{
  return &wk->wazaRec;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] Que領域予約
 *
 * @param   wk
 *
 * @retval  const BTL_WAZAREC*
 */
//--------------------------------------------------------------------------------------
u16 BTL_SVFTOOL_ReserveQuePos( BTL_SVFLOW_WORK* wk, ServerCmd cmd )
{
  return SCQUE_RESERVE_Pos( wk->que, cmd );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 死亡ポケレコードのポインタを取得
 *
 * @param   wk
 *
 * @retval  const BTL_DEADREC*
 */
//--------------------------------------------------------------------------------------
const BTL_DEADREC* BTL_SVFTOOL_GetDeadRecord( BTL_SVFLOW_WORK* wk )
{
  return &wk->deadRec;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 現在の経過ターン数を取得
 *
 * @param   wk
 *
 * @retval  u16
 */
//--------------------------------------------------------------------------------------
u16 BTL_SVFTOOL_GetTurnCount( BTL_SVFLOW_WORK* wk )
{
  return wk->turnCount;
}
//--------------------------------------------------------------------------------------
/**
 * メンバー交替カウンタの値を取得
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetPokeChangeCount( BTL_SVFLOW_WORK* wk, u8 clientID )
{
  return wk->memberChangeCount[ clientID ];
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] ポケモンが場にいるか
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsExistPokemon( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  return BTL_POSPOKE_IsExist( &wk->pospokeWork, pokeID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] バトルルール取得
 *
 * @param   wk
 *
 * @retval  BtlRule
 */
//--------------------------------------------------------------------------------------
BtlRule BTL_SVFTOOL_GetRule( BTL_SVFLOW_WORK* wk )
{
  return BTL_MAIN_GetRule( wk->mainModule );
}
//--------------------------------------------------------------------------------------
/**
 *  [ハンドラ用ツール] 対戦者タイプを取得
 *
 * @param   wk
 *
 * @retval  BtlCompetitor
 */
//--------------------------------------------------------------------------------------
BtlCompetitor BTL_SVFTOOL_GetCompetitor( BTL_SVFLOW_WORK* wk )
{
  return BTL_MAIN_GetCompetitor( wk->mainModule );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 戦闘地形タイプを取得
 *
 * @param   wk
 *
 * @retval  BtlBgType
 */
//--------------------------------------------------------------------------------------
BtlBgAttr BTL_SVFTOOL_GetLandForm( BTL_SVFLOW_WORK* wk )
{
  const BTL_FIELD_SITUATION* sit = BTL_MAIN_GetFieldSituation( wk->mainModule );
  return sit->bgAttr;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 前衛位置の数を返す
 *
 * @param   wk
 *
 * @retval  u32
 */
//--------------------------------------------------------------------------------------
u32 BTL_SVFTOOL_GetFrontPosNum( BTL_SVFLOW_WORK* wk )
{
  return BTL_MAIN_GetFrontPosNum( wk->mainModule );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] マルチモードかどうかチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsMultiMode( BTL_SVFLOW_WORK* wk )
{
  return BTL_MAIN_IsMultiMode( wk->mainModule );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 進化前ポケモンかどうかチェック
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_CheckSinkaMae( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  u16 monsno = BPP_GetMonsNo( bpp );
  u16 i;
  for(i=SHINKA_COND_START; i<SHINKA_COND_MAX; ++i){
    if( SHINKA_GetParamByHandle(wk->sinkaArcHandle, monsno, 0, i) ){
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 素早さ計算
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  u16
 */
//--------------------------------------------------------------------------------------
u16 BTL_SVFTOOL_CalcAgility( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp, BOOL fTrickRoomEnable )
{
  return scEvent_CalcAgility( wk, bpp, fTrickRoomEnable );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 素早さ計算->場に出ているポケモン内でのランクを返す
 *
 * @param   wk
 * @param   bpp_target
 *
 * @retval  u16 ランク（0～）
 */
//--------------------------------------------------------------------------------------
u16 BTL_SVFTOOL_CalcAgilityRank( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp_target, BOOL fTrickRoomEnable )
{
  FRONT_POKE_SEEK_WORK fps;
  BTL_POKEPARAM* bpp;
  u16 rank, agi, target_agi;
  u8 target_pokeID;

  target_agi = BTL_SVFTOOL_CalcAgility( wk, bpp_target, fTrickRoomEnable );
  target_pokeID = BPP_GetID( bpp_target );
  rank = 0;

  FRONT_POKE_SEEK_InitWork( &fps, wk );
  while( FRONT_POKE_SEEK_GetNext( &fps, wk, &bpp ) )
  {
    if( BPP_GetID(bpp) == target_pokeID ){
      continue;
    }
    agi = BTL_SVFTOOL_CalcAgility( wk, bpp, fTrickRoomEnable );
    if( agi > target_agi ){
      ++rank;
    }
  }

  return rank;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] “浮いている”状態チェック
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsFloatingPoke( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  return scproc_CheckFloating( wk, bpp, TRUE );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] アイテム利用可否チェック
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_CheckItemUsable( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );

  if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_BUKIYOU ){
    return FALSE;
  }
  if( BPP_CheckSick(bpp, WAZASICK_SASIOSAE) ){
    return FALSE;
  }
  if( BTL_FIELD_CheckEffect(BTL_FLDEFF_MAGICROOM) )
  {
    return FALSE;
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 天候取得
 *
 * @param   wk
 *
 * @retval  BtlWeather
 */
//--------------------------------------------------------------------------------------
BtlWeather BTL_SVFTOOL_GetWeather( BTL_SVFLOW_WORK* wk )
{
  return scEvent_GetWeather( wk );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] トリプルで外れるワザチェック
 *
 * @param   wk
 * @param   atkPokeID
 * @param   defPokeID
 * @param   waza
 *
 * @retval  BOOL    トリプルバトルの時、かつ外れる位置のワザならTRUE／それ以外FALSE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_CheckFarPoke( BTL_SVFLOW_WORK* wk, u8 atkPokeID, u8 defPokeID, WazaID waza )
{
  const BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, atkPokeID );
  const BTL_POKEPARAM* defender = BTL_POKECON_GetPokeParam( wk->pokeCon, defPokeID );
  return  IsTripleFarPos( wk, attacker, defender, waza );
}

//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] デバッグフラグ取得
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  u16
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_GetDebugFlag( BTL_SVFLOW_WORK* wk, BtlDebugFlag flag )
{
  return BTL_MAIN_GetDebugFlag( wk->mainModule, flag );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定サイドエフェクトが働いているか判定
 *
 * @param   wk
 * @param   BtlPokePos      pos
 * @param   BtlSideEffect
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsExistSideEffect( BTL_SVFLOW_WORK* wk, BtlSide side, BtlSideEffect sideEffect )
{
  return BTL_HANDER_SIDE_IsExist( side, sideEffect );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定サイドエフェクトのカウンタを取得
 *
 * @param   wk
 * @param   BtlPokePos      pos
 * @param   BtlSideEffect
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
u32 BTL_SVFTOOL_GetSideEffectCount( BTL_SVFLOW_WORK* wk, BtlPokePos pos, BtlSideEffect sideEffect )
{
  BtlSide side = BTL_MAINUTIL_PosToSide( pos );
  return BTL_HANDLER_SIDE_GetAddCount( side, sideEffect );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定サイドエフェクトのカウンタを取得
 *
 * @param   wk
 * @param   pos
 * @param   effect
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsExistPosEffect( BTL_SVFLOW_WORK* wk, BtlPokePos pos, BtlPosEffect effect )
{
  return BTL_HANDLER_POS_IsRegistered( effect, pos );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 指定クライアントの味方クライアントIDを返す（味方が居ない時は BTL_CLIENTID_NULL）
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  u8
 */
//--------------------------------------------------------------------------------------
u8 BTL_SVFTOOL_GetFriendClientID( BTL_SVFLOW_WORK* wk, u8 clientID )
{
  return BTL_MAIN_GetFriendCleintID( wk->mainModule, clientID );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] メンバー入れ替え時の割り込みアクション解決中であるかチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsMemberOutIntr( BTL_SVFLOW_WORK* wk )
{
  return wk->fMemberOutIntr;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] メンバー入れ替え時の割り込みリクエスト
 *
 * @param   wk
 * @param   pokeID    割り込みたいポケID
 */
//--------------------------------------------------------------------------------------
void BTL_SVFTOOL_AddMemberOutIntr( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  if( wk->MemberOutIntrPokeCount < NELEMS(wk->MemberOutIntrPokeID) )
  {
    wk->MemberOutIntrPokeID[ wk->MemberOutIntrPokeCount++ ] = pokeID;
  }
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] そらをとぶ・ダイビング・フリーフォール等で画面から消えている状態のポケモンかどうか判定
 *
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsTameHidePoke( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  return BPP_CONTFLAG_CheckWazaHide( bpp ) != BPP_CONTFLG_NULL;
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] フリーフォール実行側のポケモンかどうか判定
 *
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsFreeFallUserPoke( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  return checkFreeFallUsing( bpp );
}
//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] フリーフォール実行側・または捕まれている側のポケモンかどうか判定
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SVFTOOL_IsFreeFallPoke( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  if( BTL_SVFTOOL_IsFreeFallUserPoke(wk, pokeID) ){
    return TRUE;
  }
  else{
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
    if( BPP_CheckSick(bpp, WAZASICK_FREEFALL) ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------
/**
 * [ハンドラ用ツール] 体重取得
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  u32
 */
//--------------------------------------------------------------------------------------
u32 BTL_SVFTOOL_GetWeight( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );

  fx32 ratio = svEvent_GetWaitRatio( wk, bpp );
  u32  weight = BPP_GetWeight( bpp );

  weight = BTL_CALC_MulRatio( weight, ratio );
  if( weight < BTL_POKE_WEIGHT_MIN ){
    weight = BTL_POKE_WEIGHT_MIN;
  }
  return weight;
}


//=============================================================================================
/**
 * [ハンドラからの操作呼び出し]
 *
 * @param   wk
 * @param   effIndex
 */
//=============================================================================================
void BTL_SVFRET_SetWazaEffectIndex( BTL_SVFLOW_WORK* wk, u8 effIndex )
{
  wazaEffCtrl_SetEffectIndex( wk->wazaEffCtrl, effIndex );
}
//=============================================================================================
/**
 * [ハンドラからのシステム呼び出し]
 *  スタンドアローンバトルで、プレイヤーポケモンの使用したハンドラから呼び出された場合のみ
 *  おこづかいを上乗せする（ネコにこばん専用）
 *
 * @param   wk
 * @param   volume    上乗せ額
 * @param   pokeID    ワザ使用ポケモンID
 *
 * @retval  BOOL      条件を満たした（上乗せした）場合にTRUEが返る
 */
//=============================================================================================
BOOL BTL_SVFRET_AddBonusMoney( BTL_SVFLOW_WORK* wk, u32 volume, u8 pokeID )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk->mainModule );

  if( ((competitor == BTL_COMPETITOR_WILD) || (competitor == BTL_COMPETITOR_TRAINER))
  ){
    BTL_SERVER_AddBonusMoney( wk->server, volume );
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * [ハンドラからのシステム呼び出し]
 * スタンドアロンバトルで、終了時にもらえるお金を倍にする（効果は１度だけ）
 *
 * @param   wk
 * @param   pokeID
 */
//=============================================================================================
void BTL_SVFRET_SetMoneyDblUp( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk->mainModule );
  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( pokeID );

  if( ((competitor == BTL_COMPETITOR_WILD)||(competitor == BTL_COMPETITOR_TRAINER))
  &&  (clientID == BTL_MAIN_GetPlayerClientID(wk->mainModule))
  ){
    BTL_SERVER_SetMoneyDblUp( wk->server );
  }
}
//=============================================================================================
/**
 * [ハンドラからの操作呼び出し]  マジックコート跳ね返し登録
 *
 * @param   wk
 * @param   pokeID          マジックコートを使った側のポケモンID
 * @param   targetPokeID    跳ね返す先のポケモンID
 */
//=============================================================================================
void BTL_SVFRET_AddMagicCoatAction( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 targetPokeID )
{
  BtlPokePos targetPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, targetPokeID );
  wazaRobParam_Add( wk->magicCoatParam, pokeID, targetPokeID, targetPos );
}
//=============================================================================================
/**
 * [ハンドラからの操作呼び出し]  フリーフォール溜めターン処理
 *
 * @param   wk
 * @param   atkPokeID
 * @param   targetPokeID
 *
 * @retval  BOOL    成功すればTRUE
 */
//=============================================================================================
BOOL BTL_SVFRET_FreeFallStart( BTL_SVFLOW_WORK* wk, u8 atkPokeID, u8 targetPokeID, BOOL* fFailMsgDisped )
{
  BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, atkPokeID );
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, targetPokeID );

  return scproc_FreeFall_Start( wk, attacker, target, fFailMsgDisped );
}
//=============================================================================================
/**
 * [ハンドラからの操作呼び出し]  フリーフォール解放ターン処理
 *
 * @param   wk
 * @param   atkPokeID
 * @param   targetPokeID
 *
 * @retval  BOOL    成功すればTRUE
 */
//=============================================================================================
void BTL_SVFRET_FreeFallRelease( BTL_SVFLOW_WORK* wk, u8 atkPokeID )
{
  BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, atkPokeID );

  scproc_FreeFall_CheckRelease( wk, attacker, TRUE );
}
//=============================================================================================
/**
 * [btl_server_flow_subから操作呼び出し]  装備アイテム効果処理
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_SVFRET_UseItemEquip( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp )
{
  return scproc_UseItemEquip( wk, bpp );
}
//=============================================================================================
/**
 * [btl_server_flow_subから操作呼び出し]  メンバー入場イベント呼び出し
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
void BTL_SVFRET_Event_AfterMemberIn( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* bpp )
{
  scEvent_AfterMemberIn( wk, bpp );
}



//=============================================================================================
/**
 * 戦闘結果判定処理
 *
 * @param   wk
 *
 * @retval  BtlResult
 */
//=============================================================================================
BtlResult BTL_SVFLOW_ChecBattleResult( BTL_SVFLOW_WORK* wk )
{
  return BTL_SVFSUB_CheckBattleResult( wk );
}


//---------------------------------------------------------------------------------------------
// 当ターン生き返りポケレコード
//---------------------------------------------------------------------------------------------

void BTL_SVFRET_AddRelivePokeRecord( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  relivePokeRec_Add( wk, pokeID );
}

// 初期化
static void relivePokeRec_Init( BTL_SVFLOW_WORK* wk )
{
  wk->numRelivePoke = 0;
}

// 生き返り記録追加
static void relivePokeRec_Add( BTL_SVFLOW_WORK* wk, u8 pokeID )
{
  u32 i;

  for(i=0; i<wk->numRelivePoke; ++i)
  {
    if( wk->relivedPokeID[i] == pokeID ){
      return;
    }
  }

  if( i < NELEMS(wk->relivedPokeID) )
  {
    wk->relivedPokeID[i] = pokeID;
    wk->numRelivePoke++;
  }
}

// ターン終了時、ポケモン入場の必要があるか判定
static BOOL relivePokeRec_CheckNecessaryPokeIn( BTL_SVFLOW_WORK* wk )
{
  u32 i;
  u8 pos[ BTL_POSIDX_MAX ];
  u8 clientID;

  for(i=0; i<wk->numRelivePoke; ++i)
  {
    clientID = BTL_MAINUTIL_PokeIDtoClientID( wk->relivedPokeID[i] );

    BTL_N_Printf( DBGSTR_SVFL_RelivePokeStart, clientID);

    // １個所でも空き位置があれば入場させる必要アリ
    if( BTL_POSPOKE_GetClientEmptyPos(&wk->pospokeWork, clientID, pos) ){
      BTL_N_Printf( DBGSTR_SVFL_ReliveEndIn );
      return TRUE;
    }
    BTL_N_Printf( DBGSTR_SVFL_ReliveEndIgnore );
  }
  return FALSE;
}

/*--------------------------------------------------------------------------------------------*/
/* POKESET Stack : Action毎にPush,Popして各用途のPokeSetを個別に用意、利用するための仕組み    */
/*--------------------------------------------------------------------------------------------*/
// 初期化
static void PSetStack_Init( BTL_SVFLOW_WORK* wk )
{
  wk->pokesetStackPtr = 0;
  psetstack_setup( wk, 0, TRUE );
}
// Push
static void PSetStack_Push( BTL_SVFLOW_WORK* wk )
{
  if( wk->pokesetStackPtr < (NELEMS(wk->pokesetUnit)-1) )
  {
    wk->pokesetStackPtr++;
    psetstack_setup( wk, wk->pokesetStackPtr, TRUE );
  }
  else{
    GF_ASSERT(0);
  }
}
// Pop
static void PSetStack_Pop( BTL_SVFLOW_WORK* wk )
{
  if( wk->pokesetStackPtr )
  {
    wk->pokesetStackPtr--;
    psetstack_setup( wk, wk->pokesetStackPtr, FALSE );
  }
  else
  {
    GF_ASSERT(0);
  }
}
// 下請けカレント要素クリア
static void psetstack_setup( BTL_SVFLOW_WORK* wk, u32 sp, BOOL fClear )
{
  POKESET_STACK_UNIT* unit = &(wk->pokesetUnit[ sp ]);

  wk->psetTargetOrg  = &unit->TargetOrg;
  wk->psetTarget     = &unit->Target;
  wk->psetFriend     = &unit->Friend;
  wk->psetEnemy      = &unit->Enemy;
  wk->psetDamaged    = &unit->Damaged;
  wk->psetRobTarget  = &unit->RobTarget;
  wk->psetTmp        = &unit->psetTmp;
  wk->wazaEffCtrl    = &unit->effCtrl;
  wk->wazaParam      = &unit->wazaParam;
  wk->wazaParamOrg   = &unit->wazaParamOrg;
  wk->hitCheckParam  = &unit->hitCheck;
  wk->wazaRobParam   = &unit->wazaRobParam;
  wk->magicCoatParam = &unit->magicCoatParam;
  wk->calcDmgFriend  = &unit->calcDmgFriend;
  wk->calcDmgEnemy   = &unit->calcDmgEnemy;
  wk->defaultTargetPos = unit->defaultTargetPos;
  wk->fMemberChangeReserve = unit->fMemberChangeReserve;

  if( fClear ){
    BTL_POKESET_Clear( wk->psetTargetOrg );
    BTL_POKESET_Clear( wk->psetTarget );
    BTL_POKESET_Clear( wk->psetFriend );
    BTL_POKESET_Clear( wk->psetEnemy );
    BTL_POKESET_Clear( wk->psetDamaged );
    BTL_POKESET_Clear( wk->psetRobTarget );
    BTL_POKESET_Clear( wk->psetTmp );
    wazaEffCtrl_Init( wk->wazaEffCtrl );
    GFL_STD_MemClear( wk->wazaParam, sizeof(*(wk->wazaParam)) );
    GFL_STD_MemClear( wk->wazaParamOrg, sizeof(*(wk->wazaParamOrg)) );
    GFL_STD_MemClear( wk->hitCheckParam, sizeof(*(wk->hitCheckParam)) );
    GFL_STD_MemClear( wk->wazaRobParam, sizeof(*(wk->wazaRobParam)) );
    GFL_STD_MemClear( wk->magicCoatParam, sizeof(*(wk->magicCoatParam)) );
    GFL_STD_MemClear( wk->calcDmgFriend, sizeof(*(wk->calcDmgFriend)) );
    GFL_STD_MemClear( wk->calcDmgEnemy, sizeof(*(wk->calcDmgEnemy)) );

    wk->defaultTargetPos = BTL_POS_NULL;
    wk->fMemberChangeReserve = 0;
  }
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

//=============================================================================================
/**
 * HandEx ワーク１件取得
 *
 * @param   wk
 * @param   eq_type
 * @param   userPokeID
 *
 * @retval  void*
 */
//=============================================================================================
void* BTL_SVF_HANDEX_Push( BTL_SVFLOW_WORK* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID )
{
  return BTL_Hem_PushWork( &wk->HEManager, eq_type, userPokeID );
}
void BTL_SVF_HANDEX_PushRun( BTL_SVFLOW_WORK* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID )
{
  void* work = BTL_SVF_HANDEX_Push( wk, eq_type, userPokeID );
  BTL_SVF_HANDEX_Pop( wk, work );
}

//=============================================================================================
/**
 * HandEx ワーク１件実行
 *
 * @param   wk
 * @param   eq_type
 * @param   userPokeID
 *
 * @retval  void
 */
//=============================================================================================
void BTL_SVF_HANDEX_Pop( BTL_SVFLOW_WORK* wk, void* handExWork )
{
  HandEx_Exe( wk, (BTL_HANDEX_PARAM_HEADER*)handExWork );
  BTL_Hem_PopWork( &wk->HEManager, handExWork );
}


//----------------------------------------------------------------------------------
/**
 * イベント実行結果を取得
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static HandExResult scproc_HandEx_Result( BTL_SVFLOW_WORK* wk )
{
  if( BTL_Hem_IsUsed(&wk->HEManager) )
  {
    BOOL fSucceed = BTL_Hem_GetTotalResult( &wk->HEManager );
    return (fSucceed)? HandExResult_Enable : HandExResult_Fail;
  }

  return HandExResult_NULL;
}
HandExResult BTL_SVF_HandEx_Result( BTL_SVFLOW_WORK* wk )
{
  return scproc_HandEx_Result( wk );
}

static void HandEx_Exe( BTL_SVFLOW_WORK* wk, BTL_HANDEX_PARAM_HEADER* handEx_header )
{
  u16 useItemID = BTL_Hem_GetUseItemNo( &wk->HEManager );
  u8 fPrevSucceed;

  if( BTL_Hem_IsUsed(&wk->HEManager) ){
    fPrevSucceed = BTL_Hem_GetPrevResult(&wk->HEManager);
  }else{
    fPrevSucceed = TRUE;
  }

  if( handEx_header->failSkipFlag && (fPrevSucceed == FALSE) ){
    BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_SVFL_HandExContFail );
    return;
  }
  if( handEx_header->autoRemoveFlag ){
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParamConst( wk->pokeCon, handEx_header->userPokeID );
    if( BPP_IsDead(bpp) ){
      BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_SVFL_HandExContDead );
      return;
    }
  }

  switch( handEx_header->equip ){
  case BTL_HANDEX_USE_ITEM_EFFECT:    fPrevSucceed = scproc_HandEx_UseItemAct( wk, handEx_header ); break;
  case BTL_HANDEX_TOKWIN_IN:          fPrevSucceed = scproc_HandEx_TokWinIn( wk, handEx_header ); break;
  case BTL_HANDEX_TOKWIN_OUT:         fPrevSucceed = scproc_HandEx_TokWinOut( wk, handEx_header ); break;
  case BTL_HANDEX_USE_ITEM:           fPrevSucceed = scproc_HandEx_useItem( wk, handEx_header ); break;
  case BTL_HANDEX_RECOVER_HP:         fPrevSucceed = scproc_HandEx_recoverHP( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_DRAIN:              fPrevSucceed = scproc_HandEx_drain( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_DAMAGE:             fPrevSucceed = scproc_HandEx_damage( wk, handEx_header ); break;
  case BTL_HANDEX_SHIFT_HP:           fPrevSucceed = scproc_HandEx_shiftHP( wk, handEx_header ); break;
  case BTL_HANDEX_RECOVER_PP:         fPrevSucceed = scproc_HandEx_recoverPP( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_DECREMENT_PP:       fPrevSucceed = scproc_HandEx_decrementPP( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_CURE_SICK:          fPrevSucceed = scproc_HandEx_cureSick( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_ADD_SICK:           fPrevSucceed = scproc_HandEx_addSick( wk, handEx_header ); break;
  case BTL_HANDEX_RANK_EFFECT:        fPrevSucceed = scproc_HandEx_rankEffect( wk, handEx_header, useItemID ); break;
  case BTL_HANDEX_SET_RANK:           fPrevSucceed = scproc_HandEx_setRank( wk, handEx_header ); break;
  case BTL_HANDEX_RECOVER_RANK:       fPrevSucceed = scproc_HandEx_recoverRank( wk, handEx_header ); break;
  case BTL_HANDEX_RESET_RANK:         fPrevSucceed = scproc_HandEx_resetRank( wk, handEx_header ); break;
  case BTL_HANDEX_SET_STATUS:         fPrevSucceed = scproc_HandEx_setStatus( wk, handEx_header ); break;
  case BTL_HANDEX_KILL:               fPrevSucceed = scproc_HandEx_kill( wk, handEx_header ); break;
  case BTL_HANDEX_CHANGE_TYPE:        fPrevSucceed = scproc_HandEx_changeType( wk, handEx_header ); break;
  case BTL_HANDEX_MESSAGE:            fPrevSucceed = scproc_HandEx_message( wk, handEx_header ); break;
  case BTL_HANDEX_SET_TURNFLAG:       fPrevSucceed = scproc_HandEx_setTurnFlag( wk, handEx_header ); break;
  case BTL_HANDEX_RESET_TURNFLAG:     fPrevSucceed = scproc_HandEx_resetTurnFlag( wk, handEx_header ); break;
  case BTL_HANDEX_SET_CONTFLAG:       fPrevSucceed = scproc_HandEx_setContFlag( wk, handEx_header ); break;
  case BTL_HANDEX_RESET_CONTFLAG:     fPrevSucceed = scproc_HandEx_resetContFlag( wk, handEx_header ); break;
  case BTL_HANDEX_SIDEEFF_ADD:        fPrevSucceed = scproc_HandEx_sideEffectAdd( wk, handEx_header ); break;
  case BTL_HANDEX_SIDEEFF_REMOVE:     fPrevSucceed = scproc_HandEx_sideEffectRemove( wk, handEx_header ); break;
  case BTL_HANDEX_ADD_FLDEFF:         fPrevSucceed = scproc_HandEx_addFieldEffect( wk, handEx_header ); break;
  case BTL_HANDEX_CHANGE_WEATHER:     fPrevSucceed = scproc_HandEx_changeWeather( wk, handEx_header ); break;
  case BTL_HANDEX_REMOVE_FLDEFF:      fPrevSucceed = scproc_HandEx_removeFieldEffect( wk, handEx_header ); break;
  case BTL_HANDEX_POSEFF_ADD:         fPrevSucceed = scproc_HandEx_PosEffAdd( wk, handEx_header ); break;
  case BTL_HANDEX_CHANGE_TOKUSEI:     fPrevSucceed = scproc_HandEx_tokuseiChange( wk, handEx_header ); break;
  case BTL_HANDEX_SET_ITEM:           fPrevSucceed = scproc_HandEx_setItem( wk, handEx_header ); break;
  case BTL_HANDEX_CHECK_ITEM_EQUIP:   fPrevSucceed = scproc_HandEx_CheckItemEquip( wk, handEx_header ); break;
  case BTL_HANDEX_ITEM_SP:            fPrevSucceed = scproc_HandEx_ItemSP( wk, handEx_header ); break;
  case BTL_HANDEX_CONSUME_ITEM:       fPrevSucceed = scproc_HandEx_consumeItem( wk, handEx_header ); break;
  case BTL_HANDEX_SWAP_ITEM:          fPrevSucceed = scproc_HandEx_swapItem( wk, handEx_header ); break;
  case BTL_HANDEX_UPDATE_WAZA:        fPrevSucceed = scproc_HandEx_updateWaza( wk, handEx_header ); break;
  case BTL_HANDEX_COUNTER:            fPrevSucceed = scproc_HandEx_counter( wk, handEx_header ); break;
  case BTL_HANDEX_DELAY_WAZADMG:      fPrevSucceed = scproc_HandEx_delayWazaDamage( wk, handEx_header ); break;
  case BTL_HANDEX_QUIT_BATTLE:        fPrevSucceed = scproc_HandEx_quitBattle( wk, handEx_header ); break;
  case BTL_HANDEX_CHANGE_MEMBER:      fPrevSucceed = scproc_HandEx_changeMember( wk, handEx_header ); break;
  case BTL_HANDEX_BATONTOUCH:         fPrevSucceed = scproc_HandEx_batonTouch( wk, handEx_header ); break;
  case BTL_HANDEX_ADD_SHRINK:         fPrevSucceed = scproc_HandEx_addShrink( wk, handEx_header ); break;
  case BTL_HANDEX_RELIVE:             fPrevSucceed = scproc_HandEx_relive( wk, handEx_header ); break;
  case BTL_HANDEX_SET_WEIGHT:         fPrevSucceed = scproc_HandEx_setWeight( wk, handEx_header ); break;
  case BTL_HANDEX_PUSHOUT:            fPrevSucceed = scproc_HandEx_pushOut( wk, handEx_header ); break;
  case BTL_HANDEX_INTR_POKE:          fPrevSucceed = scproc_HandEx_intrPoke( wk, handEx_header ); break;
  case BTL_HANDEX_INTR_WAZA:          fPrevSucceed = scproc_HandEx_intrWaza( wk, handEx_header ); break;
  case BTL_HANDEX_SEND_LAST:          fPrevSucceed = scproc_HandEx_sendLast( wk, handEx_header ); break;
  case BTL_HANDEX_SWAP_POKE:          fPrevSucceed = scproc_HandEx_swapPoke( wk, handEx_header ); break;
  case BTL_HANDEX_HENSIN:             fPrevSucceed = scproc_HandEx_hensin( wk, handEx_header ); break;
  case BTL_HANDEX_FAKE_BREAK:         fPrevSucceed = scproc_HandEx_fakeBreak( wk, handEx_header ); break;
  case BTL_HANDEX_JURYOKU_CHECK:      fPrevSucceed = scproc_HandEx_juryokuCheck( wk, handEx_header ); break;
  case BTL_HANDEX_TAMEHIDE_CANCEL:    fPrevSucceed = scproc_HandEx_TameHideCancel( wk, handEx_header ); break;
  case BTL_HANDEX_ADD_EFFECT:         fPrevSucceed = scproc_HandEx_effectByPos( wk, handEx_header ); break;
  case BTL_HANDEX_CHANGE_FORM:        fPrevSucceed = scproc_HandEx_changeForm( wk, handEx_header ); break;
  case BTL_HANDEX_SET_EFFECT_IDX:     fPrevSucceed = scproc_HandEx_setWazaEffectIndex( wk, handEx_header ); break;
  case BTL_HANDEX_WAZAEFFECT_ENABLE:  fPrevSucceed = scproc_HandEx_setWazaEffectEnable( wk, handEx_header ); break;
  default:
    GF_ASSERT_MSG(0, "illegal handEx type = %d, userPokeID=%d", handEx_header->equip, handEx_header->userPokeID);
  }

  BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_SVFL_HandExSetResult, fPrevSucceed );
  BTL_Hem_SetResult( &wk->HEManager, fPrevSucceed );
}

/**
 * アイテム使用エフェクトのみ発動
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_UseItemAct( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param_header->userPokeID) )
  {
    BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
    scPut_UseItemAct( wk, pp_user );
    return 1;
  }
  return 0;

}
/**
 * とくせいウィンドウ表示
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_TokWinIn( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param_header->userPokeID) )
  {
    BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
    scPut_TokWin_In( wk, pp_user );
    return 1;
  }
  return 0;
}
/**
 * とくせいウィンドウ消去
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_TokWinOut( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
//  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param_header->userPokeID) )
  {
    BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
    scPut_TokWin_Out( wk, pp_user );
    return 1;
  }
  return 0;
}
/**
 * ポケモンHP回復
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_recoverHP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_RECOVER_HP* param = (const BTL_HANDEX_PARAM_RECOVER_HP*)param_header;

  u8 result = 0;
/// 道具使用による実行もあるので、場にいなくても有効
//  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
    BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

    if( !scproc_RecoverHP_CheckFailBase(wk, pp_target) )
    {
      if( param_header->tokwin_flag ){
        scPut_TokWin_In( wk, pp_user );
      }

      if( param->fFailCheckThru == FALSE )
      {
        result = !scproc_RecoverHP_CheckFailSP( wk, pp_target, TRUE );
      }
      else{
        result = TRUE;
      }

      if( result )
      {
        scproc_RecoverHP_Core( wk, pp_target, param->recoverHP );

        if( param->exStr.type != BTL_STRTYPE_NULL ){
            handexSub_putString( wk, &(param->exStr) );
        }else if( itemID != ITEM_DUMMY_DATA ){
          SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_UseItem_RecoverHP, param->pokeID, itemID );
        }
      }

      if( param_header->tokwin_flag ){
        scPut_TokWin_Out( wk, pp_user );
      }
    }
  }
  return result;
}
/**
 * ポケモンHP回復（ドレイン系）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_drain( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_DRAIN* param = (BTL_HANDEX_PARAM_DRAIN*)param_header;
  BTL_POKEPARAM* pp_damaged = NULL;
  if( param->damagedPokeID != BTL_POKEID_NULL ){
    pp_damaged = BTL_POKECON_GetPokeParam( wk->pokeCon, param->damagedPokeID );
  }

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->recoverPokeID) )
  {
    BTL_POKEPARAM* pp_recover = BTL_POKECON_GetPokeParam( wk->pokeCon, param->recoverPokeID );

    if( !BPP_IsDead(pp_recover) )
    {
      if( scproc_DrainCore(wk, pp_recover, pp_damaged, param->recoverHP) ){
        if( param->exStr.type != BTL_STRTYPE_NULL ){
          handexSub_putString( wk, &(param->exStr) );
        }
        return 1;
      }
    }
  }
  return 0;
}
/**
 * ポケモンにダメージ
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_damage( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_DAMAGE* param = (BTL_HANDEX_PARAM_DAMAGE*)param_header;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

    if( !BPP_IsDead(pp_target) )
    {
      if( (!(param->fAvoidHidePoke))
      ||  (BPP_CONTFLAG_CheckWazaHide(pp_target) == BPP_CONTFLG_NULL)
      ){
        if( scproc_SimpleDamage_CheckEnable(wk, pp_target, param->damage) )
        {
          if( param->fExEffect ){
            scproc_ViewEffect( wk, param->effectNo, param->pos_from, param->pos_to, FALSE, 0 );
          }
          scproc_SimpleDamage_Core(wk, pp_target, param->damage, &param->exStr );
          return 1;
        }
      }
    }
  }
  return 0;
}
/**
 * ポケモンHP増減（回復／ダメージと見なさない）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_shiftHP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_SHIFT_HP* param = (BTL_HANDEX_PARAM_SHIFT_HP*)param_header;
  u32 i;
  u8 result = 0;

  for(i=0; i<param->poke_cnt; ++i)
  {
    if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID[i]) )
    {
      BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID[i] );
      if( !BPP_IsDead(pp_target) ){
        scPut_SimpleHp( wk, pp_target, param->volume[i], !param->fEffectDisable );
        scproc_CheckItemReaction( wk, pp_target, BTL_ITEMREACTION_HP );
        result = 1;
      }
    }
  }
  return result;
}
/**
 * ポケモンPP回復
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_recoverPP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_PP* param = (BTL_HANDEX_PARAM_PP*)param_header;
  BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );

/// 道具使用による実行もあるので、場にいなくても有効
//  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

    if( BPP_IsFightEnable(pp_target) || (param->fDeadPokeEnable) )
    {
      BOOL fOrgWaza = (!(param->fSurfacePP));
      if( !BPP_WAZA_IsPPFull(pp_target, param->wazaIdx, fOrgWaza) )
      {
        scPut_RecoverPP( wk, pp_target, param->wazaIdx, param->volume, fOrgWaza );
        handexSub_putString( wk, &param->exStr );
        return 1;
      }
    }
  }
  return 0;
}
/**
 * ポケモンPP減少
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_decrementPP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_PP* param = (BTL_HANDEX_PARAM_PP*)param_header;

// 大爆発->おんねん があるので、場にいなくても有効
//  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( (!BPP_IsDead(target)) || (param->fDeadPokeEnable) )
    {
      if( scproc_decrementPP(wk, target, param->wazaIdx, param->volume) )
      {
        handexSub_putString( wk, &param->exStr );
        if( scEvent_DecrementPP_Reaction(wk, target, param->wazaIdx ) )
        {
          scproc_UseItemEquip( wk, target );
        }
      }
      return 1;
    }
  }
  return 0;
}
/**
 * ポケモン状態異常回復
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_cureSick( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_CURE_SICK* param = (BTL_HANDEX_PARAM_CURE_SICK*)param_header;
  BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
  u8 result = 0;

  if( param_header->tokwin_flag ){
    scPut_TokWin_In( wk, pp_user );
  }

  {
    u32 i;
    for(i=0; i<param->poke_cnt; ++i)
    {
      // 場にいるポケモンに限定してしまうと「アロマセラピー」などが効かない
//      if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID[i]) )
      {
        BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID[i] );
        if( BPP_IsFightEnable(pp_target) )
        {
          BtlWazaSickEx exCode;
          BPP_SICK_CONT oldCont;
          WazaSick check_sick;

          exCode = param->sickCode;
          while( (check_sick = trans_sick_code(pp_target, &exCode)) != WAZASICK_NULL )
          {
            scPut_CureSick( wk, pp_target, check_sick, &oldCont );
            if( !param->fStdMsgDisable )
            {
              if( BTL_SICK_MakeDefaultCureMsg(check_sick, oldCont, pp_target, itemID, &wk->strParam) ){
                handexSub_putString( wk, &wk->strParam );
              }
            }
            else
            {
              handexSub_putString( wk, &param->exStr );
            }
            if( check_sick == WAZASICK_SASIOSAE ){
              scproc_CheckItemReaction( wk, pp_target, BTL_ITEMREACTION_GEN );
            }
            result = 1;
          }
        }
      }
    }
  }

  if( param_header->tokwin_flag ){
    scPut_TokWin_Out( wk, pp_user );
  }

  return result;
}
/**
 * ポケモン状態異常化
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_addSick( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_ADD_SICK* param = (BTL_HANDEX_PARAM_ADD_SICK*)param_header;
  BTL_POKEPARAM* pp_user = (param_header->userPokeID != BTL_POKEID_NULL)?
                      BTL_POKECON_GetPokeParam(wk->pokeCon, param_header->userPokeID) : NULL;

  BTL_POKEPARAM* target;
  u8 fDefaultMsg = ( (HANDEX_STR_IsEnable(&param->exStr) == FALSE) && (param->fStdMsgDisable == FALSE) );

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( !BPP_IsDead(target) )
    {
      BTL_N_Printf( DBGSTR_SVFL_HandEx_AddSick, param->pokeID, param->sickID, param->fAlmost );

      if( !scproc_AddSickCheckFail(wk, target, pp_user, param->sickID, param->sickCont,
        param->overWriteMode, param->fAlmost) )
      {
        if( param->header.tokwin_flag ){
          scPut_TokWin_In( wk, pp_user );
        }

        scproc_AddSickCore( wk, target, pp_user, param->sickID, param->sickCont, fDefaultMsg, &param->exStr );

        if( param->header.tokwin_flag ){
          scPut_TokWin_Out( wk, pp_user );
        }

        return 1;
      }
    }
  }

  return 0;
}
/**
 * ポケモンランク効果（段階数を指定し、上昇または下降させる。相手のとくせい等により失敗することがあり得る）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_rankEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header, u16 itemID )
{
  const BTL_HANDEX_PARAM_RANK_EFFECT* param = (BTL_HANDEX_PARAM_RANK_EFFECT*)param_header;
  BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
  BTL_POKEPARAM* pp_target;
  u8 fEffective = FALSE;
  u8 result = 0;
  u32 i;


  for(i=0; i<param->poke_cnt; ++i)
  {
    if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID[i]) )
    {
      pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID[i] );
      if( !BPP_IsDead( pp_target )
      &&  BPP_IsRankEffectValid( pp_target, param->rankType, param->rankVolume )
      ){
        fEffective = TRUE;
        break;
      }
    }
  }
  if( fEffective && param_header->tokwin_flag ){
    scPut_TokWin_In( wk, pp_user );
  }

  for(i=0; i<param->poke_cnt; ++i)
  {
    if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID[i]) )
    {
      pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID[i] );
      if( !BPP_IsDead(pp_target) )
      {
        if( scproc_RankEffectCore(wk, param_header->userPokeID, pp_target, param->rankType, param->rankVolume,
          BTL_POKEID_NULL, itemID, 0, param->fAlmost, !(param->fStdMsgDisable))
        ){
          handexSub_putString( wk, &param->exStr );
          result = 1;
        }
      }
    }
  }

  if( fEffective && param_header->tokwin_flag ){
    scPut_TokWin_Out( wk, pp_user );
  }

  return result;
}
/**
 * ポケモンランク強制セット（指定値に強制的に書き換える。失敗しない）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SET_RANK* param = (BTL_HANDEX_PARAM_SET_RANK*)param_header;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( !BPP_IsDead(pp_target) )
    {
      BPP_RankSet( pp_target, BPP_ATTACK_RANK,     param->attack );
      BPP_RankSet( pp_target, BPP_DEFENCE_RANK,    param->defence );
      BPP_RankSet( pp_target, BPP_SP_ATTACK_RANK,  param->sp_attack );
      BPP_RankSet( pp_target, BPP_SP_DEFENCE_RANK, param->sp_defence );
      BPP_RankSet( pp_target, BPP_AGILITY_RANK,    param->agility );
      BPP_RankSet( pp_target, BPP_HIT_RATIO,       param->hit_ratio );
      BPP_RankSet( pp_target, BPP_AVOID_RATIO,     param->avoid_ratio );

      SCQUE_PUT_OP_RankSet7( wk->que,
        param->pokeID, param->attack, param->defence, param->sp_attack, param->sp_defence, param->agility,
                       param->hit_ratio, param->avoid_ratio );
      return 1;
    }
  }
  return 0;
}
/**
 * ポケモンランクの内、マイナスになっているもののみフラットに戻す
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_recoverRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_RECOVER_RANK* param = (BTL_HANDEX_PARAM_RECOVER_RANK*)param_header;
  BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  if( !BPP_IsDead(pp_target) )
  {
    u8 result;
    SCQUE_PUT_OP_RankRecover( wk->que, param->pokeID );
    result = BPP_RankRecover( pp_target );
    return result;
  }
  return 0;
}
/**
 * ポケモンランクを全てフラットに戻す
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_resetRank( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_RESET_RANK* param = (BTL_HANDEX_PARAM_RESET_RANK*)param_header;
  BTL_POKEPARAM* pp_target;
  u32 i;
  u8 result = 0;

  for(i=0; i<param->poke_cnt; ++i)
  {
    pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID[i] );
    if( !BPP_IsDead(pp_target) ){
      SCQUE_PUT_OP_RankReset( wk->que, param->pokeID[i] );
      BPP_RankReset( pp_target );
      result = 1;
    }
  }
  return result;
}
/**
 * ポケモン能力値（攻撃、防御等）を指定値に書き換え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setStatus( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_SET_STATUS* param = (BTL_HANDEX_PARAM_SET_STATUS*)param_header;
  BTL_POKEPARAM* bpp;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( param->fAttackEnable ){
      BPP_SetBaseStatus( bpp, BPP_ATTACK, param->attack );
      SCQUE_PUT_OP_SetStatus( wk->que, param->pokeID, BPP_ATTACK, param->attack );
    }
    if( param->fDefenceEnable ){
      BPP_SetBaseStatus( bpp, BPP_DEFENCE, param->defence );
      SCQUE_PUT_OP_SetStatus( wk->que, param->pokeID, BPP_DEFENCE, param->defence );
    }
    if( param->fSpAttackEnable ){
      BPP_SetBaseStatus( bpp, BPP_SP_ATTACK, param->sp_attack );
      SCQUE_PUT_OP_SetStatus( wk->que, param->pokeID, BPP_SP_ATTACK, param->sp_attack );
    }
    if( param->fSpDefenceEnable ){
      BPP_SetBaseStatus( bpp, BPP_SP_DEFENCE, param->sp_defence );
      SCQUE_PUT_OP_SetStatus( wk->que, param->pokeID, BPP_SP_DEFENCE, param->sp_defence );
    }
    if( param->fAgilityEnable ){
      BPP_SetBaseStatus( bpp, BPP_AGILITY, param->agility );
      SCQUE_PUT_OP_SetStatus( wk->que, param->pokeID, BPP_AGILITY, param->agility );
    }

    handexSub_putString( wk, &param->exStr );

    return 1;
  }
  return 0;
}
/**
 * ポケモンを強制的にひん死にさせる
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_kill( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_KILL* param = (BTL_HANDEX_PARAM_KILL*)param_header;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* pp_target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( !BPP_IsDead(pp_target) )
    {
      handexSub_putString( wk, &param->exStr );
      scproc_KillPokemon( wk, pp_target );
      return 1;
    }
  }
  return 0;
}
/**
 * ポケモンタイプ変更
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_changeType( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CHANGE_TYPE* param= (const BTL_HANDEX_PARAM_CHANGE_TYPE*)param_header;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
    if( (!BPP_IsDead(bpp) )
    &&  (!BTL_TABLES_IsTypeChangeForbidPoke(BPP_GetMonsNo(bpp)))
    ){
      SCQUE_PUT_OP_ChangePokeType( wk->que, param->pokeID, param->next_type );
      BPP_ChangePokeType( bpp, param->next_type );

      if( !(param->fStdMsgDisable) )
      {
        if( PokeTypePair_IsPure( param->next_type ) ){
          PokeType type_pure = PokeTypePair_GetType1( param->next_type );
          SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_ChangePokeType, param->pokeID, type_pure );
        }
      }

      return 1;
    }
  }
  return 0;
}
/**
 * SETメッセージ表示
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_message( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_MESSAGE* param = (const BTL_HANDEX_PARAM_MESSAGE*)(param_header);
  const BTL_POKEPARAM* pp_user = NULL;
  if( param_header->userPokeID != BTL_POKEID_NULL ){
    pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
  }

  if( param_header->tokwin_flag && (pp_user!=NULL) ){
    scPut_TokWin_In( wk, pp_user );
  }

  handexSub_putString( wk, &param->str );

  if( param_header->tokwin_flag && (pp_user!=NULL) ){
    scPut_TokWin_Out( wk, pp_user );
  }

  return 1;
}
/**
 * ポケモンターンフラグセット
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setTurnFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_TURNFLAG* param = (const BTL_HANDEX_PARAM_TURNFLAG*)(param_header);
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  if( !BPP_IsDead(bpp) ){
    BPP_TURNFLAG_Set( bpp, param->flag );
    return 1;
  }
  return 0;
}
/**
 * ポケモンターンフラグリセット
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_resetTurnFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_TURNFLAG* param = (const BTL_HANDEX_PARAM_TURNFLAG*)(param_header);
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  if( !BPP_IsDead(bpp) ){
    BPP_TURNFLAG_ForceOff( bpp, param->flag );
    return 1;
  }
  return 0;
}
/**
 * ポケモン継続フラグセット
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setContFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SET_CONTFLAG* param = (const BTL_HANDEX_PARAM_SET_CONTFLAG*)(param_header);
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  if( !BPP_IsDead(bpp) ){
    scPut_SetContFlag( wk, bpp, param->flag );
    return 1;
  }
  return 0;
}
/**
 * ポケモン継続フラグリセット
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_resetContFlag( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SET_CONTFLAG* param = (const BTL_HANDEX_PARAM_SET_CONTFLAG*)(param_header);
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  if( !BPP_IsDead(bpp) ){
    scPut_ResetContFlag( wk, bpp, param->flag );
    return 1;
  }
  return 0;
}
/**
 * サイドエフェクト追加
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_sideEffectAdd( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SIDEEFF_ADD* param = (const BTL_HANDEX_PARAM_SIDEEFF_ADD*)(param_header);
  BPP_SICK_CONT cont = param->cont;

  scEvent_CheckSideEffectParam( wk, param_header->userPokeID, param->effect, param->side, &cont );

  if( BTL_HANDLER_SIDE_Add(param->side, param->effect, cont) )
  {
    handexSub_putString( wk, &param->exStr );
    return 1;
  }
  return 0;
}
/**
 * サイドエフェクトハンドラ消去
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_sideEffectRemove( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SIDEEFF_REMOVE* param = (const BTL_HANDEX_PARAM_SIDEEFF_REMOVE*)(param_header);
  u32 i;
  u8 result = 0;
  for(i=BTL_SIDEEFF_START; i<BTL_SIDEEFF_MAX; ++i)
  {
    if( BTL_CALC_BITFLG_Check(param->flags, i) )
    {
      if( BTL_HANDLER_SIDE_Remove(param->side, i) )
      {
        scPut_SideEffectOffMsg( wk, i, param->side );
        result = 1;
      }
    }
  }
  return result;
}
/**
 * フィールドエフェクトハンドラ追加
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_addFieldEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_ADD_FLDEFF* param = (const BTL_HANDEX_PARAM_ADD_FLDEFF*)(param_header);

  if( scproc_FieldEffectCore(wk, param->effect, param->cont, param->fAddDependPoke) )
  {
    handexSub_putString( wk, &param->exStr );
    return 1;
  }
  return 0;
}
/**
 * フィールドエフェクトハンドラ削除
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_removeFieldEffect( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_REMOVE_FLDEFF* param = (const BTL_HANDEX_PARAM_REMOVE_FLDEFF*)(param_header);

  if( BTL_FIELD_RemoveEffect(param->effect) ){
    scproc_FieldEff_End( wk, param->effect );
    return 1;
  }
  return 0;
}

/**
 * 天候変化
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_changeWeather( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CHANGE_WEATHER* param = (const BTL_HANDEX_PARAM_CHANGE_WEATHER*)(param_header);
  const BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
  BOOL result;

  if( param->weather != BTL_WEATHER_NONE )
  {
    if( scproc_ChangeWeatherCheck(wk, param->weather, param->turn) )
    {
      if( param_header->tokwin_flag ){
        scPut_TokWin_In( wk, pp_user );
      }
      scproc_ChangeWeatherCore( wk, param->weather, param->turn );
      handexSub_putString( wk, &param->exStr );
      result = TRUE;
      if( param_header->tokwin_flag ){
        scPut_TokWin_Out( wk, pp_user );
      }
    }
  }
  // エアロック
  else if( param->fAirLock )
  {
    if( param_header->tokwin_flag ){
      scPut_TokWin_In( wk, pp_user );
    }

    handexSub_putString( wk, &param->exStr );
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_NotifyAirLock( wk );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }

    result = TRUE;

    if( param_header->tokwin_flag ){
      scPut_TokWin_Out( wk, pp_user );
    }
  }

  return result;
}


//----------------------------------------------------------------------------------
// 文字出力共通処理
//----------------------------------------------------------------------------------
static BOOL handexSub_putString( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_STR_PARAMS* strParam )
{
  switch( strParam->type ){
  case BTL_STRTYPE_STD:
    scPut_Message_StdEx( wk, strParam->ID, strParam->argCnt, strParam->args );
    return TRUE;


  case BTL_STRTYPE_SET:
    scPut_Message_SetEx( wk, strParam->ID, strParam->argCnt, strParam->args );
    return TRUE;
  }
  return FALSE;
}

/**
 * 位置エフェクトハンドラ追加
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_PosEffAdd( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_POSEFF_ADD* param = (const BTL_HANDEX_PARAM_POSEFF_ADD*)(param_header);

  if( BTL_HANDLER_POS_Add(param->effect, param->pos, param_header->userPokeID, param->param, param->param_cnt) != NULL ){
    BTL_N_Printf( DBGSTR_SVFL_PosEffAdd, param->pos, param->effect, param_header->userPokeID);
    return 1;
  }
  BTL_N_Printf( DBGSTR_SVFL_PosEffDupFail, param->pos, param->effect, param_header->userPokeID );
  return 0;
}
/**
 * ポケモンとくせい書き換え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_tokuseiChange( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CHANGE_TOKUSEI* param = (const BTL_HANDEX_PARAM_CHANGE_TOKUSEI*)(param_header);

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  u16 prevTokusei = BPP_GetValue( bpp, BPP_TOKUSEI );

  if( BTL_TABLES_IsNeverChangeTokusei(prevTokusei) ){
    return 0;
  }

  if( ((param->fSameTokEffective) || ( param->tokuseiID != prevTokusei ))
  ){
    if( param_header->tokwin_flag ){
      SCQUE_PUT_TOKWIN_IN( wk->que, param_header->userPokeID );
    }

    SCQUE_PUT_ACTOP_ChangeTokusei( wk->que, param->pokeID, param->tokuseiID );
    handexSub_putString( wk, &param->exStr );

    // とくせい書き換え直前イベント
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_ChangeTokuseiBefore( wk, param->pokeID, prevTokusei, param->tokuseiID );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }

    BTL_HANDLER_TOKUSEI_Remove( bpp );
    BPP_ChangeTokusei( bpp, param->tokuseiID );
    SCQUE_PUT_OP_ChangeTokusei( wk->que, param->pokeID, param->tokuseiID );
    {
      BTL_EVENT_FACTOR* p = BTL_HANDLER_TOKUSEI_Add( bpp );
    }


    SCQUE_PUT_TOKWIN_OUT( wk->que, param->pokeID );

    // とくせい書き換え完了イベント
    if( prevTokusei != param->tokuseiID )
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_ChangeTokuseiAfter( wk, param->pokeID );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
    }

    if( param_header->tokwin_flag ){
      SCQUE_PUT_TOKWIN_OUT( wk->que, param_header->userPokeID );
    }

    if( !BPP_CheckSick(bpp, WAZASICK_IEKI) )
    {
      if( prevTokusei == POKETOKUSEI_BUKIYOU )
      {
        scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_GEN );
      }
      if( prevTokusei == POKETOKUSEI_KINCHOUKAN )
      {
        scproc_KintyoukanMoved( wk, bpp );
      }
    }

    return 1;
  }
  return 0;
}
/**
 * ポケモン装備アイテム書き換え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SET_ITEM* param = (const BTL_HANDEX_PARAM_SET_ITEM*)(param_header);

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  // 自分自身へのリクエストでなければアイテムセットの失敗チェック
  if( param_header->userPokeID != param->pokeID )
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    u8  failed = scEvent_CheckItemSet( wk, bpp, param->itemID );
    BTL_Hem_PopState( &wk->HEManager, hem_state );

    if( failed ){
      return 0;
    }
  }

  // ここまで来たら成功
  if( param_header->tokwin_flag ){
    SCQUE_PUT_TOKWIN_IN( wk->que, param_header->userPokeID );
  }
  handexSub_putString( wk, &param->exStr );

  scproc_ItemChange( wk, bpp, param->itemID );

  if( param_header->tokwin_flag ){
    SCQUE_PUT_TOKWIN_OUT( wk->que, param_header->userPokeID );
  }

  // 消費情報をクリア
  if( param->fClearConsume )
  {
    BPP_ClearConsumedItem( bpp );
    SCQUE_PUT_OP_ClearConsumedItem( wk->que, param->pokeID );
  }
  if( param->fClearConsumeOtherPoke )
  {
    BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->clearConsumePokeID );
    BPP_ClearConsumedItem( target );
    SCQUE_PUT_OP_ClearConsumedItem( wk->que, param->clearConsumePokeID );
  }

  scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_GEN );
  return 1;
}
/**
 * ポケモン装備アイテム交換
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_swapItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SWAP_ITEM* param = (const BTL_HANDEX_PARAM_SWAP_ITEM*)(param_header);

  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  BTL_POKEPARAM* self = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );
  u16 selfItem = BPP_GetItem( self );
  u16 targetItem = BPP_GetItem( target );

  // 対象の能力で失敗するケースをチェック
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    u8  failed = scEvent_CheckItemSet( wk, target, selfItem );
    BTL_Hem_PopState( &wk->HEManager, hem_state );
    if( failed ){
      return 0;
    }
  }

  // ここまで来たら成功
  if( param_header->tokwin_flag ){
    scPut_TokWin_In( wk, self );
  }
  handexSub_putString( wk, &param->exStr );
  handexSub_putString( wk, &param->exSubStr1 );
  handexSub_putString( wk, &param->exSubStr2 );
  if( param_header->tokwin_flag ){
    scPut_TokWin_Out( wk, self );
  }

  {
    scproc_ItemChange( wk, self, targetItem );
    scproc_ItemChange( wk, target, selfItem );
  }

  scproc_CheckItemReaction( wk, self, BTL_ITEMREACTION_GEN );
  scproc_CheckItemReaction( wk, target, BTL_ITEMREACTION_GEN );

  return 1;
}
/**
 * ポケモン装備アイテム発動チェック
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_CheckItemEquip( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CHECK_ITEM_EQUIP* param = (const BTL_HANDEX_PARAM_CHECK_ITEM_EQUIP*)(param_header);
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  scproc_CheckItemReaction( wk, bpp, BTL_ITEMREACTION_GEN );
  return 1;
}
/**
 * 装備アイテム使用
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_useItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_USE_ITEM* param = (const BTL_HANDEX_PARAM_USE_ITEM*)(param_header);
  BTL_POKEPARAM* pp_user = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );

  if( !BPP_IsDead(pp_user) || param->fUseDead )
  {
    if( param->fSkipHPFull && BPP_IsHPFull(pp_user) ){
      return 0;
    }

    if( scproc_UseItemEquip(wk, pp_user) ){
      return 1;
    }
  }
  return 0;
}
/**
 * ポケモンにアイテム効果を強制発動
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_ItemSP( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_ITEM_SP* param = (BTL_HANDEX_PARAM_ITEM_SP*)param_header;
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  if( !BPP_IsDead(bpp) )
  {
    BTL_EVENT_FACTOR* factor = BTL_HANDLER_ITEM_TMP_Add( bpp, param->itemID );
    if( factor )
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      scEvent_ItemEquipTmp( wk, bpp, param_header->userPokeID );
      BTL_Hem_PopState( &wk->HEManager, hem_state );
      BTL_HANDLER_ITEM_TMP_Remove( factor );
      return 1;
    }
  }
  return 0;
}
/**
 * ポケモン自分のアイテム消費＆メッセージ表示
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_consumeItem( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CONSUME_ITEM* param = (const BTL_HANDEX_PARAM_CONSUME_ITEM*)param_header;
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );

  if( param->fNoAction == FALSE ){
    scPut_UseItemAct( wk, bpp );
    handexSub_putString( wk, &param->exStr );
  }

  scproc_ItemChange( wk, bpp, ITEM_DUMMY_DATA );
  scproc_ConsumeItem( wk, bpp );

  return 1;
}

/**
 * ポケモンわざ書き換え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_updateWaza( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_UPDATE_WAZA* param = (const BTL_HANDEX_PARAM_UPDATE_WAZA*)(param_header);

  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  SCQUE_PUT_OP_UpdateWaza( wk->que, param->pokeID, param->wazaIdx, param->waza, param->ppMax, param->fPermanent );
  BPP_WAZA_UpdateID( target, param->wazaIdx, param->waza, param->ppMax, param->fPermanent );
  return 1;
}
/**
 * ポケモンカウンタ値書き換え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_counter( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_COUNTER* param = (const BTL_HANDEX_PARAM_COUNTER*)(param_header);

  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  scPut_SetBppCounter( wk, target, param->counterID, param->value );
  return 1;
}
/**
 * 時間差ワザダメージ
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_delayWazaDamage( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_DELAY_WAZADMG* param = (const BTL_HANDEX_PARAM_DELAY_WAZADMG*)param_header;

  BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, param->attackerPokeID );
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->targetPokeID );

  SVFL_WAZAPARAM   wazaParam;
  WAZAEFF_CTRL  ctrlBackup;
  u16  que_reserve_pos;
  u8 result;

  scEvent_GetWazaParam( wk, param->wazaID, attacker, &wazaParam );

  // ワザメッセージ，エフェクト，ワザ出し確定
  que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_WAZA_EFFECT );

  // ワザ対象をワークに取得
  BTL_POKESET_Clear( wk->psetDamaged );
  BTL_POKESET_Clear( wk->psetTmp );
  BTL_POKESET_Add( wk->psetTmp, target );
  BTL_POKESET_SetDefaultTargetCount( wk->psetTmp, 1 );

  // 場にいないポケモンには当たらない（補正なし）
  BTL_POKESET_RemoveDeadPoke( wk->psetTmp );
  if( BTL_POKESET_IsRemovedAll(wk->psetTmp) ){
    scPut_WazaFail( wk, attacker, wazaParam.wazaID );
    return 0;
  }

  // 対象ごとの無効チェック＆回避チェック
  flowsub_CheckPokeHideAvoid( wk, &wazaParam, attacker, wk->psetTmp );
  flowsub_checkWazaAffineNoEffect( wk, &wazaParam, attacker, wk->psetTmp, &wk->dmgAffRec );
  flowsub_checkNotEffect( wk, &wazaParam, attacker, wk->psetTmp );
  flowsub_checkWazaAvoid( wk, &wazaParam, attacker, wk->psetTmp );
  // 最初は居たターゲットが残っていない -> 何もせず終了
  if( BTL_POKESET_IsRemovedAll(wk->psetTmp) ){
    return 0;
  }

  // ワザエフェクト管理のバックアップを取り、システム初期化
  ctrlBackup = *(wk->wazaEffCtrl);
  wazaEffCtrl_Init( wk->wazaEffCtrl );
  wazaEffCtrl_Setup( wk->wazaEffCtrl, wk, attacker, wk->psetTmp );

  scproc_Fight_Damage_Root( wk, &wazaParam, attacker, wk->psetTmp, &wk->dmgAffRec, TRUE );

  // ワザ効果あり確定→演出表示コマンド生成などへ
  result = wazaEffCtrl_IsEnable( wk->wazaEffCtrl );
  if( result ){
    wazaEffCtrl_SetEffectIndex( wk->wazaEffCtrl, BTLV_WAZAEFF_DELAY_ATTACK );
    scPut_WazaEffect( wk, wazaParam.wazaID, wk->wazaEffCtrl, que_reserve_pos );
  }

  *(wk->wazaEffCtrl) = ctrlBackup;
  return result;
}
/**
 * バトル離脱
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_quitBattle( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );

  if( scproc_NigeruCmdSub(wk, bpp, TRUE) )
  {
    wk->flowResult = SVFLOW_RESULT_BTL_QUIT;
    return 1;
  }
  return 0;
}
/**
 * メンバー入れ替え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_changeMember( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_CHANGE_MEMBER* param = (BTL_HANDEX_PARAM_CHANGE_MEMBER*)param_header;
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  if( (!scproc_CheckShowdown(wk))
  &&  (!BTL_SVFTOOL_IsFreeFallPoke(wk, param->pokeID))
  &&  (wk->flowResult == SVFLOW_RESULT_DEFAULT)
  ){
    handexSub_putString( wk, &param->preStr );
    if( scproc_MemberOutForChange(wk, bpp, param->fIntrDisable) )
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, param->pokeID );

      BTL_SERVER_RequestChangePokemon( wk->server, pos );
      handexSub_putString( wk, &param->exStr );
      wk->flowResult = SVFLOW_RESULT_POKE_CHANGE;
      return 1;
    }
  }
  return 0;
}
/**
 * バトンタッチ
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_batonTouch( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_BATONTOUCH* param = (BTL_HANDEX_PARAM_BATONTOUCH*)param_header;

  BTL_POKEPARAM* user = BTL_POKECON_GetPokeParam( wk->pokeCon, param->userPokeID );
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->targetPokeID );

  BPP_BatonTouchParam( target, user );
  SCQUE_PUT_OP_BatonTouch( wk->que, param->userPokeID, param->targetPokeID );
  if( BPP_MIGAWARI_IsExist(target) )
  {
    BtlPokePos pos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, param->targetPokeID );
    SCQUE_PUT_ACT_MigawariCreate( wk->que, pos );
  }
  return 1;
}
/**
 * ひるませる
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_addShrink( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_ADD_SHRINK* param = (BTL_HANDEX_PARAM_ADD_SHRINK*)param_header;

  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  if( scproc_AddShrinkCore(wk, target, param->per) ){
    return 1;
  }
  return 0;
}
/**
 * 瀕死から回復
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_relive( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_RELIVE* param = (BTL_HANDEX_PARAM_RELIVE*)param_header;
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  BTL_N_Printf( DBGSTR_SVFL_HandExRelive, param->pokeID, param->recoverHP );
  BPP_HpPlus( target, param->recoverHP );
  SCQUE_PUT_OP_HpPlus( wk->que, param->pokeID, param->recoverHP );
  wk->pokeDeadFlag[param->pokeID] = FALSE;
  handexSub_putString( wk, &param->exStr );

  {
    BtlPokePos targetPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, param->pokeID );
    if( targetPos != BTL_POS_NULL )
    {
      u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( param->pokeID );
      u8 posIdx = BTL_MAIN_BtlPosToPosIdx( wk->mainModule, targetPos );
      /*
      BTL_POSPOKE_PokeIn( &wk->pospokeWork, targetPos, param->pokeID, wk->pokeCon );
      SCQUE_PUT_ACT_RelivePoke( wk->que, param->pokeID );
      BTL_HANDLER_TOKUSEI_Add( target );
      BTL_HANDLER_ITEM_Add( target );
      */

//      scproc_MemberInCore(
      scproc_MemberInForChange( wk, clientID, posIdx, posIdx, TRUE );
      scproc_AfterMemberIn( wk );

    }
  }


  return 1;
}
/**
 * 体重をセット
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setWeight( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_SET_WEIGHT* param = (BTL_HANDEX_PARAM_SET_WEIGHT*)param_header;
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );

  BPP_SetWeight( target, param->weight );
  SCQUE_PUT_OP_SetWeight( wk->que, param->pokeID, param->weight );
  handexSub_putString( wk, &param->exStr );
  return 1;
}
/**
 * 場から吹き飛ばす
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_pushOut( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_PUSHOUT* param = (BTL_HANDEX_PARAM_PUSHOUT*)param_header;
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  BTL_POKEPARAM* attacker = BTL_POKECON_GetPokeParam( wk->pokeCon, param_header->userPokeID );

  BOOL fFailMsgDisped;

  if( scproc_PushOutCore(wk, attacker, target, param->fForceChange, &fFailMsgDisped,
        param->effectNo, param->fIgnoreLevel, &param->exStr) )
  {
    return 1;
  }
  return 0;
}
/**
 * 指定ポケモンの行動を割り込みさせる
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_intrPoke( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_INTR_POKE* param = (BTL_HANDEX_PARAM_INTR_POKE*)param_header;

  if( ActOrder_IntrReserve(wk, param->pokeID) ){
    handexSub_putString( wk, &param->exStr );
    return 1;
  }
  return 0;
}
/**
 * 指定ワザ使用ポケモンの行動を割り込みさせる
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_intrWaza( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_INTR_WAZA* param = (BTL_HANDEX_PARAM_INTR_WAZA*)param_header;

  if( ActOrder_IntrReserveByWaza(wk, param->waza) ){
    return 1;
  }
  return 0;
}
/**
 * 指定ポケモンの行動順を最後に回す
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_sendLast( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_SEND_LAST* param = (BTL_HANDEX_PARAM_SEND_LAST*)param_header;

  if( ActOrder_SendLast(wk, param->pokeID) ){
    handexSub_putString( wk, &param->exStr );
    return 1;
  }
  return 0;
}
/**
 * 指定ポケモン同士の場所入れ替え
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_swapPoke( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_SWAP_POKE* param = (BTL_HANDEX_PARAM_SWAP_POKE*)param_header;

  if( param->pokeID1 != param->pokeID2 )
  {
    u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( param->pokeID1 );
    if( clientID == BTL_MAINUTIL_PokeIDtoClientID(param->pokeID2) )
    {
      const BTL_POKEPARAM* bpp1 = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID1 );
      const BTL_POKEPARAM* bpp2 = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID2 );
      if( !BPP_IsDead(bpp1) && !BPP_IsDead(bpp2) )
      {
        const BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
        s16 posIdx1 = BTL_PARTY_FindMember( party, bpp1 );
        s16 posIdx2 = BTL_PARTY_FindMember( party, bpp2 );
        if( (posIdx1 >= 0) && (posIdx2 >= 0) )
        {
          scproc_MoveCore( wk, clientID, posIdx1, posIdx2, TRUE );
          handexSub_putString( wk, &param->exStr );
          return 1;
        }
      }
    }
  }
  return 0;
}
/**
 * ヘッダポケモンが指定ポケモンに変身
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_hensin( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_HENSIN* param = (BTL_HANDEX_PARAM_HENSIN*)param_header;

  BTL_POKEPARAM* user = BTL_POKECON_GetPokeParam( wk->pokeCon,  param_header->userPokeID );
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon,  param->pokeID );

  if( (!BPP_IsFakeEnable(user))
  &&  (!BPP_IsFakeEnable(target))
  &&  (BPP_CONTFLAG_CheckWazaHide(user) == BPP_CONTFLG_NULL)
  ){
    u16 prevTokusei = BPP_GetValue( user, BPP_TOKUSEI );
//    TAYA_Printf("PokeSick=%d, line=%d\n", BPP_GetPokeSick(user), __LINE__);
    if( BPP_HENSIN_Set(user, target) )
    {
      u8 usrPokeID = BPP_GetID( user );
      u8 tgtPokeID = BPP_GetID( target );

//      TAYA_Printf("PokeSick=%d, line=%d\n", BPP_GetPokeSick(user), __LINE__);

      if( param_header->tokwin_flag ){
        scPut_TokWin_In( wk, user );
      }

      BTL_HANDLER_Waza_RemoveForceAll( user );
      BTL_HANDLER_TOKUSEI_Remove( user );
      BTL_HANDLER_TOKUSEI_Add( user );
      SCQUE_PUT_ACT_Hensin( wk->que, usrPokeID, tgtPokeID );

      handexSub_putString( wk, &param->exStr );
      if( param_header->tokwin_flag ){
        scPut_TokWin_Out( wk, user );
      }

      if( prevTokusei != BPP_GetValue(user, BPP_TOKUSEI_EFFECTIVE) )
      {
        u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
        scEvent_ChangeTokuseiAfter( wk, usrPokeID );
        BTL_Hem_PopState( &wk->HEManager, hem_state );
      }
      return 1;
    }
  }
  return 0;
}
/**
 * イリュージョン解除
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_fakeBreak( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  BTL_HANDEX_PARAM_FAKE_BREAK* param = (BTL_HANDEX_PARAM_FAKE_BREAK*)param_header;

  if( BTL_POSPOKE_IsExist(&wk->pospokeWork, param->pokeID) )
  {
    BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon,  param->pokeID );

    if( BPP_IsFakeEnable(bpp) )
    {
      BPP_FakeDisable( bpp );
      SCQUE_PUT_ACT_FakeDisable( wk->que, param->pokeID );
      handexSub_putString( wk, &param->exStr );
      return 1;
    }
  }
  return 0;
}
/**
 * じゅうりょく発動時チェック（そらとぶ・ダイビング＆ふゆうフラグを落とす）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_juryokuCheck( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  u8 pokeID[ BTL_POS_MAX ];
  u8 cnt, pos, fFall, i;
  BtlExPos exPos;
  BTL_POKEPARAM* bpp;

  pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, param_header->userPokeID );
  exPos = EXPOS_MAKE( BTL_EXPOS_FULL_ALL, pos );
  cnt = BTL_SVFTOOL_ExpandPokeID( wk, exPos, pokeID );
  for(i=0; i<cnt; ++i)
  {
    bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID[i] );
    fFall = FALSE;
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_SORAWOTOBU) )
    {
      scproc_TameHideCancel( wk, bpp, BPP_CONTFLG_SORAWOTOBU );
      fFall = TRUE;
    }
    if( scEvent_CheckFloating(wk, bpp, TRUE) )
    {
      fFall = TRUE;
    }
    if( BPP_CheckSick(bpp, WAZASICK_FLYING) )
    {
      scPut_CureSick( wk, bpp, WAZASICK_FLYING, NULL );
      fFall = TRUE;
    }

    if( fFall ){
      SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_JyuryokuFall, pokeID[i] );
    }
  }

  return 1;
}
/**
 * 溜めワザ消え状態のキャンセル（そらとぶ・ダイビング等）
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_TameHideCancel( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_TAMEHIDE_CANCEL* param = (const BTL_HANDEX_PARAM_TAMEHIDE_CANCEL*)param_header;

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  if( !BPP_IsDead(bpp) )
  {
    if( scproc_TameHideCancel(wk, bpp, param->flag) )
    {
      handexSub_putString( wk, &param->exStr );
      return 1;
    }
  }
  return 0;
}
/**
 * 位置指定エフェクト生成
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_effectByPos( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_ADD_EFFECT* param = (const BTL_HANDEX_PARAM_ADD_EFFECT*)(param_header);

  if( param->fMsgWinVanish ){
    SCQUE_PUT_ACT_MsgWinHide( wk->que, 0 );
  }

  scproc_ViewEffect( wk, param->effectNo, param->pos_from, param->pos_to, param->fQueReserve, param->reservedQuePos );

  return 1;
}

/**
 * フォルムナンバーチェンジ
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_changeForm( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_CHANGE_FORM* param = (const BTL_HANDEX_PARAM_CHANGE_FORM*)param_header;

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, param->pokeID );
  if( !BPP_IsDead(bpp) )
  {
    u8 currentForm = BPP_GetValue( bpp, BPP_FORM );
    if( currentForm != param->formNo )
    {
      if( param_header->tokwin_flag ){
        scPut_TokWin_In( wk, bpp );
      }

      BPP_ChangeForm( bpp, param->formNo );
      SCQUE_PUT_ACT_ChangeForm( wk->que, param->pokeID, param->formNo );
      handexSub_putString( wk, &param->exStr );

      if( param_header->tokwin_flag ){
        scPut_TokWin_Out( wk, bpp );
      }

      return 1;
    }
  }
  return 0;
}
/**
 * ワザエフェクトインデックス変更
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setWazaEffectIndex( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_SET_EFFECT_IDX* param = (const BTL_HANDEX_PARAM_SET_EFFECT_IDX*)param_header;

  wazaEffCtrl_SetEffectIndex( wk->wazaEffCtrl, param->effIndex );
  return 1;
}
/**
 * ワザエフェクトを強制的に有効にする
 * @return 成功時 1 / 失敗時 0
 */
static u8 scproc_HandEx_setWazaEffectEnable( BTL_SVFLOW_WORK* wk, const BTL_HANDEX_PARAM_HEADER* param_header )
{
  const BTL_HANDEX_PARAM_WAZAEFFECT_ENABLE* param = (const BTL_HANDEX_PARAM_WAZAEFFECT_ENABLE*)param_header;

  wazaEffCtrl_SetEnable( wk->wazaEffCtrl );
  return 1;
}


/*

typedef struct {

  u8    type;       ///< ほのお、みず、くさなどのタイプ
  u8    category;   ///< ワザカテゴリ（※１）
  s8    priority;   ///< 優先度
  u8    hitPer;     ///< 命中率
  u8    damageType; ///< ダメージタイプ（"なし"=0, "物理"=1, "特殊"=2）
  u8    power;      ///< 威力値
  u8    critical;   ///< クリティカルランク
  u8    hitMax;     ///< 最大ヒット回数
  u8    hitMin;     ///< 最小ヒット回数
  u8    shrinkPer;  ///< ひるみ確率

  u8    sickID;     ///< 状態異常効果
  u8    sickCont;   ///< 状態異常の継続パターン
  u8    sickTurnMin;///< 状態異常の最小ターン数
  u8    sickTurnMax;///< 状態異常の最大ターン数
  u8    sickPer;    ///< 追加効果による状態異常の発生率

  struct {
    u8 statusID;
    s8 value;
    u8 per;
  }rankEffect[ 3 ];

  s8    recoverRatio_damage;
  s8    recoverRatio_HP;

  u8    coverArea;

  u16   touchFlag : 1;      ///< 接触ワザ
  u16   tameFlag  : 1;      ///< １ターン溜め
  u16   tireFlag  : 1;      ///< １ターン反動で動けない
  u16   guardFlag : 1;      ///< 「まもる」「みきり」の対象
  u16   magicCoatFlag : 1;  ///< 「マジックコート」の対象
  u16   yokodoriFlag : 1;   ///< 「よこどり」の対象
  u16   oumuFlag  : 1;      ///< 「オウムがえし」の対象
  u16   hitFarFlag : 1;     ///< トリプルで離れたポケモンにも当たる



}WAZADATA;

/*
※１
"シンプルダメージ"=0
"シンプルシック"
"

*/

