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
#include "gamesystem/pm_weather.h"

#include "field/fieldmap.h"
#include "field/fes_gimmick.h"

#include "./event_fieldmap_control.h"
#include "./event_battle.h"

#include "sound/pm_sndsys.h"

#include "battle/battle.h"
#include "system/main.h" //HEAPID_PROC参照

//#include "poke_tool/monsno_def.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/natsuki.h"
#include "poke_tool/status_rcv.h"
#include "waza_tool/wazano_def.h"

//#include "event_encount_effect.h"
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
#include "savedata/record.h"

#ifndef DISABLE_INTRUDE //swanで侵入を削除
#include "event_mission_help_after.h"
#endif  //DISABLE_INTRUDE swanで侵入を削除

#include "debug/debug_flg.h"

#include "trial_house.h"

#include "sound/swan_sound_data.sadl" //サウンドラベルファイル

#include "savedata/encount_sv.h"
#include "field/encount_data_ex_def.h"

#include "field/zonedata.h" // for ZONEDATA_GetAreaID

#include "savedata/infinity_dungeon_save.h"
#include "infinity_dungeon_gimmick.h"
#include "infinity_dungeon_creator.h"

FS_EXTERN_OVERLAY(amprotect);

#ifdef AMPROTECT_FUNC
static void *magicon1_failure_func   ( void *param1, void *param2 );
static void *magicon2_need_func   ( void *param1, void *param2 );
static void *magicon3_failure_func( void *param1, void *param2 );
static void *magicon4_need_func( void *param1, void *param2 );
extern void magicon_dummy_task(GFL_TCB *tcb, void *pWork);
extern int  debug_tcb_num;

#endif

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
 TR_BEACON_SPACIAL,
 TR_BEACON_TYPE_MAX,
}TRAINER_BEACON_TYPE;

typedef enum{
 BTL_BEACON_ST_START,
 BTL_BEACON_ST_WIN,
 BTL_BEACON_ST_ESCAPE,        //自分が逃げた
 BTL_BEACON_ST_ESCAPE_ENEMY,  //相手が逃げた
 BTL_BEACON_ST_MAX,
}BTL_BEACON_ST;

static const TR_BEACON_SET_FUNC DATA_TrBeaconSetFuncTbl[TR_BEACON_TYPE_MAX][2] = {
 { GAMEBEACON_Set_BattleTrainerStart, GAMEBEACON_Set_BattleTrainerVictory },
 { GAMEBEACON_Set_BattleLeaderStart, GAMEBEACON_Set_BattleLeaderVictory },
 { GAMEBEACON_Set_BattleSpTrainerStart, GAMEBEACON_Set_BattleSpTrainerVictory },
};

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
  u32 flags;  //戦闘呼び出し時に指定したフラグ
}BATTLE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );

static void BeaconReq_BtlTrainer( u16 tr_id, BTL_BEACON_ST state );
static void BeaconReq_BtlWild( BATTLE_SETUP_PARAM* bp, BTL_BEACON_ST state );
static void BeaconReq_BattleEnd( BATTLE_EVENT_WORK* bew );
static void ReflectRecord( BATTLE_EVENT_WORK* bew );

static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew);
static void BEW_ReflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata);
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
  u8  sub_event_f;
  u8  n_poke_id;
  u8  fishing_f;
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

      //バトルセットアップパラムから戦闘するポケモンを取得
      {
        POKEPARTY* enemy = BATTLE_PARAM_GetPokePartyPointer( wbew->bp, BTL_CLIENT_ENEMY1 );
        POKEMON_PARAM *pp = PokeParty_GetMemberPointer(enemy, 0);
        int monsno = PP_Get( pp, ID_PARA_monsno, NULL );
        int formno = PP_Get( pp, ID_PARA_form_no, NULL );
        FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
        //エンカウントエフェクトとＢＧＭを取得してセット
        ENCEFFNO_GetWildEncEffNoBgmNo( monsno, formno, wbew->EncType, ( wbew->bp->rule == BTL_RULE_DOUBLE),
            fieldmap, &bew->EncEffNo, &bew->battle_param->musicDefault );
      }

      GMEVENT_CallEvent( event, battle_main_event );
    }
    (*seq) ++;
    break;

  case 1:
    {
      u32 btl_result = GAMEDATA_GetLastBattleResult(gamedata);  //ポケモンゲットなど結果詳細
      BOOL is_lose = FIELD_BATTLE_IsLoseResult( btl_result, BTL_COMPETITOR_WILD); //全滅したか？

      //Ｎポケモン捕獲チェック
      if( btl_result == BTL_RESULT_CAPTURE && wbew->n_poke_id != ENC_N_POKE_NONE){
        ENC_SV_PTR enc_save = EncDataSave_GetSaveDataPtr( GAMEDATA_GetSaveControlWork(gamedata) );
        EncDataSave_SetNPokeCaptureFlag( enc_save, wbew->n_poke_id );
      }
      //釣り時の映りこみOffフラグをリセット
      if( wbew->fishing_f ){
        MMDLSYS* mmdlsys = GAMEDATA_GetMMdlSys( gamedata );
        MMDL* mmdl = MMDLSYS_SearchMMdlPlayer(mmdlsys);
        if( MMDL_CheckStatusBitReflectOff(mmdl)){
          MMDL_SetStatusBitReflectOff( mmdl, FALSE );
        }
      }
      if( !is_lose && PAIRSYS_CheckPairFlag( gamedata )){
        //連れ歩き中は手持ち全回復
        POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
        STATUS_RCV_PokeParty_RecoverAll( party );
      }
      //このイベント自体がサブイベントの場合、何もせずに戻る
      if ( wbew->sub_event_f == TRUE )
      {
        COMM_PLAYER_SUPPORT_Init(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
        return GMEVENT_RES_FINISH;
      }

      //このイベントがトップのイベントの場合、戦闘後処理を行う
      if ( is_lose )
      {
        //負けた場合は敗北処理へ
        COMM_PLAYER_SUPPORT_Init(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
        GMEVENT_ChangeEvent( event, EVENT_NormalLose(gsys) );
      }
      else
      {
        //負けていない場合はフェードイン
        GMEVENT* fade_event;
        fade_event = EVENT_FieldFadeIn_Black(gsys, GAMESYSTEM_GetFieldMapWork(gsys), FIELD_FADE_WAIT);
        GMEVENT_CallEvent(event, fade_event);
      }
    }
    (*seq) ++;
    break;

  case 2:
#ifndef DISABLE_INTRUDE //swanで侵入を削除
    //侵入相手がたすけてくれたメッセージイベント
    {
      COMM_PLAYER_SUPPORT *cps = GAMEDATA_GetCommPlayerSupportPtr(gamedata);
      if(COMM_PLAYER_SUPPORT_GetBattleEndSupportType(cps) == TRUE){
        GMEVENT_CallEvent(event, EVENT_Intrude_BattleHelpAfterEvent( gsys, HEAPID_PROC ));
      }
    }
#endif  //swanで侵入を削除
    (*seq)++;
    break;
  
  case 3:
    COMM_PLAYER_SUPPORT_Init(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
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
 * @param n_poke_id 未指定の場合ENC_N_POKE_NONE 
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_WildPokeBattleEx( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp,
    BOOL sub_event_f, ENCOUNT_TYPE enc_type, u8 n_poke_id )
{
  GMEVENT * event = GMEVENT_Create( gsys, NULL, wildBattleEvent, sizeof(WILD_BATTLE_EVENT_WORK) );
  WILD_BATTLE_EVENT_WORK * wbew = GMEVENT_GetEventWork( event );
  wbew->gsys = gsys;
  wbew->bp = bp;
  wbew->sub_event_f = sub_event_f;
  wbew->EncType = enc_type;
  wbew->n_poke_id = n_poke_id;
  wbew->fishing_f = BATTLE_PARAM_CheckBtlStatusFlag( bp, BTL_STATUS_FLAG_FISHING );

  if( enc_type == ENC_TYPE_EFFECT ){
    BATTLE_PARAM_SetBtlStatusFlag( bp, BTL_STATUS_FLAG_EFF_ENC );
  }
#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif
  //すれ違いビーコン送信リクエスト
  BeaconReq_BtlWild( bp, BTL_BEACON_ST_START );
  {
    RECORD* record = GAMEDATA_GetRecordPtr( GAMESYSTEM_GetGameData(gsys));
    RECORD_Inc( record, RECID_BTL_ENCOUNT );
  }

  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
  return event;
}

GMEVENT * EVENT_WildPokeBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp,
    BOOL sub_event_f, ENCOUNT_TYPE enc_type )
{
  return EVENT_WildPokeBattleEx( gsys, fieldmap, bp, sub_event_f, enc_type, ENC_N_POKE_NONE );
}

//--------------------------------------------------------------
/**
 * フィールドトレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param BtlRule
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int rule, int partner_id, int tr_id0, int tr_id1, u32 flags )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;
  BATTLE_SETUP_PARAM* bp;
  
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  bew->flags = flags;

  {
    FIELD_ENCOUNT* enc = FIELDMAP_GetEncount(fieldmap);
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    RECORD* record = GAMEDATA_GetRecordPtr( gdata );

    bp = BATTLE_PARAM_Create(HEAPID_PROC);
    FIELD_ENCOUNT_SetTrainerBattleParam( enc, bp, rule, partner_id, tr_id0, tr_id1, HEAPID_PROC );
    BTL_SETUP_SetDifficulty( bp, gdata, FIELDMAP_GetZoneID( fieldmap ) );

    BeaconReq_BtlTrainer( tr_id0, BTL_BEACON_ST_START );
    RECORD_Inc( record, RECID_BTL_TRAINER);
    RECORD_Inc( record, RECID_DAYCNT_TRAINER_BATTLE);
  }
#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif

  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = TRUE;   //スクリプトから呼ばれる＝トップレベルのイベントでない
  if ( (flags & SCR_BATTLE_MODE_NOLOSE) != 0 )
  {
    BATTLE_PARAM_SetBtlStatusFlag( bp, BTL_STATUS_FLAG_NO_LOSE );
    bew->is_no_lose = TRUE;
  }
  
  if ( flags & SCR_BATTLE_MODE_FES_TRAINER )
  {
    POKEPARTY* party = BATTLE_PARAM_GetPokePartyPointer( bp, BTL_CLIENT_ENEMY1 );
    
    FES_GIMMICK_FesTrainerPartySetForNpcMission(
      FIELDMAP_GetFesGimmickSys( fieldmap ), party );
  
    IWASAWA_Printf( "FesTrLevel = %d\n", PP_Get( PokeParty_GetMemberPointer(party,0), ID_PARA_level, NULL ));
  }

  if ( (flags & SCR_BATTLE_MODE_MUGEN) != 0 )
  {
    u16 area_id = ZONEDATA_GetAreaID( FIELDMAP_GetZoneID(fieldmap));
    if (AREADATA_IsDungeonArea(area_id))
    {
      GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
      SAVE_CONTROL_WORK *p_scw = GAMEDATA_GetSaveControlWork( gdata );
      INFINITYDUNGEON_SAVE *p_save = SaveData_GetINFINITYDUNGEON( p_scw );
      u8 dungeon_area = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
      INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(fieldmap);
      u8 dungeon_version = INF_TOOL_DUNGEON_GetDungeonColor( p_dungeon_data );
      if( dungeon_version == VERSION_B2 )
      {
        bp->fieldSituation.bgAttr = BATTLE_BG_ATTR_MUGEN_B;
      }
      else
      {
        bp->fieldSituation.bgAttr = BATTLE_BG_ATTR_MUGEN_W;
      }
      // 背景設定
      bp->fieldSituation.bgType = dungeon_area;
      // アイテム使用禁止
      bp->bag_nouse_flag = TRUE;
      // 見た登録禁止
      bp->ZukanSeeFlagNoSetFlag = TRUE;
    
      ONOUE_Printf("無限ダンジョン用背景差替え");
    }
    else
    {
      GF_ASSERT_MSG(0,"無限ダンジョン外では呼べません\n");
    }
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
#if 0
  bew->battle_param->musicDefault = SEQ_BGM_VS_SUBWAY_TRAINER;
  bew->battle_param->musicWin = SEQ_BGM_WIN2;
#endif
   
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
 * 無限ダンジョン内トレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_InfinityDungeonTrainerBattle(
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
   
  // アイテム使用禁止
  bp->bag_nouse_flag = TRUE;
  
  // 見た登録禁止
  bp->ZukanSeeFlagNoSetFlag = TRUE;
   
#ifdef PM_DEBUG
  debug_FieldDebugFlagSet( bp );
#endif
  
  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  
  bew->is_sub_event = TRUE; //サブイベント呼び出し
  
  bew->is_no_lose = FALSE; //敗戦処理有り
  bew->not_free_bsp = FALSE; //開放する
  
  //エンカウントエフェクト
  bew->EncEffNo = ENCEFFID_SUBWAY;
  // BGMはトレーナー固定
  bew->battle_param->musicDefault = SEQ_BGM_VS_TRAINER;
  // 勝利BGM固定
  bew->battle_param->musicWin = SEQ_BGM_WIN2;

  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
  return event;
}

//--------------------------------------------------------------
/**
 * WBT用対戦イベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param tr_id
 * @param flags
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_WBTBattle(
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
  
  bew->is_sub_event = TRUE; //サブイベント呼び出し
  
  bew->is_no_lose = TRUE; //敗戦処理無し
  bew->not_free_bsp = TRUE; //開放しない。呼び出し元で開放する
  
  //エンカウントエフェクトセット
  bew->EncEffNo = ENCEFFID_WBT;
   
  //エフェクトエンカウント　エフェクト復帰キャンセル
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

    PP_Setup( pp, MONSNO_YOOTERII, 5, 0 );
    PP_SetWazaPos( pp, WAZANO_TAIATARI, 0 );
    PP_SetWazaPos( pp, WAZANO_NIRAMITUKERU, 1 );
    PP_SetWazaPos( pp, WAZANO_KAGIWAKERU, 2 );
    PP_SetWazaPos( pp, WAZANO_NULL, 3 );
    PokeParty_Add( partyMine, pp );
    
    PP_Setup( pp, MONSNO_TYORONEKO, 2, 0 );
    PP_SetWazaPos( pp, WAZANO_HIKKAKU, 0 ); //チョロネコのPPは見えないけど念のため
    PP_SetWazaPos( pp, WAZANO_NAKIGOE, 1 );
    PP_SetWazaPos( pp, WAZANO_NULL,    2 );
    PP_SetWazaPos( pp, WAZANO_NULL,    3 );
    PokeParty_Add( partyEnemy, pp );

    BTL_FIELD_SITUATION_SetFromFieldStatus( &sit, gdata, fieldmap );
    sit.bgAttr = BATTLE_BG_ATTR_E_GRASS;
    sit.bgType = BATTLE_BG_TYPE_GRASS_SEASON;
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
  ENCEFFNO_GetWildEncEffNoBgmNo( MONSNO_TYORONEKO, 0, ENC_TYPE_DEMO, FALSE, fieldmap, &bew->EncEffNo, &bew->battle_param->musicDefault );
  //捕獲でもは野生弱エンカウントエフェクト固定
  bew->EncEffNo = ENCEFFID_WILD_NORMAL;

  return event;
}


//--------------------------------------------------------------
/**
 * フィールドトレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @param BtlRule
 * @param tr_id
 * @param flags
 * @param trade_type    交換後　タイプ
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TradeAfterTrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int rule, int partner_id, int tr_id0, int tr_id1, u32 flags, TRPOKE_AFTER_SAVE_TYPE trade_type )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;
  POKEPARTY* enemy_party;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  TRPOKE_AFTER_SAVE* trpoke_sv = GAMEDATA_GetTrPokeAfterSaveData( gdata );
  POKEMON_PARAM* pp;
  u32 monsno;


  event = EVENT_TrainerBattle( gsys, fieldmap, rule, partner_id, tr_id0, tr_id1, flags );


  if( TRPOKE_AFTER_SV_IsData( trpoke_sv, trade_type ) ){
    // trade_typeのポケモンに書き換え
    bew = GMEVENT_GetEventWork(event);
    enemy_party = bew->battle_param->party[ BTL_CLIENT_ENEMY1 ];
    
    // パーティの１番目
    pp = PokeParty_GetMemberPointer( enemy_party, 0 );

    // MonsNo Waza引継ぎ
    monsno = TRPOKE_AFTER_SV_GetTradeMonsNo( trpoke_sv, trade_type, GET_VERSION());

    PP_SetupEx( pp, monsno, 
        TRPOKE_AFTER_SV_GetLevel( trpoke_sv, trade_type ),
        TRPOKE_AFTER_SV_GetID( trpoke_sv, trade_type ),
        PTL_SETUP_POW_AUTO,
        TRPOKE_AFTER_SV_GetRnd( trpoke_sv, trade_type ) );

    // 第３特性に対する処理
    if( TRPOKE_AFTER_SV_IsSpeabino3( trpoke_sv, trade_type ) ){
      // 特性３フラグセット
      PP_Put( pp, ID_PARA_tokusei_3_flag, TRUE );
      // とくせいセット
      PP_Put( pp, ID_PARA_speabino, TRPOKE_AFTER_SV_GetSpeabino3( trpoke_sv, trade_type ) );
    }
    PP_Put( pp, ID_PARA_nickname_raw, (u32)TRPOKE_AFTER_SV_GetNickName( trpoke_sv, trade_type ) );
    // 進化させる。
    PP_ChangeMonsNo( pp, monsno );

    // デフォルト技を設定
    PP_SetWazaDefault( pp );

  }else{

    // かきかえられていない。
    GF_ASSERT_MSG( 0, "交換ポケモン情報なし\n" );
  }

  return event;
}


//--------------------------------------------------------------
/**
 * フィールドバトルイベント
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work )
{
  BATTLE_EVENT_WORK * bew      = work;
  GAMESYS_WORK *      gsys     = bew->gsys;
  GAMEDATA *          gamedata = bew->gamedata;
  FIELDMAP_WORK *     fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  u32 magicon_number  = 6451*3191;
  
  
  switch( *seq ) {
  // 戦闘用ＢＧＭセット
  case 0:
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(gsys, bew->battle_param->musicDefault));
    (*seq)++;
    break;

  //エンカウントエフェクト
  case 1:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldmap), event, bew->EncEffNo);
    (*seq)++;
    break;

  // フィールドマップ終了
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
    TAMADA_Printf(" BSP ADDRESS %08x\n", bew->battle_param );
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, bew->battle_param );
    (*seq)++;
    break;
    
  case 4:
    if ( bew->battle_param->PokewoodMode == BTL_POKEWOOD_NONE )
    {
      bew->btlret_param.btlResult = bew->battle_param;
      bew->btlret_param.gameData  = gamedata;
      GMEVENT_CallProc( event, FS_OVERLAY_ID(battle_return), &BtlRet_ProcData, &bew->btlret_param );
    }
    (*seq)++;
    break;

  case 5:
#ifdef AMPROTECT_FUNC
    GFL_OVERLAY_Load( FS_OVERLAY_ID(amprotect) );  
    AM_Init();
#endif
    //侵入システムにフィールド中であることを伝える
    {
      FIELD_STATUS *fldstatus = GAMEDATA_GetFieldStatus(gamedata);
      FIELD_STATUS_SetProcAction( fldstatus, PROC_ACTION_FIELD );
      COMM_PLAYER_SUPPORT_SetBattleEnd(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
    }
#ifdef AMPROTECT_FUNC
    AM_SetCallback( magicon2_need_func, magicon1_failure_func );
#endif
    (*seq) ++;
    break;

  case 6: 
#ifdef AMPROTECT_FUNC
//    magicon_number += 977*(!AM_IsNotMagiconA1(magicon1_failure_func));
    AM_IsMagiconB1( &magicon_number, bew );
#endif
    //戦闘結果反映処理
    BEW_ReflectBattleResult( bew, gamedata ); 
    //採点処理
    if (bew->Examination)
    {
      TRIAL_HOUSE_WORK_PTR *ptr = GAMEDATA_GetTrialHouseWorkPtr(gamedata);
      NOZOMU_Printf("検定採点\n");
      TRIAL_HOUSE_AddBtlPoint( *ptr, bew->battle_param );
    }
    (*seq) ++;
#ifdef AMPROTECT_FUNC
//    magicon_number += 1489*(!AM_IsNotMagiconA2(magicon2_need_func));
    AM_SetCallback( magicon4_need_func, magicon3_failure_func );

#endif
    break;

  case 7:
#ifdef AMPROTECT_FUNC
//    magicon_number += 157*AM_IsMagiconA3(magicon3_failure_func);
      AM_IsMagiconB2( &magicon_number, bew );
#endif
    // 負けた場合, BGMを停止する
    if( (bew->is_no_lose == FALSE) && (BEW_IsLoseResult(bew) == TRUE) ) { 
      GMEVENT_CallEvent( event, EVENT_FSND_ResetBGM( gsys, FSND_FADE_SHORT ) );
    }
    // そうでなければ, フィールドBGMを復帰
    else {
      GMEVENT_CallEvent( event, EVENT_FSND_PopPlayBGM_fromBattle( gsys ) );
    }
#ifdef AMPROTECT_FUNC
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(amprotect) );  
#endif
    (*seq) ++;
    break;

  case 8: 
    //勝ち負け判定
    if( (bew->is_no_lose == FALSE) && (BEW_IsLoseResult(bew) == TRUE) ) {
      //負けの場合、イベントはここで終了。
      //復帰イベントへと遷移する
      if (bew->is_sub_event == TRUE) {
        BEW_Destructor( bew );
        return GMEVENT_RES_FINISH;
      } else {
        BEW_Destructor( bew );
        GMEVENT_ChangeEvent( event, EVENT_NormalLose(gsys) );
        return GMEVENT_RES_CONTINUE;  //ChangeEventではFINISHさせてはいけない
      }
    }
    else {
      if( PAIRSYS_CheckPairFlag( gamedata )){
        //連れ歩き中は手持ち全回復
        POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
        STATUS_RCV_PokeParty_RecoverAll( party );
      }
      GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    }
    (*seq) ++;
    break;

  // BGMの復帰待ち
  case 9:
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( gsys ) );
    (*seq) ++;
    break;

  case 10:
    if ( bew->is_sub_event == FALSE ) {
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );
    }
    (*seq) ++;
    break;

  case 11:
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }


#ifdef AMPROTECT_FUNC

  if(magicon_number % 6451){
    GFL_TCB *dust_tcb = GFUser_VIntr_CreateTCB( magicon_dummy_task, NULL, 127 );
    debug_tcb_num++;
  }
#endif
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
  static u16 sp_trtype_tbl[] = {  //トレーナーグループ判定できないものをここに登録
    TRTYPE_BCHAMP_1,  //ノボリ
    TRTYPE_BCHAMP_2,  //クダリ
    TRTYPE_NINJA,     //ダーク
    TRTYPE_SWSAGE2,   //七賢人ロット
    TRTYPE_LEADER1A,  //ポッド
    TRTYPE_LEADER1B,  //デント
    TRTYPE_LEADER1C,  //コーン
  };

  u16 tr_type = TT_TrainerDataParaGet( tr_id, ID_TD_tr_type );
  u8 grp = TT_TrainerTypeGrpGet( tr_type );

  {
    //BTS738対策 ギアステーション前のノボリ・クダリとのマルチイベント戦は特別なトレーナー扱いにする
    //BTS1977 元七賢人ロットとニンジャ(ダーク)、ポッド・デント・コーンは特別なトレーナー扱い
    int i;

    for( i = 0; i < NELEMS(sp_trtype_tbl);i++ ){
      if( tr_type == sp_trtype_tbl[i]){
        return TR_BEACON_SPACIAL;
      }
    }
  }
  if( grp == TRTYPE_GRP_NONE ||
      grp == TRTYPE_GRP_PLASMA ){
    return TR_BEACON_NORMAL;
  }
  if( grp == TRTYPE_GRP_LEADER ){
    return TR_BEACON_LEADER;
  }
  return TR_BEACON_SPACIAL;
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
  case BTL_BEACON_ST_ESCAPE:
    GAMEBEACON_Set_Escape( monsno );
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
  
  POKEPARTY * party = GAMEDATA_GetMyPokemon( bew->gamedata );
  POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, PokeParty_GetMemberTopIdxNotEgg( party ));

  if( (bp->result != BTL_RESULT_LOSE) && (bp->result != BTL_RESULT_DRAW))
  {
	u16	monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    STRBUF* nickname = GFL_STR_CreateBuffer( BUFLEN_POKEMON_NAME, GFL_HEAP_LOWID( HEAPID_PROC ) );
    PP_Get( pp, ID_PARA_nickname, nickname );

    //先頭ポケモンの残HPチェック
    {
      u16 hp = PP_Get(pp,ID_PARA_hp,NULL);
      u16 hp_max = PP_Get(pp,ID_PARA_hpmax,NULL);

      if( hp == 0 ){
        GAMEBEACON_Set_Dying( monsno, nickname);
      }else if( (hp*2) <= hp_max ){
        GAMEBEACON_Set_HPLittle( monsno, nickname);
      }
    }
    { //残PPチェック
      int i;

      for(i = 0;i < 4;i++){
        if( ( PP_Get( pp, ID_PARA_waza1+i, NULL) != WAZANO_NULL ) &&
            ( PP_Get( pp, ID_PARA_pp1+i,NULL) == 0) ){
          GAMEBEACON_Set_PPLittle( monsno, nickname);
          break;
        }
      }
    }
    { //状態異常チェック
      if( PP_Get( pp, ID_PARA_condition, NULL) != 0 ) {
        GAMEBEACON_Set_StateIsAbnormal(nickname);
      }
    }
    GFL_STR_DeleteBuffer( nickname );
  }

  switch( bp->competitor ){
  case BTL_COMPETITOR_WILD:
    if( bp->result == BTL_RESULT_RUN ){
      BeaconReq_BtlWild( bp, BTL_BEACON_ST_ESCAPE );
    }else if( bp->result == BTL_RESULT_RUN_ENEMY ){
      BeaconReq_BtlWild( bp, BTL_BEACON_ST_ESCAPE_ENEMY );
    }else if( bp->result == BTL_RESULT_WIN ){
      BeaconReq_BtlWild( bp, BTL_BEACON_ST_WIN );
    }
    break;
  case BTL_COMPETITOR_TRAINER:
    if( (bp->result == BTL_RESULT_WIN ) &&
        ((bew->flags & SCR_BATTLE_MODE_FES_TRAINER) == FALSE) ){
      BeaconReq_BtlTrainer( bp->tr_data[BTL_CLIENT_ENEMY1]->tr_id, BTL_BEACON_ST_WIN );
    }
    break;
  }
}

//--------------------------------------------------------------
/*
 *  @brief  バトル終了時レコードインクリメント
 */
//--------------------------------------------------------------
static void ReflectRecord( BATTLE_EVENT_WORK* bew )
{
  BATTLE_SETUP_PARAM* bp = bew->battle_param;
  RECORD* record = GAMEDATA_GetRecordPtr( GAMESYSTEM_GetGameData(bew->gsys));
  
  if( bp->competitor == BTL_COMPETITOR_WILD ){
    switch( bp->result ){
    case BTL_RESULT_RUN_ENEMY:
      RECORD_Inc( record, RECID_NIGERARETA );
      break;
    }
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
 *  戦闘結果からGAMEDATAへの勝敗反映などの処理を実装する
 *  前作では貯金への反映、サファリボールカウントの反映などを行っていた
 */
//--------------------------------------------------------------
static void BEW_ReflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata)
{
  GAMEDATA_SetLastBattleResult( gamedata, bew->battle_param->result );

#ifndef SWAN_MOVE_POKE_DISABLE
  //移動ポケモン戦闘後処理
  MP_SetAfterBattle( gamedata, bew->battle_param);
#endif  //SWAN_MOVE_POKE_DISABLE

  //ビーコンリクエスト
  BeaconReq_BattleEnd( bew );

  //レコードデータインクリメント
  ReflectRecord( bew );

  //諸々の戦闘結果を反映して、天候を再設定する
  PM_WEATHER_UpdateZoneChangeWeatherNo( bew->gsys, PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork( gamedata )) );
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



#ifdef AMPROTECT_FUNC
static void *magicon1_failure_func( void *param1, void *param2 )
{
  u32 *magicon_number = (u32*)param1;
  int primal = 71*83;
  primal += GFL_STD_MtRand(GFL_STD_RAND_MAX)+GFL_STD_MtRand(GFL_STD_RAND_MAX);

  *magicon_number += 977;
  return param1;
}
static void *magicon2_need_func( void *param1, void *param2 )
{
  int primal = 2111*101;
  primal += GFL_STD_MtRand(GFL_STD_RAND_MAX)+GFL_STD_MtRand(GFL_STD_RAND_MAX);
  return param2;
}
static void *magicon3_failure_func( void *param1, void *param2 )
{
  int primal = 1759*2677;
  u32 *magicon_number = (u32*)param1;
  primal += GFL_STD_MtRand(GFL_STD_RAND_MAX)+GFL_STD_MtRand(GFL_STD_RAND_MAX);

  *magicon_number += 157;
  return param1;
}

static void *magicon4_need_func( void *param1, void *param2 )
{
  return param2;
}
#endif
