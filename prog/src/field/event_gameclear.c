//============================================================================================
/**
 * @file  event_gameclear.c
 * @brief イベント：ゲームクリア処理
 * @date  2009.12.14
 * @author  tamada GAME FREAK inc.
 *
 * 2009.12.14 event_gameover.cからコピペで作成
 *
 */
//============================================================================================
#include "playable_version.h"
#include <gflib.h>

#include "system/main.h"  // for HEAPID_xxxx
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "sound/pm_sndsys.h"    // for PMSND_xxxx
#include "app/local_tvt_sys.h"  // for LocalTvt_ProcData etc.  
#include "demo/demo3d.h"        // for Demo3DProcData etc.
#include "demo/demo3d_demoid.h" // for DEMO3D_ID_xxxx
#include "demo/dendou_demo.h"
#include "demo/staff_roll.h"
#include "demo/the_end.h"

#include "script.h"           // for SCRIPT_CallGameClearScript
#include "event_gameclear.h"  // for GAMECLEAR_MODE
#include "event_field_fade.h"
#include "event_fieldmap_control_local.h"
#include "event_gamestart.h"
#include "event_demo3d.h"
#include "move_pokemon.h" 
#include "proc_gameclear_save.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#ifdef PLAYABLE_VERSION
extern GFL_PROC_DATA PlayableEndProcData;
#endif
//==============================================================================================
//==============================================================================================
#define MAX_SEQ_NUM (16) // 実行するシーケンスの最大数
#define WAIT_FRAME  (60)


//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {

  GAMESYS_WORK*       gsys;
  GAMEDATA*           gamedata;
  GAMECLEAR_MODE      clear_mode;
  BOOL                saveSuccessFlag;
  const MYSTATUS*     mystatus; 
  GAMECLEAR_MSG_PARAM para_child;
  DENDOUDEMO_PARAM    dendouDemoParam;    // 殿堂入り デモのパラメータ
  STAFFROLL_DATA      staffRollDemoParam; // スタッフロール デモのパラメータ
  THE_END_PARAM       theEndDemoParam;    // 「THE END」デモ画面用のパラメータ

  int nowSeq; // 現在のシーケンス
  int seqArray[ MAX_SEQ_NUM ]; // 実行シーケンス配列
  int waitCount; // 待ちフレーム数カウンタ

} GAMECLEAR_WORK;


//============================================================================================
//============================================================================================
// メインシーケンス
enum {
  GMCLEAR_SEQ_INIT,             // 初期化
  GMCLEAR_SEQ_FADEOUT,          // フェードアウト
  GMCLEAR_SEQ_COMM_END_REQ,     // 通信終了リクエスト発行
  GMCLEAR_SEQ_COMM_END_WAIT,    // 通信終了待ち
  GMCLEAR_SEQ_FIELD_CLOSE_WAIT, // フィールドマップ終了待ち
  GMCLEAR_SEQ_DENDOU_DEMO,      // 殿堂入りデモ
  GMCLEAR_SEQ_DENDOU_DEMO_WAIT, // 殿堂入りデモ終了待ち
  GMCLEAR_SEQ_STAFF_ROLL,       // スタッフロール
  GMCLEAR_SEQ_STAFF_ROLL_WAIT,  // スタッフロール終了待ち
  GMCLEAR_SEQ_ENDING_DEMO,      // エンディングデモ
  GMCLEAR_SEQ_CLEAR_SCRIPT,     // ゲームクリアスクリプト処理
  GMCLEAR_SEQ_SAVE_MESSAGE,     // セーブ中メッセージ表示
  GMCLEAR_SEQ_FRAME_WAIT,       // フレーム経過待ち
  GMCLEAR_SEQ_THE_END,          //「THE END」表示
  GMCLEAR_SEQ_END,              // 終了
};


//============================================================================================
//============================================================================================
static void SetupDendouDemoParam( GAMECLEAR_WORK* work ); // 殿堂入りデモのパラメータをセットアップする
static void SetupStaffRollDemoParam( GAMECLEAR_WORK* work ); // スタッフロールデモのパラメータをセットアップする
static void SetupSequence( GAMECLEAR_WORK* work ); // シーケンスの流れをセットアップする
static u16 GetEndingDemoID( void ); // エンディングの3DデモIDを取得する
static void NowSeqFinish( GAMECLEAR_WORK* work, int* seq ); // 現在のシーケンスを終了する
static void ElboardStartChampNews( GAMECLEAR_WORK* work ); // 電光掲示板にチャンピオンニュースの表示を開始する


//============================================================================================
//============================================================================================

//-----------------------------------------------------------------------------
/**
 * @brief ゲームクリアイベント
 * @param event   イベント制御ワークへのポインタ
 * @retval  TRUE    イベント終了
 * @retval  FALSE   イベント継続中
 *
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_GameClear(GMEVENT * event, int * seq, void *wk)
{
  GAMECLEAR_WORK * work = wk;
  GMEVENT * call_event;
  GAMESYS_WORK * gsys = work->gsys;
  GAMEDATA * gamedata = work->gamedata;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( gamedata );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( work->gsys );
  GAME_COMM_SYS_PTR gameComm = GAMESYSTEM_GetGameCommSysPtr( work->gsys );

  switch( work->nowSeq ) {
  case GMCLEAR_SEQ_INIT:
#ifndef  PLAYABLE_VERSION
    if( work->clear_mode == GAMECLEAR_MODE_DENDOU ) {
      GF_ASSERT( EVENTWORK_CheckEventFlag( GAMEDATA_GetEventWork( gamedata ), SYS_FLAG_GAME_CLEAR ) ); // ゲームクリアなしで殿堂入り ( 通常はありえない )
      ElboardStartChampNews( wk ); // 電光掲示板にチャンピオンニュースを表示
    }
#endif
    PMSND_FadeOutBGM( 30 );
    NowSeqFinish( work, seq );
    break;

  // フィールドマップをフェードアウト
  case GMCLEAR_SEQ_FADEOUT:
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeOut_Black( gsys, fieldmap, FIELD_FADE_WAIT ) ); 
    NowSeqFinish( work, seq );
    break;

  // 通信終了リクエスト発行
  case GMCLEAR_SEQ_COMM_END_REQ:
    //通信が動いている場合は終了させる
	  if(GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL){
      GameCommSys_ExitReq(gameComm);
    }
    NowSeqFinish( work, seq );
    break;

  // 通信終了待ち
  case GMCLEAR_SEQ_COMM_END_WAIT:
    if( GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL ) {
      break;
    }
    NowSeqFinish( work, seq );
    break;

  // フィールドマップを終了待ち
  case GMCLEAR_SEQ_FIELD_CLOSE_WAIT:
    GMEVENT_CallEvent( event, EVENT_FieldClose_FieldProcOnly( gsys, fieldmap ) );
    NowSeqFinish( work, seq );
    break;

  // 殿堂入りデモ呼び出し
  case GMCLEAR_SEQ_DENDOU_DEMO:
    GAMESYSTEM_CallProc( gsys, 
        FS_OVERLAY_ID(dendou_demo), &DENDOUDEMO_ProcData, &work->dendouDemoParam );
    NowSeqFinish( work, seq );
    break; 
  // 殿堂入りデモ終了待ち
  case GMCLEAR_SEQ_DENDOU_DEMO_WAIT:  
    if( GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL ) { break; }
    NowSeqFinish( work, seq );
    break;

  // スタッフロール デモ呼び出し
  case GMCLEAR_SEQ_STAFF_ROLL:
    GAMESYSTEM_CallProc( gsys, 
        FS_OVERLAY_ID(staff_roll), &STAFFROLL_ProcData, &work->staffRollDemoParam );
    NowSeqFinish( work, seq );
    break; 

  // スタッフロール終了待ち
  case GMCLEAR_SEQ_STAFF_ROLL_WAIT:
    if( GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL ) { break; }
    NowSeqFinish( work, seq );
    break;

  // エンディングデモ
  case GMCLEAR_SEQ_ENDING_DEMO:
    GMEVENT_CallEvent( event, 
        EVENT_CallDemo3D( gsys, event,  GetEndingDemoID(), 0, FALSE ) );
    NowSeqFinish( work, seq );
    break;

  case GMCLEAR_SEQ_CLEAR_SCRIPT:
    SCRIPT_CallGameClearScript( gsys, HEAPID_PROC ); 
    NowSeqFinish( work, seq );
    break; 
  case GMCLEAR_SEQ_SAVE_MESSAGE:
    GMEVENT_CallProc( event, 
        FS_OVERLAY_ID(gameclear_demo), &GameClearMsgProcData, &work->para_child );
    NowSeqFinish( work, seq );
    break;

  case GMCLEAR_SEQ_FRAME_WAIT:
    if( WAIT_FRAME < work->waitCount ) {
      work->waitCount = 0;
      NowSeqFinish( work, seq );
    }
    work->waitCount++;
    break;

  case GMCLEAR_SEQ_THE_END:
#ifdef  PLAYABLE_VERSION
    GMEVENT_CallProc( event,
        FS_OVERLAY_ID( gameclear_demo ), &PlayableEndProcData, gamedata );
#else
    GMEVENT_CallProc( event, 
        FS_OVERLAY_ID(the_end), &THE_END_ProcData, &work->theEndDemoParam );
		GAMESYSTEM_SetAlwaysNetFlag( gsys, FALSE );
#endif
    NowSeqFinish( work, seq );
    break;

  case GMCLEAR_SEQ_END:
    return GMEVENT_RES_FINISH;

  case GMCLEAR_SEQ_END + 1:
    /* 無限ループ */
    return GMEVENT_RES_CONTINUE;

  }
  return GMEVENT_RES_CONTINUE;
}


//-----------------------------------------------------------------------------
/**
 * @brief イベントコマンド：通常全滅処理
 * @param event   イベント制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_GameClear( GAMESYS_WORK * gsys, GAMECLEAR_MODE mode )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  GMEVENT * event;
  GAMECLEAR_WORK * gcwk;

  event = GMEVENT_Create( gsys, NULL, GMEVENT_GameClear, sizeof(GAMECLEAR_WORK) );

  gcwk = GMEVENT_GetEventWork( event );
  gcwk->gsys = gsys;
  gcwk->gamedata = gamedata;
  gcwk->clear_mode = mode;
  gcwk->mystatus = GAMEDATA_GetMyStatus( gamedata );
  gcwk->waitCount = 0;

  //子プロセスに渡すパラメータ
  gcwk->para_child.gsys = gsys;
  gcwk->para_child.clear_mode = mode;

  //移動ポケモン復活チェック
  MP_RecoverMovePoke( gamedata );

  // デモパラメータを設定
  SetupDendouDemoParam( gcwk );    // 殿堂入り
  SetupStaffRollDemoParam( gcwk ); // スタッフロール

  // 実行するシーケンスの流れを決定
  SetupSequence( gcwk );

  return event;
} 



//==============================================================================================
//==============================================================================================

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入りデモのパラメータをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupDendouDemoParam( GAMECLEAR_WORK* work )
{
  work->dendouDemoParam.party    = GAMEDATA_GetMyPokemon( work->gamedata );
  work->dendouDemoParam.mystatus = GAMEDATA_GetMyStatus( work->gamedata );
  work->dendouDemoParam.ptime    = GAMEDATA_GetPlayTimeWork( work->gamedata );
}

//-----------------------------------------------------------------------------
/**
 * @brief スタッフロールのデモパラメータをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupStaffRollDemoParam( GAMECLEAR_WORK* work )
{
  // 殿堂入りの場合はスキップ可能
  work->staffRollDemoParam.fastMode = ( work->clear_mode == GAMECLEAR_MODE_DENDOU );

  // 文字モードを設定
  {
    SAVE_CONTROL_WORK* save;
    MOJIMODE mojiMode;
    CONFIG* config;
    
    save = GAMEDATA_GetSaveControlWork( work->gamedata );
    config = SaveData_GetConfig( save );
    mojiMode = CONFIG_GetMojiMode( config );

    work->staffRollDemoParam.mojiMode = mojiMode;

    // 「THE END」画面用の設定
    work->theEndDemoParam.mojiMode    = mojiMode;
  }
  
  
}

//-----------------------------------------------------------------------------
/**
 * @brief シーケンスの流れをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SetupSequence( GAMECLEAR_WORK* work )
{
  int pos = 0;

#ifdef  PLAYABLE_VERSION
  switch( work->clear_mode ) {
  case 0:
    //work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
    work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_ENDING_DEMO;  //LegendMeet
    work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;      //「製品版をお楽しみに」
    work->seqArray[pos++] = GMCLEAR_SEQ_END;
    break;
  case 1:
    work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_ENDING_DEMO;  //LegendMeet
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;      //「製品版をお楽しみに」
    work->seqArray[pos++] = GMCLEAR_SEQ_END;
    break;
  case 2:
    work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
    work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
    //work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_ENDING_DEMO;  //LegendMeet
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
    //work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
    //work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;      //「製品版をお楽しみに」
    work->seqArray[pos++] = GMCLEAR_SEQ_END;
    break;
  }
#else
  switch( work->clear_mode ) {
  default: GF_ASSERT(0);
  case GAMECLEAR_MODE_FIRST:
    work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
    work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_ENDING_DEMO;
    work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
    work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
    work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;
    work->seqArray[pos++] = GMCLEAR_SEQ_END;
    break;
  case GAMECLEAR_MODE_DENDOU:
    work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
    work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
    work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_DENDOU_DEMO;
    work->seqArray[pos++] = GMCLEAR_SEQ_DENDOU_DEMO_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
    work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
    work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
    work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;
    work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;
    work->seqArray[pos++] = GMCLEAR_SEQ_END;
    break;
  }
#endif

  work->nowSeq = work->seqArray[0];
}

//-----------------------------------------------------------------------------
/**
 * @brief エンディングの3DデモIDを取得する
 * 
 * @return エンディングのデモID
 */
//-----------------------------------------------------------------------------
static u16 GetEndingDemoID( void )
{
  u16 demo_id;

  switch( GetVersion() ) {
  default: GF_ASSERT(0);
#ifdef  PLAYABLE_VERSION
  case VERSION_WHITE: demo_id = DEMO3D_ID_N_LEGEND_MEAT_W; break;
  case VERSION_BLACK: demo_id = DEMO3D_ID_N_LEGEND_MEAT_B; break;
#else
  case VERSION_WHITE: demo_id = DEMO3D_ID_ENDING_W; break;
  case VERSION_BLACK: demo_id = DEMO3D_ID_ENDING_B; break;
#endif
  }

  return demo_id;
}

//-----------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 */
//-----------------------------------------------------------------------------
static void NowSeqFinish( GAMECLEAR_WORK* work, int* seq )
{
  (*seq)++;
  work->nowSeq = work->seqArray[ *seq ];
}

//-----------------------------------------------------------------------------
/**
 * @brief 電光掲示板にチャンピオンニュースの表示を開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ElboardStartChampNews( GAMECLEAR_WORK* work )
{
  MISC* misc;
  int minutes;

  misc = GAMEDATA_GetMiscWork( work->gamedata );
  minutes = 60 * 24; // 表示する時間[min]
  MISC_SetChampNewsMinutes( misc, minutes );
}
