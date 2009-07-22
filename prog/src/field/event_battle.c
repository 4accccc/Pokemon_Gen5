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

extern const GFL_PROC_DATA DebugSogabeMainProcData;

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
/// DEBUG_BATTLE_WORK
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  FIELD_MAIN_WORK * fieldmap;
  BATTLE_SETUP_PARAM para;
  u16 timeWait;
}DEBUG_BATTLE_WORK;

//======================================================================
//  proto
//======================================================================
//BATTLE_SETUP_PARAMの解放処理
static const void BATTLE_SETUP_PARAM_Destructor(BATTLE_SETUP_PARAM * para);
//POKEPARTYにポケモンを加える
static const void addPartyPokemon(POKEPARTY * party, u16 monsno, u8 level, u16 id);

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );

//debug
static GMEVENT_RESULT DebugBattleEvent(
    GMEVENT * event, int *  seq, void * work );

const u32 data_EncountPoke200905[];
const u32 data_EncountPoke200905Max;


//======================================================================
//  フィールド　バトルイベント
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグイベント フィールドバトルイベント
 * @param gsys  GAMESYS_WORK
 * @param fieldmap FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * EVENT_Battle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap )
{
  GMEVENT * event;
  BATTLE_SETUP_PARAM * para;
  DEBUG_BATTLE_WORK * dbw;

  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(DEBUG_BATTLE_WORK) );

  dbw = GMEVENT_GetEventWork(event);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;
  para = &dbw->para;
#if 0
  {
    para->engine = BTL_ENGINE_ALONE;
    para->rule = BTL_RULE_SINGLE;
    para->competitor = BTL_COMPETITOR_WILD;

    para->netHandle = NULL;
    para->commMode = BTL_COMM_NONE;
    para->commPos = 0;
    para->netID = 0;

    //プレイヤーのパーティ
    para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );
    PokeParty_Copy(
        GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)),
        para->partyPlayer);

    //1vs1時の敵AI, 2vs2時の１番目敵AI用
    para->partyEnemy1 = PokeParty_AllocPartyWork( HEAPID_CORE );
    {
      u32 pokeNo200905 = GFUser_GetPublicRand(data_EncountPoke200905Max);
  //  u32 pokeNo = MONSNO_ARUSEUSU+1;
      u32 pokeNo = data_EncountPoke200905[pokeNo200905];
      addPartyPokemon( para->partyEnemy1, pokeNo, 15, 3594 );
      //PokeParty_Add( para->partyEnemy1, PP_Create(pokeNo,15,3594,HEAPID_CORE) );
    }

    //2vs2時の味方AI（不要ならnull）
    para->partyPartner = NULL;
    //2vs2時の２番目敵AI用（不要ならnull）
    para->partyEnemy2 = NULL;
    //
    para->statusPlayer = SaveData_GetMyStatus( SaveControl_GetPointer() );

    //デフォルト時のBGMナンバー
//    para->musicDefault = SEQ_WB_BA_TEST_250KB;
    para->musicDefault = SEQ_VS_NORAPOKE;
    //ピンチ時のBGMナンバー
    para->musicPinch = SEQ_WB_BA_PINCH_TEST_150KB;

    dbw->timeWait = 0;
  }
#else
  {
    FIELD_ENCOUNT *enc = FIELDMAP_GetEncount( fieldmap );
    FIELD_ENCOUNT_GetBattleSetupParam( enc, para );
  }
#endif
  
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
  DEBUG_BATTLE_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;

  switch (*seq) {
  case 0:
    // サウンドテスト
    // ＢＧＭ一時停止→退避
    PMSND_PauseBGM(TRUE);
    PMSND_PushBGM();

    // サウンドテスト
    // 戦闘用ＢＧＭセット
    PMSND_PlayBGM(dbw->para.musicDefault);

    //エンカウントエフェクト
    GMEVENT_CallEvent( event,
        EVENT_FieldEncountEffect(gsys,dbw->fieldmap) );
    (*seq)++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
    (*seq)++;
    break;
  case 2:
    GAMESYSTEM_CallProc(
        gsys, FS_OVERLAY_ID(battle), &BtlProcData, &dbw->para);
    (*seq)++;
    break;
  case 3:
    if (GAMESYSTEM_IsProcExists(gsys)) break;
    // サウンドテスト
    // 戦闘ＢＧＭフェードアウト
    dbw->timeWait = 60;
    PMSND_FadeOutBGM(60);
    (*seq) ++;
    break;
  case 4:
    if(dbw->timeWait){
      dbw->timeWait--;
    } else {
      (*seq) ++;
    }
    break;
  case 5:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    // サウンドテスト
    // ＢＧＭ取り出し→再開
    PMSND_PopBGM();
    PMSND_PauseBGM(FALSE);
    PMSND_FadeInBGM(60);
    //
    (*seq) ++;
    break;
  case 6:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, dbw->fieldmap, 0));
    (*seq) ++;
    break;
  case 7:
    BATTLE_SETUP_PARAM_Destructor(&dbw->para);
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
  DEBUG_BATTLE_WORK * dbw;

  event = GMEVENT_Create(
      gsys, NULL, DebugBattleEvent, sizeof(DEBUG_BATTLE_WORK) );

  dbw = GMEVENT_GetEventWork(event);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;
  para = &dbw->para;

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

    dbw->timeWait = 0;
  }

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
  DEBUG_BATTLE_WORK * dbw = work;
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
    // サウンドテスト
    // ＢＧＭ一時停止→退避
    PMSND_PauseBGM(TRUE);
    PMSND_PushBGM();
    (*seq)++;
    break;
  case 1:
    GAMESYSTEM_CallProc(
        gsys, FS_OVERLAY_ID(battle), &BtlProcData, &dbw->para);
    // サウンドテスト
    // 戦闘用ＢＧＭセット
    PMSND_PlayBGM(dbw->para.musicDefault);
    (*seq)++;
    break;
  case 2:
    if (GAMESYSTEM_IsProcExists(gsys)) break;
    // サウンドテスト
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
    // サウンドテスト
    // ＢＧＭ取り出し→再開
    PMSND_PopBGM();
    PMSND_PauseBGM(FALSE);
    PMSND_FadeInBGM(60);
    //
    (*seq) ++;
    break;
  case 5:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, dbw->fieldmap, 0));
    (*seq) ++;
    break;
  case 6:
    BATTLE_SETUP_PARAM_Destructor(&dbw->para);
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
static const void BATTLE_SETUP_PARAM_Destructor(BATTLE_SETUP_PARAM * para)
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
