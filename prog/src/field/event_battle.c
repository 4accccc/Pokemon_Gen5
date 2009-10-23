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

#include "sound/wb_sound_data.sadl" //サウンドラベルファイル
#include "sound/pm_sndsys.h"

#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h" //GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "event_encount_effect.h"

#include "field_sound.h"

#include "event_gameover.h" //EVENT_NormalLose
#include "field/field_comm/intrude_work.h"
#include "field/field_comm/bingo_system.h"
#include "event_battle_return.h"


//======================================================================
//  define
//======================================================================
#define HEAPID_CORE GFL_HEAPID_APP

//======================================================================
//  OVERLAY
//======================================================================
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(battle_return);

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// BATTLE_EVENT_WORK
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  BATTLE_SETUP_PARAM* battle_param;
  u16 timeWait;
  u16 bgmpush_off;
}BATTLE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================
//BATTLE_SETUP_PARAMのデバッグ生成処理
static void BATTLE_SETUP_PARAM_DebugConstructer(BATTLE_SETUP_PARAM * para, GAMESYS_WORK * gsys);
//POKEPARTYにポケモンを加える
static const void addPartyPokemon(POKEPARTY * party, u16 monsno, u8 level, u16 id);

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );
static GMEVENT_RESULT bingoBattleEvent(
    GMEVENT * event, int *  seq, void * work );

//debug
static GMEVENT_RESULT DebugBattleEvent( GMEVENT * event, int *  seq, void * work );

const u32 data_EncountPoke200905[];
const u32 data_EncountPoke200905Max;


static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew);
static void BEW_reflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata);
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, BATTLE_SETUP_PARAM* bp);
static void BEW_Destructor(BATTLE_EVENT_WORK * bew);
//======================================================================
//  フィールド　バトルイベント
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド野生ポケモンバトルイベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_WildPokeBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;

  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, fieldmap, bp );

  return event;
}

//--------------------------------------------------------------
/**
 * フィールドトレーナーバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int tr_id )
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
    FIELD_ENCOUNT_SetTrainerBattleParam( enc, bp, tr_id, HEAPID_PROC );
  }

  BEW_Initialize( bew, gsys, fieldmap, bp );
  bew->bgmpush_off = TRUE; //視線イベントBGM時に退避済み

  return event;
}

//--------------------------------------------------------------
/**
 * ビンゴバトルイベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_BingoBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;

  event = GMEVENT_Create(
      gsys, NULL, bingoBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, fieldmap, bp );

  //侵入システムに現在の状態を通知
  IntrudeWork_SetActionStatus(GAMESYSTEM_GetGameCommSysPtr(gsys), INTRUDE_ACTION_BINGO_BATTLE);
  Bingo_Clear_BingoIntrusionBeforeBuffer(Bingo_GetBingoSystemWork(GameCommSys_GetAppWork(GAMESYSTEM_GetGameCommSysPtr(gsys))));

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
  BATTLE_EVENT_WORK * bew = work;
  GAMESYS_WORK * gsys = bew->gsys;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );

  switch (*seq) {
  case 0:
    // 戦闘用ＢＧＭセット
    if( bew->bgmpush_off == FALSE ){ //既に退避済み
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PushBGM( fsnd );
    }
    PMSND_PlayBGM( bew->battle_param->musicDefault );
    IWASAWA_Printf("mineParty %08x\n",bew->battle_param->partyPlayer);
    //エンカウントエフェクト
    GMEVENT_CallEvent( event,
        EVENT_FieldEncountEffect(gsys,bew->fieldmap) );
    (*seq)++;
    break;
  case 1:
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, bew->fieldmap));
    (*seq)++;
    break;
  case 2:
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, bew->battle_param );
    (*seq)++;
    break;
  case 3:
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle_return), &BtlRet_ProcData, bew->battle_param );
    (*seq)++;
    break;
  case 4:
    IWASAWA_Printf( "mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer );
    bew->timeWait = 60; // 戦闘ＢＧＭフェードアウト
    PMSND_FadeOutBGM( 60 );
    (*seq) ++;
    break;
  case 5:
    if(bew->timeWait){
      bew->timeWait--;
    } else {
      (*seq) ++;
    }
    break;
  case 6:
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    //戦闘結果反映処理
    BEW_reflectBattleResult( bew, gamedata );
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PopBGM( fsnd );
    }

    //勝ち負け判定
    if (BEW_IsLoseResult( bew) == TRUE )
    {
      //負けの場合、イベントはここで終了。
      //復帰イベントへと遷移する
      BEW_Destructor( bew );
      PMSND_PauseBGM( TRUE );
      GMEVENT_ChangeEvent( event, EVENT_NormalLose(gsys) );
    }
    else
    {
      PMSND_FadeInBGM(60);
      GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    }
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    (*seq) ++;
    break;
  case 7:
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, bew->fieldmap, 0));
    (*seq) ++;
    break;
  case 8:
    IWASAWA_Printf("mineParty%d %08x\n",*seq,bew->battle_param->partyPlayer);
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ビンゴバトルイベント
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
#include "field/field_comm/bingo_system.h"
static GMEVENT_RESULT bingoBattleEvent(
    GMEVENT * event, int *  seq, void * work )
{
  GMEVENT_RESULT result;
  BATTLE_EVENT_WORK * bew = work;
  GAMESYS_WORK * gsys = bew->gsys;

  result = fieldBattleEvent(event, seq, work);
  if(result == GMEVENT_RES_FINISH){
    //侵入システムに現在の状態を通知
    IntrudeWork_SetActionStatus(GAMESYSTEM_GetGameCommSysPtr(gsys), INTRUDE_ACTION_FIELD);
  }
  return result;
}

//======================================================================
//  フィールド　デバッグ　バトルイベント
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグイベント フィールドバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_Battle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM * bp;
  BATTLE_EVENT_WORK * bew;

  event = GMEVENT_Create(
      gsys, NULL, DebugBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  bp = BATTLE_PARAM_Create(HEAPID_PROC);
  BATTLE_SETUP_PARAM_DebugConstructer( bp, gsys );

  BEW_Initialize( bew, gsys, fieldmap , bp );

  bew->timeWait = 0;

  return event;
}

//--------------------------------------------------------------
/**
 * デバッグバトルイベント
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugBattleEvent( GMEVENT * event, int *  seq, void * work )
{
  BATTLE_EVENT_WORK * bew = work;
  GAMESYS_WORK * gsys = bew->gsys;
#if 0
  switch (*seq) {
  case 0:
    GMEVENT_CallEvent(event,
        EVENT_FieldSubProc(bew->gsys, bew->fieldmap,
          FS_OVERLAY_ID(battle), &BtlProcData, &(bew->para))
        );
    (*seq) ++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
#endif
  switch (*seq) {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, bew->fieldmap));
    // 戦闘用ＢＧＭセット
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PushPlayEventBGM( fsnd, bew->battle_param->musicDefault );
    }
    (*seq)++;
    break;
  case 1:
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, bew->battle_param );
    (*seq)++;
    break;
  case 2:
    // 戦闘ＢＧＭフェードアウト
    bew->timeWait = 60;
    PMSND_FadeOutBGM(60);
    (*seq) ++;
    break;
  case 3:
    if(bew->timeWait){
      bew->timeWait--;
    } else {
      (*seq) ++;
    }
    break;
  case 4:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PopBGM( fsnd );
    }
    PMSND_FadeInBGM(60);
    //
    (*seq) ++;
    break;
  case 5:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, bew->fieldmap, 0));
    (*seq) ++;
    break;
  case 6:
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const void addPartyPokemon(POKEPARTY * party, u16 monsno, u8 level, u16 id)
{
  POKEMON_PARAM * pp = PP_Create(monsno, level, id, HEAPID_CORE);
  PokeParty_Add( party, pp);
  GFL_HEAP_FreeMemory( pp );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void BATTLE_SETUP_PARAM_DebugConstructer(BATTLE_SETUP_PARAM * para, GAMESYS_WORK * gsys)
{
  BATTLE_PARAM_Init(para);

  para->engine = BTL_ENGINE_ALONE;
  para->rule = BTL_RULE_SINGLE;
  para->competitor = BTL_COMPETITOR_WILD;

  para->netHandle = NULL;
  para->commMode = BTL_COMM_NONE;
  para->netID = 0;

  //プレイヤーのパーティ
  para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );
  PokeParty_Copy(
      GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)),
      para->partyPlayer);

  //1vs1時の敵AI, 2vs2時の１番目敵AI用
  para->partyEnemy1 = PokeParty_AllocPartyWork( HEAPID_CORE );
  addPartyPokemon( para->partyEnemy1, MONSNO_ARUSEUSU+1, 15, 3594 );
  //PokeParty_Add( para->partyEnemy1, PP_Create(MONSNO_ARUSEUSU+1,15,3594,HEAPID_CORE) );

  //2vs2時の味方AI（不要ならnull）
  para->partyPartner = NULL;
  //2vs2時の２番目敵AI用（不要ならnull）
  para->partyEnemy2 = NULL;

  //プレイヤーステータス add
  para->statusPlayer = SaveData_GetMyStatus( SaveControl_GetPointer() );

  //デフォルト時のBGMナンバー
  para->musicDefault = SEQ_BGM_VS_NORAPOKE;
  //ピンチ時のBGMナンバー
  para->musicPinch = SEQ_BGM_BATTLEPINCH;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
/**
 * @brief 勝ち負け結果チェック
 * @retval  TRUE  負けた
 * @retval  FALSE その他
 */
//--------------------------------------------------------------
static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew)
{
  switch ((BtlResult)bew->battle_param->result)
  {
  case BTL_RESULT_LOSE:        ///< 負けた
    return TRUE;
  default:
    GF_ASSERT_MSG(0, "想定外のBtlResult %d\n", bew->battle_param->result );
    /* FALL THROUGH */
  case BTL_RESULT_WIN:         ///< 勝った
  case BTL_RESULT_DRAW:        ///< ひきわけ
  case BTL_RESULT_RUN:         ///< 逃げた
  case BTL_RESULT_RUN_ENEMY:   ///< 相手が逃げた（野生のみ）
  case BTL_RESULT_CATCH:       ///< 捕まえた（野生のみ）
    return FALSE;
  }
}

//--------------------------------------------------------------
/**
 * @brief 戦闘結果反映処理
 *
 * @todo  戦闘結果からずかんや手持ちポケモン状態などの反映処理を実装する
 */
//--------------------------------------------------------------
static void BEW_reflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata)
{
  GAMEDATA_SetLastBattleResult( gamedata, bew->battle_param->result );
  //前作では貯金への反映、サファリボールカウントの反映、
  //いったん取っておいたPokeParamの反映などを行っていた
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, BATTLE_SETUP_PARAM* bp)
{
  MI_CpuClear8( bew, sizeof(BATTLE_EVENT_WORK) );
  bew->gsys = gsys;
  bew->fieldmap = fieldmap;
  bew->battle_param = bp;
  bew->bgmpush_off = FALSE;
  bew->timeWait = 0;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void BEW_Destructor(BATTLE_EVENT_WORK * bew)
{
  BATTLE_PARAM_Delete( bew->battle_param );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// ５月末用エンカウントポケモン一覧
//--------------------------------------------------------------
const u32 data_EncountPoke200905[] =
{
  494,//ピンボー
  495,//モグリュー
  496,//ゴリダルマ
  497,//ワニメガネ
  498,//ワニグラス
//  499,//ワニバーン
  500,//ハゴロムシ
  501,//ハキシード
//  502,//オノックス
  503,//カーメント
//  504,//バンビーナ
  505,//バトロード
  506,//プルンス
  507,//コバト
  508,//ライブラ
  509,//バクパク
  510,//ユリコネ
  511,//オーバト
  512,//パンチーヌ
  513,//モコウモリ
};

const u32 data_EncountPoke200905Max = NELEMS(data_EncountPoke200905);
