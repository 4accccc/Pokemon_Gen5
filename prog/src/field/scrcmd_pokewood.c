//============================================================================================
/**
 *	@file		scrcmd_pokewood.c
 *	@brief  ポケウッドスクリプトコマンド
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.06.30
 *
 */
//============================================================================================

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"


#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "fieldmap_func.h"

#include "scrcmd_pokewood.h"
#include "event_pokewood.h"

#include "arc/message.naix"

#include "app/pkwd_book.h"  //PKWD_BOOK_PARAM
#include "pokewood_setup.h" //PKWD_BOOK_PARAM_Create

#include "battle/pokewood.h"


//SDK_COMPILER_ASSERT( SCR_PKWD_SCENARIO_MAX == POKEWOOD_SENARIO_MAX );
//SDK_COMPILER_ASSERT( SCR_PKWD_MOVIE_MAX == 4 );

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//============================================================================================
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド録画アプリ呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT * next_event;
  POKEWOOD_EVENT_PARAM pw_param;
  u16* ret_wk;
  pw_param.type       = SCRCMD_GetVMWorkValue( core, work );
  pw_param.senario_no = SCRCMD_GetVMWorkValue( core, work );
  ret_wk = SCRCMD_GetVMWork(core,work);  

  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, (void*)&pw_param );
  SCRIPT_CallEvent( scw, next_event );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド上映アプリ呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  
  int rec_id =  SCRCMD_GetVMWorkValue( core, work );
  GMEVENT *next_event;
  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)&rec_id );
  SCRIPT_CallEvent( scw, next_event );

  return VMCMD_RESULT_SUSPEND;
}

// ----------------------------------------
// ----------------------------------------
static GMEVENT * createDummyEvent( void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

	GMEVENT *new_event;
	PKWD_BOOK_PARAM 	*pkwd_param = PKWD_BOOK_PARAM_Create( heapID, gsys, NULL );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BOOK_EVENT_Show, (void *)pkwd_param );

  return new_event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド台本画面呼び出し
 *	@todo 戻り値を受け取る処理を書き換える
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSelectScenario( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

	GMEVENT *new_event;
	PKWD_BOOK_PARAM 	*pkwd_param = PKWD_BOOK_PARAM_Create( heapID, gsys, ret_wk );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BOOK_EVENT_Show, (void *)pkwd_param );

  PKWD_BOOK_PARAM_Delete( pkwd_param );
	SCRIPT_CallEvent( scw, new_event );

  //*ret_wk = 0;  /* 仮：本当はアプリから結果を受け取る */

  return VMCMD_RESULT_SUSPEND;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド上映選択画面呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodMovies( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

  GMEVENT * new_event = createDummyEvent( wk );
	SCRIPT_CallEvent( scw, new_event );

  *ret_wk = 0;  /* 仮：本当はアプリから結果を受け取る */

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッドシナリオ名をバッファにセット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  u8               idx = VMGetU8(core);
  u16           msg_id = SCRCMD_GetVMWorkValue( core, work );
  HEAPID        heapID = SCRCMD_WORK_GetHeapID( work );

  {
    GFL_MSGDATA     *man;
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, heapID );
    if( man )
    {
      GFL_MSG_GetString( man, msg_id, strbuf );
      GFL_MSG_Delete( man );
    }
    WORDSET_RegisterWord( wordset, idx, strbuf, PM_NEUTRAL, TRUE, PM_LANG );
  }
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  台本追加条件チェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetScenarioStatus( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  /* 仮：シナリオ追加状態を入れる */
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　上映データ数取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetMoviesCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  /* 仮：上映可能なデータ数を追加 */
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  上映データ削除
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodDeleteMovie( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 delete_id = SCRCMD_GetVMWorkValue( core, work );
  /* 仮：削除処理をここに追加　*/

  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  初回視聴フラグ取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetFirstTimeSeeingFlag( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  /* 仮： */
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
}

#if 0
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk )
{
  return VMCMD_RESULT_CONTINUE;
}
#endif

