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

#include "system/main.h"  // for HEAPID_xxxx
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "sound/pm_sndsys.h"    // for PMSND_xxxx
#include "app/local_tvt_sys.h"  // for LocalTvt_ProcData etc.  
#include "demo/demo3d.h"        // for Demo3DProcData etc.
#include "demo/demo3d_demoid.h" // for DEMO3D_ID_xxxx
#include "demo/dendou_demo.h"

#include "script.h"           // for SCRIPT_CallGameClearScript
#include "event_gameclear.h"  // for GAMECLEAR_MODE
#include "event_field_fade.h"
#include "event_fieldmap_control_local.h"
#include "event_gamestart.h"
#include "event_demo3d.h"
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
  DENDOUDEMO_PARAM dendouDemoParam; // 殿堂入りデモ・パラメータ
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
  GMCLEAR_SEQ_DENDOU_DEMO_WAIT, // 殿堂入りデモ終了待ち
  GMCLEAR_SEQ_ENDING_DEMO,      // エンディングデモ
	GMCLEAR_SEQ_CLEAR_SCRIPT,	    // ゲームクリアスクリプト処理
	GMCLEAR_SEQ_SAVE_MESSAGE,	    // セーブ中メッセージ表示
	GMCLEAR_SEQ_END,				      // 終了
};


//============================================================================================
//============================================================================================
static u16 GetEndingDemoID( void ); // エンディングの3DデモIDを取得する


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
  GAMESYS_WORK * gsys = gcwk->gsys;
  GAMEDATA * gamedata = gcwk->gamedata;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( gamedata );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gcwk->gsys );
  GAME_COMM_SYS_PTR gameComm = GAMESYSTEM_GetGameCommSysPtr( gcwk->gsys );

	switch (*seq) {
	case GMCLEAR_SEQ_INIT:
    PMSND_FadeOutBGM( 30 );
		(*seq) ++;
		break;

  // フィールドマップをフェードアウト
  case GMCLEAR_SEQ_FADEOUT:
    GMEVENT_CallEvent( event, 
        EVENT_FieldFadeOut_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );

    //通信が動いている場合は終了させる
    if(GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL){
      GameCommSys_ExitReq(gameComm);
    }
		(*seq) ++;
    break;

  // 通信終了待ち
  case GMCLEAR_SEQ_COMM_END_WAIT:
    if( GameCommSys_BootCheck(gameComm) != GAME_COMM_NO_NULL ) {
      break;
    }
		(*seq) ++;
    break;

  // フィールドマップを終了待ち
  case GMCLEAR_SEQ_FIELD_CLOSE_WAIT:
    GMEVENT_CallEvent( event, EVENT_FieldClose_FieldProcOnly( gsys, fieldmap ) );
    
    // 初回はエンディングへ
    if( gcwk->clear_mode == GAMECLEAR_MODE_FIRST ) {
      *seq = GMCLEAR_SEQ_ENDING_DEMO;
    }
    // 2回目以降は殿堂入りデモへ
    else {
      *seq = GMCLEAR_SEQ_DENDOU_DEMO;
    }
    break;

  // 殿堂入りデモ呼び出し
  case GMCLEAR_SEQ_DENDOU_DEMO:
    GAMESYSTEM_CallProc( gsys, 
        FS_OVERLAY_ID(dendou_demo), &DENDOUDEMO_ProcData, &gcwk->dendouDemoParam );
		(*seq) ++;
		break; 
  // 殿堂入りデモ終了待ち
	case GMCLEAR_SEQ_DENDOU_DEMO_WAIT:  
		if( GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL ) { break; }
    *seq = GMCLEAR_SEQ_CLEAR_SCRIPT;
		break;

  // エンディングデモ
  case GMCLEAR_SEQ_ENDING_DEMO:
    GMEVENT_CallEvent( event, 
        EVENT_CallDemo3D( gsys, event,  GetEndingDemoID(), 0, FALSE ) );
    *seq = GMCLEAR_SEQ_CLEAR_SCRIPT;
    break;

	case GMCLEAR_SEQ_CLEAR_SCRIPT:
    SCRIPT_CallGameClearScript( gsys, HEAPID_PROC ); 
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

  // 殿堂入りデモパラメータを設定
  gcwk->dendouDemoParam.party    = GAMEDATA_GetMyPokemon( gamedata );
  gcwk->dendouDemoParam.mystatus = GAMEDATA_GetMyStatus( gamedata );
  gcwk->dendouDemoParam.ptime    = GAMEDATA_GetPlayTimeWork( gamedata );

  return event;
} 



//==============================================================================================
//==============================================================================================

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
  case VERSION_WHITE: demo_id = DEMO3D_ID_ENDING_W; break;
  case VERSION_BLACK: demo_id = DEMO3D_ID_ENDING_B; break;
  }

  return demo_id;
}
