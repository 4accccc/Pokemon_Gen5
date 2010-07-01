//=============================================================================================
/**
 * @file  btl_server_flow_sub.c
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理（サブルーチン群）
 * @author  taya
 *
 *  元々 btl_server_flow.c 内にあった関数を、あまりに見通しが悪くなってきたので無理矢理に分離したものです。
 *  そういうわけでソースを読むときは btl_server_flow.c と行きつ戻りつしないとならない点もありご迷惑をおかけします…。
 *
 * @date  2010.05.28  作成
 */
//=============================================================================================
#include <gflib.h>


#include "sound\pm_sndsys.h"
#include "waza_tool\wazadata.h"
#include "waza_tool\wazano_def.h"
#include "item\item.h"
#include "item\itemtype_def.h"
#include "item\itemuse_def.h"
#include "poke_tool\shinka_check.h"
#include "gamesystem\g_power.h"
#include "system/rtc_tool.h"

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
#include "handler\hand_common.h"
#include "btl_server_local.h"
#include "btl_server.h"
#include "btl_hem.h"

#include "btl_server_flow_def.h"
#include "btl_server_flow_sub.h"




/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static u8 registerTarget_single( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec );
static u8 registerTarget_double( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec );
static u8 registerTarget_triple( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec );
static BOOL correctTargetDead( BTL_SVFLOW_WORK* wk, BtlRule rule, const BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, BtlPokePos targetPos, BTL_POKESET* rec );
static BTL_POKEPARAM* get_opponent_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos, u8 pokeSideIdx );
static BTL_POKEPARAM* get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos );
static u8 checkTemptPokeID( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u8 targetPokeID );
static u8 scEvent_GetWazaTargetIntr( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam );
static u8 scEvent_GetWazaTargetTempt( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u8 defaultTargetPokeID );
static u32 getexp_calc_adjust_level( const BTL_POKEPARAM* bpp, u32 base_exp, u16 getpoke_lv, u16 deadpoke_lv );
static inline u32 _calc_adjust_level_sub( u32 val );
static void PutDoryokuExp( BTL_POKEPARAM* bpp, const BTL_POKEPARAM* deadPoke );
static BOOL scproc_TrainerItem_BallRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID );
static BOOL CalcCapturePokemon( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* myPoke, const BTL_POKEPARAM* targetPoke,u16 ballID,
    u8* yure_cnt, u8* fCritical );
static fx32 GetKusamuraCaptureRatio( BTL_SVFLOW_WORK* wk );
static fx32 CalcBallCaptureRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* myPoke, const BTL_POKEPARAM* targetPoke, u16 ballID );
static BOOL CheckCaptureCritical( BTL_SVFLOW_WORK* wk, fx32 capture_value );
static u8 ItemEff_SleepRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_PoisonRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_YakedoRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_KooriRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_MahiRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_KonranRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_MeromeroRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_EffectGuard( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_Relive( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_AttackRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_DefenceRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_SPAttackRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_SPDefenceRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_AgilityRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_HitRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_CriticalUp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_PP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_AllPP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_HP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ItemEff_Common_Cure( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, WazaSick sickID );
static u8 ItemEff_Mental_Cure( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, WazaSick sickID );
static u8 ItemEff_Common_Rank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, BppValueID rankType );
static u8 ShooterEff_ItemCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ShooterEff_SkillCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ShooterEff_ItemDrop( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static u8 ShooterEff_FlatCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam );
static BtlResult CheckResult_AliveBoth( BTL_SVFLOW_WORK* wk, const u8* alivePokeCnt, const u8* totalPokeCnt, u8 playerSide );
static void calcHPRatio( BTL_SVFLOW_WORK* wk, int* HP_sum, int* HP_ratio, u8 side );



/*====================================================================================================*/
/*                                                                                                    */
/*  ワザ対象ポケモン格納                                                                              */
/*                                                                                                    */
/*====================================================================================================*/

//--------------------------------------------------------------------------
/**
 * 対象ポケモンのデータポインタを、BTL_POKESET構造体に格納
 *
 * @param   wk
 * @param   atPos
 * @param   waza
 * @param   action
 * @param   rec       [out]
 *
 * @retval  u8        対象ポケモン数
 */
//--------------------------------------------------------------------------
u8 BTL_SVFSUB_RegisterTargets( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, BTL_POKESET* rec )
{
  BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );

  u8 intrPokeID = BTL_POKEID_NULL;
  u8 numTarget;

  if( !(wazaParam->fMagicCoat) ){
    intrPokeID = scEvent_GetWazaTargetIntr( wk, attacker, wazaParam );
  }

  /*
  if( BTL_POSPOKE_GetPokeExistPos(&wk->pospokeWork, intrPokeID) == BTL_POS_NULL ){
    intrPokeID = BTL_POKEID_NULL;
  }else{
    BTL_N_Printf( DBGSTR_SVFL_IntrTargetPoke, intrPokeID);
  }
  */

  BTL_POKESET_Clear( rec );
  wk->defaultTargetPos = BTL_POS_NULL;

  switch( rule ){
  case BTL_RULE_SINGLE:
  default:
    numTarget = registerTarget_single( wk, attacker, targetPos, wazaParam, intrPokeID, rec );
    break;

  case BTL_RULE_DOUBLE:
    numTarget = registerTarget_double( wk, attacker, targetPos, wazaParam, intrPokeID, rec );
    break;

  case BTL_RULE_TRIPLE:
    numTarget = registerTarget_triple( wk, attacker, targetPos, wazaParam, intrPokeID, rec );
    break;

  case BTL_RULE_ROTATION:
    #ifdef ROTATION_NEW_SYSTEM
    numTarget = registerTarget_single( wk, attacker, targetPos, wazaParam, intrPokeID, rec );
    #else
    numTarget = registerTarget_double( wk, attacker, targetPos, wazaParam, intrPokeID, rec );
    #endif
    break;
  }

  if( numTarget == 1 )
  {
    BTL_POKEPARAM* bpp = BTL_POKESET_Get( rec, 0 );
    if( bpp ){
      wk->defaultTargetPos = BTL_POSPOKE_GetPokeLastPos( &wk->pospokeWork, BPP_GetID(bpp) );
//      TAYA_Printf(" １体ターゲットワザはデフォルト位置を記録:位置=%d, poke=%d\n", wk->defaultTargetPos, BPP_GetID(bpp) );
    }

    if( correctTargetDead(wk, rule, attacker, wazaParam, targetPos, rec) == FALSE ){
      numTarget = 0;
    }
  }

  return numTarget;
}
/**
 *  ワザ対象格納：シングル用
 */
static u8 registerTarget_single( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec )
{
  BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );

  switch( wazaParam->targetType ){
  case WAZA_TARGET_OTHER_SELECT:  ///< 自分以外の１体（選択）
  case WAZA_TARGET_ENEMY_SELECT:  ///< 敵１体（選択）
  case WAZA_TARGET_ENEMY_RANDOM:  ///< 敵ランダム
  case WAZA_TARGET_ENEMY_ALL:     ///< 敵側２体
  case WAZA_TARGET_OTHER_ALL:     ///< 自分以外全部
    if( intrPokeID == BTL_POKEID_NULL ){
      BTL_POKESET_Add( rec, get_opponent_pokeparam(wk, atPos, 0) );
    }else{
      BTL_POKESET_Add( rec, BTL_POKECON_GetPokeParam(wk->pokeCon, intrPokeID) );
    }
    return 1;

  case WAZA_TARGET_USER:                ///< 自分１体のみ
  case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方１体
    if( intrPokeID == BTL_POKEID_NULL ){
      BTL_POKESET_Add( rec, attacker );
    }else{
      BTL_POKESET_Add( rec, BTL_POKECON_GetPokeParam(wk->pokeCon, intrPokeID) );
    }
    return 1;

  case WAZA_TARGET_ALL:
    BTL_POKESET_Add( rec, attacker );
    BTL_POKESET_Add( rec, get_opponent_pokeparam(wk, atPos, 0) );
    return 2;

  case WAZA_TARGET_UNKNOWN:
    if( intrPokeID != BTL_POKEID_NULL ){
      BTL_POKESET_Add( rec, BTL_POKECON_GetPokeParam(wk->pokeCon, intrPokeID) );
      return 1;
    }else{
      return 0;
    }
    break;

  default:
    return 0;
  }
}
/**
 *  ワザ対象格納：ダブル用
 */
static u8 registerTarget_double( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec )
{
  BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );

  BTL_POKEPARAM* bpp = NULL;

  BTL_N_Printf( DBGSTR_SVFS_RegTargetDouble, wazaParam->targetType, BPP_GetID(attacker), targetPos);

  switch( wazaParam->targetType ){
  case WAZA_TARGET_OTHER_SELECT:        ///< 自分以外の１体（選択）
    bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos );
    break;
  case WAZA_TARGET_ENEMY_SELECT:        ///< 敵１体（選択）
    bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos );
    break;
  case WAZA_TARGET_ENEMY_RANDOM:        ///< 敵ランダムに１体
    bpp = get_opponent_pokeparam( wk, atPos, BTL_CALC_GetRand(2) );
    break;

  case WAZA_TARGET_ENEMY_ALL:           ///< 敵側全体
    BTL_POKESET_Add( rec, get_opponent_pokeparam(wk, atPos, 0) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam(wk, atPos, 1) );
    return 2;
  case WAZA_TARGET_OTHER_ALL:           ///< 自分以外全部
    BTL_POKESET_Add( rec, get_next_pokeparam( wk, atPos ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 0 ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 1 ) );
    return 3;
  case WAZA_TARGET_ALL:                ///< 全部
    BTL_POKESET_Add( rec, attacker );
    BTL_POKESET_Add( rec, get_next_pokeparam( wk, atPos ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 0 ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 1 ) );
    return 3;

  case WAZA_TARGET_USER:      ///< 自分１体のみ
    if( intrPokeID == BTL_POKEID_NULL ){
      BTL_POKESET_Add( rec, attacker );
    }else{
      BTL_POKESET_Add( rec, BTL_POKECON_GetPokeParam(wk->pokeCon, intrPokeID) );
    }
    return 1;

  case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方１体（選択）
    BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, targetPos) );
    return 1;
  case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方１体
    BTL_POKESET_Add( rec, get_next_pokeparam(wk, atPos) );
    return 1;

  case WAZA_TARGET_UNKNOWN:
    if( intrPokeID != BTL_POKEID_NULL )
    {
      bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, intrPokeID );
    }
    else{
      return 0;
    }
    break;

  default:
    return 0;
  }

  // 対象が敵含む１体のワザならここまで来る
  if( bpp )
  {
    u8 targetPokeID = BPP_GetID( bpp );
    u8 temptPokeID = checkTemptPokeID( wk, attacker, wazaParam, targetPokeID );
    if( temptPokeID != BTL_POKEID_NULL ){
      bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, temptPokeID );
    }
    BTL_N_Printf( DBGSTR_SVFL_DoubleTargetRegister, BPP_GetID(bpp) );
    BTL_POKESET_Add( rec, bpp );
    return 1;
  }
  else
  {
    return 0;
  }
}
/**
 *  ワザ対象格納：トリプル用
 */
static u8 registerTarget_triple( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, u8 intrPokeID, BTL_POKESET* rec )
{
  BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );
  const BTL_TRIPLE_ATTACK_AREA* area = BTL_MAINUTIL_GetTripleAttackArea( atPos );
  u32 i, cnt;

  BTL_POKEPARAM* bpp = NULL;

  BTL_N_Printf( DBGSTR_SVFS_RegTargetDouble, wazaParam->targetType, BPP_GetID(attacker), targetPos);


  switch( wazaParam->targetType ){
  case WAZA_TARGET_OTHER_SELECT:        ///< 自分以外の１体（選択）
    bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos );
    break;
  case WAZA_TARGET_ENEMY_SELECT:        ///< 敵１体（選択）
    bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, targetPos );
    break;
  case WAZA_TARGET_ENEMY_RANDOM:        ///< 敵ランダムに１体
    {
      u8 r = BTL_CALC_GetRand( area->numEnemys );
      bpp = BTL_POKECON_GetFrontPokeData( wk->pokeCon, area->enemyPos[r] );
    }
    break;

  case WAZA_TARGET_ENEMY_ALL:           ///< 敵側全体
    for(i=0; i<area->numEnemys; ++i){
      BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData( wk->pokeCon, area->enemyPos[i]) );
    }
    return area->numEnemys;

  case WAZA_TARGET_OTHER_ALL:           ///< 自分以外全部
    cnt = 0;
    for(i=0; i<area->numEnemys; ++i){
      BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, area->enemyPos[i]) );
      ++cnt;
    }
    for(i=0; i<area->numFriends; ++i){
      if( area->friendPos[i] == atPos ){ continue; }
      BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, area->friendPos[i]) );
      ++cnt;
    }
    return cnt;

  case WAZA_TARGET_ALL:                ///< 全部
    BTL_POKESET_Add( rec, attacker );
    for(i=0; i<BTL_POSIDX_MAX; ++i)
    {
      {
        BtlPokePos  pos = BTL_MAINUTIL_GetFriendPokePos( atPos, i );
        if( pos != atPos )
        {
          BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, pos) );
        }
      }
    }
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 0 ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 1 ) );
    BTL_POKESET_Add( rec, get_opponent_pokeparam( wk, atPos, 2 ) );
    return 6;

  case WAZA_TARGET_USER:      ///< 自分１体のみ
    if( intrPokeID == BTL_POKEID_NULL ){
      BTL_POKESET_Add( rec, attacker );
    }else{
      BTL_POKESET_Add( rec, BTL_POKECON_GetPokeParam(wk->pokeCon, intrPokeID) );
    }
    return 1;

  case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方１体（選択）
    BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, targetPos) );
    return 1;
  case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方１体
    BTL_POKESET_Add( rec, BTL_POKECON_GetFrontPokeData(wk->pokeCon, targetPos) );
    return 1;

  case WAZA_TARGET_UNKNOWN:
    if( intrPokeID != BTL_POKEID_NULL )
    {
      bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, intrPokeID );
    }
    else{
      return 0;
    }
    break;

  default:
    return 0;
  }

  // 対象が敵含む１体のワザならここまで来る
  if( bpp )
  {
    u8 targetPokeID = BPP_GetID( bpp );
    u8 temptPokeID = checkTemptPokeID( wk, attacker, wazaParam, targetPokeID );
    if( temptPokeID != BTL_POKEID_NULL ){
      bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, temptPokeID );
    }
    BTL_N_Printf( DBGSTR_SVFL_DoubleTargetRegister, BPP_GetID(bpp) );
    BTL_POKESET_Add( rec, bpp );
    return 1;
  }
  else
  {
    return 0;
  }
}
/**
 *  対象ポケモンが死んでいた時の対象修正
 *
 *  @retval   BOOL 補正できたらTRUE／補正対象個所にもポケモンが居ない場合FALSE
 */
static BOOL correctTargetDead( BTL_SVFLOW_WORK* wk, BtlRule rule, const BTL_POKEPARAM* attacker,
  const SVFL_WAZAPARAM* wazaParam, BtlPokePos targetPos, BTL_POKESET* rec )
{
  // 戦闘位置が陣営あたり２以上（ダブル、トリプル、ローテ）
  u8 numFrontPos = BTL_RULE_GetNumFrontPos( rule );
  if( numFrontPos > 1 )
  {
    // 元々の対象が１体で、
    if( BTL_POKESET_GetCount(rec) == 1 )
    {
      // 攻撃ポケの敵で、かつ死んでいる
      BTL_POKEPARAM* defaulTarget = BTL_POKESET_Get( rec, 0 );
      if( BPP_IsDead(defaulTarget)
      &&  !BTL_MAINUTIL_IsFriendPokeID( BPP_GetID(attacker), BPP_GetID(defaulTarget) )
      ){
        u8  nextTargetPos[ BTL_POS_MAX ];
        BTL_POKEPARAM* nextTarget[ BTL_POSIDX_MAX ];
        BtlPokePos  atPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(attacker) );
        BtlExPos    exPos;
        u8 nextTargetCnt, aliveCnt, i;

        BTL_N_Printf( DBGSTR_SVFL_CorrectTarget_Info,
          BPP_GetID(attacker), atPos, BPP_GetID(defaulTarget), targetPos);

      // 元々の対象は死んでるので除外しておく
        BTL_POKESET_Remove( rec, defaulTarget );

      // 元々の対象範囲を列挙
        if( WAZADATA_GetFlag(wazaParam->wazaID, WAZAFLAG_TripleFar) ){
          exPos = EXPOS_MAKE( BTL_EXPOS_FULL_ENEMY, atPos );
          BTL_N_Printf( DBGSTR_SVFL_CorrectHitFarOn, wazaParam->wazaID);
        }else{
          exPos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, atPos );
          BTL_N_Printf( DBGSTR_SVFL_CorrectHitFarOff, wazaParam->wazaID);
        }
        nextTargetCnt = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, nextTargetPos );

      // 元々の対象範囲から生きてるポケを列挙
        for(i=0; i<NELEMS(nextTarget); ++i){
          nextTarget[i] = NULL;
        }
        aliveCnt = 0;
        for(i=0; i<nextTargetCnt; ++i)
        {
          nextTarget[aliveCnt] = BTL_POKECON_GetFrontPokeData( wk->pokeCon, nextTargetPos[i] );
          if( !BPP_IsDead(nextTarget[aliveCnt]) ){
            nextTargetPos[ aliveCnt ] = nextTargetPos[i]; // 位置情報もIndexを揃えておく
            ++aliveCnt;
          }
        }

        BTL_N_Printf( DBGSTR_SVFL_CorrectTargetNum, aliveCnt, nextTargetCnt );
        if( aliveCnt == 0 ){
          return FALSE;
        }

        // 対象が２体じゃなければ１体のハズなので、候補リストの先頭を狙う
        if( aliveCnt != 2 ){
          i = 0;
        }
        // 候補が２体の場合（トリプルの場合のみ起こりうる）
        else
        {
          // 基本は、元々の対象に近い方を狙う
          int diff_0 = BTL_CALC_ABS( (int)(nextTargetPos[0]) - (int)targetPos );
          int diff_1 = BTL_CALC_ABS( (int)(nextTargetPos[1]) - (int)targetPos );

          BTL_N_Printf( DBGSTR_SVFL_CorrectTargetDiff,
            BPP_GetID(nextTarget[0]), diff_0, BPP_GetID(nextTarget[1]), diff_1);

          // 元々の対象との距離も同じ場合（真ん中を狙った場合のみ起こりうる）、HPの少ない方を狙う
          if( diff_0 == diff_1 )
          {
            diff_0 = BPP_GetValue( nextTarget[0], BPP_HP );
            diff_1 = BPP_GetValue( nextTarget[1], BPP_HP );
            BTL_N_Printf( DBGSTR_SVFL_CorrectTargetHP,
              BPP_GetID(nextTarget[0]), diff_0, BPP_GetID(nextTarget[1]), diff_1);

          }

          if( diff_0 < diff_1 ){
            i = 0;
          }else if( diff_1 < diff_0 ){
            i = 1;
          }else{
            // HPも同じならランダム
            i = BTL_CALC_GetRand( 2 );
          }
        }

        if( nextTarget[i] != NULL )
        {
          BTL_N_Printf( DBGSTR_SVFL_CorrectResult,
            nextTargetPos[i], BPP_GetID(nextTarget[i]) );
          BTL_POKESET_Add( rec, nextTarget[ i ] );
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
/**
 * 指定位置から見て対戦相手のポケモンデータを返す
 */
static BTL_POKEPARAM* get_opponent_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos, u8 pokeSideIdx )
{
  SVCL_WORK* clwk;
  BtlPokePos targetPos;
  u8 clientID, posIdx;

  targetPos = BTL_MAIN_GetOpponentPokePos( wk->mainModule, pos, pokeSideIdx );
  BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, targetPos, &clientID, &posIdx );

  clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

  return BTL_PARTY_GetMemberData( clwk->party, posIdx );
}
/**
 * 指定位置から見て隣のポケモンデータを返す
 */
static BTL_POKEPARAM* get_next_pokeparam( BTL_SVFLOW_WORK* wk, BtlPokePos pos )
{
  SVCL_WORK* clwk;
  BtlPokePos nextPos;
  u8 clientID, posIdx;

  nextPos = BTL_MAIN_GetNextPokePos( wk->mainModule, pos );
  BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, nextPos, &clientID, &posIdx );
  clwk = BTL_SERVER_GetClientWork( wk->server, clientID );

  return BTL_PARTY_GetMemberData( clwk->party, posIdx );
}

static u8 checkTemptPokeID( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u8 targetPokeID )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  u8 temptPokeID = scEvent_GetWazaTargetTempt( wk, attacker, wazaParam, targetPokeID );
  BTL_Hem_PopState( &wk->HEManager, hem_state );
  return temptPokeID;
}

//----------------------------------------------------------------------------------
/**
 * [Event] ワザターゲットを自動決定（攻撃側のハンドラによる）
 *
 * @param   wk
 * @param   attacker
 * @param   wazaParam
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 scEvent_GetWazaTargetIntr( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam )
{
  u8 pokeID = BTL_POKEID_NULL;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_POKEID_DEF, pokeID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_FIX_FLAG, FALSE );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_DECIDE_TARGET );
    pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
  BTL_EVENTVAR_Pop();
  return pokeID;
}
//----------------------------------------------------------------------------------
/**
 * [Event] ワザターゲットを強制書き換え（ゆびをふる等、防御側のハンドラによる）
 *
 * @param   wk
 * @param   attacker
 * @param   wazaParam
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 scEvent_GetWazaTargetTempt( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* attacker, const SVFL_WAZAPARAM* wazaParam, u8 defaultTargetPokeID )
{
  u8 pokeID;
  BTL_EVENTVAR_Push();
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_POKEID_ATK, BPP_GetID(attacker) );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZA_TYPE, wazaParam->wazaType );
    BTL_EVENTVAR_SetConstValue( BTL_EVAR_WAZAID, wazaParam->wazaID );
    BTL_EVENTVAR_SetRewriteOnceValue( BTL_EVAR_POKEID_DEF, defaultTargetPokeID );
    BTL_EVENT_CallHandlers( wk, BTL_EVENT_TEMPT_TARGET );
    pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
  BTL_EVENTVAR_Pop();
  if( pokeID != defaultTargetPokeID ){
    return pokeID;
  }
  return BTL_POKEID_NULL;
}




/*====================================================================================================*/
/*                                                                                                    */
/*  経験値計算処理                                                                                    */
/*                                                                                                    */
/*====================================================================================================*/

//----------------------------------------------------------------------------------
/**
 * 経験値計算結果を専用ワークに出力
 *
 * @param   wk
 * @param   party     プレイヤー側パーティ
 * @param   deadPoke  しんだポケモン
 * @param   result    [out] 計算結果格納先
 */
//----------------------------------------------------------------------------------
void BTL_SVFSUB_CalcExp( BTL_SVFLOW_WORK* wk, BTL_PARTY* party, const BTL_POKEPARAM* deadPoke, CALC_EXP_WORK* result )
{
  u32 baseExp = BTL_CALC_CalcBaseExp( deadPoke );
  u16 memberCount  = BTL_PARTY_GetMemberCount( party );
  u16 gakusyuCount = 0;

  const BTL_POKEPARAM* bpp;
  u16 i;

  // トレーナー戦は1.5倍
  if( BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_TRAINER ){
    baseExp = (baseExp * 15) / 10;
  }

  // ワーククリア
  for(i=0; i<BTL_PARTY_MEMBER_MAX; ++i){
    GFL_STD_MemClear( &result[i], sizeof(CALC_EXP_WORK) );
  }

  // がくしゅうそうち分の経験値計算
  for(i=0; i<memberCount; ++i)
  {
    // がくしゅうそうちを装備しているポケモンが対象
    bpp = BTL_PARTY_GetMemberDataConst( party, i );
    if( !BPP_IsDead(bpp)
    &&  (BPP_GetItem(bpp) == ITEM_GAKUSYUUSOUTI)
    ){
      ++gakusyuCount;
    }
  }
  if( gakusyuCount )
  {
    u32 gakusyuExp = (baseExp / 2);
    baseExp -= gakusyuExp;

    gakusyuExp /= gakusyuCount;
    if( gakusyuExp == 0 ){
      gakusyuExp = 1;
    }

    for(i=0; i<memberCount; ++i)
    {
      bpp = BTL_PARTY_GetMemberDataConst( party, i );
      if( !BPP_IsDead(bpp)
      &&  (BPP_GetItem(bpp) == ITEM_GAKUSYUUSOUTI)
      ){
        result[i].exp = gakusyuExp;
      }
    }
  }

  BTL_N_Printf( DBGSTR_SVFL_ExpCalc_Base, baseExp );


  // 対戦経験値取得
  {
    u8 confrontCnt = BPP_CONFRONT_REC_GetCount( deadPoke );
    u8 aliveCnt = 0;
    u8 pokeID;

    u32 addExp;

    for(i=0; i<confrontCnt; ++i)
    {
      pokeID = BPP_CONFRONT_REC_GetPokeID( deadPoke, i );
      bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
      if( !BPP_IsDead(bpp) ){
        ++aliveCnt;
      }
    }
    BTL_N_Printf( DBGSTR_SVFL_ExpCalc_MetInfo, BPP_GetID(deadPoke), confrontCnt, aliveCnt);

    addExp = baseExp / aliveCnt;
    if( addExp == 0 ){
      addExp = 1;
    }

    for(i=0; i<memberCount; ++i)
    {
      bpp = BTL_PARTY_GetMemberDataConst( party, i );
      if( !BPP_IsDead(bpp) )
      {
        u8 j;
        pokeID = BPP_GetID( bpp );
        for(j=0; j<confrontCnt; ++j)
        {
          if( BPP_CONFRONT_REC_GetPokeID(deadPoke, j) == pokeID ){
            BTL_N_Printf( DBGSTR_SVFL_ExpCalc_DivideInfo, i, addExp);
            result[i].exp += addExp;
          }
        }
      }
    }
  }

  // ボーナス計算
  for(i=0; i<memberCount; ++i)
  {
    if( result[i].exp )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );
      const POKEMON_PARAM* pp;
      u16 myLv, enemyLv;
      bpp = BTL_PARTY_GetMemberDataConst( party, i );
      pp = BPP_GetSrcData( bpp );

      // 倒したポケとのレベル差によって経験値が増減する（WBから追加された仕様）
      myLv = BPP_GetValue( bpp, BPP_LEVEL );
      enemyLv = BPP_GetValue( deadPoke, BPP_LEVEL );

      result[i].exp = getexp_calc_adjust_level( bpp, result[i].exp, myLv, enemyLv );

      // 他人が親ならボーナス
      if( !PP_IsMatchOya(pp, status) )
      {
        // 外国の親ならx1.7，そうじゃなければx1.5
        fx32 ratio;
        ratio = ( PP_Get(pp, ID_PARA_country_code, NULL) != MyStatus_GetRegionCode(status) )?
            FX32_CONST(1.7f) : FX32_CONST(1.5f);

        result[i].exp = BTL_CALC_MulRatio( result[i].exp, ratio );
        result[i].fBonus = TRUE;
      }

      // しあわせタマゴ持ってたらボーナス x1.5
      if( BPP_GetItem(bpp) == ITEM_SIAWASETAMAGO ){
        result[i].exp = BTL_CALC_MulRatio( result[i].exp, FX32_CONST(1.5f) );
        result[i].fBonus = TRUE;
      }

      // Ｇパワー補正
      result[i].exp = GPOWER_Calc_Exp( result[i].exp );

      BTL_N_Printf( DBGSTR_SVFL_ExpCalc_Result, i, result[i].exp);
    }
  }

  // 努力値計算
  for(i=0; i<memberCount; ++i)
  {
    if( result[i].exp )
    {
      BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberData( party, i );
      PutDoryokuExp( bpp, deadPoke );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * 倒されたポケモンと経験値を取得するポケモンのレベル差に応じて経験値を補正する（ＷＢより）
 *
 * @param   base_exp      基本経験値（GSまでの仕様による計算結果）
 * @param   getpoke_lv    経験値を取得するポケモンのレベル
 * @param   deadpoke_lv   倒されたポケモンのレベル
 *
 * @retval  u32   補正後経験値
 */
//----------------------------------------------------------------------------------
static u32 getexp_calc_adjust_level( const BTL_POKEPARAM* bpp, u32 base_exp, u16 getpoke_lv, u16 deadpoke_lv )
{
  u32 denom, numer;
  u32  denom_int, numer_int, result, expMargin;

  numer_int = deadpoke_lv * 2 + 10;
  denom_int = deadpoke_lv + getpoke_lv + 10;

  numer = _calc_adjust_level_sub( numer_int );
  denom = _calc_adjust_level_sub( denom_int );

  result = ((base_exp * numer) / denom) + 1;

  expMargin = BPP_GetExpMargin( bpp );
  if( result > expMargin ){
    result = expMargin;
  }

  BTL_N_Printf( DBGSTR_SVFL_ExpAdjustCalc,
      getpoke_lv, deadpoke_lv, numer, denom, base_exp, result );

  return result;
}
/**
 *  経験値補正計算サブ(2.5乗）
 */
static inline u32 _calc_adjust_level_sub( u32 val )
{
  fx32  fx_val, fx_sqrt;
  u64 result;

  fx_val = FX32_CONST( val );
  fx_sqrt = FX_Sqrt( fx_val );
  val *= val;
  result = (val * fx_sqrt) >> FX32_SHIFT;

  return result;
}
/**
 *  努力値計算、結果をSrcデータに反映させる
 */
static void PutDoryokuExp( BTL_POKEPARAM* bpp, const BTL_POKEPARAM* deadPoke )
{
  /**
   *  ステータスIndex
   */
  enum {
    _HP = 0,
    _POW,
    _DEF,
    _AGI,
    _SPOW,
    _SDEF,
    _PARAM_MAX,
  };

  /**
   *  関連パラメータ（上記ステータスIndex順）
   */
  static const struct {
    u8   personalParamID;   ///< パーソナル贈与努力値ID
    u16  pokeParamID;       ///< POKEMON_PARAM 努力値ID
    u16  boostItemID;       ///< 努力値加算アイテムID
  }RelationTbl[] = {

    { POKEPER_ID_pains_hp,      ID_PARA_hp_exp,     ITEM_PAWAAUEITO   },  // HP
    { POKEPER_ID_pains_pow,     ID_PARA_pow_exp,    ITEM_PAWAARISUTO  },  // 攻撃
    { POKEPER_ID_pains_def,     ID_PARA_def_exp,    ITEM_PAWAABERUTO  },  // 防御
    { POKEPER_ID_pains_agi,     ID_PARA_agi_exp,    ITEM_PAWAAANKURU  },  // 素早さ
    { POKEPER_ID_pains_spepow,  ID_PARA_spepow_exp, ITEM_PAWAARENZU   },  // 特攻
    { POKEPER_ID_pains_spedef,  ID_PARA_spedef_exp, ITEM_PAWAABANDO   },  // 特防

  };

  u16 mons_no = BPP_GetMonsNo( deadPoke );
  u16 form_no = BPP_GetValue( deadPoke, BPP_FORM );

  u8  exp[ _PARAM_MAX ];
  u8  i;

  // 基本努力値をパーソナルから取得
  for(i=0; i<_PARAM_MAX; ++i){
    exp[ i ] = POKETOOL_GetPersonalParam( mons_no, form_no, RelationTbl[i].personalParamID );
  }

  // きょうせいギプスで倍
  if( BPP_GetItem(bpp) == ITEM_KYOUSEIGIPUSU )
  {
    for(i=0; i<_PARAM_MAX; ++i){
      exp[ i ] *= 2;
    }
  }

  // 各種、努力値増加アイテム分を加算
  for(i=0; i<_PARAM_MAX; ++i)
  {
    if( BPP_GetItem(bpp) == RelationTbl[i].boostItemID ){
      exp[ i ] += BTL_CALC_ITEM_GetParam( RelationTbl[i].boostItemID, ITEM_PRM_ATTACK );
    }
  }

  // Srcデータに反映
  {
    POKEMON_PARAM* pp = (POKEMON_PARAM*)BPP_GetSrcData( bpp );
    BOOL fFastMode = PP_FastModeOn( pp );

      // ポケルスで倍
      if( PP_Get(pp, ID_PARA_pokerus, NULL) )
      {
        for(i=0; i<_PARAM_MAX; ++i){
          exp[ i ] *= 2;
        }
      }

      // 努力値合計を取得
      for(i=0; i<_PARAM_MAX; ++i)
      {
        if( exp[i] )
        {
          u32 sum = exp[i] + PP_Get( pp, RelationTbl[i].pokeParamID, NULL );
          if( sum > PARA_EXP_MAX ){
            sum = PARA_EXP_MAX;
          }
          PP_Put( pp, RelationTbl[i].pokeParamID, sum );
        }
      }

    PP_FastModeOff( pp, fFastMode );
  }
}


/*====================================================================================================*/
/*                                                                                                    */
/*  トレーナーアイテム使用                                                                            */
/*                                                                                                    */
/*====================================================================================================*/
TrItemResult BTL_SVFSUB_TrainerItemProc( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, u8 actParam, u8 targetIdx )
{
  enum {
    RANGE_FULL,   ///< 前衛・後衛・控えに全て効く
    RANGE_VIEW,   ///< 前衛・後衛（画面に見えている）に効く
    RANGE_FRONT,  ///< 前衛にのみ効く
  };
  enum {
    AREA_FRONT,    ///< 前衛
    AREA_BACK,     ///< 後衛（ローテのみ）
    AREA_RESERVE,  ///< 控え
  };

  typedef u8 (*pItemEffFunc)( BTL_SVFLOW_WORK*, BTL_POKEPARAM*, u16, int, u8 );

  // 通常アイテム効果＆範囲テーブル
  static const struct {
    u16            effect;
    u8             range;
    u8             fShooterOnly;
    pItemEffFunc   func;
  }ItemEffectTable[] = {
    { ITEM_PRM_SLEEP_RCV,     RANGE_FULL,  FALSE, ItemEff_SleepRcv      },   // 眠り回復
    { ITEM_PRM_POISON_RCV,    RANGE_FULL,  FALSE, ItemEff_PoisonRcv     },   // 毒回復
    { ITEM_PRM_BURN_RCV,      RANGE_FULL,  FALSE, ItemEff_YakedoRcv     },   // 火傷回復
    { ITEM_PRM_ICE_RCV,       RANGE_FULL,  FALSE, ItemEff_KooriRcv      },   // 氷回復
    { ITEM_PRM_PARALYZE_RCV,  RANGE_FULL,  FALSE, ItemEff_MahiRcv       },   // 麻痺回復
    { ITEM_PRM_PANIC_RCV,     RANGE_FULL,  FALSE, ItemEff_KonranRcv     },   // 混乱回復
    { ITEM_PRM_MEROMERO_RCV,  RANGE_FULL,  FALSE, ItemEff_MeromeroRcv   },   // メンタル回復
    { ITEM_PRM_ABILITY_GUARD, RANGE_VIEW,  FALSE, ItemEff_EffectGuard   },   // 能力ガード
    { ITEM_PRM_DEATH_RCV,     RANGE_FULL,  FALSE, ItemEff_Relive        },   // 瀕死回復
    { ITEM_PRM_ATTACK_UP,     RANGE_VIEW,  FALSE, ItemEff_AttackRank    },   // 攻撃力アップ
    { ITEM_PRM_DEFENCE_UP,    RANGE_VIEW,  FALSE, ItemEff_DefenceRank   },   // 防御力アップ
    { ITEM_PRM_SP_ATTACK_UP,  RANGE_VIEW,  FALSE, ItemEff_SPAttackRank  },   // 特攻アップ
    { ITEM_PRM_SP_DEFENCE_UP, RANGE_VIEW,  FALSE, ItemEff_SPDefenceRank },   // 特防アップ
    { ITEM_PRM_AGILITY_UP,    RANGE_VIEW,  FALSE, ItemEff_AgilityRank   },   // 素早さアップ
    { ITEM_PRM_HIT_UP,        RANGE_VIEW,  FALSE, ItemEff_HitRank       },   // 命中率アップ
    { ITEM_PRM_CRITICAL_UP,   RANGE_VIEW,  FALSE, ItemEff_CriticalUp    },   // クリティカル率アップ
    { ITEM_PRM_PP_RCV,        RANGE_FULL,  FALSE, ItemEff_PP_Rcv        },   // PP回復
    { ITEM_PRM_ALL_PP_RCV,    RANGE_FULL,  FALSE, ItemEff_AllPP_Rcv     },   // PP回復（全ての技）
    { ITEM_PRM_HP_RCV,        RANGE_FULL,  FALSE, ItemEff_HP_Rcv        },   // HP回復
    // 以下シューター専用
    { ITEM_AITEMUKOORU,       RANGE_FRONT, TRUE,  ShooterEff_ItemCall  },
    { ITEM_SUKIRUKOORU,       RANGE_FRONT, TRUE,  ShooterEff_SkillCall },
    { ITEM_AITEMUDOROPPU,     RANGE_FRONT, TRUE,  ShooterEff_ItemDrop  },
    { ITEM_HURATTOKOORU,      RANGE_FRONT, TRUE,  ShooterEff_FlatCall  },
  };

  u8 clientID = BTL_MAINUTIL_PokeIDtoClientID( BPP_GetID(bpp) );
  u8 targetPokeID, targetArea;
  BTL_POKEPARAM* target = NULL;
  int i, itemParam;
  u32 hem_state;
  BtlPokePos targetPos;

  // 対象ポケモンの各種情報決定
  if( targetIdx != BTL_PARTY_MEMBER_MAX )
  {
    BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
    u8 frontPosCount = BTL_MAIN_GetClientFrontPosCount( wk->mainModule, clientID );

    target = BTL_PARTY_GetMemberData( party, targetIdx );
    targetPokeID = BPP_GetID( target );
    targetPos = BTL_POSPOKE_GetPokeExistPos( &wk->pospokeWork, targetPokeID );
    targetArea = (targetIdx < frontPosCount)? AREA_FRONT : AREA_RESERVE;
    if( (BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION) && (targetArea == AREA_RESERVE) )
    {
      if( targetIdx < BTL_ROTATION_VISIBLE_POS_NUM )
      {
        targetArea = AREA_BACK;
      }
    }
    BTL_N_Printf( DBGSTR_SVFL_TrainerItemTarget, targetIdx, targetPokeID, targetPos );
  }

  // ○○は××を使った！  メッセージ
  if( wk->bagMode != BBAG_MODE_SHOOTER )
  {
    // 通常バッグ用
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_UseItem_Self, clientID, itemID );

    // ボール投げならボール投げシーケンスへ
    if( BTL_CALC_ITEM_GetParam(itemID, ITEM_PRM_ITEM_TYPE) == ITEMTYPE_BALL )
    {
      if( scproc_TrainerItem_BallRoot( wk, bpp, itemID ) ){
        BTL_MAIN_DecrementPlayerItem( wk->mainModule, clientID, itemID );
      }
      return TRITEM_RESULT_NORMAL;
    }
    // 逃げアイテムなら逃げシーケンスへ
    if( BTL_CALC_ITEM_GetParam(itemID, ITEM_PRM_BATTLE) == ITEMUSE_BTL_ESCAPE )
    {
      BTL_MAIN_DecrementPlayerItem( wk->mainModule, clientID, itemID );
      return TRITEM_RESULT_ESCAPE;
    }
  }
  else{
    // シューター用
    SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_UseItem_Shooter, clientID, targetPokeID, itemID );

    if( clientID == BTL_MAIN_GetPlayerClientID(wk->mainModule) ){
      BTL_MAIN_RECORDDATA_Inc( wk->mainModule, RECID_USE_SHOOTER_COUNT );
    }
  }


  // その場に効く道具か判定
  {
    // 前衛にあたるIndexで生きてるなら何でも効く
    BOOL fEffective = ((targetArea == AREA_FRONT) && (targetPos!=BTL_POS_NULL));

    if( fEffective == FALSE )
    {
      for(i=0; i<NELEMS(ItemEffectTable); ++i)
      {
        if( ((ItemEffectTable[i].fShooterOnly) && (itemID == ItemEffectTable[i].effect))
        ||  ((!ItemEffectTable[i].fShooterOnly) && (BTL_CALC_ITEM_GetParam(itemID, ItemEffectTable[i].effect)))
        ){
          u8 range = ItemEffectTable[i].range;

          // アイテム自体が何処でもOK型 -> 効く
          if( range == RANGE_FULL ){
            fEffective = TRUE;
            break;
          }
          // アイテムが可視範囲のみOK型 -> 控えでなく生きてるなら効く
          if( range == RANGE_VIEW )
          {
            if( (targetArea < AREA_RESERVE)
            &&  (targetPos != BTL_POS_NULL)
            ){
              fEffective = TRUE;
              break;
            }
          }
        }
      }
    }

    // 効かないよ
    if( !fEffective ){
      TAYA_Printf("アイテムの効果がある範囲に居ない、あるいは生存してない targetIdx=%d\n", targetIdx);
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_ItemNoEffPos );
      return TRITEM_RESULT_NORMAL;
    }
  }

  if( wk->bagMode == BBAG_MODE_SHOOTER )
  {
    // シューターのかっこいい演出
    if( targetPos != BTL_POS_NULL ){
      SCQUE_PUT_ACT_EffectByPos( wk->que, targetPos, BTLEFF_SHOOTER_EFFECT );
    }

    // シューター専用のアイテム処理
    for(i=0; i<NELEMS(ItemEffectTable); ++i)
    {
      if( ItemEffectTable[i].fShooterOnly && (itemID == ItemEffectTable[i].effect) )
      {
        u16 que_reserve_pos;

        if( !BTL_SVFTOOL_IsTameHidePoke(wk, targetPokeID) )
        {
          que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_EFFECT_BYPOS );
          if( ItemEffectTable[i].func(wk, target, itemID, 0, actParam) )
          {
            SCQUE_PUT_ReservedPos( wk->que, que_reserve_pos, SC_ACT_EFFECT_BYPOS, targetPos, BTLEFF_USE_ITEM );
            SCQUE_PUT_ACT_EffectSimple( wk->que, BTLEFF_CAMERA_INIT );// USE_ITEM SHOOTER_EFFECT のカメラ状態を初期化
            return TRITEM_RESULT_NORMAL;
          }
        }
        SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_UseItem_NoEffect );
        SCQUE_PUT_ACT_EffectSimple( wk->que, BTLEFF_CAMERA_INIT );// USE_ITEM SHOOTER_EFFECT のカメラ状態を初期化
        return TRITEM_RESULT_NORMAL;
      }
    }
  }

  // 通常のアイテム処理
  {
    BOOL fEffective = FALSE;
    u16  que_reserve_pos = SCQUE_RESERVE_Pos( wk->que, SC_ACT_EFFECT_BYPOS );

    hem_state = BTL_Hem_PushState( &wk->HEManager );
    for(i=0; i<NELEMS(ItemEffectTable); ++i)
    {
      itemParam = BTL_CALC_ITEM_GetParam( itemID, ItemEffectTable[i].effect );
      if( itemParam )
      {
        TAYA_Printf("アイテムID=%d, 効果=%dあり Idx=%d\n", itemID, itemParam, i);
        if( ItemEffectTable[i].func(wk, target, itemID, itemParam, actParam) ){
          fEffective = TRUE;
        }
      }
    }
    // 使用効果あり
    if( fEffective )
    {

      {
        u16 effectNo = (targetPos!=BTL_POS_NULL)? BTLEFF_USE_ITEM : BTLEFF_BENCH_RECOVERY;
        SCQUE_PUT_ReservedPos( wk->que, que_reserve_pos, SC_ACT_EFFECT_BYPOS, targetPos, effectNo );
      }

      if( wk->bagMode != BBAG_MODE_SHOOTER )
      {
        BTL_MAIN_DecrementPlayerItem( wk->mainModule, clientID, itemID );
        BTL_MAIN_CalcNatsukiItemUse( wk->mainModule, target, itemID );
      }
    }
    else{
      SCQUE_PUT_MSG_STD( wk->que, BTL_STRID_STD_UseItem_NoEffect );
    }
    BTL_Hem_PopState( &wk->HEManager, hem_state );
  }

  SCQUE_PUT_ACT_EffectSimple( wk->que, BTLEFF_CAMERA_INIT ); // USE_ITEM SHOOTER_EFFECT のカメラ状態を初期化

  return TRITEM_RESULT_NORMAL;
}

//----------------------------------------------------------------------------------
/**
 * ボール使用ルート
 *
 * @param   wk
 * @param   bpp
 * @param   itemID
 *
 * @retval  ボールを消費する場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL scproc_TrainerItem_BallRoot( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID )
{
  BtlPokePos  targetPos = BTL_POS_NULL;
  BTL_POKEPARAM* targetPoke = NULL;

  // 投げる位置（生きているポケモンをシークして最初のヒット位置）を決める
  {
    u8 posAry[ BTL_POS_MAX ];
    u8 basePos, posCnt, i;
    BtlExPos exPos;

    basePos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(bpp) );
    exPos = EXPOS_MAKE( BTL_EXPOS_FULL_ENEMY, basePos );
    posCnt = BTL_MAIN_ExpandBtlPos( wk->mainModule, exPos, posAry );

    for(i=0; i<posCnt; ++i)
    {
      targetPoke = BTL_POKECON_GetFrontPokeData( wk->pokeCon, posAry[i] );
      if( !BPP_IsDead(targetPoke) ){
        targetPos = posAry[i];
        break;
      }
    }
  }

  // 野生戦 = 通常処理
  if( BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_WILD )
  {
    if( targetPos != BTL_POS_NULL )
    {
      u8 yure_cnt, fSuccess, fCritical, fZukanRegister;

      fSuccess = CalcCapturePokemon( wk, bpp, targetPoke, itemID, &yure_cnt, &fCritical );
//      OS_TPrintf("捕獲 .. critical=%d\n", fCritical);

      if( fSuccess )
      {
        wk->flowResult = SVFLOW_RESULT_POKE_GET;
        wk->getPokePos = targetPos;
        fZukanRegister = !BTL_MAIN_IsZukanRegistered( wk->mainModule, targetPoke );
        BPP_SetCaptureBallID( targetPoke, itemID );
      }
      else{
        fZukanRegister = FALSE;
      }

      SCQUE_PUT_ACT_BallThrow( wk->que, targetPos, yure_cnt, fSuccess, fZukanRegister, fCritical, itemID );
    }

    return TRUE;
  }
  // トレーナー戦では失敗（ボールは減らない）
  else
  {
    if( targetPos != BTL_POS_NULL ){
      SCQUE_PUT_ACT_BallThrowTrainer( wk->que, targetPos, itemID );
    }
    return FALSE;
  }
}
//----------------------------------------------------------------------------------
/**
 * ボール投げ効果計算
 *
 * @param   wk
 * @param   myPoke
 * @param   targetPoke
 * @param   ballID
 * @param   yure_cnt    [out] 揺れ演出の発生回数
 * @param   fCritical   [out] クリティカルフラグ
 *
 * @retval  BOOL    捕獲できる場合はTRUE
 */
//----------------------------------------------------------------------------------
static BOOL CalcCapturePokemon( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* myPoke, const BTL_POKEPARAM* targetPoke,u16 ballID,
    u8* yure_cnt, u8* fCritical )
{
  *fCritical = FALSE;

  if( ballID == ITEM_MASUTAABOORU ){
    *yure_cnt = BTL_CAPTURE_BALL_YURE_CNT_MAX;
    return TRUE;
  }
  else
  {
    u32 hp_base = BPP_GetValue( targetPoke, BPP_MAX_HP ) * 3;
    u32 hp_value = ( hp_base - BPP_GetValue(targetPoke, BPP_HP) * 2 );
    fx32 capture_value = FX32_CONST( hp_value );

//    OS_TPrintf("捕獲値：初期 %08x(%d) ..MaxHPx3=%d, HPVal=%d\n", capture_value, capture_value>>FX32_SHIFT, hp_base, hp_value );

    // 草むらＢ補正
    if( BTL_MAIN_GetSetupStatusFlag(wk->mainModule, BTL_STATUS_FLAG_HIGH_LV_ENC) )
    {
      fx32 kusa_ratio = GetKusamuraCaptureRatio( wk );
      capture_value = FX_Mul( capture_value, kusa_ratio );
    }

    // 種族ごとの捕獲値計算
    {
      u16 mons_no = BPP_GetMonsNo( targetPoke );
      u16 form_no = BPP_GetValue( targetPoke, BPP_FORM );
      u16 ratio = POKETOOL_GetPersonalParam( mons_no, form_no, POKEPER_ID_get_rate );
      capture_value *= ratio;
//      OS_TPrintf("捕獲値：種族補正 %08x(%d) ..（monsno=%d, ratio=%d)\n",
//          capture_value, capture_value>>FX32_SHIFT, mons_no, ratio);
    }

    // ボールによる捕獲率補正
    {
      fx32 ball_ratio = CalcBallCaptureRatio( wk, myPoke, targetPoke, ballID );
      capture_value = FX_Mul( capture_value, ball_ratio );

//      OS_TPrintf("捕獲値：ボール補正 %08x(%d) ..（BallID=%d, ratio=%08x)\n",
//          capture_value, capture_value>>FX32_SHIFT, ballID, ball_ratio);
    }

    capture_value /= hp_base;
//    OS_TPrintf("捕獲値：基本 %08x(%d) .. (MaxHPx3 = %d)\n",
//          capture_value, capture_value>>FX32_SHIFT, hp_base);


    // 状態異常による補正
    switch( BPP_GetPokeSick(targetPoke) ){
    case POKESICK_NEMURI:
    case POKESICK_KOORI:
      capture_value = FX_Mul( capture_value, FX32_CONST(2.5) );
      break;
    case POKESICK_DOKU:
    case POKESICK_MAHI:
    case POKESICK_YAKEDO:
      capture_value = FX_Mul( capture_value, FX32_CONST(1.5) );
      break;
    }
  //  OS_TPrintf("捕獲値：最終 %08x(%d)\n",
//          capture_value, capture_value>>FX32_SHIFT);

    // Ｇパワー補正
    capture_value = GPOWER_Calc_Capture( capture_value );

    // クリティカルチェック
    *fCritical = CheckCaptureCritical( wk, capture_value );

    if( capture_value >= FX32_CONST(255) ){
      *yure_cnt = (*fCritical)? 1 : BTL_CAPTURE_BALL_YURE_CNT_MAX;
      return TRUE;
    }
    else
    {
      s32 check_value;
      u32 check_count, captured_count, i;

      check_count = (*fCritical)? 1 : BTL_CAPTURE_BALL_YURE_CNT_MAX;

      capture_value = FX_Div( FX32_CONST(255), capture_value );
      capture_value = FX_Sqrt( capture_value );
      capture_value = FX_Sqrt( capture_value );

      capture_value = FX_Div( FX32_CONST(65536), capture_value );
      check_value = FX_Whole( capture_value );
//      OS_TPrintf("CheckValue=%08x(%d)\n", capture_value, check_value);
      *yure_cnt = 0;
      for(i=0; i<check_count; ++i)
      {
        if( BTL_CALC_GetRand(65536) < check_value ){
          ++(*yure_cnt);
        }else{
          return FALSE;
        }
      }
      return TRUE;
    }
  }
}
/**
 *  草むら補正率を取得
 */
static fx32 GetKusamuraCaptureRatio( BTL_SVFLOW_WORK* wk )
{
  u32 capturedPokeCnt = BTL_MAIN_GetZukanCapturedCount( wk->mainModule );

  if( capturedPokeCnt > 600 ){
    return FX32_CONST(1);
  }
  if( capturedPokeCnt > 450 ){
    return FX32_CONST(0.9f);
  }
  if( capturedPokeCnt > 300 ){
    return FX32_CONST(0.8f);
  }
  if( capturedPokeCnt > 150 ){
    return FX32_CONST(0.7f);
  }
  if( capturedPokeCnt > 30 ){
    return FX32_CONST(0.5f);
  }

  return FX32_CONST(0.3f);

}
/**
 *  ボールによる捕獲率計算
 */
static fx32 CalcBallCaptureRatio( BTL_SVFLOW_WORK* wk, const BTL_POKEPARAM* myPoke, const BTL_POKEPARAM* targetPoke, u16 ballID )
{
  switch( ballID ){
  case ITEM_SUUPAABOORU:  return FX32_CONST(1.5f);
  case ITEM_HAIPAABOORU:  return FX32_CONST(2.0f);

  case ITEM_NETTOBOORU: // ネットボール
    if( BPP_IsMatchType(targetPoke, POKETYPE_MIZU) || BPP_IsMatchType(targetPoke, POKETYPE_MUSHI) ){
      return FX32_CONST(3);
    }
    break;

  case ITEM_DAIBUBOORU: // ネットボール
    if( BTL_MAIN_GetSetupStatusFlag(wk->mainModule, BTL_STATUS_FLAG_FISHING) ){
      return FX32_CONST(3.5f);
    }
    {
      const BTL_FIELD_SITUATION* fldSit = BTL_MAIN_GetFieldSituation( wk->mainModule );
      if( fldSit->bgAttr == BATTLE_BG_ATTR_WATER ){
        return FX32_CONST(3.5f);
      }
    }
    break;

  case ITEM_NESUTOBOORU: // ネストボール
    {
      u16 targetLevel = BPP_GetValue( targetPoke, BPP_LEVEL );

      if( targetLevel < 30 )
      {
        u16 levelDiff = 41 - targetLevel;
        if( levelDiff > 40 ){
          levelDiff = 40;
        }
        return FX32_CONST(levelDiff) / 10;
      }
    }
    break;

  case ITEM_RIPIITOBOORU: // リピートボール
    if( BTL_MAIN_IsZukanRegistered(wk->mainModule, targetPoke) ){
      return FX32_CONST(3);
    }
    break;

  case ITEM_TAIMAABOORU:  // タイマーボール
    {
      fx32 ratio = FX32_CONST(1) + FX32_CONST(0.3f) * wk->turnCount;
      if( ratio > FX32_CONST(4) ){
        ratio = FX32_CONST(4);
      }
      return ratio;
    }
    break;

  case ITEM_DAAKUBOORU:  // ダークボール
    {
      const BTL_FIELD_SITUATION* fldSit = BTL_MAIN_GetFieldSituation( wk->mainModule );
      u8  timezone = PM_RTC_ConvertHourToTimeZone( fldSit->season, fldSit->hour );

      if( (fldSit->bgType == BATTLE_BG_TYPE_CAVE) || (fldSit->bgType == BATTLE_BG_TYPE_CAVE_DARK) ){
        return FX32_CONST(3.5);
      }
      if( (BTL_TABLES_IsOutdoorBGType(fldSit->bgType))
      &&  ((timezone == TIMEZONE_NIGHT) || (timezone == TIMEZONE_MIDNIGHT))
      ){
        return FX32_CONST(3.5);
      }
    }
    break;

  case ITEM_KUIKKUBOORU:  // クイックボール
    if( wk->turnCount == 0 )
    {
      return FX32_CONST(5);
    }
    break;

  }
  return FX32_CONST(1);
}
//----------------------------------------------------------------------------------
/**
 * 捕獲クリティカル発生チェック
 *
 * @param   wk
 * @param   capture_value   計算後の捕獲変数（固定小数 0～255）
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL CheckCaptureCritical( BTL_SVFLOW_WORK* wk, fx32 capture_value )
{
  u32 capturedPokeCnt = BTL_MAIN_GetZukanCapturedCount( wk->mainModule );

  fx32 ratio = 0;

  if( capturedPokeCnt > 600 ){
    ratio = FX32_CONST(2.5);
  }else if(  capturedPokeCnt > 450 ){
    ratio = FX32_CONST(2.0f);
  }else if(  capturedPokeCnt > 300 ){
    ratio = FX32_CONST(1.5f);
  }else if(  capturedPokeCnt > 150 ){
    ratio = FX32_CONST(1.0f);
  }else if(  capturedPokeCnt > 30 ){
    ratio = FX32_CONST(0.5f);
  }else{
    return FALSE;
  }

  if( capture_value > FX32_CONST(255) ){
    capture_value = FX32_CONST(255);
  }
  capture_value = FX_Mul( capture_value, ratio ) / 6;
  if( BTL_CALC_GetRand(256) < FX_Whole(capture_value) )
  {
    return TRUE;
  }
  return FALSE;
}



// アイテム効果：ねむり回復
static u8 ItemEff_SleepRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_NEMURI );
}
static u8 ItemEff_PoisonRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_DOKU );
}
static u8 ItemEff_YakedoRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_YAKEDO );
}
static u8 ItemEff_KooriRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_KOORI );
}
static u8 ItemEff_MahiRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_MAHI );
}
static u8 ItemEff_KonranRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Cure( wk, bpp, itemID, itemParam, WAZASICK_KONRAN );
}
static u8 ItemEff_MeromeroRcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Mental_Cure( wk, bpp, itemID, itemParam, WAZASICK_MEROMERO );
}
static u8 ItemEff_EffectGuard( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  BtlSide side = BTL_MAINUTIL_PokeIDtoSide( BPP_GetID(bpp) );
  BPP_SICK_CONT cont = BPP_SICKCONT_MakeTurn( 5 );
  if( BTL_HANDLER_SIDE_Add(side, BTL_SIDEEFF_SIROIKIRI, cont) )
  {
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_MESSAGE, BTL_POKEID_NULL );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_SiroiKiri );
      HANDEX_STR_AddArg( &param->str, side );
    BTL_SVF_HANDEX_Pop( wk, param );
    return TRUE;
  }
  return FALSE;
}
static u8 ItemEff_Relive( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  if( BPP_IsDead(bpp))
  {
    u8 pokeID = BPP_GetID( bpp );
    BTL_HANDEX_PARAM_RELIVE* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RELIVE, pokeID );
      param->pokeID = pokeID;
      itemParam = BTL_CALC_ITEM_GetParam( itemID, ITEM_PRM_HP_RCV_POINT );
      switch( itemParam ){
      case ITEM_RECOVER_HP_FULL:
        param->recoverHP = BPP_GetValue( bpp, BPP_MAX_HP ); break;
      case ITEM_RECOVER_HP_HALF:
        param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 2 ); break;
      case ITEM_RECOVER_HP_QUOT:
        param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 4 ); break;
      default:
        param->recoverHP = BTL_CALC_ITEM_GetParam( itemID, ITEM_PRM_HP_RCV_POINT ); break;
      }

      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Relive );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
    BTL_SVF_HANDEX_Pop( wk, param );

//    TAYA_Printf("ポケモン(ID:%d)生き返る -> HP=%d\n", pokeID, param->recoverHP);

    // 当ターンに生き返ったポケモンを記録
    BTL_SVFRET_AddRelivePokeRecord( wk, pokeID );
    return TRUE;
  }
  return FALSE;
}
static u8 ItemEff_AttackRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_ATTACK_RANK );
}
static u8 ItemEff_DefenceRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_DEFENCE_RANK );
}
static u8 ItemEff_SPAttackRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_SP_ATTACK_RANK );
}
static u8 ItemEff_SPDefenceRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_SP_DEFENCE_RANK );
}
static u8 ItemEff_AgilityRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_AGILITY_RANK );
}
static u8 ItemEff_HitRank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  return ItemEff_Common_Rank( wk, bpp, itemID, itemParam, BPP_HIT_RATIO );
}
static u8 ItemEff_CriticalUp( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  u8 result = FALSE;
  if( !BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_KIAIDAME) )
  {
    BPP_CONTFLAG_Set( bpp, BPP_CONTFLG_KIAIDAME );
    SCQUE_PUT_OP_SetContFlag( wk->que, BPP_GetID(bpp), BPP_CONTFLG_KIAIDAME );

    result = TRUE;
  }
  if( itemParam > 1 ){
    --itemParam;
    if( BPP_AddCriticalRank(bpp, itemParam) ){
      SCQUE_PUT_OP_AddCritical( wk->que, BPP_GetID(bpp), itemParam );
      result = TRUE;
    }
  }
  if( result )
  {
    SCQUE_PUT_MSG_SET( wk->que, BTL_STRID_SET_KiaiDame, BPP_GetID(bpp) );
    return TRUE;
  }
  return FALSE;
}
static u8 ItemEff_PP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  if( BPP_WAZA_GetCount_Org(bpp) > actParam )
  {
    u8 pokeID = BPP_GetID( bpp );
    u8 volume = BPP_WAZA_GetPPShort_Org( bpp, actParam );
    u8 ppValue = BTL_CALC_ITEM_GetParam( itemID, ITEM_PRM_PP_RCV_POINT );
    if( ppValue != ITEM_RECOVER_PP_FULL ){
      if( volume > ppValue ){
        volume = ppValue;
      }
    }
    if( volume )
    {
      BTL_HANDEX_PARAM_PP* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RECOVER_PP, pokeID );
        param->volume = volume;
        param->pokeID = pokeID;
        param->wazaIdx = actParam;
        param->fDeadPokeEnable = TRUE;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_PP_Recover );
        HANDEX_STR_AddArg( &param->exStr, pokeID );
        HANDEX_STR_AddArg( &param->exStr, BPP_WAZA_GetID_Org(bpp, actParam) );
      BTL_SVF_HANDEX_Pop( wk, param );
      return 1;
    }
  }
  return 0;
}
static u8 ItemEff_AllPP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  BTL_HANDEX_PARAM_MESSAGE* msg_param;
  u32 cnt, volume, i;
  u8 pokeID = BPP_GetID( bpp );
  u8 ppValue = BTL_CALC_ITEM_GetParam( itemID, ITEM_PRM_PP_RCV_POINT );
  u8 fEffective = FALSE;

  cnt = BPP_WAZA_GetCount_Org( bpp );

  for(i=0; i<cnt; ++i)
  {
    volume = BPP_WAZA_GetPPShort_Org( bpp, i );
    if( volume > ppValue ){
      volume = ppValue;
    }

    if( volume )
    {
      BTL_HANDEX_PARAM_PP* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RECOVER_PP, pokeID );
        param->wazaIdx = i;
        param->pokeID = pokeID;
        param->volume = volume;
        param->fDeadPokeEnable = TRUE;
      BTL_SVF_HANDEX_Pop( wk, param );
      fEffective = TRUE;
    }
  }

  if( fEffective )
  {
    msg_param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_PP_AllRecover );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    BTL_SVF_HANDEX_Pop( wk, msg_param );
  }

  return fEffective;
}
static u8 ItemEff_HP_Rcv( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  if( !BPP_IsHPFull(bpp)
  &&  !BPP_IsDead(bpp)
  ){
    u8 pokeID = BPP_GetID( bpp );
    BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RECOVER_HP, pokeID );
      param->pokeID = pokeID;
      param->fFailCheckThru = TRUE;
      itemParam = BTL_CALC_ITEM_GetParam( itemID, ITEM_PRM_HP_RCV_POINT );
      switch( itemParam ){
      case ITEM_RECOVER_HP_FULL:
        param->recoverHP = BPP_GetValue( bpp, BPP_MAX_HP ); break;
      case ITEM_RECOVER_HP_HALF:
        param->recoverHP = BTL_CALC_QuotMaxHP(bpp, 2 ); break;
      case ITEM_RECOVER_HP_QUOT:
        param->recoverHP = BTL_CALC_QuotMaxHP(bpp, 4 ); break;
      default:
        param->recoverHP = itemParam; break;
      }

      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_HP_Recover );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
    BTL_SVF_HANDEX_Pop( wk, param );
    return 1;
  }
  return 0;
}

/**
 *  状態異常治し系共通
 */
static u8 ItemEff_Common_Cure( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, WazaSick sickID )
{
  if( !BPP_IsDead(bpp) )
  {
    if( BPP_CheckSick(bpp, sickID) )
    {
      BTL_HANDEX_PARAM_CURE_SICK* param;
      u8 pokeID = BPP_GetID( bpp );
      param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_CURE_SICK, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->sickCode = sickID;
      BTL_SVF_HANDEX_Pop( wk, param );
      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  メンタルハーブ専用
 */
static u8 ItemEff_Mental_Cure( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, WazaSick sickID )
{
  if( !BPP_IsDead(bpp) && (!BTL_POSPOKE_IsExist(&wk->pospokeWork, BPP_GetID(bpp))) )
  {
    WazaSick sickID;
    u8  result = 0;
    u32 idx = 0;
    while( 1 )
    {
      sickID = BTL_TABLES_GetMentalSickID( idx++ );
      if( sickID == WAZASICK_NULL ){
        break;
      }
      if( BPP_CheckSick(bpp, sickID) )
      {
        u8 pokeID = BPP_GetID( bpp );
        BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_CURE_SICK, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->sickCode = sickID;
        BTL_SVF_HANDEX_Pop( wk, param );
        result = 1;
      }
    }
    return result;
  }
  return FALSE;

}
/**
 *  ランクアップ系共通
 */
static u8 ItemEff_Common_Rank( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, BppValueID rankType )
{
  u8 pokeID = BPP_GetID( bpp );

  if( (BTL_POSPOKE_GetPokeExistPos(&wk->pospokeWork, pokeID) != BTL_POS_NULL )
  &&  !BPP_IsDead(bpp)
  ){
    if( BPP_IsRankEffectValid(bpp, rankType, itemParam) )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->rankType = rankType;
        param->rankVolume = itemParam;
        param->fAlmost = TRUE;
      BTL_SVF_HANDEX_Pop( wk, param );
      return 1;
    }
  }
  else
  {
    BTL_Printf("場に出ていないのでランク効果なし\n");
  }

  return 0;
}
/**
 *  シューター専用：アイテムコール
 */
static u8 ShooterEff_ItemCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  u16 equipItemID = BPP_GetItem( bpp );
  if( !BTL_TABLES_CheckItemCallNoEffect(equipItemID) )
  {
    u8 pokeID = BPP_GetID( bpp );

    BTL_N_Printf( DBGSTR_SVFL_UseItemCall, BPP_GetID(bpp) );

    return BTL_SVFRET_UseItemEquip( wk, bpp );
  }
  return FALSE;
}
/**
 *  シューター専用：スキルコール
 */
static u8 ShooterEff_SkillCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  if( BPP_GetValue(bpp, BPP_TOKUSEI) != POKETOKUSEI_KAWARIMONO )
  {
    u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
    HandExResult  result;

    BTL_SVFRET_EventSkillCall( wk, bpp );
    result = BTL_SVF_HandEx_Result( wk );
    BTL_Hem_PopState( &wk->HEManager, hem_state );

    TAYA_Printf("スキルコールresult=%d\n", result);

    return (result == HandExResult_Enable);
  }
  return FALSE;
}
/**
 *  シューター専用：アイテムドロップ
 */
static u8 ShooterEff_ItemDrop( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  if( !HandCommon_CheckCantChangeItemPoke(wk, BPP_GetID(bpp)) )
  {
    u16 equipItemID = BPP_GetItem( bpp );
    if( equipItemID != ITEM_DUMMY_DATA )
    {
      u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
      {
        BTL_HANDEX_PARAM_SET_ITEM* param;
        u8 pokeID = BPP_GetID( bpp );

        param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_SET_ITEM, pokeID );
          param->pokeID = pokeID;
          param->itemID = ITEM_DUMMY_DATA;
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Shooter_ItemDrop );
          HANDEX_STR_AddArg( &param->exStr, pokeID );
          HANDEX_STR_AddArg( &param->exStr, equipItemID );
        BTL_SVF_HANDEX_Pop( wk, param );
      }
      BTL_Hem_PopState( &wk->HEManager, hem_state );
      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  シューター専用：フラットコール
 */
static u8 ShooterEff_FlatCall( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, int itemParam, u8 actParam )
{
  u32 hem_state = BTL_Hem_PushState( &wk->HEManager );
  {
    BTL_HANDEX_PARAM_RESET_RANK* reset_param;
    BTL_HANDEX_PARAM_MESSAGE* msg_param;
    u8 pokeID = BPP_GetID( bpp );

    reset_param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_RESET_RANK, pokeID );
      reset_param->poke_cnt = 1;
      reset_param->pokeID[0] = pokeID;
    BTL_SVF_HANDEX_Pop( wk, reset_param );

    msg_param = BTL_SVF_HANDEX_Push( wk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Shooter_FlatCall );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    BTL_SVF_HANDEX_Pop( wk, msg_param );
  }
  BTL_Hem_PopState( &wk->HEManager, hem_state );
  return TRUE;
}


/*====================================================================================================*/
/*                                                                                                    */
/*  勝敗判定                                                                                          */
/*                                                                                                    */
/*====================================================================================================*/

//=============================================================================================
/**
 * 勝敗判定
 *
 * @param   wk
 *
 * @retval  BtlResult   勝敗コード
 */
//=============================================================================================
BtlResult BTL_SVFSUB_CheckBattleResult( BTL_SVFLOW_WORK* wk )
{
  u8 alivePokeCnt[2], totalPokeCnt[2];
  u8 playerClientID;
  u32 i;

// 両陣営の生き残りポケ数＆総ポケ数カウント
  alivePokeCnt[0] = alivePokeCnt[1] = 0;
  totalPokeCnt[0] = totalPokeCnt[1] = 0;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk->mainModule, i) )
    {
      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, i );
      if( BTL_PARTY_GetMemberCount(party) )
      {
        u8 side = BTL_MAIN_GetClientSide( wk->mainModule, i );
        alivePokeCnt[ side ] += BTL_PARTY_GetAliveMemberCount( party );
        totalPokeCnt[ side ] += BTL_PARTY_GetMemberCount( party );
      }
    }
  }
  BTL_N_Printf( DBGSTR_SVFS_Result_RestCnt, alivePokeCnt[0], alivePokeCnt[1] );

  {
    u8 playerClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
    u8 playerSide = BTL_MAIN_GetClientSide( wk->mainModule, playerClientID );
    u8 enemySide = playerSide ^ 1;

    // どちらか全滅、もう一方は生き残りのケース -> 簡単
    if( (alivePokeCnt[playerSide] == 0) && (alivePokeCnt[enemySide] != 0) )
    {
      return BTL_RESULT_LOSE;
    }
    if( (alivePokeCnt[enemySide] == 0) && (alivePokeCnt[playerSide] != 0) )
    {
      return BTL_RESULT_WIN;
    }

    // 厳密判定が不要ならこの時点で引き分け
    if( !BTL_MAIN_IsResultStrictMode(wk->mainModule) ){
      return BTL_RESULT_DRAW;
    }

    //---------------------------------------------------
    /*
     *  サブウェイ・通信対戦用の厳密判定
     */
    //---------------------------------------------------


    // 両方全滅 -> 最後に倒れた側の勝ち
    if( alivePokeCnt[0] == 0 )
    {
      u8 lastPokeID = BTL_DEADREC_GetLastDeadPokeID( &wk->deadRec );
      if( lastPokeID != BTL_POKEID_NULL )
      {
        u8 lastPokeSide = BTL_MAINUTIL_PokeIDtoSide( lastPokeID );

        BTL_N_Printf( DBGSTR_SVFS_LastPokeID, lastPokeID );

        return (lastPokeSide == playerSide)? BTL_RESULT_WIN : BTL_RESULT_LOSE;
      }
      else{
        // （有り得ないけどねんのため）
        return BTL_RESULT_DRAW;
      }
    }
    // 両方生き残り（タイムアウト等）
    else
    {
      return CheckResult_AliveBoth( wk, alivePokeCnt, totalPokeCnt, playerSide );
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * 両陣営生き残り時の厳密な勝敗判定
 *
 * @param   wk
 * @param   playerSide
 *
 * @retval  BtlResult
 */
//----------------------------------------------------------------------------------
static BtlResult CheckResult_AliveBoth( BTL_SVFLOW_WORK* wk, const u8* alivePokeCnt, const u8* totalPokeCnt, u8 playerSide )
{
  int deadPoke[ 2 ];
  u8 winSide;

  // 倒したポケ数で優勢な方が勝ち
  deadPoke[0] = totalPokeCnt[0] - alivePokeCnt[0];
  deadPoke[1] = totalPokeCnt[1] - alivePokeCnt[1];
  BTL_N_Printf( DBGSTR_SVFL_DeadPokeCount, deadPoke[0], deadPoke[1]);

  if( deadPoke[0] != deadPoke[1] ){
    winSide = (deadPoke[0] < deadPoke[1])? 0 : 1;
    return (winSide == playerSide)? BTL_RESULT_WIN : BTL_RESULT_LOSE;
  }

  // …それでも引き分けなら残りHP割合で優勢な方の勝ち
  {
    int HP_total[ 2 ];
    int HP_ratio[ 2 ];

    calcHPRatio( wk, &HP_total[0], &HP_ratio[0], 0 );
    calcHPRatio( wk, &HP_total[1], &HP_ratio[1], 1 );

    BTL_N_Printf( DBGSTR_SVFL_HPRatio, 0, HP_total[0], HP_ratio[0] );
    BTL_N_Printf( DBGSTR_SVFL_HPRatio, 1, HP_total[1], HP_ratio[1] );

    if( HP_ratio[0] != HP_ratio[1] ){
      winSide = (HP_ratio[0] > HP_ratio[1])? 0 : 1;
      return (winSide == playerSide)? BTL_RESULT_WIN : BTL_RESULT_LOSE;
    }
    // …まだ引き分けなら残りHP総和で優勢な方の勝ち
    if( HP_total[0] != HP_total[1] ){
      winSide = (HP_total[0] > HP_total[1])? 0 : 1;
      return (winSide == playerSide)? BTL_RESULT_WIN : BTL_RESULT_LOSE;
    }

    return BTL_RESULT_DRAW;
  }
}


//----------------------------------------------------------------------------------
/**
 * 指定陣営側パーティのHP残総和＆割合計算
 *
 * @param   wk
 * @param   HP_sum      [out] 総和を格納
 * @param   HP_ratio    [out] 割合（%）を格納
 * @param   side
 */
//----------------------------------------------------------------------------------
static void calcHPRatio( BTL_SVFLOW_WORK* wk, int* HP_sum, int* HP_ratio, u8 side )
{
  u32 i;
  u32 HPMax_sum = 0;

  *HP_sum = 0;
  *HP_ratio = 0;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( (BTL_MAIN_IsExistClient(wk->mainModule, i))
    &&  (BTL_MAIN_GetClientSide(wk->mainModule, i) == side)
    ){
      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, i );
      u32 member_count = BTL_PARTY_GetMemberCount( party );
      u32 p;
      for(p=0; p<member_count; ++p)
      {
        const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( party, p );
        HPMax_sum += BPP_GetValue( bpp, BPP_MAX_HP );
        (*HP_sum) += BPP_GetValue( bpp, BPP_HP );
      }
    }
  }


  *HP_ratio = ((*HP_sum)*100) / HPMax_sum;
}


/*====================================================================================================*/
/*                                                                                                    */
/*  相性記録ワーク                                                                                    */
/*                                                                                                    */
/*====================================================================================================*/

//----------------------------------------------------------------------------------
/**
 * 相性記録ワーク：初期化
 *
 * @param   rec
 */
//----------------------------------------------------------------------------------
void BTL_DMGAFF_REC_Init( BTL_DMGAFF_REC* rec )
{
  u32 i;
  for(i=0; i<NELEMS(rec->aff); ++i){
    rec->aff[i] = BTL_TYPEAFF_MAX;
  }
}
//----------------------------------------------------------------------------------
/**
 * 相性記録ワーク：１体分追加
 *
 * @param   rec
 * @param   pokeID
 * @param   aff
 */
//----------------------------------------------------------------------------------
void BTL_DMGAFF_REC_Add( BTL_DMGAFF_REC* rec, u8 pokeID, BtlTypeAff aff )
{
  GF_ASSERT(pokeID < BTL_POKEID_MAX);
  rec->aff[ pokeID ] = aff;
}
//----------------------------------------------------------------------------------
/**
 * 相性記録ワーク：１体分取得（記録が無ければASSERT停止）
 *
 * @param   rec
 * @param   pokeID
 */
//----------------------------------------------------------------------------------
BtlTypeAff BTL_DMGAFF_REC_Get( const BTL_DMGAFF_REC* rec, u8 pokeID )
{
  GF_ASSERT(pokeID < BTL_POKEID_MAX);
  GF_ASSERT(rec->aff[pokeID] != BTL_TYPEAFF_MAX);

  return rec->aff[ pokeID ];
}

//----------------------------------------------------------------------------------
/**
 * 相性記録ワーク：１体分取得（記録がなければ BTL_TYPEAFF_NULL が帰る）
 *
 * @param   rec
 * @param   pokeID
 */
//----------------------------------------------------------------------------------
BtlTypeAff BTL_DMGAFF_REC_GetIfEnable( const BTL_DMGAFF_REC* rec, u8 pokeID )
{
  GF_ASSERT(pokeID < BTL_POKEID_MAX);

  return rec->aff[ pokeID ];
}
