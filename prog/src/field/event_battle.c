//======================================================================
/**
 * @file  event_battle.c
 * @brief イベント：フィールドバトル
 * @author  tamada GAMEFREAK inc.
 * @date  2008.01.19
 */
//======================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/btl_setup.h"

#include "field/fieldmap.h"

#include "./event_fieldmap_control.h"
#include "./event_battle.h"

#include "sound/pm_sndsys.h"

#include "battle/battle.h"
#include "system/main.h" //HEAPID_PROC参照

//#include "poke_tool/monsno_def.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "waza_tool/wazano_def.h"

#include "event_encount_effect.h"
#include "effect_encount.h"
#include "field_sound.h"
#include "event_gameover.h" //EVENT_NormalLose
#include "move_pokemon.h"
#include "event_battle_return.h"

#include "script_def.h"   //SCR_BATTLE_～

#include "enceffno_def.h"
#include "enceffno.h"

#include "gamesystem/game_beacon.h"
#include "tr_tool/trtype_def.h"
#include "tr_tool/tr_tool.h"
#include "battle/battle.h"

#include "event_mission_help_after.h"

#include "debug/debug_flg.h"

#include "trial_house.h"

#include "sound/wb_sound_data.sadl" //サウンドラベルファイル

//======================================================================
//  define
//======================================================================
enum {
  BATTLE_BGM_FADEOUT_WAIT = 60,
  BATTLE_BGM_FADEIN_WAIT = 60,
};

typedef void (*TR_BEACON_SET_FUNC)( u16 tr_no );

typedef enum{
 TR_BEACON_NORMAL,
 TR_BEACON_LEADER,
 TR_BEACON_BIGFOUR,
 TR_BEACON_CHAMPION,
 TR_BEACON_TYPE_MAX,
}TRAINER_BEACON_TYPE;

typedef enum{
 BTL_BEACON_ST_START,
 BTL_BEACON_ST_WIN,
 BTL_BEACON_ST_CAPTURE,
 BTL_BEACON_ST_MAX,
}BTL_BEACON_ST;

static const TR_BEACON_SET_FUNC DATA_TrBeaconSetFuncTbl[TR_BEACON_TYPE_MAX][2] = {
 { GAMEBEACON_Set_BattleTrainerStart, GAMEBEACON_Set_BattleTrainerVictory },
 { GAMEBEACON_Set_BattleLeaderStart, GAMEBEACON_Set_BattleLeaderVictory },
 { GAMEBEACON_Set_BattleBigFourStart, GAMEBEACON_Set_BattleBigFourVictory },
 { GAMEBEACON_Set_BattleChampionStart, GAMEBEACON_Set_BattleChampionVictory },
};

static u8 DATA_TrBeaconLeaderTbl[] = {
 TRTYPE_LEADER1A,
 TRTYPE_LEADER1B,
 TRTYPE_LEADER1C,
 TRTYPE_LEADER2,
 TRTYPE_LEADER3,
 TRTYPE_LEADER4,
 TRTYPE_LEADER5,
 TRTYPE_LEADER6,
 TRTYPE_LEADER7,
 TRTYPE_LEADER8A,
 TRTYPE_LEADER8B,
};
#define TR_BEACON_LEADER_NUM NELEMS( DATA_TrBeaconLeaderTbl)

static u8 DATA_TrBeaconBigFourTbl[] = {
 TRTYPE_BIGFOUR1,
 TRTYPE_BIGFOUR2,
 TRTYPE_BIGFOUR3,
 TRTYPE_BIGFOUR4,
};
#define TR_BEACON_BIGFOUR_NUM NELEMS( DATA_TrBeaconLeaderTbl)

static u8 DATA_TrBeaconChampTbl[] = {
 TRTYPE_SAGE1,
};
#define TR_BEACON_CHAMP_NUM NELEMS( DATA_TrBeaconChampTbl )



//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/**
 * @struct  BATTLE_EVENT_WORK
 * @brief 戦闘呼び出しイベントの制御ワーク定義
 *
 */
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;    ///<GAMESYS_WORKへのポインタ
  GAMEDATA * gamedata;    ///<GAMEDATAへのポインタ（簡易アクセスのため）

  ///戦闘呼び出し用パラメータへのポインタ
  BATTLE_SETUP_PARAM* battle_param;

  ///戦闘後処理（ずかん追加画面、進化画面など）呼び出し用のパラメータ
  BTLRET_PARAM        btlret_param;

  ///BGMがすでに退避済みか？フラグ
  BOOL bgm_pushed_flag;

  /** @brief  サブイベントかどうか？フラグ
   * サブイベントの場合は、敗北処理など終了部分を呼び出し元から
   * 呼び出してもらう必要があるため、フックする
   */
  BOOL is_sub_event;

  /** @brief  敗北処理がないバトルか？のフラグ */
  BOOL is_no_lose;

  BOOL Examination;   //トライアルハウス審査処理を行うか？TRUEで行う
  
  BOOL not_free_bsp; //戦闘後、BATTLE_SETUP_PARAMを開放しない

  //エンカウントエフェクトナンバー
  int EncEffNo;

}BATTLE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );

static void BeaconReq_BtlTrainer( u16 tr_id, BTL_BEACON_ST state );
static void BeaconReq_BtlWild( BATTLE_SETUP_PARAM* bp, BTL_BEACON_ST state );
static void BeaconReq_BattleEnd( BATTLE_EVENT_WORK* bew );

static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew);
static void BEW_reflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata);
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM* bp);
static void BEW_Destructor(BATTLE_EVENT_WORK * bew);


//======================================================================
//
//
//  フィールド　バトルイベント
//
//
//======================================================================
#ifdef PM_DEBUG
static void debug_FieldDebugFlagSet( BATTLE_SETUP_PARAM* bp )
{
  if( DEBUG_FLG_GetFlg( DEBUG_FLG_ShortcutBtlIn )){
    BTL_SETUP_SetDebugFlag( bp, BTL_DEBUGFLAG_SKIP_BTLIN );
  }
}
#endif  //PM_DEBUG

//--------------------------------------------------------------
//  野生戦用ラッパーイベントのパラメータ
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  BATTLE_SETUP_PARAM * bp;
  BOOL sub_event_f;
  ENCOUNT_TYPE EncType;
}WILD_BATTLE_EVENT_WORK;

//--------------------------------------------------------------
//  野生戦用ラッパーイベント
//--------------------------------------------------------------
static GMEVENT_RESULT wildBattleEvent( GMEVENT * event, int *seq, void *wk )
{
  WILD_BATTLE_EVENT_WORK * wbew = wk;
  GAMESYS_WORK * gsys =  wbew->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  switch (*seq)
  {
  case 0:
    { //バトルイベントの呼び出し
      GMEVENT * battle_main_event;
      BATTLE_EVENT_WORK * bew;
      battle_main_event = GMEVENT_Create( gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );
      bew = GMEVENT_GetEventWork( battle_main_event );
      BEW_Initialize( bew, gsys, wbew->bp );
      bew->is_sub_event = TRUE; //このイベントが存在する以上、常にバトルはサブイベント
      //bew->is_sub_event = wbew->sub_event_f;

      //バトルセットアップパラムから戦闘するポケモンを取得
      {
        POKEPARTY* enemy = BATTLE_PARAM_GetPokePartyPointer( wbew->bp, BTL_CLIENT_ENEMY1 );
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer(enemy, 0);
        int monsno = PP_Get( pp, ID_PARA_monsno, NULL );
        FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
        //エンカウントエフェクトとＢＧＭを取得してセット
        ENCEFFNO_GetWildEncEffNoBgmNo( monsno, wbew->EncType, fieldmap, &bew->EncEffNo, &bew->battle_param->musicDefault );
      }

      GMEVENT_CallEvent( event, battle_main_event );
    }
    (*seq) ++;
    break;

  case 1:
    //このイベント自体がサブイベントの場合、何もせずに戻る
    if ( wbew->sub_event_f == TRUE )
    {
      return GMEVENT_RES_FINISH;
    }

    //このイベントがトップのイベントの場合、戦闘後処理を行う
    if (FIELD_BATTLE_IsLoseResult(GAMEDATA_GetLastBattleResult(gamedata), BTL_COMPETITOR_WILD) == TRUE)
    {
      //負けた場合は敗北処理へ
      GMEVENT_ChangeEvent( event, EVENT_NormalLose(gsys) );
    }
    else
    {
      //負けていない場合はフェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, GAMESYSTEM_GetFieldMapWork(gsys), FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;

  case 2:
    //侵入相手がたすけてくれたメッセージイベント
    {
      COMM_PLAYER_SUPPORT *cps = GAMEDATA_GetCommPlayerSupportPtr(gamedata);
      if(COMM_PLAYER_SUPPORT_GetBattleEndSupportType(cps) == TRUE){
        GMEVENT_CallEvent(event, EVENT_Intrude_BattleHelpAfterEvent( gsys, HEAPID_PROC ));
      }
    }
    (*seq)++;
    break;
  
  case 3:
    return GMEVENT_RES_FINISH;
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フィールド野生ポケモンバトルイベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param bp
 * @param sub_event
 * @param enc_type
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_WildPokeBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp, BOOL sub_event_f, ENCOUNT_TYPE enc_type )
{
#if 0
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;

  event = GMEVENT_Create( gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = sub_event_f;
  bew->EncEffNo = enc_eff_no;     //エンカウントエフェクトセット
#endif
  GMEVENT * event = GMEVENT_Create( gsys, NULL, wildBattleEvent, sizeof(WILD_BATTLE_EVENT_WORK) );
  WILD_BATTLE_EVENT_WORK * wbew = GMEVENT_GetEventWork( event );
  wbew->gsys = gsys;
  wbew->bp = bp;
  wbew->sub_event_f = sub_event_f;
  wbew->EncType = enc_type;

#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif
  //すれ違いビーコン送信リクエスト
  BeaconReq_BtlWild( bp, BTL_BEACON_ST_START );

  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
  return event;
}

//--------------------------------------------------------------
/**
 * フィールドトレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int partner_id, int tr_id0, int tr_id1, u32 flags )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;
  BATTLE_SETUP_PARAM* bp;
  
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);

  {
    FIELD_ENCOUNT* enc = FIELDMAP_GetEncount(fieldmap);

    bp = BATTLE_PARAM_Create(HEAPID_PROC);
    FIELD_ENCOUNT_SetTrainerBattleParam( enc, bp, partner_id, tr_id0, tr_id1, HEAPID_PROC );

    BeaconReq_BtlTrainer( tr_id0, BTL_BEACON_ST_START );

#if 0
    //2009年12月末ロム用処理：ジムリーダー戦のみ、BGMを変更する
    switch (tr_id0)
    {
    case TRID_LEADER1A_01:
    case TRID_LEADER1B_01:
    case TRID_LEADER1C_01:
    case TRID_LEADER2_01:
    case TRID_LEADER3_01:
    case TRID_LEADER4_01:
    case TRID_LEADER5_01:
    case TRID_LEADER6_01:
    case TRID_LEADER7_01:
    case TRID_LEADER8A_01:
    case TRID_LEADER8B_01:
      bp->musicDefault = SEQ_BGM_VS_GYMLEADER;
    }
#endif
  }

#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif

  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = TRUE;   //スクリプトから呼ばれる＝トップレベルのイベントでない
  if ( (flags & SCR_BATTLE_MODE_NOLOSE) != 0 )
  {
    bew->is_no_lose = TRUE;
  }
  //エンカウントエフェクトとＢＧＭをセット
  ENCEFFNO_GetTrEncEffNoBgmNo(tr_id0, fieldmap, &bew->EncEffNo, &bew->battle_param->musicDefault);
  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));

  return event;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_BSubwayTrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM *bp )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK *bew;
  
#if 0
  {
    POKEPARTY * myparty = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys));
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( myparty, 0 );
    PP_Put( pp, ID_PARA_hp, 1 );
  }
#endif
  
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );
  
#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif
  
  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  
#if 0
  bew->is_sub_event = TRUE; //サブイベント呼び出し
#else
  bew->is_sub_event = FALSE; //戦闘後のフェードイン目当て
#endif
  
  bew->is_no_lose = TRUE; //敗戦処理無し
  bew->not_free_bsp = TRUE; //開放はしない
  
  //エンカウントエフェクトとＢＧＭセット(サブウェイ固有)
  bew->EncEffNo = ENCEFFID_SUBWAY;
  bew->battle_param->musicDefault = SEQ_BGM_VS_SUBWAY_TRAINER;
  
  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
  return event;
}

//--------------------------------------------------------------
/**
 * トライアルハウストレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TrialHouseTrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM *bp )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK *bew;
  
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );
  
#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
#if 0
  bew->is_sub_event = TRUE; //サブイベント呼び出し
#else
  bew->is_sub_event = FALSE; //戦闘後のフェードイン目当て
#endif
  bew->Examination = TRUE;    //採点する
  bew->is_no_lose = TRUE; //敗戦処理無し
  //エンカウントエフェクトとＢＧＭセット(サブウェイと同じ) 
  bew->EncEffNo = ENCEFFID_SUBWAY;
  bew->battle_param->musicDefault = SEQ_BGM_VS_SUBWAY_TRAINER;

  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
  return event;
}

//--------------------------------------------------------------
/**
 * フィールド捕獲デモバトルイベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_CaptureDemoBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, HEAPID heapID )
{
  GMEVENT * event;
  GAMEDATA* gdata;
  BATTLE_EVENT_WORK * bew;
  BATTLE_SETUP_PARAM* bp;
  HEAPID tmpHeapID = GFL_HEAP_LOWID( heapID );

  gdata = GAMESYSTEM_GetGameData( gsys );
  //バトルパラメータセット
  bp = BATTLE_PARAM_Create( heapID );

  {
    BTL_FIELD_SITUATION sit;
    POKEPARTY* partyMine = PokeParty_AllocPartyWork( tmpHeapID );
    POKEPARTY* partyEnemy = PokeParty_AllocPartyWork( tmpHeapID );
    POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( tmpHeapID, POKETOOL_GetWorkSize() );

    PP_Setup( pp, MONSNO_TIRAAMHI, 5, 0 );
    PP_Put( pp, ID_PARA_waza1, WAZANO_HATAKU );
    PP_Put( pp, ID_PARA_waza2, WAZANO_NAKIGOE );
    PP_Put( pp, ID_PARA_waza3, WAZANO_NULL );
    PP_Put( pp, ID_PARA_waza4, WAZANO_NULL );
    PokeParty_Add( partyMine, pp );
    
    PP_Setup( pp, MONSNO_MINEZUMI, 2, 0 );
    PP_Put( pp, ID_PARA_waza1, WAZANO_TAIATARI );
    PP_Put( pp, ID_PARA_waza2, WAZANO_NIRAMITUKERU );
    PP_Put( pp, ID_PARA_waza3, WAZANO_NULL );
    PP_Put( pp, ID_PARA_waza4, WAZANO_NULL );
    PokeParty_Add( partyEnemy, pp );

    BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, gdata, fieldmap );
    sit.bgAttr = BATTLE_BG_ATTR_GRASS;
    sit.bgType = BATTLE_BG_TYPE_GRASS;
    BTL_SETUP_CaptureDemo( bp, gdata, partyMine, partyEnemy, &sit, heapID );

    GFL_HEAP_FreeMemory( pp );
    GFL_HEAP_FreeMemory( partyEnemy );
    GFL_HEAP_FreeMemory( partyMine );
  }

  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = TRUE;
  //エンカウントエフェクトとＢＧＭをセット
  ENCEFFNO_GetWildEncEffNoBgmNo( MONSNO_MINEZUMI, ENC_TYPE_DEMO, fieldmap, &bew->EncEffNo, &bew->battle_param->musicDefault );

  return event;
}

//--------------------------------------------------------------
/**
 * フィールドバトルイベント
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 *
 * @todo
 * 今は単一のイベントですべてのフィールド戦闘呼び出しを
 * 実行しているが、バリエーションが増えたところで
 * 分割するべきかどうかを検討する
 */
//--------------------------------------------------------------
static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work )
{
  BATTLE_EVENT_WORK * bew = work;
  GAMESYS_WORK * gsys = bew->gsys;
  GAMEDATA * gamedata = bew->gamedata;
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_SOUND * fsnd = GAMEDATA_GetFieldSound( gamedata );
  
  switch (*seq) {
  case 0:
    // 戦闘用ＢＧＭセット
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, bew->battle_param->musicDefault));
    bew->bgm_pushed_flag = TRUE;
    (*seq)++;
    break;
  case 1:
    //エンカウントエフェクト
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, bew->EncEffNo);
/**
    GMEVENT_CallEvent( event,
        EVENT_FieldEncountEffect(gsys,fieldmap) );
*/
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;
  case 3:
    //侵入システムに戦闘中であることを伝える
    {
      FIELD_STATUS *fldstatus = GAMEDATA_GetFieldStatus(gamedata);
      FIELD_STATUS_SetProcAction( fldstatus, PROC_ACTION_BATTLE );
      COMM_PLAYER_SUPPORT_Init(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
    }
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, bew->battle_param );
    (*seq)++;
    break;
  case 4:
    bew->btlret_param.btlResult = bew->battle_param;
    bew->btlret_param.gameData  = gamedata;
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle_return), &BtlRet_ProcData, &bew->btlret_param );
    (*seq)++;
    break;
  case 5:
    //侵入システムにフィールド中であることを伝える
    {
      FIELD_STATUS *fldstatus = GAMEDATA_GetFieldStatus(gamedata);
      FIELD_STATUS_SetProcAction( fldstatus, PROC_ACTION_FIELD );
      COMM_PLAYER_SUPPORT_SetBattleEnd(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
    }
    (*seq) ++;
    break;
  case 6: 
    if (bew->bgm_pushed_flag == TRUE) {
      GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle(gsys) );
      bew->bgm_pushed_flag = FALSE;
    }
    (*seq) ++;
    break;
  case 7: 
    //戦闘結果反映処理
    BEW_reflectBattleResult( bew, gamedata );

    //採点処理
    if (bew->Examination)
    {
      TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);
      NOZOMU_Printf("検定採点\n");
      TRIAL_HOUSE_AddBtlPoint( *ptr, bew->battle_param );
    }

    //勝ち負け判定
    if (bew->is_no_lose == FALSE && BEW_IsLoseResult( bew) == TRUE )
    {
      //負けの場合、イベントはここで終了。
      //復帰イベントへと遷移する
      if (bew->is_sub_event == TRUE) {
        PMSND_PauseBGM( TRUE );
        BEW_Destructor( bew );
        return GMEVENT_RES_FINISH;
      } else {
        PMSND_PauseBGM( TRUE );
        BEW_Destructor( bew );
        GMEVENT_ChangeEvent( event, EVENT_NormalLose(gsys) );
        return GMEVENT_RES_CONTINUE;  //ChangeEventではFINISHさせてはいけない
      }
    }
    else
    {
      //PMSND_FadeInBGM(BATTLE_BGM_FADEIN_WAIT);
      GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    }
    (*seq) ++;
    break;
  case 8:
    if ( bew->is_sub_event == FALSE )
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case 9:
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}


//======================================================================
//
//  ビーコン配信関連　サブ関数
//
//
//======================================================================

//--------------------------------------------------------------
/*
 *  @brief  トレーナービーコンタイプ取得
 */
//--------------------------------------------------------------
static u8 btl_trainer_GetBeaconType( u16 tr_id )
{
  int i;
  u16 tr_type = TT_TrainerDataParaGet( tr_id, ID_TD_tr_type );
  
  for(i = 0;i < TR_BEACON_CHAMP_NUM;i++){
    if( tr_type == DATA_TrBeaconChampTbl[i] ){
      return TR_BEACON_CHAMPION;
    }
  }
  for(i = 0;i < TR_BEACON_BIGFOUR_NUM;i++){
    if( tr_type == DATA_TrBeaconBigFourTbl[i] ){
      return TR_BEACON_BIGFOUR;
    }
  }
  for(i = 0;i < TR_BEACON_LEADER_NUM;i++){
    if( tr_type == DATA_TrBeaconLeaderTbl[i] ){
      return TR_BEACON_LEADER;
    }
  }
  return TR_BEACON_NORMAL;
}

//--------------------------------------------------------------
/*
 *  @brief  トレーナー戦闘ビーコンリクエスト
 */
//--------------------------------------------------------------
static void BeaconReq_BtlTrainer( u16 tr_id, BTL_BEACON_ST state )
{
  TRAINER_BEACON_TYPE type = btl_trainer_GetBeaconType( tr_id );

  DATA_TrBeaconSetFuncTbl[type][state]( tr_id );
}

//--------------------------------------------------------------
/*
 *  @brief  野生戦ビーコンリクエスト
 */
//--------------------------------------------------------------
static void BeaconReq_BtlWild( BATTLE_SETUP_PARAM* bp, BTL_BEACON_ST state )
{
  POKEPARTY* party = BATTLE_PARAM_GetPokePartyPointer( bp, BTL_CLIENT_ENEMY1 );
  u16 monsno = PP_Get( PokeParty_GetMemberPointer(party,0), ID_PARA_monsno, NULL );

  switch( state ){
  case BTL_BEACON_ST_START:
    GAMEBEACON_Set_BattlePokeStart( monsno );
    break;
  case BTL_BEACON_ST_WIN:
    GAMEBEACON_Set_BattlePokeVictory( monsno );
    break;
  case BTL_BEACON_ST_CAPTURE:
    monsno = PP_Get( PokeParty_GetMemberPointer(party,bp->capturedPokeIdx), ID_PARA_monsno, NULL );
    GAMEBEACON_Set_PokemonGet( monsno );
    break;
  }
}

//--------------------------------------------------------------
/*
 *  @brief  バトル終了時ビーコンリクエスト
 */
//--------------------------------------------------------------
static void BeaconReq_BattleEnd( BATTLE_EVENT_WORK* bew )
{
  BATTLE_SETUP_PARAM* bp = bew->battle_param;

  switch( bp->competitor ){
  case BTL_COMPETITOR_WILD:
    if( bp->result == BTL_RESULT_CAPTURE ){
      BeaconReq_BtlWild( bp, BTL_BEACON_ST_CAPTURE );
    }else if( bp->result == BTL_RESULT_WIN ){
      BeaconReq_BtlWild( bp, BTL_BEACON_ST_WIN );
    }
    break;
  case BTL_COMPETITOR_TRAINER:
    if( bp->result == BTL_RESULT_WIN ){
      BeaconReq_BtlTrainer( bp->tr_data[BTL_CLIENT_ENEMY1]->tr_id, BTL_BEACON_ST_WIN );
    }
    break;
  }
}

//======================================================================
//
//
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   バトル画面でのアトリビュート指定を返す
 * @param value   マップアトリビュート
 * @retval  BtlBgAttr
 */
//--------------------------------------------------------------
BtlBgAttr FIELD_BATTLE_GetBattleAttrID( MAPATTR_VALUE value )
{
  if ( MAPATTR_VALUE_CheckLawn(value) ) return BATTLE_BG_ATTR_LAWN; //芝生
  if ( MAPATTR_VALUE_CheckMonoGround(value) ) return BATTLE_BG_ATTR_NORMAL_GROUND; //通常地面
  if ( MAPATTR_VALUE_CheckSeasonGround1(value) ) return BATTLE_BG_ATTR_GROUND1; //地面１
  if ( MAPATTR_VALUE_CheckSeasonGround2(value) ) return BATTLE_BG_ATTR_GROUND2; //地面２
  if ( MAPATTR_VALUE_CheckEncountGrass(value) ) return BATTLE_BG_ATTR_GRASS;
  if ( MAPATTR_VALUE_CheckWaterType(value) ) return BATTLE_BG_ATTR_WATER; //みず
  if ( MAPATTR_VALUE_CheckSnowType(value) ) return BATTLE_BG_ATTR_SNOW;
  if ( MAPATTR_VALUE_CheckSandType(value) ) return BATTLE_BG_ATTR_SAND;
  if ( MAPATTR_VALUE_CheckMarsh(value) ) return BATTLE_BG_ATTR_MARSH;
  if ( MAPATTR_VALUE_CheckEncountGround(value) ) return BATTLE_BG_ATTR_CAVE; //どうくつ
  if ( MAPATTR_VALUE_CheckPool(value) ) return BATTLE_BG_ATTR_POOL;
  if ( MAPATTR_VALUE_CheckShoal(value) ) return BATTLE_BG_ATTR_SHOAL;
  if ( MAPATTR_VALUE_CheckIce(value) ) return BATTLE_BG_ATTR_ICE;
  if ( MAPATTR_VALUE_CheckEncountIndoor(value) ) return BATTLE_BG_ATTR_INDOOR;
  OS_Printf("Warning! GetBattleAttrID = 0x%02x\n", value ); //ふつーは定義済みのでいいだろ！
  return BATTLE_BG_ATTR_LAWN; //標準
}

//--------------------------------------------------------------
/**
 * @brief 「敗北処理」とするべきかどうかの判定
 * @param result  バトルシステムが返す戦闘結果
 * @param competitor  対戦相手の種類
 * @return  BOOL  TRUEのとき、敗北処理をするべき
 */
//--------------------------------------------------------------
BOOL FIELD_BATTLE_IsLoseResult(BtlResult result, BtlCompetitor competitor)
{
  enum {
    RES_LOSE = 0,
    RES_WIN,
    RES_ERR,
  };

  static const u8 result_table[ BTL_RESULT_MAX ][ BTL_COMPETITOR_MAX ] = {
    //野生        トレーナー  サブウェイ  通信
    { RES_LOSE,   RES_LOSE,   RES_LOSE,   RES_LOSE }, //BTL_RESULT_LOSE
    { RES_WIN,    RES_WIN,    RES_WIN,    RES_WIN  }, //BTL_RESULT_WIN
    { RES_LOSE,   RES_LOSE,   RES_LOSE,   RES_LOSE }, //BTL_RESULT_DRAW
    { RES_WIN,    RES_ERR,    RES_ERR,    RES_LOSE }, //BTL_RESULT_RUN
    { RES_WIN,    RES_ERR,    RES_WIN,    RES_WIN  }, //BTL_RESULT_RUN_ENEMY
    { RES_WIN,    RES_ERR,    RES_ERR,    RES_ERR  }, //BTL_RESULT_CAPTURE
  };

  SDK_COMPILER_ASSERT( BTL_COMPETITOR_WILD == 0 );
  SDK_COMPILER_ASSERT( BTL_COMPETITOR_TRAINER == 1 );
  SDK_COMPILER_ASSERT( BTL_COMPETITOR_SUBWAY == 2 );
  SDK_COMPILER_ASSERT( BTL_COMPETITOR_COMM == 3 );
  SDK_COMPILER_ASSERT( BTL_RESULT_CAPTURE == 5 );

  u8 lose_flag;
  GF_ASSERT_MSG( result <= BTL_RESULT_CAPTURE, "想定外のBtlResult(%d)\n", result );
  GF_ASSERT_MSG( competitor < BTL_COMPETITOR_MAX, "想定外のBtlCompetitor(%d)\n", competitor);

  if( competitor == BTL_COMPETITOR_DEMO_CAPTURE ){
    return FALSE;
  }
  lose_flag = result_table[result][competitor];
   
  if( lose_flag == RES_ERR )
  {
    OS_Printf("バトルからありえない結果(Result=%d, Competitor=%d)\n", result, competitor );
  }
  return (lose_flag == RES_LOSE);
}

//--------------------------------------------------------------
/**
 * @brief 野生戦　再戦コードチェック
 * @param result  バトルシステムが返す戦闘結果
 * @return  SCR_WILD_BTL_RES_XXXX
 */
//--------------------------------------------------------------
u8 FIELD_BATTLE_GetWildBattleRevengeCode(BtlResult result)
{
  enum {
    RES_ERR = 0xFF,
  };

  static const u8 result_table[6] = {
    RES_ERR,                 //BTL_RESULT_LOSE
    SCR_WILD_BTL_RET_WIN,    //BTL_RESULT_WIN
    RES_ERR,                 //BTL_RESULT_DRAW
    SCR_WILD_BTL_RET_ESCAPE, //BTL_RESULT_RUN
    SCR_WILD_BTL_RET_ESCAPE, //BTL_RESULT_RUN_ENEMY
    SCR_WILD_BTL_RET_CAPTURE,//BTL_RESULT_CAPTURE
  };

  SDK_COMPILER_ASSERT( BTL_COMPETITOR_WILD == 0 );

  GF_ASSERT_MSG( result <= BTL_RESULT_CAPTURE, "想定外のBtlResult(%d)\n", result );
  if( result_table[result] == RES_ERR )
  {
    OS_Printf("負けた時はこのチェックは必要ないはず\n");
    return SCR_WILD_BTL_RET_ESCAPE;
  }
  return result_table[result];
}

//--------------------------------------------------------------
/**
 * @brief 勝ち負け結果チェック
 * @retval  TRUE  負けた
 * @retval  FALSE その他
 */
//--------------------------------------------------------------
static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew)
{
  return FIELD_BATTLE_IsLoseResult( bew->battle_param->result, bew->battle_param->competitor );
}

//--------------------------------------------------------------
/**
 * @brief 戦闘結果反映処理
 * @param bew   BATTLE_EVENT_WORKへのポインタ
 * @param gamedata
 *
 * @todo  戦闘結果からずかんや手持ちポケモン状態などの反映処理を実装する
 */
//--------------------------------------------------------------
static void BEW_reflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata)
{
  GAMEDATA_SetLastBattleResult( gamedata, bew->battle_param->result );

  //移動ポケモン戦闘後処理
  MP_SetAfterBattle( gamedata, bew->battle_param);

  //ビーコンリクエスト
  BeaconReq_BattleEnd( bew );
  
  //前作では貯金への反映、サファリボールカウントの反映、
  //いったん取っておいたPokeParamの反映などを行っていた
}

//--------------------------------------------------------------
/**
 * @brief BATTLE_EVENT_WORKの初期化処理
 * @param bew   BATTLE_EVENT_WORKへのポインタ
 * @param gsys
 * @param bp
 */
//--------------------------------------------------------------
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM* bp)
{
  GFL_STD_MemClear32( bew, sizeof(BATTLE_EVENT_WORK) );
  bew->gsys = gsys;
  bew->gamedata = GAMESYSTEM_GetGameData( gsys );
  bew->battle_param = bp;
  bew->bgm_pushed_flag = FALSE;
  bew->is_sub_event = FALSE;
  bew->is_no_lose = FALSE;
  bew->Examination = FALSE;
}

//--------------------------------------------------------------
/**
 * @brief BATTLE_EVENT_WORKの終了処理
 * @param bew   BATTLE_EVENT_WORKへのポインタ
 */
//--------------------------------------------------------------
static void BEW_Destructor(BATTLE_EVENT_WORK * bew)
{
  if( bew->not_free_bsp == FALSE ){
    BATTLE_PARAM_Delete( bew->battle_param );
  }
}

