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
#include "net_app/key_system.h"
#include "savedata/medalbox.h"

#include "fieldmap/zone_id.h"   // for ZONE_ID_
#include "field/field_const.h"  // for GRID_TO_FX32
#include "event_mapchange.h"    // for 
#include "event_fieldmap_control.h" // EVENT_FieldOpen
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/script/c09p03_def.h" // for SCRID_C09P03_
#include "../../../resource/fldmapdata/script/c12_def.h" // for SCRID_C12_

//==============================================================================================
//==============================================================================================
#define MAX_SEQ_NUM (31) // 実行するシーケンスの最大数
#define WAIT_FRAME  (60)

#define	BGM_FADE_FRAME  ( 64 )			// BGMフェードアウトフレーム数

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
  KEY_SYSTEM_PARAM    keySystemParam;     // クリア時のキー入手デモパラメータ

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
  GMCLEAR_SEQ_GET_CLEAR_KEY,    // クリア時のキー入手デモ
  GMCLEAR_SEQ_SAVE_MESSAGE,     // セーブ中メッセージ表示
  GMCLEAR_SEQ_FRAME_WAIT,       // フレーム経過待ち
  GMCLEAR_SEQ_THE_END,          //「THE END」表示
  // swan追加 ichiraku
  GMCLEAR_SEQ_MAP_CHANGE_FIRST, // マップチェンジ(C09P03)
  GMCLEAR_SEQ_MAP_CHANGE_SECOND,// マップチェンジ(C12)
  GMCLEAR_SEQ_SCRIPT_CALL_FIRST,// スクリプト呼び出し(C09P03)
  GMCLEAR_SEQ_SCRIPT_CALL_SECOND,// スクリプト呼び出し(C12)
  GMCLEAR_SEQ_FIELD_OPEN_WAIT,  // フィールドマップ開始待ち
  GMCLEAR_SEQ_FADEIN,           // フェードイン
  GMCLEAR_SEQ_BGM_PLAY,         // BGM再生
  GMCLEAR_SEQ_BGM_FADE_OUT,     // BGMフェードアウト
  GMCLEAR_SEQ_BGM_STOP,         // BGM停止
  //

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
static void CheckMedalAchieve( GAMECLEAR_WORK* work );  // クリア時のメダルチェック


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
    // swanではゲームクリア = 殿堂入りになりました。
    //if( work->clear_mode == GAMECLEAR_MODE_DENDOU ) {
      //GF_ASSERT( EVENTWORK_CheckEventFlag( GAMEDATA_GetEventWork( gamedata ), SYS_FLAG_GAME_CLEAR ) ); // ゲームクリアなしで殿堂入り ( 通常はありえない )
      ElboardStartChampNews( wk ); // 電光掲示板にチャンピオンニュースを表示
    //}
    
    // スクリプト初期化用のフラグをたてる
    //  c09p03.ev c12.ev の初期化に使用
    EVENTWORK_SetEventFlag( GAMEDATA_GetEventWork( gamedata ), SYS_FLAG_ENDING );
 
    // メダル達成処理
    CheckMedalAchieve( wk );

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

  case GMCLEAR_SEQ_GET_CLEAR_KEY:
    //念のため、既に持っていたら出さない
    {
      KEY_SYSTEM_SAVE *p_key_sv = SAVE_CONTROL_GetKeySystem(save);
      if( !KEY_SYSTEM_SAVE_IsHave(p_key_sv, work->keySystemParam.get_key) )
      {
        GMEVENT_CallProc( event, 
            FS_OVERLAY_ID(key_system), &KEY_SYSTEM_ProcData, &work->keySystemParam );
      }
    }
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
    GMEVENT_CallProc( event, 
        FS_OVERLAY_ID(the_end), &THE_END_ProcData, &work->theEndDemoParam );
		GAMESYSTEM_SetAlwaysNetFlag( gsys, FALSE );
    NowSeqFinish( work, seq );
    break;

  //=====================================
  // ↓ここから↓
  // swan追加 ichiraku
  
  // マップチェンジ(C09P03)
  case GMCLEAR_SEQ_MAP_CHANGE_FIRST:
    {
      VecFx32 pos = { GRID_TO_FX32(31), GRID_TO_FX32(47), GRID_TO_FX32(1) };
      GMEVENT_CallEvent( event, 
          EVENT_ChangeMapPosNoFadeBGMKeep( gsys, fieldmap, ZONE_ID_C09P03, &pos, DIR_DOWN ) ); 
      NowSeqFinish( work, seq );
    }
    break;

  // マップチェンジ(C12)
  case GMCLEAR_SEQ_MAP_CHANGE_SECOND:
    {
      VecFx32 pos = { GRID_TO_FX32(43), GRID_TO_FX32(1), GRID_TO_FX32(757) };
      GMEVENT_CallEvent( event, 
          EVENT_ChangeMapPosNoFadeBGMKeep( gsys, fieldmap, ZONE_ID_C12, &pos, DIR_DOWN ) ); 
      NowSeqFinish( work, seq );
    }
    break;

  // スクリプト呼び出し(C09P03)
  case GMCLEAR_SEQ_SCRIPT_CALL_FIRST:
    {
      SCRIPT_CallScript( event, SCRID_C09P03_ENDING, NULL, NULL, HEAPID_PROC );
      NowSeqFinish( work, seq );
    }
    break;

  // スクリプト呼び出し(C12)
  case GMCLEAR_SEQ_SCRIPT_CALL_SECOND:
    {
      SCRIPT_CallScript( event, SCRID_C12_ENDING, NULL, NULL, HEAPID_PROC );
      NowSeqFinish( work, seq );
    }
    break;

  // フィールドマップ開始待ち
  case GMCLEAR_SEQ_FIELD_OPEN_WAIT:
    {
      GMEVENT_CallEvent( event, EVENT_FieldOpen_FieldProcOnly( gsys ) );
      NowSeqFinish( work, seq );
    }
    break;

  // フェードイン
  case GMCLEAR_SEQ_FADEIN:
    {
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) ); 
      NowSeqFinish( work, seq );
    }
    break;

  // BGM再生
  case GMCLEAR_SEQ_BGM_PLAY:
    {
      // BGM再生開始
      // 2回目のスクリプトが終了まで再生する。
      //
      // 【仮】ラベルは仮
      PMSND_PlayBGM( SEQ_BGM_ENDING );

      NowSeqFinish( work, seq );
    }
    break;

  // BGMフェードアウト
  case GMCLEAR_SEQ_BGM_FADE_OUT:
    {
      if( PMSND_CheckPlayBGM() == TRUE ){
        PMSND_FadeOutBGM( BGM_FADE_FRAME );
      }
      NowSeqFinish( work, seq );
    }
    break;

  // BGM停止
  case GMCLEAR_SEQ_BGM_STOP:
    {
      if( PMSND_CheckFadeOnBGM() == TRUE ){
        break;
      }
      if( PMSND_CheckPlayBGM() == TRUE ){
        PMSND_StopBGM();
      }
      NowSeqFinish( work, seq );
    }
    break;

  case GMCLEAR_SEQ_END:
    // スクリプト初期化用のフラグをリセット
    EVENTWORK_ResetEventFlag( GAMEDATA_GetEventWork( gamedata ), SYS_FLAG_ENDING );

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
  {
    gcwk->keySystemParam.p_gamedata = gamedata;
    gcwk->keySystemParam.mode       = KEY_SYSTEM_MODE_GET;
    if( GET_VERSION() == VERSION_B2 )
    {
      gcwk->keySystemParam.get_key    = KEY_SYSTEM_TYPE_DIFFICULT_HARD;
    }
    else
    {
      gcwk->keySystemParam.get_key    = KEY_SYSTEM_TYPE_DIFFICULT_EASY;
    }
  }

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

#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_ichiraku_katsuhiko)
  // デバッグモード起動
  work->staffRollDemoParam.fastMode = STAFFROLL_MODE_DEBUG;
#endif
#endif


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

  // *****************************************
  // シーケンスを増やしたら
  // シーケンスの最大数の定義を増やすこと！
  // *****************************************
  work->seqArray[pos++] = GMCLEAR_SEQ_INIT;
  work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_REQ;
  work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
  work->seqArray[pos++] = GMCLEAR_SEQ_COMM_END_WAIT;
  work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;
  work->seqArray[pos++] = GMCLEAR_SEQ_DENDOU_DEMO;
  work->seqArray[pos++] = GMCLEAR_SEQ_DENDOU_DEMO_WAIT;
  if ( work->clear_mode == GAMECLEAR_MODE_FIRST )
  {
    work->seqArray[pos++] = GMCLEAR_SEQ_GET_CLEAR_KEY;
    work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;
  }
  work->seqArray[pos++] = GMCLEAR_SEQ_CLEAR_SCRIPT;
  work->seqArray[pos++] = GMCLEAR_SEQ_SAVE_MESSAGE;
  work->seqArray[pos++] = GMCLEAR_SEQ_FRAME_WAIT;

  // 最初のスクリプト
  work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_OPEN_WAIT;
  work->seqArray[pos++] = GMCLEAR_SEQ_MAP_CHANGE_FIRST;
  work->seqArray[pos++] = GMCLEAR_SEQ_BGM_PLAY;
  work->seqArray[pos++] = GMCLEAR_SEQ_SCRIPT_CALL_FIRST;
  work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;

  // スタッフロール
  work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL;
  work->seqArray[pos++] = GMCLEAR_SEQ_STAFF_ROLL_WAIT;

  // 2回目のスクリプト
  work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_OPEN_WAIT;
  work->seqArray[pos++] = GMCLEAR_SEQ_MAP_CHANGE_SECOND;
  work->seqArray[pos++] = GMCLEAR_SEQ_FADEIN;
  work->seqArray[pos++] = GMCLEAR_SEQ_SCRIPT_CALL_SECOND;
  work->seqArray[pos++] = GMCLEAR_SEQ_BGM_FADE_OUT;
  work->seqArray[pos++] = GMCLEAR_SEQ_FADEOUT;
  work->seqArray[pos++] = GMCLEAR_SEQ_BGM_STOP;
  work->seqArray[pos++] = GMCLEAR_SEQ_FIELD_CLOSE_WAIT;

  work->seqArray[pos++] = GMCLEAR_SEQ_THE_END;
  work->seqArray[pos++] = GMCLEAR_SEQ_END;

#if 0
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
    work->seqArray[pos++] = GMCLEAR_SEQ_GET_CLEAR_KEY;
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
  case VERSION_W2: demo_id = DEMO3D_ID_ENDING_W; break;
  case VERSION_B2: demo_id = DEMO3D_ID_ENDING_B; break;
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

//----------------------------------------------------------------------------
/**
 *	@brief  クリア時メダル達成処理
 *
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void CheckMedalAchieve( GAMECLEAR_WORK* work )
{
  GAMEDATA *p_gdata = GAMESYSTEM_GetGameData( work->gsys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gdata );
  MEDALBOX *p_medal_box = SaveData_GetMedalBox( p_sv );

  // ヒントメダル達成
  MEDALBOX_SetHintMedalAchive( p_medal_box , MDLID_TAMAGO_LV1 );        // タマゴビギナー
  MEDALBOX_SetHintMedalAchive( p_medal_box , MDLID_SODATEYA_CNT_LV1 );  //そだてやマニア

  // 手持ちチェック
  {
    s32 i;
    POKEPARTY *p_my_party = GAMEDATA_GetMyPokemon( p_gdata );			// 手持ちモケモン
    s32 party_cnt = PokeParty_GetPokeCount( p_my_party );
    s32 mons_num = 0;
    s32 type_cnt[POKETYPE_MAX];

    // タイプと同じ並びの指定持ち物限定クリアメダルID
    const MedalID type_medal_id_tbl[] =
    {
      MDLID_CHAMPION_NORMAL,
      MDLID_CHAMPION_KAKUTOU,
      MDLID_CHAMPION_HIKOU,
      MDLID_CHAMPION_DOKU,
      MDLID_CHAMPION_JIMEN,
      MDLID_CHAMPION_IWA,
      MDLID_CHAMPION_MUSHI,
      MDLID_CHAMPION_GHOST,
      MDLID_CHAMPION_HAGANE,
      MDLID_CHAMPION_HONOO,
      MDLID_CHAMPION_MIZU,
      MDLID_CHAMPION_KUSA,
      MDLID_CHAMPION_DENKI,
      MDLID_CHAMPION_ESPER,
      MDLID_CHAMPION_KOORI,
      MDLID_CHAMPION_DRAGON,
      MDLID_CHAMPION_AKU,
    };

    // クリア
    GFL_STD_MemClear( type_cnt, sizeof(type_cnt) );

    // パーティー数
    for( i = 0; i < party_cnt; i++ )
    {
      POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_my_party , i );
      BOOL is_egg = PP_Get( p_pp , ID_PARA_tamago_flag , NULL );
      if( is_egg != TRUE )
      {
        s32 type1, type2;
        // タイプ１は必ずカウント
        type1 = PP_Get( p_pp, ID_PARA_type1, NULL );
        type_cnt[type1]++;

        // タイプ１と違うときのみカウント
        type2 = PP_Get( p_pp, ID_PARA_type2, NULL );
        if( type1 != type2 )
        {
          type_cnt[type2]++;
        }
        // モンスター数カウント
        mons_num++;
      }
    }

    // タイプごとのポケモン数と比較し、手持ちと同じ数ならメダル獲得
    for( i = 0; i < POKETYPE_MAX; i++ )
    {
      if( type_cnt[i] == mons_num ) 
      {
        // ヒントメダル達成
        MEDALBOX_SetTitleMedalAchive( p_medal_box , type_medal_id_tbl[i] );
        ONOUE_Printf("メダル取得：%d\n",i);
      }
    }
  }
}

