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
#include "system/main.h" //HEAPID_PROC参照

//#include "poke_tool/monsno_def.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "event_encount_effect.h"
#include "effect_encount.h"
#include "field_sound.h"
#include "event_gameover.h" //EVENT_NormalLose
#include "move_pokemon.h"
#include "event_battle_return.h"

#include "script_def.h"   //SCR_BATTLE_～

//======================================================================
//  define
//======================================================================
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

  ///タイミング用ワーク（主にBGMフェード制御）
  u16 timeWait;

  ///BGMがすでに退避済みか？フラグ
  BOOL bgm_pushed_flag;

  /** @brief  サブイベントかどうか？フラグ
   * サブイベントの場合は、敗北処理など終了部分を呼び出し元から
   * 呼び出してもらう必要があるため、フックする
   */
  BOOL is_sub_event;

  /** @brief  敗北処理がないバトルか？のフラグ */
  BOOL is_no_lose;

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
GMEVENT * EVENT_WildPokeBattle( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp, BOOL sub_event_f )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;

  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = sub_event_f;

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
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int tr_id0, int tr_id1, u32 flags )
{
  GMEVENT * event;
  BATTLE_EVENT_WORK * bew;
  BATTLE_SETUP_PARAM* bp;

#if 0
  {
    POKEPARTY * myparty = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys));
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( myparty, 0 );
    PP_Put( pp, ID_PARA_hp, 1 );
  }
#endif
  event = GMEVENT_Create(
      gsys, NULL, fieldBattleEvent, sizeof(BATTLE_EVENT_WORK) );

  bew = GMEVENT_GetEventWork(event);

  {
    FIELD_ENCOUNT* enc = FIELDMAP_GetEncount(fieldmap);

    bp = BATTLE_PARAM_Create(HEAPID_PROC);
    FIELD_ENCOUNT_SetTrainerBattleParam( enc, bp, tr_id0, tr_id1, HEAPID_PROC );
  }

  BEW_Initialize( bew, gsys, bp );
  bew->is_sub_event = TRUE;   //スクリプトから呼ばれる＝トップレベルのイベントでない
  if ( (flags & SCR_BATTLE_MODE_NOLOSE) != 0 )
  {
    bew->is_no_lose = TRUE;
  }
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
  
  bew = GMEVENT_GetEventWork(event);
  BEW_Initialize( bew, gsys, bp );
  
  //エフェクトエンカウト　エフェクト復帰キャンセル
  EFFECT_ENC_EffectRecoverCancel( FIELDMAP_GetEncount(fieldmap));
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
    switch ( FIELD_SOUND_GetBGMPushCount( fsnd ) )
    {
    case FSND_PUSHCOUNT_NONE:
      //通常フィールドBGM階層をPush
      FIELD_SOUND_PushBGM( fsnd );
      break;
    case FSND_PUSHCOUNT_BASEBGM:
      //イベントBGM階層なので何もしない（ここにくるまでにPushされている）
      break;
    case FSND_PUSHCOUNT_EVENTBGM:
    case FSND_PUSHCOUNT_OVER:
      GF_ASSERT_MSG(0, "EventBattle:BGM階層を重ねすぎています！！\n");
    }
    bew->bgm_pushed_flag = TRUE;
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
  case 3:
    bew->btlret_param.btlResult = bew->battle_param;
    bew->btlret_param.gameData  = gamedata;
    GMEVENT_CallProc( event, FS_OVERLAY_ID(battle_return), &BtlRet_ProcData, &bew->btlret_param );
    (*seq)++;
    break;
  case 4:
    //侵入システムにフィールド中であることを伝える
    {
      FIELD_STATUS *fldstatus = GAMEDATA_GetFieldStatus(gamedata);
      FIELD_STATUS_SetProcAction( fldstatus, PROC_ACTION_FIELD );
      COMM_PLAYER_SUPPORT_Init(GAMEDATA_GetCommPlayerSupportPtr(gamedata));
    }
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
      FIELD_SOUND_PopBGM( fsnd );
      bew->bgm_pushed_flag = FALSE;
    }

    //戦闘結果反映処理
    BEW_reflectBattleResult( bew, gamedata );

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
      PMSND_FadeInBGM(BATTLE_BGM_FADEIN_WAIT);
      GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    }
    (*seq) ++;
    break;
  case 7:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
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
  GF_ASSERT_MSG( competitor <= BTL_COMPETITOR_COMM, "想定外のBtlCompetitor(%d)\n", competitor);
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
  bew->timeWait = 0;
}

//--------------------------------------------------------------
/**
 * @brief BATTLE_EVENT_WORKの終了処理
 * @param bew   BATTLE_EVENT_WORKへのポインタ
 */
//--------------------------------------------------------------
static void BEW_Destructor(BATTLE_EVENT_WORK * bew)
{
  BATTLE_PARAM_Delete( bew->battle_param );
}

