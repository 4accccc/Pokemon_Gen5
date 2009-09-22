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


//======================================================================
//  define
//======================================================================
#define HEAPID_CORE GFL_HEAPID_APP

//======================================================================
//  OVERLAY
//======================================================================
FS_EXTERN_OVERLAY(battle);

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// BATTLE_EVENT_WORK
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  BATTLE_SETUP_PARAM para;
  u16 timeWait;
  u16 bgmpush_off;
}BATTLE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================
//BATTLE_SETUP_PARAMの解放処理
static void BATTLE_SETUP_PARAM_Destructor(BATTLE_SETUP_PARAM * para);
//BATTLE_SETUP_PARAMのデバッグ生成処理
static void BATTLE_SETUP_PARAM_DebugConstructer(BATTLE_SETUP_PARAM * para, GAMESYS_WORK * gsys);
//POKEPARTYにポケモンを加える
static const void addPartyPokemon(POKEPARTY * party, u16 monsno, u8 level, u16 id);

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );

//debug
static GMEVENT_RESULT DebugBattleEvent(
    GMEVENT * event, int *  seq, void * work );

const u32 data_EncountPoke200905[];
const u32 data_EncountPoke200905Max;


static BOOL BEW_IsLoseResult(BATTLE_EVENT_WORK * bew);
static void BEW_reflectBattleResult(BATTLE_EVENT_WORK * bew, GAMEDATA * gamedata);
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);
static void BEW_Destructor(BATTLE_EVENT_WORK * bew);
//======================================================================
//  フィールド　バトルイベント
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドバトルイベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_Battle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM * para;
  BATTLE_EVENT_WORK * dbw;
  
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );
  
  dbw = GMEVENT_GetEventWork(event);
  BEW_Initialize( dbw, gsys, fieldmap );

  para = &dbw->para;
  {
    FIELD_ENCOUNT *enc = FIELDMAP_GetEncount( fieldmap );
    FIELD_ENCOUNT_GetBattleSetupParam( enc, para );
  }

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
  BATTLE_SETUP_PARAM * para;
  BATTLE_EVENT_WORK * dbw;

  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  dbw = GMEVENT_GetEventWork(event);
  BEW_Initialize( dbw, gsys, fieldmap );
  dbw->bgmpush_off = TRUE; //視線イベントBGM時に退避済み
  
  para = &dbw->para;
  {
    FIELD_ENCOUNT *enc = FIELDMAP_GetEncount( fieldmap );
    FIELD_ENCOUNT_SetTrainerBattleSetupParam(
        enc, para, tr_id, HEAPID_PROC );
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
    PMSND_PlayBGM( bew->para.musicDefault );
    
    //エンカウントエフェクト
    GMEVENT_CallEvent( event,
        EVENT_FieldEncountEffect(gsys,bew->fieldmap) );
    (*seq)++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, bew->fieldmap));
    (*seq)++;
    break;
  case 2:
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, &bew->para );
    (*seq)++;
    break;
  case 3:
    bew->timeWait = 60; // 戦闘ＢＧＭフェードアウト
    PMSND_FadeOutBGM( 60 );
    (*seq) ++;
    break;
  case 4:
    if(bew->timeWait){
      bew->timeWait--;
    } else {
      (*seq) ++;
    }
    break;
  case 5:
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
    (*seq) ++;
    break;
  case 6:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, bew->fieldmap, 0));
    (*seq) ++;
    break;
  case 7:
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
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
  BATTLE_SETUP_PARAM * para;
  BATTLE_EVENT_WORK * dbw;

  event = GMEVENT_Create(
      gsys, NULL, DebugBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  dbw = GMEVENT_GetEventWork(event);
  BEW_Initialize( dbw, gsys, fieldmap );
  para = &dbw->para;
  BATTLE_SETUP_PARAM_DebugConstructer( para, gsys );

  dbw->timeWait = 0;

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
static GMEVENT_RESULT DebugBattleEvent(
    GMEVENT * event, int *  seq, void * work )
{
  BATTLE_EVENT_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;
#if 0
  switch (*seq) {
  case 0:
    GMEVENT_CallEvent(event,
        EVENT_FieldSubProc(dbw->gsys, dbw->fieldmap,
          FS_OVERLAY_ID(battle), &BtlProcData, &(dbw->para))
        );
    (*seq) ++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
#endif
  switch (*seq) {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
    // 戦闘用ＢＧＭセット
    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PushPlayEventBGM( fsnd, dbw->para.musicDefault );
    }
    (*seq)++;
    break;
  case 1:
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, &dbw->para );
    (*seq)++;
    break;
  case 2:
    // 戦闘ＢＧＭフェードアウト
    dbw->timeWait = 60;
    PMSND_FadeOutBGM(60);
    (*seq) ++;
    break;
  case 3:
    if(dbw->timeWait){
      dbw->timeWait--;
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
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, dbw->fieldmap, 0));
    (*seq) ++;
    break;
  case 6:
    BEW_Destructor( dbw );
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
  para->musicDefault = SEQ_WB_BA_TEST_250KB;
  //ピンチ時のBGMナンバー
  para->musicPinch = SEQ_WB_BA_PINCH_TEST_150KB;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void BATTLE_SETUP_PARAM_Destructor(BATTLE_SETUP_PARAM * para)
{
  if (para->partyPlayer)
  {
    GFL_HEAP_FreeMemory(para->partyPlayer);
  }
  if (para->partyPartner)
  {
    GFL_HEAP_FreeMemory(para->partyPartner);
  }
  if (para->partyEnemy1)
  {
    GFL_HEAP_FreeMemory(para->partyEnemy1);
  }
  if (para->partyEnemy2)
  {
    GFL_HEAP_FreeMemory(para->partyEnemy2);
  }
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
  switch ((BtlResult)bew->para.result)
  {
  case BTL_RESULT_LOSE:        ///< 負けた
    return TRUE;
  default:
    GF_ASSERT_MSG(0, "想定外のBtlResult %d\n", bew->para.result );
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
  //前作では貯金への反映、サファリボールカウントの反映、
  //いったん取っておいたPokeParamの反映などを行っていた
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  MI_CpuClear8( bew, sizeof(BATTLE_EVENT_WORK) );
  bew->gsys = gsys;
  bew->fieldmap = fieldmap;
  bew->bgmpush_off = FALSE;
  bew->timeWait = 0;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void BEW_Destructor(BATTLE_EVENT_WORK * bew)
{
  BATTLE_SETUP_PARAM_Destructor(&bew->para);
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
