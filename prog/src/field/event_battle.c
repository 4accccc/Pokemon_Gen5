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

#include "event_battle_return.h"


//======================================================================
//  define
//======================================================================
#define HEAPID_CORE GFL_HEAPID_APP

enum {
  BATTLE_BGM_FADEOUT_WAIT = 60,
  BATTLE_BGM_FADEIN_WAIT = 60,
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

  u16 timeWait;
  BOOL bgm_pushed_flag;
  BOOL is_sub_event;
}BATTLE_EVENT_WORK;

//======================================================================
//  proto
//======================================================================

static GMEVENT_RESULT fieldBattleEvent(
    GMEVENT * event, int *  seq, void * work );


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
  BEW_Initialize( bew, gsys, bp );

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

  BEW_Initialize( bew, gsys, bp );
  bew->bgm_pushed_flag = TRUE; //視線イベントBGM時に退避済み
  bew->is_sub_event = TRUE;   //スクリプトから呼ばれる＝トップレベルのイベントでない

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

  switch (*seq) {
  case 0:
    // 戦闘用ＢＧＭセット
    if( bew->bgm_pushed_flag == FALSE ){
      FIELD_SOUND_PushBGM( GAMEDATA_GetFieldSound( gamedata ) );
      bew->bgm_pushed_flag = TRUE;
    }
    PMSND_PlayBGM( bew->battle_param->musicDefault );
    //エンカウントエフェクト
    GMEVENT_CallEvent( event,
        EVENT_FieldEncountEffect(gsys,fieldmap) );
    (*seq)++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, fieldmap));
    (*seq)++;
    break;
  case 2:
    //バトルプロセス呼び出し：プロセスが終了したらこのイベントに復帰する
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle), &BtlProcData, bew->battle_param );
    (*seq)++;
    break;
  case 3:
    bew->btlret_param.btlResult = bew->battle_param;
    bew->btlret_param.gameData  = gamedata;
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle_return), &BtlRet_ProcData, &bew->btlret_param );
    (*seq)++;
    break;
  case 4:
    bew->timeWait = BATTLE_BGM_FADEOUT_WAIT; // 戦闘ＢＧＭフェードアウト
    PMSND_FadeOutBGM( BATTLE_BGM_FADEOUT_WAIT );
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
    if (bew->bgm_pushed_flag == TRUE) {
      FIELD_SOUND_PopBGM( GAMEDATA_GetFieldSound( gamedata ) );
      bew->bgm_pushed_flag = FALSE;
    }

    //戦闘結果反映処理
    BEW_reflectBattleResult( bew, gamedata );

    //勝ち負け判定
    if (BEW_IsLoseResult( bew) == TRUE )
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
      PMSND_FadeInBGM(BATTLE_BGM_FADEIN_WAIT);
      GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    }
    (*seq) ++;
    break;
  case 7:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, fieldmap, 0));
    (*seq) ++;
    break;
  case 8:
    BEW_Destructor( bew );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}



//======================================================================
//
//
//
//
//======================================================================
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
  case BTL_RESULT_CAPTURE:     ///< 捕まえた（野生のみ）
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
static void BEW_Initialize(BATTLE_EVENT_WORK * bew, GAMESYS_WORK * gsys, BATTLE_SETUP_PARAM* bp)
{
  GFL_STD_MemClear32( bew, sizeof(BATTLE_EVENT_WORK) );
  bew->gsys = gsys;
  bew->gamedata = GAMESYSTEM_GetGameData( gsys );
  bew->battle_param = bp;
  bew->bgm_pushed_flag = FALSE;
  bew->is_sub_event = FALSE;
  bew->timeWait = 0;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void BEW_Destructor(BATTLE_EVENT_WORK * bew)
{
  BATTLE_PARAM_Delete( bew->battle_param );
}

