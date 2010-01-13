//============================================================================================
/**
 * @file	event_gameover.c
 * @brief	イベント：全滅処理
 * @since	2006.04.18
 * @date  2009.09.20
 * @author	tamada GAME FREAK inc.
 *
 * 2006.04.18 scr_tool.c,field_encount.cからゲームオーバー処理を持ってきて再構成した
 * 2009.09.20 HGSSから移植開始
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

#include "event_gameover.h"

#include "system/main.h"  //HEAPID_～

#include "system/brightness.h"

//#include "system/snd_tool.h"
//#include "fld_bgm.h"

//#include "system/window.h"
#include "system/wipe.h"

#include "warpdata.h"			//WARPDATA_～
#include "script.h"       //SCRIPT_CallScript
#include "event_fieldmap_control.h" //
#include "event_gamestart.h"
#include "event_mapchange.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h" //SCRID_～

//#include "situation_local.h"		//Situation_Get～
//#include "scr_tool.h"
//#include "mapdefine.h"
//#include "..\fielddata\script\common_scr_def.h"		//SCRID_GAME_OVER_RECOVER_PC
//#include "sysflag.h"
//#include "fld_flg_init.h"			//FldFlgInit_GameOver
//#include "system/savedata.h"
//#include "poketool/pokeparty.h"
//#include "poketool\status_rcv.h"

#include "field/field_msgbg.h"
#include "system/bmp_winframe.h"
//==============================================================================================
//
//	全滅関連
//
//==============================================================================================
//#include "system/fontproc.h"						
//#include "system/msgdata.h"							//MSGMAN_TYPE_DIRECT
//#include "system/wordset.h"							//WORDSET_Create
#include "print/wordset.h"

//#include "fld_bmp.h"						

//#include "msgdata/msg.naix"							//NARC_msg_??_dat
//#include "msgdata/msg_gameover.h"					//msg_all_dead_??
//#include "arc/
//#include "system/arc_util.h"
//#include "system/font_arc.h"
#include "font/font.naix" //NARC_font_large_gftr
#include "message.naix"
#include "msg/msg_gameover.h"

#include "arc/fieldmap/zone_id.h"

#include "pleasure_boat.h"

//==============================================================================================
//==============================================================================================


//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static REVIVAL_TYPE getRevType( GAMEDATA * gamedata )
{
  if(	WARPDATA_GetInitializeID() == GAMEDATA_GetWarpID(gamedata) ) {
    return REVIVAL_TYPE_HOME;
  } else {
    return REVIVAL_TYPE_POKECEN;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief	通常戦闘：全滅シーケンス
 * @param	event		イベント制御ワークへのポインタ
 * @retval	TRUE		イベント終了
 * @retval	FALSE		イベント継続中
 *
 * @todo
 * ポケセンでの復活処理を以前の仕様で作るためにはジョーイさんOBJの
 * OBJIDを何らかの方法で取得し、アニメーションさせる必要がある
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_NormalGameOver(GMEVENT * event, int * seq, void *work)
{
  GAMEOVER_WORK * param = work;

	switch (*seq) {
	case 0:
		//警告BG以外を表示オフ
		SetBrightness( BRIGHTNESS_BLACK, (PLANEMASK_ALL^PLANEMASK_BG3), MASK_MAIN_DISPLAY);
		SetBrightness( BRIGHTNESS_BLACK, PLANEMASK_ALL, MASK_SUB_DISPLAY);

		//ゲームオーバー警告
    GMEVENT_CallProc( event, FS_OVERLAY_ID(gameclear_demo), &GameOverMsgProcData, param );
		(*seq) ++;
		break;

	case 1:
		//イベントコマンド：フィールドマッププロセス復帰
    GMEVENT_CallEvent( event, EVENT_GameOver(param->gsys) );
		(*seq)++;
		break;

	case 2:
		//表示オフ解除
		SetBrightness( BRIGHTNESS_NORMAL, PLANEMASK_ALL, MASK_DOUBLE_DISPLAY);

		//気をつけてねスクリプト
		
		//話しかけ対象のOBJを取得する処理が必要になる
		//OS_Printf( "field_encount zone_id = %d\n", fsys->location->zone_id );
    if ( param->rev_type == REVIVAL_TYPE_HOME ) {
			//初期値のワープID＝＝最初の戻り先なので自分の家
			SCRIPT_CallScript( event, SCRID_GAMEOVER_RECOVER_HOME, NULL, NULL, HEAPID_FIELDMAP );
		}else{
			//それ以外＝＝ポケセンのはず
			SCRIPT_CallScript( event, SCRID_GAMEOVER_RECOVER_POKECEN, NULL, NULL, HEAPID_FIELDMAP );
		}
		(*seq) ++;
		break;

	case 6:
		//サウンドリスタート処理(06/07/10いらないので削除)
		//Snd_RestartSet( fsys );

		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}


//-----------------------------------------------------------------------------
/**
 * @brief	イベントコマンド：通常全滅処理
 * @param	event		イベント制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_NormalLose( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	GMEVENT * event;
  GAMEOVER_WORK * param;
  event = GMEVENT_Create( gsys, NULL, GMEVENT_NormalGameOver, sizeof(GAMEOVER_WORK) );
  param = GMEVENT_GetEventWork( event );
  param->gsys = gsys;
  param->rev_type = getRevType( gamedata );
  param->mystatus = GAMEDATA_GetMyStatus( gamedata );

  //遊覧船ワーク解放
  {
    PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);
    PL_BOAT_End(ptr);
  }
  return event;
}


