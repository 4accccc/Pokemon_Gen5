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
#include "event_field_fade.h"
#include "event_fieldmap_control_local.h"

#include "system/main.h"  //HEAPID_～

#include "script.h"       //SCRIPT_CallScript
#include "event_gamestart.h"

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
  GAMEDATA* gamedata;
  GAMECLEAR_MODE clear_mode;
  BOOL saveSuccessFlag;
  const MYSTATUS * mystatus;

  GAMECLEAR_MSG_PARAM para_child;
}GAMECLEAR_WORK;


//============================================================================================
//============================================================================================
// メインシーケンス
enum {
	GMCLEAR_SEQ_INIT,			        // 初期化
  GMCLEAR_SEQ_FADEOUT,          // フェードアウト
  GMCLEAR_SEQ_COMM_END_WAIT,    // 通信終了待ち
  GMCLEAR_SEQ_FIELD_CLOSE_WAIT, // フィールドマップ終了待ち
	GMCLEAR_SEQ_DENDOU_DEMO,	    // 殿堂入りデモ
	GMCLEAR_SEQ_SAVE_MESSAGE,	    // セーブ中メッセージ表示
	GMCLEAR_SEQ_END,				      // 終了
};


//============================================================================================
//============================================================================================

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
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gcwk->gsys );
  GAME_COMM_SYS_PTR gameComm = GAMESYSTEM_GetGameCommSysPtr( gcwk->gsys );

	switch (*seq) {
	case GMCLEAR_SEQ_INIT:
    PMSND_FadeOutBGM( 30 );
		(*seq) ++;
		break;

  case GMCLEAR_SEQ_FADEOUT:
    //フィールドマップをフェードアウト
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeOut_Black( gcwk->gsys, fieldmap, FIELD_FADE_WAIT ) );

    //通信が動いている場合は終了させる
    if(GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL){
      GameCommSys_ExitReq(gameComm);
    }
		(*seq) ++;
    break;

  case GMCLEAR_SEQ_COMM_END_WAIT:
    //通信終了待ち
    if( GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL ) {
      break;
    }
		(*seq) ++;
    break;

  case GMCLEAR_SEQ_FIELD_CLOSE_WAIT:
    //フィールドマップを終了待ち
    GMEVENT_CallEvent( event, 
        EVENT_FieldClose_FieldProcOnly( gcwk->gsys, fieldmap ) );
		(*seq) ++;
    break;

	case GMCLEAR_SEQ_DENDOU_DEMO:
    SCRIPT_CallGameClearScript( gcwk->gsys, HEAPID_PROC ); 
		(*seq) ++;
		break;

  case GMCLEAR_SEQ_SAVE_MESSAGE:
    GMEVENT_CallProc(
        event, FS_OVERLAY_ID(gameclear_demo), &GameClearMsgProcData, &gcwk->para_child );
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
  gcwk->gamedata = gamedata;
  gcwk->clear_mode = mode;
  gcwk->mystatus = GAMEDATA_GetMyStatus( gamedata );

  //子プロセスに渡すパラメータ
  gcwk->para_child.gsys = gsys;
  gcwk->para_child.clear_mode = mode;

  //移動ポケモン復活チェック
  MP_RecoverMovePoke( gamedata );

  return event;
} 
