//============================================================================================
/**
 * @file	event_gameclear.c
 * @brief	イベント：ゲームクリア処理
 * @date  2009.12.14
 * @author	tamada GAME FREAK inc.
 *
 * 2009.12.14 event_gameover.cからコピペで作成
 *
 * @todo
 * ゲームオーバーメッセージをPROCにしたのでメモリが足りないときには
 * 別オーバーレイ領域に引越しする
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"

#include "event_gameclear.h"  //GAMECLEAR_MODE

#include "system/main.h"  //HEAPID_～

#include "script.h"       //SCRIPT_CallScript
#include "event_gamestart.h"

#include "demo/demo3d.h"  //Demo3DProcData etc.
#include "app/local_tvt_sys.h"  //LocalTvt_ProcData etc.

#include "sound/pm_sndsys.h"  //PMSND_

#include "move_pokemon.h"

#include "proc_gameclear_save.h"

#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "savedata/gametime.h"
#include "savedata/playtime.h"
#include "ev_time.h"

//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA* gamedata;
  GAMECLEAR_MODE clear_mode;
  BOOL saveSuccessFlag;
  const MYSTATUS * mystatus;

  BOOL dendouSaveInitFlag;   //「殿堂入り」データの分割セーブ初期化が完了したかどうか
  BOOL dendouSaveFinishFlag; //「殿堂入り」データの分割セーブが完了したかどうか

  GAMECLEAR_MSG_PARAM para_child;
}GAMECLEAR_WORK;


//============================================================================================
//============================================================================================
// メインシーケンス
enum {
	GMCLEAR_SEQ_INIT,			    // 初期化
	GMCLEAR_SEQ_DENDOU_DEMO,	// 殿堂入りデモ
	GMCLEAR_SEQ_SAVE_START,		// セーブ開始
	GMCLEAR_SEQ_SAVE_MESSAGE,	// セーブ中メッセージ表示
	GMCLEAR_SEQ_SAVE_MAIN,		// セーブメイン
	GMCLEAR_SEQ_SAVE_END,			// セーブ終了
	GMCLEAR_SEQ_WAIT1,				// ウェイト１
	GMCLEAR_SEQ_FADE_OUT,			// フェードアウトセット
	GMCLEAR_SEQ_BGM_FADE_OUT,	// BGMフェードアウトセット
	GMCLEAR_SEQ_WAIT2,				// ウェイト２
	GMCLEAR_SEQ_ENDING_DEMO,	// エンディングデモ
	GMCLEAR_SEQ_END,				  // 終了
};


//============================================================================================
//============================================================================================
static void ElboardStartChampNews( GAMECLEAR_WORK* work ); // 電光掲示板にチャンピオンニュースの表示を開始する
static void UpdateFirstClearDendouData( GAMECLEAR_WORK* work ); // 初回クリア時の殿堂入りデータを登録する
static void UpdateFirstClearRecord( GAMECLEAR_WORK* work ); // 初回クリア時のレコードを登録する
static void UpdateDendouRecord( GAMECLEAR_WORK* work ); // 殿堂入り時のレコードを登録する
static void DendouSave_init( GAMECLEAR_WORK* work ); // 殿堂入りデータの更新準備
static void DendouSave_main( GAMECLEAR_WORK* work ); // 殿堂入りデータの更新処理

//============================================================================================
//============================================================================================

//-----------------------------------------------------------------------------
/**
 * @brief	ゲームクリアイベント
 * @param	event		イベント制御ワークへのポインタ
 * @retval	TRUE		イベント終了
 * @retval	FALSE		イベント継続中
 *
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_GameClear(GMEVENT * event, int * seq, void *work)
{
  GAMECLEAR_WORK * gcwk = work;
  GMEVENT * call_event;
  GAMEDATA * gamedata = gcwk->gamedata;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( gamedata );

	switch (*seq) {
	case GMCLEAR_SEQ_INIT:
    PMSND_FadeOutBGM( 30 );
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_DENDOU_DEMO:
    //if ( PMSND_CheckFadeOnBGM() == FALSE ) { break; }
    //if ( GFL_FADE_CheckFade() == TRUE ) { break; }
    //フィールドマップを終了させる
    { 
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gcwk->gsys );
      GMEVENT * new_event = DEBUG_EVENT_GameEnd( gcwk->gsys, fieldmap );
      GMEVENT_CallEvent( event, new_event );
    }
    SCRIPT_CallGameClearScript( gcwk->gsys, HEAPID_PROC ); 
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_START:
    // 電光掲示板にチャンピオンニュースを表示
    ElboardStartChampNews( gcwk );
    //「初回クリア」をセーブ
    if( gcwk->clear_mode == GAMECLEAR_MODE_FIRST ) {
      UpdateFirstClearDendouData( gcwk ); // 初「殿堂入り」データ
      EVTIME_SetGameClearDateTime( gamedata ); // 初回クリアの日時
      UpdateFirstClearRecord( gcwk ); // レコードデータ
    }
    //「殿堂入り」をセーブ
    else {
      UpdateDendouRecord( gcwk );
      DendouSave_init( gcwk );
    }
    (*seq) ++;
    break;

  case GMCLEAR_SEQ_SAVE_MESSAGE:
    GMEVENT_CallProc(
        event, FS_OVERLAY_ID(gameclear_demo), &GameClearMsgProcData, &gcwk->para_child );
    (*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_MAIN:
    // 殿堂入りデータをセーブ
    if( gcwk->dendouSaveInitFlag ) {
      DendouSave_main( gcwk );

      // セーブ終了
      if( gcwk->dendouSaveFinishFlag ) {
        *seq = GMCLEAR_SEQ_END;
      }
    }
    else {
      *seq = GMCLEAR_SEQ_END;
    }
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
 * @brief	イベントコマンド：通常全滅処理
 * @param	event		イベント制御ワークへのポインタ
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
  gcwk->dendouSaveInitFlag = FALSE;
  gcwk->dendouSaveFinishFlag = FALSE;

  //子プロセスに渡すパラメータ
  gcwk->para_child.gsys = gsys;
  gcwk->para_child.clear_mode = mode;

  //移動ポケモン復活チェック
  MP_RecoverMovePoke( gamedata );

  return event;
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

//-----------------------------------------------------------------------------
/**
 * @brief 初回クリア時の殿堂入りデータを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateFirstClearDendouData( GAMECLEAR_WORK* work )
{
  RTCDate date;
  POKEPARTY* party; 
  DENDOU_RECORD* record; 

  RTC_GetDate( &date );
  party  = GAMEDATA_GetMyPokemon( work->gamedata ); 
  record = GAMEDATA_GetDendouRecord( work->gamedata ); 
  DendouRecord_Add( record, party, &date, HEAPID_PROC );
}

//-----------------------------------------------------------------------------
/**
 * @brief 初回クリア時のレコードを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateFirstClearRecord( GAMECLEAR_WORK* work )
{
  SAVE_CONTROL_WORK* save;
  RECORD* record;
  PLAYTIME* ptime;
  u32 hour, minute, second, time;

  save   = GAMEDATA_GetSaveControlWork( work->gamedata );
  record = GAMEDATA_GetRecordPtr( work->gamedata );
  ptime  = SaveData_GetPlayTime( save );

  // プレイ時間を取得
  hour   = PLAYTIME_GetHour( ptime );
  minute = PLAYTIME_GetMinute( ptime );
  second = PLAYTIME_GetSecond( ptime ); 
  GF_ASSERT( hour   <= PTIME_HOUR_MAX );
  GF_ASSERT( minute <= PTIME_MINUTE_MAX );
  GF_ASSERT( second <= PTIME_SECOND_MAX );

  // プレイ時間を登録
  time = hour * 10000 + minute * 100 + second;
  RECORD_Set( record, RECID_CLEAR_TIME, time );
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入り時のレコードを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateDendouRecord( GAMECLEAR_WORK* work )
{
  RECORD* record;

  record = GAMEDATA_GetRecordPtr( work->gamedata );
  RECORD_Add( record, RECID_DENDOU_CNT, 1 );
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入りデータの更新準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DendouSave_init( GAMECLEAR_WORK* work )
{
  RTCDate date;
  POKEPARTY* party;
  SAVE_CONTROL_WORK* save;
  DENDOU_SAVEDATA* dendouData;
  LOAD_RESULT result; 

  RTC_GetDate( &date );
  party = GAMEDATA_GetMyPokemon( work->gamedata ); 
  save = GAMEDATA_GetSaveControlWork( work->gamedata );
  result = SaveControl_Extra_Load( save, SAVE_EXTRA_ID_DENDOU, HEAPID_PROC );
  dendouData = SaveControl_Extra_DataPtrGet( save, SAVE_EXTRA_ID_DENDOU, EXGMDATA_ID_DENDOU );

  DendouData_AddRecord( dendouData, party, &date, HEAPID_PROC ); 
  SaveControl_Extra_SaveAsyncInit( save, SAVE_EXTRA_ID_DENDOU ); 

  work->dendouSaveInitFlag = TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入りデータの更新処理
 *
 * @param work 
 */
//-----------------------------------------------------------------------------
static void DendouSave_main( GAMECLEAR_WORK* work )
{
  SAVE_CONTROL_WORK* save;
  SAVE_RESULT save_ret;

  // セーブ実行
  save     = GAMEDATA_GetSaveControlWork( work->gamedata );
  save_ret = SaveControl_Extra_SaveAsyncMain( save, SAVE_EXTRA_ID_DENDOU ); 

  // セーブ終了
  if( save_ret == SAVE_RESULT_OK || save_ret == SAVE_RESULT_NG ) { 
    SaveControl_Extra_Unload( save, SAVE_EXTRA_ID_DENDOU ); // 外部データを解放
    work->dendouSaveFinishFlag = TRUE;
  }
}
