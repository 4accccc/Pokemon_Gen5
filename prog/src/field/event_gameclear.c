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
#include "event_mapchange.h"

#include "demo/demo3d.h"  //Demo3DProcData etc.
#include "app/local_tvt_sys.h"  //LocalTvt_ProcData etc.

#include "sound/pm_sndsys.h"  //PMSND_

#include "move_pokemon.h"

#include "proc_gameclear_save.h"

//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMECLEAR_MODE clear_mode;
  BOOL saveSuccessFlag;
  const MYSTATUS * mystatus;
  void * pWork;
}GAMECLEAR_WORK;


//============================================================================================
//============================================================================================
// メインシーケンス
enum {
	GMCLEAR_SEQ_INIT = 0,			// 初期化
	GMCLEAR_SEQ_DENDOU_DEMO,		// 殿堂入りデモ
	GMCLEAR_SEQ_SAVE_START,			// セーブ開始
	GMCLEAR_SEQ_SAVE_MESSAGE,		// セーブ中メッセージ表示
	GMCLEAR_SEQ_SAVE_MAIN,			// セーブメイン
	GMCLEAR_SEQ_SAVE_END,			// セーブ終了
	GMCLEAR_SEQ_WAIT1,				// ウェイト１
	GMCLEAR_SEQ_FADE_OUT,			// フェードアウトセット
	GMCLEAR_SEQ_BGM_FADE_OUT,		// BGMフェードアウトセット
	GMCLEAR_SEQ_WAIT2,				// ウェイト２
	GMCLEAR_SEQ_ENDING_DEMO,		// エンディングデモ
	GMCLEAR_SEQ_END,				// 終了
};



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
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gcwk->gsys );

	switch (*seq) {
	case GMCLEAR_SEQ_INIT:
    PMSND_FadeOutBGM( 30 );
    { //フィールドマップを終了させる
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gcwk->gsys );
      GMEVENT * new_event = DEBUG_EVENT_GameEnd( gcwk->gsys, fieldmap );
      GMEVENT_CallEvent( event, new_event );
    }
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_DENDOU_DEMO:
    if ( PMSND_CheckFadeOnBGM() == FALSE ) break;
    GFL_FADE_SetMasterBrightReq( 
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
        0, 16, -16 );
    SCRIPT_CallGameClearScript( gcwk->gsys, HEAPID_PROC );
#if 0
    { //殿堂入りの代わりにNの城デモ
      DEMO3D_PARAM * param = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(DEMO3D_PARAM) );
      param->demo_id = DEMO3D_ID_N_CASTLE;
      GMEVENT_CallProc( event, FS_OVERLAY_ID(demo3d), &Demo3DProcData, param );
      gcwk->pWork = param;
    }
#endif

		(*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_START:

    //「上書きします」とか表示する予定

		//
    //GAMEDATA_SaveAsyncStart( gamedata );
    (*seq) ++;
		break;

  case GMCLEAR_SEQ_SAVE_MESSAGE:
    GMEVENT_CallProc( event, FS_OVERLAY_ID(gameclear_demo), &GameClearMsgProcData, gcwk->gsys );
    (*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_MAIN:
    return GMEVENT_RES_FINISH;
		break;

		(*seq) ++;
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
  gcwk->clear_mode = mode;
  gcwk->mystatus = GAMEDATA_GetMyStatus( gamedata );

  //移動ポケモン復活チェック
  MP_RecoverMovePoke( gamedata );

  return event;
}


